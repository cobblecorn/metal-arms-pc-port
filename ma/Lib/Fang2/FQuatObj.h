//////////////////////////////////////////////////////////////////////////////////////
// FQuatObj.h - System for CFQuatA motion objects in Fang.
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
// 03/14/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FQUATOBJ_H_
#define _FQUATOBJ_H_ 1

#include "FMotionObj.h"

// =============================================================================================================

//class CFVec3AObj;

// =============================================================================================================

//// Abstract base class defining interface for CFQuatA motion objects.
//
class CFQuatObj : public CFMotionObj
{
public:
	static BOOL ModuleStartup();
	static void ModuleShutdown();
	static BOOL LevelInit();
	static void LevelUninit();

	virtual void Work() = 0;

	CFQuatA &GetValue() { FASSERT(IsInitted()); return(m_qValue); }

protected:
	CFQuatA m_qValue;
};
//
////

// =============================================================================================================

FCLASS_ALIGN_PREFIX class CFQOTwirlY : public CFQuatObj
{
public:
	CFQOTwirlY();
	~CFQOTwirlY();

	// Restart resets the behavior of this motion object and optionally starts it again as well.
	void Reset(BOOL bAutoStart = TRUE) { Init(m_qInitValue, m_fOmegaY, bAutoStart); }
	void SetTimePos(f32 fNewTime) {}

	static CFQOTwirlY *GetAvailable();
	BOOL Init(CFQuatA &qInitValue, f32 fOmegaY, BOOL bAutoStart = TRUE);
	void Work();
	void Release();

protected:
	CFQuatA m_qInitValue;
	f32 m_fTimer, m_fOmegaY;

private:
	static u32 m_uBufPos, m_uNumInPool;
//	static CFQOTwirlY m_aoPool[10];

	FCLASS_STACKMEM_ALIGN(CFQOTwirlY);
} FCLASS_ALIGN_SUFFIX;

// =============================================================================================================

#endif
