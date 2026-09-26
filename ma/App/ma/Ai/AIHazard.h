//////////////////////////////////////////////////////////////////////////////////////
// AIHazard.h -  Management of hazards.  Hazards are things that might block pathfinding
//
// Author: Pat MacKellar 
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
// 05/24/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _AIHAZARD_H_
#define _AIHAZARD_H_ 1

class CAIHazardReg
{
public:
	CAIHazardReg(void);
	~CAIHazardReg(void);

	void Work(void);
	void DebugRender(void);

	u8 Register(void* pHazard);
	void* GetRegistered(u8 uHazardId);

protected:
	void* m_aRegistry[256];
	u8	m_uNextHazardId;
};



#endif

