#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include "structures.h"
#include "communication.h"
#include "game.h"
#include "server.h"
#include "list.h"

/*Consumer-like threads for processing messages*/
pthread_t th[NUMBER_OF_THREADS];
/*Data which threads need for processing messages*/
struct thread_data thread_data;
/*List of games*/
struct game *games;
/*Buffer for received messages*/
struct list *buffer;
/*Buffer for sent messages*/
struct list *sent_messages;
/*Semaphore for indicating new message in buffer*/
sem_t sem;
/*Mutex for mutual exclusion when working with shared variables*/
pthread_mutex_t mutex;

/*
 * Creates new server socket and sets server attributes
 *
 *
 * server_socket: server socket to be created
 *
 * addr: address to listen at
 *
 * port: port to listen at
 *
 *
 * return: server attributes
 * */
struct sockaddr_in create_server_socket(int *server_socket, char *addr, int port) {
	struct sockaddr_in server_addr;
	int i;

	*server_socket = socket(AF_INET, SOCK_DGRAM, 0);

	memset(&server_addr, 0, sizeof(struct sockaddr_in));

	for (i = 0; i < strlen(addr); i++) {
		addr[i] = (char) toupper(addr[i]);
	}

	server_addr.sin_family = AF_INET;
	if (strcmp(addr, "INADDR_ANY") == 0) server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	else if (strcmp(addr, "LOCALHOST") == 0) server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	else server_addr.sin_addr.s_addr = inet_addr(addr);
	server_addr.sin_port = htons(port);

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
		printf("Bind ERROR\n");
		exit(-1);
	} else {
		printf("Bind OK\n");
	}
}

/*
 * Prints statistics
 * */
void print_stats() {
	printf("\nReceived: %d\nUnparseable: %d\nSent: %d\nResent: %d\n\n", number_of_received, number_of_unparseable,
	       number_of_sent, number_of_resent);
}

/*
 * Stops threads and exits server
 *
 *
 * signal: signal that killed it
 * */
void exit_handler(int signal) {
	int i;

	print_stats();
	printf("Canceling threads...\n");
	for (i = 0; i < NUMBER_OF_THREADS; i++) {
		pthread_cancel(th[i]);
	}
	printf("Exiting...\n");
	exit(0);
}

/*
 * Sets timeout to recvfrom and initializes global variables
 *
 *
 * server_socket: server socket on which the timeout should be set
 * */
void init_server(int server_socket) {
	struct timeval timeout;
	timeout.tv_sec = TIMEOUT;
	timeout.tv_usec = 0;

	if (setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) != 0) {
		printf("Set timeout error\n");
		exit(1);
	}

	games = NULL;
	buffer = NULL;
	sent_messages = NULL;
	sem_init(&sem, 0, 0);
	pthread_mutex_init(&mutex, NULL);
	number_of_received = 0;
	number_of_unparseable = 0;
	number_of_sent = 0;
	number_of_resent = 0;

	thread_data.server_socket = server_socket;
	thread_data.games = &games;
	thread_data.buffer = &buffer;
	thread_data.sent_messages = &sent_messages;
	thread_data.sem = &sem;
	thread_data.mutex = &mutex;
}

/*
 * Receives a message from a client and tries to parse it
 *
 *
 * server_socket: server socket to be used for receiving a message
 *
 *
 * return: received message or message with type -1 if timeout occurred or with type -2 if it was unparseable
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
		number_of_received++;
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
		if (m.data_size > 0) {
			tmp = strtok(NULL, ";,");
			if (tmp == NULL) {
				m.type = -2;
				return m;
			}
			m.data = malloc(m.data_size * sizeof(char));
			strcpy(m.data, tmp);
		} else m.data = NULL;
		m.client_addr = client_addr;
		m.client_addr_length = client_addr_length;

		return m;
	}
}

/*
 * Checks list of sent messages and resends the one that is there too long. It also ends game if the message is not
 * acknowledged for a certain time
 *
 *
 * server_socket: socket to be used for sending messages
 * */
void check_sent_messages(int server_socket) {
	struct list *iter, *prev, *next;

	iter = sent_messages;
	prev = NULL;
	next = NULL;
	pthread_mutex_lock(&mutex);
	while (iter != NULL) {
		struct timespec time;
		time_t now;

		clock_gettime(CLOCK_REALTIME, &time);
		now = time.tv_sec;
		if (now - iter->sent_time > TIME_TO_ACK) {
			if (now - iter->sent_time > 10 * TIME_TO_ACK) {
				printf("Connection with %s lost, ending game\n", iter->player->name);
				send_unreachable_client(server_socket, &games, iter->player, &sent_messages);
				if (prev != NULL) prev->next = iter->next;
				else next = iter->next;
				iter->next = NULL;
				free_list(iter);
				if (prev == NULL) {
					iter = next;
					sent_messages = next;
				}
				else iter = prev->next;
				continue;
			} else if (now - iter->sent_time > 3 * TIME_TO_ACK) {
				respond_type_8(server_socket, &games, iter->player, &sent_messages);
				iter->player->state = 0;
			}
			if (prev != NULL) prev->next = iter->next;
			else next = iter->next;
			iter->next = NULL;
			send_message(server_socket, iter->player, iter->message, &sent_messages, 0);
			number_of_resent++;
			free_list(iter);
			if (prev == NULL) {
				iter = next;
				sent_messages = next;
			}
			else iter = prev->next;
			continue;
		}
		prev = iter;
		iter = iter->next;
	}
	pthread_mutex_unlock(&mutex);
}

/*
 * Creates and starts consumer-like threads that process messages
 * */
void run_threads() {
	int i;

	for (i = 0; i < NUMBER_OF_THREADS; i++) {
		pthread_create(&th[i], NULL, respond, (void *) &thread_data);
	}
}

/*
 * Method that runs the server's main loop. It reads a message or waits for timeout, checks sent messages, and puts
 * received messages into buffer
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
		} else if (received.type == -2) {
			printf("Can't parse message\n");
			number_of_unparseable++;
		} else {
			pthread_mutex_lock(&mutex);
			add_message(&buffer, received, NULL, 0);
			pthread_mutex_unlock(&mutex);
			sem_post(&sem);
		}
	}
}