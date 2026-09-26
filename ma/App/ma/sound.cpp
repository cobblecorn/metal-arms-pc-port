//////////////////////////////////////////////////////////////////////////////////////
// sound.cpp - General purpose sound functions.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 03/2/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fsound.h"
#include "ai/aienviro.h"
#include "fclib.h"


static BOOL _bSystemInitialized;


static u8 _ConvertAICallback( cchar *pszString );
static void _NotifyAICallback( const CFVec3A *pPos_WS, s32 nPlayerIndex, u32 nSoundType, f32 fSoundRadius );


// Note that sound_LoadSoundGroups() gets called BEFORE sound_InitSystem().
BOOL sound_LoadSoundGroups( void ) {
	return CFSoundGroup::BuildGroupsFromFile( "sounds", _NotifyAICallback, _ConvertAICallback );
}


BOOL sound_InitSystem( void ) {
	FASSERT( !_bSystemInitialized );

	_bSystemInitialized = TRUE;

	if( !sound_LoadSoundGroups() ) {
		_bSystemInitialized = FALSE;
		return FALSE;
	}

	return TRUE;
}


void sound_UninitSystem( void ) {
	if( _bSystemInitialized ) {
		_bSystemInitialized = FALSE;
	}
}


static u8 _ConvertAICallback( cchar *pszString ) {
	static const struct { cchar *pszType; u8 nType; } __aTypeConverTable[] = {
		"None",		AISOUNDTYPE_COUNT,
		"Bot",		AISOUNDTYPE_BOT,
		"Bounce",	AISOUNDTYPE_GRENADE,
		"Explode",	AISOUNDTYPE_EXPLOSION,
		"Impact",	AISOUNDTYPE_WEAPON,

		NULL,		NULL
	};

	u32 i;

	for( i=0; __aTypeConverTable[i].pszType; ++i ) {
		if( !fclib_stricmp( pszString, __aTypeConverTable[i].pszType ) ) {
			// Strings match...
			return __aTypeConverTable[i].nType;
		}
	}

	// Could not find string...

	DEVPRINTF( "Sound::_ConvertAICallback(): Unrecognized AI sound type string '%s'.\n", pszString );

	return AISOUNDTYPE_COUNT;
}


static void _NotifyAICallback( const CFVec3A *pPos_WS, s32 nPlayerIndex, u32 nSoundType, f32 fSoundRadius ) {
	if( (nPlayerIndex >= 0) && (nSoundType == AISOUNDTYPE_BOT) ) {
		// Use optimized call...

		AIEnviro_BoostPlayerSoundTo( nPlayerIndex, fSoundRadius );

		return;
	}

	if( pPos_WS ) {
		if( nSoundType != AISOUNDTYPE_COUNT ) {
			AIEnviro_AddSound( *pPos_WS, fSoundRadius, 0.3f, nSoundType, AISOUNDCTRL_NONE );
		}
	}
}


