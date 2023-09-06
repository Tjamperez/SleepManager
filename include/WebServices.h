#ifndef WEBSERVICES_H
#define WEBSERVICES_H

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include "ClientPC.h"
#include "WebServices.h"
#include <stdint.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <array>
#include <algorithm>
#include <sstream>
#include <iomanip>

#define TIMEOUT_MS 5
#define BUFFER_SIZE 512
#define PACKET_SIZE 132
#define SERVER_PORT 35000
#define MONITOR_PORT 35005
#define CLIENT_MONITOR_PORT 35008
#define REPLICATION_PORT 35011

enum packetTypes {PTYPE_NULL, PTYPE_DISCOVERY, PTYPE_DISCOVERY_ACK, PTYPE_MONITOR_PROBE, PTYPE_MONITORING_PROBE_RESP, PTYPE_SERVER_SHUTDOWN,PTYPE_SERVER_PROBE,PTYPE_SERVER_PROBE_RESP, PTYPE_LIST_SIZE, PTYPE_LIST_ELEMENT, PTYPE_ELEMENT_REQUEST, PTYPE_LIST_END};
extern std::vector<std::string> packetTypesNames;
 typedef struct __packet{
 uint16_t type; //Tipo do pacote
 uint16_t timestamp; // Timestamp do dado
 char _payload[128]; //Dados da mensagem
 } basePacket;

 //packet

class WebServices
{
    public:
        WebServices();
        virtual ~WebServices();
        static struct sockaddr_in server_addr;

    static std::string networkInterface;
    static bool sendBroadcast(int sockfd, const struct sockaddr_in &server_addr, basePacket p);
    static basePacket waitForResponse(int sockfd, struct sockaddr_in server_addr,long timeOutMs);
    static bool initializeSocket(int &sockfd, const struct sockaddr_in &server_addr);
    static char* serializePacket(basePacket p);
    static basePacket deserializePacket(char* serializedData);
    static std::string getMACAddress();
    protected:

    private:
};

#endif // WEBSERVICES_H
