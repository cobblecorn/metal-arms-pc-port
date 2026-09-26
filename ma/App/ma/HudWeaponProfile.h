//////////////////////////////////////////////////////////////////////////////////////
// HudWeaponProfile.h - HUD Weapon Profile Class for Mettle Arms.
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
// 02/17/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _HUDWEAPONPROFILE_H_
#define _HUDWEAPONPROFILE_H_ 1

class CHudWeaponProfile  
{
public:
	CHudWeaponProfile();
	~CHudWeaponProfile();

	void SetProfile(u32 uWeapon1, u32 uWeapon2);

	u32 m_auSelectedWeapon[2];
	BOOL m_bIsValid;

	FCLASS_STACKMEM_NOALIGN(CHudWeaponProfile);
};

#endif
