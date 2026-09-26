//////////////////////////////////////////////////////////////////////////////////////
// FQuatTwirl.h - Quaternion object that rotates continuously about an axis.
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
// 04/29/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FQUATTWIRL_H_
#define _FQUATTWIRL_H_ 1

// =============================================================================================================

#include "FQuatObj.h"

// =============================================================================================================

static const u32 CFQuatTwirl_uPoolSize = 50;

// =============================================================================================================

FCLASS_ALIGN_PREFIX class CFQuatTwirl : public CFQuatObj
{
public:
	static BOOL ModuleStartup() { return(TRUE); }
	static void ModuleShutdown() {}
	static BOOL LevelInit() { m_uBufPos = 0; return(TRUE); }
	static void LevelUninit() {}

	CFQuatTwirl();

	// Restart resets the behavior of this motion object and optionally starts it again as well.
	void Reset(BOOL bAutoStart = TRUE) {}
	void SetTimePos(f32 fNewTime) {}

	static CFQuatTwirl *GetAvailable();
	BOOL Init(CFQuatA &qInit, CFVec3A &vecAxis, f32 fOmega, BOOL bAutoStart = TRUE);
	void Work();
	void Release();

protected:
	CFQuatA m_qInit;
	CFVec3A m_vecAxis;
	f32 m_fTimer, m_fOmega;

private:
	static u32 m_uBufPos, m_uNumInPool;
//	static CFQuatTwirl m_aoPool[CFQuatTwirl_uPoolSize];

	FCLASS_STACKMEM_ALIGN(CFQuatTwirl);
} FCLASS_ALIGN_SUFFIX;


// =============================================================================================================

#endif