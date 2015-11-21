#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "communication.h"
#include "game.h"

/*
 * Peeks at the message and returns the size of the data
 *
 *
 * server_socket: server socket to be used for receiving a message
 *
 * client_addr: saves the client's attributes
 *
 * client_addr_length: length of client's attributes
 *
 *
 * return: size of the received data
 * */
void peek_message(int *to_read, int server_socket, struct sockaddr_in *client_addr, socklen_t *client_addr_length) {
	char *text, *tmp;

	text = malloc(PEEK_SIZE);
	*client_addr_length = sizeof(client_addr);
	recvfrom(server_socket, text, PEEK_SIZE, MSG_PEEK, (struct sockaddr *) client_addr,
	         client_addr_length);
	*client_addr_length = sizeof(client_addr);

	tmp = strtok(text, ";");
	to_read[0] = (int) (strlen(tmp) + 1);
	tmp = strtok(NULL, ";");
	to_read[0] += strlen(tmp) + 1;
	tmp = strtok(NULL, ";");
	to_read[0] += strlen(tmp) + 1;

	to_read[1] = atoi(tmp);
	to_read[0] += to_read[1];

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

	m.data = malloc((size_t) m.data_size);
	received = malloc((size_t) (PEEK_SIZE + m.data_size + 1));
	*client_addr_length = sizeof(*client_addr);
	read_bytes = (int) recvfrom(server_socket, received, (size_t) read_bytes, 0,
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

int number_length(int number) {
	if (number < 10) return 1;
	else if (number < 100) return 2;
	else if (number < 1000) return 3;
	else if (number < 10000) return 4;
	else if (number < 100000) return 5;
	else if (number < 1000000) return 6;
	else if (number < 10000000) return 7;
	else if (number < 100000000) return 8;
	else if (number < 1000000000) return 9;
	else return 10;
}

/*
 * Creates acknowledgement message to the received message
 *
 *
 * sent_datagrams: number of actually sending datagram
 *
 * received: message to acknowledgement to
 *
 *
 * return: acknowledgement message to the received message
 * */
struct message create_ack(int *sent_datagrams, struct message received) {
	struct message ack;

	ack.number = (*sent_datagrams)++;
	ack.type = 2;
	ack.data_size = number_length(received.number);
	ack.data = malloc((size_t) ack.data_size);
	sprintf(ack.data, "%d", received.number);

	return ack;
}

/*
 * Sends message to a client
 *
 *
 * server_socket: server socket to be used for sending
 *
 * client_addr: client's attributes
 *
 * client_addr_length: client's attributes length
 *
 * message: message to be sent
 * */
void send_message(int server_socket, struct player *player, struct message m) {
	char *message;

	message = malloc((size_t) (PEEK_SIZE + m.data_size));
	sprintf(message, "%d;%d;%d;%s", m.number, m.type, m.data_size, m.data);
	printf("Server is sending: %s\n", message);
	sendto(server_socket, message, strlen(message), 0, (struct sockaddr *) &(player->client_addr),
	       player->client_addr_length);
	player->last_message = m;

	free(message);
}

/*
 * Sends acknowledgement message to the received message
 *
 *
 * server_socket: server socket to be used for sending
 *
 * received: received message to send acknowledgement to
 *
 * player: player to whom the acknowledgement should be sent
 * */
void send_ack(int server_socket, struct message received, struct player *player) {
	struct message ack;
	char *message;

	ack = create_ack(&(player->sent_datagrams), received);
	message = malloc((size_t) (PEEK_SIZE + ack.data_size));
	sprintf(message, "%d;%d;%d;%s", ack.number, ack.type, ack.data_size, ack.data);
	printf("Server is sending: %s\n", message);
	sendto(server_socket, message, strlen(message), 0, (struct sockaddr *) &(player->client_addr),
	       player->client_addr_length);

	free(ack.data);
	free(message);
}

/*
 * Sends acknowledgement message to an unknown player
 *
 *
 * server_socket: server socket to be used for sending
 *
 * client_addr: client's attributes
 *
 * client_addr_length: client's attributes length
 *
 * received: received message to send acknowledgement to
 * */
void send_ack_unknown_player(int server_socket, struct sockaddr_in client_addr, socklen_t client_addr_length,
                             struct message received) {
	struct message ack;
	int sent;
	char *message;

	sent = 0;
	ack = create_ack(&sent, received);
	message = malloc((size_t) (PEEK_SIZE + ack.data_size));
	sprintf(message, "%d;%d;%d;%s", ack.number, ack.type, ack.data_size, ack.data);
	printf("Server is sending: %s\n", message);
	sendto(server_socket, message, strlen(message), 0, (struct sockaddr *) &client_addr, client_addr_length);

	free(ack.data);
	free(message);
}

void respond_type_3(int server_socket, struct player *p) {
	if (p != NULL && p->last_message.data != NULL) {
		send_message(server_socket, p, p->last_message);
	} else fprintf(stderr, "Unknown player\n");
}

void respond_type_4(int server_socket, struct sockaddr_in client_addr, socklen_t client_addr_length,
                    struct game **games, struct message received) {
	char *data, *name;
	int opponents;
	struct message message;
	struct player *new;

	data = malloc((size_t) received.data_size);
	strcpy(data, received.data);

	name = strtok(data, ",");
	opponents = atoi(strtok(NULL, ","));

	new = create_player(client_addr, client_addr_length, opponents, name);
	add_player_to_game(games, new);

	free(data);

	message.number = new->sent_datagrams++;
	message.type = 5;
	message.data_size = 1;
	message.data = malloc((size_t) message.data_size);
	sprintf(message.data, "0");

	send_message(server_socket, new, message);

	free(message.data);
}

void respond_type_4_0(int server_socket, struct sockaddr_in client_addr, socklen_t client_addr_length) {
	char *message;

	message = malloc(7 * sizeof(char));
	sprintf(message, "1;5;1;1");
	sendto(server_socket, message, strlen(message), 0, (struct sockaddr *) &client_addr, client_addr_length);
}

void respond_type_4_2(int server_socket, struct sockaddr_in client_addr, socklen_t client_addr_length) {
	char *message;

	message = malloc(7 * sizeof(char));
	sprintf(message, "1;5;1;2");
	sendto(server_socket, message, strlen(message), 0, (struct sockaddr *) &client_addr, client_addr_length);
}

void respond_type_6(int server_socket, struct game **games, struct player *player) {
	struct message message;
	struct game *game;
	int size;

	game = find_game(games, player->game);
	size = (int) strlen(game->guessed_word) + 1;
	size += strlen(game->guessed_letters) + 1;
	size += number_length(player->wrong_guesses);

	message.number = player->sent_datagrams;
	message.type = 7;
	message.data_size = size;
	message.data = malloc((size_t) size);
	sprintf(message.data, "%s,%s,%d", game->guessed_word, game->guessed_letters, player->wrong_guesses);

	send_message(server_socket, player, message);

	free(message.data);
}

void respond_type_9(int server_socket, struct game **games, struct player *player) {
	struct message message;
	struct game *game;
	int i;

	message.type = 10;
	message.data_size = (int) strlen(player->name);
	message.data = malloc((size_t) message.data_size);
	strcpy(message.data, player->name);

	game = find_game(games, player->game);
	for (i = 0; i < game->players_count; i++) {
		message.number = game->players[i]->sent_datagrams;
		send_message(server_socket, game->players[i], message);
	}

	free(message.data);
}

/*
 * Method that responds to a received message
 *
 *
 * server_socket: server socket to be used for sending
 *
 * client_addr: client's attributes
 *
 * client_addr_length: client's attributes length
 *
 * received: received message to respond to
 *
 * games: list of games
 *
 *
 * return: 0 if a response has been sent, 1 if a response has been sent and another message should come from a client and -1 otherwise
 * */
int respond(int server_socket, struct sockaddr_in client_addr, socklen_t client_addr_length, struct message received,
            struct game **games) {
//	TODO create logical responses
	struct player *player;

	player = find_player(games, client_addr);
	if (player != NULL) {
//		TODO check for the correct order of datagrams
		player->received_datagrams++;
		send_ack(server_socket, received, player);
	} else send_ack_unknown_player(server_socket, client_addr, client_addr_length, received);

	switch (received.type) {
		case 2:
			break;
		case 3:
			respond_type_3(server_socket, player);
			break;
		case 4:
			if (player == NULL) {
				respond_type_4(0, client_addr, client_addr_length, games, received);
			} else {
				struct game *g = find_game(games, player->game);
				if (g->state == 0 || g->state == 1) {
					respond_type_4_0(server_socket, client_addr, client_addr_length);
				} else if (g->state == 2) {
					respond_type_4_2(server_socket, client_addr, client_addr_length);
				}
			}
			break;
		case 6:
			respond_type_6(server_socket, games, player);
			break;
		case 9:
			respond_type_9(server_socket, games, player);
			break;
		case 11:
			break;
		case 14:
			break;
		case 18:
			break;
		case 20:
			break;
		default:
			break;
	}

	return -1;
}
