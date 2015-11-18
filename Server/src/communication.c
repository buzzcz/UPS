#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "communication.h"

/*
 * Peeks at the message and returns the size of the data
 *
 * server_socket: server socket to be used for receiving a message
 * client_addr: saves the client's attributes
 * client_addr_length: length of client's attributes
 *
 * return: size of the received data
 * */
int peek_message(int server_socket, struct sockaddr_in *client_addr, socklen_t *client_addr_length) {
	int size;
	char *text, *tmp;

	text = malloc(PEEK_SIZE);
	*client_addr_length = sizeof(client_addr);
	recvfrom(server_socket, text, PEEK_SIZE, MSG_WAITALL | MSG_PEEK, (struct sockaddr *) client_addr,
	         client_addr_length);
	*client_addr_length = sizeof(client_addr);

	strtok(text, ";");
	strtok(NULL, ";");
	tmp = strtok(NULL, ";");

	size = atoi(tmp);

	free(text);

	return size;
}

/*
 * Receives a message from a client
 *
 * server_socket: server socket to be used for receiving a message
 * client_addr: saves the client's attributes
 * client_addr_length: length of client's attributes
 *
 * return: received message
 * */
struct message receive_message(int server_socket, struct sockaddr_in *client_addr, socklen_t *client_addr_length) {
	int read_bytes;
	char *received, *tmp;
	struct message m;

	m.size = peek_message(server_socket, client_addr, client_addr_length);
	m.data = malloc((size_t) m.size);
	received = malloc((size_t) (PEEK_SIZE + m.size + 1));
	*client_addr_length = sizeof(*client_addr);
	read_bytes = (int) recvfrom(server_socket, received, (size_t) (PEEK_SIZE + m.size), 0,
	                            (struct sockaddr *) client_addr, client_addr_length);
	*client_addr_length = sizeof(*client_addr);
	received[read_bytes] = '\0';

	m.number = atoi(strtok(received, ";"));
	m.type = atoi(strtok(NULL, ";"));
	strtok(NULL, ";");
	tmp = strtok(NULL, ";");
	strcpy(m.data, tmp);

	free(received);

	return m;
}

/*
 * Creates acknowledgement message to the received message
 *
 * sent_datagrams: number of actually sending datagram
 * received: message to acknowledgement to
 *
 * return: acknowledgement message to the received message
 * */
struct message createAck(int *sent_datagrams, struct message received) {
	struct message ack;

	ack.number = (*sent_datagrams)++;
	ack.type = 2;
	if (received.number < 10) ack.size = 1;
	else if (received.number < 100) ack.size = 2;
	else if (received.number < 1000) ack.size = 3;
	else if (received.number < 10000) ack.size = 4;
	else if (received.number < 100000) ack.size = 5;
	else if (received.number < 1000000) ack.size = 6;
	else if (received.number < 10000000) ack.size = 7;
	else if (received.number < 100000000) ack.size = 8;
	else if (received.number < 1000000000) ack.size = 9;
	else ack.size = 10;
	ack.data = malloc((size_t) ack.size);
	sprintf(ack.data, "%d", received.number);

	return ack;
}

/*
 * Sends message to a client
 *
 * server_socket: server socket to be used for sending
 * client_addr: client's attributes
 * client_addr_length: client's attributes length
 * message: message to be sent
 * */
void sendMessage(int server_socket, struct sockaddr_in client_addr, socklen_t client_addr_length, struct message m) {
	char *message;

	message = malloc((size_t) (PEEK_SIZE + m.size));
	sprintf(message, "%d;%d;%d;%s\n", m.number, m.type, m.size, m.data);
	printf("Server is sending: %s", message);
	sendto(server_socket, message, strlen(message), 0, (struct sockaddr *) &client_addr, client_addr_length);
}

/*
 * Sends acknowledgement message to the received message
 *
 * server_socket: server socket to be used for sending
 * client_addr: client's attributes
 * client_addr_length: client's attributes length
 * sent_datagrams: number of actually sending datagram
 * received: received message to send acknowledgement to
 * */
void sendAck(int server_socket, struct sockaddr_in client_addr, socklen_t client_addr_length, int *sent_datagrams,
             struct message received) {
	struct message ack;

	ack = createAck(sent_datagrams, received);
	sendMessage(server_socket, client_addr, client_addr_length, ack);
}

/*
 * Method that responds to a received message
 *
 * sent_datagrams: number of actually sending datagram
 * received: received message to respond to
 *
 * return: 0 if a response has been sent, 1 if a response has been sent and another message should come from a client and -1 otherwise
 * */
int respond(int server_socket, struct sockaddr_in client_addr, socklen_t client_addr_length, int *sent_datagrams,
            struct message received) {
//	TODO create logical responses

	sendAck(server_socket, client_addr, client_addr_length, sent_datagrams, received);

	switch (received.type) {
		case 1:
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			break;
		case 7:
			break;
		case 8:
			break;
		case 9:
			break;
		case 10:
			break;
		case 11:
			break;
		case 12:
			break;
		case 13:
			break;
		case 14:
			break;
		case 15:
			break;
		case 16:
			break;
		case 17:
			break;
		case 18:
			break;
		case 19:
			break;
		case 20:
			break;
		case 21:
			break;
		default:
			break;
	}

	return -1;
}
