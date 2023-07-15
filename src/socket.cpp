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
    uint16_t dest_port)
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
    uint16_t dest_port)
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
    uint16_t dest_port)
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

optional<size_t> ReceiverSocket::receive_with_block_type(
    uint8_t *buffer,
    size_t capacity,
    ReceiverSocket::BlockType block_type)
{
    int flags;
    switch (block_type) {
        case ReceiverSocket::BLOCKING:
            flags = 0;
            break;
        case ReceiverSocket::NON_BLOCKING:
            flags = MSG_DONTWAIT;
            break;
    }
    ssize_t res = recv(this->fd, buffer, capacity, flags);
    if (res < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            return optional<size_t>();
        }
        throw IOException("recvfrom");
    }
    return make_optional(res);
}

size_t ReceiverSocket::receive(
    uint8_t *buffer,
    size_t capacity)
{
    return this->receive_with_block_type(
        buffer,
        capacity,
        ReceiverSocket::BLOCKING
    ).value();
}

optional<size_t> ReceiverSocket::try_receive(
    uint8_t *buffer,
    size_t capacity)
{
    return this->receive_with_block_type(
        buffer,
        capacity,
        ReceiverSocket::NON_BLOCKING
    );
}

string ReceiverSocket::receive()
{
    string message(STR_BUF_SIZE, '\0');
    size_t read =
        this->receive((uint8_t *) message.data(), STR_BUF_SIZE);
    message.resize(read);
    return message;
}

optional<string> ReceiverSocket::try_receive()
{
    string message(STR_BUF_SIZE, '\0');
    optional<size_t> read =
        this->try_receive((uint8_t *) message.data(), STR_BUF_SIZE);
    if (read.has_value()) {
        message.resize(read.value());
        return make_optional(message);
    }
    return optional<string>();
}

bool ReceiverSocket::receive(Packet& packet)
{
    string message = this->receive();
    PacketDeserializer deserializer(message);
    return packet.deserialize(deserializer);
}

optional<bool> ReceiverSocket::try_receive(Packet& packet)
{
    optional<string> message = this->try_receive();
    if (message.has_value()) {
        PacketDeserializer deserializer(message.value());
        return packet.deserialize(deserializer);
    }
    return optional<bool>();
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

atomic<uint64_t> ClientSocket::seqn = 0;

ClientSocket::Request ClientSocket::request(
    PacketBody packet_body,
    IpAddress dest_ip_address,
    uint16_t dest_port)
{
    Packet packet;
    packet.header.sender_addresses = NodeAddresses::load_host();
    packet.header.seqn = ClientSocket::seqn++;
    struct timespec timespec;
    clock_gettime(CLOCK_REALTIME, &timespec);
    packet.header.timestamp =
        timespec.tv_nsec / 1000 + timespec.tv_sec * 1000000;
    packet.header.direction = PacketHeader::REQUEST;
    packet.body = packet_body;
    ClientSocket::Request request(dest_ip_address, dest_port, packet);
    request.send();
    return request;
}

void ClientSocket::enable_broadcast(bool enable)
{
    this->udp.enable_broadcast(enable);
}

ClientSocket::Request::Request(
    IpAddress dest_ip_address__,
    uint16_t dest_port__,
    Packet sent_packet__):
        dest_ip_address_(dest_ip_address__),
        dest_port_(dest_port__),
        sent_packet_(sent_packet__)
{
}

void ClientSocket::Request::send()
{
    SenderSocket socket;
    socket.send(
        this->sent_packet(),
        this->dest_ip_address(),
        this->dest_port()
    );
}

Packet ClientSocket::Request::sent_packet() const
{
    return this->sent_packet_;
}

IpAddress ClientSocket::Request::dest_ip_address() const
{
    return this->dest_ip_address_;
}

uint16_t ClientSocket::Request::dest_port() const
{
    return this->dest_port_;
}

Packet ClientSocket::Request::receive_response(
    uint16_t port,
    uint64_t try_wait_ms)
{
    bool done = false;
    ReceiverSocket socket(port);
    Packet received_packet;
    while (!done) {
        optional<bool> read = socket.try_receive(received_packet);
        if (read.has_value() && read.value()) {
            done = true;
        } else {
            struct timespec timespec;
            timespec.tv_sec = try_wait_ms / 1000;
            timespec.tv_nsec = try_wait_ms * 1000000;
            nanosleep(&timespec, nullptr);
            this->send();
        }
    }
    return received_packet;
}
