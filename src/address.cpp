#include "../include/address.h"
#include <iterator>
#include <sstream>

InvalidAddressException::InvalidAddressException(string message_):
    message(message_)
{
}

char const *InvalidAddressException::what() const noexcept
{
    return this->message.c_str();
}

istream& operator >>(istream& input_stream, MacAddress& address)
{
    bool first = true;
    ios_base::fmtflags fmtflags = input_stream.flags();
    input_stream >> hex >> noshowbase;
    for (auto it = address.end(); it != address.begin(); it--) {
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
        input_stream >> *(it - 1);
    }
    input_stream.flags(fmtflags);
    return input_stream;
}

istream& operator >>(istream& input_stream, IpAddress& address)
{
    bool first = true;
    ios_base::fmtflags fmtflags = input_stream.flags();
    input_stream >> dec >> noshowbase;
    for (auto it = address.end(); it != address.begin(); it--) {
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
        input_stream >> *(it - 1);
    }
    input_stream.flags(fmtflags);
    return input_stream;
}

ostream& operator <<(ostream& output_stream, MacAddress const& address)
{
    bool first = true;
    ios_base::fmtflags fmtflags = output_stream.flags();
    output_stream << hex << noshowbase;
    for (auto it = address.cend(); it != address.cbegin(); it--) {
        if (!first) {
            output_stream << ":";
        }
        first = false;
        output_stream << *(it - 1);
    }
    output_stream.flags(fmtflags);
    return output_stream;
}

ostream& operator <<(ostream& output_stream, IpAddress const& address)
{
    bool first = true;
    ios_base::fmtflags fmtflags = output_stream.flags();
    output_stream << dec << noshowbase;
    for (auto it = address.cend(); it != address.cbegin(); it--) {
        if (!first) {
            output_stream << ".";
        }
        first = false;
        output_stream << *(it - 1);
    }
    output_stream.flags(fmtflags);
    return output_stream;
}

MacAddress parse_mac_address(string const& text)
{
    istringstream input_stream(text);
    MacAddress address;
    input_stream >> address;
    if (input_stream.peek() != input_stream.eof()) {
        throw InvalidAddressException(string("expected end of input"));
    }
    return address;
}

IpAddress parse_ip_address(string const& text)
{
    istringstream input_stream(text);
    IpAddress address;
    input_stream >> address;
    if (input_stream.peek() != input_stream.eof()) {
        throw InvalidAddressException(string("expected end of input"));
    }
    return address;
}

string render_ip_address(IpAddress const& address)
{
    ostringstream output_stream;
    output_stream << address;
    return output_stream.str();
}

string render_mac_address(MacAddress const& address)
{
    ostringstream output_stream;
    output_stream << address;
    return output_stream.str();
}
