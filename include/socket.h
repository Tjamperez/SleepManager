#ifndef SOCKET_H_
#define SOCKET_H_

#include <optional>
#include <map>
#include "../include/address.h"
#include "../include/packet.h"

#define DEFAULT_PORT 8010
#define BROADCAST_PORT 9020

using namespace std;

class SenderSocket {
    private:
        int fd;
    public:
        SenderSocket();
        SenderSocket(SenderSocket const& obj) = delete;
        ~SenderSocket();

        SenderSocket &operator=(SenderSocket const& obj) = delete;

        void send(
            uint8_t const *buffer,
            size_t length,
            IpAddress dest_ip_address,
            uint16_t dest_port = DEFAULT_PORT
        );

        void send(
            string message,
            IpAddress dest_ip_address,
            uint16_t dest_port = DEFAULT_PORT
        );

        void send(
            Packet const& packet,
            IpAddress dest_ip_address,
            uint16_t dest_port = DEFAULT_PORT
        );

        void enable_broadcast(bool enable = true);
};

class ReceiverSocket {
    private:
        int fd;
    public:
        ReceiverSocket(uint16_t port);
        ReceiverSocket(ReceiverSocket const& obj) = delete;
        ~ReceiverSocket();

        ReceiverSocket &operator=(ReceiverSocket const& obj) = delete;

        size_t receive(uint8_t *buffer, size_t capacity);
        string receive();
        bool receive(Packet& packet);

        void enable_broadcast(bool enable = true);
};

class ServerSocket {
    public:
        class Request {
            friend ServerSocket;

            private:
                Packet received_packet_;
                Request(Packet received_packet__);
            public:
                Packet received_packet() const;
                void respond(uint16_t port = DEFAULT_PORT);
                void respond(
                    PacketBody packet_body,
                    uint16_t port = DEFAULT_PORT
                );
        };

    private:
        ReceiverSocket udp;

    public:
        ServerSocket(uint16_t port);
        Request receive();
        void enable_broadcast(bool enable = true);
};

#endif
