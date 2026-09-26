//////////////////////////////////////////////////////////////////////////////////////
// entitycontrol.cpp - Entity control class.
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
// 04/23/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "entitycontrol.h"




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CHumanControl
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CHumanControl::CHumanControl() : CEntityControl( CEntityControl::TYPE_HUMAN ) {
	CHumanControl::Zero();
}


void CHumanControl::Zero( void ) {
	m_fForward = 0.0f;
	m_fStrafeRight = 0.0f;
	m_fAimDown = 0.0f;
	m_fRotateCW = 0.0f;
	m_fFire1 = 0.0f;
	m_fFire2 = 0.0f;

	m_nPadFlagsAction = 0;
	m_nPadFlagsJump = 0;
	m_nPadFlagsPause = 0;
	m_nPadFlagsSelect1 = 0;
	m_nPadFlagsSelect2 = 0;
	m_nPadFlagsMelee = 0;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotControl
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CBotControl::CBotControl() : CEntityControl( CEntityControl::TYPE_BOT ) {
	CBotControl::Zero();
}


void CBotControl::Zero( void ) {
	m_TargetPoint_WS.Zero();
	m_XlatNormSpeedXZ_WS.Zero();
	m_fAimDown = 0.0f;
	m_fRotateCW = 0.0f;
	m_fFire1 = 0.0f;
	m_fFire2 = 0.0f;
	m_fFlyUp = 0.0f;
	m_nButtons = BUTTONFLAG_NONE;
	m_nFlags = FLAG_NONE;
	m_HeadLookPt_WS.Zero();
}


