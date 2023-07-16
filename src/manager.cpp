#include "../include/work_station.h"
#include "../include/manager.h"
#include "../include/discovery.h"
#include "../include/socket.h"
#include "../include/interface.h"
#include <csignal>
#include <thread>

static void sigint_handler(int signal);

void manager_main(void)
{
    signal(SIGINT, sigint_handler);

    shared_ptr<WorkStationTable> participants;
    thread discovery_thread(discovery_main, participants);
    discovery_thread.detach();

    manager_interface_main(participants);
}

static void sigint_handler(int signal)
{
    ClientSocket client_socket;
    client_socket.enable_broadcast();
    PacketBody packet_body;
    packet_body.type = PacketBody::EXIT;
    client_socket.request(packet_body, IpAddress { 255, 255, 255, 255 });
    exit(130);
}
