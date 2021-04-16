#ifndef _GAME_H
#define _GAME_H

#include <stdint.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "response.h"
#include "request.h"

#define GAME_TYPE_TTT 1
#define GAME_TYPE_RPS 2

#define MAX_PLAYERS_TTT 2
#define MAX_PLAYERS_RPS 2

#define RPS_MOVE_ROCK 1
#define RPS_MOVE_PAPER 2
#define RPS_MOVE_SCISSORS 3

enum GameState {
    Error = -1,
    Initializing = 0,
    AwaitingPlayers,
    Starting,
    AwaitingMove,
    RoundOver,
    GameOver
};

enum ClientState {
    ClientConnected = 0,
    ClientCanMove,
    ClientHasMoved
};

typedef struct {
    int fd;
    int game_id;
    int state;
    int move;
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

int GameAddClient(Game *game, Client *client);
int GameClientIndex(Game *game, int cfd);
int GameRemoveClientAtIndex(Game *game, int index);

int RPS_GameStart(Game *game);
int RPS_HandleMove(Game *game, Request *req);
int RPS_CheckMoves(Game *game);
int RPS_GameAwaitMove(Game *game);
static int RPS_CompareMoves(int a, int b);

#endif