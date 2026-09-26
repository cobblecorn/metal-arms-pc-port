//////////////////////////////////////////////////////////////////////////////////////
// FQuatTang.h - CFVec3Obj defining a path from an ordered set of points.
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

#include "fang.h"
#include "FMotionObj.h"

// This CFQuatObj creates a quaternion that will look from one Vec3 object to another Vec3 object.
FCLASS_ALIGN_PREFIX class CFQOLookAt1 : public CFQuatObj
{
public:
	CFQOLookAt1();

	// Restart resets the behavior of this motion object and optionally starts it again as well.
	virtual void Restart(BOOL bAutoStart = TRUE) { Init(m_pV3OLookFrom, m_pV3OLookTo, bAutoStart); }

	static CFQOLookAt1 *GetAvailable();
	BOOL Init(CFVec3Obj *pV3OLookFrom, CFVec3Obj *pV3OLookTo, BOOL bAutoStart = TRUE);
	void Work();
	void Release();

protected:
	CFVec3Obj *m_pV3OLookFrom, *m_pV3OLookTo;

private:
	static u32 m_uBufPos, m_uNumInPool;
	static CFQOLookAt1 m_aoPool[10];

	FCLASS_STACKMEM_ALIGN(CFQOLookAt1);
} FCLASS_ALIGN_SUFFIX;

#endif