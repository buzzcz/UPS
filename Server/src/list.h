#include "structures.h"

#ifndef LIST_H
#define LIST_H

void add_message(struct list **l, struct message message, struct player *player, int new_time);

struct message *get_message(struct list **l);

void ack_message(struct list **sent_messages, struct message *ack);

void free_list(struct list *l);

#endif