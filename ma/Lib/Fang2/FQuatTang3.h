//////////////////////////////////////////////////////////////////////////////////////
// FQuatTang3.h - 
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
#ifndef _FQUATTANG3_H_
#define _FQUATTANG3_H_ 1

// =============================================================================================================

#include "FQuatObj.h"

// =============================================================================================================

class CFVec3AObj;

// =============================================================================================================

static const u32 CFQuatTang3_uPoolSize = 50;

// =============================================================================================================

// This CFQuatObj creates a quaternion that models the effects of acceleration on a freely
//   hanging object.
// This CFQuatObj is intended to be used in conjunction with CFQuatTang2 and possibly other
//   CFQuatObj's.

FCLASS_ALIGN_PREFIX class CFQuatTang3 : public CFQuatObj
{
public:
	static BOOL ModuleStartup() { return(TRUE); }
	static void ModuleShutdown() {}
	static BOOL LevelInit();
	static void LevelUninit() {}

	CFQuatTang3();

	// Restart resets the behavior of this motion object and optionally starts it again as well.
	void Reset(BOOL bAutoStart = TRUE);
	void SetTimePos(f32 fNewTime) {}

	static CFQuatTang3 *GetAvailable();
	BOOL Init(CFVec3AObj *pV3OPath, BOOL bAutoStart = TRUE);
	void Work();
	void Release();

	static CFQuatTang3 *GetPool( void );
	static s32 GetPoolNumEntries( void );


protected:
	void Calculate();

	CFVec3AObj *m_pV3OPath;
	CFVec3A m_vecVel, m_vecPos;			// These are angular quantities.

private:
	static u32 m_uBufPos, m_uNumInPool;
//	static CFQuatTang3 m_aoPool[CFQuatTang3_uPoolSize];

	FCLASS_STACKMEM_ALIGN(CFQuatTang3);
} FCLASS_ALIGN_SUFFIX;

// =============================================================================================================

#endif
