#include "structures.h"

#ifndef LIST_H
#define LIST_H

void add_message(struct list **l, struct message message);

struct message *find_and_remove_message(struct list **l, char *player);

void free_list(struct list *l);

#endif