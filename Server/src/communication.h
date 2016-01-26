#ifndef COMMUNICATION
#define COMMUNICATION

int number_of_received, number_of_unparseable, number_of_sent, number_of_resent;

int check_checksum(struct message *received);

void calculate_checksum(struct message *m);

void send_message(int server_socket, struct player *player, struct message m, struct list **sent_messages,
                  int new_time);

void respond_type_8(int server_socket, struct game **games, struct player *player, struct list **sent_messages);

void send_unreachable_client(int server_socket, struct game **games, struct player *player,
                             struct list **sent_messages);

void *respond(void *thread_data);

#endif
