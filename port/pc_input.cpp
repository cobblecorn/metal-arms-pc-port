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
static bool s_connected[FPADIO_MAX_DEVICES];		// by XInput pad index
static DWORD s_lastProbe[FPADIO_MAX_DEVICES];
static volatile LONG s_mouseLook, s_mouseDX, s_mouseDY;
static volatile LONG s_lookAllowed;		// the keyboard port is in gameplay (set by the game thread)
static volatile LONG s_lookSwitchedOff;	// F1 turned automatic mouse look off
static bool s_rawMouse;
static float s_mouseDegrees = 0.1f;
static float s_frameYaw, s_framePitch;
static PcAimAssistMode s_aimAssistMode = PCINPUT_AIM_ASSIST_AUTO;
static volatile LONG s_mouseAiming;	// the keyboard port's most recent aiming came from the mouse
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
		if (platform == FPADIO_INPUT_EMULATION_PLATFORM_GC) {
			v[FPADIO_INPUT_GC_DBUTTON_TRIGGER_LEFT-1] = p.bLeftTrigger > 230;
			v[FPADIO_INPUT_GC_DBUTTON_TRIGGER_RIGHT-1] = p.bRightTrigger > 230;
			v[FPADIO_INPUT_GC_DBUTTON_TRIGGER_Z-1] = (p.wButtons & (XINPUT_GAMEPAD_RIGHT_SHOULDER | XINPUT_GAMEPAD_RIGHT_THUMB)) != 0;
		} else {
			v[FPADIO_INPUT_XB_DBUTTON_BACK-1] = (p.wButtons & XINPUT_GAMEPAD_BACK) != 0;
			v[FPADIO_INPUT_XB_DBUTTON_STICK_LEFT-1] = (p.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0;
			v[FPADIO_INPUT_XB_DBUTTON_STICK_RIGHT-1] = (p.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;
			v[FPADIO_INPUT_XB_ABUTTON_BLACK-1] = (p.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
			v[FPADIO_INPUT_XB_ABUTTON_WHITE-1] = (p.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
		}
	}
	if (!primary) return;
	const bool *k = state.keys;
	float x = float(k['D']) - float(k['A']), y = float(k['W']) - float(k['S']);
	if (x && y) { x *= 0.70710678f; y *= 0.70710678f; }
	v[FPADIO_INPUT_STICK_LEFT_X-1] = Stronger(v[FPADIO_INPUT_STICK_LEFT_X-1], x);
	v[FPADIO_INPUT_STICK_LEFT_Y-1] = Stronger(v[FPADIO_INPUT_STICK_LEFT_Y-1], y);
	// Merging keyboard and stick axes must not create faster diagonal movement.
	x = v[FPADIO_INPUT_STICK_LEFT_X-1]; y = v[FPADIO_INPUT_STICK_LEFT_Y-1];
	const float length = sqrtf(x*x + y*y);
	if (length > 1.0f) { v[FPADIO_INPUT_STICK_LEFT_X-1] /= length; v[FPADIO_INPUT_STICK_LEFT_Y-1] /= length; }
	x = float(k[VK_RIGHT]) - float(k[VK_LEFT]);
	y = float(k[VK_UP]) - float(k[VK_DOWN]);
	v[FPADIO_INPUT_STICK_RIGHT_X-1] = Stronger(v[FPADIO_INPUT_STICK_RIGHT_X-1], x);
	v[FPADIO_INPUT_STICK_RIGHT_Y-1] = Stronger(v[FPADIO_INPUT_STICK_RIGHT_Y-1], y);
	if (k[VK_SPACE]) v[FPADIO_INPUT_CROSS_BOTTOM-1] = 1.0f;
	if (k['E']) v[FPADIO_INPUT_CROSS_TOP-1] = 1.0f;
	// GAMEPAD_MAP_MAIN1: CROSS_LEFT selects the secondary (throwables) list, CROSS_RIGHT the primary
	// (guns); a tap of the primary button reloads. Q = throwables, R = guns/reload (user choice).
	if (k['Q']) v[FPADIO_INPUT_CROSS_LEFT-1] = 1.0f;
	if (k['R']) v[FPADIO_INPUT_CROSS_RIGHT-1] = 1.0f;
	if (k[VK_ESCAPE] || k[VK_RETURN]) v[FPADIO_INPUT_START-1] = 1.0f;
	if (k[VK_LBUTTON]) v[FPADIO_INPUT_TRIGGER_RIGHT-1] = 1.0f;
	if (k[VK_RBUTTON]) v[FPADIO_INPUT_TRIGGER_LEFT-1] = 1.0f;
	if (k['F']) v[(platform == FPADIO_INPUT_EMULATION_PLATFORM_GC ? FPADIO_INPUT_GC_DBUTTON_TRIGGER_Z : FPADIO_INPUT_XB_DBUTTON_STICK_RIGHT)-1] = 1.0f;
	x = float(k['2']) - float(k['4']); y = float(k['1']) - float(k['3']);
	v[FPADIO_INPUT_DPAD_X-1] = Stronger(v[FPADIO_INPUT_DPAD_X-1], x);
	v[FPADIO_INPUT_DPAD_Y-1] = Stronger(v[FPADIO_INPUT_DPAD_Y-1], y);
	if (platform == FPADIO_INPUT_EMULATION_PLATFORM_GC) {
		if (k[VK_RBUTTON]) v[FPADIO_INPUT_GC_DBUTTON_TRIGGER_LEFT-1] = 1.0f;
		if (k[VK_LBUTTON]) v[FPADIO_INPUT_GC_DBUTTON_TRIGGER_RIGHT-1] = 1.0f;
	}
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
	if (!s_menuLockReady) {
		InitializeCriticalSection(&s_menuLock);
		s_menuLockReady = true;
	}
	char sensitivity[32];
	DWORD length = GetEnvironmentVariableA("MA_PORT_MOUSE_SENSITIVITY", sensitivity, sizeof(sensitivity));
	if (length && length < sizeof(sensitivity)) {
		char *end;
		const double value = strtod(sensitivity, &end);
		if (*end == 0 && value >= 0.001 && value <= 10.0) s_mouseDegrees = (float)value;
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
	RAWINPUTDEVICE mouse = {0x01, 0x02, 0, s_window};
	s_rawMouse = RegisterRawInputDevices(&mouse, 1, sizeof(mouse)) != FALSE;
	memset(s_connected, 0, sizeof(s_connected));
	for (u32 i = 0; i < FPADIO_MAX_DEVICES; i++) s_lastProbe[i] = GetTickCount() - 2000;
	const char *dlls[] = { "xinput1_4.dll", "xinput9_1_0.dll" };
	for (u32 i = 0; i < sizeof(dlls)/sizeof(dlls[0]); i++) {
		s_xinput = LoadLibraryExA(dlls[i], NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
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
	memset(s_connected, 0, sizeof(s_connected));
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
	if (layout == PCINPUT_LAYOUT_SEPARATE) return port == 0 ? -1 : int(port) - 1;
	return int(port);
}

u32 pcinput_KeyboardPort() { return 0; }

PcInputLayout pcinput_Layout() { return s_layout; }

void pcinput_GetDeviceInfo(u32 index, FPadio_DeviceInfo_t *info) {
	memset(info, 0, sizeof(*info));
	const int pad = pcinput_PadForPort(s_layout, index);
	const bool keyboard = index == pcinput_KeyboardPort();
	if (index >= FPADIO_MAX_DEVICES || (!keyboard && (pad < 0 || !s_connected[pad]))) return;
	if (keyboard && pad >= 0) sprintf(info->szName, "Keyboard/mouse + XInput controller %d", pad + 1);
	else if (keyboard) sprintf(info->szName, "Keyboard/mouse");
	else sprintf(info->szName, "XInput controller %d", pad + 1);
	info->oeID = FPADIO_INPUT_DX_GAMEPAD;
	info->uInputs = FPADIO_MAX_INPUTS;
	for (u32 i = 0; i < FPADIO_MAX_INPUTS; i++) info->aeInputIDs[i] = (FPadio_InputID_e)(i + 1);
}

void pcinput_Sample(u32 index, FPadio_Sample_t *sample) {
	PcInputState state = {};
	if (index >= FPADIO_MAX_DEVICES) { memset(sample, 0, sizeof(*sample)); return; }
	const int pad = pcinput_PadForPort(s_layout, index);
	const bool keyboard = index == pcinput_KeyboardPort();
	const DWORD now = GetTickCount();
	if (pad >= 0 && s_getState && (s_connected[pad] || now - s_lastProbe[pad] >= 2000)) {
		XINPUT_STATE padState = {};
		s_lastProbe[pad] = now;
		s_connected[pad] = s_getState(pad, &padState) == ERROR_SUCCESS;
		if (s_connected[pad]) state.pad = padState.Gamepad;
	}
	state.connected = pad >= 0 && s_connected[pad];
	// Aiming with the right stick hands target assistance back to the controller.
	if (keyboard && state.connected &&
		(abs(state.pad.sThumbRX) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || abs(state.pad.sThumbRY) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE))
		InterlockedExchange(&s_mouseAiming, 0);
	state.focused = s_window && GetForegroundWindow() == s_window && !IsIconic(s_window);
	if (keyboard) {
		if (state.focused) {
			// Only inspect gameplay keys, and only while the game owns focus.
			const int keys[] = { 'W','A','S','D','E','Q','R','F','1','2','3','4', VK_SPACE,
				VK_UP,VK_DOWN,VK_LEFT,VK_RIGHT,VK_RETURN,VK_ESCAPE,VK_LBUTTON,VK_RBUTTON };
			for (u32 i = 0; i < sizeof(keys)/sizeof(keys[0]); i++) state.keys[keys[i]] = (GetAsyncKeyState(keys[i]) & 0x8000) != 0;
			// A menu with its own pointer takes the buttons as clicks, not as the triggers (on the launch
			// screen a held right trigger starts the level-unlock code and blocks other input).
			if (MenuDrawsPointer()) state.keys[VK_LBUTTON] = state.keys[VK_RBUTTON] = false;
		}
	}
	pcinput_MapSample(state, keyboard, s_platform, sample);
}

bool pcinput_WindowMessage(UINT message, WPARAM wParam, LPARAM lParam) {
	if (!s_window) return false;
	if (message == WM_KILLFOCUS || (message == WM_ACTIVATEAPP && !wParam) || message == WM_DESTROY) ReleaseMouse();
	if (message == WM_KEYDOWN && !(lParam & (1L << 30))) {
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
	if (message == WM_INPUT && GetForegroundWindow() == s_window) {
		RAWINPUT data;
		UINT size = sizeof(data);
		const UINT read = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &data, &size, sizeof(RAWINPUTHEADER));
		if (read != (UINT)-1 && read >= sizeof(RAWINPUTHEADER) + sizeof(RAWMOUSE) &&
			data.header.dwType == RIM_TYPEMOUSE && !(data.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)) {
			const RAWMOUSE &mouse = data.data.mouse;
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
	if (allowLook || MouseLook() || !s_window || IsIconic(s_window) || !GetClientRect(s_window, &client) ||
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
