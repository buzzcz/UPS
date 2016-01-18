#include <stdlib.h>
#include <string.h>
#include "list.h"

void add_message(struct list **l, struct message message) {
	struct list *new;

	new = malloc(sizeof(struct list));
	new->message = message;
	new->next = NULL;

	if (*l == NULL) {
		*l = new;
	} else {
		struct list *iter;

		iter = *l;
		while (iter->next != NULL) {
			iter = iter->next;
		}
		iter->next = new;
	}
}

struct message *get_message(struct list **l) {
	if (*l != NULL) {
		struct message *m = &((*l)->message);
		*l = (*l)->next;
		return m;
	}
	return NULL;
}

void free_list(struct list *l) {
	if (l->next != NULL) {
		free_list(l->next);
	}
	free(l);
}