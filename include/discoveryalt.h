#ifndef DISCOVERY_H_
#define DISCOVERY_H_

#include <pthread.h>

#define MAX_WORKSTATIONS 10
#define INET_ADDRSTRLEN 16
#define PORT 4000
#define DISCOVERY_INTERVAL 5

int client_udp(int argc, char *argv[]);
int manager_udp(int argc, char *argv[]);
int hasTimeoutPassed(clock_t start, int timeoutInMicroseconds);
void discovery_main(void);


#endif