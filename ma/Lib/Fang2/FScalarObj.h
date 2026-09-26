//////////////////////////////////////////////////////////////////////////////////////
// FScalarObj.h - System for scalar motion objects in Fang.
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
// 03/11/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FSCALAROBJ_H_
#define _FSCALAROBJ_H_ 1

#include "FMotionObj.h"

// =============================================================================================================

//// Abstract base class defining interface for scalar motion objects.
//
class CFScalarObj : public CFMotionObj
{
public:
	static BOOL ModuleStartup();
	static void ModuleShutdown();
	static BOOL LevelInit();
	static void LevelUninit();

	virtual void Work() = 0;

//	f32 GetValue() { FASSERT(IsActive()); return(m_fValue); }
	f32 GetValue() { return(m_fValue); }

protected:
	f32 m_fValue;
};
//
////

// =============================================================================================================
// =============================================================================================================

#endif
