//////////////////////////////////////////////////////////////////////////////////////
// CamCutscene.cpp - 
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
// 01/24/03 Foushee		Created.
//////////////////////////////////////////////////////////////////////////////////////

// =============================================================================================================

#include "CamCutscene.h"
#include "floop.h"

// =============================================================================================================

CCamCutscene::CCamCutscene()
{
	Reset();
}

// =============================================================================================================

// This is a static function.
BOOL CCamCutscene::InitSystem()
{
	return(TRUE);
}

// =============================================================================================================

// This is a static function.
void CCamCutscene::UninitSystem()
{
}

// =============================================================================================================

BOOL CCamCutscene::Init(CCamCutsceneInfo *pCamInfo)
{
	if(m_pCamera == NULL)
	{
		DEVPRINTF("CCamCutscene::Init() : No camera assigned, call AssignCamera() before Init().\n" );
		return(FALSE);
	}

	if(pCamInfo == NULL)
	{
		DEVPRINTF("CCamCutscene::Init() : CCamCutscene requires a CCamCutsceneInfo to init.\n");
		return(FALSE);
	}

	if( pCamInfo->m_pCamAnimInst == NULL ) {
		DEVPRINTF("CCamCutscene::Init() : NULL CFCamAnimInst passed into CamInfo.\n");
		return(FALSE);
	}

	Reset();
	m_pCamInfo = pCamInfo;

	return(TRUE);
}

// =============================================================================================================

void CCamCutscene::Reset()
{
	m_pCamInfo = NULL;
}

// =============================================================================================================

void CCamCutscene::Work()
{
	if(m_pCamInfo == NULL)
	{
		return;
	}

	if(m_bFreezeShot)
	{
		return;
	}

	if( m_pCamInfo->m_pCamAnimInst != NULL )
	{
		//drive the animation instance here...
		if( m_pCamInfo->m_bDriveAnimInst ) {
			m_pCamInfo->m_pCamAnimInst->DeltaTime(FLoop_fPreviousLoopSecs, TRUE);
		}

		//update the camera position and orientation here...
		m_pCamInfo->m_pCamAnimInst->ComputeFrame( m_fHalfFOV, m_qOrient, m_vPos );
		m_pCameraData->m_Xfm.BuildFromInvQuat( m_qOrient, m_vPos );

		//update the camera FOV here...
		m_pCamera->SetFOV( m_fHalfFOV ); //this routine expects a HALF fov.

		if( m_pCamInfo->m_pCamAnimInst->GetUnitTime() >= 0.9999f ) {
			//this animation is done... set the CamInfo->m_bDone Flag
			m_pCamInfo->m_bDone = TRUE;
		}
	}
}

// =============================================================================================================
