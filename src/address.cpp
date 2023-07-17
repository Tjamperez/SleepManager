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
#include <cstring>

NodeAddresses::NodeAddresses(): mac {0}, ip {0}
{
}

void NodeAddresses::load_hostname()
{
    char hostname_c[HOST_NAME_MAX + 1] = {0};
    if (gethostname(hostname_c, HOST_NAME_MAX + 1) < 0) {
        throw IOException("gethostname");
    }
    this->hostname = hostname_c;
}

void NodeAddresses::load_ip()
{
    struct addrinfo addrinfo_hints = {0};
    struct addrinfo *addr_list;
    if (
        getaddrinfo(
            this->hostname.c_str(),
            nullptr,
            &addrinfo_hints,
            &addr_list)
        < 0
    ) {
        throw IOException("getaddrinfo for finding ipv4");
    }
    struct addrinfo *addr_curr = addr_list;
    bool found_ipv4 = false;
    while (addr_curr != nullptr && !found_ipv4) {
        if (addr_curr->ai_family == AF_INET) {
            copy_n(
                &addr_curr->ai_addr->sa_data[2],
                4,
                this->ip.begin()
            );
            found_ipv4 = true;
        }
        addr_curr = addr_curr->ai_next;
    }
    freeaddrinfo(addr_list);

    if (!found_ipv4) {
        throw NoIpAddressFoundException(this->hostname);
    }
}

void NodeAddresses::load_mac()
{
    struct ifaddrs *ifaddr_list;
    getifaddrs(&ifaddr_list);
    struct ifaddrs *ifaddr_curr = ifaddr_list;
    bool found_mac = false;
    while (ifaddr_curr != nullptr && !found_mac) {
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        struct ifreq ifreq;
        if (fd < 0) {
            throw IOException("socket for finding MAC");
        }
        strcpy(ifreq.ifr_name, ifaddr_curr->ifa_name);
        if (ioctl(fd, SIOCGIFADDR, &ifreq) >= 0) {
            struct sockaddr_in *inet_addr =
                (struct sockaddr_in *) &ifreq.ifr_addr;
            IpAddress if_ip_addr;
            copy_n(
                (uint8_t *) &inet_addr->sin_addr.s_addr,
                4,
                if_ip_addr.begin()
            );
            if (if_ip_addr == this->ip) {
                if (ioctl(fd, SIOCGIFHWADDR, &ifreq) < 0) {
                    throw IOException("get MAC address");
                }
                copy_n(
                    (uint8_t *) &ifreq.ifr_hwaddr.sa_data,
                    6,
                    this->mac.begin()
                );
                found_mac = true;
            }
        }
        ifaddr_curr = ifaddr_curr->ifa_next;
    }
    freeifaddrs(ifaddr_list);

    if (!found_mac) {
        throw NoMacAddressFoundException(this->ip);
    }
}

NodeAddresses NodeAddresses::load_host()
{
    NodeAddresses addresses;
    addresses.load_hostname();
    addresses.load_ip();
    addresses.load_mac();
    return addresses;
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
