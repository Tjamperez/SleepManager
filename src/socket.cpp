#include "../include/socket.h"
#include "../include/io_exception.h"
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <net/if.h>
#include <limits.h>
#include <algorithm>
#include <ifaddrs.h>

#define STR_BUF_SIZE 0xffff

SenderSocket::SenderSocket(): fd(-1)
{
    socket(AF_INET, SOCK_DGRAM, 0);
    if (this->fd < 0) {
        throw IOException("sender socket");
    }
}

SenderSocket::~SenderSocket()
{
    if (this->fd >= 0) {
        close(this->fd);
    }
}

void SenderSocket::send(
    uint8_t const *buffer,
    size_t length,
    IpAddress dest_ip_address,
    uint16_t dest_port
)
{
    struct sockaddr_in sockaddr;
    socklen_t socklen;

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = dest_port;
    copy(
        dest_ip_address.cbegin(),
        dest_ip_address.cend(),
        (uint8_t *) &sockaddr.sin_addr.s_addr
    );

    ssize_t res = sendto(
        this->fd,
        buffer,
        length,
        0,
        (struct sockaddr *) &sockaddr,
        socklen
    );
    if (res < 0) {
        throw IOException("sendto");
    }
}

void SenderSocket::send(
    string message,
    IpAddress dest_ip_address,
    uint16_t dest_port
)
{
    this->send(
        (uint8_t *) message.c_str(),
        message.length(),
        dest_ip_address,
        dest_port
    );
}

void SenderSocket::send(
    Packet const& packet,
    IpAddress dest_ip_address,
    uint16_t dest_port
)
{
    PacketSerializer serializer;
    packet.serialize(serializer);
    string message = serializer.finish();
    this->send(message, dest_ip_address, dest_port);
}

void SenderSocket::enable_broadcast(bool enable)
{
    int enabled = enable;
    int status = setsockopt(
        this->fd, SOL_SOCKET, SO_BROADCAST, &enabled, sizeof(enabled)
    );
    if (status < 0) {
        throw IOException("setsockopt for sender broadcast");
    }
}

ReceiverSocket::ReceiverSocket(uint16_t port): fd(-1)
{
    NodeAddresses addresses = NodeAddresses::load_host();
    socket(AF_INET, SOCK_DGRAM, 0);
    if (this->fd < 0) {
        throw IOException("receiver socket");
    }
    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    copy(
        addresses.ip.begin(),
        addresses.ip.end(),
        (uint8_t *) &sockaddr.sin_addr.s_addr
    );
    if (bind(this->fd, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) < 0) {
        throw IOException("receiver bind");
    }
}

ReceiverSocket::~ReceiverSocket()
{
    if (this->fd >= 0) {
        close(this->fd);
    }
}

size_t ReceiverSocket::receive(uint8_t *buffer, size_t capacity)
{
    ssize_t res = recv(this->fd, buffer, capacity, 0);
    if (res < 0) {
        throw IOException("recvfrom");
    }
    return res;
}

string ReceiverSocket::receive()
{
    string message(STR_BUF_SIZE, '\0');
    size_t read = this->receive((uint8_t *) message.data(), STR_BUF_SIZE);
    message.resize(read);
    return message;
}

bool ReceiverSocket::receive(Packet &packet)
{
    string message = this->receive();
    PacketDeserializer deserializer(message);
    return packet.deserialize(deserializer);
}

void ReceiverSocket::enable_broadcast(bool enable)
{
    int enabled = enable;
    int status = setsockopt(
        this->fd, SOL_SOCKET, SO_BROADCAST, &enabled, sizeof(enabled)
    );
    if (status < 0) {
        throw IOException("setsockopt for receiver broadcast");
    }
}

ServerSocket::ServerSocket(uint16_t port): udp(port)
{
}

ServerSocket::Request ServerSocket::receive()
{
    Packet packet;
    while (!this->udp.receive(packet)) { }
    return Request(packet);
}

void ServerSocket::enable_broadcast(bool enable)
{
    this->udp.enable_broadcast(enable);
}

ServerSocket::Request::Request(Packet received_packet__):
    received_packet_(received_packet__)
{
}

Packet ServerSocket::Request::received_packet() const
{
    return this->received_packet_;
}

void ServerSocket::Request::respond(uint16_t port)
{
    this->respond(this->received_packet_.body, port);
}

void ServerSocket::Request::respond(PacketBody packet_body, uint16_t port)
{
    Packet response_packet;
    response_packet.header = this->received_packet_.header;
    response_packet.header.direction = PacketHeader::RESPONSE;
    response_packet.header.sender_addresses = NodeAddresses::load_host();
    struct timespec timespec;
    clock_gettime(CLOCK_REALTIME, &timespec);
    response_packet.header.timestamp =
        timespec.tv_nsec / 1000 + timespec.tv_sec * 1000000;
    response_packet.body = packet_body;
    SenderSocket sender_socket;
    sender_socket.send(
        response_packet,
        this->received_packet_.header.sender_addresses.ip,
        port
    );
}
