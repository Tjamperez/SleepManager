#include "../include/discovery.h"
#include "../include/socket.h"
#include "../include/address.h"
#include "../include/management.h"
#include <thread>

static void discovery_response(
    ServerSocket::Request request,
    shared_ptr<ManagementService> management_service)
{
    NodeAddresses sender_addresses =
        request.received_packet().header.sender_addresses;
    shared_ptr<WorkStation> work_station(new WorkStation(
        sender_addresses,
        WorkStation::AWAKEN
    ));
    management_service->insert(work_station);
    request.respond(DEFAULT_PORT);
}

void discovery_main(shared_ptr<ManagementService> management_service)
{
    ServerSocket socket(DISCOVERY_PORT);
    socket.enable_broadcast();

    while (true) {
        ServerSocket::Request request = socket.receive();
        thread response_thread(discovery_response, request, management_service);
        response_thread.detach();
    }
}
