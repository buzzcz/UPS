#include <netinet/in.h>
#include "communication.h"

#ifndef GAME
#define GAME

/*
 * Structure representing a player
 * */
struct player {
	/*Clients address*/
	struct sockaddr_in client_addr;
	/*Length of client's address*/
	socklen_t client_addr_length;
	/*Number of sent datagrams to a player*/
	int sent_datagrams;
	/*Number of received datagrams from a player*/
	int received_datagrams;
	/*Number of opponents player wants to play*/
	int opponents;
	/*Id of game player is in*/
	int game;
	/*Number of wrongly guessed letters*/
	int wrong_guesses;
	/*Nickname of a player*/
	char *name;
	/*Last sent message*/
	struct message last_message;
};

/*
 * Structure representing a (list of) game(s)
 * */
struct game {
	/*Id of a game*/
	int id;
	/*State of a game
	 *
	 * 0: waiting for opponent
	 * 1: in progress
	 * 2: waiting for disconnected player
	 * */
	int state;
	/*Number of players supposed to be in a game*/
	int players_count;
	/*Next game in a list*/
	struct game *next;
	/*Field of players in a game*/
	struct player *players[];
	/*Word that is being guessed*/
	char *guessed_word;
	/*Guessed letters*/
	char *guessed_letters;
};

struct player *create_player(struct sockaddr_in client_addr, socklen_t client_addr_length, int opponents, char *name);
struct player *find_player(struct game **games, struct sockaddr_in client_addr);
void add_player_to_game(struct game **games, struct player *player);
struct game *find_game(struct game **games, int id);
void remove_game(struct game **games, int id);
void remove_player_from_game(struct game **games, struct game *game, struct player *player);

#endif