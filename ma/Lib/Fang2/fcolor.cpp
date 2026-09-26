//////////////////////////////////////////////////////////////////////////////////////
// fcolor.cpp - Fang color module.
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
// 10/26/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fcolor.h"
#include "fres.h"
#include "fvid.h"
#include "floop.h"
#include "fclib.h"


#define _MOTIF_WORK_DELTA_SECS	(1.0f / 60.0f)


CFColorMotifEntry FColor_aMotifTable[FCOLORMOTIF_COUNT];

CFColorMotif FColor_MotifBlack;
CFColorMotif FColor_MotifWhite;
CFColorMotif FColor_MotifRed;
CFColorMotif FColor_MotifGreen;
CFColorMotif FColor_MotifBlue;



#define _FLAME_HISTORY_FRAMES		(60*4)
#define _FLICKER_HISTORY_FRAMES		(60*4)
#define _LIGHTNING_HISTORY_FRAMES	(30)
#define _PULSATELOW_HISTORY_FRAMES	(240)
#define _PULSATEMED_HISTORY_FRAMES	(120)
#define _PULSATEHI_HISTORY_FRAMES	(60)
#define _ROCKET_HISTORY_FRAMES		(30)

#define _FLAME1_HISTORY_OFFSET		(_FLAME_HISTORY_FRAMES*1/8)
#define _FLAME2_HISTORY_OFFSET		(_FLAME_HISTORY_FRAMES*2/8)
#define _FLAME3_HISTORY_OFFSET		(_FLAME_HISTORY_FRAMES*3/8)
#define _FLAME4_HISTORY_OFFSET		(_FLAME_HISTORY_FRAMES*4/8)
#define _FLAME5_HISTORY_OFFSET		(_FLAME_HISTORY_FRAMES*5/8)
#define _FLAME6_HISTORY_OFFSET		(_FLAME_HISTORY_FRAMES*6/8)
#define _FLAME7_HISTORY_OFFSET		(_FLAME_HISTORY_FRAMES*7/8)

#define _FLICKERON1_HISTORY_OFFSET	(_FLICKER_HISTORY_FRAMES*1/8)
#define _FLICKERON2_HISTORY_OFFSET	(_FLICKER_HISTORY_FRAMES*2/8)
#define _FLICKERON3_HISTORY_OFFSET	(_FLICKER_HISTORY_FRAMES*3/8)
#define _FLICKERON4_HISTORY_OFFSET	(_FLICKER_HISTORY_FRAMES*4/8)
#define _FLICKERON5_HISTORY_OFFSET	(_FLICKER_HISTORY_FRAMES*5/8)
#define _FLICKERON6_HISTORY_OFFSET	(_FLICKER_HISTORY_FRAMES*6/8)
#define _FLICKERON7_HISTORY_OFFSET	(_FLICKER_HISTORY_FRAMES*7/8)

#define _FLICKEROFF1_HISTORY_OFFSET	(_FLICKER_HISTORY_FRAMES*1/8)
#define _FLICKEROFF2_HISTORY_OFFSET	(_FLICKER_HISTORY_FRAMES*2/8)
#define _FLICKEROFF3_HISTORY_OFFSET	(_FLICKER_HISTORY_FRAMES*3/8)
#define _FLICKEROFF4_HISTORY_OFFSET	(_FLICKER_HISTORY_FRAMES*4/8)
#define _FLICKEROFF5_HISTORY_OFFSET	(_FLICKER_HISTORY_FRAMES*5/8)
#define _FLICKEROFF6_HISTORY_OFFSET	(_FLICKER_HISTORY_FRAMES*6/8)
#define _FLICKEROFF7_HISTORY_OFFSET	(_FLICKER_HISTORY_FRAMES*7/8)

#define _LIGHTNING1_HISTORY_OFFSET	(_LIGHTNING_HISTORY_FRAMES*1/8)
#define _LIGHTNING2_HISTORY_OFFSET	(_LIGHTNING_HISTORY_FRAMES*2/8)
#define _LIGHTNING3_HISTORY_OFFSET	(_LIGHTNING_HISTORY_FRAMES*3/8)
#define _LIGHTNING4_HISTORY_OFFSET	(_LIGHTNING_HISTORY_FRAMES*4/8)
#define _LIGHTNING5_HISTORY_OFFSET	(_LIGHTNING_HISTORY_FRAMES*5/8)
#define _LIGHTNING6_HISTORY_OFFSET	(_LIGHTNING_HISTORY_FRAMES*6/8)
#define _LIGHTNING7_HISTORY_OFFSET	(_LIGHTNING_HISTORY_FRAMES*7/8)

#define _PULSATELOW1_HISTORY_OFFSET	(_PULSATELOW_HISTORY_FRAMES*1/8)
#define _PULSATELOW2_HISTORY_OFFSET	(_PULSATELOW_HISTORY_FRAMES*2/8)
#define _PULSATELOW3_HISTORY_OFFSET	(_PULSATELOW_HISTORY_FRAMES*3/8)
#define _PULSATELOW4_HISTORY_OFFSET	(_PULSATELOW_HISTORY_FRAMES*4/8)
#define _PULSATELOW5_HISTORY_OFFSET	(_PULSATELOW_HISTORY_FRAMES*5/8)
#define _PULSATELOW6_HISTORY_OFFSET	(_PULSATELOW_HISTORY_FRAMES*6/8)
#define _PULSATELOW7_HISTORY_OFFSET	(_PULSATELOW_HISTORY_FRAMES*7/8)

#define _PULSATEMED1_HISTORY_OFFSET	(_PULSATEMED_HISTORY_FRAMES*1/8)
#define _PULSATEMED2_HISTORY_OFFSET	(_PULSATEMED_HISTORY_FRAMES*2/8)
#define _PULSATEMED3_HISTORY_OFFSET	(_PULSATEMED_HISTORY_FRAMES*3/8)
#define _PULSATEMED4_HISTORY_OFFSET	(_PULSATEMED_HISTORY_FRAMES*4/8)
#define _PULSATEMED5_HISTORY_OFFSET	(_PULSATEMED_HISTORY_FRAMES*5/8)
#define _PULSATEMED6_HISTORY_OFFSET	(_PULSATEMED_HISTORY_FRAMES*6/8)
#define _PULSATEMED7_HISTORY_OFFSET	(_PULSATEMED_HISTORY_FRAMES*7/8)

#define _PULSATEHI1_HISTORY_OFFSET	(_PULSATEHI_HISTORY_FRAMES*1/8)
#define _PULSATEHI2_HISTORY_OFFSET	(_PULSATEHI_HISTORY_FRAMES*2/8)
#define _PULSATEHI3_HISTORY_OFFSET	(_PULSATEHI_HISTORY_FRAMES*3/8)
#define _PULSATEHI4_HISTORY_OFFSET	(_PULSATEHI_HISTORY_FRAMES*4/8)
#define _PULSATEHI5_HISTORY_OFFSET	(_PULSATEHI_HISTORY_FRAMES*5/8)
#define _PULSATEHI6_HISTORY_OFFSET	(_PULSATEHI_HISTORY_FRAMES*6/8)
#define _PULSATEHI7_HISTORY_OFFSET	(_PULSATEHI_HISTORY_FRAMES*7/8)

#define _ROCKET1_HISTORY_OFFSET		(_ROCKET_HISTORY_FRAMES*1/8)
#define _ROCKET2_HISTORY_OFFSET		(_ROCKET_HISTORY_FRAMES*2/8)
#define _ROCKET3_HISTORY_OFFSET		(_ROCKET_HISTORY_FRAMES*3/8)
#define _ROCKET4_HISTORY_OFFSET		(_ROCKET_HISTORY_FRAMES*4/8)
#define _ROCKET5_HISTORY_OFFSET		(_ROCKET_HISTORY_FRAMES*5/8)
#define _ROCKET6_HISTORY_OFFSET		(_ROCKET_HISTORY_FRAMES*6/8)
#define _ROCKET7_HISTORY_OFFSET		(_ROCKET_HISTORY_FRAMES*7/8)




typedef enum {
	_FLICKERTYPE_FLICKER_ON,
	_FLICKERTYPE_FLICKER_OFF,

	_FLICKERTYPE_COUNT
} _FlickerType_t;

typedef enum {
	_FLICKERSTAGE_IDLE,
	_FLICKERSTAGE_ACTIVE,

	_FLICKERSTAGE_COUNT
} _FlickerStage_t;


typedef void _MotifBuildFcn_t( FColorMotif_e nRootMotif );

typedef struct {
	FColorMotif_e nRootMotif;		// The root motif where colors are sampled from (FCOLORMOTIF_COUNT=end)
	_MotifBuildFcn_t *pBuildFcn;	// Pointer which builds the root motif color
	CFColorRGBA *pHistoryTable;		// Pointer to the history table
	u32 nHistoryFrames;				// Number of frames in the history table
	u32 nNextFrame;					// Next history table entry the color will be stored into
} _MotifVariety_t;

typedef struct {
	FColorMotif_e nMotifIndex;		// FCOLORMOTIF_COUNT: end of list
	u32 nHistoryOffset;
	_MotifVariety_t *pMotifVariety;
} _MotifHistory_t;


static BOOL _bModuleInitialized;
static f32 _fSecsUntilNextWork;

static int _nLightningInterval, _nLightningCounter;
static float _fLightningAttenuation, _fLightningIntensity;

static _FlickerStage_t _anFlickerStage[_FLICKERTYPE_COUNT];
static u32 _anFlickerIdleCount[_FLICKERTYPE_COUNT];
static u32 _anFlickerFlashCount[_FLICKERTYPE_COUNT];

static BOOL _bRocketSawtoothIncreasing;
static f32 _fRocketUnitSawtoothWave;
static f32 _fRocketUnitMinVal;
static f32 _fRocketUnitMaxVal;

static CFColorRGBA _aFlameHistoryTable[_FLAME_HISTORY_FRAMES];
static CFColorRGBA _aFlickerOnHistoryTable[_FLICKER_HISTORY_FRAMES];
static CFColorRGBA _aFlickerOffHistoryTable[_FLICKER_HISTORY_FRAMES];
static CFColorRGBA _aLightningHistoryTable[_LIGHTNING_HISTORY_FRAMES];
static CFColorRGBA _aPulsateLowHistoryTable[_PULSATELOW_HISTORY_FRAMES];
static CFColorRGBA _aPulsateMedHistoryTable[_PULSATEMED_HISTORY_FRAMES];
static CFColorRGBA _aPulsateHiHistoryTable[_PULSATEHI_HISTORY_FRAMES];
static CFColorRGBA _aRocketHistoryTable[_ROCKET_HISTORY_FRAMES];



static void _MotifWork( void );
static void _InitMotifTable( void );
static void _InitMotifs( void );
static float _GetFlickerIntensity( _FlickerType_t nFlickerType );
static void _InitMotifs( void );
static void _GenerateMotif_Flame( FColorMotif_e nRootMotif );
static void _GenerateMotif_FlickerOn( FColorMotif_e nRootMotif );
static void _GenerateMotif_FlickerOff( FColorMotif_e nRootMotif );
static float _GetFlickerIntensity( _FlickerType_t nFlickerType );
static void _GenerateMotif_Lightning( FColorMotif_e nRootMotif );
static void _GenerateMotif_PulsateLow( FColorMotif_e nRootMotif );
static void _GenerateMotif_PulsateMed( FColorMotif_e nRootMotif );
static void _GenerateMotif_PulsateHi( FColorMotif_e nRootMotif );
static void _GenerateMotif_Rocket( FColorMotif_e nRootMotif );



static _MotifVariety_t _aMotifVariety[] = {
	FCOLORMOTIF_FLAME0,			_GenerateMotif_Flame,			_aFlameHistoryTable,		_FLAME_HISTORY_FRAMES,			0,
	FCOLORMOTIF_FLICKERON0,		_GenerateMotif_FlickerOn,		_aFlickerOnHistoryTable,	_FLICKER_HISTORY_FRAMES,		0,
	FCOLORMOTIF_FLICKEROFF0,	_GenerateMotif_FlickerOff,		_aFlickerOffHistoryTable,	_FLICKER_HISTORY_FRAMES,		0,
	FCOLORMOTIF_LIGHTNING0,		_GenerateMotif_Lightning,		_aLightningHistoryTable,	_LIGHTNING_HISTORY_FRAMES,		0,
	FCOLORMOTIF_PULSATELOW0,	_GenerateMotif_PulsateLow,		_aPulsateLowHistoryTable,	_PULSATELOW_HISTORY_FRAMES,		0,
	FCOLORMOTIF_PULSATEMED0,	_GenerateMotif_PulsateMed,		_aPulsateMedHistoryTable,	_PULSATEMED_HISTORY_FRAMES,		0,
	FCOLORMOTIF_PULSATEHI0,		_GenerateMotif_PulsateHi,		_aPulsateHiHistoryTable,	_PULSATEHI_HISTORY_FRAMES,		0,
	FCOLORMOTIF_ROCKET0,		_GenerateMotif_Rocket,			_aRocketHistoryTable,		_ROCKET_HISTORY_FRAMES,			0,

	FCOLORMOTIF_COUNT
};

static _MotifHistory_t _aMotifHistory[] = {
	FCOLORMOTIF_FLAME1,			_FLAME1_HISTORY_OFFSET,			&_aMotifVariety[0],
	FCOLORMOTIF_FLAME2,			_FLAME2_HISTORY_OFFSET,			&_aMotifVariety[0],
	FCOLORMOTIF_FLAME3,			_FLAME3_HISTORY_OFFSET,			&_aMotifVariety[0],
	FCOLORMOTIF_FLAME4,			_FLAME4_HISTORY_OFFSET,			&_aMotifVariety[0],
	FCOLORMOTIF_FLAME5,			_FLAME5_HISTORY_OFFSET,			&_aMotifVariety[0],
	FCOLORMOTIF_FLAME6,			_FLAME6_HISTORY_OFFSET,			&_aMotifVariety[0],
	FCOLORMOTIF_FLAME7,			_FLAME7_HISTORY_OFFSET,			&_aMotifVariety[0],

	FCOLORMOTIF_FLICKERON1,		_FLICKERON1_HISTORY_OFFSET,		&_aMotifVariety[1],
	FCOLORMOTIF_FLICKERON2,		_FLICKERON2_HISTORY_OFFSET,		&_aMotifVariety[1],
	FCOLORMOTIF_FLICKERON3,		_FLICKERON3_HISTORY_OFFSET,		&_aMotifVariety[1],
	FCOLORMOTIF_FLICKERON4,		_FLICKERON4_HISTORY_OFFSET,		&_aMotifVariety[1],
	FCOLORMOTIF_FLICKERON5,		_FLICKERON5_HISTORY_OFFSET,		&_aMotifVariety[1],
	FCOLORMOTIF_FLICKERON6,		_FLICKERON6_HISTORY_OFFSET,		&_aMotifVariety[1],
	FCOLORMOTIF_FLICKERON7,		_FLICKERON7_HISTORY_OFFSET,		&_aMotifVariety[1],

	FCOLORMOTIF_FLICKEROFF1,	_FLICKEROFF1_HISTORY_OFFSET,	&_aMotifVariety[2],
	FCOLORMOTIF_FLICKEROFF2,	_FLICKEROFF2_HISTORY_OFFSET,	&_aMotifVariety[2],
	FCOLORMOTIF_FLICKEROFF3,	_FLICKEROFF3_HISTORY_OFFSET,	&_aMotifVariety[2],
	FCOLORMOTIF_FLICKEROFF4,	_FLICKEROFF4_HISTORY_OFFSET,	&_aMotifVariety[2],
	FCOLORMOTIF_FLICKEROFF5,	_FLICKEROFF5_HISTORY_OFFSET,	&_aMotifVariety[2],
	FCOLORMOTIF_FLICKEROFF6,	_FLICKEROFF6_HISTORY_OFFSET,	&_aMotifVariety[2],
	FCOLORMOTIF_FLICKEROFF7,	_FLICKEROFF7_HISTORY_OFFSET,	&_aMotifVariety[2],

	FCOLORMOTIF_LIGHTNING1,		_LIGHTNING1_HISTORY_OFFSET,		&_aMotifVariety[3],
	FCOLORMOTIF_LIGHTNING2,		_LIGHTNING2_HISTORY_OFFSET,		&_aMotifVariety[3],
	FCOLORMOTIF_LIGHTNING3,		_LIGHTNING3_HISTORY_OFFSET,		&_aMotifVariety[3],
	FCOLORMOTIF_LIGHTNING4,		_LIGHTNING4_HISTORY_OFFSET,		&_aMotifVariety[3],
	FCOLORMOTIF_LIGHTNING5,		_LIGHTNING5_HISTORY_OFFSET,		&_aMotifVariety[3],
	FCOLORMOTIF_LIGHTNING6,		_LIGHTNING6_HISTORY_OFFSET,		&_aMotifVariety[3],
	FCOLORMOTIF_LIGHTNING7,		_LIGHTNING7_HISTORY_OFFSET,		&_aMotifVariety[3],

	FCOLORMOTIF_PULSATELOW1,	_PULSATELOW1_HISTORY_OFFSET,	&_aMotifVariety[4],
	FCOLORMOTIF_PULSATELOW2,	_PULSATELOW2_HISTORY_OFFSET,	&_aMotifVariety[4],
	FCOLORMOTIF_PULSATELOW3,	_PULSATELOW3_HISTORY_OFFSET,	&_aMotifVariety[4],
	FCOLORMOTIF_PULSATELOW4,	_PULSATELOW4_HISTORY_OFFSET,	&_aMotifVariety[4],
	FCOLORMOTIF_PULSATELOW5,	_PULSATELOW5_HISTORY_OFFSET,	&_aMotifVariety[4],
	FCOLORMOTIF_PULSATELOW6,	_PULSATELOW6_HISTORY_OFFSET,	&_aMotifVariety[4],
	FCOLORMOTIF_PULSATELOW7,	_PULSATELOW7_HISTORY_OFFSET,	&_aMotifVariety[4],

	FCOLORMOTIF_PULSATEMED1,	_PULSATEMED1_HISTORY_OFFSET,	&_aMotifVariety[5],
	FCOLORMOTIF_PULSATEMED2,	_PULSATEMED2_HISTORY_OFFSET,	&_aMotifVariety[5],
	FCOLORMOTIF_PULSATEMED3,	_PULSATEMED3_HISTORY_OFFSET,	&_aMotifVariety[5],
	FCOLORMOTIF_PULSATEMED4,	_PULSATEMED4_HISTORY_OFFSET,	&_aMotifVariety[5],
	FCOLORMOTIF_PULSATEMED5,	_PULSATEMED5_HISTORY_OFFSET,	&_aMotifVariety[5],
	FCOLORMOTIF_PULSATEMED6,	_PULSATEMED6_HISTORY_OFFSET,	&_aMotifVariety[5],
	FCOLORMOTIF_PULSATEMED7,	_PULSATEMED7_HISTORY_OFFSET,	&_aMotifVariety[5],

	FCOLORMOTIF_PULSATEHI1,		_PULSATEHI1_HISTORY_OFFSET,		&_aMotifVariety[6],
	FCOLORMOTIF_PULSATEHI2,		_PULSATEHI2_HISTORY_OFFSET,		&_aMotifVariety[6],
	FCOLORMOTIF_PULSATEHI3,		_PULSATEHI3_HISTORY_OFFSET,		&_aMotifVariety[6],
	FCOLORMOTIF_PULSATEHI4,		_PULSATEHI4_HISTORY_OFFSET,		&_aMotifVariety[6],
	FCOLORMOTIF_PULSATEHI5,		_PULSATEHI5_HISTORY_OFFSET,		&_aMotifVariety[6],
	FCOLORMOTIF_PULSATEHI6,		_PULSATEHI6_HISTORY_OFFSET,		&_aMotifVariety[6],
	FCOLORMOTIF_PULSATEHI7,		_PULSATEHI7_HISTORY_OFFSET,		&_aMotifVariety[6],

	FCOLORMOTIF_ROCKET1,		_ROCKET1_HISTORY_OFFSET,		&_aMotifVariety[7],
	FCOLORMOTIF_ROCKET2,		_ROCKET2_HISTORY_OFFSET,		&_aMotifVariety[7],
	FCOLORMOTIF_ROCKET3,		_ROCKET3_HISTORY_OFFSET,		&_aMotifVariety[7],
	FCOLORMOTIF_ROCKET4,		_ROCKET4_HISTORY_OFFSET,		&_aMotifVariety[7],
	FCOLORMOTIF_ROCKET5,		_ROCKET5_HISTORY_OFFSET,		&_aMotifVariety[7],
	FCOLORMOTIF_ROCKET6,		_ROCKET6_HISTORY_OFFSET,		&_aMotifVariety[7],
	FCOLORMOTIF_ROCKET7,		_ROCKET7_HISTORY_OFFSET,		&_aMotifVariety[7],

	FCOLORMOTIF_COUNT
};




BOOL fcolor_ModuleStartup( void ) {
	u32 i;

	FASSERT( !_bModuleInitialized );

	_fSecsUntilNextWork = 0.0f;

	_InitMotifTable();

	for( i=0; i<_FLICKERTYPE_COUNT; i++ ) {
		_anFlickerStage[i] = _FLICKERSTAGE_IDLE;
		_anFlickerIdleCount[i] = 1;
		_anFlickerFlashCount[i] = 1;
	}

	_nLightningInterval = 30;
	_nLightningCounter = 0;
	_fLightningAttenuation = 0.0f;
	_fLightningIntensity = 1.0f;

	_bRocketSawtoothIncreasing = TRUE;
	_fRocketUnitSawtoothWave = 0.0f;
	_fRocketUnitMinVal = 0.5f;
	_fRocketUnitMaxVal = 1.0f;

	FColor_aMotifTable[0].ColorRGBA.OpaqueWhite();

	FColor_MotifBlack.OpaqueBlack();
	FColor_MotifWhite.OpaqueWhite();
	FColor_MotifRed.OpaqueRed();
	FColor_MotifGreen.OpaqueGreen();
	FColor_MotifBlue.OpaqueBlue();

	_bModuleInitialized = TRUE;

	return TRUE;
}

void fcolor_ModuleShutdown( void ) {
	FASSERT( _bModuleInitialized );

	_bModuleInitialized = FALSE;
}

u32 fcolor_ConvertStringToBaseMotif( cchar *pszMotifName ) {
	static const struct { cchar *pszMotifName; FColorMotif_e nMotifBaseIndex; } __aMotifConvertTable[] = {
		"Constant",		FCOLORMOTIF_CONSTANT,
		"None",			FCOLORMOTIF_CONSTANT,

		"Flame",		FCOLORMOTIF_FLAME0,
		"FlickerOn",	FCOLORMOTIF_FLICKERON0,
		"FlickerOff",	FCOLORMOTIF_FLICKEROFF0,
		"Lightning",	FCOLORMOTIF_LIGHTNING0,
		"PulsateLow",	FCOLORMOTIF_PULSATELOW0,
		"PulsateMed",	FCOLORMOTIF_PULSATEMED0,
		"PulsateHi",	FCOLORMOTIF_PULSATEHI0,
		"Rocket",		FCOLORMOTIF_ROCKET0,

		NULL,			FCOLORMOTIF_CONSTANT
	};

	u32 i;

	for( i=0; __aMotifConvertTable[i].pszMotifName; ++i ) {
		if( !fclib_stricmp( pszMotifName, __aMotifConvertTable[i].pszMotifName ) ) {
			return __aMotifConvertTable[i].nMotifBaseIndex;
		}
	}

	DEVPRINTF( "fcolor_ConvertStringToBaseMotif(): Unrecognized motif name provided '%s'.\n", pszMotifName );

	return FCOLORMOTIF_CONSTANT;
}

u32 fcolor_GetRandomMotif( u32 nBaseMotif ) {
	FASSERT( nBaseMotif < FCOLORMOTIF_COUNT );

	if( nBaseMotif == FCOLORMOTIF_CONSTANT ) {
		return FCOLORMOTIF_CONSTANT;
	}

	return nBaseMotif + fmath_RandomChoice( 8 );
}

void fcolor_GenerateMotifs( void ) {
	f32 fDeltaSecs;
	u32 nPass, nPassCount;

	fDeltaSecs = FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMAX( fDeltaSecs, (1.0f / 30.0f) );

	_fSecsUntilNextWork -= fDeltaSecs;
	if( _fSecsUntilNextWork > 0.0f ) {
		// Not time to do work yet...
		return;
	}

	// Let's do some work...

	nPassCount = 0;
	while( _fSecsUntilNextWork <= 0.0f ) {
		_fSecsUntilNextWork += _MOTIF_WORK_DELTA_SECS;
		++nPassCount;
	}

	for( nPass=0; nPass<nPassCount; ++nPass ) {
		_MotifWork();
	}
}

// Generates all static light motif color values.
// Gameloop should call this once per frame.
static void _MotifWork( void ) {
	_MotifVariety_t *pMotifVariety;
	_MotifHistory_t *pMotifHistory;
	u32 nHistoryIndex;

	FASSERT( _bModuleInitialized );

	// Generate root motif colors...
	for( pMotifVariety=_aMotifVariety; pMotifVariety->nRootMotif!=FCOLORMOTIF_COUNT; pMotifVariety++ ) {
		(pMotifVariety->pBuildFcn)( pMotifVariety->nRootMotif );

		// Store generated color in next slot of history table...
		pMotifVariety->pHistoryTable[ pMotifVariety->nNextFrame ] = FColor_aMotifTable[ pMotifVariety->nRootMotif ].ColorRGBA;

		// Bump history index up by one...
		if( ++pMotifVariety->nNextFrame >= pMotifVariety->nHistoryFrames ) {
			pMotifVariety->nNextFrame = 0;
		}
	}

	// Generate motif colors that are based on the root history...
	for( pMotifHistory=_aMotifHistory; pMotifHistory->nMotifIndex!=FCOLORMOTIF_COUNT; pMotifHistory++ ) {
		pMotifVariety = pMotifHistory->pMotifVariety;

		// Compute index into history table...
		nHistoryIndex = pMotifVariety->nNextFrame + pMotifHistory->nHistoryOffset;
		while( nHistoryIndex >= pMotifVariety->nHistoryFrames ) {
			nHistoryIndex -= pMotifVariety->nHistoryFrames;
		}

		// Store color into motif table...
		FColor_aMotifTable[ pMotifHistory->nMotifIndex ].ColorRGBA = pMotifVariety->pHistoryTable[nHistoryIndex];
	}
}

void fcolor_Motif_SetLightningIntensity( f32 fIntensity ) {
	FASSERT( _bModuleInitialized );
	_fLightningIntensity = fIntensity;
}

static void _InitMotifTable( void ) {
	u32 i;

	for( i=0; i<FCOLORMOTIF_COUNT; i++ ) {
		FColor_aMotifTable[i].ColorRGBA.OpaqueBlack();
	}

	_InitMotifs();
}

static void _InitMotifs( void ) {
	_MotifVariety_t *pMotifVariety;

	for( pMotifVariety=_aMotifVariety; pMotifVariety->nRootMotif!=FCOLORMOTIF_COUNT; pMotifVariety++ ) {
		pMotifVariety->nNextFrame = 0;
	}
}

static void _GenerateMotif_Flame( FColorMotif_e nRootMotif ) {
	f32 fIntensity, fFrame;

	#define FLAME_SPEED	6.0f
	#define FLAME_K1	(0.02960205078125f*FLAME_SPEED)
	#define FLAME_K2	(0.04364013671875f*FLAME_SPEED)
	#define FLAME_K3	(0.06195068359375f*FLAME_SPEED)
	#define FLAME_K4	(0.03387451171875f*FLAME_SPEED)
	#define FLAME_K5	(0.05401611328125f*FLAME_SPEED)
	#define FLAME_K6	(0.09124755859375f*FLAME_SPEED)

	fFrame = (f32)(FVid_nFrameCounter & 0xffff);

	fIntensity = (fmath_Sin(fFrame*FLAME_K1)+fmath_Sin(fFrame*FLAME_K2)
				+ fmath_Sin(fFrame*FLAME_K3)+fmath_Sin(fFrame*FLAME_K4)
				+ fmath_Sin(fFrame*FLAME_K5)+fmath_Sin(fFrame*FLAME_K6))*0.1f + 0.7f;

	FMATH_CLAMPMAX( fIntensity, 1.0f );

	FColor_aMotifTable[nRootMotif].ColorRGBA.Set( fIntensity, fIntensity*0.4f, 0.0f, fIntensity*0.7f );
}

static void _GenerateMotif_FlickerOn( FColorMotif_e nRootMotif ) {
	f32 fIntensity;

	fIntensity = _GetFlickerIntensity( _FLICKERTYPE_FLICKER_ON );
	FColor_aMotifTable[nRootMotif].ColorRGBA.Set( fIntensity, fIntensity );
}

static void _GenerateMotif_FlickerOff( FColorMotif_e nRootMotif ) {
	f32 fIntensity;

	fIntensity = _GetFlickerIntensity( _FLICKERTYPE_FLICKER_OFF );
	FColor_aMotifTable[nRootMotif].ColorRGBA.Set( fIntensity, fIntensity );
}

static float _GetFlickerIntensity( _FlickerType_t nFlickerType ) {
	f32 fIntensity;

	switch( _anFlickerStage[nFlickerType] ) {
	case _FLICKERSTAGE_IDLE:
		if( --_anFlickerIdleCount[nFlickerType] == 0 ) {
			_anFlickerStage[nFlickerType] = _FLICKERSTAGE_ACTIVE;
			_anFlickerFlashCount[nFlickerType] = fmath_RandomRange( 10, 50 );
		}

		switch( nFlickerType ) {
		case _FLICKERTYPE_FLICKER_ON:
			fIntensity = 0.15f;
			break;
		case _FLICKERTYPE_FLICKER_OFF:
			fIntensity = 1.0f;
			break;
		default:
			fIntensity = 0.0f;
			FASSERT_NOW;
			break;
		}
		break;

	case _FLICKERSTAGE_ACTIVE:
		if( --_anFlickerFlashCount[nFlickerType] == 0) {
			_anFlickerStage[nFlickerType] = _FLICKERSTAGE_IDLE;
			_anFlickerIdleCount[nFlickerType] = fmath_RandomRange( 10, 30 );
		}

		if( FVid_nFrameCounter & 2 ) {
			fIntensity = fmath_RandomFloat()*0.25f + 0.50f;
		} else {
			fIntensity = fmath_RandomFloat()*0.25f + 0.25f;
		}
		break;
	}

	return fIntensity;
}

static void _GenerateMotif_Lightning( FColorMotif_e nRootMotif ) {
	f32 fFrame, fIntensity;

	fFrame = (f32)((u32)((f32)FVid_nFrameCounter * (123456789.0f * 0.5f)) & 0xfff);
	fIntensity = 0.0f;

	if( _fLightningIntensity ) {
		if( !--_nLightningInterval ) {
			_nLightningInterval = fmath_RandomRange( 100, 300 );
			_nLightningCounter = fmath_RandomRange( 40, 60 );
			_fLightningAttenuation = 1.0f;
		}

		if( _nLightningCounter ) {
			_nLightningCounter--;
			fIntensity = (fmath_Sin(fFrame)*0.5f+0.5f) * (f32)(FVid_nFrameCounter&1)*_fLightningAttenuation * _fLightningIntensity;
			_fLightningAttenuation *= 0.97f;
		}
	}

	FColor_aMotifTable[nRootMotif].ColorRGBA.Set( fIntensity, fIntensity );
}

static void _GenerateMotif_PulsateLow( FColorMotif_e nRootMotif ) {
	f32 fIntensity;

	fIntensity = 0.5f + 0.5f*fmath_Sin( (f32)FVid_nFrameCounter * (FMATH_2PI / 240.0f) );
	FColor_aMotifTable[nRootMotif].ColorRGBA.Set( fIntensity, fIntensity, fIntensity, fIntensity );
}

static void _GenerateMotif_PulsateMed( FColorMotif_e nRootMotif ) {
	f32 fIntensity;

	fIntensity = 0.5f + 0.5f*fmath_Sin( (f32)FVid_nFrameCounter * (FMATH_2PI / 120.0f) );
	FColor_aMotifTable[nRootMotif].ColorRGBA.Set( fIntensity, fIntensity, fIntensity, fIntensity );
}

static void _GenerateMotif_PulsateHi( FColorMotif_e nRootMotif ) {
	f32 fIntensity;

	fIntensity = 0.5f + 0.5f*fmath_Sin( (f32)FVid_nFrameCounter * (FMATH_2PI / 60.0f) );
	FColor_aMotifTable[nRootMotif].ColorRGBA.Set( fIntensity, fIntensity, fIntensity, fIntensity );
}

static void _GenerateMotif_Rocket( FColorMotif_e nRootMotif ) {
	#define __ROCKET_FLICKER_SPEED	15.0f

	f32 fIntensity;

	if( _bRocketSawtoothIncreasing ) {
		_fRocketUnitSawtoothWave += _MOTIF_WORK_DELTA_SECS * __ROCKET_FLICKER_SPEED;

		if( _fRocketUnitSawtoothWave >= 1.0f ) {
			_fRocketUnitSawtoothWave = 1.0f;
			_bRocketSawtoothIncreasing = FALSE;

			_fRocketUnitMinVal = fmath_RandomFloatRange( 0.5f, 0.7f );
		}
	} else {
		_fRocketUnitSawtoothWave -= _MOTIF_WORK_DELTA_SECS * __ROCKET_FLICKER_SPEED;

		if( _fRocketUnitSawtoothWave < 0.0f ) {
			_fRocketUnitSawtoothWave = 0.0f;
			_bRocketSawtoothIncreasing = TRUE;

			_fRocketUnitMaxVal = fmath_RandomFloatRange( 0.8f, 1.0f );
		}
	}

	fIntensity = FMATH_FPOT( _fRocketUnitSawtoothWave, _fRocketUnitMinVal, _fRocketUnitMaxVal );
	FColor_aMotifTable[nRootMotif].ColorRGBA.Set( fIntensity, fIntensity, fIntensity, fIntensity );

	#undef __ROCKET_FLICKER_SPEED
}

