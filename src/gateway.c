#include "gateway.h"

#include <pthread.h>

#include "cJSON.h"
#include "util.h"

// GATEWAY OP CODES
#define CCORDGW_DISPATCH                    0  // Receive
#define CCORDGW_HEARTBEAT                   1  // Send/Receive
#define CCORDGW_IDENTIFY                    2  // Send
#define CCORDGW_PRESENCE_UPDATE             3  // Send
#define CCORDGW_VOICE_STATE_UPDATE          4  // Send
#define CCORDGW_RESUME                      6  // Send
#define CCORDGW_RECONNECT                   7  // Receive
#define CCORDGW_REQUEST_GUILD_MEMBERS       8  // Send
#define CCORDGW_INVALID_SESSION             9  // Receive
#define CCORDGW_HELLO                       10 // Receive
#define CCORDGW_HEARTBEAT_ACK               11 // Receive
#define CCORDGW_REQUEST_SOUNDBOARD_SOUNDS   31 // Send

// Error codes                                   // Should reconnect
#define CCORDGWE_UNKNOWN_ERROR              4000 // True
#define CCORDGWE_UNKNOWN_OPCODE             4001 // True
#define CCORDGWE_DECODE_ERROR               4002 // True
#define CCORDGWE_NOT_AUTHENTICATED          4003 // True
#define CCORDGWE_AUTHENTICATION_FAILED      4004 // False
#define CCORDGWE_ALREADY_AUTHENTICATED      4005 // True
#define CCORDGWE_INVALID_SEQ                4007 // True
#define CCORDGWE_RATE_LIMITED               4008 // True
#define CCORDGWE_SESSION_TIMED_OUT          4009 // True
#define CCORDGWE_INVALID_SHARD              4010 // False
#define CCORDGWE_SHARDING_REQUIRED          4011 // False
#define CCORDGWE_INVALID_API_VERSION        4012 // False
#define CCORDGWE_INVALID_INTENT             4013 // False
#define CCORDGWE_DISALLOWED_INTENT          4014 // False

typedef uint32_t gateway_opcode_t;

typedef enum {
    GW_FLAG_HELLO   = (1 << 0),
    GW_FLAG_ACK     = (1 << 1),
    GW_FLAG_READY   = (1 << 2),
} gateway_flag_t;

const char *error_code_to_string_tbl[15] = {
    "UNKNOWN_ERROR",
    "UNKNOWN_OPCODE",
    "DECODE_ERROR",
    "NOT_AUTHENTICATED",
    "AUTHENTICATION_FAILED",
    "ALREADY_AUTHENTICATED",
    "",
    "INVALID_SEQ",
    "RATE_LIMITED",
    "SESSION_TIMED_OUT",
    "INVALID_SHARD",
    "SHARDING_REQUIRED",
    "INVALID_API_VERSION",
    "INVALID_INTENT",
    "DISALLOWED_INTENT",
};
static const char *error_code_to_string(gateway_opcode_t op)
{
    if(op < 4000 || op > 4014)
        return "";
    
    return error_code_to_string_tbl[op - 4000];
}

bool is_error_code(gateway_opcode_t code)
{
    return code >= 4000;
}

// 4006 not defined, so we set arbitrary value
bool gateway_should_reconnnect_tbl[15] = {true, true, true, true, false, true, true, true, true, true, false, false, false, false, false};
bool gateway_should_reconnect(gateway_opcode_t error_code) // We assume code is a valid error code >= 4000
{
    return gateway_should_reconnnect_tbl[error_code - 4000];
}

typedef struct {
    int op;                 // opcode
    cJSON *d;                // event data
    int s;                  // sequence number (-1 means null)
    char t[256];                // event name
} gateway_event_payload_t;

gateway_event_payload_t deserialize_packet(char *packet, size_t packet_size)
{
    // Null-terminated version
    char *packet_str = malloc(packet_size + 1);
    memcpy(packet_str, packet, packet_size);
    packet_str[packet_size] = '\0';

    gateway_event_payload_t payload = {0};
    cJSON *root = cJSON_Parse(packet_str);

    cJSON *json_op = cJSON_GetObjectItemCaseSensitive(root, "op");
    cJSON *json_d = cJSON_GetObjectItemCaseSensitive(root, "d");
    cJSON *json_s = cJSON_GetObjectItemCaseSensitive(root, "s");
    cJSON *json_t = cJSON_GetObjectItemCaseSensitive(root, "t");

    payload.op = json_op->valueint;

    if(!json_d || cJSON_IsNull(json_d))
        payload.d = NULL;
    else
        payload.d = cJSON_Duplicate(json_d, true);

    if(!json_s || cJSON_IsNull(json_s))
        payload.s = -1;
    else
        payload.s = json_s->valueint;

    if(!json_t || cJSON_IsNull(json_t))
        payload.t[0] = '\0';
    else
        strcpy(payload.t, json_t->valuestring);

    free(packet_str);
    cJSON_Delete(root);

    return payload;
}

int gateway_send_payload(CCORDgateway *gw, char *payload, size_t size)
{
    size_t sent;
    CURLcode res = curl_ws_send(gw->ws, payload, size, &sent, 0, CURLWS_TEXT);
    if(res != CURLE_OK) {
        fprintf(stderr, "[Gateway] Frame sending failed: %s\n", curl_easy_strerror(res));
        return -1;
    }

    return 0;
}

int gateway_heartbeat(CCORDgateway *gw, int event)
{
    char hb_payload[256];
    if(event == -1)
        return gateway_send_payload(gw, "{\"op\":1,\"d\":null}", 17);

    sprintf(hb_payload, "{\"op\":1,\"d\":%d}", event);
    return gateway_send_payload(gw, hb_payload, strlen(hb_payload));
}

void *gateway_thread(void *arg)
{
    CCORDgateway *gw = (CCORDgateway *)(arg);

    size_t frame_buf_size = 4096; // Default size (maybe make a macro?)
    char *frame_buf = malloc(frame_buf_size);

    int event = -1;
    long next_heartbeat = 0;

    CURLcode res;
    pthread_mutex_lock(&gw->lock);
    while(!gw->close)
    {
        pthread_mutex_unlock(&gw->lock);

        // -------------------------
        // Read frame(s) from websocket
        size_t rlen = 0;
        const struct curl_ws_frame *meta;
        char *buf_ptr = frame_buf;

        while(1)
        {
            res = curl_ws_recv(gw->ws, buf_ptr, frame_buf_size - (buf_ptr - frame_buf), &rlen, &meta);
            buf_ptr += rlen;

            if(res != CURLE_OK)
                break;

            // When frame buffer size isn't big enough to read the entire frame, increase size of frame buffer
            while(meta->bytesleft > 0)
            {
                frame_buf_size *= 2;
                frame_buf = realloc(frame_buf, frame_buf_size);

                res = curl_ws_recv(gw->ws, buf_ptr, frame_buf_size - (buf_ptr - frame_buf), &rlen, &meta);
                buf_ptr += rlen;
            }
            
            if(!(meta->flags & CURLWS_CONT))
                break;
            
            // If we make it to the end of the loop, there is another frame to read
        }
        // -------------------------

        if(res == CURLE_AGAIN) {
            //usleep(10000);
        } else if(res != CURLE_OK) {
            // TODO: handle this error gracefully
            break;
        } else {
            gateway_event_payload_t payload = deserialize_packet(frame_buf, buf_ptr - frame_buf);

            if(payload.s != -1)
                event = payload.s;

            if(is_error_code(payload.op))
            {
                // TODO: Handle error codes
                printf("Error code: %d\n%s\n", payload.op, error_code_to_string(payload.op));
            }

            switch(payload.op)
            {
                case CCORDGW_DISPATCH: // Dispatch
                    pthread_mutex_lock(&gw->events_lock);

                    if(gw->events_count < MAX_EVENTS_IN_QUEUE)
                        gw->events[gw->events_count++] = (dispatch_event_t){event_type_from_str(payload.t), payload.d};

                    pthread_mutex_unlock(&gw->events_lock);

                    break;
                case CCORDGW_HEARTBEAT: // Heartbeat
                    gateway_heartbeat(gw, event);
                    next_heartbeat = current_time_ms() + gw->heartbeat;
                    gw->state_flags &= ~GW_FLAG_ACK;
                    break;
                case CCORDGW_RECONNECT: // Reconnect TODO
                    break;
                case CCORDGW_INVALID_SESSION: // Invalid Session TODO
                    break;
                case CCORDGW_HELLO: // Hello
                    gw->heartbeat = cJSON_GetObjectItemCaseSensitive(payload.d, "heartbeat_interval")->valueint;

                    next_heartbeat = current_time_ms() + (rand() % (gw->heartbeat + 1));

                    // Send identify
                    char identify[1024];
                    sprintf(identify, "{\"op\":2,\"d\":{\"token\":\"%s\",\"intents\":%d,\"properties\":{\"$os\":\"linux\",\"$browser\":\"my-c-client\",\"$device\":\"my-c-client\"}}}", gw->token, gw->intents);
                    gateway_send_payload(gw, identify, strlen(identify));

                    gw->state_flags |= GW_FLAG_HELLO;
                    break;
                case CCORDGW_HEARTBEAT_ACK: // ACK
                    gw->state_flags |= GW_FLAG_ACK;
                    break;
            }

            if(payload.op != CCORDGW_DISPATCH)
                cJSON_Delete(payload.d);
        }

        // TODO: Reconnect logic... (triggered by flag?)
        
        if(gw->state_flags & GW_FLAG_HELLO && current_time_ms() >= next_heartbeat)
        {
            if(!(gw->state_flags & GW_FLAG_ACK))
            {
                // TODO: Reconnect - ACK confirmation not received
            }

            // TODO: Handle return values...
            gateway_heartbeat(gw, event);
            next_heartbeat = current_time_ms() + gw->heartbeat;
            gw->state_flags &= ~GW_FLAG_ACK;
        }

        usleep(10000);

        pthread_mutex_lock(&gw->lock);
    }
    pthread_mutex_unlock(&gw->lock);

    free(frame_buf);
}

int ccord_gateway_init(CCORDgateway *gw, const char *endpoint, const char *token, int intents)
{
    CURL *ws = curl_easy_init();
    if(!ws)
    {
        // CURL ws failed to init
        return -1;
    }

    curl_easy_setopt(ws, CURLOPT_URL, endpoint);
    curl_easy_setopt(ws, CURLOPT_CONNECT_ONLY, 2L);

    CURLcode res;
    res = curl_easy_perform(ws);
    if(res != CURLE_OK) {
        // Connect failed
        return -1;
    }

    gw->ws = ws;
    gw->close = false;
    gw->state_flags = 0b00000000;
    gw->lock = PTHREAD_MUTEX_INITIALIZER;
    gw->events_lock = PTHREAD_MUTEX_INITIALIZER;
    gw->intents = intents;
    strcpy(gw->token, token);
    strcpy(gw->endpoint, endpoint);

    if(pthread_create(&gw->thread, NULL, gateway_thread, gw) != 0) {
        // Thread failed
        curl_easy_cleanup(ws);
        return -1;
    }

    return 0;
}