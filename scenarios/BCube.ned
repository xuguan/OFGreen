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


network BCube
{
    @display("bgb=1048,481");

    types:
        channel ethernetline extends ThruputMeteringChannel
        {
            delay = 1us;
            datarate = 1000Mbps;
            thruputDisplayFormat = "u";
        }
    submodules:
        configurator: FlatNetworkConfigurator {
            parameters:
                @display("p=44,37");
        }
        controller: Open_Flow_Controller {
            @display("p=162,37");
        }
        ovs1: Open_Flow_Switch {
            @display("p=91,304");
        }

        hostovs1: Open_Flow_Switch {
            @display("p=47,387");
        }
        ovs5: Open_Flow_Switch {
            @display("p=83,198");
        }
        hostovs2: Open_Flow_Switch {
            @display("p=106,387");
        }
        hostovs3: Open_Flow_Switch {
            @display("p=172,387");
        }
        hostovs4: Open_Flow_Switch {
            @display("p=238,387");
        }
        H1: StandardHost {
            @display("p=46,433;is=s");
        }

        H2: StandardHost {
            @display("p=106,433;is=s");
        }
        H3: StandardHost {
            @display("p=172,433;is=s");
        }
        H4: StandardHost {
            @display("p=238,433;is=s");
        }
        H5: StandardHost {
            @display("p=309,429;is=s");
        }
        H6: StandardHost {
            @display("p=365,429;is=s");
        }
        H7: StandardHost {
            @display("p=415,429;is=s");
        }
        H8: StandardHost {
            @display("p=479,429;is=s");
        }
        H9: StandardHost {
            @display("p=537,429;is=s");
        }
        H10: StandardHost {
            @display("p=597,429;is=s");
        }
        H11: StandardHost {
            @display("p=663,429;is=s");
        }
        H12: StandardHost {
            @display("p=729,429;is=s");
        }
        H13: StandardHost {
            @display("p=798,422;is=s");
        }
        H14: StandardHost {
            @display("p=858,429;is=s");
        }
        H15: StandardHost {
            @display("p=924,429;is=s");
        }
        H16: StandardHost {
            @display("p=990,429;is=s");
        }
        ovs2: Open_Flow_Switch {
            @display("p=365,296");
        }

        hostovs5: Open_Flow_Switch {
            @display("p=309,387");
        }
        hostovs6: Open_Flow_Switch {
            @display("p=365,387");
        }
        hostovs7: Open_Flow_Switch {
            @display("p=415,387");
        }
        hostovs8: Open_Flow_Switch {
            @display("p=479,387");
        }
        ovs3: Open_Flow_Switch {
            @display("p=613,296");
        }

        hostovs10: Open_Flow_Switch {
            @display("p=597,387");
        }
        hostovs9: Open_Flow_Switch {
            @display("p=538,387");
        }
        hostovs11: Open_Flow_Switch {
            @display("p=663,387");
        }
        hostovs12: Open_Flow_Switch {
            @display("p=729,387");
        }
        hostovs13: Open_Flow_Switch {
            @display("p=799,387");
        }
        hostovs14: Open_Flow_Switch {
            @display("p=858,387");
        }
        hostovs15: Open_Flow_Switch {
            @display("p=924,387");
        }
        hostovs16: Open_Flow_Switch {
            @display("p=990,387");
        }

        ovs4: Open_Flow_Switch {
            @display("p=874,296");
        }

        ovs6: Open_Flow_Switch {
            @display("p=297,189");
        }
        ovs7: Open_Flow_Switch {
            @display("p=462,189");
        }
        ovs8: Open_Flow_Switch {
            @display("p=597,189");
        }
    connections allowunconnected:

        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs1.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs2.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs3.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs4.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs5.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs6.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs7.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs8.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> hostovs1.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> hostovs2.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> hostovs3.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> hostovs4.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> hostovs5.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> hostovs6.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> hostovs7.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> hostovs8.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> hostovs9.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> hostovs10.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> hostovs11.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> hostovs12.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> hostovs13.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> hostovs14.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> hostovs15.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> hostovs16.gate_controller++;

        ovs1.ethg++ <--> ethernetline <--> hostovs1.ethg++;
        ovs1.ethg++ <--> ethernetline <--> hostovs2.ethg++;
        ovs1.ethg++ <--> ethernetline <--> hostovs3.ethg++;
        ovs1.ethg++ <--> ethernetline <--> hostovs4.ethg++;
        ovs2.ethg++ <--> ethernetline <--> hostovs8.ethg++;
        ovs2.ethg++ <--> ethernetline <--> hostovs5.ethg++;
        ovs2.ethg++ <--> ethernetline <--> hostovs7.ethg++;
        ovs2.ethg++ <--> ethernetline <--> hostovs6.ethg++;
        ovs3.ethg++ <--> ethernetline <--> hostovs12.ethg++;
        ovs3.ethg++ <--> ethernetline <--> hostovs9.ethg++;
        ovs3.ethg++ <--> ethernetline <--> hostovs11.ethg++;
        ovs3.ethg++ <--> ethernetline <--> hostovs10.ethg++;
        ovs4.ethg++ <--> ethernetline <--> hostovs15.ethg++;
        ovs4.ethg++ <--> ethernetline <--> hostovs16.ethg++;
        ovs4.ethg++ <--> ethernetline <--> hostovs13.ethg++;
        ovs4.ethg++ <--> ethernetline <--> hostovs14.ethg++;

        hostovs1.ethg++ <--> ethernetline <--> H1.ethg++;
        hostovs2.ethg++ <--> ethernetline <--> H2.ethg++;
        hostovs3.ethg++ <--> ethernetline <--> H3.ethg++;
        hostovs4.ethg++ <--> ethernetline <--> H4.ethg++;
        hostovs6.ethg++ <--> ethernetline <--> H6.ethg++;
        hostovs5.ethg++ <--> ethernetline <--> H5.ethg++;
        hostovs7.ethg++ <--> ethernetline <--> H7.ethg++;
        hostovs8.ethg++ <--> ethernetline <--> H8.ethg++;
        hostovs9.ethg++ <--> ethernetline <--> H9.ethg++;
        hostovs10.ethg++ <--> ethernetline <--> H10.ethg++;
        hostovs11.ethg++ <--> ethernetline <--> H11.ethg++;
        hostovs12.ethg++ <--> ethernetline <--> H12.ethg++;
        hostovs16.ethg++ <--> ethernetline <--> H16.ethg++;
        hostovs14.ethg++ <--> ethernetline <--> H14.ethg++;
        hostovs13.ethg++ <--> ethernetline <--> H13.ethg++;
        hostovs15.ethg++ <--> ethernetline <--> H15.ethg++;

        ovs5.ethg++ <--> ethernetline <--> hostovs1.ethg++;
        ovs6.ethg++ <--> ethernetline <--> hostovs2.ethg++;
        ovs7.ethg++ <--> ethernetline <--> hostovs3.ethg++;
        ovs8.ethg++ <--> ethernetline <--> hostovs4.ethg++;
        ovs5.ethg++ <--> ethernetline <--> hostovs5.ethg++;
        ovs6.ethg++ <--> ethernetline <--> hostovs6.ethg++;
        ovs7.ethg++ <--> ethernetline <--> hostovs7.ethg++;
        ovs8.ethg++ <--> ethernetline <--> hostovs8.ethg++;
        ovs5.ethg++ <--> ethernetline <--> hostovs9.ethg++;
        ovs6.ethg++ <--> ethernetline <--> hostovs10.ethg++;
        ovs7.ethg++ <--> ethernetline <--> hostovs11.ethg++;
        ovs8.ethg++ <--> ethernetline <--> hostovs12.ethg++;
        ovs5.ethg++ <--> ethernetline <--> hostovs13.ethg++;
        ovs6.ethg++ <--> ethernetline <--> hostovs14.ethg++;
        ovs7.ethg++ <--> ethernetline <--> hostovs15.ethg++;
        ovs8.ethg++ <--> ethernetline <--> hostovs16.ethg++;
}
