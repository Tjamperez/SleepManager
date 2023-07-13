#include "../include/packet.h"
#include <sstream>

#define FIELD_END ';'

InvalidPacketException::InvalidPacketException(string message_):
    message(message_)
{
}

char const *InvalidPacketException::what() const noexcept
{
    return this->message.c_str();
}

string Packet::serialize() const
{
    PacketSerializer serializer;
    serializer.write(this->seqn);
    serializer.write(this->length);
    serializer.write(this->timestamp);
    serializer.write(this->payload);
    serializer.write(this->ip_address);
    serializer.write(this->mac_address);
    serializer.write(this->hostname);
    serializer.write(this->is_manager);
    return serializer.finish();
}

Packet Packet::deserialize(string message)
{
    PacketDeserializer deserializer(message);
    Packet packet;
    packet.seqn = deserializer.read_uint16();
    packet.length = deserializer.read_uint16();
    packet.timestamp = deserializer.read_uint16();
    packet.payload = deserializer.read_string();
    packet.ip_address = deserializer.read_ip_address();
    packet.mac_address = deserializer.read_mac_address();
    packet.hostname = deserializer.read_string();
    packet.is_manager = deserializer.read_bool();
    return packet;
}

void PacketSerializer::write(uint16_t uint)
{
    this->serialized += to_string(uint);
    this->serialized += FIELD_END;
}

void PacketSerializer::write(string text)
{
    bool escape = false;
    for (char ch : text) {
        if (escape) {
            this->serialized += ch;
            escape = false;
        } else if (ch == '\\') {
            escape = true;
        }
    }
    this->serialized += FIELD_END;
}

void PacketSerializer::write(bool boolean)
{
    this->serialized += to_string(boolean);
    this->serialized += FIELD_END;
}

void PacketSerializer::write(IpAddress ip_address)
{
    this->serialized += render_ip_address(ip_address);
    this->serialized += FIELD_END;
}

void PacketSerializer::write(MacAddress mac_address)
{
    this->serialized += render_mac_address(mac_address);
    this->serialized += FIELD_END;
}

string PacketSerializer::finish() const
{
    return this->serialized;
}

PacketDeserializer::PacketDeserializer(string serialized):
    serialized(serialized),
    i(0)
{
}

size_t PacketDeserializer::probe_field_end()
{
    size_t j = i;
    bool escape = false;
    while (
        j < this->serialized.length()
        && (this->serialized[j] != FIELD_END || escape)
    ) {
        escape = this->serialized[j] == '\\';
        j++;
    }
    if (j >= this->serialized.length()) {
        throw InvalidPacketException("missing field end ;");
    }
    return j;
}

uint16_t PacketDeserializer::read_uint16()
{
    size_t j = this->probe_field_end();
    uint16_t uint = stoi(this->serialized.substr(this->i, j - this->i));
    this->i = j + 1;
    return uint;
}

string PacketDeserializer::read_string()
{
    size_t j = this->probe_field_end();
    string text = this->serialized.substr(this->i, j - this->i);
    this->i = j + 1;
    return text;
}

bool PacketDeserializer::read_bool()
{
    size_t j = this->probe_field_end();
    string substr = this->serialized.substr(this->i, j - this->i);
    bool value;
    if (substr == "false") {
        this->i = j + 1;
        value = false;
    } else if (substr == "true") {
        value = true;
    } else {
        throw new InvalidPacketException("invalid boolean field");
    }
    this->i = j + 1;
    return value;
}

IpAddress PacketDeserializer::read_ip_address()
{
    string str = this->read_string();
    return parse_ip_address(str);
}

MacAddress PacketDeserializer::read_mac_address()
{
    string str = this->read_string();
    return parse_mac_address(str);
}
