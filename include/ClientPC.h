#ifndef CLIENTPC_H
#define CLIENTPC_H

#include <vector>
#include <string>
#include <mutex>

enum pcStatus {STATUS_SLEEPING = 0, STATUS_AWAKE = 1};

class NetworkPC
{
    public:
        NetworkPC();
        NetworkPC(std::string IP, std::string MAC);
        NetworkPC(const NetworkPC &t);
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
