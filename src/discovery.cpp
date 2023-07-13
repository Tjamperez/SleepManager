#include "../include/discovery.h"
#include "../include/socket.h"
#include "../include/address.h"

void discovery_main(shared_ptr<WorkStationTable> work_station_table)
{
    NodeAddresses host = NodeAddresses::load_host();
    UdpSocket socket(host.ip);
    socket.enable_broadcast();
    while (true) {
        Packet packet;
        if (
            socket.receive(packet, BROADCAST_PORT)
            && packet.type == Packet::DISCOVERY_REQ
        ) {
        }
    }
}
