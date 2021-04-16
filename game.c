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
    int added = 0;
    for (size_t i = 0; i < game->max_clients; i++) {
        if (!added && game->clients[i] == 0) {
            game->clients[i] = client;
            client->state =  ClientConnected;
            game->num_clients++;
            added = 1;
        }
    }

    // If the game is full, Start the game
    if (game->num_clients == game->max_clients) {
        if (game->type == GAME_TYPE_RPS) {
            if (RPS_GameStart(game) != 0) {
                puts("Could not start game");
            }
        }
    }

    return game->num_clients;
}

int GameClientIndex(Game *game, int cfd) {
    puts("Finding Client's index in game");
    for (size_t i = 0; i < game->max_clients; i++) {
        if (game->clients[i] != 0 && game->clients[i]->fd == cfd) {
            printf("Found Client %d at index %ld\n", cfd, i);
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

int RPS_GameStart(Game *game) {
    printf("\nStarting RPS Game %u ...\n", game->id);
    game->state = Starting;

    Response res = {};
    res.type = RES_TYPE_UPDATE;
    res.context = RES_UPDATE_START;
    res.plen = 0;

    for (size_t i = 0; i < game->max_clients; i++) {
        if (game->clients[i] != 0) {
            game->clients[i]->move = -1;
            sendResponse(&res, game->clients[i]->fd);
        }
    }

    RPS_GameAwaitMove(game);

    return 0;
}

int RPS_GameAwaitMove(Game *game) {
    printf("\nAwaiting move(s)... \n");
    game->state = AwaitingMove;
    for (size_t i = 0; i < game->max_clients; i++) {
        if (game->clients[i] != 0) {
            game->clients[i]->state = ClientCanMove;
        }
    }

    return 0;
}

int RPS_HandleMove(Game *game, Request *req) {
    Response res = {};
    if (game->state != AwaitingMove) {
        puts ("Game is not awaiting move");
        return -1;
    }

    int cfd = req->cfd;
    int c_index = GameClientIndex(game, cfd);
    if (c_index == -1) {
        return -1;
    }

    Client *client = game->clients[c_index];

    if (client->state != ClientCanMove) {
        puts("This client cannot make a move right now");
        return -1;
    }

    client->move = req->payload[0];
    res.type = RES_TYPE_SUCCESS;
    res.context = req->type;
    res.plen = 0;

    sendResponse(&res, client->fd);

    client->state = ClientHasMoved;

    for (size_t i = 0; i < game->max_clients; i++) {
        if (game->clients[i] != 0 && game->clients[i]->state != ClientHasMoved) {
            return 0;
        }
    }

    game->state = GameOver;
    RPS_CheckMoves(game);

    return 0;
}

int RPS_CheckMoves(Game *game) {
    Response res = {};
    res.type = RES_TYPE_UPDATE;
    res.context = RES_UPDATE_END;
    res.plen = RES_PAYLOAD_END_SIZE;
    res.payload = malloc(res.plen * sizeof(uint8_t));
    if (game->state != GameOver) {
        return -1;
    }

    Client *clientA = game->clients[0];
    Client *clientB = game->clients[1];
    int move_a = clientA->move;
    int move_b = clientB->move;
    uint16_t payload_a;
    uint16_t payload_b;

    int result = RPS_CompareMoves(move_a, move_b);
    if (result == -1) {
        payload_a = 3;
        payload_b = 3;
    } else if (result == 0) {
        payload_a = 1;
        payload_b = 2;
    } else if (result == 1) {
        payload_a = 2;
        payload_b = 1;
    }

    payload_a <<= 8;
    payload_b <<= 8;
    payload_a += move_b;
    payload_b += move_a;

    parseIntoPayload(&res, payload_a);
    sendResponse(&res, clientA->fd);
    parseIntoPayload(&res, payload_b);
    sendResponse(&res, clientB->fd);

    free(res.payload);

    return 0;
}

// 0 - a wins
// 1 - b wins
// -1 - Draw
static int RPS_CompareMoves(int a, int b) {
    if (a == RPS_MOVE_ROCK && b == RPS_MOVE_SCISSORS) {
        return 0;
    }
    if (b == RPS_MOVE_ROCK && a == RPS_MOVE_SCISSORS) {
        return 1;
    }

    if (a > b) {
        return 0;
    } else if (b > a) {
        return 1;
    } else {
        return -1;
    }
}