#include "game.h"

int GameInitTTT(Game *game, int game_id) {
    game->state = Initializing;
    game->type = GAME_TYPE_TTT;
    game->clients = (Client **)calloc(sizeof(Client *), MAX_PLAYERS_TTT);
    game->num_clients = 0;
    game->max_clients = MAX_PLAYERS_TTT;
    game->id = game_id;
    game->state = AwaitingPlayers;
    return 0;
}

int GameInitRPS(Game *game, int game_id) {
    game->state = Initializing;
    game->type = GAME_TYPE_RPS;
    game->clients = (Client **)calloc(sizeof(Client *), MAX_PLAYERS_RPS);
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
        if (game->type == GAME_TYPE_TTT) {
            if (TTT_GameStart(game) != 0) {
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
    free(game->clients[index]);
    game->clients[index] = 0;
    game->num_clients--;

    game->state = AwaitingPlayers;

    Response res = {};
    res.type = RES_TYPE_UPDATE;
    res.context = RES_UPDATE_DC;
    res.plen = 0;

    for (size_t i = 0; i < game->max_clients; i++) {
        if (game->clients[i] != 0) {
            game->clients[i]->move = -1;
            sendResponse(&res, game->clients[i]->fd);
        }
    }

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

int TTT_GameStart(Game *game) {
    printf("\nStarting TTT Game %u ...\n", game->id);
    game->state = Starting;
    game->moveCount = 0;
    game->lastMove = -1;

    bzero(game->board, sizeof(int) * 9);

    game->clients[0]->team = TTT_TEAM_X;
    game->clients[1]->team = TTT_TEAM_O;

    Response resA = {};
    resA.type = RES_TYPE_UPDATE;
    resA.context = RES_UPDATE_START;
    resA.plen = RES_PAYLOAD_TEAM_SIZE;

    Response resB = {};
    resB.type = RES_TYPE_UPDATE;
    resB.context = RES_UPDATE_START;
    resB.plen = RES_PAYLOAD_TEAM_SIZE;

    resA.payload = calloc(resA.plen, sizeof(uint8_t));
    resB.payload = calloc(resB.plen, sizeof(uint8_t));

    parseIntoPayload(&resA, game->clients[0]->team);
    parseIntoPayload(&resB, game->clients[1]->team);

    sendResponse(&resA, game->clients[0]->fd);
    sendResponse(&resB, game->clients[1]->fd);

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

int TTT_GameAwaitMove(Game *game) {
    printf("\nAwaiting move(s)... \n");
    game->state = AwaitingMove;
    game->clients[0]->state = ClientCanMove;
    game->clients[1]->state = ClientAwaitingMove;

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

    if (RPS_CheckMoves(game) == 0) {
        return 1;
    } 

    return 0;
}

int TTT_HandleMove(Game *game, Request *req) {
    puts("Handling Move...");
    Response res = {};
    res.context = req->type;
    res.plen = 0;

    if (game->state != AwaitingMove) {
        // Should Probably send a response
        puts ("Game is not awaiting move");
        return -1;
    }

    int cfd = req->cfd;
    int c_index = GameClientIndex(game, cfd);
    if (c_index == -1) {
        return -1;
    }

    Client *client = game->clients[c_index];
    Client *otherClient = 0;
    int found = 0;
    for (size_t i = 0; i < game->max_clients; i++) {
        if (!found && game->clients[i] != 0 && game->clients[i] != client) {
            found = 1;
            otherClient = game->clients[i];
        }
    }

    if (otherClient == 0) {
        puts("Could not get other client");
        return 0;
    }

    // If not his turn, send response and stop handling
    if (client->state != ClientCanMove) {
        puts("This client cannot make a move right now");
        res.type = RES_TYPE_GAMERR_TURN;
        sendResponse(&res, client->fd);
        return 0;
    }

    int move = req->payload[0];
    // If invalid index, send response and stop handling
    if (move > 8) {
        res.type = RES_TYPE_GAMERR_ACTION;
        sendResponse(&res, client->fd);
        return 0;
    }

    int cellValue = game->board[move];
    // If this cell has been played, send response and stop handling
    if (cellValue != 0) {
        res.type = RES_TYPE_GAMERR_ACTION;
        sendResponse(&res, client->fd);
        return 0;
    }

    // Then you CAN do that

    // Send Success to client
    game->board[move] = client->team;
    game->lastMove = move;
    game->moveCount++;
    client->state = ClientHasMoved;
    res.type = RES_TYPE_SUCCESS;
    sendResponse(&res, client->fd);

    // Do gameover checking shit before going any further
    if (TTT_CheckMoves(game)) {
        return 1;
    } 

    // Switch players
    otherClient->state = ClientCanMove;
    client->state = ClientAwaitingMove;

    // Send Update to other client
    res.type = RES_TYPE_UPDATE;
    res.context = RES_UPDATE_MOVE;
    res.plen = RES_PAYLOAD_MOVE_SIZE;
    res.payload = malloc(res.plen);

    parseIntoPayload(&res, move);
    sendResponse(&res, otherClient->fd);

    return 0;
}

int RPS_CheckMoves(Game *game) {
    Response resA = {};
    resA.type = RES_TYPE_UPDATE;
    resA.context = RES_UPDATE_END;
    resA.plen = RES_PAYLOAD_END_SIZE;
    resA.payload = malloc(resA.plen * sizeof(uint8_t));

    Response resB = {};
    resB.type = RES_TYPE_UPDATE;
    resB.context = RES_UPDATE_END;
    resB.plen = RES_PAYLOAD_END_SIZE;
    resB.payload = malloc(resB.plen * sizeof(uint8_t));

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

    parseIntoPayload(&resA, payload_a);
    sendResponse(&resA, clientA->fd);
    parseIntoPayload(&resB, payload_b);
    sendResponse(&resB, clientB->fd);

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

// 1 - Game Over
// 0 - Game Not Over
int TTT_CheckMoves(Game *game) {
    Response resA = {};
    resA.type = RES_TYPE_UPDATE;
    resA.context = RES_UPDATE_END;
    resA.plen = RES_PAYLOAD_END_SIZE;
    resA.payload = malloc(resA.plen);

    Response resB = {};
    resB.type = RES_TYPE_UPDATE;
    resB.context = RES_UPDATE_END;
    resB.plen = RES_PAYLOAD_END_SIZE;
    resB.payload = malloc(resB.plen);

    uint16_t payloadA;
    uint16_t payloadB;

    int winner = TTT_CheckWin(game->board);
    if (winner) {
        Client *clientA, *clientB;
        clientA = game->clients[0];
        clientB = game->clients[1];

        if (winner == clientA->team) {
            payloadA = 1;
            payloadB = 2;
        } else {
            payloadB = 1;
            payloadA = 2;
        }

        game->state = GameOver;
    } else if (game->moveCount == 9) {
        payloadA = 3;
        payloadB = 3;

        game->state = GameOver;
    }

    if (game->state == GameOver) {
        payloadA <<= 8;
        payloadB <<= 8;
        payloadA += game->lastMove;
        payloadB += game->lastMove;
        parseIntoPayload(&resA, payloadA);
        parseIntoPayload(&resB, payloadB);
        sendResponse(&resA, game->clients[0]->fd);
        sendResponse(&resB, game->clients[1]->fd);
        return 1;
    } else {
        free(resA.payload);
        free(resB.payload);
        return 0;
    }
}

static int TTT_CheckWin(int board[9]) {
    if (board[0] != 0) {
        if (board[0] == board[1] && board[0] == board[2]) {
            return board[0];
        }
        if (board[0] == board[3] && board[0] == board[6]) {
            return board[0];
        }
    }
    if (board[8] != 0) {
        if (board[8] == board[5] && board[9] == board[2]) {
            return board[8];
        }
        if (board[8] == board[7] && board[8] == board[6]) {
            return board[8];
        }
    }
    if (board[4] != 0) {
        if (board[4] == board[0] && board[4] == board[8]) {
            return board[4];
        }
        if (board[4] == board[2] && board[4] == board[6]) {
            return board[4];
        }
        if (board[4] == board[3] && board[4] == board[5]) {
            return board[4];
        }
        if (board[4] == board[1] && board[4] == board[7]) {
            return board[4];
        }
    }

    return 0;
}