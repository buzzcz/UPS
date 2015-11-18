#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "server.h"
#include "communication.h"

struct sockaddr_in create_server_socket(int *server_socket) {
	struct sockaddr_in server_addr;

	*server_socket = socket(AF_INET, SOCK_DGRAM, 0);

	memset(&server_addr, 0, sizeof(struct sockaddr_in));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(10000);

	return server_addr;
}

void bind_server_socket(int server_socket, struct sockaddr_in server_addr, socklen_t server_addr_length) {
	if (bind(server_socket, (struct sockaddr *) &server_addr, server_addr_length) != 0) {
		fprintf(stderr, "Bind ERROR\n");
		exit(-1);
	} else {
		printf("Bind OK\n");
	}
}

void run_server(int server_socket) {
	int sent_datagrams = 1;
	while (1) {
		int client_socket;
		socklen_t client_addr_length;
		struct sockaddr_in client_addr;
		char *received;

		printf("Server is waiting for data\n");
		received = receive_message(server_socket, &client_addr, &client_addr_length);
		printf("Client sent: %s", received);
		sendAck(server_socket, client_addr, client_addr_length, &sent_datagrams, received);

		close(client_socket);

//		TODO remove
		if(sent_datagrams < 0) {
			break;
		}
	}
}