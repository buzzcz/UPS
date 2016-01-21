#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "structures.h"
#include "game.h"
#include "communication.h"
#include "list.h"

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
	int i, checksum, size;
	char *check;

	check = malloc(strlen(received->nick) + received->data_size + 1);
	sprintf(check, "%s,%s", received->nick, received->data);
	size = (int) strlen(check);
	checksum = received->number + received->type + size;
	for (i = 0; i < size; i++) {
		checksum += check[i];
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
void send_message(int server_socket, struct player *player, struct message m, struct list **sent_messages) {
	char *message;

	message = malloc((size_t) (PEEK_SIZE + m.data_size));
	sprintf(message, "%d;%d;%d;%d;%s", m.number, m.type, m.checksum, m.data_size, m.data);
	printf("Server is sending: %s\n", message);
	sendto(server_socket, message, strlen(message), 0, (struct sockaddr *) &(player->client_addr),
	       player->client_addr_length);
	add_message(sent_messages, m, player);

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
void send_ack(int server_socket, struct player *player, struct message received) {
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

void respond_type_5_0(int server_socket, struct player *player, struct list **sent_messages) {
	struct message m;

	m.number = player->sent_datagrams++;
	m.type = 6;
	m.data_size = 1;
	m.data = malloc(m.data_size * sizeof(char));
	m.data = "1";
	calculate_checksum(&m);
	send_message(server_socket, player, m, sent_messages);

	free(m.data);
}

void respond_type_5_2(int server_socket, struct player *player, struct list **sent_messages) {
	struct message m;

	m.number = player->sent_datagrams++;
	m.type = 6;
	m.data_size = 1;
	m.data = malloc(m.data_size * sizeof(char));
	m.data = "2";
	calculate_checksum(&m);
	send_message(server_socket, player, m, sent_messages);

	free(m.data);
}

void send_start(int server_socket, struct game **games, struct game *game, struct list **sent_messages) {
	struct message m;
	int i;

	m.type = 9;
	m.data_size = number_length((int) strlen(game->guessed_word));
	m.data = malloc(m.data_size * sizeof(char));
	sprintf(m.data, "%d", (int) strlen(game->guessed_word));
	for (i = 0; i < game->players_count; i++) {
		m.number = game->players[i]->sent_datagrams++;
		calculate_checksum(&m);
		send_message(server_socket, game->players[i], m, sent_messages);
	}
}

void respond_type_5(int server_socket, struct sockaddr_in client_addr, socklen_t client_addr_length,
                    struct game **games, struct message received, struct list **sent_messages) {
	struct player *player;

	player = find_player(games, received.nick);
	if (player == NULL) {
		int opponents;
		struct message message;
		struct player *new;
		struct game *game;

		opponents = atoi(received.data);

		new = create_player(client_addr, client_addr_length, opponents, received.nick);
		game = add_player_to_game(games, new);

		message.number = new->sent_datagrams++;
		message.type = 6;
		message.data_size = 1;
		message.data = malloc((size_t) message.data_size);
		sprintf(message.data, "0");
		calculate_checksum(&message);

		send_message(server_socket, new, message, sent_messages);

		free(message.data);

		if (game != NULL) {
			send_start(server_socket, games, game, sent_messages);
		}
	} else {
		struct game *game;

		game = find_game(games, player->game);
		if (game->state == 0 || game->state == 1) {
			respond_type_5_0(server_socket, player, sent_messages);
		} else if (game->state == 2) {
			respond_type_5_2(server_socket, player, sent_messages);
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
void respond_type_7(int server_socket, struct game **games, struct player *player, struct list **sent_messages) {
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

	send_message(server_socket, player, message, sent_messages);

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
void respond_type_10(int server_socket, struct game **games, struct player *player, struct list **sent_messages) {
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
			send_message(server_socket, game->players[i], message, sent_messages);
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
void respond_type_18(int server_socket, struct game **games, struct player *player, struct message received,
                     struct list **sent_messages) {
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
	send_message(server_socket, player, message, sent_messages);

	message.type = 20;
	message.data_size = (int) strlen(player->name) + message.data_size + 1;
	message.data = malloc((size_t) message.data_size);
	sprintf(message.data, "%s,%c,%s", player->name, received.data[0], reply);

	for (i = 0; i < game->players_count; i++) {
		if (strcmp(game->players[i]->name, player->name) != 0) {
			message.number = game->players[i]->sent_datagrams++;
			calculate_checksum(&message);
			send_message(server_socket, game->players[i], message, sent_messages);
		}
	}

	free(reply);
	free(message.data);
}

void respond_type_21_win(int server_socket, struct player *player, struct game *game, struct list **sent_messages) {
	struct message message;
	int i;

	message.number = player->sent_datagrams++;
	message.type = 14;
	message.data_size = 0;
	message.data = NULL;
	calculate_checksum(&message);
	send_message(server_socket, player, message, sent_messages);

	message.type = 15;
	message.data_size = (int) strlen(player->name);
	message.data = malloc(message.data_size * sizeof(char));
	strcpy(message.data, player->name);
	for (i = 0; i < game->players_count; i++) {
		if (strcmp(game->players[i]->name, player->name) != 0) {
			message.number = game->players[i]->sent_datagrams++;
			calculate_checksum(&message);
			send_message(server_socket, game->players[i], message, sent_messages);
		}
	}
}

void respond_type_21_lose(int server_socket, struct player *player, struct game *game, struct list **sent_messages) {
	struct message message;
	int i;

	message.number = player->sent_datagrams++;
	message.type = 12;
	message.data_size = 0;
	message.data = NULL;
	calculate_checksum(&message);
	send_message(server_socket, player, message, sent_messages);

	message.type = 13;
	message.data_size = (int) strlen(player->name);
	message.data = malloc(message.data_size * sizeof(char));
	strcpy(message.data, player->name);
	for (i = 0; i < game->players_count; i++) {
		if (strcmp(game->players[i]->name, player->name) != 0) {
			message.number = game->players[i]->sent_datagrams++;
			calculate_checksum(&message);
			send_message(server_socket, game->players[i], message, sent_messages);
		}
	}
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
void respond_type_21(int server_socket, struct game **games, struct player *player, struct message received,
                     struct list **sent_messages) {
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
			send_message(server_socket, game->players[i], message, sent_messages);
		}
	}

	free(message.data);

	if (strcmp(received.data, game->guessed_word) == 0) {
		respond_type_21_win(server_socket, player, game, sent_messages);
	} else {
		respond_type_21_lose(server_socket, player, game, sent_messages);
	}
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
void *respond(void *thread_data) {
	struct thread_data *data;
	int server_socket;
	struct game **games;
	struct message *received;
	struct player *player;

	data = (struct thread_data *) thread_data;
	server_socket = data->server_socket;
	games = data->games;

	while (1) {
		sem_wait(data->sem);
		pthread_mutex_lock(data->mutex);
		received = get_message(data->buffer);

		if (check_checksum(received) == 1) {
			player = find_player(games, received->nick);
			if (player == NULL || received->number == player->received_datagrams + 1) {
				if (player != NULL && received->type != 5) {
					player->received_datagrams++;
					send_ack(server_socket, player, *received);
				}

				switch (received->type) {
					case 2:    //Ack
						ack_message(data->sent_messages, received);
						break;
					case 5:    // Connect request
						respond_type_5(server_socket, received->client_addr, received->client_addr_length, games,
						               *received,
						               data->sent_messages);
						break;
					case 7:    // Reconnect request
						respond_type_7(server_socket, games, player, data->sent_messages);
						break;
					case 10:    // Disconnecting
						respond_type_10(server_socket, games, player, data->sent_messages);
						break;
					case 18:    // Move
						respond_type_18(server_socket, games, player, *received, data->sent_messages);
						break;
					case 21:    // Guessing the word
						respond_type_21(server_socket, games, player, *received, data->sent_messages);
						break;
					default:
						fprintf(stderr, "Unknown type\n");
						break;
				}
			} else if (received->number <= player->received_datagrams) {
				send_ack(server_socket, player, *received);
			} else {
				fprintf(stderr, "Number not correct\n");
			}
		} else {
			fprintf(stderr, "Wrong checksum\n");
		}
		pthread_mutex_unlock(data->mutex);
		if (received->nick != NULL) free(received->nick);
		if (received->data != NULL) free(received->data);
	}
	pthread_exit(0);
}