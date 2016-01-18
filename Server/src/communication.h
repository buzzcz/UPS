#ifndef COMMUNICATION
#define COMMUNICATION

/*
 * Constant for size of the datagram "header"
 * */
static const int PEEK_SIZE = 30;
/*
 * Constant for timeout before recvfrom stops waiting for data
 * */
static const int TIMEOUT = 5;
static const int TIME_TO_ACK = 5000;
static const int NUMBER_OF_THREADS = 3;

int check_checksum(struct message *received);

void send_message(int server_socket, struct player *player, struct message m);

void *respond(void *thread_data);

#endif
