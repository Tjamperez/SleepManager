#ifndef ADDRESS_H_
#define ADDRESS_H_

#include <array>
#include <cstdint>
#include <iostream>
#include <exception>

using namespace std;

using MacAddress = array<uint8_t, 6>;

using IpAddress = array<uint8_t, 4>;

class InvalidAddressException: public exception {
    private:
        string message;
    public:
        InvalidAddressException(string message_);
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
