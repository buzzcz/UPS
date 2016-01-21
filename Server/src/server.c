#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "structures.h"
#include "communication.h"
#include "game.h"
#include "server.h"
#include "list.h"

pthread_t th[NUMBER_OF_THREADS];
struct thread_data thread_data;
struct game *games;
struct list *buffer, *sent_messages;
sem_t sem;
pthread_mutex_t mutex;

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
//	TODO proper exit after Ctrl + C - frees, message to clients and so on
	int i;

	printf("Canceling threads...\n");
	for (i = 0; i < NUMBER_OF_THREADS; i++) {
		pthread_cancel(th[i]);
	}
	printf("Exiting...\n");
	sleep(3);
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
	pthread_mutex_init(&mutex, NULL);

	thread_data.server_socket = server_socket;
	thread_data.games = &games;
	thread_data.buffer = &buffer;
	thread_data.sent_messages = &sent_messages;
	thread_data.sem = &sem;
	thread_data.mutex = &mutex;
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
struct message receive_message(int server_socket) {
	struct sockaddr_in client_addr;
	socklen_t client_addr_length;
	int read_bytes;
	char received[BUFFER_SIZE], *tmp;
	struct message m;

	read_bytes = BUFFER_SIZE;
	client_addr_length = sizeof(client_addr);
	read_bytes = (int) recvfrom(server_socket, received, (size_t) read_bytes, 0, (struct sockaddr *) &client_addr,
	                            &client_addr_length);

	if (read_bytes == -1 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
		m.type = -1;
		return m;
	} else {
		received[read_bytes] = '\0';
		printf("Client sent: %s\n", received);

		tmp = strtok(received, ";,");
		if (tmp == NULL) {
			m.type = -2;
			return m;
		}
		m.number = atoi(tmp);
		tmp = strtok(NULL, ";,");
		if (tmp == NULL) {
			m.type = -2;
			return m;
		}
		m.type = atoi(tmp);
		tmp = strtok(NULL, ";,");
		if (tmp == NULL) {
			m.type = -2;
			return m;
		}
		m.checksum = atoi(tmp);
		tmp = strtok(NULL, ";,");
		if (tmp == NULL) {
			m.type = -2;
			return m;
		}
		m.data_size = atoi(tmp);
		tmp = strtok(NULL, ";,");
		if (tmp == NULL) {
			m.type = -2;
			return m;
		}
		m.nick = malloc(strlen(tmp) * sizeof(char));
		strcpy(m.nick, tmp);
		m.data_size -= strlen(tmp) + 1;
		tmp = strtok(NULL, ";,");
		if (tmp == NULL) {
			m.type = -2;
			return m;
		}
		if (m.data_size > 0) {
			m.data = malloc(m.data_size * sizeof(char));
			strcpy(m.data, tmp);
		} else m.data = NULL;
		m.client_addr = client_addr;
		m.client_addr_length = client_addr_length;

		return m;
	}
}

void check_sent_messages(int server_socket) {
	struct list *iter, *prev, *next;

	iter = sent_messages;
	prev = NULL;
	next = NULL;
	pthread_mutex_lock(&mutex);
	while (iter != NULL) {
		clock_t now;

		now = clock();
		if (now - iter->sent_time > TIME_TO_ACK) {
			if (now - iter->sent_time > 10 * TIME_TO_ACK) {
				fprintf(stderr, "Connection lost with %s\n", iter->player->name);
//				TODO remove player
			} else if (now - iter->sent_time > 3 * TIME_TO_ACK) {
				struct message m;

				m.number = iter->player->sent_datagrams++;
				m.type = 1;
				m.data_size = 0;
				m.data = NULL;
				calculate_checksum(&m);
				send_message(server_socket, iter->player, m, &sent_messages);
			}
			if (prev != NULL) prev->next = iter->next;
			else next = iter->next;
			iter->next = NULL;
			send_message(server_socket, iter->player, iter->message, &sent_messages);
			free_list(iter);
			if (prev == NULL) iter = next;
			continue;
		}
		prev = iter;
		iter = iter->next;
	}
	pthread_mutex_unlock(&mutex);
}

void run_threads() {
	int i;

	for (i = 0; i < NUMBER_OF_THREADS; i++) {
		pthread_create(&th[i], NULL, respond, (void *) &thread_data);
	}
}

/*
 * Method that runs the server's endless loop
 *
 *
 * server_socket: socket to be used for receiving and sending messages
 * */
void run_server(int server_socket) {

	signal(SIGINT, exit_handler);
	init_server(server_socket);

	run_threads();

	while (1) {
		struct message received;

		received = receive_message(server_socket);
		check_sent_messages(server_socket);
		if (received.type == -1) {
			fprintf(stderr, "Timeout\n");
		} else if (received.type == -2) {
			fprintf(stderr, "Can't parse message\n");
		} else {
			add_message(&buffer, received, NULL);
			sem_post(&sem);
		}
	}
}