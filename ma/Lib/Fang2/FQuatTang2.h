//////////////////////////////////////////////////////////////////////////////////////
// FQuatTang2.h - 
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
// 04/19/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FQUATTANG2_H_
#define _FQUATTANG2_H_ 1

// =============================================================================================================

#include "FQuatObj.h"

// =============================================================================================================

class CFVec3AObj;

// =============================================================================================================

static const u32 CFQuatTang2_uPoolSize = 50;

// =============================================================================================================

// This CFQuatObj creates a quaternion that looks along the tangent to a curve.
// This differs from CFQOTang1 in that it reflects the orientation of a hanging object.
// This means that rotation only occurs along the y axis.
FCLASS_ALIGN_PREFIX class CFQuatTang2 : public CFQuatObj
{
public:
	static BOOL ModuleStartup() { return(TRUE); }
	static void ModuleShutdown() {}
	static BOOL LevelInit();
	static void LevelUninit() {}

	CFQuatTang2();

	// Restart resets the behavior of this motion object and optionally starts it again as well.
	void Reset(BOOL bAutoStart = TRUE) { /*Init(m_pV3OLookFrom, m_pV3OLookTo, bAutoStart);*/ }
	void SetTimePos(f32 fNewTime) {}

	static CFQuatTang2 *GetAvailable();
	BOOL Init(CFVec3AObj *pV3OPath, BOOL bAutoStart = TRUE);
	void Work();
	void Release();

	static CFQuatTang2 *GetPool( void );
	static s32 GetPoolNumEntries( void );

protected:
	void Calculate();

	CFVec3AObj *m_pV3OPath;

private:
	static u32 m_uBufPos, m_uNumInPool;
//	static CFQuatTang2 m_aoPool[CFQuatTang2_uPoolSize];

	FCLASS_STACKMEM_ALIGN(CFQuatTang2);
} FCLASS_ALIGN_SUFFIX;

#endif
