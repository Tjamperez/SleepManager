#include "../include/management.h"
#include "../include/participant.h"
#include "../include/interface.h"
#include "../include/socket.h"
#include <thread>
#include <csignal>

static atomic<IpAddress> manager_ip;

static void sigint_handler(int signal);

static NodeAddresses connect_to_manager(ClientSocket& client_socket);

static void disconnect_to_manager(
    ClientSocket& client_socket,
    IpAddress manager_ip);

static void listen_wol();

void participant_main(void)
{
    ClientSocket client_socket;
    
    thread wol_thread(listen_wol);
    wol_thread.detach();

    NodeAddresses manager_addresses = connect_to_manager(client_socket);
    manager_ip = manager_addresses.ip;
    signal(SIGINT, sigint_handler);

    auto channel_pair = Mpsc<ParticipantMsg>::open();
    Mpsc<ParticipantMsg>::Sender sender = move(get<0>(channel_pair));
    Mpsc<ParticipantMsg>::Receiver receiver = move(get<1>(channel_pair));

    thread interface_thread(participant_interface_main, sender);
    interface_thread.detach();

    bool exit = false;
    while (!exit) {
        auto message = receiver.receive();
        if (message.has_value()) {
            switch (message.value()) {
                case PARTICIPANT_EXIT:
                    exit = true;
                    break;
            }
        } else {
            exit = true;
        }
    }

    disconnect_to_manager(client_socket, manager_addresses.ip);
}

static NodeAddresses connect_to_manager(ClientSocket& client_socket)
{
    client_socket.enable_broadcast();

    PacketBody packet_body;
    packet_body.type = PacketBody::DISCOVERY;
    auto request = client_socket.request(
        packet_body,
        IpAddress { 255, 255, 255, 255 },
        DISCOVERY_PORT 
    );
    Packet response = request.receive_response(DISCOVERY_PORT);

    client_socket.enable_broadcast(false);

    return response.header.sender_addresses;
}

static void disconnect_to_manager(
    ClientSocket& client_socket,
    IpAddress manager_ip)
{
    PacketBody packet_body;
    packet_body.type = PacketBody::EXIT;
    auto request = client_socket.request(packet_body, manager_ip, EXIT_PORT);
    request.receive_response(EXIT_PORT);
}

static void listen_wol()
{
    ServerSocket socket(WOL_PARTICIPANT_PORT);
    socket.enable_broadcast();
    while (true) {
        socket.handle_wol(IpAddress { 255, 255, 255, 255 }, WOL_PARTICIPANT_PORT);
    }
}

static void sigint_handler(int signal)
{
    ClientSocket client_socket;
    disconnect_to_manager(client_socket, manager_ip);
    exit(130);
}
