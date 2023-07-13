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

#define PACKET_BUF_SIZE 4096

UdpSocket::UdpSocket(IpAddress bind_to_ip): fd(-1)
{
    socket(AF_INET, SOCK_DGRAM, 0);
    if (this->fd < 0) {
        throw IOException("server socket");
    }
}

UdpSocket::~UdpSocket()
{
    if (this->fd >= 0) {
        close(this->fd);
    }
}

void UdpSocket::send(
    uint8_t const *buffer,
    size_t length,
    IpAddress dest_ip_address,
    uint16_t dest_port
) const
{
    struct sockaddr_in sockaddr;
    socklen_t socklen;

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = dest_port;
    copy_n(
        dest_ip_address.cbegin(),
        dest_ip_address.size(),
        (uint8_t *) &sockaddr.sin_addr.s_addr
    );

    size_t sent = 0;
    while (sent < length) {
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
        sent += res;
    }
}

void UdpSocket::send(
    string message,
    IpAddress dest_ip_address,
    uint16_t dest_port
) const
{
    this->send(
        (uint8_t *) message.c_str(),
        message.length(),
        dest_ip_address,
        dest_port
    );
}

void UdpSocket::send(
    Packet const& packet,
    IpAddress dest_ip_address,
    uint16_t dest_port
) const
{
    PacketSerializer serializer;
    packet.serialize(serializer);
    string message = serializer.finish();
    this->send(message, dest_ip_address, dest_port);
}

size_t UdpSocket::receive(
    uint8_t *buffer,
    size_t capacity,
    IpAddress dest_ip_address,
    uint16_t dest_port
) const
{
    ssize_t res = recvfrom(this->fd, buffer, capacity, 0, nullptr, nullptr);
    if (res < 0) {
        throw IOException("recvfrom");
    }
    return res;
}

void UdpSocket::receive(
    Packet &packet,
    IpAddress dest_ip_address,
    uint16_t dest_port
) const
{
    size_t buf_size = packet.max_recv_heuristics();
    std::string message(buf_size, '\0');
    size_t read = this->receive(
        (uint8_t *) message.data(), buf_size,
        dest_ip_address,
        dest_port
    );
    message.resize(read);
    PacketDeserializer deserializer(message);
    packet.deserialize(deserializer);
}

void UdpSocket::enable_broadcast() const
{
    int enabled = 1;
    int status = setsockopt(
        this->fd, SOL_SOCKET, SO_BROADCAST, &enabled, sizeof(enabled)
    );
    if (status < 0) {
        throw IOException("setsockopt for enabling broadcast");
    }
}

void UdpSocket::disable_broadcast() const
{
    int enabled = 0;
    int status = setsockopt(
        this->fd, SOL_SOCKET, SO_BROADCAST, &enabled, sizeof(enabled)
    );
    if (status < 0) {
        throw IOException("setsockopt for disabling broadcast");
    }
}
