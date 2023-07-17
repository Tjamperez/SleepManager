#include "../include/management.h"
#include "../include/manager.h"
#include "../include/discovery.h"
#include "../include/monitoring.h"
#include "../include/socket.h"
#include "../include/interface.h"
#include <csignal>
#include <thread>

static void sigint_handler(int signal);

void manager_main(void)
{
    signal(SIGINT, sigint_handler);

    shared_ptr<ManagementService> management_service;
    thread monitoring_thread(monitoring_main, management_service);
    monitoring_thread.detach();
    thread discovery_thread(discovery_main, management_service);
    discovery_thread.detach();

    manager_interface_main(management_service);
}

static void sigint_handler(int signal)
{
    ClientSocket client_socket;
    client_socket.enable_broadcast();
    PacketBody packet_body;
    packet_body.type = PacketBody::EXIT;
    client_socket.request(
        packet_body, IpAddress { 255, 255, 255, 255 },
        EXIT_PORT);
    exit(130);
}
