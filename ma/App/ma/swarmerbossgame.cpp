//////////////////////////////////////////////////////////////////////////////////////
// swarmerbossgame.cpp - 
//
// Author: Mike Elliott     
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
// 04.02.03 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////

#if 0

#include "fang.h"
#include "swarmerbossgame.h"
#include "fworld.h"
#include "ftext.h"
#include "floop.h"
#include "botswarmerboss.h"
#include "player.h"
#include "sas_user.h"
#include "ESphere.h"

//TMP
#include "gamepad.h"

#define _SPHERE_RADIUS		( 132.0f )
#define _OO_SPHERE_RADIUS	( 1.0f / _SPHERE_RADIUS )



CSwarmerBossGame*	CSwarmerBossGame::m_pSwarmerBossGame	= NULL;
cchar*				CSwarmerBossGame::m_apszBossNames[]		= { "Boss1" };
CFVec3A				CSwarmerBossGame::m_SphereCtr;




void _UnitVecSlerpedStep( CFVec3A *pvResult, const CFVec3A &vStart, const CFVec3A &vEnd, f32 fAngle );


/////////////////////////
//STATICS

BOOL CSwarmerBossGame::LoadLevel( LevelEvent_e eEvent ) {
	// Only load the data on LEVEL_EVENT_POST_WORLD_LOAD, this was the original behavior
	if( eEvent != LEVEL_EVENT_POST_WORLD_LOAD ) {
		return !!m_pSwarmerBossGame;
	}

	FASSERT( !m_pSwarmerBossGame );
	FASSERT( FWorld_pWorld );

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	DEVPRINTF( "**ME:  CSwarmerBossGame::LoadLevel(): Beginning level load\n" );
#endif

	// set up some static type vars...
	m_SphereCtr.Zero();

	FResFrame_t frame = fres_GetFrame();

	m_pSwarmerBossGame = fnew CSwarmerBossGame;
	if( !m_pSwarmerBossGame ) {
		DEVPRINTF( "CSwarmerBossGame::LoadLevel(): ERROR:  Unable to allocate game data\n" );
		goto _ExitWithError;
	}

	if( !m_pSwarmerBossGame->_Create() ) {
		DEVPRINTF( "CSwarmerBossGame::LoadLevel(): ERROR:  Unable to init game data\n" );
		goto _ExitWithError;
	}

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	DEVPRINTF( "**ME:  CSwarmerBossGame::LoadLevel(): Level loaded\n" );
#endif

	return TRUE;

_ExitWithError:
	FASSERT_NOW;
	UnloadLevel();
	fres_ReleaseFrame( frame );
	return FALSE;
}


void CSwarmerBossGame::UnloadLevel( void ) {
#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	DEVPRINTF( "**ME:  CSwarmerBossGame::UnloadLevel(): Beginning level unload\n" );
#endif
	if( m_pSwarmerBossGame ) {
        m_pSwarmerBossGame->_Destroy();
		fdelete( m_pSwarmerBossGame );
		m_pSwarmerBossGame = NULL;
	}	

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	DEVPRINTF( "**ME:  CSwarmerBossGame::UnloadLevel(): Level unloaded\n" );
#endif
}


/////////////////////////
//PRIVATES

CSwarmerBossGame::CSwarmerBossGame() {
	m_bCreated = FALSE;
	_ClearDataMembers();
}


void CSwarmerBossGame::_ClearDataMembers( void ) {
	for( u32 i=0; i<_MAX_BOSSES; i++ ) {
		m_apSBosses[i] = NULL;

		m_aSBBrains[i].bActive = FALSE;
		m_aSBBrains[i].qPos.Identity();
		m_aSBBrains[i].qDesiredPos.Identity();
	}
}


BOOL CSwarmerBossGame::_Create( void ) {
	FASSERT( !m_bCreated );
	m_bCreated = TRUE;
	u32 i;

	FResFrame_t frame = fres_GetFrame();

	for( i=0; i<_MAX_BOSSES; i++ ) {
		m_apSBosses[i] = fnew CBotSwarmerBoss;
		if( !m_apSBosses[i] ) {
			DEVPRINTF( "CSwarmerBossGame::_Create():  ERROR:  Unable to allocate CBotSwarmerBoss\n" );
			goto _ExitWithError;
		}

		if( !m_apSBosses[i]->Create( -1, FALSE, m_apszBossNames[i] ) ) {
			DEVPRINTF( "CSwarmerBossGame::_Create():  ERROR:  Unable to create CBotSwarmerBoss %s\n", m_apszBossNames[i] );
			goto _ExitWithError;
		}
	}

	// create the control structures for the bots
	for( i=0; i<_MAX_BOSSES; i++ ) {
		m_apSBControls[i] = fnew CBotControl;
		if( !m_apSBControls[i] ) {
			DEVPRINTF( "CSwarmerBossGame::_Create():  ERROR:  Unable to allocate CBotControl\n" );
			goto _ExitWithError;
		}
        
		m_apSBControls[i]->Zero();
		m_apSBosses[i]->SetControls( m_apSBControls[i] );

	}

	//METMP
	m_apSBosses[0]->AddToWorld();
	m_apSBosses[0]->EnterMoveStateWall();
	m_aSBBrains[0].vPos = CFVec3A::m_UnitAxisY;


	// Initialize data (will be loaded from data files eventually)
	m_uNoBands = 5;
	m_paBands = (Band_t*)fres_Alloc( m_uNoBands * sizeof( Band_t ) );
	if( m_paBands == NULL ) {
		DEVPRINTF( "CSwarmerBossGame::LoadLevel(): ERROR:  Unable to allocate band data\n" );
		goto _ExitWithError;
	}

	// set up band data (to be read from csv)
	m_paBands[0].uID				= 0;
	m_paBands[0].uBandFlags 		= 0; //BANDFLAG_ALLOW_FLOOR_JUMP | BANDFLAG_ALLOW_WALL_JUMP;
	m_paBands[0].fYTop				= _SPHERE_RADIUS;
	m_paBands[0].fYBottom			= 25.0f;
	m_paBands[0].fFloorJumpY		= 20.0f;
	m_paBands[0].fFloorJumpMinXZ	= 25.0f;
	m_paBands[0].fFloorJumpMaxXZ	= 50.0f;
	m_paBands[0].fWallJumpMaxY		= 40.0f;
	m_paBands[0].fWallJumpMinY		= 25.0f;
	m_paBands[0].behavior			= BEHAVIOR_MATCH_PROJECTION;

	m_paBands[1].uID				= 1;
	m_paBands[1].uBandFlags 		= BANDFLAG_ALLOW_FLOOR_JUMP; // | BANDFLAG_ALLOW_WALL_JUMP;
	m_paBands[1].fFloorJumpY		= -10.0f;
	m_paBands[1].fFloorJumpMinXZ	= 25.0f;
	m_paBands[1].fFloorJumpMaxXZ	= 50.0f;
	m_paBands[1].fYTop				= m_paBands[0].fYBottom;
	m_paBands[1].fYBottom			= -15.0f;
	m_paBands[1].fWallJumpMaxY		= 15.0f;
	m_paBands[1].fWallJumpMinY		= -5.0f;
	m_paBands[1].behavior			= BEHAVIOR_MATCH_LEVEL;

	m_paBands[2].uID				= 2;
	m_paBands[2].uBandFlags 		= BANDFLAG_ALLOW_FLOOR_JUMP; // | BANDFLAG_ALLOW_WALL_JUMP;
	m_paBands[2].fFloorJumpY		= -40.0f;
	m_paBands[2].fFloorJumpMinXZ	= 25.0f;
	m_paBands[2].fFloorJumpMaxXZ	= 50.0f;
	m_paBands[2].fYTop				= m_paBands[1].fYBottom;
	m_paBands[2].fYBottom			= -45.0f;
	m_paBands[2].fWallJumpMaxY		= -15.0f;
	m_paBands[2].fWallJumpMinY		= -35.0f;
	m_paBands[2].behavior			= BEHAVIOR_MATCH_LEVEL;

	m_paBands[3].uID				= 3;
	m_paBands[3].uBandFlags 		= BANDFLAG_ALLOW_FLOOR_JUMP; // | BANDFLAG_ALLOW_WALL_JUMP;
	m_paBands[3].fFloorJumpY		= -70.0f;
	m_paBands[3].fFloorJumpMinXZ	= 25.0f;
	m_paBands[3].fFloorJumpMaxXZ	= 50.0f;
	m_paBands[3].fYTop				= m_paBands[2].fYBottom;
	m_paBands[3].fYBottom			= -75.0f;
	m_paBands[3].fWallJumpMaxY		= -45.0f;
	m_paBands[3].fWallJumpMinY		= -65.0f;
	m_paBands[3].behavior			= BEHAVIOR_MATCH_LEVEL;

	m_paBands[4].uID				= 3;
	m_paBands[4].uBandFlags 		= BANDFLAG_ALLOW_FLOOR_JUMP; // | BANDFLAG_ALLOW_WALL_JUMP;
	m_paBands[4].fFloorJumpY		= -70.0f;
	m_paBands[4].fFloorJumpMinXZ	= 25.0f;
	m_paBands[4].fFloorJumpMaxXZ	= 50.0f;
	m_paBands[4].fYTop				= m_paBands[3].fYBottom;
	m_paBands[4].fYBottom			= -_SPHERE_RADIUS;
	m_paBands[4].fWallJumpMaxY		= -45.0f;
	m_paBands[4].fWallJumpMinY		= -65.0f;
	m_paBands[4].behavior			= BEHAVIOR_NOT_WALKABLE;



	// init the avoid spheres
	if( !(m_apAvoidSpheres[0] = (CESphere*)CEntity::Find( "acidhole01" )) ) {
		goto _ExitWithError;
	}
	if( !(m_apAvoidSpheres[1] = (CESphere*)CEntity::Find( "acidhole02" )) ) {
		goto _ExitWithError;
	}
	if( !(m_apAvoidSpheres[2] = (CESphere*)CEntity::Find( "acidhole03" )) ) {
		goto _ExitWithError;
	}
	if( !(m_apAvoidSpheres[3] = (CESphere*)CEntity::Find( "acidhole04" )) ) {
		goto _ExitWithError;
	}
	if( !(m_apAvoidSpheres[4] = (CESphere*)CEntity::Find( "acidhole05" )) ) {
		goto _ExitWithError;
	}
	if( !(m_apAvoidSpheres[5] = (CESphere*)CEntity::Find( "acidhole06" )) ) {
		goto _ExitWithError;
	}
	if( !(m_apAvoidSpheres[6] = (CESphere*)CEntity::Find( "glasshole01" )) ) {
		goto _ExitWithError;
	}
	if( !(m_apAvoidSpheres[7] = (CESphere*)CEntity::Find( "glasshole02" )) ) {
		goto _ExitWithError;
	}
	if( !(m_apAvoidSpheres[8] = (CESphere*)CEntity::Find( "elevhole01" )) ) {
		goto _ExitWithError;
	}

	return TRUE;

_ExitWithError:
	FASSERT_NOW;
	_Destroy();
	fres_ReleaseFrame( frame );
	return FALSE;
}


void CSwarmerBossGame::_Destroy( void ) {
	if( !m_bCreated ) {
		return;
	}

	for( u32 i=0; i<_MAX_BOSSES; i++ ) {
		if( m_apSBosses[i] ) {
			m_apSBosses[i]->Destroy();
			fdelete( m_apSBosses[i] );
			m_apSBosses[i] = NULL;

			fdelete( m_apSBControls[i] );
			m_apSBControls[i] = NULL;
		}
	}
}


void CSwarmerBossGame::_Work( void ) {

//	floop_SetTimeScale( 0.05f );
	_Think( 0 );
	_Move( 0 );

	
#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	if( Gamepad_aapSample[Player_aPlayer[0].m_nControllerIndex][GAMEPAD_MAIN_ACTION]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
		m_apSBosses[0]->JumpFromWallToPoint( m_aSBBrains[0].vJumpPos );
	}
	_DebugFn();
#endif
}


void CSwarmerBossGame::_Think( u32 uID ) {
	FASSERT( uID < _MAX_BOSSES );

	CBot *pBot = (CBot*)Player_aPlayer[0].m_pEntityCurrent;		//ME:  just grab this once.  Safe?
	FASSERT( pBot && pBot->TypeBits() & ENTITY_BIT_BOTGLITCH );

	switch( m_apSBosses[uID]->GetMoveState() ) {
		case CBotSwarmerBoss::MOVESTATE_WALL:
			_WallCalcDest( m_apSBosses[uID]->MtxToWorld()->m_vPos, pBot->MtxToWorld()->m_vPos, &m_aSBBrains[uID].vTgtPos );
			_CheckForPlatformJump( m_apSBosses[uID]->MtxToWorld()->m_vPos, pBot->MtxToWorld()->m_vPos, &m_aSBBrains[uID].vJumpPos );
			break;

		default:
			_PlatformCalcDest( m_apSBosses[uID]->MtxToWorld()->m_vPos, pBot->MtxToWorld()->m_vPos, &m_aSBBrains[uID].vTgtPos );
			break;
	}


	m_apSBosses[uID]->SetTargetPos( &pBot->MtxToWorld()->m_vPos );

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	ftext_DebugPrintf( 0.2f, 0.45f, "~w1%0.2f", pBot->MtxToWorld()->m_vPos.y );
	ftext_DebugPrintf( 0.2f, 0.42f, "~w1%0.2f", m_apSBosses[0]->MtxToWorld()->m_vPos.Dist( m_aSBBrains[0].vJumpPos ) );
#endif
}


void CSwarmerBossGame::_Move( u32 uID ) {
	FASSERT( uID < _MAX_BOSSES );


	if( m_apSBosses[uID]->GetMoveState() == CBotSwarmerBoss::MOVESTATE_WALL ) {
		CFVec3A vNewPos;	

		_MoveAlongSphereToPoint( m_aSBBrains[uID].vPos, m_aSBBrains[uID].vTgtPos, &vNewPos );

		//for( u32 i=0; i<_MAX_AVOID_SPHERES; i++ ) {
		//	if( m_aAvoidSpheres[i].IsIntersecting( vNewPos.v3 ) ) {
		//		CFVec3A vTmp;
		//		vTmp = vNewPos;
		//		vTmp.Sub( m_aAvoidSpheres[i].m_Pos );
		//		if( vTmp.SafeUnitAndMag( vTmp ) > 0.01f ) {
		//			vTmp.Mul( m_aAvoidSpheres[i].m_fRadius ).Add( m_aAvoidSpheres[i].m_Pos );
		//			_MoveAlongSphereToPoint( m_aSBBrains[uID].vPos, vTmp, &vNewPos );
		//			
		//		}
		//	}
		//}

		_OrientBossOnWall( uID );
		m_aSBBrains[uID].vPos = vNewPos;
	} else if( m_apSBosses[uID]->GetMoveState() == CBotSwarmerBoss::MOVESTATE_NORMAL ) {
		_MoveToward( uID, m_aSBBrains[uID].vTgtPos );
		_FaceToward( uID, m_aSBBrains[uID].vTgtPos );
	}
}


void CSwarmerBossGame::_MoveAlongSphereToPoint( const CFVec3A &vFrom, const CFVec3A &vTo, CFVec3A *pvResult ) {
	FASSERT( pvResult );
	
	_UnitVecSlerpedStep( pvResult, vFrom, vTo, m_apSBosses[0]->GetMaxFlatSurfaceSpeed() * _OO_SPHERE_RADIUS * FLoop_fPreviousLoopSecs );
}


#define _ROT_ANGLE		( FMATH_DEG2RAD( 180.0f ) )

void CSwarmerBossGame::_OrientBossOnWall( u32 uID ) {
	FASSERT( uID < _MAX_BOSSES );

	CFVec3A vCurrentOr;
	CFVec3A vDesiredOr;
	f32 fMag2;

	vCurrentOr = m_apSBosses[uID]->MtxToWorld()->m_vFront;
	vDesiredOr.Sub( m_aSBBrains[uID].vTgtPos, m_aSBBrains[uID].vPos );
	fMag2 = vDesiredOr.MagSq();

	if( fMag2 > 0.0001f ) {
		vDesiredOr.Mul( fmath_InvSqrt( fMag2 ) );
		_UnitVecSlerpedStep( &CFMtx43A::m_Temp.m_vFront, vCurrentOr, vDesiredOr, _ROT_ANGLE * FLoop_fPreviousLoopSecs );
		//CFMtx43A::m_Temp.m_vFront = vCurrentOr;
	} else {
        CFMtx43A::m_Temp.m_vFront = vCurrentOr;
	}

	CFMtx43A::m_Temp.m_vUp.ReceiveNegative( m_aSBBrains[uID].vPos );
	CFMtx43A::m_Temp.m_vRight.Cross( CFMtx43A::m_Temp.m_vUp, CFMtx43A::m_Temp.m_vFront );
	CFMtx43A::m_Temp.m_vRight.Unitize();
	CFMtx43A::m_Temp.m_vFront.Cross( CFMtx43A::m_Temp.m_vRight, CFMtx43A::m_Temp.m_vUp );
	CFMtx43A::m_Temp.m_vPos.Mul( m_aSBBrains[uID].vPos, _SPHERE_RADIUS );

	m_apSBosses[uID]->Relocate_RotXlatFromUnitMtx_WS( &CFMtx43A::m_Temp );
}


#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
void CSwarmerBossGame::_DebugFn( void ) {
	//CFVec3A vTmp;
	//vTmp.Mul( m_aSBBrains[0].vTgtPos, _SPHERE_RADIUS );
	//fdraw_DevSphere( &(vTmp.v3 ), 3.0f, &FColor_MotifGreen, 4, 4 );
	//vTmp.Mul( m_aSBBrains[0].vPos, _SPHERE_RADIUS );
	//fdraw_DevSphere( &(vTmp.v3 ), 6.0f, &FColor_MotifBlue, 4, 4 );

	//for( u32 i=0; i<_MAX_AVOID_SPHERES; i++ ) {
	//	//fdraw_DevSphere( &(m_aAvoidSpheres[i].m_Pos), m_aAvoidSpheres[i].m_fRadius, &FColor_MotifGreen, 4, 4 );
	//	fdraw_DevSphere( &(m_apAvoidSpheres[i]->Sphere_WS()->m_Pos.v3), m_apAvoidSpheres[i]->Sphere_WS()->m_fRadius, &FColor_MotifGreen, 4, 4 );
	//}

	//// draw jump target
	//if( m_aSBBrains[0].vJumpPos != CFVec3A::m_Null ) {
	//	fdraw_DevSphere( &(m_aSBBrains[0].vJumpPos.v3), 2.5f, &FColor_MotifRed, 4, 4 );
	//}

	//CFVec3A vUp;
	//vUp.Mul( m_apSBosses[0]->MtxToWorld()->m_vUp, 1000.0f ).Add( m_apSBosses[0]->MtxToWorld()->m_vPos );
	//fdraw_DevLine( &(m_apSBosses[0]->MtxToWorld()->m_vPos.v3), &(vUp.v3), &FColor_MotifRed );
}
#endif



const CSwarmerBossGame::Band_t* CSwarmerBossGame::_GetBand( f32 fYPos ) {
	for( u32 i=0; i<m_uNoBands; i++ ) {
		if( m_paBands[i].fYBottom <= fYPos ) {
			return &m_paBands[i];
		}
	}
//	FASSERT_NOW;
	return NULL;
}



#define _ROTATE_THRESHOLD		( 0.05f )

void CSwarmerBossGame::_FaceToward( u32 uID, const CFVec3A &vTgtPos ) {
	FASSERT( uID < _MAX_BOSSES );
	
	CBotSwarmerBoss *pBoss = m_apSBosses[uID];
	FASSERT( pBoss );

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	CFVec3A vTmp;
	vTmp.Mul( pBoss->MtxToWorld()->m_vRight, 1000.0f ).Add( pBoss->MtxToWorld()->m_vPos );
	fdraw_DevLine( &pBoss->MtxToWorld()->m_vPos.v3, &vTmp.v3, &FColor_MotifRed, &FColor_MotifRed );
#endif

    
	f32 fMag2;
	CFVec3A vToTgt;
	vToTgt.Sub( vTgtPos, pBoss->MtxToWorld()->m_vPos );
	fMag2 = vToTgt.MagSq();

	if( fMag2 < 0.0001f ) {
		m_apSBControls[uID]->m_fRotateCW = 0.0f;
		return;
	}

	vToTgt.Mul( fmath_InvSqrt( fMag2 ) );
    
	f32 fRightMag = vToTgt.Dot( pBoss->MtxToWorld()->m_vRight );
	if( FMATH_FABS( fRightMag ) < _ROTATE_THRESHOLD ) {
		m_apSBControls[uID]->m_fRotateCW = 0.0f;
	} else if( fRightMag > 0.0f ) {
        m_apSBControls[uID]->m_fRotateCW = fmath_Sqrt( fRightMag );
	} else {
		m_apSBControls[uID]->m_fRotateCW = -fmath_Sqrt( -fRightMag );
	}
}

#define _MOVE_THRESHOLD			( 5.0f )
#define _MOVE_THRESHOLD_SQ		( _MOVE_THRESHOLD * _MOVE_THRESHOLD )


void CSwarmerBossGame::_MoveToward( u32 uID, const CFVec3A &vTgtPos ) {
	FASSERT( uID < _MAX_BOSSES );
	
	CBotSwarmerBoss *pBoss = m_apSBosses[uID];
	FASSERT( pBoss );

	CFVec3A vToTgt;
	vToTgt.Sub( vTgtPos, pBoss->MtxToWorld()->m_vPos );
	f32 fMag2 = vToTgt.MagSq();
	if( fMag2 > _MOVE_THRESHOLD_SQ ) {
		vToTgt.Mul( fmath_InvSqrt( fMag2 ) );
		m_apSBControls[uID]->m_XlatNormSpeedXZ_WS = vToTgt;
	} else {
		m_apSBControls[uID]->m_XlatNormSpeedXZ_WS.Zero();
	}

}


void CSwarmerBossGame::_WallCalcDest( const CFVec3A &vPos, const CFVec3A &vTgt, CFVec3A *pvDest ) {
	// project glitchy on the wall at the correct altitude.
	f32 fV;
	const Band_t *pBand = _GetBand( vTgt.y );
	if( !pBand ) {
		*pvDest = vPos;
		return;
	}

	pvDest->Sub( vTgt, m_SphereCtr );

	pvDest->y = (pBand->fWallJumpMinY + pBand->fWallJumpMaxY) * 0.5f;		// randomize

	fV = pvDest->MagSqXZ();
	if( fV > 0.001f ) {
		fV = fmath_InvSqrt( fV );
		pvDest->x *= fV;
		pvDest->z *= fV;

		fV = fmath_Sqrt( (_SPHERE_RADIUS * _SPHERE_RADIUS) - (pvDest->y * pvDest->y) );
		pvDest->x *= fV;
		pvDest->z *= fV;

		pvDest->Unitize();
	} else {
		*pvDest = CFVec3A::m_UnitAxisY;
	}
}


BOOL CSwarmerBossGame::_CheckForPlatformJump( const CFVec3A &vPos, const CFVec3A &vTgt, CFVec3A *pvDest ) {
	const Band_t *pBand = _GetBand( vTgt.y );		// shouldn't get this over & over again, the player's only going to be in one band a frame
	if( !pBand ) {
		return FALSE;
	}

	if( (vPos.y > pBand->fWallJumpMinY) && (vPos.y < pBand->fWallJumpMaxY) ) {
		// good to go
		
		pvDest->Sub( vTgt, vPos );
	
		f32 fDistXZ2 = pvDest->MagSqXZ();

		if( fDistXZ2 > 0.01f ) {
			fDistXZ2 = fmath_InvSqrt( fDistXZ2 );
			pvDest->x *= fDistXZ2;
			pvDest->z *= fDistXZ2;
		} else {
            pvDest->Sub( m_SphereCtr, vPos );		// bot is on the wall, so mag should be sphere radius
			pvDest->Mul( fmath_Inv( _SPHERE_RADIUS ) );
		}

		pvDest->Mul( pBand->fFloorJumpMaxXZ );
		pvDest->Add( vPos );
		pvDest->y = pBand->fFloorJumpY;

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
		fdraw_DevSphere( &pvDest->v3, 2.0f, &FColor_MotifRed );
#endif


		return TRUE;

	} else {
		return FALSE;
	}
}


void CSwarmerBossGame::_PlatformCalcDest( const CFVec3A &vPos, const CFVec3A &vTgt, CFVec3A *pvDest ) {
	const Band_t *pBand = _GetBand( vTgt.y );
	if( !pBand ) {
		FASSERT_NOW;
		return;
	}

	CFSphere sphere;
	sphere.Set( m_SphereCtr.v3, _SPHERE_RADIUS - pBand->fFloorJumpMaxXZ );

	CFVec3A vIntersectPt;

	if( !sphere.IsIntersecting( vPos.v3, vTgt.v3, &pvDest->v3 ) ) {
		*pvDest = vTgt;
	} else {
        CFVec3A vToCtr, vToTgt;
		vToCtr.Sub( m_SphereCtr, vPos );
		// impossible for this vector to be length 0
		vToCtr.UnitizeXZ();
		pvDest->x = vToCtr.z;
		pvDest->z = -vToCtr.x;
		pvDest->y = 0.0f;

		vToTgt.Sub( vTgt, vPos );
        

		if( pvDest->Dot( vToTgt ) < 0.0f ) {
			pvDest->Negate();
		}
		pvDest->Mul( 15.0f );
		pvDest->Add( vPos );

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
		fdraw_DevSphere( &pvDest->v3, 3.0f, &FColor_MotifRed );
#endif

	}
    
#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	fdraw_DevSphere( &vPos.v3, 3.0f, &FColor_MotifGreen );
	fdraw_DevSphere( &pvDest->v3, 2.5f, &FColor_MotifBlue );

	fdraw_DevSphere( &vTgt.v3, 3.0f, &FColor_MotifGreen );
	fdraw_DevLine( &vPos.v3, &pvDest->v3, &FColor_MotifGreen, &FColor_MotifBlue );
	fdraw_DevLine( &pvDest->v3, &vTgt.v3, &FColor_MotifGreen, &FColor_MotifGreen );
#endif
}



const CFSphereA* CSwarmerBossGame::_FindAvoidSphere( const CFVec3A &vPt ) {
	for( u32 i=0; i<_MAX_AVOID_SPHERES; i++ ) {
		if( m_apAvoidSpheres[i]->Sphere_WS()->IsIntersecting( vPt ) ) {
			return m_apAvoidSpheres[i]->Sphere_WS();
		}
	}

	return NULL;
}



////////////////////
//UTILITY

void _UnitVecSlerpedStep( CFVec3A *pvResult, const CFVec3A &vStart, const CFVec3A &vEnd, f32 fAngle ) {
	FASSERT( vStart.MagSq() > 0.0001f );
	FASSERT( vEnd.MagSq() > 0.0001f );

    CFVec3A vAxis;
	f32 fMaxSin;
	f32 fSin, fCos;

	vAxis.Cross( vStart, vEnd );
	fMaxSin = vAxis.Mag();

	if( fMaxSin < 0.000001f ) {
		*pvResult = vStart;
		return;
	}

	vAxis.Mul( fmath_Inv( fMaxSin ) );

	if( (fmath_Sin( fAngle ) < fMaxSin) || (vStart.Dot( vEnd ) < 0.0f) ) {
		CFQuatA qRot;

		fmath_SinCos( fAngle * 0.5f , &fSin, &fCos );

		qRot.x = vAxis.x * fSin;                        			
		qRot.y = vAxis.y * fSin;
		qRot.z = vAxis.z * fSin;
		qRot.w = fCos;

		qRot.MulPoint( *pvResult, vStart );

	} else {
		// angle greater than required, just return end vect
		*pvResult = vEnd;
	}
}



#endif //0