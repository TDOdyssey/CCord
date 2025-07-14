#include "ccord.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "cJSON.h"

/*
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata);
long current_time_ms();
void reverse(char str[], int length);
char* itoa(int num, char* str, int base);

CCORD *ccord_init(const char *token)
{
    CCORD *ccord = NULL;
    CURL *curl;
    struct curl_slist *headers = NULL;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if(curl)
    {
        char buffer[256] = "Authorization: ";
        strcat(buffer, token);
        headers = curl_slist_append(headers, buffer);
        headers = curl_slist_append(headers, "Content-Type: application/json");;
        
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        ccord = malloc(sizeof(CCORD));
        ccord->curl = curl;
        ccord->headers = headers;
    }
    
    return ccord;
}

CCORDresponse ccord_request(CCORD *ccord, CCORDrequestType type, const char *url, const char *payload)
{
    if(type == POST)
    {
        curl_easy_setopt(ccord->curl, CURLOPT_HTTPGET, 0L);  
        curl_easy_setopt(ccord->curl, CURLOPT_POST, 1L);
        curl_easy_setopt(ccord->curl, CURLOPT_POSTFIELDS, payload);
        curl_easy_setopt(ccord->curl, CURLOPT_POSTFIELDSIZE, strlen(payload));    
    }
    else if(type == GET)
    {
        curl_easy_setopt(ccord->curl, CURLOPT_HTTPGET, 1L); 
        curl_easy_setopt(ccord->curl, CURLOPT_POSTFIELDS, NULL);
        curl_easy_setopt(ccord->curl, CURLOPT_POSTFIELDSIZE, 0);
        curl_easy_setopt(ccord->curl, CURLOPT_POST, 0L);
    }

    curl_easy_setopt(ccord->curl, CURLOPT_URL, url);

    CCORDresponse response = {0};
    curl_easy_setopt(ccord->curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(ccord->curl, CURLOPT_WRITEDATA, (void *)&response);

    CURLcode res = curl_easy_perform(ccord->curl);
            
    long http_code = 0;
    curl_easy_getinfo(ccord->curl, CURLINFO_RESPONSE_CODE, &http_code);
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    else if(http_code != 200 && http_code != 201)
        printf("\nHTTP Error Code: %ld\n", http_code);
    
    return response;
}

CCORDresponse ccord_send(CCORD *ccord, const char *content, const char *channel_id)
{
    char api_url[256] = "https://discord.com/api/v10/channels/";
    strcat(api_url, channel_id);
    strcat(api_url, "/messages");

    char json_payload[512] = "{\"content\": \"";
    strcat(json_payload, content);
    strcat(json_payload, "\"}");

    return ccord_request(ccord, POST, api_url, json_payload);
}

CCORDresponse ccord_get(CCORD *ccord, const char *channel_id, int count)
{
    char api_url[256] = "https://discord.com/api/v10/channels/";
    strcat(api_url, channel_id);
    sprintf(&api_url[strlen(api_url)], "/messages?limit=%d", count);

    return ccord_request(ccord, GET, api_url, NULL);
}

void ccord_cleanup(CCORD *ccord)
{
    if(ccord->curl)
        curl_easy_cleanup(ccord->curl);

    curl_slist_free_all(ccord->headers);
    curl_global_cleanup();

    free(ccord);
}

void process_messages(const char *json_response)
{
    cJSON *root = cJSON_Parse(json_response);
    if(!root || !cJSON_IsArray(root))
    {
        fprintf(stderr, "Invalid or non-array JSON response.\n");
        return;
    }

    int count = cJSON_GetArraySize(root);
    for(int i = count - 1; i >= 0; i--)
    {
        cJSON *msg = cJSON_GetArrayItem(root, i);

        cJSON *content = cJSON_GetObjectItemCaseSensitive(msg, "content");
        cJSON *author = cJSON_GetObjectItemCaseSensitive(msg, "author");
        cJSON *username = author ? cJSON_GetObjectItemCaseSensitive(author, "username") : NULL;
        cJSON *bot = cJSON_GetObjectItemCaseSensitive(author, "bot");

        if (cJSON_IsString(content) && cJSON_IsString(username)) {
            printf("%s%s: %s\n", username->valuestring, cJSON_IsTrue(bot) ? " (Bot)" : "", content->valuestring);
        }
    }

    cJSON_Delete(root);
}

*/


// Refactor

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
    
    *recv = NULL;
    *recv_size = 0;
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