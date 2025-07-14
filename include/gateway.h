#ifndef GATEWAY_H
#define GATEWAY_H

#include <stdbool.h>

#include "cJSON.h"
#include "websocket.h"

#define CCORD_GATEWAY_DEFAULT_URL "wss://gateway.discord.gg/?v=10&encoding=json"
#define CCORD_GATEWAY_HEARTBEAT_FORMAT "{\"op\":1,\"d\":%s}"

#define CCORD_MAX_CALLBACKS 64

typedef void (*gateway_event_cb)(cJSON *payload);

typedef struct {
    const char *event_name;
    gateway_event_cb callback;
} event_cb_registry_entry_t;

typedef struct {
    event_cb_registry_entry_t entries[CCORD_MAX_CALLBACKS];
    int count;
} gateway_event_cb_registry_t;

typedef struct {
    websocket_t *ws;
    const char *token;

    gateway_event_cb_registry_t registry;

    int intents;
    bool closed;

    int heartbeat;
    long next_heartbeat;

    bool hello;
    bool ack;
    int event;
} gateway_t;

gateway_t gateway_init(const char *token, int intents);

void gateway_update(gateway_t *gw);

void gateway_close(gateway_t *gw);

void gateway_register_event(gateway_t *gw, const char *event_name, gateway_event_cb cb);

#endif