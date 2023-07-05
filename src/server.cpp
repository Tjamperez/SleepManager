#include "../include/server.h"

Server_Connection::Server_Connection(char *hostname)
{
    this->count = 0;
    this->hostname = hostname;
    this->server = gethostbyname(this->hostname); // nome do server/manager
    this->serv_addr.sin_family = AF_INET;     
    this->serv_addr.sin_port = htons(PORT);    
    this->serv_addr.sin_addr = *((struct in_addr *)this->server->h_addr);
    this->serv_addr.sin_addr.s_addr = INADDR_ANY;
};

int Server_Connection::start_client()
{
    this->start = clock();
    char buffer[256];

    if (this->server == NULL) 
    {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    };	

    int bcast_sock;
    if ((bcast_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        printf("ERROR opening socket");

    int broadcast_enable = 1; // cliente manda em broadcast pra ver quem é o lider
    if(setsockopt(bcast_sock,SOL_SOCKET,SO_BROADCAST,&broadcast_enable,sizeof(broadcast_enable)) < 0)
        printf("Error setting broadcast"); 

    bzero(&(this->serv_addr.sin_zero), 8);
    int n = 0;
    printf("Enter the message: ");
    bzero(buffer, 256);
    fgets(buffer, 256, stdin);

    while(1)
    {
        n = sendto(bcast_sock, buffer, strlen(buffer), 0, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
        if (n < 0) 
            printf("ERROR sendto");

        unsigned int length = sizeof(struct sockaddr_in);
        int timeoutInMicroseconds = 1;

        while (!hasTimeoutPassed(timeoutInMicroseconds)) {
            // checamos se recebeu algo
            if (n = recvfrom(bcast_sock, buffer, 256, 0, (struct sockaddr *) &from, &length) > 0) 
            {
                // recebeu quebra o loop e vai pro manager
                printf("Received the answer\n");
                break;
                //manda pro manager()??????
            }
            else
            {
                if (n < 0)
                    printf("ERROR recvfrom");
            };
            // continua esperando timeout
        }
    }
    
    close(bcast_sock);
    return 0;
};

int Server_Connection::start_server()
{
    int peer2peer, n;
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    char buf[256];
        
    if ((peer2peer = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
        printf("ERROR opening socket");

    bzero(&(serv_addr.sin_zero), 8);    
    
    if (bind(peer2peer, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0) 
        printf("ERROR on binding");
    
    clilen = sizeof(struct sockaddr_in);
    
    while (1) {
        /* receive from socket */
        n = recvfrom(peer2peer, buf, 256, 0, (struct sockaddr *) &cli_addr, &clilen);
        if (n < 0) 
            printf("ERROR on recvfrom");
        printf("Received a datagram: %s\n", buf);
        
        /* send to socket */
        n = sendto(peer2peer, "I am the manager\n", 17, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
        if (n  < 0) 
            printf("ERROR on sendto");
    }
    
    close(peer2peer);
    return 0;
};

int Server_Connection::hasTimeoutPassed( int timeoutInMicroseconds){
    clock_t current = clock();
    clock_t elapsed = (current - this->start) / (CLOCKS_PER_SEC / 1000000);  // Convert ticks to microseconds
    return (elapsed >= timeoutInMicroseconds);
}