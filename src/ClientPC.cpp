#include "ClientPC.h"

NetworkPC::NetworkPC()
{
    this->IP = "0.0.0.0";
    this->MAC = "0:0:0:0:0";
    this->status = STATUS_AWAKE;
}

NetworkPC::NetworkPC(const NetworkPC& t) {
    this->IP = t.IP;
    this->MAC = t.MAC;
    this->status = t.status;
}

NetworkPC::NetworkPC(std::string IP, std::string MAC)
{
    this->IP = IP;
    this->MAC = MAC;
    this->status = STATUS_AWAKE;
}

NetworkPC::~NetworkPC()
{
    //dtor
}


