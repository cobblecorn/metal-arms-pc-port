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
	bool keys[256];
};

void pcinput_MapSample(const PcInputState &state, bool primary,
	FPadio_InputEmulationPlatform_e platform, FPadio_Sample_t *sample);
bool pcinput_Install(u32 window, FPadio_InputEmulationPlatform_e platform);
void pcinput_Uninstall();
void pcinput_GetDeviceInfo(u32 index, FPadio_DeviceInfo_t *info);
void pcinput_Sample(u32 index, FPadio_Sample_t *sample);
// Window thread receives raw motion; game thread consumes it once per input frame.
bool pcinput_WindowMessage(UINT message, WPARAM wParam, LPARAM lParam);
void pcinput_BeginFrame(bool allowLook);
float pcinput_TakeMouseAxis(u32 controller, bool pitch);

// The game's target assistance (reticle snapping, aim biasing, shot focusing) is tuned for
// sticks. AUTO applies it unless the controller's most recent aiming came from the mouse;
// ON and OFF force it. Set with -aim-assist or MA_PORT_AIM_ASSIST (auto, on, off).
enum PcAimAssistMode { PCINPUT_AIM_ASSIST_AUTO, PCINPUT_AIM_ASSIST_ON, PCINPUT_AIM_ASSIST_OFF };
bool pcinput_ParseAimAssistMode(const char *text, PcAimAssistMode *mode);
bool pcinput_AimAssistAllowed(u32 controller);
