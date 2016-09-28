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

#include "TCPOFMultiSendy.h"
#include "northboundWrapper.h"
#include "IPvXAddressResolver.h"
#include "TCPSocket.h"
#include "RoutingTable.h"
#include "northboundMsg_m.h"
#include "TCP.h"
#include "TCPConnection.h"
#include "GenericAppMsg_m.h"
#include "TCP_Session_Message_m.h"

Define_Module(TCPOFMultiSendy);
long TCPOFMultiSendy::packet_length_overall = 0;
TCPOFMultiSendy::TCPOFMultiSendy()
{
    // TODO Auto-generated constructor stub

}

TCPOFMultiSendy::~TCPOFMultiSendy()
{
    // TODO Auto-generated destructor stub
}

void TCPOFMultiSendy::initialize(int stage)
{
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
    flowRateVector.setName("flow Rate Vector");
    flowNum = par("flowNum");

}

void TCPOFMultiSendy::activity()
{
    cPar& arrivalTime = par("arrivalTime");
    cPar& deadline = par("tDeadline");
    cPar& flowsizeByte = par("sendBytes");
    simtime_t tClose = par("tClose");
    isTimeOut = 0;

    localPort = par("localPort");
    const char *localAddress = this->getParentModule()->getFullPath().c_str();

    localAddr = IPvXAddressResolver().resolve(localAddress);
    double sendTime = 0;
    double sleepRatio = par("sleepRatio");
    while (flowNum-- && !isTimeOut)
    {
        simtime_t arriveTime = (simtime_t) arrivalTime;
        if (sendTime > 0)
            arriveTime = sendTime;
        simtime_t tDeadline = (simtime_t) deadline;
//        simtime_t tOpen = par("tOpen");
//        simtime_t tSend = par("tSend");
        simtime_t countstarttime = 0;
        simtime_t tSend = simTime() + arriveTime;
        simtime_t tOpen = tSend;

        waitUntil(tSend);
        simtime_t countwaitTime = simTime();
        EV << "arrive time: " << tSend << endl;
        const char *connectAddress = par("connectAddress").stdstringValue().c_str();
        connectPort = par("connectPort");
        while (!strcmp(connectAddress, localAddress))
        {
            connectAddress = par("connectAddress").stdstringValue().c_str();
        }
        connetAddr = IPvXAddressResolver().resolve(connectAddress);
        connetAddrs.push_back(connetAddr);
        sendBytes = (int) flowsizeByte;
        int sumint = par("sendByteSum");
        if (packet_length_overall == sumint)
            return;
        packet_length_overall += sendBytes;
        if (packet_length_overall > sumint)
        {
            sendBytes = packet_length_overall - sumint;
            packet_length_overall = sumint;
        }
        int flowScheduleType = par("flowScheduleType");
        if (flowScheduleType == 1)
        {
            EV << this->getParentModule()->getFullPath() << "(" << localAddr << ")  to  " << connectAddress << "(" << connetAddr << ")" << endl;
            cModule * ctrlModle = this->getParentModule()->getParentModule()->getSubmodule("controller")->getSubmodule("ctrlApp");
            northboundMsg * northboundmsg = new northboundMsg();
            oxm_basic_match *match = new oxm_basic_match();
            match->OFB_IPV4_DST = connetAddr.get4();
            match->OFB_IPV4_SRC = localAddr.get4();
            match->OFB_ETH_TYPE = 0x0800; //ETHERTYPE_IPv4
            match->OFB_TP_DST = connectPort;
            match->OFB_TP_SRC = localPort;
            match->wildcards = OFPFW_IN_PORT + OFPFW_DL_SRC + OFPFW_DL_DST + OFPFW_TP_DST + OFPFW_TP_SRC;
            EV << "Sending nb request " << localAddr.get4() << "  to  " << connetAddr.get4() << endl;
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
            cMessage *msg = NULL;
            while ((msg = receive()) != timeoutMsg)
            {
                if (!(dynamic_cast<northboundMsg *>(msg)))
                    continue;
                countstarttime = simTime();
                tSend = countstarttime;
                countwaitTime = countstarttime - countwaitTime;
                northboundMsg * northboundmsg = check_and_cast<northboundMsg *>(msg);
                cancelAndDelete(timeoutMsg);
                oxm_basic_match match = northboundmsg->getMatch();
                IPv4Address IPv4src = match.OFB_IPV4_SRC;
                IPv4Address IPv4dst = match.OFB_IPV4_DST;
                EV << "Receiving nb response of " << IPv4src << "  to  " << IPv4dst << "\t";
                if (northboundmsg->getCanSend() != 1)
                {
                    EV << "Canot send" << endl;
                    break;
                }
//                startSending(IPv4src, IPv4dst);
                EV << "Can send" << endl;
                bool active = par("active");
                if (socket.getState() != TCPSocket::NOT_BOUND)
                    socket.renewSocket();
                socket.setOutputGate(gate("tcpOut"));
                socket.readDataTransferModePar(*this);
                socket.bind(localAddr, localPort + 100 + flowNum);
                EV << "issuing OPEN command\n";

                if (ev.isGUI())
                    getDisplayString().setTagArg("t", 0, active ? "connecting" : "listening");

                if (active)
                    socket.connect(connetAddr, connectPort);
                else
                    socket.listenOnce();

                // wait until connection gets established
                while (socket.getState() != TCPSocket::CONNECTED)
                {
                    socket.processMessage(receive());
                    if (socket.getState() == TCPSocket::SOCKERROR)
                    {
                        EV << "socket error!" << endl;
                        return;
                    }
                }

                EV << "connection established, starting sending\n";
                if (ev.isGUI())
                    getDisplayString().setTagArg("t", 0, "connected");

                if (sendBytes <= 0)
                    break;
                for (long il = 0; il < sendBytes; il++)
                {
                    long sendBytesonce = 1000000;
                    cPacket *msg = genDataMsg(sendBytesonce);
                    bytesSent += sendBytesonce;
                    if (ev.isGUI())
                    {
                        char buf[32];
                        sprintf(buf, "sent: %ld bytes", bytesSent);
                        getDisplayString().setTagArg("t", 0, buf);
                    }
                    emit(sentPkSignal, msg);
                    socket.send(msg);
                }
//                long onceBytes = sendBytes * 1000000; //B
//                EV << "sending " << onceBytes << " bytes\n";
//                cPacket *msg = genDataMsg(onceBytes);
////                msg->setKind(3);
//                bytesSent += onceBytes;
//                if (ev.isGUI()) {
//                    char buf[32];
//                    sprintf(buf, "sent: %ld bytes", bytesSent);
//                    getDisplayString().setTagArg("t", 0, buf);
//                }
//                emit(sentPkSignal, msg);
//                socket.send(msg);

                cPacket *finishMsg = new cPacket("finishingPkt");
                finishMsg->addPar("finishingTag") = "finishingTag";
                finishMsg->setByteLength(89);
                emit(sentPkSignal, finishMsg);
                socket.send(finishMsg);

                timeoutMsg = new cMessage("timeout");
                scheduleAt(tClose, timeoutMsg);

                cMessage *tmpmsg = NULL;
                while (tmpmsg = receive())
                {
                    if (tmpmsg != timeoutMsg)
                    {
                        if (tmpmsg->getKind() == TCP_I_DATA || tmpmsg->getKind() == TCP_I_URGENT_DATA)
                        {
                            cPacket *pkt = check_and_cast<cPacket *>(tmpmsg);
                            int exist = pkt->getParList().exist("finishingTagRes");
                            if (exist)
                            {
                                EV << "Sending finished" << endl;
                                simtime_t flowTime = simTime() - tSend;
                                flowCompletionTime.record(flowTime);
                                double Size = sendBytes;
                                Size = Size / flowTime.dbl();
                                flowRateVector.record(Size);
                                break;
                            }
                        }
                        count(tmpmsg);
                        socket.processMessage(tmpmsg);
                    }
                    else
                    {
                        isTimeOut = 1;
                        delete tmpmsg;
                        break;
                    }
                }
                if (timeoutMsg->isScheduled())
                    cancelAndDelete(timeoutMsg);

                EV << "close socket and send finish msg to controller!" << endl;
                socket.close();
                while (socket.getState() != TCPSocket::CLOSED)
                {
                    cMessage *endingmsg = receive();
                    count(endingmsg);
                    EV << "Socket state: " << socket.getState() << endl;
                    socket.processMessage(endingmsg);
                }
                EV << "socket closed!" << endl;
                northboundmsg->setCanEnd(1);
                sendDirect(northboundmsg, ctrlModle->gate("crtlIn"));
                break;
            }

        }
        else if (flowScheduleType == 2)
        {
            if (sendBytes <= 0)
            {
                EV << "flow size setting error" << endl;
                break;
            }
            countstarttime = simTime();
            countwaitTime = 0;
            bool active = par("active");
            if (socket.getState() != TCPSocket::NOT_BOUND)
                socket.renewSocket();
            socket.setOutputGate(gate("tcpOut"));
            socket.readDataTransferModePar(*this);
            socket.bind(localAddr, localPort + 100 + flowNum);

            EV << "issuing OPEN command\n";

            if (ev.isGUI())
                getDisplayString().setTagArg("t", 0, active ? "connecting" : "listening");

            if (active)
                socket.connect(connetAddr, connectPort);
            else
                socket.listenOnce();

            // wait until connection gets established
            while (socket.getState() != TCPSocket::CONNECTED)
            {
                socket.processMessage(receive());
                if (socket.getState() == TCPSocket::SOCKERROR)
                    return;
            }

            EV << "connection established, starting sending\n";
            if (ev.isGUI())
                getDisplayString().setTagArg("t", 0, "connected");

            for (long il = 0; il < sendBytes; il++)
            {
                long sendBytesonce = 1000000;
                cPacket *msg = genDataMsg(sendBytesonce);
                bytesSent += sendBytesonce;
                if (ev.isGUI())
                {
                    char buf[32];
                    sprintf(buf, "sent: %ld bytes", bytesSent);
                    getDisplayString().setTagArg("t", 0, buf);
                }
                emit(sentPkSignal, msg);
                socket.send(msg);
            }
//            long onceBytes = sendBytes * 1000000; //B
//            EV << "sending " << onceBytes << " bytes\n";
//            cPacket *sendmsg = genDataMsg(onceBytes);
//            bytesSent += onceBytes;
//            emit(sentPkSignal, sendmsg);
//            socket.send(sendmsg);

            cPacket *finishMsg = new cPacket("finishingPkt");
            finishMsg->addPar("finishingTag") = "finishingTag";
            finishMsg->setByteLength(90);
            emit(sentPkSignal, finishMsg);
            socket.send(finishMsg);

            EV << "issuing CLOSE command\n";
            if (ev.isGUI())
                getDisplayString().setTagArg("t", 0, "closing");

            cMessage *timeoutMsg = new cMessage("timeout");
            scheduleAt(tClose, timeoutMsg);

            cMessage* msg = NULL;
            while (msg = receive())
            {
                if (msg != timeoutMsg)
                {
                    if (msg->getKind() == TCP_I_DATA || msg->getKind() == TCP_I_URGENT_DATA)
                    {
                        cPacket *pkt = check_and_cast<cPacket *>(msg);
                        int exist = pkt->getParList().exist("finishingTagRes");
                        if (exist)
                        {
                            EV << "Sending finished" << endl;
                            simtime_t flowTime = simTime() - tSend;
                            flowCompletionTime.record(flowTime);
                            double Size = sendBytes;
                            Size = Size / flowTime.dbl();
                            flowRateVector.record(Size);
                            break;
                        }
                        delete msg;
                    }
                    else
                    {
                        count(msg);
                        socket.processMessage(msg);
                    }
                }
                else
                {
                    isTimeOut = 1;
                    EV << "Fail to receive " << endl;
                    delete msg;
                    break;
                }
            }
            if (timeoutMsg->isScheduled())
                cancelAndDelete(timeoutMsg);

            EV << "close socket" << endl;
            socket.close();
//            for (;;) {
//                cMessage *msg = receive();
//                count(msg);
//                socket.processMessage(msg);
//            }
            while (socket.getState() != TCPSocket::CLOSED)
            {
                cMessage *endingmsg = receive();
                count(endingmsg);
                EV << "Socket state: " << socket.getState() << endl;
                socket.processMessage(endingmsg);
            }
            EV << "socket closed!" << endl;
        }

        simtime_t countEndTime = simTime();
        sendTime = (countEndTime - countstarttime - countwaitTime).dbl();
    }

}
void TCPOFMultiSendy::finish()
{
    if (bytesRcvd != 0)
    {
        EV << getFullPath() << ": received " << bytesRcvd << " bytes in " << packetsRcvd << " packets\n";
    }
    EV << getFullPath() << ": sending " << connetAddrs.size() << " flows " << endl;
    for (int i = 0; i < connetAddrs.size(); i++)
    {
        EV << localAddr << " to " << connetAddrs.at(i) << endl;
        connetAddrs.pop_back();
    }
    recordScalar("bytesRcvd", bytesRcvd);
    recordScalar("bytesSent", bytesSent);

}
