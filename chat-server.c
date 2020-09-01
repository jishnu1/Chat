#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#define PORT 9000
#define NAME_LENGTH 16
#define MESSAGE_LENGTH 256

char my_name[NAME_LENGTH];
char other_name[NAME_LENGTH];

int ex = 0;

typedef struct Args
{
	int connfd;
} Args;

void trim(char * str, int len)
{
	str[len-1] = '\0';
}
void * handle_send(void * args)
{
	Args * args1 = (Args *) args;
	char msg[MESSAGE_LENGTH];

	while (1)
	{
		do
		{
			printf("> ");
			fgets(msg, MESSAGE_LENGTH, stdin);
			trim(msg, strlen(msg));
		}
		while (strlen(msg) < 1);

		write(args1->connfd, msg, strlen(msg));

		if (strcmp(msg, "exit") == 0)
		{
			printf("You have ended the chat\n");
			ex = 1;
			break;
		}
		bzero(msg, MESSAGE_LENGTH);
	}
	return NULL;
}
void * handle_recv(void * args)
{
	Args * args1 = (Args *) args;
	char msg[MESSAGE_LENGTH];

	while (1)
	{
		do
		{
			recv(args1->connfd, msg, MESSAGE_LENGTH, 0);
		}
		while (strlen(msg) < 1);

		if (strcmp(msg, "exit") == 0)
		{
			printf("\n%s has ended the chat\n", other_name);
			ex = 1;
			break;
		}
		printf("%s: %s\n> ", other_name, msg);
		fflush(stdout);
		bzero(msg, MESSAGE_LENGTH);
	}
	return NULL;
}

int main(int argc, char** argv)
{
	struct sockaddr_in serv_addr;
	int sockfd, connfd;
    pthread_t send_tid, recv_tid;
	int reuse = 1;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0)
	{
        printf("ERROR: socket creation failed\n"); 
        exit(1); 
    }

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PORT);

	if(setsockopt(sockfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char *) &reuse, sizeof(reuse)) < 0)
	{
		printf("ERROR: socket options failed");
		exit(1);
	}
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("ERROR: socket bind failed\n");
		exit(1);
	}
	if (listen(sockfd, 5) < 0)
	{
		printf("ERROR: listen failed\n");
		exit(1);
	}
	else
	{
		printf("Waiting for connection...\n");
	}

	connfd = accept(sockfd, (struct sockaddr *) NULL, NULL);
	if (connfd < 0)
	{ 
		printf("ERROR: connection failed\n"); 
		exit(1);
	}
	
	printf("============== WELCOME ==============\n");
	printf("Type \"exit\" to exit\n");

	do
	{
		printf("Enter your name: ");
		fgets(my_name, NAME_LENGTH, stdin);
		trim(my_name, strlen(my_name));
	}
	while (strlen(my_name) < 1);
	
	write(connfd, my_name, strlen(my_name));

	recv(connfd, other_name, NAME_LENGTH, 0);
	printf("You are connected with %s\n", other_name);

	Args * args = (Args *) malloc(sizeof(Args));
	args->connfd = connfd;

	if (pthread_create(&send_tid, NULL, &handle_send, (void *) args) != 0)
	{
		printf("ERROR: sender thread creation failed\n");
		exit(1);
	}
	if (pthread_create(&recv_tid, NULL, &handle_recv, (void *) args) != 0)
	{
		printf("ERROR: receiver thread creation failed\n");
		exit(1);
	}
	while (!ex)
	{
		sleep(1);
	}

	printf("============== GOODBYE ==============\n");
	return 0;
}

/*
Makefile 1

all: server client

server: chat-server.c
	gcc -Wall -Werror -fsanitize=address -pthread chat-server.c -o chat-server

client: chat-client.c
	gcc -Wall -Werror -fsanitize=address -pthread chat-client.c -o chat-client

clean:
	rm -rf chat-server; rm -rf chat-client

******************************************************************************

Makefile 2

all: server client

server: chat-server.c
	gcc -pthread chat-server.c -o chat-server

client: chat-client.c
	gcc -pthread chat-client.c -o chat-client

clean:
	rm -rf chat-server; rm -rf chat-client
*/