#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "list.h"

void add_message(struct list **l, struct message message, struct player *player) {
	struct list *new;

	new = malloc(sizeof(struct list));
	new->message = message;
	new->player = player;
	new->sent_time = clock();
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

void ack_message(struct list **sent_messages, struct message *ack) {
	struct list *iter, *prev = NULL, *next = NULL;

	iter = *sent_messages;
	while (iter != NULL) {
		int ack_number = atoi(ack->data);

		if (iter->message.number == ack_number && strcmp(iter->message.nick, ack->nick) == 0) {
			if (prev != NULL) prev->next = iter->next;
			else next = iter->next;
			iter->next = NULL;
			free_list(iter);
			if (prev == NULL) iter = next;
			break;
		}
		prev = iter;
		iter = iter->next;
	}
}

void free_list(struct list *l) {
	if (l->next != NULL) {
		free_list(l->next);
	}
	free(l);
}