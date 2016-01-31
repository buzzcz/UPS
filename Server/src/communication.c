#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
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
	if (number >= 0 && number < 10) return 1;
	else if (number > -10 && number < 100) return 2;
	else if (number > -100 && number < 1000) return 3;
	else if (number > -1000 && number < 10000) return 4;
	else if (number > -10000 && number < 100000) return 5;
	else if (number > -100000 && number < 1000000) return 6;
	else if (number > -1000000 && number < 10000000) return 7;
	else if (number > -10000000 && number < 100000000) return 8;
	else if (number > -100000000 && number < 1000000000) return 9;
	else return 10;
}

/*
 * Calculates checksum for a message
 *
 *
 * m: message which checksum should be calculated
 * */
void calculate_checksum(struct message *m) {
	int i;

	m->checksum = m->number + m->type + m->data_size;
	for (i = 0; i < m->data_size; i++) {
		m->checksum += m->data[i];
	}
	m->checksum = m->checksum % 256;
}

/*
 * Checks checksum of a message
 *
 *
 * received: message which checksum should be checked
 *
 *
 * return: 1 if checksum is ok, 0 otherwise
 * */
int check_checksum(struct message *received) {
	int i, checksum, size;
	char *check;

	if (received->data_size > 0) {
		check = malloc(strlen(received->nick) + received->data_size + 1);
		sprintf(check, "%s,%s", received->nick, received->data);
	} else {
		check = malloc(strlen(received->nick));
		strcpy(check, received->nick);
	}
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
struct message create_ack(int sent_datagrams, struct message received) {
	struct message ack;

	ack.number = sent_datagrams;
	ack.type = 1;
	ack.data_size = number_length(received.number);
	ack.data = malloc((size_t) ack.data_size + 1);
	sprintf(ack.data, "%d", received.number);
	calculate_checksum(&ack);

	return ack;
}

/*
 * Sends message to a client and adds it to the sent messages list
 *
 *
 * server_socket: server socket to be used for sending
 *
 * player: player to whom the message should be sent
 *
 * m: message to be sent
 *
 * sent_messages: list of sent messages to store the message until it is acknowledged
 *
 * new_time: indicates if new time should be stored in the sent messages list
 * */
void send_message(int server_socket, struct player *player, struct message m, struct list **sent_messages,
                  int new_time) {
	char *message;

	message = malloc(BUFFER_SIZE);
	sprintf(message, "%d;%d;%d;%d;%s", m.number, m.type, m.checksum, m.data_size, m.data);
	printf("Server is sending: %s to %s\n", message, player->name);
	sendto(server_socket, message, strlen(message), 0, (struct sockaddr *) &(player->client_addr),
	       player->client_addr_length);
	if (new_time == 1) add_message(sent_messages, m, player, new_time);
	number_of_sent++;
	bytes_sent += strlen(message);

	free(message);
}

/*
 * Sends acknowledgement message to the received message
 *
 *
 * server_socket: server socket to be used for sending
 *
 * player: player to whom the acknowledgement should be sent
 *
 * received: received message to send acknowledgement to
 * */
void send_ack(int server_socket, struct player *player, struct message received) {
	struct message ack;
	char *message;

	ack = create_ack(player->sent_datagrams, received);
	message = malloc(BUFFER_SIZE);
	sprintf(message, "%d;%d;%d;%d;%s", ack.number, ack.type, ack.checksum, ack.data_size, ack.data);
	printf("Server is sending: %s to %s\n", message, player->name);
	sendto(server_socket, message, strlen(message), 0, (struct sockaddr *) &(player->client_addr),
	       player->client_addr_length);
	number_of_sent++;

	free(ack.data);
	free(message);
}

/*
 * Sends message to a player that a player with the same nick is already in game and is online
 *
 *
 * server_socket: server socket to be used for sending
 *
 * player: player to whom the acknowledgement should be sent
 *
 * sent_messages: list of sent messages to store the message until it is acknowledged
 * */
void respond_type_3_0(int server_socket, struct player *player, struct list **sent_messages) {
	struct message m;

	m.number = player->sent_datagrams++;
	m.type = 4;
	m.data_size = 1;
	m.data = "1";
	calculate_checksum(&m);
	send_message(server_socket, player, m, sent_messages, 1);
}

/*
 * Sends message to a player that a player with the same nick is already in game but is disconnected
 *
 *
 * server_socket: server socket to be used for sending
 *
 * player: player to whom the acknowledgement should be sent
 *
 * sent_messages: list of sent messages to store the message until it is acknowledged
 * */
void respond_type_3_2(int server_socket, struct player *player, struct list **sent_messages) {
	struct message m;

	m.number = player->sent_datagrams++;
	m.type = 4;
	m.data_size = 1;
	m.data = "2";
	calculate_checksum(&m);
	send_message(server_socket, player, m, sent_messages, 1);
}

/*
 * Sends message to a player that it is his turn and to others in game whose turn it is
 *
 *
 * server_socket: server socket to be used for sending
 *
 * game: game in which a turn notifications should be sent
 *
 * sent_messages: list of sent messages to store the message until it is acknowledged
 *
 * next: indicates if it is next player's turn or if the same one's
 * */
void send_your_move(int server_socket, struct game *game, struct list **sent_messages, int next) {
	struct message m;
	int i;

	if (next == 1) {
		if (game->players_move == game->players_count - 1) {
			game->players_move = 0;
		} else game->players_move++;
	}

	while (game->players[game->players_move]->wrong_guesses >= 11) {
		if (game->players_move == game->players_count - 1) {
			game->players_move = 0;
		} else game->players_move++;
	}

	m.number = game->players[game->players_move]->sent_datagrams++;
	m.type = 14;
	m.data_size = 0;
	m.data = NULL;
	calculate_checksum(&m);
	send_message(server_socket, game->players[game->players_move], m, sent_messages, 1);

	m.type = 15;
	m.data_size = (int) strlen(game->players[game->players_move]->name);
	m.data = malloc(m.data_size * sizeof(char) + 1);
	sprintf(m.data, "%s", game->players[game->players_move]->name);
	for (i = 0; i < game->players_count; i++) {
		if (i != game->players_move) {
			m.number = game->players[i]->sent_datagrams++;
			calculate_checksum(&m);
			send_message(server_socket, game->players[i], m, sent_messages, 1);
		}
	}
}

/*
 * Sends message to players that the game should start
 *
 *
 * server_socket: server socket to be used for sending
 *
 * game: a game which starts
 *
 * sent_messages: list of sent messages to store the message until it is acknowledged
 * */
void send_start(int server_socket, struct game *game, struct list **sent_messages) {
	struct message m;
	int i;

	m.type = 7;
	m.data_size = number_length((int) strlen(game->guessed_word));
	m.data = malloc(m.data_size * sizeof(char) + 1);
	sprintf(m.data, "%d", (int) strlen(game->guessed_word));
	for (i = 0; i < game->players_count; i++) {
		m.number = game->players[i]->sent_datagrams++;
		calculate_checksum(&m);
		send_message(server_socket, game->players[i], m, sent_messages, 1);
	}

	free(m.data);

	send_your_move(server_socket, game, sent_messages, 0);
}

/*
 * Sends message to a player if he has been connected to a game or not
 *
 *
 * server_socket: server socket to be used for sending
 *
 * player: player to whom the acknowledgement should be sent
 *
 * games: list of games
 *
 * received: received message
 *
 * sent_messages: list of sent messages to store the message until it is acknowledged
 * */
void respond_type_3(int server_socket, struct player *player, struct game **games, struct message received,
                    struct list **sent_messages) {
	if (player == NULL) {
		int opponents;
		struct message message;
		struct player *new;
		struct game *game;

		opponents = atoi(received.data);

		new = create_player(received.client_addr, received.client_addr_length, opponents, received.nick);
		send_ack(server_socket, new, received);
		game = add_player_to_game(games, new);

		message.number = new->sent_datagrams++;
		message.type = 4;
		message.data_size = 1;
		message.data = "0";
		calculate_checksum(&message);

		send_message(server_socket, new, message, sent_messages, 1);

		if (game != NULL) {
			send_start(server_socket, game, sent_messages);
		}
	} else {
		struct game *game;

		game = find_game(games, player->game);
		if (game->state == 0 || game->state == 1) {
			respond_type_3_0(server_socket, player, sent_messages);
		} else if (game->state == 2) {
			respond_type_3_2(server_socket, player, sent_messages);
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
 *
 * sent_messages: list of sent messages to store the message until it is acknowledged
 * */
void respond_type_5(int server_socket, struct game **games, struct player *player, struct message received,
                    struct list **sent_messages) {
	struct message message;
	struct game *game;
	int size, i, j;
	char *word;

	if (player == NULL) game = NULL;
	else game = find_game(games, player->game);

	if (game != NULL) {
		word = malloc(strlen(game->guessed_word) + 1);
		memset(word, '-', strlen(game->guessed_word));
		game->guessed_word[strlen(game->guessed_word)] = '\0';
		for (i = 0; i < 28; i++) {
			if (game->guessed_letters[i] != '0') {
				for (j = 0; j < strlen(game->guessed_word); j++) {
					if (game->guessed_word[j] == game->guessed_letters[i]) word[j] = game->guessed_letters[i];
				}
			}
		}

		size = number_length(game->state) + 1;
		size += number_length(game->players_count) + 1;
		size += (int) strlen(game->guessed_word) + 1;
		size += 27 + 1;
		size += number_length(player->wrong_guesses);

		message.number = player->sent_datagrams++;
		message.type = 6;
		message.data_size = size;
		message.data = malloc((size_t) size + 1);
		sprintf(message.data, "%d,%d,%s,%s,%d", game->state, game->players_count, word, game->guessed_letters,
		        player->wrong_guesses);
		printf("Data: %s\n", message.data);
		calculate_checksum(&message);

		send_message(server_socket, player, message, sent_messages, 1);

		free(message.data);

		game->wait_for--;
		if (game->wait_for <= 0) {
			game->wait_for = 0;
			game->state = 1;
			send_your_move(server_socket, game, sent_messages, 0);
		}
	} else {
		if (player == NULL) {
			player = malloc(sizeof(struct player));
			player->client_addr = received.client_addr;
			player->client_addr_length = received.client_addr_length;
			player->sent_datagrams = 1;
			player->received_datagrams = 0;
			player->state = 1;
			player->opponents = 0;
			player->game = -1;
			player->wrong_guesses = 0;
			player->name = malloc(strlen(received.nick));
			strcpy(player->name, received.nick);
			send_ack(server_socket, player, received);
		}
		message.number = player->sent_datagrams++;
		message.type = 6;
		message.data_size = 4;
		message.data = "-1,1";
		calculate_checksum(&message);
		send_message(server_socket, player, message, sent_messages, 1);
	}
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
 *
 * sent_messages: list of sent messages to store the message until it is acknowledged
 * */
void respond_type_8(int server_socket, struct game **games, struct player *player, struct list **sent_messages) {
	struct message message;
	struct game *game;
	int i;

	if (player != NULL) {
		message.type = 9;
		message.data_size = (int) strlen(player->name);
		message.data = malloc((size_t) message.data_size + 1);
		sprintf(message.data, "%s", player->name);

		game = find_game(games, player->game);
		if (game->state != 0) game->state = 2;
		game->wait_for++;
		for (i = 0; i < game->players_count; i++) {
			if (game->players[i] != NULL && strcmp(game->players[i]->name, player->name) != 0 &&
			    game->players[i]->state != 0) {
				message.number = game->players[i]->sent_datagrams++;
				calculate_checksum(&message);
				send_message(server_socket, game->players[i], message, sent_messages, 1);
			}
		}

		free(message.data);
	}
}

/*
 * Sends a win message to a player and notification about a winner to others
 *
 *
 * server_socket: server socket to be used for sending
 *
 * player: player who won
 *
 * games: list of games
 *
 * game: a game in which a player has won
 *
 * sent_messages: list of sent messages to store the message until it is acknowledged
 * */
void send_win(int server_socket, struct player *player, struct game **games, struct game *game,
              struct list **sent_messages) {
	struct message message;
	int i;

	message.number = player->sent_datagrams++;
	message.type = 12;
	message.data_size = 0;
	message.data = NULL;
	calculate_checksum(&message);
	send_message(server_socket, player, message, sent_messages, 1);

	message.type = 13;
	message.data_size = (int) strlen(player->name);
	message.data = malloc(message.data_size * sizeof(char) + 1);
	sprintf(message.data, "%s", player->name);
	for (i = 0; i < game->players_count; i++) {
		if (strcmp(game->players[i]->name, player->name) != 0) {
			message.number = game->players[i]->sent_datagrams++;
			calculate_checksum(&message);
			send_message(server_socket, game->players[i], message, sent_messages, 1);
		}
	}

	remove_game(games, game->id, sent_messages);
}

/*
 * Sends a lose message to a player and notification about a loser to others
 *
 *
 * server_socket: server socket to be used for sending
 *
 * player: player who lost
 *
 * game: game in which a player has lost
 *
 * sent_messages: list of sent messages to store the message until it is acknowledged
 * */
void send_lose(int server_socket, struct player *player, struct game *game, struct list **sent_messages) {
	struct message message;
	int i;

	player->wrong_guesses = 11;

	message.number = player->sent_datagrams++;
	message.type = 10;
	message.data_size = 0;
	message.data = NULL;
	calculate_checksum(&message);
	send_message(server_socket, player, message, sent_messages, 1);

	message.type = 11;
	message.data_size = (int) strlen(player->name);
	message.data = malloc(message.data_size * sizeof(char) + 1);
	sprintf(message.data, "%s", player->name);
	for (i = 0; i < game->players_count; i++) {
		if (strcmp(game->players[i]->name, player->name) != 0) {
			message.number = game->players[i]->sent_datagrams++;
			calculate_checksum(&message);
			send_message(server_socket, game->players[i], message, sent_messages, 1);
		}
	}
}

/*
 * Sends a message with revealed positions to a player and notification about a move to other players in a game.
 * Also send a win, lose and / or next turn messages
 *
 *
 * server_socket: server socket to be used for sending
 *
 * games: list of games
 *
 * player: player who made a move
 *
 * sent_messages: list of sent messages to store the message until it is acknowledged
 *
 * received: message from the player with the move
 * */
void respond_type_16(int server_socket, struct game **games, struct player *player, struct message received,
                     struct list **sent_messages) {
	struct message message;
	struct game *game;
	char *reply;
	int i;

	game = find_game(games, player->game);

	message.number = player->sent_datagrams++;
	message.type = 17;
	message.data_size = (int) strlen(game->guessed_word);
	reply = check_guess(game, received, (size_t) message.data_size);
	message.data = reply;
	calculate_checksum(&message);
	send_message(server_socket, player, message, sent_messages, 1);

	message.type = 18;
	message.data_size = (int) strlen(player->name) + 3 + message.data_size;
	message.data = malloc((size_t) message.data_size + 1);
	sprintf(message.data, "%s,%c,%s", player->name, received.data[0], reply);

	for (i = 0; i < game->players_count; i++) {
		if (strcmp(game->players[i]->name, player->name) != 0) {
			message.number = game->players[i]->sent_datagrams++;
			calculate_checksum(&message);
			send_message(server_socket, game->players[i], message, sent_messages, 1);
		}
	}

	if (strchr(reply, '1') == NULL) {
		player->wrong_guesses++;
		i = 1;
	} else i = 0;

	free(reply);
	free(message.data);

	if (strlen(game->guessed_word) == game->filled_word) {
		send_win(server_socket, player, games, game, sent_messages);
	} else if (player->wrong_guesses == 11) {
		send_lose(server_socket, player, game, sent_messages);
		send_your_move(server_socket, game, sent_messages, 1);
	} else {
		send_your_move(server_socket, game, sent_messages, i);
	}
}

/*
 * Sends a notification to other players about a player who guessed a word.
 * Also sends win, lose and / or next turn messages
 *
 *
 * server_socket: server socket to be used for sending
 *
 * games: list of games
 *
 * player: player who guessed the word
 *
 * received: message from the player with guessed word
 *
 * sent_messages: list of sent messages to store the message until it is acknowledged
 * */
void respond_type_19(int server_socket, struct game **games, struct player *player, struct message received,
                     struct list **sent_messages) {
	struct message message;
	struct game *game;
	int i;

	message.type = 20;
	message.data_size = (int) (strlen(player->name) + 1 + received.data_size);
	message.data = malloc((size_t) message.data_size + 1);
	sprintf(message.data, "%s,%s", player->name, received.data);

	game = find_game(games, player->game);
	for (i = 0; i < game->players_count; i++) {
		if (strcmp(game->players[i]->name, player->name) != 0) {
			message.number = game->players[i]->sent_datagrams++;
			calculate_checksum(&message);
			send_message(server_socket, game->players[i], message, sent_messages, 1);
		}
	}

	free(message.data);

	if (strcmp(received.data, game->guessed_word) == 0) {
		send_win(server_socket, player, games, game, sent_messages);
	} else {
		send_lose(server_socket, player, game, sent_messages);
		send_your_move(server_socket, game, sent_messages, 1);
	}
}

void send_not_responding_client(int server_socket, struct game **games, struct player *player,
                                struct list **sent_messages) {
	struct message m;
	struct game *game;
	int i;

	if (player != NULL) {
		m.type = 22;
		m.data_size = (int) strlen(player->name);
		m.data = malloc(m.data_size * sizeof(char) + 1);
		strcpy(m.data, player->name);

		game = find_game(games, player->game);
		if (game != NULL) {
			for (i = 0; i < game->players_count; i++) {
				if (game->players[i] != NULL && strcmp(game->players[i]->name, player->name) != 0) {
					m.number = game->players[i]->sent_datagrams++;
					calculate_checksum(&m);
					send_message(server_socket, game->players[i], m, sent_messages, 1);
				}
			}
			send_your_move(server_socket, game, sent_messages, 0);
		}
		free(m.data);
	}
}

/*
 * Sends a notification to other players about an unreachable player and removes a game
 *
 *
 * server_socket: server socket to be used for sending
 *
 * games: list of games
 *
 * player: player who is unreachable
 *
 * sent_messages: list of sent messages to store the message until it is acknowledged
 * */
void send_unreachable_client(int server_socket, struct game **games, struct player *player,
                             struct list **sent_messages) {
	struct message m;
	struct game *game;
	int i;

	if (player != NULL) {
		m.type = 2;
		m.data_size = (int) strlen(player->name);
		m.data = malloc(m.data_size * sizeof(char) + 1);
		strcpy(m.data, player->name);

		game = find_game(games, player->game);
		if (game != NULL) {
			for (i = 0; i < game->players_count; i++) {
				if (game->players[i] != NULL && strcmp(game->players[i]->name, player->name) != 0) {
					m.number = game->players[i]->sent_datagrams++;
					calculate_checksum(&m);
					send_message(server_socket, game->players[i], m, sent_messages, 1);
				}
			}
			remove_game(games, game->id, sent_messages);
		}
		free(m.data);
	}
}


/*
 * Function that responds to a received message
 *
 *
 * thread_data: data that the thread need for responding to a messages
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
			if (player != NULL &&
			    ((find_game(games, player->game)->state == 2 && received->type == 5) || received->type == 3)) {
				player->sent_datagrams = 1;
				player->received_datagrams = -1;
				player->client_addr = received->client_addr;
				player->client_addr_length = received->client_addr_length;
			}
			if (player == NULL || received->number == player->received_datagrams + 1 || received->type == 1 ||
			    received->type == 21) {
				if (player != NULL) {
					if (received->type != 1) {
						if (received->type != 21) player->received_datagrams++;
						send_ack(server_socket, player, *received);
					}
					if (player->state == 0) {
						player->state = 1;
					}
				}

				switch (received->type) {
					case 1: //Ack
						ack_message(data->sent_messages, received);
						break;
					case 3: // Connect request
						respond_type_3(server_socket, player, games, *received, data->sent_messages);
						break;
					case 5: // Reconnect request
						respond_type_5(server_socket, games, player, *received, data->sent_messages);
						break;
					case 8:    // Disconnecting
						respond_type_8(server_socket, games, player, data->sent_messages);
						break;
					case 16:    // Move
						respond_type_16(server_socket, games, player, *received, data->sent_messages);
						break;
					case 19:    // Guessing the word
						respond_type_19(server_socket, games, player, *received, data->sent_messages);
						break;
					case 21:
						break;
					default:
						printf("Unknown type\n");
						break;
				}
			} else if (player != NULL && received->number <= player->received_datagrams) {
				send_ack(server_socket, player, *received);
			} else {
				printf("Number not correct: %d, expecting: %d\n", received->number,
				       player->received_datagrams + 1);
			}
		} else {
			printf("Wrong checksum\n");
		}
		pthread_mutex_unlock(data->mutex);
		if (received->nick != NULL) free(received->nick);
		if (received->data != NULL) free(received->data);
	}
	pthread_exit(0);
}

void *ping(void *thread_data) {
	struct thread_data *data;
	int server_socket;
	struct game **games;

	data = (struct thread_data *) thread_data;
	server_socket = data->server_socket;
	games = data->games;

	while (1) {
		struct game *iter;

		pthread_mutex_lock(data->mutex);
		iter = *games;
		while (iter != NULL) {
			int i;
			struct message m;

			m.type = 21;
			m.data_size = 0;
			m.data = NULL;
			for (i = 0; i < iter->players_count; i++) {
				if (iter->players[i] != NULL) {
					struct list *list;
					int sent;

					list = *data->sent_messages;
					sent = 0;
					while (list != NULL) {
						if (strcmp(list->player->name, iter->players[i]->name) == 0 && list->message.type == 21) {
							send_message(server_socket, list->player, list->message, data->sent_messages, 0);
							sent = 1;
							break;
						}
						list = list->next;
					}

					if (sent == 0) {
						m.number = iter->players[i]->sent_datagrams;
						calculate_checksum(&m);
						send_message(server_socket, iter->players[i], m, data->sent_messages, 1);
					}
				}
			}
			iter = iter->next;
		}
		pthread_mutex_unlock(data->mutex);
		sleep(TIME_TO_ACK + 1);
	}
	pthread_exit(0);
}