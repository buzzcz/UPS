#ifndef GAME
#define GAME

struct player *create_player(struct sockaddr_in client_addr, socklen_t client_addr_length, int opponents, char *name);

struct player *find_player(struct game **games, char *nick);

char *get_word();

char *check_guess(struct game *game, struct message received, size_t data_size);

struct game *add_player_to_game(struct game **games, struct player *player);

struct game *find_game(struct game **games, int id);

void remove_game(struct game **games, int id);

#endif