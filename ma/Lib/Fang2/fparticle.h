//////////////////////////////////////////////////////////////////////////////////////
// fparticle.h - 
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
// 08/15/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FPARTICLE_H_
#define _FPARTICLE_H_ 1

#include "fang.h"
#include "fdata.h"
#include "flight.h"
#include "fmath.h"
#include "fsndfx.h"

#define FPARTICLE_MIN_INDEX					0
#define FPARTICLE_DELTA_INDEX				1

typedef void * FParticle_DefHandle_t;
typedef void * FParticle_EmitterHandle_t;
#define FPARTICLE_INVALID_HANDLE			( NULL )

#define FPARTICLE_FILE_VERSION				0x00000007		// update this anytime the fpr file format changes


struct FTexDef_t;


////////////////////
// PARTICLE KEYFRAME
////////////////////
typedef struct {
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// MACRO PARTICLE VARIABLES - THOSE AFFECTING ALL OF THE PARTICLES, THE MAX VALUE PER BURST WILL BE USED FOR ENTIRE EMITTER
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// how far away should particles be seen from
	f32 fCullDist;				// how far should the particles be seen from
	f32 fStartSkipDrawDist;		// how far should the particles be before the we start skipping the drawing of some portion of them
	f32 fMinPercentToDraw;		// even at the furthest visible distance, what percent of the particles need to be drawn
	f32 fEmulationDist;			// Anytime camera inside this distance, the software rendering pipe will be used for this group

		// what type of light should be emitted, if at all
	CFColorRGB StartLightRGB;			// the starting light color
	f32 fStartLightIntensity;			// the starting light intensity
	f32 fStartLightRadiusMultiplier;	// what should the light radius be when compared to the particle group bounding sphere
	
    CFColorRGB EndLightRGB;				// the ending light color
	f32 fEndLightIntensity;				// the ending light intensity
	f32 fEndLightRadiusMultiplier;		// what should the light radius be when compared to the particle group bounding sphere			

	f32 fMaxLightRadius;				// what is the largest light radius, in feet, allowed

		// sound vars
	f32 fSndUnitVolume;			// how loud should we play any attached sound
	f32 fSndPitchMultiplier;	// how should we pitch the sound, 1.0f is none

	/////////////////////////////////////////////////////////////////////////////////////////
	// BURST PARTICLE VARIABLES - THOSE AFFECTING ALL OF THE PARTICLE OF AN INDIVIDUAL BURST
	/////////////////////////////////////////////////////////////////////////////////////////

		// how many particles should be emitted, and how often
	CFVec2 NumPerBurst;
	CFVec2 SecsBetweenBursts;	// how often should this particle type emit an burst, if over time (this is a min/max not a min/delta value)
	f32 fSecsToEmit;			// how long should this particle type emit for
	CFVec2 EmitPause;			// if in emit forever mode, after emitting for fSecsToEmit,
								// will wait for this long before emitting for another fSecsToEmit (this is a min/max not a min/delta value)
	
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

	///////////////////////////////////////////////////////////
	// MICRO PARTICLE VARIABLES - THOSE AFFECTING EACH PARTICLE
	///////////////////////////////////////////////////////////

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

		// spike variables
	CFVec2 SpikeFreq;		// how often in secs (min - max) to spike
	CFVec2 SpikeRed;		// color to spike to (min - max)
	CFVec2 SpikeGreen;		// color to spike to (min - max)
	CFVec2 SpikeBlue;		// color to spike to (min - max)
	CFVec2 SpikeAlpha;		// alpha to spike to (min - max)
	CFVec2 SpikeScale;		// scale to spike to (min - max % of regular scale)
	CFVec2 SpikeDuration;	// secs in and out of the spike effect
	CFVec2 SpikeHold;		// secs to hold the effect at 100% (min - max)

	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa
	void ChangeEndian( void ) {
		fCullDist = fang_ConvertEndian( fCullDist );
		fStartSkipDrawDist = fang_ConvertEndian( fStartSkipDrawDist );
		fMinPercentToDraw = fang_ConvertEndian( fMinPercentToDraw );
		fEmulationDist = fang_ConvertEndian( fEmulationDist );
		StartLightRGB.ChangeEndian();
		fStartLightIntensity = fang_ConvertEndian( fStartLightIntensity );
		fStartLightRadiusMultiplier = fang_ConvertEndian( fStartLightRadiusMultiplier );
		EndLightRGB.ChangeEndian();
		fEndLightIntensity = fang_ConvertEndian( fEndLightIntensity );
		fEndLightRadiusMultiplier = fang_ConvertEndian( fEndLightRadiusMultiplier );
		fMaxLightRadius = fang_ConvertEndian( fMaxLightRadius );
		fSndUnitVolume = fang_ConvertEndian( fSndUnitVolume );
		fSndPitchMultiplier = fang_ConvertEndian( fSndPitchMultiplier );
		NumPerBurst.ChangeEndian();
		SecsBetweenBursts.ChangeEndian();
		fSecsToEmit = fang_ConvertEndian( fSecsToEmit );
		EmitPause.ChangeEndian();
		fDragMultiplierPerSec = fang_ConvertEndian( fDragMultiplierPerSec );
		fGravityPerSec = fang_ConvertEndian( fGravityPerSec );
		MaxBubbleXYZ.ChangeEndian();
		fBubbleUpdateChance = fang_ConvertEndian( fBubbleUpdateChance );
		fBubbleDragPerSec = fang_ConvertEndian( fBubbleDragPerSec );
		fMinCollAlpha = fang_ConvertEndian( fMinCollAlpha );
		fMaxCollSplit = fang_ConvertEndian( fMaxCollSplit );
		fUnitCollBounce = fang_ConvertEndian( fUnitCollBounce );
		fNumSprites = fang_ConvertEndian( fNumSprites );
		fMaxVel2 = fang_ConvertEndian( fMaxVel2 );
		fAlphaStepPerSprite = fang_ConvertEndian( fAlphaStepPerSprite );
		fSizeStepPerSprite = fang_ConvertEndian( fSizeStepPerSprite );
		fPosStepPerSprite = fang_ConvertEndian( fPosStepPerSprite );
		InitScale.ChangeEndian();
		FinalScale.ChangeEndian();
		InitRed.ChangeEndian();
		InitGreen.ChangeEndian();
		InitBlue.ChangeEndian();	
		FinalRed.ChangeEndian();
		FinalGreen.ChangeEndian();
		FinalBlue.ChangeEndian();
		InitAlpha.ChangeEndian();	
		FinalAlpha.ChangeEndian();	
		OOLifeSecs.ChangeEndian();
		EmissiveIntensity.ChangeEndian();	
		InitDirAngle.ChangeEndian();
		VelocityZ.ChangeEndian();
		fRandomInitPosRadius = fang_ConvertEndian( fRandomInitPosRadius );
		JitterOffset.ChangeEndian();
		SpikeFreq.ChangeEndian();
		SpikeRed.ChangeEndian();
		SpikeGreen.ChangeEndian();
		SpikeBlue.ChangeEndian();
		SpikeAlpha.ChangeEndian();
		SpikeScale.ChangeEndian();
		SpikeDuration.ChangeEndian();
		SpikeHold.ChangeEndian();
	}	
} FParticleKeyFrame_t;

///////////////////////////////
// PARTICLE ANIMATION FUNCTIONS
///////////////////////////////
typedef enum {
	FPARTICLE_ANIM_FUNC_TYPE_CONSTANT = 0,		// %out = fConstant
	FPARTICLE_ANIM_FUNC_TYPE_LOOKUP_TABLE,		// %out = LERP of nearest values in table indexed by %age * FPARTICLE_NUM_TABLE_ENTRIES
	FPARTICLE_ANIM_FUNC_TYPE_LINEAR_TIME,		// %out = %age
	FPARTICLE_ANIM_FUNC_TYPE_RANDOM,			// %out = random (fRandOutChance of 1.0f)
	FPARTICLE_ANIM_FUNC_TYPE_TIME_2,			// %out = %age^2
	FPARTICLE_ANIM_FUNC_TYPE_TIME_3,			// %out = %age^3
	
	FPARTICLE_ANIM_FUNC_TYPE_COUNT
} FParticleAnimFuncType_e;

#define FPARTICLE_NUM_TABLE_ENTRIES		25
#define FPARTICLE_TIMESTEP_PER_ENTRY	( 1.0f / (f32)FPARTICLE_NUM_TABLE_ENTRIES )

typedef struct {
	f32 fConstant;					// UNIT FLOAT
} FParticleFunc_Constant_t;

typedef struct {
	f32 afLookupTable[FPARTICLE_NUM_TABLE_ENTRIES];	// UNIT FLOAT
} FParticleFunc_Lookup_t;

typedef struct {
	f32 fOOFuncRange;		// set to 1 / (fOutUnitPercent - fInUnitPercent)
} FParticleFunc_Linear_t;

typedef struct {
	f32 fRandOutChance;				// UNIT FLOAT
	f32 fRandomReNormalizer;		// set to 1.0f / (1.0f - fRandOutChance)
} FParticleFunc_Random_t;

struct FParticleAnimFunc_t {
	f32 fInUnitPercent;		// UNIT FLOAT
	f32 fOutUnitPercent;	// UNIT FLOAT	
	FParticleAnimFuncType_e nFuncType;
	//f32 fEvalFreq;			// in times per second
	union {
		FParticleFunc_Constant_t	Constant;		// only used if nFuncType == FPARTICLE_ANIM_FUNC_TYPE_CONSTANT
		FParticleFunc_Lookup_t		Lookup;			// only used if nFuncType == FPARTICLE_ANIM_FUNC_TYPE_LOOKUP_TABLE
		FParticleFunc_Linear_t		Linear;			// only used if nFuncType == FPARTICLE_ANIM_FUNC_TYPE_LINEAR_TIME
		FParticleFunc_Random_t		Random;			// only used if nFuncType == FPARTICLE_ANIM_FUNC_TYPE_RANDOM
	};	


	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa
	void ChangeEndian( void ) {
		u32 i;
		f32 *pFirstVar = &fInUnitPercent;

		for( i=0; i < sizeof( FParticleAnimFunc_t ) >> 2; i++ ) {
			pFirstVar[i] = fang_ConvertEndian( pFirstVar[i] );
		}
	}
};


//////////////////
// PARTICLE FLAGS
//////////////////
enum {
	// the 1st batch of flags FPARTICLE_DEF_FLAGS_...
	FPARTICLE_DEF_FLAGS_EMIT_LIGHT					= 0x00000001,	// The particle fx should emit a light.
	FPARTICLE_DEF_FLAGS_ONE_SHOT					= 0x00000002,	// The particles are emitted in one shot, not over time.
	FPARTICLE_DEF_FLAGS_SPLIT_ON_IMPACT				= 0x00000004,	// If the particles impact, should they split into multiple smaller pieces
	FPARTICLE_DEF_FLAGS_GROUP_ZSCALE				= 0x00000008,	// Should all particles in a group be scaled uniform, instead of perspective
	FPARTICLE_DEF_FLAGS_TEXTURED					= 0x00000010,	// Are the particles textured.
	FPARTICLE_DEF_FLAGS_IGNORE_WIND					= 0x00000020,	// Should the particles in this group ignore any wind.
	FPARTICLE_DEF_FLAGS_DIRECTIONAL					= 0x00000040,	// Are the particles emitted in a general direction.
	FPARTICLE_DEF_FLAGS_DRAW_MODE_MODULATE			= 0x00000080,	// Particles should be drawn in modulate mode	
	FPARTICLE_DEF_FLAGS_DRAW_MODE_ADD				= 0x00000100,	// Particles should be drawn in additive mode
	FPARTICLE_DEF_FLAGS_NO_FOG						= 0x00000200,	// Don't fog the particles
	FPARTICLE_DEF_FLAGS_Z_BUFFER					= 0x00000400,	// Z buffer the particles
	FPARTICLE_DEF_FLAGS_SORT						= 0x00000800,	// Sort the particle emitter when drawn
	FPARTICLE_DEF_FLAGS_SIZE_IN_PIXELS				= 0x00001000,	// Is the size of the particles stated in pixels (if not then in world space units)
	FPARTICLE_DEF_FLAGS_SAMPLE_LIGHTS				= 0x00002000,	// Should the surrounding lights be sampled
	FPARTICLE_DEF_FLAGS_MULTIPLE_SPRITES			= 0x00004000,	// Is each particle made up of multiple sprites?
	FPARTICLE_DEF_FLAGS_BURST_FOREVER				= 0x00008000,	// Don't emit for a set amount of time
	FPARTICLE_DEF_FLAGS_RANDOM_INIT_COLOR			= 0x00010000,	// Pick random initial colors for the particles
	FPARTICLE_DEF_FLAGS_RANDOM_FINAL_COLOR			= 0x00020000,	// Pick random final colors for the particles
	FPARTICLE_DEF_FLAGS_BUBBLE_MOTION				= 0x00040000,	// Apply a bubble motion forces
	FPARTICLE_DEF_FLAGS_JITTER_POS					= 0x00080000,	// Apply a jitter to the particle position
	FPARTICLE_DEF_FLAGS_CLOSE_SPRITE_GAPS			= 0x00100000,	// For multiple sprite systems, ensure that there are no gaps between the sprites making up a particle
	FPARTICLE_DEF_FLAGS_SPIKE_COLOR					= 0x00200000,
	FPARTICLE_DEF_FLAGS_SPIKE_ALPHA					= 0x00400000,
	FPARTICLE_DEF_FLAGS_SPIKE_SCALE					= 0x00800000,
	FPARTICLE_DEF_FLAGS_PLAY_SOUND					= 0x01000000,

	// used to determine if spike effect is on
	FPARTICLE_DEF_FLAGS_SPIKE_EFFECT_ON				= ( FPARTICLE_DEF_FLAGS_SPIKE_COLOR	| 
														FPARTICLE_DEF_FLAGS_SPIKE_ALPHA |
														FPARTICLE_DEF_FLAGS_SPIKE_SCALE ),

	FPARTICLE_DEF_FLAGS_NONE						= 0x00000000,

	// the 2nd batch of flags FPARTICLE_DEF_FLAG2_...
	FPARTICLE_DEF_FLAG2_LIGHT_CORONA				= 0x00000001,	// Apply a light corona to the light (ignored if FPARTICLE_DEF_FLAGS_EMIT_LIGHT isn't set)
	FPARTICLE_DEF_FLAG2_AUTO_CALC_LIGHT_INTENSITY	= 0x00000002,	// Automatically calculate the light intensity from the particles opacity (ignored if FPARTICLE_DEF_FLAGS_EMIT_LIGHT isn't set)
	FPARTICLE_DEF_FLAG2_PER_PIXEL_LIGHT				= 0x00000004,	// The light that is emitted should be a per pixel light (ignored if FPARTICLE_DEF_FLAGS_EMIT_LIGHT isn't set)
	FPARTICLE_DEF_FLAG2_WORLD_SPACE_CORONA_SCALE	= 0x00000008,

	// used to determine if any lighting flag changed
	FPARTICLE_DEF_FLAG2_LIGHTING_SETTINGS			= ( FPARTICLE_DEF_FLAG2_LIGHT_CORONA |
														FPARTICLE_DEF_FLAG2_AUTO_CALC_LIGHT_INTENSITY |
														FPARTICLE_DEF_FLAG2_PER_PIXEL_LIGHT |
														FPARTICLE_DEF_FLAG2_WORLD_SPACE_CORONA_SCALE ),

	FPARTICLE_DEF_FLAG2_NONE						= 0x00000000	
};

///////////////////////////
// PARTICLE DEFINITION FILE
///////////////////////////
typedef struct {
	u32 nVersion;				// what version is this data, see FPARTICLE_FILE_VERSION
	u32 nFlags;					// see FPARTICLE_DEF_FLAGS_... for description
		
	FParticleKeyFrame_t Min;	// use these values at intensity 0.0f
	FParticleKeyFrame_t Max;	// use these values at intensity 1.0f

	// the following values do not animate over the different intensities
	char szTextureName[FDATA_TEXNAME_LEN+1];
	FTexDef_t *pTexDef;		// the loaded texture
	FParticleAnimFunc_t AnimAlphaInfo;
	FParticleAnimFunc_t AnimScaleInfo;
	FParticleAnimFunc_t AnimColorInfo;

	// lighting vars
	u32 nLightMotifIndex;
	char szCoronaTextureName[FDATA_TEXNAME_LEN+1];
	f32 fCoronaScale;
	
	u32 nFlags2;				// see FPARTICLE_DEF_FLAGS2_... for description

	union {
		char szSoundFxName[FDATA_TEXNAME_LEN+1];
		FSndFx_FxHandle_t hSound;
	};
	
	// light sampling vars
	f32 fSecsBetweenLightSamples;	// how often should the world lights be sampled
	CFVec3 SampledLightScale;		// the sampled light is scaled by these values
	
	// housekeeping and list management vars
	u32 nIndexInList;			// used to verify that handles are valid
	FLink_t Link;				// for insertion into a ParticleDef linklist
	FLinkRoot_t EmitterList;	// to keep a list of the emitters of this type currently in use


	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa
	void ChangeEndian( void ) {
		nVersion = fang_ConvertEndian( nVersion );
		nFlags = fang_ConvertEndian( nFlags );
		Min.ChangeEndian();
		Max.ChangeEndian();
		//szTextureName[FTEX_TEXNAME_LEN+1];
		pTexDef = (FTexDef_t *)fang_ConvertEndian(pTexDef);
		AnimAlphaInfo.ChangeEndian();
		AnimScaleInfo.ChangeEndian();
		AnimColorInfo.ChangeEndian();
		nLightMotifIndex = fang_ConvertEndian( nLightMotifIndex );
		//szCoronaTextureName[FDATA_TEXNAME_LEN+1];
		fCoronaScale = fang_ConvertEndian( fCoronaScale );
		nFlags2 = fang_ConvertEndian( nFlags2 );
		//szSoundFxName[FDATA_TEXNAME_LEN+1];
		fSecsBetweenLightSamples = fang_ConvertEndian( fSecsBetweenLightSamples );
		SampledLightScale.ChangeEndian();
		nIndexInList = fang_ConvertEndian( nIndexInList );
		//Link;
		//EmitterList;
	}
} FParticleDef_t;

////////////////////
// PARTICLE SHAPES
////////////////////
typedef enum {
	FPARTICLE_SHAPETYPE_BOX,
	FPARTICLE_SHAPETYPE_SPHERE,
	FPARTICLE_SHAPETYPE_CYLINDER,
	FPARTICLE_SHAPETYPE_POINT,
	
	FPARTICLE_SHAPETYPE_COUNT
} FParticleShapeType_e;

// THE BOX EMITS OUT OF THE BOX IN THE NORM Y DIRECTION
typedef struct {
	CFVec3A *pCenter;	// ptr to the center pt of the box (DO NOT MOVE)

	CFVec3A *pNormX;	// ptr to normal vector along the X dimension of the box
	f32 *pfHalfLenX;	// ptr to the X dimension of the box along pNormX
	CFVec3A *pNormY;	// ptr to normal vector along the Y dimension of the box
	f32 *pfHalfLenY;	// ptr to the Y dimension of the box along pNormX
	CFVec3A *pNormZ;	// ptr to normal vector along the Z dimension of the box
	f32 *pfHalfLenZ;	// ptr to the Z dimension of the box along pNormX
} FParticleBox_t;

// THE SPHERE EMITS RADIALY OUT OF THE SPHERE
typedef struct {
	CFVec3A *pCenter;	// ptr to the center of the sphere (DO NOT MOVE)

	f32 *pfRadius;		// ptr to the radius of the sphere
} FParticleSphere_t;

// THE CYLINDER EMITS OUT OF ALONG THE NORMAL
typedef struct {
	CFVec3A *pCenterBase;// ptr to the center of the base circle (DO NOT MOVE)		

	f32 *pfRadius;		// ptr to the radius of the cylinder
	CFVec3A *pNorm;		// ptr to the normal of the base cylinder circle
	f32 *pfHeight;		// ptr to the height of the cylinder along the the pNorm
} FParticleCylinder_t;

// THE POINT EMITS ALONG THE DIRECTION
typedef struct {
	CFVec3A *pPoint;// Emission Pos (DO NOT MOVE)		
	CFVec3A *pDir;	// ptr to the direction to emit
} FParticlePoint_t;

typedef struct {
	FParticleShapeType_e nShapeType;

	union {
		FParticleBox_t		Box;
		FParticleSphere_t	Sphere;
		FParticleCylinder_t Cylinder;
		FParticlePoint_t	Point;
	};
} FParticleShape_t;

//////////////////////////
// PARTICLE CONFIG STRUCT
//////////////////////////
typedef struct {
	f32 fUnitIntensity;					// what intensity are the emitted particles						(DEFAULT: 1.0f)
	f32 *pfUnitIntensity;				// if set, the value contained will be used as the intensity	(DEFAULT: NULL)

	const FParticleShape_t *pEmitShape;	// shape to emit from (must point to perm memory)				(DEFAULT: NULL)
	BOOL bEmitShapeWillMove;			// will the pEmitShape change position							(DEFAULT: FALSE)
	CFVec3 *pEmitterVelocityPerSec;		// if set (must point to perm memory), each particle will get this added to their initial velocity (DEFAULT: NULL)

	const FParticleShape_t *pCollShape;	// shape to collide with (must point to perm memory)			(DEFAULT: NULL)
	BOOL bKillCollidingParticles;		// should we kill any colliding particles						(DEFAULT: TRUE)
} FParticleEmitCfg_t;


// a callback to be called when a particle is about to be killed and returned to the pool
typedef void FParticleKillCallback_t( FParticle_EmitterHandle_t hHandle, void *pUserData );

//////////////////////////////////////////////////////
// METHODS TO SETUP AND CONFIGURE THE PARTICLE SYSTEM
	// called once on fang startup
extern BOOL fparticle_ModuleStartup( void );

	// called once on fang shutdown
extern void fparticle_ModuleShutdown( void );

	// calls fparticle_KillEmitter for each current emitter
extern void fparticle_KillAllEmitters( void );

	// calls fparticle_StopEmitter for each current emitter
extern void fparticle_StopAllEmitters( void );

	// must be called exactly once per frame, after all emitter spawn calls but before the emitters are drawn
extern void fparticle_Work( void );
	
	// called to get the flags from a particle def file
extern u32 fparticle_GetFlags( FParticle_DefHandle_t hParticleDef );


//////////////////////////////////////////
// METHODS TO PUT PARTICLES INTO THE WORLD
	// spawn a non-moving, point emitter with no collision shape
extern FParticle_EmitterHandle_t fparticle_SpawnEmitter( FParticle_DefHandle_t hParticleDef, 
														 const CFVec3 &rPos,
														 const CFVec3 *pUnitDir=NULL,	// any direction vec will be copied, if NULL a world up vec will be used
														 f32 fUnitIntensity=1.0f );
	// spawn a moving, point emitter with no collision shape
extern FParticle_EmitterHandle_t fparticle_SpawnEmitter( FParticle_DefHandle_t hParticleDef, 
														 const CFVec3 *pPos,			// must point to perm memory	
														 const CFVec3 *pUnitDir=NULL,	// any direction vec will be copied, if NULL a world up vec will be used
														 const CFVec3 *pEmitterVelocityPerSec=NULL,// must point to perm memory
														 f32 fUnitIntensity=1.0f );
	// fills in an emitter config struct used to setup more complex emitter configurations
extern void fparticle_SetCfgToDefaults( FParticleEmitCfg_t *pConfig );
	// spawn an emitter according to the config struct (pEmitShape MUST be set)
extern FParticle_EmitterHandle_t fparticle_SpawnEmitter( FParticle_DefHandle_t hParticleDef, 
														 FParticleEmitCfg_t *pConfig );


/////////////////////////////////////////////////////
// METHODS TO QUERY OR UPDATE BEHAVIOR OF AN EMITTER

// sets a call function that will be called as the particle is being killed
extern BOOL fparticle_SetKillCallback( FParticle_EmitterHandle_t hEmitter,
									  FParticleKillCallback_t *pCallback,
									  void *pUserData );

class CFWorldTracker;
extern const CFWorldTracker *fparticle_GetTrackerPtr( FParticle_EmitterHandle_t hEmitter );

	// gets a pointer to the emitters bounding sphere
extern const CFSphere *fparticle_GetBoundingSpherePtr( FParticle_EmitterHandle_t hEmitter );

	// freezes time, the particles will stop moving but will still be drawn
extern void fparticle_PauseEmitter( FParticle_EmitterHandle_t hEmitter, BOOL bPause );

	// will start or stop emitting particles but any particles that currently exist will finish animating correctly
extern void fparticle_EnableEmission( FParticle_EmitterHandle_t hEmitter, BOOL bEnable );

	// kills all of this emitter's particles and returns it to the unused pool (the handle is no longer valid)
extern void fparticle_KillEmitter( FParticle_EmitterHandle_t hEmitter );

	// will disable emission, allow all current particles to finish animating and then will kill the emitter
extern void fparticle_StopEmitter( FParticle_EmitterHandle_t hEmitter );

	// changes the emitter's intensity to the passed in constant
extern void fparticle_SetIntensity( FParticle_EmitterHandle_t hEmitter, const f32 &rfIntensity );

	// changes the emitter's intensity to *pfIntensity and will update it every work call
extern void fparticle_SetIntensity( FParticle_EmitterHandle_t hEmitter, const f32 *pfIntensity );

	// enables/disables the burst sound effect associated with the particle
extern void fparticle_EnableBurstSound( FParticle_EmitterHandle_t hEmitter, const BOOL &rbEnableBurstSound );

	// changes the emitter's alpha bias (this simply scales down the all of the particles final alpha by this amount)
extern void fparticle_SetAlphaBias( FParticle_EmitterHandle_t hEmitter, const f32 &rfAlphaBias );

	// changes the emitter's alpha bias to *pfAlphaBias and will update it every work call
extern void fparticle_SetAlphaBias( FParticle_EmitterHandle_t hEmitter, const f32 *pfAlphaBias );

	// forces an emitter to not add any lights to the world, regardless of whether of not the def file says to
	// also will remove any lights that have been added by this emitter
extern void fparticle_DoNotEmitLights( FParticle_EmitterHandle_t hEmitter ); 

	// changes the emitter's direction vector to use *pUnitDir, this will be update every work call
extern void fparticle_SetDirection( FParticle_EmitterHandle_t hEmitter, const CFVec3 *pUnitDir );

	// changes the emitter's duration time by scaling up the computed time by the passed in factor
extern void fparticle_SetDurationMultiplier( FParticle_EmitterHandle_t hEmitter, f32 fTimeMultiplier );

	// changes the emitter's positional offset, the default is 0.0f.
	// when particles are emitted, they will be offset in the emission direction by this amount, ignored when emitting directionless particles
extern void fparticle_SetPositionalOffset( FParticle_EmitterHandle_t hEmitter, f32 fOffset );

	// Moves all of the current particles into the new space.  This process is NOT cheap, but will allow
	// you to attach effects to quickly moving/rotating objects and have the particles stay relative
	// to the emitter point (an example of something that would use this function is fire coming out
	// of the barrel of a gun, no matter how fast you rotate the gun, the fire should stick to the tip
	// of the barrel).  This function will update the position and direction of emitter.
extern void fparticle_TransformParticleIntoNewSpace( FParticle_EmitterHandle_t hEmitter, CFMtx43A &rNewSpace );







////////////////////////////////////////////////////////////////////////////////////
// METHODS EXPRESSLY FOR THE PARTICLE TOOL, DO NOT CALL THESE IN ANY OTHER CONDITION
extern BOOL fparticle_AreAnimFunctionsEqual( const FParticleAnimFunc_t *pFunc1, const FParticleAnimFunc_t *pFunc2 );
extern FParticle_DefHandle_t fparticle_LoadDefFileFromMem( FParticleDef_t *pDef );
extern FParticle_EmitterHandle_t fparticle_SpawnToolEmitter( FParticle_DefHandle_t hParticleDef, 
															 const CFVec3 *pPos,		// must point to perm memory	
															 const CFVec3 *pUnitDir,	// must point to perm memory
															 f32 *pfUnitIntensity,		// must point to perm memory
															 const FParticleShape_t *pCollShape );
extern void fparticle_GetCounts( u32 &rnNumParticles, u32 &rnNumSprites );
extern void fparticle_PauseAllEmitter( BOOL bPause );
extern void fparticle_StopAllEmitterLights( FParticleDef_t *pDef );
extern void fparticle_ResetEmitterFlags( FParticleDef_t *pDef, BOOL bResetParticles );
extern void fparticle_DrawEmitterBoundingSpheres( FParticleDef_t *pDef );
extern void fparticle_NoNearCulling( void );
extern BOOL fparticle_GetFirstEmitterBSphere( FParticleDef_t *pDef, CFVec3 &rPos, f32 &rRadius );
extern BOOL fparticle_GetFirstEmitterLightBSphere( FParticleDef_t *pDef, CFVec3 &rPos, f32 &rRadius );
extern void fparticle_CalculateKeyFrame( FParticleKeyFrame_t *pKeyFrame, const FParticleDef_t *pDef, f32 fUnitIntensity );

#endif


