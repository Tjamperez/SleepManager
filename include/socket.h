#ifndef SOCKET_H_
#define SOCKET_H_

#include <atomic>
#include <optional>
#include "../include/address.h"
#include "../include/packet.h"

#define WOL_PARTICIPANT_PORT 7010
#define WOL_MANAGER_PORT 7011
#define EXIT_PORT 8020
#define DISCOVERY_PARTICIPANT_PORT 9040
#define DISCOVERY_MANAGER_PORT 9041
#define SLEEP_STATUS_PORT 10030

#define DEFAULT_TRY_US 1000

using namespace std;

/** A raw UDP socket used only for sending packets. Do not use directly. */
class UdpSenderSocket {
    private:
        int fd;
    public:
        /** Creates an anonymous sender socket. */
        UdpSenderSocket();
        UdpSenderSocket(UdpSenderSocket const& obj) = delete;
        /** Closes the socket (destructor). */
        ~UdpSenderSocket();

        UdpSenderSocket &operator=(UdpSenderSocket const& obj) = delete;

        /** Sends a raw serialized packet. */
        void send(
            uint8_t const *buffer,
            size_t length,
            IpAddress dest_ip_address,
            uint16_t dest_port
        );

        /** Sends a serialized packet string. */
        void send(
            string message,
            IpAddress dest_ip_address,
            uint16_t dest_port
        );

        /** Sends a packet automatically serializing it. */
        void send(
            Packet const& packet,
            IpAddress dest_ip_address,
            uint16_t dest_port
        );

        /** Enables or disables broadcast given the boolean argument. 
         * If no argument is given, then broadcast is enabled.
         */
        void enable_broadcast(bool enable = true);
};

/** A raw UDP socket used only for receiving packets. Do not use directly. */
class UdpReceiverSocket {
    private:
        enum BlockType {
            BLOCKING,
            NON_BLOCKING
        };

        int fd;
        
        thread_local static IpAddress dummy_ip;

        optional<size_t> receive_with_block_type(
            uint8_t *buffer,
            size_t capacity,
            BlockType block_type,
            IpAddress& src_address
        );

    public:
        /** Creates a receiver socket that will listen to the given port in
         * the host's IP address.
         */
        UdpReceiverSocket(uint16_t port);
        UdpReceiverSocket(UdpReceiverSocket const& obj) = delete;
        /** Closes the socket (destructor). */
        ~UdpReceiverSocket();

        UdpReceiverSocket &operator=(UdpReceiverSocket const& obj) = delete;

        /** Receives a raw serialized packet into the given buffer. Returns the
         * packet size
         */
        size_t receive(
            uint8_t *buffer,
            size_t capacity,
            IpAddress& src_address = dummy_ip
        );

        /** Like .receive() but does not block. Returns an empty value if a
         * packet could not be received.
         */
        optional<size_t> try_receive(
            uint8_t *buffer,
            size_t capacity,
            IpAddress& src_address = dummy_ip
        );

        /** Receives a serialized packet string. */
        string receive(IpAddress& src_address = dummy_ip);

        /** Like .receive() but does not block. Returns an empty value if a
         * packet could not be received.
         */
        optional<string> try_receive(IpAddress& src_address = dummy_ip);

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
                void respond(uint16_t port);

                /** Responds to the request in the given port using the given
                 * packet body.
                 */
                void respond(
                    PacketBody packet_body,
                    uint16_t port
                );
        };

    private:
        UdpReceiverSocket udp;

    public:
        /** Creates a server socket listening to the given port in the host's IP
         * address.
         */
        ServerSocket(uint16_t port);

        /** Receives the next request. */
        Request receive();

        /** Receives the next request without blocking. */
        optional<Request> try_receive();

        /** Receives a Wake-On-LAN as the next request and sends a response. */
        void handle_wol(
            IpAddress dest_ip_address,
            uint16_t dest_port
        );

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
                string sent_packet;
                IpAddress dest_ip_address_;
                uint16_t dest_port_;
                Request(
                    string sent_packet_,
                    IpAddress dest_ip_address__,
                    uint16_t dest_port__
                );
                void send();

            public:
                /** Port to where the packet is sent. */
                IpAddress dest_ip_address() const;

                /** Port to where the packet is sent. */
                uint16_t dest_port() const;

                /** Receives the response possibly repeating the request.
                 */
                Packet receive_response(
                    uint16_t port, 
                    uint64_t try_wait_us = DEFAULT_TRY_US
                );

                /** Receives the response possibly repeating the request only
                 * the given retry bound.
                 */
                optional<Packet> receive_bounded(
                    uint64_t retry_bound,
                    uint16_t port,
                    uint64_t try_wait_us = DEFAULT_TRY_US
                );
        };

    private:
        UdpSenderSocket udp;

    public:
        /** Performs a request to the given IP and port. */
        Request request(
            PacketBody packet_body,
            IpAddress dest_ip_address,
            uint16_t dest_port
        );

        /** Performs a Wake-On-LAN request to the given MAC and IP and ports. */
        Request wol_request(
            MacAddress dest_mac_address,
            IpAddress dest_ip_address,
            uint16_t dest_port
        );

        /** Enables or disables broadcast given the boolean argument. 
         * If no argument is given, then broadcast is enabled.
         */
        void enable_broadcast(bool enable = true);
};

#endif
