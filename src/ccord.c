#include "ccord.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include <curl/curl.h>

#include "cJSON.h"

// TODO move these into gateway.h
typedef enum {
    DEFAULT,
    VOICE
} CCORDgatewayType;

typedef struct {
    CCORDgatewayType type;

    CURL *ws; // Curl WebSocket

    bool connected;
    int heartbeat;

    // Used for handling protocol state depending on type. TODO
    uint8_t state_flags;
} CCORDgateway;

typedef struct CCORDcontext {
    const char *token;

    // Separate curl contexts so we don't have to constantly update headers of a single curl context
    CURL *curl_get;
    CURL *curl_post;
    struct curl_slist *headers;

    // Gateway
    CCORDgateway gateway;
    CCORDgateway voice_gateway;
} CCORDcontext;

struct write_cb_args {
    char **recv;
    size_t *recv_size;
};

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t total = size * nmemb;
    struct write_cb_args *args = (struct write_cb_args *)userdata;

    if(args->recv)
    {
        char *new_data = realloc(*args->recv, (*args->recv_size) + total + 1);
        if(!new_data)
        {
            fprintf(stderr, "realloc failed!\n");
            return 0;
        }
    
        *args->recv = new_data;
    
        memcpy(*args->recv + *args->recv_size, ptr, total);
        *args->recv_size += total;
        *args->recv[*args->recv_size] = '\0';
    }

    return size * nmemb;
}

CCORDcontext *ccord_init(const char *token)
{
    char buffer[256] = "Authorization: ";
    strcat(buffer, token);
    
    CCORDcontext *ccord = NULL;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);

    CURL *get, *post;

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, buffer);
    headers = curl_slist_append(headers, "Content-Type: application/json");;

    get = curl_easy_init();
    post = curl_easy_init();
    if(get && post)
    {
        curl_easy_setopt(get, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(post, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(get, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(post, CURLOPT_WRITEFUNCTION, write_callback);

        curl_easy_setopt(get, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(get, CURLOPT_POSTFIELDS, NULL);
        curl_easy_setopt(get, CURLOPT_POSTFIELDSIZE, 0);
        curl_easy_setopt(get, CURLOPT_POST, 0L);

        curl_easy_setopt(post, CURLOPT_HTTPGET, 0L);  
        curl_easy_setopt(post, CURLOPT_POST, 1L);
    }
    else
        return NULL;

    ccord = malloc(sizeof(CCORDcontext));
    
    ccord->token = token;
    ccord->curl_get = get;
    ccord->curl_post = post;
    ccord->headers = headers;
    ccord->gateway = (CCORDgateway){0};
    ccord->voice_gateway = (CCORDgateway){0};
    
    return ccord;
}

int ccord_login(CCORDcontext *ccord, int intents)
{
    return 1; // TODO
}

int ccord_request(CCORDcontext *ccord, CCORDrequestType type, const char *url, const char *send, size_t send_size, char **recv, size_t *recv_size)
{
    CURL *curl = NULL;
    switch(type)
    {
        case GET:
            curl = ccord->curl_get;
            break;
        case POST:
            curl = ccord->curl_post;
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, send);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, send_size);    
            break;
        default:
            break;
    }

    if(!curl)
        return 1; // Error: invalid request type
    
    curl_easy_setopt(curl, CURLOPT_URL, url);

    struct write_cb_args args = {
        .recv = recv,
        .recv_size = recv_size
    };
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&args);
    
    if(recv)
    {
        *recv = NULL;
        *recv_size = 0;
    }
    CURLcode res = curl_easy_perform(curl);
            
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return 1;
    }
    else if(http_code != 200 && http_code != 201) {
        printf("\nHTTP Error Code: %ld\n", http_code);
        return 1;
    }
    
    return 0;
}

void ccord_free(CCORDcontext *ccord)
{
    if(!ccord)
        return;

    if(ccord->curl_get)
        curl_easy_cleanup(ccord->curl_get);
    
    if(ccord->curl_post)
        curl_easy_cleanup(ccord->curl_post);

    curl_slist_free_all(ccord->headers);
    curl_global_cleanup();

    // TODO: gateway cleanup? (or write another function?) probably best to have it all here...

    free(ccord);
}