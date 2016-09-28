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

#ifndef FORWARDINGEXR_H_
#define FORWARDINGEXR_H_

#include "OFA_controller.h"
#include "MACAddress.h"
#include "INETDefs.h"
#include "IPv4Address.h"
#include <algorithm>
#include "IRoutingTable.h"
#include "IInterfaceTable.h"
#include "FlatNetworkConfigurator.h"
#include "InterfaceEntry.h"
#include "IPv4InterfaceData.h"
#include <vector>
#include "northboundWrapper.h"
#include "northboundMsg_m.h"
#include "Topology.h"

class IInterfaceTable;
class IRoutingTable;

class ForwardingEXR: public cSimpleModule, public cListener {
public:
    ForwardingEXR();
    virtual ~ForwardingEXR();
    void receiveSignal(cComponent *src, simsignal_t id, cObject *obj);
    MACAddress getMACfromIPv4(IPv4Address IPv4src);
    bool findAndReserveRouting(IPv4Address IPv4src, IPv4Address IPv4dst,
            bool isweight = 0);
    bool findTwoPathAndReserveOneRouting(IPv4Address IPv4src,
            IPv4Address IPv4dst, bool isweight = 0);
    bool findAndReserveRoutingFromsrcTodst(IPv4Address IPv4src,
            IPv4Address IPv4dst, bool isweight = 0, bool enableWeightChange = 1,bool enableReserve = 1);
    bool hasBidirectionpaths(IPv4Address IPv4src, IPv4Address IPv4dst,
            bool isweight = 0);
    void finish();
    void handleMessage(cMessage *msg);
protected:
    virtual void initialize(int stage);

protected:
    struct NodeInfo {
        NodeInfo() {
            isIPNode = false;
            ift = NULL;
            rt = NULL;
            usesDefaultRoute = false;
        }
        bool isIPNode;
        bool isOpenFlow;
        IInterfaceTable *ift;
        IRoutingTable *rt;
        IPv4Address address;
        MACAddress macAddress;
        bool usesDefaultRoute;
        int connID;
        std::string name;
        friend std::ostream& operator<<(std::ostream& os, NodeInfo ni) {
            os << "Name: " << ni.name << ", MAC: " << ni.macAddress << ", IP: "
                    << ni.address << ", connID: " << ni.connID << endl;
            return os;
        }
    };
    typedef std::vector<NodeInfo> NodeInfoVector;

//    virtual void extractTopology(cTopology& topo, NodeInfoVector& nodeInfo);
//    virtual void assignAddresses(cTopology& topo, NodeInfoVector& nodeInfo);
    virtual void extractTopology(Topology& topo, NodeInfoVector& nodeInfo);
    virtual void assignAddresses(Topology& topo, NodeInfoVector& nodeInfo);
    virtual bool processARPPacket(OFP_Packet_In *packet_in, int connID);

    virtual int numInitStages() const {
        return 5;
    }

    simsignal_t connIDSignal;
    simsignal_t PacketInSignalId;
//    simsignal_t BandwidthReqSignalId;
//    simsignal_t BandwidthResSignalId;
//    typedef std::vector<cTopology::LinkOut*> LinkPath;
    typedef std::vector<Topology::LinkOut*> LinkPath;
//    typedef struct {
//        IPv4Address ipv4src;
//        IPv4Address ipv4dst;
//    } ROUTINGrequest;
    typedef std::vector<LinkPath*> LinkPaths;
//    typedef std::vector<northboundWrapper*> ROUTINGrequests;
//    typedef std::map<routingrequest,linkPath> routingTable;
//    ROUTINGrequests *routingrequests;
//    ROUTINGrequests *routingrequestqueue;
    //TODO: ordered by deadline, so using set?
//    ROUTINGrequests *routingrequestqueuewrapper;
    typedef std::vector<northboundMsg *> NBREQs;
    NBREQs * nbreqs;
    NBREQs * nbreqqueue;
    LinkPaths *linkpaths;
//    cTopology topo_forw;
    Topology topo_forw;

    NodeInfoVector nodeInfo;
    bool isArpOracle;


private:
    OFA_controller *controller;
    typedef std::vector<std::pair<int, uint32_t> > UnsortedMap;
    UnsortedMap connID_outport;
//    void printROUTINGrequests(ROUTINGrequests * routingreq);
};

#endif /* FORWARDINGEXR_H_ */
