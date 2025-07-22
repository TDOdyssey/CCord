#ifndef CCORD_INTERNAL_H
#define CCORD_INTERNAL_H

#include <curl/curl.h>

#include "event.h"

#define MAX_EVENTS_IN_QUEUE 10

typedef struct {
    CCORDeventType type;
    cJSON *data;
} dispatch_event_t;

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

    // Events
    pthread_mutex_t events_lock;
    int events_count;
    dispatch_event_t events[MAX_EVENTS_IN_QUEUE];
} CCORDgateway;

typedef struct CCORDcontext {
    const char *token;

    // Separate curl contexts so we don't have to update headers
    CURL *curl_get;
    CURL *curl_post;
    struct curl_slist *headers;

    // Gateway
    CCORDgateway gateway;
    CCORDgateway voice_gateway;

    generic_handle_t event_handles[CCORD_EVENT_COUNT];

} CCORDcontext;

#endif