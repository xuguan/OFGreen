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

#ifndef TCPAPPOFBASED_H_
#define TCPAPPOFBASED_H_
#include <vector>

#include "INETDefs.h"

//#include "TCPSocket.h"
#include "TCPSessionApp.h"

//class TCPAppOFbased: public TCPSessionApp, public cListener {
class TCPAppOFbased: public TCPSessionApp, public TCPSocket::CallbackInterface {
public:
//    TCPAppOFbased();
//    ~TCPAppOFbased();
    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent);
    virtual void initialize(int stage);
    virtual void activity();
    virtual void finish();
//    virtual void handleMessage(cMessage *msg);
    void startSending(IPv4Address IPv4src, IPv4Address IPv4dst);
//    void receiveSignal(cComponent *src, simsignal_t id, cObject *obj);
//    simsignal_t BandwidthReqSignalId;
//    simsignal_t BandwidthResSignalId;
    IPvXAddress localAddr;
    IPvXAddress connetAddr;
    double sendTime;
    int localPort;
    int connectPort;
    long sendBytes;
    bool isTimeOut;
    int randomGenerator;//use to generate random number in the range 0...15int dice = 1 + intrand(randomGenerator,16);
    cOutVector flowCompletionTime;
};

#endif /* TCPAPPOFBASED_H_ */
