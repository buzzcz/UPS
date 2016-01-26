#ifndef STRUCTURES
#define STRUCTURES

#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>

/*
 * Constant for size of the datagram "header"
 * */
#define PEEK_SIZE 30
/*
 * Constant for timeout before recvfrom stops waiting for data
 * */
#define TIMEOUT 5
#define TIME_TO_ACK 5000
#define NUMBER_OF_THREADS 3
#define BUFFER_SIZE 65000

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
	/*Player's nickname*/
	char *nick;
	struct sockaddr_in client_addr;
	socklen_t client_addr_length;
};

struct list {
	struct message message;
	clock_t sent_time;
	struct player *player;
	struct list *next;
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
	clock_t last_received;
	/*Number of opponents player wants to play*/
	int opponents;
	/*Id of game player is in*/
	int game;
	/*Number of wrongly guessed letters*/
	int wrong_guesses;
	/*Nickname of a player*/
	char *name;
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
	/*For how many players is the game waiting*/
	int wait_for;
	/*Index of player who made a move/is supposed to make a move*/
	int players_move;
	/*Already guessed letters*/
	char guessed_letters[28];
	/*Next game in a list*/
	struct game *next;
	/*Word that is being guessed*/
	char *guessed_word;
	/*Number of filled positions in word*/
	int filled_word;
	/*Field of players in a game*/
	struct player **players;
};

struct thread_data {
	int server_socket;
	struct game **games;
	struct list **buffer;
	struct list **sent_messages;
	sem_t *sem;
	pthread_mutex_t *mutex;
};

#endif
