#include "../include/monitoring.h"
#include "../include/socket.h"

#define MONITORING_TICK_US 50000 
#define MAX_RETRIES 3

void monitoring_main(shared_ptr<ManagementService> management_service)
{
    ClientSocket client_socket;

    while (true) {
        vector<tuple<MacAddress, ClientSocket::Request>> requests;
        for (auto participant : management_service->participant_list()) {
            PacketBody packet_body;
            packet_body.type = PacketBody::SLEEP_STATUS;
            IpAddress ip = participant->addresses().ip;
            MacAddress mac = participant->addresses().mac;
            requests.push_back(make_pair(
                mac,
                client_socket.request(packet_body, ip)
            ));
        }

        for (auto mac_request_pair: requests) {
            auto mac = get<0>(mac_request_pair);
            auto request = get<1>(mac_request_pair);
            optional<Packet> maybe_packet
                = request.receive_bounded(MAX_RETRIES);
            WorkStation::Status sleep_status;
            if (maybe_packet.has_value()) {
            } else {
                sleep_status = WorkStation::ASLEEP;
            }
            management_service->update_sleep_status(mac, sleep_status);
        }

        struct timespec timespec;
        timespec.tv_sec = MONITORING_TICK_US / 1000000;
        timespec.tv_nsec = MONITORING_TICK_US * 1000;
        nanosleep(&timespec, nullptr);
    }
}
