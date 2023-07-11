#ifndef PARTICIPANT_H_
#define PARTICIPANT_H_

void participant_main(void);

class Participant {
public:
    std::array<uint8_t, 6> mac_address;
    std::array<uint8_t, 4> ip_address;

    Participant():
        mac_address(0),
        ip_address(0);

    Participant(
        std::array<uint8_t, 6> mac_address_,
        std::array<uint8_t, 4> ip_address_
    ):
        mac_address(mac_address_),
        ip_address(ip_address_);
};

#endif
