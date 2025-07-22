#ifndef EVENT_H
#define EVENT_H

#include "ccord.h"
#include "cJSON.h"

typedef enum {
    READY,
    APPLICATION_COMMAND_PERMISSIONS_UPDATE,	// Application command permission was updated
    AUTO_MODERATION_RULE_CREATE,	        // Auto Moderation rule was created
    AUTO_MODERATION_RULE_UPDATE,	        // Auto Moderation rule was updated
    AUTO_MODERATION_RULE_DELETE,	        // Auto Moderation rule was deleted
    AUTO_MODERATION_ACTION_EXECUTION,	    // Auto Moderation rule was triggered and an action was executed (e.g. a message was blocked)
    CHANNEL_CREATE,	                        // New guild channel created
    CHANNEL_UPDATE,	                        // Channel was updated
    CHANNEL_DELETE,	                        // Channel was deleted
    CHANNEL_PINS_UPDATE,	                // Message was pinned or unpinned
    THREAD_CREATE,	                        // Thread created, also sent when being added to a private thread
    THREAD_UPDATE,	                        // Thread was updated
    THREAD_DELETE,	                        // Thread was deleted
    THREAD_LIST_SYNC,	                    // Sent when gaining access to a channel, contains all active threads in that channel
    THREAD_MEMBER_UPDATE,	                // Thread member for the current user was updated
    THREAD_MEMBERS_UPDATE,	                // Some user(s) were added to or removed from a thread
    ENTITLEMENT_CREATE,	                    // Entitlement was created
    ENTITLEMENT_UPDATE,	                    // Entitlement was updated or renewed
    ENTITLEMENT_DELETE,	                    // Entitlement was deleted
    GUILD_CREATE,	                        // Lazy-load for unavailable guild, guild became available, or user joined a new guild
    GUILD_UPDATE,	                        // Guild was updated
    GUILD_DELETE,	                        // Guild became unavailable, or user left/was removed from a guild
    GUILD_AUDIT_LOG_ENTRY_CREATE,	        // A guild audit log entry was created
    GUILD_BAN_ADD,	                        // User was banned from a guild
    GUILD_BAN_REMOVE,	                    // User was unbanned from a guild
    GUILD_EMOJIS_UPDATE,	                // Guild emojis were updated
    GUILD_STICKERS_UPDATE,	                // Guild stickers were updated
    GUILD_INTEGRATIONS_UPDATE,	            // Guild integration was updated
    GUILD_MEMBER_ADD,	                    // New user joined a guild
    GUILD_MEMBER_REMOVE,	                // User was removed from a guild
    GUILD_MEMBER_UPDATE,	                // Guild member was updated
    GUILD_MEMBERS_CHUNK,	                // Response to Request Guild Members
    GUILD_ROLE_CREATE,	                    // Guild role was created
    GUILD_ROLE_UPDATE,	                    // Guild role was updated
    GUILD_ROLE_DELETE,	                    // Guild role was deleted
    GUILD_SCHEDULED_EVENT_CREATE,	        // Guild scheduled event was created
    GUILD_SCHEDULED_EVENT_UPDATE,	        // Guild scheduled event was updated
    GUILD_SCHEDULED_EVENT_DELETE,	        // Guild scheduled event was deleted
    GUILD_SCHEDULED_EVENT_USER_ADD,	    // User subscribed to a guild scheduled event
    GUILD_SCHEDULED_EVENT_USER_REMOVE,	// User unsubscribed from a guild scheduled event
    GUILD_SOUNDBOARD_SOUND_CREATE,	    // Guild soundboard sound was created
    GUILD_SOUNDBOARD_SOUND_UPDATE,	    // Guild soundboard sound was updated
    GUILD_SOUNDBOARD_SOUND_DELETE,	    // Guild soundboard sound was deleted
    GUILD_SOUNDBOARD_SOUNDS_UPDATE,	    // Guild soundboard sounds were updated
    SOUNDBOARD_SOUNDS,	                // Response to Request Soundboard Sounds
    INTEGRATION_CREATE, 	            // Guild integration was created
    INTEGRATION_UPDATE, 	            // Guild integration was updated
    INTEGRATION_DELETE, 	            // Guild integration was deleted
    INTERACTION_CREATE, 	            // User used an interaction, such as an Application Command
    INVITE_CREATE,	                    // Invite to a channel was created
    INVITE_DELETE,	                    // Invite to a channel was deleted
    MESSAGE_CREATE,                 	// Message was created
    MESSAGE_UPDATE,                 	// Message was edited
    MESSAGE_DELETE,                 	// Message was deleted
    MESSAGE_DELETE_BULK,	            // Multiple messages were deleted at once
    MESSAGE_REACTION_ADD,	            // User reacted to a message
    MESSAGE_REACTION_REMOVE,	        // User removed a reaction from a message
    MESSAGE_REACTION_REMOVE_ALL,	    // All reactions were explicitly removed from a message
    MESSAGE_REACTION_REMOVE_EMOJI,	    // All reactions for a given emoji were explicitly removed from a message
    PRESENCE_UPDATE,	                // User was updated
    STAGE_INSTANCE_CREATE,	            // Stage instance was created
    STAGE_INSTANCE_UPDATE,	            // Stage instance was updated
    STAGE_INSTANCE_DELETE,	            // Stage instance was deleted or closed
    SUBSCRIPTION_CREATE,	            // Premium App Subscription was created
    SUBSCRIPTION_UPDATE,	            // Premium App Subscription was updated
    SUBSCRIPTION_DELETE,	            // Premium App Subscription was deleted
    TYPING_START,	                    // User started typing in a channel
    USER_UPDATE,	                    // Properties about the user changed
    VOICE_CHANNEL_EFFECT_SEND,	        // Someone sent an effect in a voice channel the current user is connected to
    VOICE_STATE_UPDATE,	                // Someone joined, left, or moved a voice channel
    VOICE_SERVER_UPDATE,	            // Guild's voice server was updated
    WEBHOOKS_UPDATE,	                // Guild channel webhook was created, update, or deleted
    MESSAGE_POLL_VOTE_ADD,	            // User voted on a poll
    MESSAGE_POLL_VOTE_REMOVE,	        // User removed a vote on a poll
    CCORD_EVENT_COUNT,
} CCORDeventType;

CCORDeventType event_type_from_str(char *t);

typedef void(*generic_handle_t)(void);
typedef void(*default_handle_t)(cJSON *json);

void ccord_dispatch_events(CCORDcontext *cc);

// These events currently just return a cJSON object as a parameter to the handle.
//=========================
void ccord_register_on_ready(CCORDcontext *cc, default_handle_t handle);

void ccord_register_on_application_command_permissions_update(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_auto_moderation_rule_create(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_auto_moderation_rule_update(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_auto_moderation_rule_delete(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_auto_moderation_action_execution(CCORDcontext *cc, default_handle_t handle);

void ccord_register_on_channel_create(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_channel_update(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_channel_delete(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_channel_pins_update(CCORDcontext *cc, default_handle_t handle);

void ccord_register_on_thread_create(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_thread_update(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_thread_delete(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_thread_list_sync(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_thread_member_update(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_thread_members_update(CCORDcontext *cc, default_handle_t handle);

void ccord_register_on_entitlement_create(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_entitlement_update(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_entitlement_delete(CCORDcontext *cc, default_handle_t handle);
    
void ccord_register_on_guild_create(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_guild_update(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_guild_delete(CCORDcontext *cc, default_handle_t handle);

void ccord_register_on_guild_audit_log_entry_create(CCORDcontext *cc, default_handle_t handle);

void ccord_register_on_guild_ban_add(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_guild_ban_remove(CCORDcontext *cc, default_handle_t handle);

void ccord_register_on_guild_emojis_update(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_guild_stickers_update(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_guild_integrations_update(CCORDcontext *cc, default_handle_t handle);

void ccord_register_on_guild_member_add(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_guild_member_remove(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_guild_member_update(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_guild_members_chunk(CCORDcontext *cc, default_handle_t handle);

void ccord_register_on_guild_role_create(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_guild_role_update(CCORDcontext *cc, default_handle_t handle);
void ccord_register_on_guild_role_delete(CCORDcontext *cc, default_handle_t handle);

void ccord_register_on_guild_scheduled_event_create(CCORDcontext*, default_handle_t handle);
void ccord_register_on_guild_scheduled_event_update(CCORDcontext*, default_handle_t handle);
void ccord_register_on_guild_scheduled_event_delete(CCORDcontext*, default_handle_t handle);
void ccord_register_on_guild_scheduled_event_user_add(CCORDcontext*, default_handle_t handle);
void ccord_register_on_guild_scheduled_event_user_remove(CCORDcontext*, default_handle_t handle);

void ccord_register_on_guild_soundboard_sound_create(CCORDcontext*, default_handle_t handle);
void ccord_register_on_guild_soundboard_sound_update(CCORDcontext*, default_handle_t handle);
void ccord_register_on_guild_soundboard_sound_delete(CCORDcontext*, default_handle_t handle);
void ccord_register_on_guild_soundboard_sounds_update(CCORDcontext*, default_handle_t handle);

void ccord_register_on_soundboard_sounds(CCORDcontext*, default_handle_t handle);

void ccord_register_on_integration_create(CCORDcontext*, default_handle_t handle);
void ccord_register_on_integration_update(CCORDcontext*, default_handle_t handle);
void ccord_register_on_integration_delete(CCORDcontext*, default_handle_t handle);
void ccord_register_on_interaction_create(CCORDcontext*, default_handle_t handle);

void ccord_register_on_invite_create(CCORDcontext*, default_handle_t handle);
void ccord_register_on_invite_delete(CCORDcontext*, default_handle_t handle);

void ccord_register_on_message_create(CCORDcontext*, default_handle_t handle);
void ccord_register_on_message_update(CCORDcontext*, default_handle_t handle);
void ccord_register_on_message_delete(CCORDcontext*, default_handle_t handle);
void ccord_register_on_message_delete_bulk(CCORDcontext*, default_handle_t handle);
void ccord_register_on_message_reaction_add(CCORDcontext*, default_handle_t handle);
void ccord_register_on_message_reaction_remove(CCORDcontext*, default_handle_t handle);
void ccord_register_on_message_reaction_remove_all(CCORDcontext*, default_handle_t handle);
void ccord_register_on_message_reaction_remove_emoji(CCORDcontext*, default_handle_t handle);

void ccord_register_on_presence_update(CCORDcontext*, default_handle_t handle);

void ccord_register_on_stage_instance_create(CCORDcontext*, default_handle_t handle);
void ccord_register_on_stage_instance_update(CCORDcontext*, default_handle_t handle);
void ccord_register_on_stage_instance_delete(CCORDcontext*, default_handle_t handle);

void ccord_register_on_subscription_create(CCORDcontext*, default_handle_t handle);
void ccord_register_on_subscription_update(CCORDcontext*, default_handle_t handle);
void ccord_register_on_subscription_delete(CCORDcontext*, default_handle_t handle);

void ccord_register_on_typing_start(CCORDcontext*, default_handle_t handle);

void ccord_register_on_user_update(CCORDcontext*, default_handle_t handle);

void ccord_register_on_voice_channel_effect_send(CCORDcontext*, default_handle_t handle);
void ccord_register_on_voice_state_update(CCORDcontext*, default_handle_t handle);
void ccord_register_on_voice_server_update(CCORDcontext*, default_handle_t handle);

void ccord_register_on_webhooks_update(CCORDcontext*, default_handle_t handle);

void ccord_register_on_message_poll_vote_add(CCORDcontext*, default_handle_t handle);
void ccord_register_on_message_poll_vote_remove(CCORDcontext*, default_handle_t handle);

#endif