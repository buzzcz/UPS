#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include "structures.h"
#include "communication.h"
#include "game.h"
#include "server.h"
#include "list.h"

struct game *games;
struct list *buffer, *sent_messages;
sem_t sem;

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

void exit_handler(int signal) {
	printf("Exiting...\n");
	exit(0);
}

void init_server(int server_socket) {
	struct timeval timeout;
	timeout.tv_sec = TIMEOUT;
	timeout.tv_usec = 0;

	if (setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) != 0) {
		fprintf(stderr, "Set timeout error\n");
		exit(1);
	}

	games = NULL;
	buffer = NULL;
	sent_messages = NULL;
	sem_init(&sem, 0, 0);
}

/*
 * Peeks at the message and returns the size of the data
 *
 *
 * server_socket: server socket to be used for receiving a message
 *
 * client_addr: saves the client's attributes
 *
 * client_addr_length: length of client's attributes
 * */
void peek_message(int *to_read, int server_socket, struct sockaddr_in *client_addr, socklen_t *client_addr_length) {
	char *text, *tmp;

	text = malloc(PEEK_SIZE);
	*client_addr_length = sizeof(client_addr);
	recvfrom(server_socket, text, PEEK_SIZE, MSG_PEEK, (struct sockaddr *) client_addr, client_addr_length);

	if (errno == EWOULDBLOCK || errno == EAGAIN) {
		to_read[0] = -1;
		to_read[1] = -1;
	} else {
		*client_addr_length = sizeof(client_addr);

		tmp = strtok(text, ";");
		if (tmp != NULL) to_read[0] = (int) (strlen(tmp) + 1);
		else {
			to_read[0] = -1;
			to_read[1] = -1;
			return;
		}
		tmp = strtok(NULL, ";");
		if (tmp != NULL) to_read[0] += strlen(tmp) + 1;
		else {
			to_read[0] = -1;
			to_read[1] = -1;
			return;
		}
		tmp = strtok(NULL, ";");
		if (tmp != NULL) to_read[0] += strlen(tmp) + 1;
		else {
			to_read[0] = -1;
			to_read[1] = -1;
			return;
		}
		tmp = strtok(NULL, ";");
		if (tmp != NULL) to_read[0] += strlen(tmp) + 1;
		else {
			to_read[0] = -1;
			to_read[1] = -1;
			return;
		}

		to_read[1] = atoi(tmp);
		to_read[0] += to_read[1];
	}

	free(text);
}

/*
 * Receives a message from a client
 *
 *
 * server_socket: server socket to be used for receiving a message
 *
 * client_addr: saves the client's attributes
 *
 * client_addr_length: length of client's attributes
 *
 *
 * return: received message
 * */
struct message receive_message(int server_socket, struct sockaddr_in *client_addr, socklen_t *client_addr_length) {
	int read_bytes, *to_read;
	char *received, *tmp;
	struct message m;

	to_read = malloc(2 * sizeof(int));
	peek_message(to_read, server_socket, client_addr, client_addr_length);
	read_bytes = to_read[0];
	m.data_size = to_read[1];
	free(to_read);
	if (read_bytes == -1) {
		m.type = -1;
		return m;
	}

	received = malloc((size_t) read_bytes + 1);
	*client_addr_length = sizeof(*client_addr);
	read_bytes = (int) recvfrom(server_socket, received, (size_t) read_bytes, 0, (struct sockaddr *) client_addr,
	                            client_addr_length);
	*client_addr_length = sizeof(*client_addr);
	received[read_bytes] = '\0';

	tmp = strtok(received, ";");
	m.number = atoi(tmp);
	tmp = strtok(NULL, ";");
	m.type = atoi(tmp);
	tmp = strtok(NULL, ";");
	m.checksum = atoi(tmp);
	strtok(NULL, ";");
	tmp = strtok(NULL, ";");
	if (tmp != NULL) {
		char *nick;

		nick = strtok(NULL, ",");
		if (nick != NULL) {
			tmp = strtok(NULL, ",");
			m.data_size -= strlen(nick) + 1;
			m.nick = malloc(strlen(nick) * sizeof(char));
			strcpy(m.nick, nick);
			m.data = malloc(m.data_size * sizeof(char));
		} else {
			m.nick = NULL;
		}
		strcpy(m.data, tmp);
	}
	else m.data = NULL;

	free(received);

	return m;
}

void check_sent_messages(int server_socket) {
	struct list *iter, *prev;

	iter = sent_messages;
	prev = NULL;
	while (iter != NULL) {
		clock_t now;

		now = clock();
		if (now - iter->sent_time > TIME_TO_ACK) {
			struct list *tmp;

			if (prev != NULL) {
				prev->next = iter->next;
			}
			tmp = iter;
			while (tmp->next != NULL) tmp = tmp->next;
			iter->next = NULL;
			iter->sent_time = now;
			send_message(server_socket, iter->player, iter->message);
			tmp->next = iter;
		}
		prev = iter;
		iter = iter->next;
	}
}

void run_threads() {

}

/*
 * Method that runs the server's endless loop
 *
 *
 * server_socket: socket to be used for receiving and sending messages
 * */
void run_server(int server_socket) {
	int sent_datagrams = 1; // TODO remove
	pthread_t *th;
	struct thread_data thread_data;

	signal(SIGINT, exit_handler);
	init_server(server_socket);

//	TODO create and run consumer threads - one general and it starts new thread for each player
	thread_data.server_socket = server_socket;
	thread_data.games = &games;
	thread_data.buffer = &buffer;
	thread_data.sent_messages = &sent_messages;
	thread_data.sem = &sem;

//	TODO proper exit after Ctrl + C - frees, message to clients and so on
	while (1) {
		socklen_t client_addr_length;
		struct sockaddr_in client_addr;
		struct message received;

		received = receive_message(server_socket, &client_addr, &client_addr_length);
		check_sent_messages(server_socket);
		if (received.type != -1) {
			add_message(&buffer, received);
			sem_post(&sem);
		}

//		TODO remove
		if (sent_datagrams < 0) {
			break;
		}
	}
}