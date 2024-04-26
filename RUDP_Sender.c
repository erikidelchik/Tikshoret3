
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include "RUDP_API.h"

#define FILE_SIZE 2*1024*1024
#define BUFFER_SIZE 1024

char *util_generate_random_data(unsigned int size) {
    char *buffer = NULL;
    // Argument check.
    if (size == 0)
        return NULL;
    buffer = (char *)calloc(size, sizeof(char));
    // Error checking.
    if (buffer == NULL)
        return NULL;
    // Randomize the seed of the random number generator.
    for (unsigned int i = 0; i < size; i++)
        *(buffer + i) = ((unsigned int)rand() % 256);
    return buffer;
}

void send_file_message(RUDP_Socket* sock,FILE*file){

    char buffer[BUFFER_SIZE];
    char ack_msg[BUFFER_SIZE];
    int curr_file_pos = 0;
    int read;
    fseek(file,0,SEEK_SET);
    char* exitmsg = "exit";  //indicates that we finished sending the file

    while ((read = fread(buffer, 1, BUFFER_SIZE, file))>0) {
        
        
        rudp_send(sock, buffer, BUFFER_SIZE);

        // Receive ack from the server
        rudp_recv(sock, ack_msg, strlen(ack_msg));

      
        if (strncmp(ack_msg, "ACK", 3) == 0) {   //if recieved ack for the packet, continue reading
            curr_file_pos+=read;
            memset(buffer,0,read);
        }
        fseek(file,curr_file_pos,SEEK_SET);
    }
    rudp_send(sock,exitmsg,strlen(exitmsg));

}

int main(int argc,char* argv[]){
    char* server_ip= "127.0.0.1";
    int server_port = 9997;
    if(argc==5) {
        if (!strcmp(argv[3], "-p")) server_port = atoi(argv[4]);
        if(!strcmp(argv[1],"-ip")) server_ip = argv[2];
    }

    RUDP_Socket* sock = rudp_socket(0,0);
    int connected = rudp_connect(sock,server_ip,server_port);
    if(!connected){
        printf("connection to server failed\n");
        return -1;
    }
    
    FILE* file = fopen("asd.txt","wb+");
    char * message = util_generate_random_data(FILE_SIZE);
    
    fwrite(message, FILE_SIZE,1,file);
    free(message);


    send_file_message(sock,file);

    int n = 0;

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
    
    fclose(file);
    rudp_disconnect(sock);
    rudp_close(sock);
    return 0;


}
