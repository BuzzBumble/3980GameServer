#include "game.h"

int GameInitTTT(Game *game, int game_id) {
    game->state = Initializing;
    game->type = GAME_TYPE_TTT;
    game->clients = (Client **)malloc(MAX_PLAYERS_TTT * sizeof(Client *));
    game->num_clients = 0;
    game->max_clients = MAX_PLAYERS_TTT;
    game->id = game_id;
    game->state = AwaitingPlayers;
    return 0;
}

int GameInitRPS(Game *game, int game_id) {
    game->state = Initializing;
    game->type = GAME_TYPE_RPS;
    game->clients = (Client **)malloc(MAX_PLAYERS_RPS * sizeof(Client *));
    game->num_clients = 0;
    game->max_clients = MAX_PLAYERS_RPS;
    game->id = game_id;
    game->state = AwaitingPlayers;
    return 0;
}

int GameAddClient(Game *game, Client *client) {
    if (game->state != AwaitingPlayers) {
        puts("GameAddClient(): Game is not awaiting players");
        return -1;
    }
    if (game->num_clients >= game->max_clients) {
        puts("GameAddClient(): Game is full");
        return -1;
    }

    client->game_id = game->id;
    for (size_t i = 0; i < game->max_clients; i++) {
        if (game->clients[i] == 0) {
            game->clients[i] = client;
            game->num_clients++;
            return game->num_clients;
        }
    }
    puts("GameAddClient(): Could not add client for unknown reason");
    return -1;
}

int GameClientIndex(Game *game, int cfd) {
    puts("Finding Client's index in game");
    for (size_t i = 0; i < game->max_clients; i++) {
        if (game->clients[i] != 0 && game->clients[i]->fd == cfd) {
            printf("Found Client %d at index %d\n", cfd, i);
            return i;
        }
    }
    puts("Could not find client in game");
    return -1;
}

int GameRemoveClientAtIndex(Game *game, int index) {
    printf("Removing client %d from game %d\n", game->clients[index]->fd, game->id);
    game->clients[index] = 0;
    game->num_clients--;

    return game->num_clients;
}