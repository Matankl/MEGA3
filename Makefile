CC = gcc
CFLAGS = -Wall 

all: TCP_Sender TCP_Receiver RUDP_Sender RUDP_Receiver


RUDP_Sender: RUDP_Sender.c RUDP_API.c
	$(CC) $(CFLAGS) RUDP_Sender.c -o RUDP_Sender

RUDP_Receiver: RUDP_Receiver.c RUDP_API.c
	$(CC) $(CFLAGS) RUDP_Receiver.c -o RUDP_Receiver

TCP_Sender: TCP_Sender.c
	$(CC) $(CFLAGS) TCP_Sender.c -o TCP_Sender

TCP_Receiver: TCP_Receiver.c
	$(CC) $(CFLAGS) TCP_Receiver.c -o TCP_Receiver

.PHONY: clean

clean:
	rm -f TCP_Sender TCP_Receiver RUDP_Sender RUDP_Receiver

