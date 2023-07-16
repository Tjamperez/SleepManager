#include "../include/work_station.h"
#include "../include/participant.h"
#include "../include/interface.h"
#include "../include/socket.h"
#include <thread>

static NodeAddresses connect_to_manager(ClientSocket& client_socket);

static void disconnect_to_manager(
    ClientSocket& client_socket,
    IpAddress manager_ip);

static void listen_wakeup_message(
    IpAddress manager_ip,
    Mpsc<ParticipantMsg>::Receiver receiver);

void participant_main(void)
{
    ClientSocket client_socket;
    NodeAddresses manager_addresses = connect_to_manager(client_socket);

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
        BROADCAST_PORT
    );
    Packet response = request.receive_response();

    client_socket.enable_broadcast(false);

    return response.header.sender_addresses;
}

static void disconnect_to_manager(
    ClientSocket& client_socket,
    IpAddress manager_ip)
{
    PacketBody packet_body;
    packet_body.type = PacketBody::EXIT;
    auto request = client_socket.request(packet_body, manager_ip);
    request.receive_response();
}

static void listen_wakeup_message(
    IpAddress manager_ip,
    Mpsc<ParticipantMsg>::Receiver receiver)
{
}
