#include "../include/discovery.h"
#include "../include/packet.h"
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

class server_connection{
    clock_t start = clock();//começo do programa
    int bcast_sock, n,counter = 0;
	unsigned int length;
	struct sockaddr_in serv_addr, from;
	struct hostent *server;
    Packet packet;
    public:

        server_connection(){
            n = sendto(bcast_sock, (void*)&packet, sizeof(Packet), 0, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
		    if (n < 0) {
		    	printf("ERROR sendto");
		    }
		    length = sizeof(struct sockaddr_in);
		    int timeoutInMicroseconds = 1;
 		    while (!hasTimeoutPassed(start, timeoutInMicroseconds)) {
        	    // checamos se recebeu algo
        	    if (n = recvfrom(bcast_sock, (void*)&packet, 256, 0, (struct sockaddr *) &from, &length) > 0) {
                	// recebeu quebra o loop e vai pro manager
			    	printf("Received the answer\n");
			    	break;
                	//monitoring()
        	    }
			    else{
			    	if (n < 0){
			    		printf("ERROR recvfrom");
			    	}
			    }
        	    // continua esperando timeout
			    counter++;
    	    }
	    close(bcast_sock);
	    return ;
        }
        int connect_udp(){
            
        }
        int hasTimeoutPassed(clock_t start, int timeoutInMicroseconds){
            clock_t current = clock();
            clock_t elapsed = (current - start) / (CLOCKS_PER_SEC / 1000000);  // Convert ticks to microseconds
            return (elapsed >= timeoutInMicroseconds);
        }
};