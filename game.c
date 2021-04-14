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
    game->clients[game->num_clients] = client;
    game->num_clients++;

    return game->num_clients;
}