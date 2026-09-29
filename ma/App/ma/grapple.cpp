//////////////////////////////////////////////////////////////////////////////////////
// grapple.cpp - Sniper's grapple line.
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
// 03/18/02 Elliott       Created.
//////////////////////////////////////////////////////////////////////////////////////




#include "fang.h"
#include "grapple.h"

#include "flinklist.h"
#include "fres.h"
#include "fresload.h"
#include "MeshEntity.h"
#include "bot.h"
#include "fdraw.h"
#include "botmortar.h"
#include "ai/aigameutils.h"
#include "fvtxpool.h"
#include "frenderer.h"


#define _GRAPPLE_MESH				( "GRMNgrappl1" )

#define _RELOAD_TIME				( 1.0f )
#define _OO_RELOAD_TIME				( 1.0f / _RELOAD_TIME )
#define _FIRE_SPEED					( 250.0f )
#define _OO_NUM_GRAPPLE_PTS			( 1.0f/(f32)_NO_GRAPPLE_LINE_PTS )

#define _LINE_THICKNESS				( 0.05f )
#define _LINE_LIGHT_MIN_INTENSITY	( 0.1f )
#define _LINE_LIGHT_MAX_INTENSITY	( 1.0f )
#define _CABLE_TEXTURE				( "TRMNcable01" )

BOOL		CGrapple::m_bSystemInitialized	= FALSE;
FLinkRoot_t CGrapple::m_LinkRoot;
FMesh_t*	CGrapple::m_pMesh				= NULL;
CFVec3A		CGrapple::m_avPentagonVtx[6];
f32			CGrapple::m_afPentagonVtxIntensity[6];
CFTexInst	CGrapple::m_CableTex;




BOOL CGrapple::InitSystem( void ) {
	u32 i;

	flinklist_InitRoot( &m_LinkRoot, FANG_OFFSETOF( CGrapple, m_Link ) );
	m_bSystemInitialized = TRUE;

	m_avPentagonVtx[0].Set(  0.0f,   -1.0f,   0.0f );
	m_avPentagonVtx[1].Set(  0.951f, -0.309f, 0.0f );
	m_avPentagonVtx[2].Set(  0.588f,  0.809f, 0.0f );
	m_avPentagonVtx[3].Set( -0.588f,  0.809f, 0.0f );
	m_avPentagonVtx[4].Set( -0.951f, -0.309f, 0.0f );
	m_avPentagonVtx[5].Set( m_avPentagonVtx[0] );

	for( i=0; i<6; ++i ) {
		m_avPentagonVtx[i].Mul( _LINE_THICKNESS );
	}

	// Build pentagon lighting...
	for( i=0; i<6; i++ ) {
		m_afPentagonVtxIntensity[i] = 0.5f + 0.5f*fmath_Sin( (f32)i * (FMATH_2PI/5.0f) + FMATH_DEG2RAD(-20.0f) );
		m_afPentagonVtxIntensity[i] = FMATH_FPOT( m_afPentagonVtxIntensity[i], _LINE_LIGHT_MIN_INTENSITY, _LINE_LIGHT_MAX_INTENSITY );
	}

	return TRUE;
}


void CGrapple::UninitSystem( void ) {
	m_bSystemInitialized = FALSE;
}

BOOL CGrapple::_InitSharedData( void ) {
	FASSERT( m_LinkRoot.nCount != 0 );

	if( m_LinkRoot.nCount > 1 ) {
		return TRUE;
	}
	
	// we have exactly one created.  Init any shared data
	FResFrame_t	frame = fres_GetFrame();

	FTexDef_t *pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, _CABLE_TEXTURE );
	if( !pTexDef ) {
		DEVPRINTF( "CGrapple::_InitSharedData():  Unable to load cable texture:  %s\n", _CABLE_TEXTURE );
		goto _ExitWithError;
	}
		
	m_CableTex.SetTexDef( pTexDef );

	//// Load mesh resource...
	//m_pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, _GRAPPLE_MESH);
	//if( m_pMesh == NULL ) {
	//	DEVPRINTF( "CGrapple::_InitSharedData(): Could not find mesh '%s'\n", _GRAPPLE_MESH );
	//	goto _ExitWithError;
	//}

	return TRUE;

_ExitWithError:
	FASSERT_NOW;
	fres_ReleaseFrame( frame );
	return FALSE;
}


void CGrapple::_DestroySharedData( void ) {
	if( m_LinkRoot.nCount > 0 ) {
		return;
	}

	// destroy any shared data
	if( m_pMesh ) {
		fdelete( m_pMesh );
		m_pMesh = NULL;
	}
}


//CGrapple* CGrapple::CreateNewGrapple( CBot* pOwnerBot, cchar* pszAttachBoneName ) {
//	FASSERT( m_bSystemInitialized );
//
//	FResFrame_t frame = fres_GetFrame();
//
//	CGrapple* pGrapple = fnew CGrapple();
//	if( !pGrapple ) {
//		goto _ExitWithError;
//	}
//
//	flinklist_AddTail( &m_LinkRoot, pGrapple );
//
//	if( !_InitSharedData() ) {
//		goto _ExitWithError;
//	}
//   
//	// set up the grapple here
//	if( !pGrapple->_Create( pOwnerBot, pszAttachBoneName ) ) {
//		goto _ExitWithError;
//	}
//
//	return pGrapple;
//
//_ExitWithError:
//	if( pGrapple ) {
//		fdelete( pGrapple );
//	}
//	fres_ReleaseFrame( frame );
//	return NULL;
//}


void CGrapple::DrawAll( void ) {
	FASSERT( m_bSystemInitialized );

	BOOL bSetRenderState = FALSE;

	CGrapple *pGrapple = (CGrapple*)flinklist_GetHead( &m_LinkRoot );
	
	while( pGrapple ) {
		if( pGrapple->GetState() != STATE_INACTIVE ) {
			if( !bSetRenderState ) {
				frenderer_SetDefaultState();
				frenderer_Push( FRENDERER_DRAW, NULL );
				bSetRenderState = TRUE;
			}
            
			pGrapple->_DrawCable();
		}
		pGrapple = (CGrapple*)flinklist_GetNext( &m_LinkRoot, pGrapple );
	}

	// all done
	if( bSetRenderState ) {
		frenderer_Pop();
	}
}


//void CGrapple::DestroyGrapple( CGrapple* pGrapple ) {
//	FASSERT( pGrapple );
//
//	flinklist_Remove( &m_LinkRoot, pGrapple );
//
//	ClassHierarchyUnloadSharedResources();
//
//	fdelete( pGrapple );
//	pGrapple = NULL;
//}



CGrapple::CGrapple() {
	_ClearDataMembers();
}


CGrapple::~CGrapple() {
}


void CGrapple::_ClearDataMembers( void ) {
	m_pHookMesh = NULL;
	m_pOwnerBot = NULL;
	m_pszAttachBoneName = NULL;
	m_pMtxAttachBone = NULL;
	m_fReloadTimer = 0.0f;
}


BOOL CGrapple::Create( CBot* pOwnerBot, cchar *pszAttachBoneName ) {
	FASSERT( m_bSystemInitialized );
	m_bIsCreated = TRUE;

	s32 nBoneIdx;			//CPS 4.7.03
	
	FResFrame_t frame = fres_GetFrame();

	flinklist_AddTail( &m_LinkRoot, this );

	if( !_InitSharedData() ) {
		goto _ExitWithError;
	}

	m_pHookMesh = fnew CMeshEntity;
	if( !m_pHookMesh ) {
		DEVPRINTF( "CGrapple::_Create():  Out of memory creating mesh\n" );
		goto _ExitWithError;
	}

	if( !m_pHookMesh->Create( _GRAPPLE_MESH ) ) {
		goto _ExitWithError;
	}

	m_pHookMesh->SetCollisionFlag( FALSE );

	FASSERT( pOwnerBot && (pOwnerBot->TypeBits() & ENTITY_BIT_BOT) );
	m_pOwnerBot = pOwnerBot;
	m_pszAttachBoneName = pszAttachBoneName;
	
//CPS 4.7.03	s32 nBoneIdx = m_pOwnerBot->m_pWorldMesh->FindBone( pszAttachBoneName );
	nBoneIdx = m_pOwnerBot->m_pWorldMesh->FindBone( pszAttachBoneName );		//CPS 4.7.03
	if( nBoneIdx < 0 ) {
		DEVPRINTF( "CGrapple::Create():  Invalid attach bone %s.\n", pszAttachBoneName );
		goto _ExitWithError;
	}

	m_pMtxAttachBone = m_pOwnerBot->m_pWorldMesh->GetBoneMtxPalette()[nBoneIdx];

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( frame );
	return FALSE;
}


void CGrapple::Destroy( void ) {
	FASSERT( m_bSystemInitialized );
	if( !IsCreated() ) {
		return;
	}

	flinklist_Remove( &m_LinkRoot, this );

	_DestroySharedData();

	if( m_pHookMesh ) {
		m_pHookMesh->Destroy();
		fdelete( m_pHookMesh );
		m_pHookMesh = NULL;
	}

	m_bIsCreated = FALSE;
}


void CGrapple::AddToWorld( void ) {
	m_pHookMesh->AddToWorld();
	m_pHookMesh->Attach_UnitMtxToParent_PS( m_pOwnerBot, m_pszAttachBoneName );
	Kill();
	m_eState = STATE_INACTIVE;
}


void CGrapple::RemoveFromWorld( void ) {
	m_pHookMesh->RemoveFromWorld();
	Kill();
	m_eState = STATE_INACTIVE;
}



void CGrapple::Fire( const CFVec3A &vTgtPt ) {

	if( (m_eState != STATE_INACTIVE) || (m_fReloadTimer > 0.0f) ) {
		return;
	}

	m_vTgtPos_WS = vTgtPt;
	m_pvBasePos_WS = &(m_pMtxAttachBone->m_vPos);
	m_vPos_WS = *m_pvBasePos_WS;
	m_vLastPos_WS = m_vPos_WS;
	m_fDistTraveled = 0.0f;
	m_fXZDistTraveled = 0.0f;
	m_fUnitRadius = 1.0f;

	f32 fXZToTgt;
	CFVec3A vToTgt;
	vToTgt.Sub( m_vTgtPos_WS, m_vPos_WS );
	fXZToTgt = vToTgt.MagXZ();

    m_fXZSpacing = fmath_Div( fXZToTgt, (f32)(_NO_GRAPPLE_LINE_PTS-1) );
	DEVPRINTF( "Firing grapple, XZ spacing = %0.2f\n", m_fXZSpacing );

	m_vVel_WS.Sub( m_vTgtPos_WS, m_vPos_WS );
	FASSERT( m_vVel_WS.MagSq() > 0.01f );				// can not fire at something close.  SHould not be allowed by the sniper
	m_vVel_WS.Unitize().Mul( _FIRE_SPEED );

	m_fXZSpeed = m_vVel_WS.MagXZ();
	aiutils_ComputeTrajectoryFromXZVel( m_vVel_WS, *m_pvBasePos_WS, m_vTgtPos_WS, m_fXZSpeed );

	m_avLinePts[0] = m_vPos_WS;
	m_avStraightLinePts[0] = m_vPos_WS;
	m_uNumPtsInLine = 1;

	m_pHookMesh->DetachFromParent();
	m_pHookMesh->Relocate_RotXlatFromUnitMtx_WS( m_pMtxAttachBone );
	m_eState = STATE_LAUNCHING;
}




void CGrapple::Kill( void ) {
	m_pHookMesh->Attach_UnitMtxToParent_PS( m_pOwnerBot, m_pszAttachBoneName );
	m_uNumPtsInLine = 0;
	m_eState = STATE_INACTIVE;
	m_fReloadTimer = 0.0f;
}


void CGrapple::GetDistToGrapple( CFVec3A *pVec ) {
	FASSERT( pVec );

	pVec->Sub( m_vPos_WS, *m_pvBasePos_WS );
}


void CGrapple::Work( void ) {
	switch( m_eState ) {
		case STATE_INACTIVE:
			break;

		case STATE_LAUNCHING:
			_HandleLaunchWork();
			_MoveCableBase();
			_DroopCable();
			_CalcStraightPts();
			
			if( _CheckForHookCollision() ) {
				_CalcUpperWavePts();
				_StartTightening();
				m_eState = STATE_TIGHTENING;
			} else {
				_CheckForLineCollision();
			}
			break;

		case STATE_TIGHTENING:
			_MoveCableBase();
			_DroopCable();
			_CalcStraightPts();
			_HandleTightenWork();
			if( m_fUnitTightenFinish == 1.0f ) {
				m_eState = STATE_TIGHT;
			}
			break;

		case STATE_TIGHT:
			//METMP, this won't go here
//			_CalcStraightPts();
//			_MoveCableBase();

			_KeepCableTight();
			break;

		case STATE_RELOADING:
			if( m_fReloadTimer > 0.0f ) {
				m_fReloadTimer -= FLoop_fPreviousLoopSecs;
				FMATH_CLAMP_MIN0( m_fReloadTimer );
			} else {
				m_eState = STATE_INACTIVE;
			}
			break;

		case STATE_DETACHED:
			_DetachedWork();
			_MoveCableBase();
			_DroopCable();
			break;

		case STATE_BREAKING:
			_DroopCable();
			_BreakWork();
			break;

		default:
			FASSERT_NOW;

	}
}

#define _GRAPPLE_FIRE_SPEED				( 250.0f )
#define _GRAPPLE_MAX_DIST				( 300.0f )
#define _GRAPPLE_PT_INTERVAL			( _GRAPPLE_MAX_DIST / _NO_GRAPPLE_LINE_PTS )
#define _GRAPPLE_OO_MAX_PTS				( 1.0f / _NO_GRAPPLE_LINE_PTS )


void CGrapple::_HandleLaunchWork( void ) {
	CFVec3A vVel;
	CFVec3A vTmp;
	f32 fLastDist = m_fDistTraveled;
	f32 fLastXZDist = m_fXZDistTraveled;

	m_vVel_WS.y -= 32.0f * FLoop_fPreviousLoopSecs;

	vVel.Mul( m_vVel_WS, FLoop_fPreviousLoopSecs );

	m_fDistTraveled += _GRAPPLE_FIRE_SPEED * FLoop_fPreviousLoopSecs;
	m_fXZDistTraveled += m_fXZSpeed * FLoop_fPreviousLoopSecs;

	m_vLastPos_WS = m_vPos_WS;
	m_vPos_WS.Add( vVel );

	if( (u32)fmath_Div( fLastXZDist, m_fXZSpacing ) < (u32)fmath_Div( m_fXZDistTraveled, m_fXZSpacing ) ) {
		FASSERT( m_uNumPtsInLine < _NO_GRAPPLE_LINE_PTS );
//		m_afUnitLineTimer[m_uNumPtsInLine] = 0.0f;
		m_avLinePts[m_uNumPtsInLine++] = m_vPos_WS;
	}

	if( m_fDistTraveled > _GRAPPLE_MAX_DIST ) {
		_Detach();
		//Break();
	}

	m_pHookMesh->Relocate_Xlat_WS( &m_vPos_WS );
}

#define _LINE_SNAP_UP_SPEED_START			( 2.0f )
#define _LINE_SNAP_UP_TIME_START			( 1.0f / _LINE_SNAP_UP_SPEED_START )
#define _LINE_SNAP_UP_SPEED_FINISH			( 4.0f )
#define _LINE_SNAP_UP_TIME_FINISH			( 1.0f / _LINE_SNAP_UP_SPEED_FINISH )

#define _FINISH_TIGHTEN_THRESHOLD	( 0.66f )

void CGrapple::_HandleTightenWork( void ) {
	FASSERT( m_uNumPtsInLine > 1 );

	m_fUnitTightenStart += FLoop_fPreviousLoopSecs * _LINE_SNAP_UP_SPEED_START;
	FMATH_CLAMP_MAX1( m_fUnitTightenStart );

	if( m_fUnitTightenStart > _FINISH_TIGHTEN_THRESHOLD ) {
		m_fUnitTightenFinish += FLoop_fPreviousLoopSecs * _LINE_SNAP_UP_SPEED_FINISH;
		FMATH_CLAMP_MAX1( m_fUnitTightenFinish );
	}

	u32 i;
	f32 fUnitPtDist;
	f32 fUnitForThisPt;
	CFVec3A vDelta;

	for( i=0; i<m_uNumPtsInLine; i++ ) {
		fUnitPtDist = fmath_Div( (f32)i, (f32)(m_uNumPtsInLine-1));

		if( fUnitPtDist < 1.0f ) {
			fUnitForThisPt = fmath_Div( m_fUnitTightenStart - fUnitPtDist, 1.0f - fUnitPtDist );
			FMATH_CLAMP_MIN0( fUnitForThisPt );
		} else {
			fUnitForThisPt = 1.0f;
		}

		vDelta.Sub( m_avTopWavePts[i], m_avLinePts[i] );
		vDelta.Mul( fUnitForThisPt );
		m_avLinePts[i].Add( vDelta );
	}

	if( m_fUnitTightenFinish > 0.0f ) {
		for( i=0; i<m_uNumPtsInLine; i++ ) {
			fUnitPtDist = fmath_Div( (f32)i, (f32)(m_uNumPtsInLine-1));

			if( fUnitPtDist < 1.0f ) {
				fUnitForThisPt = fmath_Div( m_fUnitTightenFinish - fUnitPtDist, 1.0f - fUnitPtDist );
				FMATH_CLAMP_MIN0( fUnitForThisPt );
			} else {
				fUnitForThisPt = 1.0f;
			}

			vDelta.Sub( m_avStraightLinePts[i], m_avLinePts[i] );
			vDelta.Mul( fUnitForThisPt );
			m_avLinePts[i].Add( vDelta );
		}
	}
}


void CGrapple::_DroopCable( void ) {

	// should change to 1 -> pts-1
	for( u32 i=0; i<m_uNumPtsInLine; i++ ) {
		m_avLinePts[i].y -= FLoop_fPreviousLoopSecs * 0.33f * 32.0f * fmath_Sin( FMATH_PI * (f32)i * _OO_NUM_GRAPPLE_PTS );
	}
}

void CGrapple::_KeepCableTight( void ) {
	m_uNumPtsInLine = 2;
	m_avLinePts[0] = *m_pvBasePos_WS;
	m_avLinePts[1] = m_vPos_WS;
	m_afUnitDistOnLine[0] = 0.0f;
	m_afUnitDistOnLine[1] = 1.0f;

}

void CGrapple::_CalcStraightPts( void ) {
	CFVec3A vUnitToPt;
	CFVec3A vToTgt;
	f32 fPtMag;
	CFVec3A vTmp;
	f32 fOOLineLength;
	
	vToTgt.Sub( m_vTgtPos_WS, *m_pvBasePos_WS );
	
	// should not be this close
	FASSERT( vToTgt.MagSq() > 0.01f );

	fOOLineLength = fmath_Inv( vToTgt.Mag() );
	vUnitToPt.Mul( vToTgt, fOOLineLength );
    
	for( u32 i=0; i<m_uNumPtsInLine; i++ ) {
		// calc the straight line points
		vTmp.Sub( m_avLinePts[i], *m_pvBasePos_WS );
		fPtMag = vUnitToPt.Dot( vTmp );
		m_avStraightLinePts[i].Mul( vUnitToPt, fPtMag ).Add( *m_pvBasePos_WS );
		m_afUnitDistOnLine[i] = fPtMag * fOOLineLength;
		FMATH_CLAMP_UNIT_FLOAT( m_afUnitDistOnLine[i] );
	}

}

#define _TOP_WAV_DIST		( 5.0f )

void CGrapple::_CalcUpperWavePts( void ) {
	CFVec3A vTmp;
	f32 fMag2;
	f32 fSinVal;

	for( u32 i=0; i<m_uNumPtsInLine; i++ ) {
		// not real accurate, but should be ok
		fSinVal = fmath_Sin( FMATH_PI * fmath_Div( (f32)i, (f32)m_uNumPtsInLine ) );

		// calc the straight line points
		vTmp.Sub( m_avStraightLinePts[i], m_avLinePts[i] );
		fMag2 = vTmp.MagSq();
		if( fMag2 > 0.0001f ) {
			vTmp.Mul( fmath_InvSqrt( fMag2 ) * fSinVal * _TOP_WAV_DIST );            
		} else {
			vTmp.Zero();
		}

		m_avTopWavePts[i].Add( m_avStraightLinePts[i], vTmp );
	}

}


void CGrapple::_StartTightening( void ) {
	//CFVec3A vToTgt;
	//	
	m_fUnitTightenStart = 0.0f;
	m_fUnitTightenFinish = 0.0f;

	//vToTgt.Sub( m_vPos_WS, m_pMtxAttachBone->m_vPos );
	//m_fFinalDist = vToTgt.SafeUnitAndMag( vToTgt );
	//if( m_fFinalDist <= 0.0f ) {
	//	FASSERT_NOW;
	//	return;
	//}

	//m_fFinalInterval = fmath_Div( m_fFinalDist, (f32)(m_uNumPtsInLine-1) );

	//for( u32 i=0; i<m_uNumPtsInLine; i++ ) {
	//	m_avStraightLinePts[i].Mul( vToTgt, m_fFinalInterval * (f32)i ).Add( m_pMtxAttachBone->m_vPos );
	//}
}


void CGrapple::_MoveCableBase( void ) {
	if( m_uNumPtsInLine == 0 ) {
		return;
	}

	CFVec3A vCableDelta;
	vCableDelta.Sub( *m_pvBasePos_WS, m_avLinePts[0] );

	f32 fMag = vCableDelta.Mag();
	if( fMag < 0.0001f ) {
		// haven't moved very far
		return;
	}

	CFVec3A vPtDelta;

	vCableDelta.Mul( fmath_Inv( fMag ) );

	for( u32 i=0; i<m_uNumPtsInLine-1; i++ ) {
		vPtDelta.Mul( vCableDelta, fMag * fmath_Div( (f32)(m_uNumPtsInLine-i-1), (f32)(m_uNumPtsInLine-1 )) );
		m_avLinePts[i].Add( vPtDelta );
		m_avTopWavePts[i].Add( vPtDelta );
	}
	
}


void CGrapple::_Draw( void ) {
	u32 i;


//	for( i=1; i<m_uNumPtsInLine; i++ ) {
//		fdraw_SolidLine( &(m_avLinePts[i-1].v3), &(m_avLinePts[i].v3), &FColor_MotifBlue );
//		if( m_eState == STATE_TIGHTENING ) {
////			fdraw_SolidLine( &(m_avStraightLinePts[i-1].v3), &(m_avStraightLinePts[i].v3), &FColor_MotifGreen );
//		}
//	}

	if( m_eState == STATE_TIGHTENING ) {
		CFVec3A vPt0, vPt1;
		CFVec3A vToTgtPt;
		
		for( i=1; i<m_uNumPtsInLine; i++ ) {
   //         vPt0.Set( m_avStraightLinePts[i-1].x, 
			//		  m_avStraightLinePts[i-1].y + 6.0f * fmath_Sin( FMATH_PI * fmath_Div( ((f32)(i-1)), (f32)m_uNumPtsInLine ) ),
			//		  m_avStraightLinePts[i-1].z );

			//vPt1.Set( m_avStraightLinePts[i].x, 
			//		  m_avStraightLinePts[i].y + 6.0f * fmath_Sin( FMATH_PI * fmath_Div( ((f32)(i)), (f32)m_uNumPtsInLine ) ),
			//		  m_avStraightLinePts[i].z );
			//fdraw_SolidLine( &(vPt0.v3), &(vPt1.v3), &FColor_MotifRed );
		}

		CFVec3A vToTgt;
		vToTgt.Sub( m_vPos_WS, *m_pvBasePos_WS );
		vToTgt.Mul( m_fUnitTightenStart ).Add( *m_pvBasePos_WS );
//		fdraw_FacetedWireSphere( &(vToTgt.v3), 2.0f, 4, 4, &FColor_MotifGreen );

		vToTgt.Sub( m_vPos_WS, *m_pvBasePos_WS );
		vToTgt.Mul( m_fUnitTightenFinish ).Add( *m_pvBasePos_WS );
//		fdraw_FacetedWireSphere( &(vToTgt.v3), 2.0f, 4, 4, &FColor_MotifBlue );
	}



	////TEST... calc where the line should be
	//if( m_eState == STATE_TIGHTENING ) {
	//	CFVec3A vPt0, vPt1;
	//	CFVec3A vToTgtPt;
	//	vToTgtPt.Sub( m_vPos_WS, m_pMtxAttachBone->m_vPos );
	//	f32 fTotalDist = vToTgtPt.UnitAndMag( vToTgtPt );

	//	for( i=1; i<m_uNumPtsInLine; i++ ) {
	//		// calc where the point should be if the line were tight...
	//		vPt0.Mul( vToTgtPt, ((f32)i-1) * _GRAPPLE_PT_INTERVAL ).Add( m_pMtxAttachBone->m_vPos );
	//		vPt1.Mul( vToTgtPt, ((f32)i) * _GRAPPLE_PT_INTERVAL ).Add( m_pMtxAttachBone->m_vPos );
	//		fdraw_SolidLine( &(vPt0.v3), &(vPt1.v3), &FColor_MotifGreen );

	//		vPt0.y += 10.0f * fmath_Sin( FMATH_PI * fmath_Div( ((f32)(i-1))*_GRAPPLE_PT_INTERVAL, fTotalDist ) );
	//		vPt1.y += 10.0f * fmath_Sin( FMATH_PI * fmath_Div( ((f32)i) *_GRAPPLE_PT_INTERVAL, fTotalDist ) );
	//		fdraw_SolidLine( &(vPt0.v3), &(vPt1.v3), &FColor_MotifRed );
	//	}
	//}

	//for( u32 i=0; i<m_uNumPtsInLine; i++ ) {
	//	fdraw_FacetedWireSphere( &(m_avLinePts[i].v3), 0.25f, 4, 4, &FColor_MotifRed );
	//	fdraw_FacetedWireSphere( &(m_avStraightLinePts[i].v3), 0.25f, 4, 4, &FColor_MotifBlue );
	//	if( m_eState == STATE_TIGHT ) {
	//		fdraw_FacetedWireSphere( &(m_avTopWavePts[i].v3), 0.25f, 4, 4, &FColor_MotifGreen );
	//	}

	//}

	if( m_uNumPtsInLine > 0 ) {
		fdraw_FacetedWireSphere( &(m_vPos_WS.v3), 0.5f, 4, 4, &FColor_MotifBlue );
	}

}


BOOL CGrapple::_CheckForHookCollision( void ) {
	CFCollData collData;
	CFVec3A vMovement;

	vMovement.Sub( m_vPos_WS, m_vLastPos_WS );
	
	CFSphere sphere;
	sphere.Set( m_vLastPos_WS.v3, 1.0f );

	collData.nCollMask = FCOLL_MASK_CHECK_ALL;
	collData.nFlags	= FCOLL_DATA_IGNORE_BACKSIDE;
	collData.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
	collData.pCallback = _IntersectingTrackerCallback;
	collData.pLocationHint = m_pHookMesh->GetMeshInst();
	collData.pMovement = &vMovement;


	fcoll_Clear();
	FWorld_nTrackerSkipListCount = 0;
	m_pOwnerBot->AppendTrackerSkipList();

	if( fcoll_Check( &collData, &sphere ) ) {
		fcoll_SortDynamic( TRUE );
		m_vPos_WS = FColl_apSortedImpactBuf[0]->ImpactPoint;
		m_vLastPos_WS = FColl_apSortedImpactBuf[0]->ImpactPoint;
		return TRUE;
	}

	return FALSE;
}


//-----------------------------------------------------------------------------
// fcoll now expects a collision callback result bitfield rather than a BOOL.
u32 CGrapple::_IntersectingTrackerCallback( CFWorldTracker *pTracker ) {
	for( u32 i=0; i < FWorld_nTrackerSkipListCount; i++ )
	{
		if( FWorld_apTrackerSkipList[i] == pTracker )
		{
			return FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER;
		}
	}
	return FCOLL_CHECK_CB_ALL_IMPACTS;
}


void CGrapple::_DrawCable( void ) {
	if( m_uNumPtsInLine < 2 ) {
		return;
	}

	FDrawVtx_t *pVtxArray;
	CFVec3A vTmp;
	CFVec3A vScaledPt;
	CFXfm Xfm;
	f32 fDist;

	u32 uVtxCount = (m_uNumPtsInLine-1) * 6 * 2;
	pVtxArray = fvtxpool_GetArray( uVtxCount );

	if( !pVtxArray ) {
		// couldn't get any, try again next frame
		return;
	}

	Xfm.BuildFromMtx( CFMtx43A::m_IdentityMtx );
	Xfm.PushModel();

	fdraw_Depth_EnableWriting( TRUE );
	//fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_SRC );
	//fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_SetCullDir( FDRAW_CULLDIR_CW );

	fdraw_SetTexture( &m_CableTex );

	CFMtx43A::m_Temp.m_vPos.Zero();
	FDrawVtx_t *pVtx = pVtxArray;
	FDrawVtx_t *pVtxSegment;

	f32 ftest = m_avLinePts[0].Dist( m_avLinePts[1] );
	CFColorRGB nearColor, farColor;
	fworld_LightPoint( &m_avLinePts[0], &nearColor, TRUE );
	fworld_LightPoint( &m_avLinePts[m_uNumPtsInLine-1], &farColor, TRUE );

	CFColorRGB v1Color, v2Color;

	for( u32 i=0; i<m_uNumPtsInLine-1; i++ ) {			// points in the line
		pVtxSegment = pVtx;
		f32 fS = 0.0f;
		
		v1Color.Set( FMATH_FPOT( m_afUnitDistOnLine[i], nearColor.fRed, farColor.fRed ),
					 FMATH_FPOT( m_afUnitDistOnLine[i], nearColor.fGreen, farColor.fGreen ),
					 FMATH_FPOT( m_afUnitDistOnLine[i], nearColor.fBlue, farColor.fBlue ) );

		v2Color.Set( FMATH_FPOT( m_afUnitDistOnLine[i+1], nearColor.fRed, farColor.fRed ),
					 FMATH_FPOT( m_afUnitDistOnLine[i+1], nearColor.fGreen, farColor.fGreen ),
					 FMATH_FPOT( m_afUnitDistOnLine[i+1], nearColor.fBlue, farColor.fBlue ) );

		for( u32 j=0; j<6; j++ ) {						// vertexes per point
			vTmp.Sub( m_avLinePts[i+1], m_avLinePts[i] );
			fDist = vTmp.Mag();
			if( fDist > 0.01f ) {
				vTmp.Mul( fmath_Inv( fDist ) );
				CFMtx43A::m_Temp.UnitMtxFromUnitVec( &vTmp );

				vScaledPt.Mul( m_avPentagonVtx[j], m_fUnitRadius );
				CFMtx43A::m_Temp.MulPoint( pVtx->Pos_MS, vScaledPt.v3 );
				pVtx->Pos_MS.Add( m_avLinePts[i].v3 );

				pVtx->ColorRGBA.Set( v1Color.fRed * m_afPentagonVtxIntensity[j],
									 v1Color.fGreen * m_afPentagonVtxIntensity[j],
									 v1Color.fBlue * m_afPentagonVtxIntensity[j], 
									 1.0f );
				pVtx->ST.Set( fS, 0.0f );
				pVtx++;

				vScaledPt.Mul( m_avPentagonVtx[j], m_fUnitRadius );
				CFMtx43A::m_Temp.MulPoint( pVtx->Pos_MS, vScaledPt.v3 );
				pVtx->Pos_MS.Add( m_avLinePts[i+1].v3 );
				//pVtx->ColorRGBA.OpaqueWhite();
				pVtx->ColorRGBA.Set( v2Color.fRed * m_afPentagonVtxIntensity[j],
									 v2Color.fGreen * m_afPentagonVtxIntensity[j],
									 v2Color.fBlue * m_afPentagonVtxIntensity[j], 
									 1.0f );
				pVtx->ST.Set( fS, fDist * 0.5f );
				pVtx++;
				fS += 0.2f;
			}
		}

		fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, pVtxSegment, 12 );
	}

	CFXfm::PopModel();
	fvtxpool_ReturnArray( pVtxArray );
}


void CGrapple::_Detach( void ) {
	m_eState = STATE_DETACHED;
	m_vDetachedBasePos_WS = *m_pvBasePos_WS;
	m_pvBasePos_WS = &m_vDetachedBasePos_WS;
	m_fUnitDeathTimer = 1.0f;
	m_fUnitRadius = 1.0f;

}

#define _DETACH_DEATH_TIME			( 0.5f )
#define _DETACH_DEATH_OO_TIME		( 1.0f / _DETACH_DEATH_TIME )
#define _BREAK_DEATH_TIME			( 0.5f )
#define _BREAK_DEATH_OO_TIME		( 1.0f / _BREAK_DEATH_TIME )


void CGrapple::_DetachedWork( void ) {
	CFVec3A vTmp;
	vTmp.Mul( m_vVel_WS, FLoop_fPreviousLoopSecs );
	m_vDetachedBasePos_WS.Add( vTmp );
	if( m_fUnitDeathTimer > 0.0f ) {
		m_fUnitDeathTimer -= FLoop_fPreviousLoopSecs * _DETACH_DEATH_OO_TIME;
		FMATH_CLAMP_MIN0( m_fUnitDeathTimer );
	} else {
		Kill();
	}
	m_fUnitRadius = m_fUnitDeathTimer;

}

#define _NUM_COLLISION_RAYS			( 3 )


void CGrapple::_CheckForLineCollision( void ) {
	if( m_uNumPtsInLine < 2 ) {
		return;
	}

    u32 uTestInterval = (m_uNumPtsInLine-1) / _NUM_COLLISION_RAYS;

	if( uTestInterval == 0 ) {
		return;
	}

	FCollImpact_t impact;
	CFVec3A *pStart, *pEnd;
    
	for( u32 i=0; i<_NUM_COLLISION_RAYS; i++ ) {
        pStart = &m_avLinePts[i*uTestInterval];
		pEnd = &m_avLinePts[(i+1)*uTestInterval];
		if( fworld_FindClosestImpactPointToRayStart( &impact, pStart, pEnd, 0, NULL, TRUE, NULL, FCOLL_USER_TYPE_BITS_ALL ) ) {
			_BreakLine( &(impact.ImpactPoint) );
			//DEVPRINTF( "Breaking grapple at %0.2f, %0.2f, %0.2f\n", impact.ImpactPoint.x, impact.ImpactPoint.y, impact.ImpactPoint.z );
		}
	}
}


void CGrapple::_BreakLine( const CFVec3A *pvBreakPos_WS /*=NULL*/ ) {
	// if the line is short enough, just kill it
	if( m_uNumPtsInLine < 2 ) {
		Kill();
		return;
	}

	m_eState = STATE_BREAKING;
	m_fUnitDeathTimer = 1.0f;
	m_fUnitRadius = 1.0f;

	if( (m_uNumPtsInLine < 3) || !pvBreakPos_WS ) {
		m_uBreakPointClose = 0;
		m_uBreakPointFar = 1;
		return;
	}

	// find which point line should break on
	if( pvBreakPos_WS ) {
		f32 fDistFromStartSq = m_pvBasePos_WS->DistSq( *pvBreakPos_WS );
		for( u32 i=1; i<m_uNumPtsInLine-1; i++ ) {
			if( m_pvBasePos_WS->DistSq( m_avLinePts[i] ) > fDistFromStartSq ) {
				m_uBreakPointClose = i;
				m_uBreakPointFar = i+1;
				return;
			}
		}
	} else {
		m_uBreakPointClose = fmath_RandomRange( 1, m_uNumPtsInLine - 1 );
		m_uBreakPointFar = m_uBreakPointClose+1;
	}
}


void CGrapple::_BreakWork( void ) {
	if( m_fUnitDeathTimer > 0.0f ) {
		m_fUnitDeathTimer -= FLoop_fPreviousLoopSecs * _BREAK_DEATH_OO_TIME;
		FMATH_CLAMP_MIN0( m_fUnitDeathTimer );
	} else {
		Kill();
	}
	m_fUnitRadius = m_fUnitDeathTimer;
}
