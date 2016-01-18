#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include "structures.h"
#include "communication.h"
#include "game.h"
#include "server.h"
#include "list.h"

struct list *buffer, *acks, *server_buffer;
pthread_mutex_t b, a, sb;
pthread_cond_t b_cond, a_cond, sb_cond;

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
//	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
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

void int_handler(int signal) {
	printf("Exiting...\n");
	exit(0);
}

void init_variables() {
	buffer = NULL;
	acks = NULL;
	server_buffer = NULL;
	pthread_mutex_init(&b, NULL);
	pthread_mutex_init(&a, NULL);
	pthread_mutex_init(&sb, NULL);
	pthread_cond_init(&b_cond, NULL);
	pthread_cond_init(&a_cond, NULL);
	pthread_cond_init(&sb_cond, NULL);
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

	signal(SIGINT, int_handler);
	init_variables();

	games = NULL;

//	TODO create and run consumer threads - one general and it starts new thread for each player

//	TODO proper exit after Ctrl + C - frees, message to clients and so on
	while (1) {
		socklen_t client_addr_length;
		struct sockaddr_in client_addr;
		struct message received;

		received = receive_message(server_socket, &client_addr, &client_addr_length);
//		TODO which messages to server buffer??
		if (received.type == 2) {
			add_message(&acks, received);
			pthread_cond_broadcast(&a_cond);
		} else {
			add_message(&buffer, received);
			pthread_cond_broadcast(&b_cond);
		}

//		TODO remove
		if (sent_datagrams < 0) {
			break;
		}
	}
}