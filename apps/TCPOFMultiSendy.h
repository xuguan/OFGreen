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

#ifndef TCPOFMULTISENDY_H_
#define TCPOFMULTISENDY_H_
#include <vector>
#include "INETDefs.h"
#include "TCPSessionApp.h"
#include "northboundMsg_m.h"

class TCPOFMultiSendy: public TCPSessionApp {
public:
    TCPOFMultiSendy();
    virtual ~TCPOFMultiSendy();
    virtual void initialize(int stage);
    virtual void activity();
    virtual void finish();
    IPvXAddress localAddr;
    IPvXAddress connetAddr;
    std::vector<IPvXAddress > connetAddrs;
    double sendTime;
    int localPort;
    int connectPort;
    long sendBytes;
    bool isTimeOut;
    int randomGenerator; //use to generate random number in the range 0...15int dice = 1 + intrand(randomGenerator,16);
    cOutVector flowCompletionTime;
    cOutVector flowRateVector;
    int flowNum;
    static long packet_length_overall;
};

#endif /* TCPOFMULTISENDY_H_ */
