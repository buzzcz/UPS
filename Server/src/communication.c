#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "communication.h"

/*
 * Receives a message from a client
 *
 * server_socket: server socket to be used for receiving a message
 * client_addr: saves the client's attributes
 * client_addr_length: length of client's attributes
 *
 * return: received message
 * */
char *receive_message(int server_socket, struct sockaddr_in *client_addr, socklen_t *client_addr_length) {
	int read_bytes;
	char *received;

	*client_addr_length = sizeof(*client_addr);
	read_bytes = (int) recvfrom(server_socket, received, 100, 0, (struct sockaddr *) client_addr, client_addr_length);
	*client_addr_length = sizeof(*client_addr);
	received[read_bytes] = '\0';

	return received;
}

/*
 * Creates acknowledgement message to the received message
 *
 * sent_datagrams: number of actually sending datagram
 * received: message to acknowledgement to
 *
 * return: acknowledgement message to the received message
 * */
char *createAck(int *sent_datagrams, char *received) {
	char *message;
	int confirmed_datagram;

	confirmed_datagram = atoi(strtok(received, ";"));
	message = malloc(32);
	sprintf(message, "%d;2;%d\n", (*sent_datagrams)++, confirmed_datagram);

	return message;
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
             char *received) {
	char *ack = createAck(sent_datagrams, received);
	sendMessage(server_socket, client_addr, client_addr_length, ack);
}

/*
 * Method that responds to a received message
 *
 * sent_datagrams: number of actually sending datagram
 * received: received message to respond to
 *
 * return: 0 if a response has been sent, 1 otherwise
 * */
int respond(int *sent_datagrams, char *received) {
//	TODO create logical responds

}

/*
 * Sends message to a client
 *
 * server_socket: server socket to be used for sending
 * client_addr: client's attributes
 * client_addr_length: client's attributes length
 * message: message to be sent
 * */
void sendMessage(int server_socket, struct sockaddr_in client_addr, socklen_t client_addr_length, char *message) {
	printf("Server is sending: %s", message);
	sendto(server_socket, message, strlen(message), 0, (struct sockaddr *) &client_addr, client_addr_length);
}
