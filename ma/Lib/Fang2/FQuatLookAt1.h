//////////////////////////////////////////////////////////////////////////////////////
// FQuatLookAt1.h - Quaternion look at object.
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
#ifndef _FQUATLOOKAT1_H_
#define _FQUATLOOKAT1_H_ 1

// =============================================================================================================

#include "FQuatObj.h"

// =============================================================================================================

class CFVec3AObj;

// =============================================================================================================

static const u32 CFQuatLookAt1_uPoolSize = 10;

// =============================================================================================================

// This CFQuatObj creates a quaternion that will look from one Vec3 object to another Vec3 object.
FCLASS_ALIGN_PREFIX class CFQuatLookAt1 : public CFQuatObj
{
public:
	static BOOL ModuleStartup() { return(TRUE); }
	static void ModuleShutdown() {}
	static BOOL LevelInit() { m_uBufPos = 0; return(TRUE); }
	static void LevelUninit() {}

	CFQuatLookAt1();

	// Restart resets the behavior of this motion object and optionally starts it again as well.
	void Reset(BOOL bAutoStart = TRUE) { Init(m_pV3OLookFrom, m_pV3OLookTo, bAutoStart); }
	void SetTimePos(f32 fNewTime) {}

	static CFQuatLookAt1 *GetAvailable();
	BOOL Init(CFVec3AObj *pV3OLookFrom, CFVec3AObj *pV3OLookTo, BOOL bAutoStart = TRUE);
	void Work();
	void Release();

protected:
	CFVec3AObj *m_pV3OLookFrom, *m_pV3OLookTo;

private:
	static u32 m_uBufPos, m_uNumInPool;
//	static CFQuatLookAt1 m_aoPool[CFQuatLookAt1_uPoolSize];

	FCLASS_STACKMEM_ALIGN(CFQuatLookAt1);
} FCLASS_ALIGN_SUFFIX;


// =============================================================================================================

#endif


