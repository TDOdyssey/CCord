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

The project is currently undergoing big changes which include a lot of restructering of existing code. The only thing supported right now is sending and receiving payloads from discord's rest API. This is done by initializing the library with `ccord_init`, which requires a token as input and returns an opaque context pointer. To make a request, use `ccord_request`. At the moment, you have to write your payload manually into a buffer. This function has a lot of details and edge-cases, and more documentation will be provided at a later date. You will also have to free the context pointer by calling `ccord_free`.

## TODO

Once the gateway event system has been rewritten, the next goal will be to provide wrapper structures and functions for handling discord JSON objects, including messages, channels, guilds, etc.

### Refactoring
✅ General CCord interface

⬛ Gateway connection

⬛ Event system

### Finish implementation
⬛ WebSockets should be able to handle multiple frames

⬛ Add functions for common discord API operations

⬛ Voice/audio system

### General
⬛ FIX: Gateway protocols do not disconnect properly and do not support resuming