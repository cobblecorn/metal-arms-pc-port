#pragma once

#include "amx.h"
#include "fclib.h"

// Recognize the original booth script before interpreting its private globals.
// All offsets refer to expanded SmallAMX code/data, never to a save-file layout.
static BOOL PortBoothArrivalLayout(const char *name, const AMX *amx, unsigned dataSize) {
    if (!name || fclib_stricmp(name, "rsboothdoor.sma") || !amx || !amx->base) return FALSE;
    const AMX_HEADER *h = (const AMX_HEADER *)amx->base;
    if (h->magic != AMX_MAGIC || h->cod < (int)sizeof(AMX_HEADER) ||
        h->cod % sizeof(cell) || h->dat < h->cod ||
        h->dat - h->cod < 0x14c4 || h->hea < h->dat ||
        h->hea - h->dat < 120 || dataSize < 120) return FALSE;
    const cell *c = (const cell *)(amx->base + h->cod);
    // LOAD_PRI arrivals; EQ_C_PRI 4; CONST_PRI 4; STOR_PRI sceneState.
    // Guard GOTO stage finishes by setting -1; the normal closing stage sets 1.
    return c[0x878/4] == 1 && c[0x87c/4] == 92 &&
        c[0x880/4] == 105 && c[0x884/4] == 4 &&
        c[0x89c/4] == 11 && c[0x8a0/4] == 4 &&
        c[0x8a4/4] == 15 && c[0x8a8/4] == 32 &&
        c[0x13d8/4] == 11 && c[0x13dc/4] == -1 &&
        c[0x13e0/4] == 15 && c[0x13e4/4] == 32 &&
        c[0x1478/4] == 11 && c[0x147c/4] == 1 &&
        c[0x1480/4] == 15 && c[0x1484/4] == 32;
}

static BOOL PortBoothArrivalFallback(const char *name, const AMX *amx,
    void *data, unsigned dataSize, float seconds, BOOL sceneAllowed,
    BOOL presentationDone, float &waitSeconds) {
    if (!sceneAllowed || !data || !PortBoothArrivalLayout(name, amx, dataSize)) {
        waitSeconds = 0.0f;
        return FALSE;
    }
    cell *g = (cell *)data;
    if (g[108/4] != 1 || g[32/4] != -1 || g[92/4] < 0 || g[92/4] >= 4) {
        waitSeconds = 0.0f;
        return FALSE;
    }
    if (seconds > 0.0f) waitSeconds += seconds;
    if (waitSeconds < 15.0f || !presentationDone) return FALSE;
    // Resume the existing close-door -> camera-done -> camera release ->
    // half-second delay -> Game_EndCutScene -> checkpoint sequence. Mark all
    // arrivals consumed so a late real guard entry cannot restart that ending.
    g[92/4] = 4;
    g[32/4] = 4;
    waitSeconds = 0.0f;
    return TRUE;
}
