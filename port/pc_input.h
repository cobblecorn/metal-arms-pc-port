// Desktop controls feeding Fang's existing controller samples.
#pragma once

#include <windows.h>
#include <Xinput.h>
#include "fpadio.h"

// Separate acquisition from mapping so focus, deadzones and button semantics can
// be checked without a controller or synthetic input on the user's desktop.
struct PcInputState {
	XINPUT_GAMEPAD pad;
	bool connected;
	bool focused;
	bool menus;		// the port is in a menu (not the gameplay control map): Escape is Back, not START
	bool textInput;	// a native text field owns printable keyboard input
	bool keys[256];
};

void pcinput_MapSample(const PcInputState &state, bool primary,
	FPadio_InputEmulationPlatform_e platform, FPadio_Sample_t *sample);

// Keyboard/mouse always use game port 0. Normal SHARED solo/front-end input
// also routes whichever connected controller is being used to that port. Explicit
// SHARED sessions map pad N to port N; SEPARATE keeps keyboard-only port 0 and
// maps pads 1-3 to ports 1-3. Set with -input-layout / MA_PORT_INPUT_LAYOUT.
// AUTO uses explicit ownership for menu-created sessions: the device opening join is P1,
// other controllers preview unused join slots and are bound when those players accept.
// Once launched, unclaimed controllers may go to keyboard P1; joined partner devices
// retain their identity across disconnect/reconnect. Keyboard/mouse always remain P1.
enum PcInputLayout { PCINPUT_LAYOUT_SHARED, PCINPUT_LAYOUT_SEPARATE, PCINPUT_LAYOUT_AUTO };
bool pcinput_ParseLayout(const char *text, PcInputLayout *layout);
// The XInput pad index feeding a game port, or -1 for none.
int pcinput_PadForPort(PcInputLayout layout, u32 port);
// The game port the keyboard and mouse feed.
u32 pcinput_KeyboardPort();
// The layout chosen at install.
PcInputLayout pcinput_Layout();
// Temporary local multiplayer routing for a session of `players` players; does not change the
// configured layout. Starting a session clears the AUTO pad assignment.
void pcinput_SetLocalCoopSession(bool active, PcInputLayout layout = PCINPUT_LAYOUT_AUTO, u32 players = 2);
// Changes an AUTO session's port count, keeping the pads already dealt to ports below it (the co-op
// join screen deals for four; the game keeps those pads for the ports that joined).
void pcinput_SetLocalCoopPlayers(u32 players);
// Menu-only ownership. Player connection alone never commits a join.
void pcinput_BeginLocalJoin(u32 ownerPort);
void pcinput_ClaimLocalJoinPort(u32 port);
void pcinput_KeepLocalJoinPlayers(u32 mask);
void pcinput_FinishLocalJoin(u32 mask);
// XInput pads connected now (probed at most every quarter second while absent), and one by XInput slot.
u32 pcinput_ConnectedPadCount();
bool pcinput_XInputPadConnected(u32 pad);
// Changes whenever AUTO deals or drops a pad, so the game can report the new assignment.
u32 pcinput_PadAssignmentSerial();
bool pcinput_Install(u32 window, FPadio_InputEmulationPlatform_e platform);
void pcinput_Uninstall();
void pcinput_GetDeviceInfo(u32 index, FPadio_DeviceInfo_t *info);
void pcinput_Sample(u32 index, FPadio_Sample_t *sample);
// True when the XInput device assigned to this game port is currently connected. This describes
// the XInput transport only; it does not identify Xbox versus PlayStation controller hardware.
bool pcinput_XInputConnected(u32 port);
// Window thread receives raw motion; game thread consumes it once per input frame.
bool pcinput_WindowMessage(UINT message, WPARAM wParam, LPARAM lParam);
void pcinput_BeginFrame(bool allowLook);
float pcinput_TakeMouseAxis(u32 controller, bool pitch);

// Menus: an absolute pointer over the client area while mouse look is not captured. Positions are
// fractions of the client area (0..1 across and down), which the back buffer is stretched over.
// The pointer shows when the mouse moves or clicks over the client area and hides when it leaves,
// focus is lost, or pcinput_HideMenuPointer() reports keyboard/pad menu input.
bool pcinput_MenuPointer(float *x, float *y);	// false while hidden
bool pcinput_MenuPointerMoved();				// moved or clicked over the client area this frame
// The oldest waiting button press and where it happened (x/y may be NULL); each press is returned once.
bool pcinput_TakeMenuClick(bool right, float *x, float *y);
int pcinput_TakeMenuWheel();					// wheel notches this frame, positive away from the user
void pcinput_HideMenuPointer();
// Called each frame by a menu that draws its own pointer: the system cursor is hidden over the
// client area while that continues.
void pcinput_DrawsMenuPointer();

// Mouse sensitivity in degrees per raw count, before the profile look multiplier.
// Saved in the PC settings file; a valid launch/environment override locks editing and saving.
bool pcinput_ParseMouseSensitivity(const char *text, float *value);
float pcinput_MouseSensitivity();
bool pcinput_MouseSensitivityIsOverride();
bool pcinput_SetMouseSensitivity(float value);
bool pcinput_SaveMouseSensitivity();

// The game's target assistance (reticle snapping, aim biasing, shot focusing) is tuned for
// sticks. AUTO applies it unless the controller's most recent aiming came from the mouse;
// ON and OFF force it. Set with -aim-assist or MA_PORT_AIM_ASSIST (auto, on, off).
enum PcAimAssistMode { PCINPUT_AIM_ASSIST_AUTO, PCINPUT_AIM_ASSIST_ON, PCINPUT_AIM_ASSIST_OFF };
bool pcinput_ParseAimAssistMode(const char *text, PcAimAssistMode *mode);
bool pcinput_AimAssistAllowed(u32 controller);
// True while the controller's port is aiming with captured mouse look (the mouse moved last).
bool pcinput_IsMouseAiming(u32 controller);
// True when the active prompt theme uses controller buttons rather than keyboard keys.
bool pcinput_PromptsForPad();

// Prompt presentation can follow the most recently used device (AUTO), or be held to a chosen
// keyboard, Xbox, or PlayStation layout. Set it in Advanced Settings or override it with
// -button-prompts / MA_PORT_BUTTON_PROMPTS.
enum PcPromptStyle {
	PCINPUT_PROMPT_STYLE_AUTO,
	PCINPUT_PROMPT_STYLE_KEYBOARD,
	PCINPUT_PROMPT_STYLE_XBOX,
	PCINPUT_PROMPT_STYLE_PLAYSTATION
};
bool pcinput_ParsePromptStyle(const char *text, PcPromptStyle *style);
// The selected preference (AUTO, keyboard, Xbox, or PlayStation), before AUTO resolves against the
// most recently used device. A valid -button-prompts / MA_PORT_BUTTON_PROMPTS value locks this run.
PcPromptStyle pcinput_PromptStyleSetting();
bool pcinput_PromptStyleIsCommandLineOverride();
bool pcinput_SetPromptStyleSetting(PcPromptStyle style); // updates the current run; false when locked
bool pcinput_SavePromptStyleSetting();                 // saves the selected preference for future runs
PcPromptStyle pcinput_ResolvedPromptStyle();
// A manual style applies to every port. AUTO follows the most recently active device on the
// keyboard's shared port, keeps separate-layout keyboard prompts on keys, and uses Xbox glyphs for
// pad-only ports because XInput cannot identify Sony hardware.
PcPromptStyle pcinput_PromptStyleForPort(u32 port);
bool pcinput_UseKeyboardPrompts();
bool pcinput_UsePlayStationPrompts();
bool pcinput_UseKeyboardPromptsForPort(u32 port);
bool pcinput_UsePlayStationPromptsForPort(u32 port);

// Native text fields call SetTextInput while active. Printable WM_CHAR input, Backspace ('\b') and
// Enter ('\r') are queued separately from the controller sample so typed characters never fire
// gameplay bindings (Enter is not START while a field is active).
void pcinput_SetTextInput(bool active);
bool pcinput_TakeTextInput(wchar_t *character);
bool pcinput_IsTextInput();

// A key held down while the game window has focus (screens that read extra keys directly, such as the
// pause menu's page keys). Never true while another window is in front.
bool pcinput_KeyHeld(int key);

// Device-independent readings for new menus and port UI. Values are sampled from the same
// focus-filtered, normalized input sample that feeds the original gamepad maps. Directional values
// and triggers range from 0 to 1; digital buttons are 0 or 1. PRESSED/RELEASED are edges from the
// previous game frame (active at 0.35 or above). CONFIRM and BACK follow the active menu context;
// BACK is false during gameplay. NAV_* includes stick directions while a menu is active. LOOK_* is
// the right stick; captured mouse look remains pcinput_TakeMouseAxis.
enum PcInputAction {
	PCINPUT_ACTION_CONFIRM,
	PCINPUT_ACTION_BACK,
	PCINPUT_ACTION_PAUSE,
	PCINPUT_ACTION_NAV_UP,
	PCINPUT_ACTION_NAV_DOWN,
	PCINPUT_ACTION_NAV_LEFT,
	PCINPUT_ACTION_NAV_RIGHT,
	PCINPUT_ACTION_MOVE_FORWARD,
	PCINPUT_ACTION_MOVE_BACK,
	PCINPUT_ACTION_MOVE_LEFT,
	PCINPUT_ACTION_MOVE_RIGHT,
	PCINPUT_ACTION_LOOK_UP,
	PCINPUT_ACTION_LOOK_DOWN,
	PCINPUT_ACTION_LOOK_LEFT,
	PCINPUT_ACTION_LOOK_RIGHT,
	PCINPUT_ACTION_FIRE_PRIMARY,
	PCINPUT_ACTION_FIRE_SECONDARY,
	PCINPUT_ACTION_JUMP,
	PCINPUT_ACTION_ACTION,
	PCINPUT_ACTION_SELECT_PRIMARY,
	PCINPUT_ACTION_SELECT_SECONDARY,
	PCINPUT_ACTION_QUICK_SELECT_UP,
	PCINPUT_ACTION_QUICK_SELECT_DOWN,
	PCINPUT_ACTION_QUICK_SELECT_LEFT,
	PCINPUT_ACTION_QUICK_SELECT_RIGHT,
	PCINPUT_ACTION_MELEE,
	PCINPUT_ACTION_MELEE_SECONDARY,
	PCINPUT_ACTION_UP_EUK,
	PCINPUT_ACTION_COUNT
};

enum PcInputActionPhase {
	PCINPUT_ACTION_HELD,
	PCINPUT_ACTION_PRESSED,
	PCINPUT_ACTION_RELEASED
};

float pcinput_ActionValue(u32 port, PcInputAction action);
bool pcinput_Action(u32 port, PcInputAction action, PcInputActionPhase phase);
