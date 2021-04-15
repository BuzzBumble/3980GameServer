#include "response.h"

int sendResponse(Response *res, int cfd) {
    write(cfd, &res->type, RES_TYPE_SIZE);
    write(cfd, &res->context, RES_CONTEXT_SIZE);
    write(cfd, &res->plen, RES_LEN_SIZE);
    write(cfd, &res->payload, res->plen);

    return 0;
}

int parseIntoPayload(Response *res, u_int32_t value) {
    uint8_t byte;
    for (size_t i = res->plen -1; i >= 0; i--) {
        byte = value & BYTE_MASK;
        res->payload[i] = htons(byte);
        value >>= 8;
    }
    return 0;
}