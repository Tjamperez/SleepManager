#ifndef PACKET_H_
#define PACKET_H_

#include <array>
#include <string>
#include <cstdint>
#include "../include/address.h"

using namespace std;

class PacketSerializer;
class PacketDeserializer;

/** An unrecoverable error happened during a packet's parsing */
class InvalidPacketException: public exception {
    private:
        string message;
    public:
        InvalidPacketException(string message_);
        virtual char const *what() const noexcept;
};

using MagicPacket = array<uint8_t, 6 * 17>;

void make_magic_packet(MacAddress const &address, MagicPacket &dest);

/** The header of a packet, which can be read by a task, but not written.
 * A socket class however could write to it.
 */
struct PacketHeader {
    /** The direction of the packet: from the client (request) or from the
     * server (response)?
     */
    enum Direction { 
        REQUEST = 0,
        RESPONSE = 1
    };

    /** Serial sequence number. */
    uint64_t seqn;
    /** Timestamp of packet's creation. */
    uint64_t timestamp;
    /** Addresses of the sender. */
    NodeAddresses sender_addresses;
    /** Request or response? */
    Direction direction;

    PacketHeader();

    /** Tries to convert an integer to a direction, returns whether it succeeded
     */
    bool parse_direction(uint64_t uint);

    /** Serializes the packet header into the given serializer. */
    void serialize(PacketSerializer& serializer) const;
    /** Deserializes the packet header from the given deserializer. */
    bool deserialize(PacketDeserializer& deserializer);
};

/** The header of a packet, which can be read and written by a task.
 * A socket class should not care about it.
 */
struct PacketBody {
    /** An identtification of the packet's nature. 
     */
    enum Type { 
        DISCOVERY = 0,
        EXIT = 1,
        WOL = 2
    };

    /** Packet type. What is it for? What is being requested or responded? */
    Type type;

    PacketBody();

    /** Tries to convert an integer to a type, returns whether it succeeded */
    bool parse_type(uint64_t uint);

    /** Serializes the packet body into the given serializer. */
    void serialize(PacketSerializer& serializer) const;
    /** Deserializes the packet body from the given deserializer. */
    bool deserialize(PacketDeserializer& deserializer);
};

/** An unserialized packet's data, more specifically, packet data used by this
 * application.
 */
struct Packet {
    PacketHeader header;
    PacketBody body;

    /** Serializes the packet into the given serializer. */
    void serialize(PacketSerializer& serializer) const;
    /** Deserializes the packet from the given deserializer. */
    bool deserialize(PacketDeserializer& deserializer);
};

/** A helper to serialize a packet. */
class PacketSerializer {
    private:
        string serialized;
    public:
        /** Renders an integer as decimal. */
        void write(uint64_t uint);
        /** Renders plaintext escaping control characters . */
        void write(string text);
        /** Renders a boolean. */
        void write(bool boolean);
        /** Renders an IP address in the format A.B.C.D */
        void write(IpAddress ip_address);
        /** Renders a MAC address in the format A:B:C:D:E:F */
        void write(MacAddress mac_address);
        /** Produces the serialized string. */
        string finish() const;
};

/** A helper to deserialize a packet. */
class PacketDeserializer {
    private:
        string serialized;
        size_t i;
        size_t probe_field_end();
    public:
        /** Creates the deserializer frm the given serialized string. */
        PacketDeserializer(string serialized);
        /** Parses an integer from decimal. */
        uint64_t read_uint();
        /** Parses plaintext de-escaping control characters . */
        string read_string();
        /** Parses a boolean. */
        bool read_bool();
        /** Parses an IP address from the format A.B.C.D */
        IpAddress read_ip_address();
        /** Parses a MAC address from the format A:B:C:D:E:F */
        MacAddress read_mac_address();
};

#endif
