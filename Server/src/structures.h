#ifndef STRUCTURES
#define STRUCTURES

/*
 * Structure for messages
 * */
struct message {
	/*Sequence number of datagram*/
	int number;
	/*Type of data in datagram*/
	int type;
	/*Checksum*/
	int checksum;
	/*Size of data in datagram*/
	int data_size;
	/*Data in datagram*/
	char *data;
};

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
	/*Word that is being guessed*/
	char *guessed_word;
	/*Guessed letters*/
	char *guessed_letters;
	/*Field of players in a game*/
	struct player **players;
};

#endif
