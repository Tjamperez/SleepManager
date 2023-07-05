#include "../include/discovery.h"
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include "../include/helper.h"
#include <arpa/inet.h>
#include <time.h>

class server_connection{
    public:
	clock_t start = clock();//começo do programa
    int bcast_sock, n, count = 0;
	unsigned int length;
	struct sockaddr_in serv_addr, from;
	struct hostent *server;
	string *ip_address, *mac_address;
    Packet packet;

		server_connection(string *ip_address, string *mac_address){
			start_connection();
		}

        void start_connection(){
            n = sendto(bcast_sock, (void*)&packet, sizeof(Packet), 0, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
		    if (n < 0) {
		    	printf("ERROR sendto");
		    }
		    length = sizeof(struct sockaddr_in);
		    int timeoutInMicroseconds = 1;
			while(count < 3){
 		    	while (!hasTimeoutPassed(start, timeoutInMicroseconds)) {
        	    	// checamos se recebeu algo
        	    	if (n = recvfrom(bcast_sock, (void*)&packet, 256, 0, (struct sockaddr *) &from, &length) > 0) {
                		// recebeu quebra o loop e vai pro manager
			    		printf("Received the answer\n");
			    		break;
                		//monitoring()
        	   	 }
        	   	 // continua esperando timeout
    	   	 }
				if (n < 0 and hasTimeoutPassed(start, timeoutInMicroseconds)){
			 	   printf("ERROR recvfrom");
				}
			}
			count++;
	   	 	close(bcast_sock);
        	}
        int hasTimeoutPassed(clock_t start, int timeoutInMicroseconds){
            clock_t current = clock();
            clock_t elapsed = (current - start) / (CLOCKS_PER_SEC / 1000000);  // Convert ticks to microseconds
            return (elapsed >= timeoutInMicroseconds);
        }
};

int client_udp(int argc, char *argv[])
{
	clock_t start = clock();//começo do programa
    int bcast_sock, n;
	unsigned int length;
	struct sockaddr_in serv_addr, from;
	struct hostent *server;
	char buffer[256];

	if (argc < 2) {
		fprintf(stderr, "usage %s hostname\n", argv[0]);
		exit(0);

	}

	server = gethostbyname(argv[1]); // nome do server/manager
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }	
	
	if ((bcast_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		printf("ERROR opening socket");
    int broadcast_enable = 1; // cliente manda em broadcast pra ver quem é o lider
    if(setsockopt(bcast_sock,SOL_SOCKET,SO_BROADCAST,&broadcast_enable,sizeof(broadcast_enable)) < 0){
        printf("Error setting broadcast"); 
    }
	
	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(PORT);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);
	n = 0;
	printf("Enter the message: ");
	bzero(buffer, 256);
	fgets(buffer, 256, stdin);
	while(1){
		n = sendto(bcast_sock, buffer, strlen(buffer), 0, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
		if (n < 0) {
			printf("ERROR sendto");
		}
		length = sizeof(struct sockaddr_in);
		int timeoutInMicroseconds = 1;
 		while (!hasTimeoutPassed(start, timeoutInMicroseconds)) {
        	// checamos se recebeu algo
        	if (n = recvfrom(bcast_sock, buffer, 256, 0, (struct sockaddr *) &from, &length) > 0) {
            	// recebeu quebra o loop e vai pro manager
				printf("Received the answer\n");
				break;
            	//manda pro manager()??????
        	}
			else{
				if (n < 0){
					printf("ERROR recvfrom");
				}
			}
        	// continua esperando timeout
    	}
	}
	
	close(bcast_sock);
	return 0;
}
int hasTimeoutPassed(clock_t start, int timeoutInMicroseconds)
{
    clock_t current = clock();
    clock_t elapsed = (current - start) / (CLOCKS_PER_SEC / 1000000);  // Convert ticks to microseconds
    return (elapsed >= timeoutInMicroseconds);
}

int manager_udp(int argc, char *argv[])
{
    int peer2peer, n;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	char buf[256];
		
    if ((peer2peer = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
		printf("ERROR opening socket");
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
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
}
