
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include "RUDP_API.h"

#define FILE_SIZE 2*1024*1024
#define FILE_SIZE_IN_MB 2
#define BUFFER_SIZE 1024

int main(int argc,char *argv[]){

    int server_port = 9997;
    if(argc==3){
    	if(strcmp(argv[1],"-p")) server_port = atoi(argv[2]);
    }


    RUDP_Socket* server_address = rudp_socket(1,server_port);

    printf("waiting for clients..\n");

    while(1){
        if(!rudp_accept(server_address)){
            continue;
        }
        printf("new client accepted\n");

        clock_t start_time, end_time;
        double time = 0;
        double avgtime = 0;
        double speed = 0;
        double avgspeed = 0;
        int msgnum = 0;

        char buffer[BUFFER_SIZE];
      
        int totalBitsReceived = 0;
        int bitsReceived=0;
        char* ack_msg ="ACK";
        


        while(1){
            memset(buffer, 0, BUFFER_SIZE);
            start_time = clock();
	    
            while((bitsReceived = rudp_recv(server_address,buffer,BUFFER_SIZE))>0){
                if(!strncmp("exit",buffer,4)) break;
       

                rudp_send(server_address,ack_msg, strlen(ack_msg)+1);
               
                totalBitsReceived += bitsReceived;
                
                memset(buffer, 0, BUFFER_SIZE);
               
     

            }

            if(!strncmp("FIN",buffer,3)) break;
            end_time = clock();
            printf("total Bits Received: %d\n",totalBitsReceived);



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

            totalBitsReceived=0;

        }
        rudp_disconnect(server_address);
        printf("Receiver end\n");
    }

    return 0;
}
