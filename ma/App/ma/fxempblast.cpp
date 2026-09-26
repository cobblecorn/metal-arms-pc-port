/////////////////////////////////////////////////////////////////////////////////////
// fxempblast.cpp
//
// Author: Nathan Miller
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
// 11/20/02 Miller		Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "fvis.h"
#include "fdraw.h"
#include "floop.h"
#include "entity.h"
#include "fcamera.h"
#include "gamecam.h"
#include "frenderer.h"
#include "fparticle.h"
#include "fresload.h"
#include "fxempblast.h"
#include "fvtxpool.h"
#include "fsound.h"


// Used for both EMP and bolt groups
#define _EMP_MAX_BLASTS					( 2 )

// Lightning bolt defines
#define _LIGHTB_BOLT_TEXTURE			"tf_bolt03"
#define _LIGHTB_MAX_LIGHT_BOLTS			( 55 )
#define _LIGHTB_MAX_BRANCHES			( 6 )
#define _LIGHTB_RADIUS_SCALE_MIN		( 0.75f )
#define _LIGHTB_RADIUS_SCALE_MAX		( 1.65f )
#define _LIGHTB_LIFE_MIN				( 1.0f )
#define _LIGHTB_LIFE_MAX				( 3.0f )
#define _LIGHTB_TIME_SPAWN_MIN			( 0.005f )
#define _LIGHTB_TIME_SPAWN_MAX			( 0.005f )
#define _LIGHTB_NUM_SPAWN_MIN			( 1 )
#define _LIGHTB_NUM_SPAWN_MAX			( 5 )
#define _LIGHTB_BOLT_GROW_RATE			( 200.0f )
#define _LIGHTB_BRANCH_WIDTH			( 0.2f )
// This was for the m_vecUnitOfsFromParent and m_OffsetDir code
//#define _LIGHTB_BRANCH_OFFSET_VEL_MIN	( 0.5f ) 
//#define _LIGHTB_BRANCH_OFFSET_VEL_MAX	( 1.0f )
#define _LIGHTB_BRANCH_OFFSET_VEL_MIN	( ( -FMATH_QUARTER_PI * 0.75f ) )
#define _LIGHTB_BRANCH_OFFSET_VEL_MAX	( ( FMATH_QUARTER_PI * 0.75f ) )
#define _LIGHTB_BOLT_ALPHA_RANDOM_MIN	( 0.2f )
#define _LIGHTB_BOLT_ALPHA_RANDOM_MAX	( 0.7f )

// EMP defines
#define _EMP_BLAST_SPHERE				"gf_emp02"
#define _EMP_PARTICLE_TRAIL_NAME		"emp_part01"
#define _EMP_PARTICLE_BURST_NAME		"emp_part02"
#define _EMP_BLAST_GROW_RATE_MIN		( 20.0f )
#define _EMP_BLAST_GROW_RATE_MAX		( 60.0f )
#define _EMP_BLAST_GROW_RATE_WOBBLE		( 1.5f )
#define _EMP_SPHERE_FADE_OUT_TIME		( 1.0f )
#define _EMP_BLAST_SPHERE_SCALE			( 1.0f ) // Change this value if the blast sphere has a halo on it or something similar
#define _EMP_PARTICLE_RADIUS_GROW_RATE  ( 50.0f )
#define _EMP_PARTICLE_BURST_RATE_MIN	( 0.3f )
#define _EMP_PARTICLE_BURST_RATE_MAX	( 1.5f )
#define _EMP_PARTICLE_BASE_VEL			( FMATH_PI * 0.85f )
#define _EMP_PARTICLE_BASE_VEL_MIN		( 0.5f )
#define _EMP_PARTICLE_BASE_VEL_MAX		( 1.0f )

#define _EMP_SOUND_FADEOUT_TIME			( 1.0f )

// Lightning bolt code derived from BlinkSpeed...

// 
// CEMPLBBranch 
//
FCLASS_ALIGN_PREFIX class CEMPLBBranch {
public:
	CEMPLBBranch()	{ Clear(); }
	~CEMPLBBranch()	{ }

	void Clear( void );
	void Init( f32 fTheta, f32 fLength );
	void AddToVtxList(FDrawVtx_t *aVertArray, u32 &uCurVtx, const CFVec3A &vecParentPos, const CFVec3A &vecRight_WS, const CFVec3A &vecUp_WS, f32 fAlpha);

	BOOL m_bDraw;						// Should we draw this branch
	f32 m_fDistTotal;					// If we should draw, how much of the m_fDistFromParent should we use
	f32 m_fDistFromParent;				// Distance from parent
	f32 m_fOffsetVel;					// How fast to move the branch
	//CFVec2 m_vecUnitOfsFromParent;	// 2D vector that gives direction of bolt
	//CFVec2 m_OffsetDir;
	f32 m_fAngle;						// Angle of the branch

	CEMPLBBranch *m_apChildren[2];

	FCLASS_STACKMEM_ALIGN( CEMPLBBranch );
} FCLASS_ALIGN_SUFFIX;

// 
// CEMPLightningBolt
//
FCLASS_ALIGN_PREFIX class CEMPLightningBolt {

public:
	CEMPLightningBolt()  { Clear(); }
	~CEMPLightningBolt() { }

	FINLINE const f32 &GetAlpha( void ) { return m_fCurAlpha; }

	void Clear( void );
	void Work( void );
	void Init( const f32 &fLifeTime, const f32 &fRadius );
	void FDraw( const CFXfm *pCamXfm, const CFVec3A &PosWS );

private:
	CEMPLBBranch *_BuildBolt( u32 &uStartBranchIdx, u32 uDepth, f32 fThetaIn, f32 fLengthIn );
	
	f32 m_fCurAlpha;									// Current alpha of this branch.  When GetAlpha() == 0.0f, we are dead
	f32 m_fFadeRate;									// How much to fade each frame
	CEMPLBBranch m_aBranches[_LIGHTB_MAX_BRANCHES];		// Possible branches for this bolt
	
public:
	FLink_t m_Link;

	FCLASS_STACKMEM_ALIGN( CEMPLightningBolt );
} FCLASS_ALIGN_SUFFIX;

// 
// CEMPLightningBoltGroup
//
FCLASS_ALIGN_PREFIX class CEMPLightningBoltGroup {

public:
	static BOOL InitSystem( void );
	static void UninitSystem( void );
	static void Work( void );
	static void Draw( void );
	static void Release( CEMPLightningBoltGroup *pBoltGroup );
	static CEMPLightningBoltGroup *GetLightning( void );

	BOOL IsAlive( void )	{ return m_ActiveBolts.nCount != 0; }
	void SetNoSpawn( void ) { m_bNoSpawn = TRUE; }

	BOOL Init( u32 uNumBolts, const CFVec3A &PosWS, const f32 &fLife, const f32 &fRadius );
	
private:
	CEMPLightningBoltGroup();
	~CEMPLightningBoltGroup();

	void _Clear( void );
	void _Work( void );

	BOOL m_bAlive;										// Is this bolt group alive
	BOOL m_bNoSpawn;									// If TRUE, don't spawn any more bolts

	f32 m_fRadius;										// Max radius for this bolt group
	f32 m_fTimeTillNextSpawn;							// Time till the next spawn of bolts

	CFVec3A m_PosWS;									// Position to shoot from in WS
	CEMPLightningBolt m_aLB[_LIGHTB_MAX_LIGHT_BOLTS];	// The bolts

	FLinkRoot_t m_FreeBolts;							// Free bolts
	FLinkRoot_t m_ActiveBolts;							// Active bolts

	static BOOL m_bSystemInitialized;
	static u32 m_uNumAlive;								// Number of bolt groups that are alive
	static CEMPLightningBoltGroup *m_aBoltGroupPool;	// Pool of bolt groups
	static CFTexInst s_oTexInst;						// Texture to use

	FCLASS_STACKMEM_ALIGN( CEMPLightningBoltGroup );
} FCLASS_ALIGN_SUFFIX;

// 
// Lightning bolt stuff
//
BOOL CEMPLightningBoltGroup::m_bSystemInitialized = FALSE;
CEMPLightningBoltGroup *CEMPLightningBoltGroup::m_aBoltGroupPool;
CFTexInst CEMPLightningBoltGroup::s_oTexInst;
u32 CEMPLightningBoltGroup::m_uNumAlive = 0;

// 
// EMP stuff
//
BOOL CFXEMPBlast::m_bSystemInitialized = FALSE;
CFXEMPBlast *CFXEMPBlast::m_pBlasts = 0;
BOOL CFXEMPBlast::m_bActive = FALSE;

#if FXEMPBLAST_INCLUDE_SPARKLERS
	FParticle_DefHandle_t CFXEMPBlast::m_hTrailParticleDef = FPARTICLE_INVALID_HANDLE;
	FParticle_DefHandle_t CFXEMPBlast::m_hBurstParticleDef = FPARTICLE_INVALID_HANDLE;
#endif

CFMeshInst *CFXEMPBlast::m_pMeshInst = NULL;
f32 CFXEMPBlast::m_fOOMeshRadiusOuter = 0.0f;

//
// CFXEMPBlast
//
CFXEMPBlast::CFXEMPBlast() {
	m_bAlive = FALSE;
	m_fLife = 0.0f;
	m_fMaxRadius = 0.0f;
	m_fMaxWobbleRadius = 0.0f;
	m_pBoltGroup = NULL;
	m_fOrigSoundUnitVolume = 0.0f;
	m_pAudioEmitterEffect = NULL;
}

BOOL CFXEMPBlast::InitSystem( void ) {
//	DEVPRINTF( "*** %d CFXEMPBlast\n", sizeof( CFXEMPBlast ) );
//	DEVPRINTF( "*** %d bolt grp\n", sizeof( CEMPLightningBoltGroup ) );
//	DEVPRINTF( "*** %d bolt\n", sizeof( CEMPLightningBolt ) );
//	DEVPRINTF( "*** %d bolt branch\n", sizeof( CEMPLBBranch ) );
	FResFrame_t Frame;
	FMesh_t *pMesh = NULL;

	Frame = fres_GetFrame();

	m_bSystemInitialized = TRUE;

	if( !CEMPLightningBoltGroup::InitSystem() ) {
		goto _ExitInitSystemWithError;
	}

	m_pBlasts = fnew CFXEMPBlast[_EMP_MAX_BLASTS];

	if( m_pBlasts == NULL ) {
		DEVPRINTF( "CFXEMPBlast::InitSystem(): Could not allocate blast array\n" );
		goto _ExitInitSystemWithError;
	}

	// Kill them
	for( u32 i = 0; i < _EMP_MAX_BLASTS; ++i ) {
		m_pBlasts[i].m_bAlive = FALSE;
	}

#if FXEMPBLAST_INCLUDE_SPARKLERS
	// Particle effects
	m_hTrailParticleDef = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, _EMP_PARTICLE_TRAIL_NAME );
		
	if( m_hTrailParticleDef == FPARTICLE_INVALID_HANDLE ) {
		DEVPRINTF( "CFXEMPBlast::ClassHierarchyBuild(): Could not find particle definition '%s'.\n", _EMP_PARTICLE_TRAIL_NAME );
	}

	m_hBurstParticleDef = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, _EMP_PARTICLE_BURST_NAME );
		
	if( m_hBurstParticleDef == FPARTICLE_INVALID_HANDLE ) {
		DEVPRINTF( "CFXEMPBlast::ClassHierarchyBuild(): Could not find particle definition '%s'.\n", _EMP_PARTICLE_BURST_NAME );
	}
#endif

	// Outer sphere mesh
	pMesh = (FMesh_t*) fresload_Load( FMESH_RESTYPE, _EMP_BLAST_SPHERE );
	if( pMesh == NULL ) {
		DEVPRINTF( "CFXEMPBlast::InitSystem():  Unable to load mesh %s\n", _EMP_BLAST_SPHERE );
		goto _ExitInitSystemWithError;
	}

	m_pMeshInst = fnew CFMeshInst;
	
	if( m_pMeshInst == NULL ) {
		DEVPRINTF( "CFXEMPBlast::Create(): Not enough memory to create CFMeshInst.\n" );
		goto _ExitInitSystemWithError;
	}

	m_pMeshInst->Init( pMesh );

	if( m_pMeshInst->m_pMesh->BoundSphere_MS.m_fRadius > 0.0f ) {
		m_fOOMeshRadiusOuter = fmath_Inv( m_pMeshInst->m_pMesh->BoundSphere_MS.m_fRadius * _EMP_BLAST_SPHERE_SCALE );
	}

	if( ! fworld_IsWorldCallbackFunctionRegistered( _WorldLoadAndUnloadCallback ) ) {
		fworld_RegisterWorldCallbackFunction( _WorldLoadAndUnloadCallback );
	}

	return TRUE;

_ExitInitSystemWithError:
	UninitSystem();
	fres_ReleaseFrame( Frame );
	return FALSE;
}

void CFXEMPBlast::UninitSystem( void ) {
	if( !m_bSystemInitialized ) {
		return;
	}

	m_bSystemInitialized = FALSE;
	
	fdelete_array( m_pBlasts );
	m_pBlasts = NULL;

	fdelete( m_pMeshInst );
	m_pMeshInst = NULL;

	CEMPLightningBoltGroup::UninitSystem();

	if( fworld_IsWorldCallbackFunctionRegistered( _WorldLoadAndUnloadCallback ) ) {
		fworld_UnregisterWorldCallbackFunction( _WorldLoadAndUnloadCallback );
	}
}

void CFXEMPBlast::KillAll( void ) {
	m_bActive = FALSE;

	for( u32 i = 0; i < _EMP_MAX_BLASTS; ++i ) {
		if( m_pBlasts[i].m_bAlive ) {
			m_pBlasts[i]._Kill();
		}
	}
}

void CFXEMPBlast::Work( void ) {
	FASSERT( m_bSystemInitialized );

	if( !m_bActive ) {
		return;
	}

	CEMPLightningBoltGroup::Work();

	m_bActive = FALSE;

	for( u32 i = 0; i < _EMP_MAX_BLASTS; ++i ) {
		if( m_pBlasts[i].m_bAlive ) {
			m_pBlasts[i]._Work();
			m_bActive = TRUE;
		}
	}
}

void CFXEMPBlast::Draw( void ) {
	FASSERT( m_bSystemInitialized );

	if( !m_bActive ) {
		return;
	}

	// Draw the bolts
	CEMPLightningBoltGroup::Draw();

	CFMtx43A Mtx;
	CFCamera *pCam = gamecam_GetActiveCamera();
	f32 fScale;

	FASSERT( pCam );

	frenderer_Push( FRENDERER_MESH, NULL );
	fmesh_Ambient_Set( 1.0f, 1.0f, 1.0f, 1.0f );

	for( u32 i = 0; i < _EMP_MAX_BLASTS; ++i ) {
		if( m_pBlasts[i].m_bAlive ) {
			for( u32 j = 0; j < _MAX_BLAST_SPHERES; ++j ) {
				if( m_pBlasts[i].m_BlastSpheres[j].bActive ) {
					CFMtx43A::m_RotZ.Identity();
					CFMtx43A::m_RotZ.RotateZ( m_pBlasts[i].m_BlastSpheres[j].fRotateZ );
								
					Mtx = pCam->GetFinalXfm()->m_MtxR;
					Mtx.Mul( CFMtx43A::m_RotZ );
					Mtx.m_vPos = m_pBlasts[i].m_PosWS;

					fScale = m_pBlasts[i].m_BlastSpheres[j].fCurrentRadius * m_fOOMeshRadiusOuter;

					m_pMeshInst->m_Xfm.BuildFromMtx( Mtx, fScale );

					//Need to push the model matrix onto the stack so that the object is culled correctly.
					m_pMeshInst->m_Xfm.PushModel();

					m_pMeshInst->SetMeshAlpha( m_pBlasts[i].m_BlastSpheres[j].fAlpha );
					m_pMeshInst->Draw( FVIEWPORT_PLANESMASK_ALL );

					//Push the model matrix off the stack.
					m_pMeshInst->m_Xfm.PopModel();
				}
			}
		}
	}

	frenderer_Pop();
}

void CFXEMPBlast::SpawnBlast( const f32 &fLife, const f32 &fMaxRadius, const CFVec3A &PosWS, CFSoundGroup *pSoundGroupEffect, BOOL bRecruiterEffect ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( fMaxRadius > 0.0f );
	FASSERT( fLife > 0.0f );

	m_bActive = TRUE;

	CFXEMPBlast *pBlast = _GetNewBlast();

	pBlast->m_bAlive = TRUE;
	pBlast->m_bRecruiterEffect = bRecruiterEffect;
	pBlast->m_PosWS = PosWS;
	pBlast->m_fLifeMax = fLife;
	pBlast->m_fLife = 0.0f;
	pBlast->m_fMaxRadius = fMaxRadius;
	pBlast->m_fMaxWobbleRadius = bRecruiterEffect ? 4.0f : fMaxRadius;

	if( pBlast->m_pAudioEmitterEffect ) {
		pBlast->m_pAudioEmitterEffect->Destroy();
		pBlast->m_pAudioEmitterEffect = NULL;
	}

	pBlast->m_pAudioEmitterEffect = CFSoundGroup::AllocAndPlaySound( pSoundGroupEffect, FALSE, &PosWS, 1.0f, 1.0f, -1.0f, &pBlast->m_fOrigSoundUnitVolume );

	if( pBlast->m_pBoltGroup ) {
		CEMPLightningBoltGroup::Release( pBlast->m_pBoltGroup );
		pBlast->m_pBoltGroup = NULL;
	}

	pBlast->m_pBoltGroup = CEMPLightningBoltGroup::GetLightning();
	pBlast->m_pBoltGroup->Init( _LIGHTB_MAX_LIGHT_BOLTS,  PosWS, fLife, pBlast->m_fMaxWobbleRadius );

	u32 i;

	// _SpawnBlastSphere() creates wobblers, we don't want that for the first spheres
	for( i = 0; i < _MAX_BLAST_SPHERES; ++i ) {
		pBlast->m_BlastSpheres[i].bActive = TRUE;
		pBlast->m_BlastSpheres[i].bWobble = FALSE;
		pBlast->m_BlastSpheres[i].fGrowRate = fmath_RandomFloatRange( _EMP_BLAST_GROW_RATE_MIN * 3.0f, _EMP_BLAST_GROW_RATE_MAX * 3.0f );
		pBlast->m_BlastSpheres[i].fWobbleDir = 1.0f;
		pBlast->m_BlastSpheres[i].fMaxRadius = fMaxRadius * 1.5f;
		pBlast->m_BlastSpheres[i].fOOMaxRadius = fmath_Inv( pBlast->m_BlastSpheres[i].fMaxRadius );
		pBlast->m_BlastSpheres[i].fCurrentRadius = 1.0f;
		pBlast->m_BlastSpheres[i].fMinWobbleRadius = 0.0f;
		pBlast->m_BlastSpheres[i].fMaxWobbleRadius = 0.0f;
		pBlast->m_BlastSpheres[i].fAlpha = 1.0f;
		pBlast->m_BlastSpheres[i].fRotateZ = fmath_RandomFloatRange( 0.0f, FMATH_2PI );
	}

	#if FXEMPBLAST_INCLUDE_SPARKLERS
		if( !bRecruiterEffect ) {
			for( i = 0; i < _MAX_SPARKLERS; ++i ) {
				pBlast->_SpawnSparkler( i );
			}
		}
	#endif

	// Add a light
	FLightInit_t LightInit;

	LightInit.szPerPixelTexName[0] = 0;
	LightInit.szCoronaTexName[0] = 0;
	LightInit.nLightID = 0xffff;
	LightInit.mtxOrientation.Identity();
	LightInit.fIntensity = 1.0f;
	LightInit.Motif.Set( 0.4f, 1.0f, 1.0f, 1.0f );
	LightInit.Motif.SetIndex( FCOLORMOTIF_PULSATEHI0 );
	LightInit.nFlags = FLIGHT_FLAG_HASPOS | FLIGHT_FLAG_PER_PIXEL;
	LightInit.nType = FLIGHT_TYPE_OMNI;
	LightInit.spInfluence.m_Pos = PosWS.v3;
	LightInit.spInfluence.m_fRadius = (bRecruiterEffect ? 1.0f : 2.0f) * pBlast->m_fMaxRadius;
	LightInit.szName[0] = 0;
	pBlast->m_WorldLight.Init( &LightInit );

}

void CFXEMPBlast::_Work( void ) {
	m_fLife += FLoop_fPreviousLoopSecs;

	if( m_fLife >= m_fLifeMax ) {
		// Kill sound...
		if( m_pAudioEmitterEffect ) {
			m_pAudioEmitterEffect->Destroy();
			m_pAudioEmitterEffect = NULL;
		}

		// The bolts are still alive, tell it not to spawn more so we can fade out
		if( m_pBoltGroup && m_pBoltGroup->IsAlive() ) {
			m_pBoltGroup->SetNoSpawn();
		} else {
			_Kill();
			return;
		}
	}

	#if FXEMPBLAST_INCLUDE_SPARKLERS
		if( !m_bRecruiterEffect ) {
			_WorkSparklers();
		}
	#endif

	f32 fLifeLeft = ( m_fLifeMax - m_fLife );

	// DFS -- If the bolts are still alive, this could be negative...
	if( fLifeLeft < 0.0f )
		fLifeLeft = 0.0f;

	if( m_pAudioEmitterEffect ) {
		if( fLifeLeft < _EMP_SOUND_FADEOUT_TIME ) {
			f32 fUnitVolume = fLifeLeft * (1.0f / _EMP_SOUND_FADEOUT_TIME);
			FMATH_CLAMPMAX( fUnitVolume, 1.0f );

			m_pAudioEmitterEffect->SetVolume( m_fOrigSoundUnitVolume * fUnitVolume );
		}
	}

	if( fLifeLeft <= ( _EMP_SPHERE_FADE_OUT_TIME * 2.0f ) ) {
		if( m_pBoltGroup && m_pBoltGroup->IsAlive() ) {
			m_pBoltGroup->SetNoSpawn();
		}
	}

	for( u32 i = 0; i < _MAX_BLAST_SPHERES; ++i ) {
		if( m_BlastSpheres[i].bActive ) {
			m_BlastSpheres[i].fCurrentRadius += ( FLoop_fPreviousLoopSecs *  m_BlastSpheres[i].fGrowRate *  m_BlastSpheres[i].fWobbleDir );

			if( m_BlastSpheres[i].bWobble ) {
				if( m_BlastSpheres[i].fCurrentRadius > m_BlastSpheres[i].fMaxWobbleRadius ) {
					m_BlastSpheres[i].fWobbleDir = -1.0f;
					m_BlastSpheres[i].fCurrentRadius = m_BlastSpheres[i].fMaxWobbleRadius;
					m_BlastSpheres[i].fGrowRate = _EMP_BLAST_GROW_RATE_WOBBLE;
				} else if( m_BlastSpheres[i].fCurrentRadius < m_BlastSpheres[i].fMinWobbleRadius ) {
					m_BlastSpheres[i].fWobbleDir = 1.0f;
					m_BlastSpheres[i].fCurrentRadius =  m_BlastSpheres[i].fMinWobbleRadius;
				}
			} else {
				if( m_BlastSpheres[i].fCurrentRadius > m_BlastSpheres[i].fMaxRadius ) {
					_SpawnBlastSphere( i, TRUE );
					continue;
				}
			}

			m_BlastSpheres[i].fAlpha = 1.0f - ( m_BlastSpheres[i].fCurrentRadius * m_BlastSpheres[i].fOOMaxRadius );

			// So we can fade out at the end of our life
			if( fLifeLeft <= _EMP_SPHERE_FADE_OUT_TIME ) {
				f32 fUnitIntensity = (fLifeLeft * (1.0f / _EMP_SPHERE_FADE_OUT_TIME));

				m_BlastSpheres[i].fAlpha *= fUnitIntensity;

				m_WorldLight.m_Light.SetIntensity( fUnitIntensity );
			}
		}
	}
}

void CFXEMPBlast::_Kill( void ) {
	m_WorldLight.RemoveFromWorld();

	#if FXEMPBLAST_INCLUDE_SPARKLERS
		if( !m_bRecruiterEffect ) {
			_KillParticles();
		}
	#endif

	if( m_pBoltGroup ) {
		CEMPLightningBoltGroup::Release( m_pBoltGroup );
		m_pBoltGroup = NULL;
	}

	m_bAlive = FALSE;

	if( m_pAudioEmitterEffect ) {
		m_pAudioEmitterEffect->Destroy();
		m_pAudioEmitterEffect = NULL;
	}
}

void CFXEMPBlast::_SpawnBlastSphere( const u32 &uIndex, BOOL bWobble ) {
	f32 fMaxWobbleRadius = bWobble ? m_fMaxWobbleRadius : m_fMaxRadius;

	m_BlastSpheres[uIndex].bActive = TRUE;
	m_BlastSpheres[uIndex].bWobble = bWobble;
	m_BlastSpheres[uIndex].fGrowRate = fmath_RandomFloatRange( _EMP_BLAST_GROW_RATE_MIN, _EMP_BLAST_GROW_RATE_MAX );
	m_BlastSpheres[uIndex].fWobbleDir = 1.0f;
	m_BlastSpheres[uIndex].fCurrentRadius = 1.0f;
	m_BlastSpheres[uIndex].fMaxRadius = m_fMaxRadius;
	m_BlastSpheres[uIndex].fMaxWobbleRadius = fMaxWobbleRadius;
	m_BlastSpheres[uIndex].fMinWobbleRadius = fMaxWobbleRadius * fmath_RandomFloatRange( 0.8f, 0.9f );
	m_BlastSpheres[uIndex].fOOMaxRadius = fmath_Inv( m_fMaxRadius );
	m_BlastSpheres[uIndex].fAlpha = 1.0f;
	m_BlastSpheres[uIndex].fRotateZ = fmath_RandomFloatRange( 0.0f, FMATH_2PI );
}

#if FXEMPBLAST_INCLUDE_SPARKLERS
void CFXEMPBlast::_WorkSparklers( void ) {
	CFQuatA QuatX, QuatY, QuatZ;

	for( u32 i = 0; i < _MAX_SPARKLERS; ++i ) {
		if( m_Sparklers[i].bAlive ) {
			m_Sparklers[i].fLife -= FLoop_fPreviousLoopSecs;

			if( m_Sparklers[i].fLife < 0.0f ) {	
				m_Sparklers[i].bAlive = FALSE;

				// stop emitting particles
				if( m_Sparklers[i].hTrailEmitter != FPARTICLE_INVALID_HANDLE ) {
					fparticle_StopEmitter( m_Sparklers[i].hTrailEmitter );
					m_Sparklers[i].hTrailEmitter = FPARTICLE_INVALID_HANDLE;
				}
				
				// Boom
				fparticle_SpawnEmitter( m_hBurstParticleDef, m_Sparklers[i].Position.v3, NULL, 1.0f );
			}

			m_Sparklers[i].fCurrentRadius += ( FLoop_fPreviousLoopSecs * _EMP_PARTICLE_RADIUS_GROW_RATE );

			if( m_Sparklers[i].fCurrentRadius > m_Sparklers[i].fMaxRadius ) {
				m_Sparklers[i].fCurrentRadius = m_Sparklers[i].fMaxRadius;
			}

			m_Sparklers[i].Rotate.x += ( FLoop_fPreviousLoopSecs * m_Sparklers[i].RotateVel.x );
			m_Sparklers[i].Rotate.y += ( FLoop_fPreviousLoopSecs * m_Sparklers[i].RotateVel.y );
			m_Sparklers[i].Rotate.z += ( FLoop_fPreviousLoopSecs * m_Sparklers[i].RotateVel.z );

			QuatX.BuildQuatRotX( m_Sparklers[i].Rotate.x );
			QuatY.BuildQuatRotY( m_Sparklers[i].Rotate.y );
			QuatZ.BuildQuatRotZ( m_Sparklers[i].Rotate.z );
			QuatX.Mul( QuatY );
			QuatX.Mul( QuatZ );
			QuatX.MulPoint( m_Sparklers[i].Position, CFVec3A::m_UnitAxisX );

			m_Sparklers[i].Position.Mul( m_Sparklers[i].fCurrentRadius );
			m_Sparklers[i].Position.Add( m_PosWS );
		}
	}
}

void CFXEMPBlast::_KillParticles( void ) {
	for( u32 i = 0; i < _MAX_SPARKLERS; ++i ) {
		if( m_Sparklers[i].bAlive ) {
			m_Sparklers[i].bAlive = FALSE;

			// stop emitting particles
			if( m_Sparklers[i].hTrailEmitter != FPARTICLE_INVALID_HANDLE ) {
				fparticle_StopEmitter( m_Sparklers[i].hTrailEmitter );
				m_Sparklers[i].hTrailEmitter = FPARTICLE_INVALID_HANDLE;
			}

			fparticle_SpawnEmitter( m_hBurstParticleDef, m_Sparklers[i].Position.v3, NULL, 1.0f );
		}
	}
}

void CFXEMPBlast::_SpawnSparkler( u32 uNdx ) {
	m_Sparklers[uNdx].bAlive = TRUE;
	m_Sparklers[uNdx].fLife = m_fLifeMax * fmath_RandomFloatRange( 0.6f, 1.0f );
	m_Sparklers[uNdx].fMaxRadius = m_fMaxRadius * fmath_RandomFloatRange( 0.3f, 1.0f );
	m_Sparklers[uNdx].fCurrentRadius = 0.1f;
	m_Sparklers[uNdx].Position.Set(m_PosWS);
	m_Sparklers[uNdx].hTrailEmitter = fparticle_SpawnEmitter( m_hTrailParticleDef, &m_Sparklers[uNdx].Position.v3, NULL, NULL, 1.0f ); 
	m_Sparklers[uNdx].Rotate.Set( CFVec3A::m_Null );

	m_Sparklers[uNdx].RotateVel.x = _EMP_PARTICLE_BASE_VEL * fmath_RandomFloatRange( _EMP_PARTICLE_BASE_VEL_MIN, _EMP_PARTICLE_BASE_VEL_MAX );
	m_Sparklers[uNdx].RotateVel.y = _EMP_PARTICLE_BASE_VEL * fmath_RandomFloatRange( _EMP_PARTICLE_BASE_VEL_MIN, _EMP_PARTICLE_BASE_VEL_MAX );
	m_Sparklers[uNdx].RotateVel.z = _EMP_PARTICLE_BASE_VEL * fmath_RandomFloatRange( _EMP_PARTICLE_BASE_VEL_MIN, _EMP_PARTICLE_BASE_VEL_MAX );

	if( fmath_RandomChoice( 2 ) ) {
		m_Sparklers[uNdx].RotateVel.x = -m_Sparklers[uNdx].RotateVel.x;
	}

	if( fmath_RandomChoice( 2 ) ) {
		m_Sparklers[uNdx].RotateVel.y = -m_Sparklers[uNdx].RotateVel.y;
	}

	if( fmath_RandomChoice( 2 ) ) {
		m_Sparklers[uNdx].RotateVel.z = -m_Sparklers[uNdx].RotateVel.z;
	}

	m_Sparklers[uNdx].Rotate.y = fmath_RandomFloatRange( -FMATH_PI, FMATH_PI );
}
#endif

CFXEMPBlast *CFXEMPBlast::_GetNewBlast( void ) {
	u32 uBest = -1;
	f32 fLowLife = FMATH_MAX_FLOAT;

	for( u32 i = 0; i < _EMP_MAX_BLASTS; ++i ) {
		if( !m_pBlasts[i].m_bAlive ) {
			return &m_pBlasts[i];
		}

		if( m_pBlasts[i].m_fLife < fLowLife ) {
			fLowLife = m_pBlasts[i].m_fLife;
			uBest = i;
		}
	}

	FASSERT( uBest != -1 );

	m_pBlasts[uBest]._Kill();

	return &m_pBlasts[uBest];
}

BOOL CFXEMPBlast::_WorldLoadAndUnloadCallback( FWorldEvent_e eEvent ) {
	if( FWORLD_EVENT_WORLD_PREDESTROY == eEvent ) {
		KillAll();
	}

	return TRUE;
}

// 
// CEMPLBBranch
//
void CEMPLBBranch::Clear( void ) {
	m_apChildren[0] = NULL;
	m_apChildren[1] = NULL;

	m_fDistFromParent = 0.0f;
	m_fDistTotal = 0.0f;

	m_bDraw = FALSE;

	//m_vecUnitOfsFromParent.Set( 0.0f, 0.0f );
	//m_OffsetDir.Set( 0.0f, 0.0f );
	m_fAngle = 0.0f;
	m_fOffsetVel = 0.0f;
}

void CEMPLBBranch::Init(f32 fTheta, f32 fLength) {
	// Justin says -
	// These are reversed from what you might normally think because we want to measure
	//   the angle from off of the x+ axis.
	/*fmath_SinCos( fTheta, &m_vecUnitOfsFromParent.y, &m_vecUnitOfsFromParent.x );

	if( fmath_RandomChance( 0.3f ) ) {
		m_OffsetDir.Set( 0.0f, 0.0f );
	} else {
		f32 x, y;

		x = FMATH_FSIGN( fmath_RandomFloatRange( -1.0f, 1.0f ) );
		y = FMATH_FSIGN( fmath_RandomFloatRange( -1.0f, 1.0f ) );

		m_OffsetDir.Set( x, y );
		m_OffsetDir.Unitize();
	}*/

	// New less space hungry method
	m_fDistFromParent = fLength;
	m_fOffsetVel = fmath_RandomFloatRange( _LIGHTB_BRANCH_OFFSET_VEL_MIN, _LIGHTB_BRANCH_OFFSET_VEL_MAX );

	m_fAngle = fTheta;
}

// Justin
void CEMPLBBranch::AddToVtxList(FDrawVtx_t *aVertArray, u32 &uCurVtx, const CFVec3A &vecParentPos, const CFVec3A &vecRight_WS, const CFVec3A &vecUp_WS, f32 fAlpha)
{
	static CFVec3A vecDelY, vecDelX;		// These are in the space of the branch.
	static CFVec3A aavecPts[2][2];
	static CFVec3A vecTemp;
	static CFVec3A vecNewUp_WS, vecNewRight_WS;
	static CFColorRGBA Color( 0.53f, 0.71f, 0.91f );
	static f32 fY;
	static f32 fX;

	fY = fmath_Sin( m_fAngle );
	fX = fmath_Cos( m_fAngle );

	Color.fAlpha = fAlpha;

	//////////////////////////////////////////////////////////////////////
	// Calculate some interesting and useful vectors.
	// Don't you just love hand-crafted vector math?
	vecNewRight_WS = vecUp_WS;
	vecNewRight_WS.Mul(fY/*m_vecUnitOfsFromParent.y*/);
	vecTemp = vecRight_WS;
	vecTemp.Mul(fX/*m_vecUnitOfsFromParent.x*/);
	vecNewRight_WS.Add(vecTemp);

	vecDelX = vecNewRight_WS;
	//vecDelX.Mul(m_fDistFromParent);
	// !!Nate
	vecDelX.Mul(m_fDistTotal);

	vecNewUp_WS = vecUp_WS;
	vecNewUp_WS.Mul(fX/*m_vecUnitOfsFromParent.x*/);
	vecTemp = vecRight_WS;
	vecTemp.Mul(fY/*m_vecUnitOfsFromParent.y*/);
	vecNewUp_WS.Sub(vecTemp);

	vecDelY = vecNewUp_WS;
	vecDelY.Mul( _LIGHTB_BRANCH_WIDTH );

	CFVec3A vecEndPos = vecParentPos;
	vecEndPos.Add(vecDelX);
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	//
	aavecPts[0][0] = vecParentPos;
	aavecPts[0][0].Add(vecDelY);

	aavecPts[0][1] = vecParentPos;
	aavecPts[0][1].Sub(vecDelY);

	if((m_apChildren[0] != NULL) || (m_apChildren[1] != NULL))
	{
		// We have at least one children, draw the branch as usual.
		aavecPts[1][0] = aavecPts[0][0];
		aavecPts[1][0].Add(vecDelX);

		aavecPts[1][1] = aavecPts[0][1];
		aavecPts[1][1].Add(vecDelX);
	}
	else
	{
		// We have no children, let's taper this last branch.
		aavecPts[1][0] = vecEndPos;

		aavecPts[1][1] = vecEndPos;
	}
	//
	//////////////////////////////////////////////////////////////////////
	aVertArray[uCurVtx].ColorRGBA.Set( Color );
	aVertArray[uCurVtx].Pos_MS = aavecPts[0][0].v3;
	aVertArray[uCurVtx++].ST.Set(0.0f, 0.0f);

	aVertArray[uCurVtx].ColorRGBA.Set( Color );
	aVertArray[uCurVtx].Pos_MS = aavecPts[1][0].v3;
	aVertArray[uCurVtx++].ST.Set(0.0f, 1.0f);

	aVertArray[uCurVtx].ColorRGBA.Set( Color );
	aVertArray[uCurVtx].Pos_MS = aavecPts[0][1].v3;
	aVertArray[uCurVtx++].ST.Set(1.0f, 0.0f);

	aVertArray[uCurVtx].ColorRGBA.Set( Color );
	aVertArray[uCurVtx].Pos_MS = aavecPts[1][0].v3;
	aVertArray[uCurVtx++].ST.Set(0.0f, 1.0f);

	aVertArray[uCurVtx].ColorRGBA.Set( Color );
	aVertArray[uCurVtx].Pos_MS = aavecPts[0][1].v3;
	aVertArray[uCurVtx++].ST.Set(1.0f, 0.0f);

	aVertArray[uCurVtx].ColorRGBA.Set( Color );
	aVertArray[uCurVtx].Pos_MS = aavecPts[1][1].v3;
	aVertArray[uCurVtx++].ST.Set(1.0f, 1.0f);

	if(m_apChildren[0] != NULL)
	{
		if( m_apChildren[0]->m_bDraw ) {
			m_apChildren[0]->AddToVtxList( aVertArray, uCurVtx, vecEndPos, vecRight_WS, vecUp_WS, fAlpha );
		}

		if(m_apChildren[1] != NULL && m_apChildren[1]->m_bDraw )
		{
			m_apChildren[1]->AddToVtxList( aVertArray, uCurVtx, vecEndPos, vecRight_WS, vecUp_WS, fAlpha );
		}
	}

	// Justin says -
	// If this ASSERTs, you have several choices (in order of recommendation):
	//   1) Decrease the number of lightning bolts.
	//   2) Increase the size of the vertex array (and change the number below).
	//   3) Decrease the probability of spawning children branches (probably not the best choice).
	FASSERT(uCurVtx < 2000);
}


// 
// CEMPLightningBolt
//
void CEMPLightningBolt::Clear( void ) {
	m_fCurAlpha = 0.0f;
	m_fFadeRate = 0.0f;

	for( u32 i = 0; i < _LIGHTB_MAX_BRANCHES; ++i ) {
		m_aBranches[i].Clear();
	}
}

void CEMPLightningBolt::Work( void ) {
	m_fCurAlpha -= m_fFadeRate * FLoop_fPreviousLoopSecs;
	FMATH_CLAMP_MIN0( m_fCurAlpha );

	// No alpha, won't see anything
	if( m_fCurAlpha == 0.0f ) {
		return;
	}

	CEMPLBBranch *pCurBranch;
	CFVec2 Temp;

	for( u32 i = 0; i < _LIGHTB_MAX_BRANCHES; ++i ) {
		pCurBranch = &m_aBranches[i];

		// Grow the branches to make it look like the bolt is growing outward
		if( pCurBranch->m_bDraw ) {
			pCurBranch->m_fDistTotal += ( FLoop_fPreviousLoopSecs * _LIGHTB_BOLT_GROW_RATE );

			if( pCurBranch->m_fDistTotal >= pCurBranch->m_fDistFromParent ) {
				FMATH_CLAMPMAX( pCurBranch->m_fDistTotal, pCurBranch->m_fDistFromParent );

				if( pCurBranch->m_apChildren[0] ) {
					pCurBranch->m_apChildren[0]->m_bDraw = TRUE;
				}

				if( pCurBranch->m_apChildren[1] ) {
					pCurBranch->m_apChildren[1]->m_bDraw = TRUE;
				}
			}

			// Old space hungry way
			/*Temp.x = pCurBranch->m_fOffsetVel * ( FLoop_fPreviousLoopSecs * pCurBranch->m_OffsetDir.x );
			Temp.y = pCurBranch->m_fOffsetVel * ( FLoop_fPreviousLoopSecs * pCurBranch->m_OffsetDir.y );

			pCurBranch->m_vecUnitOfsFromParent.x += Temp.x;
			pCurBranch->m_vecUnitOfsFromParent.y += Temp.y;
			// Can get away without this, but some branches get really long
			pCurBranch->m_vecUnitOfsFromParent.Unitize();*/

			pCurBranch->m_fAngle += ( FLoop_fPreviousLoopSecs * pCurBranch->m_fOffsetVel );
		}
	}
}

void CEMPLightningBolt::Init( const f32 &fLifeTime, const f32 &fRadius ) {
	m_fCurAlpha = fmath_RandomFloatRange( _LIGHTB_BOLT_ALPHA_RANDOM_MIN, _LIGHTB_BOLT_ALPHA_RANDOM_MAX );

	f32 fRadScale = fmath_RandomFloatRange( _LIGHTB_RADIUS_SCALE_MIN, _LIGHTB_RADIUS_SCALE_MAX );
	f32 fTheta = fmath_RandomFloatRange( 0.0f, FMATH_2PI );
	u32 uBranchCount = 0;

	m_fFadeRate = fmath_Inv( fLifeTime );

	_BuildBolt( uBranchCount, 0, fTheta, fRadius * fRadScale );

	m_aBranches[0].m_bDraw = TRUE;
}

void CEMPLightningBolt::FDraw( const CFXfm *pCamXfm, const CFVec3A &PosWS ) {
	FDrawVtx_t *aVertArray = fvtxpool_GetArray( _LIGHTB_MAX_BRANCHES * 6 ); // 6 verts for each branch ( 2 triangles, 3 verts each )
	u32 uCurVtx = 0;

	if( !aVertArray ) {
		return;
	}

	m_aBranches[0].AddToVtxList( aVertArray, uCurVtx, PosWS, pCamXfm->m_MtxR.m_vX, pCamXfm->m_MtxR.m_vY, m_fCurAlpha );

	fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, aVertArray, uCurVtx );

	fvtxpool_ReturnArray( aVertArray );
}

// Derived from BlinkSpeed
CEMPLBBranch *CEMPLightningBolt::_BuildBolt( u32 &uStartBranchIdx, u32 uDepth, f32 fThetaIn, f32 fLengthIn ) {
	if( uStartBranchIdx == _LIGHTB_MAX_BRANCHES ) {
		return NULL;
	}

	CEMPLBBranch *pCurBranch = &m_aBranches[uStartBranchIdx];
	f32 fLengthUse = fLengthIn * fmath_RandomFloatRange( 0.01f, 0.2f );

	// Later on: Perhaps use uDepth in making this calculation.
	fThetaIn += fmath_RandomBipolarUnitFloat() * 0.70f;
	fLengthIn -= fLengthUse;

	// Initialize our first branch for this bolt.
	pCurBranch->Init(fThetaIn, fLengthUse);

	++uStartBranchIdx;

	f32 fProb = 1.0f - (f32)(uDepth) * 0.20f; 	// Decide if we want to build child bolts.

	FMATH_CLAMP_MIN0(fProb);

	if( fmath_RandomChance( fProb ) && fLengthIn > 0.1f ) {
		pCurBranch->m_apChildren[0] = _BuildBolt( uStartBranchIdx, uDepth + 1, fThetaIn, fLengthIn );

		fProb = 0.9f - (f32)(uDepth) * 0.20f;
		FMATH_CLAMP_MIN0( fProb );

		if( fmath_RandomChance( fProb ) ) {
			pCurBranch->m_apChildren[1] = _BuildBolt( uStartBranchIdx, uDepth + 1, fThetaIn, fLengthIn );
		} else {
			pCurBranch->m_apChildren[1] = NULL;
		}
	} else {
		pCurBranch->m_apChildren[0] = NULL;
		pCurBranch->m_apChildren[1] = NULL;
	}
	
	return pCurBranch;
}


//
// CEMPLightningBoltGroup
//
BOOL CEMPLightningBoltGroup::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );

	FResFrame_t Frame;
	FTexDef_t *pTexDef = NULL;			//CPS 4.7.03
		
	Frame = fres_GetFrame();

	m_bSystemInitialized = TRUE;

	m_aBoltGroupPool = fnew CEMPLightningBoltGroup[_EMP_MAX_BLASTS];

	if( m_aBoltGroupPool == NULL ) {
		DEVPRINTF( "CEMPLightningBoltGroup::InitSystem() : Out of memory.\n" );
		goto _ExitWithError;
	}

//CPS 4.7.03 FTexDef_t *pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, _LIGHTB_BOLT_TEXTURE );
	pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, _LIGHTB_BOLT_TEXTURE );		//CPS 4.7.03

	if( pTexDef == NULL ) {
		DEVPRINTF( "CEMPLightningBoltGroup::InitSystem(): Could not load '%s'.\n", _LIGHTB_BOLT_TEXTURE );
		return FALSE;
	}

	s_oTexInst.SetTexDef( pTexDef );

	m_uNumAlive = 0;

	return TRUE;

_ExitWithError:
	UninitSystem();
	fres_ReleaseFrame( Frame );

	return FALSE;
}

void CEMPLightningBoltGroup::UninitSystem( void ) {
	if( !m_bSystemInitialized ) {
		return;
	}

	fdelete_array( m_aBoltGroupPool );
	m_aBoltGroupPool = NULL;

	m_bSystemInitialized = FALSE;
}

CEMPLightningBoltGroup::CEMPLightningBoltGroup() {
	_Clear();
}

CEMPLightningBoltGroup::~CEMPLightningBoltGroup() {
	
}

void CEMPLightningBoltGroup::Work( void ) {
	FASSERT( m_bSystemInitialized );

	if( !m_uNumAlive ) {
		return;
	}

	u32 uPoolIdx;
	CEMPLightningBoltGroup *pCurGroup;

	for( uPoolIdx = 0; uPoolIdx < _EMP_MAX_BLASTS; ++uPoolIdx ) {
		pCurGroup = &m_aBoltGroupPool[uPoolIdx];

		if( pCurGroup->m_bAlive ) {
			pCurGroup->_Work();
		}
	}
}

void CEMPLightningBoltGroup::Draw( void ) {
	FASSERT( m_bSystemInitialized );

	if( !m_uNumAlive ) {
		return;
	}

	// Get camera, pass it to _FDraw functions.
	CFCamera *pCam = gamecam_GetActiveCamera();

	frenderer_Push( FRENDERER_DRAW, NULL );

	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER );
	fdraw_Depth_EnableWriting( FALSE );

	fdraw_SetTexture(&s_oTexInst);

	u32 uPoolIdx;
	CEMPLightningBoltGroup *pCurGroup;
	CEMPLightningBolt *pCurBolt;
	const CFXfm *pFXfm = pCam->GetFinalXfm();

	for( uPoolIdx = 0; uPoolIdx < _EMP_MAX_BLASTS; ++uPoolIdx ) {
		pCurGroup = &m_aBoltGroupPool[uPoolIdx];

		// Draw all of the bolts for the current group
		if( pCurGroup->m_bAlive ) {
			pCurBolt = (CEMPLightningBolt *) flinklist_GetHead( &pCurGroup->m_ActiveBolts );

			while( pCurBolt ) {
				pCurBolt->FDraw( pFXfm, pCurGroup->m_PosWS );
				pCurBolt = (CEMPLightningBolt *) flinklist_GetNext( &pCurGroup->m_ActiveBolts, pCurBolt );
			}
		}
	}

	frenderer_Pop();
}

void CEMPLightningBoltGroup::Release( CEMPLightningBoltGroup *pBoltGroup ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( pBoltGroup );
	FASSERT( m_uNumAlive );

	--m_uNumAlive;
	pBoltGroup->m_bAlive = FALSE;
}

CEMPLightningBoltGroup *CEMPLightningBoltGroup::GetLightning( void ) {
	FASSERT( m_bSystemInitialized );

	CEMPLightningBoltGroup *pCurLBGroup;
	u32 uCurIdx;

	for( uCurIdx = 0; uCurIdx < _EMP_MAX_BLASTS; ++uCurIdx ) {
		pCurLBGroup = &m_aBoltGroupPool[uCurIdx];

		if( !pCurLBGroup->m_bAlive ) {
			break;
		}
	}

	if( uCurIdx == _EMP_MAX_BLASTS ) {
		DEVPRINTF( "CEMPLightningBoltGroup::GetLightning() : Could not acquire, pool exhausted.\n" );
		return NULL;
	}

	pCurLBGroup->m_bAlive = TRUE;
	pCurLBGroup->m_bNoSpawn = FALSE;

	++m_uNumAlive;

	return pCurLBGroup;
}

BOOL CEMPLightningBoltGroup::Init( u32 uNumBolts, const CFVec3A &PosWS, const f32 &fLife, const f32 &fRadius ) {
	FASSERT( uNumBolts <= _LIGHTB_MAX_LIGHT_BOLTS );

	u32 uCurBoltIdx;

	flinklist_InitRoot( &m_FreeBolts, (s32)FANG_OFFSETOF( CEMPLightningBolt, m_Link ) );
	flinklist_InitRoot( &m_ActiveBolts, (s32)FANG_OFFSETOF( CEMPLightningBolt, m_Link ) );

	m_PosWS = PosWS;
	m_fRadius = fRadius;

	// Fill the free list
	for( uCurBoltIdx = 0; uCurBoltIdx < uNumBolts; ++uCurBoltIdx ) {
		m_aLB[uCurBoltIdx].Clear();

		flinklist_AddTail( &m_FreeBolts, &m_aLB[uCurBoltIdx] );
	}

	// So we spawn right away
	m_fTimeTillNextSpawn = 0.0f;

	return TRUE;
}

void CEMPLightningBoltGroup::_Clear( void ) {
	m_bAlive = FALSE;
	m_bNoSpawn = FALSE;

	m_fRadius = 0.0f;

	m_PosWS.Set( CFVec3A::m_Null );
}

void CEMPLightningBoltGroup::_Work( void ) {
	CEMPLightningBolt *pCurBolt, *pNextBolt;

	pCurBolt = (CEMPLightningBolt *) flinklist_GetHead( &m_ActiveBolts );

	while( pCurBolt ) {
		pNextBolt = (CEMPLightningBolt *) flinklist_GetNext( &m_ActiveBolts, pCurBolt );

		pCurBolt->Work();

		if( pCurBolt->GetAlpha() == 0.0f ) {
			flinklist_Remove( &m_ActiveBolts, pCurBolt );
			flinklist_AddTail( &m_FreeBolts, pCurBolt );
		}

		pCurBolt = pNextBolt;
	}

	m_fTimeTillNextSpawn -= FLoop_fPreviousLoopSecs;

	if( m_fTimeTillNextSpawn < 0.0f && !m_bNoSpawn ) {
		m_fTimeTillNextSpawn = fmath_RandomFloatRange( _LIGHTB_TIME_SPAWN_MIN, _LIGHTB_TIME_SPAWN_MAX );

		if( m_FreeBolts.nCount ) {
			u32 uNumSpawn = fmath_RandomRange( _LIGHTB_NUM_SPAWN_MIN, _LIGHTB_NUM_SPAWN_MAX );
			f32 fRadScale;
			f32 fLife;

			while( uNumSpawn && m_FreeBolts.nCount ) {
				pCurBolt = (CEMPLightningBolt *) flinklist_RemoveHead( &m_FreeBolts );
				flinklist_AddTail( &m_ActiveBolts, pCurBolt );

				fRadScale = fmath_RandomFloatRange( _LIGHTB_RADIUS_SCALE_MIN, _LIGHTB_RADIUS_SCALE_MAX );
				fLife = fmath_RandomFloatRange( _LIGHTB_LIFE_MIN, _LIGHTB_LIFE_MAX );

				pCurBolt->Clear();
				pCurBolt->Init( fLife, m_fRadius * fRadScale );

				--uNumSpawn;
			}
		}
	}

	// Don't do this since this will put us back into the free pool of bolt groups
	// We want to work until we are told to die, even if we have nothing spawned
	/*if( m_ActiveBolts.nCount == 0 ) {
		m_bAlive = FALSE;
	}*/
}