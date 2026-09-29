//////////////////////////////////////////////////////////////////////////////////////
// fparticle.cpp - 
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
// 08/20/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "fparticle.h"
#include "fpsprite.h"
#include "floop.h"
#include "fvis.h"
#include "fresload.h"
#include "flightgroup.h"
#include "frenderer.h"
#include "fdraw.h"
#include "ftext.h"
#include "fsndfx.h"

//====================
// private definitions

#define _USE_POOLED_RANDOM_FLOATS		FALSE
#define _USE_JOHNS_COOL_OPTIMIZATION	TRUE

#if _USE_POOLED_RANDOM_FLOATS

// random number generation optimization routine
#define _NUM_POOLED_RANDS			64// must be a power of 2
#define _NUM_POOLED_RANDS_TO_SKIP	7// by doing this we effectively double the size of our pool because we won't see repeats for a while

static u32 _nRandIndex;
static f32 _afRandomUnitFloats[_NUM_POOLED_RANDS];

static void _ResetRandPool( void ) {
	_nRandIndex = 0;
}
static f32 _GetPooledRandomFloat( void ) {
	
	if( _nRandIndex < _NUM_POOLED_RANDS ) {
		// pool is not full yet, generate a new random number
		f32 fRandom = fmath_RandomFloat();
		_afRandomUnitFloats[_nRandIndex++] = fRandom;
		
		return fRandom;
	} else {
		// the pool is full, start recycling randoms
		_nRandIndex += _NUM_POOLED_RANDS_TO_SKIP;
		
		return _afRandomUnitFloats[ _nRandIndex & (_NUM_POOLED_RANDS-1) ];
	}
}

static f32 _GetPooledRandomBipolarUnitFloat( void ) {
	f32 fRandom = _GetPooledRandomFloat();

	fRandom -= 0.5f;// -0.5f to 0.5f
	fRandom *= 2.0f;// -1.0f to 1.0f

	return fRandom;
}
#endif


#define _TOO_CLOSE_TO_CAM			10.0f
#define _OO_TOO_CLOSE_TO_CAM		( 1.0f / _TOO_CLOSE_TO_CAM )
#define _TOO_CLOSE_DROP_PERCENT		1.0f//0.50f
#define _SMALLEST_PERCENT_TO_DRAW	0.35f

#define _INITIAL_BS_RADIUS			1.0f

#define _EMISSION_CUT_FOR_NON_VISIBLE	0.5f

typedef f32 EvalFunction_t( const FParticleAnimFunc_t *pFuncData, f32 fUnitAge );

#define _EMITTER_SIGNATURE			0x12345678

#define _SECS_TO_INTRODUCE_TAIL		( 1.0f / 5.0f )
#define _OO_SECS_TO_INTRODUCE_TAIL	( 1.0f / _SECS_TO_INTRODUCE_TAIL )

enum {
	_BURST_FLAGS_INITIAL_VEL_ADDED		= 0x00000001,
	_BURST_FLAGS_APPLY_DRAG_FORCE		= 0x00000002,
	_BURST_FLAGS_APPLY_BUBBLE_FORCE		= 0x00000004,
	_BURST_FLAGS_JITTER_POS				= 0x00000008,
	_BURST_FLAGS_NEG_DRAG_FORCE			= 0x00000010,
    
	_BURST_FLAGS_NONE					= 0
};

//////////////////////////////////////////////////////////////////////
// A BURST IS COLLECTION OF PARTICLES WHO WERE ALL CREATED AT THE SAME
// INSTANT IN TIME, THEY WILL USE THE CONTAINED VARIABLES DURING THEIR 
// LIFETIME. 
FCLASS_ALIGN_PREFIX struct _Burst_t {
	f32 fDragMultiplierPerSec;
	f32 fGravityPerSec;

	f32 fBubbleUpdateChance;
	f32 fBubbleDragPerSec;
	CFVec3A MaxBubbleXYZ;
	
	CFVec3A JitterOffset;

	CFVec3A EmitterVelPerSec;

	u32 nFlags;// see _BURST_FLAGS_...

	u32 nMaxSprites;
	f32 fMaxVel2;
	f32 fOOMaxVel;
	f32 fAlphaStepPerSprite;
	f32 fSizeStepPerSprite;
	f32 fPosStepPerSprite;

	// tbd...
	f32 fMinCollAlpha;
	f32 fMaxCollSplit;
	f32 fUnitCollBounce;

	FLink_t Link;					// this is used to create the emitter or free pool linklist
	FLinkRoot_t ParticleList;		// a linked list of _Particle_t

	FCLASS_STACKMEM_ALIGN( _Burst_t );
} FCLASS_ALIGN_SUFFIX;

////////////////////////////////////////////////////////////////////
// A PARTICLE IS EVERYTHING ABOUT A PARTICLE EXCEPT THE RENDER DATA,
// SOME PARTICLES WILL BE RENDERED WITH MORE THAN 1 SPRITE
FCLASS_ALIGN_PREFIX struct _Particle_t {
	_Burst_t *pBurst;
	f32 fAge;		// how old is this particle (in secs)
	f32 fUnitAge;	// a unit float age (1.0f == dead)
	f32 fOOLifeSpan;// 1 / how long this particle will live (in secs)
	
	CFVec3A PosWS;
	CFVec3A VelPerSec;

	CFVec3A BubbleForce;

	CFColorRGB InitRGB;
	CFColorRGB FinalRGB;
	f32 fColorPercent;

	f32 fInitAlpha;
	f32 fFinalAlpha;
	f32 fAlphaPercent;
	
	f32 fInitSize;
	f32 fFinalSize;
	f32 fSizePercent;

	f32 fEmissiveIntensity;

	FLink_t BurstLink;	// this is used to create the burst's linklist
	FLink_t Link;		// this is used to create the emitter or free pool linklist

	FCLASS_STACKMEM_ALIGN( _Particle_t );
} FCLASS_ALIGN_SUFFIX;

enum {
	_EMITTER_INIT_FLAGS_KEEP_POS_PTR		= 0x00000001,
	_EMITTER_INIT_FLAGS_EMIT_SHAPE_MOVES	= 0x00000002,
	_EMITTER_INIT_FLAGS_KILL_COLL_PARTICLES = 0x00000004,
	_EMITTER_INIT_FLAGS_KEEP_VEL_PTR		= 0x00000008,
	_EMITTER_INIT_FLAGS_KEEP_INTENSITY_PTR	= 0x00000010,

	_EMITTER_INIT_FLAGS_NONE				= 0
};

enum {
	_EMITTER_FLAGS_IN_USE							= 0x00000001,// an emitter is in use
	_EMITTER_FLAGS_PAUSE							= 0x00000002,// freeze the particles and time	
	_EMITTER_FLAGS_ENABLE_EMIT						= 0x00000004,// allow burst emission to occur
	_EMITTER_FLAGS_KILL_ON_LAST_PARTICLE			= 0x00000008,// once the last particle dies, kill the emitter
	_EMITTER_FLAGS_EMIT_LIGHTS						= 0x00000010,// emit lights, inited to what the def file says, but can be cleared by the user
	_EMITTER_FLAGS_EMITTER_MOVES					= 0x00000020,// the emitter moves, use the more expensive UpdateTracker call
	_EMITTER_FLAGS_KILL_EXITING_PARTICLES			= 0x00000040,// instead of bouncing particles off coll shape, kill them when they change sides
	_EMITTER_FLAGS_KEEP_PARTICLES_INSIDE			= 0x00000080,// used to determine what side of the coll shape all particles should always be on
	_EMITTER_FLAGS_FIRST_WORK						= 0x00000100,// only set in the Init() and unset after 1st work call
	_EMITTER_FLAGS_FIRST_BURST						= 0x00000200,// only set in the Init() and unset after 1st burst is emitted
	_EMITTER_FLAGS_BURST_DELAYS_ON					= 0x00000400,// when TRUE, there is a short delay after a series of bursts (not used for 1 shot effects)
	_EMITTER_FLAGS_ENABLE_BURST_SOUND				= 0x00000800,// if a sound has been specified for the particle, this flag enables the playing of the sound
	_EMITTER_FLAGS_EMITTER_WAS_DRAWN_AT_LEAST_ONCE	= 0x00001000,// set if the particle group was drawn
	_EMITTER_FLAGS_KILL_IF_NOT_DRAW_THIS_FRAME		= 0x00002000,// kill this emitter if it is not drawn this frame

	_EMITTER_FLAGS_NONE						= 0
};

typedef struct {
	CFColorRGB m_LightRGB;
	f32 m_fLightIntensity;
	f32 m_fLightRadiusMultiplier;
} _BurstLightSetting_t;

/////////////////////////////////
// EMITS PARTICLES INTO THE WORLD
/////////////////////////////////
FCLASS_ALIGN_PREFIX class CPEmitter : public CFWorldPSGroup 
{
public:
	CPEmitter();

	BOOL Init( FParticleDef_t *pDef, 
			   const f32 *pfUnitIntensity,
			   const CFVec3 *pPos,
			   const CFVec3 *pUnitDir,
			   const CFVec3 *pVelPerSec,
			   const FParticleShape_t *pEmitShape,
			   const FParticleShape_t *pCollShape,
			   u32 nInitFlags );	// see _EMITTER_INIT_FLAGS_...
	void SetPSGroupFlagsAndTexture( FParticleDef_t *pDef );
	// called when all resources that this emitter is using should be returned and the emitter removed from the world
	void Kill();

	// called from fparticle_Work, returns TRUE when the emitter should be killed
	BOOL Work( f32 fSecsSinceLastWork );

	FINLINE void EnableBurstSound( BOOL bEnable )			{ FMATH_WRITEBIT( bEnable, m_nFlags, _EMITTER_FLAGS_ENABLE_BURST_SOUND ); }
	FINLINE void SetIntensity( const f32 &rfIntensity )		{ m_fUnitIntensity = rfIntensity; m_pfUnitIntensity = &m_fUnitIntensity; }
	FINLINE void SetIntensity( const f32 *pfIntensity )		{ m_pfUnitIntensity = (f32 *)pfIntensity; }
	FINLINE f32 GetIntensity()								{ return *m_pfUnitIntensity; }
	FINLINE void SetAlphaBias( const f32 &rfAlphaBias )		{ m_fBiasAlpha = rfAlphaBias; m_pfBiasAlpha = &m_fBiasAlpha; }
	FINLINE void SetAlphaBias( const f32 *pfAlphaBias )		{ m_pfBiasAlpha = (f32 *)pfAlphaBias; }
	void DoNotEmitLights();
	FINLINE void SetUnitDir( const CFVec3 *pUnitDir )		{ m_pUnitDir = (CFVec3 *)pUnitDir; }
	FINLINE void SetUnitDir( const CFVec3 &rUnitDir )		{ m_UnitDir = rUnitDir; m_pUnitDir = &m_UnitDir; }
	FINLINE void SetPosition( const CFVec3 &rPos )			{ m_Pos = rPos;  m_pPos = &m_Pos; }
	void Pause( BOOL bPause );
	void EnableEmission( BOOL bEnable );
	void Stop();
	FINLINE BOOL SetupSprites( f32 fDistToCamSq );
	FINLINE void SetDurationMultiplier( f32 fMultiplier );
	FINLINE void SetPositionalOffset( f32 fOffset );
	void SetupFramePtr( const FParticleDef_t *pDef, f32 fUnitIntensity );
	void InitBurst( _Burst_t *pBurst, const FParticleKeyFrame_t *pKeyFrame );
	void TurnOffLights();
	FINLINE void GetLocalXYUnitVec( const CFVec3A &rUnitDir, CFVec3A &rLocalX, CFVec3A &rLocalY ) const;
	
	u32 m_nSignature;			// to ensure the validity of handles and pointers (MUST ALWAYS BE SET TO _EMITTER_SIGNATURE)

	u32 m_nFlags;				// see _EMITTER_FLAGS_...
	FLinkRoot_t m_BurstList;	// a linklist of _Burst_t
	FLinkRoot_t m_ParticleList;	// a linklist of _Particle_t
	FLink_t m_Link;				// for inserting the emitter into the particle def or unused linklists
	const FParticleDef_t *m_pDef;	// the particle definition file
	FParticleKillCallback_t *m_pCallback;
	void *m_pCallbackUserData;

	CFVec3 m_UnitDir;				// holds non-animating unit emission direction
	CFVec3 *m_pUnitDir;				// usually will point to m_UnitDir, but if animating then will point somewhere else

	CFVec3 m_Pos;					// holds the non-animating position of the next burst location
	CFVec3 *m_pPos;					// usually will point to m_Pos, but if animating then will point somewhere else
	f32 m_fPositionOffset;			// how far along the unit dir should emission actually occur (default is 0.0f)

	// hold dynamic light data on the last burst 
	CFVirtualWorldLight *m_pVLight;
	_BurstLightSetting_t m_StartBurstLight;
	_BurstLightSetting_t m_EndBurstLight;
	f32 m_fMaxLightRadius;

	// holds the sprite culling data
    f32 m_fStartSkipDrawDist;
	f32 m_fOOSkipDrawRange;
	f32 m_fMinPercentToDraw;

protected:
	static BOOL _PreDrawCallback( CFWorldPSGroup *pPSGroup, f32 fDistToCamSq );
	
private:
	f32 m_fTotalSecs;				// how many secs has this emitter been active
	f32 m_fBurstSecs;				// only burst while m_fTotalSecs < m_fBurstSecs
	f32 m_fBurstMultiplier;			// when setting m_fBurstSecs, multiply the computed value by this multiplier
	f32 m_fSecsTillNextBurst;		// secs till the next burst, burst when <= 0.0f
	f32 m_fPeriodTimer;				// keeps track of the amount of time we have been in this period or cycle (if in
	f32 m_fTotalPeriodSecs;			// how long is the burst & delay times for 1 cycle

	f32 m_fSecsTillNextLightSample; // secs till the next light sample
	CFColorRGB m_SampledWorldColor;	// used to sample the world's light to help properly color the emitter's particles
	
	f32 m_fUnitIntensity;			// holds non-animating unit intensity
	f32 *m_pfUnitIntensity;			// usually will point to m_fUnitIntensity, but if animating then will point somewhere else

	f32 m_fBiasAlpha;				// holds non-animating a bias
	f32 *m_pfBiasAlpha;				// usually will point to m_fBiasAlpha, but if animating then will point somewhere else

	const FParticleShape_t *m_pEmitShape;
	CFVec3 *m_pEmitterVelocityPerSec;
	const FParticleShape_t *m_pCollShape;
	
	FINLINE void AgeTheParticles( f32 fSecsSinceLastWork );
	FINLINE void EvalAlphaFunction();
	FINLINE void EvalColorFunction();
	FINLINE void EvalScaleFunction();
	FINLINE void UpdatePosAndVelocity( f32 fSecsSinceLastWork );
	FINLINE f32 CalculatePercentOfParticlesToDraw( f32 fDistToCam );
	FINLINE f32 Eval_Constant_Func( const FParticleAnimFunc_t *pFuncData, f32 fUnitAge );
	FINLINE f32 Eval_LookupTable_Func( const FParticleAnimFunc_t *pFuncData, f32 fUnitAge );
	FINLINE f32 Eval_LinearTime_Func( const FParticleAnimFunc_t *pFuncData, f32 fUnitAge );
	FINLINE f32 Eval_Random_Func( const FParticleAnimFunc_t *pFuncData, f32 fUnitAge );
	FINLINE f32 Eval_Time2_Func( const FParticleAnimFunc_t *pFuncData, f32 fUnitAge );
	FINLINE f32 Eval_Time3_Func( const FParticleAnimFunc_t *pFuncData, f32 fUnitAge );
	FINLINE void AddParticle( _Particle_t *pPart );
	FINLINE void RemoveParticle( _Particle_t *pPart );
	FINLINE f32 PickValueFromMinAndDelta( const CFVec2 &rVec2 );
	FINLINE f32 PickValueFromMinAndDelta( const CFVec2 &rVec2, f32 fRandom );
	FINLINE void RandomUnitVec3( CFVec3A &rResult );
	FINLINE void CalcMinMaxParticleBox( CFVec3A &rMin, CFVec3A &rMax );
	FINLINE void LightWork();
	FINLINE void ComputeBurstPosAndDir( CFVec3A &rPos, BOOL &rbDirectional,
										CFVec3A &rUnitDir, CFMtx43A &rMtx43 );
	
											   
	FCLASS_STACKMEM_ALIGN( CPEmitter );
} FCLASS_ALIGN_SUFFIX;

//=================
// public variables

//==================
// private variables

typedef enum {
	_SYSTEM_STATE_NONE = 0,
	_SYSTEM_STATE_STARTUP_OK,
	_SYSTEM_STATE_INSTALL_OK,

	_SYSTEM_STATE_COUNT
} _SystemState_e;

	// system vars:
static _SystemState_e _nSystemState = _SYSTEM_STATE_NONE;
static FResLoadReg_t _ResLoadRegistration;
static f32 _fTooCloseDropPercent = _TOO_CLOSE_DROP_PERCENT;

	// linklist vars:
static FLinkRoot_t _LoadedDefFiles;		// linklist of loaded FParticleDef_t
static FLinkRoot_t _BurstPool;			// linklist of _Burst_t
static FLinkRoot_t _ParticlePool;		// linklist of _Particle_t
static FLinkRoot_t _EmitterPool;		// linklist of CPEmitter

// allocated memory that needs to be freed:
static CPEmitter *_paAllocatedEmitters;
static _Burst_t *_paAllocatedBursts;
static _Particle_t *_paAllocatedParticles;
static FPSprite_t *_paAllocatedSprites;

// stats used to track memory usage:
static u32 _nNumEmittersAllocated, _nMaxEmittersUsed;
static u32 _nNumBurstsAllocated, _nMaxBurstsUsed;
static u32 _nNumParticlesAllocated, _nMaxParticlesUsed;
static u32 _nNumSpritesAllocated, _nMaxSpritesUsed;
static u32 _nCurrentParticles, _nCurrentSprites;

// temp vars for optimizations:
static FParticleKeyFrame_t _KeyFrameInfo;
static FParticleKeyFrame_t *_pFrame;

//===================
// private prototypes

static BOOL _ResLoadCreate( FResHandle_t hRes, void *pLoadedBase, u32 nLoadedBytes, cchar *pszResName );
static void _ResLoadDestroy( void *pBase );
static void _ReleaseAllEmitters( FParticleDef_t *pDef );

FINLINE FParticleDef_t *_GetParticleDefPtr( FParticle_DefHandle_t hParticleDef );
FINLINE CPEmitter *_GetEmitterPtr( FParticle_EmitterHandle_t hEmitter );

static BOOL _Install( u32 nMaxEmitters, u32 nMaxParticles, u32 nMaxSpritesPerEmitter );

//=================
// public functions

BOOL fparticle_ModuleStartup( void ) {

	_nSystemState = _SYSTEM_STATE_NONE;

	// init our allocated memory vars
	_paAllocatedEmitters = NULL;
	_paAllocatedBursts = NULL;
	_paAllocatedParticles = NULL;
	
	// init our linklist vars
	flinklist_InitRoot( &_LoadedDefFiles, FANG_OFFSETOF( FParticleDef_t, Link ) );
	flinklist_InitRoot( &_BurstPool, FANG_OFFSETOF( _Burst_t, Link ) );
	flinklist_InitRoot( &_ParticlePool, FANG_OFFSETOF( _Particle_t, Link ) );
	flinklist_InitRoot( &_EmitterPool, FANG_OFFSETOF( CPEmitter, m_Link ) );

	// register our fres loader function
	fres_CopyType( _ResLoadRegistration.sResType, FPARTICLE_RESTYPE );
	_ResLoadRegistration.pszFileExtension = "fpr";
	_ResLoadRegistration.nMemType = FRESLOAD_MEMTYPE_PERM;
	_ResLoadRegistration.nAlignment = 4;
	_ResLoadRegistration.pFcnCreate = _ResLoadCreate;
	_ResLoadRegistration.pFcnDestroy = _ResLoadDestroy;

	if( !fresload_RegisterHandler( &_ResLoadRegistration ) ) {
		// Registration failed...
		DEVPRINTF( "fparticle_ModuleStartup(): Could not register resource, particle system will be disabled.\n" );
		return FALSE;
	}

	_fTooCloseDropPercent = _TOO_CLOSE_DROP_PERCENT;

	_nSystemState = _SYSTEM_STATE_STARTUP_OK;

#if _USE_POOLED_RANDOM_FLOATS
	_ResetRandPool();	
#endif

	// go ahead and install the particle system
	return _Install( Fang_ConfigDefs.nMaxParticleEmitters, Fang_ConfigDefs.nMaxParticles, Fang_ConfigDefs.nMaxParticleEmitterSprites );
}

void fparticle_ModuleShutdown( void ) {

	if( _paAllocatedEmitters ) {
		fdelete_array( _paAllocatedEmitters );
		_paAllocatedEmitters = NULL;
	}
	if( _paAllocatedBursts ) {
		fdelete_array( _paAllocatedBursts );
		_paAllocatedBursts = NULL;
	}
	if( _paAllocatedParticles ) {
		fdelete_array( _paAllocatedParticles );
		_paAllocatedParticles = NULL;
	}
	_paAllocatedSprites = NULL;

	// print our our memory usage vars
	if( _nSystemState == _SYSTEM_STATE_INSTALL_OK ) {
		u32 nMemoryThatCouldBeSaved;
		nMemoryThatCouldBeSaved = ( (_nNumEmittersAllocated - _nMaxEmittersUsed) * sizeof( CPEmitter ) ) +
								  ( (_nNumBurstsAllocated - _nMaxBurstsUsed) * sizeof( _Burst_t ) ) +
								  ( (_nNumParticlesAllocated - _nMaxParticlesUsed) * sizeof( _Particle_t ) ) +
								  ( (_nNumSpritesAllocated - _nMaxSpritesUsed) * sizeof( FPSprite_t ) );
		DEVPRINTF( "fparticle usage stats: Emitters %d of %d, Bursts %d of %d, Particles %d of %d, Sprites %d of %d, Potenial memory savings %d bytes.\n",
					_nMaxEmittersUsed, _nNumEmittersAllocated, 
					_nMaxBurstsUsed, _nNumBurstsAllocated, 
					_nMaxParticlesUsed, _nNumParticlesAllocated, 
					_nMaxSpritesUsed, _nNumSpritesAllocated, 
					nMemoryThatCouldBeSaved );
	}

	_nSystemState = _SYSTEM_STATE_NONE;
}

void fparticle_KillAllEmitters( void ) {
	FParticleDef_t *pDef;

	if( !(_nSystemState & _SYSTEM_STATE_INSTALL_OK) ) {
		return;
	}
	
	// run through each loaded particle def, killing all emitters
	pDef = (FParticleDef_t *)flinklist_GetHead( &_LoadedDefFiles );
	while( pDef ) {
		_ReleaseAllEmitters( pDef );		

		// get the next loaded particle def
		pDef = (FParticleDef_t *)flinklist_GetNext( &_LoadedDefFiles, pDef );
	}
}

void fparticle_StopAllEmitters( void ) {
	FParticleDef_t *pDef;
	CPEmitter *pEmitter;

	if( !(_nSystemState & _SYSTEM_STATE_INSTALL_OK) ) {
		return;
	}
	
	// run through each loaded particle def, stopping all emitters
	pDef = (FParticleDef_t *)flinklist_GetHead( &_LoadedDefFiles );
	while( pDef ) {
		pEmitter = (CPEmitter *)flinklist_GetHead( &pDef->EmitterList );
		while( pEmitter ) {
			pEmitter->Stop();

			pEmitter = (CPEmitter *)flinklist_GetNext( &pDef->EmitterList, pEmitter );
		}

		// get the next loaded particle def
		pDef = (FParticleDef_t *)flinklist_GetNext( &_LoadedDefFiles, pDef );
	}
}

void fparticle_Work( void ) {
	FParticleDef_t *pDef;
	CPEmitter *pEmitter, *pNext;

	if( FLoop_bGamePaused ) {
		return;
	}

	if( !(_nSystemState & _SYSTEM_STATE_INSTALL_OK) ) {
		return;
	}
#if _USE_POOLED_RANDOM_FLOATS
	_ResetRandPool();	
#endif

	_nCurrentParticles = 0;
	_nCurrentSprites = 0;

	// run through each loaded particle def, calling the work function
	pDef = (FParticleDef_t *)flinklist_GetHead( &_LoadedDefFiles );
	while( pDef ) {
		// walk all of the emitters of this type
		pEmitter = (CPEmitter *)flinklist_GetHead( &pDef->EmitterList );
		while( pEmitter ) {
			pNext = (CPEmitter *)flinklist_GetNext( &pDef->EmitterList, pEmitter );

			if( pEmitter->Work( FLoop_fPreviousLoopSecs ) ) {
				// pEmitter needs to be removed and returned to the free pool
				pEmitter->Kill();
				flinklist_Remove( &pDef->EmitterList, pEmitter );
				flinklist_AddTail( &_EmitterPool, pEmitter );			
			} else {
				_nCurrentParticles += pEmitter->m_ParticleList.nCount;
			}

			// goto the next emitter in the list
			pEmitter = pNext;
		}		

		// get the next loaded particle def
		pDef = (FParticleDef_t *)flinklist_GetNext( &_LoadedDefFiles, pDef );
	}

#if 0
	// print out the number of used lights
	u32 nNumRealLights, nNumVLights;
	CFLightGroupMgr::GetCounts( nNumRealLights, nNumVLights );
	ftext_Printf( 0.40f, 0.65f, L"~f1~C85900575~w0~aL~s0.95%d lights, %d VLs", nNumRealLights, nNumVLights );
#endif

	// update our stats
	_nMaxEmittersUsed = FMATH_MAX( (_nNumEmittersAllocated - _EmitterPool.nCount), _nMaxEmittersUsed );
	_nMaxBurstsUsed = FMATH_MAX( (_nNumBurstsAllocated - _BurstPool.nCount), _nMaxEmittersUsed );
	_nMaxParticlesUsed = FMATH_MAX( (_nNumParticlesAllocated - _ParticlePool.nCount), _nMaxParticlesUsed );	
}

u32 fparticle_GetFlags( FParticle_DefHandle_t hParticleDef ) {
	FParticleDef_t *pPartDef = _GetParticleDefPtr( hParticleDef );

	return pPartDef->nFlags;
}

FParticle_EmitterHandle_t fparticle_SpawnEmitter( FParticle_DefHandle_t hParticleDef, 
												  const CFVec3 &rPos,
												  const CFVec3 *pUnitDir/*=NULL*/,	// any direction vec will be copied
												  f32 fUnitIntensity/*=1.0f*/ ) {
	if( hParticleDef == FPARTICLE_INVALID_HANDLE ) {
		return FPARTICLE_INVALID_HANDLE;
	}
	if( !(_nSystemState & _SYSTEM_STATE_INSTALL_OK) ) {
		return FPARTICLE_INVALID_HANDLE;
	}
	FParticleDef_t *pPartDef = _GetParticleDefPtr( hParticleDef );
	CPEmitter *pEmitter = (CPEmitter *)flinklist_RemoveHead( &_EmitterPool );

	if( !pEmitter ) {
		return FPARTICLE_INVALID_HANDLE;
	}
	
	FASSERT( pEmitter->m_BurstList.nCount == 0 );
	FASSERT( pEmitter->m_ParticleList.nCount == 0 );

	if( !pEmitter->Init( pPartDef, 
						 &fUnitIntensity,
						 &rPos,
						 pUnitDir,
						 NULL, 
						 NULL,
						 NULL,
						 _EMITTER_INIT_FLAGS_NONE ) ) {
		// trouble initing the emitter, return it to the pool and fail
		flinklist_AddTail( &_EmitterPool, pEmitter );
		return FPARTICLE_INVALID_HANDLE;
	}

	// add this emitter to the def's linklist
	flinklist_AddTail( &pPartDef->EmitterList, pEmitter );
	
	return (FParticle_EmitterHandle_t)pEmitter;
}

FParticle_EmitterHandle_t fparticle_SpawnEmitter( FParticle_DefHandle_t hParticleDef, 
												  const CFVec3 *pPos,			// must point to perm memory	
												  const CFVec3 *pUnitDir/*=NULL*/,	// any direction vec will be copied
												  const CFVec3 *pEmitterVelocityPerSec/*=NULL*/,// must point to perm memory
												  f32 fUnitIntensity/*=1.0f*/ ) {
	if( hParticleDef == FPARTICLE_INVALID_HANDLE ) {
		return FPARTICLE_INVALID_HANDLE;
	}
	if( !(_nSystemState & _SYSTEM_STATE_INSTALL_OK) ) {
		return FPARTICLE_INVALID_HANDLE;
	}
	FParticleDef_t *pPartDef = _GetParticleDefPtr( hParticleDef );
	CPEmitter *pEmitter = (CPEmitter *)flinklist_RemoveHead( &_EmitterPool );

	FASSERT( pPos );

	if( !pEmitter ) {
		return FPARTICLE_INVALID_HANDLE;
	}	

	u32 nFlags = _EMITTER_INIT_FLAGS_NONE;
	nFlags |= _EMITTER_INIT_FLAGS_KEEP_POS_PTR;
	if( pEmitterVelocityPerSec ) {
		nFlags |= _EMITTER_INIT_FLAGS_KEEP_VEL_PTR;
	}

	FASSERT( pEmitter->m_BurstList.nCount == 0 );
	FASSERT( pEmitter->m_ParticleList.nCount == 0 );

	if( !pEmitter->Init( pPartDef, 
						 &fUnitIntensity,
						 pPos,
						 pUnitDir,
						 pEmitterVelocityPerSec, 
						 NULL,
						 NULL,
						 nFlags ) ) {
		// trouble initing the emitter, return it to the pool and fail
		flinklist_AddTail( &_EmitterPool, pEmitter );
		return FPARTICLE_INVALID_HANDLE;
	}

	// add this emitter to the def's linklist
	flinklist_AddTail( &pPartDef->EmitterList, pEmitter );
	
	return (FParticle_EmitterHandle_t)pEmitter;
}

void fparticle_SetCfgToDefaults( FParticleEmitCfg_t *pConfig ) {

	if( !(_nSystemState & _SYSTEM_STATE_INSTALL_OK) ) {
		return;
	}

	pConfig->fUnitIntensity = 1.0f;
	pConfig->pfUnitIntensity = NULL;
	pConfig->pEmitShape = NULL;
	pConfig->bEmitShapeWillMove = FALSE;
	pConfig->pEmitterVelocityPerSec = NULL;
	pConfig->pCollShape = NULL;
	pConfig->bKillCollidingParticles = TRUE;
}

FParticle_EmitterHandle_t fparticle_SpawnEmitter( FParticle_DefHandle_t hParticleDef,
												  FParticleEmitCfg_t *pConfig ) {
	if( hParticleDef == FPARTICLE_INVALID_HANDLE ) {
		return FPARTICLE_INVALID_HANDLE;
	}
	if( !(_nSystemState & _SYSTEM_STATE_INSTALL_OK) ) {
		return FPARTICLE_INVALID_HANDLE;
	}
	FParticleDef_t *pPartDef = _GetParticleDefPtr( hParticleDef );
	CPEmitter *pEmitter = (CPEmitter *)flinklist_RemoveHead( &_EmitterPool );
	f32 *pfIntensity;

	FASSERT( pConfig );

	if( !pEmitter ) {
		return FPARTICLE_INVALID_HANDLE;
	}

	// setup the flags
	u32 nFlags = _EMITTER_INIT_FLAGS_NONE;
	
	FASSERT( pConfig->pEmitShape );
	if( pConfig->bEmitShapeWillMove ) {
		nFlags |= _EMITTER_INIT_FLAGS_EMIT_SHAPE_MOVES;
	}
	if( pConfig->pEmitterVelocityPerSec ) {
		nFlags |= _EMITTER_INIT_FLAGS_KEEP_VEL_PTR;
	}
	if( pConfig->bKillCollidingParticles ) {
		nFlags |= _EMITTER_INIT_FLAGS_KILL_COLL_PARTICLES;
	}
	if( pConfig->pfUnitIntensity ) {
		nFlags |= _EMITTER_INIT_FLAGS_KEEP_INTENSITY_PTR;
		pfIntensity = pConfig->pfUnitIntensity;
	} else {
		pfIntensity = &pConfig->fUnitIntensity;
	}

	FASSERT( pEmitter->m_BurstList.nCount == 0 );
	FASSERT( pEmitter->m_ParticleList.nCount == 0 );

	if( !pEmitter->Init( pPartDef, 
						 pfIntensity,
						 NULL,
						 NULL,
						 pConfig->pEmitterVelocityPerSec, 
						 pConfig->pEmitShape,
						 pConfig->pCollShape,
						 nFlags ) ) {
		// trouble initing the emitter, return it to the pool and fail
		flinklist_AddTail( &_EmitterPool, pEmitter );
		return FPARTICLE_INVALID_HANDLE;
	}

	// add this emitter to the def's linklist
	flinklist_AddTail( &pPartDef->EmitterList, pEmitter );
	
	return (FParticle_EmitterHandle_t)pEmitter;	
}

const CFSphere *fparticle_GetBoundingSpherePtr( FParticle_EmitterHandle_t hEmitter ) {
	CPEmitter *pEmitter = _GetEmitterPtr( hEmitter );

	if( !(_nSystemState & _SYSTEM_STATE_INSTALL_OK) ||
		!pEmitter ) {
		return NULL;
	}

	return &pEmitter->m_BoundSphere_MS;// ms for our psgroups is world space
}

BOOL fparticle_SetKillCallback( FParticle_EmitterHandle_t hEmitter, FParticleKillCallback_t *pCallback, void *pUserData ) {
	CPEmitter *pEmitter = _GetEmitterPtr( hEmitter );

	if( !(_nSystemState & _SYSTEM_STATE_INSTALL_OK) ||
		!pEmitter ) {
		return FALSE;
	}
	pEmitter->m_pCallback = pCallback;
	pEmitter->m_pCallbackUserData = pUserData;

	return TRUE;
}

const CFWorldTracker *fparticle_GetTrackerPtr( FParticle_EmitterHandle_t hEmitter ) {
	CPEmitter *pEmitter = _GetEmitterPtr( hEmitter );

	return pEmitter;// since an emitter is a tracker
}

void fparticle_PauseEmitter( FParticle_EmitterHandle_t hEmitter, BOOL bPause ) {
	CPEmitter *pEmitter = _GetEmitterPtr( hEmitter );

	if( !pEmitter ) {
		return;
	}
	pEmitter->Pause( bPause );
}

void fparticle_EnableEmission( FParticle_EmitterHandle_t hEmitter, BOOL bEnable ) {
	CPEmitter *pEmitter = _GetEmitterPtr( hEmitter );

	if( !pEmitter ) {
		return;
	}
	pEmitter->EnableEmission( bEnable );
}

void fparticle_KillEmitter( FParticle_EmitterHandle_t hEmitter ) {
	if( hEmitter == FPARTICLE_INVALID_HANDLE ) {
		return;
	}

	CPEmitter *pEmitter = _GetEmitterPtr( hEmitter );
	if( !pEmitter ) {
		return;
	}
	FParticleDef_t *pPartDef = (FParticleDef_t *)pEmitter->m_pDef;

	pEmitter->Kill();

	// remove pEmitter from the def linklist and return it to the free pool
	flinklist_Remove( &pPartDef->EmitterList, pEmitter );
	flinklist_AddTail( &_EmitterPool, pEmitter );
}

void fparticle_StopEmitter( FParticle_EmitterHandle_t hEmitter ) {
	CPEmitter *pEmitter = _GetEmitterPtr( hEmitter );

	if( !pEmitter ) {
		return;
	}
	pEmitter->Stop();
}

void fparticle_SetIntensity( FParticle_EmitterHandle_t hEmitter, const f32 &rfIntensity ) {
	CPEmitter *pEmitter = _GetEmitterPtr( hEmitter );

	if( !pEmitter ) {
		return;
	}
	pEmitter->SetIntensity( rfIntensity );
}

void fparticle_EnableBurstSound( FParticle_EmitterHandle_t hEmitter, const BOOL &rbEnableBurstSound ) {
	CPEmitter *pEmitter = _GetEmitterPtr( hEmitter );

	if( !pEmitter ) {
		return;
	}
	pEmitter->EnableBurstSound( rbEnableBurstSound );
}

void fparticle_SetIntensity( FParticle_EmitterHandle_t hEmitter, const f32 *pfIntensity ) {
	CPEmitter *pEmitter = _GetEmitterPtr( hEmitter );

	if( !pEmitter ) {
		return;
	}
	pEmitter->SetIntensity( pfIntensity );
}

void fparticle_SetAlphaBias( FParticle_EmitterHandle_t hEmitter, const f32 &rfAlphaBias ) {
	CPEmitter *pEmitter = _GetEmitterPtr( hEmitter );

	if( !pEmitter ) {
		return;
	}
	pEmitter->SetAlphaBias( rfAlphaBias );
}

void fparticle_SetAlphaBias( FParticle_EmitterHandle_t hEmitter, const f32 *pfAlphaBias ) {
	CPEmitter *pEmitter = _GetEmitterPtr( hEmitter );

	if( !pEmitter ) {
		return;
	}
	pEmitter->SetAlphaBias( pfAlphaBias );
}

void fparticle_DoNotEmitLights( FParticle_EmitterHandle_t hEmitter ) {
	CPEmitter *pEmitter = _GetEmitterPtr( hEmitter );

	if( !pEmitter ) {
		return;
	}
	pEmitter->DoNotEmitLights();
}

void fparticle_SetDirection( FParticle_EmitterHandle_t hEmitter, const CFVec3 *pUnitDir ) {
	CPEmitter *pEmitter = _GetEmitterPtr( hEmitter );

	if( !pEmitter ) {
		return;
	}
	pEmitter->SetUnitDir( pUnitDir );
}

void fparticle_SetDurationMultiplier( FParticle_EmitterHandle_t hEmitter, f32 fTimeMultiplier ) {
	CPEmitter *pEmitter = _GetEmitterPtr( hEmitter );

	if( !pEmitter ) {
		return;
	}
	pEmitter->SetDurationMultiplier( fTimeMultiplier );
}

void fparticle_SetPositionalOffset( FParticle_EmitterHandle_t hEmitter, f32 fOffset ) {
	CPEmitter *pEmitter = _GetEmitterPtr( hEmitter );

	if( !pEmitter ) {
		return;
	}
	pEmitter->SetPositionalOffset( fOffset );
}

void fparticle_TransformParticleIntoNewSpace( FParticle_EmitterHandle_t hEmitter, CFMtx43A &rNewSpace ) {
	CPEmitter *pEmitter = _GetEmitterPtr( hEmitter );

	if( !pEmitter ) {
		return;
	}

	if( pEmitter->m_ParticleList.nCount ) {
		// create a matrix from the current settings
		CFVec3A Z, X, Y, Pos, Dir;
		Z.Set( *pEmitter->m_pUnitDir );
		Pos.Set( *pEmitter->m_pPos );
		pEmitter->GetLocalXYUnitVec( Z, X, Y );

		CFMtx43A OldMtx;
		OldMtx.Set( X, Y, Z, Pos );
		OldMtx.AffineInvert_KnowScale2( 1.0f );

        // walk all of the particles
		_Particle_t *pPart;
		pPart = (_Particle_t *)flinklist_GetHead( &pEmitter->m_ParticleList );
		while( pPart ) {
			// tranform the pts into old model space
			OldMtx.MulDir( Dir, pPart->VelPerSec );
			OldMtx.MulPoint( Pos, pPart->PosWS );
			
			// transform the pts into new world space
			pPart->PosWS = rNewSpace.MulPoint( Pos ).v3;
			pPart->VelPerSec = rNewSpace.MulDir( Dir ).v3;

			// move to the next particle
			pPart = (_Particle_t *)flinklist_GetNext( &pEmitter->m_ParticleList, pPart );
		}		
	}

	// update the pos and dir of the emitter
	pEmitter->SetPosition( rNewSpace.m_vPos.v3 );
	pEmitter->SetUnitDir( rNewSpace.m_vFront.v3 );
}

/////////////////////
// CPEmitter Methods
/////////////////////

CPEmitter::CPEmitter() {
	m_nFlags = _EMITTER_FLAGS_NONE;
	m_pDef = NULL;
	flinklist_InitRoot( &m_BurstList, FANG_OFFSETOF( _Burst_t, Link ) );
	flinklist_InitRoot( &m_ParticleList, FANG_OFFSETOF( _Particle_t, Link ) );

	m_nSignature = _EMITTER_SIGNATURE;
	m_pCallback = NULL;
	m_pCallbackUserData = NULL;

	m_pVLight = NULL;

	m_SampledWorldColor.White();

	// set the callback function
	SetPreDrawCallback( _PreDrawCallback );
}

// DOES NOT ADD THE EMITTER TO THE pDef->EmitterList LINK LIST
BOOL CPEmitter::Init( FParticleDef_t *pDef, 
					 const f32 *pfUnitIntensity,
					 const CFVec3 *pPos,
					 const CFVec3 *pUnitDir,
					 const CFVec3 *pVelPerSec,
					 const FParticleShape_t *pEmitShape,
					 const FParticleShape_t *pCollShape,
					 u32 nInitFlags ) {
	
	// INIT THE PSGROUP VARS
	m_Xfm.Identity();				// always set this to the identity
	
	if( pPos ) {
		m_BoundSphere_MS.m_Pos = *pPos;	
	} else {
		FASSERT( pEmitShape );

		m_BoundSphere_MS.m_Pos = pEmitShape->Box.pCenter->v3;
	}
	m_BoundSphere_MS.m_fRadius = _INITIAL_BS_RADIUS;
	
	// set our distance culling vars (just use the min values for now, the real starting values will be set during the 1st burst)
	m_fCullDist = pDef->Min.fCullDist;
	m_fEmulationDistance = pDef->Min.fEmulationDist;
	m_fStartSkipDrawDist = pDef->Min.fStartSkipDrawDist;
	m_fOOSkipDrawRange = fmath_Inv( (pDef->Min.fCullDist - m_fStartSkipDrawDist) );
	m_fMinPercentToDraw = 0.0f;
	
	SetPSGroupFlagsAndTexture( pDef );
	
	m_nMaxCount = _nNumSpritesAllocated;
	m_pBase = _paAllocatedSprites;
	m_nRenderCount = 0;
	m_nRenderStartIndex = 0;

	// INIT THE EMITTER VARS
	m_nFlags = (_EMITTER_FLAGS_IN_USE | _EMITTER_FLAGS_ENABLE_EMIT | _EMITTER_FLAGS_FIRST_WORK | _EMITTER_FLAGS_FIRST_BURST);
	m_pDef = pDef;
	
	// setup the intensity
	if( nInitFlags & _EMITTER_INIT_FLAGS_KEEP_INTENSITY_PTR ) {
		m_pfUnitIntensity = (f32 *)pfUnitIntensity;
	} else {
		m_fUnitIntensity = *pfUnitIntensity;
		m_pfUnitIntensity = &m_fUnitIntensity;
	}

	// setup the time vars to do burst on the 1st work call
	m_fTotalSecs = 0.0f;
#if _USE_POOLED_RANDOM_FLOATS
	m_fSecsTillNextLightSample = m_pDef->fSecsBetweenLightSamples * _GetPooledRandomFloat();
#else
	m_fSecsTillNextLightSample = m_pDef->fSecsBetweenLightSamples * fmath_RandomFloat();
#endif
	m_fSecsTillNextBurst = 0.0f;
	m_fBurstSecs = FMATH_FPOT( *m_pfUnitIntensity, pDef->Min.fSecsToEmit, pDef->Max.fSecsToEmit );
	m_fBurstMultiplier = 1.0f;
	m_fPeriodTimer = 0.0f;
	if( pDef->nFlags & FPARTICLE_DEF_FLAGS_BURST_FOREVER ) {
		m_fTotalPeriodSecs = FMATH_FPOT( *m_pfUnitIntensity, pDef->Min.EmitPause.x, pDef->Max.EmitPause.x );
		if( m_fTotalPeriodSecs > 0.0 ) {
			m_fTotalPeriodSecs += m_fBurstSecs;
			m_nFlags |= _EMITTER_FLAGS_BURST_DELAYS_ON;
		} else {
			m_fTotalPeriodSecs = 0.0f;
		}
	} else {
		m_fTotalPeriodSecs = 0.0f;
	}

	m_fPositionOffset = 0.0f;

	m_fBiasAlpha = 1.0f;
	m_pfBiasAlpha = &m_fBiasAlpha;

	m_UnitDir = (pUnitDir) ? *pUnitDir : CFVec3::m_UnitAxisY;
	m_pUnitDir = &m_UnitDir;

	if( pPos ) {
		if( nInitFlags & _EMITTER_INIT_FLAGS_KEEP_POS_PTR ) {
			m_pPos = (CFVec3 *)pPos;
			m_nFlags |= _EMITTER_FLAGS_EMITTER_MOVES;// assume that the emitter will be moving
		} else {
			m_Pos = *pPos;
			m_pPos = &m_Pos;
		}
	} else {
		m_pPos = &pEmitShape->Box.pCenter->v3;
	}

	if( pEmitShape ) {
		m_pEmitShape = pEmitShape;
		if( nInitFlags & _EMITTER_INIT_FLAGS_EMIT_SHAPE_MOVES ) {
			m_nFlags |= _EMITTER_FLAGS_EMITTER_MOVES;
		}
	} else {
		m_pEmitShape = NULL;
	}

	if( pCollShape ) {
		m_pCollShape = pCollShape;
		if( nInitFlags & _EMITTER_INIT_FLAGS_KILL_COLL_PARTICLES ) {
			m_nFlags |= _EMITTER_FLAGS_KILL_EXITING_PARTICLES;
		}
	} else {
		m_pCollShape = NULL;
	}

	if( pVelPerSec && (nInitFlags & _EMITTER_INIT_FLAGS_KEEP_VEL_PTR) ) {
		m_pEmitterVelocityPerSec = (CFVec3 *)pVelPerSec;		
	} else {
		m_pEmitterVelocityPerSec = NULL;
	}
		
	// only need to set the sampled color if we aren't sampling the world light
	if( !(pDef->nFlags & FPARTICLE_DEF_FLAGS_SAMPLE_LIGHTS) ) {
		m_SampledWorldColor.White();
	}

	// if the emitter says to emit a light, allow the emitter to emit lights
	if( pDef->nFlags & FPARTICLE_DEF_FLAGS_EMIT_LIGHT ) {
		m_nFlags |= _EMITTER_FLAGS_EMIT_LIGHTS;
	}

	m_pCallback = NULL;
	m_pCallbackUserData = NULL;

	m_pVLight = NULL;

	// tbd...test the emitter to coll shape to see if we should keep particle in or out of the coll shape
	
	// add to the world
	UpdateTracker();

	return TRUE;
}

void CPEmitter::SetPSGroupFlagsAndTexture( FParticleDef_t *pDef ) {

	m_nPSFlags = FPSPRITE_FLAG_NONE;
	if( pDef->nFlags & FPARTICLE_DEF_FLAGS_NO_FOG ) {
		m_nPSFlags |= FPSPRITE_FLAG_NO_FOG;
	}
	if( pDef->nFlags & FPARTICLE_DEF_FLAGS_Z_BUFFER ) {
		m_nPSFlags |= FPSPRITE_FLAG_ENABLE_DEPTH_WRITES;
	}
	if( pDef->nFlags & FPARTICLE_DEF_FLAGS_SORT ) {
		m_nPSFlags |= FPSPRITE_FLAG_SORT;
	}
	if( pDef->nFlags & FPARTICLE_DEF_FLAGS_SIZE_IN_PIXELS ) {
		m_nPSFlags |= FPSPRITE_FLAG_SIZE_IN_PIXELS;
	}
	if( pDef->nFlags & FPARTICLE_DEF_FLAGS_GROUP_ZSCALE ) {
		m_nPSFlags |= FPSPRITE_FLAG_UNIFORM_ZBASEDSCALE;
	}
	if( pDef->nFlags & FPARTICLE_DEF_FLAGS_DRAW_MODE_MODULATE ) {
		m_nBlend = FPSPRITE_BLEND_MODULATE;
	} else if( pDef->nFlags & FPARTICLE_DEF_FLAGS_DRAW_MODE_ADD ) {
		m_nBlend = FPSPRITE_BLEND_ADD;
	}	
	
	m_TexInst.SetTexDef( pDef->pTexDef );
}

// returns all resources that this emitter is using and returns everything that this emitter uses
// DOES NOT RETURN THE EMITTER PTR BACK TO THE EMITTER POOL
void CPEmitter::Kill() {
	_Burst_t *pBurst;
	_Particle_t *pPart, *pNextPart;

	// walk all of the particles and remove them 
	pPart = (_Particle_t *)flinklist_GetHead( &m_ParticleList );
	while( pPart ) {
		pNextPart = (_Particle_t *)flinklist_GetNext( &m_ParticleList, pPart );
		RemoveParticle( pPart );
		// move to the next particles
		pPart = pNextPart;
	}
	FASSERT( m_ParticleList.nCount == 0 );

	// walk all of the bursts and remove them
	pBurst = (_Burst_t *)flinklist_RemoveHead( &m_BurstList );
	while( pBurst ) {
		FASSERT( pBurst->ParticleList.nCount == 0 );
		
		// return the burst back to the burst pool
		flinklist_AddTail( &_BurstPool, pBurst );

		pBurst = (_Burst_t *)flinklist_RemoveHead( &m_BurstList );
	}

	// release the sprites
	m_pBase = NULL;

	// free the light that we may have
	TurnOffLights();

	if( m_pCallback ) {
		m_pCallback( (FParticle_EmitterHandle_t)this, m_pCallbackUserData );
		m_pCallback = NULL;
		m_pCallbackUserData = NULL;
	}

	// remove from world
	RemoveFromWorld();

	// reset a few key vars 
	m_pDef = NULL;
	m_nFlags = _EMITTER_FLAGS_NONE;
}

// returns TRUE when Kill() should be called because the emitter is done
// FALSE otherwise
BOOL CPEmitter::Work( f32 fSecsSinceLastWork ) {
	BOOL bEmitBurst, bFirstFrame;
	CFVec3A Min, Max, PosV3A, UnitDir, EmitterPos_WS;
	f32 fParticlesToEmit, fURand1, fURand2, fURand3, fURand4;
	_Burst_t *pBurst;
	_Particle_t *pPart;
	u32 nVisFlags;
	
	if( m_nFlags & _EMITTER_FLAGS_PAUSE ) {
		return FALSE;
	}
	
	// increase the time
	m_fTotalSecs += fSecsSinceLastWork;

	if( !(m_nFlags & _EMITTER_FLAGS_FIRST_WORK) ) {
		bFirstFrame = FALSE;
#if _USE_JOHNS_COOL_OPTIMIZATION
		// see if we can kill this emitter for being 1 shot and not being drawn
		if( m_nFlags & _EMITTER_FLAGS_KILL_IF_NOT_DRAW_THIS_FRAME ) {
			if( !(m_nFlags & _EMITTER_FLAGS_EMITTER_WAS_DRAWN_AT_LEAST_ONCE) ) {
				// it wasn't drawn, kill this emitter
				return TRUE;
			}
			// oh well, looks like we'll keep this emitter till its regular death
			m_nFlags &= ~_EMITTER_FLAGS_KILL_IF_NOT_DRAW_THIS_FRAME;
		}
#endif

		// determine if we should emit a burst this frame	
		if( m_pDef->nFlags & FPARTICLE_DEF_FLAGS_BURST_FOREVER ) {
			// handle emit forever this way
			if( m_nFlags & _EMITTER_FLAGS_BURST_DELAYS_ON ) {
				// emit for a while, wait, emit some more
				m_fPeriodTimer += fSecsSinceLastWork;
				if( m_fPeriodTimer <= m_fBurstSecs ) {
					// still in the bursting portion of the period
					m_fSecsTillNextBurst -= fSecsSinceLastWork;
					bEmitBurst = (m_nFlags & _EMITTER_FLAGS_ENABLE_EMIT) && (m_fSecsTillNextBurst <= 0.0f);
				} else if( m_fPeriodTimer >= m_fTotalPeriodSecs ) {
					// reset the period and emit
					bEmitBurst = (m_nFlags & _EMITTER_FLAGS_ENABLE_EMIT);
					m_fPeriodTimer = 0.0f;
				} else {
					bEmitBurst = FALSE;
				}
			} else {
				// simply emit forever
				m_fSecsTillNextBurst -= fSecsSinceLastWork;
				bEmitBurst = (m_nFlags & _EMITTER_FLAGS_ENABLE_EMIT) && (m_fSecsTillNextBurst <= 0.0f);
			}
		} else {
			// handle timed and one-shot this way
			
			if( m_fTotalSecs >= m_fBurstSecs ) {
				// don't emit anymore bursts
				m_nFlags &= ~_EMITTER_FLAGS_ENABLE_EMIT;
				m_nFlags |= _EMITTER_FLAGS_KILL_ON_LAST_PARTICLE;
				bEmitBurst = FALSE;
			} else {
				m_fSecsTillNextBurst -= fSecsSinceLastWork;
				bEmitBurst = (m_nFlags & _EMITTER_FLAGS_ENABLE_EMIT) && (m_fSecsTillNextBurst <= 0.0f);
			}
		}
	
		// do the work for the existing particles
		AgeTheParticles( fSecsSinceLastWork );
		EvalAlphaFunction();
		EvalColorFunction();
		EvalScaleFunction();
		UpdatePosAndVelocity( fSecsSinceLastWork );

		// tbd...collision work
	} else {
		// no longer the 1st work
		bFirstFrame = TRUE;
		m_nFlags &= ~_EMITTER_FLAGS_FIRST_WORK;
		bEmitBurst = TRUE;		
	}
	
	// sample the light
	m_fSecsTillNextLightSample -= fSecsSinceLastWork;
	if( ((m_pDef->nFlags & FPARTICLE_DEF_FLAGS_SAMPLE_LIGHTS) && (m_fSecsTillNextLightSample <= 0.0f)) || bFirstFrame ) {
		
		PosV3A.Set( m_BoundSphere_MS.m_Pos );
		fworld_LightPoint( &PosV3A, &m_SampledWorldColor, TRUE, this );
		m_SampledWorldColor.v3 = m_SampledWorldColor.v3 * m_pDef->SampledLightScale;
		
		// set the light sample timer
		m_fSecsTillNextLightSample += m_pDef->fSecsBetweenLightSamples;
	}	

	///////////////////////////////////////////////////////////////	
	// Emit a new burst only if we are in the active list right now
	if( bEmitBurst ) {
		nVisFlags = GetVolumeFlags();
		if( nVisFlags & FVIS_VOLUME_IN_ACTIVE_LIST ) {
			// setup our frame ptr
			SetupFramePtr( m_pDef, *m_pfUnitIntensity );

			// average the current LOD values with the new ones
			if( m_nFlags & _EMITTER_FLAGS_FIRST_BURST ) {
				m_nFlags &= ~_EMITTER_FLAGS_FIRST_BURST;
				// since this is the 1st frame, set our values to the frame values
				m_fCullDist = _pFrame->fCullDist;
				m_fStartSkipDrawDist = _pFrame->fStartSkipDrawDist;
				m_fOOSkipDrawRange = fmath_Inv( (_pFrame->fCullDist - m_fStartSkipDrawDist) );
				m_fMinPercentToDraw = _pFrame->fMinPercentToDraw;
				m_fEmulationDistance = _pFrame->fEmulationDist;
			} else {
				// not the 1st burst, animate to the new value to avoid popping
				m_fCullDist += _pFrame->fCullDist;
				m_fCullDist *= 0.5f;
				m_fStartSkipDrawDist += _pFrame->fStartSkipDrawDist;
				m_fStartSkipDrawDist *= 0.5f;
				m_fOOSkipDrawRange = fmath_Inv( (_pFrame->fCullDist - m_fStartSkipDrawDist) );
				m_fMinPercentToDraw += _pFrame->fMinPercentToDraw;
				m_fMinPercentToDraw *= 0.5f;		
				m_fEmulationDistance += _pFrame->fEmulationDist;
				m_fEmulationDistance *= 0.5f;
			}

			// grab a burst structure
			pBurst = (_Burst_t *)flinklist_RemoveHead( &_BurstPool );
			if( pBurst ) {
				// fill in the burst structure
				InitBurst( pBurst, _pFrame );

				// Handle sound spawning...
				if( m_nFlags & _EMITTER_FLAGS_ENABLE_BURST_SOUND ) {
					if( m_pDef->hSound != FSNDFX_INVALID_FX_HANDLE ) {
						EmitterPos_WS.Set( *m_pPos );
						fsndfx_Play3D( m_pDef->hSound, &EmitterPos_WS, 100.0f, -1.0f, _pFrame->fSndUnitVolume, _pFrame->fSndPitchMultiplier );
					}
				}

				// set the light vars
				if( m_nFlags & _EMITTER_FLAGS_EMIT_LIGHTS ) {
					// record the light settings for that will be used later
					m_StartBurstLight.m_LightRGB = _pFrame->StartLightRGB;
					m_StartBurstLight.m_fLightIntensity = _pFrame->fStartLightIntensity;
					m_StartBurstLight.m_fLightRadiusMultiplier = _pFrame->fStartLightRadiusMultiplier;

					m_EndBurstLight.m_LightRGB = _pFrame->EndLightRGB;
					m_EndBurstLight.m_fLightIntensity = _pFrame->fEndLightIntensity;
					m_EndBurstLight.m_fLightRadiusMultiplier = _pFrame->fEndLightRadiusMultiplier;					

					m_fMaxLightRadius = _pFrame->fMaxLightRadius;
				}

				BOOL bDirectional;

				ComputeBurstPosAndDir( PosV3A, bDirectional, UnitDir, CFMtx43A::m_Temp );

				if( m_pEmitterVelocityPerSec ) {
					pBurst->EmitterVelPerSec.Set( *m_pEmitterVelocityPerSec );					
					pBurst->nFlags |= _BURST_FLAGS_INITIAL_VEL_ADDED;
				} else {
					pBurst->EmitterVelPerSec.Zero();
				}

				fParticlesToEmit = PickValueFromMinAndDelta( _pFrame->NumPerBurst );
				if( !(nVisFlags & FVIS_VOLUME_IN_VISIBLE_LIST) ) {
					// cut down the number of particles emitted if they are not in the visible list
					fParticlesToEmit *= _EMISSION_CUT_FOR_NON_VISIBLE;
				}

				BOOL bBubble = (m_pDef->nFlags & FPARTICLE_DEF_FLAGS_BUBBLE_MOTION);

				do { 
					// grab a particle
					pPart = (_Particle_t *)flinklist_RemoveHead( &_ParticlePool );
					if( !pPart ) {
						break;
					}

					// grab some random numbers
#if _USE_POOLED_RANDOM_FLOATS
					fURand1 = _GetPooledRandomFloat();
					fURand2 = _GetPooledRandomFloat();
					fURand3 = _GetPooledRandomFloat();
					fURand4 = _GetPooledRandomFloat();
#else
					fURand1 = fmath_RandomFloat();
					fURand2 = fmath_RandomFloat();
					fURand3 = fmath_RandomFloat();
					fURand4 = fmath_RandomFloat();
#endif

					// init the particle fields
					pPart->pBurst = pBurst;
					
					// pick a position and direction
					if( bDirectional ) {
						// position
						if( _pFrame->fRandomInitPosRadius > 0.01f ) {
							pPart->PosWS.ReceiveRotationZ( CFVec3A::m_UnitAxisX, (fURand4 * FMATH_2PI) );
							pPart->PosWS.Mul( (fURand1 * _pFrame->fRandomInitPosRadius) );
							CFMtx43A::m_Temp.MulPoint( pPart->PosWS );
						} else {
							pPart->PosWS.Set( PosV3A );	
						}

						// pick a random unit direction in the dir cone
						fmath_SinCos( PickValueFromMinAndDelta( _pFrame->InitDirAngle, fURand2 ),
									  &pPart->VelPerSec.x, &pPart->VelPerSec.z );
						pPart->VelPerSec.y = 0.0f;
						pPart->VelPerSec.RotateZ( (fURand3 * (FMATH_2PI * 12.3456789f) ) );
						// scale the direction vec by the initial velocity
						pPart->VelPerSec.Mul( ( _pFrame->VelocityZ.x + (fURand4 * _pFrame->VelocityZ.y) ) );
						/// transform the dir into world space
						CFMtx43A::m_Temp.MulDir( pPart->VelPerSec );

					} else {
						// position
						if( _pFrame->fRandomInitPosRadius > 0.01f ) {
							RandomUnitVec3( pPart->PosWS );
							pPart->PosWS.Mul( (fURand1 * _pFrame->fRandomInitPosRadius) );
							pPart->PosWS.Add( PosV3A );
						} else {
							pPart->PosWS.Set( PosV3A );	
						}

						// pick a random direction and scale it up
						RandomUnitVec3( pPart->VelPerSec );
						pPart->VelPerSec.Mul( ( _pFrame->VelocityZ.x + (fURand2 * _pFrame->VelocityZ.y) ) );
					}
					// add on the emitter's velocity
					//pPart->VelPerSec.Add( pBurst->EmitterVelPerSec );	
					
					if( bBubble ) {
						pPart->BubbleForce.Zero();
					}

					pPart->fAge = 0.0f;
					pPart->fUnitAge = 0.0f;
					pPart->fOOLifeSpan = FMATH_FPOT( fURand3, _pFrame->OOLifeSecs.x, _pFrame->OOLifeSecs.y );

					if( m_pDef->nFlags & FPARTICLE_DEF_FLAGS_RANDOM_INIT_COLOR ) {
						pPart->InitRGB.RandomRGB();
					} else {
						pPart->InitRGB.fRed = PickValueFromMinAndDelta( _pFrame->InitRed, fURand4 );
						pPart->InitRGB.fGreen = PickValueFromMinAndDelta( _pFrame->InitGreen, fURand4 );
						pPart->InitRGB.fBlue = PickValueFromMinAndDelta( _pFrame->InitBlue, fURand4 );
					}
					
					if( m_pDef->nFlags & FPARTICLE_DEF_FLAGS_RANDOM_FINAL_COLOR ) {
						pPart->FinalRGB.RandomRGB();
					} else {
						pPart->FinalRGB.fRed = PickValueFromMinAndDelta( _pFrame->FinalRed, fURand1 );
						pPart->FinalRGB.fGreen = PickValueFromMinAndDelta( _pFrame->FinalGreen, fURand1 );
						pPart->FinalRGB.fBlue = PickValueFromMinAndDelta( _pFrame->FinalBlue, fURand1 );
					}
					pPart->fColorPercent = 0.0f;

					pPart->fInitAlpha = PickValueFromMinAndDelta( _pFrame->InitAlpha, fURand2 );
					pPart->fFinalAlpha = PickValueFromMinAndDelta( _pFrame->FinalAlpha, fURand3 );
					pPart->fAlphaPercent = 0.0f;
					
					pPart->fInitSize = PickValueFromMinAndDelta( _pFrame->InitScale, fURand3 );
					pPart->fFinalSize = PickValueFromMinAndDelta( _pFrame->FinalScale, fURand2 );
					pPart->fSizePercent = 0.0f;

					pPart->fEmissiveIntensity = PickValueFromMinAndDelta( _pFrame->EmissiveIntensity, fURand1 );
					
					// add the particle to the burst
					AddParticle( pPart );
					
					fParticlesToEmit--;
				} while( fParticlesToEmit >= 1.0f );

				if( pBurst->ParticleList.nCount ) {
					// the burst has at least 1 particle, add the burst to the emitter
					flinklist_AddTail( &m_BurstList, pBurst );
				} else {
					// return the burst to the burst pool
					flinklist_AddTail( &_BurstPool, pBurst );
				}
			}

			// pick another burst time
			if( !(m_pDef->nFlags & FPARTICLE_DEF_FLAGS_ONE_SHOT) ) {
#if _USE_POOLED_RANDOM_FLOATS
				fURand1 = _GetPooledRandomFloat();
				fURand2 = _GetPooledRandomFloat();
#else
				fURand1 = fmath_RandomFloat();
				fURand2 = fmath_RandomFloat();
#endif
				m_fSecsTillNextBurst = FMATH_FPOT( fURand1, _pFrame->SecsBetweenBursts.x, _pFrame->SecsBetweenBursts.y );
				m_fBurstSecs = (_pFrame->fSecsToEmit * m_fBurstMultiplier);

				m_fTotalPeriodSecs = FMATH_FPOT( fURand2, _pFrame->EmitPause.x, _pFrame->EmitPause.y );
				if( m_fTotalPeriodSecs > 0.0f ) {
					m_fTotalPeriodSecs *= m_fBurstMultiplier;
					m_fTotalPeriodSecs += m_fBurstSecs;
					m_nFlags |= _EMITTER_FLAGS_BURST_DELAYS_ON;
				} else {
					m_fTotalPeriodSecs = 0.0f;
					m_nFlags &= ~_EMITTER_FLAGS_BURST_DELAYS_ON;
				}			
			}
		}
		
		// turn off bursts altogether if this is a one shot
		if( m_pDef->nFlags & FPARTICLE_DEF_FLAGS_ONE_SHOT ) {
			m_nFlags &= ~_EMITTER_FLAGS_ENABLE_EMIT;
			m_nFlags |= _EMITTER_FLAGS_KILL_ON_LAST_PARTICLE;

			// if this 1 shot emitter isn't drawn this frame, kill the emitter
			m_nFlags |= _EMITTER_FLAGS_KILL_IF_NOT_DRAW_THIS_FRAME;
		}
	}

	if( m_ParticleList.nCount ) {
		// recalculate the bounding sphere
		if( m_ParticleList.nCount > 1 ) {
			CalcMinMaxParticleBox( Min, Max );
			Max.Sub( Min );
			Max.Mul( 0.5f );
			m_BoundSphere_MS.m_fRadius = Max.Mag();
			FMATH_CLAMPMIN( m_BoundSphere_MS.m_fRadius, _INITIAL_BS_RADIUS );
			Min.Add( Max );
			m_BoundSphere_MS.m_Pos = Min.v3;
		} else {
			// just 1 point, create the sphere from that 1 particle
			pPart = (_Particle_t *)flinklist_GetHead( &m_ParticleList );
			m_BoundSphere_MS.m_fRadius = _INITIAL_BS_RADIUS;
			m_BoundSphere_MS.m_Pos = pPart->PosWS.v3;
		}		
		// update the tracker
		UpdateTracker( (m_nFlags & _EMITTER_FLAGS_EMITTER_MOVES) );		
	} else if( m_nFlags & _EMITTER_FLAGS_KILL_ON_LAST_PARTICLE ) {
		// there are no more particles, we can free this emitter
		return TRUE;
	} else {
		// we are not done yet, but there are no particles		

		// if this is a moving emitter, we need to update the tracker so that when we do want to emit, it could be in the active list
		if( m_nFlags & _EMITTER_FLAGS_EMITTER_MOVES ) {
			m_BoundSphere_MS.m_fRadius = _INITIAL_BS_RADIUS;
			FASSERT( m_pPos );
			m_BoundSphere_MS.m_Pos = *m_pPos;
			
            UpdateTracker( TRUE );
		}		
	}
	
	// always call the light work function after the bounding sphere has been calculated
	LightWork();
	
	// more processing is needed next frame
	return FALSE;
}

void CPEmitter::DoNotEmitLights() {
	
	// clear the emit light flag so that no more light work is done
	m_nFlags &= (~_EMITTER_FLAGS_EMIT_LIGHTS);
}

void CPEmitter::Pause( BOOL bPause ) {

	if( bPause ) {
		m_nFlags |= _EMITTER_FLAGS_PAUSE;
	} else {
		m_nFlags &= ~_EMITTER_FLAGS_PAUSE;
	}
}

void CPEmitter::EnableEmission( BOOL bEnable ) {

	if( bEnable ) {
		if( (m_pDef->nFlags & FPARTICLE_DEF_FLAGS_BURST_FOREVER) ||
			(m_fTotalSecs < m_fBurstSecs) ) {
			m_nFlags |= _EMITTER_FLAGS_ENABLE_EMIT;
		}
	} else {
		m_nFlags &= (~_EMITTER_FLAGS_ENABLE_EMIT);
	}
}

void CPEmitter::Stop() {
	// kill the emitter when the last particle dies
	m_nFlags |= _EMITTER_FLAGS_KILL_ON_LAST_PARTICLE;
	// don't emit anymore bursts
	m_nFlags &= (~_EMITTER_FLAGS_ENABLE_EMIT);
}

BOOL CPEmitter::_PreDrawCallback( CFWorldPSGroup *pPSGroup, f32 fDistToCamSq ) {
	CPEmitter *pEmitter = (CPEmitter *)pPSGroup;
#if 0
	if( pEmitter->m_nSignature != _EMITTER_SIGNATURE ||
		!(pEmitter->m_nFlags & _EMITTER_FLAGS_IN_USE) ||
		!pEmitter->m_pDef ) {
		DEVPRINTF( "uh oh, %d %d %d\n", pEmitter->m_nSignature, pEmitter->m_nFlags, pEmitter->m_pDef );
		return FALSE;
	}
#endif
		
	return pEmitter->SetupSprites( fDistToCamSq );
}

FINLINE BOOL CPEmitter::SetupSprites( f32 fDistToCamSq ) {
	s32 nNumParticles;
	_Burst_t *pBurst;
	_Particle_t *pPart;
	f32 fDrawPercent, fMag2, fDistToCam, fDim, fStepMag2, fAlpaFadeFactor;
	FPSprite_t *pSprite;
	CFVec3 Color;
	CFVec3A PosStep, UnitPosStep, TempVec;
	u32 i, nNumSprites;
    
	if( !m_pDef ) {
		// this emitter has finished already
		return FALSE;
	}
	// calculate the percent of particles to actually render
	fDistToCam = fmath_Sqrt( fDistToCamSq );// - m_BoundSphere_MS.m_fRadius;
	if( fDistToCam < 0.0f ) {
		fDistToCam = 0.0f;
	}
	fDrawPercent = CalculatePercentOfParticlesToDraw( fDistToCam );
	if( fDrawPercent <= 0.01f ) {
		return FALSE;
	}
	if( fDrawPercent <= _SMALLEST_PERCENT_TO_DRAW ) {
		// we are drawing the min percent of particles, start fading what we do draw off
		fAlpaFadeFactor = fDrawPercent * (1.0f/_SMALLEST_PERCENT_TO_DRAW);
		fAlpaFadeFactor *= fAlpaFadeFactor;
		fDrawPercent = _SMALLEST_PERCENT_TO_DRAW;
	} else {
		fAlpaFadeFactor = 1.0f;
	}

	BOOL bAdditive = ( m_nBlend == FPSPRITE_BLEND_ADD );
	BOOL bCloseSpriteGaps = (m_pDef->nFlags & FPARTICLE_DEF_FLAGS_CLOSE_SPRITE_GAPS);

	m_nRenderCount = 0;
	pSprite = m_pBase;

	// walk all of the bursts
	pBurst = (_Burst_t *)flinklist_GetHead( &m_BurstList );
	while( pBurst ) {
		// calculate the the number of particles to draw
		nNumParticles = (s32)( fDrawPercent * pBurst->ParticleList.nCount );
		FMATH_CLAMPMIN( nNumParticles, 1 );// draw at least 1 particle per burst

		// walk all of the particles
		pPart = (_Particle_t *)flinklist_GetHead( &pBurst->ParticleList );
		while( pPart && (nNumParticles >= 1) ) {
			// fill in the sprite
			pSprite->Point_MS = pPart->PosWS.v3;
			
			pSprite->fDim_MS = FMATH_FPOT( pPart->fSizePercent, pPart->fInitSize, pPart->fFinalSize );
			
			pSprite->ColorRGBA.ColorRGB = m_SampledWorldColor;
			pSprite->ColorRGBA.ColorRGB += pPart->fEmissiveIntensity;
			pSprite->ColorRGBA.ColorRGB.Clamp1();
			Color.ReceiveLerpOf( pPart->fColorPercent, pPart->InitRGB.v3, pPart->FinalRGB.v3 );
			pSprite->ColorRGBA.ColorRGB.v3 = pSprite->ColorRGBA.ColorRGB.v3 * Color;
			
			pSprite->ColorRGBA.fAlpha = FMATH_FPOT( pPart->fAlphaPercent, pPart->fInitAlpha, pPart->fFinalAlpha );
			pSprite->ColorRGBA.fAlpha *= *m_pfBiasAlpha * fAlpaFadeFactor;

			if( bAdditive ) {
				pSprite->ColorRGBA.ColorRGB.v3 *= pSprite->ColorRGBA.fAlpha;
			}
					
			// advance to the next sprite
			pSprite++;
			m_nRenderCount++;
			if( m_nRenderCount >= _nNumSpritesAllocated ) {
				// can't add any more sprites, we are out
				_nMaxSpritesUsed = _nNumSpritesAllocated;
				_nCurrentSprites += _nNumSpritesAllocated;

				m_nFlags |= _EMITTER_FLAGS_EMITTER_WAS_DRAWN_AT_LEAST_ONCE;
				return TRUE;
			}

			if( pBurst->nMaxSprites > 1 ) {
				// more than 1 sprite per particle
				fMag2 = pPart->VelPerSec.MagSq();
				if( fMag2 >= pBurst->fMaxVel2 ) {
					nNumSprites = pBurst->nMaxSprites;
				} else {
					// go ahead and make the min 2 sprites, since this effect is supposed to be multi sprite
					nNumSprites = 2 + (u32)( fmath_Sqrt( fMag2 ) * pBurst->fOOMaxVel * (f32)(pBurst->nMaxSprites-2) );
				}
			
				// calculate the positional offset
				if( pPart->fAge >= _SECS_TO_INTRODUCE_TAIL ) {
                    PosStep.Mul( pPart->VelPerSec, -pBurst->fPosStepPerSprite );
				} else {
					PosStep.Mul( pPart->VelPerSec, -pBurst->fPosStepPerSprite * pPart->fAge * _OO_SECS_TO_INTRODUCE_TAIL );
				}
				
				// make sure that we should draw the extra particles
				fStepMag2 = PosStep.MagSq();
				if( fStepMag2 > 0.000001f ) {
					// make sure that the pos step isn't so far that the sprites look like seperate particles	
					if( bCloseSpriteGaps ) {
                        UnitPosStep.Mul( PosStep, fmath_InvSqrt( fStepMag2 ) );
					}

					for( i=1; i < nNumSprites; i++ ) {
						// use the previous sprite which has already been setup 
						*pSprite = pSprite[-1];

						if( !bCloseSpriteGaps ) {
							// don't worry about the gaps between the sprites
							pSprite->Point_MS += PosStep.v3;
							pSprite->fDim_MS *= pBurst->fSizeStepPerSprite;
							pSprite->ColorRGBA.fAlpha *= pBurst->fAlphaStepPerSprite;
						} else {
							// ensure that the gaps between particles don't get too big
							fDim = pSprite->fDim_MS * (0.5f * 0.70710678118654752440084436210485f);
							pSprite->fDim_MS *= pBurst->fSizeStepPerSprite;
							
							if( fStepMag2 <= (fDim * fDim) ) {
								// go ahead and use the already computed PosStep cause it won't cause seperation between the sprites
								pSprite->Point_MS += PosStep.v3;					
							} else {
								TempVec.Mul( UnitPosStep, fDim );
								pSprite->Point_MS += TempVec.v3;
							}
							pSprite->ColorRGBA.fAlpha *= pBurst->fAlphaStepPerSprite;
						}

						// advance to the next sprite
						pSprite++;
						m_nRenderCount++;
						if( m_nRenderCount >= _nNumSpritesAllocated ) {
							// can't add any more sprites, we are out
							_nMaxSpritesUsed = _nNumSpritesAllocated;
							_nCurrentSprites += _nNumSpritesAllocated;

							m_nFlags |= _EMITTER_FLAGS_EMITTER_WAS_DRAWN_AT_LEAST_ONCE;
							return TRUE;
						}
					}
				}
			}

			nNumParticles--;

			// move to the next particle
			pPart = (_Particle_t *)flinklist_GetNext( &pBurst->ParticleList, pPart );
		}
		// move to the next burst
		pBurst = (_Burst_t *)flinklist_GetNext( &m_BurstList, pBurst );
	}

	// update our used sprite stats
	_nMaxSpritesUsed = FMATH_MAX( m_nRenderCount, _nMaxSpritesUsed );
	_nCurrentSprites += m_nRenderCount;

	m_nFlags |= _EMITTER_FLAGS_EMITTER_WAS_DRAWN_AT_LEAST_ONCE;

	return (m_nRenderCount > 0);
}

FINLINE void CPEmitter::SetDurationMultiplier( f32 fMultiplier ) {
	
	FASSERT( fMultiplier >= 0.0f );
	
	m_fBurstMultiplier = fMultiplier;
    m_fBurstSecs *= m_fBurstMultiplier;
	m_fTotalPeriodSecs *= m_fBurstMultiplier;	
}

FINLINE void CPEmitter::SetPositionalOffset( f32 fOffset ) {
	FASSERT( fOffset >= 0.0f );

	m_fPositionOffset = fOffset;
}

FINLINE void CPEmitter::AgeTheParticles( f32 fSecsSinceLastWork ) {
	_Burst_t *pBurst, *pNextBurst;
	_Particle_t *pPart, *pNextPart;
	
	pBurst = (_Burst_t *)flinklist_GetHead( &m_BurstList );
	while( pBurst ) {
		pNextBurst = (_Burst_t *)flinklist_GetNext( &m_BurstList, pBurst );

		pPart = (_Particle_t *)flinklist_GetHead( &pBurst->ParticleList );
		while( pPart ) {
			pNextPart = (_Particle_t *)flinklist_GetNext( &pBurst->ParticleList, pPart );

			pPart->fAge += fSecsSinceLastWork;
			pPart->fUnitAge = pPart->fAge * pPart->fOOLifeSpan;
			if( pPart->fUnitAge > 1.0f ) {
				// time to remove this particle
				RemoveParticle( pPart );				
			}
			// move to the next particle
			pPart = pNextPart;
		}

		if( pBurst->ParticleList.nCount == 0 ) {
			// time to remove this burst
			flinklist_Remove( &m_BurstList, pBurst );
			flinklist_AddTail( &_BurstPool, pBurst );
		}

		// move to the next burst
		pBurst = pNextBurst;
	}
}

FINLINE void CPEmitter::EvalAlphaFunction() {
	_Particle_t *pPart;

	switch( m_pDef->AnimAlphaInfo.nFuncType ) {

	case FPARTICLE_ANIM_FUNC_TYPE_CONSTANT:
		// walk all of the particles
		pPart = (_Particle_t *)flinklist_GetHead( &m_ParticleList );
		while( pPart ) {
			pPart->fAlphaPercent = Eval_Constant_Func( &m_pDef->AnimAlphaInfo, pPart->fUnitAge );
			// move to the next particle
			pPart = (_Particle_t *)flinklist_GetNext( &m_ParticleList, pPart );
		}
		break;

	case FPARTICLE_ANIM_FUNC_TYPE_LOOKUP_TABLE:
		// walk all of the particles
		pPart = (_Particle_t *)flinklist_GetHead( &m_ParticleList );
		while( pPart ) {
			pPart->fAlphaPercent = Eval_LookupTable_Func( &m_pDef->AnimAlphaInfo, pPart->fUnitAge );
			// move to the next particle
			pPart = (_Particle_t *)flinklist_GetNext( &m_ParticleList, pPart );
		}
		break;

	case FPARTICLE_ANIM_FUNC_TYPE_LINEAR_TIME:
		// walk all of the particles
		pPart = (_Particle_t *)flinklist_GetHead( &m_ParticleList );
		while( pPart ) {
			pPart->fAlphaPercent = Eval_LinearTime_Func( &m_pDef->AnimAlphaInfo, pPart->fUnitAge );
			// move to the next particle
			pPart = (_Particle_t *)flinklist_GetNext( &m_ParticleList, pPart );
		}
		break;

	case FPARTICLE_ANIM_FUNC_TYPE_RANDOM:
		// walk all of the particles
		pPart = (_Particle_t *)flinklist_GetHead( &m_ParticleList );
		while( pPart ) {
			pPart->fAlphaPercent = Eval_Random_Func( &m_pDef->AnimAlphaInfo, pPart->fUnitAge );
			// move to the next particle
			pPart = (_Particle_t *)flinklist_GetNext( &m_ParticleList, pPart );
		}
		break;

	case FPARTICLE_ANIM_FUNC_TYPE_TIME_2:
		// walk all of the particles
		pPart = (_Particle_t *)flinklist_GetHead( &m_ParticleList );
		while( pPart ) {
			pPart->fAlphaPercent = Eval_Time2_Func( &m_pDef->AnimAlphaInfo, pPart->fUnitAge );
			// move to the next particle
			pPart = (_Particle_t *)flinklist_GetNext( &m_ParticleList, pPart );
		}
		break;

	case FPARTICLE_ANIM_FUNC_TYPE_TIME_3:
		// walk all of the particles
		pPart = (_Particle_t *)flinklist_GetHead( &m_ParticleList );
		while( pPart ) {
			pPart->fAlphaPercent = Eval_Time3_Func( &m_pDef->AnimAlphaInfo, pPart->fUnitAge );
			// move to the next particle
			pPart = (_Particle_t *)flinklist_GetNext( &m_ParticleList, pPart );
		}
		break;
	}
}

FINLINE void CPEmitter::EvalColorFunction() {
	_Particle_t *pPart;

	switch( m_pDef->AnimColorInfo.nFuncType ) {

	case FPARTICLE_ANIM_FUNC_TYPE_CONSTANT:
		// walk all of the particles
		pPart = (_Particle_t *)flinklist_GetHead( &m_ParticleList );
		while( pPart ) {
			pPart->fColorPercent = Eval_Constant_Func( &m_pDef->AnimColorInfo, pPart->fUnitAge );
			// move to the next particle
			pPart = (_Particle_t *)flinklist_GetNext( &m_ParticleList, pPart );
		}
		break;

	case FPARTICLE_ANIM_FUNC_TYPE_LOOKUP_TABLE:
		// walk all of the particles
		pPart = (_Particle_t *)flinklist_GetHead( &m_ParticleList );
		while( pPart ) {
			pPart->fColorPercent = Eval_LookupTable_Func( &m_pDef->AnimColorInfo, pPart->fUnitAge );
			// move to the next particle
			pPart = (_Particle_t *)flinklist_GetNext( &m_ParticleList, pPart );
		}
		break;

	case FPARTICLE_ANIM_FUNC_TYPE_LINEAR_TIME:
		// walk all of the particles
		pPart = (_Particle_t *)flinklist_GetHead( &m_ParticleList );
		while( pPart ) {
			pPart->fColorPercent = Eval_LinearTime_Func( &m_pDef->AnimColorInfo, pPart->fUnitAge );
			// move to the next particle
			pPart = (_Particle_t *)flinklist_GetNext( &m_ParticleList, pPart );
		}
		break;

	case FPARTICLE_ANIM_FUNC_TYPE_RANDOM:
		// walk all of the particles
		pPart = (_Particle_t *)flinklist_GetHead( &m_ParticleList );
		while( pPart ) {
			pPart->fColorPercent = Eval_Random_Func( &m_pDef->AnimColorInfo, pPart->fUnitAge );
			// move to the next particle
			pPart = (_Particle_t *)flinklist_GetNext( &m_ParticleList, pPart );
		}
		break;

	case FPARTICLE_ANIM_FUNC_TYPE_TIME_2:
		// walk all of the particles
		pPart = (_Particle_t *)flinklist_GetHead( &m_ParticleList );
		while( pPart ) {
			pPart->fColorPercent = Eval_Time2_Func( &m_pDef->AnimColorInfo, pPart->fUnitAge );
			// move to the next particle
			pPart = (_Particle_t *)flinklist_GetNext( &m_ParticleList, pPart );
		}
		break;

	case FPARTICLE_ANIM_FUNC_TYPE_TIME_3:
		// walk all of the particles
		pPart = (_Particle_t *)flinklist_GetHead( &m_ParticleList );
		while( pPart ) {
			pPart->fColorPercent = Eval_Time3_Func( &m_pDef->AnimColorInfo, pPart->fUnitAge );
			// move to the next particle
			pPart = (_Particle_t *)flinklist_GetNext( &m_ParticleList, pPart );
		}
		break;
	}
}

FINLINE void CPEmitter::EvalScaleFunction() {
	_Particle_t *pPart;

	switch( m_pDef->AnimScaleInfo.nFuncType ) {

	case FPARTICLE_ANIM_FUNC_TYPE_CONSTANT:
		// walk all of the particles
		pPart = (_Particle_t *)flinklist_GetHead( &m_ParticleList );
		while( pPart ) {
			pPart->fSizePercent = Eval_Constant_Func( &m_pDef->AnimScaleInfo, pPart->fUnitAge );
			// move to the next particle
			pPart = (_Particle_t *)flinklist_GetNext( &m_ParticleList, pPart );
		}
		break;

	case FPARTICLE_ANIM_FUNC_TYPE_LOOKUP_TABLE:
		// walk all of the particles
		pPart = (_Particle_t *)flinklist_GetHead( &m_ParticleList );
		while( pPart ) {
			pPart->fSizePercent = Eval_LookupTable_Func( &m_pDef->AnimScaleInfo, pPart->fUnitAge );
			// move to the next particle
			pPart = (_Particle_t *)flinklist_GetNext( &m_ParticleList, pPart );
		}
		break;

	case FPARTICLE_ANIM_FUNC_TYPE_LINEAR_TIME:
		// walk all of the particles
		pPart = (_Particle_t *)flinklist_GetHead( &m_ParticleList );
		while( pPart ) {
			pPart->fSizePercent = Eval_LinearTime_Func( &m_pDef->AnimScaleInfo, pPart->fUnitAge );
			// move to the next particle
			pPart = (_Particle_t *)flinklist_GetNext( &m_ParticleList, pPart );
		}
		break;

	case FPARTICLE_ANIM_FUNC_TYPE_RANDOM:
		// walk all of the particles
		pPart = (_Particle_t *)flinklist_GetHead( &m_ParticleList );
		while( pPart ) {
			pPart->fSizePercent = Eval_Random_Func( &m_pDef->AnimScaleInfo, pPart->fUnitAge );
			// move to the next particle
			pPart = (_Particle_t *)flinklist_GetNext( &m_ParticleList, pPart );
		}
		break;

	case FPARTICLE_ANIM_FUNC_TYPE_TIME_2:
		// walk all of the particles
		pPart = (_Particle_t *)flinklist_GetHead( &m_ParticleList );
		while( pPart ) {
			pPart->fSizePercent = Eval_Time2_Func( &m_pDef->AnimScaleInfo, pPart->fUnitAge );
			// move to the next particle
			pPart = (_Particle_t *)flinklist_GetNext( &m_ParticleList, pPart );
		}
		break;

	case FPARTICLE_ANIM_FUNC_TYPE_TIME_3:
		// walk all of the particles
		pPart = (_Particle_t *)flinklist_GetHead( &m_ParticleList );
		while( pPart ) {
			pPart->fSizePercent = Eval_Time3_Func( &m_pDef->AnimScaleInfo, pPart->fUnitAge );
			// move to the next particle
			pPart = (_Particle_t *)flinklist_GetNext( &m_ParticleList, pPart );
		}
		break;
	}
}

FINLINE void CPEmitter::UpdatePosAndVelocity( f32 fSecsSinceLastWork ) {
	_Burst_t *pBurst;
	_Particle_t *pPart;
	f32 fDrag, fGravity, fBubbleDrag, fMag2;
	CFVec3A Delta, InitialVel;

	// walk all of the bursts
	pBurst = (_Burst_t *)flinklist_GetHead( &m_BurstList );
	while( pBurst ) {

		// calculate the drag
		if( pBurst->nFlags & _BURST_FLAGS_APPLY_DRAG_FORCE ) {
			fDrag = pBurst->fDragMultiplierPerSec * fSecsSinceLastWork;
		}

		// calculate the gravity
		fGravity = pBurst->fGravityPerSec * fSecsSinceLastWork;

		// calculate the bubble force
		if( pBurst->nFlags & _BURST_FLAGS_APPLY_BUBBLE_FORCE ) {
			fBubbleDrag = pBurst->fBubbleDragPerSec * fSecsSinceLastWork;			
		}

		if( pBurst->nFlags & _BURST_FLAGS_INITIAL_VEL_ADDED ) {
			InitialVel.Mul( pBurst->EmitterVelPerSec, fSecsSinceLastWork );
		} else {
			InitialVel.Zero();
		}

		// there should be particles in every burst (or else why is the burst still around)
		FASSERT( pBurst->ParticleList.nCount );	

		// walk all of the particles
		pPart = (_Particle_t *)flinklist_GetHead( &pBurst->ParticleList );
		while( pPart ) {
			// first update the position
			Delta.Mul( pPart->VelPerSec, fSecsSinceLastWork );
			pPart->PosWS.Add( Delta );
			pPart->PosWS.Add( InitialVel );

			// jitter the position
			if( pBurst->nFlags & _BURST_FLAGS_JITTER_POS ) {
#if _USE_POOLED_RANDOM_FLOATS
				Delta.Set( _GetPooledRandomBipolarUnitFloat(),
						   _GetPooledRandomBipolarUnitFloat(),
						   _GetPooledRandomBipolarUnitFloat() );
#else
				Delta.Set( fmath_RandomBipolarUnitFloat(),
						   fmath_RandomBipolarUnitFloat(),
						   fmath_RandomBipolarUnitFloat() );
#endif
				Delta.Mul( pBurst->JitterOffset );
				// offset the position
				pPart->PosWS.Add( Delta );
			}

			// apply drag (which is a function of the current velocity)
			if( pBurst->nFlags & _BURST_FLAGS_APPLY_DRAG_FORCE ) {				
				Delta.Mul( pPart->VelPerSec, fDrag );
				if( pBurst->nFlags & _BURST_FLAGS_NEG_DRAG_FORCE ) {
					// make sure that we don't slow the part down so far that it starts in the other direction
					if( Delta.MagSq() < pPart->VelPerSec.MagSq() ) {
						// this is an allowable amount to slow down by
						pPart->VelPerSec.Add( Delta );
					} else {
						// zero out the the velocity
						pPart->VelPerSec.Zero();
					}
				} else {
					// we are accelerating the particle, don't allow the velocity to blow up
					pPart->VelPerSec.Add( Delta );
					fMag2 = pPart->VelPerSec.MagSq();
					if( fMag2 > (500.0f * 500.0f) ) {
						// clamp the velocity to the max allowable
						pPart->VelPerSec.Mul( fmath_InvSqrt( fMag2 ) * 500.0f );
					}
				}
			}
			
			// apply the bubble force
			if( pBurst->nFlags & _BURST_FLAGS_APPLY_BUBBLE_FORCE ) {
				if( fmath_RandomChance( pBurst->fBubbleUpdateChance ) ) {
					// add a new random force to the total bubble forces
					RandomUnitVec3( Delta );		
					Delta.Mul( pBurst->MaxBubbleXYZ );
					pPart->BubbleForce.Add( Delta );
				} else {
					// apply drag to the bubble force 
					Delta.Mul( pPart->BubbleForce, fBubbleDrag );
					pPart->BubbleForce.Add( Delta );
				}
				// make sure that the bubble force isn't outrageous
				fMag2 = pPart->BubbleForce.MagSq();
				if( fMag2 > (5000.0f * 5000.0f) ) {
					// clamp the bubble force to the max allowable
					pPart->BubbleForce.Mul( fmath_InvSqrt( fMag2 ) * 5000.0f );
				}
				// add bubble force to the current velocity
				Delta.Mul( pPart->BubbleForce, fSecsSinceLastWork );				
				pPart->VelPerSec.Add( Delta );
			}

			// lastly update the velocity with our gravity force
			pPart->VelPerSec.y += fGravity;

			// move to the next particle
			pPart = (_Particle_t *)flinklist_GetNext( &pBurst->ParticleList, pPart );
		}

		////////////////////////////////////////////////
		// apply forces to the initial emitter velocity (do this independent of the particle so that tails will work properly)
		if( pBurst->nFlags & _BURST_FLAGS_INITIAL_VEL_ADDED ) {
			
// nat: you can tweak this value if you want to, the more negative number, the faster the emitter velocity influence will go to zero
			Delta.Mul( pBurst->EmitterVelPerSec, -3.0f * fSecsSinceLastWork );
			// make sure that we don't slow the part down so far that it starts in the other direction
			if( Delta.MagSq() < pBurst->EmitterVelPerSec.MagSq() ) {
				// this is an allowable amount to slow down by
				pBurst->EmitterVelPerSec.Add( Delta );
			} else {
				// zero out the the velocity
				pBurst->EmitterVelPerSec.Zero();
			}
		}

        // move to the next burst
		pBurst = (_Burst_t *)flinklist_GetNext( &m_BurstList, pBurst );	
	}
}

FINLINE f32 CPEmitter::CalculatePercentOfParticlesToDraw( f32 fDistToCam ) {

#if FANG_WINGC
	// The consoles thinned an emitter's particles between its skip-draw distance and its cull distance
	// to save fill rate; a PC draws them all until the emitter is culled. (Particles right in front of
	// the camera are still thinned below, which keeps them from covering the view.)
	if( fDistToCam > m_fStartSkipDrawDist ) {
		return 1.0f;
	}
#endif
	if( fDistToCam > m_fStartSkipDrawDist ) {
		// compute what percent of the particles to draw
		fDistToCam -= m_fStartSkipDrawDist;
		fDistToCam *= m_fOOSkipDrawRange;

		FMATH_CLAMPMAX( fDistToCam, 1.0f );

		return ( FMATH_FPOT( fDistToCam, 1.0f, m_fMinPercentToDraw ) );
	} else if( fDistToCam < _TOO_CLOSE_TO_CAM ) {
		fDistToCam *= _OO_TOO_CLOSE_TO_CAM;

		return ( FMATH_FPOT( fDistToCam, _fTooCloseDropPercent, 1.0f ) );
	}

	return 1.0f;
}

FINLINE f32 CPEmitter::Eval_Constant_Func( const FParticleAnimFunc_t *pFuncData, f32 fUnitAge ) {

	if( fUnitAge <= pFuncData->fInUnitPercent ) {
		return 0.0f;
	} else if( fUnitAge >= pFuncData->fOutUnitPercent ) {
		return 1.0f;
	} else {
		return pFuncData->Constant.fConstant;
	}
}

FINLINE f32 CPEmitter::Eval_LookupTable_Func( const FParticleAnimFunc_t *pFuncData, f32 fUnitAge ) {
	
	if( fUnitAge <= pFuncData->fInUnitPercent ) {
		return 0.0f;
	} else if( fUnitAge >= pFuncData->fOutUnitPercent ) {
		return 1.0f;
	} else {
		u32 nIndex = (u32)( fUnitAge * ((f32)FPARTICLE_NUM_TABLE_ENTRIES) );
		fUnitAge -= (nIndex * FPARTICLE_TIMESTEP_PER_ENTRY);
		fUnitAge *= FPARTICLE_TIMESTEP_PER_ENTRY;

		return ( FMATH_FPOT( fUnitAge, pFuncData->Lookup.afLookupTable[nIndex], pFuncData->Lookup.afLookupTable[nIndex+1] ) );
	}
}

FINLINE f32 CPEmitter::Eval_LinearTime_Func( const FParticleAnimFunc_t *pFuncData, f32 fUnitAge ) {
	
	if( fUnitAge <= pFuncData->fInUnitPercent ) {
		return 0.0f;
	} else if( fUnitAge >= pFuncData->fOutUnitPercent ) {
		return 1.0f;
	} else {
		fUnitAge -= pFuncData->fInUnitPercent;
		fUnitAge *= pFuncData->Linear.fOOFuncRange;

		return fUnitAge;
	}
}

FINLINE f32 CPEmitter::Eval_Random_Func( const FParticleAnimFunc_t *pFuncData, f32 fUnitAge ) {

	if( fUnitAge <= pFuncData->fInUnitPercent ) {
		return 0.0f;
	} else if( fUnitAge >= pFuncData->fOutUnitPercent ) {
		return 1.0f;
	} else {
#if _USE_POOLED_RANDOM_FLOATS
		fUnitAge = _GetPooledRandomFloat();
#else
		fUnitAge = fmath_RandomFloat();
#endif
		if( fUnitAge < pFuncData->Random.fRandOutChance ) {
			return 1.0f;
		}
		fUnitAge -= pFuncData->Random.fRandOutChance;
		fUnitAge *= pFuncData->Random.fRandomReNormalizer;

		return fUnitAge;
	}
}

FINLINE f32 CPEmitter::Eval_Time2_Func( const FParticleAnimFunc_t *pFuncData, f32 fUnitAge ) {
	
	if( fUnitAge <= pFuncData->fInUnitPercent ) {
		return 0.0f;
	} else if( fUnitAge >= pFuncData->fOutUnitPercent ) {
		return 1.0f;
	} else {
		return ( fUnitAge * fUnitAge );
	}
}

FINLINE f32 CPEmitter::Eval_Time3_Func( const FParticleAnimFunc_t *pFuncData, f32 fUnitAge ) {
	
	if( fUnitAge <= pFuncData->fInUnitPercent ) {
		return 0.0f;
	} else if( fUnitAge >= pFuncData->fOutUnitPercent ) {
		return 1.0f;
	} else {
		return ( fUnitAge * fUnitAge * fUnitAge );
	}
}

// sets up _pFrame with either Min, Max, or a computed LERP version
void CPEmitter::SetupFramePtr( const FParticleDef_t *pDef, f32 fUnitIntensity ) {
	
	if( fUnitIntensity >= 0.99f ) {
		// close enough to 1 to just use the max
		_pFrame = (FParticleKeyFrame_t *)&pDef->Max;
	} else if( fUnitIntensity <= 0.01f ) {
		// close enough to 0 to just use the min
		_pFrame = (FParticleKeyFrame_t *)&pDef->Min;
	} else {
		_pFrame = &_KeyFrameInfo;

		fparticle_CalculateKeyFrame( _pFrame, pDef, fUnitIntensity );
	}
}

// adds a particle to both the burst and emitter linklists
// NOTE: pPart->pBurst MUST BE SET BEFORE CALLING
FINLINE void CPEmitter::AddParticle( _Particle_t *pPart ) {

	FASSERT( pPart->pBurst );

	flinklist_AddTail( &pPart->pBurst->ParticleList, pPart );
	flinklist_AddTail( &m_ParticleList, pPart );
}

// removes a particle from both the burst and emitter linklist and returns it to the free pool
FINLINE void CPEmitter::RemoveParticle( _Particle_t *pPart ) {
	flinklist_Remove( &pPart->pBurst->ParticleList, pPart );
	flinklist_Remove( &m_ParticleList, pPart );

	flinklist_AddTail( &_ParticlePool, pPart );
}

FINLINE f32 CPEmitter::PickValueFromMinAndDelta( const CFVec2 &rVec2 ) {
#if _USE_POOLED_RANDOM_FLOATS
	return rVec2.x + (_GetPooledRandomFloat() * rVec2.y);	
#else
	return rVec2.x + (fmath_RandomFloat() * rVec2.y);
#endif
}

FINLINE f32 CPEmitter::PickValueFromMinAndDelta( const CFVec2 &rVec2, f32 fRandom ) {
	return rVec2.x + (fRandom * rVec2.y);
}

void CPEmitter::InitBurst( _Burst_t *pBurst, const FParticleKeyFrame_t *pKeyFrame ) {

	pBurst->fDragMultiplierPerSec = pKeyFrame->fDragMultiplierPerSec;
	pBurst->nFlags = _BURST_FLAGS_NONE;
	// if the drag is small enough, just set it to zero to avoid doing any work
	if( pBurst->fDragMultiplierPerSec <= -0.01f ) {
		pBurst->nFlags |= _BURST_FLAGS_APPLY_DRAG_FORCE | _BURST_FLAGS_NEG_DRAG_FORCE;
	} else if( pBurst->fDragMultiplierPerSec < 0.01f ) {
		// so small of a drag force, set it to zero
		pBurst->fDragMultiplierPerSec = 0.0f;
	} else {
		// a positive drag force
		pBurst->nFlags |= _BURST_FLAGS_APPLY_DRAG_FORCE;
	}

	pBurst->fGravityPerSec = pKeyFrame->fGravityPerSec;
	
	// only copy the bubble vars if they are going to be used
	if( m_pDef->nFlags & FPARTICLE_DEF_FLAGS_BUBBLE_MOTION ) {
		pBurst->MaxBubbleXYZ.Set( pKeyFrame->MaxBubbleXYZ );
		pBurst->fBubbleUpdateChance = pKeyFrame->fBubbleUpdateChance;
		pBurst->fBubbleDragPerSec = pKeyFrame->fBubbleDragPerSec;		
		// if the drag is small enough, just set it to zero to avoid doing any work
		if( pBurst->fBubbleDragPerSec < 0.01f && pBurst->fBubbleDragPerSec > -0.01f ) {
			// so small of a drag force, set it to zero
			pBurst->fBubbleDragPerSec = 0.0f;
		} else {
			pBurst->nFlags |= _BURST_FLAGS_APPLY_BUBBLE_FORCE;
		}
	}
	// only copy the jitter vars if they are going to be used
	if( m_pDef->nFlags & FPARTICLE_DEF_FLAGS_JITTER_POS ) {
		pBurst->JitterOffset.Set( pKeyFrame->JitterOffset );
		pBurst->nFlags |= _BURST_FLAGS_JITTER_POS;
	}
	// only copy the multi sprite vars if they are going to be used
	if( m_pDef->nFlags & FPARTICLE_DEF_FLAGS_MULTIPLE_SPRITES ) {
		pBurst->nMaxSprites = (u32)pKeyFrame->fNumSprites;
		pBurst->fMaxVel2 = pKeyFrame->fMaxVel2;
		pBurst->fOOMaxVel = fmath_InvSqrt( pBurst->fMaxVel2 );
		pBurst->fAlphaStepPerSprite = pKeyFrame->fAlphaStepPerSprite;
		pBurst->fSizeStepPerSprite = pKeyFrame->fSizeStepPerSprite;
		pBurst->fPosStepPerSprite = pKeyFrame->fPosStepPerSprite;
	} else {
		pBurst->nMaxSprites = 1;
	}
	// only copy the collision vars if they are going to be used
	if( m_pDef->nFlags & FPARTICLE_DEF_FLAGS_SPLIT_ON_IMPACT ) {
		pBurst->fMinCollAlpha = pKeyFrame->fMinCollAlpha;
		pBurst->fMaxCollSplit = pKeyFrame->fMaxCollSplit;
		pBurst->fUnitCollBounce = pKeyFrame->fUnitCollBounce;
	}
}

FINLINE void CPEmitter::GetLocalXYUnitVec( const CFVec3A &rUnitDir, CFVec3A &rLocalX, CFVec3A &rLocalY ) const {
	f32 fDot;

	// compute the local x & z axis to the unitdir
	rLocalX = CFVec3A::m_UnitAxisX;
	fDot = rLocalX.Dot( rUnitDir );
	if( fDot > 0.75f || fDot < -0.75f ) {
		// pick a different initial vector to cross with
		rLocalX.Cross( CFVec3A::m_UnitAxisY, rUnitDir );	
		rLocalY.Cross( rUnitDir, rLocalX );
	} else {
		rLocalY.Cross( rUnitDir, rLocalX );
		rLocalX.Cross( rLocalY, rUnitDir );	
	}
}

FINLINE void CPEmitter::RandomUnitVec3( CFVec3A &rResult ) {
	
	rResult.Set( CFVec3A::m_UnitAxisZ );
#if _USE_POOLED_RANDOM_FLOATS
	rResult.RotateY( _GetPooledRandomFloat() * (FMATH_2PI * 1.23456789f) );
	rResult.RotateX( _GetPooledRandomFloat() * (FMATH_2PI * 12.3456789f) );
	rResult.RotateZ( _GetPooledRandomFloat() * (-FMATH_2PI * 123.456789f) );	
#else
	rResult.RotateY( fmath_RandomFloat() * (FMATH_2PI * 1.23456789f) );
	rResult.RotateX( fmath_RandomFloat() * (FMATH_2PI * 12.3456789f) );
	rResult.RotateZ( fmath_RandomFloat() * (-FMATH_2PI * 123.456789f) );
#endif
}

FINLINE void CPEmitter::CalcMinMaxParticleBox( CFVec3A &rMin, CFVec3A &rMax ) {
	_Particle_t *pPart;

	// walk all of the particles
	pPart = (_Particle_t *)flinklist_GetHead( &m_ParticleList );
	if( pPart ) {
		rMin = pPart->PosWS;
		rMax = pPart->PosWS;

		pPart = (_Particle_t *)flinklist_GetNext( &m_ParticleList, pPart );
		while( pPart ) {
			rMin.ClampMax( pPart->PosWS );
			rMax.ClampMin( pPart->PosWS );

			// move to the next particle
			pPart = (_Particle_t *)flinklist_GetNext( &m_ParticleList, pPart );
		}
	}
}

FINLINE void CPEmitter::LightWork() {
	
	if( !(m_nFlags & _EMITTER_FLAGS_EMIT_LIGHTS) ) {
		// we aren't emitting light anymore, return this light
		TurnOffLights();
		return;
	} else {
		if( !m_ParticleList.nCount ) {
			// even though lights are enabled, there are no particles
			TurnOffLights();
			return;
		}

		// calculate the emitters unit age, by averaging each particles age
		_Particle_t *pPart;
		f32 fIntensity, fUnitAge = 0.0f, fOOCount = fmath_Inv( (f32)m_ParticleList.nCount );
		CFVec3A AvgPos;

		AvgPos.Zero();
		
        if( m_pDef->nFlags2 & FPARTICLE_DEF_FLAG2_AUTO_CALC_LIGHT_INTENSITY ) {
			// auto calculate the light intensity from the particle opacity
			// while we are at it, calculate our unit age too
			f32 fOpacitySum = 0.0f;
			
			// walk all of the particles
			pPart = (_Particle_t *)flinklist_GetHead( &m_ParticleList );
			while( pPart ) {
				fOpacitySum += FMATH_FPOT( pPart->fAlphaPercent, pPart->fInitAlpha, pPart->fFinalAlpha ); 
				fUnitAge += pPart->fUnitAge;

				AvgPos.Add( pPart->PosWS );

				pPart = (_Particle_t *)flinklist_GetNext( &m_ParticleList, pPart );		
			}
			fUnitAge *= fOOCount;

            fIntensity = fOpacitySum * fOOCount;			
		} else {
			// calculate our unit age and intensity by interpolating our last burst value
			
			// walk all of the particles
			pPart = (_Particle_t *)flinklist_GetHead( &m_ParticleList );
			while( pPart ) {
				fUnitAge += pPart->fUnitAge;

				AvgPos.Add( pPart->PosWS );

				pPart = (_Particle_t *)flinklist_GetNext( &m_ParticleList, pPart );		
			}
			fUnitAge *= fOOCount;

			fIntensity = FMATH_FPOT( fUnitAge, m_StartBurstLight.m_fLightIntensity, m_EndBurstLight.m_fLightIntensity );
		}

		AvgPos.Mul( fOOCount );
		
		// calculate the color
		CFColorRGB RGB;
		RGB.v3.ReceiveLerpOf( fUnitAge, m_StartBurstLight.m_LightRGB.v3, m_EndBurstLight.m_LightRGB.v3 );

		f32 fRadius = FMATH_FPOT( fUnitAge, m_StartBurstLight.m_fLightRadiusMultiplier, m_EndBurstLight.m_fLightRadiusMultiplier );
		fRadius *= GetBoundingSphere().m_fRadius;
		if( m_fMaxLightRadius > 0.0f ) {
            FMATH_CLAMPMAX( fRadius, m_fMaxLightRadius );
		}

		// update our light
		if( m_pVLight ) {
			m_pVLight->UpdateColor( RGB );
			m_pVLight->UpdateIntensity( fIntensity );
			m_pVLight->UpdatePos( AvgPos.v3 );
			m_pVLight->UpdateRadius( fRadius );
		} else {
			// we need to fill in a Light Init and get a light
			FLightInit_t LightInit;

			LightInit.nFlags = FLIGHT_FLAG_ENABLE | FLIGHT_FLAG_HASPOS;
	
			if( m_pDef->nFlags2 & FPARTICLE_DEF_FLAG2_LIGHT_CORONA ) {
				LightInit.nFlags |= FLIGHT_FLAG_CORONA; 
			}
			if( m_pDef->nFlags2 & FPARTICLE_DEF_FLAG2_PER_PIXEL_LIGHT ) {
				LightInit.nFlags |= FLIGHT_FLAG_PER_PIXEL;
			}
			if( m_pDef->nFlags2 & FPARTICLE_DEF_FLAG2_WORLD_SPACE_CORONA_SCALE ) {
				LightInit.nFlags |= FLIGHT_FLAG_CORONA_WORLDSPACE;
			}
			LightInit.szName[0] = 0;
			LightInit.szPerPixelTexName[0] = 0;
			fclib_strcpy( LightInit.szCoronaTexName, m_pDef->szCoronaTextureName );
			LightInit.nLightID = 0xFFFF;
			LightInit.nType = FLIGHT_TYPE_OMNI;
			LightInit.nParentBoneIdx = -1;
			LightInit.fIntensity = fIntensity;
			LightInit.Motif.Set( RGB, 0.0f );
			LightInit.Motif.nMotifIndex = m_pDef->nLightMotifIndex;
			LightInit.spInfluence.m_Pos = AvgPos.v3;
			LightInit.spInfluence.m_fRadius = fRadius;
			LightInit.fCoronaScale = m_pDef->fCoronaScale;

			m_pVLight = CFLightGroupMgr::GetVirtualLight( &LightInit );
		}
	}
}

void CPEmitter::TurnOffLights() {
	if( m_pVLight ) {
		CFLightGroupMgr::ReleaseVirtualLight( m_pVLight );
		m_pVLight = NULL;
	}
}

// will compute where a burst should be emitted from, and will (if needed)
// compute the Unit Direction and rMtx43 (the z axis will always be the unit direction)
FINLINE void CPEmitter::ComputeBurstPosAndDir( CFVec3A &rPos, BOOL &rbDirectional,
											   CFVec3A &rUnitDir, CFMtx43A &rMtx43 ) {

	CFVec3A LocalX, LocalY;

	if( !m_pEmitShape ) {
		FASSERT( m_pPos );
		// emit from the specified position
		rPos.Set( *m_pPos );
		if( m_pDef->nFlags & FPARTICLE_DEF_FLAGS_DIRECTIONAL ) {
			rUnitDir.Set( *m_pUnitDir );

			// offset the position
			if( m_fPositionOffset > 0.0f ) {
				LocalY.Mul( rUnitDir, m_fPositionOffset );
				rPos.Add( LocalY );
			}
			
			// compute the local x y axis from the unit dir
			GetLocalXYUnitVec( rUnitDir, LocalX, LocalY );
			
			// build a mtx43 to represent the burst
			rMtx43.Set( LocalX, LocalY, rUnitDir, rPos );

			rbDirectional = TRUE;
		} else {
			// the def file specifics to emit in all directions
			rbDirectional = FALSE;
		}
	} else {

		CFVec3A TempV3A;

		switch( m_pEmitShape->nShapeType ) {

		case FPARTICLE_SHAPETYPE_BOX:
			{
				const FParticleBox_t *pBox = &m_pEmitShape->Box;

				// pick a random point inside the box
				rPos.Set( *pBox->pCenter );
				TempV3A.Mul( *pBox->pNormX, (fmath_RandomBipolarUnitFloat() * *pBox->pfHalfLenX) );
				rPos.Add( TempV3A );
				TempV3A.Mul( *pBox->pNormY, (fmath_RandomBipolarUnitFloat() * *pBox->pfHalfLenY) );
				rPos.Add( TempV3A );
				TempV3A.Mul( *pBox->pNormZ, (fmath_RandomBipolarUnitFloat() * *pBox->pfHalfLenZ) );
				rPos.Add( TempV3A );

				// pick a direction
				if( m_pDef->nFlags & FPARTICLE_DEF_FLAGS_DIRECTIONAL ) {
					rbDirectional = TRUE;
				
					// the direction will be the y axis of the box
					rUnitDir.Set( *pBox->pNormY );
					
					// offset the position
					if( m_fPositionOffset > 0.0f ) {
						LocalY.Mul( rUnitDir, m_fPositionOffset );
						rPos.Add( LocalY );
					}

					LocalY.ReceiveNegative( *pBox->pNormZ );

					// build a mtx43 to represent the burst
					rMtx43.Set( *pBox->pNormX, LocalY, rUnitDir, rPos );
				} else {
					rbDirectional = FALSE;	
				}
			}
			break;

		case FPARTICLE_SHAPETYPE_SPHERE:
			{
				const FParticleSphere_t *pSphere = &m_pEmitShape->Sphere;

				// pick a random point in the sphere
				RandomUnitVec3( rUnitDir );
#if _USE_POOLED_RANDOM_FLOATS
				rPos.Mul( rUnitDir, _GetPooledRandomFloat() * *pSphere->pfRadius );	
#else
				rPos.Mul( rUnitDir, fmath_RandomFloat() * *pSphere->pfRadius );
#endif
				rPos.Add( *pSphere->pCenter );

				// pick a direction
				if( m_pDef->nFlags & FPARTICLE_DEF_FLAGS_DIRECTIONAL ) {
					rbDirectional = TRUE;

					// the direction will be the radial already computed to pick the pos
					GetLocalXYUnitVec( rUnitDir, LocalX, LocalY );
					
					// build a mtx43 to represent the burst
					rMtx43.Set( LocalX, LocalY, rUnitDir, rPos );
				} else {
					rbDirectional = FALSE;
				}
			}
			break;

		case FPARTICLE_SHAPETYPE_CYLINDER:
			{
				const FParticleCylinder_t *pCylinder = &m_pEmitShape->Cylinder;
				f32 fHeight;

				// set the direction
				rUnitDir.Set( *pCylinder->pNorm );

				// pick a random point in the cylinder
				fHeight = *pCylinder->pfHeight - (2.0f * *pCylinder->pfRadius);
				if( fHeight <= 0.0f ) {
					fHeight = 0.0f;
				} else {
#if _USE_POOLED_RANDOM_FLOATS
					fHeight *= _GetPooledRandomFloat();
#else
                    fHeight *= fmath_RandomFloat();
#endif
				}

				fHeight += m_fPositionOffset;
				TempV3A.Mul( rUnitDir, fHeight );
				TempV3A.Add( *pCylinder->pCenterBase );

				RandomUnitVec3( LocalX );
#if _USE_POOLED_RANDOM_FLOATS
				rPos.Mul( LocalX, ( *pCylinder->pfRadius * _GetPooledRandomFloat() ) );
#else
				rPos.Mul( LocalX, ( *pCylinder->pfRadius * fmath_RandomFloat() ) );
#endif
				rPos.Add( TempV3A );

				// pick a direction
				if( m_pDef->nFlags & FPARTICLE_DEF_FLAGS_DIRECTIONAL ) {
					rbDirectional = TRUE;

					GetLocalXYUnitVec( rUnitDir, LocalX, LocalY );

					// build a mtx43 to represent the burst
					rMtx43.Set( LocalX, LocalY, rUnitDir, rPos );
				} else {
					rbDirectional = FALSE;
				}
			}
			break;

		case FPARTICLE_SHAPETYPE_POINT:
			{
				const FParticlePoint_t *pPoint = &m_pEmitShape->Point;

				// copy the pos
				rPos.Set( *pPoint->pPoint );
				
				// pick a direction
				if( m_pDef->nFlags & FPARTICLE_DEF_FLAGS_DIRECTIONAL ) {
					rbDirectional = TRUE;
				
					rUnitDir.Set( *pPoint->pDir );
					
					// offset the position
					if( m_fPositionOffset > 0.0f ) {
						LocalY.Mul( rUnitDir, m_fPositionOffset );
						rPos.Add( LocalY );
					}

					GetLocalXYUnitVec( rUnitDir, LocalX, LocalY );

					// build a mtx43 to represent the burst
					rMtx43.Set( LocalX, LocalY, rUnitDir, rPos );
				} else {
					rbDirectional = FALSE;	
				}
			}
			break;

		default:
			FASSERT_NOW;
			break;
		}		
	}
} 

//==================
// private functions

static BOOL _ResLoadCreate( FResHandle_t hRes, void *pLoadedBase, u32 nLoadedBytes, cchar *pszResName ) {
	FParticleDef_t *pDef = (FParticleDef_t *)pLoadedBase;

	if( !(_nSystemState & _SYSTEM_STATE_INSTALL_OK) ) {
		DEVPRINTF( "fparticle::_ResLoadCreate(): You must install the particle system before you can load particle def files.\n" ); 
		return FALSE;
	}

	if( pDef->nVersion != FPARTICLE_FILE_VERSION ) {
		DEVPRINTF( "fparticle::_ResLoadCreate(): The file '%s' is not the correct version, you must update the file before trying to use it.\n", pszResName ); 
		return FALSE;
	}

	// load any needed textures
	if( pDef->nFlags & FPARTICLE_DEF_FLAGS_TEXTURED ) {
		pDef->pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, pDef->szTextureName );
		if( !pDef->pTexDef ) {
			DEVPRINTF( "fparticle::_ResLoadCreate(): Could not load the required texture '%s' referenced by '%s', unloading this particle type.\n",
					   pDef->szTextureName, pszResName );
			return FALSE;
		}
	} else {
		pDef->pTexDef = NULL;
	}

	if( pDef->szSoundFxName[0] ) {
		// Sound effect specified...

		pDef->hSound = fsndfx_GetFxHandle( pDef->szSoundFxName );
	} else {
		// No sound effect specified...

		pDef->hSound = FSNDFX_INVALID_FX_HANDLE;
	}

	// init the emitter linklist
	flinklist_InitRoot( &pDef->EmitterList, FANG_OFFSETOF( CPEmitter, m_Link ) );

	// record the index into the linklist
	pDef->nIndexInList = _LoadedDefFiles.nCount;

	// insert into the loaded def list
	flinklist_AddTail( &_LoadedDefFiles, pDef );

	fres_SetBase( hRes, pLoadedBase );

	return TRUE;
}

static void _ResLoadDestroy( void *pBase ) {
	FParticleDef_t *pDef = (FParticleDef_t *)pBase;
	FParticleDef_t *pItem;

	FASSERT( _nSystemState & _SYSTEM_STATE_INSTALL_OK );
	
	pItem = (FParticleDef_t *)flinklist_GetHead( &_LoadedDefFiles );
	while( pItem ) {
		if( pItem == pDef ) {
			flinklist_Remove( &_LoadedDefFiles, pDef );
			_ReleaseAllEmitters( pDef );
			return;
		}
		pItem = (FParticleDef_t *)flinklist_GetNext( &_LoadedDefFiles, pItem );
	}	
}

static void _ReleaseAllEmitters( FParticleDef_t *pDef ) {
	CPEmitter *pEmitter, *pNext;

	pEmitter = (CPEmitter *)flinklist_GetHead( &pDef->EmitterList );
	while( pEmitter ) {
		pNext = (CPEmitter *)flinklist_GetNext( &pDef->EmitterList, pEmitter );

		pEmitter->Kill();
		flinklist_Remove( &pDef->EmitterList, pEmitter );
		flinklist_AddTail( &_EmitterPool, pEmitter );
		
		pEmitter = pNext;
	}
}

////////////////////////////
// HANDLE CONVERSION METHODS
////////////////////////////

FINLINE FParticleDef_t *_GetParticleDefPtr( FParticle_DefHandle_t hParticleDef ) {
	FParticleDef_t *pDef = (FParticleDef_t *)hParticleDef;
	
	FASSERT( pDef );
	FASSERT( pDef->nIndexInList < _LoadedDefFiles.nCount );

	return pDef;
}

// returns NULL i hEmitter is no a valid handle
FINLINE CPEmitter *_GetEmitterPtr( FParticle_EmitterHandle_t hEmitter ) {
	CPEmitter *pEmitter = (CPEmitter *)hEmitter;

	if( !pEmitter ) {
		return NULL;
	}
	if( pEmitter->m_nSignature != _EMITTER_SIGNATURE ||
		!(pEmitter->m_nFlags & _EMITTER_FLAGS_IN_USE) ) {
		return NULL;
	}

	return pEmitter;
}

// allows the application to specify the amount of memory this system will use
// nMaxEmitters will be the max number of simultaneous emitters in the world at any given time
// nMaxParticles is the max number of particles belonging to the emitters.  each of these will independently animate
// nMaxSpritesPerEmitter is the max number of psprites that any emitter will be made up of.
static BOOL _Install( u32 nMaxEmitters, u32 nMaxParticles, u32 nMaxSpritesPerEmitter ) {
	FResFrame_t ResFrame;
	u32 i, nUsedMemory = 0;

	FASSERT( _nSystemState == _SYSTEM_STATE_STARTUP_OK );
	FASSERT( nMaxEmitters >= 1 );
	FASSERT( nMaxParticles >= 64 );// at least this many are going to be required

	// align up the number of particles up to the nearest 16
	nMaxParticles = FMATH_BYTE_ALIGN_UP( nMaxParticles, 16 );

	ResFrame = fres_GetFrame();

	// setup our stat vars
	_nNumEmittersAllocated = nMaxEmitters;
	_nMaxEmittersUsed = 0;
	_nNumBurstsAllocated = ((nMaxEmitters << 1) + 3) & ~0x3;
	_nMaxBurstsUsed = 0;
	_nNumParticlesAllocated = nMaxParticles;
	_nMaxParticlesUsed = 0;
	_nNumSpritesAllocated = nMaxSpritesPerEmitter;
	_nMaxSpritesUsed = 0;
	_nCurrentParticles = 0;
	_nCurrentSprites = 0;

	// allocate the memory we need for our system
	_paAllocatedEmitters = fnew CPEmitter[_nNumEmittersAllocated];
	if( !_paAllocatedEmitters ) {
		goto _ExitInstall;
	}
	nUsedMemory += sizeof( CPEmitter ) * _nNumEmittersAllocated;

	_paAllocatedBursts = fnew _Burst_t[_nNumBurstsAllocated];
	if( !_paAllocatedBursts ) {
		goto _ExitInstall;
	}
	nUsedMemory += sizeof( _Burst_t ) * _nNumBurstsAllocated;

	_paAllocatedParticles = fnew _Particle_t[_nNumParticlesAllocated];
	if( !_paAllocatedParticles ) {
		goto _ExitInstall;
	}
	nUsedMemory += sizeof( _Particle_t ) * _nNumParticlesAllocated;

	_paAllocatedSprites = (FPSprite_t *)fres_AlignedAllocAndZero( _nNumSpritesAllocated * sizeof( FPSprite_t ), FPSPRITE_BYTE_ALIGNMENT );
	if( !_paAllocatedSprites ) {
		goto _ExitInstall;
	}
	nUsedMemory += sizeof( FPSprite_t ) * _nNumSpritesAllocated;

	// setup our pools
	flinklist_InitPool( &_EmitterPool, _paAllocatedEmitters, sizeof( CPEmitter ), _nNumEmittersAllocated );
	flinklist_InitPool( &_BurstPool, _paAllocatedBursts, sizeof( _Burst_t ), _nNumBurstsAllocated );
	flinklist_InitPool( &_ParticlePool, _paAllocatedParticles, sizeof( _Particle_t ), _nNumParticlesAllocated );

	// init each burst's particle linklist
	for( i=0; i < _nNumBurstsAllocated; i++ ) {
		flinklist_InitRoot( &_paAllocatedBursts[i].ParticleList, FANG_OFFSETOF( _Particle_t, BurstLink ) );		
	}
	
	// we are ready to go
	_nSystemState = _SYSTEM_STATE_INSTALL_OK;

	return TRUE;

_ExitInstall:
	if( _paAllocatedEmitters ) {
		fdelete_array( _paAllocatedEmitters );
		_paAllocatedEmitters = NULL;
	}
	if( _paAllocatedBursts ) {
		fdelete_array( _paAllocatedBursts );
		_paAllocatedBursts = NULL;
	}
	if( _paAllocatedParticles ) {
		fdelete_array( _paAllocatedParticles );
		_paAllocatedParticles = NULL;
	}
	_paAllocatedSprites = NULL;
	fres_ReleaseFrame( ResFrame );

	return FALSE;
}











/////////////////////////////////////////////////
// TOOLS METHODS - DON'T CALL FROM ANYWHERE ELSE
/////////////////////////////////////////////////
#if !FANG_PRODUCTION_BUILD

BOOL fparticle_AreAnimFunctionsEqual( const FParticleAnimFunc_t *pFunc1, const FParticleAnimFunc_t *pFunc2 ) {
	u32 i;

	// test the common fields
	if( pFunc1->nFuncType != pFunc2->nFuncType ||
		pFunc1->fInUnitPercent != pFunc2->fInUnitPercent ||
		pFunc1->fOutUnitPercent != pFunc2->fOutUnitPercent ) {
		return FALSE;
	}

	// test the type specific fields
	switch( pFunc1->nFuncType ) {

	case FPARTICLE_ANIM_FUNC_TYPE_CONSTANT:
		if( pFunc1->Constant.fConstant != pFunc2->Constant.fConstant ) {
			return FALSE;
		}
		break;
		
	case FPARTICLE_ANIM_FUNC_TYPE_LOOKUP_TABLE:
		for( i=0; i < FPARTICLE_NUM_TABLE_ENTRIES; i++ ) {
			if( pFunc1->Lookup.afLookupTable[i] != pFunc2->Lookup.afLookupTable[i] ) {
				return FALSE;
			}
		}
		break;

	case FPARTICLE_ANIM_FUNC_TYPE_LINEAR_TIME:
		if( pFunc1->Linear.fOOFuncRange != pFunc2->Linear.fOOFuncRange ) {
			return FALSE;
		}
		break;

	case FPARTICLE_ANIM_FUNC_TYPE_RANDOM:
		if( pFunc1->Random.fRandomReNormalizer != pFunc2->Random.fRandomReNormalizer ||
			pFunc1->Random.fRandOutChance != pFunc2->Random.fRandomReNormalizer ) {
			return FALSE;
		}
		break;

	case FPARTICLE_ANIM_FUNC_TYPE_TIME_2:
		break;

	case FPARTICLE_ANIM_FUNC_TYPE_TIME_3:
		break;
	}
	return TRUE;
}

FParticle_DefHandle_t fparticle_LoadDefFileFromMem( FParticleDef_t *pDef ) {
	
	FASSERT( _nSystemState & _SYSTEM_STATE_INSTALL_OK );
	FASSERT( pDef->nVersion == FPARTICLE_FILE_VERSION );

	// init the emitter linklist
	flinklist_InitRoot( &pDef->EmitterList, FANG_OFFSETOF( CPEmitter, m_Link ) );

	// record the index into the linklist
	pDef->nIndexInList = _LoadedDefFiles.nCount;

	// insert into the loaded def list
	flinklist_AddTail( &_LoadedDefFiles, pDef );

	return (FParticle_DefHandle_t)pDef;
}

FParticle_EmitterHandle_t fparticle_SpawnToolEmitter( FParticle_DefHandle_t hParticleDef, 
													  const CFVec3 *pPos,
													  const CFVec3 *pUnitDir,
													  f32 *pfUnitIntensity,
													  const FParticleShape_t *pCollShape ) {

	FParticleDef_t *pPartDef = _GetParticleDefPtr( hParticleDef );
	CPEmitter *pEmitter = (CPEmitter *)flinklist_RemoveHead( &_EmitterPool );

	FASSERT( pPos );
	FASSERT( pfUnitIntensity );
	FASSERT( _nSystemState & _SYSTEM_STATE_INSTALL_OK );

	if( !pEmitter ) {
		return FPARTICLE_INVALID_HANDLE;
	}

	u32 nFlags = _EMITTER_INIT_FLAGS_NONE;
	nFlags |= (_EMITTER_INIT_FLAGS_KEEP_POS_PTR | _EMITTER_INIT_FLAGS_KEEP_INTENSITY_PTR | _EMITTER_INIT_FLAGS_EMIT_SHAPE_MOVES);
	
	if( !pEmitter->Init( pPartDef, 
						 pfUnitIntensity,
						 pPos,
						 pUnitDir,
						 NULL, 
						 NULL,
						 pCollShape,
						 nFlags ) ) {
		// trouble initing the emitter, return it to the pool and fail
		flinklist_AddTail( &_EmitterPool, pEmitter );
		return FPARTICLE_INVALID_HANDLE;
	}

	// go ahead and set the unit dir pointer
	pEmitter->SetUnitDir( pUnitDir );

	// add this emitter to the def's linklist
	flinklist_AddTail( &pPartDef->EmitterList, pEmitter );

	return (FParticle_EmitterHandle_t)pEmitter;
}

void fparticle_GetCounts( u32 &rnNumParticles, u32 &rnNumSprites ) {

	rnNumParticles = _nCurrentParticles;
	rnNumSprites = _nCurrentSprites;
}

void fparticle_PauseAllEmitter( BOOL bPause ) {
	FParticleDef_t *pDef;
	CPEmitter *pEmitter;
	
	FASSERT( _nSystemState & _SYSTEM_STATE_INSTALL_OK );

	// run through each loaded particle def
	pDef = (FParticleDef_t *)flinklist_GetHead( &_LoadedDefFiles );
	while( pDef ) {
		// walk all of the emitters of this type
		pEmitter = (CPEmitter *)flinklist_GetHead( &pDef->EmitterList );
		while( pEmitter ) {
			pEmitter->Pause( bPause );

			// goto the next emitter in the list
			pEmitter = (CPEmitter *)flinklist_GetNext( &pDef->EmitterList, pEmitter );
		}		

		// get the next loaded particle def
		pDef = (FParticleDef_t *)flinklist_GetNext( &_LoadedDefFiles, pDef );
	}
}

void fparticle_StopAllEmitterLights( FParticleDef_t *pDef ) {
	CPEmitter *pEmitter;
	
	FASSERT( _nSystemState & _SYSTEM_STATE_INSTALL_OK );

	// walk all of the emitters of this type
	pEmitter = (CPEmitter *)flinklist_GetHead( &pDef->EmitterList );
	while( pEmitter ) {
		pEmitter->TurnOffLights();

		if( (pDef->nFlags & FPARTICLE_DEF_FLAGS_EMIT_LIGHT) == 0 ) {
			pEmitter->m_nFlags &= ~_EMITTER_FLAGS_EMIT_LIGHTS;
		} else {
			pEmitter->m_nFlags |= _EMITTER_FLAGS_EMIT_LIGHTS;

			// the following fields must be setup if this emitter has any particles, 
			// because a light will be emitted using these values next frame
			if( pEmitter->m_ParticleList.nCount ) {
				FParticleKeyFrame_t *pMin = &pDef->Min;
				FParticleKeyFrame_t *pMax = &pDef->Max;
				f32 fIntensity = pEmitter->GetIntensity();

				pEmitter->m_StartBurstLight.m_LightRGB.v3.ReceiveLerpOf( fIntensity, pMin->StartLightRGB.v3, pMax->StartLightRGB.v3 );
				pEmitter->m_StartBurstLight.m_fLightIntensity = FMATH_FPOT( fIntensity, pMin->fStartLightIntensity, pMax->fStartLightIntensity );
				pEmitter->m_StartBurstLight.m_fLightRadiusMultiplier = FMATH_FPOT( fIntensity, pMin->fStartLightRadiusMultiplier, pMax->fStartLightRadiusMultiplier );

				pEmitter->m_EndBurstLight.m_LightRGB.v3.ReceiveLerpOf( fIntensity, pMin->EndLightRGB.v3, pMax->EndLightRGB.v3 );
				pEmitter->m_EndBurstLight.m_fLightIntensity = FMATH_FPOT( fIntensity, pMin->fEndLightIntensity, pMax->fEndLightIntensity );
				pEmitter->m_EndBurstLight.m_fLightRadiusMultiplier = FMATH_FPOT( fIntensity, pMin->fEndLightRadiusMultiplier, pMax->fEndLightRadiusMultiplier );

				pEmitter->m_fMaxLightRadius = FMATH_FPOT( fIntensity, pMin->fMaxLightRadius, pMax->fMaxLightRadius );
			}
		}

		// goto the next emitter in the list
		pEmitter = (CPEmitter *)flinklist_GetNext( &pDef->EmitterList, pEmitter );
	}
}

void fparticle_ResetEmitterFlags( FParticleDef_t *pDef, BOOL bResetParticles ) {
	CPEmitter *pEmitter;
	_Burst_t *pBurst;
	_Particle_t *pPart;

	FASSERT( _nSystemState & _SYSTEM_STATE_INSTALL_OK );

	pEmitter = (CPEmitter *)flinklist_GetHead( &pDef->EmitterList );
	while( pEmitter ) {
		pEmitter->SetPSGroupFlagsAndTexture( pDef );

		pEmitter->SetupFramePtr( pEmitter->m_pDef, pEmitter->GetIntensity() );

		// re-init all of the bursts
		pBurst = (_Burst_t *)flinklist_GetHead( &pEmitter->m_BurstList );
		while( pBurst ) {
			// re init the burst vars
			pEmitter->InitBurst( pBurst, _pFrame );

			// move to the next burst
			pBurst = (_Burst_t *)flinklist_GetNext( &pEmitter->m_BurstList, pBurst );
		}

		// snap the LOD values to the new settings, they were probably being adjusted anyway
		pEmitter->m_fCullDist = _pFrame->fCullDist;
		pEmitter->m_fStartSkipDrawDist = _pFrame->fStartSkipDrawDist;
		pEmitter->m_fOOSkipDrawRange = fmath_Inv( (_pFrame->fCullDist - pEmitter->m_fStartSkipDrawDist) );
		pEmitter->m_fMinPercentToDraw = _pFrame->fMinPercentToDraw;
		pEmitter->m_fEmulationDistance = _pFrame->fEmulationDist;
		
		if( bResetParticles ) {
			// set all of the emitter's particles Bubble Forces
			pPart = (_Particle_t *)flinklist_GetHead( &pEmitter->m_ParticleList );
			while( pPart ) {
				pPart->BubbleForce.Zero();

				pPart = (_Particle_t *)flinklist_GetNext( &pEmitter->m_ParticleList, pPart );
			}	
		}

		// goto the next emitter in the list
		pEmitter = (CPEmitter *)flinklist_GetNext( &pDef->EmitterList, pEmitter );
	}
}

void fparticle_DrawEmitterBoundingSpheres( FParticleDef_t *pDef ) {
	CPEmitter *pEmitter;
	
	FASSERT( _nSystemState & _SYSTEM_STATE_INSTALL_OK );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	// walk all of the emitters of this type
	pEmitter = (CPEmitter *)flinklist_GetHead( &pDef->EmitterList );
	while( pEmitter ) {
		fdraw_FacetedWireSphere( &pEmitter->GetBoundingSphere().m_Pos,
								 pEmitter->GetBoundingSphere().m_fRadius,
								 &FColor_MotifWhite );	
		// goto the next emitter in the list
		pEmitter = (CPEmitter *)flinklist_GetNext( &pDef->EmitterList, pEmitter );
	}
}

void fparticle_NoNearCulling( void ) {
	_fTooCloseDropPercent = 1.0f;
}

BOOL fparticle_GetFirstEmitterBSphere( FParticleDef_t *pDef, CFVec3 &rPos, f32 &rRadius ) {
	CPEmitter *pEmitter;
	
	FASSERT( _nSystemState & _SYSTEM_STATE_INSTALL_OK );

	pEmitter = (CPEmitter *)flinklist_GetHead( &pDef->EmitterList );
	if( !pEmitter ) {
		return FALSE;
	}

	rPos = pEmitter->m_BoundSphere_MS.m_Pos;
	rRadius = pEmitter->m_BoundSphere_MS.m_fRadius;

	return TRUE;
}

BOOL fparticle_GetFirstEmitterLightBSphere( FParticleDef_t *pDef, CFVec3 &rPos, f32 &rRadius ) {
	CPEmitter *pEmitter;
	
	FASSERT( _nSystemState & _SYSTEM_STATE_INSTALL_OK );

	pEmitter = (CPEmitter *)flinklist_GetHead( &pDef->EmitterList );
	if( !pEmitter ) {
		return FALSE;
	}

	if( !pEmitter->m_pVLight ) {
		return FALSE;
	}

	rPos = *pEmitter->m_pVLight->GetCurrentPos();
	rRadius = *pEmitter->m_pVLight->GetCurrentRadius();

	return TRUE;
}
#endif// FANG_PRODUCTION_BUILD

void fparticle_CalculateKeyFrame( FParticleKeyFrame_t *pKeyFrame, const FParticleDef_t *pDef, f32 fUnitIntensity ) {
	const FParticleKeyFrame_t *pMin, *pMax;
	pMin = &pDef->Min;
	pMax = &pDef->Max;

	// KEEP THE VARS IN THE SAME ORDER THAT THEY APPEAR IN THE KEYFRAME
	// IT IS JUST EASIER TO KEEP TRACK OF THE MANY VARS THAT WAY
	pKeyFrame->fCullDist = FMATH_FPOT( fUnitIntensity,			pMin->fCullDist,			pMax->fCullDist );
	pKeyFrame->fStartSkipDrawDist = FMATH_FPOT( fUnitIntensity,	pMin->fStartSkipDrawDist,	pMax->fStartSkipDrawDist );
	pKeyFrame->fMinPercentToDraw = FMATH_FPOT( fUnitIntensity,	pMin->fMinPercentToDraw,	pMax->fMinPercentToDraw );
	pKeyFrame->fEmulationDist = FMATH_FPOT( fUnitIntensity,		pMin->fEmulationDist,		pMax->fEmulationDist );

	pKeyFrame->StartLightRGB.v3.ReceiveLerpOf( fUnitIntensity,	pMin->StartLightRGB.v3,		pMax->StartLightRGB.v3 );
	pKeyFrame->fStartLightIntensity = FMATH_FPOT( fUnitIntensity,pMin->fStartLightIntensity,pMax->fStartLightIntensity );
	pKeyFrame->fStartLightRadiusMultiplier = FMATH_FPOT( fUnitIntensity, pMin->fStartLightRadiusMultiplier, pMax->fStartLightRadiusMultiplier );

	pKeyFrame->EndLightRGB.v3.ReceiveLerpOf( fUnitIntensity,	pMin->EndLightRGB.v3,		pMax->EndLightRGB.v3 );
	pKeyFrame->fEndLightIntensity = FMATH_FPOT( fUnitIntensity,	pMin->fEndLightIntensity,	pMax->fEndLightIntensity );
	pKeyFrame->fEndLightRadiusMultiplier = FMATH_FPOT( fUnitIntensity, pMin->fEndLightRadiusMultiplier, pMax->fEndLightRadiusMultiplier );

	pKeyFrame->fMaxLightRadius = FMATH_FPOT( fUnitIntensity,	pMin->fMaxLightRadius,		pMax->fMaxLightRadius );

	pKeyFrame->fSndUnitVolume = FMATH_FPOT( fUnitIntensity,		pMin->fSndUnitVolume,		pMax->fSndUnitVolume );
	pKeyFrame->fSndPitchMultiplier = FMATH_FPOT( fUnitIntensity, pMin->fSndPitchMultiplier, pMax->fSndPitchMultiplier );

	pKeyFrame->NumPerBurst.ReceiveLerpOf( fUnitIntensity,		pMin->NumPerBurst,			pMax->NumPerBurst );
	pKeyFrame->SecsBetweenBursts.ReceiveLerpOf( fUnitIntensity,	pMin->SecsBetweenBursts,	pMax->SecsBetweenBursts );
	pKeyFrame->fSecsToEmit = FMATH_FPOT( fUnitIntensity,		pMin->fSecsToEmit,			pMax->fSecsToEmit );
	pKeyFrame->EmitPause.ReceiveLerpOf( fUnitIntensity,			pMin->EmitPause,			pMax->EmitPause );
	
	pKeyFrame->fDragMultiplierPerSec = FMATH_FPOT( fUnitIntensity,pMin->fDragMultiplierPerSec,pMax->fDragMultiplierPerSec );
	pKeyFrame->fGravityPerSec = FMATH_FPOT( fUnitIntensity,		pMin->fGravityPerSec,		pMax->fGravityPerSec );
	pKeyFrame->MaxBubbleXYZ.ReceiveLerpOf( fUnitIntensity,		pMin->MaxBubbleXYZ,			pMax->MaxBubbleXYZ );
	pKeyFrame->fBubbleUpdateChance = FMATH_FPOT( fUnitIntensity,pMin->fBubbleUpdateChance,	pMax->fBubbleUpdateChance );
	pKeyFrame->fBubbleDragPerSec = FMATH_FPOT( fUnitIntensity,	pMin->fBubbleDragPerSec,	pMax->fBubbleDragPerSec );
	
	pKeyFrame->fMinCollAlpha = FMATH_FPOT( fUnitIntensity,		pMin->fMinCollAlpha,		pMax->fMinCollAlpha );
	pKeyFrame->fMaxCollSplit = FMATH_FPOT( fUnitIntensity,		pMin->fMaxCollSplit,		pMax->fMaxCollSplit );
	pKeyFrame->fUnitCollBounce = FMATH_FPOT( fUnitIntensity,	pMin->fUnitCollBounce,		pMax->fUnitCollBounce );

	pKeyFrame->fNumSprites = FMATH_FPOT( fUnitIntensity,		pMin->fNumSprites,			pMax->fNumSprites );
	pKeyFrame->fMaxVel2 = FMATH_FPOT( fUnitIntensity,			pMin->fMaxVel2,				pMax->fMaxVel2 );
	pKeyFrame->fAlphaStepPerSprite = FMATH_FPOT( fUnitIntensity,pMin->fAlphaStepPerSprite,	pMax->fAlphaStepPerSprite );
	pKeyFrame->fSizeStepPerSprite = FMATH_FPOT( fUnitIntensity,	pMin->fSizeStepPerSprite,	pMax->fSizeStepPerSprite );
	pKeyFrame->fPosStepPerSprite = FMATH_FPOT( fUnitIntensity,	pMin->fPosStepPerSprite,	pMax->fPosStepPerSprite );

	pKeyFrame->InitScale.ReceiveLerpOf( fUnitIntensity,			pMin->InitScale,			pMax->InitScale );
	pKeyFrame->FinalScale.ReceiveLerpOf( fUnitIntensity,		pMin->FinalScale,			pMax->FinalScale );

	pKeyFrame->InitRed.ReceiveLerpOf( fUnitIntensity,			pMin->InitRed,				pMax->InitRed );
	pKeyFrame->InitGreen.ReceiveLerpOf( fUnitIntensity,			pMin->InitGreen,			pMax->InitGreen );
	pKeyFrame->InitBlue.ReceiveLerpOf( fUnitIntensity,			pMin->InitBlue,				pMax->InitBlue );	
	pKeyFrame->FinalRed.ReceiveLerpOf( fUnitIntensity,			pMin->FinalRed,				pMax->FinalRed );
	pKeyFrame->FinalGreen.ReceiveLerpOf( fUnitIntensity,		pMin->FinalGreen,			pMax->FinalGreen );
	pKeyFrame->FinalBlue.ReceiveLerpOf( fUnitIntensity,			pMin->FinalBlue,			pMax->FinalBlue );
	
	pKeyFrame->InitAlpha.ReceiveLerpOf( fUnitIntensity,			pMin->InitAlpha,			pMax->InitAlpha );	
	pKeyFrame->FinalAlpha.ReceiveLerpOf( fUnitIntensity,		pMin->FinalAlpha,			pMax->FinalAlpha );	
	
	pKeyFrame->OOLifeSecs.ReceiveLerpOf( fUnitIntensity,		pMin->OOLifeSecs,			pMax->OOLifeSecs );

	pKeyFrame->EmissiveIntensity.ReceiveLerpOf( fUnitIntensity,	pMin->EmissiveIntensity,	pMax->EmissiveIntensity );
	
	pKeyFrame->InitDirAngle.ReceiveLerpOf( fUnitIntensity,		pMin->InitDirAngle,			pMax->InitDirAngle );
	pKeyFrame->VelocityZ.ReceiveLerpOf( fUnitIntensity,			pMin->VelocityZ,			pMax->VelocityZ );

	pKeyFrame->fRandomInitPosRadius = FMATH_FPOT( fUnitIntensity,pMin->fRandomInitPosRadius,pMax->fRandomInitPosRadius );
	pKeyFrame->JitterOffset.ReceiveLerpOf( fUnitIntensity,		pMin->JitterOffset,			pMax->JitterOffset );

	pKeyFrame->SpikeFreq.ReceiveLerpOf( fUnitIntensity,			pMin->SpikeFreq,			pMax->SpikeFreq );
	pKeyFrame->SpikeRed.ReceiveLerpOf( fUnitIntensity,			pMin->SpikeRed,				pMax->SpikeRed );
	pKeyFrame->SpikeGreen.ReceiveLerpOf( fUnitIntensity,		pMin->SpikeGreen,			pMax->SpikeGreen );
	pKeyFrame->SpikeBlue.ReceiveLerpOf( fUnitIntensity,			pMin->SpikeBlue,			pMax->SpikeBlue );
	pKeyFrame->SpikeAlpha.ReceiveLerpOf( fUnitIntensity,		pMin->SpikeAlpha,			pMax->SpikeAlpha );
	pKeyFrame->SpikeScale.ReceiveLerpOf( fUnitIntensity,		pMin->SpikeScale,			pMax->SpikeScale );
	pKeyFrame->SpikeDuration.ReceiveLerpOf( fUnitIntensity,		pMin->SpikeDuration,		pMax->SpikeDuration );
	pKeyFrame->SpikeHold.ReceiveLerpOf( fUnitIntensity,			pMin->SpikeHold,			pMax->SpikeHold );	
}
