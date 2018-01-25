#include "newserv.h"

static Connection* g_clist;
int version = VERSION;

int sd_close(int sockno){
	for(Connection* curr = g_clist; curr->next!=NULL; curr = curr->next){
		if(curr->next->socket == sockno){
			Connection* tmp = curr->next->next;
			free(curr->next);
			curr->next = tmp;
			close(sockno);
			return 1;
		}
	}
	return -1; 
}

int add_connection(int sockno){
	Connection* newcon = (Connection*)malloc(sizeof(Connection));
	newcon->socket = sockno;
	newcon->total = 0;
	char* name = "noname";
	newcon->name = malloc(strlen(name) + 1);
	strcpy(newcon->name, name);
	newcon->next = g_clist->next;
	newcon->inbox = NULL;
	g_clist->next = newcon;
	return 1;
}

int num_connections(){
	Connection* curr = g_clist->next;
	int ret = 0;
	while(curr != NULL){
		ret++;
		curr = curr->next;
	}
	return ret;
}

int recv_name(Connection* conn){
	int sd = conn->socket;
	size_t rec_size;

	recv(sd, &rec_size, sizeof(size_t), 0);
	char* message = malloc(rec_size);
	memset(message, 0, rec_size);

	printf("Recieving %zd bytes\n", rec_size);
	recv(sd, message, rec_size, 0);
	
	printf("Got: %s\n", message);
	
	free(conn->name);
	conn->name = message;
	return 0;
}

int add_message(Connection* conn, char* messageIn){
	pthread_mutex_lock(&conn->lock);
	char* message = malloc(strlen(messageIn) + 1);
	strcpy(message, messageIn);
	Message* new_message = malloc(sizeof(Message));
	new_message->next = NULL;
	new_message->msg = message;
	Message* curr = conn->inbox;
	if(!curr)
		conn->inbox = new_message;
	else {
		while(curr->next)
			curr = curr->next;
		curr->next = new_message;
	}
	pthread_mutex_unlock(&conn->lock);
	return 0;
}

int recv_message(Connection* conn){
	int sd = conn->socket;
	size_t rec_size;
	char* name = conn->name;
	recv(sd, &rec_size, sizeof(size_t), 0);

	char* message = malloc(strlen(name) + 2 + rec_size);
	printf("Recieving %zd bytes\n", rec_size);
	char* curr = message;
	strcpy(curr, name);
	curr+=strlen(name);
	strcpy(curr, ": ");
	curr+=2;
	recv(sd, curr, rec_size, 0);
	printf("%s\n", message);

	Connection* curr_conn = g_clist->next;
	while(curr_conn){
		add_message(curr_conn, message);
		curr_conn = curr_conn -> next;
	}

				
	
	return 0;
}


int main(int argc, char** argv){

	if(argc != 3 || strcmp(argv[1], "-p")) {
		printf("Use -p <port>\n");
		return -1;
	}

	int port = atoi(argv[2]);
	printf("using port %d\n", port);

	struct sockaddr_in server_addr;
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if(sock<0)
		failwith("Error with socket()");

	if(bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		failwith("Error with bind. Try another port?");
	
	if(listen(sock, 12) < 0)
		failwith("Error with listen()");
	

	int max_sd;
	fd_set fds, write_fds;
	
	unsigned char buffer[70000];
	bzero(buffer, 70000);
	
	int addr_len = sizeof(server_addr);
	
	Connection* clist = malloc(sizeof(Connection));
	clist->next = NULL;
	g_clist = clist;

	while(1){

		FD_ZERO(&fds);
		FD_ZERO(&write_fds);
		FD_SET(sock, &fds);
		max_sd = sock;
	
		Connection* curr_conn = clist->next;

		//FD_SET and compute max_sd
		while(curr_conn != NULL){
			int curr_sd = curr_conn->socket;
			FD_SET(curr_sd, &fds);
			FD_SET(curr_sd, &write_fds);
			if (curr_sd > max_sd)
				max_sd = curr_sd;
			curr_conn = curr_conn->next;
		}
		
		//last parameter: NULL - no timeout		
		if(select(max_sd + 1, &fds, &write_fds, NULL, NULL) < 0)
			failwith("Error with select()");
		
		//a new client
		if(FD_ISSET(sock, &fds)){
			int new_sd = accept(sock, (struct sockaddr*)&server_addr, (socklen_t*)&addr_len);
			if(new_sd < 0)
				failwith("Error with accept()");

			//if(new_sd > max_sd) //need to make sure this isnt necessary 01/19
			//	max_sd = new_sd;
			add_connection(new_sd);

			printf("Added a new connection. Currently have %d connections\n", num_connections());

		}
		
		//check all connections
		curr_conn = clist->next;

		while(curr_conn != NULL){

			int curr_sd = curr_conn->socket;
			


			if(FD_ISSET(curr_sd, &write_fds)){

				pthread_mutex_lock(&curr_conn->lock);
				if(curr_conn->inbox){
					
					Message* msg = curr_conn->inbox;
					curr_conn->inbox = curr_conn->inbox->next;
					
					char* mail = msg->msg;
					int mlength = strlen(mail) + 1;
					size_t send_size = strlen(mail) + 1;
					int type = NAMEMSG;

					send(curr_sd, &version, sizeof(int), 0);
					send(curr_sd, &type, sizeof(int), 0);

					send(curr_sd, &send_size, sizeof(size_t), 0);

					//int ret = 
					send(curr_sd, mail, mlength, 0);
					//printf("sent, ret value %d\n", ret);
					free(msg->msg);
					free(msg);
				}
				pthread_mutex_unlock(&curr_conn->lock);
			}


			if(FD_ISSET(curr_sd, &fds)){
				int version, type;
				
				int hbr = recv(curr_sd, &version, sizeof(int), 0);
				recv(curr_sd, &type, sizeof(int), 0);
				if(hbr <= 0){
					sd_close(curr_sd);
				}
				else if(version != VERSION){
					printf("version doesn't match, closing\n");
					sd_close(curr_sd);
				}
				else {
					switch(type){
						case NAME:
							recv_name(curr_conn);
							break;

						case MESSAGE:
							recv_message(curr_conn);
							break;

						default:
							printf("unsupported message type, closing\n");
							sd_close(curr_sd);
							break;

					}	
				}				
			}
			
			curr_conn = curr_conn->next;
		}
	}

}