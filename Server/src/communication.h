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

/*
 * Structure for messages
 * */
struct message {
	/*Sequence number of datagram*/
	int number;
	/*Type of data in datagram*/
	int type;
	/*Size of data in datagram*/
	int size;
	/*Data in datagram*/
	char *data;
};

struct message receive_message(int server_socket, struct sockaddr_in *client_addr, socklen_t *client_addr_length);
int respond(int server_socket, struct sockaddr_in client_addr, socklen_t client_addr_length, int *sent_datagrams,
            struct message received);

#endif
