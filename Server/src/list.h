#include "structures.h"

#ifndef LIST_H
#define LIST_H

void add_message(struct list **l, struct message message);

struct message *get_message(struct list **l);

void free_list(struct list *l);

#endif