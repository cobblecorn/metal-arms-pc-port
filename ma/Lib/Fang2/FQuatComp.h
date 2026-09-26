//////////////////////////////////////////////////////////////////////////////////////
// FQuatComp.h - Class for composition of quaternion objects.
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
// 04/22/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FQUATCOMP_H_
#define _FQUATCOMP_H_ 1

// =============================================================================================================

#include "FQuatObj.h"

// =============================================================================================================

static const u32 CFQuatComp_uPoolSize = 50;

// =============================================================================================================

// This CFQuatObj creates a quaternion that is the functional composition of two other quaternions.

FCLASS_ALIGN_PREFIX class CFQuatComp : public CFQuatObj
{
public:
	static BOOL ModuleStartup() { return(TRUE); }
	static void ModuleShutdown() {}
	static BOOL LevelInit();
	static void LevelUninit() {}

	CFQuatComp();

	// Restart resets the behavior of this motion object and optionally starts it again as well.
	void Reset(BOOL bAutoStart = TRUE) { /*Init(m_pV3OLookFrom, m_pV3OLookTo, bAutoStart);*/ }
	void SetTimePos(f32 fNewTime) {}

	static CFQuatComp *GetAvailable();
	BOOL Init(CFQuatObj *pQOLeft, CFQuatObj *pQORight, BOOL bAutoStart = TRUE);
	void Work();
	void Release();

	static CFQuatComp *GetPool( void );
	static s32 GetPoolNumEntries( void );

protected:
	void Calculate();

	CFQuatObj *m_pQOLeft, *m_pQORight;
	CFVec3A m_vecVel;

private:
	static u32 m_uBufPos, m_uNumInPool;
//	static CFQuatComp m_aoPool[CFQuatComp_uPoolSize];

	FCLASS_STACKMEM_ALIGN(CFQuatComp);
} FCLASS_ALIGN_SUFFIX;

// =============================================================================================================

#endif
