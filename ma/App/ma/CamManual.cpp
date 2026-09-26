//////////////////////////////////////////////////////////////////////////////////////
// CamManual.cpp - 
//
// Author: Justin Link
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
// 04/01/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

// =============================================================================================================

#include "CamManual.h"
#include "FVec3Obj.h"
#include "FQuatObj.h"
#include "SplitScreen.h"

// =============================================================================================================

CCamManual::CCamManual()
{
	Reset();
}

// =============================================================================================================

// This is a static function.
BOOL CCamManual::InitSystem()
{
	return(TRUE);
}

// =============================================================================================================

// This is a static function.
void CCamManual::UninitSystem()
{
}

// =============================================================================================================

BOOL CCamManual::Init(CCamManualInfo *pCamInfo)
{
	if(m_pCamera == NULL)
	{
		DEVPRINTF("CCamManual::Init() : No camera assigned, call AssignCamera() before Init().\n" );
		return(FALSE);
	}

	if(pCamInfo == NULL)
	{
		DEVPRINTF("CCamManual::Init() : CCamManual requires a CCamManualInfo to init.\n");
		return(FALSE);
	}

	if((pCamInfo->m_pPosObj != NULL) && (!pCamInfo->m_pPosObj->IsInitted()))
	{
		DEVPRINTF("CCamManual::Init() : Passed in a vec3obj that was not initialized.\n");
		return(FALSE);
	}

	if((pCamInfo->m_pQuatObj != NULL) && (!pCamInfo->m_pQuatObj->IsInitted()))
	{
		DEVPRINTF("CCamManual::Init() : Passed in a quatobj that was not initialized.\n");
		return(FALSE);
	}

	Reset();
	m_pCamInfo = pCamInfo;

	m_pCamera->SetFOV( splitscreen_RemapFOV( m_pCamera->GetID(), pCamInfo->m_fHalfFOV ) );

	return(TRUE);
}

// =============================================================================================================

void CCamManual::Reset()
{
	m_pCamInfo = NULL;
}

// =============================================================================================================

void CCamManual::Work()
{
	if(m_pCamInfo == NULL)
	{
		return;
	}

	if(m_bFreezeShot)
	{
		return;
	}

	//update the camera FOV here...
	f32 fNewFOV = splitscreen_RemapFOV( m_pCamera->GetID(), m_pCamInfo->m_fHalfFOV );
	f32 fCurrentFOV;
	m_pCamera->GetFOV( &fCurrentFOV );
	if( fCurrentFOV != fNewFOV ) {
		m_pCamera->SetFOV( fNewFOV );
	}

	if(m_pCamInfo->m_pQuatObj != NULL)
	{
		if(m_pCamInfo->m_pPosObj != NULL)
		{
			m_pCameraData->m_Xfm.BuildFromInvQuat(m_pCamInfo->m_pQuatObj->GetValue(), m_pCamInfo->m_pPosObj->GetValue());
		}
		else
		{
			FASSERT(m_pCamInfo->m_pvecPos != NULL);
			m_pCameraData->m_Xfm.BuildFromInvQuat(m_pCamInfo->m_pQuatObj->GetValue(), *m_pCamInfo->m_pvecPos);
		}
	}
	else if(m_pCamInfo->m_pqQuat != NULL)
	{
		if(m_pCamInfo->m_pPosObj != NULL)
		{
			m_pCameraData->m_Xfm.BuildFromInvQuat(*m_pCamInfo->m_pqQuat, m_pCamInfo->m_pPosObj->GetValue());
		}
		else
		{
			FASSERT(m_pCamInfo->m_pvecPos != NULL);
			m_pCameraData->m_Xfm.BuildFromInvQuat(*m_pCamInfo->m_pqQuat, *m_pCamInfo->m_pvecPos);
		}
	}
	else
	{
		FASSERT(m_pCamInfo->m_pmtxMtx != NULL);
		m_pCameraData->m_Xfm.BuildFromInvMtx(*m_pCamInfo->m_pmtxMtx);
	}
}

// =============================================================================================================
