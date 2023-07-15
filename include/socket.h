#ifndef SOCKET_H_
#define SOCKET_H_

#include <atomic>
#include <optional>
#include "../include/address.h"
#include "../include/packet.h"

#define DEFAULT_PORT 8010
#define BROADCAST_PORT 9020

#define DEFAULT_TRY_MS 100

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
        enum BlockType {
            BLOCKING,
            NON_BLOCKING
        };

        int fd;

        optional<size_t> receive_with_block_type(
            uint8_t *buffer,
            size_t capacity,
            BlockType block_type
        );

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

        /** Like .receive() but does not block. Returns an empty value if a
         * packet could not be received.
         */
        optional<size_t> try_receive(uint8_t *buffer, size_t capacity);

        /** Receives a serialized packet string. */
        string receive();

        /** Like .receive() but does not block. Returns an empty value if a
         * packet could not be received.
         */
        optional<string> try_receive();

        /** Receives a packet that is automatically deserialized. If the packet
         * is not recognized in the deserialization, this method returns false.
         */
        bool receive(Packet& packet);

        /** Like .receive() but does not block. Returns an empty value if a
         * packet could not be received.
         */
        optional<bool> try_receive(Packet& packet);

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

/** A socket used as a client. Use this to actively send requests and then
 * listen to the response.
 */
class ClientSocket {
    public:
        /** An active sent request handle created from request method. */
        class Request {
            friend ClientSocket;

            private:
                IpAddress dest_ip_address_;
                uint16_t dest_port_;
                Packet sent_packet_;
                Request(
                    IpAddress dest_ip_address__,
                    uint16_t dest_port__,
                    Packet sent_packet__
                );
                void send();

            public:
                /** Returns the packet sent in the request. */
                Packet sent_packet() const;

                /** Port to where the packet is sent. */
                IpAddress dest_ip_address() const;

                /** Port to where the packet is sent. */
                uint16_t dest_port() const;

                /** Receives the response possibly repeating the request */
                Packet receive_response(
                    uint16_t port = DEFAULT_PORT,
                    uint64_t try_wait_ms = DEFAULT_TRY_MS
                );
        };

    private:
        SenderSocket udp;

        static atomic<uint64_t> seqn;

    public:
        /** Performs a request to the given IP and port. */
        Request request(
            PacketBody packet_body,
            IpAddress dest_ip_address,
            uint16_t dest_port = DEFAULT_PORT
        );

        /** Enables or disables broadcast given the boolean argument. 
         * If no argument is given, then broadcast is enabled.
         */
        void enable_broadcast(bool enable = true);
};

#endif
