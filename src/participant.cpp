#include "../include/participant.h"
#include "../include/interface.h"
#include "../include/server.h"

void participant_main(void)
{
    Server_Connection server;
    server.start_client();
    
    list<Work_Station> participants; //Adding this just to test interface. Discovery subservice will either return a list of participants or set a global variable

    bool is_participant = true;
    Interface interface(is_participant,participants);
  
    interface.terminal();
}