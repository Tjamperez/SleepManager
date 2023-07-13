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

Packet::Packet():
    type(Packet::DISCOVERY_REQ),
    seqn(0),
    timestamp(0)
{
}

void Packet::serialize(PacketSerializer& serializer) const
{
    serializer.write((uint64_t) this->type);
    serializer.write(this->sender_addresses.mac);
    serializer.write(this->sender_addresses.ip);
    serializer.write(this->sender_addresses.hostname);
    serializer.write((uint64_t) this->seqn);
    serializer.write((uint64_t) (int64_t) this->timestamp);
}

bool Packet::deserialize(PacketDeserializer& deserializer)
{
    uint8_t type = deserializer.read_uint();
    switch (type) {
        case Type::DISCOVERY_REQ:
            this->type = Type::DISCOVERY_REQ;
            break;
        case Type::DISCOVERY_RESP:
            this->type = Type::DISCOVERY_RESP;
            break;
        default:
            return false;
    }
    this->sender_addresses.mac = deserializer.read_mac_address();
    this->sender_addresses.ip = deserializer.read_ip_address();
    this->sender_addresses.hostname = deserializer.read_string();
    this->seqn = deserializer.read_uint();
    this->timestamp = (int64_t) deserializer.read_uint();

    return true;
}

size_t Packet::max_recv_heuristics() const
{
    return 4096;
}

void PacketSerializer::write(uint64_t uint)
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

uint64_t PacketDeserializer::read_uint()
{
    size_t j = this->probe_field_end();
    uint16_t uint = stoull(this->serialized.substr(this->i, j - this->i));
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
