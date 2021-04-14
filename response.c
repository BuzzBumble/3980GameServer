#include "response.h"

int sendResponse(Response *res, int cfd) {
    write(cfd, &res->type, RES_TYPE_SIZE);
    write(cfd, &res->context, RES_CONTEXT_SIZE);
    write(cfd, &res->plen, RES_LEN_SIZE);
    write(cfd, &res->payload, res->plen);

    return 0;
}