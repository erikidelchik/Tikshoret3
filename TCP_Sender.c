/*
        TCP/IP client
*/

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#define FILE_SIZE (2*1024*1024) //file size it bytes

#define BUFFER_SIZE 1024

void send_file_message(int sock,FILE *file){
    
    
    char buffer[BUFFER_SIZE];
    int curr_file_pos = 0;
    int read;
    fseek(file,0,SEEK_SET);
    while ((read = fread(buffer,1,BUFFER_SIZE,file))>0) {
    	fseek(file,curr_file_pos,SEEK_SET);
    	
        if (send(sock, buffer, read, 0) == -1) {
            perror("Error sending data");
            break;
        }
        curr_file_pos+=read;
        memset(buffer,0,read);
        
    }
    
}

char *util_generate_random_data(unsigned int size) {
	 char *buffer = NULL;
	 // Argument check.
	 if (size == 0)
	 	return NULL;
	 buffer = (char *)calloc(size, sizeof(char));
	 // Error checking.
	 if (buffer == NULL)
	 	return NULL;
	 
	 for (unsigned int i = 0; i < size; i++)
		 *(buffer + i) = ((unsigned int)rand() % 256);
	 return buffer;
}



int main(int argc,char ** argv) {
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock == -1) {
        printf("Could not create socket : %d", errno);
        return -1;
    }
    
    //set the port and the tcp congestion control algorithm
    
    int SERVER_PORT = 9997;
    char *SERVER_IP_ADDRESS = "127.0.0.1";
    if(argv[1] && argv[2] && !strcmp(argv[1],"-ip")){
     	SERVER_IP_ADDRESS = argv[2];
     }
    printf("server address: %s: \n",SERVER_IP_ADDRESS);
    
    if(argv[3] && argv[4] && !strcmp(argv[3],"-p")){
     	SERVER_PORT = atoi(argv[4]);
     }
    
    char buf[256];
    socklen_t len;
	
    
    if(argv[6] && argv[5] && !strcmp(argv[5],"-algo") && (!strcmp(argv[6],"cubic") || !strcmp(argv[6],"reno"))) strcpy(buf, argv[6]);
    else strcpy(buf,"cubic"); 
    len = strlen(buf);

    if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, buf, len) != 0)
    {
        perror("setsockopt");
        return -1;
    }
    
    printf("Current algo: %s\n", buf); 

    
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);                                    
    int rval = inet_pton(AF_INET, (const char *)SERVER_IP_ADDRESS, &serverAddress.sin_addr);  
    if (rval <= 0) {
        printf("inet_pton() failed");
        return -1;
    }

    // Make a connection to the server with socket 
    int connectResult = connect(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (connectResult == -1) {
        printf("connect() failed with error code : %d", errno);
        // cleanup the socket;
        close(sock);
        return -1;
    }

    printf("connected to server\n");

    // Sends file to server
    
    
    
    FILE *file = fopen("asd.txt","wb+");
    if(file==NULL){
        printf("error");
        return -1;
    }
    
    char *message = util_generate_random_data(FILE_SIZE);
    
    fwrite(message,FILE_SIZE,1,file);
    
    free(message);
    
    
    int n = 0;
    
    send_file_message(sock,file);
    

    while(1) {
        printf("send again? 1-yes, 2-no: ");
        scanf("%d",&n);
      	if(n==1){ 
      		printf("sending file again\n");
      		send_file_message(sock,file);
      		
      	}
        else if(n==2) break;
        else{
        	printf("invalid command\n");
        } 
  
    }
    char *exit_message = "exit";
    int res = send(sock,exit_message, strlen(exit_message)+1,0);
    if(res==-1){
    	printf("error sending exit message");
    	return -1;
    }
    fclose(file);
    close(sock);
    return 0;
}

