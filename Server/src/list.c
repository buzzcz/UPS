#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "list.h"

void add_message(struct list **l, struct message message, struct player *player, int new_time) {
	struct list *new;

	new = malloc(sizeof(struct list));
	new->message = message;
	new->player = player;
	if (new_time == 1) new->sent_time = clock();
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
	struct list *iter, *prev, *next;
	int ack_number;

	iter = *sent_messages;
	prev = NULL;
	next = NULL;
	ack_number = atoi(ack->data);
	while (iter != NULL) {
		if (iter->message.number == ack_number && strcmp(iter->player->name, ack->nick) == 0) {
			if (prev != NULL) prev->next = iter->next;
			else next = iter->next;
			iter->next = NULL;
			free_list(iter);
			if (prev == NULL) *sent_messages = next;
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