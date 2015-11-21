#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "server.h"
#include "communication.h"
#include "game.h"

/*
 * Creates new server socket and sets server attributes
 *
 *
 * server_socket: server socket to be created
 *
 *
 * return: server attributes
 * */
struct sockaddr_in create_server_socket(int *server_socket) {
	struct sockaddr_in server_addr;

	*server_socket = socket(AF_INET, SOCK_DGRAM, 0);

	memset(&server_addr, 0, sizeof(struct sockaddr_in));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
//	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(10000);

	return server_addr;
}

/*
 * Binds server to host and port from server attributes
 *
 *
 * server_socket: server socket to be bound
 *
 * server_addr: server attributes for the socket to be bound to
 *
 * server_addr_length: length of the server attributes
 * */
void bind_server_socket(int server_socket, struct sockaddr_in server_addr, socklen_t server_addr_length) {
	if (bind(server_socket, (struct sockaddr *) &server_addr, server_addr_length) != 0) {
		fprintf(stderr, "Bind ERROR\n");
		exit(-1);
	} else {
		printf("Bind OK\n");
	}
}

/*
 * Method that runs the server's endless loop
 *
 *
 * server_socket: socket to be used for receiving and sending messages
 * */
void run_server(int server_socket) {
	int sent_datagrams = 1;
	struct game *games;

	games = NULL;
	while (1) {
		socklen_t client_addr_length;
		struct sockaddr_in client_addr;
		struct message received;

		printf("Server is waiting for data\n");
		received = receive_message(server_socket, &client_addr, &client_addr_length);
		printf("Client sent: %s\n", received.data);
		respond(server_socket, client_addr, client_addr_length, received, &games);

//		TODO remove
		if (sent_datagrams < 0) {
			break;
		}
	}
}