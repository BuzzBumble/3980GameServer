#ifndef _GAMESERVER_H
#define _GAMESERVER_H

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

#define GAMES_COUNT_INC 10

#include "game.h"
#include "request.h"
#include "response.h"

typedef struct {
    uint8_t ptcl_version;
    int tcpfd;
    int udpfd;
    int fdmaxp1;
    int nready;
    int port;
    fd_set m_rset;
    struct sockaddr_in servaddr;
    Game **games;
    int num_games;
} GameServer;

int GameServerInit(GameServer *gs, int ver, int port);
int GameServerLoop(GameServer *gs);
int GameServerAccept(GameServer *gs);
int GameServerHandleUDP(GameServer *gs);
int GameServerHandleRequest(GameServer *gs, Request *req);
int GameServerHandleTCP(GameServer *gs, int cfd);
int HandleConfirm(GameServer *gs, Request *req);
int HandleConfirmRuleset(GameServer *gs, Request *req);
int MakeNewGame(GameServer *gs, int index, int game_type, Client *client);
int FindAvailableGame(GameServer *gs, int game_type, Client *client);
int RemoveClient(GameServer *gs, int cfd);
int HandleGameAction(GameServer *gs, Request *req);
int HandleMove(GameServer *gs, Request *req);
Game *FindGameWithClient(GameServer *gs, uint32_t cfd);
int DestroyGame(GameServer *gs, Game* game);

#endif