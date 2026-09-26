//////////////////////////////////////////////////////////////////////////////////////
// CamManual.h - A simple cameraman driven by a position and orientation object.
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
// 04/01/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _CAMMANUAL_H_
#define _CAMMANUAL_H_ 1

// =============================================================================================================

#include "fang.h"
#include "fcamera.h"

// =============================================================================================================

// Forward declarations.
class CFVec3AObj;
class CFQuatObj;

// =============================================================================================================

class CCamManualInfo
{
public:
	// Constructors:	
	CCamManualInfo() : m_fHalfFOV(0.0f), m_pPosObj(NULL), m_pvecPos(NULL), m_pQuatObj(NULL), m_pqQuat(NULL), m_pmtxMtx(NULL) {}
	CCamManualInfo(f32 fHalfFOV) : m_fHalfFOV( fHalfFOV ), m_pPosObj(NULL), m_pvecPos(NULL), m_pQuatObj(NULL), m_pqQuat(NULL), m_pmtxMtx(NULL) {}

	f32 m_fHalfFOV;				// in Radians

	// You must either:
	// 1) Set one member from Group 1 *and* one member from Group 2,
	//   or
	// 2) Set the member in Group 3.

	// Group 1:
	CFVec3AObj *m_pPosObj;
	CFVec3A *m_pvecPos;

	// Group 2:
	CFQuatObj *m_pQuatObj;
	CFQuatA *m_pqQuat;

	// Group 3:
	CFMtx43A *m_pmtxMtx;

	FCLASS_STACKMEM_NOALIGN(CCamManualInfo);
};

// =============================================================================================================

FCLASS_ALIGN_PREFIX class CCamManual : public CFCameraMan
{
public:
	CCamManual();
//	~CCamManual();

	static BOOL InitSystem();
	static void UninitSystem();

	BOOL Init(CCamManualInfo *pCamInfo);
	void Reset();
	void Work();

private:
	CCamManualInfo *m_pCamInfo;

	FCLASS_STACKMEM_ALIGN(CCamManual);
} FCLASS_ALIGN_SUFFIX;

#endif