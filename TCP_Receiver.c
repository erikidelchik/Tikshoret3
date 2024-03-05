/*
    TCP/IP-server
*/

#include <stdio.h>

#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>



#define BUFFER_SIZE 1024
#define FILE_SIZE_IN_MB 2 //file size in mb
#define FILE_SIZE_IN_BYTES 2*1024*1024

int main(int argc,char ** argv) {
    
    
    // Open the listening (server) socket
    int listeningSocket = -1;
    listeningSocket = socket(AF_INET, SOCK_STREAM, 0); 
    if (listeningSocket == -1) {
        printf("Could not create listening socket : %d", errno);
        return 1;
    }
    
    //set the port and the tcp congestion control algorithm
    
    int SERVER_PORT = 9997;
    if(argv[1] && argv[2] && !strcmp(argv[1],"-p")){
     	SERVER_PORT = atoi(argv[2]);
    }
    
    char buf[256];
    socklen_t len;
	
    
    if(argv[4] && argv[3] && !strcmp(argv[3],"-algo") && (!strcmp(argv[4],"cubic") || !strcmp(argv[4],"reno"))) strcpy(buf, argv[4]);
    else strcpy(buf,"cubic"); 
    len = strlen(buf);

    if (setsockopt(listeningSocket, IPPROTO_TCP, TCP_CONGESTION, buf, len) != 0)
    {
        perror("setsockopt");
        return -1;
    }

    printf("Current algo: %s\n", buf); 





    
    
    int enableReuse = 1;
    int ret = setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int));
    if (ret < 0) {
        printf("setsockopt() failed with error code : %d", errno);
        return 1;
    }

    
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY; // any IP at this port (Address to accept any incoming messages)
    serverAddress.sin_port = htons(SERVER_PORT);  // network order (makes byte order consistent)

    // Bind the socket to the port with any IP at this port
    int bindResult = bind(listeningSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (bindResult == -1) {
        printf("Bind failed with error code : %d", errno);
        // close the socket
        close(listeningSocket);
        return -1;
    }

    printf("Bind success to port %d\n",SERVER_PORT);

    // Make the socket listening; actually mother of all client sockets.
    // 500 is a Maximum size of queue connection requests
    // number of concurrent connections
    int listenResult = listen(listeningSocket, 3);
    if (listenResult == -1) {
        printf("listen() failed with error code : %d", errno);
        // close the socket
        close(listeningSocket);
        return -1;
    }

    // Accept and incoming connection
    printf("Waiting for incoming TCP-connections...\n");
    struct sockaddr_in clientAddress;  //
    socklen_t clientAddressLen = sizeof(clientAddress);
    
  

    while (1) {
        memset(&clientAddress, 0, sizeof(clientAddress));
        clientAddressLen = sizeof(clientAddress);
        int clientSocket = accept(listeningSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
        if (clientSocket == -1) {
            printf("listen failed with error code : %d", errno);
            // close the sockets
            close(listeningSocket);
            return -1;
        }

        printf("A new client connection accepted\n");
        
        double time = 0;
        double avgtime = 0;
        double speed = 0;
        double avgspeed = 0;
        int msgnum = 0;
        int bytesReceived = 0;
        
        clock_t start_time, end_time;
        

        // Receive a message from client
        char buffer[BUFFER_SIZE];
        
        unsigned int totalBytesReceived = 0;
        
        int flag = 1;
	
	while(1){
		memset(buffer, 0, BUFFER_SIZE);
		start_time = clock();
		
		
		while((bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0))>0){
		    if(!strncmp(buffer,"exit",4)){
			    close(clientSocket);
			    flag = 0;
			    break;
        	}

		 
        	    if(totalBytesReceived>=FILE_SIZE_IN_BYTES) break;
		    totalBytesReceived += bytesReceived;        
		    memset(buffer, 0, BUFFER_SIZE);
		    

		}
		
		if(!flag) break;
		end_time = clock();
		
		
		
        	time = ((double)(end_time - start_time) / CLOCKS_PER_SEC)*100;
        	printf("time it took to receive the message: %.2f ms\n",time);
        	msgnum+=1;
		avgtime+=time;
		printf("average time: %.2f ms \n",avgtime/msgnum);
		speed = (FILE_SIZE_IN_MB/time)*1000;
		avgspeed +=speed;
		printf("bandwidth is: %.2f MB/s\n",speed);
		printf("average bandwidth is: %.2f MB/s\n",avgspeed/msgnum);
		printf("**********************************************\n");
	
        	totalBytesReceived=0;
        	
        	
        }
        printf("Receiver end\n");

        
      
    }

    printf("closing receiver");

    close(listeningSocket);

    return 0;
}

