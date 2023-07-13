#ifndef SOCKET_H_
#define SOCKET_H_

#include <optional>
#include "../include/address.h"
#include "../include/packet.h"

#define DEFAULT_PORT 8010

using namespace std;

class UdpSocket {
    private:
        int fd;

    public:
        UdpSocket(IpAddress bind_to_ip, uint16_t bind_to_port);
        UdpSocket(UdpSocket const& obj) = delete;
        ~UdpSocket();

        UdpSocket &operator=(UdpSocket const& obj) = delete;

        void send(
            uint8_t const *buffer,
            size_t length,
            IpAddress dest_ip_address,
            uint16_t dest_port = DEFAULT_PORT
        ) const;

        void send(
            string message,
            IpAddress dest_ip_address,
            uint16_t dest_port = DEFAULT_PORT
        ) const;

        void send(
            Packet packet,
            IpAddress dest_ip_address,
            uint16_t dest_port = DEFAULT_PORT
        ) const;

        size_t receive(
            uint8_t *buffer,
            size_t capacity,
            IpAddress dest_ip_address,
            uint16_t dest_port = DEFAULT_PORT
        ) const;

        Packet receive(
            IpAddress dest_ip_address,
            uint16_t dest_port = DEFAULT_PORT
        ) const;
};

#endif
