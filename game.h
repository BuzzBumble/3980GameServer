#ifndef _GAME_H
#define _GAME_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define GAME_TYPE_TTT 1
#define GAME_TYPE_RPS 2

#define MAX_PLAYERS_TTT 2
#define MAX_PLAYERS_RPS 3

enum GameState {
    Error = -1,
    Initializing = 0,
    AwaitingPlayers,
    AwaitingMove,
    GameOver
};

typedef struct {
    int fd;
    int game_id;
} Client;

typedef struct {
    int type;
    int id;
    int state;
    Client **clients;
    int num_clients;
    int max_clients;
} Game;

int GameInitTTT(Game *game, int game_id);

int GameInitRPS(Game *game, int game_id);

// Add a client to a game
// 
// RETURNS: Number of players in game or -1
int GameAddClient(Game *game, Client *client);
int GameClientIndex(Game *game, int cfd);
int GameRemoveClientAtIndex(Game *game, int index);

#endif