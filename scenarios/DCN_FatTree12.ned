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


network DCN_FatTree8
{
    @display("bgb=1564,1017");

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
            @display("p=45,138");
        }
        ovs: Open_Flow_Switch {
            @display("p=225,228");
        }

        ovs1: Open_Flow_Switch {
            @display("p=100,408");
        }
        ovs2: Open_Flow_Switch {
            @display("p=250,408");
        }
        ovs3: Open_Flow_Switch {
            @display("p=400,408");
        }
        ovs4: Open_Flow_Switch {
            @display("p=550,408");
        }
        ovs5: Open_Flow_Switch {
            @display("p=700,408");
        }
        ovs6: Open_Flow_Switch {
            @display("p=850,410");
        }

        ovs7: Open_Flow_Switch {
            @display("p=100,318");
        }
        ovs8: Open_Flow_Switch {
            @display("p=250,318");
        }
        ovs9: Open_Flow_Switch {
            @display("p=400,318");
        }
        ovs10: Open_Flow_Switch {
            @display("p=550,318");
        }
        ovs11: Open_Flow_Switch {
            @display("p=700,320");
        }
        ovs12: Open_Flow_Switch {
            @display("p=850,322");
        }

        H1: StandardHost {
            @display("p=25,498;is=s");
        }
        H2: StandardHost {
            @display("p=50,498;is=s");
        }
        H3: StandardHost {
            @display("p=75,498;is=s");
        }
        H4: StandardHost {
            @display("p=100,498;is=s");
        }
        H5: StandardHost {
            @display("p=125,498;is=s");
        }
        H6: StandardHost {
            @display("p=150,498;is=s");
        }
        H7: StandardHost {
            @display("p=175,498;is=s");
        }
        H8: StandardHost {
            @display("p=200,498;is=s");
        }
        H9: StandardHost {
            @display("p=225,498;is=s");
        }
        H10: StandardHost {
            @display("p=250,495;is=s");
        }
        H11: StandardHost {
            @display("p=275,498;is=s");
        }
        H12: StandardHost {
            @display("p=300,498;is=s");
        }
        H13: StandardHost {
            @display("p=325,498;is=s");
        }
        H14: StandardHost {
            @display("p=350,498;is=s");
        }
        H15: StandardHost {
            @display("p=375,498;is=s");
        }
        H16: StandardHost {
            @display("p=400,498;is=s");
        }
        H17: StandardHost {
            @display("p=425,498;is=s");
        }
        H18: StandardHost {
            @display("p=450,498;is=s");
        }
        H19: StandardHost {
            @display("p=475,500;is=s");
        }
        H20: StandardHost {
            @display("p=500,500;is=s");
        }
        H21: StandardHost {
            @display("p=525,500;is=s");
        }
        H22: StandardHost {
            @display("p=550,499;is=s");
        }
        H23: StandardHost {
            @display("p=575,499;is=s");
        }
        H24: StandardHost {
            @display("p=600,499;is=s");
        }
        H25: StandardHost {
            @display("p=625,499;is=s");
        }
        H26: StandardHost {
            @display("p=651,499;is=s");
        }
        H27: StandardHost {
            @display("p=675,499;is=s");
        }
        H28: StandardHost {
            @display("p=700,499;is=s");
        }
        H29: StandardHost {
            @display("p=725,499;is=s");
        }
        H30: StandardHost {
            @display("p=750,499;is=s");
        }
        H31: StandardHost {
            @display("p=775,499;is=s");
        }
        H32: StandardHost {
            @display("p=800,499;is=s");
        }
        H33: StandardHost {
            @display("p=825,499;is=s");
        }
        H34: StandardHost {
            @display("p=850,499;is=s");
        }
        H35: StandardHost {
            @display("p=875,499;is=s");
        }
        H36: StandardHost {
            @display("p=900,499;is=s");
        }
    connections allowunconnected:

        //        client1.ethg++ <--> {delay=1ms;} <--> ovs.ethg++;
        //        ovs.ethg++ <--> {delay=1ms;} <--> H1.ethg++;
        //        controller.ethg++ <--> {delay=1ms;} <--> ovs.gate_controller++;
        //        ovs.ethg++ <--> {delay=1ms;} <--> H2.ethg++;
        //        ovs.ethg++ <--> {delay=1ms;} <--> H3.ethg++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs7.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs1.gate_controller++;
        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs.gate_controller++;
        ovs7.ethg++ <--> ethernetline <--> ovs1.ethg++;
        ovs7.ethg++ <--> ethernetline <--> ovs2.ethg++;
        ovs7.ethg++ <--> ethernetline <--> ovs3.ethg++;
        ovs7.ethg++ <--> ethernetline <--> ovs4.ethg++;
        ovs7.ethg++ <--> ethernetline <--> ovs5.ethg++;
        ovs7.ethg++ <--> ethernetline <--> ovs6.ethg++;
        ovs8.ethg++ <--> ethernetline <--> ovs1.ethg++;
        ovs8.ethg++ <--> ethernetline <--> ovs2.ethg++;
        ovs8.ethg++ <--> ethernetline <--> ovs3.ethg++;
        ovs8.ethg++ <--> ethernetline <--> ovs4.ethg++;
        ovs8.ethg++ <--> ethernetline <--> ovs5.ethg++;
        ovs8.ethg++ <--> ethernetline <--> ovs6.ethg++;
        ovs9.ethg++ <--> ethernetline <--> ovs1.ethg++;
        ovs9.ethg++ <--> ethernetline <--> ovs2.ethg++;
        ovs9.ethg++ <--> ethernetline <--> ovs3.ethg++;
        ovs9.ethg++ <--> ethernetline <--> ovs4.ethg++;
        ovs9.ethg++ <--> ethernetline <--> ovs5.ethg++;
        ovs9.ethg++ <--> ethernetline <--> ovs6.ethg++;
        ovs10.ethg++ <--> ethernetline <--> ovs1.ethg++;
        ovs10.ethg++ <--> ethernetline <--> ovs2.ethg++;
        ovs10.ethg++ <--> ethernetline <--> ovs3.ethg++;
        ovs10.ethg++ <--> ethernetline <--> ovs4.ethg++;
        ovs10.ethg++ <--> ethernetline <--> ovs5.ethg++;
        ovs10.ethg++ <--> ethernetline <--> ovs6.ethg++;
        ovs11.ethg++ <--> ethernetline <--> ovs1.ethg++;
        ovs11.ethg++ <--> ethernetline <--> ovs2.ethg++;
        ovs11.ethg++ <--> ethernetline <--> ovs3.ethg++;
        ovs11.ethg++ <--> ethernetline <--> ovs4.ethg++;
        ovs11.ethg++ <--> ethernetline <--> ovs5.ethg++;
        ovs11.ethg++ <--> ethernetline <--> ovs6.ethg++;
        ovs12.ethg++ <--> ethernetline <--> ovs1.ethg++;
        ovs12.ethg++ <--> ethernetline <--> ovs2.ethg++;
        ovs12.ethg++ <--> ethernetline <--> ovs3.ethg++;
        ovs12.ethg++ <--> ethernetline <--> ovs4.ethg++;
        ovs12.ethg++ <--> ethernetline <--> ovs5.ethg++;
        ovs12.ethg++ <--> ethernetline <--> ovs6.ethg++;

        ovs1.ethg++ <--> ethernetline <--> H1.ethg++;
        ovs1.ethg++ <--> ethernetline <--> H2.ethg++;
        ovs1.ethg++ <--> ethernetline <--> H3.ethg++;
        ovs1.ethg++ <--> ethernetline <--> H4.ethg++;
        ovs1.ethg++ <--> ethernetline <--> H5.ethg++;
        ovs1.ethg++ <--> ethernetline <--> H6.ethg++;

        ovs2.ethg++ <--> ethernetline <--> H7.ethg++;
        ovs2.ethg++ <--> ethernetline <--> H8.ethg++;
        ovs2.ethg++ <--> ethernetline <--> H9.ethg++;
        ovs2.ethg++ <--> ethernetline <--> H10.ethg++;
        ovs2.ethg++ <--> ethernetline <--> H11.ethg++;
        ovs2.ethg++ <--> ethernetline <--> H12.ethg++;

        ovs3.ethg++ <--> ethernetline <--> H13.ethg++;
        ovs3.ethg++ <--> ethernetline <--> H14.ethg++;
        ovs3.ethg++ <--> ethernetline <--> H15.ethg++;
        ovs3.ethg++ <--> ethernetline <--> H16.ethg++;
        ovs3.ethg++ <--> ethernetline <--> H17.ethg++;
        ovs3.ethg++ <--> ethernetline <--> H18.ethg++;

        ovs4.ethg++ <--> ethernetline <--> H19.ethg++;
        ovs4.ethg++ <--> ethernetline <--> H20.ethg++;
        ovs4.ethg++ <--> ethernetline <--> H21.ethg++;
        ovs4.ethg++ <--> ethernetline <--> H22.ethg++;
        ovs4.ethg++ <--> ethernetline <--> H23.ethg++;
        ovs4.ethg++ <--> ethernetline <--> H24.ethg++;

        ovs5.ethg++ <--> ethernetline <--> H25.ethg++;
        ovs5.ethg++ <--> ethernetline <--> H26.ethg++;
        ovs5.ethg++ <--> ethernetline <--> H27.ethg++;
        ovs5.ethg++ <--> ethernetline <--> H28.ethg++;
        ovs5.ethg++ <--> ethernetline <--> H29.ethg++;
        ovs5.ethg++ <--> ethernetline <--> H30.ethg++;
        ovs6.ethg++ <--> ethernetline <--> H31.ethg++;
        ovs6.ethg++ <--> ethernetline <--> H32.ethg++;
        ovs6.ethg++ <--> ethernetline <--> H33.ethg++;
        ovs6.ethg++ <--> ethernetline <--> H34.ethg++;
        ovs6.ethg++ <--> ethernetline <--> H35.ethg++;
        ovs6.ethg++ <--> ethernetline <--> H36.ethg++;

        ovs.ethg++ <--> ethernetline <--> ovs7.ethg++;

}
