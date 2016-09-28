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

#ifndef TCPSINKOFBASED_H_
#define TCPSINKOFBASED_H_

#include "TCPEchoApp.h"
#include "TCPSocket.h"
class TCPSinkOFbased: public TCPEchoApp, public TCPSocket::CallbackInterface {
public:
    TCPSinkOFbased();
    ~TCPSinkOFbased();
    void finish();
    void initialize();
    void handleMessage(cMessage *msg);
    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent);
    static double packet_length_overall; //in GB
    static double packet_length_overall_tmp;
    double lasttime;
    cOutVector packetLengthOverallVector;
    double sampleTime;
    long receiveBytes;
    long receivedBytes;
    cOutVector flowCompletionTime;
    double lasttimeFlowCompletion;
    int flowNum;
};

#endif /* TCPSINKOFBASED_H_ */
