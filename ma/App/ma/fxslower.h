//////////////////////////////////////////////////////////////////////////////////////
// fxslower.h
//
// Author: Michael Scholz
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
// 02/16/03 Scholz		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FXSLOWER_H_
#define _FXSLOWER_H_		1

#include "fang.h"
#include "fworld.h"
#include "fparticle.h"
#include "explosion.h"

class CEntity;
class CFMeshInst;
class CBot;

FCLASS_ALIGN_PREFIX class CFXSlower 
{
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	typedef struct
	{
		cchar*	pszBlastSphere;	//	,"gf_EMP01",#	_SLOWER_BLAST_SPHERE
		cchar*	pszPartTrailName;	// "EMP_part01",#	_SLOWER_PARTICLE_TRAIL_NAME
		cchar*	pszPartBlastName;	//,"EMP_part02",#	_SLOWER_PARTICLE_BURST_NAME
		cchar*	pszDetonateBlastName;
		f32		fGrowRateMin;		//	,20.0,#		_SLOWER_BLAST_GROW_RATE_MIN
		f32		fGrowRateMax;		//	,60.0,#		_SLOWER_BLAST_GROW_RATE_MAX
		f32		fGrowRateWobble;		//,1.5,#		_SLOWER_BLAST_GROW_RATE_WOBBLE
		f32		fFadeOutTime;		// ,1.0,#		_SLOWER_SPHERE_FADE_OUT_TIME
		f32		fSphereScale;		//	 ,1.0,#		_SLOWER_BLAST_SPHERE_SCALE
		f32		fPartRadiusGrow;		//	,50.0,#		_SLOWER_PARTICLE_RADIUS_GROW_RATE
		f32		fPartBurstRateMin;	//	,0.3,#		_SLOWER_PARTICLE_BURST_RATE_MIN
		f32		fPartBurstRateMax;		// ,1.5,#		_SLOWER_PARTICLE_BURST_RATE_MAX
		f32		fPartBaseVel;		//,2.7,#		_SLOWER_PARTICLE_BASE_VEL
		f32		fPartBaseVelMin;		// ,0.5,#		_SLOWER_PARTICLE_BASE_VEL_MIN
		f32		fPartBaseVelMax;//	,1.0,#		_SLOWER_PARTICLE_BASE_VEL_MAX
		u32		uMaxBlasts;			// ,1
		u32		uMaxSpheres;			// ,3,		_MAX_BLAST_SPHERES
		u32		uMaxSparklers;		//	,13,# 		_MAX_SPARKLERS
		f32		fMaxRadius;
		f32		fLife;
		f32		fUnitSlowedDown;
		f32		fFullEffectTime;
		f32		fRecoveryTime;
	} _UserProps_t;
private:
//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------

	typedef struct {
		CFVec3A Position; // 4 * 4  = 16 bytes
		CFVec3 Rotate;	  // 3 * 4  = 12 bytes
		CFVec3 RotateVel; // 3 * 4  = 12 bytes == 40 bytes
		
		f32 fLife;								// 1 * 4  =  4 bytes
		f32 fCurrentRadius;						// 1 * 4  =  4 bytes
		f32 fMaxRadius;							// 1 * 4  =  4 bytes
		FParticle_EmitterHandle_t hTrailEmitter;// 1 * 4  =  4 bytes
		BOOL bAlive;							// 1 * 4  =  4 bytes == 20 bytes
						// one byte wasted, then... 
		FCLASS_STACKMEM_ALIGN( BlastSparkler_t );
	} BlastSparkler_t;

	typedef struct {
		BOOL bActive;			// Is the sphere active
		BOOL bWobble;			// Should we wobble the sphere after hitting the max radius
		f32 fGrowRate;			// How fast should the sphere grow
		f32 fMaxRadius;			// Max radius
		f32 fOOMaxRadius;		// fmath_Inv( m_fMaxRadius )
		f32 fCurrentRadius;		// Current sphere radius
		f32 fMinWobbleRadius;	// Min wobble radius
		f32 fWobbleDir;			// Direction to wobble: 1.0f = grow, -1.0f = shrink, even if m_bWobble = FALSE, must be 1.0f
		f32 fRotateZ;			// How much to rotate about Z
		f32 fAlpha;				// Alpha of this sphere
		FCLASS_STACKMEM_NOALIGN( BlastSphere_t );
	} BlastSphere_t;

	static const _UserProps_t* m_pProps;

	static BOOL m_bSystemInitialized;					// TRUE if the system has been initialized
	static BOOL m_bActive;								// TRUE if there are blasts that are alive
	static CFXSlower *m_pBlasts;
	static FParticle_DefHandle_t m_hTrailParticleDef;
	static FParticle_DefHandle_t m_hBurstParticleDef;
	static FExplosion_GroupHandle_t m_hExplosionGroup;
	static CFMeshInst *m_pMeshInst;
	static f32 m_fOOMeshRadiusOuter;
	
	static f32	m_fUnitSlowedDown;							// How much does it slow'em down
	static f32	m_fFullEffectTime;							// What's the total effect's residual time
	static f32	m_fRecoveryTime;							// How long to recover

	CFMtx43A m_PosWS;									// WS position
	BlastSparkler_t* m_apSparklers;						// Sparklers
	BlastSphere_t*	 m_apBlastSpheres;					// Blast spheres
	BOOL m_bAlive;										// Is the blast alive
	BOOL m_bPosterXYZ;									// Is the blast drawn postered?
	f32 m_fLife;										// Time lived
	f32 m_fLifeMax;										// Max life to live
	f32 m_fMaxRadius;									// Max blast radius
	CFVec3A* m_pAttachPosWS;							// WS attach position
	CBot* m_pLaunchBot;
//	CFWorldLight m_WorldLight;

public:
	static BOOL InitSystem( const _UserProps_t* pProps );
	static void UninitSystem( void );

	static FINLINE BOOL IsSystemInitialized( void ) { return m_bSystemInitialized; };

	static void KillAll( void );
	static void Work( void );
	static void Draw( void );
	static void SpawnSlowerEffect( CBot* pLaunchBot, const CFMtx43A &PosWS, CFVec3A* pFollowPosWS = NULL, BOOL bPosterXYZ = FALSE );
	static void SpawnSlowerBurst( FExplosionSpawnParams_t& rParams );

protected:
	CFXSlower();

	void _Work( void );
	void _Kill( void );

	void _SpawnBlastSphere( const u32 &uIndex, BOOL bWobble = FALSE );
	void _WorkSparklers( void );
	void _KillParticles( void );
	void _SpawnSparkler( u32 uNdx );

	static CFXSlower *_GetNewBlast( void );
	static BOOL _WorldLoadAndUnloadCallback( FWorldEvent_e eEvent );
	static BOOL _SlowerTrackerCallback( CFWorldTracker *pTracker, FVisVolume_t *pWorldLeafNode );
FCLASS_STACKMEM_ALIGN( CFXSlower );
} FCLASS_ALIGN_SUFFIX;

#endif