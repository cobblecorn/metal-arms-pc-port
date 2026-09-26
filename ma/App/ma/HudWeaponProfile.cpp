//////////////////////////////////////////////////////////////////////////////////////
// Hud2.cpp - (New) HUD Class for Mettle Arms.
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
// 02/05/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "HudWeaponProfile.h"

// =============================================================================================================

CHudWeaponProfile::CHudWeaponProfile()
{
	m_bIsValid = FALSE;
}

// =============================================================================================================

CHudWeaponProfile::~CHudWeaponProfile()
{
	m_bIsValid = TRUE;
}

// =============================================================================================================

void CHudWeaponProfile::SetProfile(u32 uWeapon1, u32 uWeapon2)
{
	m_auSelectedWeapon[0] = uWeapon1;
	m_auSelectedWeapon[1] = uWeapon2;
	m_bIsValid = TRUE;
}

// =============================================================================================================