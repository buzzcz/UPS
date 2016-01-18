#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "structures.h"
#include "game.h"
#include "communication.h"
#include "list.h"

struct player *player;

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
	strtok(NULL, ";");
	tmp = strtok(NULL, ";");
	if (tmp != NULL) to_read[0] += strlen(tmp) + 1;
	else {
		to_read[0] = -1;
		to_read[1] = -1;
		return;
	}

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
	if (read_bytes == -1) {
		m.type = -1;
		return m;
	}

	m.data = malloc((size_t) m.data_size);
	received = malloc((size_t) (PEEK_SIZE + m.data_size + 1));
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
//	TODO get player nick from data
	if (tmp != NULL) strcpy(m.data, tmp);
	else m.data = NULL;

	free(received);

	return m;
}

/*
 * Finds out how many digits are there in a number
 *
 *
 * number: number of which "length" should be found
 *
 *
 * return: number of digits in the number
 * */
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

void calculate_checksum(struct message *m) {
	int i;

	m->checksum = m->number + m->type + m->data_size;
	for (i = 0; i < m->data_size; i++) {
		m->checksum += m->data[i];
	}
	m->checksum = m->checksum % 256;
}

int check_checksum(struct message *received) {
	int i, checksum;

	checksum = received->number + received->type + received->data_size;
	for (i = 0; i < received->data_size; i++) {
		checksum += received->data[i];
	}
	checksum = checksum % 256;

	if (checksum == received->checksum) return 1;
	return 0;
}

/*
 * Creates acknowledgement message to the received message
 *
 *
 * sent_datagrams: number of actually sending datagram
 *
 * received: message to create acknowledgement to
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
	calculate_checksum(&ack);

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
	sprintf(message, "%d;%d;%d;%d;%s", m.number, m.type, m.checksum, m.data_size, m.data);
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
void send_ack(int server_socket, struct message received) {
	struct message ack;
	char *message;

	ack = create_ack(&(player->sent_datagrams), received);
	message = malloc((size_t) (PEEK_SIZE + ack.data_size));
	sprintf(message, "%d;%d;%d;%d;%s", ack.number, ack.type, ack.checksum, ack.data_size, ack.data);
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

	sent = 1;
	ack = create_ack(&sent, received);
	message = malloc((size_t) (PEEK_SIZE + ack.data_size));
	sprintf(message, "%d;%d;%d;%d;%s\n", ack.number, ack.type, ack.checksum, ack.data_size, ack.data);
	printf("Server is sending: %s", message);
	sendto(server_socket, message, strlen(message), 0, (struct sockaddr *) &client_addr, client_addr_length);

	free(ack.data);
	free(message);
}

void send_invalid_data(int server_socket) {
	struct message m;

	m.number = player->sent_datagrams++;
	m.type = 3;
	m.data_size = 0;
	m.data = "";
	calculate_checksum(&m);

	send_message(server_socket, player, m);
}

/*
 * Sends last message of a player to him
 *
 *
 * server_socket: server socket to be used for sending
 *
 * p: player to whom should be sent his last message
 * */
void respond_type_4(int server_socket) {
	if (player->last_message.data != NULL) {
		send_message(server_socket, player, player->last_message);
	} else fprintf(stderr, "Last message is null\n");
}

void respond_type_5_0(int server_socket) {
	char *message;

//	TODO checksum
	message = malloc(7 * sizeof(char));
	sprintf(message, "1;6;1;1");
	sendto(server_socket, message, strlen(message), 0, (struct sockaddr *) &(player->client_addr),
	       player->client_addr_length);
}

void respond_type_5_2(int server_socket) {
	char *message;

//	TODO checksum
	message = malloc(7 * sizeof(char));
	sprintf(message, "1;6;1;2");
	sendto(server_socket, message, strlen(message), 0, (struct sockaddr *) &(player->client_addr),
	       player->client_addr_length);
}

void respond_type_5(int server_socket, struct sockaddr_in client_addr, socklen_t client_addr_length,
                    struct game **games, struct message received) {
	char *data, *name;

	data = malloc((size_t) received.data_size);
	strcpy(data, received.data);

	name = strtok(data, ",");
	if (is_already_logged(games, name) == 1) {
		int opponents;
		struct message message;
		struct player *new;

		opponents = atoi(strtok(NULL, ","));

		new = create_player(client_addr, client_addr_length, opponents, name);
		add_player_to_game(games, new);

		message.number = new->sent_datagrams++;
		message.type = 6;
		message.data_size = 1;
		message.data = malloc((size_t) message.data_size);
		sprintf(message.data, "0");
		calculate_checksum(&message);

		send_message(server_socket, new, message);

		free(message.data);
	} else {
		struct game *game = find_game(games, player->game);

		if (game->state == 0 || game->state == 1) {
			respond_type_5_0(server_socket);
		} else if (game->state == 2) {
			respond_type_5_2(server_socket);
		}
	}
}

/*
 * Sends a state of a game to a reconnecting user
 *
 *
 * server_socket: server socket to be used for sending
 *
 * games: list of games
 *
 * player: reconnecting player
 * */
void respond_type_7(int server_socket, struct game **games) {
	struct message message;
	struct game *game;
	int size;


	game = find_game(games, player->game);
	size = (int) strlen(game->guessed_word) + 1;
	size += strlen(game->guessed_letters) + 1;
	size += number_length(player->wrong_guesses);

	player->sent_datagrams = 1;
	player->received_datagrams = 1;

	message.number = player->sent_datagrams++;
	message.type = 8;
	message.data_size = size;
	message.data = malloc((size_t) size);
	sprintf(message.data, "%d,%s,%d", (int) strlen(game->guessed_word), game->guessed_letters, player->wrong_guesses);
	calculate_checksum(&message);

	send_message(server_socket, player, message);

	free(message.data);
}

/*
 * Sends a notification to other players in a game about a disconnecting player
 *
 *
 * server_socket: server socket to be used for sending
 *
 * games: list of games
 *
 * player: disconnecting player
 * */
void respond_type_10(int server_socket, struct game **games) {
	struct message message;
	struct game *game;
	int i;

	message.type = 11;
	message.data_size = (int) strlen(player->name);
	message.data = malloc((size_t) message.data_size);
	strcpy(message.data, player->name);

	game = find_game(games, player->game);
	for (i = 0; i < game->players_count; i++) {
		if (strcmp(game->players[i]->name, player->name) == 0) {
			remove_player_from_game(games, game, player);
		} else {
			message.number = game->players[i]->sent_datagrams++;
			calculate_checksum(&message);
			send_message(server_socket, game->players[i], message);
		}
	}

	free(message.data);
}

/*
 * Sends a message with revealed positions to a player and notification about a move to other players in a game
 *
 *
 * server_socket: server socket to be used for sending
 *
 * games: list of games
 *
 * player: player who made a move
 *
 * received: message from the player with the move
 * */
void respond_type_18(int server_socket, struct game **games, struct message received) {
	struct message message;
	struct game *game;
	char *reply;
	int i;

	game = find_game(games, player->game);

	message.number = player->sent_datagrams++;
	message.type = 19;
	message.data_size = (int) strlen(game->guessed_word);
	reply = check_guess(game, received, (size_t) message.data_size);
	message.data = reply;
	calculate_checksum(&message);
	send_message(server_socket, player, message);

	message.type = 20;
	message.data_size = (int) strlen(player->name) + message.data_size + 1;
	message.data = malloc((size_t) message.data_size);
	sprintf(message.data, "%s,%c,%s", player->name, received.data[0], reply);

	for (i = 0; i < game->players_count; i++) {
		if (strcmp(game->players[i]->name, player->name) != 0) {
			message.number = game->players[i]->sent_datagrams++;
			calculate_checksum(&message);
			send_message(server_socket, game->players[i], message);
		}
	}

	free(reply);
	free(message.data);
}

/*
 * Sends a notification to other players about a player who guessed a word and sends win or lose messages
 *
 *
 * server_socket: server socket to be used for sending
 *
 * games: list of games
 *
 * player: player who guessed the word
 *
 * received: message from the player with guessed word
 * */
void respond_type_21(int server_socket, struct game **games, struct message received) {
	struct message message;
	struct game *game;
	int i;

	message.type = 22;
	message.data_size = (int) (strlen(player->name) + 1 + received.data_size);
	message.data = malloc((size_t) message.data_size);
	sprintf(message.data, "%s,%s", player->name, received.data);

	game = find_game(games, player->game);
	for (i = 0; i < game->players_count; i++) {
		if (strcmp(game->players[i]->name, player->name) != 0) {
			message.number = game->players[i]->sent_datagrams++;
			calculate_checksum(&message);
			send_message(server_socket, game->players[i], message);
		}
	}

	free(message.data);

//	TODO send win or lose message
}

void respond_type_21_win(int server_socket, struct game **games, struct message received) {

}

void respond_type_21_lose(int server_socket, struct game **games, struct message received) {

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
 * return: 0 if a response has been sent, 1 if a response has been sent and another message should come from a client
 * and -1 otherwise
 * */
void *respond_player(void *thread_data) {
	struct thread_data *data;
	int server_socket;
	struct game **games;
	struct message *received;
	int run;

	data = (struct thread_data *) thread_data;
	server_socket = data->server_socket;
	games = data->games;
	run = 1;

	while (run) {
		while ((received = find_and_remove_message(data->buffer, player->name)) == NULL)
			pthread_cond_wait(data->b_cond, data->b);

		if (received->type == -1 || check_checksum(received) != 1) {
			send_invalid_data(server_socket);
		} else if (received->number == player->received_datagrams + 1) {
			player->received_datagrams++;
			send_ack(server_socket, *received);

			switch (received->type) {
				case 3:    // Invalid data
				case 4:    // Resend last message
					respond_type_4(server_socket);
					break;
				case 5:    // Connect request
					respond_type_5(server_socket, player->client_addr, player->client_addr_length, games, *received);
					break;
				case 7:    // Reconnect request
					respond_type_7(server_socket, games);
					break;
				case 10:    // Disconnecting
					respond_type_10(server_socket, games);
					break;
				case 18:    // Move
					respond_type_18(server_socket, games, *received);
					break;
				case 21:    // Guessing the word
					respond_type_21(server_socket, games, *received);
					break;
				default:
					break;
			}
		} else if (received->number <= player->received_datagrams) {
			send_ack(server_socket, *received);
		}
//		TODO free received
	}
	pthread_exit(0);
}