typedef struct _rudp_socket RUDP_Socket;
RUDP_Socket* rudp_socket(int, unsigned short int);
int rudp_connect(RUDP_Socket*, const char*, unsigned short int );
int rudp_accept(RUDP_Socket*);
int rudp_recv(RUDP_Socket*, char*, unsigned int);
int rudp_send(RUDP_Socket*, char*, unsigned int);
int rudp_disconnect(RUDP_Socket*);
void rudp_close(RUDP_Socket*);
