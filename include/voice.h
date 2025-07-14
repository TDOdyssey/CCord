#ifndef VOICE_H
#define VOICE_H

#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "cJSON.h"
#include "websocket.h"

#define CCORD_VOICE_GATEWAY_DEFAULT_URL "wss://gateway.discord.gg/?v=10&encoding=json"
#define CCORD_VOICE_GATEWAY_HEARTBEAT_FORMAT "{\"op\":1,\"d\":%s}"

typedef struct {
    websocket_t *ws;
    const char *server_id;
    const char *user_id;
    const char *session_id;
    const char *token;
    const char *endpoint;

    bool closed;
    bool hello;
    bool ack;
    bool ready;

    int heartbeat;
    long next_heartbeat;

    int nonce;

    int ssrc;
    int port;
    char ip[256];

    int udp_socket;
    struct sockaddr_in discord_addr;
    struct sockaddr_in my_addr;

    uint8_t secret_key[32];
} voice_gateway_t;

voice_gateway_t voice_gateway_init(const char *server_id, const char *user_id, const char *session_id, const char *token, const char *endpoint);

void voice_gateway_update(voice_gateway_t *gw);

void voice_gateway_close(voice_gateway_t *gw);

#endif