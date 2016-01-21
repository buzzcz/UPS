#ifndef COMMUNICATION
#define COMMUNICATION

int check_checksum(struct message *received);

void calculate_checksum(struct message *m);

void send_message(int server_socket, struct player *player, struct message m, struct list **sent_messages);

void *respond(void *thread_data);

#endif
