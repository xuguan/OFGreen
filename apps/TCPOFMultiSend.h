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

#ifndef TCPOFMULTISEND_H_
#define TCPOFMULTISEND_H_
#include <vector>
#include "INETDefs.h"
#include "TCPSessionApp.h"
#include "northboundMsg_m.h"

class TCPOFMultiSend: public TCPSessionApp {
public:
//    TCPOFMultiSend();
//    virtual ~TCPOFMultiSend();
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    void sendRequestNB();
    void handleNBmsgAndStartFlow(northboundMsg *msg);
    void sendDirectly();
    virtual void activity();
    virtual void finish();
    void stopAndStartNextFlow(cMessage* msg);
    void startSending(IPv4Address IPv4src, IPv4Address IPv4dst);
    IPvXAddress localAddr;
    IPvXAddress connetAddr;
    simtime_t tOpen; //tOpen does not
    simtime_t tClose;
    simtime_t tSend;
    simtime_t tDeadline;
    double sendTime;
    int localPort;
    int connectPort;
    long sendBytes;
    bool isTimeOut;
    int randomGenerator; //use to generate random number in the range 0...15int dice = 1 + intrand(randomGenerator,16);
    cOutVector flowCompletionTime;
    cMessage *timeoutMsg;
    northboundMsg* nbmsg;
    cModule * ctrlModle;
};

#endif /* TCPOFMULTISEND_H_ */
