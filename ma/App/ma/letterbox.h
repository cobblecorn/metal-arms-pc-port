//////////////////////////////////////////////////////////////////////////////////////
// letterbox.h - Letterbox bars.
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
// 09/19/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _LETTERBOX_H_
#define _LETTERBOX_H_ 1

#include "fang.h"


typedef enum {
	LETTERBOX_STATE_MOTIONLESS,		// Bars are motionless
	LETTERBOX_STATE_SLIDING_ON,		// Bars are sliding onto the screen
	LETTERBOX_STATE_SLIDING_OFF,	// Bars are sliding off of the screen

	LETTERBOX_STATE_COUNT
} LetterboxState_e;


extern BOOL letterbox_InitSystem( void );
extern void letterbox_UninitSystem( void );

extern void letterbox_Work( void );
extern void letterbox_Draw( void );

extern void letterbox_SlideOn( BOOL bImmediate = FALSE ); // have the letterbox appear immediately as opposed to sliding on
extern void letterbox_SlideOff( BOOL bImmediate = FALSE ); // have the letterbox disappear immediately as opposed to sliding off
extern void letterbox_Reset( void );

extern f32 letterbox_GetUnitSlideOnAmount( void );
extern LetterboxState_e letterbox_GetState( void );

#if FANG_WINGC
// Local co-op (PC): a player revived after dying gets their HUD back (it went off at death); while
// the letterbox is up it comes back when the bars leave.
extern void letterbox_PortRestoreHud( s32 nPlayer );
#endif


#endif

