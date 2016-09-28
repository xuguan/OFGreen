//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include <omnetpp.h>
#include "ForwardingEXR.h"

#include "openflow.h"
#include "OFP_Packet_In_m.h"
#include "OF_Wrapper.h"

#include "OFA_controller.h"
#include "Open_Flow_Message_m.h"
#include "IPvXAddressResolver.h"
#include <algorithm>
#include "IRoutingTable.h"
#include "IInterfaceTable.h"
#include "IPv4Datagram.h"
#include "InterfaceEntry.h"
#include "IPv4InterfaceData.h"
#include "EtherMAC.h"
#include "IPv4Address.h"
#include "ARPPacket_m.h"
#include "northboundMsg_m.h"
using namespace std;

Define_Module(ForwardingEXR);

ForwardingEXR::ForwardingEXR() {

}

ForwardingEXR::~ForwardingEXR() {

}

void ForwardingEXR::initialize(int stage) {
    if (stage == 0) {
        EV << "ForwardingEXR: Initialize Stage 0" << endl;
        cModule *ITModule = getParentModule()->getSubmodule("ofa_controller");
        controller = check_and_cast<OFA_controller *>(ITModule);

        // RegisterSignal is here only used to get signal ID
        // This module does not emit any events for these signals
        connIDSignal = registerSignal("connID");
        PacketInSignalId = registerSignal("PacketIn");
//        BandwidthReqSignalId = registerSignal("BandwidthReq");
//        BandwidthResSignalId = registerSignal("BandwidthRes");

        // Register to these signals as they are used for communication between ctrlApp and ofa_controller
        // Subscribe one level in hierarchy above because signals are only propagated upwards
        getParentModule()->subscribe(connIDSignal, this);
        getParentModule()->subscribe(PacketInSignalId, this);
//        getParentModule()->getParentModule()->subscribe(BandwidthReqSignalId,this);

        // Does this controller app answer ARP requests?
        isArpOracle = par("isArpOracle").boolValue();

        WATCH_VECTOR(nodeInfo);

    }

    if (stage == 4) {
        EV << "ForwardingEXR: Initialize Stage 4" << endl;

        extractTopology(topo_forw, nodeInfo);

        assignAddresses(topo_forw, nodeInfo);
        linkpaths = new LinkPaths;
//        routingrequestqueue = new ROUTINGrequests;
//        routingrequests = new ROUTINGrequests;
//        routingrequestqueuewrapper = new ROUTINGrequests;
        nbreqqueue = new NBREQs;
        nbreqs = new NBREQs;

    }
}

//bool selectFunction(cModule *mod, void *){
//
//    return strcmp(mod->getNedTypeName(),"openflow.nodes.StandardHostRandom")==0
//            || strcmp(mod->getNedTypeName(),"openflow.nodes.Open_Flow_Switch")==0;
//
//}

//Only the truth return can allow the module imported
bool selectFunctionForwardingEXR(cModule *mod, void *v_ptr) {
    int id = *(int *) v_ptr;
    int domainID = -1;

    if (mod->hasPar("doaminID")) {
        domainID = mod->par("domainID").longValue();
    } else if (mod->getParentModule()->hasPar("doaminID")) {
        domainID = mod->getParentModule()->par("domainID").longValue();
    }

    return (strcmp(mod->getNedTypeName(), "inet.nodes.inet.StandardHost") == 0
            || strcmp(mod->getNedTypeName(), "openflow.nodes.Open_Flow_Switch")
                    == 0) && domainID == id;
}

void ForwardingEXR::handleMessage(cMessage* msg) {
    EV << "handle the msg from" << msg->getSenderModule()->getFullPath().c_str()
            << endl;
    if (msg->isSelfMessage()) {
        EV << msg->getArrivalModule()->getFullName();
        return;
    }
    northboundMsg * northboundmsg = check_and_cast<northboundMsg *>(msg);
    oxm_basic_match match = northboundmsg->getMatch();
    IPv4Address IPv4src = match.OFB_IPV4_SRC;
    IPv4Address IPv4dst = match.OFB_IPV4_DST;
//    cModule * clientModule = msg->getSenderModule();
    cModule * clientModule = (cModule *) northboundmsg->getClientModuleptr();

//1. new request
    if (!northboundmsg->getCanSend() && !northboundmsg->getCanEnd()) {

//1.1 request reservation
        bool isWeightedNode = par("isWeightedNode");
        bool routeFound = findAndReserveRouting(IPv4src, IPv4dst,
                isWeightedNode);
        if (routeFound) {

//1.1.1 succeed, allowing the flow through the path
            EV << "starting" << IPv4src << "to" << IPv4dst << endl;
            northboundmsg->setCanSend(1);
            nbreqs->push_back(northboundmsg);
            sendDirect(northboundmsg, clientModule->gate("nbIn"));
        } else {
//1.1.2 failed, suspending the flow
            EV << "queueing" << IPv4src << "to" << IPv4dst << endl;
//            routingrequestqueue->push_back(wrapper);
            nbreqqueue->push_back(northboundmsg);
        }
    } else if (northboundmsg->getCanSend() && northboundmsg->getCanEnd()) {
//2 request release

        EV << "receiving finishing ctrl msg from " << IPv4src << " to "
                << IPv4dst << endl;
        uint i = 0;
        bool loopback = 0;
//2.1 get the current path for the thasrc, thadst
        for (; i < nbreqs->size(); i++) {
            if (nbreqs->at(i) == northboundmsg) {
                //TODO: there is a routing reservation already
                EV << "finishing" << IPv4dst << "to" << IPv4src << endl;
                break;
            }
        }
        if (i == nbreqs->size()) {
            //TODO: error delete unexisting routing
            EV << "the finishing flow is not in the active queue!" << endl;
            return;
        } else {
//2.2 restore the path
            EV << "release the reserved links, with length "
                    << linkpaths->at(i)->size() << endl;
            if (linkpaths->at(i)->size() == 0)
                loopback = 1;
            //recycle the links by enable them
            for (uint j = 0; j < linkpaths->at(i)->size(); j++) {
                Topology::LinkOut *curlink = linkpaths->at(i)->at(j);
                //TODO: print the link
                EV << "enable link"
                        << curlink->getRemoteNode()->getModule()->getName()
                        << endl;
                if (curlink->isEnabled()) {
                    //TODO: error, the link should be already disabled
                    EV << "The link is already enabled" << endl;
                    return;
                }
                curlink->enable();
            }
            //
            nbreqs->erase(nbreqs->begin() + i);
            LinkPaths::iterator todelpathiter = linkpaths->begin() + i;
            delete *todelpathiter;
            linkpaths->erase(todelpathiter);
        }
        if (!loopback) {
            uint queuesize = nbreqqueue->size();

            EV << "queue size " << queuesize;

            if (queuesize == 0) {
                EV << ", now return" << endl;
                return;
            }
//2.3 check suspended request
            EV << ", now check queued request." << endl;
            NBREQs::iterator nbreqs_iter = nbreqqueue->begin();
            while (nbreqs_iter != nbreqqueue->end()) {
                oxm_basic_match match = (*nbreqs_iter)->getMatch();
                IPv4Address ipv4src = match.OFB_IPV4_SRC;
                IPv4Address ipv4dst = match.OFB_IPV4_DST;
                if (findAndReserveRouting(ipv4src, ipv4dst)) {
                    cModule * clientMod =
                            (cModule *) (*nbreqs_iter)->getClientModuleptr();
                    nbreqs->push_back(*nbreqs_iter);
                    (*nbreqs_iter)->setCanSend(1);
                    EV << "restarting" << ipv4src << "to" << ipv4dst << endl;
//                    emit(BandwidthResSignalId, (*nbreqs_iter));
                    sendDirect((*nbreqs_iter), clientMod->gate("nbIn"));
                    nbreqqueue->erase(nbreqs_iter);
                    continue;
                }
                nbreqs_iter++;
            }
        }
        return;
    } else {
        EV << "error northbound msg!" << endl;
    }
//    sendDirect(msg, msg->getSenderModule()->gate("nbIn"));
}

void ForwardingEXR::extractTopology(Topology &topo, NodeInfoVector &nodeInfo) {

    // We either have knowledge about the entire network or only about our own domain
    if (par("domainID").longValue() >= 0) {
        int domainID = getParentModule()->par("domainID").longValue();
        topo.extractFromNetwork(selectFunctionForwardingEXR, (int *) &domainID);
        EV << "Topology found " << topo.getNumNodes() << " nodes for domain "
                << domainID << ".\n";
    } else {
        // Use specified ned type to extract relevant nodes
        topo.extractByNedTypeName(cStringTokenizer(par("nedTypes")).asVector());
        EV << "Topology found " << topo.getNumNodes() << " nodes\n";
    }

    double nodeweight = par("nodeweight");
    nodeInfo.resize(topo.getNumNodes());
    for (int i = 0; i < topo.getNumNodes(); i++) {
        cModule *mod = topo.getNode(i)->getModule();
        nodeInfo[i].isIPNode = IPvXAddressResolver().findInterfaceTableOf(mod)
                != NULL;
        nodeInfo[i].isOpenFlow = (mod->findSubmodule("flow_Table") != -1);
        nodeInfo[i].name = topo.getNode(i)->getModule()->getFullPath();
        if (nodeInfo[i].isIPNode) {
            nodeInfo[i].ift = IPvXAddressResolver().interfaceTableOf(mod);
            nodeInfo[i].rt = IPvXAddressResolver().routingTableOf(mod);
        }
        if (nodeInfo[i].isOpenFlow) {
            EV << "extractTopology: OF-Switch found" << endl;
            topo.getNode(i)->setWeight(nodeweight);
        }
    }
}

void ForwardingEXR::assignAddresses(Topology &topo, NodeInfoVector &nodeInfo) {
    for (int i = 0; i < topo.getNumNodes(); i++) {
        if (!nodeInfo[i].isIPNode)
            continue;

        IInterfaceTable *ift = nodeInfo[i].ift;
        for (int k = 0; k < ift->getNumInterfaces(); k++) {
            InterfaceEntry *ie = ift->getInterface(k);
            if (!ie->isLoopback()) {
                IPv4Address addr = ie->ipv4Data()->getIPAddress();
                nodeInfo[i].address = addr;
                nodeInfo[i].macAddress = ie->getMacAddress();
                break;
            }
        }
    }
}

void ForwardingEXR::receiveSignal(cComponent *src, simsignal_t id,
        cObject *obj) {
    EV << "Received signal " << getSignalName(id) << " at module "
            << this->getFullName() << "." << endl;
    if (id == PacketInSignalId) {

        if (dynamic_cast<OF_Wrapper *>(obj) != NULL) {
            OF_Wrapper *wrapper = (OF_Wrapper *) obj;
            Open_Flow_Message *of_msg = wrapper->msg;
            OFP_Packet_In *packet_in = (OFP_Packet_In *) of_msg;
            uint32_t buffer_id = packet_in->getBuffer_id();

            MACAddress src_mac;
            MACAddress dst_mac;
            IPv4Address dst_ip;
            if (buffer_id == OFP_NO_BUFFER) {

                src_mac =
                        dynamic_cast<EthernetIIFrame *>(packet_in->getEncapsulatedPacket())->getSrc();
                dst_mac =
                        dynamic_cast<EthernetIIFrame *>(packet_in->getEncapsulatedPacket())->getDest();
            } else {

                src_mac = packet_in->getMatch().OFB_ETH_SRC;
                dst_mac = packet_in->getMatch().OFB_ETH_DST;
            }
            TCPCommand *ind =
                    dynamic_cast<TCPCommand *>(packet_in->getControlInfo());

            if (!ind)
                opp_error(
                        "Switch: no TCPCommand control info in message (not from TCP?)");
            int connID = ind->getConnId();
            EV << "connID: " << connID << endl;

            int outputGateId;

            bool routeFound = 0;

            for (int i = 0; i < topo_forw.getNumNodes(); i++) {
                Topology::Node *destNode = topo_forw.getNode(i);

                EtherMAC *eth =
                        (EtherMAC *) destNode->getModule()->getSubmodule("eth",
                                0)->getSubmodule("mac");
                MACAddress mac = eth->getMACAddress();

                // Skip forwarding for Ethernet frames with braodcast destination address
                if (mac.isBroadcast()) {
                    break;
                }

                if (mac == dst_mac) {
                    routeFound = 1;
                    std::string destModName =
                            destNode->getModule()->getFullName();

                    topo_forw.calculateUnweightedSingleShortestPathsTo(
                            destNode);

                    Topology::Node *nextNode = NULL;

                    while (nextNode != destNode) {

                        for (int j = 0; j < topo_forw.getNumNodes(); j++) {
                            if (i == j)
                                continue;
                            // Only choose OpenFlow switches, otherwise proceed to next node in for loop
                            if (!nodeInfo[j].isOpenFlow)
                                continue;

                            Topology::Node *atNode = topo_forw.getNode(j);
                            // Only choose nodes which have shortest path to destination node
                            if (atNode->getNumPaths() == 0)
                                continue;

                            // Only proceed if nextNode has not been chosen yet or atNode is equal to nextNode
                            if (nextNode != NULL && atNode != nextNode)
                                continue;

                            // Only proceed if connection id of atNode is equal to connection id of OpenFlow switch which has sent packet-in message
                            // or nextNode has been chosen
                            if (nextNode == NULL
                                    && nodeInfo[j].connID != connID)
                                continue;

                            outputGateId =
                                    atNode->getPath(0)->getLocalGate()->getIndex();
                            uint32_t outport = outputGateId;
                            EV << "--";
                            EV << atNode->getModule()->getFullName() << endl;
                            EV << "port" << outport;
                            EV << " to node "
                                    << atNode->getPath(0)->getRemoteNode()->getModule()->getFullName()
                                    << endl;
                            nextNode = atNode->getPath(0)->getRemoteNode();

                            connID_outport.push_back(
                                    std::pair<int, uint32_t>(nodeInfo[j].connID,
                                            outport));

                        }
                    }
                    typedef UnsortedMap::iterator iter;
                    iter begin(connID_outport.begin());
                    iter end(connID_outport.end());
//                    std::map<int, uint32_t>::reverse_iterator i_first = connID_outport.end();
                    reverse_iterator<iter> rev_end(begin);
                    reverse_iterator<iter> rev_iterator(end);
//                    std::map<int, uint32_t>::reverse_iterator i_last = connID_outport.begin();
                    while (rev_iterator != rev_end) {
                        oxm_basic_match *match = new oxm_basic_match();
                        if (buffer_id == OFP_NO_BUFFER) {

                            //                            match->OFB_IN_PORT = packet_in->getEncapsulatedPacket()->getArrivalGate()->getIndex();
                            match->OFB_ETH_SRC =
                                    dynamic_cast<EthernetIIFrame *>(packet_in->getEncapsulatedPacket())->getSrc();
                            match->OFB_ETH_DST =
                                    dynamic_cast<EthernetIIFrame *>(packet_in->getEncapsulatedPacket())->getDest();
                            match->OFB_ETH_TYPE =
                                    dynamic_cast<EthernetIIFrame *>(packet_in->getEncapsulatedPacket())->getEtherType();

                        } else {

                            //                            match->OFB_IN_PORT = packet_in->getMatch().OFB_IN_PORT;
                            match->OFB_ETH_SRC =
                                    packet_in->getMatch().OFB_ETH_SRC;
                            match->OFB_ETH_DST =
                                    packet_in->getMatch().OFB_ETH_DST;
                            match->OFB_ETH_TYPE =
                                    packet_in->getMatch().OFB_ETH_TYPE;
                        }
                        match->wildcards = OFPFW_IN_PORT + OFPFW_TP_DST
                                + OFPFW_TP_SRC + OFPFW_NW_PROTO;
                        //                                    OFPFW_IN_PORT + OFPFW_DL_TYPE + OFPFW_TP_DST+ OFPFW_TP_SRC + OFPFW_TP_DST + OFPFW_TP_SRC;

//                        EV << "nodeinfo.connid: " << nodeInfo[j].connID<< "connID: " << connID << endl;
//                        controller->sendFlowModMessage(OFPFC_ADD, match, outport, nodeInfo[j].connID);
//                        if (nodeInfo[j].connID == connID)
//                            controller->sendPacket(buffer_id, packet_in, outport, connID);

                        controller->sendFlowModMessage(OFPFC_ADD, match,
                                rev_iterator->second, rev_iterator->first);
                        if (rev_iterator->first == connID)
                            controller->sendPacket(buffer_id, packet_in,
                                    rev_iterator->second, connID);
                        rev_iterator++;
                    }
                    connID_outport.clear();
                }
            }

            if (!routeFound && isArpOracle) {
                routeFound = processARPPacket(packet_in, connID);

            }

            if (routeFound == 0) {
                controller->floodPacket(buffer_id, packet_in, connID);
            }
        }

    } else if (id == connIDSignal) {
        if (dynamic_cast<OF_Wrapper *>(obj) != NULL) {
            OF_Wrapper *wrapper = (OF_Wrapper *) obj;
            int connID = wrapper->connID;
            IPv4Address *ip_src = wrapper->ip_src;
            for (int i = 0; i < topo_forw.getNumNodes(); i++) {
                //                cTopology::Node *destNode = topo_forw.getNode(i);
                //                EtherMAC *eth =  (EtherMAC *) destNode->getModule()->getSubmodule("eth", 0)->getSubmodule("mac");
                //                                MACAddress mac = eth->getMACAddress();
                IPv4Address *ip = &nodeInfo[i].address;
                //                    EV << "-----------------------------------------------------------------" << endl;
                //                    EV << *ip << " "<< *ip_src << endl;
                if (*ip == *ip_src) {
                    nodeInfo[i].connID = connID;
                }
            }
        }

    }
//    else if (id == BandwidthReqSignalId) {
//        if (dynamic_cast<northboundWrapper *>(obj) != NULL) {
//            northboundWrapper* wrapper = (northboundWrapper *) obj;
//            IPv4Address IPv4src = wrapper->ip_src.get4();
//            IPv4Address IPv4dst = wrapper->ip_dst.get4();
//
//            bool loopback = IPv4src.equals(IPv4dst);
//
//            EV << "ipv4src" << IPv4src << "ipv4dst" << IPv4dst << endl;
////            MACAddress thasrc = getMACfromIPv4(IPv4src);
////            MACAddress thadst = getMACfromIPv4(IPv4dst);
//
//            if (wrapper->canEnd && wrapper->canSend) {
//                //TODO: get the current path for the thasrc, thadst
//                //TODO: restore the path
//                //TODO:
//                uint i = 0;
//                for (; i < routingrequests->size(); i++) {
//                    if (routingrequests->at(i)->ip_dst.get4().equals(IPv4dst)
//                            && routingrequests->at(i)->ip_src.get4().equals(
//                                    IPv4src)) {
//                        //TODO: there is a routing reservation already
//                        EV << "finishing" << IPv4dst << "to" << IPv4src << endl;
//                        break;
//                    }
//                }
//                if (i == routingrequests->size()) {
//                    //TODO: error delete unexisting routing
//                    EV << "the flow is not routed before in the controller!"
//                            << endl;
//                    return;
//                } else {
//                    EV << "release the reserved links, with length "
//                            << linkpaths->at(i)->size() << endl;
//                    //recycle the links by enable them
//                    for (uint j = 0; j < linkpaths->at(i)->size(); j++) {
//                        cTopology::LinkOut *curlink = linkpaths->at(i)->at(j);
//                        //TODO: print the link
//                        EV << "enable link"
//                                << curlink->getRemoteNode()->getModule()->getName()
//                                << endl;
//                        if (curlink->isEnabled()) {
//                            //TODO: error, the link should be already disabled
//                            EV << "The link is already enabled" << endl;
//                            return;
//                        }
//                        curlink->enable();
//                    }
//                    //
//                    routingrequests->erase(routingrequests->begin() + i);
//                    linkpaths->erase(linkpaths->begin() + i);
//                }
//                if (!loopback) {
//                    uint queuesize = routingrequestqueue->size();
//                    EV << "queue size " << queuesize
//                            << ", now check queued request." << endl;
//                    std::vector<northboundWrapper*>::iterator wrapper_iter =
//                            routingrequestqueue->begin();
//                    while (wrapper_iter != routingrequestqueue->end()) {
//                        if (findAndReserveRouting(
//                                (*wrapper_iter)->ip_src.get4(),
//                                (*wrapper_iter)->ip_dst.get4())) {
//                            routingrequests->push_back(*wrapper_iter);
//                            routingrequestqueue->erase(wrapper_iter);
//                            (*wrapper_iter)->canSend = 1;
//                            EV << "restarting" << (*wrapper_iter)->ip_src.get4()
//                                    << "to" << (*wrapper_iter)->ip_dst.get4()
//                                    << endl;
//                            emit(BandwidthResSignalId, (*wrapper_iter));
//                        }
//                        wrapper_iter++;
//                    }
//                }
//                return;
////                routingrequestqueue->push_back(wrapper);
//            }
//
//            //TODO: find the path
//            //TODO: reserve the path
//
//            bool routeFound = findAndReserveRouting(IPv4src, IPv4dst);
////            bool routeFound = 0;
////
////            for (int i = 0; i < topo_forw.getNumNodes(); i++) {
////                cTopology::Node *destNode = topo_forw.getNode(i);
////
////                EtherMAC *eth =
////                        (EtherMAC *) destNode->getModule()->getSubmodule("eth",
////                                0)->getSubmodule("mac");
////                MACAddress mac = eth->getMACAddress();
////
////                // Skip forwarding for Ethernet frames with braodcast destination address
////                if (mac.isBroadcast()) {
////                    break;
////                }
////
////                if (mac == thadst) {
////                    routeFound = 1;
////                    LinkPath *curlinkpath = new LinkPath;
////                    linkpaths->push_back(curlinkpath);
////                    std::string destModName =
////                            destNode->getModule()->getFullName();
////
////                    topo_forw.calculateUnweightedSingleShortestPathsTo(
////                            destNode);
////
////                    cTopology::Node *nextNode = NULL;
////
////                    while (nextNode != destNode) {
////                        for (int j = 0; j < topo_forw.getNumNodes(); j++) {
////                            // Only choose OpenFlow switches, otherwise proceed to next node in for loop
////                            if (i == j || !nodeInfo[j].isOpenFlow)
////                                continue;
////                            cTopology::Node *atNode = topo_forw.getNode(j);
////
////                            // Only choose nodes which have shortest path to destination node
////                            if (atNode->getNumPaths() == 0)
////                                continue;
////                            // Only proceed if nextNode has not been chosen yet or atNode is equal to nextNode
////                            if (nextNode != NULL && atNode != nextNode)
////                                continue;
////                            // Only proceed if connection id of atNode is equal to connection id of OpenFlow switch which has sent packet-in message
////                            // or nextNode has been chosen
////                            if (nextNode == NULL)
////                                continue;
////
////                            //TODO: the link to be deleted
////                            cTopology::LinkOut* atlink = atNode->getPath(0);
////                            curlinkpath->push_back(atlink);
////                            atlink->disable();
////
////                            uint32_t outputGateId =
////                                    atNode->getPath(0)->getLocalGate()->getIndex();
////                            uint32_t outport = outputGateId;
////                            EV << "--------------------------------------";
////                            EV << "shortest path for : "
////                                    << atNode->getModule()->getFullName()
////                                    << endl;
////                            EV << outport << endl;
////                            EV << "remote node: "
////                                    << atNode->getPath(0)->getRemoteNode()->getModule()->getFullName()
////                                    << endl;
////                            nextNode = atNode->getPath(0)->getRemoteNode();
////
////                            connID_outport.push_back(
////                                    std::pair<int, uint32_t>(nodeInfo[j].connID,
////                                            outport));
////
////                        }
////                    }
////                    typedef UnsortedMap::iterator iter;
////                    iter begin(connID_outport.begin());
////                    iter end(connID_outport.end());
////                    reverse_iterator<iter> rev_end(begin);
////                    reverse_iterator<iter> rev_iterator(end);
////                    while (rev_iterator != rev_end) {
////                        oxm_basic_match *match = new oxm_basic_match();
////                        match->OFB_ETH_SRC = thasrc;
////                        match->OFB_ETH_DST = thadst;
//////                        match->OFB_ETH_TYPE=
////                        match->wildcards = OFPFW_IN_PORT + OFPFW_DL_TYPE;
////                        controller->sendFlowModMessage(OFPFC_ADD, match,
////                                rev_iterator->second, rev_iterator->first);
////                        rev_iterator++;
////                    }
////                    connID_outport.clear();
////                }
////            }
//
//            if (routeFound) {
//                EV << "starting" << IPv4src << "to" << IPv4dst << endl;
//                routingrequests->push_back(wrapper);
//                wrapper->canSend = 1;
//                emit(BandwidthResSignalId, wrapper);
//            } else {
//                EV << "queueing" << IPv4src << "to" << IPv4dst << endl;
//                routingrequestqueue->push_back(wrapper);
//            }
//
//        }
//    }

}

//find a path, reserve it
bool ForwardingEXR::findAndReserveRoutingFromsrcTodst(IPv4Address IPv4src,
        IPv4Address IPv4dst, bool isweight, bool enableWeightChange,
        bool enableReserve) {
    EV << "findAndReserveRoutingFromsrcTodst! " << IPv4src << " --> " << IPv4dst
            << endl;
    if (IPv4src.equals(IPv4dst)) {
        LinkPath *curlinkpath = new LinkPath;
        linkpaths->push_back(curlinkpath);
        return true;
    }
    MACAddress thasrc = getMACfromIPv4(IPv4src);
    MACAddress thadst = getMACfromIPv4(IPv4dst);
//    EV << "macsrc" << thasrc << "macdst" << thadst << endl;
    int routeFound = 0;

    Topology::Node * srcNode;
    Topology::Node * destNode;
    for (int i = 0; i < topo_forw.getNumNodes(); i++) {

        Topology::Node *curNode = topo_forw.getNode(i);
        //ignore ovs
        if (curNode->getModule()->getFullName()[0] == 'o')
            continue;

        EtherMAC *eth = (EtherMAC *) curNode->getModule()->getSubmodule("eth",
                0)->getSubmodule("mac");
        MACAddress mac = eth->getMACAddress();
        if (mac == thadst) {
            destNode = curNode;
            routeFound += 1;
        } else if (mac == thasrc) {
            srcNode = curNode;
            routeFound += 1;
        }
    }

//    EV << "node" << destNode->getModule()->getFullName() << "mac" << mac
//            << endl;

    if (routeFound == 2) {

        LinkPath *curlinkpath = NULL;
        if (enableReserve) {
            curlinkpath = new LinkPath;
            linkpaths->push_back(curlinkpath);
        }
        std::string destModName = destNode->getModule()->getFullName();

        if (isweight)
            topo_forw.calculateWeightedSingleShortestPathsTo(destNode);
        else
            topo_forw.calculateUnweightedSingleShortestPathsTo(destNode);

        if (srcNode->getNumPaths() == 0)
            return false;
        Topology::Node *nextNode = srcNode->getPath(0)->getRemoteNode();

        while (nextNode != destNode) {
            for (int j = 0; j < topo_forw.getNumNodes(); j++) {
                // Only choose OpenFlow switches, otherwise proceed to next node in for loop
                if (!nodeInfo[j].isOpenFlow)
                    continue;
                Topology::Node *atNode = topo_forw.getNode(j);

                // Only choose nodes which have shortest path to destination node
                if (atNode->getNumPaths() == 0)
                    continue;
                // Only proceed if nextNode has not been chosen yet or atNode is equal to nextNode
                if (nextNode != NULL && atNode != nextNode)
                    continue;
                // Only proceed if connection id of atNode is equal to connection id of OpenFlow switch which has sent packet-in message
                // or nextNode has been chosen
//                    if (nextNode == NULL && nodeInfo[j].connID != connID)
//                        continue;

                //TODO: the link to be deleted
                Topology::LinkOut* atlink = atNode->getPath(0);
                //servers are initialized zero.
                if (enableWeightChange) {
                    Topology::Node * atnode = atlink->getRemoteNode();
                    double atweight = atnode->getWeight();
                    if (atweight != 0)
                        atnode->setWeight(atweight / 2);
                }
                if (enableReserve) {
                    curlinkpath->push_back(atlink);
                    atlink->disable();
                    EV << "disable link " << atNode->getModule()->getName()
                            << " to "
                            << atlink->getRemoteNode()->getModule()->getName()
                            << endl;
                }
                uint32_t outputGateId =
                        atNode->getPath(0)->getLocalGate()->getIndex();
                uint32_t outport = outputGateId;
                EV << "--";
                EV << "link from " << atNode->getModule()->getFullName();
                EV << " Port " << outport;
                EV << " to node "
                        << atNode->getPath(0)->getRemoteNode()->getModule()->getFullName()
                        << endl;
                nextNode = atNode->getPath(0)->getRemoteNode();

                connID_outport.push_back(
                        std::pair<int, uint32_t>(nodeInfo[j].connID, outport));
            }
        }
        typedef UnsortedMap::iterator iter;
        iter begin(connID_outport.begin());
        iter end(connID_outport.end());
        reverse_iterator<iter> rev_end(begin);
        reverse_iterator<iter> rev_iterator(end);
        while (rev_iterator != rev_end) {
            oxm_basic_match *match = new oxm_basic_match();
            match->OFB_ETH_SRC = thasrc;
            match->OFB_ETH_DST = thadst;
//          match->OFB_ETH_TYPE=
            match->wildcards = OFPFW_IN_PORT + OFPFW_DL_TYPE + OFPFW_NW_PROTO
                    + OFPFW_TP_DST + OFPFW_TP_SRC; // + OFPFW_DL_DST + OFPFW_DL_SRC;
            controller->sendFlowModMessage(OFPFC_ADD, match,
                    rev_iterator->second, rev_iterator->first);
            rev_iterator++;
        }
        connID_outport.clear();
    }

    return routeFound == 2;
}

bool ForwardingEXR::hasBidirectionpaths(IPv4Address IPv4src,
        IPv4Address IPv4dst, bool isweight) {
    if (IPv4src.equals(IPv4dst))
        return true;
    MACAddress thasrc = getMACfromIPv4(IPv4src);
    MACAddress thadst = getMACfromIPv4(IPv4dst);

    int routeFound = 0;

    Topology::Node * srcNode;
    Topology::Node * destNode;
    for (int i = 0; i < topo_forw.getNumNodes(); i++) {

        Topology::Node *curNode = topo_forw.getNode(i);
        //ignore ovs
        if (curNode->getModule()->getFullName()[0] == 'o')
            continue;

        EtherMAC *eth = (EtherMAC *) curNode->getModule()->getSubmodule("eth",
                0)->getSubmodule("mac");
        MACAddress mac = eth->getMACAddress();
        if (mac == thadst) {
            destNode = curNode;
            routeFound += 1;
        } else if (mac == thasrc) {
            srcNode = curNode;
            routeFound += 1;
        }
    }

//    EV << "node" << destNode->getModule()->getFullName() << "mac" << mac
//            << endl;

    if (routeFound == 2) {
        if (isweight)
            topo_forw.calculateWeightedSingleShortestPathsTo(destNode);
        else
            topo_forw.calculateUnweightedSingleShortestPathsTo(destNode);
        EV << "--src to dst " << srcNode->getNumPaths() << "paths" << endl;
        if (srcNode->getNumPaths() == 0)
            return false;
        EV << "--dst to src " << srcNode->getNumPaths() << "paths" << endl;
        topo_forw.calculateUnweightedSingleShortestPathsTo(srcNode);

        if (destNode->getNumPaths() == 0)
            return false;
    }

    return routeFound == 2;
}
bool ForwardingEXR::findAndReserveRouting(IPv4Address IPv4src,
        IPv4Address IPv4dst, bool isweight) {
    bool isBiPathReserved = par("isBiPathReserved");
    if (isBiPathReserved) {
        EV << "findAndReserveRouting from " << IPv4src << "to" << IPv4dst
                << endl;
        bool bireouteFound = hasBidirectionpaths(IPv4src, IPv4dst, isweight);

        if (isBiPathReserved)
            if (bireouteFound) {
                findAndReserveRoutingFromsrcTodst(IPv4dst, IPv4src, isweight,
                        FALSE, TRUE);
                findAndReserveRoutingFromsrcTodst(IPv4src, IPv4dst, isweight,
                        isweight, TRUE);

                LinkPath *secondlinkpath = linkpaths->back();
                linkpaths->pop_back();
                LinkPath *curlinkpath = linkpaths->back();
                for (uint i = 0; i < secondlinkpath->size(); i++)
                    curlinkpath->push_back(secondlinkpath->at(i));
                EV << "Find src" << IPv4src << "to dst" << IPv4dst << endl;
                return TRUE;
            }
        EV << "Find no" << IPv4src << "to dst" << IPv4dst << endl;
        return FALSE;
    } else {
        return findTwoPathAndReserveOneRouting(IPv4src, IPv4dst, isweight);
    }
}

bool ForwardingEXR::findTwoPathAndReserveOneRouting(IPv4Address IPv4src,
        IPv4Address IPv4dst, bool isweight) {
    EV << "find 2 And Reserve 1 Routing from " << IPv4src << "to" << IPv4dst
            << endl;
    //check whether bidirection is ok!

    bool bireouteFound = hasBidirectionpaths(IPv4src, IPv4dst, true);

    if (bireouteFound) {
        //the ack would be the same as the flow direction, without influence on the statues of node and link.
        findAndReserveRoutingFromsrcTodst(IPv4dst, IPv4src, isweight, FALSE,
                FALSE);
        //Thus the above should act first.
        findAndReserveRoutingFromsrcTodst(IPv4src, IPv4dst, isweight, isweight,
                TRUE);
//        LinkPath *secondlinkpath = linkpaths->back();
//        linkpaths->pop_back();
////release the reverse path
//        for (uint i = 0; i < secondlinkpath->size(); i++) {
//
//            secondlinkpath->at(i)->enable();
//            EV << "enable link "
//                    << secondlinkpath->at(i)->getLocalGate()->getFullName()
//                    << " to "
//                    << secondlinkpath->at(i)->getRemoteGate()->getFullName()
//                    << endl;
//        }
        EV << "Find src" << IPv4src << "to dst" << IPv4dst << endl;
        return true;
    }
    EV << "Find no" << IPv4src << "to dst" << IPv4dst << endl;
    return false;
}
//bool ForwardingEXR::findTwoPathAndReserveOneRouting(IPv4Address IPv4src,
//        IPv4Address IPv4dst) {
//    EV << "findTwoPathAndReserveOneRouting" << endl;
//    bool bireouteFound = findAndReserveRoutingFromsrcTodst(IPv4src, IPv4dst)
//            && findAndReserveRoutingFromsrcTodst(IPv4dst, IPv4src);
//    if (bireouteFound) {
//        LinkPath *secondlinkpath = linkpaths->back();
//        linkpaths->pop_back();
//        LinkPath *curlinkpath = linkpaths->back();
//        for (uint i = 0; i < secondlinkpath->size(); i++)
//            curlinkpath->push_back(secondlinkpath->at(i));
//
//        return true;
//    }
//    return false;
//}
MACAddress ForwardingEXR::getMACfromIPv4(IPv4Address IPv4src) {
    MACAddress thasrc;
    for (unsigned int i = 0; i < nodeInfo.size(); i++) {
        if (nodeInfo[i].address == IPv4src) {
            thasrc = nodeInfo[i].macAddress;
            return thasrc;
        }
    }
    EV << "this IPv4 address is valid: not existed" << endl;
    return 0;
}
bool ForwardingEXR::processARPPacket(OFP_Packet_In *packet_in, int connID) {
    oxm_basic_match *match = new oxm_basic_match();
    ARPPacket *arpPacket;
    EthernetIIFrame *frame;
    if (packet_in->getBuffer_id() == OFP_NO_BUFFER) {
        frame = check_and_cast<EthernetIIFrame *>(
                packet_in->getEncapsulatedPacket());
        if (frame != NULL) {
            match->OFB_IN_PORT = frame->getArrivalGate()->getIndex();
            match->OFB_ETH_SRC = frame->getSrc();
            match->OFB_ETH_DST = frame->getDest();
            match->OFB_ETH_TYPE = frame->getEtherType();
            //extract ARP specific match fields if present
            if (frame->getEtherType() == ETHERTYPE_ARP) {
                arpPacket = check_and_cast<ARPPacket *>(
                        frame->getEncapsulatedPacket());
                match->OFB_ARP_OP = arpPacket->getOpcode();
                match->OFB_ARP_SHA = arpPacket->getSrcMACAddress();
                match->OFB_ARP_THA = arpPacket->getDestMACAddress();
                match->OFB_ARP_SPA = arpPacket->getSrcIPAddress();
                match->OFB_ARP_TPA = arpPacket->getDestIPAddress();
            }
        }
    } else {
        match = &packet_in->getMatch();
        arpPacket = new ARPPacket();
        frame = new EthernetIIFrame();
    }

    if (match->OFB_ETH_TYPE != ETHERTYPE_ARP) {
        return false;
    }

    EV << "Processing ARP packet:\n";

// "?Is the opcode ares_op$REQUEST?  (NOW look at the opcode!!)"
    switch (match->OFB_ARP_OP) {
    case ARP_REQUEST: {
        EV << "Packet was ARP REQUEST, sending REPLY\n";

        arpPacket->setName("arpREPLY");

        MACAddress tha;

        for (unsigned int i = 0; i < nodeInfo.size(); i++) {
            EV << "Testing " << nodeInfo[i].address << " and "
                    << match->OFB_ARP_TPA << endl;
            if (nodeInfo[i].address == match->OFB_ARP_TPA) {
                tha = nodeInfo[i].macAddress;
                break;
            }

        }

        if (tha.isUnspecified()) {
            EV
                    << "No MAC address found for ARP REQUEST, triggering broadcast\n";
            return false;
        }

        arpPacket->setDestIPAddress(match->OFB_ARP_SPA);
        arpPacket->setDestMACAddress(match->OFB_ARP_SHA);
        arpPacket->setSrcIPAddress(match->OFB_ARP_TPA);
        arpPacket->setSrcMACAddress(tha);
        arpPacket->setOpcode(ARP_REPLY);

        frame->setSrc(tha);
        frame->setDest(match->OFB_ARP_SHA);

        if (packet_in->getBuffer_id() != OFP_NO_BUFFER) {
            frame->setEtherType(ETHERTYPE_ARP);
            frame->encapsulate(arpPacket);
            packet_in->encapsulate(frame);
            // Delete buffered ARP REQUEST
            controller->sendPacket(packet_in->getBuffer_id(), packet_in,
                    OFPP_ANY, connID);
        }

        // Trigger sending of ARP REPLY
        controller->sendPacket(OFP_NO_BUFFER, packet_in, match->OFB_IN_PORT,
                connID);

        return true;
    }
    case ARP_REPLY: {
        EV << "Packet was ARP REPLY and will be ignored\n";

        break;
    }
    default: {
        error("Unsupported opcode %d in received ARP packet",
                match->OFB_ARP_OP);
        break;
    }

    }
    return false;

}

//void ForwardingEXR::printROUTINGrequests(ROUTINGrequests * routingreq) {
//    if (routingreq->size()) {
//        std::vector<northboundWrapper*>::iterator wrapper_iter =
//                routingreq->begin();
//        while (wrapper_iter != routingreq->end()) {
//            EV << "src" << (*wrapper_iter)->ip_src.get4() << "to dst"
//                    << (*wrapper_iter)->ip_dst.get4() << endl;
//            wrapper_iter++;
//        }
//    }
//}

void ForwardingEXR::finish() {
    EV << "routingrequestqueue is " << nbreqqueue->size() << endl;
//    printROUTINGrequests(routingrequestqueue);
    EV << "routingrequests is " << nbreqs->size() << endl;
//    printROUTINGrequests(routingrequests);
//    EV << "routingrequestqueuewrapper is " << endl;
//    printROUTINGrequests(routingrequestqueuewrapper);
//    NBREQs::iterator nbreq_iter = nbreqqueue->begin();
//    while(nbreq_iter != nbreqqueue->end(){
//        EV<<"src"<<(*nbreq_iter)->getMatch()
//    }
}
