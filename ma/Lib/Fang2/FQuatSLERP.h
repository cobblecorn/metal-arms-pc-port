//////////////////////////////////////////////////////////////////////////////////////
// FQuatSLERP.h - 
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
// 07/31/02 Link        Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FQUATSLERP_H_
#define _FQUATSLERP_H_ 1

#include "fang.h"

// =============================================================================================================

#include "FQuatObj.h"

// =============================================================================================================

// This CFQuatObj creates a quaternion that will SLERP between two other quaterions.
FCLASS_ALIGN_PREFIX class CFQuatSLERP : public CFQuatObj
{
public:
	static BOOL ModuleStartup() { return(TRUE); }
	static void ModuleShutdown() {}
	static BOOL LevelInit() { m_uBufPos = 0; return(TRUE); }
	static void LevelUninit() {}

	CFQuatSLERP();

	// Restart resets the behavior of this motion object and optionally starts it again as well.
	void Reset(BOOL bAutoStart = TRUE) { /*Init(m_pV3OLookFrom, m_pV3OLookTo, bAutoStart);*/ }
	void SetTimePos(f32 fNewTime) {}

	static CFQuatSLERP *GetAvailable();
	BOOL Init(CFQuatA *pQuat1, CFQuatA *pQuat2, f32 fTotalTime, BOOL bAutoStart = TRUE);
	void Work();
	void Release();

protected:
	void Calculate();

private:
	static u32 m_uBufPos, m_uNumInPool;
	f32 m_fCurTimePos, m_fTotalTime, m_fOOTotalTime;
	CFQuatA m_qStart, m_qEnd;

	FCLASS_STACKMEM_ALIGN(CFQuatSLERP);
} FCLASS_ALIGN_SUFFIX;

// =============================================================================================================

#endif
