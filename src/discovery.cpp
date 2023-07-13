#include "../include/discovery.h"
#include "../include/socket.h"
#include "../include/address.h"

void discovery_main(shared_ptr<WorkStationTable> work_station_table)
{
    NodeAddresses host = NodeAddresses::load_host();
    UdpSocket socket(localhost.ip);
    socket.enable_broadcast();
    while (1) {
        Packet packet;
        socket.receive();
    }
}
