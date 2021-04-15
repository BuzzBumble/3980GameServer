#ifndef _RESPONSE_H
#define _RESPONSE_H

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

#define RES_TYPE_SIZE 1
#define RES_CONTEXT_SIZE 1
#define RES_LEN_SIZE 1
#define RES_PAYLOAD_PID_SIZE 4

#define RES_TYPE_SUCCESS 10
#define RES_TYPE_UPDATE 20
#define RES_TYPE_CLIERR_REQUEST 30
#define RES_TYPE_CLIERR_UID 31
#define RES_TYPE_CLIERR_TYPE 32
#define RES_TYPE_CLIERR_CONTEXT 33
#define RES_TYPE_CLIERR_PAYLOAD 34
#define RES_TYPE_SERVERR 40
#define RES_TYPE_GAMERR_ACTION 50
#define RES_TYPE_GAMERR_TURN 51

#define RES_UPDATE_START 1
#define RES_UPDATE_MOVE 2
#define RES_UPDATE_END 3
#define RES_UPDATE_DC 4
#define RES_SUCCESS_RULESET 1

typedef struct {
    uint8_t type;
    uint8_t context;
    uint8_t plen;
    uint32_t payload;
} Response;

int sendResponse(Response *res, int cfd);

#endif