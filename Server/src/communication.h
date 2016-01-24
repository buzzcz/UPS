#ifndef COMMUNICATION
#define COMMUNICATION

int check_checksum(struct message *received);

void calculate_checksum(struct message *m);

void send_message(int server_socket, struct player *player, struct message m, struct list **sent_messages);

void respond_type_10(int server_socket, struct game **games, struct player *player, struct list **sent_messages);

void *respond(void *thread_data);

#endif
