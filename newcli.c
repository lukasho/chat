#include "newcli.h"

int sock; //global variable: socket for server





int send_name(char* name){
	int version = VERSION;
	int type = NAME;
	size_t len;

	char* message = malloc(strlen(name));
	strcpy(message, name);
	message[strlen(name) - 1] = '\0';
	
	printf("Chose %s\n", message);
	len = strlen(message);

	send(sock, &version, sizeof(int), 0);
	send(sock, &type, sizeof(int), 0);

	send(sock, &len, sizeof(size_t), 0);
	send(sock, message, len, 0);

	free(message);

	return 0;
}

int send_message(char* input){
	int version = VERSION;
	int type = MESSAGE;
	size_t len;
	char* message = malloc(strlen(input));
	strcpy(message, input);
	message[strlen(input) - 1] = '\0';
	printf("Sending %s\n", message);
	len = strlen(message) + 1;

	send(sock, &version, sizeof(int), 0);
	send(sock, &type, sizeof(int), 0);

	send(sock, &len, sizeof(size_t), 0);
	send(sock, message, len, 0);

	free(message);

	return 0;
}

void* Input_handler(){
	char* buffer = NULL;
	int num_read;
	size_t len;
	
	while(1){
		num_read = getline(&buffer, &len, stdin);
		if(num_read > 0){
			
			printf("Read %s\n", buffer);
			send_message(buffer);
		}
			
		else
			printf("Nothing read??\n");
	}
}



int main(int argc, char** argv){

	int hasargs[2] = {0};
	char* addr;
	int port, i;
	size_t len = 0;

	if(argc != 5)
		failwith("Use arguments: -a [ip address] -p [port]");


	for(i = 0; i < 2; i++){
		if (!strcmp(argv[1 + 2*i], "-a")){
			hasargs[0] = 1;
			addr = argv[2 + 2*i];
		}
		if (!strcmp(argv[1 + 2*i], "-p")){
			hasargs[1] = 1;
			port = atoi(argv[2 + 2*i]);
		}
	}
	for(i = 0; i < 2; i++)
		if(!hasargs[i])
			failwith("Use arguments: -a [ip address] -p [port]");
	
	struct sockaddr_in server_addr;
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(addr);

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock<0)
		failwith("Error with socket()");

	if(connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
		failwith("Error with connect()");



	

	
	char* buffer = NULL;
	int num_read = 0;
	printf("choose a name: \n");
	
	while(len <= 2)
		num_read = getline(&buffer, &len, stdin);

	send_name(buffer);

	printf("Done\n");
	pthread_t input_thread;
	if(pthread_create(&input_thread, NULL, Input_handler, NULL))
		failwith("Error creating the input handler thread!");


	while(1);
}