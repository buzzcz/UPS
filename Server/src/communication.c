#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "communication.h"

char *receive_message(int server_socket, struct sockaddr_in *client_addr, socklen_t *client_addr_length) {
	int read_bytes;
	char *received;

	*client_addr_length = sizeof(*client_addr);
	read_bytes = (int) recvfrom(server_socket, received, 100, 0, (struct sockaddr *) client_addr, client_addr_length);
	*client_addr_length = sizeof(*client_addr);
	received[read_bytes] = '\0';

	return received;
}

char *createAck(int *sent_datagrams, char *received) {
	char *message;
	int confirmed_datagram;

	confirmed_datagram = atoi(strtok(received, ";"));
	message = malloc(32);
	sprintf(message, "%d;2;%d\n", (*sent_datagrams)++, confirmed_datagram);

	return message;
}

void sendAck(int server_socket, struct sockaddr_in client_addr, socklen_t client_addr_length, int *sent_datagrams,
             char *received) {
	char *ack = createAck(sent_datagrams, received);
	sendMessage(server_socket, client_addr, client_addr_length, ack);
}

void sendMessage(int server_socket, struct sockaddr_in client_addr, socklen_t client_addr_length, char *message) {
	printf("Server is sending: %s", message);
	sendto(server_socket, message, strlen(message), 0, (struct sockaddr *) &client_addr, client_addr_length);
}