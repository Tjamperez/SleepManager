#include "../include/management.h"
#include "../include/participant.h"
#include "../include/interface.h"
#include "../include/socket.h"
#include <thread>
#include <csignal>

#define EXIT_RETRY_BOUND 3

static atomic<IpAddress> manager_ip;

static void sigint_handler(int signal);

static NodeAddresses connect_to_manager(ClientSocket& client_socket);

static void disconnect_to_manager(
    ClientSocket& client_socket,
    IpAddress manager_ip);

static void listen_wol();

static void listen_monitoring(Mpsc<ParticipantMsg>::Sender sender);

void participant_main()
{
    ClientSocket client_socket;
    
    thread wol_thread(listen_wol);
    wol_thread.detach();

    NodeAddresses manager_addresses = connect_to_manager(client_socket);

    cerr << "Connected to " << manager_addresses.ip << endl;

    manager_ip = manager_addresses.ip;
    signal(SIGINT, sigint_handler);

    auto channel = Mpsc<ParticipantMsg>::Channel::open();
    Mpsc<ParticipantMsg>::Sender sender = move(channel.sender);
    Mpsc<ParticipantMsg>::Receiver receiver = move(channel.receiver);

    thread monitoring_thread(listen_monitoring, sender);
    monitoring_thread.detach();


    thread interface_thread(participant_interface_main, sender);
    interface_thread.detach();

    bool exit = false;
    while (!exit) {
        auto message = receiver.receive();
        if (message.has_value()) {
            switch (message.value()) {
                case PARTICIPANT_EXIT:
                    disconnect_to_manager(client_socket, manager_addresses.ip);
                case PARTICIPANT_MANG_EXITED:
                    exit = true;
                    break;
            }
        } else {
            exit = true;
            disconnect_to_manager(client_socket, manager_addresses.ip);
        }
    }

}

static NodeAddresses connect_to_manager(ClientSocket& client_socket)
{
    client_socket.enable_broadcast();

    PacketBody packet_body;
    packet_body.type = PacketBody::DISCOVERY;
    auto request = client_socket.request(
        packet_body,
        IpAddress { 255, 255, 255, 255 },
        DISCOVERY_MANAGER_PORT 
    );
    Packet response = request.receive_response(DISCOVERY_PARTICIPANT_PORT);

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
    request.receive_bounded(EXIT_RETRY_BOUND, EXIT_PORT);
}

static void listen_wol()
{
    ServerSocket socket(WOL_PARTICIPANT_PORT);
    socket.enable_broadcast();
    while (true) {
        socket.handle_wol(IpAddress { 255, 255, 255, 255 }, WOL_PARTICIPANT_PORT);
    }
}

static void listen_monitoring(Mpsc<ParticipantMsg>::Sender sender)
{
    ServerSocket socket(SLEEP_STATUS_PARTICIPANT_PORT);
    bool done = false;
    while (!done) {
        ServerSocket::Request request = socket.receive();
        switch (request.received_packet().body.type) {
            case PacketBody::SLEEP_STATUS:
                request.respond(SLEEP_STATUS_MANAGER_PORT);
                break;
            case PacketBody::EXIT:
                request.respond(EXIT_PORT);
                done = true;
                break;
        }
    }
    sender.send(PARTICIPANT_MANG_EXITED);
}

static void sigint_handler(int signal)
{
    ClientSocket client_socket;
    disconnect_to_manager(client_socket, manager_ip);
    exit(130);
}
