all:TCP_Receiver TCP_Sender RUDP_Receiver RUDP_Sender

TCP_Receiver: TCP_Receiver.o
	gcc -o TCP_Receiver TCP_Receiver.o

TCP_Sender: TCP_Sender.o
	gcc -o TCP_Sender TCP_Sender.o 

RUDP_Receiver: RUDP_Receiver.o RUDP_API.a
	gcc -o RUDP_Receiver RUDP_Receiver.o RUDP_API.a

RUDP_Sender: RUDP_Sender.o RUDP_API.a
	gcc -o RUDP_Sender RUDP_Sender.o RUDP_API.a

RUDP_API.a: RUDP_API.o
	ar -rcs RUDP_API.a RUDP_API.o

RUDP_Sender.o: RUDP_Sender.c 
	gcc -c -Wall RUDP_Sender.c -o RUDP_Sender.o

RUDP_Receiver.o: RUDP_Receiver.c
	gcc -c -Wall RUDP_Receiver.c -o RUDP_Receiver.o

RUDP_API.o: RUDP_API.c
	gcc -c -Wall RUDP_API.c -o RUDP_API.o


TCP_Sender.o: TCP_Sender.c RUDP_API.h
	gcc -c -Wall TCP_Sender.c -o TCP_Sender.o

TCP_Receiver.o: TCP_Receiver.c RUDP_API.h
	gcc -c -Wall TCP_Receiver.c -o TCP_Receiver.o
	
	
clean: 
	rm -f *.o *.a TCP_Receiver TCP_Sender RUDP_Receiver RUDP_Sender
	
	
	
	
	
