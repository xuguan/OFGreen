//1. read network topology; read feasible paths for all pairs
//2. record used paths with reserving time. when flows finished, the following
//2.1 test the real time ending and the estimated ending, is there a leak.
//3. scheduling flows with a real-time control on the flow state,
//as ctrl app can admit or deny a flow,
//as the SDN controller can install and remove flow paths according to your wish.
//4.

#include "BEERs.h"
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
Define_Module(BEERs);

BEERs::BEERs()
{
    // TODO Auto-generated constructor stub

}

BEERs::~BEERs()
{
    // TODO Auto-generated destructor stub
}

void BEERs::initialize(int stage)
{
    if (stage == 0)
    {
        EV << "BEERs: Initialize Stage 0" << endl;
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
        max_multi_path_num = par("max_multi_path_num");

        WATCH_VECTOR(nodeInfo);

    }

    if (stage == 4)
    {
        EV << "BEERs: Initialize Stage 4" << endl;

        extractTopology(topo_forw, nodeInfo);

        assignAddresses(topo_forw, nodeInfo);
        linkpaths = new LinkPaths;
//        routingrequestqueue = new ROUTINGrequests;
//        routingrequests = new ROUTINGrequests;
//        routingrequestqueuewrapper = new ROUTINGrequests;
        nbreqqueue = new NBREQs;
        nbreqs = new NBREQs;

        my_graph_pt = new Graph();
        my_graph_pt->clear();
        my_graph_pt->setVetexNum(nodeInfo.size());
        for (int i = 0; i < topo_forw.getNumNodes(); i++)
        {
            Topology::Node *curNode = topo_forw.getNode(i);
            for (int j = 0; j < curNode->getNumOutLinks(); j++)
            {
                Topology::LinkOut * curLinkOut = curNode->getLinkOut(j);
                Topology::Node *curNodeout = curLinkOut->getRemoteNode();
                int k = 0;
                for (; topo_forw.getNode(k) != curNodeout; k++)
                    ;
                my_graph_pt->addEdge(i, k);
                EV << "link: " << topo_forw.getNode(i)->getModule()->getFullName() << " \t <==> \t" << topo_forw.getNode(k)->getModule()->getFullName() << "weight "
                        << my_graph_pt->get_edge_weight(my_graph_pt->get_vertex(i), my_graph_pt->get_vertex(k)) << endl;
            }
        }
        my_graph_pt->finishAddingVertex();

//test whether the input data is right for the graph
//        for (int i = 0; i < topo_forw.getNumNodes(); i++) {
//            set<BaseVertex*> *adjNodes = new set<BaseVertex*>;
//            my_graph_pt->get_adjacent_vertices(my_graph_pt->get_vertex(i),
//                    *adjNodes);
//            set<BaseVertex*>::iterator setiter = adjNodes->begin();
//            while (setiter != adjNodes->end()) {
//                EV << "weight betewwn "
//                        << topo_forw.getNode(i)->getModule()->getFullName()
//                        << " \t and\t"
//                        << topo_forw.getNode((*setiter)->getID())->getModule()->getFullName()
//                        << "is "
//                        << my_graph_pt->get_edge_weight(
//                                my_graph_pt->get_vertex(i), *setiter) << endl;
//                setiter++;
//            }
//        }

        basePathAllPair = new BasePathsAllPair;
        for (int i = 0; i < topo_forw.getNumNodes(); i++)
        {
            if (topo_forw.getNode(i)->getModule()->getFullName()[0] == 'o')
                continue;
            BasePathsPerSrc *basePathsPerSrc = new BasePathsPerSrc;

            basePathAllPair->insert(make_pair(i, basePathsPerSrc));
            for (int j = 0; j < topo_forw.getNumNodes(); j++)
            {
                if (i == j)
                    continue;
                if (topo_forw.getNode(j)->getModule()->getFullName()[0] == 'o')
                    continue;
                BaseVertex* srcVertex = my_graph_pt->get_vertex(i);
                BaseVertex* dstVertex = my_graph_pt->get_vertex(j);
                YenTopKShortestPathsAlg yenAlg(*my_graph_pt, srcVertex, dstVertex);
                BasePaths *curPaths = new BasePaths;
                yenAlg.get_shortest_paths(srcVertex, dstVertex, max_multi_path_num, *curPaths);
                basePathsPerSrc->insert(make_pair(j, curPaths));
                EV << curPaths->size() << "paths betewwn " << topo_forw.getNode(i)->getModule()->getFullName() << " \t and " << topo_forw.getNode(j)->getModule()->getFullName()
                        << endl;
                for (int p = 0; p < curPaths->size(); p++)
                {
                    EV << p << "th path:\t";
                    BasePath * path = curPaths->at(p);
                    int pathlength = path->length();
                    for (int pl = 0; pl < pathlength; pl++)
                    {
                        EV << path->GetVertex(pl)->getID() << "->";
                    }
                    EV << endl;
                }
//                DijkstraShortestPathAlg shortest_path_alg(my_graph_pt);
//                BasePath* result = shortest_path_alg.get_shortest_path(
//                        my_graph_pt->get_vertex(i), my_graph_pt->get_vertex(j));
//                EV << "SP length" << result->length() << endl;

//                int i = 0;
//                while (yenAlg.has_next() && i < max_multi_path_num ) {
//                    ++i;
//                    BasePath * curpath=yenAlg.next();
////                    curpath->PrintOut(cout);
//                    curpath->PrintOut(cout);
//                }
            }
        }
        flowsOrderedByWeight = new std::vector<FlowID_t>;
        flowsWeights = new std::vector<double>;
        flowsRateAverage = new std::vector<double>;
        isactiveflow = new std::vector<int>;
        flowOrderedByWeightPaths = new std::vector<BasePath*>;
        bestPaths = new BasePaths;
        activeflowpaths = new FlowPaths;
        linkDensity = new FlowNumPerNode_t;
        allFlows = new AllFlows_t;
        flow_ID = 0;
        linkCapacity = par("linkcapacity").doubleValue(); //in MByte/s

    }
}

//bool selectFunction(cModule *mod, void *){
//
//    return strcmp(mod->getNedTypeName(),"openflow.nodes.StandardHostRandom")==0
//            || strcmp(mod->getNedTypeName(),"openflow.nodes.Open_Flow_Switch")==0;
//
//}

//Only the truth return can allow the module imported
bool selectFunctionBEERs(cModule *mod, void *v_ptr)
{
    int id = *(int *) v_ptr;
    int domainID = -1;

    if (mod->hasPar("doaminID"))
    {
        domainID = mod->par("domainID").longValue();
    }
    else if (mod->getParentModule()->hasPar("doaminID"))
    {
        domainID = mod->getParentModule()->par("domainID").longValue();
    }

    return (strcmp(mod->getNedTypeName(), "inet.nodes.inet.StandardHost") == 0 || strcmp(mod->getNedTypeName(), "openflow.nodes.Open_Flow_Switch") == 0) && domainID == id;
}

void BEERs::handleMessage(cMessage* msg)
{
    EV << "handle the msg from" << msg->getSenderModule()->getFullPath().c_str() << endl;
    if (msg->isSelfMessage())
    {
        EV << msg->getArrivalModule()->getFullName();
        return;
    }
    northboundMsg * northboundmsg = check_and_cast<northboundMsg *>(msg);
    oxm_basic_match match = northboundmsg->getMatch();
    IPv4Address IPv4src = match.OFB_IPV4_SRC;
    IPv4Address IPv4dst = match.OFB_IPV4_DST;
    EV << "Flow from " << IPv4src << " to " << IPv4dst << endl;

    uint8_t nbcanSent = northboundmsg->getCanSend();
    uint8_t nbcanEnd = northboundmsg->getCanEnd();
    cModule * clientModule = (cModule *) northboundmsg->getClientModuleptr();
    if (!nbcanSent && !nbcanEnd)
    {
//request reservation

        bool isWeightedNode = par("isWeightedNode");
        //add the new flow into queue, and return the minimum links needed for existing flows.
        //record the minimum set of paths
        int linkNum = flowScheduleMinSwitches(IPv4src, IPv4dst, northboundmsg, isWeightedNode);
        bool routeFound = 0;
        if (linkNum > 0)
            routeFound = 1;
        if (routeFound)
        {
            EV << "Flow " << northboundmsg->getFlowID() << " from " << IPv4src << "  to  " << IPv4dst << "has a path!" << endl;
            schedulFlows();
//            EV << "starting" << IPv4src << "to" << IPv4dst << endl;
//            northboundmsg->setCanSend(1);
//            nbreqs->push_back(northboundmsg);
//            sendDirect(northboundmsg, clientModule->gate("nbIn"));
        }
        else
        {
//            EV << "queueing" << IPv4src << "to" << IPv4dst << endl;
//            nbreqqueue->push_back(northboundmsg);
            EV << "Error" << endl;
        }
    }
    else if (nbcanSent && nbcanEnd)
    {
//request release
        EV << "Finshing " << northboundmsg->getFlowID() << " flow path from" << IPv4src << "to" << IPv4dst << endl;
        finishFlow(northboundmsg->getFlowID());
        schedulFlows();
    }
    else
    {
        EV << "error northbound msg!" << endl;
    }
//    sendDirect(msg, msg->getSenderModule()->gate("nbIn"));
}

bool BEERs::exclusiveRouting()
{
    uint ordered_index = bestPaths->size() - 1;
    double minLength = 0;
    uint min_index = 0;
    for (; ordered_index >= 0; ordered_index--)
    {
        BasePath* curpath = bestPaths->at(ordered_index);
        //find the flow with the minimal deadline
        double length = allFlows->find(flowsOrderedByWeight->at(ordered_index))->second->getByteLength();
        if (minLength > length || minLength == 0)
        {
            minLength = length;
            min_index = ordered_index;
        }
    }

    return TRUE;
}

bool BEERs::canStartFlowIndex(int curFlow)
{

//EXR here, find the flow with the minimal deadline
    uint i = 0;
    for (; i < bestPaths->size() && i < curFlow; i++)
        //check only from 1st to curFlow for corruption.
        if (canStartFlowIndex(i, curFlow))
            continue;
        else
        {
//            EV << "Flow ID (" << flowsOrderedByWeight->at(curFlow)
//                    << " suspended";
            return FALSE;
        }
    //TODO: for BEERs, add your rules here to allow or deny curFlow.

//    EV << endl << "Flow ID " << flowsOrderedByWeight->at(curFlow) << "start!";
    return TRUE;
}

bool BEERs::canStartFlowIndex(int priFlow, int curFlow)
{
//check whether curFlow can start with EXR
    //first check if curFlow want a link of priFlow when priFlow is active.
    if (isactiveflow->at(priFlow) == 0)
        return true;
    if (curFlow < priFlow)
    {
        EV << "The current flow has smaller weight" << endl;
        return TRUE;
    }
    BasePath * priPath = bestPaths->at(priFlow);
    BasePath * curPath = bestPaths->at(curFlow);
    int sharedLinksNumber = priPath->SharedLinks(curPath);
    if (sharedLinksNumber != 0)
    {
        EV << "Found " << sharedLinksNumber << " shared links with " << priFlow << "th flow!";
        return FALSE;
    }
    else
        return TRUE;
    //then check if curFlow fall in the available interval.

}

bool BEERs::schedulFlows()
{
    //TODO: get the next active flows from bestPaths.
    int pathsSize = 0;
    int maxpaths = bestPaths->size();
    EV << "Schedule " << maxpaths << " Flows" << endl;
    EV << maxpaths << "flows" << endl;
    for (; pathsSize < maxpaths; pathsSize++)
    {
        if (isactiveflow->at(pathsSize) == 1)
        {
            EV << "The " << pathsSize << "th flow \t is active" << endl;
            continue;
        }
        EV << "The " << pathsSize << "th flow\t";
        if (bestPaths->at(pathsSize) == 0)
        {
            EV << "This flow has no best path!" << endl;
            continue;
        }
        northboundMsg* nbmsg = allFlows->find(flowsOrderedByWeight->at(pathsSize))->second;
        //can the flow active? canstartFlow
        if (canStartFlowIndex(pathsSize))
        {
            EV << "\t(ID: " << flowsOrderedByWeight->at(pathsSize) << ", " << nbmsg->getMatch().OFB_IPV4_SRC << " to " << nbmsg->getMatch().OFB_IPV4_DST << "start) " << endl;

            BasePath* curpath = bestPaths->at(pathsSize);
            installPath(nbmsg, curpath);
            startFlow(nbmsg);
            isactiveflow->at(pathsSize) = 1;
        }
        else
        {
            EV << "\t(ID: " << flowsOrderedByWeight->at(pathsSize) << ", " << nbmsg->getMatch().OFB_IPV4_SRC << " to " << nbmsg->getMatch().OFB_IPV4_DST << " suspended) " << endl;
        }

    }
    return TRUE;
}

void BEERs::extractTopology(Topology &topo, NodeInfoVector &nodeInfo)
{

    // We either have knowledge about the entire network or only about our own domain
    if (par("domainID").longValue() >= 0)
    {
        int domainID = getParentModule()->par("domainID").longValue();
        topo.extractFromNetwork(selectFunctionBEERs, (int *) &domainID);
        EV << "Topology found " << topo.getNumNodes() << " nodes for domain " << domainID << ".\n";
    }
    else
    {
        // Use specified ned type to extract relevant nodes
        topo.extractByNedTypeName(cStringTokenizer(par("nedTypes")).asVector());
        EV << "Topology found " << topo.getNumNodes() << " nodes\n";
    }

    double nodeweight = par("nodeweight");
    nodeInfo.resize(topo.getNumNodes());
    for (int i = 0; i < topo.getNumNodes(); i++)
    {
        cModule *mod = topo.getNode(i)->getModule();
        nodeInfo[i].isIPNode = IPvXAddressResolver().findInterfaceTableOf(mod) != NULL;
        nodeInfo[i].isOpenFlow = (mod->findSubmodule("flow_Table") != -1);
        nodeInfo[i].name = topo.getNode(i)->getModule()->getFullPath();
        if (nodeInfo[i].isIPNode)
        {
            nodeInfo[i].ift = IPvXAddressResolver().interfaceTableOf(mod);
            nodeInfo[i].rt = IPvXAddressResolver().routingTableOf(mod);
        }
        if (nodeInfo[i].isOpenFlow)
        {
            EV << "extractTopology: OF-Switch found" << endl;
            topo.getNode(i)->setWeight(nodeweight);
        }
    }
}

void BEERs::assignAddresses(Topology &topo, NodeInfoVector &nodeInfo)
{
    for (int i = 0; i < topo.getNumNodes(); i++)
    {
        if (!nodeInfo[i].isIPNode)
            continue;

        IInterfaceTable *ift = nodeInfo[i].ift;
        for (int k = 0; k < ift->getNumInterfaces(); k++)
        {
            InterfaceEntry *ie = ift->getInterface(k);
            if (!ie->isLoopback())
            {
                IPv4Address addr = ie->ipv4Data()->getIPAddress();
                nodeInfo[i].address = addr;
                nodeInfo[i].macAddress = ie->getMacAddress();
                break;
            }
        }
    }
}

void BEERs::receiveSignal(cComponent *src, simsignal_t id, cObject *obj)
{
    EV << "Received signal " << getSignalName(id) << " at module " << this->getFullName() << "." << endl;
    if (id == PacketInSignalId)
    {
        if (dynamic_cast<OF_Wrapper *>(obj) != NULL)
        {
            OF_Wrapper *wrapper = (OF_Wrapper *) obj;
            Open_Flow_Message *of_msg = wrapper->msg;
            OFP_Packet_In *packet_in = (OFP_Packet_In *) of_msg;
            uint32_t buffer_id = packet_in->getBuffer_id();
            MACAddress src_mac;
            MACAddress dst_mac;
            IPv4Address dst_ip;
            int ethType;
            if (buffer_id == OFP_NO_BUFFER)
            {
                src_mac = dynamic_cast<EthernetIIFrame *>(packet_in->getEncapsulatedPacket())->getSrc();
                dst_mac = dynamic_cast<EthernetIIFrame *>(packet_in->getEncapsulatedPacket())->getDest();
                ethType = dynamic_cast<EthernetIIFrame *>(packet_in->getEncapsulatedPacket())->getEtherType();
            }
            else
            {
                src_mac = packet_in->getMatch().OFB_ETH_SRC;
                dst_mac = packet_in->getMatch().OFB_ETH_DST;
                ethType = packet_in->getMatch().OFB_ETH_TYPE;
            }

            TCPCommand *ind = dynamic_cast<TCPCommand *>(packet_in->getControlInfo());
            if (!ind)
                opp_error("Switch: no TCPCommand control info in message (not from TCP?)");
            int connID = ind->getConnId();
            EV << "connID: " << connID << endl;
            int outputGateId;
            bool routeFound = 0;
            if (ethType == 0x0800)
            {
                EV << "Processing IPv4 packet: no operation!" << endl;
            }
            else if (isArpOracle && ethType == 0x0806)
            {
                EV << "Processing ARP packet" << endl;
                routeFound = processARPPacket(packet_in, connID);
            }
            else
            {
                EV << "Invalid packet in msg, " << "is this session start without controller's permission!" << endl;
//                controller->floodPacket(buffer_id, packet_in, connID);
            }
        }
    }
    else if (id == connIDSignal)
    {
        if (dynamic_cast<OF_Wrapper *>(obj) != NULL)
        {
            OF_Wrapper *wrapper = (OF_Wrapper *) obj;
            int connID = wrapper->connID;
            IPv4Address *ip_src = wrapper->ip_src;
            for (int i = 0; i < topo_forw.getNumNodes(); i++)
            {
                //                cTopology::Node *destNode = topo_forw.getNode(i);
                //                EtherMAC *eth =  (EtherMAC *) destNode->getModule()->getSubmodule("eth", 0)->getSubmodule("mac");
                //                                MACAddress mac = eth->getMACAddress();
                IPv4Address *ip = &nodeInfo[i].address;
                //                    EV << "-----------------------------------------------------------------" << endl;
                //                    EV << *ip << " "<< *ip_src << endl;
                if (*ip == *ip_src)
                    nodeInfo[i].connID = connID;
            }
        }
    }

}

bool BEERs::startFlow(northboundMsg *nbmsg)
{
//    oxm_basic_match match = nbmsg->getMatch();
    //    IPv4Address IPv4src = match.OFB_IPV4_SRC;
    //    IPv4Address IPv4dst = match.OFB_IPV4_DST;
    uint8_t nbcanSent = nbmsg->getCanSend();
    uint8_t nbcanEnd = nbmsg->getCanEnd();
    EV << "start flow: " << nbmsg->getFlowID() << endl;
    if (nbcanSent == 1 || nbcanEnd == 1)
        return FALSE;
    cModule * clientModule = (cModule *) nbmsg->getClientModuleptr();
    nbmsg->setCanSend(1);
    nbreqs->push_back(nbmsg);
    sendDirect(nbmsg, clientModule->gate("nbIn"));
    return TRUE;
}

bool BEERs::installPath(northboundMsg *nbmsg, BasePath *path)
{
// given a path
    // connID, outputPort
    oxm_basic_match *match = new oxm_basic_match(nbmsg->getMatch());
    installPath(match, path);
    installVersePath(match, path);
    delete match;
    return TRUE;
}

bool BEERs::installPath(oxm_basic_match * match, BasePath *path)
{
    EV << "Install path" << endl;
    uint vertex_index = 0;
    for (; vertex_index < path->length() - 1; vertex_index++)
    {

        int curNodeID = path->GetVertex(vertex_index)->getID();
        EV << nodeInfo[curNodeID].name.c_str() << "  \t ";
        if (nodeInfo.at(curNodeID).isOpenFlow == false)
        {
            EV << "The head host!" << endl;
            continue;
        }
        int nextNodeID = path->GetVertex(vertex_index + 1)->getID();
        int curconnID = nodeInfo[curNodeID].connID;
        uint curOutport = 0;
        Topology::Node *curNode = topo_forw.getNode(curNodeID);
        Topology::Node *nextNode = topo_forw.getNode(nextNodeID);
        for (; curOutport < curNode->getNumOutLinks(); curOutport++)
            if (curNode->getLinkOut(curOutport)->getRemoteNode() == nextNode)
                break;
        oxm_basic_match *matchdup = new oxm_basic_match(*match);
        controller->sendFlowModMessage(OFPFC_ADD, matchdup, curOutport, curconnID);
    }
    EV << nodeInfo[path->GetVertex(vertex_index)->getID()].name.c_str() << "  \t The tail host!" << endl;
    return TRUE;
}

bool BEERs::installVersePath(oxm_basic_match * match, BasePath *path)
{
    EV << "Install path" << endl;
    int vertex_index = path->length() - 1;
    IPv4Address tmp = match->OFB_IPV4_DST;
    match->OFB_IPV4_DST = match->OFB_IPV4_SRC;
    match->OFB_IPV4_SRC = tmp;
    for (; vertex_index > 0; vertex_index--)
    {
        int curNodeID = path->GetVertex(vertex_index)->getID();
        EV << nodeInfo[curNodeID].name.c_str() << "  \t ";
        if (nodeInfo.at(curNodeID).isOpenFlow == false)
        {
            EV << "The head host!" << endl;
            continue;
        }
        int nextNodeID = path->GetVertex(vertex_index - 1)->getID();
        int curconnID = nodeInfo[curNodeID].connID;
        uint curOutport = 0;
        Topology::Node *curNode = topo_forw.getNode(curNodeID);
        Topology::Node *nextNode = topo_forw.getNode(nextNodeID);
        for (; curOutport < curNode->getNumOutLinks(); curOutport++)
            if (curNode->getLinkOut(curOutport)->getRemoteNode() == nextNode)
                break;
        oxm_basic_match *matchdup = new oxm_basic_match(*match);
        controller->sendFlowModMessage(OFPFC_ADD, matchdup, curOutport, curconnID);
    }
    EV << nodeInfo[path->GetVertex(vertex_index)->getID()].name.c_str() << "  \t The tail host!" << endl;
    return TRUE;
}

bool BEERs::removePath(northboundMsg *nbmsg, BasePath *path)
{

    oxm_basic_match *match = new oxm_basic_match(nbmsg->getMatch());
    removePath(match, path);
    removeVersePath(match, path);
    delete match;
    return TRUE;
}
bool BEERs::removePath(oxm_basic_match * match, BasePath *path)
{
    EV << "remove path" << endl;
    uint vertex_index = 0;
    for (; vertex_index < path->length() - 1; vertex_index++)
    {

        int curNodeID = path->GetVertex(vertex_index)->getID();
        EV << nodeInfo[curNodeID].name.c_str() << endl;
        if (nodeInfo.at(curNodeID).isOpenFlow == false)
        {
            EV << "The head host!" << endl;
            continue;
        }
        int nextNodeID = path->GetVertex(vertex_index + 1)->getID();
        int curconnID = nodeInfo[curNodeID].connID;
        uint curOutport = 0;
        Topology::Node *curNode = topo_forw.getNode(curNodeID);
        Topology::Node *nextNode = topo_forw.getNode(nextNodeID);
        for (; curOutport < curNode->getNumOutLinks(); curOutport++)
            if (curNode->getLinkOut(curOutport)->getRemoteNode() == nextNode)
                break;
        oxm_basic_match *matchdup = new oxm_basic_match(*match);
        controller->sendFlowModMessage(OFPFC_DELETE, matchdup, curOutport, curconnID);
    }
    EV << nodeInfo[path->GetVertex(vertex_index)->getID()].name.c_str() << endl;
    return TRUE;
}

bool BEERs::removeVersePath(oxm_basic_match * match, BasePath *path)
{
    EV << "remove path" << endl;
    int vertex_index = path->length() - 1;
    IPv4Address tmp = match->OFB_IPV4_DST;
    match->OFB_IPV4_DST = match->OFB_IPV4_SRC;
    match->OFB_IPV4_SRC = tmp;
    for (; vertex_index > 0; vertex_index--)
    {
        int curNodeID = path->GetVertex(vertex_index)->getID();
        EV << nodeInfo[curNodeID].name.c_str() << endl;
        if (nodeInfo.at(curNodeID).isOpenFlow == false)
        {
            continue;
        }
        int nextNodeID = path->GetVertex(vertex_index - 1)->getID();
        int curconnID = nodeInfo[curNodeID].connID;
        uint curOutport = 0;
        Topology::Node *curNode = topo_forw.getNode(curNodeID);
        Topology::Node *nextNode = topo_forw.getNode(nextNodeID);
        for (; curOutport < curNode->getNumOutLinks(); curOutport++)
            if (curNode->getLinkOut(curOutport)->getRemoteNode() == nextNode)
                break;
        oxm_basic_match *matchdup = new oxm_basic_match(*match);
        controller->sendFlowModMessage(OFPFC_DELETE, matchdup, curOutport, curconnID);
    }
    EV << nodeInfo[path->GetVertex(vertex_index)->getID()].name.c_str() << endl;
    return TRUE;
}

MACAddress BEERs::getMACfromIPv4(IPv4Address IPv4src)
{
    MACAddress thasrc;
    for (unsigned int i = 0; i < nodeInfo.size(); i++)
    {
        if (nodeInfo[i].address == IPv4src)
        {
            thasrc = nodeInfo[i].macAddress;
            return thasrc;
        }
    }
    EV << "this IPv4 address is valid: not existed" << endl;
    return 0;
}

bool BEERs::processARPPacket(OFP_Packet_In *packet_in, int connID)
{
    oxm_basic_match *match = new oxm_basic_match();
    ARPPacket *arpPacket;
    EthernetIIFrame *frame;
    if (packet_in->getBuffer_id() == OFP_NO_BUFFER)
    {
        frame = check_and_cast<EthernetIIFrame *>(packet_in->getEncapsulatedPacket());
        if (frame != NULL)
        {
            match->OFB_IN_PORT = frame->getArrivalGate()->getIndex();
            match->OFB_ETH_SRC = frame->getSrc();
            match->OFB_ETH_DST = frame->getDest();
            match->OFB_ETH_TYPE = frame->getEtherType();
            //extract ARP specific match fields if present
            if (frame->getEtherType() == ETHERTYPE_ARP)
            {
                arpPacket = check_and_cast<ARPPacket *>(frame->getEncapsulatedPacket());
                match->OFB_ARP_OP = arpPacket->getOpcode();
                match->OFB_ARP_SHA = arpPacket->getSrcMACAddress();
                match->OFB_ARP_THA = arpPacket->getDestMACAddress();
                match->OFB_ARP_SPA = arpPacket->getSrcIPAddress();
                match->OFB_ARP_TPA = arpPacket->getDestIPAddress();
            }
        }
    }
    else
    {
        match = &packet_in->getMatch();
        arpPacket = new ARPPacket();
        frame = new EthernetIIFrame();
    }

    if (match->OFB_ETH_TYPE != ETHERTYPE_ARP)
    {
        return false;
    }

    EV << "Processing ARP packet:\n";

// "?Is the opcode ares_op$REQUEST?  (NOW look at the opcode!!)"
    switch (match->OFB_ARP_OP)
    {
        case ARP_REQUEST: {
            EV << "Packet was ARP REQUEST, sending REPLY\n";

            arpPacket->setName("arpREPLY");

            MACAddress tha;

            for (unsigned int i = 0; i < nodeInfo.size(); i++)
            {
                EV << "Testing " << nodeInfo[i].address << " and " << match->OFB_ARP_TPA << endl;
                if (nodeInfo[i].address == match->OFB_ARP_TPA)
                {
                    tha = nodeInfo[i].macAddress;
                    break;
                }

            }

            if (tha.isUnspecified())
            {
                EV << "No MAC address found for ARP REQUEST, triggering broadcast\n";
                return false;
            }

            arpPacket->setDestIPAddress(match->OFB_ARP_SPA);
            arpPacket->setDestMACAddress(match->OFB_ARP_SHA);
            arpPacket->setSrcIPAddress(match->OFB_ARP_TPA);
            arpPacket->setSrcMACAddress(tha);
            arpPacket->setOpcode(ARP_REPLY);

            frame->setSrc(tha);
            frame->setDest(match->OFB_ARP_SHA);

            if (packet_in->getBuffer_id() != OFP_NO_BUFFER)
            {
                frame->setEtherType(ETHERTYPE_ARP);
                frame->encapsulate(arpPacket);
                packet_in->encapsulate(frame);
                // Delete buffered ARP REQUEST
                controller->sendPacket(packet_in->getBuffer_id(), packet_in, OFPP_ANY, connID);
            }
            // Trigger sending of ARP REPLY
            controller->sendPacket(OFP_NO_BUFFER, packet_in, match->OFB_IN_PORT, connID);

            return true;
        }
        case ARP_REPLY: {
            EV << "Packet was ARP REPLY and will be ignored\n";

            break;
        }
        default: {
            error("Unsupported opcode %d in received ARP packet", match->OFB_ARP_OP);
            break;
        }

    }
    return false;

}

void BEERs::finish()
{
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

bool BEERs::flowScheduleHeuristic()
{
//1. update the link density once a flow request arrives.
//2.choose the flow order, which should be removed once the flow ends.

//3 update flow rate; obviously, the value of queued flows will increase while the active flows are decreasing.

// 3.create the flowsOrderedByWeight, which is refreshed each time the new flow comes.
    return 0;
}

int BEERs::getIDbyMAC(MACAddress macAddress)
{
    for (int i = 0; i < nodeInfo.size(); i++)
    {
        if (nodeInfo.at(i).macAddress.equals(macAddress))
            return i;
    }
    return -1;
}

int BEERs::getIDbyIPv4(IPv4Address ipAddress)
{
    for (uint i = 0; i < nodeInfo.size(); i++)
    {
        if (nodeInfo.at(i).address.equals(ipAddress))
            return i;
    }
    return -1;

}

std::vector<BasePath *> * BEERs::getPathsByIPv4(IPv4Address ipAddresssrc, IPv4Address ipAddressdst)
{
    return basePathAllPair->find(getIDbyIPv4(ipAddresssrc))->second->find(getIDbyIPv4(ipAddressdst))->second;
}

std::vector<BasePath *> * BEERs::getPathsByFlowID(FlowID_t flowID)
{
    northboundMsg * northboundmsg = allFlows->find(flowID)->second;
    oxm_basic_match match = northboundmsg->getMatch();
    IPv4Address IPv4src = match.OFB_IPV4_SRC;
    IPv4Address IPv4dst = match.OFB_IPV4_DST;
    return basePathAllPair->find(getIDbyIPv4(IPv4src))->second->find(getIDbyIPv4(IPv4dst))->second;
}

//count the number of a set of links
int BEERs::getPathNum(BasePaths *flowpaths)
{
    if (flowpaths->size() == 0)
        return -1;
    int count = 0;
    if (flowpaths->size() != flowsOrderedByWeight->size())
    {
        EV << "ERROR: flowpaths not equal to flowweights";
        return -1;
    }
    typedef std::map<int, int> edge_Num;
    std::map<int, edge_Num*> edges;
    for (uint i = 0; i < flowpaths->size(); i++)
    {
        BasePath * curpath = flowpaths->at(i);
        if (curpath == 0 || curpath->length() <= 1)
            return -1;
        for (int j = curpath->length() - 2; j >= 0; j--)
        {
            int fromID = curpath->GetVertex(j)->getID();
            int toID = curpath->GetVertex(j + 1)->getID();
            int largerID = fromID > toID ? fromID : toID;
            int smallID = fromID < toID ? fromID : toID;
            if (edges.find(largerID) != edges.end())
            {
                edge_Num * edgeNum = edges.find(largerID)->second;
                if (edgeNum->find(smallID) != edgeNum->end())
                    edgeNum->find(smallID)->second = 1;
                else
                {
                    edgeNum->insert(make_pair(smallID, 1));
                    count++;
                }
            }
            else
            {
                edge_Num * edgeNum = new edge_Num;
                edges.insert(make_pair(largerID, edgeNum));
                edgeNum->insert(make_pair(smallID, 1));
                count++;
            }
        }
    }
    for (std::map<int, edge_Num*>::iterator itr = edges.begin(); itr != edges.end(); itr++)
    {
        edge_Num * edgeNum = itr->second;
        edgeNum->clear();
        delete edgeNum;
    }
    edges.clear();
    return count;
}

//add new flow with flow rate, without specifying the best flow path.
bool BEERs::addNewFlow(IPv4Address IPv4src, IPv4Address IPv4dst, northboundMsg *nbmsg)
{
//adding this flow into queue of flowsOrderedByWeight and flowsWeights
    int srcID = getIDbyIPv4(IPv4src);
    int dstID = getIDbyIPv4(IPv4dst);
    if (srcID == -1 || srcID == -1)
    {
        EV << "ERROR no such node" << endl;
        return 0;
    }
//1 find all paths of this flow;
    BasePaths *basepaths = basePathAllPair->find(srcID)->second->find(dstID)->second;
//2 calculate this flow's average rate.
    double MMFAverageRate = 0;
    for (uint i = 0; i < basepaths->size(); i++)
    {
        BasePath * basepath_ptr = basepaths->at(i);
        double count = 0;
        for (int j = 0; j < basepath_ptr->length() - 1; j++)
        {
            int fromID = basepath_ptr->GetVertex(j)->getID();
            int toID = basepath_ptr->GetVertex(j + 1)->getID();
            FlowNumPerNode_t::iterator linkDensity_itr = linkDensity->find(fromID);
            if (linkDensity_itr == linkDensity->end())
            {
                FlowNumPerPort_t* flowNumPerPort = new FlowNumPerPort_t;
                linkDensity->insert(make_pair(fromID, flowNumPerPort));
                linkDensity_itr = linkDensity->find(fromID);
            }
            FlowNumPerPort_t::iterator linkDensity_itr_itr = linkDensity_itr->second->find(toID);
            if (linkDensity_itr_itr == linkDensity_itr->second->end())
            {
                int flowNum = 0;
                linkDensity_itr->second->insert(make_pair(toID, flowNum));
                linkDensity_itr_itr = linkDensity_itr->second->find(toID);
            }
            linkDensity_itr_itr->second += 1;
            if (count < linkDensity_itr_itr->second)
                count = linkDensity_itr_itr->second;
        }
        MMFAverageRate += 1 / count;
    }
//3 record the flow with a flow_id
    allFlows->insert(make_pair(++flow_ID, nbmsg));
    nbmsg->setFlowID(flow_ID);

//4 insert the flow
    //TODO inserting flow would change the rate of other flows. so it is not accurate. Maybe update flowWeight and flowOrderedByWeight
    int po = 0;
    for (; po < flowsWeights->size(); po++)
        if (flowsWeights->at(po) < MMFAverageRate)
            continue;
    lastArrivalTime = simTime();
    flowsWeights->insert(flowsWeights->begin() + po, MMFAverageRate);
    flowsOrderedByWeight->insert(flowsOrderedByWeight->begin() + po, flow_ID);
    double flowRate = nbmsg->getSendBytes(); //@unit(MiB)
    flowRate *= 8; //from MiB to Mb
    flowRate /= (nbmsg->getTdeadline() - simTime()).dbl(); //@unit(Mbps)
    flowsRateAverage->insert(flowsRateAverage->begin() + po, flowRate);
    isactiveflow->insert(isactiveflow->begin() + po, FALSE);
    flowOrderedByWeightPaths->insert(flowOrderedByWeightPaths->begin() + po, NULL);
    bestPaths->insert(bestPaths->begin() + po, NULL);
//    flowsRateAverage->insert()
    return TRUE;
}

//update flow rate before adding a new flow.
//deadline is not the relative value.
bool BEERs::updateFlowRate()
{
    if (flowsOrderedByWeight->size() == 0)
        return TRUE;
//activeFlows, updated with resident length and new start time.
//queued flows, updated with new start time.
    for (uint i = 0; i < flowsOrderedByWeight->size(); i++)
    {
        simtime_t deadline = allFlows->find(flowsOrderedByWeight->at(i))->second->getTdeadline();
        simtime_t now = simTime();
        double lastFlowRate = flowsRateAverage->at(i);
        if (isactiveflow->at(i) == 1)
        {
            flowsRateAverage->at(i) = ((deadline - lastArrivalTime).dbl() * lastFlowRate - (now - lastArrivalTime).dbl() * linkCapacity) / (deadline - now);
        }
        else
        {
            long flowsize = allFlows->find(flowsOrderedByWeight->at(i))->second->getSendBytes();
            double flowrate = (double) flowsize / (deadline - now).dbl();
            flowsRateAverage->at(i) = flowrate;
        }
    }
    return TRUE;
}

bool BEERs::finishFlow(FlowID_t flowID)
{

//release the flow
//move the flow to the history
//    bool canStartNewFLow = 0;
//    northboundMsg* northboundmsg = allFlows->find(flowID)->second;
    EV << "finish flow: " << flowID << endl;
    uint flowID_index = 0;
    for (; flowID_index < flowsOrderedByWeight->size(); flowID_index++)
    {
        if (flowsOrderedByWeight->at(flowID_index) == flowID)
            break;
    }
    if (flowID_index == flowsOrderedByWeight->size())
        return FALSE;
    flowsOrderedByWeight->erase(flowsOrderedByWeight->begin() + flowID_index);
    flowsWeights->erase(flowsWeights->begin() + flowID_index);
    flowsRateAverage->erase(flowsRateAverage->begin() + flowID_index);
    flowOrderedByWeightPaths->erase(flowOrderedByWeightPaths->begin() + flowID_index);
    isactiveflow->erase(isactiveflow->begin() + flowID_index);
//delete the flow path from bestPaths
    removePath(allFlows->find(flowID)->second, bestPaths->at(flowID_index));
    bestPaths->erase(bestPaths->begin() + flowID_index);
    allFlows->erase(flowID);

    return TRUE;
//    uint i = 0;
//    uint sizenbreqs = nbreqs->size();
//    for (; i < sizenbreqs; i++) {
//        if (nbreqs->at(i) == northboundmsg) {
//            //TODO: there is a routing reservation already
//            EV << "The flow position: " << i << " / " << sizenbreqs << endl;
//            break;
//        }
//    }
//    if (i == sizenbreqs) {
//        EV << "This flow is already removed from the active set!" << endl;
//        return 0;
//    }
//
//    EV << "release the reserved links, with length " << linkpaths->at(i)->size()
//            << endl;
//    if (linkpaths->at(i)->size() != 0)
//        canStartNewFLow = 1;
//    EV << "Release the path: ";
//    for (uint j = 0; j < linkpaths->at(i)->size(); j++) {
//        Topology::LinkOut *curlink = linkpaths->at(i)->at(j);
//        EV << curlink->getRemoteNode()->getModule()->getName() << " ";
//        if (curlink->isEnabled()) {
//            EV << " ERROR: already enabled!" << endl;
//            return 0;
//        }
//        curlink->enable();
//    }
//    EV << endl;
//    nbreqs->erase(nbreqs->begin() + i);
//    LinkPaths::iterator todelpathiter = linkpaths->begin() + i;
//    delete *todelpathiter;
//    linkpaths->erase(todelpathiter);
//    return canStartNewFLow;
}
//find paths of queued flows on a new coming flow, ignoring active flows
int BEERs::flowScheduleMinSwitches(northboundMsg *nbmsg, bool isweight)
{
    return 0;
}

//find paths of queued flows on a new coming flow, ignoring active flows
int BEERs::flowScheduleMinSwitches(IPv4Address IPv4src, IPv4Address IPv4dst, northboundMsg *nbmsg, bool isweight)
{
//add flow into flow queue, and calculate flow paths.
    updateFlowRate();
    addNewFlow(IPv4src, IPv4dst, nbmsg);
    BasePaths *minPaths = new BasePaths;
    LinkBWs_t* linkBWs = new LinkBWs_t;
//find the set of flow paths that can get the minimal links.
//    traverseFlowPaths();
    time_t startTime = time(NULL);
    int minlinks = findMinPathset(0, minPaths, linkBWs);
    time_t endTime = time(NULL);
    EV << "findMinPathset for " << bestPaths->size() << " paths in " << (endTime - startTime) << "s" << endl;
    if (minlinks == -1)
    {
        EV << "There are multiple solutions for existing flows" << endl;
        //find the set of sending order of each flow.
    }
//    bestPaths = minPaths;
    EV << "The best paths(minimal links) are : " << endl;
    printBestPaths();
//    printPaths(bestPaths);
//    for (int p = 0; p < bestPaths->size(); p++) {
//        EV << p << "th path:\t";
//        BasePath * path = bestPaths->at(p);
//        if (path == 0) {
//            EV << "This flow has no path!" << endl;
//            continue;
//        }
//        int pathlength = path->length();
//        for (int pl = 0; pl < pathlength;) {
//            int vertexID = path->GetVertex(pl)->getID();
//            Topology::Node *vertexNode = topo_forw.getNode(vertexID);
//            EV << vertexNode->getModule()->getName();
//            pl++;
//            if (pl < pathlength)
//                EV << "->";
//            else
//                break;
//        }
//        EV << endl;
//    }
    return getPathNum(bestPaths);
}

//LinkBWs-curpath
bool BEERs::removePathFromLinkBWs(BasePath *curpath, LinkBWs_t* linkBWs, double flowrate)
{
    for (int j = curpath->length() - 2; j >= 0; j--)
    {
        int fromID = curpath->GetVertex(j)->getID();
        int toID = curpath->GetVertex(j + 1)->getID();

        if (linkBWs->find(fromID) == linkBWs->end())
        {
            LinkBW_available_t* linkBW = new LinkBW_available_t;
            linkBWs->insert(make_pair(fromID, linkBW));
        }

        LinkBW_available_t* linkBW = linkBWs->find(fromID)->second;
        if (linkBW->find(toID) == linkBW->end())
            linkBW->insert(make_pair(toID, linkCapacity));

        if (linkBW->find(toID)->second >= flowrate)
            continue;
        else
            return FALSE;
    }
    for (int j = curpath->length() - 2; j >= 0; j--)
    {
        int fromID = curpath->GetVertex(j)->getID();
        int toID = curpath->GetVertex(j + 1)->getID();
        LinkBW_available_t* linkBW = linkBWs->find(fromID)->second;
        linkBW->find(toID)->second -= flowrate;
    }
    return TRUE;
}

//LinkBWs+curpath
bool BEERs::releasePathFromLinkBWs(BasePath *curpath, LinkBWs_t* linkBWs, double flowrate)
{
    for (int j = curpath->length() - 2; j >= 0; j--)
    {
        int fromID = curpath->GetVertex(j)->getID();
        int toID = curpath->GetVertex(j + 1)->getID();
        LinkBW_available_t* linkBW = linkBWs->find(fromID)->second;
        linkBW->find(toID)->second += flowrate;
    }
    return true;
}
void BEERs::printBestPaths()
{
    for (int p = 0; p < bestPaths->size(); p++)
    {
        if (flowsRateAverage->at(p) > 0)
            EV << p << "th path: " << flowsRateAverage->at(p) << "Mbps\t";
        else
            EV << p << "th path should finished now if it send with full speed!!!" <<endl;
        BasePath* flowpath = bestPaths->at(p);
        if (flowpath == 0)
        {
            northboundMsg *msg = allFlows->find(flowsOrderedByWeight->at(p))->second;

            EV << "No path avaiable from" << msg->getMatch().OFB_IPV4_SRC << "  to  " << msg->getMatch().OFB_IPV4_DST << endl;
            continue;
        }
        printPath(flowpath);
    }

}
void BEERs::printPaths(BasePaths* flowpaths)
{
    for (int p = 0; p < flowpaths->size(); p++)
    {
        EV << p << "th path:\t";
        BasePath* flowpath = flowpaths->at(p);
        if (flowpath == 0)
        {

            EV << "This flow has no path!" << endl;
            continue;
        }
        printPath(flowpath);
    }
}
void BEERs::printPath(BasePath* flowpath)
{
    int pathlength = flowpath->length();
    for (int pl = 0; pl < pathlength;)
    {
        int vertexID = flowpath->GetVertex(pl)->getID();
        Topology::Node *vertexNode = topo_forw.getNode(vertexID);
        EV << vertexNode->getModule()->getName();
        pl++;
        if (pl < pathlength)
            EV << "->";
        else
            break;
    }
    EV << endl;
}
//input: 1st flow's id, paths,
//output: minimum number of links
int BEERs::findMinPathset(int FlowID_index, BasePaths *minPaths, LinkBWs_t* linkBWs)
{
//if this path is the best, return length;else return -1
    int bestlength;
    int length;
    if (FlowID_index == flowsOrderedByWeight->size())
    {
        if (bestPaths == NULL)
            bestPaths = new BasePaths;

        if (bestPaths->size() == 0)
        {
            for (int i = 0; i < minPaths->size(); i++)
                bestPaths->push_back(minPaths->at(i));
            bestlength = getPathNum(bestPaths);
            return bestlength;
        }
        else if (bestPaths->size() == minPaths->size())
        {
            bestlength = getPathNum(bestPaths);
//            printPaths(minPaths);
            length = getPathNum(minPaths);
//            EV << "Path Length: " << length << endl;
            //new best path set
            if (bestlength > length || bestlength <= 0)
            {
                bestPaths->clear();
                for (int i = 0; i < minPaths->size(); i++)
                    bestPaths->push_back(minPaths->at(i));
                return length;

            }
            else
            {
                return -1;
            }
        }
        else
        {
            return -1;
        }
        return getPathNum(minPaths);
    }

//link bandwidth > the path set. i.e. minPaths + curPath > link capacity?
    int linkNum = -1;
    FlowID_t curflow = flowsOrderedByWeight->at(FlowID_index);
    double flowRate = flowsRateAverage->at(FlowID_index);
    BasePaths *curpaths = getPathsByFlowID(curflow);
    for (uint i = 0; i < curpaths->size(); i++)
    {
        BasePath* curpath = curpaths->at(i);
        if (!removePathFromLinkBWs(curpath, linkBWs, flowRate))
            continue;
        minPaths->push_back(curpath);
        linkNum = findMinPathset(FlowID_index + 1, minPaths, linkBWs);
        minPaths->pop_back();
        releasePathFromLinkBWs(curpath, linkBWs, flowRate);
    }
    return linkNum;
}

//bool BEERs::traverseFlowPaths() {
////ergodically flows from highest to lowest avaialble
//
//    FlowPaths* minflowpaths = new FlowPaths;
//    //TODO: EXR can be implemented here with new flow paths checked by extra energy from the new paths
//    for (int i = flowsOrderedByWeight->size() - 1; i >= 0; i--) {
//        //1 get the flow
//        FlowID curflow = flowsOrderedByWeight->at(i);
//        northboundMsg * northboundmsg = allFlows->find(curflow)->second;
//        oxm_basic_match match = northboundmsg->getMatch();
//        IPv4Address IPv4src = match.OFB_IPV4_SRC;
//        IPv4Address IPv4dst = match.OFB_IPV4_DST;
//        //1.1 active flows would not change, so use the formal path
//        if (activeflowpaths->find(curflow) != activeflowpaths->end())
//            continue;
//        //1.2
//        BasePaths *curpaths = getPathsByFlowID(curflow);
//        for (int j = 0; j < curpaths->size(); i++) {
//            if (minflowpaths->find(curflow) == minflowpaths->end())
//                continue;
//
//            if (queuedflowpaths->find(curflow) == queuedflowpaths->end()) {
//                queuedflowpaths->insert(make_pair(curflow, curpaths->at(j)));
//            }
//        }
//        //1.3
//
//        //TODO: increamingly inserting flows
////        if (queuedflowpaths->find(curflow)!=queuedflowpaths->end())
////            continue;
//    }
//}

////find a path, reserve it
bool BEERs::findAndReserveRoutingFromsrcTodst(IPv4Address IPv4src, IPv4Address IPv4dst, bool isweight, bool enableWeightChange, bool enableReserve)
{
    return FALSE;
}
//bool BEERs::findAndReserveRoutingFromsrcTodst(IPv4Address IPv4src,
//        IPv4Address IPv4dst, bool isweight, bool enableWeightChange,
//        bool enableReserve) {
//    EV << "findAndReserveRoutingFromsrcTodst! " << IPv4src << " --> " << IPv4dst
//            << endl;
//    if (IPv4src.equals(IPv4dst)) {
//        LinkPath *curlinkpath = new LinkPath;
//        linkpaths->push_back(curlinkpath);
//        return true;
//    }
//    MACAddress thasrc = getMACfromIPv4(IPv4src);
//    MACAddress thadst = getMACfromIPv4(IPv4dst);
////    EV << "macsrc" << thasrc << "macdst" << thadst << endl;
//    int routeFound = 0;
//    Topology::Node * srcNode;
//    Topology::Node * destNode;
//    for (int i = 0; i < topo_forw.getNumNodes(); i++) {
//
//        Topology::Node *curNode = topo_forw.getNode(i);
//        //ignore ovs
//        if (curNode->getModule()->getFullName()[0] == 'o')
//            continue;
//
//        EtherMAC *eth = (EtherMAC *) curNode->getModule()->getSubmodule("eth",
//                0)->getSubmodule("mac");
//        MACAddress mac = eth->getMACAddress();
//        if (mac == thadst) {
//            destNode = curNode;
//            routeFound += 1;
//        } else if (mac == thasrc) {
//            srcNode = curNode;
//            routeFound += 1;
//        }
//    }
//    if (routeFound == 2) {
//        LinkPath *curlinkpath = NULL;
//        if (enableReserve) {
//            curlinkpath = new LinkPath;
//            linkpaths->push_back(curlinkpath);
//        }
//        std::string destModName = destNode->getModule()->getFullName();
//        if (isweight)
//            topo_forw.calculateWeightedSingleShortestPathsTo(destNode);
//        else
//            topo_forw.calculateUnweightedSingleShortestPathsTo(destNode);
//        if (srcNode->getNumPaths() == 0)
//            return false;
//        Topology::Node *nextNode = srcNode->getPath(0)->getRemoteNode();
//        while (nextNode != destNode) {
//            for (int j = 0; j < topo_forw.getNumNodes(); j++) {
//                // Only choose OpenFlow switches, otherwise proceed to next node in for loop
//                if (!nodeInfo[j].isOpenFlow)
//                    continue;
//                Topology::Node *atNode = topo_forw.getNode(j);
//
//                // Only choose nodes which have shortest path to destination node
//                if (atNode->getNumPaths() == 0)
//                    continue;
//                // Only proceed if nextNode has not been chosen yet or atNode is equal to nextNode
//                if (nextNode != NULL && atNode != nextNode)
//                    continue;
//                // Only proceed if connection id of atNode is equal to connection id of OpenFlow switch which has sent packet-in message
//                // or nextNode has been chosen
////                    if (nextNode == NULL && nodeInfo[j].connID != connID)
////                        continue;
//
//                //TODO: the link to be deleted
//                Topology::LinkOut* atlink = atNode->getPath(0);
//                //servers are initialized zero.
//                if (enableWeightChange) {
//                    Topology::Node * atnode = atlink->getRemoteNode();
//                    double atweight = atnode->getWeight();
//                    if (atweight != 0)
//                        atnode->setWeight(atweight / 2);
//                }
//                if (enableReserve) {
//                    curlinkpath->push_back(atlink);
//                    atlink->disable();
//                    EV << "disable link " << atNode->getModule()->getName()
//                            << " to "
//                            << atlink->getRemoteNode()->getModule()->getName()
//                            << endl;
//                }
//                uint32_t outputGateId =
//                        atNode->getPath(0)->getLocalGate()->getIndex();
//                uint32_t outport = outputGateId;
//                EV << "--link from " << atNode->getModule()->getFullName()
//                        << " Port " << outport << " to node "
//                        << atNode->getPath(0)->getRemoteNode()->getModule()->getFullName()
//                        << endl;
//                nextNode = atNode->getPath(0)->getRemoteNode();
//
//                connID_outport.push_back(
//                        std::pair<int, uint32_t>(nodeInfo[j].connID, outport));
//            }
//        }
//        typedef UnsortedMap::iterator iter;
//        iter begin(connID_outport.begin());
//        iter end(connID_outport.end());
//        reverse_iterator<iter> rev_end(begin);
//        reverse_iterator<iter> rev_iterator(end);
//        while (rev_iterator != rev_end) {
//            oxm_basic_match *match = new oxm_basic_match();
//            match->OFB_ETH_SRC = thasrc;
//            match->OFB_ETH_DST = thadst;
////          match->OFB_ETH_TYPE=
//            match->wildcards = OFPFW_IN_PORT + OFPFW_DL_TYPE;
//            controller->sendFlowModMessage(OFPFC_ADD, match,
//                    rev_iterator->second, rev_iterator->first);
//            rev_iterator++;
//        }
//        connID_outport.clear();
//    }
//    return routeFound == 2;
//}
//
bool BEERs::hasBidirectionpaths(IPv4Address IPv4src, IPv4Address IPv4dst, bool isweight)
{
    return FALSE;
}
//bool BEERs::hasBidirectionpaths(IPv4Address IPv4src, IPv4Address IPv4dst,
//        bool isweight) {
//    if (IPv4src.equals(IPv4dst))
//        return true;
//    MACAddress thasrc = getMACfromIPv4(IPv4src);
//    MACAddress thadst = getMACfromIPv4(IPv4dst);
//
//    int routeFound = 0;
//
//    Topology::Node * srcNode;
//    Topology::Node * destNode;
//    for (int i = 0; i < topo_forw.getNumNodes(); i++) {
//
//        Topology::Node *curNode = topo_forw.getNode(i);
//        //ignore ovs
//        if (curNode->getModule()->getFullName()[0] == 'o')
//            continue;
//
//        EtherMAC *eth = (EtherMAC *) curNode->getModule()->getSubmodule("eth",
//                0)->getSubmodule("mac");
//        MACAddress mac = eth->getMACAddress();
//        if (mac == thadst) {
//            destNode = curNode;
//            routeFound += 1;
//        } else if (mac == thasrc) {
//            srcNode = curNode;
//            routeFound += 1;
//        }
//    }
//    if (routeFound == 2) {
//        if (isweight)
//            topo_forw.calculateWeightedSingleShortestPathsTo(destNode);
//        else
//            topo_forw.calculateUnweightedSingleShortestPathsTo(destNode);
//        EV << "--src to dst " << srcNode->getNumPaths() << "paths" << endl;
//        if (srcNode->getNumPaths() == 0)
//            return false;
//        EV << "--dst to src " << srcNode->getNumPaths() << "paths" << endl;
//        topo_forw.calculateUnweightedSingleShortestPathsTo(srcNode);
//
//        if (destNode->getNumPaths() == 0)
//            return false;
//    }
//
//    return routeFound == 2;
//}
//
bool BEERs::findAndReserveRouting(IPv4Address IPv4src, IPv4Address IPv4dst, bool isweight)
{
    return FALSE;
}
//bool BEERs::findAndReserveRouting(IPv4Address IPv4src, IPv4Address IPv4dst,
//        bool isweight) {
//    bool isBiPathReserved = par("isBiPathReserved");
//    if (isBiPathReserved) {
//        EV << "findAndReserveRouting from " << IPv4src << "to" << IPv4dst
//                << endl;
//        bool bireouteFound = hasBidirectionpaths(IPv4src, IPv4dst, isweight);
//
//        if (isBiPathReserved)
//            if (bireouteFound) {
//                findAndReserveRoutingFromsrcTodst(IPv4dst, IPv4src, isweight,
//                        FALSE, TRUE);
//                findAndReserveRoutingFromsrcTodst(IPv4src, IPv4dst, isweight,
//                        isweight, TRUE);
//
//                LinkPath *secondlinkpath = linkpaths->back();
//                linkpaths->pop_back();
//                LinkPath *curlinkpath = linkpaths->back();
//                for (uint i = 0; i < secondlinkpath->size(); i++)
//                    curlinkpath->push_back(secondlinkpath->at(i));
//                EV << "Find src" << IPv4src << "to dst" << IPv4dst << endl;
//                return TRUE;
//            }
//        EV << "Find no" << IPv4src << "to dst" << IPv4dst << endl;
//        return FALSE;
//    } else {
//        return findTwoPathAndReserveOneRouting(IPv4src, IPv4dst, isweight);
//    }
//}
//
bool BEERs::findTwoPathAndReserveOneRouting(IPv4Address IPv4src, IPv4Address IPv4dst, bool isweight)
{
    return FALSE;
}
//bool BEERs::findTwoPathAndReserveOneRouting(IPv4Address IPv4src,
//        IPv4Address IPv4dst, bool isweight) {
//    EV << "find 2 And Reserve 1 Routing from " << IPv4src << "to" << IPv4dst
//            << endl;
////check whether bidirection is ok!
//    bool bireouteFound = hasBidirectionpaths(IPv4src, IPv4dst, true);
//
//    if (bireouteFound) {
//        //the ack would be the same as the flow direction, without influence on the statues of node and link.
//        findAndReserveRoutingFromsrcTodst(IPv4dst, IPv4src, isweight, FALSE,
//                FALSE);
//        //Thus the above should act first.
//        findAndReserveRoutingFromsrcTodst(IPv4src, IPv4dst, isweight, isweight,
//                TRUE);
//        EV << "Find src" << IPv4src << "to dst" << IPv4dst << endl;
//        return true;
//    }
//    EV << "Find no" << IPv4src << "to dst" << IPv4dst << endl;
//    return false;
//}

//void BEERs::receiveSignal(cComponent *src, simsignal_t id, cObject *obj) {
//    EV << "Received signal " << getSignalName(id) << " at module "
//            << this->getFullName() << "." << endl;
//    if (id == PacketInSignalId) {
//        if (dynamic_cast<OF_Wrapper *>(obj) != NULL) {
//            OF_Wrapper *wrapper = (OF_Wrapper *) obj;
//            Open_Flow_Message *of_msg = wrapper->msg;
//            OFP_Packet_In *packet_in = (OFP_Packet_In *) of_msg;
//            uint32_t buffer_id = packet_in->getBuffer_id();
//            MACAddress src_mac;
//            MACAddress dst_mac;
//            IPv4Address dst_ip;
//            if (buffer_id == OFP_NO_BUFFER) {
//                src_mac =
//                        dynamic_cast<EthernetIIFrame *>(packet_in->getEncapsulatedPacket())->getSrc();
//                dst_mac =
//                        dynamic_cast<EthernetIIFrame *>(packet_in->getEncapsulatedPacket())->getDest();
//            } else {
//                src_mac = packet_in->getMatch().OFB_ETH_SRC;
//                dst_mac = packet_in->getMatch().OFB_ETH_DST;
//            }
//            TCPCommand *ind =
//                    dynamic_cast<TCPCommand *>(packet_in->getControlInfo());
//            if (!ind)
//                opp_error(
//                        "Switch: no TCPCommand control info in message (not from TCP?)");
//            int connID = ind->getConnId();
//            EV << "connID: " << connID << endl;
//            int outputGateId;
//            bool routeFound = 0;
//            for (int i = 0; i < topo_forw.getNumNodes(); i++) {
//                Topology::Node *destNode = topo_forw.getNode(i);
//
//                EtherMAC *eth =
//                        (EtherMAC *) destNode->getModule()->getSubmodule("eth",
//                                0)->getSubmodule("mac");
//                MACAddress mac = eth->getMACAddress();
//// Skip forwarding for Ethernet frames with braodcast destination address
//                if (mac.isBroadcast())
//                    break;
//                if (mac == dst_mac) {
//                    routeFound = 1;
//                    std::string destModName =
//                            destNode->getModule()->getFullName();
//                    topo_forw.calculateUnweightedSingleShortestPathsTo(
//                            destNode);
//                    Topology::Node *nextNode = NULL;
//                    while (nextNode != destNode) {
//                        for (int j = 0; j < topo_forw.getNumNodes(); j++) {
//                            if (i == j)
//                                continue;
//                            // Only choose OpenFlow switches, otherwise proceed to next node in for loop
//                            if (!nodeInfo[j].isOpenFlow)
//                                continue;
//                            Topology::Node *atNode = topo_forw.getNode(j);
//                            // Only choose nodes which have shortest path to destination node
//                            if (atNode->getNumPaths() == 0)
//                                continue;
//                            // Only proceed if nextNode has not been chosen yet or atNode is equal to nextNode
//                            if (nextNode != NULL && atNode != nextNode)
//                                continue;
//                            // Only proceed if connection id of atNode is equal to connection id of OpenFlow switch which has sent packet-in message
//                            // or nextNode has been chosen
//                            if (nextNode == NULL
//                                    && nodeInfo[j].connID != connID)
//                                continue;
//                            outputGateId =
//                                    atNode->getPath(0)->getLocalGate()->getIndex();
//                            uint32_t outport = outputGateId;
//                            EV << "--";
//                            EV << atNode->getModule()->getFullName() << endl;
//                            EV << "port" << outport;
//                            EV << " to node "
//                                    << atNode->getPath(0)->getRemoteNode()->getModule()->getFullName()
//                                    << endl;
//                            nextNode = atNode->getPath(0)->getRemoteNode();
//                            connID_outport.push_back(
//                                    std::pair<int, uint32_t>(nodeInfo[j].connID,
//                                            outport));
//                        }
//                    }
//                    typedef UnsortedMap::iterator iter;
//                    iter begin(connID_outport.begin());
//                    iter end(connID_outport.end());
////                    std::map<int, uint32_t>::reverse_iterator i_first = connID_outport.end();
//                    reverse_iterator<iter> rev_end(begin);
//                    reverse_iterator<iter> rev_iterator(end);
////                    std::map<int, uint32_t>::reverse_iterator i_last = connID_outport.begin();
//                    while (rev_iterator != rev_end) {
//                        oxm_basic_match *match = new oxm_basic_match();
//                        if (buffer_id == OFP_NO_BUFFER) {
//                            //                            match->OFB_IN_PORT = packet_in->getEncapsulatedPacket()->getArrivalGate()->getIndex();
//                            match->OFB_ETH_SRC =
//                                    dynamic_cast<EthernetIIFrame *>(packet_in->getEncapsulatedPacket())->getSrc();
//                            match->OFB_ETH_DST =
//                                    dynamic_cast<EthernetIIFrame *>(packet_in->getEncapsulatedPacket())->getDest();
//                            match->OFB_ETH_TYPE =
//                                    dynamic_cast<EthernetIIFrame *>(packet_in->getEncapsulatedPacket())->getEtherType();
//                            match->wildcards = OFPFW_IN_PORT;
//                        } else {
//                            //                            match->OFB_IN_PORT = packet_in->getMatch().OFB_IN_PORT;
//                            match->OFB_ETH_SRC =
//                                    packet_in->getMatch().OFB_ETH_SRC;
//                            match->OFB_ETH_DST =
//                                    packet_in->getMatch().OFB_ETH_DST;
//                            match->OFB_ETH_TYPE =
//                                    packet_in->getMatch().OFB_ETH_TYPE;
//                            match->wildcards = OFPFW_IN_PORT;
//                        }
//                        controller->sendFlowModMessage(OFPFC_ADD, match,
//                                rev_iterator->second, rev_iterator->first);
//                        if (rev_iterator->first == connID)
//                            controller->sendPacket(buffer_id, packet_in,
//                                    rev_iterator->second, connID);
//                        rev_iterator++;
//                    }
//                    connID_outport.clear();
//                }
//            }
//            if (!routeFound && isArpOracle)
//                routeFound = processARPPacket(packet_in, connID);
//            if (routeFound == 0)
//                controller->floodPacket(buffer_id, packet_in, connID);
//        }
//
//    } else if (id == connIDSignal) {
//        if (dynamic_cast<OF_Wrapper *>(obj) != NULL) {
//            OF_Wrapper *wrapper = (OF_Wrapper *) obj;
//            int connID = wrapper->connID;
//            IPv4Address *ip_src = wrapper->ip_src;
//            for (int i = 0; i < topo_forw.getNumNodes(); i++) {
//                //                cTopology::Node *destNode = topo_forw.getNode(i);
//                //                EtherMAC *eth =  (EtherMAC *) destNode->getModule()->getSubmodule("eth", 0)->getSubmodule("mac");
//                //                                MACAddress mac = eth->getMACAddress();
//                IPv4Address *ip = &nodeInfo[i].address;
//                //                    EV << "-----------------------------------------------------------------" << endl;
//                //                    EV << *ip << " "<< *ip_src << endl;
//                if (*ip == *ip_src)
//                    nodeInfo[i].connID = connID;
//            }
//        }
//    }
//
//}
