#ifndef PARTICIPANT_H_
#define PARTICIPANT_H_

#define HOSTNAME_SIZE 10
#define MAC_ADRESS_SIZE 17
#define IP_ADRESS_SIZE 15
#define STATUS_SIZE 6

typedef struct{
    char hostname[HOSTNAME_SIZE];
    char mac_adress[MAC_ADRESS_SIZE];
    char ip_address[IP_ADRESS_SIZE];
    char status[STATUS_SIZE];
}Participant;

void participant_main(void);

#endif
