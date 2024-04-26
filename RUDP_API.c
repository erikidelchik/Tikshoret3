
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
#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 9997
#define BUFFER_SIZE 1024

typedef struct _rudp_socket
{
    int socket_fd; // UDP socket file descriptor
    int isServer; // 1 if the RUDP socket acts like a server, 0 for client.
    int isConnected; // 1 if there is an active connection, 0 otherwise.
    struct sockaddr_in dest_addr; // Destination address. Client fills it when it connects via rudp_connect(), server fills it when it accepts a connection via rudp_accept().
} RUDP_Socket;


// Allocates a new structure for the RUDP socket (contains basic information about the socket itself). Also creates a UDP socket as a baseline for the RUDP. isServer means that this socket acts like a server. If set to server socket, it also binds the socket to a specific port.
RUDP_Socket* rudp_socket(int isServer, unsigned short int listen_port){
    RUDP_Socket* sock = malloc(sizeof(RUDP_Socket));
    sock->isServer = isServer;
    sock->isConnected = 0;
    int socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socketfd == -1) {
        perror("Could not create socket");
        exit(1);
    }
    sock->socket_fd = socketfd;
    memset((char *)&sock->dest_addr, 0, sizeof(sock->dest_addr));
    
    int reuse = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        close(socketfd);
        exit(EXIT_FAILURE);
    }

    if(isServer) {
        struct sockaddr_in self_addr;
        memset((char *)&self_addr,0,sizeof (self_addr));
        self_addr.sin_family = AF_INET;
        self_addr.sin_port = htons(listen_port);
        int ret = inet_pton(AF_INET, (const char *)SERVER_IP_ADDRESS, &(self_addr.sin_addr));
        if (ret <= 0) {
            perror("inet_pton failed");
            exit(1);
        }

        int bindResult = bind(sock->socket_fd, (struct sockaddr *) &self_addr, sizeof(self_addr));
        if (bindResult == -1) {
            perror("bind failed");
            close(sock->socket_fd);
            exit(1);
        }
    }
    printf("created socket\n");
    return sock;

}

// Tries to connect to the other side via RUDP to given IP and port. Returns 0 on failure and 1 on success. Fails if called when the socket is connected/set to server.
int rudp_connect(RUDP_Socket *sockfd, const char *dest_ip, unsigned short int dest_port){
    if(sockfd->isConnected || sockfd->isServer) return 0;

    char *synAcc = "ACK";
    sockfd->dest_addr.sin_port = htons(dest_port);
    int ret = inet_pton(AF_INET,dest_ip,&(sockfd->dest_addr.sin_addr));
    if (ret <= 0) {
        perror("inet_pton failed");
        exit(1);
    }

    int sendResult = sendto(sockfd->socket_fd, synAcc, strlen(synAcc)+1, 0, (struct sockaddr *)&sockfd->dest_addr, sizeof(sockfd->dest_addr));
    if (sendResult == -1) {
        perror("send failed");
        return 0;
    }

    char connectionAckBuffer [BUFFER_SIZE];
    memset(connectionAckBuffer,0,sizeof (connectionAckBuffer));
    socklen_t socklen = sizeof(sockfd->dest_addr);
    int connectionAckBytes = recvfrom(sockfd->socket_fd, connectionAckBuffer, sizeof(connectionAckBuffer), 0, (struct sockaddr *)&sockfd->dest_addr, &socklen);
    if (connectionAckBytes == -1) {
        perror("Connection failed");
        exit(1);
    }
    if(!strncmp(connectionAckBuffer,"ACK",3)){
        printf("connection established\n");
        sockfd->isConnected=1;
        return 1;
    }
    return 0;


}

// Accepts incoming connection request and completes the handshake, returns 0 on failure and 1 on success. Fails if called when the socket is connected/set to client.
int rudp_accept(RUDP_Socket *sockfd){
    if(sockfd->isConnected) return 0;
    char connectionAckBuffer [BUFFER_SIZE];
    char* ack_msg = "ACK";
    socklen_t socklen = sizeof(sockfd->dest_addr);
    int connectionAckBytes = recvfrom(sockfd->socket_fd, connectionAckBuffer, sizeof(connectionAckBuffer),0,(struct sockaddr *)&sockfd->dest_addr, &socklen);
    if (connectionAckBytes == -1) {
        perror("Connection failed");
        exit(1);
    }
    if(strncmp(connectionAckBuffer,"ACK",3)) return 0;

    int sendResult = sendto(sockfd->socket_fd, ack_msg, sizeof(ack_msg)+1, 0, (struct sockaddr *)&sockfd->dest_addr, sizeof(sockfd->dest_addr));
    if (sendResult == -1) {
        perror("send failed");
        exit(1);
    }
    sockfd->isConnected= 1;

    return 1;

}

// Receives data from the other side and put it into the buffer. Returns the number of received bytes on success, 0 if got FIN packet (disconnect), and -1 on error. Fails if called when the socket is disconnected.
int rudp_recv(RUDP_Socket *sockfd, char *buffer, unsigned int buffer_size){
    if(!sockfd->isConnected) return -1;
    socklen_t socklen = sizeof(sockfd->dest_addr);
    
    int connectionAckBytes = recvfrom(sockfd->socket_fd, buffer, BUFFER_SIZE,0,(struct sockaddr *)&sockfd->dest_addr, &socklen);
    if (connectionAckBytes == -1) {
        perror("Connection failed");
        exit(1);
    }
    if(!strncmp(buffer,"FIN",3)){
        return 0;
    }
    return connectionAckBytes;
}

// Sends data stores in buffer to the other side. Returns the number of sent bytes on success, 0 if got FIN packet (disconnect), and -1 on error. Fails if called when the socket is disconnected.
int rudp_send(RUDP_Socket *sockfd, char *buffer, unsigned int buffer_size){
    int sendResult = sendto(sockfd->socket_fd, buffer, buffer_size+1, 0, (struct sockaddr *)&sockfd->dest_addr, sizeof(sockfd->dest_addr));
    if (sendResult == -1) {
        perror("send failed");
        exit(1);
    }
    if(!strncmp(buffer,"FIN",3)) return 0;
    return sendResult;

}

// Disconnects from an actively connected socket. Returns 1 on success, 0 when the socket is already disconnected (failure).
int rudp_disconnect(RUDP_Socket *sockfd){
    if(sockfd->isConnected==0) return 0;
    if(sockfd->isServer) sockfd->isConnected = 0;
    else{
       char* finAcc = "FIN";
       rudp_send(sockfd,finAcc, strlen(finAcc));
       sockfd->isConnected = 0;
    }
    return 1;

}

// This function releases all the memory allocation and resources of the socket.
void rudp_close(RUDP_Socket *sockfd){
    close(sockfd->socket_fd);
    free(sockfd);
}
