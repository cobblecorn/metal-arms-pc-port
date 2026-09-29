#include "pc_input.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static HWND s_window;
static HMODULE s_xinput;
typedef DWORD (WINAPI *GetStateFn)(DWORD, XINPUT_STATE *);
static GetStateFn s_getState;
static FPadio_InputEmulationPlatform_e s_platform;
static PcInputLayout s_layout = PCINPUT_LAYOUT_SHARED;
static bool s_localCoopSession = false;
static PcInputLayout s_localCoopLayout = PCINPUT_LAYOUT_SHARED;
static volatile LONG s_connected[FPADIO_MAX_DEVICES];	// by XInput pad index
static DWORD s_lastProbe[FPADIO_MAX_DEVICES];
static volatile LONG s_autoPortPad[FPADIO_MAX_DEVICES];	// AUTO: pad index + 1 dealt to each port, 0 for none
static volatile LONG s_autoPlayers = 2;
static volatile LONG s_autoDealt;			// AUTO: the session's first deal is done
static volatile LONG s_padAssignmentSerial;
static CRITICAL_SECTION s_padLock;			// AUTO dealing: sampling thread and session start
static bool s_padLockReady;
static const DWORD XINPUT_REPROBE_MS = 250;
static volatile LONG s_mouseLook, s_mouseDX, s_mouseDY;
static volatile LONG s_lookAllowed;		// the keyboard port is in gameplay (set by the game thread)
static volatile LONG s_lookSwitchedOff;	// F1 turned automatic mouse look off
static bool s_rawMouse;
static float s_mouseDegrees = 0.1f;
static bool s_mouseSensitivityOverride = false;
static float s_frameYaw, s_framePitch;
static PcAimAssistMode s_aimAssistMode = PCINPUT_AIM_ASSIST_AUTO;
static volatile LONG s_mouseAiming;	// the keyboard port's most recent aiming came from the mouse
static volatile LONG s_promptsForPad;	// most recently active device, for prompts without a port context
static volatile LONG s_portPromptsForPad[FPADIO_MAX_DEVICES];	// AUTO prompt source, per game port
static PcPromptStyle s_promptStyle = PCINPUT_PROMPT_STYLE_AUTO;
static bool s_promptStyleCommandLine;
static volatile LONG s_textInput;

struct PcInputActionFrame {
	float values[PCINPUT_ACTION_COUNT];
	bool held[PCINPUT_ACTION_COUNT];
	bool pressed[PCINPUT_ACTION_COUNT];
	bool released[PCINPUT_ACTION_COUNT];
};

static FPadio_Sample_t s_actionSamples[FPADIO_MAX_DEVICES];
static bool s_actionMenus[FPADIO_MAX_DEVICES];
static bool s_actionSampleReady[FPADIO_MAX_DEVICES];
static PcInputActionFrame s_actionFrames[FPADIO_MAX_DEVICES];
static CRITICAL_SECTION s_actionLock;
static bool s_actionLockReady;
// Menu pointer: the window thread counts presses and wheel motion; the game thread samples the
// position and takes the counts once per frame.
// The position comes from the window's mouse messages (client pixels, packed y << 16 | x), so it also
// follows messages posted by test tools; the real cursor is only polled to notice it leaving.
static volatile LONG s_menuWheel, s_menuPos, s_menuMoves;
static volatile LONG s_menuPointerDrawnTick;	// GetTickCount() when a menu last drew its own pointer
static bool s_menuShown, s_menuMoved;
// Presses keep their own positions, in order, so quick clicks on different items all land: the window
// thread queues them under the lock and the game thread moves them to its own queue each frame.
#define MENU_CLICK_QUEUE 8
struct MenuClickQueue { LONG pos[MENU_CLICK_QUEUE]; int count; };
static CRITICAL_SECTION s_menuLock;
static bool s_menuLockReady;
static MenuClickQueue s_menuPosted[2], s_menuClicks[2];	// [0] left, [1] right
static RECT s_menuClient;
static LONG s_menuLastMoves;
static POINT s_menuLastCursor;
static float s_menuX, s_menuY;
static int s_menuFrameWheel, s_menuWheelRemainder;

// Scripted key taps for unattended tests and screenshots (MA_PORT_TEST_KEYS / -test-keys
// "seconds:vk,..."): each key reads as held for a quarter second from that many seconds after install,
// whichever window has focus, so a test never has to take the keyboard from the desktop.
// A "g" before the seconds ("g8:0x1B") counts from the first gameplay frame instead (the first frame
// with the gameplay control map and the game not paused), so a test can pause a level at a set point
// in its play however long it took to load.
struct TestKey { DWORD at; int key; bool fromGameplay; };
static TestKey s_testKeys[32];
static int s_testKeyCount;
static DWORD s_installTick;
static volatile LONG s_gameplayTick;	// GetTickCount() of the first gameplay frame, 0 until then

static bool TestKeyHeld(int key) {
	const DWORD now = GetTickCount();
	const DWORD gameplayTick = (DWORD)InterlockedCompareExchange(&s_gameplayTick, 0, 0);
	for (int i = 0; i < s_testKeyCount; i++) {
		if (s_testKeys[i].key != key) continue;
		if (s_testKeys[i].fromGameplay && !gameplayTick) continue;
		const DWORD elapsed = now - (s_testKeys[i].fromGameplay ? gameplayTick : s_installTick);
		if (elapsed >= s_testKeys[i].at && elapsed < s_testKeys[i].at + 250) return true;
	}
	return false;
}

static void ParseTestKeys(const char *text) {
	s_testKeyCount = 0;
	while (text && *text && s_testKeyCount < (int)(sizeof(s_testKeys) / sizeof(s_testKeys[0]))) {
		char *end;
		const bool fromGameplay = (*text == 'g' || *text == 'G');
		if (fromGameplay) text++;
		const double seconds = strtod(text, &end);
		if (end == text || *end != ':') break;
		text = end + 1;
		const long key = strtol(text, &end, 0);
		if (end == text || key <= 0 || key > 255) break;
		s_testKeys[s_testKeyCount].at = (DWORD)(seconds * 1000.0);
		s_testKeys[s_testKeyCount].key = (int)key;
		s_testKeys[s_testKeyCount].fromGameplay = fromGameplay;
		s_testKeyCount++;
		text = *end == ',' ? end + 1 : end;
	}
}

#define TEXT_INPUT_QUEUE 64
struct TextInputQueue { wchar_t characters[TEXT_INPUT_QUEUE]; int count; };
static CRITICAL_SECTION s_textLock;
static bool s_textLockReady;
static TextInputQueue s_textPosted, s_textCharacters;

static float Clamp(float value, float low, float high) {
	return value < low ? low : value > high ? high : value;
}

static void Stick(float x, float y, float deadzone, float *outX, float *outY) {
	const float length = sqrtf(x * x + y * y);
	if (length <= deadzone) { *outX = *outY = 0.0f; return; }
	const float scale = (Clamp(length, 0.0f, 32767.0f) - deadzone) / (32767.0f - deadzone) / length;
	*outX = x * scale;
	*outY = y * scale;
}

static float Trigger(BYTE value) {
	return value <= XINPUT_GAMEPAD_TRIGGER_THRESHOLD ? 0.0f :
		(value - XINPUT_GAMEPAD_TRIGGER_THRESHOLD) / (255.0f - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
}

static float Stronger(float a, float b) { return fabsf(a) >= fabsf(b) ? a : b; }

void pcinput_MapSample(const PcInputState &state, bool primary,
	FPadio_InputEmulationPlatform_e platform, FPadio_Sample_t *sample) {
	memset(sample, 0, sizeof(*sample));
	// Keep the keyboard's port connected when focus is lost, but release all inputs.
	sample->bValid = primary || state.connected;
	if (!sample->bValid || !state.focused) return;
	float *v = sample->afInputValues;
	if (state.connected) {
		const XINPUT_GAMEPAD &p = state.pad;
		Stick(p.sThumbLX, p.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,
			&v[FPADIO_INPUT_STICK_LEFT_X-1], &v[FPADIO_INPUT_STICK_LEFT_Y-1]);
		Stick(p.sThumbRX, p.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,
			&v[FPADIO_INPUT_STICK_RIGHT_X-1], &v[FPADIO_INPUT_STICK_RIGHT_Y-1]);
		v[FPADIO_INPUT_TRIGGER_LEFT-1] = Trigger(p.bLeftTrigger);
		v[FPADIO_INPUT_TRIGGER_RIGHT-1] = Trigger(p.bRightTrigger);
		v[FPADIO_INPUT_START-1] = (p.wButtons & XINPUT_GAMEPAD_START) != 0;
		v[FPADIO_INPUT_CROSS_BOTTOM-1] = (p.wButtons & XINPUT_GAMEPAD_A) != 0;
		v[FPADIO_INPUT_CROSS_RIGHT-1] = (p.wButtons & XINPUT_GAMEPAD_B) != 0;
		v[FPADIO_INPUT_CROSS_LEFT-1] = (p.wButtons & XINPUT_GAMEPAD_X) != 0;
		v[FPADIO_INPUT_CROSS_TOP-1] = (p.wButtons & XINPUT_GAMEPAD_Y) != 0;
		v[FPADIO_INPUT_DPAD_X-1] = float((p.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0) - float((p.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0);
		v[FPADIO_INPUT_DPAD_Y-1] = float((p.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0) - float((p.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0);
		v[FPADIO_INPUT_XB_DBUTTON_BACK-1] = (p.wButtons & XINPUT_GAMEPAD_BACK) != 0;
		// The PC wrapper uses Xbox menu slots even while gameplay follows the GameCube map.
		// These two Xbox-only slots do not alias GameCube inputs.
		v[FPADIO_INPUT_XB_ABUTTON_BLACK-1] = (p.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
		v[FPADIO_INPUT_XB_ABUTTON_WHITE-1] = (p.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
		if (platform == FPADIO_INPUT_EMULATION_PLATFORM_GC) {
			// Input slot 15 aliases the GC digital left trigger and the Xbox Back button. PC wrapper
			// menus use the Xbox map, so keep the native Back reading in this slot instead of
			// overwriting it with left-trigger state. The analog trigger is still available in slot 5.
			v[FPADIO_INPUT_GC_DBUTTON_TRIGGER_RIGHT-1] = p.bRightTrigger > 230;
			v[FPADIO_INPUT_GC_DBUTTON_TRIGGER_Z-1] = (p.wButtons & (XINPUT_GAMEPAD_RIGHT_SHOULDER | XINPUT_GAMEPAD_RIGHT_THUMB)) != 0;
		} else {
			v[FPADIO_INPUT_XB_DBUTTON_STICK_LEFT-1] = (p.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0;
			v[FPADIO_INPUT_XB_DBUTTON_STICK_RIGHT-1] = (p.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;
		}
	}
	if (!primary) return;
	const bool *k = state.keys;
	float x = state.textInput ? 0.0f : float(k['D']) - float(k['A']);
	float y = state.textInput ? 0.0f : float(k['W']) - float(k['S']);
	if (x && y) { x *= 0.70710678f; y *= 0.70710678f; }
	v[FPADIO_INPUT_STICK_LEFT_X-1] = Stronger(v[FPADIO_INPUT_STICK_LEFT_X-1], x);
	v[FPADIO_INPUT_STICK_LEFT_Y-1] = Stronger(v[FPADIO_INPUT_STICK_LEFT_Y-1], y);
	// Merging keyboard and stick axes must not create faster diagonal movement.
	x = v[FPADIO_INPUT_STICK_LEFT_X-1]; y = v[FPADIO_INPUT_STICK_LEFT_Y-1];
	const float length = sqrtf(x*x + y*y);
	if (length > 1.0f) { v[FPADIO_INPUT_STICK_LEFT_X-1] /= length; v[FPADIO_INPUT_STICK_LEFT_Y-1] /= length; }
	x = state.textInput ? 0.0f : float(k[VK_RIGHT]) - float(k[VK_LEFT]);
	y = state.textInput ? 0.0f : float(k[VK_UP]) - float(k[VK_DOWN]);
	v[FPADIO_INPUT_STICK_RIGHT_X-1] = Stronger(v[FPADIO_INPUT_STICK_RIGHT_X-1], x);
	v[FPADIO_INPUT_STICK_RIGHT_Y-1] = Stronger(v[FPADIO_INPUT_STICK_RIGHT_Y-1], y);
	if (!state.textInput) {
		if (k[VK_SPACE]) v[FPADIO_INPUT_CROSS_BOTTOM-1] = 1.0f;
		if (k['E']) v[FPADIO_INPUT_CROSS_TOP-1] = 1.0f;
		// In gameplay, Q selects the secondary list and R selects the primary/reloads.
		// In PC menus, R is the Xbox-style left face button; Q remains reserved for pause-page navigation.
		if (k['Q'] && !state.menus) v[FPADIO_INPUT_CROSS_LEFT-1] = 1.0f;
		if (k['R']) v[(state.menus ? FPADIO_INPUT_CROSS_LEFT : FPADIO_INPUT_CROSS_RIGHT)-1] = 1.0f;
	}
	// Enter is START (menus accept it); Escape pauses in gameplay and is B/CROSS_RIGHT Back in PC menus.
	// While a text field is typed into, Enter arrives as a character (the field's Done) instead.
	if ((k[VK_RETURN] && !state.textInput) || (k[VK_ESCAPE] && !state.menus)) v[FPADIO_INPUT_START-1] = 1.0f;
	if (k[VK_ESCAPE] && state.menus) v[FPADIO_INPUT_CROSS_RIGHT-1] = 1.0f;
	if (!state.textInput && k[VK_LBUTTON]) v[FPADIO_INPUT_TRIGGER_RIGHT-1] = 1.0f;
	if (!state.textInput && k[VK_RBUTTON]) v[FPADIO_INPUT_TRIGGER_LEFT-1] = 1.0f;
	if (!state.textInput)
		if (k['F']) v[(platform == FPADIO_INPUT_EMULATION_PLATFORM_GC ? FPADIO_INPUT_GC_DBUTTON_TRIGGER_Z : FPADIO_INPUT_XB_DBUTTON_STICK_RIGHT)-1] = 1.0f;
	x = state.textInput ? 0.0f : float(k['2']) - float(k['4']);
	y = state.textInput ? 0.0f : float(k['1']) - float(k['3']);
	v[FPADIO_INPUT_DPAD_X-1] = Stronger(v[FPADIO_INPUT_DPAD_X-1], x);
	v[FPADIO_INPUT_DPAD_Y-1] = Stronger(v[FPADIO_INPUT_DPAD_Y-1], y);
	if (platform == FPADIO_INPUT_EMULATION_PLATFORM_GC && !state.textInput) {
		if (k[VK_RBUTTON]) v[FPADIO_INPUT_GC_DBUTTON_TRIGGER_LEFT-1] = 1.0f;
		if (k[VK_LBUTTON]) v[FPADIO_INPUT_GC_DBUTTON_TRIGGER_RIGHT-1] = 1.0f;
	}
}

static float SampleValue(const FPadio_Sample_t &sample, FPadio_InputID_e input) {
	if (input <= FPADIO_INPUT_NONE || input > FPADIO_MAX_INPUTS) return 0.0f;
	return sample.afInputValues[input - 1];
}

static float PositiveValue(float value) { return value > 0.0f ? value : 0.0f; }

static float ActionValueFromSample(const FPadio_Sample_t &sample, bool menus, PcInputAction action) {
	if (!sample.bValid || action < 0 || action >= PCINPUT_ACTION_COUNT) return 0.0f;
	const float leftX = SampleValue(sample, FPADIO_INPUT_STICK_LEFT_X);
	const float leftY = SampleValue(sample, FPADIO_INPUT_STICK_LEFT_Y);
	const float rightX = SampleValue(sample, FPADIO_INPUT_STICK_RIGHT_X);
	const float rightY = SampleValue(sample, FPADIO_INPUT_STICK_RIGHT_Y);
	const float dpadX = SampleValue(sample, FPADIO_INPUT_DPAD_X);
	const float dpadY = SampleValue(sample, FPADIO_INPUT_DPAD_Y);
	const float start = SampleValue(sample, FPADIO_INPUT_START);
	const float confirm = SampleValue(sample, FPADIO_INPUT_CROSS_BOTTOM);
	const float crossLeft = SampleValue(sample, FPADIO_INPUT_CROSS_LEFT);
	const float crossRight = SampleValue(sample, FPADIO_INPUT_CROSS_RIGHT);
	float navX = dpadX, navY = dpadY;
	if (menus) {
		navX = Stronger(navX, Stronger(leftX, rightX));
		navY = Stronger(navY, Stronger(leftY, rightY));
	}
	switch (action) {
	case PCINPUT_ACTION_CONFIRM:
		return menus ? Stronger(confirm, start) : confirm;
	case PCINPUT_ACTION_BACK:
		if (!menus) return 0.0f;
		return Stronger(crossRight, SampleValue(sample, FPADIO_INPUT_XB_DBUTTON_BACK));
	case PCINPUT_ACTION_PAUSE: return start;
	case PCINPUT_ACTION_NAV_UP: return PositiveValue(navY);
	case PCINPUT_ACTION_NAV_DOWN: return PositiveValue(-navY);
	case PCINPUT_ACTION_NAV_LEFT: return PositiveValue(-navX);
	case PCINPUT_ACTION_NAV_RIGHT: return PositiveValue(navX);
	case PCINPUT_ACTION_MOVE_FORWARD: return PositiveValue(leftY);
	case PCINPUT_ACTION_MOVE_BACK: return PositiveValue(-leftY);
	case PCINPUT_ACTION_MOVE_LEFT: return PositiveValue(-leftX);
	case PCINPUT_ACTION_MOVE_RIGHT: return PositiveValue(leftX);
	case PCINPUT_ACTION_LOOK_UP: return PositiveValue(rightY);
	case PCINPUT_ACTION_LOOK_DOWN: return PositiveValue(-rightY);
	case PCINPUT_ACTION_LOOK_LEFT: return PositiveValue(-rightX);
	case PCINPUT_ACTION_LOOK_RIGHT: return PositiveValue(rightX);
	case PCINPUT_ACTION_FIRE_PRIMARY: return SampleValue(sample, FPADIO_INPUT_TRIGGER_RIGHT);
	case PCINPUT_ACTION_FIRE_SECONDARY: return SampleValue(sample, FPADIO_INPUT_TRIGGER_LEFT);
	case PCINPUT_ACTION_JUMP: return confirm;
	case PCINPUT_ACTION_ACTION: return SampleValue(sample, FPADIO_INPUT_CROSS_TOP);
	case PCINPUT_ACTION_SELECT_PRIMARY: return crossRight;
	case PCINPUT_ACTION_SELECT_SECONDARY: return crossLeft;
	case PCINPUT_ACTION_QUICK_SELECT_UP: return PositiveValue(dpadY);
	case PCINPUT_ACTION_QUICK_SELECT_DOWN: return PositiveValue(-dpadY);
	case PCINPUT_ACTION_QUICK_SELECT_LEFT: return PositiveValue(-dpadX);
	case PCINPUT_ACTION_QUICK_SELECT_RIGHT: return PositiveValue(dpadX);
	case PCINPUT_ACTION_MELEE:
		return SampleValue(sample, s_platform == FPADIO_INPUT_EMULATION_PLATFORM_GC ?
			FPADIO_INPUT_GC_DBUTTON_TRIGGER_Z : FPADIO_INPUT_XB_DBUTTON_STICK_RIGHT);
	case PCINPUT_ACTION_MELEE_SECONDARY:
		return s_platform == FPADIO_INPUT_EMULATION_PLATFORM_GC ? 0.0f :
			SampleValue(sample, FPADIO_INPUT_XB_ABUTTON_BLACK);
	case PCINPUT_ACTION_UP_EUK:
		return s_platform == FPADIO_INPUT_EMULATION_PLATFORM_GC ? 0.0f :
			SampleValue(sample, FPADIO_INPUT_XB_ABUTTON_WHITE);
	default: return 0.0f;
	}
}

static void SnapshotActions() {
	if (!s_actionLockReady) return;
	const bool focused = s_window && GetForegroundWindow() == s_window && !IsIconic(s_window);
	EnterCriticalSection(&s_actionLock);
	for (u32 port = 0; port < FPADIO_MAX_DEVICES; port++) {
		PcInputActionFrame &frame = s_actionFrames[port];
		for (int action = 0; action < PCINPUT_ACTION_COUNT; action++) {
			const float value = focused && s_actionSampleReady[port] ?
				ActionValueFromSample(s_actionSamples[port], s_actionMenus[port], (PcInputAction)action) : 0.0f;
			const bool held = value >= 0.35f;
			frame.values[action] = value;
			frame.pressed[action] = held && !frame.held[action];
			frame.released[action] = !held && frame.held[action];
			frame.held[action] = held;
		}
	}
	LeaveCriticalSection(&s_actionLock);
}

static bool MouseLook() { return InterlockedCompareExchange(&s_mouseLook, 0, 0) != 0; }

// A menu is drawing its own pointer (it reports this every frame it draws).
static bool MenuDrawsPointer() {
	return GetTickCount() - (DWORD)InterlockedCompareExchange(&s_menuPointerDrawnTick, 0, 0) < 250;
}

static void ClipToGame() {
	RECT r;
	if (GetClientRect(s_window, &r) && r.right > r.left && r.bottom > r.top) {
		POINT tl = {r.left, r.top}, br = {r.right, r.bottom};
		if (ClientToScreen(s_window, &tl) && ClientToScreen(s_window, &br)) {
			RECT screen = {tl.x, tl.y, br.x, br.y};
			ClipCursor(&screen);
		}
	}
}

static void ReleaseMouse() {
	if (InterlockedExchange(&s_mouseLook, 0)) {
		ClipCursor(NULL);
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	}
	InterlockedExchange(&s_mouseDX, 0);
	InterlockedExchange(&s_mouseDY, 0);
}

static void CaptureMouse() {
	InterlockedExchange(&s_mouseDX, 0);
	InterlockedExchange(&s_mouseDY, 0);
	InterlockedExchange(&s_mouseLook, 1);
	ClipToGame();
	SetCursor(NULL);
}

// Mouse look captures itself when the game is in gameplay, owns focus, the player has not
// turned it off with F1, and the mouse moves or clicks over the client area.
static bool MayCaptureMouse() {
	if (!s_rawMouse || MouseLook() || GetForegroundWindow() != s_window || IsIconic(s_window) ||
		!InterlockedCompareExchange(&s_lookAllowed, 0, 0) || InterlockedCompareExchange(&s_lookSwitchedOff, 0, 0))
		return false;
	POINT cursor;
	RECT client;
	return GetCursorPos(&cursor) && ScreenToClient(s_window, &cursor) && GetClientRect(s_window, &client) &&
		PtInRect(&client, cursor);
}

static const char *PromptStyleName(PcPromptStyle style) {
	switch (style) {
	case PCINPUT_PROMPT_STYLE_KEYBOARD: return "keyboard";
	case PCINPUT_PROMPT_STYLE_XBOX: return "xbox";
	case PCINPUT_PROMPT_STYLE_PLAYSTATION: return "playstation";
	default: return "auto";
	}
}

static bool PromptSettingsPath(char *path, size_t capacity) {
	char root[MAX_PATH];
	DWORD length = GetEnvironmentVariableA("LOCALAPPDATA", root, sizeof(root));
	if (!length || length >= sizeof(root)) {
		length = GetEnvironmentVariableA("APPDATA", root, sizeof(root));
	}
	if (!length || length >= sizeof(root)) return false;

	char directory[MAX_PATH + 32];
	const int directoryLength = _snprintf(directory, sizeof(directory), "%s\\Metal Arms Source Port", root);
	if (directoryLength <= 0 || directoryLength >= sizeof(directory)) return false;
	CreateDirectoryA(directory, NULL);
	const int pathLength = _snprintf(path, capacity, "%s\\settings.ini", directory);
	return pathLength > 0 && pathLength < capacity;
}

static PcPromptStyle LoadPromptStyleSetting() {
	char path[MAX_PATH + 64], value[32];
	PcPromptStyle style = PCINPUT_PROMPT_STYLE_AUTO;
	if (PromptSettingsPath(path, sizeof(path))) {
		GetPrivateProfileStringA("Input", "ButtonPrompts", "auto", value, sizeof(value), path);
		pcinput_ParsePromptStyle(value, &style);
	}
	return style;
}

bool pcinput_ParseMouseSensitivity(const char *text, float *value) {
	if (!text || !*text || !value) return false;
	char *end;
	const double parsed = strtod(text, &end);
	if (end == text || *end || !(parsed >= 0.001 && parsed <= 10.0)) return false;
	*value = (float)parsed;
	return true;
}

float pcinput_MouseSensitivity() { return s_mouseDegrees; }
bool pcinput_MouseSensitivityIsOverride() { return s_mouseSensitivityOverride; }
bool pcinput_SetMouseSensitivity(float value) {
	if (s_mouseSensitivityOverride || !(value >= 0.001f && value <= 10.0f)) return false;
	s_mouseDegrees = value;
	return true;
}
bool pcinput_SaveMouseSensitivity() {
	if (s_mouseSensitivityOverride) return false;
	char path[MAX_PATH + 64], value[32];
	if (!PromptSettingsPath(path, sizeof(path))) return false;
	_snprintf(value, sizeof(value), "%.6f", s_mouseDegrees);
	return WritePrivateProfileStringA("Input", "MouseSensitivity", value, path) != FALSE;
}

static HMODULE LoadSystemXInput(const char *dll) {
	HMODULE module = LoadLibraryExA(dll, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (module) return module;

	// Older Windows versions may not support LOAD_LIBRARY_SEARCH_SYSTEM32. Build an explicit
	// system-directory path instead of falling back to the executable's working directory.
	char systemDirectory[MAX_PATH];
	const UINT length = GetSystemDirectoryA(systemDirectory, sizeof(systemDirectory));
	if (!length || length >= sizeof(systemDirectory)) return NULL;
	char path[MAX_PATH + 32];
	const int pathLength = _snprintf(path, sizeof(path), "%s\\%s", systemDirectory, dll);
	if (pathLength <= 0 || pathLength >= sizeof(path)) return NULL;
	return LoadLibraryA(path);
}

bool pcinput_Install(u32 window, FPadio_InputEmulationPlatform_e platform) {
	s_window = (HWND)window;
	s_platform = platform;
	s_mouseLook = s_mouseDX = s_mouseDY = 0;
	s_lookAllowed = s_lookSwitchedOff = 0;
	s_frameYaw = s_framePitch = 0;
	s_mouseDegrees = 0.1f;
	s_menuWheel = s_menuPos = s_menuMoves = s_menuPointerDrawnTick = 0;
	s_menuShown = s_menuMoved = false;
	s_menuLastMoves = 0;
	s_menuLastCursor.x = s_menuLastCursor.y = -1;
	memset(s_menuPosted, 0, sizeof(s_menuPosted));
	memset(s_menuClicks, 0, sizeof(s_menuClicks));
	s_menuFrameWheel = s_menuWheelRemainder = 0;
	s_textInput = 0;
	memset(&s_textPosted, 0, sizeof(s_textPosted));
	memset(&s_textCharacters, 0, sizeof(s_textCharacters));
	if (!s_menuLockReady) {
		InitializeCriticalSection(&s_menuLock);
		s_menuLockReady = true;
	}
	if (!s_textLockReady) {
		InitializeCriticalSection(&s_textLock);
		s_textLockReady = true;
	}
	if (!s_actionLockReady) {
		InitializeCriticalSection(&s_actionLock);
		s_actionLockReady = true;
	}
	if (!s_padLockReady) {
		InitializeCriticalSection(&s_padLock);
		s_padLockReady = true;
	}
	EnterCriticalSection(&s_actionLock);
	memset(s_actionSamples, 0, sizeof(s_actionSamples));
	memset(s_actionMenus, 0, sizeof(s_actionMenus));
	memset(s_actionSampleReady, 0, sizeof(s_actionSampleReady));
	memset(s_actionFrames, 0, sizeof(s_actionFrames));
	LeaveCriticalSection(&s_actionLock);
	char sensitivity[32], settingsPath[MAX_PATH + 64];
	s_mouseSensitivityOverride = false;
	if (PromptSettingsPath(settingsPath, sizeof(settingsPath))) {
		GetPrivateProfileStringA("Input", "MouseSensitivity", "0.1", sensitivity, sizeof(sensitivity), settingsPath);
		pcinput_ParseMouseSensitivity(sensitivity, &s_mouseDegrees);
	}
	DWORD length = GetEnvironmentVariableA("MA_PORT_MOUSE_SENSITIVITY", sensitivity, sizeof(sensitivity));
	if (length && length < sizeof(sensitivity)) {
		s_mouseSensitivityOverride = pcinput_ParseMouseSensitivity(sensitivity, &s_mouseDegrees);
	}
	char assist[16];
	s_aimAssistMode = PCINPUT_AIM_ASSIST_AUTO;
	s_mouseAiming = 0;
	length = GetEnvironmentVariableA("MA_PORT_AIM_ASSIST", assist, sizeof(assist));
	if (length && length < sizeof(assist)) pcinput_ParseAimAssistMode(assist, &s_aimAssistMode);
	char layout[16];
	s_layout = PCINPUT_LAYOUT_SHARED;
	length = GetEnvironmentVariableA("MA_PORT_INPUT_LAYOUT", layout, sizeof(layout));
	if (length && length < sizeof(layout)) pcinput_ParseLayout(layout, &s_layout);
	char testKeys[512];
	s_installTick = GetTickCount();
	InterlockedExchange(&s_gameplayTick, 0);
	length = GetEnvironmentVariableA("MA_PORT_TEST_KEYS", testKeys, sizeof(testKeys));
	ParseTestKeys(length && length < sizeof(testKeys) ? testKeys : NULL);
	char prompts[16];
	s_promptStyle = LoadPromptStyleSetting();
	s_promptStyleCommandLine = false;
	s_promptsForPad = 0;
	for (u32 i = 0; i < FPADIO_MAX_DEVICES; i++) InterlockedExchange(&s_portPromptsForPad[i], 0);
	length = GetEnvironmentVariableA("MA_PORT_BUTTON_PROMPTS", prompts, sizeof(prompts));
	if (length && length < sizeof(prompts)) {
		PcPromptStyle overrideStyle;
		if (pcinput_ParsePromptStyle(prompts, &overrideStyle)) {
			s_promptStyle = overrideStyle;
			s_promptStyleCommandLine = true;
		}
	}
	RAWINPUTDEVICE mouse = {0x01, 0x02, 0, s_window};
	s_rawMouse = RegisterRawInputDevices(&mouse, 1, sizeof(mouse)) != FALSE;
	const DWORD now = GetTickCount();
	for (u32 i = 0; i < FPADIO_MAX_DEVICES; i++) {
		InterlockedExchange(&s_connected[i], 0);
		s_lastProbe[i] = now - XINPUT_REPROBE_MS;
	}
	if (s_xinput) FreeLibrary(s_xinput);
	s_xinput = NULL;
	s_getState = NULL;
	const char *dlls[] = { "xinput1_4.dll", "xinput1_3.dll", "xinput1_2.dll", "xinput1_1.dll", "xinput9_1_0.dll" };
	for (u32 i = 0; i < sizeof(dlls)/sizeof(dlls[0]); i++) {
		s_xinput = LoadSystemXInput(dlls[i]);
		if (!s_xinput) continue;
		s_getState = (GetStateFn)GetProcAddress(s_xinput, "XInputGetState");
		if (s_getState) break;
		FreeLibrary(s_xinput); s_xinput = NULL;
	}
	return s_rawMouse;
}

void pcinput_Uninstall() {
	// The caller joins the polling thread first; it can no longer call into the DLL.
	ReleaseMouse();
	if (s_rawMouse) {
		RAWINPUTDEVICE mouse = {0x01, 0x02, RIDEV_REMOVE, NULL};
		RegisterRawInputDevices(&mouse, 1, sizeof(mouse));
	}
	s_rawMouse = false;
	if (s_xinput) FreeLibrary(s_xinput);
	s_xinput = NULL; s_getState = NULL; s_window = NULL;
	pcinput_SetTextInput(false);
	for (u32 i = 0; i < FPADIO_MAX_DEVICES; i++) InterlockedExchange(&s_connected[i], 0);
	if (s_actionLockReady) {
		EnterCriticalSection(&s_actionLock);
		memset(s_actionSamples, 0, sizeof(s_actionSamples));
		memset(s_actionMenus, 0, sizeof(s_actionMenus));
		memset(s_actionSampleReady, 0, sizeof(s_actionSampleReady));
		memset(s_actionFrames, 0, sizeof(s_actionFrames));
		LeaveCriticalSection(&s_actionLock);
	}
}

bool pcinput_ParseLayout(const char *text, PcInputLayout *layout) {
	if (!text) return false;
	if (!_stricmp(text, "shared")) *layout = PCINPUT_LAYOUT_SHARED;
	else if (!_stricmp(text, "separate")) *layout = PCINPUT_LAYOUT_SEPARATE;
	else return false;
	return true;
}

int pcinput_PadForPort(PcInputLayout layout, u32 port) {
	if (port >= FPADIO_MAX_DEVICES) return -1;
	if (layout == PCINPUT_LAYOUT_AUTO) return (int)InterlockedCompareExchange(&s_autoPortPad[port], 0, 0) - 1;
	if (layout == PCINPUT_LAYOUT_SEPARATE) return port == 0 ? -1 : int(port) - 1;
	return int(port);
}

// AUTO: probe every pad and deal the connected ones to ports (see PCINPUT_LAYOUT_AUTO). Runs on the
// sampling thread before port 0 is sampled, and once when a session starts so the level's first frame
// already sees every player's controller.
static void UpdateAutoPadAssignmentLocked();
static void UpdateAutoPadAssignment() {
	if (!s_padLockReady) return;
	EnterCriticalSection(&s_padLock);
	UpdateAutoPadAssignmentLocked();
	LeaveCriticalSection(&s_padLock);
}

static void UpdateAutoPadAssignmentLocked() {
	const u32 players = (u32)InterlockedCompareExchange(&s_autoPlayers, 0, 0);
	const DWORD now = GetTickCount();
	bool connected[FPADIO_MAX_DEVICES];
	u32 connectedCount = 0;
	for (u32 pad = 0; pad < FPADIO_MAX_DEVICES; pad++) {
		connected[pad] = InterlockedCompareExchange(&s_connected[pad], 0, 0) != 0;
		if (s_getState && (connected[pad] || now - s_lastProbe[pad] >= XINPUT_REPROBE_MS)) {
			XINPUT_STATE padState;
			s_lastProbe[pad] = now;
			connected[pad] = s_getState(pad, &padState) == ERROR_SUCCESS;
			InterlockedExchange(&s_connected[pad], connected[pad] ? 1 : 0);
		}
		if (connected[pad]) connectedCount++;
	}
	bool changed = false;
	int portOfPad[FPADIO_MAX_DEVICES];
	for (u32 pad = 0; pad < FPADIO_MAX_DEVICES; pad++) portOfPad[pad] = -1;
	for (u32 port = 0; port < FPADIO_MAX_DEVICES; port++) {
		const int pad = (int)InterlockedCompareExchange(&s_autoPortPad[port], 0, 0) - 1;
		if (pad < 0) continue;
		if (port >= players || !connected[pad]) {
			InterlockedExchange(&s_autoPortPad[port], 0);
			changed = true;
		} else {
			portOfPad[pad] = (int)port;
		}
	}
	const bool firstDeal = !InterlockedCompareExchange(&s_autoDealt, 0, 0);
	for (u32 pad = 0; pad < FPADIO_MAX_DEVICES; pad++) {
		if (!connected[pad] || portOfPad[pad] >= 0) continue;
		int port = -1;
		if (firstDeal && connectedCount >= players) {
			for (u32 p = 0; p < players && port < 0; p++) if (!s_autoPortPad[p]) port = (int)p;
		} else {
			for (u32 p = 1; p < players && port < 0; p++) if (!s_autoPortPad[p]) port = (int)p;
			if (port < 0 && !s_autoPortPad[0]) port = 0;
		}
		if (port < 0) break;
		InterlockedExchange(&s_autoPortPad[port], (LONG)pad + 1);
		changed = true;
	}
	InterlockedExchange(&s_autoDealt, 1);
	if (changed) InterlockedIncrement(&s_padAssignmentSerial);
}

void pcinput_SetLocalCoopPlayers(u32 players) {
	if (!s_padLockReady) return;
	EnterCriticalSection(&s_padLock);
	InterlockedExchange(&s_autoPlayers, (LONG)(players < 1 ? 1 : players > FPADIO_MAX_DEVICES ? FPADIO_MAX_DEVICES : players));
	if (s_localCoopSession && s_localCoopLayout == PCINPUT_LAYOUT_AUTO) UpdateAutoPadAssignmentLocked();
	InterlockedIncrement(&s_padAssignmentSerial);
	LeaveCriticalSection(&s_padLock);
}

u32 pcinput_ConnectedPadCount() {
	u32 count = 0;
	for (u32 pad = 0; pad < FPADIO_MAX_DEVICES; pad++) if (InterlockedCompareExchange(&s_connected[pad], 0, 0)) count++;
	return count;
}

bool pcinput_XInputPadConnected(u32 pad) {
	return pad < FPADIO_MAX_DEVICES && InterlockedCompareExchange(&s_connected[pad], 0, 0) != 0;
}

u32 pcinput_PadAssignmentSerial() { return (u32)InterlockedCompareExchange(&s_padAssignmentSerial, 0, 0); }

bool pcinput_XInputConnected(u32 port) {
	const int pad = pcinput_PadForPort(pcinput_Layout(), port);
	return pad >= 0 && InterlockedCompareExchange(&s_connected[pad], 0, 0) != 0;
}

u32 pcinput_KeyboardPort() { return 0; }

PcInputLayout pcinput_Layout() { return s_localCoopSession ? s_localCoopLayout : s_layout; }
void pcinput_SetLocalCoopSession(bool active, PcInputLayout layout, u32 players) {
	if (active && layout == PCINPUT_LAYOUT_AUTO && s_padLockReady) {
		EnterCriticalSection(&s_padLock);
		for (u32 port = 0; port < FPADIO_MAX_DEVICES; port++) InterlockedExchange(&s_autoPortPad[port], 0);
		InterlockedExchange(&s_autoPlayers, (LONG)(players < 1 ? 1 : players > FPADIO_MAX_DEVICES ? FPADIO_MAX_DEVICES : players));
		InterlockedExchange(&s_autoDealt, 0);
		for (u32 pad = 0; pad < FPADIO_MAX_DEVICES; pad++) s_lastProbe[pad] = GetTickCount() - XINPUT_REPROBE_MS;
		UpdateAutoPadAssignmentLocked();
		InterlockedIncrement(&s_padAssignmentSerial);
		s_localCoopLayout = layout;
		s_localCoopSession = active;
		LeaveCriticalSection(&s_padLock);
		return;
	}
	s_localCoopLayout = layout;
	s_localCoopSession = active;
}

bool pcinput_ParsePromptStyle(const char *text, PcPromptStyle *style) {
	if (!text) return false;
	if (!_stricmp(text, "auto")) *style = PCINPUT_PROMPT_STYLE_AUTO;
	else if (!_stricmp(text, "keyboard")) *style = PCINPUT_PROMPT_STYLE_KEYBOARD;
	else if (!_stricmp(text, "xbox")) *style = PCINPUT_PROMPT_STYLE_XBOX;
	else if (!_stricmp(text, "playstation") || !_stricmp(text, "ps")) *style = PCINPUT_PROMPT_STYLE_PLAYSTATION;
	else return false;
	return true;
}

PcPromptStyle pcinput_PromptStyleSetting() { return s_promptStyle; }

bool pcinput_PromptStyleIsCommandLineOverride() { return s_promptStyleCommandLine; }

bool pcinput_SetPromptStyleSetting(PcPromptStyle style) {
	if (s_promptStyleCommandLine || style < PCINPUT_PROMPT_STYLE_AUTO || style > PCINPUT_PROMPT_STYLE_PLAYSTATION) {
		return false;
	}
	s_promptStyle = style;
	return true;
}

bool pcinput_SavePromptStyleSetting() {
	if (s_promptStyleCommandLine) return false;
	char path[MAX_PATH + 64];
	if (!PromptSettingsPath(path, sizeof(path))) return false;
	return WritePrivateProfileStringA("Input", "ButtonPrompts", PromptStyleName(s_promptStyle), path) != FALSE;
}

PcPromptStyle pcinput_ResolvedPromptStyle() {
	if (s_promptStyle != PCINPUT_PROMPT_STYLE_AUTO) return s_promptStyle;
	return InterlockedCompareExchange(&s_promptsForPad, 0, 0) ? PCINPUT_PROMPT_STYLE_XBOX : PCINPUT_PROMPT_STYLE_KEYBOARD;
}

PcPromptStyle pcinput_PromptStyleForPort(u32 port) {
	// Only the keyboard's port can show keys: a controller-only player (local co-op, pads on ports
	// 2-4) sees controller glyphs, Xbox unless PlayStation is chosen, whatever the setting says.
	if (port < FPADIO_MAX_DEVICES && port != pcinput_KeyboardPort())
		return s_promptStyle == PCINPUT_PROMPT_STYLE_PLAYSTATION ? PCINPUT_PROMPT_STYLE_PLAYSTATION : PCINPUT_PROMPT_STYLE_XBOX;
	if (s_promptStyle != PCINPUT_PROMPT_STYLE_AUTO) return s_promptStyle;
	if (port >= FPADIO_MAX_DEVICES) return pcinput_ResolvedPromptStyle();
	// On a shared port, follow whichever device was used most recently. In a separate layout, the
	// keyboard port stays on keys while each pad-only port uses Xbox glyphs. XInput cannot identify
	// Sony hardware, so PlayStation pads wrapped as XInput use the explicit PlayStation override.
	return InterlockedCompareExchange(&s_portPromptsForPad[port], 0, 0) ?
		PCINPUT_PROMPT_STYLE_XBOX : (port == pcinput_KeyboardPort() ?
		PCINPUT_PROMPT_STYLE_KEYBOARD : PCINPUT_PROMPT_STYLE_XBOX);
}

bool pcinput_KeyHeld(int key) {
	if (s_testKeyCount && TestKeyHeld(key)) return true;
	if (!s_window || GetForegroundWindow() != s_window || IsIconic(s_window)) return false;
	return (GetAsyncKeyState(key) & 0x8000) != 0;
}

bool pcinput_UseKeyboardPrompts() { return pcinput_ResolvedPromptStyle() == PCINPUT_PROMPT_STYLE_KEYBOARD; }

bool pcinput_UsePlayStationPrompts() { return pcinput_ResolvedPromptStyle() == PCINPUT_PROMPT_STYLE_PLAYSTATION; }

bool pcinput_UseKeyboardPromptsForPort(u32 port) {
	return pcinput_PromptStyleForPort(port) == PCINPUT_PROMPT_STYLE_KEYBOARD;
}

bool pcinput_UsePlayStationPromptsForPort(u32 port) {
	return pcinput_PromptStyleForPort(port) == PCINPUT_PROMPT_STYLE_PLAYSTATION;
}

void pcinput_GetDeviceInfo(u32 index, FPadio_DeviceInfo_t *info) {
	memset(info, 0, sizeof(*info));
	const int pad = pcinput_PadForPort(pcinput_Layout(), index);
	const bool keyboard = index == pcinput_KeyboardPort();
	if (index >= FPADIO_MAX_DEVICES || (!keyboard && (pad < 0 || !pcinput_XInputConnected(index)))) return;
	if (keyboard && pad >= 0 && pcinput_XInputConnected(index)) sprintf(info->szName, "Keyboard/mouse + XInput controller %d", pad + 1);
	else if (keyboard) sprintf(info->szName, "Keyboard/mouse");
	else sprintf(info->szName, "XInput controller %d", pad + 1);
	info->oeID = FPADIO_INPUT_DX_GAMEPAD;
	info->uInputs = FPADIO_MAX_INPUTS;
	for (u32 i = 0; i < FPADIO_MAX_INPUTS; i++) info->aeInputIDs[i] = (FPadio_InputID_e)(i + 1);
}

void pcinput_Sample(u32 index, FPadio_Sample_t *sample) {
	PcInputState state = {};
	if (index >= FPADIO_MAX_DEVICES) { memset(sample, 0, sizeof(*sample)); return; }
	if (index == 0 && pcinput_Layout() == PCINPUT_LAYOUT_AUTO) UpdateAutoPadAssignment();
	const int pad = pcinput_PadForPort(pcinput_Layout(), index);
	const bool keyboard = index == pcinput_KeyboardPort();
	const DWORD now = GetTickCount();
	if (pad >= 0 && s_getState &&
		(InterlockedCompareExchange(&s_connected[pad], 0, 0) || now - s_lastProbe[pad] >= XINPUT_REPROBE_MS)) {
		XINPUT_STATE padState = {};
		s_lastProbe[pad] = now;
		const bool connected = s_getState(pad, &padState) == ERROR_SUCCESS;
		InterlockedExchange(&s_connected[pad], connected ? 1 : 0);
		if (connected) state.pad = padState.Gamepad;
	}
	state.connected = pcinput_XInputConnected(index);
	// Aiming with the right stick hands target assistance back to the controller.
	if (keyboard && state.connected &&
		(abs(state.pad.sThumbRX) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || abs(state.pad.sThumbRY) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE))
		InterlockedExchange(&s_mouseAiming, 0);
	state.focused = s_window && GetForegroundWindow() == s_window && !IsIconic(s_window);
	state.menus = !InterlockedCompareExchange(&s_lookAllowed, 0, 0);
	state.textInput = keyboard && InterlockedCompareExchange(&s_textInput, 0, 0) != 0;
	if (state.connected && state.focused) {
		const XINPUT_GAMEPAD &p = state.pad;
		if (p.wButtons || p.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD || p.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD ||
			abs(p.sThumbLX) > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || abs(p.sThumbLY) > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
			abs(p.sThumbRX) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || abs(p.sThumbRY) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
			InterlockedExchange(&s_promptsForPad, 1);
			InterlockedExchange(&s_portPromptsForPad[index], 1);
		}
	}
	if (keyboard) {
		// Only inspect gameplay keys, and only while the game owns focus (or a test scripts them).
		const int keys[] = { 'W','A','S','D','E','Q','R','F','1','2','3','4', VK_SPACE,
			VK_UP,VK_DOWN,VK_LEFT,VK_RIGHT,VK_RETURN,VK_ESCAPE,VK_LBUTTON,VK_RBUTTON };
		if (state.focused) {
			for (u32 i = 0; i < sizeof(keys)/sizeof(keys[0]); i++) {
				state.keys[keys[i]] = (GetAsyncKeyState(keys[i]) & 0x8000) != 0;
				if (state.keys[keys[i]]) {
					InterlockedExchange(&s_promptsForPad, 0);
					InterlockedExchange(&s_portPromptsForPad[index], 0);
				}
			}
		}
		if (s_testKeyCount) {
			bool scriptedKeyHeld = false;
			for (u32 i = 0; i < sizeof(keys)/sizeof(keys[0]); i++) {
				if (TestKeyHeld(keys[i])) {
					state.keys[keys[i]] = true;
					scriptedKeyHeld = true;
				}
			}
			if (scriptedKeyHeld && !state.focused) {
				// Only injected keys bypass focus. A connected physical pad must remain neutral.
				memset(&state.pad, 0, sizeof(state.pad));
				state.focused = true;
			}
		}
		// A menu with its own pointer takes the buttons as clicks, not as the triggers (on the launch
		// screen a held right trigger starts the level-unlock code and blocks other input).
		if (MenuDrawsPointer()) state.keys[VK_LBUTTON] = state.keys[VK_RBUTTON] = false;
		// Escape is START in gameplay and Back in menus. A press that began before the switch must not
		// count as the other button: pausing with it would at once back out of the pause menu, and
		// backing out would pause again.
		static bool lastMenus, escapeHeldOver;
		if (state.menus != lastMenus && state.keys[VK_ESCAPE]) escapeHeldOver = true;
		if (!state.keys[VK_ESCAPE]) escapeHeldOver = false;
		lastMenus = state.menus;
		if (escapeHeldOver) state.keys[VK_ESCAPE] = false;
	}
	pcinput_MapSample(state, keyboard, s_platform, sample);
	if (s_actionLockReady) {
		EnterCriticalSection(&s_actionLock);
		s_actionSamples[index] = *sample;
		s_actionMenus[index] = state.menus;
		s_actionSampleReady[index] = true;
		LeaveCriticalSection(&s_actionLock);
	}
}

bool pcinput_WindowMessage(UINT message, WPARAM wParam, LPARAM lParam) {
	if (!s_window) return false;
	if (message == WM_CHAR && InterlockedCompareExchange(&s_textInput, 0, 0) && s_textLockReady) {
		const wchar_t character = (wchar_t)wParam;
		if (character == L'\b' || character == L'\r' || (character >= L' ' && character <= L'~')) {
			InterlockedExchange(&s_promptsForPad, 0);
			InterlockedExchange(&s_portPromptsForPad[pcinput_KeyboardPort()], 0);
			EnterCriticalSection(&s_textLock);
			if (s_textPosted.count < TEXT_INPUT_QUEUE) s_textPosted.characters[s_textPosted.count++] = character;
			LeaveCriticalSection(&s_textLock);
		}
		return true;
	}
	if (message == WM_KILLFOCUS || (message == WM_ACTIVATEAPP && !wParam) || message == WM_DESTROY) ReleaseMouse();
	if (message == WM_KEYDOWN && !(lParam & (1L << 30))) {
		if (GetForegroundWindow() == s_window) {
			InterlockedExchange(&s_promptsForPad, 0);
			InterlockedExchange(&s_portPromptsForPad[pcinput_KeyboardPort()], 0);
		}
		if (wParam == VK_ESCAPE) ReleaseMouse();
		// F1 switches automatic mouse look off (freeing the cursor) and back on.
		if (wParam == VK_F1 && s_rawMouse && GetForegroundWindow() == s_window) {
			if (InterlockedCompareExchange(&s_lookSwitchedOff, 0, 0)) {
				InterlockedExchange(&s_lookSwitchedOff, 0);
				if (MayCaptureMouse()) CaptureMouse();
			} else {
				InterlockedExchange(&s_lookSwitchedOff, 1);
				ReleaseMouse();
			}
		}
	}
	if (message == WM_SETCURSOR && LOWORD(lParam) == HTCLIENT && (MouseLook() || MenuDrawsPointer())) {
		SetCursor(NULL); return true;
	}
	if (!MouseLook()) {
		if (message == WM_MOUSEMOVE || message == WM_LBUTTONDOWN || message == WM_LBUTTONDBLCLK ||
			message == WM_RBUTTONDOWN || message == WM_RBUTTONDBLCLK) {
			// Windows also sends WM_MOUSEMOVE without motion (window changes); only a new position counts.
			const LONG pos = (LONG)(((DWORD)(WORD)HIWORD(lParam) << 16) | (WORD)LOWORD(lParam));
			if (InterlockedExchange(&s_menuPos, pos) != pos) InterlockedIncrement(&s_menuMoves);
		}
		const int button = (message == WM_LBUTTONDOWN || message == WM_LBUTTONDBLCLK) ? 0 :
			(message == WM_RBUTTONDOWN || message == WM_RBUTTONDBLCLK) ? 1 : -1;
		if (button >= 0 && s_menuLockReady) {
			EnterCriticalSection(&s_menuLock);
			MenuClickQueue &queue = s_menuPosted[button];
			if (queue.count < MENU_CLICK_QUEUE) queue.pos[queue.count++] = (LONG)lParam;
			LeaveCriticalSection(&s_menuLock);
		}
		if (message == WM_MOUSEWHEEL) InterlockedExchangeAdd(&s_menuWheel, (short)HIWORD(wParam));
	}
	if ((message == WM_MOVE || message == WM_SIZE) && MouseLook()) ClipToGame();
	if (message == WM_MOUSEMOVE || message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN || message == WM_MOUSEWHEEL) {
		InterlockedExchange(&s_promptsForPad, 0);
		InterlockedExchange(&s_portPromptsForPad[pcinput_KeyboardPort()], 0);
	}
	if (message == WM_INPUT && GetForegroundWindow() == s_window) {
		RAWINPUT data;
		UINT size = sizeof(data);
		const UINT read = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &data, &size, sizeof(RAWINPUTHEADER));
		if (read != (UINT)-1 && read >= sizeof(RAWINPUTHEADER) + sizeof(RAWMOUSE) &&
			data.header.dwType == RIM_TYPEMOUSE && !(data.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)) {
			const RAWMOUSE &mouse = data.data.mouse;
			if (mouse.lLastX || mouse.lLastY || mouse.usButtonFlags) {
				InterlockedExchange(&s_promptsForPad, 0);
				InterlockedExchange(&s_portPromptsForPad[pcinput_KeyboardPort()], 0);
			}
			if (MouseLook()) {
				InterlockedExchangeAdd(&s_mouseDX, mouse.lLastX);
				InterlockedExchangeAdd(&s_mouseDY, mouse.lLastY);
			} else if ((mouse.lLastX || mouse.lLastY || (mouse.usButtonFlags & (RI_MOUSE_LEFT_BUTTON_DOWN | RI_MOUSE_RIGHT_BUTTON_DOWN))) &&
				MayCaptureMouse()) {
				// The motion that brings the mouse in is not applied, so the view does not jump.
				CaptureMouse();
			}
		}
	}
	// WM_INPUT must still reach DefWindowProc for the foreground packet cleanup.
	return false;
}

static void MenuPointerFrame(bool allowLook) {
	MenuClickQueue posted[2] = {};
	if (s_menuLockReady) {
		EnterCriticalSection(&s_menuLock);
		memcpy(posted, s_menuPosted, sizeof(posted));
		memset(s_menuPosted, 0, sizeof(s_menuPosted));
		LeaveCriticalSection(&s_menuLock);
	}
	const LONG wheel = InterlockedExchange(&s_menuWheel, 0);
	const LONG moves = InterlockedCompareExchange(&s_menuMoves, 0, 0);
	const LONG pos = InterlockedCompareExchange(&s_menuPos, 0, 0);
	const bool movedSinceLast = moves != s_menuLastMoves;
	s_menuLastMoves = moves;
	s_menuMoved = false;
	s_menuFrameWheel = 0;
	RECT client;
	if (allowLook || MouseLook() || !s_window || GetForegroundWindow() != s_window || IsIconic(s_window) || !GetClientRect(s_window, &client) ||
		client.right <= 0 || client.bottom <= 0) {
		s_menuShown = false;
		s_menuWheelRemainder = 0;
		memset(s_menuClicks, 0, sizeof(s_menuClicks));
		return;
	}
	s_menuClient = client;
	// Presses wait in order until the menu takes them (one per frame).
	for (int b = 0; b < 2; b++)
		for (int i = 0; i < posted[b].count && s_menuClicks[b].count < MENU_CLICK_QUEUE; i++)
			s_menuClicks[b].pos[s_menuClicks[b].count++] = posted[b].pos[i];
	// The real cursor moving outside the client area hides the pointer.
	POINT cursor;
	if (GetCursorPos(&cursor) && (cursor.x != s_menuLastCursor.x || cursor.y != s_menuLastCursor.y)) {
		s_menuLastCursor = cursor;
		if (ScreenToClient(s_window, &cursor) && !PtInRect(&client, cursor)) s_menuShown = false;
	}
	const POINT at = { (short)LOWORD(pos), (short)HIWORD(pos) };
	if (!PtInRect(&client, at)) {
		s_menuShown = false;
		s_menuWheelRemainder = 0;
		return;
	}
	s_menuX = (at.x + 0.5f) / client.right;
	s_menuY = (at.y + 0.5f) / client.bottom;
	s_menuMoved = movedSinceLast || posted[0].count || posted[1].count;
	if (s_menuMoved) s_menuShown = true;
	s_menuWheelRemainder += wheel;
	s_menuFrameWheel = s_menuWheelRemainder / WHEEL_DELTA;
	s_menuWheelRemainder -= s_menuFrameWheel * WHEEL_DELTA;
}

bool pcinput_MenuPointer(float *x, float *y) {
	if (!s_menuShown) return false;
	*x = s_menuX; *y = s_menuY;
	return true;
}

bool pcinput_MenuPointerMoved() { return s_menuMoved; }

bool pcinput_TakeMenuClick(bool right, float *x, float *y) {
	MenuClickQueue &queue = s_menuClicks[right ? 1 : 0];
	if (!queue.count) return false;
	const LONG pos = queue.pos[0];
	queue.count--;
	memmove(queue.pos, queue.pos + 1, queue.count * sizeof(queue.pos[0]));
	if (x) *x = ((short)LOWORD(pos) + 0.5f) / s_menuClient.right;
	if (y) *y = ((short)HIWORD(pos) + 0.5f) / s_menuClient.bottom;
	return true;
}

int pcinput_TakeMenuWheel() {
	const int notches = s_menuFrameWheel;
	s_menuFrameWheel = 0;
	return notches;
}

void pcinput_HideMenuPointer() { s_menuShown = false; }

void pcinput_DrawsMenuPointer() { InterlockedExchange(&s_menuPointerDrawnTick, (LONG)GetTickCount()); }

void pcinput_BeginFrame(bool allowLook) {
	const LONG dx = InterlockedExchange(&s_mouseDX, 0), dy = InterlockedExchange(&s_mouseDY, 0);
	s_frameYaw = s_framePitch = 0;
	InterlockedExchange(&s_lookAllowed, allowLook ? 1 : 0);
	if (s_actionLockReady) {
		EnterCriticalSection(&s_actionLock);
		for (u32 port = 0; port < FPADIO_MAX_DEVICES; port++) s_actionMenus[port] = !allowLook;
		LeaveCriticalSection(&s_actionLock);
	}
	SnapshotActions();
	if (allowLook && !InterlockedCompareExchange(&s_gameplayTick, 0, 0)) InterlockedExchange(&s_gameplayTick, (LONG)(GetTickCount() | 1));
	// Menus, and losing focus by any route the window messages missed, free the cursor.
	if (!allowLook || GetForegroundWindow() != s_window) ReleaseMouse();
	if (allowLook && MouseLook() && GetForegroundWindow() == s_window) {
		// Distance, not stick deflection: no turn-speed cap, acceleration curve or dt scaling.
		const float radiansPerCount = s_mouseDegrees * (3.14159265358979323846f / 180.0f);
		s_frameYaw = dx * radiansPerCount;
		s_framePitch = dy * radiansPerCount;
		if (dx || dy) InterlockedExchange(&s_mouseAiming, 1);
	}
	MenuPointerFrame(allowLook);
}

bool pcinput_ParseAimAssistMode(const char *text, PcAimAssistMode *mode) {
	if (!text) return false;
	if (!_stricmp(text, "auto")) *mode = PCINPUT_AIM_ASSIST_AUTO;
	else if (!_stricmp(text, "on")) *mode = PCINPUT_AIM_ASSIST_ON;
	else if (!_stricmp(text, "off")) *mode = PCINPUT_AIM_ASSIST_OFF;
	else return false;
	return true;
}

bool pcinput_PromptsForPad() { return !pcinput_UseKeyboardPrompts(); }

float pcinput_ActionValue(u32 port, PcInputAction action) {
	if (port >= FPADIO_MAX_DEVICES || action < 0 || action >= PCINPUT_ACTION_COUNT || !s_actionLockReady) return 0.0f;
	EnterCriticalSection(&s_actionLock);
	const float value = s_actionFrames[port].values[action];
	LeaveCriticalSection(&s_actionLock);
	return value;
}

bool pcinput_Action(u32 port, PcInputAction action, PcInputActionPhase phase) {
	if (port >= FPADIO_MAX_DEVICES || action < 0 || action >= PCINPUT_ACTION_COUNT || !s_actionLockReady) return false;
	EnterCriticalSection(&s_actionLock);
	bool result = false;
	if (phase == PCINPUT_ACTION_HELD) result = s_actionFrames[port].held[action];
	else if (phase == PCINPUT_ACTION_PRESSED) result = s_actionFrames[port].pressed[action];
	else if (phase == PCINPUT_ACTION_RELEASED) result = s_actionFrames[port].released[action];
	LeaveCriticalSection(&s_actionLock);
	return result;
}

void pcinput_SetTextInput(bool active) {
	const LONG wanted = active ? 1 : 0;
	if (InterlockedExchange(&s_textInput, wanted) == wanted) return;
	if (s_textLockReady) {
		EnterCriticalSection(&s_textLock);
		s_textPosted.count = s_textCharacters.count = 0;
		LeaveCriticalSection(&s_textLock);
	}
}

bool pcinput_TakeTextInput(wchar_t *character) {
	if (!character || !s_textLockReady) return false;
	if (!s_textCharacters.count) {
		EnterCriticalSection(&s_textLock);
		memcpy(&s_textCharacters, &s_textPosted, sizeof(s_textCharacters));
		s_textPosted.count = 0;
		LeaveCriticalSection(&s_textLock);
	}
	if (!s_textCharacters.count) return false;
	*character = s_textCharacters.characters[0];
	s_textCharacters.count--;
	memmove(s_textCharacters.characters, s_textCharacters.characters + 1,
		s_textCharacters.count * sizeof(s_textCharacters.characters[0]));
	return true;
}

bool pcinput_IsTextInput() { return InterlockedCompareExchange(&s_textInput, 0, 0) != 0; }

bool pcinput_IsMouseAiming(u32 controller) {
	return controller == pcinput_KeyboardPort() && MouseLook() && InterlockedCompareExchange(&s_mouseAiming, 0, 0);
}

bool pcinput_AimAssistAllowed(u32 controller) {
	if (s_aimAssistMode == PCINPUT_AIM_ASSIST_ON) return true;
	if (s_aimAssistMode == PCINPUT_AIM_ASSIST_OFF) return false;
	// Only the keyboard port receives the mouse; captured mouse look that aimed last disables assistance.
	return !(controller == pcinput_KeyboardPort() && MouseLook() && InterlockedCompareExchange(&s_mouseAiming, 0, 0));
}

float pcinput_TakeMouseAxis(u32 controller, bool pitch) {
	if (controller != pcinput_KeyboardPort()) return 0;
	float &axis = pitch ? s_framePitch : s_frameYaw;
	const float delta = axis;
	axis = 0; // Repeated bot work/substeps cannot apply the same mouse delta twice.
	return MouseLook() && GetForegroundWindow() == s_window ? delta : 0.0f;
}
