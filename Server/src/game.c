#include <string.h>
#include <stdlib.h>
#include <stdio.h>
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
	new->last_message = NULL;

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
struct player *find_player(struct game **games, struct sockaddr_in client_addr) {
	struct game *iter;

	iter = *games;
	while (iter != NULL) {
		int i;

		for (i = 0; i < iter->players_count; i++) {
//			TODO compare client address in if-clause
			if (iter->players[i] != NULL &&
			    iter->players[i]->client_addr.sin_addr.s_addr == client_addr.sin_addr.s_addr) {
				return iter->players[i];
			}
		}
		iter = iter->next;
	}
	return NULL;
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
		iter->next = new;
	} else {
		new->id = 0;
		iter = new;
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
			int i, occupied;

			occupied = 0;
			for (i = 0; i < iter->players_count; i++) {
				if (iter->players[i] == NULL) iter->players[i] = player;
				occupied++;
			}
			if (occupied == iter->players_count) {
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
				free(del->guessed_letters);
				for (i = 0; i < del->players_count; i++) {
					free_player(del->players[i]);
				}
				free(del);
				break;
			}
			iter = iter->next;
		}
		if (iter->id == id) {
			for (i = 0; i < iter->players_count; i++) {
				free_player(iter->players[i]);
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
//				TODO if count < 2
				if (game->players_count < 2) {
					remove_game(games, game->id);
				}
				break;
			}
		}
	}
}
