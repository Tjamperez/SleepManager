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
#include <cstring>
#include <ifaddrs.h>
#include <poll.h>

#define STR_BUF_SIZE 0xffff

static atomic<uint64_t> global_seqn = 0;

static NodeAddresses host_addresses(int socket, string const& interface);

UdpSenderSocket::UdpSenderSocket(): fd(-1)
{
    this->fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (this->fd < 0) {
        throw IOException("sender socket");
    }
}

UdpSenderSocket::~UdpSenderSocket()
{
    if (this->fd >= 0) {
        close(this->fd);
    }
}

void UdpSenderSocket::send(
    uint8_t const *buffer,
    size_t length,
    IpAddress dest_ip_address,
    uint16_t dest_port)
{
    struct sockaddr_in sockaddr;
    socklen_t socklen = sizeof(sockaddr);

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(dest_port);
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

void UdpSenderSocket::send(
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

void UdpSenderSocket::send(
    Packet const& packet,
    IpAddress dest_ip_address,
    uint16_t dest_port)
{
    PacketSerializer serializer;
    packet.serialize(serializer);
    string message = serializer.finish();
    this->send(message, dest_ip_address, dest_port);
}

void UdpSenderSocket::enable_broadcast(bool enable)
{
    int enabled = enable;
    int status = setsockopt(
        this->fd, SOL_SOCKET, SO_BROADCAST, &enabled, sizeof(enabled)
    );
    if (status < 0) {
        throw IOException("setsockopt for sender broadcast");
    }
}

NodeAddresses UdpSenderSocket::load_host_addresses(string const& interface)
{
    return host_addresses(this->fd, interface);
}

UdpReceiverSocket::UdpReceiverSocket(uint16_t port): fd(-1)
{
    IpAddress ip { 0, 0, 0, 0 };
    this->fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (this->fd < 0) {
        throw IOException("receiver socket");
    }
    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    copy(
        ip.begin(),
        ip.end(),
        (uint8_t *) &sockaddr.sin_addr.s_addr
    );
    if (bind(this->fd, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) < 0) {
        throw IOException("receiver bind");
    }
}

NodeAddresses UdpReceiverSocket::load_host_addresses(string const& interface)
{
    return host_addresses(this->fd, interface);
}

UdpReceiverSocket::~UdpReceiverSocket()
{
    if (this->fd >= 0) {
        close(this->fd);
    }
}

thread_local IpAddress UdpReceiverSocket::dummy_ip;

size_t UdpReceiverSocket::receive(
    uint8_t *buffer,
    size_t capacity,
    IpAddress& src_address)
{
    struct sockaddr_in sockaddr;
    socklen_t addrlen = sizeof(sockaddr);
    ssize_t res = recvfrom(
        this->fd,
        buffer,
        capacity,
        0,
        (struct sockaddr *) &sockaddr,
        &addrlen
    );
    if (res < 0) {
        throw IOException("recvfrom");
    }

    // unused
    uint16_t src_port_ = ntohs(sockaddr.sin_port);

    copy_n(
        (uint8_t *) &sockaddr.sin_addr.s_addr,
        src_address.size(),
        src_address.begin()
    );

    return res;
}

optional<size_t> UdpReceiverSocket::receive_timeout(
    uint8_t *buffer,
    size_t capacity,
    uint64_t wait_ms,
    IpAddress& src_address)
{
    struct pollfd fds[1];
    fds[0].fd = this->fd;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    if (poll(fds, sizeof(fds) / sizeof(fds[0]), wait_ms) < 0) {
        throw IOException("poll");
    }

    if ((fds[0].revents & POLLIN) == 0) {
        return optional<size_t>();
    }

    struct sockaddr_in sockaddr;
    socklen_t addrlen = sizeof(sockaddr);
    ssize_t res = recvfrom(
        this->fd,
        buffer,
        capacity,
        0,
        (struct sockaddr *) &sockaddr,
        &addrlen
    );

    // unused
    uint16_t src_port_ = ntohs(sockaddr.sin_port);

    copy_n(
        (uint8_t *) &sockaddr.sin_addr.s_addr,
        src_address.size(),
        src_address.begin()
    );

    return make_optional(res);
}

string UdpReceiverSocket::receive(IpAddress& src_address)
{
    string message(STR_BUF_SIZE, '\0');
    size_t read =
        this->receive((uint8_t *) message.data(), STR_BUF_SIZE, src_address);
    message.resize(read);
    return message;
}

optional<string> UdpReceiverSocket::receive_timeout(uint64_t wait_ms, IpAddress& src_address)
{
    string message(STR_BUF_SIZE, '\0');
    optional<size_t> read = this->receive_timeout(
        (uint8_t *) message.data(),
        STR_BUF_SIZE,
        wait_ms,
        src_address
    );
    if (read.has_value()) {
        message.resize(read.value());
        return make_optional(message);
    }
    return optional<string>();
}

bool UdpReceiverSocket::receive(Packet& packet)
{
    string message = this->receive();
    PacketDeserializer deserializer(message);
    return packet.deserialize(deserializer);
}

optional<bool> UdpReceiverSocket::receive_timeout(Packet& packet, uint64_t wait_ms)
{
    optional<string> message = this->receive_timeout(wait_ms);
    if (message.has_value()) {
        PacketDeserializer deserializer(message.value());
        return make_optional(packet.deserialize(deserializer));
    }
    return optional<bool>();
}

void UdpReceiverSocket::enable_broadcast(bool enable)
{
    int enabled = enable;
    int status = setsockopt(
        this->fd, SOL_SOCKET, SO_BROADCAST, &enabled, sizeof(enabled)
    );
    if (status < 0) {
        throw IOException("setsockopt for receiver broadcast");
    }
}

ServerSocket::ServerSocket(uint16_t port):
    ServerSocket(get_global_interface_name(), port)
{
}

ServerSocket::ServerSocket(string interface_name_, uint16_t port):
    udp(port),
    interface_name(interface_name_)
{
}

ServerSocket::Request ServerSocket::receive()
{
    Packet packet;
    while (!this->udp.receive(packet)) { }
    return Request(packet, this->interface_name);
}

optional<ServerSocket::Request> ServerSocket::receive_timeout(uint64_t wait_ms)
{
    Packet packet;
    if (this->udp.receive_timeout(packet, wait_ms)) {
        return make_optional(Request(packet, this->interface_name));
    }
    return optional<ServerSocket::Request>();
}

void ServerSocket::handle_wol(IpAddress dest_ip_address, uint16_t dest_port)
{
    MagicPacket request_packet;
    size_t read =
        this->udp.receive(request_packet.data(), request_packet.size());
    if (read != request_packet.size()) {
        throw InvalidPacketException("WOL packet must have length 6 * 17");
    }
    Packet response_packet;
    response_packet.header.seqn = global_seqn++;
    response_packet.header.direction = PacketHeader::RESPONSE;
    response_packet.header.sender_addresses = this->udp.load_host_addresses(this->interface_name);
    struct timespec timespec;
    clock_gettime(CLOCK_REALTIME, &timespec);
    response_packet.header.timestamp =
        timespec.tv_nsec / 1000 + timespec.tv_sec * 1000000;
    response_packet.body.type = PacketBody::WOL;
    UdpSenderSocket sender_socket;
    sender_socket.send(response_packet, dest_ip_address, dest_port);
}

void ServerSocket::enable_broadcast(bool enable)
{
    this->udp.enable_broadcast(enable);
}

ServerSocket::Request::Request(Packet received_packet__, string interface_name_):
    received_packet_(received_packet__),
    interface_name(interface_name_)
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
    UdpSenderSocket sender_socket;
    Packet response_packet;
    response_packet.header = this->received_packet_.header;
    response_packet.header.direction = PacketHeader::RESPONSE;
    response_packet.header.sender_addresses = sender_socket.load_host_addresses(this->interface_name);
    struct timespec timespec;
    clock_gettime(CLOCK_REALTIME, &timespec);
    response_packet.header.timestamp =
        timespec.tv_nsec / 1000 + timespec.tv_sec * 1000000;
    response_packet.body = packet_body;
    sender_socket.send(
        response_packet,
        this->received_packet_.header.sender_addresses.ip,
        port
    );
}

ClientSocket::ClientSocket():
    ClientSocket(get_global_interface_name())
{
}

ClientSocket::ClientSocket(string interface_name_):
    interface_name(interface_name_)
{
}

ClientSocket::Request ClientSocket::request(
    PacketBody packet_body,
    IpAddress dest_ip_address,
    uint16_t dest_port)
{
    Packet packet;
    packet.header.sender_addresses = this->udp.load_host_addresses(this->interface_name);
    packet.header.seqn = global_seqn++;
    struct timespec timespec;
    clock_gettime(CLOCK_REALTIME, &timespec);
    packet.header.timestamp =
        timespec.tv_nsec / 1000 + timespec.tv_sec * 1000000;
    packet.header.direction = PacketHeader::REQUEST;
    packet.body = packet_body;
    PacketSerializer serializer;
    packet.serialize(serializer);
    string message = serializer.finish();
    ClientSocket::Request request(message, dest_ip_address, dest_port);
    request.send();
    return request;
}

ClientSocket::Request ClientSocket::wol_request(
    MacAddress mac_address,
    IpAddress dest_ip_address,
    uint16_t dest_port)
{
    MagicPacket packet;
    for (size_t i = 0; i < 6; i++) {
        packet[i] = 0xff;
    }
    for (size_t i = 0; i < 16; i++) {
        for (size_t j = 0; j < mac_address.size(); j++) {
            packet[6 + i * 6 + j] = mac_address[j];
        }
    }
    string message((char *) packet.data(), packet.size());
    ClientSocket::Request request(message, dest_ip_address, dest_port);
    request.send();
    return request;
}

void ClientSocket::enable_broadcast(bool enable)
{
    this->udp.enable_broadcast(enable);
}

ClientSocket::Request::Request(
    string sent_packet_,
    IpAddress dest_ip_address__,
    uint16_t dest_port__):
        sent_packet(sent_packet_),
        dest_ip_address_(dest_ip_address__),
        dest_port_(dest_port__)
{
}

void ClientSocket::Request::send()
{
    UdpSenderSocket socket;
    if (this->dest_ip_address() == IpAddress { 255, 255, 255, 255 }) {
        socket.enable_broadcast();
    }
    socket.send(
        this->sent_packet,
        this->dest_ip_address(),
        this->dest_port()
    );
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
    UdpReceiverSocket socket(port);
    Packet received_packet;
    while (!done) {
        optional<bool> read = socket.receive_timeout(received_packet, try_wait_ms);
        if (read.has_value() && read.value()) {
            done = true;
        } else {
            this->send();
        }
    }
    return received_packet;
}

optional<Packet> ClientSocket::Request::receive_bounded(
    uint64_t retry_bound,
    uint16_t port,
    uint64_t try_wait_ms)
{
    UdpReceiverSocket socket(port);
    Packet received_packet;
    while (retry_bound > 0) {
        optional<bool> read = socket.receive_timeout(received_packet, try_wait_ms);
        if (read.has_value() && read.value()) {
            return make_optional(received_packet);
        } else {
            this->send();
            retry_bound--;
        }
    }
    return optional<Packet>();
}

static NodeAddresses host_addresses(int socket, string const& interface)
{
    NodeAddresses addresses;

    char hostname_c[HOST_NAME_MAX + 1] = {0};
    if (gethostname(hostname_c, HOST_NAME_MAX + 1) < 0) {
        throw IOException("gethostname");
    }
    addresses.hostname = hostname_c;

    struct ifreq ifreq;
    strcpy(ifreq.ifr_name, interface.c_str());
    if (ioctl(socket, SIOCGIFHWADDR, &ifreq) < 0) {
        throw IOException("SIOCGIFHWADDR");
    }
    copy_n((uint8_t *) &ifreq.ifr_hwaddr.sa_data, 6, addresses.mac.begin());
    if (ioctl(socket, SIOCGIFADDR, &ifreq) < 0) {
        throw IOException("SIOCGIFADDR");
    }
    copy_n(
        (uint8_t *) &((struct sockaddr_in *) &ifreq.ifr_addr)->sin_addr,
        4,
        addresses.ip.begin()
    );

    return addresses;
}
