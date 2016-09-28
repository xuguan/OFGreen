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


network TestTCPclient
{
    @display("bgb=743,493");

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
            @display("p=228,202");
        }

        ovs1: Open_Flow_Switch {
            @display("p=225,287");
        }
        H1: StandardHost {
            @display("p=234,349;is=s");
        }
        H2: StandardHost {
            @display("p=181,352;is=s");
        }
    connections allowunconnected:

        controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> ovs1.gate_controller++;
        ovs1.ethg++ <--> ethernetline <--> H1.ethg++;
        ovs1.ethg++ <--> ethernetline <--> H2.ethg++;
}
