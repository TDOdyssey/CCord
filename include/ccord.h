#ifndef CCORD_H
#define CCORD_H

#include <stdbool.h>

#include <curl/curl.h>
#include <pthread.h>

#define API_BASE "https://discord.com/api/v10"

#define API_SEND_MSG APIBASE "/channels/%s"

#define CC_COLOR_GREEN  "\x1b[1;32m"
#define CC_COLOR_YELLOW   "\x1b[1;33m"
#define CC_COLOR_MAGENTA  "\x1b[1;35m"
#define CC_COLOR_RESET  "\x1b[0m"

#define GATEWAY_MSG CC_COLOR_GREEN "[GATEWAY] " CC_COLOR_RESET
#define MAIN_MSG CC_COLOR_YELLOW "[MAIN] " CC_COLOR_RESET
#define QUEUE_MSG CC_COLOR_MAGENTA "[QUEUE] " CC_COLOR_RESET

typedef struct {
    CURL *curl;
    bool connected;
    int heartbeat;
} CCORDgateway;

typedef struct {
    CURL *curl;
    struct curl_slist *headers;
    CCORDgateway *gateway;
} CCORD;

typedef struct {
    char *data;
    size_t size;
} CCORDresponse;

typedef enum {
    GET,
    POST
} CCORDrequestType;

// TEMPP!!!!

#define CCORD_MAX_EVENTS 10
#define CCORD_MAX_EVENT_SIZE 8192

typedef struct {
    char events[CCORD_MAX_EVENTS][CCORD_MAX_EVENT_SIZE];
    int head, tail;
    pthread_mutex_t lock;
} CCORDeventQueue;

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t cond;
    CCORDgateway *gateway;
    char *command;
    int has_command;
    CCORDeventQueue event_queue;
} shared_state_t;

typedef struct {
    shared_state_t *state;
} gateway_thread_args_t;

CCORD *ccord_init(const char *token);

CCORDresponse ccord_request(CCORD *ccord, CCORDrequestType type, const char *url, const char *payload);

CCORDresponse ccord_send(CCORD *ccord, const char *content, const char *channel_id);

CCORDresponse ccord_get(CCORD *ccord, const char *channel_id, int count);

void ccord_cleanup(CCORD *ccord);

void process_messages(const char *json_response);

#endif