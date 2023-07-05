#include "../include/manager.h"
#include "../include/interface.h"
//#include "../include/discoveryalt.h"
#include "../include/server.h"

void manager_main(void)
{
    //manager_udp();
    Server_Connection server("");//Colocar o hostname hardcodded aqui por enquanto!!!!!!!
    server.start_client();

    list<Work_Station> participants; //Adding this just to test interface. Discovery subservice will eiter return a list of participants or set a global variable

    bool is_participant = false;
    Interface interface(is_participant,participants);
    
    interface.terminal();
}
