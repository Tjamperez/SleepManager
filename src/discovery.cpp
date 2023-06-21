#include "../include/discovery.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void discovery_main(void)
{
    /*
     * TODO: implement this function
     */
}


pthread_mutex_t activeListMutex;
char activeWorkstations[MAX_WORKSTATIONS][INET_ADDRSTRLEN];
int activeCount = 0;

void* sendDiscoveryMessage(void* arg) {
    int sock = *(int*)arg;
    struct sockaddr_in broadcastAddr;
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    broadcastAddr.sin_port = htons(PORT);

    while (1) {
        // Create a discovery message
        char message[] = "Discovery message";

        // Broadcast the message
        sendto(sock, message, sizeof(message), 0, (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));

        usleep(DISCOVERY_INTERVAL * 1000000);
    }

    return NULL;
}

void* receiveDiscoveryMessage(void* arg) {
    int sock = *(int*)arg;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    while (1) {
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));

        // Receive the incoming message
        recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &clientAddrLen);

        // Extract workstation information from the received message
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);

        // Add the workstation to the active list
        pthread_mutex_lock(&activeListMutex);
        if (activeCount < MAX_WORKSTATIONS) {
            strncpy(activeWorkstations[activeCount], clientIP, INET_ADDRSTRLEN);
            activeCount++;
        }
        pthread_mutex_unlock(&activeListMutex);
    }

    return NULL;
}