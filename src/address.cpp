#include "../include/address.h"
#include <reverse_iterator>
#include <sstream>

InvalidAddressException::InvalidAddressException(string message_):
    message(message_)
{
}

virtual char const *InvalidAddressException::what() const noexcept
{
    return this->message.c_str();
}

istream& operator >>(istream& input_stream, MacAddress& address)
{
    bool first = true;
    ios_base::fmtflags fmtflags = input_stream.flags();
    input_stream << hex << noshowbase;
    for (uint8_t& byte : make_reverse_iterator(address)) {
        if (!first) {
            char ch;
            input_stream >> ch;
            if (ch != ':') {
                throw InvalidAddressException(string(
                    "expected `:` in MAC address"
                ));
            }
        }
        first = false;
        input_stream >> byte;
    }
    input_stream.flags(fmtflags);
    return input_stream;
}

istream& operator >>(istream& input_stream, IpAddress& address)
{
    bool first = true;
    ios_base::fmtflags fmtflags = input_stream.flags();
    input_stream << dec << noshowbase;
    for (uint8_t& byte : make_reverse_iterator(address)) {
        if (!first) {
            char ch;
            input_stream >> ch;
            if (ch != '.') {
                throw InvalidAddressException(string(
                    "expected `.` in IPV4 address"
                ));
            }
        }
        first = false;
        input_stream >> byte;
    }
    input_stream.flags(fmtflags);
    return input_stream;
}

ostream& operator <<(ostream& output_stream, MacAddress const& address)
{
    bool first = true;
    ios_base::fmtflags fmtflags = output_stream.flags();
    output_stream << hex << noshowbase;
    for (uint8_t const& byte : make_reverse_iterator(address)) {
        if (!first) {
            output_stream << ":";
        }
        first = false;
        output_stream << byte;
    }
    output_stream.flags(fmtflags);
    return output_stream;
}

ostream& operator <<(ostream& output_stream, IpAddress const& address)
{
    bool first = true;
    ios_base::fmtflags fmtflags = output_stream.flags();
    output_stream << dec << noshowbase;
    for (uint8_t const& byte : make_reverse_iterator(address)) {
        if (!first) {
            output_stream << ".";
        }
        first = false;
        output_stream << byte;
    }
    output_stream.flags(fmtflags);
    return output_stream;
}

MacAddress parse_mac_address(string const& text)
{
}

IpAddress parse_ip_address(string const& text)
{
}

string render_ip_address(IpAddress const& address)
{
}

string render_mac_address(MacAddress const& address)
{
}
