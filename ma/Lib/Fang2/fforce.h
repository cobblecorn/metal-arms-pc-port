//////////////////////////////////////////////////////////////////////////////////////
// fforce.h - Force feedback system to handle high-level waveforms.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2002
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 03/13/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FFORCE_H_
#define _FFORCE_H_ 1

#include "fang.h"


//
//	EFFECT LIBRARY
//
//	Enum for all of the canned force feedback effects that
//	can be reproduced by Fang.  Note that the behaviour of
//	the effect may differ between platforms due to varying
//	hardware support:
//
typedef enum
{
	FFORCE_EFFECT_NONE = 0,
	FFORCE_EFFECT_ARROW_THUNK_LIGHT,
	FFORCE_EFFECT_ARROW_THUNK_MED,
	FFORCE_EFFECT_ARROW_THUNK_HEAVY,
	FFORCE_EFFECT_LIGHT_PULSE,
	FFORCE_EFFECT_ROCKET_THRUST_LIGHT,
	FFORCE_EFFECT_ROCKET_THRUST_HEAVY,
	FFORCE_EFFECT_ROUGH_RUMBLE,
	FFORCE_EFFECT_ROUGH_RUMBLE_LOW,
	FFORCE_EFFECT_ROUGH_RUMBLE_MED,
	FFORCE_EFFECT_ROUGH_RUMBLE_HIGH,
	FFORCE_EFFECT_ROUGH_RUMBLE_MAX,
	FFORCE_EFFECT_LIGHT_BUZZ,
	FFORCE_EFFECT_MENU_RUMBLE,// used in the wrappers

	FFORCE_EFFECT_ENERGY_PULSE,

	FFORCE_MAX_FEEDBACK_EFFECTS
	
} FForce_Effect_e;

typedef enum
{
	FFORCE_DOMAIN_NORMAL,	// The vibration is to be played during normal gameplay
	FFORCE_DOMAIN_PAUSED,	// The vibration is to be played while the game is paused

	FFORCE_DOMAIN_COUNT
} FForceDomain_e;


//
//
typedef struct FForceHandle_s
{
	u32 nKey;				// Used to determine whether this handle is obsolete
	void *apEnv[2];			// NULL if this is a NULL handle
} FForceHandle_t;


//
//
extern BOOL fforce_ModuleStartup( void );
extern void fforce_ModuleShutdown( void );
extern void fforce_Reset( void );
extern f32  fforce_SetMasterIntensity( u32 nDeviceIndex, f32 fUnitIntensity );
extern f32  fforce_GetMasterIntensity( u32 nDeviceIndex );
extern void fforce_Work( void );
extern BOOL fforce_Play( u32 nDeviceIndex, FForce_Effect_e nEffect, FForceHandle_t *phDestHandle = NULL, FForceDomain_e nDomain = FFORCE_DOMAIN_NORMAL );
extern void fforce_Kill( FForceHandle_t *phHandle );

FINLINE void fforce_NullHandle( FForceHandle_t *phForce ) {
	phForce->apEnv[0] = NULL;
	phForce->apEnv[1] = NULL;
	phForce->nKey = 0;
}


#endif

