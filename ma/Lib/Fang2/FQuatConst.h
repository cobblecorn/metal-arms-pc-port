//////////////////////////////////////////////////////////////////////////////////////
// FQuatConst.h - Const quaternion object.
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
// 04/30/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FQUATCONST_H_
#define _FQUATCONST_H_ 1

// =============================================================================================================

#include "FQuatObj.h"

// =============================================================================================================

static const u32 CFQuatConst_uPoolSize = 5;

// =============================================================================================================

FCLASS_ALIGN_PREFIX class CFQuatConst : public CFQuatObj
{
public:
	static BOOL ModuleStartup() { return(TRUE); }
	static void ModuleShutdown() {}
	static BOOL LevelInit() { m_uBufPos = 0; return(TRUE); }
	static void LevelUninit() {}

	CFQuatConst();

	// Restart resets the behavior of this motion object and optionally starts it again as well.
	void Reset(BOOL bAutoStart = TRUE) { Init(m_qValue, bAutoStart); }
	void SetTimePos(f32 fNewTime) {}

	static CFQuatConst *GetAvailable();
	BOOL Init(CFQuatA &qValue, BOOL bAutoStart = TRUE);
	BOOL Init(f32 fW, f32 fX, f32 fY, f32 fZ, BOOL bAutoStart = TRUE);
	void Work();
	void Release();

protected:

private:
	static u32 m_uBufPos, m_uNumInPool;
//	static CFQuatConst m_aoPool[CFQuatConst_uPoolSize];

	FCLASS_STACKMEM_ALIGN(CFQuatConst);
} FCLASS_ALIGN_SUFFIX;

// =============================================================================================================

#endif
