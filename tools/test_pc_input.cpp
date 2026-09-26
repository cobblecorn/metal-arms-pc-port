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
	printf("PC input mapping checks: %s\n", failures ? "FAILED" : "passed");
	return failures ? 1 : 0;
}
