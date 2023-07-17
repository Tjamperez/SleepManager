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

#define TIMEOUT_MS 5
#define BUFFER_SIZE 512
#define PACKET_SIZE 132

enum packetTypes {PTYPE_NULL, PTYPE_DISCOVERY, PTYPE_DISCOVERY_ACK, PTYPE_SSR, PTYPE_SSR_RESP, PTYPE_SERVER_SHUTDOWN};
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

    static bool sendBroadcast(int sockfd, const struct sockaddr_in &server_addr, basePacket p);
    static basePacket waitForResponse(int sockfd, struct sockaddr_in server_addr,long timeOutMs);
    static bool initializeSocket(int &sockfd, const struct sockaddr_in &server_addr);
    static char* serializePacket(basePacket p);
    static basePacket deserializePacket(char* serializedData);
    protected:

    private:
};

#endif // WEBSERVICES_H
