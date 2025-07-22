# CCord
A simple C library that interacts with the Discord API.

## Overview
This library is still in very early development. Structure, naming conventions, etc. are very likely to change. This began as a personal project of mine and I am working on turning this into a full library. That means there is a lot of refactoring to be done.

The library interacts with Discord's REST API to make simple POST and GET requests, and provides a wrapper for commonly used operations, such as sending messages, reactions, getting channel or guild information, etc.

Operations that are not directly supported can be implemented by the user using CCord's functions. (TODO: Give details)

The library supports real time events by connecting to discord's WebSocket gateway, and allows users to register callback functions for specified events.

I am also working on supporting voice connections through a UDP socket connection with discord's audio servers. Currently receiving audio from a voice channel is partially supported, but sending audio packets has not been implemented yet. Most of the work has been done in a separate project, so that part of the library is only partially complete.

## Dependencies

The dependencies are the latest versions of curl, sodium, opus, and PortAudio. Specific required versions will be listed at a later date, when the library is closer to completion.

The project is built in a cygwin environment and includes a makefile that builds the library as a .dll.

## Usage

The project is currently undergoing big changes which include a lot of restructering of existing code. The library now supports sending and receiving JSON payloads from discord's rest API, and also supports connecting to the discord gateway and receiving events from it.

Here is an example program that shows how to use the library:
```
#include "../include/ccord.h"
#include "../include/gateway.h"

#include <stdlib.h>

#define TOKEN "Bot YOUR_TOKEN_HERE"
#define CHANNEL_ID "YOUR_CHANNELID_HERE"

void on_ready(cJSON *data)
{
    printf("Bot is ready!\n");
}

void on_message(cJSON *data)
{
    printf("Message!\n> %s\n", cJSON_GetObjectItemCaseSensitive(data, "content")->valuestring);
}

int main()
{
    CCORDcontext *cc = ccord_init(TOKEN);

    char *url = "https://discord.com/api/v10/channels/" CHANNEL_ID "/messages";
    char *json_payload = "{\"content\": \"Hello!\"}";

    ccord_request(cc, POST, url, json_payload, strlen(json_payload), NULL, NULL);

    cc->gateway = (CCORDgateway){.type = GW_DEFAULT};

    ccord_register_on_ready(cc, on_ready);
    ccord_register_on_message_create(cc, on_message);

    int res = ccord_gateway_init(&cc->gateway, "wss://gateway.discord.gg/?v=10&encoding=json", TOKEN, GUILDS | GUILD_MESSAGES | MESSAGE_CONTENT);
    if(res != 0)
    {
        printf("ERROR!\n");
    }

    while(1)
    {
        ccord_dispatch_events(cc);
        sleep(1);
    }

    ccord_free(cc);
    return 0;
}
```
Eventually, the gateway will be automatically initiated by CCORD. Event handles require cJSON input parameters, but in the future, these will be replaced with the library's own implementaiton of Discord's API JSON objects.

## TODO

The next goal will be to provide wrapper structures and functions for handling discord JSON objects, including messages, channels, guilds, etc.

### Finish implementation
⬛ Add functions for common discord API operations

⬛ Add custom data structures for JSON objects

⬛ Voice/audio system

### General
⬛ FIX: Gateway protocols do not disconnect properly and do not support resuming