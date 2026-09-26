//////////////////////////////////////////////////////////////////////////////////////
// smoketrail.h - Smoke trail system.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 08/20/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _SMOKETRAIL_H_
#define _SMOKETRAIL_H_ 1

#include "fang.h"
#include "fmath.h"
#include "fcolor.h"


struct FTexDef_t;

typedef void *SmokeTrailHandle_t;
#define SMOKETRAIL_NULLHANDLE	((SmokeTrailHandle_t)NULL)


enum {
	SMOKETRAIL_FLAG_NO_FOG				= 0x00000001,	// Don't apply fogging
	SMOKETRAIL_FLAG_ENABLE_DEPTH_WRITES	= 0x00000002,	// Enable depth buffer writing
	SMOKETRAIL_FLAG_ADDITIVE_BLEND_MODE	= 0x00000004,	// Use additive blending (default is modulation blending)

	SMOKETRAIL_FLAG_NONE				= 0x00000000
};


typedef struct {
	u32 nFlags;							// Flags (see SMOKETRAIL_FLAG_* for info)
	FTexDef_t *pTexDef;					// The texture to use

	f32 fScaleMin_WS;					// Initial scale range
	f32 fScaleMax_WS;
	f32 fScaleSpeedMin_WS;				// Initial scale speed range
	f32 fScaleSpeedMax_WS;
	f32 fScaleAccelMin_WS;				// Scale acceleration
	f32 fScaleAccelMax_WS;

	f32 fXRandSpread_WS;				// +/- amount the initial horizontal coordinate can vary
	f32 fYRandSpread_WS;				// +/- amount the initial vertical coordinate can vary
	f32 fDistBetweenPuffs_WS;			// Distance between puffs
	f32 fDistBetweenPuffsRandSpread_WS;	// +/- amount the distance between puffs can vary

	f32 fYSpeedMin_WS;					// Initial Y speed
	f32 fYSpeedMax_WS;
	f32 fYAccelMin_WS;					// Y acceleration
	f32 fYAccelMax_WS;

	f32 fUnitOpaqueMin_WS;				// Initial unit opaqueness range
	f32 fUnitOpaqueMax_WS;
	f32 fUnitOpaqueSpeedMin_WS;			// Initial unit opaqueness speed
	f32 fUnitOpaqueSpeedMax_WS;
	f32 fUnitOpaqueAccelMin_WS;			// Unit opaqueness acceleration
	f32 fUnitOpaqueAccelMax_WS;

	CFColorRGB StartColorRGB;			// Overall starting color bias of puffs
	CFColorRGB EndColorRGB;				// Overall ending color bias of puffs
	f32 fStartColorUnitIntensityMin;	// Unit intensity range applied to start color
	f32 fStartColorUnitIntensityMax;
	f32 fEndColorUnitIntensityMin;		// Unit intensity range applied to end color
	f32 fEndColorUnitIntensityMax;

	f32 fColorUnitSliderSpeedMin;		// Unit slider speed range
	f32 fColorUnitSliderSpeedMax;
	f32 fColorUnitSliderAccelMin;		// Unit slider acceleration range
	f32 fColorUnitSliderAccelMax;
} SmokeTrailAttrib_t;



extern BOOL smoketrail_InitSystem( void );
extern void smoketrail_UninitSystem( void );
extern BOOL smoketrail_IsSystemInitialized( void );

extern BOOL smoketrail_Allocate( u32 nSmokeTrailCount, u32 nMaxPuffCount );

extern SmokeTrailHandle_t smoketrail_GetFromFreePool( BOOL bPostDraw=FALSE );
extern SmokeTrailHandle_t smoketrail_GetFromFreePoolAndSetAttributes( SmokeTrailAttrib_t *pAttrib, BOOL bPostDraw=FALSE );
extern void smoketrail_ReturnToFreePool( SmokeTrailHandle_t hSmokeTrail, BOOL bWaitForNoMorePuffs );
extern void smoketrail_ReturnAllToFreePoolNow( void );

extern void smoketrail_SetAttributes( SmokeTrailHandle_t hSmokeTrail, SmokeTrailAttrib_t *pAttrib );
extern void smoketrail_Puff( SmokeTrailHandle_t hSmokeTrail, const CFVec3A *pPos_WS );

extern BOOL smoketrail_SpawnMuzzlePuff( SmokeTrailAttrib_t *pSmokeTrailAttrib, const CFVec3A *pStartPos_WS, const CFVec3A *pUnitDir_WS, f32 fLength );

extern void smoketrail_WorkAll( void );
extern void smoketrail_PostDrawAll( void );


#endif

