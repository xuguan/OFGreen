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


network DCN_FatTreeN
{
    parameters:
        @display("bgb=743,493");
        int n = default(4); // half number of the ports per switch
    types:
        channel ethernetline extends ThruputMeteringChannel
        {
            delay = 1us;
            datarate = 100Mbps;
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
        coreOVS[n*n]: Open_Flow_Switch{
            @display("p=,107");
        }
        podOVS[4*n*n]: Open_Flow_Switch{
            @display("p=,307");
        }
        server[2*n*n*n]: StandardHost{
            @display("p=,437");
        }

    connections allowunconnected:

        //        client1.ethg++ <--> {delay=1ms;} <--> ovs.ethg++;
        //        ovs.ethg++ <--> {delay=1ms;} <--> H1.ethg++;
        //        controller.ethg++ <--> {delay=1ms;} <--> ovs.gate_controller++;
        //        ovs.ethg++ <--> {delay=1ms;} <--> H2.ethg++;
        //        ovs.ethg++ <--> {delay=1ms;} <--> H3.ethg++;
        for i = 0..n*n-1 {            
            controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> coreOVS[i].gate_controller++;            	
        }
        
        for i = 0..4*n*n-1 {            
            controller.ethg++ <--> ethernetline { @display("ls=,,da"); } <--> podOVS[i].gate_controller++;            	
        }
        
        for ij=0..2*n-1, for ii = n..2*n-1 , for ik=0..n-1{
        	    podOVS[ij*2*n+ii].ethg++  <--> ethernetline <--> coreOVS[ii*n-n*n+ik].ethg++;   
        }
        
        for hp=0..2*n-1, for hi = 0..n-1 , for hk=0..n-1{
        	    podOVS[hp*2*n+hi].ethg++  <--> ethernetline <--> server[hp*n*n+hi*n+hk].ethg++;   
        	    podOVS[hp*2*n+hi].ethg++  <--> ethernetline <--> podOVS[hp*2*n+n+hi].ethg++;
        }
        
}
