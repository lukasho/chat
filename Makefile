CC=gcc
CFLAGS=-Wextra -Wall -Werror -W -std=c99
all: client server
client: newcli.c newcli.h
	$(CC) $(CFLAGS) newcli.c -o client.x

server:	newserv.c newserv.h
	$(CC) $(CFLAGS) newserv.c -o server.x
clean: 
	rm server.x client.x