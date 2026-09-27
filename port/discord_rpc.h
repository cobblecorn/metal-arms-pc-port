// Discord Rich Presence for the PC port, spoken directly over the local Discord client's IPC pipe
// (\\.\pipe\discord-ipc-N), so there is no SDK or DLL to ship.
//
// It needs a Discord application ID (create an application at discord.com/developers; its name and
// app icon are what Discord shows as "Playing ..."). Pass it with -discord-app-id <id> or
// MA_PORT_DISCORD_APP_ID. A rich-presence image asset key or URL is optional. Without an app ID, or
// without a running Discord client, every call does nothing. All pipe I/O happens on a background
// thread; the game only hands over strings.
#pragma once

// Where connection changes and Discord's error answers are reported (called on the worker thread).
void discord_SetLog(void (*log)(const char *text));
bool discord_Start(const char *appId, const char *largeImage = 0, const char *largeText = 0);
// Sets what Discord shows under the game's name. resetTimer restarts the "elapsed" clock.
void discord_SetActivity(const char *details, const char *state, bool resetTimer);
void discord_Stop();
