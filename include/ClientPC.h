#ifndef CLIENTPC_H
#define CLIENTPC_H

#include <vector>
#include <string>
#include <mutex>

enum pcStatus {STATUS_SLEEPING, STATUS_AWAKE};

class NetworkPC
{
    public:
        NetworkPC(std::string IP, std::string MAC);
        NetworkPC(NetworkPC &t);
        virtual ~NetworkPC();

        std::string getIP(){return IP;}
        std::string getMAC(){return MAC;}
        pcStatus getStatus(){return status;}
        void setStatus(pcStatus status){this->status = status;}

        bool testAwake();
        bool awaken();

    protected:
        std::string IP;
        std::string MAC;

        pcStatus status = STATUS_AWAKE;

    private:
};

#endif // CLIENTPC_H
