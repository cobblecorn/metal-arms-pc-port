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
