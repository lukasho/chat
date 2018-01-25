#include <stdio.h>
#include <stdlib.h> //atoi
#include <string.h> //strcmp

#include <arpa/inet.h> //socket, bind, listen, etc

#include <errno.h>

#include <pthread.h>
#include <unistd.h>		//close

#define VERSION 1

#define NEW_CONNECTION 0
#define NAME 1
#define MESSAGE 2
#define NAMEMSG 3

void failwith(char* msg){
	fprintf(stderr, "%s | errno: %d\n", msg, errno);
	exit(errno);
}