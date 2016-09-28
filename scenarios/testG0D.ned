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

package openflow.scenarios;

import inet.nodes.ethernet.EtherSwitch;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import inet.networklayer.autorouting.ipv4.FlatNetworkConfigurator;
import inet.util.ThruputMeteringChannel;

import openflow.nodes.*;


network testG0D
{
    @display("bgb=743,493");

    types:
        channel ethernetline extends ThruputMeteringChannel
        {
            delay = 0.0000001us;
            datarate = 1Gbps;
            thruputDisplayFormat = "u";
        }
    submodules:
        H1: StandardHost {
            @display("p=21,370;is=s");
        }
        configurator: FlatNetworkConfigurator {
            parameters:
                @display("p=44,37");
        }
        ovs: Open_Flow_Switch {
            @display("p=67,238");
        }
        controller: Open_Flow_Controller {
            @display("p=193,37");
        }

        H2: StandardHost {
            @display("p=59,370;is=s");
        }
        H3: StandardHost {
            @display("p=112,370;is=s");
        }
        ovs1: Open_Flow_Switch {
            @display("p=67,303");
        }
        ovs2: Open_Flow_Switch {
            @display("p=128,303");
        }
        ovs3: Open_Flow_Switch {
            @display("p=128,238");
        }
        H4: StandardHost {
            @display("p=162,370;is=s");
        }
        ovs16: Open_Flow_Switch {
            @display("p=193,140");
        }
    connections allowunconnected:

        //        client1.ethg++ <--> {delay=1ms;} <--> ovs.ethg++;
        //        ovs.ethg++ <--> {delay=1ms;} <--> H1.ethg++;
        //        controller.ethg++ <--> {delay=1ms;} <--> ovs.gate_controller++;
        //        ovs.ethg++ <--> {delay=1ms;} <--> H2.ethg++;
        //        ovs.ethg++ <--> {delay=1ms;} <--> H3.ethg++;
        controller.ethg++ <--> ethernetline { @display("ls=white,,da"); } <--> ovs.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=white,,da"); } <--> ovs1.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=white,,da"); } <--> ovs2.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=white,,da"); } <--> ovs3.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=white,,da"); } <--> ovs16.gate_controller++;
        ovs.ethg++ <--> ethernetline <--> ovs1.ethg++;
        ovs3.ethg++ <--> ethernetline <--> ovs2.ethg++;
        ovs1.ethg++ <--> ethernetline <--> H1.ethg++;
        ovs1.ethg++ <--> ethernetline <--> H2.ethg++;
        ovs2.ethg++ <--> ethernetline <--> H3.ethg++;
        ovs2.ethg++ <--> ethernetline <--> H4.ethg++;

        ovs16.ethg++ <--> ethernetline <--> ovs.ethg++;
        ovs16.ethg++ <--> ethernetline <--> ovs3.ethg++;
}
