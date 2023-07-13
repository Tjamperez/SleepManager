#ifndef ADDRESS_H_
#define ADDRESS_H_

#include <array>
#include <cstdint>
#include <iostream>
#include <exception>

using namespace std;

using MacAddress = array<uint8_t, 6>;

using IpAddress = array<uint8_t, 4>;

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
        static NodeAddresses localhost();
};

class InvalidAddressException: public exception {
    private:
        string message;
    public:
        InvalidAddressException(string message_);
        virtual char const *what() const noexcept;
};

class NoIpAddressFoundException: public exception {
    private:
        string hostname_;
        string message;
    public:
        NoIpAddressFoundException(string hostname__);
        string const& hostname() const;
        virtual char const *what() const noexcept;
};

class NoMacAddressFoundException: public exception {
    private:
        IpAddress ip_address_;
        string message;
    public:
        NoMacAddressFoundException(IpAddress ip_address__);
        IpAddress ip_address() const;
        virtual char const *what() const noexcept;
};

istream& operator >>(istream& input_stream, MacAddress& address);

istream& operator >>(istream& input_stream, IpAddress& address);

ostream& operator <<(ostream& output_stream, MacAddress const& address);

ostream& operator <<(ostream& output_stream, IpAddress const& address);

MacAddress parse_mac_address(string const& text);

IpAddress parse_ip_address(string const& text);

string render_ip_address(IpAddress const& address);

string render_mac_address(MacAddress const& address);

#endif
