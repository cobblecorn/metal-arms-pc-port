//////////////////////////////////////////////////////////////////////////////////////
// fcamanim.cpp - Fang camera animation module.
//
// Author: Russell Foushee     
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
// 01/23/03 Foushee     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fcamanim.h"
#include "fresload.h"
#include "fdata.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// LOCAL VARIABLES

static BOOL _bModuleInitialized;
static FResLoadReg_t _ResLoadRegistration;


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// LOCAL FUNCTIONS

static BOOL _ResLoadCreate( FResHandle_t hRes, void *pLoadedBase, u32 nLoadedBytes, cchar *pszResName );

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// PUBLIC INTERFACE FUNCTIONS

BOOL fcamanim_ModuleStartup( void ) {
	FASSERT( !_bModuleInitialized );

	//register the camera animation loading type
	fres_CopyType( _ResLoadRegistration.sResType, FCAMANIM_RESTYPE );
	_ResLoadRegistration.pszFileExtension = "cam";
	_ResLoadRegistration.nMemType = FRESLOAD_MEMTYPE_PERM;
	_ResLoadRegistration.nAlignment = FCLASS_BYTE_ALIGN;
	_ResLoadRegistration.pFcnCreate = _ResLoadCreate;
	_ResLoadRegistration.pFcnDestroy = NULL;

	if( !fresload_RegisterHandler( &_ResLoadRegistration ) ) {
		// Registration failed...
		DEVPRINTF( "fcamanim_ModuleStartup(): Could not register resource.\n" );
		goto _ExitModuleStartup;
	}

	// Success...

	_bModuleInitialized = TRUE;

	return TRUE;

_ExitModuleStartup:
	// Failure...
	return FALSE;
}

void fcamanim_ModuleShutdown( void ) {
	if( _bModuleInitialized ) {
		_bModuleInitialized = FALSE;
	}
}



//-------------------------------------------------------------------------------------------------------------------
// Camera Animation Resource Object:
//-------------------------------------------------------------------------------------------------------------------

static BOOL _ResLoadCreate( FResHandle_t hRes, void *pLoadedBase, u32 nLoadedBytes, cchar *pszResName ) {

	FASSERT( pLoadedBase );

	#define __FIXUP_POINTER( pointer, type )	if (pointer) pointer = (type *)((u32)pLoadedBase + (u32)pointer)

	FCamAnim_t *pCamAnim = (FCamAnim_t *)pLoadedBase;
	__FIXUP_POINTER( pCamAnim->paKeyTimes, FCamAnimKeyTime_t );
	__FIXUP_POINTER( pCamAnim->paFKeyTimeIdx, FCamAnimKeyTimeIdx_t );
	__FIXUP_POINTER( pCamAnim->paTKeyTimeIdx, FCamAnimKeyTimeIdx_t );
	__FIXUP_POINTER( pCamAnim->paOKeyTimeIdx, FCamAnimKeyTimeIdx_t );
	__FIXUP_POINTER( pCamAnim->paFKeyData, FCamAnimFKey_t );
	__FIXUP_POINTER( pCamAnim->paTKeyData, FCamAnimTKey_t );
	__FIXUP_POINTER( pCamAnim->paOKeyData, FCamAnimOKey_t );

	fres_SetBase( hRes, pCamAnim );

	#undef __FIXUP_POINTER

	return TRUE;
}



//-------------------------------------------------------------------------------------------------------------------
// CFCamAnimInst:
//-------------------------------------------------------------------------------------------------------------------

CFCamAnimInst *CFCamAnimInst::Load( cchar *pszCamAnimResName ) {
	FCamAnim_t *pCamAnimRes;
	CFCamAnimInst *pCamAnimInst = NULL;

	FResFrame_t ResFrame = fres_GetFrame();

	pCamAnimRes = (FCamAnim_t *)fresload_Load( FCAMANIM_RESTYPE, pszCamAnimResName );

	if( pCamAnimRes == NULL ) {
		DEVPRINTF( "CFCamAnimInst::Load(): Could not find camera animation '%s'\n", pszCamAnimResName );
		goto _ExitLoadWithError;
	}

	pCamAnimInst = fnew CFCamAnimInst;

	if( pCamAnimInst == NULL ) {
		DEVPRINTF( "CFCamAnimInst::Load(): Not enough memory to create CFCamAnimInst for animation '%s'\n", pszCamAnimResName );
		goto _ExitLoadWithError;
	}

	if( !pCamAnimInst->Create( pCamAnimRes ) ) {
		DEVPRINTF( "CFCamAnimInst::Load(): Trouble creating CFCamAnimInst for  '%s'\n", pszCamAnimResName );
		goto _ExitLoadWithError;
	}

	return pCamAnimInst;

	// Error:
_ExitLoadWithError:
	fdelete( pCamAnimInst );
	fres_ReleaseFrame( ResFrame );
	return NULL;
}


CFCamAnimInst::CFCamAnimInst() { 
	m_pCamAnim=NULL;
	Reset(); //resets the animation variables;
}


void CFCamAnimInst::Reset( void ) {
	m_fCamAnimSeconds=0.0f; 
	m_fUnitCamAnimSecs = 0.f; 

	m_uCurrentFIdx = 0;
	m_uCurrentTIdx = 0;
	m_uCurrentOIdx = 0;
}


BOOL CFCamAnimInst::Create( FCamAnim_t *pCamAnim ) 
{
	FASSERT( pCamAnim );

	m_pCamAnim = pCamAnim;

	return TRUE;
}



void CFCamAnimInst::UpdateTime( f32 fNewTime ) 
{
//	f32 fDeltaTime;

	FMATH_DEBUG_FCHECK( fNewTime );

	if( fNewTime < 0.0f ) {
		fNewTime = 0.0f;
	}

	if( fNewTime > GetTotalTime() ) {
		fNewTime = GetTotalTime();
	}

	m_fCamAnimSeconds = fNewTime;

}



void CFCamAnimInst::DeltaTime( f32 fDeltaTime, BOOL bClamp ) {
	f32 fNewTime;

	FMATH_DEBUG_FCHECK( fDeltaTime );

	fNewTime = GetTime() + fDeltaTime;
	if( !bClamp ) {
		// we want to wrap the animation
		if( fNewTime > GetTotalTime() ) {
			fNewTime -= GetTotalTime();
		}
	}
	UpdateTime( fNewTime );
}



void CFCamAnimInst::ComputeFrame( f32 &fFOV, CFQuatA &qOrient, CFVec3 &vPos ) {
	//take the current time and figure out where we are in

	//check the time bounds first...
	if( m_fCamAnimSeconds <= 0.0f ) {
		m_uCurrentFIdx = 0;
		m_uCurrentTIdx = 0;
		m_uCurrentOIdx = 0;

		fFOV = m_pCamAnim->paFKeyData[ m_uCurrentFIdx ].fFOV;
		vPos = m_pCamAnim->paTKeyData[ m_uCurrentTIdx ].vPosition;
		qOrient = m_pCamAnim->paOKeyData[ m_uCurrentOIdx ].qOrientation;
	}
	else if( m_fCamAnimSeconds >= m_pCamAnim->fTotalSeconds ) {
		m_uCurrentFIdx = m_pCamAnim->nFKeyCount - 1;
		m_uCurrentTIdx = m_pCamAnim->nTKeyCount - 1;
		m_uCurrentOIdx = m_pCamAnim->nOKeyCount - 1;

		fFOV = m_pCamAnim->paFKeyData[ m_uCurrentFIdx ].fFOV;
		vPos = m_pCamAnim->paTKeyData[ m_uCurrentTIdx ].vPosition;
		qOrient = m_pCamAnim->paOKeyData[ m_uCurrentOIdx ].qOrientation;
	}
	else {
		//see if our indexes have fallen behind the animation time....
		while ( ( m_uCurrentFIdx < m_pCamAnim->nFKeyCount - 1 ) && 
			    m_pCamAnim->paKeyTimes[ m_pCamAnim->paFKeyTimeIdx[ m_uCurrentFIdx + 1 ].uIdx ].fTime < m_fCamAnimSeconds ) {  
			m_uCurrentFIdx++;
		}
		while ( ( m_uCurrentTIdx < m_pCamAnim->nTKeyCount - 1 ) &&
				m_pCamAnim->paKeyTimes[ m_pCamAnim->paTKeyTimeIdx[ m_uCurrentTIdx + 1 ].uIdx ].fTime < m_fCamAnimSeconds ) {  
			m_uCurrentTIdx++;
		}
		while ( ( m_uCurrentOIdx < m_pCamAnim->nOKeyCount - 1 ) &&
				m_pCamAnim->paKeyTimes[ m_pCamAnim->paOKeyTimeIdx[ m_uCurrentOIdx + 1 ].uIdx ].fTime < m_fCamAnimSeconds ) {  
			m_uCurrentOIdx++;
		}

		//see if our indexes have crept ahead of the current animation time....
		while ( ( m_uCurrentFIdx > 0 ) &&
				m_pCamAnim->paKeyTimes[ m_pCamAnim->paFKeyTimeIdx[ m_uCurrentFIdx ].uIdx ].fTime > m_fCamAnimSeconds ) {  
			m_uCurrentFIdx--;
		}
		while ( ( m_uCurrentTIdx > 0 ) &&
				m_pCamAnim->paKeyTimes[ m_pCamAnim->paTKeyTimeIdx[ m_uCurrentTIdx ].uIdx ].fTime > m_fCamAnimSeconds ) {  
			m_uCurrentTIdx--;
		}
		while ( ( m_uCurrentOIdx > 0 ) &&
				m_pCamAnim->paKeyTimes[ m_pCamAnim->paOKeyTimeIdx[ m_uCurrentOIdx ].uIdx ].fTime > m_fCamAnimSeconds ) {  
			m_uCurrentOIdx--;
		}

		//now, calculate the FOV
		f32 fKeytime1, fKeytime2;
		f32 fFOV1, fFOV2;
		fFOV1 = m_pCamAnim->paFKeyData[ m_uCurrentFIdx ].fFOV;
		fFOV2 = m_pCamAnim->paFKeyData[ m_uCurrentFIdx + 1 ].fFOV;
		if( fFOV1 == fFOV2 ) {
			fFOV = fFOV1;
		}
		else {
			fKeytime1 = m_pCamAnim->paKeyTimes[ m_pCamAnim->paFKeyTimeIdx[ m_uCurrentFIdx ].uIdx ].fTime;
			fKeytime2 = m_pCamAnim->paKeyTimes[ m_pCamAnim->paFKeyTimeIdx[ m_uCurrentFIdx + 1 ].uIdx ].fTime;
			f32 fFOVInc = ( fFOV2 - fFOV1 ) / ( fKeytime2 - fKeytime1 );
			fFOV = fFOV1 + fFOVInc * ( m_fCamAnimSeconds - fKeytime1 );
			FASSERT( fFOV > 0.0 );
		}

		//now, calculate the Translation
		fKeytime1 = m_pCamAnim->paKeyTimes[ m_pCamAnim->paTKeyTimeIdx[ m_uCurrentTIdx ].uIdx ].fTime;
		fKeytime2 = m_pCamAnim->paKeyTimes[ m_pCamAnim->paTKeyTimeIdx[ m_uCurrentTIdx + 1 ].uIdx ].fTime;
		if( m_pCamAnim->paTKeyData[ m_uCurrentTIdx ].vPosition == m_pCamAnim->paTKeyData[ m_uCurrentTIdx + 1 ].vPosition ) {
			vPos = m_pCamAnim->paTKeyData[ m_uCurrentTIdx ].vPosition;
		}
		else {
			//we need to interpolate...
			f32 fSlider = ( m_fCamAnimSeconds - fKeytime1 ) / ( fKeytime2 - fKeytime1 );
			vPos.ReceiveLerpOf( fSlider,
				m_pCamAnim->paTKeyData[ m_uCurrentTIdx ].vPosition,
				m_pCamAnim->paTKeyData[ m_uCurrentTIdx + 1 ].vPosition );
		}

		//now, calculate the orientation
		fKeytime1 = m_pCamAnim->paKeyTimes[ m_pCamAnim->paOKeyTimeIdx[ m_uCurrentOIdx ].uIdx ].fTime;
		fKeytime2 = m_pCamAnim->paKeyTimes[ m_pCamAnim->paOKeyTimeIdx[ m_uCurrentOIdx + 1 ].uIdx ].fTime;
		
		//for some reason, you can't compare a quat4a for equality on the GC... skip the test...
#if !FANG_PLATFORM_GC
		if( m_pCamAnim->paOKeyData[ m_uCurrentOIdx ].qOrientation == m_pCamAnim->paOKeyData[ m_uCurrentOIdx + 1 ].qOrientation ) {
			qOrient = m_pCamAnim->paOKeyData[ m_uCurrentOIdx ].qOrientation;
		}
		else {
#endif
			//we need to interpolate...
			f32 fSlider = ( m_fCamAnimSeconds - fKeytime1 ) / ( fKeytime2 - fKeytime1 );
			qOrient.ReceiveSlerpOf( fSlider,
				m_pCamAnim->paOKeyData[ m_uCurrentOIdx ].qOrientation,
				m_pCamAnim->paOKeyData[ m_uCurrentOIdx + 1 ].qOrientation );
#if !FANG_PLATFORM_GC
		}
#endif
	}

	// The last thing to do is half the FOV because this
	// function passes back a Half FOV...
	fFOV *= 0.5f;
}
