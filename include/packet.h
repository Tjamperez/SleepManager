#ifndef PACKET_H_
#define PACKET_H_

#include <string>
#include <cstdint>
#include "../include/address.h"

using namespace std;

class PacketSerializer;
class PacketDeserializer;

class InvalidPacketException: public exception {
    private:
        string message;
    public:
        InvalidPacketException(string message_);
        virtual char const *what() const noexcept;
};

struct PacketHeader {
    enum Direction { 
        REQUEST = 0,
        RESPONSE = 1
    };

    uint64_t seqn;
    time_t timestamp;
    NodeAddresses sender_addresses;
    Direction direction;

    PacketHeader();

    bool parse_direction(uint64_t uint);

    void serialize(PacketSerializer& serializer) const;
    bool deserialize(PacketDeserializer& deserializer);
};

struct PacketBody {
    enum Type { 
        DISCOVERY = 0
    };

    Type type;

    PacketBody();

    bool parse_type(uint64_t uint);

    void serialize(PacketSerializer& serializer) const;
    bool deserialize(PacketDeserializer& deserializer);
};

struct Packet {
    PacketHeader header;
    PacketBody body;

    void serialize(PacketSerializer& serializer) const;
    bool deserialize(PacketDeserializer& deserializer);
};

class PacketSerializer {
    private:
        string serialized;
    public:
        void write(uint64_t uint);
        void write(string text);
        void write(bool boolean);
        void write(IpAddress ip_address);
        void write(MacAddress mac_address);
        string finish() const;
};

class PacketDeserializer {
    private:
        string serialized;
        size_t i;
        size_t probe_field_end();
    public:
        PacketDeserializer(string serialized);
        uint64_t read_uint();
        string read_string();
        bool read_bool();
        IpAddress read_ip_address();
        MacAddress read_mac_address();
};

#endif
