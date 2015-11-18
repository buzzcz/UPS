#ifndef COMMUNICATION
#define COMMUNICATION

char *receive_message(int server_socket, struct sockaddr_in *client_addr, int *client_addr_length);

void sendAck(int server_socket, struct sockaddr_in client_addr, int client_addr_length, int *sent_datagrams,
             char *received);

void sendMessage(int server_socket, struct sockaddr_in client_addr, int client_addr_length, char *message);

#endif
