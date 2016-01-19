#ifndef COMMUNICATION
#define COMMUNICATION

/*
 * Constant for size of the datagram "header"
 * */
#define PEEK_SIZE 30
/*
 * Constant for timeout before recvfrom stops waiting for data
 * */
#define TIMEOUT 5
#define TIME_TO_ACK 5000
#define NO_ACKS_BEFORE_NO_NET 3
#define NUMBER_OF_THREADS 3

int check_checksum(struct message *received);

void send_message(int server_socket, struct player *player, struct message m, struct list **sent_messages);

void *respond(void *thread_data);

#endif
