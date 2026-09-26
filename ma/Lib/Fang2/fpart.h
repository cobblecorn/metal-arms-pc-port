//////////////////////////////////////////////////////////////////////////////////////
// fpart.h - Fang Particle module.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 09/14/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FPART_H_
#define _FPART_H_ 1

#include "fang.h"
#include "fmath.h"

class CFColorRGB;
struct FTexDef_t;

typedef enum {
	FPART_TYPE_UNASSIGNED = -1,
	FPART_TYPE_SPARK_BURST = 0,
	FPART_TYPE_SPARK_MINIBURST,
	FPART_TYPE_SMOKE_PUFF,
	FPART_TYPE_STEAM,
	FPART_TYPE_SPARK_EXPLOSION,
	FPART_TYPE_SMOKE_CLOUD,
	FPART_TYPE_STEAM_PUFF,
	FPART_TYPE_PIXEL_DUST1,
	FPART_TYPE_DUST_PUFF,
	FPART_TYPE_PIXIE_PUFF,			// Not to be confused with Power Puff

	FPART_TYPE_COUNT
} FPart_Type_e;


#if 0
extern BOOL fpart_ModuleStartup( void );
extern void fpart_ModuleShutdown( void );

///////////////////////////////////////////////////////////////////////
// To use fpart system, call Setup and then load the desired textures
// and call SetTextures().  At the end of each level, call Reset() and 
// then call SetTextures() after the next level is loaded.
//////////////////////////////////////////////////////////////////////

// Allocates memory for particle emitters, does not load any textures.
extern BOOL fpart_InitSystem( u32 nMaxEmitters, u32 nMaxParticles );

extern void fpart_UninitSystem( void ); 

// Stops all active emitters and returns the system as if Setup() was just called.
// Unattaches any textures that may have been set.
extern void fpart_Reset( void );

// Kills all active particles, but doesn't reset the system.
extern void fpart_KillAllActiveParticles( void );

// Attaches texture(s) to a particle type, if this is not called, then solid colored particles will be emitted.
// This function must be called after Setup() or Reset().
extern BOOL fpart_SetTextures( FPart_Type_e nType, FTexDef_t **apTexturePalette, u32 nNumTextures );
#endif


// Emits particles into the world
extern BOOL fpart_SpawnParticleEmitter( FPart_Type_e nType, f32 fUnitIntensity, const CFVec3A &rPos, const CFVec3A *pUnitDir=NULL,
									    BOOL bSpawnLight=FALSE, const CFVec3A *pEmitterVelPerSec_WS=NULL, const CFColorRGB *prgbaColorBias = NULL );

#endif

