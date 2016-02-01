#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "list.h"
#include "game.h"

/*
 * Adds message to a list or creates new one
 *
 *
 * l: list into which a message should be added
 *
 * message: message which should be added to a list
 *
 * player: player for whom the message if
 *
 * new_time: 1 if new time should be stored in list
 * */
void add_message(struct list **l, struct message message, struct player *player, int new_time) {
	struct list *new;
	struct timespec time;

	new = malloc(sizeof(struct list));
	new->message = message;
	new->player = player;
	if (new_time == 1) {
		clock_gettime(CLOCK_REALTIME, &time);
		new->sent_time = time.tv_sec;
	}
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

/*
 * Gets first message from a list and removes it
 *
 *
 * l: list of messages
 *
 *
 * return: first message from a list
 * */
struct message *get_message(struct list **l) {
	if (*l != NULL) {
		struct message *m = &((*l)->message);
		*l = (*l)->next;
		return m;
	}
	return NULL;
}

/*
 * Goes through sent messages and removes message which is acknowledged by ack
 *
 *
 * sent_messages: list of sent messages
 *
 * ack: acknowledgement for a message
 * */
void ack_message(struct game **games, struct list **sent_messages, struct message *ack) {
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
			if (iter->player->game == -1) {
				free(iter->player->name);
				free(iter->player);
			} else if (iter->message.type == 12 || iter->message.type == 13) {
				int j;
				struct list *look;

				iter->next = NULL;
				if (prev == NULL) *sent_messages = next;

				j = 0;
				look = *sent_messages;
				while (look != NULL) {
					if (look->player->game == iter->player->game) {
						j++;
						break;
					}
					look = look->next;
				}
				if (j == 0) {
					remove_game(games, iter->player->game, sent_messages);
				}
				free_list(iter);
				break;
			}
			iter->next = NULL;
			free_list(iter);
			if (prev == NULL) *sent_messages = next;
			break;
		}
		prev = iter;
		iter = iter->next;
	}
}

/*
 * Frees list
 *
 *
 * l: list to be freed
 * */
void free_list(struct list *l) {
	if (l->next != NULL) {
		free_list(l->next);
	}
	free(l);
}