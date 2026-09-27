#include "pc_input.h"
#include <math.h>
#include <stdio.h>

static int failures;
static void Check(bool result, const char *message) {
	if (!result) { fprintf(stderr, "FAIL: %s\n", message); ++failures; }
}
static bool Near(float a, float b) { return fabsf(a-b) < 0.0001f; }
static float Value(const FPadio_Sample_t &s, FPadio_InputID_e id) { return s.afInputValues[id-1]; }
static bool Neutral(const FPadio_Sample_t &s) {
	for (u32 i=0; i<FPADIO_MAX_INPUTS; i++) if (s.afInputValues[i] != 0.0f) return false;
	return true;
}

// Exercise the actual INI read/write path in a disposable settings directory.
static void CheckMouseSettings() {
	char temp[MAX_PATH], root[MAX_PATH], path[MAX_PATH + 64];
	char oldLocal[32768] = {}, oldOverride[32768] = {}, oldTestKeys[32768] = {};
	GetEnvironmentVariableA("LOCALAPPDATA", oldLocal, sizeof(oldLocal));
	GetEnvironmentVariableA("MA_PORT_TEST_KEYS", oldTestKeys, sizeof(oldTestKeys));
	GetEnvironmentVariableA("MA_PORT_MOUSE_SENSITIVITY", oldOverride, sizeof(oldOverride));
	if (!GetTempPathA(sizeof(temp), temp) || !GetTempFileNameA(temp, "mai", 0, root)) {
		Check(false, "create isolated settings directory"); return;
	}
	DeleteFileA(root);
	if (!CreateDirectoryA(root, NULL)) { Check(false, "create isolated settings directory"); return; }
	SetEnvironmentVariableA("LOCALAPPDATA", root);
	SetEnvironmentVariableA("MA_PORT_MOUSE_SENSITIVITY", NULL);
	const FPadio_InputEmulationPlatform_e gc = FPADIO_INPUT_EMULATION_PLATFORM_GC;
	pcinput_Install(0, gc);
	Check(Near(pcinput_MouseSensitivity(), 0.1f), "missing settings preserve default mouse speed");
	Check(pcinput_SetMouseSensitivity(0.17f) && pcinput_SaveMouseSensitivity(), "mouse sensitivity saves");
	Check(!pcinput_SetMouseSensitivity(0) && !pcinput_SetMouseSensitivity(11) && Near(pcinput_MouseSensitivity(), 0.17f), "invalid mouse speeds leave current setting intact");
	pcinput_Uninstall();
	pcinput_Install(0, gc);
	Check(Near(pcinput_MouseSensitivity(), 0.17f), "mouse sensitivity survives input reinstall");
	pcinput_Uninstall();
	SetEnvironmentVariableA("MA_PORT_MOUSE_SENSITIVITY", "0.25");
	pcinput_Install(0, gc);
	Check(Near(pcinput_MouseSensitivity(), 0.25f) && pcinput_MouseSensitivityIsOverride(), "launch override takes precedence over saved mouse speed");
	Check(!pcinput_SetMouseSensitivity(0.3f) && !pcinput_SaveMouseSensitivity(), "override cannot edit or overwrite saved preference");
	pcinput_Uninstall();
	SetEnvironmentVariableA("MA_PORT_MOUSE_SENSITIVITY", "invalid");
	pcinput_Install(0, gc);
	Check(Near(pcinput_MouseSensitivity(), 0.17f) && !pcinput_MouseSensitivityIsOverride(), "invalid override preserves editable saved preference");
	pcinput_Uninstall();
	_snprintf(path, sizeof(path), "%s\\Metal Arms Source Port\\settings.ini", root);
	WritePrivateProfileStringA("Input", "MouseSensitivity", "nan", path);
	SetEnvironmentVariableA("MA_PORT_MOUSE_SENSITIVITY", NULL);
	pcinput_Install(0, gc);
	Check(Near(pcinput_MouseSensitivity(), 0.1f), "corrupt saved sensitivity falls back to default");
	pcinput_Uninstall();
	SetEnvironmentVariableA("MA_PORT_TEST_KEYS", "0:0x28");
	pcinput_Install(0, gc);
	FPadio_Sample_t scripted;
	pcinput_Sample(pcinput_KeyboardPort(), &scripted);
	Check(Near(Value(scripted, FPADIO_INPUT_STICK_RIGHT_Y), -1), "scripted menu keys reach unfocused game");
	Check(Value(scripted, FPADIO_INPUT_TRIGGER_RIGHT) == 0 && Value(scripted, FPADIO_INPUT_STICK_LEFT_X) == 0, "background script does not admit physical controller input");
	pcinput_Uninstall();
	SetEnvironmentVariableA("MA_PORT_TEST_KEYS", NULL);
	pcinput_Install(0, gc);
	pcinput_Sample(pcinput_KeyboardPort(), &scripted);
	Check(Neutral(scripted), "background game without scripted keys stays neutral");
	pcinput_Uninstall();
	SetEnvironmentVariableA("MA_PORT_TEST_KEYS", *oldTestKeys ? oldTestKeys : NULL);
	SetEnvironmentVariableA("LOCALAPPDATA", *oldLocal ? oldLocal : NULL);
	SetEnvironmentVariableA("MA_PORT_MOUSE_SENSITIVITY", *oldOverride ? oldOverride : NULL);
	DeleteFileA(path);
	_snprintf(path, sizeof(path), "%s\\Metal Arms Source Port", root);
	RemoveDirectoryA(path);
	RemoveDirectoryA(root);
}

int main() {
	const FPadio_InputEmulationPlatform_e gc = FPADIO_INPUT_EMULATION_PLATFORM_GC;
	FPadio_Sample_t out;
	PcInputState state = {};
	state.focused = true;
	pcinput_MapSample(state, true, gc, &out);
	Check(out.bValid && Neutral(out), "keyboard remains available without a controller");
	pcinput_MapSample(state, false, gc, &out);
	Check(!out.bValid && Neutral(out), "empty secondary port is disconnected");
	state.keys['W'] = state.keys['D'] = true;
	state.keys[VK_SPACE] = state.keys['E'] = state.keys['F'] = true;
	state.keys[VK_LBUTTON] = state.keys[VK_RBUTTON] = true;
	pcinput_MapSample(state, true, gc, &out);
	Check(Near(Value(out,FPADIO_INPUT_STICK_LEFT_X),0.70710678f) && Near(Value(out,FPADIO_INPUT_STICK_LEFT_Y),0.70710678f), "diagonal keyboard movement has unit length");
	Check(Value(out,FPADIO_INPUT_CROSS_BOTTOM)==1 && Value(out,FPADIO_INPUT_CROSS_TOP)==1 && Value(out,FPADIO_INPUT_GC_DBUTTON_TRIGGER_Z)==1, "jump/action/melee mapping");
	Check(Value(out,FPADIO_INPUT_TRIGGER_LEFT)==1 && Value(out,FPADIO_INPUT_TRIGGER_RIGHT)==1 && Value(out,FPADIO_INPUT_GC_DBUTTON_TRIGGER_LEFT)==1 && Value(out,FPADIO_INPUT_GC_DBUTTON_TRIGGER_RIGHT)==1, "mouse buttons drive analog and GC digital triggers");
	state.keys['S'] = state.keys['A'] = true;
	pcinput_MapSample(state,true,gc,&out);
	Check(Value(out,FPADIO_INPUT_STICK_LEFT_X)==0 && Value(out,FPADIO_INPUT_STICK_LEFT_Y)==0, "opposite movement keys cancel");
	state.focused = false;
	state.connected = true;
	state.pad.wButtons = 0xffff;
	state.pad.sThumbLX = 32767;
	pcinput_MapSample(state,true,gc,&out);
	Check(out.bValid && Neutral(out), "focus loss releases keyboard, mouse and gamepad");
	state = PcInputState(); state.focused = state.connected = true;
	state.pad.sThumbLX = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
	state.pad.sThumbRY = -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
	state.pad.bLeftTrigger = XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
	pcinput_MapSample(state,false,gc,&out);
	Check(out.bValid && Neutral(out), "stick and trigger deadzone boundaries are neutral");
	state.pad.sThumbLX = -32768; state.pad.sThumbLY = 32767;
	state.pad.sThumbRX = 32767; state.pad.sThumbRY = 0;
	state.pad.bLeftTrigger = 255; state.pad.bRightTrigger = 255;
	state.pad.wButtons = XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_START | XINPUT_GAMEPAD_RIGHT_SHOULDER;
	pcinput_MapSample(state,false,gc,&out);
	float x=Value(out,FPADIO_INPUT_STICK_LEFT_X), y=Value(out,FPADIO_INPUT_STICK_LEFT_Y);
	Check(x<0 && y>0 && Near(x*x+y*y,1), "extreme signed stick values normalize to the unit circle");
	Check(Near(Value(out,FPADIO_INPUT_STICK_RIGHT_X),1), "full stick reaches full output");
	Check(Value(out,FPADIO_INPUT_TRIGGER_LEFT)==1 && Value(out,FPADIO_INPUT_TRIGGER_RIGHT)==1, "independent triggers can both be fully pressed");
	Check(Value(out,FPADIO_INPUT_START)==1 && Value(out,FPADIO_INPUT_CROSS_BOTTOM)==1 && Value(out,FPADIO_INPUT_GC_DBUTTON_TRIGGER_Z)==1, "XInput buttons map to GC gameplay actions");
	state.connected = false;
	pcinput_MapSample(state,false,gc,&out);
	Check(!out.bValid && Neutral(out), "disconnect releases a previously active pad");
	pcinput_MapSample(state,true,gc,&out);
	Check(out.bValid && Neutral(out), "disconnect preserves keyboard and clears old controller input");
	state = PcInputState(); state.focused = true;
	state.keys['Q']=state.keys['R']=state.keys[VK_RETURN]=state.keys['2']=state.keys['1']=true;
	state.keys[VK_RIGHT]=state.keys[VK_DOWN]=true;
	pcinput_MapSample(state,true,gc,&out);
	Check(Value(out,FPADIO_INPUT_CROSS_RIGHT)==1 && Value(out,FPADIO_INPUT_CROSS_LEFT)==1 && Value(out,FPADIO_INPUT_START)==1, "weapon selection and pause");
	Check(Value(out,FPADIO_INPUT_DPAD_X)==1 && Value(out,FPADIO_INPUT_DPAD_Y)==1, "keyboard quick-select directions");
	state = PcInputState(); state.focused = true; state.keys['Q'] = true;
	pcinput_MapSample(state,true,gc,&out);
	Check(Value(out,FPADIO_INPUT_CROSS_LEFT)==1 && Value(out,FPADIO_INPUT_CROSS_RIGHT)==0, "Q opens the throwables (secondary) list");
	state.keys['Q'] = false; state.keys['R'] = true;
	pcinput_MapSample(state,true,gc,&out);
	Check(Value(out,FPADIO_INPUT_CROSS_RIGHT)==1 && Value(out,FPADIO_INPUT_CROSS_LEFT)==0, "R opens the gun (primary) list and reloads on a tap");
	state = PcInputState(); state.focused = true; state.keys[VK_ESCAPE] = true;
	pcinput_MapSample(state,true,gc,&out);
	Check(Value(out,FPADIO_INPUT_START)==1 && Value(out,FPADIO_INPUT_CROSS_LEFT)==0, "Escape pauses during gameplay");
	state.menus = true;
	pcinput_MapSample(state,true,gc,&out);
	// PC menus use the Xbox menu map on both platforms (gamepad_SetMapping), so Back is CROSS_RIGHT (B)
	Check(Value(out,FPADIO_INPUT_START)==0 && Value(out,FPADIO_INPUT_CROSS_RIGHT)==1 && Value(out,FPADIO_INPUT_CROSS_LEFT)==0, "Escape is Back (Xbox B) in menus, GameCube gameplay map");
	pcinput_MapSample(state,true,FPADIO_INPUT_EMULATION_PLATFORM_XB,&out);
	Check(Value(out,FPADIO_INPUT_START)==0 && Value(out,FPADIO_INPUT_CROSS_RIGHT)==1, "Escape is Back (Xbox B) in menus");
	state.keys[VK_ESCAPE] = false; state.keys[VK_RETURN] = true;
	pcinput_MapSample(state,true,gc,&out);
	Check(Value(out,FPADIO_INPUT_START)==1, "Enter is START in menus");
	state.keys[VK_RETURN] = false; state.keys['Q'] = true;
	pcinput_MapSample(state,true,gc,&out);
	Check(Value(out,FPADIO_INPUT_CROSS_LEFT)==0, "Q is not Back in menus (the pause menu turns pages with it)");
	state.keys['Q'] = false; state.menus = false; state.textInput = true;
	state.keys['E']=state.keys['R']=state.keys['W']=state.keys[VK_SPACE]=state.keys['F']=true;
	pcinput_MapSample(state,true,gc,&out);
	Check(Value(out,FPADIO_INPUT_CROSS_TOP)==0 && Value(out,FPADIO_INPUT_CROSS_RIGHT)==0 && Value(out,FPADIO_INPUT_STICK_LEFT_Y)==0 &&
		Value(out,FPADIO_INPUT_CROSS_BOTTOM)==0 && Value(out,FPADIO_INPUT_GC_DBUTTON_TRIGGER_Z)==0, "typing a name fires no game bindings");
	state.keys[VK_RETURN] = true;
	pcinput_MapSample(state,true,gc,&out);
	Check(Value(out,FPADIO_INPUT_START)==0, "Enter is the text field's Done, not START, while typing");
	state = PcInputState(); state.focused = true;
	state.keys[VK_RIGHT]=state.keys[VK_DOWN]=true; state.keys['R'] = false;
	pcinput_MapSample(state,true,gc,&out);
	Check(Value(out,FPADIO_INPUT_STICK_RIGHT_X)==1 && Value(out,FPADIO_INPUT_STICK_RIGHT_Y)==-1, "arrow keys preserve controller look directions");
	state = PcInputState(); state.focused=state.connected=true;
	state.pad.sThumbLX=32767; state.keys['W']=true;
	pcinput_MapSample(state,true,gc,&out);
	x=Value(out,FPADIO_INPUT_STICK_LEFT_X); y=Value(out,FPADIO_INPUT_STICK_LEFT_Y);
	Check(Near(x*x+y*y,1), "mixed keyboard/controller movement cannot exceed unit length");
	state = PcInputState(); state.focused=state.connected=true;
	state.pad.wButtons=XINPUT_GAMEPAD_BACK | XINPUT_GAMEPAD_LEFT_THUMB | XINPUT_GAMEPAD_RIGHT_THUMB | XINPUT_GAMEPAD_RIGHT_SHOULDER | XINPUT_GAMEPAD_LEFT_SHOULDER;
	pcinput_MapSample(state,false,FPADIO_INPUT_EMULATION_PLATFORM_XB,&out);
	Check(Value(out,FPADIO_INPUT_XB_DBUTTON_BACK)==1 && Value(out,FPADIO_INPUT_XB_DBUTTON_STICK_LEFT)==1 && Value(out,FPADIO_INPUT_XB_DBUTTON_STICK_RIGHT)==1 && Value(out,FPADIO_INPUT_XB_ABUTTON_BLACK)==1 && Value(out,FPADIO_INPUT_XB_ABUTTON_WHITE)==1, "Xbox aliases use Xbox semantics");
	Check(pcinput_PadForPort(PCINPUT_LAYOUT_SHARED,0)==0 && pcinput_PadForPort(PCINPUT_LAYOUT_SHARED,3)==3, "shared layout keeps pad n on port n");
	Check(pcinput_PadForPort(PCINPUT_LAYOUT_SEPARATE,0)==-1 && pcinput_PadForPort(PCINPUT_LAYOUT_SEPARATE,1)==0 && pcinput_PadForPort(PCINPUT_LAYOUT_SEPARATE,3)==2, "separate layout moves pads off the keyboard port");
	Check(pcinput_PadForPort(PCINPUT_LAYOUT_SHARED,FPADIO_MAX_DEVICES)==-1, "ports past the last device have no pad");
	PcInputLayout layout = PCINPUT_LAYOUT_SHARED;
	Check(pcinput_ParseLayout("Separate",&layout) && layout==PCINPUT_LAYOUT_SEPARATE && !pcinput_ParseLayout("split",&layout) && layout==PCINPUT_LAYOUT_SEPARATE, "layout names parse case-insensitively and bad names change nothing");
	float sensitivity = 0.1f;
	Check(pcinput_ParseMouseSensitivity("0.001", &sensitivity) && Near(sensitivity, 0.001f), "minimum mouse sensitivity parses");
	Check(pcinput_ParseMouseSensitivity("10", &sensitivity) && Near(sensitivity, 10), "maximum mouse sensitivity parses");
	const char *invalid[] = { "", "nan", "inf", "0", "-1", "10.001", "0.1junk", "1e9999" };
	for (unsigned i = 0; i < sizeof(invalid)/sizeof(invalid[0]); ++i)
		Check(!pcinput_ParseMouseSensitivity(invalid[i], &sensitivity) && Near(sensitivity, 10), "invalid mouse sensitivity leaves output unchanged");
	Check(!pcinput_ParseMouseSensitivity(NULL, &sensitivity) && !pcinput_ParseMouseSensitivity("0.1", NULL), "missing sensitivity input/output rejected");
	// Session routing supports controllers for every player and restores user config.
	const PcInputLayout configuredLayout = pcinput_Layout();
	pcinput_SetLocalCoopSession(true, PCINPUT_LAYOUT_SHARED);
	for (u32 port = 0; port < 4; ++port)
		Check(pcinput_PadForPort(pcinput_Layout(), port) == (int)port, "controller-only co-op keeps pad N on player N");
	pcinput_SetLocalCoopSession(true, PCINPUT_LAYOUT_SEPARATE);
	Check(pcinput_PadForPort(pcinput_Layout(), 0) == -1, "separate co-op reserves P1 for keyboard");
	for (u32 port = 1; port < 4; ++port)
		Check(pcinput_PadForPort(pcinput_Layout(), port) == (int)port - 1, "separate co-op shifts pads to P2-P4");
	pcinput_SetLocalCoopSession(false);
	Check(pcinput_Layout() == configuredLayout, "leaving co-op restores configured layout");
	pcinput_SetLocalCoopSession(true, (PcInputLayout)99);
	Check(pcinput_Layout() == PCINPUT_LAYOUT_SHARED, "invalid session layout defaults to shared");
	pcinput_SetLocalCoopSession(false);
	CheckMouseSettings();
	printf("PC input mapping checks: %s\n", failures ? "FAILED" : "passed");
	return failures ? 1 : 0;
}
