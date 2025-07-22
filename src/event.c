#include "../include/event.h"

#include "ccord_internal.h"

#include <string.h>
#include <pthread.h>

void ccord_dispatch_events(CCORDcontext *cc)
{
    pthread_mutex_lock(&(cc->gateway.events_lock));
    for(int i = 0; i < cc->gateway.events_count; i++)
    {
        dispatch_event_t event = cc->gateway.events[i];
        default_handle_t handle = (default_handle_t)cc->event_handles[event.type];

        if(handle != NULL)
            handle(event.data);

        cJSON_Delete(event.data);
    }
    cc->gateway.events_count = 0;
    pthread_mutex_unlock(&(cc->gateway.events_lock));
}

const struct {
    CCORDeventType type;
    const char *str;
} event_str_conversion_tbl[] = {
    {READY, "READY"},
    {APPLICATION_COMMAND_PERMISSIONS_UPDATE, "APPLICATION_COMMAND_PERMISSIONS_UPDATE"},
    {AUTO_MODERATION_RULE_CREATE,	   "AUTO_MODERATION_RULE_CREATE"},
    {AUTO_MODERATION_RULE_UPDATE,	   "AUTO_MODERATION_RULE_UPDATE"},
    {AUTO_MODERATION_RULE_DELETE,	   "AUTO_MODERATION_RULE_DELETE"},
    {AUTO_MODERATION_ACTION_EXECUTION, "AUTO_MODERATION_ACTION_EXECUTION"},
    {CHANNEL_CREATE,	                "CHANNEL_CREATE"},
    {CHANNEL_UPDATE,	                "CHANNEL_UPDATE"},
    {CHANNEL_DELETE,	                "CHANNEL_DELETE"},
    {CHANNEL_PINS_UPDATE,	           "CHANNEL_PINS_UPDATE"},
    {THREAD_CREATE,	                 "THREAD_CREATE"},
    {THREAD_UPDATE,                  "THREAD_UPDATE"},
    {THREAD_DELETE,	                 "THREAD_DELETE"},
    {THREAD_LIST_SYNC,	              "THREAD_LIST_SYNC"},
    {THREAD_MEMBER_UPDATE,	          "THREAD_MEMBER_UPDATE"},
    {THREAD_MEMBERS_UPDATE,	         "THREAD_MEMBERS_UPDATE"},
    {ENTITLEMENT_CREATE,	            "ENTITLEMENT_CREATE"},
    {ENTITLEMENT_UPDATE,	            "ENTITLEMENT_UPDATE"},
    {ENTITLEMENT_DELETE,	            "ENTITLEMENT_DELETE"},
    {GUILD_CREATE,	                  "GUILD_CREATE"},
    {GUILD_UPDATE,	                  "GUILD_UPDATE"},
    {GUILD_DELETE,	                  "GUILD_DELETE"},
    {GUILD_AUDIT_LOG_ENTRY_CREATE,	  "GUILD_AUDIT_LOG_ENTRY_CREATE"},
    {GUILD_BAN_ADD,	                 "GUILD_BAN_ADD"},
    {GUILD_BAN_REMOVE,	              "GUILD_BAN_REMOVE"},
    {GUILD_EMOJIS_UPDATE,	           "GUILD_EMOJIS_UPDATE"},
    {GUILD_STICKERS_UPDATE,	         "GUILD_STICKERS_UPDATE"},
    {GUILD_INTEGRATIONS_UPDATE,	     "GUILD_INTEGRATIONS_UPDATE"},
    {GUILD_MEMBER_ADD,	              "GUILD_MEMBER_ADD"},
    {GUILD_MEMBER_REMOVE,	           "GUILD_MEMBER_REMOVE"},
    {GUILD_MEMBER_UPDATE,	           "GUILD_MEMBER_UPDATE"},
    {GUILD_MEMBERS_CHUNK,	           "GUILD_MEMBERS_CHUNK"},
    {GUILD_ROLE_CREATE,	             "GUILD_ROLE_CREATE"},
    {GUILD_ROLE_UPDATE,	             "GUILD_ROLE_UPDATE"},
    {GUILD_ROLE_DELETE,	             "GUILD_ROLE_DELETE"},
    {GUILD_SCHEDULED_EVENT_CREATE,	  "GUILD_SCHEDULED_EVENT_CREATE"},
    {GUILD_SCHEDULED_EVENT_UPDATE,	  "GUILD_SCHEDULED_EVENT_UPDATE"},
    {GUILD_SCHEDULED_EVENT_DELETE,	  "GUILD_SCHEDULED_EVENT_DELETE"},
    {GUILD_SCHEDULED_EVENT_USER_ADD,	"GUILD_SCHEDULED_EVENT_USER_ADD"},
    {GUILD_SCHEDULED_EVENT_USER_REMOVE, "GUILD_SCHEDULED_EVENT_USER_REMOVE"},
    {GUILD_SOUNDBOARD_SOUND_CREATE, "GUILD_SOUNDBOARD_SOUND_CREATE"},
    {GUILD_SOUNDBOARD_SOUND_UPDATE, "GUILD_SOUNDBOARD_SOUND_UPDATE"},
    {GUILD_SOUNDBOARD_SOUND_DELETE,	 "GUILD_SOUNDBOARD_SOUND_DELETE"},
    {GUILD_SOUNDBOARD_SOUNDS_UPDATE,	"GUILD_SOUNDBOARD_SOUNDS_UPDATE"},
    {SOUNDBOARD_SOUNDS,	             "SOUNDBOARD_SOUNDS"},
    {INTEGRATION_CREATE, 	           "INTEGRATION_CREATE"},
    {INTEGRATION_UPDATE, 	           "INTEGRATION_UPDATE"},
    {INTEGRATION_DELETE, 	           "INTEGRATION_DELETE"},
    {INTERACTION_CREATE, 	           "INTERACTION_CREATE"},
    {INVITE_DELETE,	                 "INVITE_DELETE"},
    {MESSAGE_CREATE,                 "MESSAGE_CREATE"},
    {MESSAGE_UPDATE,                 "MESSAGE_UPDATE"},
    {MESSAGE_DELETE,                 "MESSAGE_DELETE"},
    {MESSAGE_DELETE_BULK,	           "MESSAGE_DELETE_BULK"},
    {MESSAGE_REACTION_ADD,	          "MESSAGE_REACTION_ADD"},
    {MESSAGE_REACTION_REMOVE,	       "MESSAGE_REACTION_REMOVE"},
    {MESSAGE_REACTION_REMOVE_ALL,	   "MESSAGE_REACTION_REMOVE_ALL"},
    {MESSAGE_REACTION_REMOVE_EMOJI,	 "MESSAGE_REACTION_REMOVE_EMOJI"},
    {PRESENCE_UPDATE,	               "PRESENCE_UPDATE"},
    {STAGE_INSTANCE_CREATE,	         "STAGE_INSTANCE_CREATE"},
    {STAGE_INSTANCE_UPDATE,	         "STAGE_INSTANCE_UPDATE"},
    {STAGE_INSTANCE_DELETE,	         "STAGE_INSTANCE_DELETE"},
    {SUBSCRIPTION_CREATE,	           "SUBSCRIPTION_CREATE"},
    {SUBSCRIPTION_UPDATE,	           "SUBSCRIPTION_UPDATE"},
    {SUBSCRIPTION_DELETE,	           "SUBSCRIPTION_DELETE"},
    {TYPING_START,	                  "TYPING_START"},
    {USER_UPDATE,	                   "USER_UPDATE"},
    {VOICE_CHANNEL_EFFECT_SEND,	     "VOICE_CHANNEL_EFFECT_SEND"},
    {VOICE_STATE_UPDATE,	            "VOICE_STATE_UPDATE"},
    {VOICE_SERVER_UPDATE,	           "VOICE_SERVER_UPDATE"},
    {WEBHOOKS_UPDATE,	               "WEBHOOKS_UPDATE"},
    {MESSAGE_POLL_VOTE_ADD,	         "MESSAGE_POLL_VOTE_ADD"},
    {MESSAGE_POLL_VOTE_REMOVE,	      "MESSAGE_POLL_VOTE_REMOVE"},
};

CCORDeventType event_type_from_str(char *t)
{
    for(int i = 0; i < CCORD_EVENT_COUNT; i++)
    {
        if(!strcmp(t, event_str_conversion_tbl[i].str))
            return event_str_conversion_tbl[i].type;
    }

    // Should never reach this (TODO: error handling?)
    return 0;
}

void ccord_register_on_ready(CCORDcontext *cc, default_handle_t handle)    { cc->event_handles[READY] = (generic_handle_t)handle; }

void ccord_register_on_application_command_permissions_update(CCORDcontext *cc, default_handle_t handle)    { cc->event_handles[APPLICATION_COMMAND_PERMISSIONS_UPDATE] = (generic_handle_t)handle; }

void ccord_register_on_auto_moderation_rule_create(CCORDcontext *cc, default_handle_t handle)               { cc->event_handles[AUTO_MODERATION_RULE_CREATE] = (generic_handle_t)handle; }
void ccord_register_on_auto_moderation_rule_update(CCORDcontext *cc, default_handle_t handle)               { cc->event_handles[AUTO_MODERATION_RULE_UPDATE] = (generic_handle_t)handle; }
void ccord_register_on_auto_moderation_rule_delete(CCORDcontext *cc, default_handle_t handle)               { cc->event_handles[AUTO_MODERATION_RULE_DELETE] = (generic_handle_t)handle; }
void ccord_register_on_auto_moderation_action_execution(CCORDcontext *cc, default_handle_t handle)          { cc->event_handles[AUTO_MODERATION_ACTION_EXECUTION] = (generic_handle_t)handle; }

void ccord_register_on_channel_create(CCORDcontext *cc, default_handle_t handle)                            { cc->event_handles[CHANNEL_CREATE] = (generic_handle_t)handle; }
void ccord_register_on_channel_update(CCORDcontext *cc, default_handle_t handle)                            { cc->event_handles[CHANNEL_UPDATE] = (generic_handle_t)handle; }
void ccord_register_on_channel_delete(CCORDcontext *cc, default_handle_t handle)                            { cc->event_handles[CHANNEL_DELETE] = (generic_handle_t)handle; }
void ccord_register_on_channel_pins_update(CCORDcontext *cc, default_handle_t handle)                       { cc->event_handles[CHANNEL_PINS_UPDATE] = (generic_handle_t)handle; }

void ccord_register_on_thread_create(CCORDcontext *cc, default_handle_t handle)                             { cc->event_handles[THREAD_CREATE] = (generic_handle_t)handle; }
void ccord_register_on_thread_update(CCORDcontext *cc, default_handle_t handle)                             { cc->event_handles[THREAD_UPDATE] = (generic_handle_t)handle; }
void ccord_register_on_thread_delete(CCORDcontext *cc, default_handle_t handle)                             { cc->event_handles[THREAD_DELETE] = (generic_handle_t)handle; }
void ccord_register_on_thread_list_sync(CCORDcontext *cc, default_handle_t handle)                          { cc->event_handles[THREAD_LIST_SYNC] = (generic_handle_t)handle; }
void ccord_register_on_thread_member_update(CCORDcontext *cc, default_handle_t handle)                      { cc->event_handles[THREAD_MEMBER_UPDATE] = (generic_handle_t)handle; }
void ccord_register_on_thread_members_update(CCORDcontext *cc, default_handle_t handle)                     { cc->event_handles[THREAD_MEMBERS_UPDATE] = (generic_handle_t)handle; }

void ccord_register_on_entitlement_create(CCORDcontext *cc, default_handle_t handle)                        { cc->event_handles[ENTITLEMENT_CREATE] = (generic_handle_t)handle; }
void ccord_register_on_entitlement_update(CCORDcontext *cc, default_handle_t handle)                        { cc->event_handles[ENTITLEMENT_UPDATE] = (generic_handle_t)handle; }
void ccord_register_on_entitlement_delete(CCORDcontext *cc, default_handle_t handle)                        { cc->event_handles[ENTITLEMENT_DELETE] = (generic_handle_t)handle; }
    
void ccord_register_on_guild_create(CCORDcontext *cc, default_handle_t handle)                              { cc->event_handles[GUILD_CREATE] = (generic_handle_t)handle; }
void ccord_register_on_guild_update(CCORDcontext *cc, default_handle_t handle)                              { cc->event_handles[GUILD_UPDATE] = (generic_handle_t)handle; }
void ccord_register_on_guild_delete(CCORDcontext *cc, default_handle_t handle)                              { cc->event_handles[GUILD_DELETE] = (generic_handle_t)handle; }

void ccord_register_on_guild_audit_log_entry_create(CCORDcontext *cc, default_handle_t handle)              { cc->event_handles[GUILD_AUDIT_LOG_ENTRY_CREATE] = (generic_handle_t)handle; }

void ccord_register_on_guild_ban_add(CCORDcontext *cc, default_handle_t handle)                             { cc->event_handles[GUILD_BAN_ADD] = (generic_handle_t)handle; }
void ccord_register_on_guild_ban_remove(CCORDcontext *cc, default_handle_t handle)                          { cc->event_handles[GUILD_BAN_REMOVE] = (generic_handle_t)handle; }

void ccord_register_on_guild_emojis_update(CCORDcontext *cc, default_handle_t handle)                       { cc->event_handles[GUILD_EMOJIS_UPDATE] = (generic_handle_t)handle; }
void ccord_register_on_guild_stickers_update(CCORDcontext *cc, default_handle_t handle)                     { cc->event_handles[GUILD_STICKERS_UPDATE] = (generic_handle_t)handle; }
void ccord_register_on_guild_integrations_update(CCORDcontext *cc, default_handle_t handle)                 { cc->event_handles[GUILD_INTEGRATIONS_UPDATE] = (generic_handle_t)handle; }

void ccord_register_on_guild_member_add(CCORDcontext *cc, default_handle_t handle)                          { cc->event_handles[GUILD_MEMBER_ADD] = (generic_handle_t)handle; }
void ccord_register_on_guild_member_remove(CCORDcontext *cc, default_handle_t handle)                       { cc->event_handles[GUILD_MEMBER_REMOVE] = (generic_handle_t)handle; }
void ccord_register_on_guild_member_update(CCORDcontext *cc, default_handle_t handle)                       { cc->event_handles[GUILD_MEMBER_UPDATE] = (generic_handle_t)handle; }
void ccord_register_on_guild_members_chunk(CCORDcontext *cc, default_handle_t handle)                       { cc->event_handles[GUILD_MEMBERS_CHUNK] = (generic_handle_t)handle; }

void ccord_register_on_guild_role_create(CCORDcontext *cc, default_handle_t handle)                         { cc->event_handles[GUILD_ROLE_CREATE] = (generic_handle_t)handle; }
void ccord_register_on_guild_role_update(CCORDcontext *cc, default_handle_t handle)                         { cc->event_handles[GUILD_ROLE_UPDATE] = (generic_handle_t)handle; }
void ccord_register_on_guild_role_delete(CCORDcontext *cc, default_handle_t handle)                         { cc->event_handles[GUILD_ROLE_DELETE] = (generic_handle_t)handle; }

void ccord_register_on_guild_scheduled_event_create(CCORDcontext *cc, default_handle_t handle)              { cc->event_handles[GUILD_SCHEDULED_EVENT_CREATE] = (generic_handle_t)handle; }	  
void ccord_register_on_guild_scheduled_event_update(CCORDcontext *cc, default_handle_t handle)              { cc->event_handles[GUILD_SCHEDULED_EVENT_UPDATE] = (generic_handle_t)handle; }	        // Guild scheduled event was updated
void ccord_register_on_guild_scheduled_event_delete(CCORDcontext *cc, default_handle_t handle)              { cc->event_handles[GUILD_SCHEDULED_EVENT_DELETE] = (generic_handle_t)handle; }	        // Guild scheduled event was deleted
void ccord_register_on_guild_scheduled_event_user_add(CCORDcontext *cc, default_handle_t handle)            { cc->event_handles[GUILD_SCHEDULED_EVENT_USER_ADD] = (generic_handle_t)handle; }	    // User subscribed to a guild scheduled event
void ccord_register_on_guild_scheduled_event_user_remove(CCORDcontext *cc, default_handle_t handle)         { cc->event_handles[GUILD_SCHEDULED_EVENT_USER_REMOVE] = (generic_handle_t)handle; }	// User unsubscribed from a guild scheduled event

void ccord_register_on_guild_soundboard_sound_create(CCORDcontext *cc, default_handle_t handle)             { cc->event_handles[GUILD_SOUNDBOARD_SOUND_CREATE] = (generic_handle_t)handle; }	    // Guild soundboard sound was created
void ccord_register_on_guild_soundboard_sound_update(CCORDcontext *cc, default_handle_t handle)             { cc->event_handles[GUILD_SOUNDBOARD_SOUND_UPDATE] = (generic_handle_t)handle; }	    // Guild soundboard sound was updated
void ccord_register_on_guild_soundboard_sound_delete(CCORDcontext *cc, default_handle_t handle)             { cc->event_handles[GUILD_SOUNDBOARD_SOUND_DELETE] = (generic_handle_t)handle; }	    // Guild soundboard sound was deleted
void ccord_register_on_guild_soundboard_sounds_update(CCORDcontext *cc, default_handle_t handle)            { cc->event_handles[GUILD_SOUNDBOARD_SOUNDS_UPDATE] = (generic_handle_t)handle; }	    // Guild soundboard sounds were updated

void ccord_register_on_soundboard_sounds(CCORDcontext *cc, default_handle_t handle)                         { cc->event_handles[SOUNDBOARD_SOUNDS] = (generic_handle_t)handle; }	                // Response to Request Soundboard Sounds

void ccord_register_on_integration_create(CCORDcontext *cc, default_handle_t handle)                        { cc->event_handles[INTEGRATION_CREATE] = (generic_handle_t)handle; } 	            // Guild integration was created
void ccord_register_on_integration_update(CCORDcontext *cc, default_handle_t handle)                        { cc->event_handles[INTEGRATION_UPDATE] = (generic_handle_t)handle; } 	            // Guild integration was updated
void ccord_register_on_integration_delete(CCORDcontext *cc, default_handle_t handle)                        { cc->event_handles[INTEGRATION_DELETE] = (generic_handle_t)handle; } 	            // Guild integration was deleted

void ccord_register_on_interaction_create(CCORDcontext *cc, default_handle_t handle)                        { cc->event_handles[INTERACTION_CREATE] = (generic_handle_t)handle; } 	            // User used an interaction(CCORDcontext *cc, default_handle_t handle) { cc->event_handles[] = (generic_handle_t)handle; } such as an Application Command

void ccord_register_on_invite_create(CCORDcontext *cc, default_handle_t handle)                             { cc->event_handles[INVITE_CREATE] = (generic_handle_t)handle; }	                    // Invite to a channel was created
void ccord_register_on_invite_delete(CCORDcontext *cc, default_handle_t handle)                             { cc->event_handles[INVITE_DELETE] = (generic_handle_t)handle; }	                    // Invite to a channel was deleted

void ccord_register_on_message_create(CCORDcontext *cc, default_handle_t handle)                            { cc->event_handles[MESSAGE_CREATE] = (generic_handle_t)handle; }                 	// Message was created
void ccord_register_on_message_update(CCORDcontext *cc, default_handle_t handle)                            { cc->event_handles[MESSAGE_UPDATE] = (generic_handle_t)handle; }                 	// Message was edited
void ccord_register_on_message_delete(CCORDcontext *cc, default_handle_t handle)                            { cc->event_handles[MESSAGE_DELETE] = (generic_handle_t)handle; }                 	// Message was deleted
void ccord_register_on_message_delete_bulk(CCORDcontext *cc, default_handle_t handle)                       { cc->event_handles[MESSAGE_DELETE_BULK] = (generic_handle_t)handle; }	            // Multiple messages were deleted at once

void ccord_register_on_message_reaction_add(CCORDcontext *cc, default_handle_t handle)                      { cc->event_handles[MESSAGE_REACTION_ADD] = (generic_handle_t)handle; }	            // User reacted to a message
void ccord_register_on_message_reaction_remove(CCORDcontext *cc, default_handle_t handle)                   { cc->event_handles[MESSAGE_REACTION_REMOVE] = (generic_handle_t)handle; }	        // User removed a reaction from a message
void ccord_register_on_message_reaction_remove_all(CCORDcontext *cc, default_handle_t handle)               { cc->event_handles[MESSAGE_REACTION_REMOVE_ALL] = (generic_handle_t)handle; }	    // All reactions were explicitly removed from a message
void ccord_register_on_message_reaction_remove_emoji(CCORDcontext *cc, default_handle_t handle)             { cc->event_handles[MESSAGE_REACTION_REMOVE_EMOJI] = (generic_handle_t)handle; }	    // All reactions for a given emoji were explicitly removed from a message

void ccord_register_on_presence_update(CCORDcontext *cc, default_handle_t handle)                           { cc->event_handles[PRESENCE_UPDATE] = (generic_handle_t)handle; }	                // User was updated

void ccord_register_on_stage_instance_create(CCORDcontext *cc, default_handle_t handle)                     { cc->event_handles[STAGE_INSTANCE_CREATE] = (generic_handle_t)handle; }	            // Stage instance was created
void ccord_register_on_stage_instance_update(CCORDcontext *cc, default_handle_t handle)                     { cc->event_handles[STAGE_INSTANCE_UPDATE] = (generic_handle_t)handle; }	            // Stage instance was updated
void ccord_register_on_stage_instance_delete(CCORDcontext *cc, default_handle_t handle)                     { cc->event_handles[STAGE_INSTANCE_DELETE] = (generic_handle_t)handle; }	            // Stage instance was deleted or closed

void ccord_register_on_subscription_create(CCORDcontext *cc, default_handle_t handle)                       { cc->event_handles[SUBSCRIPTION_CREATE] = (generic_handle_t)handle; }	            // Premium App Subscription was created
void ccord_register_on_subscription_update(CCORDcontext *cc, default_handle_t handle)                       { cc->event_handles[SUBSCRIPTION_UPDATE] = (generic_handle_t)handle; }	            // Premium App Subscription was updated
void ccord_register_on_subscription_delete(CCORDcontext *cc, default_handle_t handle)                       { cc->event_handles[SUBSCRIPTION_DELETE] = (generic_handle_t)handle; }	            // Premium App Subscription was deleted

void ccord_register_on_typing_start(CCORDcontext *cc, default_handle_t handle)                              { cc->event_handles[TYPING_START] = (generic_handle_t)handle; }	                    // User started typing in a channel

void ccord_register_on_user_update(CCORDcontext *cc, default_handle_t handle)                               { cc->event_handles[USER_UPDATE] = (generic_handle_t)handle; }	                    // Properties about the user changed

void ccord_register_on_voice_channel_effect_send(CCORDcontext *cc, default_handle_t handle)                 { cc->event_handles[VOICE_CHANNEL_EFFECT_SEND] = (generic_handle_t)handle; }	        // Someone sent an effect in a voice channel the current user is connected to
void ccord_register_on_voice_state_update(CCORDcontext *cc, default_handle_t handle)                        { cc->event_handles[VOICE_STATE_UPDATE] = (generic_handle_t)handle; }	                // Someone joined(CCORDcontext *cc, default_handle_t handle) { cc->event_handles[] = (generic_handle_t)handle; } left(CCORDcontext *cc, default_handle_t handle) { cc->event_handles[] = (generic_handle_t)handle; } or moved a voice channel
void ccord_register_on_voice_server_update(CCORDcontext *cc, default_handle_t handle)                       { cc->event_handles[VOICE_SERVER_UPDATE] = (generic_handle_t)handle; }	            // Guild's voice server was updated

void ccord_register_on_webhooks_update(CCORDcontext *cc, default_handle_t handle)                           { cc->event_handles[WEBHOOKS_UPDATE] = (generic_handle_t)handle; }	                // Guild channel webhook was created(CCORDcontext *cc, default_handle_t handle) { cc->event_handles[] = (generic_handle_t)handle; } update(CCORDcontext *cc, default_handle_t handle) { cc->event_handles[] = (generic_handle_t)handle; } or deleted

void ccord_register_on_message_poll_vote_add(CCORDcontext *cc, default_handle_t handle)                     { cc->event_handles[MESSAGE_POLL_VOTE_ADD] = (generic_handle_t)handle; }	            // User voted on a poll
void ccord_register_on_message_poll_vote_remove(CCORDcontext *cc, default_handle_t handle)                  { cc->event_handles[MESSAGE_POLL_VOTE_REMOVE] = (generic_handle_t)handle; }	        // User removed a vote on a poll