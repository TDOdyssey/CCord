#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <stdbool.h>

#include <curl/curl.h>
#include <pthread.h>

#define CC_WEBSOCKET_MAX_FRAMES 10
#define CC_WEBSOCKET_MAX_FRAME_SIZE 9000

typedef struct {
    char frames[CC_WEBSOCKET_MAX_FRAMES][CC_WEBSOCKET_MAX_FRAME_SIZE];
    int head, tail;
    pthread_mutex_t lock;
} websocket_frame_queue_t;

typedef struct {
    CURL *curl;
    bool connected;
    const char *name;
    websocket_frame_queue_t frame_q;

    pthread_t ws_thread;
    pthread_mutex_t lock;
} websocket_t;

websocket_t *websocket_init(const char *endpoint, const char *name);

void websocket_close(websocket_t *socket);

void websocket_send_frame(websocket_t *socket, char *frame, size_t size);

int websocket_receive_frame(websocket_t *socket, char *out_frame);

#endif