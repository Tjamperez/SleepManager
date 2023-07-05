#include "../include/participant.h"
#include "../include/interface.h"
//#include "../include/discoveryalt.h"
#include "../include/server.h"
void participant_main(void)
{
    //client_udp();
    
    Server_Connection server("");//Colocar o hostname hardcodded aqui por enquanto!!!!!!!
    server.start_server();
    list<Work_Station> participants; //Adding this just to test interface. Discovery subservice will either return a list of participants or set a global variable

    bool is_participant = true;
    Interface interface(is_participant,participants);
  
    interface.terminal();
}