#ifndef GAME
#define GAME

struct player *create_player(struct sockaddr_in client_addr, socklen_t client_addr_length, int opponents, char *name);
struct player *find_player(struct game **games, struct sockaddr_in client_addr);
int is_already_logged(struct game **games, char *name);
void add_player_to_game(struct game **games, struct player *player);
struct game *find_game(struct game **games, int id);
void remove_game(struct game **games, int id);
void remove_player_from_game(struct game **games, struct game *game, struct player *player);

#endif