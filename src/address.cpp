#include "../include/address.h"
#include "../include/io_exception.h"
#include <iterator>
#include <sstream>
#include <iomanip>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <net/if.h>
#include <limits.h>
#include <algorithm>
#include <ifaddrs.h>
#include <shared_mutex>
#include <cstring>

static string global_interface_name;

static shared_mutex global_interface_name_mutex;

NodeAddresses::NodeAddresses(): mac {0}, ip {0}
{
}

InvalidAddressException::InvalidAddressException(string message_):
    message(message_)
{
}

char const *InvalidAddressException::what() const noexcept
{
    return this->message.c_str();
}

NoIpAddressFoundException::NoIpAddressFoundException(string hostname__):
    hostname_(hostname__),
    message("no matching IP address found for hostname ")
{
    this->message += this->hostname();
}

string const& NoIpAddressFoundException::hostname() const
{
    return this->hostname_;
}

char const *NoIpAddressFoundException::what() const noexcept
{
    return this->message.c_str();
}

NoMacAddressFoundException::NoMacAddressFoundException(IpAddress ip_address__):
    ip_address_(ip_address__),
    message("no matching MAC address found for IP ")
{
    this->message += render_ip_address(this->ip_address());
}

IpAddress NoMacAddressFoundException::ip_address() const
{
    return this->ip_address_;
}

char const *NoMacAddressFoundException::what() const noexcept
{
    return this->message.c_str();
}

string get_global_interface_name()
{
    shared_lock lock(global_interface_name_mutex);
    return global_interface_name;
}

void set_global_interface_name(string interface_name)
{
    lock_guard lock(global_interface_name_mutex);
    global_interface_name = interface_name;
}

istream& operator >>(istream& input_stream, MacAddress& address)
{
    bool first = true;
    ios_base::fmtflags fmtflags = input_stream.flags();
    input_stream >> hex >> noshowbase;
    for (auto it = address.begin(); it != address.end(); it++) {
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
        unsigned byte;
        input_stream >> byte;
        *it = byte;
    }
    input_stream.flags(fmtflags);
    return input_stream;
}

istream& operator >>(istream& input_stream, IpAddress& address)
{
    bool first = true;
    ios_base::fmtflags fmtflags = input_stream.flags();
    input_stream >> dec >> noshowbase;
    for (auto it = address.begin(); it != address.end(); it++) {
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
        unsigned byte;
        input_stream >> byte;
        *it = byte;
    }
    input_stream.flags(fmtflags);
    return input_stream;
}

ostream& operator <<(ostream& output_stream, MacAddress const& address)
{
    bool first = true;
    ios_base::fmtflags fmtflags = output_stream.flags();
    output_stream << hex << noshowbase;
    for (auto it = address.cbegin(); it != address.cend(); it++) {
        if (!first) {
            output_stream << ":";
        }
        first = false;
        output_stream << setfill('0') << setw(2) << (unsigned) *it;
    }
    output_stream.flags(fmtflags);
    return output_stream;
}

ostream& operator <<(ostream& output_stream, IpAddress const& address)
{
    bool first = true;
    ios_base::fmtflags fmtflags = output_stream.flags();
    output_stream << dec << noshowbase;
    for (auto it = address.cbegin(); it != address.cend(); it++) {
        if (!first) {
            output_stream << ".";
        }
        first = false;
        output_stream << (unsigned) *it;
    }
    output_stream.flags(fmtflags);
    return output_stream;
}

MacAddress parse_mac_address(string const& text)
{
    istringstream input_stream(text);
    MacAddress address;
    input_stream >> address;
    if (!input_stream.eof()) {
        throw InvalidAddressException(string("expected end of input"));
    }
    return address;
}

IpAddress parse_ip_address(string const& text)
{
    istringstream input_stream(text);
    IpAddress address;
    input_stream >> address;
    if (!input_stream.eof()) {
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
