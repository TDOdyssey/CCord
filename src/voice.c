#include "voice.h"

#include "cJSON.h"
#include "util.h"

voice_gateway_t voice_gateway_init(const char *server_id, const char *user_id, const char *session_id, const char *token, const char *endpoint)
{
    return (voice_gateway_t){
        .ws = websocket_init(endpoint, "voice websocket"),
        .server_id = server_id,
        .user_id = user_id,
        .session_id = session_id,
        .token = token,
        .endpoint = endpoint,
        .closed = false,
        .hello = false,
        .ack = true,
        .ready = false,
        .nonce = rand()
    };
}

static void voice_gateway_heartbeat(voice_gateway_t *gw)
{
    char hb_payload[256];
    char buf[32];
    sprintf(hb_payload, "{\"op\":3,\"d\":%s}", itoa(gw->nonce, buf, 10));
    websocket_send_frame(gw->ws, hb_payload, strlen(hb_payload));

    gw->next_heartbeat = current_time_ms() + gw->heartbeat;
}

void voice_gateway_update(voice_gateway_t *gw)
{
    char buffer[CC_WEBSOCKET_MAX_FRAME_SIZE];
    while(websocket_receive_frame(gw->ws, buffer))
    {
        cJSON *frame = cJSON_Parse(buffer);
        cJSON *op = cJSON_GetObjectItemCaseSensitive(frame, "op");

        printf("VOICE OP: %d\n", op->valueint);

        switch(op->valueint)
        {
            cJSON *d;
            case 2: // Ready
                printf("[VOICE GATEWAY] READY!\n");

                d = cJSON_GetObjectItemCaseSensitive(frame, "d");

                char *str = cJSON_Print(d);

                printf("READY: %s\n", str);
                
                gw->ssrc = cJSON_GetObjectItemCaseSensitive(d, "ssrc")->valueint;
                gw->port = cJSON_GetObjectItemCaseSensitive(d, "port")->valueint;
                strcpy(gw->ip, cJSON_GetObjectItemCaseSensitive(d, "ip")->valuestring);

                gw->udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

                struct sockaddr_in discord_addr;
                memset(&discord_addr, 0, sizeof(discord_addr));
                discord_addr.sin_family = AF_INET;
                discord_addr.sin_port = htons(gw->port);
                discord_addr.sin_addr.s_addr = inet_addr(gw->ip);

                gw->discord_addr = discord_addr;

                // IP Discovery

                // Field	Description	                                                    Size
                // Type	    Values 0x1 and 0x2 indicate request and response, respectively	2 bytes
                // Length	Message length excluding Type and Length fields (value 70)	    2 bytes
                // SSRC	    Unsigned integer	                                            4 bytes
                // Address	Null-terminated string in response	                            64 bytes
                // Port	    Unsigned short	                                                2 bytes
                char packet[74];

                // Type
                packet[0] = 0x00;
                packet[1] = 0x01;

                // Length
                packet[2] = 0x00;
                packet[3] = 0x46;

                // SSRC
                uint32_t ssrc = htonl(gw->ssrc);
                memcpy(&packet[4], &ssrc, 4);

                ssize_t sent = sendto(gw->udp_socket, packet, sizeof(packet), 0, (struct sockaddr *)&gw->discord_addr, sizeof(gw->discord_addr));

                uint8_t buf[74];
                struct sockaddr_in from;
                socklen_t from_len = sizeof(from);

                ssize_t recvd = recvfrom(gw->udp_socket, packet, sizeof(packet), 0, (struct sockaddr *)&from, &from_len);

                if (recvd < 74) {
                    printf("recvfrom error\n");
                    break;
                }

                // Type: bytes 0–1
                uint16_t type = (packet[0] << 8) | packet[1];
                if (type != 0x0002) {
                    fprintf(stderr, "Invalid response type: 0x%04x\n", type);
                    break;
                }

                // Length: bytes 2–3 (not strictly needed unless you're validating)
                uint16_t length = (packet[2] << 8) | packet[3];
                if (length != 70) {
                    fprintf(stderr, "Unexpected length: %d\n", length);
                    break;  
                }

                char ip_out[100];

                // IP Address: bytes 8–71
                strncpy(ip_out, (char *)&packet[8], sizeof(ip_out) - 1);
                ip_out[sizeof(ip_out) - 1] = '\0';

                // Port: bytes 72–73 (big endian)
                int port_out = (packet[72] << 8) | packet[73];

                printf("IP DISCOVERY:\nIP: %s\nPORT: %d\n\n", ip_out, port_out);

                // Select protocol
                char select[256];
                sprintf(select, "{\"op\":1,\"d\":{\"protocol\":\"udp\",\"data\":{\"address\":\"%s\",\"port\":%d,\"mode\":\"aead_xchacha20_poly1305_rtpsize\"}}}", ip_out, port_out);
                websocket_send_frame(gw->ws, select, strlen(select));

                struct sockaddr_in my_addr;
                memset(&my_addr, 0, sizeof(my_addr));
                my_addr.sin_family = AF_INET;
                my_addr.sin_port = htons(port_out);
                my_addr.sin_addr.s_addr = INADDR_ANY;

                gw->my_addr = my_addr;
                if(bind(gw->udp_socket, (struct sockaddr *)&gw->my_addr, sizeof(gw->my_addr)) >= 0)
                    printf("BOUND SOCKET\n");

                break;
            case 3: // Heartbeat
                voice_gateway_heartbeat(gw);
                break;
            case 4: // Session Description
                d = cJSON_GetObjectItemCaseSensitive(frame, "d");
                cJSON *sk = cJSON_GetObjectItemCaseSensitive(d, "secret_key");

                printf("MODE!!! %s\n", cJSON_GetObjectItemCaseSensitive(d, "mode")->valuestring);

                printf("Secret key:\n");
                for(int i = 0; i < 32; i++)
                {
                    gw->secret_key[i] = (uint8_t)cJSON_GetArrayItem(sk, i)->valueint;
                    printf("%d\n", gw->secret_key[i]);
                }

                gw->ready = true;

                break;
            case 6: // ACK
                printf("[VOICE GATEWAY] Received ACK!\n");
                gw->ack = true;
                break;
            case 8: // Hello
                printf("[VOICE GATEWAY] Received Hello!\n");

                d = cJSON_GetObjectItemCaseSensitive(frame, "d");
                cJSON *heartbeat_interval = cJSON_GetObjectItemCaseSensitive(d, "heartbeat_interval");

                gw->heartbeat = heartbeat_interval->valueint;
                gw->next_heartbeat = current_time_ms() + (rand() % (gw->heartbeat + 1));

                gw->hello = true;

                // Send identify
                char identify[1024];
                sprintf(identify, "{\"op\":0,\"d\":{\"server_id\":\"%s\",\"user_id\":\"%s\",\"session_id\":\"%s\",\"token\":\"%s\"}}", gw->server_id, gw->user_id, gw->session_id, gw->token);
                websocket_send_frame(gw->ws, identify, strlen(identify));
                break;
        }

        cJSON_free(frame);
    }

    if(gw->hello && current_time_ms() >= gw->next_heartbeat)
    {
        if(!gw->ack)
            voice_gateway_close(gw);
        
        voice_gateway_heartbeat(gw);
        gw->ack = false;
    }


}

void voice_gateway_close(voice_gateway_t *gw)
{
    websocket_close(gw->ws);
    close(gw->udp_socket);
    gw->closed = true;
}