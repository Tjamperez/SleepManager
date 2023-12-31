#include "../include/monitoring.h"
#include "../include/socket.h"

#define MONITORING_TICK_US 50000 
#define MAX_RETRIES 3

void monitoring_main(shared_ptr<ManagementService> management_service)
{
    ClientSocket client_socket;
    ServerSocket server_socket(EXIT_PORT);

    while (true) {
        vector<tuple<MacAddress, ClientSocket::Request>> requests;
        for (auto participant : management_service->participant_list()) {
            PacketBody packet_body;
            packet_body.type = PacketBody::SLEEP_STATUS;
            IpAddress ip = participant->addresses().ip;
            MacAddress mac = participant->addresses().mac;
            requests.push_back(make_pair(
                mac,
                client_socket.request(packet_body, ip, SLEEP_STATUS_PORT)
            ));
        }

        for (auto mac_request_pair: requests) {
            auto mac = get<0>(mac_request_pair);
            auto request = get<1>(mac_request_pair);
            optional<Packet> maybe_packet
                = request.receive_bounded(MAX_RETRIES, SLEEP_STATUS_PORT);
            WorkStation::Status sleep_status;
            if (maybe_packet.has_value()) {
            } else {
                sleep_status = WorkStation::ASLEEP;
            }
            management_service->update_sleep_status(mac, sleep_status);
        }

        bool exit = false;
        while (!exit) {
            auto maybe_packet = server_socket.try_receive();
            if (maybe_packet.has_value()) {
                management_service->remove_by_mac_address(
                    maybe_packet.value().received_packet().header.sender_addresses.mac
                );
            } else {
                exit = true;
            }
        }

        struct timespec timespec;
        timespec.tv_sec = MONITORING_TICK_US / 1000000;
        timespec.tv_nsec = MONITORING_TICK_US * 1000;
        nanosleep(&timespec, nullptr);
    }
}
