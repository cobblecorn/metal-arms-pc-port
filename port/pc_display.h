// Display preferences: window mode, resolution and gameplay field of view.
// Stored in settings.ini [Display] beside the input settings. Mode and resolution are read at
// launch (command-line -res / -fullscreen / -borderless / -windowed override them for that run);
// the field of view applies live.
#pragma once

enum PcDisplayMode { PCDISPLAY_WINDOWED, PCDISPLAY_FULLSCREEN, PCDISPLAY_BORDERLESS, PCDISPLAY_MODE_COUNT };

// Read settings.ini (call after the save root is known).
void pcdisplay_Load();
// Write the current choices to settings.ini.
bool pcdisplay_Save();

PcDisplayMode pcdisplay_Mode();
void pcdisplay_SetMode(PcDisplayMode mode);
const char *pcdisplay_ModeName(PcDisplayMode mode);

// The saved resolution; 0x0 when none is saved (the launcher's default is used).
void pcdisplay_Resolution(int *width, int *height);
void pcdisplay_SetResolution(int width, int height);

// The resolutions offered in the menu (the adapter's fullscreen modes), smallest first.
void pcdisplay_AddAvailableResolution(int width, int height);
int pcdisplay_AvailableResolutionCount();
void pcdisplay_AvailableResolution(int index, int *width, int *height);

// What this run was started with, to tell whether a change needs a restart.
void pcdisplay_SetLaunched(PcDisplayMode mode, int width, int height);
bool pcdisplay_RestartNeeded();

// Extra full field of view, in degrees, for the third-person and vehicle cameras.
int pcdisplay_FovOffset();
void pcdisplay_SetFovOffset(int degrees);
// Changes whenever the offset does, so cameras can re-apply it.
unsigned pcdisplay_FovVersion();
// A retail half field of view (radians) with the player's offset added.
float pcdisplay_AdjustHalfFov(float halfFovRadians);

enum { PCDISPLAY_FOV_MIN = -10, PCDISPLAY_FOV_MAX = 40, PCDISPLAY_FOV_STEP = 5 };
