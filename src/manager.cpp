#include "../include/address.h"
#include "../include/management.h"
#include "../include/manager.h"
#include "../include/discovery.h"
#include "../include/monitoring.h"
#include "../include/socket.h"
#include "../include/interface.h"
#include <csignal>
#include <thread>

#define EXIT_RETRY_BOUND 3

static void sigint_handler(int signal);

void manager_main()
{
    signal(SIGINT, sigint_handler);

    shared_ptr<ManagementService> management_service(new ManagementService());
    thread monitoring_thread(monitoring_main, management_service);
    monitoring_thread.detach();
    thread discovery_thread(discovery_main, management_service);
    discovery_thread.detach();

    manager_interface_main(management_service);

    manager_send_exit();
}

void manager_send_exit()
{
    ClientSocket client_socket;
    client_socket.enable_broadcast();
    PacketBody packet_body;
    packet_body.type = PacketBody::EXIT;
    auto request = client_socket.request(
        packet_body, IpAddress { 255, 255, 255, 255 },
        SLEEP_STATUS_PARTICIPANT_PORT);
    request.receive_bounded(EXIT_RETRY_BOUND, EXIT_PORT);
}

static void sigint_handler(int signal)
{
    manager_send_exit();
    exit(130);
}
