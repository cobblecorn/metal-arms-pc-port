//////////////////////////////////////////////////////////////////////////////////////
// FQOTang1.h - CFVec3Obj defining a path from an ordered set of points.
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
// 04/09/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FQUATTANG_H_
#define _FQUATTANG_H_ 1

// =============================================================================================================

#include "FQuatObj.h"

// =============================================================================================================

class CFVec3AObj;

// =============================================================================================================

static const u32 CFQOTang1_uPoolSize = 50;

// =============================================================================================================

// This CFQuatObj creates a quaternion that looks along the tangent to a curve.
FCLASS_ALIGN_PREFIX class CFQOTang1 : public CFQuatObj
{
public:
	static BOOL ModuleStartup() { return(TRUE); }
	static void ModuleShutdown() {}
	static BOOL LevelInit();
	static void LevelUninit() {}

	CFQOTang1();

	// Restart resets the behavior of this motion object and optionally starts it again as well.
	void Reset(BOOL bAutoStart = TRUE) { /*Init(m_pV3OLookFrom, m_pV3OLookTo, bAutoStart);*/ }
	void SetTimePos(f32 fNewTime) {}

	static CFQOTang1 *GetAvailable();
	BOOL Init(CFVec3AObj *pV3OPath, BOOL bAutoStart = TRUE);
	void Work();
	void Release();

	static CFQOTang1 *GetPool( void );
	static s32 GetPoolNumEntries( void );


protected:
	void Calculate();

	CFVec3AObj *m_pV3OPath;

private:
	static u32 m_uBufPos, m_uNumInPool;
//	static CFQOTang1 m_aoPool[CFQOTang1_uPoolSize];

	FCLASS_STACKMEM_ALIGN(CFQOTang1);
} FCLASS_ALIGN_SUFFIX;

// =============================================================================================================

#endif
