//////////////////////////////////////////////////////////////////////////////////////
// tracer.h - Tracer object module.
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
// 08/14/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _TRACER_H_
#define _TRACER_H_ 1

#include "fang.h"
#include "fdraw.h"
#include "ftex.h"
#include "fworld.h"
#include "fmath.h"
#include "fcolor.h"
#include "fcoll.h"
#include "damage.h"


typedef void *TracerGroupHandle_t;
#define TRACER_NULLGROUPHANDLE	((TracerGroupHandle_t)-1)


typedef enum {
	TRACER_KILLREASON_REACHED_MAX_DIST,		// Killed because tracer reached its maximum distance (TracerDef_t::fMaxTailDist_WS)
	TRACER_KILLREASON_HIT_GEO,				// Killed because tracer hit terrain or object geometry

	TRACER_KILLREASON_COUNT
} TracerKillReason_e;


typedef struct _TracerDef_s TracerDef_t;


// pTracerDef = tracer to be killed.
// nKillReason = reason it will be killed.
// pImpact = collision impact when reason has indicated TRACER_KILLREASON_HIT_GEO (NULL if not).
//           (pImpact->pTag will either be NULL for terrain or a pointer to the CFWorldMesh that it hit)
typedef void TracerKillCallback_t( TracerDef_t *pTracerDef, TracerKillReason_e nKillReason, const FCollImpact_t *pImpact );

typedef void BuildTrackerSkipList_t( void *pUser );

enum
{
	TRACERFLAG_NONE						= 0x00,
	TRACERFLAG_USE_ALPHA_FADES			= 0x01,
	TRACERFLAG_USE_GLOBAL_MOVEMENTCB	= 0x02,
	TRACERFLAG_DRAW_ADDITIVE			= 0x04,
	TRACERFLAG_THICK_PROJECTILE			= 0x08,		// Set to TRUE to collide with materials flagged for thick projectile collision
};

FCLASS_ALIGN_PREFIX struct _TracerDef_s {
	void *pUser;									// User-specified field
	u16 nUser;										// User-specified field
	u8 uFlags;										// See TRACERFLAG_* for info
	TracerKillCallback_t	*pFcnKillCallback;		// Called when tracer is killed (NULL=none)
	BuildTrackerSkipList_t	*pFcnBuildSkipList;		// called each frame before collision detection so whoever launched this tracer has a chance to do the skiplist thing

	CFColorRGBA ColorRGBA;							// Diffuse color and opaqueness of tracer
	CFVec3A UnitDir_WS;								// Unit direction the tracer is traveling in world space
	CFVec3A TailPos_WS;								// Tracer's tail position in world space
	f32 fWidth_WS;									// Width of tracer in world space
	f32 fLength_WS;									// Length of tracer in world space
	f32 fSpeed_WS;									// Speed in world space
	f32 fMaxTailDist_WS;							// Maximum distance the tail can move before the tracer is killed
	f32 fBeginDeathUnitFade_WS;						// Unit distance at which to begin fading to transparent
	f32 fAlphaFadeStartTime;						// How long until overall alpha will start to fade out
	f32 fAlphaFadeTime;								// How long overall alpha will take to fade out

	CDamageForm::Damager_t Damager;					// Carries along damager information


	FCLASS_STACKMEM_ALIGN( _TracerDef_s );
} FCLASS_ALIGN_SUFFIX;

#define TRACER_STILL_ALIVE 0
#define TRACER_DIED 1
typedef void GlobalTracerMoveCB(void* pUserData, const CFVec3A& NewPos_WS, BOOL bDied);


extern BOOL tracer_InitSystem( void );
extern void tracer_UninitSystem( void );

extern BOOL tracer_IsSystemInitialized( void );
extern u32 tracer_GetGroupCount( void );

extern TracerGroupHandle_t tracer_CreateGroup( CFTexInst *pTexInst, u32 nMaxTracerCount );
extern void tracer_DestroyGroup( TracerGroupHandle_t hGroup );

extern void tracer_SetNewTexture( TracerGroupHandle_t hGroup, CFTexInst *pTexInst );

extern u32 tracer_GetMaxTracerCount( TracerGroupHandle_t hGroup );
extern u32 tracer_GetActiveTracerCount( TracerGroupHandle_t hGroup );
extern u32 tracer_GetFreeTracerCount( TracerGroupHandle_t hGroup );

extern BOOL tracer_NewTracer( TracerGroupHandle_t hGroup, const TracerDef_t *pTracerDef, f32 fHeadDistanceOut = 0.f, BOOL bUseFadeTail = FALSE, BOOL bImmediateAbsorb = FALSE, const CDamageForm::Damager_t *pDamager=NULL );
extern void tracer_AddExtraParamsToLastTracer(GlobalTracerMoveCB* pMoveCB, void* pMoveCBData);

extern void tracer_DestroyAllTracers( void );
extern void tracer_DestroyAllTracers( TracerGroupHandle_t hGroup );


extern void tracer_Work( void );
extern void tracer_Draw( void );


#endif

