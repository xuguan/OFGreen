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


network DCN_FatTree4
{
    @display("bgb=743,493");

    types:
        channel ethernetline extends ThruputMeteringChannel
        {
            delay = 1us;
            datarate = 100Mbps;
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
            @display("p=162,37");
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
        ovs4: Open_Flow_Switch {
            @display("p=225,238");
        }
        ovs5: Open_Flow_Switch {
            @display("p=225,303");
        }
        ovs6: Open_Flow_Switch {
            @display("p=286,238");
        }
        ovs7: Open_Flow_Switch {
            @display("p=286,303");
        }
        ovs8: Open_Flow_Switch {
            @display("p=566,303");
        }
        ovs9: Open_Flow_Switch {
            @display("p=408,238");
        }
        ovs10: Open_Flow_Switch {
            @display("p=468,238");
        }
        ovs11: Open_Flow_Switch {
            @display("p=408,303");
        }
        ovs12: Open_Flow_Switch {
            @display("p=566,238");
        }
        ovs13: Open_Flow_Switch {
            @display("p=627,303");
        }
        ovs14: Open_Flow_Switch {
            @display("p=469,303");
        }
        ovs15: Open_Flow_Switch {
            @display("p=627,238");
        }
        ovs16: Open_Flow_Switch {
            @display("p=193,140");
        }
        ovs17: Open_Flow_Switch {
            @display("p=310,140");
        }
        ovs18: Open_Flow_Switch {
            @display("p=399,140");
        }
        ovs19: Open_Flow_Switch {
            @display("p=499,140");
        }
        H5: StandardHost {
            @display("p=240,370;is=s");
        }
        H6: StandardHost {
            @display("p=211,370;is=s");
        }
        H7: StandardHost {
            @display("p=278,370;is=s");
        }
        H8: StandardHost {
            @display("p=310,370;is=s");
        }
        H9: StandardHost {
            @display("p=391,370;is=s");
        }
        H10: StandardHost {
            @display("p=425,370;is=s");
        }
        H11: StandardHost {
            @display("p=468,370;is=s");
        }
        H12: StandardHost {
            @display("p=499,370;is=s");
        }
        H13: StandardHost {
            @display("p=547,370;is=s");
        }
        H14: StandardHost {
            @display("p=585,370;is=s");
        }
        H15: StandardHost {
            @display("p=666,370;is=s");
        }
        H16: StandardHost {
            @display("p=627,370;is=s");
        }
    connections allowunconnected:

        //        client1.ethg++ <--> {delay=1ms;} <--> ovs.ethg++;
        //        ovs.ethg++ <--> {delay=1ms;} <--> H1.ethg++;
        //        controller.ethg++ <--> {delay=1ms;} <--> ovs.gate_controller++;
        //        ovs.ethg++ <--> {delay=1ms;} <--> H2.ethg++;
        //        ovs.ethg++ <--> {delay=1ms;} <--> H3.ethg++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs1.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs2.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs3.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs4.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs5.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs6.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs7.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs8.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs9.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs10.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs11.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs12.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs13.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs14.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs15.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs16.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs17.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs18.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs19.gate_controller++;

        ovs.ethg++ <--> ethernetline <--> ovs1.ethg++;
        ovs.ethg++ <--> ethernetline <--> ovs2.ethg++;
        ovs3.ethg++ <--> ethernetline <--> ovs1.ethg++;
        ovs3.ethg++ <--> ethernetline <--> ovs2.ethg++;
        ovs1.ethg++ <--> ethernetline <--> H1.ethg++;
        ovs1.ethg++ <--> ethernetline <--> H2.ethg++;
        ovs2.ethg++ <--> ethernetline <--> H3.ethg++;
        ovs2.ethg++ <--> ethernetline <--> H4.ethg++;

        ovs4.ethg++ <--> ethernetline <--> ovs5.ethg++;
        ovs4.ethg++ <--> ethernetline <--> ovs7.ethg++;
        ovs6.ethg++ <--> ethernetline <--> ovs5.ethg++;
        ovs6.ethg++ <--> ethernetline <--> ovs7.ethg++;
        ovs5.ethg++ <--> ethernetline <--> H5.ethg++;
        ovs5.ethg++ <--> ethernetline <--> H6.ethg++;
        ovs7.ethg++ <--> ethernetline <--> H7.ethg++;
        ovs7.ethg++ <--> ethernetline <--> H8.ethg++;

        ovs9.ethg++ <--> ethernetline <--> ovs11.ethg++;
        ovs9.ethg++ <--> ethernetline <--> ovs14.ethg++;
        ovs10.ethg++ <--> ethernetline <--> ovs11.ethg++;
        ovs10.ethg++ <--> ethernetline <--> ovs14.ethg++;
        ovs11.ethg++ <--> ethernetline <--> H9.ethg++;
        ovs11.ethg++ <--> ethernetline <--> H10.ethg++;
        ovs14.ethg++ <--> ethernetline <--> H11.ethg++;
        ovs14.ethg++ <--> ethernetline <--> H12.ethg++;

        ovs12.ethg++ <--> ethernetline <--> ovs8.ethg++;
        ovs12.ethg++ <--> ethernetline <--> ovs13.ethg++;
        ovs15.ethg++ <--> ethernetline <--> ovs8.ethg++;
        ovs15.ethg++ <--> ethernetline <--> ovs13.ethg++;
        ovs8.ethg++ <--> ethernetline <--> H13.ethg++;
        ovs8.ethg++ <--> ethernetline <--> H14.ethg++;
        ovs13.ethg++ <--> ethernetline <--> H15.ethg++;
        ovs13.ethg++ <--> ethernetline <--> H16.ethg++;


        ovs16.ethg++ <--> ethernetline <--> ovs.ethg++;
        ovs16.ethg++ <--> ethernetline <--> ovs4.ethg++;
        ovs16.ethg++ <--> ethernetline <--> ovs9.ethg++;
        ovs16.ethg++ <--> ethernetline <--> ovs12.ethg++;
        
        ovs17.ethg++ <--> ethernetline <--> ovs.ethg++;
        ovs17.ethg++ <--> ethernetline <--> ovs4.ethg++;
        ovs17.ethg++ <--> ethernetline <--> ovs9.ethg++;
        ovs17.ethg++ <--> ethernetline <--> ovs12.ethg++;
        
        ovs18.ethg++ <--> ethernetline <--> ovs3.ethg++;
        ovs18.ethg++ <--> ethernetline <--> ovs6.ethg++;
        ovs18.ethg++ <--> ethernetline <--> ovs10.ethg++;
        ovs18.ethg++ <--> ethernetline <--> ovs15.ethg++;
        
        ovs19.ethg++ <--> ethernetline <--> ovs3.ethg++;
        ovs19.ethg++ <--> ethernetline <--> ovs6.ethg++;
        ovs19.ethg++ <--> ethernetline <--> ovs10.ethg++;
        ovs19.ethg++ <--> ethernetline <--> ovs15.ethg++;
}
