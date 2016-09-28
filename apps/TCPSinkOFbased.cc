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

#include "TCPSinkOFbased.h"
#include "TCP_Session_Message_m.h"
#include "TCPCommand_m.h"

Define_Module(TCPSinkOFbased);
double TCPSinkOFbased::packet_length_overall = 0;
double TCPSinkOFbased::packet_length_overall_tmp = 0;
void TCPSinkOFbased::initialize() {
    TCPEchoApp::initialize();
    packetLengthOverallVector.setName("Receiving G byte");
    flowCompletionTime.setName("flow Completion time per receiver");
    sampleTime = par("sampleTime");
    receiveBytes = par("receiveBytes");
    receivedBytes = 0;
    cMessage *timeevent = new cMessage("sampleTimeevent");
    scheduleAt(simTime() + sampleTime, timeevent);
    lasttime = 0;
    flowNum = 0;
}
TCPSinkOFbased::TCPSinkOFbased() {
    // TODO Auto-generated constructor stub

}

TCPSinkOFbased::~TCPSinkOFbased() {
    // TODO Auto-generated destructor stub
}

void TCPSinkOFbased::finish() {
    if (bytesRcvd != 0)
        EV << getFullPath() << ": received " << bytesRcvd << " " << flowNum
                << " flows(" << receiveBytes << " MB)" << endl;
}
void TCPSinkOFbased::socketDataArrived(int connId, void *yourPtr, cPacket *msg,
        bool urgent) {
    ev << "Received TCP data, " << msg->getByteLength() << " bytes\\n";
    delete msg;
}
void TCPSinkOFbased::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        if (packet_length_overall_tmp < packet_length_overall) {
            packetLengthOverallVector.record(
                    packet_length_overall / 1000000000); //unit(GB)
            packet_length_overall_tmp = packet_length_overall;
        }
        scheduleAt(simTime() + sampleTime, msg);
//        TCPEchoApp::sendDown(msg);
    } else {

        if (msg->getKind() == 1 || msg->getKind() == 2) {
            if (receivedBytes == 0) {
                lasttimeFlowCompletion = simTime().dbl();
            }

            cPacket *pk = check_and_cast<cPacket *>(msg);

            int exist = pk->getParList().exist("finishingTag");
            if (exist) {
                const char* finishTagStr = pk->par("finishingTag");
                if (!strcmp(finishTagStr, "finishingTag")) {
                    flowCompletionTime.record(
                            simTime().dbl() - lasttimeFlowCompletion);
                    flowNum++;
                    receivedBytes = 0;
                    pk->setKind(TCP_C_SEND);
                    TCPCommand *ind = check_and_cast<TCPCommand *>(
                            pk->removeControlInfo());
                    TCPSendCommand *cmd = new TCPSendCommand();
                    cmd->setConnId(ind->getConnId());
                    pk->setControlInfo(cmd);
                    delete ind;
                    pk->setByteLength(1);
                    pk->addPar("finishingTagRes")="finishingTagRes";
                    sendDown(pk);
                    return;
                }
                EV << "finishingTag" << endl;
            }
            long packetLength = pk->getByteLength();
            bytesRcvd += packetLength;

            if (ev.isGUI()) {
                char buf[32];
                sprintf(buf, "rcvd: %ld bytes", bytesRcvd);
                getDisplayString().setTagArg("t", 0, buf);
            }

            receivedBytes += packetLength;
            packet_length_overall += packetLength;
            if (lasttime != 0 && lasttime < simTime().dbl()) {
                EV << "Receiving " << packetLength << "B( "
                        << packet_length_overall / 1000000000
                        << " GB), flow rate"
                        << packetLength * 8 / (simTime().dbl() - lasttime)
                        << "bps" << endl;
            }
            lasttime = simTime().dbl();
            delete msg;
        } else {
            TCPEchoApp::handleMessage(msg);
        }
    }
}
