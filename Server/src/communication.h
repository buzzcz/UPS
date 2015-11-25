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

struct message receive_message(int server_socket, struct sockaddr_in *client_addr, socklen_t *client_addr_length);
int respond(int server_socket, struct sockaddr_in client_addr, socklen_t client_addr_length, struct message received,
            struct game **games);

#endif
