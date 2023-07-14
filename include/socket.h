#ifndef SOCKET_H_
#define SOCKET_H_

#include <optional>
#include "../include/address.h"
#include "../include/packet.h"

#define DEFAULT_PORT 8010
#define BROADCAST_PORT 9020

using namespace std;

/** A raw socket used only for sending packets. Do not use directly. */
class SenderSocket {
    private:
        int fd;
    public:
        /** Creates an anonymous sender socket. */
        SenderSocket();
        SenderSocket(SenderSocket const& obj) = delete;
        /** Closes the socket (destructor). */
        ~SenderSocket();

        SenderSocket &operator=(SenderSocket const& obj) = delete;

        /** Sends a raw serialized packet. */
        void send(
            uint8_t const *buffer,
            size_t length,
            IpAddress dest_ip_address,
            uint16_t dest_port = DEFAULT_PORT
        );

        /** Sends a serialized packet string. */
        void send(
            string message,
            IpAddress dest_ip_address,
            uint16_t dest_port = DEFAULT_PORT
        );

        /** Sends a packet automatically serializing it. */
        void send(
            Packet const& packet,
            IpAddress dest_ip_address,
            uint16_t dest_port = DEFAULT_PORT
        );

        /** Enables or disables broadcast given the boolean argument. 
         * If no argument is given, then broadcast is enabled.
         */
        void enable_broadcast(bool enable = true);
};

/** A raw socket used only for receiving packets. Do not use directly. */
class ReceiverSocket {
    private:
        int fd;
    public:
        /** Creates a receiver socket that will listen to the given port in
         * the host's IP address.
         */
        ReceiverSocket(uint16_t port);
        ReceiverSocket(ReceiverSocket const& obj) = delete;
        /** Closes the socket (destructor). */
        ~ReceiverSocket();

        ReceiverSocket &operator=(ReceiverSocket const& obj) = delete;

        /** Receives a raw serialized packet into the given buffer. Returns the
         * packet size
         */
        size_t receive(uint8_t *buffer, size_t capacity);

        /** Receives a serialized packet string. */
        string receive();

        /** Receives a packet that is automatically deserialized. If the packet
         * is not recognized in the deserialization, this method returns false.
         */
        bool receive(Packet& packet);

        /** Enables or disables broadcast given the boolean argument. 
         * If no argument is given, then broadcast is enabled.
         */
        void enable_broadcast(bool enable = true);
};

/** A socket used as a server. Use this to passively listen to requests and then
 * respond to them.
 */
class ServerSocket {
    public:
        /** A received request created from receive. */
        class Request {
            friend ServerSocket;

            private:
                Packet received_packet_;
                Request(Packet received_packet__);

            public:
                /** Returns the received packet. */
                Packet received_packet() const;

                /** Responds to the request in the given port using the same
                 * packet body.
                 */
                void respond(uint16_t port = DEFAULT_PORT);

                /** Responds to the request in the given port using the given
                 * packet body.
                 */
                void respond(
                    PacketBody packet_body,
                    uint16_t port = DEFAULT_PORT
                );
        };

    private:
        ReceiverSocket udp;

    public:
        /** Creates a server socket listening to the given port in the host's IP
         * address.
         */
        ServerSocket(uint16_t port);

        /** Receives the next request. */
        Request receive();

        /** Enables or disables broadcast given the boolean argument. 
         * If no argument is given, then broadcast is enabled.
         */
        void enable_broadcast(bool enable = true);
};

#endif
