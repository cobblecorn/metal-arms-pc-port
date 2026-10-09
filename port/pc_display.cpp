// Display preferences (see pc_display.h).
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pc_display.h"

bool pcinput_SettingsPath(char *path, size_t capacity);	// pc_input.cpp

static PcDisplayMode s_mode = PCDISPLAY_WINDOWED;
static int s_width = 0, s_height = 0;
static int s_fovOffset = 0;
static volatile LONG s_fovVersion = 1;
static PcDisplayMode s_launchedMode = PCDISPLAY_WINDOWED;
static int s_launchedWidth = 0, s_launchedHeight = 0;
static int s_antiAliasing = 0, s_launchedAntiAliasing = 0;

static int ClampAntiAliasing(int samples) {
	return samples >= 8 ? 8 : samples >= 4 ? 4 : samples >= 2 ? 2 : 0;
}

enum { MAX_RESOLUTIONS = 64 };
static int s_resolutions[MAX_RESOLUTIONS][2];
static int s_resolutionCount = 0;

static const char *s_modeNames[PCDISPLAY_MODE_COUNT] = { "windowed", "fullscreen", "borderless" };

const char *pcdisplay_ModeName(PcDisplayMode mode) {
	return mode >= 0 && mode < PCDISPLAY_MODE_COUNT ? s_modeNames[mode] : s_modeNames[0];
}

static int ClampFov(int degrees) {
	if (degrees < PCDISPLAY_FOV_MIN) degrees = PCDISPLAY_FOV_MIN;
	if (degrees > PCDISPLAY_FOV_MAX) degrees = PCDISPLAY_FOV_MAX;
	return degrees;
}

void pcdisplay_Load() {
	char path[MAX_PATH + 64], value[32];
	if (!pcinput_SettingsPath(path, sizeof(path))) return;
	GetPrivateProfileStringA("Display", "Mode", "windowed", value, sizeof(value), path);
	for (int i = 0; i < PCDISPLAY_MODE_COUNT; i++) {
		if (!_stricmp(value, s_modeNames[i])) s_mode = (PcDisplayMode)i;
	}
	GetPrivateProfileStringA("Display", "Resolution", "", value, sizeof(value), path);
	int width, height;
	if (sscanf(value, "%dx%d", &width, &height) == 2 && width >= 640 && height >= 480) {
		s_width = width;
		s_height = height;
	}
	s_fovOffset = ClampFov((int)GetPrivateProfileIntA("Display", "FieldOfView", 0, path));
	s_antiAliasing = ClampAntiAliasing((int)GetPrivateProfileIntA("Display", "AntiAliasing", 0, path));
}

bool pcdisplay_Save() {
	char path[MAX_PATH + 64], value[32];
	if (!pcinput_SettingsPath(path, sizeof(path))) return false;
	bool ok = WritePrivateProfileStringA("Display", "Mode", pcdisplay_ModeName(s_mode), path) != FALSE;
	if (s_width && s_height) {
		_snprintf(value, sizeof(value), "%dx%d", s_width, s_height);
		ok = WritePrivateProfileStringA("Display", "Resolution", value, path) != FALSE && ok;
	}
	_snprintf(value, sizeof(value), "%d", s_fovOffset);
	ok = WritePrivateProfileStringA("Display", "FieldOfView", value, path) != FALSE && ok;
	_snprintf(value, sizeof(value), "%d", s_antiAliasing);
	ok = WritePrivateProfileStringA("Display", "AntiAliasing", value, path) != FALSE && ok;
	return ok;
}

PcDisplayMode pcdisplay_Mode() { return s_mode; }
void pcdisplay_SetMode(PcDisplayMode mode) {
	if (mode >= 0 && mode < PCDISPLAY_MODE_COUNT) s_mode = mode;
}

void pcdisplay_Resolution(int *width, int *height) {
	if (width) *width = s_width;
	if (height) *height = s_height;
}
void pcdisplay_SetResolution(int width, int height) {
	if (width >= 640 && height >= 480) {
		s_width = width;
		s_height = height;
	}
}

void pcdisplay_AddAvailableResolution(int width, int height) {
	if (width < 640 || height < 480) return;
	int at = 0;
	for (; at < s_resolutionCount; at++) {
		const int w = s_resolutions[at][0], h = s_resolutions[at][1];
		if (w == width && h == height) return;
		if (w > width || (w == width && h > height)) break;
	}
	if (s_resolutionCount == MAX_RESOLUTIONS) return;
	memmove(&s_resolutions[at + 1], &s_resolutions[at], sizeof(s_resolutions[0]) * (s_resolutionCount - at));
	s_resolutions[at][0] = width;
	s_resolutions[at][1] = height;
	s_resolutionCount++;
}
int pcdisplay_AvailableResolutionCount() { return s_resolutionCount; }
void pcdisplay_AvailableResolution(int index, int *width, int *height) {
	if (index < 0 || index >= s_resolutionCount) {
		*width = *height = 0;
		return;
	}
	*width = s_resolutions[index][0];
	*height = s_resolutions[index][1];
}

void pcdisplay_SetLaunched(PcDisplayMode mode, int width, int height) {
	s_launchedMode = mode;
	s_launchedWidth = width;
	s_launchedHeight = height;
}
int pcdisplay_AntiAliasing() { return s_antiAliasing; }
void pcdisplay_SetAntiAliasing(int samples) { s_antiAliasing = ClampAntiAliasing(samples); }
int pcdisplay_StepAntiAliasing(int samples, int direction) {
	static const int steps[] = { 0, 2, 4, 8 };
	int at = 0;
	for (int i = 0; i < 4; i++) if (steps[i] == ClampAntiAliasing(samples)) at = i;
	at += direction > 0 ? 1 : (direction < 0 ? -1 : 0);
	return steps[at < 0 ? 0 : (at > 3 ? 3 : at)];
}
void pcdisplay_SetLaunchedAntiAliasing(int samples) { s_launchedAntiAliasing = ClampAntiAliasing(samples); }

bool pcdisplay_RestartNeeded() {
	if (s_mode != s_launchedMode) return true;
	if (s_antiAliasing != s_launchedAntiAliasing) return true;
	// Borderless always covers the desktop; the resolution choice applies to the other modes.
	if (s_mode == PCDISPLAY_BORDERLESS) return false;
	return s_width && s_height && (s_width != s_launchedWidth || s_height != s_launchedHeight);
}

static bool s_relaunch = false;
void pcdisplay_RequestRelaunch() { s_relaunch = true; }
bool pcdisplay_RelaunchRequested() { return s_relaunch; }

int pcdisplay_FovOffset() { return s_fovOffset; }
void pcdisplay_SetFovOffset(int degrees) {
	degrees = ClampFov(degrees);
	if (degrees != s_fovOffset) {
		s_fovOffset = degrees;
		InterlockedIncrement(&s_fovVersion);
	}
}
unsigned pcdisplay_FovVersion() { return (unsigned)s_fovVersion; }

float pcdisplay_AdjustHalfFov(float halfFovRadians) {
	float half = halfFovRadians + 0.5f * (float)s_fovOffset * 3.14159265f / 180.0f;
	const float minHalf = 10.0f * 3.14159265f / 180.0f, maxHalf = 75.0f * 3.14159265f / 180.0f;
	if (half < minHalf) half = minHalf;
	if (half > maxHalf) half = maxHalf;
	return half;
}
