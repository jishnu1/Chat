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
	int sockfd;
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

		write(args1->sockfd, msg, strlen(msg));

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
			recv(args1->sockfd, msg, MESSAGE_LENGTH, 0);
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
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0)
	{
        printf("ERROR: socket creation failed\n"); 
        exit(1); 
    }

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(PORT);

	connfd = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if (connfd < 0)
	{
		printf("ERROR: connection failed\n");
        exit(1);
	}

	printf("============== WELCOME ==============\n");
	printf("Type \"exit\" to exit\n");
	printf("Please wait...\n");

	recv(sockfd, other_name, NAME_LENGTH, 0);
	printf("You are connected with %s\n", other_name);
	
	do
	{
		printf("Enter your name: ");
		fgets(my_name, NAME_LENGTH, stdin);
		trim(my_name, strlen(my_name));
	}
	while (strlen(my_name) < 1);

	write(sockfd, my_name, strlen(my_name));

	Args * args = (Args *) malloc(sizeof(Args));
	args->sockfd = sockfd;

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
	close(sockfd);
	return 0;
}