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

package openflow.ctrlApps;

simple BEERs like CtrlBehavior
{
    parameters:
        @display("i=block/rightarrow");
        bool isArpOracle = default(false);
        bool isBiPathReserved = default(false);
        int max_multi_path_num = default(2);//max path number of a flow.
        int domainID = default(-1);
        double linkcapacity= default(1000);//link data rate 1000Mbps
        double nodeweight = default(1.0);//1.0 initialized, 1/2 if there is 1 flow on it.
        bool isWeightedNode = default(false);//true means node weight would care
        string nedTypes = default("inet.nodes.inet.StandardHost openflow.nodes.Open_Flow_Switch");
    gates:
        input crtlIn @directIn;

}
