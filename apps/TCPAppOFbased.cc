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

#include "TCPAppOFbased.h"
#include "northboundWrapper.h"
#include "IPvXAddressResolver.h"
#include "TCPSocket.h"
#include "RoutingTable.h"
#include "northboundMsg_m.h"
#include "TCP.h"
#include "TCPConnection.h"
#include "GenericAppMsg_m.h"
#include "TCP_Session_Message_m.h"

Define_Module(TCPAppOFbased);
void TCPAppOFbased::initialize(int stage) {

    // TODO Auto-generated constructor stub
//    BandwidthReqSignalId = registerSignal("BandwidthReq");
//    BandwidthResSignalId = registerSignal("BandwidthRes");
//    getParentModule()->getParentModule()->subscribe(BandwidthResSignalId, this);
//    simulation.getSystemModule()->subscribe("BandwidthRes", this);

    packetsRcvd = indicationsRcvd = 0;
    bytesRcvd = bytesSent = 0;
    randomGenerator = par("randomGenerator");
    WATCH(packetsRcvd);
    WATCH(bytesRcvd);
    WATCH(indicationsRcvd);

    rcvdPkSignal = registerSignal("rcvdPk");
    sentPkSignal = registerSignal("sentPk");
    recvIndicationsSignal = registerSignal("recvIndications");
    sendTime = 0;
    flowCompletionTime.setName("flow Completion Time");

}
//TCPAppOFbased::TCPAppOFbased() {
//
//}

//void TCPAppOFbased::receiveSignal(cComponent *src, simsignal_t id,
//        cObject *obj) {
//    Enter_Method_Silent();
//
//    if (id == BandwidthResSignalId) {
//
//        if (dynamic_cast<northboundWrapper *>(obj) != NULL) {
//            northboundWrapper *wrapper = (northboundWrapper *) obj;
//
//            if (!wrapper->ip_dst.equals(connetAddr))
//                return;
//            if (!wrapper->ip_src.equals(localAddr))
//                return;
////            if (wrapper->ip_dst.equals(connetAddr)
////                    && wrapper->ip_src.equals(localAddr)) {
//
////TODO choose whether to start the flow imediately
////    TCPSessionApp::activity();
//            EV << "Received signal " << getSignalName(id) << " at module "
//                    << this->getFullPath().c_str() << "." << endl;
//            bool active = par("active");
//
//            const char *script = par("sendScript");
//            parseScript(script);
//
//            if (sendBytes > 0 && commands.size() > 0)
//                throw cRuntimeError(
//                        "Cannot use both sendScript and tSend+sendBytes");
//
//            socket.setOutputGate(gate("tcpOut"));
//
//            // open
////                    waitUntil(tOpen);
//
//            socket.readDataTransferModePar(*this);
//            socket.bind(localAddr, localPort);
//
//            EV << "issuing OPEN command\n";
//
//            if (ev.isGUI())
//                getDisplayString().setTagArg("t", 0,
//                        active ? "connecting" : "listening");
//
//            if (active)
//                socket.connect(connetAddr, connectPort);
//            else
//                socket.listenOnce();
//
//            // wait until connection gets established
//            while (socket.getState() != TCPSocket::CONNECTED) {
//                socket.processMessage(receive());
//
//                if (socket.getState() == TCPSocket::SOCKERROR)
//                    return;
//            }
//
//            EV << "connection established, starting sending\n";
//            if (ev.isGUI())
//                getDisplayString().setTagArg("t", 0, "connected");
//
//            if (sendBytes > 0) {
//                EV << "sending " << sendBytes << " bytes\n";
//                cPacket *msg = genDataMsg(sendBytes);
//                bytesSent += sendBytes;
//                emit(sentPkSignal, msg);
//                socket.send(msg);
//
//                double serviceTime = sendBytes;
//                serviceTime /= 12500000;
//                EV << "wait for a sending interval" << serviceTime << endl;
//
////                    waitUntil(serviceTime);
//                cMessage *timeoutmsg = new cMessage("event");
//                timeoutmsg->setContextPointer(wrapper);
//                scheduleAt(simTime() + serviceTime, timeoutmsg);
//                cMessage * curmsg = NULL;
//                while ((curmsg = receive()) != timeoutmsg) {
//                    count(curmsg);
//                    socket.processMessage(curmsg);
//                }
//                EV << "flow transmission finishing!" << endl;
//                wrapper->canEnd = 1;
//                emit(BandwidthReqSignalId, wrapper);
//            }
//
//            for (CommandVector::iterator i = commands.begin();
//                    i != commands.end(); ++i) {
//                waitUntil(i->tSend);
//                EV << "sending " << i->numBytes << " bytes\n";
//                cPacket *msg = genDataMsg(i->numBytes);
//                bytesSent += i->numBytes;
//                emit(sentPkSignal, msg);
//                socket.send(msg);
//            }
//
//            simtime_t tClose = par("tClose");
//            if (tClose >= 0) {
//                waitUntil(tClose);
//                EV << "issuing CLOSE command\n";
//
//                if (ev.isGUI())
//                    getDisplayString().setTagArg("t", 0, "closing");
//
//                socket.close();
//            }
//
//            for (;;) {
//                cMessage *msg = receive();
//                count(msg);
//                socket.processMessage(msg);
//            }
//            return;
////            }
//        }
//
//    }
//}

//void TCPAppOFbased::handleMessage(cMessage *msg) {
//
//    if (msg->isSelfMessage()) {
//        EV << msg->getArrivalModule()->getFullName();
//        return;
//    }
////
////    EV << "flow transmission finishing!" << endl;
////    northboundWrapper *wrapper = (northboundWrapper *) msg->getContextPointer();
////    wrapper->canEnd = 1;
////    emit(BandwidthReqSignalId, wrapper);
////
////    simtime_t tClose = par("tClose");
////    if (tClose >= 0) {
////        waitUntil(tClose);
////        EV << "issuing CLOSE command\n";
////
////        if (ev.isGUI())
////            getDisplayString().setTagArg("t", 0, "closing");
////
////        socket.close();
////    }
////
////    for (;;) {
////        cMessage *msg = receive();
////        count(msg);
////        socket.processMessage(msg);
////    }
//
////     TODO Auto-generated destructor stub
//    northboundMsg * northboundmsg = check_and_cast<northboundMsg *>(msg);
//    oxm_basic_match match = northboundmsg->getMatch();
//    IPv4Address IPv4src = match.OFB_IPV4_SRC;
//    IPv4Address IPv4dst = match.OFB_IPV4_DST;
//}
//TCPAppOFbased::~TCPAppOFbased() {
//
//}

void TCPAppOFbased::socketDataArrived(int connId, void *yourPtr, cPacket *msg,
        bool urgent) {
    ev << "Received TCP data, " << msg->getByteLength() << " bytes\\n";
    delete msg;
}

void TCPAppOFbased::activity() {
    //TODO send a tcp flow request to controller
//    cModule *destinationModule = getParentModule()->getParentModule()->getSubmodule("controller")->getSubmodule("ctrlApp");
//    double delay = truncnormal(0.005, 0.0001);
//    sendDirect(new cMessage("packet"), delay, destinationModule, "inputGate");
//    sendDirect(new cMessage("packet"), destinationModule, "inputGate");

    isTimeOut = 0;
    localPort = par("localPort");
    const char *connectAddress = par("connectAddress").stdstringValue().c_str();
    connectPort = par("connectPort");
    const char *localAddress = this->getParentModule()->getFullPath().c_str();
    while (!strcmp(connectAddress, localAddress)) {
        connectAddress = par("connectAddress").stdstringValue().c_str();
    }
    localAddr = IPvXAddressResolver().resolve(localAddress);
    connetAddr = IPvXAddressResolver().resolve(connectAddress);
    simtime_t tOpen = par("tOpen"); //tOpen does not
    simtime_t tSend = par("tSend");
    simtime_t tClose = par("tClose");
    simtime_t tDeadline = par("tDeadline");
    sendBytes = par("sendBytes");
    waitUntil(tSend);
    int flowScheduleType = par("flowScheduleType");
    if (flowScheduleType == 1) {

//        //check whether the controller is able to handle the signal
//        if (!(mayHaveListeners(BandwidthReqSignalId)))
//            return;

//        northboundWrapper *wrapper = new northboundWrapper();
//        wrapper->ip_dst = connetAddr;
//        wrapper->port_dst = localPort;
//        wrapper->port_src = connectPort;
//        wrapper->ip_src = localAddr;
//        wrapper->tOpen = tOpen;
//        wrapper->tClose = tClose;
//        wrapper->sendBytes = sendBytes;

        EV << this->getParentModule()->getFullPath() << "(" << localAddr
                << ")  to  " << connectAddress << "(" << connetAddr << ")"
                << endl;
        cModule * ctrlModle =
                this->getParentModule()->getParentModule()->getSubmodule(
                        "controller")->getSubmodule("ctrlApp");
        northboundMsg * northboundmsg = new northboundMsg();
        oxm_basic_match *match = new oxm_basic_match();
        match->OFB_IPV4_DST = connetAddr.get4();
        match->OFB_IPV4_SRC = localAddr.get4();
        match->OFB_ETH_TYPE = 0x0800; //ETHERTYPE_IPv4
        match->OFB_TP_DST = connectPort;
        match->OFB_TP_SRC = localPort;
        match->wildcards = OFPFW_IN_PORT + OFPFW_DL_SRC + OFPFW_DL_DST
                + OFPFW_TP_DST + OFPFW_TP_SRC;
        EV << "Sending nb request " << localAddr.get4() << "  to  "
                << connetAddr.get4() << endl;
        northboundmsg->setMatch(*match);
        northboundmsg->setSendBytes(sendBytes);
        northboundmsg->setTdeadline(simTime() + tDeadline);
        northboundmsg->setTOpen(tOpen);
        northboundmsg->setTClose(tClose);
        northboundmsg->setCanSend(0);
        northboundmsg->setCanEnd(0);
        cModule * clientModule = this;
        northboundmsg->setClientModuleptr((int) clientModule);
        sendDirect(northboundmsg, ctrlModle->gate("crtlIn"));

        cMessage *timeoutMsg = new cMessage("timeout");
        scheduleAt(tClose, timeoutMsg);
        cModule * TCPModle = this->getParentModule()->getSubmodule("tcp");

        cMessage *msg = NULL;
        while ((msg = receive()) != timeoutMsg) {
            northboundMsg * northboundmsg = check_and_cast<northboundMsg *>(
                    msg);
            if (northboundmsg == 0)
                continue;
            oxm_basic_match match = northboundmsg->getMatch();
            IPv4Address IPv4src = match.OFB_IPV4_SRC;
            IPv4Address IPv4dst = match.OFB_IPV4_DST;
            EV << "Receiving nb response of " << IPv4dst << "  to  " << IPv4src
                    << "\t";
            if (northboundmsg->getCanSend()) {
//                startSending(IPv4src, IPv4dst);
                EV << "Can send" << endl;
                bool active = par("active");
                socket.setOutputGate(gate("tcpOut"));
                socket.readDataTransferModePar(*this);
                socket.bind(localAddr, localPort);
                EV << "issuing OPEN command\n";

                if (ev.isGUI())
                    getDisplayString().setTagArg("t", 0,
                            active ? "connecting" : "listening");

                if (active)
                    socket.connect(connetAddr, connectPort);
                else
                    socket.listenOnce();

                // wait until connection gets established
                while (socket.getState() != TCPSocket::CONNECTED) {
                    socket.processMessage(receive());

                    if (socket.getState() == TCPSocket::SOCKERROR)
                        return;
                }

                EV << "connection established, starting sending\n";
                if (ev.isGUI())
                    getDisplayString().setTagArg("t", 0, "connected");

                if (sendBytes > 0) {
                    long onceBytes = sendBytes * 1000000; //B
//                    waitUntil(tSend);
                    EV << "sending " << onceBytes << " bytes\n";
                    cPacket *msg = genDataMsg(onceBytes);
                    msg->setKind(3);
                    bytesSent += onceBytes;
                    if (ev.isGUI()) {
                        char buf[32];
                        sprintf(buf, "sent: %ld bytes", bytesSent);
                        getDisplayString().setTagArg("t", 0, buf);
                    }
                    emit(sentPkSignal, msg);
                    socket.send(msg);

                    cPacket *finishMsg = new cPacket("finishingPkt");
                    finishMsg->addPar("finishingTag")= "finishingTag";
                    finishMsg->setByteLength(111);
                    emit(sentPkSignal, finishMsg);
                    socket.send(finishMsg);

                    cMessage *timeoutMsg = new cMessage("timeout");
                    scheduleAt(tClose, timeoutMsg);
                    cMessage *tmpmsg = NULL;

                    while (tmpmsg = receive()) {
                        if (tmpmsg != timeoutMsg) {
                            if (tmpmsg->getKind() == TCP_I_DATA
                                    || tmpmsg->getKind() == TCP_I_URGENT_DATA) {
                                cPacket *pkt = check_and_cast<cPacket *>(
                                        tmpmsg);
                                int exist = pkt->getParList().exist("finishingTagRes");
                                if (exist) {
                                    EV << "Sending finished" << endl;
                                    flowCompletionTime.record(
                                            simTime() - tSend);
                                    break;
                                }
                            }
                            count(tmpmsg);
                            socket.processMessage(tmpmsg);
                        }
                    }
                    if (timeoutMsg->isScheduled())
                        cancelAndDelete(timeoutMsg);
                    else
                        delete timeoutMsg;

//                    EV << "flow transmission finishing!" << endl;
//                    wrapper->canEnd = 1;
//                    emit(BandwidthReqSignalId, wrapper);
                    northboundmsg->setCanEnd(1);
                    sendDirect(northboundmsg, ctrlModle->gate("crtlIn"));
                    socket.close();
                }

//                simtime_t tClose = par("tClose");
//                if (tClose >= 0) {
//                    waitUntil(tClose);
//                    EV << "issuing CLOSE command\n";
//
//                    if (ev.isGUI())
//                        getDisplayString().setTagArg("t", 0, "closing");
//
//                    socket.close();
//                }

                for (;;) {
                    cMessage *msg = receive();
                    count(msg);
                    socket.processMessage(msg);
                }
            }
            EV << "Canot send" << endl;

//            count(msg);
//            socket.processMessage(msg);
        }

        if (simTime() >= tClose)
            isTimeOut = 1;
        delete timeoutMsg;

    } else if (flowScheduleType == 2) {
        bool active = par("active");
        const char *script = par("sendScript");
        parseScript(script);

        if (sendBytes > 0 && commands.size() > 0)
            throw cRuntimeError(
                    "Cannot use both sendScript and tSend+sendBytes");

        socket.setOutputGate(gate("tcpOut"));

        // open
        waitUntil(tOpen);

        socket.readDataTransferModePar(*this);
        socket.bind(localAddr, localPort);

        EV << "issuing OPEN command\n";

        if (ev.isGUI())
            getDisplayString().setTagArg("t", 0,
                    active ? "connecting" : "listening");

        if (active)
            socket.connect(connetAddr, connectPort);
        else
            socket.listenOnce();

        // wait until connection gets established
        while (socket.getState() != TCPSocket::CONNECTED) {
            socket.processMessage(receive());

            if (socket.getState() == TCPSocket::SOCKERROR)
                return;
        }

        EV << "connection established, starting sending\n";
        if (ev.isGUI())
            getDisplayString().setTagArg("t", 0, "connected");

        if (sendBytes > 0) {
            long onceBytes = sendBytes * 1000000; //B
            waitUntil(tSend);
            EV << "sending " << onceBytes << " bytes\n";
            cPacket *msg = genDataMsg(onceBytes);
            bytesSent += onceBytes;
            emit(sentPkSignal, msg);
            socket.send(msg);

            cPacket *finishMsg = new cPacket("finishingPkt");
            finishMsg->addPar("finishingTag")= "finishingTag";
            finishMsg->setByteLength(222);
            emit(sentPkSignal, finishMsg);
            socket.send(finishMsg);
        }

//        for (CommandVector::iterator i = commands.begin(); i != commands.end();
//                ++i) {
//            waitUntil(i->tSend);
//            EV << "sending " << i->numBytes << " bytes\n";
//            cPacket *msg = genDataMsg(i->numBytes);
//            bytesSent += i->numBytes;
//            emit(sentPkSignal, msg);
//            socket.send(msg);
//        }

        //TODO: this may start when a new session is started.
        //TODO: ideally, this start when the process finished.
//        // close
//        if (tClose >= 0) {
//            waitUntil(tClose);
//            EV << "issuing CLOSE command\n";
//
//            if (ev.isGUI())
//                getDisplayString().setTagArg("t", 0, "closing");
//
//            socket.close();
//        }
//
//        // wait until peer closes too and all data arrive
//        for (;;) {
//            cMessage *msg = receive();
//            count(msg);
//            socket.processMessage(msg);
//        }
        //                waitUntil(tClose);
        EV << "issuing CLOSE command\n";

        if (ev.isGUI())
            getDisplayString().setTagArg("t", 0, "closing");

        // close
//        if (tClose >= 0) {
//            waitUntil(tClose);
//            EV << "issuing CLOSE command\n";
//
//            if (ev.isGUI())
//                getDisplayString().setTagArg("t", 0, "closing");
//
//            socket.close();
//        }

        cMessage *timeoutMsg = new cMessage("timeout");
        scheduleAt(tClose, timeoutMsg);
        cMessage *msg = NULL;

        while (msg = receive()) {
            if (msg != timeoutMsg) {
                if (msg->getKind() == TCP_I_DATA
                        || msg->getKind() == TCP_I_URGENT_DATA) {
                    cPacket *pkt = check_and_cast<cPacket *>(msg);
                    int exist = pkt->getParList().exist("finishingTagRes");
                    if (exist) {
                        EV << "Sending finished" << endl;
                        flowCompletionTime.record(simTime() - tSend);
                        break;
                    }
                    delete msg;
                } else {
                    count(msg);
                    socket.processMessage(msg);
                }
            }
        }
        if (timeoutMsg->isScheduled())
            cancelAndDelete(timeoutMsg);
        else
            delete timeoutMsg;
        socket.close();
        for (;;) {
            cMessage *msg = receive();
            count(msg);
            socket.processMessage(msg);
        }

    }
}
void TCPAppOFbased::startSending(IPv4Address IPv4src, IPv4Address IPv4dst) {
//    bool active = par("active");
//
//    socket.setOutputGate(gate("tcpOut"));
//    socket.readDataTransferModePar(*this);
//    socket.bind(localAddr, localPort);
//
//    EV << "issuing OPEN command\n";
//
//    if (ev.isGUI())
//        getDisplayString().setTagArg("t", 0,
//                active ? "connecting" : "listening");
//
//    if (active)
//        socket.connect(connetAddr, connectPort);
//    else
//        socket.listenOnce();
//
//    // wait until connection gets established
//    while (socket.getState() != TCPSocket::CONNECTED) {
//        socket.processMessage(receive());
//
//        if (socket.getState() == TCPSocket::SOCKERROR)
//            return;
//    }
//
//    EV << "connection established, starting sending\n";
//    if (ev.isGUI())
//        getDisplayString().setTagArg("t", 0, "connected");
//
//    if (sendBytes > 0) {
//        EV << "sending " << sendBytes << " bytes\n";
//        cPacket *msg = genDataMsg(sendBytes);
//        bytesSent += sendBytes;
//        emit(sentPkSignal, msg);
//        socket.send(msg);
//
//        double serviceTime = sendBytes;
//        serviceTime /= 12500000;
//        EV << "wait for a sending interval" << serviceTime << endl;
//
////                    waitUntil(serviceTime);
//        cMessage *timeoutmsg = new cMessage("event");
//        timeoutmsg->setContextPointer(wrapper);
//        scheduleAt(simTime() + serviceTime, timeoutmsg);
//        cMessage * curmsg = NULL;
//        while ((curmsg = receive()) != timeoutmsg) {
//            count(curmsg);
//            socket.processMessage(curmsg);
//        }
//        EV << "flow transmission finishing!" << endl;
//        wrapper->canEnd = 1;
//        emit(BandwidthReqSignalId, wrapper);
//    }
//
//    simtime_t tClose = par("tClose");
//    if (tClose >= 0) {
//        waitUntil(tClose);
//        EV << "issuing CLOSE command\n";
//
//        if (ev.isGUI())
//            getDisplayString().setTagArg("t", 0, "closing");
//
//        socket.close();
//    }
//
//    for (;;) {
//        cMessage *msg = receive();
//        count(msg);
//        socket.processMessage(msg);
//    }
    return;
}
void TCPAppOFbased::finish() {
    if (bytesRcvd != 0) {
        EV << getFullPath() << ": received " << bytesRcvd << " bytes in "
                << packetsRcvd << " packets\n";
    }
    EV << getFullPath() << ": " << localAddr << " to " << connetAddr << endl;
    recordScalar("bytesRcvd", bytesRcvd);
    recordScalar("bytesSent", bytesSent);

}
