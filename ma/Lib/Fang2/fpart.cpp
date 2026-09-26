//////////////////////////////////////////////////////////////////////////////////////
// fpart.cpp - 
//
// Author: Michael Starich   
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
// 08/02/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "fpart.h"


BOOL fpart_SpawnParticleEmitter( FPart_Type_e nType, f32 fUnitIntensity, const CFVec3A &rPos, 
								 const CFVec3A *pUnitDir/*=NULL*/, BOOL bSpawnLight/*=FALSE*/,
								 const CFVec3A *pEmitterVelPerSec_WS/*=NULL*/,
								 const CFColorRGB *prgbColorBias/* = NULL*/ ) {
	return TRUE;
}



#if 0
//====================
// private definitions

#define _GET_RAND								( fmath_RandomFloat() )
#define _GET_POS_NEG_RAND						( fmath_RandomBipolarUnitFloat() )

// SPARK DEFINES
#define _SPARK_GRAVITY							-60.0f
#define _2_SPARK_VEL2							25.0f
#define _3_SPARK_VEL2							35.0f
#define _4_SPARK_VEL2							50.0f
#define _5_SPARK_VEL2							60.0f
#define _6_SPARK_VEL2							75.0f
#define _SPARK_STEP_PERCENT						0.0125f
#define _MAX_SPRITES_PER_SPARK					6
#define _MIN_NUM_SPARKS							6//8
#define _MAX_NUM_SPARKS							21//23
#define _RAND_SPARKS							8//7
#define _ASSIGN_SPARK_INITIAL_VEL				( FMATH_FPOT( fmath_RandomFloat(), 9.0f, 20.0f ) )
#define _ASSIGN_SPARK_FADE						( 1.0f / FMATH_FPOT( fmath_RandomFloat(), 0.70f, 0.90f ) )
#define _ASSIGN_SPARK_SIZE						( FMATH_FPOT( fmath_RandomFloat(), 0.075f, 0.111f ) )
#define _ASSIGN_SPARK_GROWTH					( FMATH_FPOT( fmath_RandomFloat(), 0.025f, 0.045f ) )
#define _ASSIGN_SPARK_POS						( FMATH_FPOT( fmath_RandomFloat(), 0.001f, 0.01f ) )
#define _SPARK_R_OVER_TIME( fPercentIntoFade )	( 1.0f )
#define _SPARK_G_OVER_TIME( fPercentIntoFade )	( FMATH_FPOT( fPercentIntoFade, 1.0f, 0.5f ) )
#define _SPARK_B_OVER_TIME( fPercentIntoFade )	( FMATH_FPOT( fPercentIntoFade, 1.0f, 0.5f ) )
#define _SPARK_INNER_CONE						( 0.7f )
#define _SPARK_OUTTER_CONE						( 1.0f )
#define _SPARK_CULL_DIST						( 200.0f )
#define _SPARK_BLEND							FPSPRITE_BLEND_MODULATE

// SMOKE DEFINES
#define _MIN_NUM_PUFFS						7//8
#define _MAX_NUM_PUFFS						21//23
#define _RAND_PUFFS							9//10
#define _PUFF_CULL_DIST						( 200.0f )
#define _PUFF_BLEND							FPSPRITE_BLEND_MODULATE
#define _PUFF_INNER_CONE					( 0.30f )
#define _PUFF_OUTTER_CONE					( 0.5f )
#define _ASSIGN_PUFF_INITIAL_VEL			( FMATH_FPOT( fmath_RandomFloat(), 3.5f, 4.75f ) )
#define _ASSIGN_PUFF_OPACITY				( 0.6f )
#define _ASSIGN_PUFF_FADE( fUnitIntensity )	( 1.0f / ((FMATH_FPOT( fmath_RandomFloat(), 0.39f, 0.44f )) + (fUnitIntensity * 0.40f) ) )//0.78f, 0.88f ) )
#define _ASSIGN_PUFF_SIZE					( FMATH_FPOT( fmath_RandomFloat(), 0.25f, 0.45f ) )
#define _ASSIGN_PUFF_GROWTH					( FMATH_FPOT( fmath_RandomFloat(), 0.60f, 0.85f ) )
#define _ASSIGN_PUFF_POS					( FMATH_FPOT( fmath_RandomFloat(), 0.001f, 0.01f ) )
#define _PUFF_Y_RISE_PER_SEC				( 0.75f )
#define _PUFF_XZ_VEL_RAND					( FMATH_FPOT( fmath_RandomFloat(), -0.25f, 0.25f ) )
#define _PUFF_VEL_DECEL						( FMATH_FPOT( fmath_RandomFloat(), 3.95f, 4.05f ) )
#define _PUFF_VEL_XZ_CHANGE_DAMPEN			( 0.25f ) 

// PIXEL DUST1 DEFINES
#define _MIN_NUM_PIXEL_DUST1S									3//8
#define _MAX_NUM_PIXEL_DUST1S									18//23
#define _RAND_PIXEL_DUST1S										5//7
#define _PIXEL_DUST1_CULL_DIST									( 200.0f )
#define _PIXEL_DUST1_BLEND										FPSPRITE_BLEND_MODULATE
#define _PIXEL_DUST1_GRAVITY									3.0f
#define _PIXEL_DUST1_INNER_CONE									( 0.12f )
#define _PIXEL_DUST1_OUTTER_CONE								( 0.26f )
#define _ASSIGN_PIXEL_DUST1_FADE								( 1.0f / FMATH_FPOT( fmath_RandomFloat(), 0.75f, 0.95f ) )
#define _PIXEL_DUST1_MAX_SIZE									0.5f
#define _ASSIGN_PIXEL_DUST1_SIZE								( FMATH_FPOT( fmath_RandomFloat(), 0.01f, 0.05f ) )
#define _ASSIGN_PIXEL_DUST1_POS									( FMATH_FPOT( fmath_RandomFloat(), 0.01f, 0.4f ) )
#define _ASSIGN_PIXEL_DUST1_INITIAL_VEL							( FMATH_FPOT( fmath_RandomFloat(), 0.5f, 1.5f ) )
// 0.9f, 0.5f, 1.0f;
#define _PIXEL_DUST1_R_OVER_TIME( fPercentIntoFade, fRed )		( FMATH_FPOT( fPercentIntoFade, 1.0f, fRed ) )
#define _PIXEL_DUST1_G_OVER_TIME( fPercentIntoFade, fGreen )	( FMATH_FPOT( fPercentIntoFade, 1.0f, fGreen ) )
#define _PIXEL_DUST1_B_OVER_TIME( fPercentIntoFade, fBlue )		( FMATH_FPOT( fPercentIntoFade, 1.0f, fBlue ) )

// PIXIE PUFF DEFINES
#define _MIN_NUM_PIXIE_PUFFS						7//8
#define _MAX_NUM_PIXIE_PUFFS						21//23
#define _RAND_PIXIE_PUFFS							9//10
#define _PIXIE_PUFF_CULL_DIST						( 200.0f )
#define _PIXIE_PUFF_BLEND							FPSPRITE_BLEND_MODULATE
#define _PIXIE_PUFF_INNER_CONE						( 0.30f )
#define _PIXIE_PUFF_OUTTER_CONE						( 0.5f )
#define _ASSIGN_PIXIE_PUFF_INITIAL_VEL				( FMATH_FPOT( fmath_RandomFloat(), 3.5f, 4.75f ) )
//#define _ASSIGN_PIXIE_PUFF_OPACITY					( 1.0f )
#define _ASSIGN_PIXIE_PUFF_FADE( fUnitIntensity )	( 1.0f / ((FMATH_FPOT( fmath_RandomFloat(), 0.39f, 0.44f )) + (fUnitIntensity * 0.40f) ) )//0.78f, 0.88f ) )
#define _ASSIGN_PIXIE_PUFF_SIZE						( FMATH_FPOT( fmath_RandomFloat(), 0.25f, 0.45f ) )
#define _ASSIGN_PIXIE_PUFF_GROWTH					( FMATH_FPOT( fmath_RandomFloat(), 0.60f, 0.85f ) )
#define _ASSIGN_PIXIE_PUFF_POS						( FMATH_FPOT( fmath_RandomFloat(), 0.001f, 0.01f ) )
#define _PIXIE_PUFF_Y_RISE_PER_SEC					( 0.75f )
#define _PIXIE_PUFF_XZ_VEL_RAND						( FMATH_FPOT( fmath_RandomFloat(), -0.25f, 0.25f ) )
#define _PIXIE_PUFF_VEL_DECEL						( FMATH_FPOT( fmath_RandomFloat(), 3.95f, 4.05f ) )
#define _PIXIE_PUFF_VEL_XZ_CHANGE_DAMPEN			( 0.25f ) 



typedef struct {
	union {
		struct {
			// because of a c++ism, we have to put all vars into this anonymous struct :-(
			CFVec3 Pos;
			CFVec3 VelocityPerSec;
			f32 fOpacity;
			f32 fBirthTime;
			f32 fSecsToFade;
			f32 fSize;
			f32 fGrowthPerSec;
			u32 nNumSprites;
			CFVec2 XZChange;
			f32 fPosStepMultiplier;
			f32 fScaleMultiplier;
			f32 fOpacityMultiplier;			
		};
	};
} _ParticleData_t;

typedef enum {
	FADE_TYPE_PERCENT_LINEAR = 0,
	FADE_TYPE_PERCENT_SQUARED,
	FADE_TYPE_PERCENT_CUBED,
	FADE_TYPE_PERCENT_LINEAR_LAST_10,

	FADE_TYPE_COUNT
} FadeType_e;

#define _MAX_TEXTURES_PER_TYPE		3

/////////////////////////////////
// EMITS PARTICLES INTO THE WORLD
/////////////////////////////////
FCLASS_ALIGN_PREFIX class CEmitter : public CFWorldPSGroup 
{
public:
	CEmitter();

	BOOL Init( FPart_Type_e nType, f32 fUnitIntensity, const CFVec3A &rPos, const CFVec3A *pUnitDir=NULL, BOOL bSpawnLight=FALSE, const CFVec3A *pEmitterVelPerSec_WS=NULL, const CFColorRGB *prgbColorBias = NULL );
	void Kill();
		
	FPart_Type_e GetType() const	{ return m_nType; }
	BOOL InUse() const 				{ return (m_nType != FPART_TYPE_UNASSIGNED); }
	BOOL IsAvailable() const		{ return ( (m_nType == FPART_TYPE_UNASSIGNED) && ((u32)m_nFrameNumLastKilled != FVid_nFrameCounter) ); }

protected:
	FadeType_e m_nFadeType;

	static void Work( CFWorldTracker *pTracker );
	static f32 ComputeOpacity( CEmitter *pEmitter, f32 fPercentIntoFade );
	
	// called from ::Work, returns TRUE when the emitter should be killed
	static BOOL SparkWorkFunction( CEmitter *pEmitter, f32 fSecsSinceLastWork );
	static BOOL SmokeWorkFunction( CEmitter *pEmitter, f32 fSecsSinceLastWork );
	static BOOL PixelDustWorkFunction( CEmitter *pEmitter, f32 fSecsSinceLastWork );
	static BOOL SteamWorkFunction( CEmitter *pEmitter, f32 fSecsSinceLastWork );
	static BOOL PixiePuffWorkFunction( CEmitter *pEmitter, f32 fSecsSinceLastWork );

public:
	f32 m_fTotalSecs;				// how many secs has this emitter been active
	f32 m_fNextSpawn;				// Time for next spawn
	u32 m_nNumFramesSinceBirth;		// number of frames since the emitter's birth, used to optimize world light sampling amoung other things
	
	u32 m_nDataCount;				// how many _ParticleData_t are pointed to by m_pData
	_ParticleData_t *m_pData;		// custom data for different types of emitters 
	
	BOOL m_bLightAttached;			// this emitter spawned a Light Emitter, calculate the intensity each frame
	f32 m_fLightUnitIntensity;		// what intensity should the attached world light be at
	f32 m_fLightRadiusMultiplier;	// the light's radius is a multiple of the emitter's particle radius

	CFColorRGB m_SampledWorldColor;	// used to sample the world's light to help properly color the emitter's particles
	CFColorRGB m_rgbColor;		// Only used by pixel dust right now.

	static u32 nNumEmittersSpawned; // a count of the total number of emitters spawned, this helps us optimize light sampling

	f32 m_fGravityPerSec;// if negative, particles will rise (useful for smoke)
		
private:
	FPart_Type_e m_nType;
	f32 m_fUnitIntensity;
	s32 m_nFrameNumLastKilled;// prevents emitters that were killed this frame from being reused in the same frame

	BOOL SetupNewEmitter( f32 fUnitIntensity, u32 nMinParticles, u32 nMaxParticles, u32 nRandomParticles, u32 nMaxSpritesPerParticle );	

	FCLASS_STACKMEM_ALIGN( CEmitter );
} FCLASS_ALIGN_SUFFIX;


//////////////////////////////////////
// MANAGES A POOL OF CEmitter ELEMENTS
//////////////////////////////////////
class CEmitterPoolMgr
{
public:
	CEmitterPoolMgr();

	BOOL Create( u32 nNumEmitters );
	void Destroy( void );
	u32 GetNumUsed() const;
	CEmitter *GetUsed( u32 nIndex ) const;
	CEmitter *GetAvailable();
	void KillAll();
	void KillType( FPart_Type_e nType );
	void Release();

	u32 GetSize() const		{ return m_nTotalSize; }
	u32 GetNumFree() const	{ return m_nTotalSize - GetNumUsed(); }
	
private:
	u32 m_nTotalSize;
	CEmitter *m_pPool;

	FCLASS_STACKMEM_NOALIGN( CEmitterPoolMgr );
};


//////////////////////////////////////////////////////////////////////////////
// EMITS LIGHT INTO THE WORLD, THIS LIGHT IS ATTACHED TO ONE OR MORE CEMITTERS
//////////////////////////////////////////////////////////////////////////////
#define _MAX_EMITTERS_PER_LIGHT		8

class CLightEmitter : public CFWorldLight
{
public:
	CLightEmitter();
	
	BOOL Init( const FLightInit_t &rLightInit, FPart_Type_e nType, const CEmitter *pEmitter ); 
	BOOL AttachEmitter( const CEmitter *pEmitter );
	void Kill();
	
	const CEmitter *GetAttachedEmitter( u32 nIndex );
	
	FPart_Type_e GetType() const		{ return m_nType; }
	BOOL InUse() const					{ return (m_nType != FPART_TYPE_UNASSIGNED); }
	u32 GetNumAttachedEmitters() const	{ return m_nNumEmitters; }
		
private:
	
protected:
	FPart_Type_e m_nType;

	u32 m_nNumEmitters;
	const CEmitter *m_apEmitters[_MAX_EMITTERS_PER_LIGHT];

	static void Work( CFWorldTracker *pTracker );
	void CompactUsedEmitterList( u32 nOldCount );
	static void UpdateLightFromSingleEmitter( CLightEmitter &rLightEmitter, const CEmitter &rParticleEmitter );

	FCLASS_STACKMEM_NOALIGN( CLightEmitter );
};


///////////////////////////////////////////
// MANAGES A POOL OF CLightEmitter ELEMENTS
///////////////////////////////////////////
class CLightEmitterPoolMgr
{
public:
	CLightEmitterPoolMgr();

	BOOL Create( u32 nNumEmitters );
	void Destroy( void );
	u32 GetNumUsed() const;
	CLightEmitter *GetUsed( u32 nIndex ) const;
	CLightEmitter *GetAvailable();
	void KillAll();
	void KillType( FPart_Type_e nType );
	void Release();

	BOOL AddOrCombineALight( const FLightInit_t &rLightInit, FPart_Type_e nType, const CEmitter *pEmitter, CLightEmitter *pExclude=NULL );
	BOOL ReassignEmitter( CLightEmitter *pOldEmitter, const CEmitter *pEmitterToMove );

	u32 GetSize() const		{ return m_nTotalSize; }
	u32 GetNumFree() const	{ return m_nTotalSize - GetNumUsed(); }
	
private:
	u32 m_nTotalSize;
	CLightEmitter *m_pPool;

	FCLASS_STACKMEM_NOALIGN( CLightEmitterPoolMgr );
};


//=================
// public variables

//==================
// private variables

static BOOL _bModuleInited = FALSE;
static BOOL _bSetupOK = FALSE;

static CEmitterPoolMgr _EmmiterPool;
static CLightEmitterPoolMgr _LightPool;
static CFDataPool _SpritePool;
static CFDataPool _DataPool;

static FTexDef_t *_apTextures[FPART_TYPE_COUNT][_MAX_TEXTURES_PER_TYPE];
static u32 _anNumTexturesPerType[FPART_TYPE_COUNT];

u32 CEmitter::nNumEmittersSpawned;

#define _TRACK_MEM		TRUE
#if _TRACK_MEM
	static u32 _nMaxEmittersUsed = 0;
	static u32 _nMaxLightsUsed = 0;
#endif

//===================
// private prototypes

static void _ResetTextureVars( void );
static void _GetLocalXZUnitVec( const CFVec3 *pUnitDir, CFVec3 &rLocalX, CFVec3 &rLocalZ );
static void _RandomVectorInsideACone( CFVec3 &rResult, const CFVec3 &rLocalUnitX, const CFVec3 &rLocalUnitZ, f32 fInnerCone, f32 fOuterCone, BOOL bInner );

//=================
// public functions

BOOL fpart_ModuleStartup( void ) {
	FASSERT( !_bModuleInited );

	_bModuleInited = TRUE;
	_bSetupOK = FALSE;

	_ResetTextureVars();

	CEmitter::nNumEmittersSpawned = 0;

#if _TRACK_MEM
	_nMaxEmittersUsed = 0;
	_nMaxLightsUsed = 0;
#endif

	return TRUE;
}

void fpart_ModuleShutdown( void ) {
	FASSERT( _bModuleInited );

	_bModuleInited = FALSE;
	_bSetupOK = FALSE;
}

// Allocates memory for particle emitters, does not load any textures.
BOOL fpart_InitSystem( u32 nMaxEmitters, u32 nMaxParticles ) {
	FResFrame_t ResFrame;

	FASSERT( _bModuleInited );
	FASSERT( !_bSetupOK );

	ResFrame = fres_GetFrame();

	// allocate memory for the emitter pool
	if( !_EmmiterPool.Create( nMaxEmitters ) ) {
		goto _ExitSetupWithError;
	}
	// allocate memory for the light pool
	if( !_LightPool.Create( ((nMaxEmitters * 3)>>2) ) ) {
		goto _ExitSetupWithError;
	}
	// allocate memory for the sprite pool
	if( !_SpritePool.Create( sizeof( FPSprite_t ), nMaxParticles, 16 ) ) {
		goto _ExitSetupWithError;
	}
	// allocate memory for the data pool (worst case assume 1-1 mapping with sprite pool)
	if( !_DataPool.Create( sizeof( _ParticleData_t ), nMaxParticles, 4 ) ) {
		goto _ExitSetupWithError;
	}
		
	CEmitter::nNumEmittersSpawned = 0;

	_bSetupOK = TRUE;

	return TRUE;

_ExitSetupWithError:
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

void fpart_UninitSystem( void ) {

	if( _bSetupOK ) {
		u32 nMaxEmitters = _EmmiterPool.GetSize();
		u32 nMaxLights = _LightPool.GetSize();

		_EmmiterPool.Destroy();
		_LightPool.Destroy();
		_bSetupOK = FALSE;

#if _TRACK_MEM
		DEVPRINTF( "fpart(): %d of %d Emitters used, %d of %d Lights used, potential saving = %d.\n", 
								_nMaxEmittersUsed,
								nMaxEmitters,
								_nMaxLightsUsed,
								nMaxLights,
								((nMaxEmitters - _nMaxEmittersUsed) * sizeof( CEmitter )) + ((nMaxLights - _nMaxLightsUsed) * sizeof( CLightEmitter )) );
#endif
	}
}

// Stops all active emitters and returns the system as if Setup() was just called.
// Unattaches any textures that may have been set.
void fpart_Reset( void ) {
	if( !_bSetupOK ) {
		return;
	}

	fpart_KillAllActiveParticles();
	
	_ResetTextureVars();
}

// Kills all active particles, but doesn't reset the system.
void fpart_KillAllActiveParticles( void ) {
	if( !_bSetupOK ) {
		return;
	}

	_EmmiterPool.KillAll();
	_LightPool.KillAll();

	FASSERT( _SpritePool.GetNumUsed() == 0 );
	FASSERT( _DataPool.GetNumUsed() == 0 );

	CEmitter::nNumEmittersSpawned = 0;
}

// Attaches texture(s) to a particle type, if this is not called, then solid colored particles will be emitted.
// This function must be called after Setup() or Reset().
BOOL fpart_SetTextures( FPart_Type_e nType, FTexDef_t **apTexturePalette, u32 nNumTextures ) {
	if( !_bSetupOK ) {
		return FALSE;
	}
	FASSERT( nType < FPART_TYPE_COUNT && nType != FPART_TYPE_UNASSIGNED );
	FASSERT( apTexturePalette );
	FASSERT( nNumTextures );

	u32 i;

	for( i=0; i < nNumTextures; i++ ) {
		_apTextures[nType][i] = apTexturePalette[i];
	}
	_anNumTexturesPerType[nType] = nNumTextures;

	return TRUE;
}

BOOL fpart_SpawnParticleEmitter( FPart_Type_e nType, f32 fUnitIntensity, const CFVec3A &rPos, 
								 const CFVec3A *pUnitDir/*=NULL*/, BOOL bSpawnLight/*=FALSE*/,
								 const CFVec3A *pEmitterVelPerSec_WS/*=NULL*/,
								 const CFColorRGB *prgbColorBias/* = NULL*/ ) {
	if( !_bSetupOK ) {
		return FALSE;
	}
	FASSERT( nType > FPART_TYPE_UNASSIGNED && nType < FPART_TYPE_COUNT );

	CEmitter *pEmitter;
	
	// grab a free emitter
	pEmitter = _EmmiterPool.GetAvailable();
	if( !pEmitter ) {
		return FALSE;
	}
	if( !pEmitter->Init( nType, fUnitIntensity, rPos, pUnitDir, bSpawnLight, pEmitterVelPerSec_WS, prgbColorBias ) ) {
		return FALSE;
	}
	
	return TRUE;	
}

//==================
// private functions

static void _ResetTextureVars( void ) {
	u32 i, j;

	for( i=0; i < FPART_TYPE_COUNT; i++ ) {
		for( j=0; j < _MAX_TEXTURES_PER_TYPE; j++ ) {
			_apTextures[i][j] = NULL;
		}
		_anNumTexturesPerType[i] = 0;
	}
}

static void _GetLocalXZUnitVec( const CFVec3 *pUnitDir, CFVec3 &rLocalX, CFVec3 &rLocalZ ) {
	f32 fDot;

	if( !pUnitDir ) {
		// assume an upward trajectory
		rLocalX.Set( 1.0f, 0.0f, 0.0f );
		rLocalZ.Set( 0.0f, 0.0f, 1.0f );
	} else {
		// compute the local x & z axis to the unitdir
		rLocalX.Set( 1.0f, 0.0f, 0.0f );
		fDot = rLocalX.Dot( *pUnitDir );
		if( fDot > 0.75f || fDot < -0.75f ) {
			// pick a different initial vector to cross with
			rLocalX.Set( 0.0f, 0.0f, 1.0f );
		}
		rLocalZ = pUnitDir->Cross( rLocalX );
		rLocalX = pUnitDir->Cross( rLocalZ );
	}
}

static void _RandomVectorInsideACone( CFVec3 &rResult, const CFVec3 &rLocalUnitX, const CFVec3 &rLocalUnitZ, 
									  f32 fInnerCone, f32 fOuterCone, BOOL bInner ) {
	f32 fRandX, fRandZ;

	if( bInner ) {
		fRandX = _GET_POS_NEG_RAND * fInnerCone;
		fRandZ = _GET_POS_NEG_RAND * fInnerCone;
	} else {
		fRandX = _GET_POS_NEG_RAND * fOuterCone;
		fRandZ = _GET_POS_NEG_RAND * fOuterCone;
	}
	rResult += rLocalUnitZ * fRandZ;
	rResult += rLocalUnitX * fRandX;
}

//////////////////////////
// CEmitter implementation
//////////////////////////

CEmitter::CEmitter() : CFWorldPSGroup() {
	m_nType = FPART_TYPE_UNASSIGNED;
	m_nMaxCount = 0;
	m_nDataCount = 0;
	m_pData = NULL;
	m_fTotalSecs = 0.0f;
	m_nNumFramesSinceBirth = 0;
	m_bLightAttached = FALSE;
	m_fLightRadiusMultiplier = 0.0f;
	m_fLightUnitIntensity = 0.0f;
	m_SampledWorldColor.White();
	m_nFrameNumLastKilled = -1;

	m_pFcnWork = CEmitter::Work;
}

BOOL CEmitter::SetupNewEmitter( f32 fUnitIntensity, u32 nMinParticles, u32 nMaxParticles, u32 nRandomParticles, u32 nMaxSpritesPerParticle ) {
	u32 nNumParticleDataNeeded;

	// determine how many particles we are going to have
	nNumParticleDataNeeded = (u32)FMATH_FPOT( fUnitIntensity, nMinParticles, nMaxParticles );
	nNumParticleDataNeeded += fmath_RandomChoice( nRandomParticles );
	// determine the max number of sprites that will be needed for this many particles
	m_nMaxCount = nNumParticleDataNeeded * nMaxSpritesPerParticle;
	m_nRenderCount = 0;
	m_nRenderStartIndex = 0;
	// grab some sprites from the pool
	m_pBase = (FPSprite_t *)_SpritePool.GetArray( m_nMaxCount );
	if( !m_pBase ) {
		return FALSE;
	}
	// grab some particle data
	m_nDataCount = nNumParticleDataNeeded;
	m_pData = (_ParticleData_t *)_DataPool.GetArray( m_nDataCount );
	if( !m_pData ) {
		_SpritePool.ReturnToPool( m_pBase, m_nMaxCount );
		m_pBase = NULL;
		return FALSE;
	}
	return TRUE;
}

BOOL CEmitter::Init( FPart_Type_e nType, f32 fUnitIntensity, const CFVec3A &rPos, 
					 const CFVec3A *pUnitDir/*=NULL*/, BOOL bSpawnLight/*=FALSE*/,
					 const CFVec3A *pEmitterVelPerSec_WS/*=NULL*/,
					 const CFColorRGB *prgbColorBias/* = NULL*/) {
	
	FASSERT( nType != FPART_TYPE_UNASSIGNED );
	FASSERT( m_nType == FPART_TYPE_UNASSIGNED );

	u32 i;
	_ParticleData_t *pParticleData;
	CFVec3 LocalX, LocalZ;
	FLightInit_t LightInit;

	m_fUnitIntensity = fUnitIntensity;

	if(prgbColorBias != NULL)
	{
		m_rgbColor = *prgbColorBias;
	}
	else
	{
		// They didn't tell us otherwise, so we won't change the color at all.
		m_rgbColor.White();
	}


	/////////////////////////////////////////////////////////////
	// create a local x & z unit vector from our direction vector
	/////////////////////////////////////////////////////////////
	_GetLocalXZUnitVec( &pUnitDir->v3, LocalX, LocalZ );	

	switch( nType ) {

	//////////////
	// SPARK BURST
	//////////////
	case FPART_TYPE_SPARK_MINIBURST:
	case FPART_TYPE_SPARK_BURST:
		if ( nType == FPART_TYPE_SPARK_BURST )
		{
			if( !SetupNewEmitter( fUnitIntensity, _MIN_NUM_SPARKS, _MAX_NUM_SPARKS, _RAND_SPARKS, _MAX_SPRITES_PER_SPARK ) ) {
				return FALSE;
			}
		}
		else
		{
			if( !SetupNewEmitter( fUnitIntensity, 1, (_MAX_NUM_SPARKS / 3) , 3, _MAX_SPRITES_PER_SPARK ) ) {
				return FALSE;
			}
		}
		// setup some of the CFPSpriteGroup's variables
		m_fCullDist = _SPARK_CULL_DIST;
		m_nPSFlags = FPSPRITE_FLAG_NONE;
		m_nBlend = _SPARK_BLEND;

		m_fGravityPerSec = _SPARK_GRAVITY;
		m_nFadeType = FADE_TYPE_PERCENT_CUBED;

		//////////////////
		// emit the sparks
		//////////////////
		for( i=0; i < m_nDataCount; i++ ) {
			pParticleData = &m_pData[i];
			
			if( !pUnitDir ) {
				pParticleData->VelocityPerSec.Set( 0.0f, 1.0f, 0.0f );
			} else {
				pParticleData->VelocityPerSec = pUnitDir->v3;
			}
			// randomize the trajectory
			_RandomVectorInsideACone( pParticleData->VelocityPerSec, LocalX, LocalZ, 
									  _SPARK_INNER_CONE, _SPARK_OUTTER_CONE,
									  (s32)( i & 0x1 ) );
			pParticleData->VelocityPerSec.Unitize();
			pParticleData->VelocityPerSec *= ((fUnitIntensity * 0.5f) + _ASSIGN_SPARK_INITIAL_VEL);

			if( pEmitterVelPerSec_WS ) {
				pParticleData->VelocityPerSec += pEmitterVelPerSec_WS->v3;
			}

			pParticleData->fOpacity = 1.0f;
			pParticleData->fSecsToFade = _ASSIGN_SPARK_FADE;
			
			pParticleData->fSize = _ASSIGN_SPARK_SIZE;
			pParticleData->fGrowthPerSec = _ASSIGN_SPARK_GROWTH;

			pParticleData->nNumSprites = 2;

			pParticleData->fPosStepMultiplier = -1.0f;
			pParticleData->fScaleMultiplier = 0.0175f;
			pParticleData->fOpacityMultiplier = 0.025f;

			pParticleData->Pos = pParticleData->VelocityPerSec * _ASSIGN_SPARK_POS;
			
			// set the sprites for this spark
			m_pBase[m_nRenderCount].fDim_MS = pParticleData->fSize;
			m_pBase[m_nRenderCount].Point_MS = pParticleData->Pos;
			m_pBase[m_nRenderCount].ColorRGBA.Set( _SPARK_R_OVER_TIME( 0.0f ), 
													   _SPARK_G_OVER_TIME( 0.0f ), 
													   _SPARK_B_OVER_TIME( 0.0f ),
													   1.0f );
			m_nRenderCount++;
		}
		break;

	//////////////
	// SMOKE PUFF
	//////////////
	case FPART_TYPE_SMOKE_PUFF:
		if( !SetupNewEmitter( fUnitIntensity, _MIN_NUM_PUFFS, _MAX_NUM_PUFFS, _RAND_PUFFS, 1 ) ) {
			return FALSE;
		}
		// setup some of the CFPSpriteGroup's variables
		m_fCullDist = _PUFF_CULL_DIST;
		m_nPSFlags = FPSPRITE_FLAG_NONE;//FPSPRITE_FLAG_SORT;
		m_nBlend = _PUFF_BLEND;

		m_fGravityPerSec = _PUFF_Y_RISE_PER_SEC;
		m_nFadeType = FADE_TYPE_PERCENT_LINEAR;

		//////////////////
		// emit the puffs
		//////////////////
		fworld_LightPoint( &rPos, &m_SampledWorldColor, TRUE );

		for( i=0; i < m_nDataCount; i++ ) {
			pParticleData = &m_pData[i];
			
			if( !pUnitDir ) {
				pParticleData->VelocityPerSec.Set( 0.0f, 1.0f, 0.0f );
			} else {
				pParticleData->VelocityPerSec = pUnitDir->v3;
			}
			// randomize the trajectory
			_RandomVectorInsideACone( pParticleData->VelocityPerSec, LocalX, LocalZ, 
									  _PUFF_INNER_CONE, _PUFF_OUTTER_CONE,
									  (s32)( i & 0x1 ) );			
			pParticleData->VelocityPerSec.Unitize();
			pParticleData->VelocityPerSec *= (fUnitIntensity + _ASSIGN_PUFF_INITIAL_VEL);

			if( pEmitterVelPerSec_WS ) {
				pParticleData->VelocityPerSec += pEmitterVelPerSec_WS->v3;
			}

			pParticleData->fOpacity = _ASSIGN_PUFF_OPACITY;
			pParticleData->fSecsToFade = _ASSIGN_PUFF_FADE( fUnitIntensity );
			
			pParticleData->fSize = _ASSIGN_PUFF_SIZE + (fUnitIntensity * 0.05f);
			pParticleData->fGrowthPerSec = _ASSIGN_PUFF_GROWTH;

			pParticleData->nNumSprites = 1;
			pParticleData->XZChange.Zero();

			pParticleData->Pos = pParticleData->VelocityPerSec * _ASSIGN_PUFF_POS;
						
			// set the sprites for this puff
			m_pBase[m_nRenderCount].fDim_MS = pParticleData->fSize;
			m_pBase[m_nRenderCount].Point_MS = pParticleData->Pos;
			m_pBase[m_nRenderCount].ColorRGBA.Set( m_SampledWorldColor, pParticleData->fOpacity ); 
			m_nRenderCount++;
		}
		break;

	//////////////
	// STEAM
	//////////////
	case FPART_TYPE_STEAM:
		if( !SetupNewEmitter( fUnitIntensity, 24, 24, 0, 1 ) ) 
		{
			return FALSE;
		}
		// setup some of the CFPSpriteGroup's variables
		m_fCullDist = _PUFF_CULL_DIST;
		m_nPSFlags = FPSPRITE_FLAG_NONE;//FPSPRITE_FLAG_SORT;
		m_nBlend = _PUFF_BLEND;

		m_fGravityPerSec = ((f32)_PUFF_Y_RISE_PER_SEC * 1.5f);
		m_nFadeType = FADE_TYPE_PERCENT_LINEAR;
		
		m_nRenderCount = 0;

		fworld_LightPoint( &rPos, &m_SampledWorldColor, TRUE );

		// Setup one sprite so that we don't have issues with the tracker (size/radius)
		{
			_ParticleData_t *pSpawnParticle = &m_pData[0];
			
			pSpawnParticle->VelocityPerSec.Set( 0.0f, 1.0f, 0.0f );
			
			CFVec3 vLocalX( 1.f, 0.f, 0.f ), vLocalZ( 0.f, 0.f, 1.f );

			// randomize the trajectory
			_RandomVectorInsideACone( pSpawnParticle->VelocityPerSec, vLocalX, vLocalZ, 
									  _PUFF_INNER_CONE, _PUFF_OUTTER_CONE * 2,
									  (s32)( 0x1 ) );			
			pSpawnParticle->VelocityPerSec.Unitize();
			pSpawnParticle->VelocityPerSec *= (m_fUnitIntensity + _ASSIGN_PUFF_INITIAL_VEL);

			pSpawnParticle->fOpacity = _ASSIGN_PUFF_OPACITY;
			pSpawnParticle->fBirthTime = 0.f;
			pSpawnParticle->fSecsToFade = fmath_Inv( _ASSIGN_PUFF_FADE( m_fUnitIntensity ) * 0.7f );
			
			
			pSpawnParticle->fSize = (_ASSIGN_PUFF_SIZE / 2) + (m_fUnitIntensity * 0.05f);
			pSpawnParticle->fGrowthPerSec = _ASSIGN_PUFF_GROWTH;

			pSpawnParticle->nNumSprites = 1;
			pSpawnParticle->XZChange.Zero();

			pSpawnParticle->Pos = pSpawnParticle->VelocityPerSec * _ASSIGN_PUFF_POS;

			// set the sprites for this puff
			m_pBase[m_nRenderCount].fDim_MS = pSpawnParticle->fSize;
			m_pBase[m_nRenderCount].Point_MS = pSpawnParticle->Pos;
			m_pBase[m_nRenderCount].ColorRGBA.Set( m_SampledWorldColor, pSpawnParticle->fOpacity ); 
			m_nRenderCount++;
			
			m_fNextSpawn = 0.07f + (fmath_RandomUnitFloatLessThan1() * 0.05f);
			
			for ( i = 1; i < m_nDataCount; i++ ) 
			{
				m_pData[i].fBirthTime = 0.f;
				m_pData[i].fOpacity = 0.f;
			}
		}

		break;

	//////////////////
	// SPARK EXPLOSION
	//////////////////
	case FPART_TYPE_SPARK_EXPLOSION:
		if( !SetupNewEmitter( fUnitIntensity, 17, 34, 8, _MAX_SPRITES_PER_SPARK ) ) {// 20, 37, 10
			return FALSE;
		}
		// setup some of the CFPSpriteGroup's variables
		m_fCullDist = _SPARK_CULL_DIST;
		m_nPSFlags = FPSPRITE_FLAG_NONE;
		m_nBlend = _SPARK_BLEND;

		m_fGravityPerSec = _SPARK_GRAVITY * 0.415f;
		m_nFadeType = FADE_TYPE_PERCENT_SQUARED;

		//////////////////
		// emit the sparks
		//////////////////
		for( i=0; i < m_nDataCount; i++ ) {
			pParticleData = &m_pData[i];
			
			// randomize the trajectory - much larger coverage area
			if( !pUnitDir ) {
				pParticleData->VelocityPerSec.Set( 0.0f, 1.0f, 0.0f );
			} else {
				pParticleData->VelocityPerSec = pUnitDir->v3;
			}
			_RandomVectorInsideACone( pParticleData->VelocityPerSec, LocalX, LocalZ, 
									  (_SPARK_INNER_CONE + 0.40f), (_SPARK_OUTTER_CONE + 0.75f),
									  (s32)( i & 0x1 ) );
			pParticleData->VelocityPerSec.Unitize();
			pParticleData->VelocityPerSec *= ( (fUnitIntensity * 6.0f) + ( FMATH_FPOT( fmath_RandomFloat(), 15.5f, 18.0f ) ) );// higher velocity

			if( pEmitterVelPerSec_WS ) {
				pParticleData->VelocityPerSec += pEmitterVelPerSec_WS->v3;
			}

			pParticleData->fOpacity = 1.0f;
			pParticleData->fSecsToFade = ( 1.0f / FMATH_FPOT( fmath_RandomFloat(), 0.20f, 0.45f ) );// last shorter
			
			pParticleData->fSize = (_ASSIGN_SPARK_SIZE * 1.85f);// start larger
			pParticleData->fGrowthPerSec = _ASSIGN_SPARK_GROWTH;

			pParticleData->nNumSprites = 2;

			pParticleData->fPosStepMultiplier = -2.10f;
			pParticleData->fScaleMultiplier = 0.03f;
			pParticleData->fOpacityMultiplier = 0.02f;

			pParticleData->Pos = pParticleData->VelocityPerSec * 4.5f * _ASSIGN_SPARK_POS;// more random location
			
			// set the sprites for this spark
			m_pBase[m_nRenderCount].fDim_MS = pParticleData->fSize;
			m_pBase[m_nRenderCount].Point_MS = pParticleData->Pos;
			m_pBase[m_nRenderCount].ColorRGBA.Set( _SPARK_R_OVER_TIME( 0.0f ), 
													   _SPARK_G_OVER_TIME( 0.0f ), 
													   _SPARK_B_OVER_TIME( 0.0f ),
													   1.0f );
			m_nRenderCount++;
		}
		break;

	//////////////
	// SMOKE CLOUD
	//////////////
	case FPART_TYPE_SMOKE_CLOUD:
		if( !SetupNewEmitter( fUnitIntensity, 18, 31, 15, 1 ) ) {//20, 33, 17
			return FALSE;
		}
		// setup some of the CFPSpriteGroup's variables
		m_fCullDist = _PUFF_CULL_DIST * 1.5f;// seen further away
		m_nPSFlags = FPSPRITE_FLAG_NONE;//FPSPRITE_FLAG_SORT;
		m_nBlend = _PUFF_BLEND;

		m_fGravityPerSec = _PUFF_Y_RISE_PER_SEC * 1.03f;
		m_nFadeType = FADE_TYPE_PERCENT_LINEAR;

		//////////////////
		// emit the puffs
		//////////////////
		fworld_LightPoint( &rPos, &m_SampledWorldColor, TRUE );
		for( i=0; i < m_nDataCount; i++ ) {
			pParticleData = &m_pData[i];
			
			if( !pUnitDir ) {
				pParticleData->VelocityPerSec.Set( 0.0f, 3.0f, 0.0f );
			} else {
				pParticleData->VelocityPerSec = pUnitDir->v3 * 3.0f;
			}
			// randomize the trajectory - larger cloud
			_RandomVectorInsideACone( pParticleData->VelocityPerSec, LocalX, LocalZ, 
									  (_PUFF_INNER_CONE * -7.0f), (_PUFF_OUTTER_CONE * 8.0f),
									  (s32)( i & 0x1 ) );			
			pParticleData->VelocityPerSec.Unitize();
			pParticleData->VelocityPerSec *= ((fUnitIntensity * 1.75f) + (3.75f * _ASSIGN_PUFF_INITIAL_VEL));// faster

			if( pEmitterVelPerSec_WS ) {
				pParticleData->VelocityPerSec += pEmitterVelPerSec_WS->v3;
			}

			pParticleData->fOpacity = _ASSIGN_PUFF_OPACITY;
			pParticleData->fSecsToFade = ( 1.0f / (0.91f + (fmath_RandomBipolarUnitFloat() * 0.035f)) );//0.0125f)) );				
			
			pParticleData->fSize = (_ASSIGN_PUFF_SIZE * 2.8f) + (fUnitIntensity * 0.5f);// larger
			pParticleData->fGrowthPerSec = _ASSIGN_PUFF_GROWTH * 6.89f;// get larger

			pParticleData->nNumSprites = 1;
			pParticleData->XZChange.Zero();

			pParticleData->Pos = pParticleData->VelocityPerSec * _ASSIGN_PUFF_POS * 4.0f;// more random pos
			
			// set the sprites for this puff
			m_pBase[m_nRenderCount].fDim_MS = pParticleData->fSize;
			m_pBase[m_nRenderCount].Point_MS = pParticleData->Pos;
			m_pBase[m_nRenderCount].ColorRGBA.Set( m_SampledWorldColor, pParticleData->fOpacity ); 
			m_nRenderCount++;
		}
		break;

	/////////////
	// STEAM PUFF
	/////////////
	case FPART_TYPE_STEAM_PUFF:
		if( !SetupNewEmitter( fUnitIntensity, 5, 18, 7, 1 ) ) {
			return FALSE;
		}
		// setup some of the CFPSpriteGroup's variables
		m_fCullDist = _PUFF_CULL_DIST;
		m_nPSFlags = FPSPRITE_FLAG_NONE;//FPSPRITE_FLAG_SORT;
		m_nBlend = _PUFF_BLEND;

		m_fGravityPerSec = _PUFF_Y_RISE_PER_SEC;
		m_nFadeType = FADE_TYPE_PERCENT_LINEAR;

		//////////////////
		// emit the puffs
		//////////////////
		fworld_LightPoint( &rPos, &m_SampledWorldColor, TRUE );
		for( i=0; i < m_nDataCount; i++ ) {
			pParticleData = &m_pData[i];
			
			if( !pUnitDir ) {
				pParticleData->VelocityPerSec.Set( 0.0f, 1.0f, 0.0f );
			} else {
				pParticleData->VelocityPerSec = pUnitDir->v3;
			}
			// randomize the trajectory
			_RandomVectorInsideACone( pParticleData->VelocityPerSec, LocalX, LocalZ, 
									  0.5f, 2.0f,
									  (s32)( i & 0x1 ) );			
			pParticleData->VelocityPerSec.Unitize();
			pParticleData->VelocityPerSec *= ( (fUnitIntensity * 1.4f) + (_ASSIGN_PUFF_INITIAL_VEL * 2.0f) );

			if( pEmitterVelPerSec_WS ) {
				pParticleData->VelocityPerSec += pEmitterVelPerSec_WS->v3;
			}

			pParticleData->fOpacity = 0.145f;
			pParticleData->fSecsToFade = ( 1.0f / ((FMATH_FPOT( fmath_RandomFloat(), 0.20f, 0.35f )) + (fUnitIntensity * 0.20f) ) );
			
			pParticleData->fSize = (_ASSIGN_PUFF_SIZE * 5.0f) + (fUnitIntensity * 0.5f);// bigger
			pParticleData->fGrowthPerSec = _ASSIGN_PUFF_GROWTH * 3.0f;

			pParticleData->nNumSprites = 1;
			pParticleData->XZChange.Zero();

			pParticleData->Pos = pParticleData->VelocityPerSec * _ASSIGN_PUFF_POS;
						
			// set the sprites for this puff
			m_pBase[m_nRenderCount].fDim_MS = pParticleData->fSize;
			m_pBase[m_nRenderCount].Point_MS = pParticleData->Pos;
			m_pBase[m_nRenderCount].ColorRGBA.Set( m_SampledWorldColor, pParticleData->fOpacity ); 
			m_nRenderCount++;
		}
		break;

	///////////////
	// PIXEL DUST 1
	///////////////
	case FPART_TYPE_PIXEL_DUST1:
		if( !SetupNewEmitter( fUnitIntensity, _MIN_NUM_PIXEL_DUST1S, _MAX_NUM_PIXEL_DUST1S, _RAND_PIXEL_DUST1S, 1 ) ) {
			return FALSE;
		}
		// setup some of the CFPSpriteGroup's variables
		m_fCullDist = _PIXEL_DUST1_CULL_DIST;
		m_nPSFlags = FPSPRITE_FLAG_NONE;
		m_nBlend = _PIXEL_DUST1_BLEND;

		m_fGravityPerSec = _PIXEL_DUST1_GRAVITY;
		m_nFadeType = FADE_TYPE_PERCENT_LINEAR_LAST_10;

		//////////////////////
		// emit the pixel dust
		//////////////////////
		for( i=0; i < m_nDataCount; i++ ) {
			pParticleData = &m_pData[i];
			
			if( !pUnitDir ) {
				pParticleData->VelocityPerSec.Set( 0.0f, 1.0f, 0.0f );
			} else {
				pParticleData->VelocityPerSec = pUnitDir->v3;
			}
			// randomize the trajectory
			_RandomVectorInsideACone( pParticleData->VelocityPerSec, LocalX, LocalZ, 
									  _PIXEL_DUST1_INNER_CONE, _PIXEL_DUST1_OUTTER_CONE,
									  (s32)( i & 0x1 ) );
			pParticleData->VelocityPerSec.Unitize();
			pParticleData->VelocityPerSec *= ((fUnitIntensity * 0.5f) + _ASSIGN_PIXEL_DUST1_INITIAL_VEL);

			if( pEmitterVelPerSec_WS ) {
				pParticleData->VelocityPerSec += pEmitterVelPerSec_WS->v3;
			}

			pParticleData->fOpacity = 1.0f;
			pParticleData->fSecsToFade = _ASSIGN_PIXEL_DUST1_FADE;
			
			pParticleData->fSize = _ASSIGN_PIXEL_DUST1_SIZE;
			pParticleData->fGrowthPerSec = (_PIXEL_DUST1_MAX_SIZE - pParticleData->fSize);

			pParticleData->nNumSprites = 1;
			pParticleData->Pos = pParticleData->VelocityPerSec * _ASSIGN_PIXEL_DUST1_POS;
			
			// set the sprites for this pixel
			m_pBase[m_nRenderCount].fDim_MS = pParticleData->fSize;
			m_pBase[m_nRenderCount].Point_MS = pParticleData->Pos;
			m_pBase[m_nRenderCount].ColorRGBA.Set( _PIXEL_DUST1_R_OVER_TIME( 0.0f, m_rgbColor.fRed ), 
												   _PIXEL_DUST1_G_OVER_TIME( 0.0f, m_rgbColor.fGreen ), 
												   _PIXEL_DUST1_B_OVER_TIME( 0.0f, m_rgbColor.fBlue ),
												   1.0f );
			m_nRenderCount++;
		}
		break;

	////////////
	// DUST PUFF
	////////////
	case FPART_TYPE_DUST_PUFF:
		if( !SetupNewEmitter( fUnitIntensity, _MIN_NUM_PUFFS, _MAX_NUM_PUFFS, _RAND_PUFFS, 1 ) ) {
			return FALSE;
		}
		// setup some of the CFPSpriteGroup's variables
		m_fCullDist = _PUFF_CULL_DIST;
		m_nPSFlags = FPSPRITE_FLAG_NONE;//FPSPRITE_FLAG_SORT;
		m_nBlend = _PUFF_BLEND;

		m_fGravityPerSec = _PUFF_Y_RISE_PER_SEC;
		m_nFadeType = FADE_TYPE_PERCENT_LINEAR;

		//////////////////
		// emit the puffs
		//////////////////
		fworld_LightPoint( &rPos, &m_SampledWorldColor, TRUE );

		for( i=0; i < m_nDataCount; i++ ) {
			pParticleData = &m_pData[i];
			
			if( !pUnitDir ) {
				pParticleData->VelocityPerSec.Set( 0.0f, 1.0f, 0.0f );
			} else {
				pParticleData->VelocityPerSec = pUnitDir->v3;
			}
			// randomize the trajectory
			_RandomVectorInsideACone( pParticleData->VelocityPerSec, LocalX, LocalZ, 
									  (_PUFF_INNER_CONE + 0.20f), (_PUFF_OUTTER_CONE + 0.85f),
									  (s32)( i & 0x1 ) );			
			pParticleData->VelocityPerSec.Unitize();
			pParticleData->VelocityPerSec *= ((fUnitIntensity + _ASSIGN_PUFF_INITIAL_VEL) * 0.8f);

			if( pEmitterVelPerSec_WS ) {
				pParticleData->VelocityPerSec += pEmitterVelPerSec_WS->v3;
			}

			pParticleData->fOpacity = _ASSIGN_PUFF_OPACITY * 0.70f;
			pParticleData->fSecsToFade = _ASSIGN_PUFF_FADE( fUnitIntensity ) * 0.95f;
			
			pParticleData->fSize = (_ASSIGN_PUFF_SIZE + (fUnitIntensity * 0.05f)) * 0.85f;
			pParticleData->fGrowthPerSec = (_ASSIGN_PUFF_GROWTH) * 0.90f;

			pParticleData->nNumSprites = 1;
			pParticleData->XZChange.Zero();

			pParticleData->Pos = pParticleData->VelocityPerSec * _ASSIGN_PUFF_POS;
						
			// set the sprites for this puff
			m_pBase[m_nRenderCount].fDim_MS = pParticleData->fSize;
			m_pBase[m_nRenderCount].Point_MS = pParticleData->Pos;
			m_pBase[m_nRenderCount].ColorRGBA.Set( m_SampledWorldColor, pParticleData->fOpacity ); 
			m_nRenderCount++;
		}
		break;

	////////////
	// PIXIE PUFF
	////////////
	case FPART_TYPE_PIXIE_PUFF:
		if( !SetupNewEmitter( fUnitIntensity, 5, 18, 7, 1 ) ) {
			return FALSE;
		}
		// setup some of the CFPSpriteGroup's variables
		m_fCullDist = _PIXIE_PUFF_CULL_DIST;
		m_nPSFlags = FPSPRITE_FLAG_NONE;//FPSPRITE_FLAG_SORT;
		m_nBlend = _PIXIE_PUFF_BLEND;

		m_fGravityPerSec = _PIXIE_PUFF_Y_RISE_PER_SEC;
		m_nFadeType = FADE_TYPE_PERCENT_LINEAR;

		//////////////////
		// emit the puffs
		//////////////////
		fworld_LightPoint( &rPos, &m_SampledWorldColor, TRUE );
		for( i=0; i < m_nDataCount; i++ ) {
			pParticleData = &m_pData[i];
			
			if( !pUnitDir ) {
				pParticleData->VelocityPerSec.Set( 0.0f, 1.0f, 0.0f );
			} else {
				pParticleData->VelocityPerSec = pUnitDir->v3;
			}
			// randomize the trajectory
			_RandomVectorInsideACone( pParticleData->VelocityPerSec, LocalX, LocalZ, 
									  0.5f, 2.0f,
									  (s32)( i & 0x1 ) );			
			pParticleData->VelocityPerSec.Unitize();
			pParticleData->VelocityPerSec *= ( (fUnitIntensity * 1.4f) + (_ASSIGN_PIXIE_PUFF_INITIAL_VEL * 2.0f) );

			if( pEmitterVelPerSec_WS ) {
				pParticleData->VelocityPerSec += pEmitterVelPerSec_WS->v3;
			}

			pParticleData->fOpacity = 0.145f;
			pParticleData->fSecsToFade = ( 1.0f / ((FMATH_FPOT( fmath_RandomFloat(), 0.20f, 0.35f )) + (fUnitIntensity * 0.20f) ) );
			
			pParticleData->fSize = (_ASSIGN_PIXIE_PUFF_SIZE * 5.0f) + (fUnitIntensity * 0.5f);// bigger
			pParticleData->fGrowthPerSec = _ASSIGN_PIXIE_PUFF_GROWTH * 3.0f;

			pParticleData->nNumSprites = 1;
			pParticleData->XZChange.Zero();

			pParticleData->Pos = pParticleData->VelocityPerSec * _ASSIGN_PIXIE_PUFF_POS;
						
			// set the sprites for this puff
			m_pBase[m_nRenderCount].fDim_MS = pParticleData->fSize;
			m_pBase[m_nRenderCount].Point_MS = pParticleData->Pos;
//			m_pBase[m_nRenderCount].ColorRGBA.Set( m_SampledWorldColor, pParticleData->fOpacity ); 
			// JUSTIN: Change this to be pixel puff stuff.
			m_pBase[m_nRenderCount].ColorRGBA.Set( _PIXEL_DUST1_R_OVER_TIME( 0.0f, m_rgbColor.fRed ), 
												   _PIXEL_DUST1_G_OVER_TIME( 0.0f, m_rgbColor.fGreen ), 
												   _PIXEL_DUST1_B_OVER_TIME( 0.0f, m_rgbColor.fBlue ),
												   1.0f );
			m_nRenderCount++;
		}
		break;

	default:
		FASSERT_NOW;
		break;
	}

	//////////////////////////////////////////////
	// translate our particle group in world space
	//////////////////////////////////////////////
	m_Xfm.BuildTranslation( rPos );

	//////////////////////////////////
	// assign a texture to the emitter
	//////////////////////////////////
	u32 nNumTextures = _anNumTexturesPerType[nType];
	if( nNumTextures ) {
		m_TexInst.SetTexDef( _apTextures[nType][ fmath_RandomChoice( nNumTextures ) ] );
	} else {
		m_TexInst.SetTexDef( NULL );
	}
	
	//////////////////////////////
	// compute the bounding sphere
	//////////////////////////////
	ComputeBoundingSphere();

	//////////////////////////////////////////////////
	// update the tracker (which adds it to the world)
	//////////////////////////////////////////////////
	UpdateTracker();
	
	/////////////////////////////////////////////////////////////////////////////////////
	// now that the tracker is setup, we can use the bounding sphere to help setup lights
	/////////////////////////////////////////////////////////////////////////////////////
	if( bSpawnLight ) {
		switch( nType ) {

		case FPART_TYPE_SPARK_MINIBURST:
		case FPART_TYPE_SPARK_BURST:
			m_fLightRadiusMultiplier = 2.0f;
			m_fLightUnitIntensity = 1.0f;
			// Create a light...
			LightInit.nLightID = 0xffff;
			LightInit.szPerPixelTexName[0] = 0;
			LightInit.szCoronaTexName[0] = 0;
			LightInit.mtxOrientation.Identity();
			LightInit.fIntensity = 1.0f;
			LightInit.fSpotInnerRadians = 0.1f;
			LightInit.fSpotOuterRadians = 0.2f;
			LightInit.Motif.Set( 1.0f, 0.55f, 0.25f, 1.0f );
			LightInit.nFlags = FLIGHT_FLAG_HASPOS | FLIGHT_FLAG_PER_PIXEL;
			LightInit.nType = FLIGHT_TYPE_OMNI;
			LightInit.spInfluence = GetBoundingSphere();
			LightInit.spInfluence.m_fRadius *= m_fLightRadiusMultiplier;
			LightInit.szName[0] = 0;
			// see if this light can be combined with an existing light
			m_bLightAttached = _LightPool.AddOrCombineALight( LightInit, nType, this );			
			break;

		case FPART_TYPE_PIXEL_DUST1:			
		case FPART_TYPE_STEAM_PUFF:
		case FPART_TYPE_SPARK_EXPLOSION:
		case FPART_TYPE_SMOKE_CLOUD:		
		case FPART_TYPE_SMOKE_PUFF:
		case FPART_TYPE_STEAM:
		case FPART_TYPE_DUST_PUFF:
		case FPART_TYPE_PIXIE_PUFF:
			// no lights for these types
			m_bLightAttached = FALSE;
			break;

		default:
			FASSERT_NOW;
			break;
		}
	} else {
		m_bLightAttached = FALSE;
	}
	/////////////////////////////////////////////////////
	// since everything is ok, flag this emitter has used
	/////////////////////////////////////////////////////
	m_nType = nType;
	m_fTotalSecs = 0.0f;
	m_nNumFramesSinceBirth = (CEmitter::nNumEmittersSpawned + FVid_nFrameCounter) & 0x7;

	CEmitter::nNumEmittersSpawned++;

	return TRUE;
}

void CEmitter::Work( CFWorldTracker *pTracker ) {
	CEmitter *pEmitter = (CEmitter *)pTracker;
	BOOL bKillEmitter;

	if( FLoop_bGamePaused ) {
		return;
	}
	// advance our timer
	pEmitter->m_fTotalSecs += FLoop_fPreviousLoopSecs;
	pEmitter->m_nNumFramesSinceBirth++;

	switch( pEmitter->m_nType ) {

	default:
	case FPART_TYPE_UNASSIGNED:
		FASSERT_NOW;
		bKillEmitter = TRUE;
		break;

	case FPART_TYPE_SPARK_BURST:
	case FPART_TYPE_SPARK_MINIBURST:
	case FPART_TYPE_SPARK_EXPLOSION:
		bKillEmitter = pEmitter->SparkWorkFunction( pEmitter, FLoop_fPreviousLoopSecs );
		break;

	case FPART_TYPE_STEAM:
		bKillEmitter = pEmitter->SteamWorkFunction( pEmitter, FLoop_fPreviousLoopSecs );
		break;

	case FPART_TYPE_PIXIE_PUFF:
		bKillEmitter = pEmitter->PixiePuffWorkFunction( pEmitter, FLoop_fPreviousLoopSecs );
		break;

	case FPART_TYPE_STEAM_PUFF:
	case FPART_TYPE_SMOKE_PUFF:
	case FPART_TYPE_SMOKE_CLOUD:
	case FPART_TYPE_DUST_PUFF:
		bKillEmitter = pEmitter->SmokeWorkFunction( pEmitter, FLoop_fPreviousLoopSecs );
		break;

	case FPART_TYPE_PIXEL_DUST1:
		bKillEmitter = pEmitter->PixelDustWorkFunction( pEmitter, FLoop_fPreviousLoopSecs );
		break;
	}

	if( bKillEmitter ) {
		pEmitter->Kill();
	}
}

void CEmitter::Kill() {

	if( InUse() ) {
		// remove from world
		RemoveFromWorld();
		// release the sprites
		_SpritePool.ReturnToPool( m_pBase, m_nMaxCount );
		m_pBase = NULL;
		m_nMaxCount = 0;
		// release the data elements
		_DataPool.ReturnToPool( m_pData, m_nDataCount );
		m_pData = NULL;
		m_nDataCount = 0;
		// mark emitter as not used anymore
		m_nType = FPART_TYPE_UNASSIGNED;
		m_fTotalSecs = 0.0f;
		m_nNumFramesSinceBirth = 0;
		m_bLightAttached = FALSE;
		m_fLightUnitIntensity = 0.0f;
		m_fLightRadiusMultiplier = 0.0f;
		m_SampledWorldColor.White();
		m_nFrameNumLastKilled = (s32)FVid_nFrameCounter;		
	}
}

f32 CEmitter::ComputeOpacity( CEmitter *pEmitter, f32 fPercentIntoFade ) {
	f32 fFadedOff;

	switch( pEmitter->m_nFadeType ) {
	default:
		FASSERT_NOW;
		// pass through to linear if release mode
	case FADE_TYPE_PERCENT_LINEAR:
		fFadedOff = fPercentIntoFade;
		break;
	case FADE_TYPE_PERCENT_SQUARED:
		fFadedOff = fPercentIntoFade * fPercentIntoFade;
		break;
	case FADE_TYPE_PERCENT_CUBED:
		fFadedOff = fPercentIntoFade * fPercentIntoFade * fPercentIntoFade;
		break;
	case FADE_TYPE_PERCENT_LINEAR_LAST_10:
		if( fPercentIntoFade >= 0.90f ) {
			fPercentIntoFade -= 0.90f;
			fFadedOff = fPercentIntoFade * (1.0f/0.90f);
		} else {
			fFadedOff = 0.0f;
		}
		break;
	}
	return (1.0f - fFadedOff);
}

BOOL CEmitter::SparkWorkFunction( CEmitter *pEmitter, f32 fSecsSinceLastWork ) {
	u32 i, j;
	_ParticleData_t *pSparkData;
	f32 fGravity, fVelMag2, fAmountPerStep, fOpacMulti, fScaleMulti, 
		fUnitOpacitySum, fOpacityCount, fPercentIntoFade;
	FPSprite_t *pSprites;
	CFVec3 StepSize, SpritePos;

	//////////////////////////////////////
	// compute the gravity since last work
	//////////////////////////////////////
	fGravity = fSecsSinceLastWork * pEmitter->m_fGravityPerSec;

	/////////////////////////
	// walk all of the sparks
	/////////////////////////
	fUnitOpacitySum = fOpacityCount = 0.0f;
	pSparkData = pEmitter->m_pData;
	pSprites = pEmitter->m_pBase;
	for( i=0; i < pEmitter->m_nDataCount; i++ ) {
		//////////////////////
		// if the spark is on
		/////////////////////
		if( pSparkData->fOpacity > 0.0f ) {
			/////////////////
			// fade the spark
			/////////////////
			fPercentIntoFade = (pEmitter->m_fTotalSecs * pSparkData->fSecsToFade);
			pSparkData->fOpacity = pEmitter->ComputeOpacity( pEmitter, fPercentIntoFade );

			if( pSparkData->fOpacity > 0.0f ) {
				fUnitOpacitySum += pSparkData->fOpacity;
				fOpacityCount += 1.0f;

				////////////////////////
				// grow/shrink the spark
				////////////////////////
				pSparkData->fSize += pSparkData->fGrowthPerSec * fSecsSinceLastWork;
				if( pSparkData->fSize > 0.0f ) {
					/////////////////
					// move the spark
					/////////////////
					pSparkData->Pos += pSparkData->VelocityPerSec * fSecsSinceLastWork;
					///////////////////////////////
					// add gravity to the velocity
					//////////////////////////////
					pSparkData->VelocityPerSec.y += fGravity;

					//////////////////////////////////////////////////////
					// determine how many sprite this particle should have
					//////////////////////////////////////////////////////
					fVelMag2 = pSparkData->VelocityPerSec.Mag2();
					if( fVelMag2 < _2_SPARK_VEL2 ) {
						pSparkData->nNumSprites = 2;
						fAmountPerStep = (_SPARK_STEP_PERCENT/2.0f);
					} else if( fVelMag2 < _3_SPARK_VEL2 ) {
						pSparkData->nNumSprites = 3;
						fAmountPerStep = (_SPARK_STEP_PERCENT/3.0f);
					} else if( fVelMag2 < _4_SPARK_VEL2 ) {
						pSparkData->nNumSprites = 4;
						fAmountPerStep = (_SPARK_STEP_PERCENT/4.0f);
					} else if( fVelMag2 < _5_SPARK_VEL2 ) {
						pSparkData->nNumSprites = 5;
						fAmountPerStep = (_SPARK_STEP_PERCENT/5.0f);
					} else {
						pSparkData->nNumSprites = 6;
						fAmountPerStep = (_SPARK_STEP_PERCENT/6.0f);
					}
					StepSize = pSparkData->VelocityPerSec * (fAmountPerStep * pSparkData->fPosStepMultiplier);
					SpritePos = pSparkData->Pos;
					
					///////////////////////////////////
					// setup the sprites for this spark
					//////////////////////////////////
					for( j=0; j < pSparkData->nNumSprites; j++ ) {
						fOpacMulti = 1.0f - (j * pSparkData->fOpacityMultiplier);
						fScaleMulti = 1.0f - (j * pSparkData->fScaleMultiplier);

						pSprites->fDim_MS = pSparkData->fSize * fScaleMulti;
						pSprites->Point_MS = SpritePos;

						pSprites->ColorRGBA.Set( _SPARK_R_OVER_TIME( fPercentIntoFade ), 
												 _SPARK_G_OVER_TIME( fPercentIntoFade ), 
												 _SPARK_B_OVER_TIME( fPercentIntoFade ),
												 pSparkData->fOpacity * fOpacMulti );
						SpritePos += StepSize;
						pSprites++;	
					}					
				} else {
					pSparkData->fOpacity = 0.0f;
				}
			}
		}
		////////////////////////////	
		// move on to the next spark
		////////////////////////////
		pSparkData++;
	}

	///////////////////////////////////
	// count the number of sprites used
	///////////////////////////////////
	pEmitter->m_nRenderCount = (u32)(pSprites - pEmitter->m_pBase);

	if( !pEmitter->m_nRenderCount ) {
		// no more sprites to draw return TRUE which will Kill() this emitter, bye...
		return TRUE;
	}
	FASSERT( pEmitter->m_nRenderCount <= pEmitter->m_nMaxCount );

	//////////////////////////////
	// compute the bounding sphere
	//////////////////////////////
	pEmitter->ComputeBoundingSphere();

	if( pEmitter->m_bLightAttached ) {
		if( fOpacityCount ) {
			pEmitter->m_fLightUnitIntensity = fUnitOpacitySum / fOpacityCount;
		} else {
			pEmitter->m_fLightUnitIntensity = 0.0f;
		}
	}
	
	/////////////////////
	// update the tracker
	/////////////////////
	pEmitter->UpdateTracker();

	return FALSE;
}


//
//
//
BOOL CEmitter::SteamWorkFunction( CEmitter *pEmitter, f32 fSecsSinceLastWork ) 
{
	f32 fRiseY, fOpacityMulti;
	_ParticleData_t *pParticleData;
	u32 i;
	FPSprite_t *pSprites;
	BOOL bSampleLight;
	
	// compute the rise since last work
	fRiseY = fSecsSinceLastWork * pEmitter->m_fGravityPerSec;

	// sample every 4 frames
	bSampleLight = ( (pEmitter->m_nNumFramesSinceBirth & 0x3) == 0x1 );

	if ( bSampleLight )
	{
		fworld_LightPoint( &pEmitter->m_Xfm.m_MtxF.m_vPos, &pEmitter->m_SampledWorldColor, TRUE );
		f32 fDelta;

		// make steam puffs more white
		fDelta = 1.0f - pEmitter->m_SampledWorldColor.fRed;
		pEmitter->m_SampledWorldColor.fRed += fDelta * 0.5f;
		fDelta = 1.0f - pEmitter->m_SampledWorldColor.fBlue;
		pEmitter->m_SampledWorldColor.fBlue += fDelta * 0.5f;
		fDelta = 1.0f - pEmitter->m_SampledWorldColor.fGreen;
		pEmitter->m_SampledWorldColor.fGreen += fDelta * 0.5f;
	}

	f32 fEndTime = (f32)(2.f * pEmitter->m_fUnitIntensity);
	
	if ( pEmitter->m_fTotalSecs < fEndTime
		&& (pEmitter->m_fTotalSecs > pEmitter->m_fNextSpawn || !pEmitter->m_nRenderCount) )
	{
		// Spawn a particle
		for ( i = 0; i < pEmitter->m_nDataCount; i++ ) 
		{
			_ParticleData_t *pSpawnParticle = &pEmitter->m_pData[i];
			
			if ( pSpawnParticle->fOpacity == 0.f )
			{
				pEmitter->m_fNextSpawn = pEmitter->m_fTotalSecs + 0.05f + (fmath_RandomUnitFloatLessThan1() * 0.05f);
				
				f32 fTimeModifier = (1.f - fmath_Div( pEmitter->m_fTotalSecs, fEndTime ));

				pSpawnParticle->VelocityPerSec.Set( 0.0f, 1.0f, 0.0f );
				
				CFVec3 vLocalX( 1.f, 0.f, 0.f ), vLocalZ( 0.f, 0.f, 1.f );

				// randomize the trajectory
				_RandomVectorInsideACone( pSpawnParticle->VelocityPerSec, vLocalX, vLocalZ, 
										  _PUFF_INNER_CONE, _PUFF_OUTTER_CONE * 2,
										  (s32)( i & 0x1 ) );
				pSpawnParticle->VelocityPerSec.Unitize();
				pSpawnParticle->VelocityPerSec *= (pEmitter->m_fUnitIntensity + (_ASSIGN_PUFF_INITIAL_VEL * fTimeModifier) );

//				if ( pEmitterVelPerSec_WS ) 
//				{
//					pSpawnParticle->VelocityPerSec += *pEmitterVelPerSec_WS;
//				}

				pSpawnParticle->fOpacity = (0.6f * fTimeModifier) + 0.1f;
				pSpawnParticle->fBirthTime = pEmitter->m_fTotalSecs;
				pSpawnParticle->fSecsToFade = fmath_Inv( _ASSIGN_PUFF_FADE( pEmitter->m_fUnitIntensity ) * 0.7f );
				
				pSpawnParticle->fSize = (_ASSIGN_PUFF_SIZE / 2) + (pEmitter->m_fUnitIntensity * 0.05f);
				pSpawnParticle->fGrowthPerSec = _ASSIGN_PUFF_GROWTH;

				pSpawnParticle->nNumSprites = 1;
				pSpawnParticle->XZChange.Zero();

				pSpawnParticle->Pos.Set( 0.f, _ASSIGN_PUFF_POS, 0.f );

				// set the sprites for this puff
				pEmitter->m_pBase[i].fDim_MS = pSpawnParticle->fSize;
				pEmitter->m_pBase[i].Point_MS = pSpawnParticle->Pos;
				pEmitter->m_pBase[i].ColorRGBA.Set( pEmitter->m_SampledWorldColor, pSpawnParticle->fOpacity ); 
				break;
			}
		}
	}

	// Walk all of the puffs updating those that are active
	pParticleData = pEmitter->m_pData;
	pSprites = pEmitter->m_pBase;
	for ( i = 0; i < pEmitter->m_nDataCount; i++, pParticleData++ ) 
	{
		if ( pParticleData->fOpacity != 0.f )
		{
			// fade the puff
			fOpacityMulti = 1 - ((pEmitter->m_fTotalSecs - pParticleData->fBirthTime) * pParticleData->fSecsToFade);
			FASSERT( fOpacityMulti <= 1.f );

			if ( fOpacityMulti > 0.0f ) 
			{
				// grow/shrink the puff
				pParticleData->fSize += pParticleData->fGrowthPerSec * fSecsSinceLastWork;
				if ( pParticleData->fSize > 0.0f ) 
				{
					// move the puff
					pParticleData->Pos += (pParticleData->VelocityPerSec * fSecsSinceLastWork);
					pParticleData->Pos.y += fRiseY;

					// add some randomness to the xz velocity
					if ( fmath_RandomChance( 0.15f ) ) 
					{
						pParticleData->XZChange.x += _PUFF_XZ_VEL_RAND;
						pParticleData->XZChange.y += -_PUFF_XZ_VEL_RAND;
					} 
					else 
					{
						pParticleData->XZChange *= 0.9f;
					}
					pParticleData->VelocityPerSec.x += pParticleData->XZChange.x * _PUFF_VEL_XZ_CHANGE_DAMPEN;
					pParticleData->VelocityPerSec.z += pParticleData->XZChange.y * _PUFF_VEL_XZ_CHANGE_DAMPEN;

					// add rise to the velocity
					pParticleData->VelocityPerSec.y += fRiseY;
					pParticleData->VelocityPerSec *= (1.0f - (_PUFF_VEL_DECEL * fSecsSinceLastWork));

					// setup the sprites for this puff
					pSprites->fDim_MS = pParticleData->fSize;
					pSprites->Point_MS = pParticleData->Pos;
					pSprites->ColorRGBA.Set( pEmitter->m_SampledWorldColor, pParticleData->fOpacity * fOpacityMulti );
					pSprites++;	
				} 
				else 
				{
					pParticleData->fOpacity = 0.0f;
				}
			} 
			else 
			{
				pParticleData->fOpacity = 0.0f;
			}
		}
	}

	// count the number of sprites used
	pEmitter->m_nRenderCount = (u32)(pSprites - pEmitter->m_pBase);

	if( !pEmitter->m_nRenderCount ) 
	{
		// no more sprites to draw, return TRUE which will Kill() this emitter, bye...
		return TRUE;
	}
//	FASSERT( pEmitter->m_nRenderCount <= pEmitter->m_nMaxCount );

	// compute the bounding sphere
	pEmitter->ComputeBoundingSphere();
	
	// update the tracker
	pEmitter->UpdateTracker();

	return FALSE;

	#undef _STEAM_START_FRAME
}


// Based on SmokeWorkFunction
BOOL CEmitter::PixiePuffWorkFunction( CEmitter *pEmitter, f32 fSecsSinceLastWork ) 
{
	f32 fRiseY, fPercentIntoFade, fOpacityMulti;
	_ParticleData_t *pPuffData;
	u32 i;
	FPSprite_t *pSprites;
	
	///////////////////////////////////
	// compute the rise since last work
	///////////////////////////////////
	fRiseY = fSecsSinceLastWork * pEmitter->m_fGravityPerSec;

	/////////////////////////
	// walk all of the puffs
	/////////////////////////
	pPuffData = pEmitter->m_pData;
	pSprites = pEmitter->m_pBase;

	BOOL bSparkleNow;

	if(fmath_RandomChance(0.1f))
	{
		pEmitter->m_SampledWorldColor.White();
		bSparkleNow = TRUE;
	}
	else
	{
		f32 fRandValue = fmath_RandomFloatRange(0.50f, 1.00f);
		pEmitter->m_SampledWorldColor.fRed = 1.0f;
//		pEmitter->m_SampledWorldColor.fGreen = fmath_RandomFloatRange(0.70f, 1.00f);
		pEmitter->m_SampledWorldColor.fGreen = fRandValue;
		pEmitter->m_SampledWorldColor.fBlue = 1.0f;
//		pEmitter->m_SampledWorldColor.fRed = 1.0f;
//		pEmitter->m_SampledWorldColor.fGreen = fRandValue;
//		pEmitter->m_SampledWorldColor.fBlue = fRandValue;
//		pEmitter->m_SampledWorldColor.fRed = fRandValue;
//		pEmitter->m_SampledWorldColor.fGreen = fRandValue;
//		pEmitter->m_SampledWorldColor.fBlue = 1.0f;

		bSparkleNow = FALSE;
	}

	for( i=0; i < pEmitter->m_nDataCount; i++ ) {
		//////////////////////
		// if the puff is on
		/////////////////////
		if( pPuffData->fOpacity > 0.0f ) {
			/////////////////
			// fade the puff
			/////////////////
			fPercentIntoFade = (pEmitter->m_fTotalSecs * pPuffData->fSecsToFade);
			fOpacityMulti = pEmitter->ComputeOpacity( pEmitter, fPercentIntoFade );			

			if( fOpacityMulti > 0.0f ) {

				////////////////////////
				// grow/shrink the puff
				////////////////////////
				pPuffData->fSize += pPuffData->fGrowthPerSec * fSecsSinceLastWork;
				if( pPuffData->fSize > 0.0f ) {
					/////////////////
					// move the puff
					/////////////////
					pPuffData->Pos += (pPuffData->VelocityPerSec * fSecsSinceLastWork);
					pPuffData->Pos.y += fRiseY;

					/////////////////////////////////////////
					// add some randomness to the xz velocity
					/////////////////////////////////////////
					if( fmath_RandomChance( 0.15f ) ) {
						pPuffData->XZChange.x += _PUFF_XZ_VEL_RAND;
						pPuffData->XZChange.y += -_PUFF_XZ_VEL_RAND;
					} else {
						pPuffData->XZChange *= 0.9f;
					}
					pPuffData->VelocityPerSec.x += pPuffData->XZChange.x * _PUFF_VEL_XZ_CHANGE_DAMPEN;
					pPuffData->VelocityPerSec.z += pPuffData->XZChange.y * _PUFF_VEL_XZ_CHANGE_DAMPEN;

					///////////////////////////////
					// add rise to the velocity
					//////////////////////////////
					pPuffData->VelocityPerSec.y += fRiseY;
					pPuffData->VelocityPerSec *= (1.0f - (_PUFF_VEL_DECEL * fSecsSinceLastWork));

					///////////////////////////////////
					// setup the sprites for this puff
					//////////////////////////////////
					pSprites->fDim_MS = pPuffData->fSize;
					pSprites->Point_MS = pPuffData->Pos;
					pSprites->ColorRGBA.Set( pEmitter->m_SampledWorldColor, bSparkleNow ? 1.0f : pPuffData->fOpacity * fOpacityMulti );
					pSprites++;	
				} else {
					pPuffData->fOpacity = 0.0f;
				}
			} else {
				pPuffData->fOpacity = 0.0f;
			}
		}
		////////////////////////////	
		// move on to the next puff
		////////////////////////////
		pPuffData++;
	}

	///////////////////////////////////
	// count the number of sprites used
	///////////////////////////////////
	pEmitter->m_nRenderCount = (u32)(pSprites - pEmitter->m_pBase);

	if( !pEmitter->m_nRenderCount ) {
		// no more sprites to draw, return TRUE which will Kill() this emitter, bye...
		return TRUE;
	}
	FASSERT( pEmitter->m_nRenderCount <= pEmitter->m_nMaxCount );

	//////////////////////////////
	// compute the bounding sphere
	//////////////////////////////
	pEmitter->ComputeBoundingSphere();
	
	/////////////////////
	// update the tracker
	/////////////////////
	pEmitter->UpdateTracker();

	return FALSE;
}


BOOL CEmitter::SmokeWorkFunction( CEmitter *pEmitter, f32 fSecsSinceLastWork ) {
	f32 fRiseY, fPercentIntoFade, fOpacityMulti;
	_ParticleData_t *pPuffData;
	u32 i;
	FPSprite_t *pSprites;
	BOOL bSampleLight;
	
	///////////////////////////////////
	// compute the rise since last work
	///////////////////////////////////
	fRiseY = fSecsSinceLastWork * pEmitter->m_fGravityPerSec;

	/////////////////////////
	// walk all of the puffs
	/////////////////////////
	pPuffData = pEmitter->m_pData;
	pSprites = pEmitter->m_pBase;

	// sample the world color
	switch( pEmitter->m_nType ) {
	default:
	case FPART_TYPE_SMOKE_PUFF:
	case FPART_TYPE_DUST_PUFF:
		// sample every 8 frames
		bSampleLight = ( (pEmitter->m_nNumFramesSinceBirth & 0x7) == 0x4 );
		break;
	case FPART_TYPE_SMOKE_CLOUD:
		// sample every other frame
		bSampleLight = (s32)( pEmitter->m_nNumFramesSinceBirth & 0x1 );
		break;
	case FPART_TYPE_STEAM:
	case FPART_TYPE_STEAM_PUFF:
/*	case FPART_TYPE_PIXIE_PUFF:*/
		// sample every 4 frames
		bSampleLight = ( (pEmitter->m_nNumFramesSinceBirth & 0x3) == 0x1 );
		break;
	}
	if( bSampleLight ) {
		fworld_LightPoint( &pEmitter->m_Xfm.m_MtxF.m_vPos, &pEmitter->m_SampledWorldColor, TRUE );
		f32 fAvgColor, fDelta;

		switch( pEmitter->m_nType ) {

		default:
			fAvgColor = pEmitter->m_SampledWorldColor.CalcIntensity();
			fDelta = pEmitter->m_SampledWorldColor.fRed - fAvgColor;
			pEmitter->m_SampledWorldColor.fRed -= fDelta * 0.65f;
			fDelta = pEmitter->m_SampledWorldColor.fBlue - fAvgColor;
			pEmitter->m_SampledWorldColor.fBlue -= fDelta * 0.65f;
			fDelta = pEmitter->m_SampledWorldColor.fGreen - fAvgColor;
			pEmitter->m_SampledWorldColor.fGreen -= fDelta * 0.65f;
			break;
		
		case FPART_TYPE_DUST_PUFF:
			fAvgColor = pEmitter->m_SampledWorldColor.CalcIntensity();
			fDelta = pEmitter->m_SampledWorldColor.fRed - fAvgColor;
			pEmitter->m_SampledWorldColor.fRed -= fDelta * 0.75f;
			fDelta = pEmitter->m_SampledWorldColor.fBlue - fAvgColor;
			pEmitter->m_SampledWorldColor.fBlue -= fDelta * 0.75f;
			fDelta = pEmitter->m_SampledWorldColor.fGreen - fAvgColor;
			pEmitter->m_SampledWorldColor.fGreen -= fDelta * 0.75f;
			break;

		case FPART_TYPE_STEAM:
		case FPART_TYPE_STEAM_PUFF:
			// make steam puffs more white
			fDelta = 1.0f - pEmitter->m_SampledWorldColor.fRed;
			pEmitter->m_SampledWorldColor.fRed += fDelta * 0.5f;
			fDelta = 1.0f - pEmitter->m_SampledWorldColor.fBlue;
			pEmitter->m_SampledWorldColor.fBlue += fDelta * 0.5f;
			fDelta = 1.0f - pEmitter->m_SampledWorldColor.fGreen;
			pEmitter->m_SampledWorldColor.fGreen += fDelta * 0.5f;
			break;

/*			// JUSTIN: You'll probably want to change this.
		case FPART_TYPE_PIXIE_PUFF:
			pEmitter->m_SampledWorldColor.fRed = 1.0f;
			pEmitter->m_SampledWorldColor.fGreen = fmath_RandomFloatRange(0.60f, 0.90f);
			pEmitter->m_SampledWorldColor.fBlue = 1.0f;
			break;*/
		}
	}

	for( i=0; i < pEmitter->m_nDataCount; i++ ) {
		//////////////////////
		// if the puff is on
		/////////////////////
		if( pPuffData->fOpacity > 0.0f ) {
			/////////////////
			// fade the puff
			/////////////////
			fPercentIntoFade = (pEmitter->m_fTotalSecs * pPuffData->fSecsToFade);
			fOpacityMulti = pEmitter->ComputeOpacity( pEmitter, fPercentIntoFade );			

			if( fOpacityMulti > 0.0f ) {

				////////////////////////
				// grow/shrink the puff
				////////////////////////
				pPuffData->fSize += pPuffData->fGrowthPerSec * fSecsSinceLastWork;
				if( pPuffData->fSize > 0.0f ) {
					/////////////////
					// move the puff
					/////////////////
					pPuffData->Pos += (pPuffData->VelocityPerSec * fSecsSinceLastWork);
					pPuffData->Pos.y += fRiseY;

					/////////////////////////////////////////
					// add some randomness to the xz velocity
					/////////////////////////////////////////
					if( fmath_RandomChance( 0.15f ) ) {
						pPuffData->XZChange.x += _PUFF_XZ_VEL_RAND;
						pPuffData->XZChange.y += -_PUFF_XZ_VEL_RAND;
					} else {
						pPuffData->XZChange *= 0.9f;
					}
					pPuffData->VelocityPerSec.x += pPuffData->XZChange.x * _PUFF_VEL_XZ_CHANGE_DAMPEN;
					pPuffData->VelocityPerSec.z += pPuffData->XZChange.y * _PUFF_VEL_XZ_CHANGE_DAMPEN;

					///////////////////////////////
					// add rise to the velocity
					//////////////////////////////
					pPuffData->VelocityPerSec.y += fRiseY;
					pPuffData->VelocityPerSec *= (1.0f - (_PUFF_VEL_DECEL * fSecsSinceLastWork));

					///////////////////////////////////
					// setup the sprites for this puff
					//////////////////////////////////
					pSprites->fDim_MS = pPuffData->fSize;
					pSprites->Point_MS = pPuffData->Pos;
					pSprites->ColorRGBA.Set( pEmitter->m_SampledWorldColor, pPuffData->fOpacity * fOpacityMulti );
					pSprites++;	
				} else {
					pPuffData->fOpacity = 0.0f;
				}
			} else {
				pPuffData->fOpacity = 0.0f;
			}
		}
		////////////////////////////	
		// move on to the next puff
		////////////////////////////
		pPuffData++;
	}

	///////////////////////////////////
	// count the number of sprites used
	///////////////////////////////////
	pEmitter->m_nRenderCount = (u32)(pSprites - pEmitter->m_pBase);

	if( !pEmitter->m_nRenderCount ) {
		// no more sprites to draw, return TRUE which will Kill() this emitter, bye...
		return TRUE;
	}
	FASSERT( pEmitter->m_nRenderCount <= pEmitter->m_nMaxCount );

	//////////////////////////////
	// compute the bounding sphere
	//////////////////////////////
	pEmitter->ComputeBoundingSphere();
	
	/////////////////////
	// update the tracker
	/////////////////////
	pEmitter->UpdateTracker();

	return FALSE;
}

BOOL CEmitter::PixelDustWorkFunction( CEmitter *pEmitter, f32 fSecsSinceLastWork ) {
	u32 i;
	_ParticleData_t *pPixieData;
	f32 fRise, fPercentIntoFade, fNewPercent, fAlpha;
	FPSprite_t *pSprites;
	CFVec3 StepSize, SpritePos;

	///////////////////////////////////
	// compute the rise since last work
	///////////////////////////////////
	fRise = fSecsSinceLastWork * pEmitter->m_fGravityPerSec;

	/////////////////////////
	// walk all of the Pixies
	/////////////////////////
	pPixieData = pEmitter->m_pData;
	pSprites = pEmitter->m_pBase;
	for( i=0; i < pEmitter->m_nDataCount; i++ ) {
		//////////////////////
		// if the Pixie is on
		/////////////////////
		if( pPixieData->fOpacity > 0.0f ) {
			/////////////////
			// fade the Pixie
			/////////////////
			fPercentIntoFade = (pEmitter->m_fTotalSecs * pPixieData->fSecsToFade);
			FMATH_CLAMPMAX( fPercentIntoFade, 1.0f );
			pPixieData->fOpacity = 0.75f;

			if( pPixieData->fOpacity > 0.0f ) {
				
				////////////////////////
				// grow/shrink the Pixie
				////////////////////////
				if( fPercentIntoFade <= 0.25f  ) {
					pPixieData->fSize = _PIXEL_DUST1_MAX_SIZE - ( (1.0f - (fPercentIntoFade * (1.0f/0.25f))) * pPixieData->fGrowthPerSec );
					pPixieData->fOpacityMultiplier = fmath_RandomFloat();
					if( pPixieData->fOpacityMultiplier >= 0.05f ) {
						// don't spike
						pPixieData->fOpacityMultiplier = 1.0f;								
					}

				} else {
					pPixieData->fSize = _PIXEL_DUST1_MAX_SIZE;
					if( pPixieData->fOpacityMultiplier >= 1.0f ) {
						// see if we should start a spike
						pPixieData->fOpacityMultiplier = fmath_RandomFloat();
						if( pPixieData->fOpacityMultiplier >= 0.05f ) {
							// don't spike
							pPixieData->fOpacityMultiplier = 1.0f;								
						} else if( fPercentIntoFade < 0.9f ) {
							pPixieData->fOpacityMultiplier = 0.05f;
						}
					} else {
						pPixieData->fOpacityMultiplier += ( (1.0f/(3.0f/60.0f)) * fSecsSinceLastWork );
						if( pPixieData->fOpacityMultiplier >= 1.0f &&
							fPercentIntoFade >= 0.9f ) {
							// kill it
							pPixieData->fOpacity = 0.0f;
						}
					}
				}
				if( pPixieData->fOpacity > 0.0f ) {
					/////////////////
					// move the Pixie
					/////////////////
					pPixieData->Pos += pPixieData->VelocityPerSec * fSecsSinceLastWork;
					pPixieData->VelocityPerSec.y += fRise;

					///////////////////////////////////
					// setup the sprites for this Pixie
					//////////////////////////////////
					pSprites->fDim_MS = pPixieData->fSize;
					pSprites->Point_MS = pPixieData->Pos;
					if( pPixieData->fOpacityMultiplier >= 1.0f ) {
						fNewPercent = 0.3f + (fPercentIntoFade * 0.7f);
						pSprites->ColorRGBA.Set( _PIXEL_DUST1_R_OVER_TIME( fNewPercent, pEmitter->m_rgbColor.fRed ), 
												 _PIXEL_DUST1_G_OVER_TIME( fNewPercent, pEmitter->m_rgbColor.fGreen ), 
												 _PIXEL_DUST1_B_OVER_TIME( fNewPercent, pEmitter->m_rgbColor.fBlue ),
												 pPixieData->fOpacity );
					} else {
						fNewPercent = pPixieData->fOpacityMultiplier * fPercentIntoFade;
						fAlpha = pPixieData->fOpacity + 
								 ((1.0f - pPixieData->fOpacityMultiplier ) * (1.0f - pPixieData->fOpacity));
						pSprites->ColorRGBA.Set( _PIXEL_DUST1_R_OVER_TIME( fNewPercent, pEmitter->m_rgbColor.fRed ), 
												 _PIXEL_DUST1_G_OVER_TIME( fNewPercent, pEmitter->m_rgbColor.fGreen ), 
												 _PIXEL_DUST1_B_OVER_TIME( fNewPercent, pEmitter->m_rgbColor.fBlue ),
												 fAlpha );
						pSprites->fDim_MS += ((_PIXEL_DUST1_MAX_SIZE * 0.5f) * (1.0f - pPixieData->fOpacityMultiplier));
					}
					pSprites++;	
				}				
			}
		}
		////////////////////////////	
		// move on to the next Pixie
		////////////////////////////
		pPixieData++;
	}

	///////////////////////////////////
	// count the number of sprites used
	///////////////////////////////////
	pEmitter->m_nRenderCount = (u32)(pSprites - pEmitter->m_pBase);

	if( !pEmitter->m_nRenderCount ) {
		// no more sprites to draw return TRUE which will Kill() this emitter, bye...
		return TRUE;
	}
	FASSERT( pEmitter->m_nRenderCount <= pEmitter->m_nMaxCount );

	//////////////////////////////
	// compute the bounding sphere
	//////////////////////////////
	pEmitter->ComputeBoundingSphere();

	/////////////////////
	// update the tracker
	/////////////////////
	pEmitter->UpdateTracker();

	return FALSE;
}

/////////////////////////////////
// CEmitterPoolMgr implementation
/////////////////////////////////

CEmitterPoolMgr::CEmitterPoolMgr() {
	m_nTotalSize = 0;
	m_pPool = NULL;
}

BOOL CEmitterPoolMgr::Create( u32 nNumEmitters ) {
	m_nTotalSize = 0;
	m_pPool = NULL;

	if( nNumEmitters ) {
		m_pPool = fnew CEmitter[nNumEmitters];
		if( !m_pPool ) {
			return FALSE;
		}

		m_nTotalSize = nNumEmitters;
	}

	return TRUE;
}

void CEmitterPoolMgr::Destroy( void ) {
	fdelete_array( m_pPool );
	m_pPool = NULL;
}

u32 CEmitterPoolMgr::GetNumUsed() const {
	u32 i, nNumUsed;

	// walk the pool and kill any used ones
	nNumUsed = 0;
	for( i=0; i < m_nTotalSize; i++ ) {
		if( m_pPool[i].InUse() ) {
			nNumUsed++;
		}
	}
	return nNumUsed;
}

CEmitter *CEmitterPoolMgr::GetUsed( u32 nIndex ) const {
	u32 i, nFoundCount;

	// walk the pool and find an nIndex used emitter
	nFoundCount = 0;
	for( i=0; i < m_nTotalSize; i++ ) {
		if( m_pPool[i].InUse() ) {
			if( nIndex == nFoundCount ) {
				return &m_pPool[i];
			}
			nFoundCount++;
		}
	}
	return NULL;	
}

CEmitter *CEmitterPoolMgr::GetAvailable() {
	u32 i;

	// walk the pool and find an available emitter
	for( i=0; i < m_nTotalSize; i++ ) {
		if( !m_pPool[i].InUse() && m_pPool[i].IsAvailable() ) {
#if _TRACK_MEM
			u32 nNumUsed = GetNumUsed();
			if( nNumUsed > _nMaxEmittersUsed ) {
				_nMaxEmittersUsed = nNumUsed;
			}
#endif
			return &m_pPool[i];
		}
	}
	return NULL;
}

void CEmitterPoolMgr::KillAll() {
	u32 i;

	// walk the pool and kill any used ones
	for( i=0; i < m_nTotalSize; i++ ) {
		if( m_pPool[i].InUse() ) {
			m_pPool[i].Kill();
		}
	}
}

void CEmitterPoolMgr::KillType( FPart_Type_e nType ) {
	u32 i;

	// walk the pool and kill any nType used ones
	for( i=0; i < m_nTotalSize; i++ ) {
		if( m_pPool[i].InUse() && m_pPool[i].GetType() == nType ) {
			m_pPool[i].Kill();
		}
	}
}

void CEmitterPoolMgr::Release() {
	
	if( m_pPool ) {
		// kill all of the emitters
		KillAll();
		// free the memory pool
		fdelete_array( m_pPool );
		m_pPool = NULL;
	}
	m_nTotalSize = 0;
}

///////////////////////////////
// CLightEmitter implementation
///////////////////////////////

CLightEmitter::CLightEmitter() : CFWorldLight() {
	u32 i;

	m_nType = FPART_TYPE_UNASSIGNED;
	m_nNumEmitters = 0;
	for( i=0; i < _MAX_EMITTERS_PER_LIGHT; i++ ) {
		m_apEmitters[i] = NULL;
	}

	m_pFcnWork = CLightEmitter::Work;
}

BOOL CLightEmitter::Init( const FLightInit_t &rLightInit, FPart_Type_e nType, const CEmitter *pEmitter ) {

	FASSERT( nType != FPART_TYPE_UNASSIGNED );
	FASSERT( m_nType == FPART_TYPE_UNASSIGNED );
	FASSERT( pEmitter );

	m_nType = nType;
	m_nNumEmitters = 1;
	m_apEmitters[0] = pEmitter;

	// put the light into the world
	CFWorldLight::Init( &rLightInit );

	return TRUE;
}

BOOL CLightEmitter::AttachEmitter( const CEmitter *pEmitter ) {

	FASSERT( pEmitter );

	if( m_nNumEmitters >= _MAX_EMITTERS_PER_LIGHT ) {
		// too many particle emitters already attached to this light emitter
		return FALSE;
	}
	// add the new emitter
	m_apEmitters[m_nNumEmitters++] = pEmitter;

	return TRUE;
}

void CLightEmitter::Kill() {

	if( InUse() ) {
		// remove from world, which also turns the light off
		RemoveFromWorld();
		// mark emitter as not used anymore
		m_nType = FPART_TYPE_UNASSIGNED;
		// reset some vars for next time
		m_nType = FPART_TYPE_UNASSIGNED;
		m_nNumEmitters = 0;
	}
}

const CEmitter *CLightEmitter::GetAttachedEmitter( u32 nIndex ) {

	if( nIndex >= m_nNumEmitters ) {
		return NULL;
	}
	return m_apEmitters[nIndex];
}

void CLightEmitter::CompactUsedEmitterList( u32 nOldCount ) {
	u32 i, j, nStartIndex;

	FASSERT( nOldCount >= m_nNumEmitters );

	nStartIndex = 0;
	// find an entry that needs to be moved
	for( i=m_nNumEmitters; i < nOldCount; i++ ) {
		if( m_apEmitters[i] ) {
			for( j=nStartIndex; j < m_nNumEmitters; j++ ) {
				if( !m_apEmitters[j] ) {
					// swap the array entries
					m_apEmitters[j] = m_apEmitters[i];
					m_apEmitters[i] = NULL;
					break;
				}
			}
			FASSERT( j < m_nNumEmitters );
		}
	}
}

void CLightEmitter::UpdateLightFromSingleEmitter( CLightEmitter &rLightEmitter, const CEmitter &rParticleEmitter ) {
	CFSphere  NewSphere;
	CFLight *pLight = &rLightEmitter.m_Light;

	NewSphere = rParticleEmitter.GetBoundingSphere();
	NewSphere.m_fRadius *= rParticleEmitter.m_fLightRadiusMultiplier;
			
	pLight->SetPositionAndRadius( &NewSphere );
	rLightEmitter.UpdateTracker();
	pLight->SetIntensity( rParticleEmitter.m_fLightUnitIntensity );	
}

void CLightEmitter::Work( CFWorldTracker *pTracker ) {
	CLightEmitter *pLightEmitter = (CLightEmitter *)pTracker;
	u32 i, nOldEmitterCount;
	const CFSphere *pSphere, *pLargestSphere;
	CFSphere NewSphere;
	f32 fRadius, fIntensity, fLargestRadius;
	CFVec3 Min, Max, TempVec3;
	const CEmitter *pEmitter;
	
	// make sure that this light emitter still has valid emitters attached
	nOldEmitterCount = pLightEmitter->m_nNumEmitters;
	for( i=0; i < nOldEmitterCount; i++ ) {
		pEmitter = pLightEmitter->m_apEmitters[i];
		if( !pEmitter->InUse() || pEmitter->GetType() != pLightEmitter->m_nType ) {
			pLightEmitter->m_apEmitters[i] = NULL;
			pLightEmitter->m_nNumEmitters--;
		}
	}
	if( pLightEmitter->m_nNumEmitters ) {
		// there are still valid emitters attached

		// see if our emitter count was reduced, if so we need 
		// to rearrange our emitter array to fill in the gaps
		if( pLightEmitter->m_nNumEmitters != nOldEmitterCount ) {
			pLightEmitter->CompactUsedEmitterList( nOldEmitterCount );			
		}

		////////////////////////////////////////////////////////////////////////////////////
		// if there is only 1 emitter, we can do a huge optimization by doing alot less work
		if( pLightEmitter->m_nNumEmitters == 1 ) {
			UpdateLightFromSingleEmitter( *pLightEmitter, *pLightEmitter->m_apEmitters[0] );		
		} else {
		
			// put the largest emitter in the first slot
			pEmitter = pLightEmitter->m_apEmitters[0];
			pLargestSphere = &pEmitter->GetBoundingSphere();
			fLargestRadius = pLargestSphere->m_fRadius * pEmitter->m_fLightRadiusMultiplier;
			
			nOldEmitterCount = pLightEmitter->m_nNumEmitters;
			for( i=1; i < nOldEmitterCount; i++ ) {
				pEmitter = pLightEmitter->m_apEmitters[i];
				pSphere = &pEmitter->GetBoundingSphere();
				fRadius = pSphere->m_fRadius * pEmitter->m_fLightRadiusMultiplier;

				if( fRadius > fLargestRadius ) {
					fLargestRadius = fRadius;
					pLargestSphere = pSphere;
					pLightEmitter->m_apEmitters[i] = pLightEmitter->m_apEmitters[0];
					pLightEmitter->m_apEmitters[0] = pEmitter;
				}
			}
			// see if any emitter doesn't fit inside of the largest light's sphere		
			for( i=1; i < nOldEmitterCount; i++ ) {
				pEmitter = pLightEmitter->m_apEmitters[i];
				pSphere = &pEmitter->GetBoundingSphere();

				if( !pLargestSphere->IsIntersecting( pSphere->m_Pos ) ) {
					// pEmitter is not inside of the largest light's radius, 
					// either reassign it to another light emitter or start a new one
					_LightPool.ReassignEmitter( pLightEmitter, pEmitter );

					pLightEmitter->m_apEmitters[i] = NULL;
					pLightEmitter->m_nNumEmitters--;
				}			
			}

			// see if our emitter count was reduced, if so we need 
			// to rearrange our emitter array to fill in the gaps
			if( pLightEmitter->m_nNumEmitters != nOldEmitterCount ) {
				pLightEmitter->CompactUsedEmitterList( nOldEmitterCount );			
			}

			if( pLightEmitter->m_nNumEmitters == 1 ) {
				UpdateLightFromSingleEmitter( *pLightEmitter, *pLightEmitter->m_apEmitters[0] );		
			} else {
				///////////////////////////////////////////////////////////////////////////////
				// run through each emitter and compute the light's bounding sphere & intensity
								
				// remember that the 1st emitter is the largest radius, so we can reuse those vars
				
				// init the min
				Min.Set( -fLargestRadius, -fLargestRadius, -fLargestRadius );
				Min += pLargestSphere->m_Pos;
				// init the max
				Max.Set( fLargestRadius, fLargestRadius, fLargestRadius );
				Max += pLargestSphere->m_Pos;
				// init the intensity
				fIntensity = pLightEmitter->m_apEmitters[0]->m_fLightUnitIntensity;

				for( i=1; i < pLightEmitter->m_nNumEmitters; i++ ) {
					pEmitter = pLightEmitter->m_apEmitters[i];
					FASSERT( pEmitter );

					pSphere = &pEmitter->GetBoundingSphere();
					fRadius = pSphere->m_fRadius * pEmitter->m_fLightRadiusMultiplier;
					// adjust our min
					TempVec3.Set( -fRadius, -fRadius, -fRadius );
					TempVec3 += pSphere->m_Pos;
					Min.Min( TempVec3 );
					// adjust our max
					TempVec3.Set( fRadius, fRadius, fRadius );
					TempVec3 += pSphere->m_Pos;
					Max.Max( TempVec3 );
					// adjust the intensity
					if( pEmitter->m_fLightUnitIntensity > fIntensity ) {
						fIntensity = pEmitter->m_fLightUnitIntensity;
					}		
				}
				// adjust the light sphere and intensity
				NewSphere.BuildFromMinMaxBox( Min, Max );
				
				// adjust down the light's sphere to account for error when combining the spheres
				NewSphere.m_fRadius *= 0.95f;

				pLightEmitter->m_Light.SetPositionAndRadius( &NewSphere );
				pLightEmitter->UpdateTracker();
				pLightEmitter->m_Light.SetIntensity( fIntensity );
			}
		}
	} else {
		pLightEmitter->Kill();
	}
}

//////////////////////////////////////
// CLightEmitterPoolMgr implementation
//////////////////////////////////////

CLightEmitterPoolMgr::CLightEmitterPoolMgr() {
	m_nTotalSize = 0;
	m_pPool = NULL;
}

void CLightEmitterPoolMgr::Destroy( void ) {
	fdelete_array( m_pPool );
	m_pPool = NULL;
}

BOOL CLightEmitterPoolMgr::Create( u32 nNumEmitters ) {
	m_nTotalSize = 0;
	m_pPool = NULL;

	if( nNumEmitters ) {
		m_pPool = fnew CLightEmitter[nNumEmitters];
		if( !m_pPool ) {
			return FALSE;
		}
		m_nTotalSize = nNumEmitters;
	}

	return TRUE;
}

u32 CLightEmitterPoolMgr::GetNumUsed() const {
	u32 i, nNumUsed;

	// walk the pool and kill any used ones
	nNumUsed = 0;
	for( i=0; i < m_nTotalSize; i++ ) {
		if( m_pPool[i].InUse() ) {
			nNumUsed++;
		}
	}
	return nNumUsed;
}

CLightEmitter *CLightEmitterPoolMgr::GetUsed( u32 nIndex ) const {
	u32 i, nFoundCount;

	// walk the pool and find an nIndex used emitter
	nFoundCount = 0;
	for( i=0; i < m_nTotalSize; i++ ) {
		if( m_pPool[i].InUse() ) {
			if( nIndex == nFoundCount ) {
				return &m_pPool[i];
			}
			nFoundCount++;
		}
	}
	return NULL;	
}

CLightEmitter *CLightEmitterPoolMgr::GetAvailable() {
	u32 i;

	// walk the pool and find an available emitter
	for( i=0; i < m_nTotalSize; i++ ) {
		if( !m_pPool[i].InUse() ) {
#if _TRACK_MEM
			u32 nNumUsed = GetNumUsed();
			if( nNumUsed > _nMaxLightsUsed ) {
				_nMaxLightsUsed = nNumUsed;
			}
#endif
			return &m_pPool[i];
		}
	}
	return NULL;
}

void CLightEmitterPoolMgr::KillAll() {
	u32 i;

	// walk the pool and kill any used ones
	for( i=0; i < m_nTotalSize; i++ ) {
		if( m_pPool[i].InUse() ) {
			m_pPool[i].Kill();
		}
	}
}

void CLightEmitterPoolMgr::KillType( FPart_Type_e nType ) {
	u32 i;

	// walk the pool and kill any nType used ones
	for( i=0; i < m_nTotalSize; i++ ) {
		if( m_pPool[i].InUse() && m_pPool[i].GetType() == nType ) {
			m_pPool[i].Kill();
		}
	}
}

void CLightEmitterPoolMgr::Release() {
	if( m_pPool ) {
		// kill all of the emitters
		KillAll();
		// free the memory pool
		fdelete_array( m_pPool );
		m_pPool = NULL;
	}
	m_nTotalSize = 0;
}

BOOL CLightEmitterPoolMgr::ReassignEmitter( CLightEmitter *pOldEmitter, const CEmitter *pEmitterToMove ) {
	FLightInit_t LightInit;

	// create a FLightInit_t from the old emitter
	fclib_strcpy( LightInit.szName, pOldEmitter->m_Light.m_szName );
	LightInit.szPerPixelTexName[0] = 0;
	LightInit.szCoronaTexName[0] = 0;
	LightInit.nType = pOldEmitter->m_Light.m_nType;
	LightInit.nLightID = pOldEmitter->m_Light.m_nLightID;
	LightInit.nFlags = pOldEmitter->m_Light.m_nFlags;
	LightInit.Motif = pOldEmitter->m_Light.m_Motif;
	LightInit.fIntensity = pEmitterToMove->m_fLightUnitIntensity;
	LightInit.spInfluence = pEmitterToMove->GetBoundingSphere();
	LightInit.spInfluence.m_fRadius *= pEmitterToMove->m_fLightRadiusMultiplier;
	LightInit.mtxOrientation = pOldEmitter->m_Light.m_mtxOrientation_WS;
	LightInit.fSpotInnerRadians = pOldEmitter->m_Light.m_fSpotInnerRadians;
	LightInit.fSpotOuterRadians = pOldEmitter->m_Light.m_fSpotOuterRadians;

	return AddOrCombineALight( LightInit, pEmitterToMove->GetType(), pEmitterToMove, pOldEmitter );
}

// helps to optimize the emitted lights, by looking for nearby spawned lights whose radii can be enlarged
// instead of adding another light to the scene.
// if pExclude is not NULL, the light will not be combined with that light emitter, used when reassigning an emitter
// Returns TRUE if the light was added to the world, by either combining it with another light or creating a new light.
// Returns FALSE if the light could not be added to the world.
BOOL CLightEmitterPoolMgr::AddOrCombineALight( const FLightInit_t &rLightInit, FPart_Type_e nType, const CEmitter *pEmitter, CLightEmitter *pExclude/*=NULL*/ ) {
	u32 nNumActiveEmitters, i;
	CLightEmitter *pActiveLight, *pClosest;
	CFVec3 LightToNew;
	f32 fRadius, fMag2, fClosestMag2, fMag;

	FASSERT( pEmitter );

	// search all of the currently used light emitters
	nNumActiveEmitters = GetNumUsed();
#if 1
	pClosest = NULL;
	for( i=0; i < nNumActiveEmitters; i++ ) {
		// grab an active light
		pActiveLight = GetUsed( i );
		if( (u32)pActiveLight != (u32)pExclude ) {
			// see if this active light is nType
			if( pActiveLight->GetType() == nType ) {
				// see if the proposed light's center is inside the existing light's sphere (gross test)
				if( pActiveLight->m_Light.m_spInfluence_WS.IsIntersecting( rLightInit.spInfluence.m_Pos ) ) {

					// determine how close the proposed light is to the existing light
					LightToNew = rLightInit.spInfluence.m_Pos - pActiveLight->m_Light.m_spInfluence_WS.m_Pos;
					fMag2 = LightToNew.Mag2();

					if( pClosest ) {
						if( fMag2 < fClosestMag2 ) {
							// a new closest light
							fClosestMag2 = fMag2;
							pClosest = pActiveLight;
						}
					} else {
						// the first light found
						pClosest = pActiveLight;
						fClosestMag2 = fMag2;
					}				
				}			
			}
		}
	}
	if( pClosest ) {
		fRadius = pClosest->m_Light.m_spInfluence_WS.m_fRadius;
		fMag = fmath_Sqrt( fClosestMag2 );
		
		if( fMag <= (fRadius * 0.39f) ) {
			// so close to an existing light, that we don't need to add this light at all
			return TRUE;
		} else if( fMag <= (fRadius * 0.985f) ) {
			// attempt to attach pEmitter to this active light
			FASSERT( pClosest != pExclude );
			if( pClosest->AttachEmitter( pEmitter ) ) {
				return TRUE;	
			}
		}
	}
#endif
	// if the light has not been added by this point, we could not combine lights, try to add a new one
	pActiveLight = GetAvailable();
	if( !pActiveLight ) {
		return FALSE;
	}
	FASSERT( pActiveLight != pExclude );
	
	if( !pActiveLight->Init( rLightInit, nType, pEmitter ) ) {
		return FALSE;
	}

#if 0
	// keep track of the max number of emitters currently used
	static u32 _nNumLightsInTheWorld = 0;
	nNumActiveEmitters = GetNumUsed();
	if( nNumActiveEmitters > _nNumLightsInTheWorld ) {
		_nNumLightsInTheWorld = nNumActiveEmitters;
		DEVPRINTF( "num lights in world = %d.\n", _nNumLightsInTheWorld );
	}
#endif

	return TRUE;
}
#endif



