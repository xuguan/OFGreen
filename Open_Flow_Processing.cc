#include <omnetpp.h>
#include <EtherFrame_m.h>
#include <MACAddress.h>
#include <ARPPacket_m.h>
#include "Flow_Table.h"
#include "Buffer.h"
#include "OF_Wrapper.h"
#include "TCPCommand_m.h"
#include "NotifierConsts.h"
#include "Open_Flow_Processing.h"
#include <vector>

using namespace std;

Define_Module(Open_Flow_Processing);

double Open_Flow_Processing::switch_active_time_overall = 0;
double Open_Flow_Processing::port_active_time_overall = 0;
double Open_Flow_Processing::energy_overall = 0;
double Open_Flow_Processing::linkUtilizationRatio = 0;
double Open_Flow_Processing::switchUtilizationRatio = 0;

Open_Flow_Processing::Open_Flow_Processing() {

}

Open_Flow_Processing::~Open_Flow_Processing() {

}
void Open_Flow_Processing::initialize() {
    cModule *ITModule = getParentModule()->getSubmodule("flow_Table");
    flow_table = check_and_cast<Flow_Table *>(ITModule);
    cModule *ITModule2 = getParentModule()->getSubmodule("buffer");
    buffer = check_and_cast<Buffer *>(ITModule2);

    NF_NO_MATCH_FOUND = registerSignal("NF_NO_MATCH_FOUND");
    NF_SEND_PACKET = registerSignal("NF_SEND_PACKET");
    NF_FLOOD_PACKET = registerSignal("NF_FLOOD_PACKET");

    getParentModule()->subscribe("NF_SEND_PACKET", this);
    getParentModule()->subscribe("NF_FLOOD_PACKET", this);

    WATCH_VECTOR(port_vector);
    // By default, all ports are enabled
    port_vector.resize(gateSize("ifIn"), 1);

    busy = false;
    serviceTime = par("serviceTime").doubleValue();
    powerSampleTime = par("powerSampleTime").doubleValue();
    active2sleepTime = par("active2sleepTime").doubleValue();
    active2sleepTimePort = par("active2sleepTimePort").doubleValue();
    linkCapacity = par("linkCapacity").doubleValue() * 125000; //from Mbps to Bps
    portNum = par("portNum");
    portTx = par("portTx");
    portpower = par("portpower");
    switchpower = par("switchpower");
    lastActiveTime = 0;
    switchActiveTime = 0;
    int n = gateCount() / 2; //we half inport, half outport.

    for (int i = 0; i < n; i++)
        portLastActiveTime.push_back(0);
    switchActiveTimeOverallVector.setName("switch Active Time Overall Vector");
//    switchActiveTimeVector.setName("switch Active Time Vector");
    portActiveTimeOverallVector.setName("port Active Time Overall Vector");
//    portActiveTimeVector.setName("port Active Time Vector");
    energyVector.setName("increasing total energy at different time vector");
    linkUtilizationRatioVector.setName("link utilization ratio");
    switchUtilizationRatioVector.setName("switch utilization ratio");
    portActiveTime = 0;
    cMessage *timeevent = new cMessage("timeevent");
    scheduleAt(simTime() + 100 * serviceTime, timeevent);
}

// invoked by Spanning Tree module;
// disable ports for broadcast packets
void Open_Flow_Processing::disablePorts(vector<int> ports) {
    EV << "disablePorts method at " << this->getParentModule()->getName()
            << endl;

    for (unsigned int i = 0; i < ports.size(); i++) {
        port_vector[ports[i]] = 0;
    }
    for (unsigned int i = 0; i < port_vector.size(); i++) {
        // Highlight links that belong to spanning tree

        if (port_vector[i] == 1) {
            cGate *gateOut = getParentModule()->gate("ethg$o", i);
            do {
                cDisplayString& connDispStrOut = gateOut->getDisplayString();
                connDispStrOut.parse("ls=green,3,dashed");
                gateOut = gateOut->getNextGate();

            } while (!gateOut->getOwnerModule()->getModuleType()->isSimple());

            cGate *gateIn = getParentModule()->gate("ethg$i", i);
            do {
                cDisplayString& connDispStrIn = gateIn->getDisplayString();
                connDispStrIn.parse("ls=green,3,dashed");
                gateIn = gateIn->getPreviousGate();

            } while (!gateIn->getOwnerModule()->getModuleType()->isSimple());
        }
    }
}

void Open_Flow_Processing::handleMessage(cMessage *msg) {
    // self message to implement service time
    if (msg->isSelfMessage()) {
        const char *msg_name = msg->getName();
        if (!strcmp(msg_name, "timeevent")) {
            if (switch_active_time_overall + port_active_time_overall != 0) {
                energy_overall += port_active_time_overall * portpower
                        + switch_active_time_overall * switchpower;
                energyVector.record(energy_overall);
            }
            if (switch_active_time_overall != 0) {
                double switchCapacity_overall = switch_active_time_overall
                        * linkCapacity * portNum * 2; //*2 means bi-direction throughput.
                double switchutilizationRatio = linkUtilizationRatio
                        / switchCapacity_overall;
                switchUtilizationRatioVector.record(switchutilizationRatio);

                switchActiveTimeOverallVector.record(
                        switch_active_time_overall);
                switch_active_time_overall = 0;
            }
            if (port_active_time_overall != 0) {
                double linkCapacity_overall = port_active_time_overall
                        * linkCapacity * 2; //*2 means that we consider bi-direction together.
                linkUtilizationRatio /= linkCapacity_overall;
                linkUtilizationRatioVector.record(linkUtilizationRatio);
                linkUtilizationRatio = 0;
                portActiveTimeOverallVector.record(port_active_time_overall);
                port_active_time_overall = 0;
            }
            scheduleAt(simTime() + powerSampleTime, msg);
            return;
        }
        cMessage *data_msg = (cMessage *) msg->getContextPointer();
        delete msg;
        processQueuedMsg(data_msg);

        //lastActiveTime
        double nowtime = simTime().dbl();
        double activeInterval = 0;
        if (nowtime > lastActiveTime + active2sleepTime + serviceTime) {
            activeInterval = active2sleepTime + serviceTime;
        } else {
            activeInterval = nowtime - lastActiveTime;
        }
//        switchActiveTime += activeInterval;
//        switchActiveTimeVector.record(switchActiveTime);
        switch_active_time_overall += activeInterval;
        lastActiveTime = nowtime;

        if (msg_list.empty()) {
            busy = false;
        } else {
            cMessage *msgfromlist = msg_list.front();
            msg_list.pop_front();
            char buf[80];
            sprintf(buf, " %d pakets in queue", msg_list.size());
            getParentModule()->getDisplayString().setTagArg("t", 0, buf);
            std::list<cMessage *>::iterator i = msg_list.begin();
            while (i != msg_list.end()) {
                EV << (*i)->getFullPath() << endl;
                i++;
            }
            cMessage *event = new cMessage("event");
            event->setContextPointer(msgfromlist);
            scheduleAt(simTime() + serviceTime, event);
        }
    } else {
        cPacket * pkt_msg = (cPacket *) msg;
        int inportID = msg->getArrivalGate()->getIndex();
        portEnergyStatistic(inportID, int(pkt_msg->getByteLength()));
        EV << "Recording Receiving Gate "
                << msg->getArrivalGate()->getFullPath().c_str()
                << "  with index " << msg->getArrivalGate()->getIndex() << endl;

        if (busy) {
            msg_list.push_back(msg);
            char buf[80];
            sprintf(buf, " %d pakets in queue", msg_list.size());
            getParentModule()->getDisplayString().setTagArg("t", 0, buf);
        } else {

            busy = true;

            cMessage *event = new cMessage("event");
            event->setContextPointer(msg);
            scheduleAt(simTime() + serviceTime, event);
        }
    }
}

void Open_Flow_Processing::processQueuedMsg(cMessage *data_msg) {
    if (dynamic_cast<EthernetIIFrame *>(data_msg) != NULL) {
        frameBeingReceived = (EthernetIIFrame *) data_msg;

        EV << "Broadcast?: " << frameBeingReceived->getDest().isBroadcast()
                << endl;
        oxm_basic_match *match = new oxm_basic_match();

        //extract match fields
        match->OFB_IN_PORT = frameBeingReceived->getArrivalGate()->getIndex();
        match->OFB_ETH_SRC = frameBeingReceived->getSrc();
        match->OFB_ETH_DST = frameBeingReceived->getDest();
        match->OFB_ETH_TYPE = frameBeingReceived->getEtherType();

        if (frameBeingReceived->getEtherType() == ETHERTYPE_IPv4) {

            IPv4Datagram * IPv4Data = check_and_cast<IPv4Datagram *>(
                    frameBeingReceived->getEncapsulatedPacket());
            match->OFB_IPV4_SRC = IPv4Data->getSrcAddress();
            match->OFB_IPV4_DST = IPv4Data->getDestAddress();
        }
        //extract ARP specific match fields if present
        if (frameBeingReceived->getEtherType() == ETHERTYPE_ARP) {
            ARPPacket *arpPacket = check_and_cast<ARPPacket *>(
                    frameBeingReceived->getEncapsulatedPacket());
            match->OFB_ARP_OP = arpPacket->getOpcode();
            match->OFB_ARP_SHA = arpPacket->getSrcMACAddress();
            match->OFB_ARP_THA = arpPacket->getDestMACAddress();
            match->OFB_ARP_SPA = arpPacket->getSrcIPAddress();
            match->OFB_ARP_TPA = arpPacket->getDestIPAddress();
        }

        if (flow_table->lookup(match)) {
            //lookup successful
            EV << "Found entry in flow table.\n" << endl;
            ofp_action_output *action_output = flow_table->returnAction(match);
            uint32_t outport = action_output->port;

            send(frameBeingReceived, "ifOut", outport);

            portEnergyStatistic(frameBeingReceived->getSenderGate()->getIndex(),
                    int(frameBeingReceived->getByteLength()));

            EV << "Recording Sender Gate "
                    << frameBeingReceived->getSenderGate()->getFullPath().c_str()
                    << "  with index "
                    << frameBeingReceived->getSenderGate()->getIndex() << endl;

        } else {
            // lookup fail; notification to Switch Application module
            EV << "owner: " << frameBeingReceived->getOwner()->getFullName()
                    << endl;
            EV << "queue in buffer" << endl;
            drop(data_msg);
            buffer->push(frameBeingReceived);
            OF_Wrapper *dummy = new OF_Wrapper();
            emit(NF_NO_MATCH_FOUND, dummy);
        }
        delete match;
    }

}

void Open_Flow_Processing::inPortEnergyStatistic(int portID, int packetsize) {
    double pkttx = packetsize;
    pkttx /= linkCapacity; //linkCapacity*1M/8bit
    if (portID >= portLastActiveTime.size())
        return;
    double nowTime = simTime().dbl();
    double lastTime = portLastActiveTime[portID];
    double activeTime = 0;
    if (nowTime - pkttx - lastTime > active2sleepTimePort)
        activeTime = pkttx + active2sleepTimePort;
    else
        activeTime = nowTime - lastTime;
    port_active_time_overall += activeTime; // * 8;
    linkUtilizationRatio += packetsize;
    portLastActiveTime[portID] = nowTime;
//    portActiveTime += activeTime;
//    portActiveTimeVector.record(portActiveTime);
}
void Open_Flow_Processing::portEnergyStatistic(int portID, int packetsize) {
    double pkttx = packetsize;
    pkttx /= linkCapacity; //linkCapacity*1M/8bit
    if (portID >= portLastActiveTime.size())
        return;
    double nowTime = simTime().dbl();
    double lastTime = portLastActiveTime[portID];
    double activeTime = 0;
    if (nowTime - pkttx - lastTime > active2sleepTimePort)
        activeTime = pkttx + active2sleepTimePort;
    else
        activeTime = nowTime - lastTime;
    port_active_time_overall += activeTime; // * 8;
    linkUtilizationRatio += packetsize;
    portLastActiveTime[portID] = nowTime;
//    portActiveTime += activeTime;
//    portActiveTimeVector.record(portActiveTime);

}
void Open_Flow_Processing::receiveSignal(cComponent *src, simsignal_t id,
        cObject *obj) {
    Enter_Method_Silent();
    //printNotificationBanner(category, details);

    if (id == NF_FLOOD_PACKET) {
        //flood packet
        OF_Wrapper *wrapper = (OF_Wrapper *) obj;
        uint32_t buffer_id = wrapper->buffer_id;
        EV << "Processing buffer-id: " << buffer_id << endl;
        EthernetIIFrame *frame;
        if (buffer_id == OFP_NO_BUFFER) {
            frame = wrapper->frame;
        } else {
            frame = buffer->returnMessage(buffer_id);
        }
        take(frame);
        int n = gateSize("ifOut");
        //int numClients = getParentModule()->getParentModule()->par("numClients");
        for (int i = 0; i < n; i++) {
            if (i == frame->getArrivalGate()->getIndex()
                    || (port_vector[i] == 0))
                continue;
            EthernetIIFrame *copy = frame->dup();
            EV << "EthernetIIFrame ID: " << copy->getId() << endl;
            send(copy, "ifOut", i);
//                 cDisplayString& connDispStr = getParentModule()->getParentModule()->gate("gate$o", i)->getDisplayString();
//                             connDispStr.parse("ls=red");
        }
        delete frame;

    }
    if (id == NF_SEND_PACKET) {
        // send packet on outport port
        OF_Wrapper *wrapper = (OF_Wrapper *) obj;
        uint32_t buffer_id = wrapper->buffer_id;
        uint16_t outport = wrapper->outport;
        EthernetIIFrame *frame;
        if (buffer_id == OFP_NO_BUFFER) {
            frame = wrapper->frame;
        } else {
            frame = buffer->returnMessage(buffer_id);
        }
        take(frame);
        send(frame, "ifOut", outport);
    }
}

