#ifndef _REQUEST_H
#define _REQUEST_H

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

#define REQ_UID_SIZE 4
#define REQ_TYPE_SIZE 1
#define REQ_CONTEXT_SIZE 1
#define REQ_LEN_SIZE 1

#define REQ_TYPE_CONFIRM 1
#define REQ_TYPE_INFO 2
#define REQ_TYPE_META 3
#define REQ_TYPE_GAME 4

#define REQ_CONFIRM_RULESET 1
#define REQ_GAME_MOVE 1
#define REQ_META_QUIT 1

typedef struct {
    uint32_t cfd;
    uint8_t type;
    uint8_t context;
    uint8_t plen;
    uint8_t *payload;
} Request;

#endif