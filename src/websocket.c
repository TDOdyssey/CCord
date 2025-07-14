#include "websocket.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

static void websocket_push_frame(websocket_t *socket, const char *msg);

static void *websocket_thread(void *arg)
{
    websocket_t *socket = (websocket_t *)(arg);

    CURLcode res;
    pthread_mutex_lock(&socket->lock);
    while(socket->connected)
    {
        pthread_mutex_unlock(&socket->lock);

        size_t rlen;
        const struct curl_ws_frame *meta;
        char buffer[CC_WEBSOCKET_MAX_FRAME_SIZE];
        char *buf_ptr = &buffer[0];

        pthread_mutex_lock(&socket->lock);
        do
        {
            res = curl_ws_recv(socket->curl, buf_ptr, sizeof(buffer) - 1 - (buf_ptr - buffer), &rlen, &meta);
            buf_ptr += rlen;
        } while(meta->bytesleft > 0);
        pthread_mutex_unlock(&socket->lock);

        if(res == CURLE_AGAIN) {
            usleep(10000);
        } else if(res != CURLE_OK) {
            fprintf(stderr, "[WebSocket - %s] Receive failed: %s\n", socket->name, curl_easy_strerror(res));
            break;
        } else {
            *buf_ptr = '\0';
            websocket_push_frame(socket, buffer);
            printf("[WebSocket - %s] Pushed received frame to queue.\n", socket->name);
        }

        pthread_mutex_lock(&socket->lock);
    }
}

websocket_t *websocket_init(const char *endpoint, const char *name)
{
    websocket_t *socket = malloc(sizeof(websocket_t));
    socket->connected = false;
    socket->name = name;
    socket->lock = PTHREAD_MUTEX_INITIALIZER;
    socket->frame_q.head = 0;
    socket->frame_q.tail = 0;
    socket->frame_q.lock = PTHREAD_MUTEX_INITIALIZER;

    socket->curl = curl_easy_init();
    if(!socket->curl)
    {
        fprintf(stderr, "[Websocket - %s] curl_easy_init failed\n", socket->name);
        return NULL;
    }

    curl_easy_setopt(socket->curl, CURLOPT_URL, endpoint);
    curl_easy_setopt(socket->curl, CURLOPT_CONNECT_ONLY, 2L);

    CURLcode res;
    res = curl_easy_perform(socket->curl);
    if(res != CURLE_OK) {
        fprintf(stderr, "[WebSocket - %s] connect failed: %s\n", socket->name, curl_easy_strerror(res));
        return NULL;
    }

    printf("[WebSocket - %s] Established connection with endpoint.\n", socket->name);
    socket->connected = true;

    if(pthread_create(&socket->ws_thread, NULL, websocket_thread, socket) != 0) {
        fprintf(stderr, "Failed to create thread!\n");
        return NULL;
    }

    return socket;
}

void websocket_close(websocket_t *socket)
{
    pthread_mutex_lock(&socket->lock);

    printf("[%s] Closing weboskcet...\n", socket->name);
    socket->connected = false;
    size_t sent;
    curl_ws_send(socket->curl, "", 0, &sent, 0, CURLWS_CLOSE);
    printf("[%s] Closed.\n", socket->name);

    curl_easy_cleanup(socket->curl);

    pthread_mutex_unlock(&socket->lock);

    pthread_join(socket->ws_thread, NULL);

    free(socket);
}

static void websocket_push_frame(websocket_t *socket, const char *frame)
{
    websocket_frame_queue_t *q = &socket->frame_q;
    pthread_mutex_lock(&q->lock);

    int next_tail = (q->tail + 1) % CC_WEBSOCKET_MAX_FRAMES;
    if(next_tail == q->head) {
        printf("[%s] Message buffer full, dropping message: %s\n", socket->name, frame);
    } else {
        strncpy(q->frames[q->tail], frame, CC_WEBSOCKET_MAX_FRAME_SIZE - 1);
        q->frames[q->tail][CC_WEBSOCKET_MAX_FRAME_SIZE - 1] = '\0';
        q->tail = next_tail;
    }

    pthread_mutex_unlock(&socket->frame_q.lock);
}

void websocket_send_frame(websocket_t *socket, char *frame, size_t size)
{
    size_t sent;
    pthread_mutex_lock(&socket->lock);
    CURLcode res = curl_ws_send(socket->curl, frame, size, &sent, 0, CURLWS_TEXT);
    pthread_mutex_unlock(&socket->lock);
    if(res != CURLE_OK) {
        fprintf(stderr, "[WebSocket - %s] Frame sending failed: %s\n", socket->name, curl_easy_strerror(res));
    } else {
        printf("[WebSocket - %s] Frame sending: %s\n", socket->name, frame);
    }
}

int websocket_receive_frame(websocket_t *socket, char *out_msg)
{
    websocket_frame_queue_t *q = &socket->frame_q;
    pthread_mutex_lock(&q->lock);

    if(q->head == q->tail) {
        pthread_mutex_unlock(&q->lock);
        return 0;
    }

    strncpy(out_msg, q->frames[q->head], CC_WEBSOCKET_MAX_FRAME_SIZE);
    q->head = (q->head + 1) % CC_WEBSOCKET_MAX_FRAMES;

    pthread_mutex_unlock(&q->lock);
    return 1;
}