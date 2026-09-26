//////////////////////////////////////////////////////////////////////////////////////
// DefFileIO.cpp - 
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
// 09/07/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "DefFileIO.h"

#define _MIN_PERCENT_TO_DRAW		0.01f

/////////////////////////////
// START OF VERSION 1 STRUCTS
struct _Ver1_KeyFrame_t {
		// how far away should particles be seen from
	f32 fCullDist;				// how far should the particles be seen from
	f32 fStartSkipDrawDist;		// how far should the particles be before the we start skipping the drawing of some portion of them
	f32 fMinPercentToDraw;		// even at the furthest visible distance, what percent of the particles need to be drawn
		// what type of light should be emitted, if at all
	CFColorRGB LightRGB;
	f32 fLightIntensity;
	f32 fLightRadiusMultiplier;	// what should the light radius when compared to the particle group bounding sphere
		// how many particles should be emitted, and how often
	CFVec2 NumPerBurst;
	f32 fSecsBetweenBursts;	// how often should this particle type emit an burst, if over time 
	f32 fSecsToEmit;			// how long should this particle type emit for
		// forces that act on the particles
	f32 fDragMultiplierPerSec;	// multiplied by velocity to simulate drag of airflow (just scales velocity vector down or up I suppose)
	f32 fGravityPerSec;			// can be + or -, but always applied to world space y of the velocity vector
		// when and how should the particles split on collision impact
	f32 fMinCollAlpha;			// only split if the alpha value of the particle is greater than this value
	f32 fMaxCollSplit;			// what is the max number of particles that should be spawned on impact
	f32 fUnitCollBounce;		// how much of the impact velocity should be passed on to the new particles
		// how many sprites make up each particle
	f32 fNumSprites;
	f32 fMaxVel2;
	f32 fAlphaStepPerSprite;
	f32 fSizeStepPerSprite;
	f32 fPosStepPerSprite;	
		// how big are the particles
	CFVec2 InitScale;
	CFVec2 FinalScale;
		// what color are the particles
	CFVec2 InitRed;
	CFVec2 InitGreen;
	CFVec2 InitBlue;	
	CFVec2 FinalRed;
	CFVec2 FinalGreen;
	CFVec2 FinalBlue;
		// what opacity are the particles
	CFVec2 InitAlpha;	
	CFVec2 FinalAlpha;	
		// 1.0f / (how long should each particles last for)
	CFVec2 OOLifeSecs;		// this is a min & max value, not min & delta value
		// how emissive should the particles be
	CFVec2 EmissiveIntensity;	
		// how should the initial velocity be computed
	CFVec2 InitDirAngle;	// in radians, the min & delta direction angle from the emission dir
	CFVec2 VelocityZ;		// how fast should the particles be traveling to start
		// where should the initial position be 
	f32 fRandomInitPosRadius;	// a radius about the emission pt to randomize the initial position of each particle
};
typedef enum {
	_VER1_ANIM_FUNC_TYPE_CONSTANT = 0,		// %out = fConstant
	_VER1_ANIM_FUNC_TYPE_LOOKUP_TABLE,		// %out = LERP of nearest values in table indexed by %age * _VER1_NUM_TABLE_ENTRIES
	_VER1_ANIM_FUNC_TYPE_LINEAR_TIME,		// %out = %age
	_VER1_ANIM_FUNC_TYPE_RANDOM,			// %out = random (fRandOutChance of 1.0f)
	_VER1_ANIM_FUNC_TYPE_TIME_2,			// %out = %age^2
	_VER1_ANIM_FUNC_TYPE_TIME_3,			// %out = %age^3
	
	_VER1_ANIM_FUNC_TYPE_COUNT
} _Ver1_AnimFuncType_e;
#define _VER1_NUM_TABLE_ENTRIES		25
struct _Ver1_Func_Constant_t {
	f32 fConstant;					// UNIT FLOAT
};
struct _Ver1_Func_Lookup_t {
	f32 afLookupTable[_VER1_NUM_TABLE_ENTRIES];	// UNIT FLOAT
};
struct _Ver1_Func_Linear_t {
	f32 fOOFuncRange;		// set to 1 / (fOutUnitPercent - fInUnitPercent)
};
struct _Ver1_Func_Random_t {
	f32 fRandOutChance;				// UNIT FLOAT
	f32 fRandomReNormalizer;		// set to 1.0f / (1.0f - fRandOutChance)
};
struct _Ver1_AnimFunc_t {
	f32 fInUnitPercent;		// UNIT FLOAT
	f32 fOutUnitPercent;	// UNIT FLOAT	
	_Ver1_AnimFuncType_e nFuncType;
	union {
		_Ver1_Func_Constant_t	Constant;		// only used if nFuncType == _VER1_ANIM_FUNC_TYPE_CONSTANT
		_Ver1_Func_Lookup_t		Lookup;			// only used if nFuncType == _VER1_ANIM_FUNC_TYPE_LOOKUP_TABLE
		_Ver1_Func_Linear_t		Linear;			// only used if nFuncType == _VER1_ANIM_FUNC_TYPE_LINEAR_TIME
		_Ver1_Func_Random_t		Random;			// only used if nFuncType == _VER1_ANIM_FUNC_TYPE_RANDOM
	};	
};
struct _Ver1_Def_t {
	u32 nVersion;				// what version is this data, see _VER1_FILE_VERSION
	u32 nFlags;					// see _VER1_DEF_FLAGS_... for description
	_Ver1_KeyFrame_t Min;	// use these values at intensity 0.0f
	_Ver1_KeyFrame_t Max;	// use these values at intensity 1.0f
	char szTextureName[FDATA_TEXNAME_LEN+1];
	FTexDef_t *pTexDef;		// the loaded texture
	_Ver1_AnimFunc_t AnimAlphaInfo;
	_Ver1_AnimFunc_t AnimScaleInfo;
	_Ver1_AnimFunc_t AnimColorInfo;
	u32 nLightMotifIndex;
	f32 fSecsBetweenLightSamples;	// how often should the world lights be sampled
	CFVec3 SampledLightScale;	// the sampled light is scaled by these values
	u32 nIndexInList;			// used to verify that handles are valid
	FLink_t Link;				// for insertion into a ParticleDef linklist
	FLinkRoot_t EmitterList;	// to keep a list of the emitters of this type currently in use
};
// END OF VERSION 1 STRUCTS
///////////////////////////


/////////////////////////////
// START OF VERSION 2 STRUCTS
typedef struct {
		// how far away should particles be seen from
	f32 fCullDist;				// how far should the particles be seen from
	f32 fStartSkipDrawDist;		// how far should the particles be before the we start skipping the drawing of some portion of them
	f32 fMinPercentToDraw;		// even at the furthest visible distance, what percent of the particles need to be drawn
		// what type of light should be emitted, if at all
	CFColorRGB LightRGB;
	f32 fLightIntensity;
	f32 fLightRadiusMultiplier;	// what should the light radius when compared to the particle group bounding sphere
		// how many particles should be emitted, and how often
	CFVec2 NumPerBurst;
	CFVec2 SecsBetweenBursts;	// how often should this particle type emit an burst, if over time (this is a min/max not a min/delta value)
	f32 fSecsToEmit;			// how long should this particle type emit for
		// forces that act on the particles
	f32 fDragMultiplierPerSec;	// multiplied by velocity to simulate drag of airflow (just scales velocity vector down or up I suppose)
	f32 fGravityPerSec;			// can be + or -, but always applied to world space y of the velocity vector
	CFVec3 MaxBubbleXYZ;		// how much +- should be added as a bubble force to the velocity
	f32 fBubbleUpdateChance;	// what chance does the bubble force have of being updated each frame
	f32 fBubbleDragPerSec;		// how much should the bubble force be dragged per sec
		// when and how should the particles split on collision impact
	f32 fMinCollAlpha;			// only split if the alpha value of the particle is greater than this value
	f32 fMaxCollSplit;			// what is the max number of particles that should be spawned on impact
	f32 fUnitCollBounce;		// how much of the impact velocity should be passed on to the new particles
		// how many sprites make up each particle
	f32 fNumSprites;
	f32 fMaxVel2;
	f32 fAlphaStepPerSprite;
	f32 fSizeStepPerSprite;
	f32 fPosStepPerSprite;	
		// how big are the particles
	CFVec2 InitScale;
	CFVec2 FinalScale;
		// what color are the particles
	CFVec2 InitRed;
	CFVec2 InitGreen;
	CFVec2 InitBlue;	
	CFVec2 FinalRed;
	CFVec2 FinalGreen;
	CFVec2 FinalBlue;
		// what opacity are the particles
	CFVec2 InitAlpha;	
	CFVec2 FinalAlpha;	
		// 1.0f / (how long should each particles last for)
	CFVec2 OOLifeSecs;		// this is a min & max value, not min & delta value
		// how emissive should the particles be
	CFVec2 EmissiveIntensity;	
		// how should the initial velocity be computed
	CFVec2 InitDirAngle;	// in radians, the min & delta direction angle from the emission dir
	CFVec2 VelocityZ;		// how fast should the particles be traveling to start
		// where should the initial position be 
	f32 fRandomInitPosRadius;	// a radius about the emission pt to randomize the initial position of each particle
	CFVec3 JitterOffset;		// how much should the particles be offset, randomly each frame
} _Ver2_KeyFrame_t;
typedef enum {
	_VER2_ANIM_FUNC_TYPE_CONSTANT = 0,		// %out = fConstant
	_VER2_ANIM_FUNC_TYPE_LOOKUP_TABLE,		// %out = LERP of nearest values in table indexed by %age * _VER2_NUM_TABLE_ENTRIES
	_VER2_ANIM_FUNC_TYPE_LINEAR_TIME,		// %out = %age
	_VER2_ANIM_FUNC_TYPE_RANDOM,			// %out = random (fRandOutChance of 1.0f)
	_VER2_ANIM_FUNC_TYPE_TIME_2,			// %out = %age^2
	_VER2_ANIM_FUNC_TYPE_TIME_3,			// %out = %age^3
	
	_VER2_ANIM_FUNC_TYPE_COUNT
} _Ver2_AnimFuncType_e;
#define _VER2_NUM_TABLE_ENTRIES		25
typedef struct {
	f32 fConstant;					// UNIT FLOAT
} _Ver2_Func_Constant_t;
typedef struct {
	f32 afLookupTable[_VER2_NUM_TABLE_ENTRIES];	// UNIT FLOAT
} _Ver2_Func_Lookup_t;
typedef struct {
	f32 fOOFuncRange;		// set to 1 / (fOutUnitPercent - fInUnitPercent)
} _Ver2_Func_Linear_t;
typedef struct {
	f32 fRandOutChance;				// UNIT FLOAT
	f32 fRandomReNormalizer;		// set to 1.0f / (1.0f - fRandOutChance)
} _Ver2_Func_Random_t;
struct _Ver2_AnimFunc_t {
	f32 fInUnitPercent;		// UNIT FLOAT
	f32 fOutUnitPercent;	// UNIT FLOAT	
	_Ver2_AnimFuncType_e nFuncType;
	//f32 fEvalFreq;			// in times per second
	union {
		_Ver2_Func_Constant_t	Constant;		// only used if nFuncType == _VER2_ANIM_FUNC_TYPE_CONSTANT
		_Ver2_Func_Lookup_t		Lookup;			// only used if nFuncType == _VER2_ANIM_FUNC_TYPE_LOOKUP_TABLE
		_Ver2_Func_Linear_t		Linear;			// only used if nFuncType == _VER2_ANIM_FUNC_TYPE_LINEAR_TIME
		_Ver2_Func_Random_t		Random;			// only used if nFuncType == _VER2_ANIM_FUNC_TYPE_RANDOM
	};	
};
typedef struct {
	u32 nVersion;				// what version is this data, see _VER2_FILE_VERSION
	u32 nFlags;					// see _VER2_DEF_FLAGS_... for description
		
	_Ver2_KeyFrame_t Min;	// use these values at intensity 0.0f
	_Ver2_KeyFrame_t Max;	// use these values at intensity 1.0f

	// the following values do not animate over the different intensities
	char szTextureName[FDATA_TEXNAME_LEN+1];
	FTexDef_t *pTexDef;		// the loaded texture
	_Ver2_AnimFunc_t AnimAlphaInfo;
	_Ver2_AnimFunc_t AnimScaleInfo;
	_Ver2_AnimFunc_t AnimColorInfo;
	u32 nLightMotifIndex;
	f32 fSecsBetweenLightSamples;	// how often should the world lights be sampled
	CFVec3 SampledLightScale;	// the sampled light is scaled by these values

	u32 nIndexInList;			// used to verify that handles are valid
	FLink_t Link;				// for insertion into a ParticleDef linklist
	FLinkRoot_t EmitterList;	// to keep a list of the emitters of this type currently in use
} _Ver2_Def_t;
// END OF VERSION 2 STRUCTS
///////////////////////////


////////////////////////////////////////////////////////////////////////////
// START OF VERSION 3 STRUCTS
//
// to move from version 3 -> 4 (switched out the CFTexInst for a CFTexDef *)
#define _VERSION3_BYTES_LARGER_THAN_VERSION4	12
#define _VERSION3_SIZE							(892 + _VERSION3_BYTES_LARGER_THAN_VERSION4)
#define _VERSION4_SIZE							892
#define _VERSION3_BYTES_TO_READ_IN_FIRST_CHUNK	508
#define _VERSION3_BYTES_TO_READ_IN_SECOND_CHUNK	(_VERSION4_SIZE - _VERSION3_BYTES_TO_READ_IN_FIRST_CHUNK)
// END OF VERSION 3 STRUCTS
///////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// START OF VERSION 4 STRUCTS
//
// to move from version 4 -> 5 (added CFVec2 EmitPause to each FParticleKeyFrame_t)
#define _VERSION5_BYTES_ADDED_TO_EACH_KEY_FRAME	8
#define _VERSION5_BYTES_LARGER_THAN_VERSION4	( _VERSION5_BYTES_ADDED_TO_EACH_KEY_FRAME * 2 )
#define _VERSION5_SIZE							( _VERSION4_SIZE + _VERSION5_BYTES_LARGER_THAN_VERSION4 )

#define _VERSION4_BYTES_TO_READ_IN_FIRST_CHUNK	60
#define _VERSION4_BYTES_TO_READ_IN_SECOND_CHUNK	240
#define _VERSION4_BYTES_TO_READ_IN_THIRD_CHUNK	592
// END OF VERSION 4 STRUCTS
///////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// START OF VERSION 5 STRUCTS
//
// to move from version 5 -> 6 (added f32 fEmulationDist to each FParticleKeyFrame_t)
#define _VERSION6_BYTES_ADDED_TO_EACH_KEY_FRAME	4
#define _VERSION6_BYTES_LARGER_THAN_VERSION5	( _VERSION6_BYTES_ADDED_TO_EACH_KEY_FRAME * 2 )
#define _VERSION6_SIZE							( _VERSION5_SIZE + _VERSION6_BYTES_LARGER_THAN_VERSION5 )

#define _VERSION5_BYTES_TO_READ_IN_FIRST_CHUNK	20
#define _VERSION5_BYTES_TO_READ_IN_SECOND_CHUNK	248
#define _VERSION5_BYTES_TO_READ_IN_THIRD_CHUNK	640
// END OF VERSION 5 STRUCTS
///////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// START OF VERSION 6 STRUCTS
//
// to move from version 6 -> 7 (added a bunch of fields)
#define _VERSION7_BYTES_LARGER_THAN_VERSION6	( 232 )
#define _VERSION7_SIZE							( _VERSION5_SIZE + _VERSION6_BYTES_LARGER_THAN_VERSION5 )

#define _VERSION6_BYTES_TO_READ_IN_CHUNK1		44
#define _VERSION6_BYTES_TO_READ_IN_CHUNK2		216
#define _VERSION6_BYTES_TO_READ_IN_CHUNK3		36
#define _VERSION6_BYTES_TO_READ_IN_CHUNK4		216
#define _VERSION6_BYTES_TO_READ_IN_CHUNK5		360
#define _VERSION6_BYTES_TO_READ_IN_CHUNK6		44
// END OF VERSION 6 STRUCTS
///////////////////////////////////////////////////////////////////////////////////////



BOOL CDefFileIO::Read( const CString &rsFilename, FParticleDef_t *pDest, BOOL &rbDataWasUpdated ) {
	u8 *pTempSrc, *pTempDest;

	FILE *pFile = fopen( rsFilename, "rb" );
	if( !pFile ) {
		return FALSE;
	}
	// get the filesize
	u32 nFileSize;
	fseek( pFile, 0, SEEK_END );
	nFileSize = ftell( pFile );
	fseek( pFile, 0, SEEK_SET );

	// determine the version number
	u32 nVersion;
	if( fread( &nVersion, 1, sizeof( u32 ), pFile ) != sizeof( u32 ) ) {
		// couldn't read the version number, bail
		fclose( pFile );
		return FALSE;
	}
	fseek( pFile, 0, SEEK_SET );

	if( nVersion == FPARTICLE_FILE_VERSION ) {
		FASSERT( nFileSize == sizeof( FParticleDef_t ) );

		// no need to do any conversion, just read the file into pDest and return
		if( fread( pDest, 1, sizeof( FParticleDef_t ), pFile ) != sizeof( FParticleDef_t ) ) {
			fclose( pFile );
			return FALSE;
		}
		fclose( pFile );

		pDest->Min.fMinPercentToDraw = _MIN_PERCENT_TO_DRAW;
		pDest->Max.fMinPercentToDraw = _MIN_PERCENT_TO_DRAW;

		// do some basic error checking on the file
		BOOL bMadeChanges = FALSE;
		if( pDest->fSecsBetweenLightSamples < (1.0f/20.0f) ) {
			pDest->fSecsBetweenLightSamples = (1.0f/20.0f);
			bMadeChanges = TRUE;
		} else if( pDest->fSecsBetweenLightSamples > 1.0f ) {
			pDest->fSecsBetweenLightSamples = 1.0f;
			bMadeChanges = TRUE;
		}
			
		rbDataWasUpdated = bMadeChanges;

		return TRUE;
	}

	/////////////////////////////
	// convert the version number
	/////////////////////////////

	// read in the old file
	void *pOld = NULL, *pNew =  NULL;

	// allocate memory to hold the old file
	pOld = fang_Malloc( nFileSize, 4 );
	if( !pOld ) {
		fclose( pFile );
		return FALSE;
	}
	// read in the old file
	if( fread( pOld, 1, nFileSize, pFile ) != nFileSize ) {
		fclose( pFile );
		fang_Free( pOld );
		return FALSE;
	}
	// we are done with the file
	fclose( pFile );
	
	// do different things depending on the version number
	switch( nVersion ) {

	case 1:
		// allocate memory for the new file
		pNew = fang_Malloc( sizeof( _Ver2_Def_t ), 4 );
		if( !pNew ) {
			fang_Free( pOld );
			return FALSE;
		}
		if( !ConvertVer1ToVer2( pOld, pNew ) ) {
			fang_Free( pOld );
			fang_Free( pNew );
			return FALSE;
		}
		fang_Free( pOld );
		pOld = pNew;
		pNew = NULL;

		nFileSize = sizeof( _Ver2_Def_t );
		
		//////////////////////////////////////
		// PASS THROUGH TO THE NEXT CONVERSION
		//////////////////////////////////////

	case 2:
		// allocate memory for the new file
		pNew = fang_Malloc( _VERSION3_SIZE, 4 );
		if( !pNew ) {
			fang_Free( pOld );
			return FALSE;
		}
		if( !ConvertVer2ToVer3( pOld, pNew ) ) {
			fang_Free( pOld );
			fang_Free( pNew );
			return FALSE;
		}
		fang_Free( pOld );
		pOld = pNew;
		pNew = NULL;

		nFileSize = _VERSION3_SIZE;
		
		//////////////////////////////////////
		// PASS THROUGH TO THE NEXT CONVERSION
		//////////////////////////////////////

	case 3:
		// CONVERT because size of the text inst structure changed
		pTempSrc = (u8 *)pOld;
		pTempDest = (u8 *)pDest;

		// copy the 1st chunk of bytes to the dest
		fang_MemCopy( pTempDest, pTempSrc, _VERSION3_BYTES_TO_READ_IN_FIRST_CHUNK );

		// advance our src and dest ptrs by the correct amounts
		pTempDest += _VERSION3_BYTES_TO_READ_IN_FIRST_CHUNK;
		pTempSrc += _VERSION3_BYTES_TO_READ_IN_FIRST_CHUNK;
		pTempSrc += _VERSION3_BYTES_LARGER_THAN_VERSION4;

		// copy the 2nd chunk of bytes to the dest
		fang_MemCopy( pTempDest, pTempSrc, _VERSION3_BYTES_TO_READ_IN_SECOND_CHUNK );

		// fill in some default fields
		pDest->pTexDef = NULL;
		pDest->nVersion = 4;
		pDest->Min.fMinPercentToDraw = _MIN_PERCENT_TO_DRAW;
		pDest->Max.fMinPercentToDraw = _MIN_PERCENT_TO_DRAW;

		// copy the smaller pDest into pOld for further version conversion
		fang_MemCopy( pOld, pDest, _VERSION4_SIZE );   

		pNew = NULL;
		nFileSize = _VERSION4_SIZE;

		//////////////////////////////////////
		// PASS THROUGH TO THE NEXT CONVERSION
		//////////////////////////////////////

	case 4:
		// CONVERT because we added 8 bytes to each keyframe
		pTempSrc = (u8 *)pOld;
		pTempDest = (u8 *)pDest;

		// copy the 1st chunk of bytes to the dest
		fang_MemCopy( pTempDest, pTempSrc, _VERSION4_BYTES_TO_READ_IN_FIRST_CHUNK );

		// advance our src and dest ptrs by the correct amounts
		pTempDest += _VERSION4_BYTES_TO_READ_IN_FIRST_CHUNK;
		pTempDest += _VERSION5_BYTES_ADDED_TO_EACH_KEY_FRAME;

		pTempSrc += _VERSION4_BYTES_TO_READ_IN_FIRST_CHUNK;

		// copy the 2nd chunk of bytes to the dest
		fang_MemCopy( pTempDest, pTempSrc, _VERSION4_BYTES_TO_READ_IN_SECOND_CHUNK );

		// advance our src and dest ptrs by the correct amounts
		pTempDest += _VERSION4_BYTES_TO_READ_IN_SECOND_CHUNK;
		pTempDest += _VERSION5_BYTES_ADDED_TO_EACH_KEY_FRAME;

		pTempSrc += _VERSION4_BYTES_TO_READ_IN_SECOND_CHUNK;

		// copy the 3rd chunk of bytes to the dest
		fang_MemCopy( pTempDest, pTempSrc, _VERSION4_BYTES_TO_READ_IN_THIRD_CHUNK );

		// fill in some default fields
		pDest->Min.EmitPause.Zero();
		pDest->Max.EmitPause.Zero();
		pDest->nVersion = 5;
		pDest->Min.fMinPercentToDraw = _MIN_PERCENT_TO_DRAW;
		pDest->Max.fMinPercentToDraw = _MIN_PERCENT_TO_DRAW;
		
		// copy the smaller pDest into pOld for further version conversion
		fang_MemCopy( pOld, pDest, _VERSION5_SIZE );   

		pNew = NULL;
		nFileSize = _VERSION5_SIZE;

		//////////////////////////////////////
		// PASS THROUGH TO THE NEXT CONVERSION
		//////////////////////////////////////

	case 5:
		// CONVERT because we added 4 bytes to each keyframe
		pTempSrc = (u8 *)pOld;
		pTempDest = (u8 *)pDest;

		// copy the 1st chunk of bytes to the dest
		fang_MemCopy( pTempDest, pTempSrc, _VERSION5_BYTES_TO_READ_IN_FIRST_CHUNK );

		// advance our src and dest ptrs by the correct amounts
		pTempDest += _VERSION5_BYTES_TO_READ_IN_FIRST_CHUNK;
		pTempDest += _VERSION6_BYTES_ADDED_TO_EACH_KEY_FRAME;

		pTempSrc += _VERSION5_BYTES_TO_READ_IN_FIRST_CHUNK;

		// copy the 2nd chunk of bytes to the dest
		fang_MemCopy( pTempDest, pTempSrc, _VERSION5_BYTES_TO_READ_IN_SECOND_CHUNK );

		// advance our src and dest ptrs by the correct amounts
		pTempDest += _VERSION5_BYTES_TO_READ_IN_SECOND_CHUNK;
		pTempDest += _VERSION6_BYTES_ADDED_TO_EACH_KEY_FRAME;

		pTempSrc += _VERSION5_BYTES_TO_READ_IN_SECOND_CHUNK;

		// copy the 3rd chunk of bytes to the dest
		fang_MemCopy( pTempDest, pTempSrc, _VERSION5_BYTES_TO_READ_IN_THIRD_CHUNK );

		// fill in some default fields
		pDest->Min.fEmulationDist = 0.0f;
		pDest->Max.fEmulationDist = 0.0f;
		pDest->nVersion = 6;
		
		// copy the smaller pDest into pOld for further version conversion
		fang_MemCopy( pOld, pDest, _VERSION6_SIZE );   

		pNew = NULL;
		nFileSize = _VERSION6_SIZE;

		//////////////////////////////////////
		// PASS THROUGH TO THE NEXT CONVERSION
		//////////////////////////////////////

	case 6:
		// CONVERT because we added a bunch of fields
		pTempSrc = (u8 *)pOld;
		pTempDest = (u8 *)pDest;

		// copy the 1st chunk and advance our src and dest ptrs by the correct amounts
		fang_MemCopy( pTempDest, pTempSrc, _VERSION6_BYTES_TO_READ_IN_CHUNK1 );
		pTempDest += _VERSION6_BYTES_TO_READ_IN_CHUNK1;
		pTempDest += 32;
		pTempSrc += _VERSION6_BYTES_TO_READ_IN_CHUNK1;

		// copy the 2nd chunk and advance our src and dest ptrs by the correct amounts
		fang_MemCopy( pTempDest, pTempSrc, _VERSION6_BYTES_TO_READ_IN_CHUNK2 );
		pTempDest += _VERSION6_BYTES_TO_READ_IN_CHUNK2;
		pTempDest += 64;
		pTempSrc += _VERSION6_BYTES_TO_READ_IN_CHUNK2;

		// copy the 3rd chunk and advance our src and dest ptrs by the correct amounts
		fang_MemCopy( pTempDest, pTempSrc, _VERSION6_BYTES_TO_READ_IN_CHUNK3 );
		pTempDest += _VERSION6_BYTES_TO_READ_IN_CHUNK3;
		pTempDest += 32;
		pTempSrc += _VERSION6_BYTES_TO_READ_IN_CHUNK3;

		// copy the 4th chunk and advance our src and dest ptrs by the correct amounts
		fang_MemCopy( pTempDest, pTempSrc, _VERSION6_BYTES_TO_READ_IN_CHUNK4 );
		pTempDest += _VERSION6_BYTES_TO_READ_IN_CHUNK4;
		pTempDest += 64;
		pTempSrc += _VERSION6_BYTES_TO_READ_IN_CHUNK4;

		// copy the 5th chunk and advance our src and dest ptrs by the correct amounts
		fang_MemCopy( pTempDest, pTempSrc, _VERSION6_BYTES_TO_READ_IN_CHUNK5 );
		pTempDest += _VERSION6_BYTES_TO_READ_IN_CHUNK5;
		pTempDest += 40;
		pTempSrc += _VERSION6_BYTES_TO_READ_IN_CHUNK5;

		// copy the 6th chunk and advance our src and dest ptrs by the correct amounts
		fang_MemCopy( pTempDest, pTempSrc, _VERSION6_BYTES_TO_READ_IN_CHUNK6 );
		pTempDest += _VERSION6_BYTES_TO_READ_IN_CHUNK6;
		pTempSrc += _VERSION6_BYTES_TO_READ_IN_CHUNK6;

		FASSERT( ((u32)pTempSrc - (u32)pOld) == _VERSION6_SIZE );
		FASSERT( ((u32)pTempDest - (u32)pDest) == sizeof( FParticleDef_t ) );
		
		// fill in some default fields
		pDest->Min.EndLightRGB = pDest->Min.StartLightRGB;
		pDest->Min.fEndLightIntensity = pDest->Min.fStartLightIntensity;
		pDest->Min.fEndLightRadiusMultiplier = pDest->Min.fStartLightRadiusMultiplier = 1.0f;
		pDest->Min.fMaxLightRadius = 5.0f;
		pDest->Min.fSndPitchMultiplier = 1.0f;
		pDest->Min.fSndUnitVolume = 1.0f;
		pDest->Min.SpikeFreq.Zero();
		pDest->Min.SpikeRed.Zero();
		pDest->Min.SpikeGreen.Zero();
		pDest->Min.SpikeBlue.Zero();
		pDest->Min.SpikeAlpha.Zero();
		pDest->Min.SpikeScale.Zero();
		pDest->Min.SpikeDuration.Zero();
		pDest->Min.SpikeHold.Zero();
		pDest->Max.EndLightRGB = pDest->Max.StartLightRGB;
		pDest->Max.fEndLightIntensity = pDest->Max.fStartLightIntensity;
		pDest->Max.fEndLightRadiusMultiplier = pDest->Max.fStartLightRadiusMultiplier = 1.0f;
		pDest->Max.fMaxLightRadius = 5.0f;
		pDest->Max.fSndPitchMultiplier = 1.0f;
		pDest->Max.fSndUnitVolume = 1.0f;
		pDest->Max.SpikeFreq.Zero();
		pDest->Max.SpikeRed.Zero();
		pDest->Max.SpikeGreen.Zero();
		pDest->Max.SpikeBlue.Zero();
		pDest->Max.SpikeAlpha.Zero();
		pDest->Max.SpikeScale.Zero();
		pDest->Max.SpikeDuration.Zero();
		pDest->Max.SpikeHold.Zero();
		fang_MemZero( pDest->szCoronaTextureName, FDATA_TEXNAME_LEN+1 );
		pDest->fCoronaScale = 1.0f;
		pDest->nFlags2 = FPARTICLE_DEF_FLAG2_NONE;
		fang_MemZero( pDest->szSoundFxName, FDATA_TEXNAME_LEN+1 );

		// disable all lights cause till this version they didn't work and we don't want bogus values
		pDest->nFlags &= ~FPARTICLE_DEF_FLAGS_EMIT_LIGHT;
		pDest->nLightMotifIndex = 0;

		pDest->nVersion = 7;
		
		// done, free our memory
		fang_Free( pOld );
		fang_Free( pNew );

		nFileSize = _VERSION7_SIZE;
		break;

	default:
		FASSERT_NOW;
		fang_Free( pOld );
		return FALSE;
		break;
	}

	rbDataWasUpdated = TRUE;
	return TRUE;
}

BOOL CDefFileIO::Write( const CString &rsFilename, FParticleDef_t *pSrc ) {

	// cache a few fields before writting out to disk
	FTexDef_t *pTexDef = pSrc->pTexDef;
	FLink_t Link = pSrc->Link;
	FLinkRoot_t LinkRoot = pSrc->EmitterList;
	u32 nIndexInList = pSrc->nIndexInList;

	// make sure that a few fields are set properly before writting out to disk
	pSrc->nVersion = FPARTICLE_FILE_VERSION;	
	pSrc->pTexDef = NULL;
	fang_MemZero( &pSrc->Link, sizeof( FLink_t ) );
	fang_MemZero( &pSrc->EmitterList, sizeof( FLinkRoot_t ) );
	pSrc->nIndexInList = 0;
	pSrc->Min.fMinPercentToDraw = _MIN_PERCENT_TO_DRAW;
	pSrc->Max.fMinPercentToDraw = _MIN_PERCENT_TO_DRAW;
		
	// open the file for writting
	FILE *pFileStream = _tfopen( rsFilename, _T("wb") );
	if( !pFileStream ) {
		// restore the cached vars
		pSrc->pTexDef = pTexDef;
		pSrc->Link = Link;
		pSrc->EmitterList = LinkRoot;
		pSrc->nIndexInList = nIndexInList;
		return FALSE;
	}
	// write out our file
	int nItemsWritten = fwrite( pSrc, sizeof( FParticleDef_t ), 1, pFileStream );
	// close our file
	fclose( pFileStream );

	// restore the cached vars
	pSrc->pTexDef = pTexDef;
	pSrc->Link = Link;
	pSrc->EmitterList = LinkRoot;
	pSrc->nIndexInList = nIndexInList;

	// if we couldn't write out the file, report it
	if( nItemsWritten != 1 ) {
		return FALSE;
	}

	return TRUE;
}

// it is assumed that pVer2 and pVer3 point to enough memory to point to their respective version sizes
BOOL CDefFileIO::ConvertVer2ToVer3( void *pVer2, void *pVer3 ) {
	_Ver2_Def_t *pDefVer2 = (_Ver2_Def_t *)pVer2;
	FParticleDef_t *pDefVer3 = (FParticleDef_t *)pVer3;
	FParticleKeyFrame_t *pOldKeyFrame;

	if( pDefVer2->nVersion != 2 ) {
		// this function only converts data from version 2 to version 3
		return FALSE;
	}

	// copy all of the common fields
	pDefVer3->nVersion = 3;
	pDefVer3->nFlags = pDefVer2->nFlags;
	fang_MemCopy( pDefVer3->szTextureName, pDefVer2->szTextureName, (FDATA_TEXNAME_LEN+1) );
//	pDefVer3->TexInst.SetToDefaults();
	fang_MemCopy( &pDefVer3->AnimAlphaInfo, &pDefVer2->AnimAlphaInfo, sizeof( _Ver2_AnimFunc_t ) );
	fang_MemCopy( &pDefVer3->AnimScaleInfo, &pDefVer2->AnimScaleInfo, sizeof( _Ver2_AnimFunc_t ) );
	fang_MemCopy( &pDefVer3->AnimColorInfo, &pDefVer2->AnimColorInfo, sizeof( _Ver2_AnimFunc_t ) );
	pDefVer3->nLightMotifIndex = pDefVer2->nLightMotifIndex;
	pDefVer3->fSecsBetweenLightSamples = pDefVer2->fSecsBetweenLightSamples;
	pDefVer3->SampledLightScale = pDefVer2->SampledLightScale;
	pDefVer3->nIndexInList = pDefVer2->nIndexInList;
	pDefVer3->Link = pDefVer2->Link;
	pDefVer3->EmitterList = pDefVer2->EmitterList;

	// the keyframe size didn't change at all
	pOldKeyFrame = (FParticleKeyFrame_t *)&pDefVer2->Min;
	pDefVer3->Min = *pOldKeyFrame;

	pOldKeyFrame = (FParticleKeyFrame_t *)&pDefVer2->Max;
	pDefVer3->Max = *pOldKeyFrame;

	return TRUE;
}

// it is assumed that pVer1 and pVer2 point to enough memory to point to their respective version sizes
BOOL CDefFileIO::ConvertVer1ToVer2( void *pVer1, void *pVer2 ) {
	_Ver1_Def_t *pDefVer1 = (_Ver1_Def_t *)pVer1;
	_Ver2_Def_t *pDefVer2 = (_Ver2_Def_t *)pVer2;
	u32 i;
	_Ver1_KeyFrame_t *pKeyVer1;
	_Ver2_KeyFrame_t *pKeyVer2;

	if( pDefVer1->nVersion != 1 ) {
		// this function only converts data from version 1 to version 2
		return FALSE;
	}

	// copy all of the common fields
	pDefVer2->nVersion = 2;
	pDefVer2->nFlags = pDefVer1->nFlags;
	fang_MemCopy( pDefVer2->szTextureName, pDefVer1->szTextureName, (FDATA_TEXNAME_LEN+1) );
	pDefVer2->pTexDef = pDefVer1->pTexDef;
	fang_MemCopy( &pDefVer2->AnimAlphaInfo, &pDefVer1->AnimAlphaInfo, sizeof( _Ver2_AnimFunc_t ) );
	fang_MemCopy( &pDefVer2->AnimScaleInfo, &pDefVer1->AnimScaleInfo, sizeof( _Ver2_AnimFunc_t ) );
	fang_MemCopy( &pDefVer2->AnimColorInfo, &pDefVer1->AnimColorInfo, sizeof( _Ver2_AnimFunc_t ) );
	pDefVer2->nLightMotifIndex = pDefVer1->nLightMotifIndex;
	pDefVer2->fSecsBetweenLightSamples = pDefVer1->fSecsBetweenLightSamples;
	pDefVer2->SampledLightScale = pDefVer1->SampledLightScale;
	pDefVer2->nIndexInList = pDefVer1->nIndexInList;
	pDefVer2->Link = pDefVer1->Link;
	pDefVer2->EmitterList = pDefVer1->EmitterList;

	// copy the keyframes
	pKeyVer1 = &pDefVer1->Min;
	pKeyVer2 = &pDefVer2->Min;
	for( i=0; i < 2; i++ ) {
		// copy all of the common fields
		pKeyVer2[i].fCullDist = pKeyVer1[i].fCullDist;
		pKeyVer2[i].fStartSkipDrawDist = pKeyVer1[i].fStartSkipDrawDist;
		pKeyVer2[i].fMinPercentToDraw = pKeyVer1[i].fMinPercentToDraw;
		pKeyVer2[i].LightRGB = pKeyVer1[i].LightRGB;
		pKeyVer2[i].fLightIntensity = pKeyVer1[i].fLightIntensity;
		pKeyVer2[i].fLightRadiusMultiplier = pKeyVer1[i].fLightRadiusMultiplier;
		pKeyVer2[i].NumPerBurst = pKeyVer1[i].NumPerBurst;
		pKeyVer2[i].fSecsToEmit = pKeyVer1[i].fSecsToEmit;
		pKeyVer2[i].fDragMultiplierPerSec = pKeyVer1[i].fDragMultiplierPerSec;
		pKeyVer2[i].fGravityPerSec = pKeyVer1[i].fGravityPerSec;
		pKeyVer2[i].fMinCollAlpha = pKeyVer1[i].fMinCollAlpha;
		pKeyVer2[i].fMaxCollSplit = pKeyVer1[i].fMaxCollSplit;
		pKeyVer2[i].fUnitCollBounce = pKeyVer1[i].fUnitCollBounce;
		pKeyVer2[i].fNumSprites = pKeyVer1[i].fNumSprites;
		pKeyVer2[i].fMaxVel2 = pKeyVer1[i].fMaxVel2;
		pKeyVer2[i].fAlphaStepPerSprite = pKeyVer1[i].fAlphaStepPerSprite;
		pKeyVer2[i].fSizeStepPerSprite = pKeyVer1[i].fSizeStepPerSprite;
		pKeyVer2[i].fPosStepPerSprite = pKeyVer1[i].fPosStepPerSprite;	
		pKeyVer2[i].InitScale = pKeyVer1[i].InitScale;
		pKeyVer2[i].FinalScale = pKeyVer1[i].FinalScale;
		pKeyVer2[i].InitRed = pKeyVer1[i].InitRed;
		pKeyVer2[i].InitGreen = pKeyVer1[i].InitGreen;
		pKeyVer2[i].InitBlue = pKeyVer1[i].InitBlue;	
		pKeyVer2[i].FinalRed = pKeyVer1[i].FinalRed;
		pKeyVer2[i].FinalGreen = pKeyVer1[i].FinalGreen;
		pKeyVer2[i].FinalBlue = pKeyVer1[i].FinalBlue;
		pKeyVer2[i].InitAlpha = pKeyVer1[i].InitAlpha;	
		pKeyVer2[i].FinalAlpha = pKeyVer1[i].FinalAlpha;	
		pKeyVer2[i].OOLifeSecs = pKeyVer1[i].OOLifeSecs;
		pKeyVer2[i].EmissiveIntensity = pKeyVer1[i].EmissiveIntensity;	
		pKeyVer2[i].InitDirAngle = pKeyVer1[i].InitDirAngle;
		pKeyVer2[i].VelocityZ = pKeyVer1[i].VelocityZ;
		pKeyVer2[i].fRandomInitPosRadius = pKeyVer1[i].fRandomInitPosRadius;

		// init the new fields
		pKeyVer2[i].SecsBetweenBursts.Set( pKeyVer1[i].fSecsBetweenBursts, pKeyVer1[i].fSecsBetweenBursts );
		pKeyVer2[i].MaxBubbleXYZ.Zero();
		pKeyVer2[i].fBubbleUpdateChance = 0.1f;
		pKeyVer2[i].fBubbleDragPerSec = 0.0f;
		pKeyVer2[i].JitterOffset.Zero();
	}

	return TRUE;
}







