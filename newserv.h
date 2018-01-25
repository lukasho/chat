/*
	Current packet structure:

	[size_t: payload size] [that many bytes]
	
	Plan:
	[One byte: version]
	[One byte: message type]
	[size_t: payload size]
	[payload]	


*/


#include <stdio.h>
#include <stdlib.h> //atoi
#include <string.h> //strcmp

#include <arpa/inet.h> //socket, bind, listen, etc
#include <sys/select.h>//select
#include <unistd.h>		//close

#include <errno.h>

#define VERSION 1

#define NEW_CONNECTION 0
#define NAME 1
#define MESSAGE 2

void failwith(char* msg){
	fprintf(stderr, "%s errno: %d\n", msg, errno);
	exit(errno);
}

typedef struct connection_struct{
	int socket;
	char* name;
	int total;
	struct connection_struct* next;
} Connection;
