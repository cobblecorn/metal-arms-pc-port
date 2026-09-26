//////////////////////////////////////////////////////////////////////////////////////
// fxslower.cpp
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
#include "fxslower.h"
#include "fvtxpool.h"
#include "Bot.h"
#include "MeshTypes.h"
//
// SLOWER stuff
//
const CFXSlower::_UserProps_t* CFXSlower::m_pProps=NULL;
BOOL CFXSlower::m_bSystemInitialized = FALSE;
CFXSlower *CFXSlower::m_pBlasts = 0;
BOOL CFXSlower::m_bActive = FALSE;
FParticle_DefHandle_t CFXSlower::m_hTrailParticleDef = FPARTICLE_INVALID_HANDLE;
FParticle_DefHandle_t CFXSlower::m_hBurstParticleDef = FPARTICLE_INVALID_HANDLE;
FExplosion_GroupHandle_t CFXSlower::m_hExplosionGroup = FEXPLOSION_INVALID_HANDLE;
CFMeshInst *CFXSlower::m_pMeshInst = NULL;
f32 CFXSlower::m_fOOMeshRadiusOuter = 0.0f;
f32	CFXSlower::m_fUnitSlowedDown=0.f;							// How much does it slow'em down
f32	CFXSlower::m_fFullEffectTime=0.f;							// What's the total effect's residual time
f32	CFXSlower::m_fRecoveryTime=0.f;							// How long to recover
//
// CFXSlower
//
CFXSlower::CFXSlower() {
	m_bAlive = FALSE;
	m_fLife = 0.0f;
	m_fMaxRadius = 0.0f;
//	m_fOOMaxRadius = 0.0f;
}

BOOL CFXSlower::InitSystem( const _UserProps_t* pProps ) {
//	DEVPRINTF( "*** %d CFXSlower\n", sizeof( CFXSlower ) );

	FResFrame_t Frame;
	FMesh_t *pMesh = NULL;			//CPS 4.7.03

	Frame = fres_GetFrame();

	m_pProps = pProps;
	m_bSystemInitialized = TRUE;

	m_pBlasts = fnew CFXSlower[m_pProps->uMaxBlasts];

	if( m_pBlasts == NULL ) {
		DEVPRINTF( "CFXSlower::InitSystem(): Could not allocate blast array\n" );
		goto _ExitInitSystemWithError;
	}

	// Init them
	for( u32 i = 0; i < m_pProps->uMaxBlasts; ++i ) 
	{
		m_pBlasts[i].m_bAlive = FALSE;
		m_pBlasts[i].m_apBlastSpheres = fnew BlastSphere_t[m_pProps->uMaxSpheres];
		if( m_pBlasts[i].m_apBlastSpheres == NULL ) 
		{
			DEVPRINTF( "CFXSlower::InitSystem(): Could not allocate blast array\n" );
			goto _ExitInitSystemWithError;
		}

		m_pBlasts[i].m_apSparklers = fnew BlastSparkler_t[m_pProps->uMaxSparklers];
		if(m_pBlasts[i].m_apSparklers == NULL ) 
		{
			DEVPRINTF( "CFXSlower::InitSystem(): Could not allocate blast array\n" );
			goto _ExitInitSystemWithError;
		}
	}


	// Particle effects
	m_hTrailParticleDef = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, m_pProps->pszPartTrailName );

	if( m_hTrailParticleDef == FPARTICLE_INVALID_HANDLE ) {
		DEVPRINTF( "CFXSlower::InitSystem(): Could not find particle definition '%s'.\n",  m_pProps->pszPartTrailName );
	}

	m_hBurstParticleDef = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, m_pProps->pszPartBlastName);

	if( m_hBurstParticleDef == FPARTICLE_INVALID_HANDLE ) {
		DEVPRINTF( "CFXSlower::InitSystem(): Could not find particle definition '%s'.\n", m_pProps->pszPartBlastName );
	}
	
	m_hExplosionGroup = CExplosion2::GetExplosionGroup( m_pProps->pszDetonateBlastName );
	if( m_hExplosionGroup == FEXPLOSION_INVALID_HANDLE ) 
	{
		DEVPRINTF( "CFXSlower::InitSystem(): Could not find particle definition '%s'.\n", m_pProps->pszDetonateBlastName );
	}
	

//CPS 4.7.03	FMesh_t *pMesh = NULL;
	pMesh = NULL;			//CPS 4.7.03

	// Outer sphere mesh
	pMesh = (FMesh_t*) fresload_Load( FMESH_RESTYPE, m_pProps->pszBlastSphere );
	if( pMesh == NULL ) {
		DEVPRINTF( "CFXSlower::InitSystem():  Unable to load mesh %s\n", m_pProps->pszBlastSphere );
		goto _ExitInitSystemWithError;
	}

	m_pMeshInst = fnew CFMeshInst;

	if( m_pMeshInst == NULL ) {
		DEVPRINTF( "CFXSlower::Create(): Not enough memory to create CFMeshInst.\n" );
		goto _ExitInitSystemWithError;
	}

	m_pMeshInst->Init( pMesh );
//	FMATH_SETBITMASK( m_pMeshInst->m_nFlags, FMESHINST_FLAG_POSTER_X|FMESHINST_FLAG_POSTER_Y|FMESHINST_FLAG_POSTER_Z); 

	if( m_pMeshInst->m_pMesh->BoundSphere_MS.m_fRadius > 0.0f ) {
		m_fOOMeshRadiusOuter = fmath_Inv( m_pMeshInst->m_pMesh->BoundSphere_MS.m_fRadius * m_pProps->fSphereScale );
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

void CFXSlower::UninitSystem( void ) {
	if( !m_bSystemInitialized ) {
		return;
	}

	m_bSystemInitialized = FALSE;

	fdelete_array( m_pBlasts );
	m_pBlasts = NULL;

	fdelete( m_pMeshInst );
	m_pMeshInst = NULL;

	if( fworld_IsWorldCallbackFunctionRegistered( _WorldLoadAndUnloadCallback ) ) {
		fworld_UnregisterWorldCallbackFunction( _WorldLoadAndUnloadCallback );
	}
}
void CFXSlower::KillAll( void ) {
	if (!m_bSystemInitialized)
		return;

	m_bActive = FALSE;

	for( u32 i = 0; i < m_pProps->uMaxBlasts; ++i ) {
		if( m_pBlasts[i].m_bAlive ) {
			m_pBlasts[i]._Kill();
		}
	}
}


void CFXSlower::Work( void ) {
	if (!m_bSystemInitialized)
		return;

	if( !m_bActive ) {
		return;
	}

	m_bActive = FALSE;

	for( u32 i = 0; i < m_pProps->uMaxBlasts; ++i ) {
		if( m_pBlasts[i].m_bAlive ) {
			m_pBlasts[i]._Work();
			m_bActive = TRUE;
		}
	}
}

void CFXSlower::Draw( void ) {
	if (!m_bSystemInitialized )
		return;

	if( !m_bActive ) {
		return;
	}

	CFMtx43A Mtx;
//	CFCamera *pCam = gamecam_GetActiveCamera();
	f32 fScale;

//	FASSERT( pCam );

	frenderer_Push( FRENDERER_MESH, NULL );
	fmesh_Ambient_Set( 1.0f, 1.0f, 1.0f, 1.0f );

	for( u32 i = 0; i < m_pProps->uMaxBlasts; ++i ) {
		if( m_pBlasts[i].m_bAlive ) {
			for( u32 j = 0; j < m_pProps->uMaxSpheres; ++j ) {
				if( m_pBlasts[i].m_apBlastSpheres[j].bActive ) {
					CFMtx43A::m_RotZ.Identity();
					CFMtx43A::m_RotZ.RotateZ( m_pBlasts[i].m_apBlastSpheres[j].fRotateZ );

					Mtx.m_vUp.Set(m_pBlasts[i].m_PosWS.m_vUp);
					Mtx.m_vFront.Set(m_pBlasts[i].m_PosWS.m_vFront);
					Mtx.m_vRight.Set(m_pBlasts[i].m_PosWS.m_vRight);
				//	Mtx.Mul( CFMtx43A::m_RotZ );
					Mtx.m_vPos.Set(m_pBlasts[i].m_PosWS.m_vPos);

					fScale = m_pBlasts[i].m_apBlastSpheres[j].fCurrentRadius * m_fOOMeshRadiusOuter;

					m_pMeshInst->m_Xfm.BuildFromMtx( Mtx, fScale );
					m_pMeshInst->SetMeshAlpha( m_pBlasts[i].m_apBlastSpheres[j].fAlpha );
					if (m_pBlasts[i].m_bPosterXYZ)
					{
						FMATH_SETBITMASK( m_pMeshInst->m_nFlags, FMESHINST_FLAG_POSTER_X|FMESHINST_FLAG_POSTER_Y|FMESHINST_FLAG_POSTER_Z); 
					}
					else
					{
						FMATH_CLEARBITMASK( m_pMeshInst->m_nFlags, FMESHINST_FLAG_POSTER_X|FMESHINST_FLAG_POSTER_Y|FMESHINST_FLAG_POSTER_Z); 
					}
					m_pMeshInst->Draw( FVIEWPORT_PLANESMASK_ALL );
				}
			}
		}
	}

	frenderer_Pop();
}

void CFXSlower::SpawnSlowerEffect( CBot* pLaunchBot, const CFMtx43A &PosWS, CFVec3A* pFollowPosWS, BOOL bPosterXYZ )
{
	FASSERT( m_bSystemInitialized );
	FASSERT( m_pProps->fMaxRadius > 0.0f );
	FASSERT( m_pProps->fLife > 0.0f );
	
	m_bActive = TRUE;

	CFXSlower *pBlast = _GetNewBlast();

	pBlast->m_bAlive = TRUE;
	pBlast->m_bPosterXYZ = bPosterXYZ;
	pBlast->m_PosWS = PosWS;
	pBlast->m_fLifeMax = m_pProps->fLife;
	pBlast->m_fLife = 0.0f;
	pBlast->m_fMaxRadius = m_pProps->fMaxRadius;
	pBlast->m_fUnitSlowedDown = m_pProps->fUnitSlowedDown;				// How much does it slow'em down
	pBlast->m_fFullEffectTime = m_pProps->fFullEffectTime;				// What's the total effect's residual time
	pBlast->m_fRecoveryTime = m_pProps->fRecoveryTime;					// How long to recover

	pBlast->m_pAttachPosWS = pFollowPosWS;
	pBlast->m_pLaunchBot =  pLaunchBot;
	u32 i;

	// _SpawnBlastSphere() creates wobblers, we don't want that for the first spheres
	for( i = 0; i < m_pProps->uMaxSpheres; ++i ) {
		pBlast->m_apBlastSpheres[i].bActive = TRUE;
		pBlast->m_apBlastSpheres[i].bWobble = TRUE;
		pBlast->m_apBlastSpheres[i].fGrowRate = fmath_RandomFloatRange( m_pProps->fGrowRateMin * 3.0f, m_pProps->fGrowRateMax * 3.0f );
		pBlast->m_apBlastSpheres[i].fWobbleDir = 1.0f;
		pBlast->m_apBlastSpheres[i].fMaxRadius = m_pProps->fMaxRadius;
		pBlast->m_apBlastSpheres[i].fOOMaxRadius = fmath_Inv( pBlast->m_apBlastSpheres[i].fMaxRadius );
		pBlast->m_apBlastSpheres[i].fCurrentRadius = 1.0f;
		pBlast->m_apBlastSpheres[i].fMinWobbleRadius = 0.1f;
		pBlast->m_apBlastSpheres[i].fAlpha = 1.0f;
		pBlast->m_apBlastSpheres[i].fRotateZ = fmath_RandomFloatRange( 0.0f, FMATH_2PI );
	}

	for( i = 0; i < m_pProps->uMaxSparklers; ++i ) {
		pBlast->_SpawnSparkler( i );
	}
 /*
	// Add a light
	FLightInit_t LightInit;

	LightInit.szPerPixelTexName[0] = 0;
	LightInit.szCoronaTexName[0] = 0;
	LightInit.nLightID = 0xffff;
	LightInit.mtxOrientation.Identity();
	LightInit.fIntensity = 1.0f;
	LightInit.Motif.Set( 0.0f, 0.0f, 1.0f, 1.0f );
	LightInit.Motif.SetIndex( FCOLORMOTIF_PULSATEHI0 );
	LightInit.nFlags = FLIGHT_FLAG_HASPOS | FLIGHT_FLAG_PER_PIXEL;
	LightInit.nType = FLIGHT_TYPE_OMNI;
	LightInit.spInfluence.m_Pos = PosWS.m_vPos.v3;
	LightInit.spInfluence.m_fRadius = m_pProps->fMaxRadius;
	LightInit.szName[0] = 0;
	pBlast->m_WorldLight.Init( &LightInit );
*/
}

void CFXSlower::_Work( void ) {
	m_fLife += FLoop_fPreviousLoopSecs;

	if (m_pAttachPosWS)
	{
		m_PosWS.m_vPos.Set(*m_pAttachPosWS);
	}
    
	if( m_fLife >= m_fLifeMax ) 
	{
		_Kill();
		return;
	}

 	_WorkSparklers();

	f32 fLifeLeft = ( m_fLifeMax - m_fLife );

	for( u32 i = 0; i < m_pProps->uMaxSpheres; ++i ) {
		if( m_apBlastSpheres[i].bActive ) {
			 m_apBlastSpheres[i].fCurrentRadius +=
				( FLoop_fPreviousLoopSecs *  m_apBlastSpheres[i].fGrowRate *  m_apBlastSpheres[i].fWobbleDir );

			if(  m_apBlastSpheres[i].fCurrentRadius > m_apBlastSpheres[i].fMaxRadius ) {
				if( !m_apBlastSpheres[i].bWobble ) {
					//_SpawnBlastSphere( i, TRUE );
					continue;
				} else {
					m_apBlastSpheres[i].fWobbleDir = -1.0f;
					m_apBlastSpheres[i].fCurrentRadius = m_apBlastSpheres[i].fMaxRadius;
					m_apBlastSpheres[i].fGrowRate = m_pProps->fGrowRateWobble;
				}
			} else if(  m_apBlastSpheres[i].bWobble &&  m_apBlastSpheres[i].fCurrentRadius <  m_apBlastSpheres[i].fMinWobbleRadius ) {
				m_apBlastSpheres[i].fWobbleDir = 1.0f;
				m_apBlastSpheres[i].fCurrentRadius =  m_apBlastSpheres[i].fMinWobbleRadius;
			}

			m_apBlastSpheres[i].fAlpha = 1.0f - ( m_apBlastSpheres[i].fCurrentRadius * m_apBlastSpheres[i].fOOMaxRadius );

			// So we can fade out at the end of our life
			if( fLifeLeft <= m_pProps->fFadeOutTime ) {
				m_apBlastSpheres[i].bWobble = TRUE;
				m_apBlastSpheres[i].fWobbleDir = 1.0f;
			}
		}
	}

	FWorld_nTrackerSkipListCount = 0;
	if (m_pLaunchBot)
	{
		m_pLaunchBot->AppendTrackerSkipList();
	}

	CFSphere Sphere;
	Sphere.m_Pos	 = m_PosWS.m_vPos.v3;
	Sphere.m_fRadius = m_fMaxRadius;
    fworld_FindTrackersIntersectingSphere( &Sphere, 
											FWORLD_TRACKERTYPE_MESH, 
											_SlowerTrackerCallback, 
											FWorld_nTrackerSkipListCount,
											FWorld_apTrackerSkipList,
											NULL, 
											MESHTYPES_ENTITY, 
											ENTITY_BIT_BOT );
}

void CFXSlower::_Kill( void ) {
//	m_WorldLight.RemoveFromWorld();

	_KillParticles();

	m_bAlive = FALSE;
}

void CFXSlower::_SpawnBlastSphere( const u32 &uIndex, BOOL bWobble ) {
	m_apBlastSpheres[uIndex].bActive = TRUE;
	m_apBlastSpheres[uIndex].bWobble = bWobble;
	m_apBlastSpheres[uIndex].fGrowRate = fmath_RandomFloatRange( m_pProps->fPartBurstRateMin, m_pProps->fPartBurstRateMax );
	m_apBlastSpheres[uIndex].fWobbleDir = 1.0f;
	m_apBlastSpheres[uIndex].fCurrentRadius = 1.0f;
	m_apBlastSpheres[uIndex].fMaxRadius = m_fMaxRadius;
	m_apBlastSpheres[uIndex].fOOMaxRadius = fmath_Inv( m_fMaxRadius );
	m_apBlastSpheres[uIndex].fMinWobbleRadius = m_fMaxRadius * fmath_RandomFloatRange( 0.8f, 0.9f );
	m_apBlastSpheres[uIndex].fAlpha = 1.0f;
	m_apBlastSpheres[uIndex].fRotateZ = fmath_RandomFloatRange( 0.0f, FMATH_2PI );
}

void CFXSlower::_WorkSparklers( void ) {
	CFQuatA QuatX, QuatY, QuatZ;

	for( u32 i = 0; i < m_pProps->uMaxSparklers; ++i ) {
		if( m_apSparklers[i].bAlive ) {
			m_apSparklers[i].fLife -= FLoop_fPreviousLoopSecs;

			if( m_apSparklers[i].fLife < 0.0f ) {
				m_apSparklers[i].bAlive = FALSE;

				// stop emitting particles
				if( m_apSparklers[i].hTrailEmitter != FPARTICLE_INVALID_HANDLE ) {
					fparticle_StopEmitter( m_apSparklers[i].hTrailEmitter );
					m_apSparklers[i].hTrailEmitter = FPARTICLE_INVALID_HANDLE;
				}

				// Boom
				fparticle_SpawnEmitter( m_hBurstParticleDef, m_apSparklers[i].Position.v3, NULL, 1.0f );
			}

			m_apSparklers[i].fCurrentRadius += ( FLoop_fPreviousLoopSecs * m_pProps->fPartRadiusGrow );

			if( m_apSparklers[i].fCurrentRadius > m_apSparklers[i].fMaxRadius ) {
				m_apSparklers[i].fCurrentRadius = m_apSparklers[i].fMaxRadius;
			}

			m_apSparklers[i].Rotate.x += ( FLoop_fPreviousLoopSecs * m_apSparklers[i].RotateVel.x );
			m_apSparklers[i].Rotate.y += ( FLoop_fPreviousLoopSecs * m_apSparklers[i].RotateVel.y );
			m_apSparklers[i].Rotate.z += ( FLoop_fPreviousLoopSecs * m_apSparklers[i].RotateVel.z );

			QuatX.BuildQuatRotX( m_apSparklers[i].Rotate.x );
			QuatY.BuildQuatRotY( m_apSparklers[i].Rotate.y );
			QuatZ.BuildQuatRotZ( m_apSparklers[i].Rotate.z );
			QuatX.Mul( QuatY );
			QuatX.Mul( QuatZ );
			QuatX.MulPoint( m_apSparklers[i].Position, CFVec3A::m_UnitAxisX );

			m_apSparklers[i].Position.Mul( m_apSparklers[i].fCurrentRadius );
			m_apSparklers[i].Position.Add( m_PosWS.m_vPos );
		}
	}
}

void CFXSlower::_KillParticles( void ) {
	for( u32 i = 0; i < m_pProps->uMaxSparklers; ++i ) {
		if( m_apSparklers[i].bAlive ) {
			m_apSparklers[i].bAlive = FALSE;

			// stop emitting particles
			if( m_apSparklers[i].hTrailEmitter != FPARTICLE_INVALID_HANDLE ) {
				fparticle_StopEmitter( m_apSparklers[i].hTrailEmitter );
				m_apSparklers[i].hTrailEmitter = FPARTICLE_INVALID_HANDLE;
			}

			//fparticle_SpawnEmitter( m_hBurstParticleDef, m_apSparklers[i].Position.v3, NULL, 1.0f );
		}
	}
}

void CFXSlower::_SpawnSparkler( u32 uNdx ) {
	m_apSparklers[uNdx].bAlive = TRUE;
	m_apSparklers[uNdx].fLife = m_fLifeMax * fmath_RandomFloatRange( 0.6f, 1.0f );
	m_apSparklers[uNdx].fMaxRadius = m_fMaxRadius * fmath_RandomFloatRange( 0.3f, 1.0f );
	m_apSparklers[uNdx].fCurrentRadius = 0.1f;
	m_apSparklers[uNdx].Position.Set(m_PosWS.m_vPos);
	m_apSparklers[uNdx].hTrailEmitter = fparticle_SpawnEmitter( m_hTrailParticleDef, &m_apSparklers[uNdx].Position.v3, NULL, NULL, 1.0f ); 
	m_apSparklers[uNdx].Rotate.Set( 0.f,0.f,0.f );

	m_apSparklers[uNdx].RotateVel.x = m_pProps->fPartBaseVel * fmath_RandomFloatRange( m_pProps->fPartBaseVelMin, m_pProps->fPartBaseVelMax );
	m_apSparklers[uNdx].RotateVel.y = m_pProps->fPartBaseVel * fmath_RandomFloatRange( m_pProps->fPartBaseVelMin, m_pProps->fPartBaseVelMax );
	m_apSparklers[uNdx].RotateVel.z = m_pProps->fPartBaseVel * fmath_RandomFloatRange( m_pProps->fPartBaseVelMin, m_pProps->fPartBaseVelMax );

	if( fmath_RandomChoice( 2 ) ) {
		m_apSparklers[uNdx].RotateVel.x = -m_apSparklers[uNdx].RotateVel.x;
	}

	if( fmath_RandomChoice( 2 ) ) {
		m_apSparklers[uNdx].RotateVel.y = -m_apSparklers[uNdx].RotateVel.y;
	}

	if( fmath_RandomChoice( 2 ) ) {
		m_apSparklers[uNdx].RotateVel.z = -m_apSparklers[uNdx].RotateVel.z;
	}

	m_apSparklers[uNdx].Rotate.y = fmath_RandomFloatRange( -FMATH_PI, FMATH_PI );
}

CFXSlower *CFXSlower::_GetNewBlast( void ) {
	u32 uBest = -1;
	f32 fLowLife = FMATH_MAX_FLOAT;

	for( u32 i = 0; i < m_pProps->uMaxBlasts; ++i ) {
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

BOOL CFXSlower::_WorldLoadAndUnloadCallback( FWorldEvent_e eEvent ) 
{
	if( FWORLD_EVENT_WORLD_PREDESTROY == eEvent ) 
	{
		KillAll();
	}

	return TRUE;
}


BOOL CFXSlower::_SlowerTrackerCallback( CFWorldTracker *pTracker, FVisVolume_t *pWorldLeafNode )
{
	// exclude skiplisted guys
	for(u32 i=0; i<FWorld_nTrackerSkipListCount; i++ ) 
	{
		if( pTracker == FWorld_apTrackerSkipList[i] ) 
		{
			return TRUE;
		}
	}

	CBot *pBot = (CBot *) pTracker->m_pUser;
	
	if( !pBot->Power_IsPoweredUp() ) 
	{
		return TRUE;
	}

	if( pBot->TypeBits() & ENTITY_BIT_VEHICLE ) 
	{
		return TRUE;
	}

	if( pBot->TypeBits() & ENTITY_BIT_WEAPON ) 
	{
		return TRUE;
	}
	
	//pBot->SlowBot(.80f,1.5f,.5f);
	pBot->SlowBot(m_fUnitSlowedDown,m_fFullEffectTime,m_fRecoveryTime);
	
	return TRUE;
}

void CFXSlower::SpawnSlowerBurst( FExplosionSpawnParams_t& rParams )
{
	// we'll probably need this handle later, save it off?	
	FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();

	if( hSpawner != FEXPLOSION_INVALID_HANDLE ) 
		{

            CExplosion2::SpawnExplosion( hSpawner, m_hExplosionGroup, &rParams );
		}
}