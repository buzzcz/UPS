#ifndef STRUCTURES
#define STRUCTURES

#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>

/*Constant for timeout before recvfrom stops waiting for data*/
#define TIMEOUT 5
/*Constant for how long a message should stay in sent messages list until it is resent*/
#define TIME_TO_ACK 5
/*Constant for how many consumer-like threads should run*/
#define NUMBER_OF_THREADS 3
/*Constant for a buffer size of received / sent message*/
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
	/*Player's connection attributes*/
	struct sockaddr_in client_addr;
	/*Player's connection attributes length*/
	socklen_t client_addr_length;
};

/*
 * Structure for a list of messages
 * */
struct list {
	/*Message*/
	struct message message;
	/*Time when the message has been sent*/
	time_t sent_time;
	/*Player for whom the message is*/
	struct player *player;
	/*Next message in list*/
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
	/*Player state
	 *
	 *
	 * 0: not responding
	 *
	 * 1: ok
	 * */
	int state;
	/*Number of opponents player wants to play with*/
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
	 *
	 * 0: waiting for opponent
	 *
	 * 1: in progress
	 *
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

/*
 * Structure for a data which threads need to process messages
 * */
struct thread_data {
	/*Socket to be used for receiving and sending messages*/
	int server_socket;
	/*List of games*/
	struct game **games;
	/*List of received messages*/
	struct list **buffer;
	/*List of sent messages*/
	struct list **sent_messages;
	/*Semaphore for indicating new message in buffer*/
	sem_t *sem;
	/*Mutex for mutual exclusion when working with shared variables*/
	pthread_mutex_t *mutex;
};

#endif
