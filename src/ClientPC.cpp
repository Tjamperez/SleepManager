#include "ClientPC.h"

NetworkPC::NetworkPC(std::string IP, std::string MAC)
{
    this->IP = IP;
    this->MAC = MAC;
}

NetworkPC::~NetworkPC()
{
    //dtor
}


