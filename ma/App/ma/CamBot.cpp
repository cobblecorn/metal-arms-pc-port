//////////////////////////////////////////////////////////////////////////////////////
// CamBot.cpp - 
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
// 01/31/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "CamBot.h"
#include "fdraw.h"
#include "floop.h"
#include "ftext.h"
#include "fworld.h"
#include "fworld_coll.h"
#include "frenderer.h"
#include "level.h"
#include "bot.h"
#include "MeshTypes.h"
#include "SplitScreen.h"
#include "MultiplayerMgr.h"
#if FANG_WINGC
#include "pc_display.h"
#endif

//====================
// private definitions

/////////////////////////////////
// used to test camera collisions
#define _SLOWEST_NON_COLL_ZOOM_FACTOR	0.04f
#define _SLOWEST_COLL_ZOOM_FACTOR		0.11f//0.125f
#define _FASTEST_COLL_ZOOM_FACTOR		0.09f//0.165f

#define _USE_PROJ_SPHERE_TEST			TRUE

#if _USE_PROJ_SPHERE_TEST
	#define _SPHERE_D					1.4f
#else
	#define _SPHERE_D					2.85f
#endif
#define _SPHERE_R							( _SPHERE_D / 2.0f )
#define _SPHERE_OO_R						( 1.0f / _SPHERE_R )
#define _SPHERE_MIN_DIST_BETWEEN_SPHERES	( _SPHERE_R / 3.0f )
///////////////////////////////////////////////////////

//=================
// public variables

//==================
// private variables

CFSphere CCamBot::m_aSphereList[_MAX_COLLISION_SPHERES];
CFSphere CCamBot::m_MasterBoundingSphere;
u32 CCamBot::m_nNumSpheresUsed;

// used during debugging
f32 CCamBot::m_afHistoryBuffer[_HISTORY_BUFFER_SIZE];
u32 CCamBot::m_anHistoryIndex;

//===================
// private prototypes

//=================
// public functions

//==================
// private functions

CCamBot::CCamBot() : CFCameraMan() {
	Reset();
}

CCamBot::~CCamBot() {
}

BOOL CCamBot::InitSystem() {
	u32 i;

	for( i=0; i < _MAX_COLLISION_SPHERES; i++ ) {
		m_aSphereList[i].Zero();
	}
	m_MasterBoundingSphere.Zero();
	m_nNumSpheresUsed = 0;

	for( i=0; i < _HISTORY_BUFFER_SIZE; i++ ) {
		m_afHistoryBuffer[i] = 0.0f;
	}
	m_anHistoryIndex = 0;

	return TRUE;
}

void CCamBot::UninitSystem() {
}

BOOL CCamBot::Init( CBot *pBot, const CamBotInfo_t *pInfo ) {

	// do some parameter error checking
	if( !m_pCamera ) {
		DEVPRINTF( "CCamBot::Init() : No camera assigned, call AssignCamera() before Init().\n" );
		return FALSE;
	}
	if( !pBot ) {
		DEVPRINTF( "CCamBot::Init() : Bot cameras require a CBot to init.\n" );
		return FALSE;
	}
	if( !pInfo ) {
		DEVPRINTF( "CCamBot::Init() : Bot cameras require a CamBotInfo_t to init.\n" );
		return FALSE;
	}	
	// setup our starting mode
	Reset();
	m_pBot = pBot;
	m_pCamInfo = pInfo;

	// If this is multiplayer, we modify our position and fov
	if ( MultiplayerMgr.IsMultiplayer() ) {
		m_DesiredLookAtToPos_MS.v3 = (pInfo->Pos_MS * pInfo->fPosScale_MP) - pInfo->TargetPos_MS;
#if !FANG_WINGC
		m_pCamera->SetFOV( splitscreen_RemapFOV( m_pCamera->GetID(), pInfo->fHalfFOV_MP ) );
#endif
	}
	else {
		m_DesiredLookAtToPos_MS.v3 = pInfo->Pos_MS - pInfo->TargetPos_MS;
#if !FANG_WINGC
		m_pCamera->SetFOV( splitscreen_RemapFOV( m_pCamera->GetID(), pInfo->fHalfFOV ) );
#endif
	}
#if FANG_WINGC
	_PortApplyFov();
#endif

	// see if this level has specified near/far planes in the data file
	if( Level_hLevelDataFile != FGAMEDATA_INVALID_FILE_HANDLE ) {
		f32 fNearZ, fFarZ, *pfNewValue;
		FGameDataTableHandle_t hTable;
		FGameData_VarType_e nDataType;
		BOOL bAdjustClipPlanes = FALSE;

		// grab the default clip planes
		m_pCamera->GetClipPlanes( &fNearZ, &fFarZ );

		// see if we have a near plane table 
		hTable = fgamedata_GetFirstTableHandle( Level_hLevelDataFile, "near_plane" );
		if( hTable != FGAMEDATA_INVALID_TABLE_HANDLE ) {
			pfNewValue = (f32 *)fgamedata_GetPtrToFieldData( hTable, 0, nDataType );
			if( nDataType == FGAMEDATA_VAR_TYPE_FLOAT ) {
				fNearZ = *pfNewValue;
				bAdjustClipPlanes = TRUE;
			}											    
		}

		// see if we have a far plane table
		hTable = fgamedata_GetFirstTableHandle( Level_hLevelDataFile, "far_plane" );
		if( hTable != FGAMEDATA_INVALID_TABLE_HANDLE ) {
			pfNewValue = (f32 *)fgamedata_GetPtrToFieldData( hTable, 0, nDataType );
			if( nDataType == FGAMEDATA_VAR_TYPE_FLOAT ) {
				fFarZ = *pfNewValue;
				bAdjustClipPlanes = TRUE;
			}											    
		}

		if( bAdjustClipPlanes ) {
			// set our clip planes
			m_pCamera->SetClipPlanes( fNearZ, fFarZ );
		}
	}	

	// default our camera to our starting 3rd person position (new cam version, no collision)
	Work_3rdPerson( FALSE );

	return TRUE;
}

void CCamBot::Reset() {
	m_nMode = CAM_BOT_3RD_PERSON;
	m_pBot = NULL;
	m_pCamInfo = NULL;
	m_bFreezeShot = FALSE;
	m_LookAtPoint_WS.Zero();
	m_MinCamPos_WS.Zero();
	m_UnitLookDir_WS.Zero();
	m_fLastUnitDist = 1.0f;
	m_DesiredPos_WS.Zero();
	m_fLastCenterRayUnitDist = 1.0f;
	m_fLastDeltaZoom = 0.0f;
#if FANG_WINGC
	m_nPortFovVersion = 0;
	m_fPortHalfFov = 0.0f;
#endif
	m_fPitch = 0.0f;
	m_DesiredLookAtToPos_MS.Zero();

	m_fOverrideUnitTime		= 0.0f;
	m_bOverrideEnabled		= FALSE;
	m_fOOOverrideTime		= 0.0f;
	m_bAlwaysOverride		= FALSE;
}

// The bot layer must have updated the following variables, before this function gets called:
// m_bCamAdjEnabled
// m_bJumping
// m_fMountPitch_WS
// m_fMountPitchMin_WS
// m_fClampedNormSpeedXZ_WS
// m_fCamZoomFactor
// m_fMountYaw_WS
// m_fCamTransTime
// m_vecCamPosAdj
// m_vecCamLookAdj
// m_MountUnitFrontXZ_WS
// MtxToWorld
// m_VelocityXZ_WS
// m_Velocity_WS
// AppendTrackerSkipList
#if FANG_WINGC
// The retail FOV plus the player's Field of View setting (pc_display).
void CCamBot::_PortApplyFov( void ) {
	const f32 fRetail = MultiplayerMgr.IsMultiplayer() ? m_pCamInfo->fHalfFOV_MP : m_pCamInfo->fHalfFOV;
	m_fPortHalfFov = splitscreen_RemapFOV( m_pCamera->GetID(), pcdisplay_AdjustHalfFov( fRetail ) );
	m_nPortFovVersion = pcdisplay_FovVersion();
	m_pCamera->SetFOV( m_fPortHalfFov );
}
#endif

void CCamBot::Work() {
	
	if( !m_pBot || !m_pCamInfo ) {
		return;
	}

#if FANG_WINGC
	// The setting changed (in the pause menu): re-apply it, unless something else (a scope's zoom,
	// a console or tether view) has the camera's FOV just now.
	if( m_nPortFovVersion != pcdisplay_FovVersion() ) {
		f32 fCurrent;
		m_pCamera->GetFOV( &fCurrent );
		if( FMATH_FABS( fCurrent - m_fPortHalfFov ) < 0.0001f ) {
			_PortApplyFov();
		}
	}
#endif

	if( m_bFreezeShot ) {
		// don't do anything...we are frozen
		return;
	}

	// do this frame's camera logic
	switch( m_nMode ) {

	default:
	case CAM_BOT_3RD_PERSON:
		Work_3rdPerson( TRUE );
		break;

	case CAM_BOT_1ST_PERSON:
		// TBD...
		break;
	}
}

static CFCollInfo _CameraCollInfo;
static BOOL CamCollisionTrackerCallback( CFWorldTracker *pTracker, FVisVolume_t *pWorldLeafNode ) {
	FASSERT( pTracker );

	u32 i;	
	u32 nImpactCount = FColl_nImpactCount;
	
	// Make sure the tracker is not in the skip list
	for ( i = 0; i < FWorld_nTrackerSkipListCount; i++ )
	{
		if ( pTracker == FWorld_apTrackerSkipList[i] )
		{
			// Tracker is in the skip list, so go on to the next one
			return TRUE;
		}
	}

	// NKM - If we collide with a Probe bot, don't change anything
	if( pTracker->m_nUser == MESHTYPES_ENTITY ) {
		if( ((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_BOTPROBE ) {
			return TRUE;
		}

		if( !((CEntity*)pTracker->m_pUser)->IsCameraCollisionEnabled() ) {
			return TRUE;
		}
	}

	((CFWorldMesh *)pTracker)->CollideWithMeshTris( &_CameraCollInfo );
	
	if ( nImpactCount == FColl_nImpactCount ) {
		return TRUE;
	}
	
	f32 fWorstDistance = _CameraCollInfo.ProjSphere.m_fMoveDistance;
	for ( i = 0; i < FColl_nImpactCount; i++ ) {
		if ( FColl_aImpactBuf[i].fImpactDistInfo < fWorstDistance )	{
			fWorstDistance = FColl_aImpactBuf[i].fImpactDistInfo;
		}
	}
	
	_CameraCollInfo.ProjSphere.ModifyMoveDistance( fWorstDistance );
	
	return TRUE;
}

void CCamBot::Work_3rdPerson( BOOL bCollide ) {
	CFVec3A vPerpLookXZ, vLag, vCamPos_WS;
	CFVec3A vTemp;
	CFVec3A vOverridePos, vOverrideLookat;
	f32		fUnitOverride;

	//Handle override if requested by Bot
	f32 fCamPosToMinPosMag, fCamLookAtDeltaY = 0.0f;

	if ( m_pBot->IsCameraOverrideChanged()) {
		if( m_pBot->m_fCamTransTime > 0.0f ) {
			m_fOOOverrideTime = fmath_Inv( m_pBot->m_fCamTransTime );
		} else {
			m_fOOOverrideTime = 1000.0f;		//go now
		}
		m_bOverrideEnabled = m_pBot->IsCameraOverriden();
		m_pBot->SetCameraOverrideChanged(FALSE); // clear flag now we've looked at it
	}

	if( m_bOverrideEnabled || (m_fOverrideUnitTime > 0.0f) ) {
		
		if( m_bOverrideEnabled ) {
			m_fOverrideUnitTime += FLoop_fPreviousLoopSecs * m_fOOOverrideTime;
		} else {
			m_fOverrideUnitTime -= FLoop_fPreviousLoopSecs * m_fOOOverrideTime;
		}
		FMATH_CLAMP_UNIT_FLOAT( m_fOverrideUnitTime );
		
	}

	fUnitOverride = m_bAlwaysOverride ? 1.0f : fmath_UnitLinearToSCurve( m_fOverrideUnitTime );
	vOverridePos.Mul( m_pBot->m_vecCamPosAdj, fUnitOverride );
	vOverrideLookat.Mul( m_pBot->m_vecCamLookAdj, fUnitOverride );
	
	///////////////////
	// setup a few vars
	vPerpLookXZ.Set( m_pBot->m_MountUnitFrontXZ_WS.z, 0.0f, -m_pBot->m_MountUnitFrontXZ_WS.x );

	if (m_pBot->IsCameraOverridenWithPitch())
		m_fPitch = FMATH_FPOT( fUnitOverride, m_pBot->m_fMountPitch_WS, 0.0f );
	else
		m_fPitch = m_pBot->m_fMountPitch_WS;

	if( m_fPitch < 0.0f ) {
	//	fCamLookAtDeltaY = (m_fPitch / -m_pCamInfo->fMaxLookPitchUp) * m_pCamInfo->fLookupYOffset;
		//fCamLookAtDeltaY = (m_fPitch / -m_pCamInfo->fMaxLookPitchUp);
		
		// NKM - This code was giving a divide by zero
		//fCamLookAtDeltaY = (m_fPitch / m_pBot->m_fMountPitchMin_WS);
		if( FMATH_SQUARE( m_pBot->m_fMountPitchMin_WS ) < 0.0001f ) {
			fCamLookAtDeltaY = 0.0f;
		} else {
			fCamLookAtDeltaY = m_fPitch * fmath_Inv( m_pBot->m_fMountPitchMin_WS );
		}

		FMATH_CLAMPMAX( fCamLookAtDeltaY, 1.0f ); 
		fCamLookAtDeltaY *= m_pCamInfo->fLookupYOffset;
	}

	///////////////////////////
	// compute our lookat point
	
	//factor in cbot adjustment
	vTemp.Add( m_pCamInfo->TargetPos_MS, vOverrideLookat );

	m_LookAtPoint_WS.Set( (vTemp.z * m_pBot->m_MountUnitFrontXZ_WS.x) + (vTemp.x * vPerpLookXZ.x),
						  vTemp.y,
						  (vTemp.z * m_pBot->m_MountUnitFrontXZ_WS.z) + (vTemp.x * vPerpLookXZ.z) );
	m_LookAtPoint_WS.Add( m_pBot->MtxToWorld()->m_vPos );

	// move the lookat point up if the user is looking up
	m_LookAtPoint_WS.y += fCamLookAtDeltaY;
	
	/////////////////////////////////////////////////
	// calculate the camera's position in world space
	m_DesiredPos_WS = m_DesiredLookAtToPos_MS;
	
	//factor in cbot adjustment
	m_DesiredPos_WS.Add( vOverridePos );

	//////////////////////////////////////////////////////////////////////
	// Adjust the camera distance according to camera zoom factor and according to
	//   the bot's forward velocity.
	f32 fZoomAmount = 0.02f * 0.90f * m_pBot->m_fCamZoomFactor * m_pBot->m_VelocityXZ_WS.Dot(m_UnitLookDir_WS);
	FMATH_CLAMP_MIN0(fZoomAmount);
	m_DesiredPos_WS.Mul(1.0f + fZoomAmount);
	//
	//////////////////////////////////////////////////////////////////////

	if( m_fPitch != 0.0f ) {
		m_DesiredPos_WS.RotateX( m_fPitch );
	}
	m_DesiredPos_WS.RotateY( m_pBot->m_fMountYaw_WS );
	m_DesiredPos_WS.Add( m_LookAtPoint_WS );

	///////////////////////////////////////////
	// lag the camera when running (only in xz)
	// do both the pos and lookat point
#if 0
	if( m_pBot->m_fClampedNormSpeedXZ_WS > 0.0f ) {
		Lag.Set( m_pBot->m_Velocity_WS.x, 0.0f, m_pBot->m_Velocity_WS.z );
		Lag.Unitize();	
		Lag *= (m_pBot->m_fClampedNormSpeedXZ_WS * m_fLastUnitDist);
		
		m_DesiredPos_WS -= Lag;
		m_LookAtPoint_WS -= Lag;		
	}
#endif

	
#if FANG_WINGC
	{
		// Port diagnostic: where the bot camera wants to be.
		static u32 _nCalls = 0;
		if( (_nCalls++ % 240) == 30 && Fang_bPortDiag ) {
			DEVPRINTF( "PORT-CAMBOT bot=(%.1f,%.1f,%.1f) lookat=(%.1f,%.1f,%.1f) desired=(%.1f,%.1f,%.1f) yaw=%.2f pitch=%.2f frontXZ=(%.2f,%.2f)\n",
				m_pBot->MtxToWorld()->m_vPos.x, m_pBot->MtxToWorld()->m_vPos.y, m_pBot->MtxToWorld()->m_vPos.z,
				m_LookAtPoint_WS.x, m_LookAtPoint_WS.y, m_LookAtPoint_WS.z,
				m_DesiredPos_WS.x, m_DesiredPos_WS.y, m_DesiredPos_WS.z,
				m_pBot->m_fMountYaw_WS, m_fPitch, m_pBot->m_MountUnitFrontXZ_WS.x, m_pBot->m_MountUnitFrontXZ_WS.z );
		}
	}
#endif

	////////////////////////////////
	// compute our lookat direction
	m_UnitLookDir_WS = m_LookAtPoint_WS;
	m_UnitLookDir_WS.Sub( m_DesiredPos_WS );
	fCamPosToMinPosMag = m_UnitLookDir_WS.Mag();
	m_UnitLookDir_WS.Unitize();

	///////////////////////////////////////////////////
	// compute the closest pos the camera can zoom into
	m_MinCamPos_WS = m_LookAtPoint_WS;
	vTemp.Mul(m_UnitLookDir_WS, m_pCamInfo->fClosestDistToTarget);
	m_MinCamPos_WS.Add(vTemp);
	fCamPosToMinPosMag += m_pCamInfo->fClosestDistToTarget;
	
	//////////////////////////////
	// check for camera collisions		
	if( bCollide ) {
		
		f32 fWorstRayCollDist = 1.0f, fCenterRayCollDist;
		BOOL bThereAreCollisions = FALSE;
		fcoll_Clear();
#if _USE_PROJ_SPHERE_TEST
		_CameraCollInfo.nCollTestType = FMESH_COLLTESTTYPE_PROJSPHERE;
		_CameraCollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_NONE;
		_CameraCollInfo.bCullBacksideCollisions = FALSE;
		_CameraCollInfo.nCollMask = FCOLL_MASK_COLLIDE_WITH_CAMERA;
		_CameraCollInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
		_CameraCollInfo.nTrackerUserTypeBitsMask = 0xffffffffffffffff;
		
		_CameraCollInfo.ProjSphere.Init( &m_MinCamPos_WS, &m_DesiredPos_WS, _SPHERE_R );
		_CameraCollInfo.bFindClosestImpactOnly = TRUE;
		_CameraCollInfo.bCalculateImpactData = FALSE;
	
		f32 fOOCenterRayDist = (1.0f/_CameraCollInfo.ProjSphere.m_fMoveDistance);
		
		// Check world polys for collision
		f32 fWorstDistance = _CameraCollInfo.ProjSphere.m_fMoveDistance;
		u32 i;
		if( fworld_CollideWithWorldTris( &_CameraCollInfo ) ) {
			for ( i = 0; i < FColl_nImpactCount; i++ ) {
				if ( FColl_aImpactBuf[i].fImpactDistInfo < fWorstDistance )	{
					fWorstDistance = FColl_aImpactBuf[i].fImpactDistInfo;
				}
			}
		}
		
		// Modify the collinfo to reflect any new collision
		if ( FColl_nImpactCount )
		{
			_CameraCollInfo.ProjSphere.ModifyMoveDistance( fWorstDistance );
			bThereAreCollisions = TRUE;
		}
		
		fcoll_Clear();
		
		// If we're not already at 0, check the trackers
		if ( fWorstDistance != 0.f )
		{
			FWorld_nTrackerSkipListCount = 0;
			m_pBot->AppendTrackerSkipList();

			CFTrackerCollideProjSphereInfo CollProjSphereInfo;
			CollProjSphereInfo.pProjSphere = &_CameraCollInfo.ProjSphere;
			CollProjSphereInfo.bIgnoreCollisionFlag = FALSE;
			CollProjSphereInfo.nTrackerTypeBits = FWORLD_TRACKERTYPEBIT_MESH;
			CollProjSphereInfo.nTrackerUserTypeBitsMask = 0xffffffffffffffff;
			CollProjSphereInfo.pCallback = CamCollisionTrackerCallback;
			CollProjSphereInfo.nTrackerSkipCount = FWorld_nTrackerSkipListCount;
			CollProjSphereInfo.ppTrackerSkipList = FWorld_apTrackerSkipList;
			fworld_CollideWithTrackers( &CollProjSphereInfo );
		}
		
		if ( FColl_nImpactCount )
		{
			bThereAreCollisions = TRUE;
		}
		
		if ( bThereAreCollisions )
		{
			fWorstRayCollDist = _CameraCollInfo.ProjSphere.m_fMoveDistance * fOOCenterRayDist * 0.999f;
			fCenterRayCollDist = fWorstRayCollDist;
		}
		else
		{
			fCenterRayCollDist = fWorstRayCollDist = 1.f;
		}
		
#else // _USE_PROJ_SPHERE_TEST
		SetupCollisionSpheres( m_DesiredPos_WS, m_MinCamPos_WS, m_UnitLookDir_WS );

		CFCollInfo CollInfo;
		FCollImpact_t CollImpact;
		u32 i, nSkipListEntryCount;
		CFVec3 WorstRayCollPoint, Pos;

		//////////////////////////////////////////////
		// test the center ray (coll start -> cam pos)
		FWorld_nTrackerSkipListCount = 0;
		m_pBot->AppendTrackerSkipList();
		nSkipListEntryCount = FWorld_nTrackerSkipListCount;
		if( fworld_FindClosestImpactPointToRayStart( &CollImpact,
													 &m_MinCamPos_WS,
													 &m_DesiredPos_WS,
													 nSkipListEntryCount,
													 (const CFWorldTracker **)FWorld_apTrackerSkipList,
													 TRUE,
													 m_pBot->m_pWorldMesh ) ) {
			fCenterRayCollDist = CollImpact.fImpactDistInfo;				
			bThereAreCollisions = TRUE;
		} else {
			fCenterRayCollDist = 1.0f;
		}


		///////////////////////////////
		// check the collision spheres
		FCollImpact_t *apWorstSphereColls[_MAX_COLLISION_SPHERES];
		f32 fPercentAlongRay, fPercentOfCylinderR, fWorstCenterDist, fTemp, fOONegCenterRayDist = (-1.0f/fCamPosToMinPosMag);

		CollInfo.nCollTestType = FMESH_COLLTESTTYPE_SPHERE;
		CollInfo.bFindFirstImpactOnly = FALSE;
		CollInfo.bFindClosestImpactOnly = FALSE;
		CollInfo.nCollMask = FCOLL_MASK_COLLIDE_WITH_CAMERA;
		CollInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
		CollInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
		CollInfo.pSphere_WS = &m_MasterBoundingSphere;

		fWorstCenterDist = 1.0f;
		if( fworld_CollideWithWorldTris( &CollInfo ) ) {
			//fcoll_Sort( FALSE );
			for( i=0; i < m_nNumSpheresUsed; i++ ) {
				apWorstSphereColls[i] = NULL;
			}
			// find the worst collision for each sphere
			for( i=0; i < FColl_nImpactCount; i++ ) {
#if 0
				nIndex = FColl_aImpactBuf[i].nSphereIndex;
				if( apWorstSphereColls[nIndex] ) {
					if( apWorstSphereColls[nIndex]->fImpactDistInfo < FColl_aImpactBuf[i].fImpactDistInfo ) {
						apWorstSphereColls[nIndex] = &FColl_aImpactBuf[i];
					}
				} else {
					apWorstSphereColls[nIndex] = &FColl_aImpactBuf[i];
				}
#else
				// compute the worst center distance
				vTemp.Sub( FColl_aImpactBuf[i].ImpactPoint, m_MinCamPos_WS );
				fPercentAlongRay = vTemp.Dot( m_UnitLookDir_WS );
				fPercentAlongRay *= fOONegCenterRayDist;

				vTemp.Lerp( fPercentAlongRay, m_MinCamPos_WS, m_DesiredPos_WS );
				vTemp.Sub( FColl_aImpactBuf[i].ImpactPoint );
				fPercentOfCylinderR = vTemp.MagSq() * (1.0f/(_SPHERE_R * _SPHERE_R));

				FMATH_CLAMPMIN( fPercentOfCylinderR, 0.165f );
				fPercentOfCylinderR -= 0.165f;
				fPercentOfCylinderR *= ( 1.0f/(1.0f-0.165f) );
	
				fTemp = FMATH_FPOT( fPercentOfCylinderR, fPercentAlongRay, 1.0f );
				if( fTemp < fWorstCenterDist ) {
					fWorstCenterDist = fTemp;
				}
#endif
			}

#if 0
			// calculate the worst center distance
			for( i=0; i < m_nNumSpheresUsed; i++ ) {
				if( apWorstSphereColls[i] ) {
					Temp = apWorstSphereColls[i]->ImpactPoint - m_MinCamPos_WS;
					fPercentAlongRay = Temp.Dot( m_UnitLookDir_WS );
					fPercentAlongRay *= fOONegCenterRayDist;

					Temp.ReceiveLerpOf( fPercentAlongRay, m_MinCamPos_WS, m_DesiredPos_WS );
					Temp -= apWorstSphereColls[i]->ImpactPoint;
					fPercentOfCylinderR = Temp.Mag2() * (1.0f/(_SPHERE_R * _SPHERE_R));
		
					fTemp = FMATH_FPOT( fPercentOfCylinderR, fPercentAlongRay, 1.0f );
					if( fTemp < fWorstCenterDist ) {
						fWorstCenterDist = fTemp;
					}
				}
			}
#endif
			bThereAreCollisions = TRUE;
			fWorstRayCollDist = fWorstCenterDist;

			if( fWorstRayCollDist < 0.4f  ) {
				fWorstRayCollDist -= 0.05f;
				FMATH_CLAMPMIN( fWorstRayCollDist, 0.0f );			
			}
			if( fCenterRayCollDist < 0.4f ) {
				fCenterRayCollDist -= 0.05f;
				FMATH_CLAMPMIN( fCenterRayCollDist, 0.0f );
			}

			//ftext_DebugPrintf( 0.2f, 0.7f, "~w1Num Sphere Colls: %d", FColl_nImpactCount );	
		}
#endif // _USE_PROJ_SPHERE_TEST
		AnimateZoom( bThereAreCollisions, fWorstRayCollDist, fCenterRayCollDist );
					
		// compute the new camera pos
		if( m_fLastUnitDist < 1.0f ) {
			vCamPos_WS.Lerp( m_fLastUnitDist, m_MinCamPos_WS, m_DesiredPos_WS );
		} else {
			vCamPos_WS = m_DesiredPos_WS;	
		}
	} else {
		m_fLastUnitDist = 1.0f;
		m_fLastCenterRayUnitDist = 1.0f;
		vCamPos_WS = m_DesiredPos_WS;
	}
	
#if FANG_WINGC
	{
		static u32 _nCalls = 0;
		if( (_nCalls++ % 240) == 30 && Fang_bPortDiag ) {
			DEVPRINTF( "PORT-CAMBOT final=(%.1f,%.1f,%.1f) min=(%.1f,%.1f,%.1f) unitDist=%.3f collide=%d impacts=%u\n",
				vCamPos_WS.x, vCamPos_WS.y, vCamPos_WS.z, m_MinCamPos_WS.x, m_MinCamPos_WS.y, m_MinCamPos_WS.z,
				m_fLastUnitDist, (int)bCollide, (u32)FColl_nImpactCount );
		}
	}
#endif

	/////////////////////////
	// compute the camera xfm
	m_pCameraData->m_Xfm.BuildLookatFromDirVec( vCamPos_WS.v3, m_UnitLookDir_WS.v3, CFVec3::m_UnitAxisY );


	// debug stuff
	m_afHistoryBuffer[m_anHistoryIndex++] = m_fLastUnitDist;
	if( m_anHistoryIndex >= _HISTORY_BUFFER_SIZE ) {
		m_anHistoryIndex = 0;
	}
	//ftext_DebugPrintf( 0.7f, 0.7f, "~w1Z: %.3f\nD: %.3f", m_fLastUnitDist, m_fLastDeltaZoom );		
}

void CCamBot::Draw() {
	
#define __POINT_SPHERE_RADIUS 0.07f

	if( !m_pBot || !m_pCamInfo ) {
		return;
	}
	frenderer_Push( FRENDERER_DRAW, NULL );
	fdraw_Depth_EnableWriting( TRUE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER_OR_EQUAL );
	fdraw_SetTexture( NULL );
//	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );

	CFColorRGBA Red, Blue, Green, White, Yellow, Orange;
	const CFVec3 *pCamPos, *pTarget;
	CFVec3A Dir, P1, P2, P3, P4, P5, P6;
		
	pCamPos = m_pCamera->GetPos();
	pTarget = &m_LookAtPoint_WS.v3;
	Red.OpaqueRed();
	Blue.OpaqueBlue();
	Green.OpaqueGreen();
	White.OpaqueWhite();
	Yellow.Set( 1.0f, 1.0f, 0.0f, 1.0f );
	Orange.Set( 1.000f, 0.392f, 0.000f, 1.0f );
		
	// draw camera z axis
	fdraw_SolidLine( pTarget, &m_DesiredPos_WS.v3, &Blue );

	// draw camera pos
	fdraw_FacetedWireSphere( pCamPos, __POINT_SPHERE_RADIUS, 1, 1, &Red, 1 );  

	// draw camera target point
	fdraw_FacetedWireSphere( pTarget, __POINT_SPHERE_RADIUS, 1, 1, &Green, 1 );

	// draw the min camera pos
	fdraw_FacetedWireSphere( &m_MinCamPos_WS.v3, __POINT_SPHERE_RADIUS, 1, 1, &White, 1 );

	// draw the desired camera pos
	fdraw_FacetedWireSphere( &m_DesiredPos_WS.v3, __POINT_SPHERE_RADIUS, 1, 1, &White, 1 );
	
	// draw the camera coll point
	if( m_fLastCenterRayUnitDist < 1.0f ) {
		P1.Lerp( m_fLastCenterRayUnitDist, m_MinCamPos_WS, m_DesiredPos_WS );	
		fdraw_FacetedWireSphere( &P1.v3, __POINT_SPHERE_RADIUS, 1, 1, &Yellow, 1 );
	}
#if 0
	// draw the collision spheres
	u32 i;
	for( i=0; i < m_nNumSpheresUsed; i++ ) {
		fdraw_FacetedWireSphere( &m_aSphereList[i].m_Pos, m_aSphereList[i].m_fRadius, 1, 1, &Green, 1 );
	}
	fdraw_FacetedWireSphere( &m_MasterBoundingSphere.m_Pos, m_MasterBoundingSphere.m_fRadius, 1, 1, &White, 1 );
#endif

#if 0
	u32 nIndex;

	// draw the box around the history data
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	Dir.Set( 0.45f, -0.65f, 1.0f );
	FXfm_pView->TransformPointR( P1, Dir );
	Dir.Set( 0.45f, -0.4f, 1.0f );
	FXfm_pView->TransformPointR( P2, Dir );
	Dir.Set( 0.75f, -0.4f, 1.0f );
	FXfm_pView->TransformPointR( P3, Dir );
	Dir.Set( 0.75f, -0.65f, 1.0f );
	FXfm_pView->TransformPointR( P4, Dir );

#define __PERCENT_PER_ENTRY  (1.0f/(_HISTORY_BUFFER_SIZE + 1.0f + _HISTORY_BUFFER_SIZE - 1.0f))

	f32 fDelta, fLastValue, fPercent = __PERCENT_PER_ENTRY;
	nIndex = m_anHistoryIndex;
	for( i=0; i < _HISTORY_BUFFER_SIZE; i++ ) {
		fLastValue = m_afHistoryBuffer[nIndex];

		P5.ReceiveLerpOf( fPercent, P2, P1 );
		Dir.ReceiveLerpOf( fPercent, P3, P4 );
		P6.ReceiveLerpOf( fLastValue, P5, Dir );
		fdraw_SolidLine( &P5, &P6, &Blue );
		
		// move to the next entry
		if( i < (_HISTORY_BUFFER_SIZE-1) ) {
			nIndex++;	
			if( nIndex >= _HISTORY_BUFFER_SIZE ) {
				nIndex = 0;
			}
			fPercent += __PERCENT_PER_ENTRY;

			P5.ReceiveLerpOf( fPercent, P2, P1 );
			Dir.ReceiveLerpOf( fPercent, P3, P4 );
			fDelta = (fLastValue - m_afHistoryBuffer[nIndex]) * 5.0f;
			FMATH_CLAMP( fDelta, -1.0f, 1.0f );
			if( fDelta < 0.0f ) {
				P6.ReceiveLerpOf( -fDelta, Dir, P5 );
				fdraw_SolidLine( &Dir, &P6, &Green );
			} else {
				P6.ReceiveLerpOf( fDelta, Dir, P5 );
				fdraw_SolidLine( &Dir, &P6, &Red );	
			}
			fPercent += __PERCENT_PER_ENTRY;
		}
	}

	fdraw_SolidLine( &P1, &P2, &White );
	fdraw_SolidLine( &P2, &P3, &White );
	fdraw_SolidLine( &P3, &P4, &White );
	fdraw_SolidLine( &P4, &P1, &White );
#endif
	
	frenderer_Pop();

#undef __PERCENT_PER_ENTRY
#undef __POINT_SPHERE_RADIUS
}

void CCamBot::ChangeCameraMode( CamBotMode_e nNewMode ) {
	
	if( !m_pBot || !m_pCamInfo ) {
		return;
	}
	m_nMode = nNewMode;
}

f32 CCamBot::AnimateZoom( BOOL bThereAreCollisions, f32 fCurRayZoom, f32 fCurCenterZoom ) {
	f32 fNewZoom, fWorstCollDist;
	f32 fDelta, fDist2Travel;

#if 1
	if( bThereAreCollisions  ) {
		// we are colliding right now...
		fWorstCollDist = FMATH_MIN( fCurCenterZoom, fCurRayZoom );
		
		if( m_fLastUnitDist > fWorstCollDist ) {
			// we are colliding worst than we were last frame, jump the camera to the collision point
			fNewZoom = fWorstCollDist;
		} else {
			fDist2Travel = FMATH_FABS( fWorstCollDist - m_fLastUnitDist );
			fDelta = fDist2Travel * 0.07f;
			FMATH_CLAMPMAX( fDelta, 0.035f );
			if( m_pBot->IsJumping() ) {
				fDelta *= 0.25f;
			}
			fNewZoom = m_fLastUnitDist + fDelta;
		}		
	} else if( m_fLastUnitDist < 1.0f ) {
		// we were colliding recently, animate the camera back to 1.0f
		fDist2Travel = FMATH_FABS( m_fLastUnitDist - 1.0f );
		fDelta = fDist2Travel * 0.07f;
		FMATH_CLAMPMAX( fDelta, 0.035f );
		if( m_pBot->IsJumping() ) {
			fDelta *= 0.25f;
		}
		fNewZoom = m_fLastUnitDist + fDelta;			
		
		// don't animate forever, at some point just snap all the way back
		if( fNewZoom >= 0.999f ) {
			fNewZoom = 1.0f;
		}				
	} else {
		fNewZoom = 1.0f;
	}
#else
	// no camera collision
	fNewZoom = 1.0f;
#endif
	
	FASSERT_UNIT_FLOAT( fNewZoom );

	m_fLastDeltaZoom = fNewZoom - m_fLastUnitDist;
	m_fLastUnitDist = fNewZoom;
	m_fLastCenterRayUnitDist = fCurCenterZoom;

	FASSERT_UNIT_FLOAT( m_fLastUnitDist );
	FASSERT_UNIT_FLOAT( m_fLastCenterRayUnitDist );

	return fNewZoom;	
}

void CCamBot::SetupCollisionSpheres( const CFVec3A &rStartPos_WS, const CFVec3A &rEndPos_WS, const CFVec3A &rUnitStartToEnd_WS ) {
	CFVec3A Ray;
	f32 fMag, fRemainder, fDistBetweenSpheres;

	Ray.Sub(rEndPos_WS, rStartPos_WS);
	fMag = Ray.Mag();
	if( fMag <= _SPHERE_D ) {
		m_nNumSpheresUsed = 1;
		m_aSphereList[0].m_Pos.ReceiveLerpOf( 0.5f, rEndPos_WS.v3, rStartPos_WS.v3 );
		m_aSphereList[0].m_fRadius = fMag * 0.5f;
		
		m_MasterBoundingSphere = m_aSphereList[0];
		return;
	}

	// there will be more than 1 sphere, set the master sphere and the first & last sphere
	m_MasterBoundingSphere.m_Pos.ReceiveLerpOf( 0.5f, rEndPos_WS.v3, rStartPos_WS.v3 );
	m_MasterBoundingSphere.m_fRadius = fMag * 0.5f;
	
	m_nNumSpheresUsed = 2;
	m_aSphereList[1].m_Pos = rStartPos_WS.v3;
	m_aSphereList[1].m_Pos += (rUnitStartToEnd_WS.v3 * _SPHERE_R);
	m_aSphereList[1].m_fRadius = _SPHERE_R;

	m_aSphereList[0].m_Pos = rEndPos_WS.v3;
	m_aSphereList[0].m_Pos += (rUnitStartToEnd_WS.v3 * -_SPHERE_R);
	m_aSphereList[0].m_fRadius = _SPHERE_R;
		
	fRemainder = fMag - _SPHERE_D;
	if( fRemainder <= _SPHERE_MIN_DIST_BETWEEN_SPHERES ) {
		// there isn't enough space to place another sphere
		return;
	}
	fRemainder -= _SPHERE_MIN_DIST_BETWEEN_SPHERES;
	// compute the number of remaining spheres
	fDistBetweenSpheres = fRemainder * (1.0f/_SPHERE_MIN_DIST_BETWEEN_SPHERES);
	
	if( fDistBetweenSpheres > ( (f32)_MAX_COLLISION_SPHERES - 2.0f ) ) {
		fDistBetweenSpheres = fRemainder * ( 1.0f/(_MAX_COLLISION_SPHERES-2) );
	} else {
		fDistBetweenSpheres = _SPHERE_MIN_DIST_BETWEEN_SPHERES;
	}

	while( fRemainder >= _SPHERE_MIN_DIST_BETWEEN_SPHERES ) {
		m_aSphereList[m_nNumSpheresUsed].m_Pos = m_aSphereList[0].m_Pos;
		m_aSphereList[m_nNumSpheresUsed].m_Pos += (rUnitStartToEnd_WS.v3 * -fRemainder);
		m_aSphereList[m_nNumSpheresUsed].m_fRadius = _SPHERE_R;
		m_nNumSpheresUsed++;

		fRemainder -= fDistBetweenSpheres;
	}
	FASSERT( m_nNumSpheresUsed <=_MAX_COLLISION_SPHERES );
}
