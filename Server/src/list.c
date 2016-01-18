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

struct message *find_and_remove_message(struct list **l, char *player) {
	struct list *iter = *l, *prev = NULL;
	while (iter != NULL) {
		if (strstr(iter->message.data, player) != NULL) {
			if (prev != NULL) {
				prev->next = iter->next;
			}
			return &(iter->message);
		}
		prev = iter;
		iter = iter->next;
	}
	return NULL;
}

void free_list(struct list *l) {
	if (l->next != NULL) {
		free_list(l->next);
	}
	free(l);
}