//////////////////////////////////////////////////////////////////////////////////////
// BarterSystem.h - 
//
// Author: Michael Starich   
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
// 10/15/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _BARTER_SYSTEM_H_
#define _BARTER_SYSTEM_H_ 1

#include "fang.h"

class CEntity;

#define BARTER_SYSTEM_GIVEN_SEE_PLAYER_DIST			(40.0f)
#define BARTER_SYSTEM_SEE_PLAYER_MAX_DIST			(270.0f)
#define BARTER_SYSTEM_ATTRACT_RADIUS				(BARTER_SYSTEM_SEE_PLAYER_MAX_DIST * 0.5f)
#define BARTER_SYSTEM_SPEAK_RADIUS					(BARTER_SYSTEM_ATTRACT_RADIUS)
#define BARTER_SYSTEM_ATTRACT_RADIUS_HYSTERISIS		(1.0f)
#define BARTER_SYSTEM_LEVELMUSIC_PAUSE_HYSTERISIS	(10.0f)


typedef enum {
	BARTER_SYSTEM_EVENTS_NONE = 0,
	BARTER_SYSTEM_EVENTS_SLIM_LOOK,
	BARTER_SYSTEM_EVENTS_SLIM_LOOK_AND_LAUGH,
	BARTER_SYSTEM_EVENTS_SLIM_LOOK_AND_NOD,
	BARTER_SYSTEM_EVENTS_SLIM_LOOK_AND_SHAKE,
	BARTER_SYSTEM_EVENTS_SLIM_SHADY_IS_WELCOMING,
	BARTER_SYSTEM_EVENTS_SHADY_LOOK_AT_SLIM,
	BARTER_SYSTEM_EVENTS_SLIM_WAVE,

	BARTER_SYSTEM_EVENTS_SLIM_COUNT
} BarterSystem_Events_e;

extern cchar *BarterSystem_apszEventStrings[BARTER_SYSTEM_EVENTS_SLIM_COUNT];

extern BOOL bartersystem_InitSystem();
extern void bartersystem_UninitSystem();

extern BOOL bartersystem_InitLevel( cchar *pszCSVFile );
extern void bartersystem_UninitLevel();

extern void bartersystem_Work();

// Returns TRUE if the barters are in the world and are actually bartering (not attract mode)
extern BOOL bartersystem_IsActive();

// Return TRUE if the barters are currently in the world somewhere, either in active or attract modes
extern BOOL bartersystem_AreInWorld();

// Forces the barters (if they are not currently Active) to appear at the point named pszPointName.
// Returns TRUE if the move was successful, FALSE if we couldn't find the point or the barters are currently Active.
extern BOOL bartersystem_PlaceInWorldAndStartAttracting( cchar *pszPointName );
extern void bartersystem_RemoveFromWorld( BOOL bFadeMusic );

// Call to force the barters (regardless of what they are currently doing)
// to hide, they will not come back till they are told to unhide
extern void bartersystem_Hide( BOOL bHide );

// Allow external notification into the barter system
extern void bartersystem_EventNotify( u32 nEvent, f32 fVal );

//Draw.  Currently only draws the censor for slim
extern void bartersystem_Draw( void );

// call after the player has been restored to a checkpoint location
extern void bartersystem_CheckpointSave( void );
extern void bartersystem_CheckpointRestore( void );

// MRS added fn to single pass the exit functionality
extern void bartersystem_AbortActiveMode(void);

extern BOOL bartersystem_SetAttractRadius(u32 uWhichTable, f32 fAttractMusicRadius, f32 fAttractSpeechRadius);

extern BOOL bartersystem_IsBarterBot(const CEntity* pEntity);

#endif
