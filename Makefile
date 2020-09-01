all: server client

server: chat-server.c
	gcc -Wall -Werror -fsanitize=address -pthread chat-server.c -o chat-server

client: chat-client.c
	gcc -Wall -Werror -fsanitize=address -pthread chat-client.c -o chat-client

clean:
	rm -rf chat-server; rm -rf chat-client