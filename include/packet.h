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

// Base type for messages exhanged between processes.
//
// Inherit this class to extend package size and behaviour.
class Packet {
public:
    enum Type {
        DISCOVERY_REQ = 0,
        DISCOVERY_RESP = 1
    };

    Type type;
    NodeAddresses sender_addresses;
    uint64_t seqn;
    time_t timestamp;

    Packet();

    virtual void serialize(PacketSerializer& serializer) const;
    virtual bool deserialize(PacketDeserializer& deserializer);
    virtual size_t max_recv_heuristics() const;
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
