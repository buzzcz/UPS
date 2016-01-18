#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "structures.h"
#include "game.h"

/*
 * Creates new player
 *
 *
 * opponents: number of opponents player wants to play
 *
 * name: nickname of the player
 *
 *
 * return: new player
 * */
struct player *create_player(struct sockaddr_in client_addr, socklen_t client_addr_length, int opponents, char *name) {
	struct player *new;

	new = malloc(sizeof(struct player));
	new->client_addr = client_addr;
	new->client_addr_length = client_addr_length;
	new->sent_datagrams = 1;
	new->received_datagrams = 1;
	new->opponents = opponents;
	new->game = -1;
	new->wrong_guesses = 0;
	new->name = malloc(strlen(name));
	strcpy(new->name, name);

	return new;
}

/*
 * Frees memory used by a player
 *
 *
 * player: player to be freed
 * */
void free_player(struct player *player) {
	free(player->name);
	free(player);
}

/*
 * Checks if the nickname is already used in a game
 *
 *
 * games: list of games
 *
 * name: name to be checked
 *
 *
 * return: 0 if it was found, 1 otherwise
 * */
int is_already_logged(struct game **games, char *name) {
	struct game *iter;

	iter = *games;
	while (iter != NULL) {
		int i;

		for (i = 0; i < iter->players_count; i++) {
			if (iter->players[i] != NULL && strcmp(iter->players[i]->name, name) == 0) {
				return 0;
			}
		}
		iter = iter->next;
	}
	return 1;
}

/*
 * Finds a player in a game
 *
 *
 * games: list of games
 *
 * client_addr: adress which identifies the player
 *
 *
 * return: player who was found, NULL otherwise
 * */
struct player *find_player(struct game **games, char *nick) {
	struct game *iter;

	iter = *games;
	while (iter != NULL) {
		int i;

		for (i = 0; i < iter->players_count; i++) {
			if (iter->players[i] != NULL &&
			    strcmp(nick, iter->players[i]->name) == 0) {
				return iter->players[i];
			}
		}
		iter = iter->next;
	}
	return NULL;
}

/*
 * Gets random word from the dictionary file
 *
 *
 * return: random word from the dictionary file
 * */
char *get_word() {
	FILE *words;
	char *word;
	int ch, lines, i;

	words = fopen("words.txt", "r");
	lines = 0;
	ch = fgetc(words);
	while (ch != EOF) {
		if (ch == '\n') lines++;
		ch = fgetc(words);
	}

	ch = rand() % lines;
	rewind(words);
	for (i = 0; i < ch; i++) {
		getline(NULL, NULL, words);
	}
	getline(&word, NULL, words);

	fclose(words);
	return word;
}

/*
 * Checks where the guessed word contains the guessed letter
 *
 *
 * game: game in which the player is
 *
 * received: received message
 *
 * data_size: size of data - length of the guessed word
 *
 *
 * return: string with 0 on positions where the letters aren't the same and 1 where they are
 * */
char *check_guess(struct game *game, struct message received, size_t data_size) {
	char *checked, guess;
	int i;

	strtok(received.data, ",");
	guess = strtok(NULL, ",")[0];

	checked = malloc(data_size + 1);
	checked[data_size] = '\0';
	for (i = 0; i < data_size; i++) {
		if (game->guessed_word[i] == guess) checked[i] = 1;
		else checked[i] = 0;
	}

	return checked;
}

/*
 * Creates new game and adds it to the list of games
 *
 *
 * games: list of games
 *
 * players_count: number of players supposed to be in the game
 *
 * player: player who creates the game is a first player in it
 * */
void create_game(struct game **games, int players_count, struct player *player) {
	struct game *iter, *new;
	int i;

	iter = *games;
	new = malloc(sizeof(struct game));
	new->state = 0;
	new->players_count = players_count;
	new->players = malloc(players_count * sizeof(struct player *));
	new->players[0] = player;
	for (i = 1; i < players_count; i++) {
		new->players[i] = NULL;
	}
	new->next = NULL;
	if (iter != NULL) {
		while (iter->next != NULL) {
			iter = iter->next;
		}
		new->id = iter->id + 1;
		new->players[0]->game = new->id;
		new->guessed_word = get_word();
		iter->next = new;
	} else {
		new->id = 0;
		new->players[0]->game = 0;
		*games = new;
	}
}

/*
 * Finds suitable game for a player or creates a new one
 *
 *
 * games: list of games
 *
 * player: player to be added to a game
 * */
void add_player_to_game(struct game **games, struct player *player) {
	struct game *iter;

	iter = *games;
	while (iter != NULL) {
		if (iter->players_count == player->opponents + 1 && iter->state == 0) {
			int i;

			for (i = 0; i < iter->players_count; i++) {
				if (iter->players[i] == NULL) {
					player->game = iter->id;
					iter->players[i] = player;
					break;
				}
			}
			if (i == iter->players_count) {
//				TODO start the game
				iter->state = 1;
			}
			return;
		}
		iter = iter->next;
	}
	create_game(games, player->opponents + 1, player);
}

/*
 * Finds a game according to the given id
 *
 *
 * games: list of games
 *
 * id: id of a game that should be found
 *
 *
 * return: Game that has been found, NULL otherwise
 * */
struct game *find_game(struct game **games, int id) {
	struct game *iter;

	iter = *games;
	while (iter != NULL) {
		if (iter->id == id) {
			return iter;
		}
		iter = iter->next;
	}
	return NULL;
}

/*
 * Removes game from the list of games
 *
 *
 * games: list of games
 *
 * id: id of the game to be removed
 * */
void remove_game(struct game **games, int id) {
	struct game *iter;

	iter = *games;
	if (iter != NULL) {
		int i;

		while (iter->next != NULL) {
			if (iter->next->id == id) {
				struct game *del;

				del = iter->next;
				iter->next = del->next;

				free(del->guessed_word);
				for (i = 0; i < del->players_count; i++) {
					if (del->players[i] != NULL) free_player(del->players[i]);
				}
				free(del->players);
				free(del);
				break;
			}
			iter = iter->next;
		}
		if (iter->id == id) {
			for (i = 0; i < iter->players_count; i++) {
				if (iter->players[i] != NULL) free_player(iter->players[i]);
			}
			free(iter);
			iter = NULL;
		}
	}
}

/*
 * Removes player from a game
 *
 *
 * games: list of games
 *
 * player: player to be removed
 * */
void remove_player_from_game(struct game **games, struct game *game, struct player *player) {
	if (game != NULL) {
		int i;

		for (i = 0; i < game->players_count; i++) {
			if (strcmp(game->players[i]->name, player->name) == 0) {
				free_player(game->players[i]);
				game->players[i] = NULL;
				game->players_count--;
				if (game->players_count < 1) {
					remove_game(games, game->id);
				}
				break;
			}
		}
	}
}
