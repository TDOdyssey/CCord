#include "gateway.h"

#include "websocket.h"
#include "cJSON.h"
#include "util.h"

gateway_t gateway_init(const char *token, int intents)
{
    return (gateway_t){
        .ws = websocket_init(CCORD_GATEWAY_DEFAULT_URL, "discord gateway"),
        .token = token,
        .intents = intents,
        .closed = false,
        .hello = false,
        .ack = true,
        .event = -1
    };
}

static void gateway_heartbeat(gateway_t *gw);

void gateway_update(gateway_t *gw)
{
    char buffer[CC_WEBSOCKET_MAX_FRAME_SIZE];
    while(websocket_receive_frame(gw->ws, buffer))
    {
        cJSON *frame = cJSON_Parse(buffer);
        cJSON *op = cJSON_GetObjectItemCaseSensitive(frame, "op");

        while(!cJSON_IsNumber(op))
        {
            websocket_receive_frame(gw->ws, &buffer[strlen(buffer)]);

            frame = cJSON_Parse(buffer);
            op = cJSON_GetObjectItemCaseSensitive(frame, "op");
        }

        printf("OP: %d\n", op->valueint);

        switch(op->valueint)
        {
            case 0: // Event
                cJSON *t = cJSON_GetObjectItemCaseSensitive(frame, "t");
                cJSON *s = cJSON_GetObjectItemCaseSensitive(frame, "s");

                printf("[GATEWAY] Received event %s!\n", t->valuestring);
                if(cJSON_IsNumber(s))
                    gw->event = s->valueint;
                
                for(int i = 0; i < gw->registry.count; i++)
                {
                    if(strcmp(t->valuestring, gw->registry.entries[i].event_name) == 0)
                    {
                        cJSON *d = cJSON_GetObjectItemCaseSensitive(frame, "d");
                        gw->registry.entries[i].callback(d);
                    }
                }

                break;
            case 1: // Heartbeat
                gateway_heartbeat(gw);
                break;
            case 4: // Voice State Update
                for(int i = 0; i < gw->registry.count; i++)
                {
                    if(strcmp("VOICE_STATE_UPDATE", gw->registry.entries[i].event_name) == 0)
                    {
                        cJSON *d = cJSON_GetObjectItemCaseSensitive(frame, "d");
                        gw->registry.entries[i].callback(d);
                    }
                }
                break;
            case 10: // Hello
                printf("[GATEWAY] Received Hello!\n");

                cJSON *d = cJSON_GetObjectItemCaseSensitive(frame, "d");
                cJSON *heartbeat_interval = cJSON_GetObjectItemCaseSensitive(d, "heartbeat_interval");

                gw->heartbeat = heartbeat_interval->valueint;
                gw->next_heartbeat = current_time_ms() + (rand() % (gw->heartbeat + 1));

                gw->hello = true;

                // Send identify
                char identify[1024];
                sprintf(identify, "{\"op\":2,\"d\":{\"token\":\"%s\",\"intents\":%d,\"properties\":{\"$os\":\"linux\",\"$browser\":\"my-c-client\",\"$device\":\"my-c-client\"}}}", gw->token, gw->intents);
                websocket_send_frame(gw->ws, identify, strlen(identify));
                break;
            case 11: // ACK
                printf("[GATEWAY] Received ACK!\n");
                gw->ack = true;
                break;
        }

        cJSON_free(frame);
    }

    if(gw->hello && current_time_ms() >= gw->next_heartbeat)
    {
        if(!gw->ack)
            gateway_close(gw);
        
        gateway_heartbeat(gw);
        gw->ack = false;
    }
}

static void gateway_heartbeat(gateway_t *gw)
{
    char hb_payload[256];
    char buf[32];
    sprintf(hb_payload, "{\"op\":1,\"d\":%s}", gw->event >= 0 ? itoa(gw->event, buf, 10) : "null");
    websocket_send_frame(gw->ws, hb_payload, strlen(hb_payload));

    gw->next_heartbeat = current_time_ms() + gw->heartbeat;
}

void gateway_close(gateway_t *gw)
{
    websocket_close(gw->ws);
    gw->closed = true;
}

void gateway_register_event(gateway_t *gw, const char *event_name, gateway_event_cb cb)
{
    if(gw->registry.count >= CCORD_MAX_CALLBACKS)
    {
        fprintf(stderr, "Too many registered callbacks\n");
        return;
    }

    gw->registry.entries[gw->registry.count] = (event_cb_registry_entry_t){.event_name = event_name, .callback = cb};
    gw->registry.count++;
}