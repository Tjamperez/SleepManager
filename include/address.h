#ifndef ADDRESS_H_
#define ADDRESS_H_

#include <array>
#include <cstdint>
#include <iostream>
#include <exception>

using namespace std;

/** MAC address in network order */
using MacAddress = array<uint8_t, 6>;

/** IPv4 address in network order */
using IpAddress = array<uint8_t, 4>;

/** Collection of addresses of a node in a network */
class NodeAddresses { 
    public:
        MacAddress mac;
        IpAddress ip;
        string hostname;
    private:
        void load_hostname();
        void load_ip();
        void load_mac();
    public:
        NodeAddresses();
        /** Loads address data of host running this process */
        static NodeAddresses load_host();
};

/** Exception thrown when parsing an invalid address */
class InvalidAddressException: public exception {
    private:
        string message;
    public:
        InvalidAddressException(string message_);
        virtual char const *what() const noexcept;
};

/** Exception thrown when no IP address is found for the hostname */
class NoIpAddressFoundException: public exception {
    private:
        string hostname_;
        string message;
    public:
        NoIpAddressFoundException(string hostname__);
        string const& hostname() const;
        virtual char const *what() const noexcept;
};

/** Exception thrown when no MAC address is found for the IP host */
class NoMacAddressFoundException: public exception {
    private:
        IpAddress ip_address_;
        string message;
    public:
        NoMacAddressFoundException(IpAddress ip_address__);
        IpAddress ip_address() const;
        virtual char const *what() const noexcept;
};

/** Reads and parses a MAC address from an input stream (including strings) */
istream& operator >>(istream& input_stream, MacAddress& address);

/** Reads and parses an IP address from an input stream (including strings) */
istream& operator >>(istream& input_stream, IpAddress& address);

/** Renders a MAC address into an output stream (including strings) */
ostream& operator <<(ostream& output_stream, MacAddress const& address);

/** Renders an IP address into an output stream (including strings) */
ostream& operator <<(ostream& output_stream, IpAddress const& address);

/** Parses a MAC address directly from a string */
MacAddress parse_mac_address(string const& text);

/** Parses an IP address directly from a string */
IpAddress parse_ip_address(string const& text);

/** Renders a MAC address directly into a string */
string render_mac_address(MacAddress const& address);

/** Renders an IP address directly into a string */
string render_ip_address(IpAddress const& address);

#endif
