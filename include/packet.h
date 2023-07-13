#ifndef PACKET_H_
#define PACKET_H_

#include <string>
#include <cstdint>
#include "../include/address.h"

using namespace std;

class InvalidPacketException: public exception {
    private:
        string message;
    public:
        InvalidPacketException(string message_);
        virtual char const *what() const noexcept;
};

//Messages exhanged between processes
struct Packet{
    uint16_t type; //Package type (p.ex. DATA | CMD)
    uint16_t seqn; //Sequence number
    uint16_t length; //Payload length
    uint16_t timestamp; // Data timestamp
    string payload;
    MacAddress mac_address;
    IpAddress ip_address;
    string hostname;
    bool is_manager;

    string serialize() const;
    static Packet deserialize(string message);
};

class PacketSerializer {
    private:
        string serialized;
    public:
        void write(uint16_t uint);
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
        uint16_t read_uint16();
        string read_string();
        bool read_bool();
        IpAddress read_ip_address();
        MacAddress read_mac_address();
};

#endif
