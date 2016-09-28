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

#include "TCPOFMultiSend.h"
#include "northboundWrapper.h"
#include "IPvXAddressResolver.h"
#include "TCPSocket.h"
#include "RoutingTable.h"
#include "northboundMsg_m.h"
#include "TCP.h"
#include "TCPConnection.h"
#include "GenericAppMsg_m.h"
#include "TCP_Session_Message_m.h"
#define NBmsg_t northboundMsg

Define_Module(TCPOFMultiSend);
//TCPOFMultiSend::TCPOFMultiSend() {
//    // TODO Auto-generated constructor stub
//
//}
//
//TCPOFMultiSend::~TCPOFMultiSend() {
//    // TODO Auto-generated destructor stub
//}

void TCPOFMultiSend::initialize(int stage) {
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

    isTimeOut = 0;
    localPort = par("localPort");
    connectPort = par("connectPort");
    int flowScheduleType = par("flowScheduleType");
    if (flowScheduleType == 1) {
        cMessage *nextEvent = new cMessage("sendRequest");
        nextEvent->addPar("sendRequest");
        simtime_t nextTime = (simtime_t) par("arrivalTime");
        scheduleAt(nextTime, nextEvent);

        ctrlModle = this->getParentModule()->getParentModule()->getSubmodule(
                "controller")->getSubmodule("ctrlApp");
    } else if (flowScheduleType == 2) {

        cMessage *nextEvent = new cMessage("sendDirect");
        nextEvent->addPar("sendDirect");
        simtime_t nextTime = (simtime_t) par("arrivalTime");
        scheduleAt(nextTime, nextEvent);
        ctrlModle = NULL;
    }

}
void TCPOFMultiSend::sendRequestNB() {
    const char *localAddress = this->getParentModule()->getFullPath().c_str();

    cPar& connectAddresscPar = par("connectAddress");
    const char *connectAddress = connectAddresscPar.stdstringValue().c_str();
    while (!strcmp(connectAddress, localAddress)) {
        connectAddress = par("connectAddress").stdstringValue().c_str();
    }

    localAddr = IPvXAddressResolver().resolve(localAddress);
    connetAddr = IPvXAddressResolver().resolve(connectAddress);
    EV << this->getParentModule()->getFullPath() << "(" << localAddr
            << ")  to  " << connectAddress << "(" << connetAddr << ")" << endl;

    tOpen = par("tOpen"); //tOpen does not
    tClose = par("tClose");
    tDeadline = par("tDeadline");
    sendBytes = par("sendBytes");

    NBmsg_t * nbReqMsg = new NBmsg_t();
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
    nbReqMsg->setMatch(*match);
    nbReqMsg->setSendBytes(sendBytes);
    nbReqMsg->setTdeadline(simTime() + tDeadline);
    nbReqMsg->setTOpen(tOpen);
    nbReqMsg->setTClose(tClose);
    nbReqMsg->setCanSend(0);
    nbReqMsg->setCanEnd(0);
    cModule * clientModule = this;
    nbReqMsg->setClientModuleptr((int) clientModule);
    sendDirect(nbReqMsg, ctrlModle->gate("crtlIn"));
}

//refresh time
void TCPOFMultiSend::sendDirectly() {
    const char *localAddress = this->getParentModule()->getFullPath().c_str();

    cPar& connectAddresscPar = par("connectAddress");
    const char *connectAddress = connectAddresscPar.stdstringValue().c_str();
    while (!strcmp(connectAddress, localAddress)) {
        connectAddress = par("connectAddress").stdstringValue().c_str();
    }

    localAddr = IPvXAddressResolver().resolve(localAddress);
    connetAddr = IPvXAddressResolver().resolve(connectAddress);
    EV << this->getParentModule()->getFullPath() << "(" << localAddr
            << ")  to  " << connectAddress << "(" << connetAddr << ")" << endl;

    tOpen = par("tOpen"); //tOpen does not
    tClose = par("tClose");
    tDeadline = par("tDeadline");
    sendBytes = par("sendBytes");

    bool active = par("active");
    if (sendBytes > 0 && commands.size() > 0)
        throw cRuntimeError("Cannot use both sendScript and tSend+sendBytes");

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
        finishMsg->addPar("finishingTag") = "finishingTag";
        finishMsg->setByteLength(222);
        emit(sentPkSignal, finishMsg);
        socket.send(finishMsg);
    }

    if (ev.isGUI())
        getDisplayString().setTagArg("t", 0, "closing");

    if (timeoutMsg->isScheduled())
        cancelAndDelete(timeoutMsg);
    timeoutMsg = new cMessage("timeout");
    scheduleAt(tClose, timeoutMsg);
    cMessage *msg = NULL;

}
void TCPOFMultiSend::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        if (msg == timeoutMsg) {
            EV << "Ops, transfer failed!" << endl;
        }
        if (msg->getParList().exist("sendRequest")) {
            sendRequestNB();
            delete msg;
        }
        if (msg->getParList().exist("sendDirect")) {
            sendDirectly();
            delete msg;
        }

    } else if (dynamic_cast<NBmsg_t *>(msg)) {
        nbmsg = check_and_cast<NBmsg_t *>(msg);
        handleNBmsgAndStartFlow(nbmsg);
    } else if (dynamic_cast<cPacket*>(msg)) {
        if (msg->getKind() == TCP_I_DATA
                || msg->getKind() == TCP_I_URGENT_DATA) {
            cPacket *pkt = check_and_cast<cPacket *>(msg);
            int exist = pkt->getParList().exist("finishingTagRes");
            if (exist) {
                EV << "Sending finished" << endl;
                flowCompletionTime.record(simTime() - tSend);
                //start new flows
                stopAndStartNextFlow(msg);
            }
        } else {
            count(msg);
            socket.processMessage(msg);
        }
    }
}

void TCPOFMultiSend::handleNBmsgAndStartFlow(NBmsg_t *nbMsg) {

    oxm_basic_match match = nbMsg->getMatch();
    IPv4Address IPv4src = match.OFB_IPV4_SRC;
    IPv4Address IPv4dst = match.OFB_IPV4_DST;
    EV << "Receiving nb response of " << IPv4dst << "  to  " << IPv4src << "\t";
    if (!nbMsg->getCanSend())
        return;
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
        bytesSent += onceBytes;
        if (ev.isGUI()) {
            char buf[32];
            sprintf(buf, "sent: %ld bytes", bytesSent);
            getDisplayString().setTagArg("t", 0, buf);
        }
        emit(sentPkSignal, msg);
        socket.send(msg);

        cPacket *finishMsg = new cPacket("finishingPkt");
        finishMsg->addPar("finishingTag") = "finishingTag";
        finishMsg->setByteLength(111);
        emit(sentPkSignal, finishMsg);
        socket.send(finishMsg);

        if (timeoutMsg->isScheduled())
            cancelAndDelete(timeoutMsg);
        timeoutMsg = new cMessage("timeout");
        scheduleAt(tClose, timeoutMsg);
    }
    EV << "Cannot send" << endl;
}

void TCPOFMultiSend::stopAndStartNextFlow(cMessage* msg) {

    socket.close();

    if (timeoutMsg->isScheduled())
        cancelAndDelete(timeoutMsg);
    else
        delete timeoutMsg;

    int flowScheduleType = par("flowScheduleType");
    if (flowScheduleType == 1) {
        nbmsg->setCanEnd(1);
        sendDirect(nbmsg, ctrlModle->gate("crtlIn"));

        cMessage *nextEvent = new cMessage("sendRequest");
        nextEvent->addPar("sendRequest");
        scheduleAt((simtime_t) par("arrivalTime"), nextEvent);
    } else if (flowScheduleType == 2) {

        cMessage *nextEvent = new cMessage("sendDirect");
        nextEvent->addPar("sendDirect");
        scheduleAt((simtime_t) par("arrivalTime"), nextEvent);
        ctrlModle = NULL;
    }

}
void TCPOFMultiSend::finish() {
    return;
}
void TCPOFMultiSend::activity() {
    return;
}
//void TCPOFMultiSend::activity() {
//
//    cPar connectAddresscPar = par("connectAddress");
//    const char *connectAddress = connectAddresscPar.stdstringValue().c_str();
////    const char *connectAddress = par("connectAddress").stdstringValue().c_str();
//
//    const char *localAddress = this->getParentModule()->getFullPath().c_str();
//    while (!strcmp(connectAddress, localAddress)) {
//        connectAddress = par("connectAddress").stdstringValue().c_str();
//    }
//    localAddr = IPvXAddressResolver().resolve(localAddress);
//    connetAddr = IPvXAddressResolver().resolve(connectAddress);
//    simtime_t tOpen = par("tOpen"); //tOpen does not
//    simtime_t tSend = par("tSend");
//    simtime_t tClose = par("tClose");
//    simtime_t tDeadline = par("tDeadline");
//    sendBytes = par("sendBytes");
//    waitUntil(tSend);
//    int flowScheduleType = par("flowScheduleType");
//    if (flowScheduleType == 1) {
//        EV << this->getParentModule()->getFullPath() << "(" << localAddr
//                << ")  to  " << connectAddress << "(" << connetAddr << ")"
//                << endl;
//        NBmsg_t * nbReqMsg = new NBmsg_t();
//        oxm_basic_match *match = new oxm_basic_match();
//        match->OFB_IPV4_DST = connetAddr.get4();
//        match->OFB_IPV4_SRC = localAddr.get4();
//        match->OFB_ETH_TYPE = 0x0800; //ETHERTYPE_IPv4
//        match->OFB_TP_DST = connectPort;
//        match->OFB_TP_SRC = localPort;
//        match->wildcards = OFPFW_IN_PORT + OFPFW_DL_SRC + OFPFW_DL_DST
//                + OFPFW_TP_DST + OFPFW_TP_SRC;
//        EV << "Sending nb request " << localAddr.get4() << "  to  "
//                << connetAddr.get4() << endl;
//        nbReqMsg->setMatch(*match);
//        nbReqMsg->setSendBytes(sendBytes);
//        nbReqMsg->setTdeadline(simTime() + tDeadline);
//        nbReqMsg->setTOpen(tOpen);
//        nbReqMsg->setTClose(tClose);
//        nbReqMsg->setCanSend(0);
//        nbReqMsg->setCanEnd(0);
//        cModule * clientModule = this;
//        nbReqMsg->setClientModuleptr((int) clientModule);
//        sendDirect(nbReqMsg, ctrlModle->gate("crtlIn"));
//
//        cMessage *timeoutMsg = new cMessage("timeout");
//        scheduleAt(tClose, timeoutMsg);
//        cModule * TCPModle = this->getParentModule()->getSubmodule("tcp");
//
//        cMessage *msg = NULL;
//        while ((msg = receive()) != timeoutMsg) {
//            NBmsg_t * nbMsg = check_and_cast<NBmsg_t *>(msg);
//            if (nbMsg == 0)
//                continue;
//            oxm_basic_match match = nbMsg->getMatch();
//            IPv4Address IPv4src = match.OFB_IPV4_SRC;
//            IPv4Address IPv4dst = match.OFB_IPV4_DST;
//            EV << "Receiving nb response of " << IPv4dst << "  to  " << IPv4src
//                    << "\t";
//            if (nbMsg->getCanSend()) {
////                startSending(IPv4src, IPv4dst);
//                EV << "Can send" << endl;
//                bool active = par("active");
//                socket.setOutputGate(gate("tcpOut"));
//                socket.readDataTransferModePar(*this);
//                socket.bind(localAddr, localPort);
//                EV << "issuing OPEN command\n";
//
//                if (ev.isGUI())
//                    getDisplayString().setTagArg("t", 0,
//                            active ? "connecting" : "listening");
//
//                if (active)
//                    socket.connect(connetAddr, connectPort);
//                else
//                    socket.listenOnce();
//
//                // wait until connection gets established
//                while (socket.getState() != TCPSocket::CONNECTED) {
//                    socket.processMessage(receive());
//                    if (socket.getState() == TCPSocket::SOCKERROR)
//                        return;
//                }
//
//                EV << "connection established, starting sending\n";
//                if (ev.isGUI())
//                    getDisplayString().setTagArg("t", 0, "connected");
//
//                if (sendBytes > 0) {
//                    long onceBytes = sendBytes * 1000000; //B
////                    waitUntil(tSend);
//                    EV << "sending " << onceBytes << " bytes\n";
//                    cPacket *msg = genDataMsg(onceBytes);
//                    bytesSent += onceBytes;
//                    if (ev.isGUI()) {
//                        char buf[32];
//                        sprintf(buf, "sent: %ld bytes", bytesSent);
//                        getDisplayString().setTagArg("t", 0, buf);
//                    }
//                    emit(sentPkSignal, msg);
//                    socket.send(msg);
//
//                    cPacket *finishMsg = new cPacket("finishingPkt");
//                    finishMsg->addPar("finishingTag") = "finishingTag";
//                    finishMsg->setByteLength(111);
//                    emit(sentPkSignal, finishMsg);
//                    socket.send(finishMsg);
//
//                    cMessage *timeoutMsg = new cMessage("timeout");
//                    scheduleAt(tClose, timeoutMsg);
//                    cMessage *tmpmsg = NULL;
//
//                    while (tmpmsg = receive()) {
//                        if (tmpmsg != timeoutMsg) {
//                            if (tmpmsg->getKind() == TCP_I_DATA
//                                    || tmpmsg->getKind() == TCP_I_URGENT_DATA) {
//                                cPacket *pkt = check_and_cast<cPacket *>(
//                                        tmpmsg);
//                                int exist = pkt->getParList().exist(
//                                        "finishingTagRes");
//                                if (exist) {
//                                    EV << "Sending finished" << endl;
//                                    flowCompletionTime.record(
//                                            simTime() - tSend);
//                                    break;
//                                }
//                            }
//                            count(tmpmsg);
//                            socket.processMessage(tmpmsg);
//                        }
//                    }
//                    if (timeoutMsg->isScheduled())
//                        cancelAndDelete(timeoutMsg);
//                    else
//                        delete timeoutMsg;
//
//                    nbMsg->setCanEnd(1);
//                    sendDirect(nbMsg, ctrlModle->gate("crtlIn"));
//                    socket.close();
//                }
//                for (;;) {
//                    cMessage *msg = receive();
//                    count(msg);
//                    socket.processMessage(msg);
//                }
//            }
//            EV << "Canot send" << endl;
//        }
//
//        if (simTime() >= tClose)
//            isTimeOut = 1;
//        delete timeoutMsg;
//
//    } else if (flowScheduleType == 2) {
//        bool active = par("active");
//        const char *script = par("sendScript");
//        parseScript(script);
//
//        if (sendBytes > 0 && commands.size() > 0)
//            throw cRuntimeError(
//                    "Cannot use both sendScript and tSend+sendBytes");
//
//        socket.setOutputGate(gate("tcpOut"));
//
//        // open
//        waitUntil(tOpen);
//
//        socket.readDataTransferModePar(*this);
//        socket.bind(localAddr, localPort);
//
//        EV << "issuing OPEN command\n";
//
//        if (ev.isGUI())
//            getDisplayString().setTagArg("t", 0,
//                    active ? "connecting" : "listening");
//
//        if (active)
//            socket.connect(connetAddr, connectPort);
//        else
//            socket.listenOnce();
//
//        // wait until connection gets established
//        while (socket.getState() != TCPSocket::CONNECTED) {
//            socket.processMessage(receive());
//
//            if (socket.getState() == TCPSocket::SOCKERROR)
//                return;
//        }
//
//        EV << "connection established, starting sending\n";
//        if (ev.isGUI())
//            getDisplayString().setTagArg("t", 0, "connected");
//
//        if (sendBytes > 0) {
//            long onceBytes = sendBytes * 1000000; //B
//            waitUntil(tSend);
//            EV << "sending " << onceBytes << " bytes\n";
//            cPacket *msg = genDataMsg(onceBytes);
//            bytesSent += onceBytes;
//            emit(sentPkSignal, msg);
//            socket.send(msg);
//
//            cPacket *finishMsg = new cPacket("finishingPkt");
//            finishMsg->addPar("finishingTag") = "finishingTag";
//            finishMsg->setByteLength(222);
//            emit(sentPkSignal, finishMsg);
//            socket.send(finishMsg);
//        }
//
////        for (CommandVector::iterator i = commands.begin(); i != commands.end();
////                ++i) {
////            waitUntil(i->tSend);
////            EV << "sending " << i->numBytes << " bytes\n";
////            cPacket *msg = genDataMsg(i->numBytes);
////            bytesSent += i->numBytes;
////            emit(sentPkSignal, msg);
////            socket.send(msg);
////        }
//
//        //TODO: this may start when a new session is started.
//        //TODO: ideally, this start when the process finished.
////        // close
////        if (tClose >= 0) {
////            waitUntil(tClose);
////            EV << "issuing CLOSE command\n";
////
////            if (ev.isGUI())
////                getDisplayString().setTagArg("t", 0, "closing");
////
////            socket.close();
////        }
////
////        // wait until peer closes too and all data arrive
////        for (;;) {
////            cMessage *msg = receive();
////            count(msg);
////            socket.processMessage(msg);
////        }
//        //                waitUntil(tClose);
//        EV << "issuing CLOSE command\n";
//
//        if (ev.isGUI())
//            getDisplayString().setTagArg("t", 0, "closing");
//
//        // close
////        if (tClose >= 0) {
////            waitUntil(tClose);
////            EV << "issuing CLOSE command\n";
////
////            if (ev.isGUI())
////                getDisplayString().setTagArg("t", 0, "closing");
////
////            socket.close();
////        }
//
//        cMessage *timeoutMsg = new cMessage("timeout");
//        scheduleAt(tClose, timeoutMsg);
//        cMessage *msg = NULL;
//
//        while (msg = receive()) {
//            if (msg != timeoutMsg) {
//                if (msg->getKind() == TCP_I_DATA
//                        || msg->getKind() == TCP_I_URGENT_DATA) {
//                    cPacket *pkt = check_and_cast<cPacket *>(msg);
//                    int exist = pkt->getParList().exist("finishingTagRes");
//                    if (exist) {
//                        EV << "Sending finished" << endl;
//                        flowCompletionTime.record(simTime() - tSend);
//                        break;
//                    }
//                    delete msg;
//                } else {
//                    count(msg);
//                    socket.processMessage(msg);
//                }
//            }
//        }
//        if (timeoutMsg->isScheduled())
//            cancelAndDelete(timeoutMsg);
//        else
//            delete timeoutMsg;
//        socket.close();
//        for (;;) {
//            cMessage *msg = receive();
//            count(msg);
//            socket.processMessage(msg);
//        }
//
//    }
//}
