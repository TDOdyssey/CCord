#ifndef GATEWAY_H
#define GATEWAY_H

#include <stdbool.h>

#include "cJSON.h"
#include <curl/curl.h>

#include "ccord_internal.h"
#include "event.h"

int ccord_gateway_init(CCORDgateway *gw, const char *endpoint, const char *token, int intents);

//TODO: void ccord_gateway_close(CCORDgateway *gw);

#endif