#include <string.h>
#include <stdlib.h>
#include "game.h"

/*
 * Creates new player
 *
 * opponents: number of opponents player wants to play
 * name: nickname of the player
 *
 * return: new player
 * */
struct player *create_player(struct sockaddr_in client_addr, socklen_t client_addr_length, int opponents, char *name) {
	struct player *p;

	p = malloc(sizeof(struct player));
	p->client_addr = client_addr;
	p->client_addr_length = client_addr_length;
	p->sent_datagrams = 0;
	p->received_datagrams = 0;
	p->opponents = opponents;
	p->game = -1;
	p->wrong_guesses = 0;
	p->name = malloc(strlen(name));
	strcpy(p->name, name);

	return p;
}

/*
 * Removes player from a game
 *
 * games: list of games
 * player: player to be removed
 * */
void remove_player(struct game **games, char *name) {
	struct game *iter;

	iter = *games;
	while (iter != NULL) {
		int i;

		for (i = 0; i < iter->players_count; i++) {
			if (strcmp(iter->players[i]->name, name) == 0) {
				free(iter->players[i]->name);
				free(iter->players[i]);
				iter->players[i] = NULL;
				iter->players_count--;
//				TODO if count < 2
				break;
			}
		}
		iter = iter->next;
	}
}

/*
 * Creates new game and adds it to the list of games
 *
 * games: list of games
 * players_count: number of players supposed to be in the game
 * player: player who creates the game is a first player in it
 * */
void add_game(struct game **games, int players_count, struct player *player) {
	struct game *iter, *new;

	iter = *games;
	new = malloc(sizeof(struct game));
	new->state = 0;
	new->players_count = players_count;
	new->players[0] = player;
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
 * Removes game from the list of games
 *
 * games: list of games
 * id: id of the game to be removed
 * */
void remove_game(struct game **games, int id) {
	struct game *iter;

	iter = *games;
	if (iter != NULL) {
		while (iter->next != NULL) {
			if (iter->next->id == id) {
				struct game *del;

				del = iter->next;
				iter->next = del->next;
				free(del);
				break;
			}
			iter = iter->next;
		}
		if (iter->id == id) {
			free(iter);
			iter = NULL;
		}
	}
}