#include "../include/discovery.h"
#include "../include/socket.h"
#include "../include/address.h"
#include <thread>

static void discovery_response(
    ServerSocket::Request request,
    shared_ptr<WorkStationTable> work_station_table)
{
    NodeAddresses sender_addresses =
        request.received_packet().header.sender_addresses;
    shared_ptr<WorkStation> work_station(new WorkStation(
        sender_addresses,
        WorkStation::AWAKE
    ));
    work_station_table->insert(work_station);
    request.respond(DEFAULT_PORT);
}

void discovery_main(shared_ptr<WorkStationTable> work_station_table)
{
    ServerSocket socket(BROADCAST_PORT);

    while (true) {
        ServerSocket::Request request = socket.receive();
        thread response_thread(discovery_response, request, work_station_table);
        response_thread.detach();
    }
}
