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

package openflow;

//Message processing on data plane;
//Perform flow table lookups;
//Notify switch application module about unmatched pakets;
//Store packets in buffer during controller request;

simple Open_Flow_Processing
{
    parameters:
        @display("i=block/arrival");
        double serviceTime @unit("s") = default(0.000015s);
        double active2sleepTime @unit("s") = default(2s);
        double active2sleepTimePort @unit("s") = default(0.02s);
        double powerSampleTime @unit("s") = default(3s);
        double switchpower @unit("w") =default (36w);
        double linkCapacity = default (1000);//1000Mbps
        double portpower @unit("w") =default (1w);
        int portNum =default(4);// # of ports per switch.
        double portTx @unit("bps") =default (1000000000bps);
    gates:
        input ifIn[];
        output ifOut[];
}
