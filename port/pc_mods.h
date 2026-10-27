#pragma once

#include "fang.h"
#include "ffile.h"

// Set only by a modded launch. Vanilla launches never consult a mod directory.
BOOL pcmods_ResolveFile(const char *requested, char *output, unsigned capacity);
const FFileMasterEntry_t *pcmods_OpenStream(const char *requested, u32 *size);
BOOL pcmods_ReadStream(const FFileMasterEntry_t *entry, u32 start, u32 amount,
    void *destination, FFileAsyncReadCallback_t *callback, void *user, s32 *result);
BOOL pcmods_Option(const char *option);
s32 pcmods_Value(const char *option, s32 fallback);
u32 pcmods_Seed();
u32 pcmods_Roll(const char *world, const char *identity, const void *position, unsigned positionBytes, u32 salt);

// Named PC adapters and additive map/media declarations (API v3).
struct PcModLevel {
    char title[128], world[16], csv[16], materials[16];
};
BOOL pcmods_Feature(const char *name);
unsigned pcmods_CustomLevelCount();
const PcModLevel *pcmods_CustomLevel(unsigned index);
BOOL pcmods_IsCustomMPIndex(unsigned menuIndex);
BOOL pcmods_IsCustomLevel(s32 levelId);
BOOL pcmods_ResolveMedia(const char *requested, char *output, unsigned capacity);
BOOL pcmods_HasMovies();
