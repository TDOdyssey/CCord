#ifndef GATEWAY_H
#define GATEWAY_H

#include <stdbool.h>

#include "cJSON.h"
#include <curl/curl.h>

typedef enum {
    GW_DEFAULT,
    GW_VOICE
} CCORDgatewayType;

typedef struct {
    CCORDgatewayType type;

    CURL *ws; // Curl WebSocket

    int heartbeat;
    bool close; // set to true if client wants gateway to gracefully shutdown
    
    // Used for handling protocol state
    // Bit  Flag
    // 0    Hello
    // 1    ACK
    // 2    Ready
    uint8_t state_flags;

    pthread_t thread;
    pthread_mutex_t lock;
    
    char endpoint[256];
    char resume[256];

    char token[256];
    int intents;
} CCORDgateway;

int ccord_gateway_init(CCORDgateway *gw, const char *endpoint, const char *token, int intents);

//TODO: void ccord_gateway_close(CCORDgateway *gw);

#endif