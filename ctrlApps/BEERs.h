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

#ifndef BEERS_H_
#define BEERS_H_

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

#include "GraphElements.h"
#include "Graph.h"
#include "DijkstraShortestPathAlg.h"
#include "YenTopKShortestPathsAlg.h"

class IInterfaceTable;
class IRoutingTable;

class BEERs: public cSimpleModule, public cListener {
public:
    BEERs();
    virtual ~BEERs();
    void receiveSignal(cComponent *src, simsignal_t id, cObject *obj);
    MACAddress getMACfromIPv4(IPv4Address IPv4src);
    bool findAndReserveRouting(IPv4Address IPv4src, IPv4Address IPv4dst,
            bool isweight = 0);
    bool startFlow(northboundMsg *nbmsg);
    bool installVersePath(oxm_basic_match * match, BasePath *path);
    bool installPath(oxm_basic_match * match, BasePath *path);
    bool removeVersePath(oxm_basic_match * match, BasePath *path);
    bool removePath(oxm_basic_match * match, BasePath *path);
    bool installPath(northboundMsg *nbmsg, BasePath *path);
    bool removePath(northboundMsg *nbmsg, BasePath *path);

    bool findBestFlowSet(IPv4Address IPv4src, IPv4Address IPv4dst,
            bool isweight);
    int flowScheduleMinSwitches(northboundMsg *nbmsg, bool isweight);
    int flowScheduleMinSwitches(IPv4Address IPv4src, IPv4Address IPv4dst,
            northboundMsg *nbmsg, bool isweight = 0);
    bool addNewFlow(IPv4Address IPv4src, IPv4Address IPv4dst,
            northboundMsg *nbmsg);
    bool flowScheduleHeuristic();
    bool findTwoPathAndReserveOneRouting(IPv4Address IPv4src,
            IPv4Address IPv4dst, bool isweight = 0);
    bool findAndReserveRoutingFromsrcTodst(IPv4Address IPv4src,
            IPv4Address IPv4dst, bool isweight = 0, bool enableWeightChange = 1,
            bool enableReserve = 1);
    bool hasBidirectionpaths(IPv4Address IPv4src, IPv4Address IPv4dst,
            bool isweight = 0);
    bool updateFlowRate();
    void finish();
    void handleMessage(cMessage *msg);
protected:
    virtual void initialize(int stage);
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

    virtual void extractTopology(Topology& topo, NodeInfoVector& nodeInfo);
    virtual void assignAddresses(Topology& topo, NodeInfoVector& nodeInfo);
    virtual bool processARPPacket(OFP_Packet_In *packet_in, int connID);
    NodeInfoVector nodeInfo;

    virtual int numInitStages() const {
        return 5;
    }

    simsignal_t connIDSignal;
    simsignal_t PacketInSignalId;
    typedef std::vector<Topology::LinkOut*> LinkPath;
    typedef std::vector<LinkPath*> LinkPaths;

    typedef std::vector<northboundMsg *> NBREQs;
    NBREQs * nbreqs;
    NBREQs * nbreqqueue;
    LinkPaths *linkpaths;

    Topology topo_forw;
    int getNodeId(Topology::Node* queryNode) {
        int NumNodes = topo_forw.getNumNodes();
        for (int i = 0; i < NumNodes; i++) {
            if (topo_forw.getNode(i) == queryNode)
                return i;
        }
//        EV << "ERROR: node not in topo_forw" << endl;
        return 0;
    }

    int getIDbyMAC(MACAddress macAddress);
    int getIDbyIPv4(IPv4Address ipAddress);
    bool isArpOracle;

    int max_multi_path_num;
    //default paths
    typedef std::vector<BasePath *> BasePaths;
    typedef int SrcID;
    typedef int DestID;
    typedef std::map<DestID, BasePaths*> BasePathsPerSrc;
    typedef std::map<SrcID, BasePathsPerSrc*> BasePathsAllPair;
    //all pair paths on default constrained with upbound
    BasePathsAllPair * basePathAllPair;
    BasePaths* getPathsByIPv4(IPv4Address ipAddresssrc,
            IPv4Address ipAddressdst);

    //link density
    typedef int OVS_ID_t;
    typedef int Flow_Num_t;
    typedef std::map<OVS_ID_t, Flow_Num_t> FlowNumPerPort_t;
    typedef std::map<OVS_ID_t, FlowNumPerPort_t*> FlowNumPerNode_t;
    FlowNumPerNode_t *linkDensity;
    int getlinkDensity(OVS_ID_t fromID, OVS_ID_t toID) {
        return linkDensity->find(fromID)->second->find(toID)->second;
    }

    //link bandwidth
    typedef std::map<OVS_ID_t, double> LinkBW_available_t;
    typedef std::map<OVS_ID_t, LinkBW_available_t*> LinkBWs_t;

    //flows
    typedef int FlowID_t;
    typedef std::map<FlowID_t, northboundMsg*> AllFlows_t;
    AllFlows_t * allFlows;

    //flows ordered by avaiable bandwidth in MMF, increasing order
    std::vector<FlowID_t> *flowsOrderedByWeight;
    std::vector<double> *flowsWeights;
    //flow rate updated once new flow arrives
    std::vector<double> *flowsRateAverage; //in MBps
    //true for active flows, false for queued
    std::vector<int> * isactiveflow;
    //path of each flow
    std::vector<BasePath*> *flowOrderedByWeightPaths;
    BasePaths* bestPaths;
    bool canStartFlowIndex(int priFlow, int curFlow);
    bool canStartFlowIndex(int curFlow);
    typedef std::map<FlowID_t, BasePath*> FlowPaths;
    //each active flow's path
    FlowPaths *activeflowpaths;

    struct FlowInfo {
        FlowInfo(northboundMsg * msg) {
            nbMsg = msg;
            arrivalTime = simTime();
            srcIP = msg->getMatch().OFB_IPV4_SRC;
            dstIP = msg->getMatch().OFB_IPV4_DST;
            srcPort = msg->getMatch().OFB_TP_SRC;
            dstPort = msg->getMatch().OFB_TP_DST;
            alterPaths = NULL;
            bestPath = NULL;
            flowRate = 0;
            flowWeight = 0;
            isActiveFlow = 0;
            flowIDVectorPtr = NULL;
            flowID = 0;
        }
        int flowID;
        IPv4Address srcIP;
        IPv4Address dstIP;
        int srcPort;
        int dstPort;
        simtime_t arrivalTime;
        northboundMsg * nbMsg;
        BasePath * bestPath;
        BasePaths * alterPaths;
        double flowRate;
        double flowWeight;
        int isActiveFlow;
        std::vector<FlowID_t> *flowIDVectorPtr;
    };
    std::map<int, FlowInfo*> flowOrdered;

    bool finishFlow(FlowID_t flowID);
    bool schedulFlows();
    bool exclusiveRouting();
//    bool findMinPathset();
    int findMinPathset(int FlowID_index, BasePaths *minPaths,
            LinkBWs_t *linkBWs);
    int getPathNum(BasePaths* flowpaths);
    void printPaths(BasePaths* flowpaths);
    void printBestPaths();
    void printPath(BasePath* flowpath);
    BasePaths* getPathsByFlowID(FlowID_t flowID);
//    double getFlowRateByFlowID(FlowID flowID);
    bool removePathFromLinkBWs(BasePath *curpath, LinkBWs_t* linkBWs,
            double flowrate);
    bool releasePathFromLinkBWs(BasePath *curpath, LinkBWs_t* linkBWs,
            double flowrate);

    //graph
    Graph* my_graph_pt;
    int flow_ID;
    double linkCapacity;
    simtime_t lastArrivalTime;
private:
    OFA_controller *controller;
    typedef std::vector<std::pair<int, uint32_t> > UnsortedMap;
    UnsortedMap connID_outport;
};

#endif /* BEERS_H_ */
