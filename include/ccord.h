#ifndef CCORD_H
#define CCORD_H

#include <stdlib.h>
#include <stdbool.h>

// Intents
typedef enum {
    GUILDS                          = (1 << 0),
    GUILD_MEMBERS                   = (1 << 1),
    GUILD_MODERATION                = (1 << 2),
    GUILD_EXPRESSIONS               = (1 << 3),
    GUILD_INTEGRATIONS              = (1 << 4),
    GUILD_WEBHOOKS                  = (1 << 5),
    GUILD_INVITES                   = (1 << 6),
    GUILD_VOICE_STATES              = (1 << 7),
    GUILD_PRESENCES                 = (1 << 8),
    GUILD_MESSAGES                  = (1 << 9),
    GUILD_MESSAGE_REACTIONS         = (1 << 10),
    GUILD_MESSAGE_TYPING            = (1 << 11),
    DIRECT_MESSAGES                 = (1 << 12),
    DIRECT_MESSAGE_REACTIONS        = (1 << 13),
    DIRECT_MESSAGE_TYPING           = (1 << 14),
    MESSAGE_CONTENT                 = (1 << 15),
    GUILD_SCHEDULED_EVENTS          = (1 << 16),
    AUTO_MODERATION_CONFIGURATION   = (1 << 20),
    AUTO_MODERATION_EXECUTION       = (1 << 21),
    GUILD_MESSAGE_POLLS             = (1 << 24),
    DIRECT_MESSAGE_POLLS            = (1 << 25),
} CCORDintent;

typedef struct CCORDcontext CCORDcontext;

typedef enum {
    GET,
    POST
} CCORDrequestType;

CCORDcontext   *ccord_init(const char *token);
int             ccord_login(CCORDcontext *ccord, int intents);
int             ccord_request(CCORDcontext *ccord, CCORDrequestType type, const char *url, const char *send, size_t send_size, char **recv, size_t *recv_size);
void            ccord_free(CCORDcontext *ccord);



// -----------------------
// DISCORD DATA STRUCTURES
// -----------------------

/*
#define optional_type(type) struct { bool present; type value; }


typedef uint64_t CCORDsnowflake;

typedef optional_type(CCORDsnowflake) OptCCORDsnowflake;
typedef optional_type(int) OptInt;

typedef enum {
    GUILD_TEXT = 0,             // a text channel within a server
    DM = 1,                     // a direct message between users
    GUILD_VOICE = 2,            // a voice channel within a server
    GROUP_DM = 3,               // a direct message between multiple users
    GUILD_CATEGORY = 4,         // an organizational category that contains up to 50 channels
    GUILD_ANNOUNCEMENT = 5,     // a channel that users can follow and crosspost into their own server (formerly news channels)
    ANNOUNCEMENT_THREAD	= 10,   // a temporary sub-channel within a GUILD_ANNOUNCEMENT channel
    PUBLIC_THREAD = 11,         // a temporary sub-channel within a GUILD_TEXT or GUILD_FORUM channel
    PRIVATE_THREAD = 12,        // a temporary sub-channel within a GUILD_TEXT channel that is only viewable by those invited and those with the MANAGE_THREADS permission
    GUILD_STAGE_VOICE = 13,     // a voice channel for hosting events with an audience
    GUILD_DIRECTORY	= 14,       // the channel in a hub containing the listed servers
    GUILD_FORUM	= 15,           // Channel that can only contain threads
    GUILD_MEDIA	= 16,           // Channel that can only contain threads, similar to GUILD_FORUM channels
} CCORDchannelType;

typedef struct {
    CCORDsnowflake id;
    int type; // either 0 (role) or 1 (member)
    
    // https://discord.com/developers/docs/topics/permissions#permissions These values are serialized as a string. They do not go beyond 31 bits, so the limit is 10 digits + 1 for escape char
    char allow[11];
    char deny[11];
} CCORDoverwrite;

typedef struct {
    CCORDsnowflake id;                  // the id of this channel
    CCORDchannelType type;              // the type of channel

    bool has_guild_id;
    CCORDsnowflake guild_id;    // the id of the guild (may be missing for some channel objects received over gateway guild dispatches)

    bool has_position;
    int position;	            // sorting position of the channel (channels with the same position are sorted by id)

    bool has_permission_overwrites;
    int permission_overwrites_count;
    CCORDoverwrite *permission_overwrites;

    bool has_name;
    char name[101];

    bool has_topic;
topic?	?string	the channel topic (0-4096 characters for GUILD_FORUM and GUILD_MEDIA channels, 0-1024 characters for all others)
nsfw?	boolean	whether the channel is nsfw
last_message_id?	?snowflake	the id of the last message sent in this channel (or thread for GUILD_FORUM or GUILD_MEDIA channels) (may not point to an existing or valid message or thread)
bitrate?	integer	the bitrate (in bits) of the voice channel
user_limit?	integer	the user limit of the voice channel
rate_limit_per_user?*	integer	amount of seconds a user has to wait before sending another message (0-21600); bots, as well as users with the permission manage_messages or manage_channel, are unaffected
recipients?	array of user objects	the recipients of the DM
icon?	?string	icon hash of the group DM
owner_id?	snowflake	id of the creator of the group DM or thread
application_id?	snowflake	application id of the group DM creator if it is bot-created
managed?	boolean	for group DM channels: whether the channel is managed by an application via the gdm.join OAuth2 scope
parent_id?	?snowflake	for guild channels: id of the parent category for a channel (each parent category can contain up to 50 channels), for threads: id of the text channel this thread was created
last_pin_timestamp?	?ISO8601 timestamp	when the last pinned message was pinned. This may be null in events such as GUILD_CREATE when a message is not pinned.
rtc_region?	?string	voice region id for the voice channel, automatic when set to null
video_quality_mode?	integer	the camera video quality mode of the voice channel, 1 when not present
message_count?**	integer	number of messages (not including the initial message or deleted messages) in a thread.
member_count?	integer	an approximate count of users in a thread, stops counting at 50
thread_metadata?	a thread metadata object	thread-specific fields not needed by other channels
member?	a thread member object	thread member object for the current user, if they have joined the thread, only included on certain API endpoints
default_auto_archive_duration?	integer	default duration, copied onto newly created threads, in minutes, threads will stop showing in the channel list after the specified period of inactivity, can be set to: 60, 1440, 4320, 10080
permissions?	string	computed permissions for the invoking user in the channel, including overwrites, only included when part of the resolved data received on a slash command interaction. This does not include implicit permissions, which may need to be checked separately
flags?	integer	channel flags combined as a bitfield
total_message_sent?	integer	number of messages ever sent in a thread, it's similar to message_count on message creation, but will not decrement the number when a message is deleted
available_tags?	array of tag objects	the set of tags that can be used in a GUILD_FORUM or a GUILD_MEDIA channel
applied_tags?	array of snowflakes	the IDs of the set of tags that have been applied to a thread in a GUILD_FORUM or a GUILD_MEDIA channel
default_reaction_emoji?	?default reaction object	the emoji to show in the add reaction button on a thread in a GUILD_FORUM or a GUILD_MEDIA channel
default_thread_rate_limit_per_user?	integer	the initial rate_limit_per_user to set on newly created threads in a channel. this field is copied to the thread at creation time and does not live update.
default_sort_order?	?integer	the default sort order type used to order posts in GUILD_FORUM and GUILD_MEDIA channels. Defaults to null, which indicates a preferred sort order hasn't been set by a channel admin
default_forum_layout?	integer	the default forum layout view used to display posts in GUILD_FORUM channels. Defaults to 0, which indicates a layout view has not been set by a channel admin
} CCORDchannel;
*/
#endif