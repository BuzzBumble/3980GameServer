#include "response.h"

int sendResponse(Response *res, int cfd) {
    printf("OUTGOING RESPONSE TO %d:\n", cfd);
    printf("\tType: %u\n", res->type);
    printf("\tContext: %u\n", res->context);
    printf("\tPayload Size: %u\n", res->plen);
    printf("\tPayload: [");
    for (size_t i = 0; i < res->plen; i++) {
        printf("%u, ", res->payload[i]);
    }
    printf("]\n");

    write(cfd, &res->type, RES_TYPE_SIZE);
    write(cfd, &res->context, RES_CONTEXT_SIZE);
    write(cfd, &res->plen, RES_LEN_SIZE);
    if (res->plen > 0) {
        write(cfd, res->payload, res->plen);
    }

    free(res->payload);
    return 0;
}

int parseIntoPayload(Response *res, uint32_t value) {
    if (res->plen == 0) {
        puts("parseIntoPayload(): Payload length is 0");
        return 0;
    }
    if (res->plen == 1) {
        res->payload[0] = (uint8_t)value;
        return 0;
    }
    if (res->plen > 2) {
        value = htonl(value);
    } else {
        value = htons(value);
    }
    uint8_t byte;
    size_t i;
    for (i = (res->plen - 1); i > 0; i--) {
        byte = value & BYTE_MASK;
        res->payload[i] = byte;
        value >>= 8;
    }
    byte = value & BYTE_MASK;
    res->payload[i] = byte;

    return res->plen;
}