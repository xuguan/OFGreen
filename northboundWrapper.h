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

#ifndef NORTHBOUNDWRAPPER_H_
#define NORTHBOUNDWRAPPER_H_
#include <omnetpp.h>
#include "openflow.h"
#include "IPvXAddress.h"

class northboundWrapper: public cObject {
public:
    northboundWrapper();
    ~northboundWrapper();
    IPvXAddress ip_src;
    IPvXAddress ip_dst;
    uint32_t port_src;
    uint32_t port_dst;
    long sendBytes;
    simtime_t tOpen;
    simtime_t tClose;
    uint8_t canSend;//default(0), modified by controller
    uint8_t canEnd;//default(0), modified by host
};

#endif /* NORTHBOUNDWRAPPER_H_ */
