// Discord Rich Presence over the Discord client's local IPC pipe. See discord_rpc.h.
//
// The pipe carries frames of { u32 opcode, u32 length, JSON }: opcode 0 is the handshake
// ({"v":1,"client_id":...}), 1 a command frame (SET_ACTIVITY), 2 close. Discord answers each frame;
// the answers are read and ignored.

#include "discord_rpc.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static CRITICAL_SECTION s_lock;
static HANDLE s_thread, s_wake;
static volatile LONG s_quit;
static char s_appId[32];
static char s_details[128], s_state[128];
static __int64 s_startTime;
static bool s_dirty;

static void JsonString(char *out, size_t size, const char *text) {
	size_t n = 0;
	if (n + 1 < size) out[n++] = '"';
	for (const char *p = text; *p && n + 7 < size; p++) {
		const unsigned char c = (unsigned char)*p;
		if (c == '"' || c == '\\') { out[n++] = '\\'; out[n++] = (char)c; }
		else if (c < 0x20) n += sprintf(out + n, "\\u%04x", c);
		else out[n++] = (char)c;
	}
	if (n + 1 < size) out[n++] = '"';
	out[n < size ? n : size - 1] = 0;
}

static bool WriteFrame(HANDLE pipe, unsigned opcode, const char *json) {
	const DWORD length = (DWORD)strlen(json);
	char header[8];
	memcpy(header, &opcode, 4);
	memcpy(header + 4, &length, 4);
	DWORD written;
	return WriteFile(pipe, header, 8, &written, NULL) && written == 8 &&
		WriteFile(pipe, json, length, &written, NULL) && written == length;
}

// Reads and discards one frame (Discord's reply); false when the pipe broke.
static bool ReadFrame(HANDLE pipe) {
	char header[8];
	DWORD read;
	if (!ReadFile(pipe, header, 8, &read, NULL) || read != 8) return false;
	DWORD length;
	memcpy(&length, header + 4, 4);
	char buffer[512];
	while (length) {
		const DWORD chunk = length < sizeof(buffer) ? length : (DWORD)sizeof(buffer);
		if (!ReadFile(pipe, buffer, chunk, &read, NULL) || !read) return false;
		length -= read;
	}
	return true;
}

static HANDLE Connect() {
	for (int i = 0; i < 10; i++) {
		char name[64];
		sprintf(name, "\\\\.\\pipe\\discord-ipc-%d", i);
		HANDLE pipe = CreateFileA(name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (pipe == INVALID_HANDLE_VALUE) continue;
		char json[128];
		sprintf(json, "{\"v\":1,\"client_id\":\"%s\"}", s_appId);
		if (WriteFrame(pipe, 0, json) && ReadFrame(pipe)) return pipe;
		CloseHandle(pipe);
	}
	return INVALID_HANDLE_VALUE;
}

static DWORD WINAPI Worker(void *) {
	HANDLE pipe = INVALID_HANDLE_VALUE;
	unsigned nonce = 0;
	while (!InterlockedCompareExchange(&s_quit, 0, 0)) {
		if (pipe == INVALID_HANDLE_VALUE) {
			pipe = Connect();
			if (pipe == INVALID_HANDLE_VALUE) {
				WaitForSingleObject(s_wake, 15000);	// Discord not running: look again later
				continue;
			}
			EnterCriticalSection(&s_lock);
			s_dirty = true;	// (re)connected: send the current activity
			LeaveCriticalSection(&s_lock);
		}

		char details[300], state[300];
		__int64 start;
		bool send;
		EnterCriticalSection(&s_lock);
		send = s_dirty;
		s_dirty = false;
		JsonString(details, sizeof(details), s_details);
		JsonString(state, sizeof(state), s_state);
		start = s_startTime;
		LeaveCriticalSection(&s_lock);

		if (send && details[1] != '"') {
			char json[1024];
			sprintf(json, "{\"cmd\":\"SET_ACTIVITY\",\"args\":{\"pid\":%lu,\"activity\":{\"details\":%s%s%s,"
				"\"timestamps\":{\"start\":%lld}}},\"nonce\":\"%u\"}",
				GetCurrentProcessId(), details, state[1] != '"' ? ",\"state\":" : "", state[1] != '"' ? state : "",
				start, ++nonce);
			if (!WriteFrame(pipe, 1, json) || !ReadFrame(pipe)) {
				CloseHandle(pipe);
				pipe = INVALID_HANDLE_VALUE;
				continue;
			}
		}
		WaitForSingleObject(s_wake, INFINITE);
	}
	if (pipe != INVALID_HANDLE_VALUE) {
		WriteFrame(pipe, 2, "{}");
		CloseHandle(pipe);
	}
	return 0;
}

bool discord_Start(const char *appId) {
	if (s_thread || !appId || !appId[0] || strlen(appId) >= sizeof(s_appId)) return false;
	for (const char *p = appId; *p; p++) {
		if (*p < '0' || *p > '9') return false;	// application IDs are numeric snowflakes
	}
	strcpy(s_appId, appId);
	InitializeCriticalSection(&s_lock);
	s_wake = CreateEventA(NULL, FALSE, FALSE, NULL);
	s_startTime = (__int64)time(NULL);
	s_quit = 0;
	s_thread = CreateThread(NULL, 0, Worker, NULL, 0, NULL);
	return s_thread != NULL;
}

void discord_SetActivity(const char *details, const char *state, bool resetTimer) {
	if (!s_thread) return;
	EnterCriticalSection(&s_lock);
	strncpy(s_details, details ? details : "", sizeof(s_details) - 1);
	strncpy(s_state, state ? state : "", sizeof(s_state) - 1);
	if (resetTimer) s_startTime = (__int64)time(NULL);
	s_dirty = true;
	LeaveCriticalSection(&s_lock);
	SetEvent(s_wake);
}

void discord_Stop() {
	if (!s_thread) return;
	InterlockedExchange(&s_quit, 1);
	SetEvent(s_wake);
	WaitForSingleObject(s_thread, 3000);	// a blocked pipe read must not hold up the exit
	CloseHandle(s_thread);
	s_thread = NULL;
}
