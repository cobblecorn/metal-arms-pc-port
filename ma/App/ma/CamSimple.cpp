//////////////////////////////////////////////////////////////////////////////////////
// CamSimple.h - Simple camera.
//
// Author: Steve Ranck     
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
// 08/01/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
// =============================================================================================================

#include "fang.h"
#include "CamSimple.h"


CCamSimple::CCamSimple() {
	Reset();
}


BOOL CCamSimple::InitSystem( void ) {
	return TRUE;
}

void CCamSimple::UninitSystem( void ) {
}


BOOL CCamSimple::Init( CCamSimpleInfo *pCamInfo ) {
	if( m_pCamera == NULL ) {
		DEVPRINTF( "CCamSimple::Init() : No camera assigned. Call AssignCamera() before Init().\n" );
		return FALSE;
	}

	if( pCamInfo == NULL ) {
		DEVPRINTF( "CCamSimple::Init() : CCamSimple requires a CCamSimpleInfo to init.\n" );
		return FALSE;
	}

	m_pCamInfo = pCamInfo;
	m_pCameraData->m_Xfm.BuildFromInvQuat( m_pCamInfo->m_Quat_WS, m_pCamInfo->m_Pos_WS );

	return TRUE;
}


void CCamSimple::Reset() {
	m_pCamInfo = NULL;
}


void CCamSimple::Work() {
	if( m_pCamInfo == NULL ) {
		return;
	}

	if( m_bFreezeShot ) {
		return;
	}

	m_pCameraData->m_Xfm.BuildFromInvQuat( m_pCamInfo->m_Quat_WS, m_pCamInfo->m_Pos_WS );
}

