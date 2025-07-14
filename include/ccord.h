#ifndef CCORD_H
#define CCORD_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct CCORDcontext CCORDcontext;

typedef enum {
    GET,
    POST
} CCORDrequestType;

CCORDcontext   *ccord_init(const char *token);
int             ccord_login(CCORDcontext *ccord, int intents);
int             ccord_request(CCORDcontext *ccord, CCORDrequestType type, const char *url, const char *send, size_t send_size, char **recv, size_t *recv_size);
void            ccord_free(CCORDcontext *ccord);

#endif