#ifndef DISCOVERY_H_
#define DISCOVERY_H_

#include <pthread.h>

#define MAX_WORKSTATIONS 10
#define INET_ADDRSTRLEN 16
#define PORT 12345
#define DISCOVERY_INTERVAL 5

void* sendDiscoveryMessage(void* arg);
void* receiveDiscoveryMessage(void* arg);

void discovery_main(void);


#endif