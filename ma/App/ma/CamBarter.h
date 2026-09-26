//////////////////////////////////////////////////////////////////////////////////////
// CamBarter.h - Barter Cameraman class for Mettle Arms.
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
// 06/03/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _CAMBARTER_H_
#define _CAMBARTER_H_ 1

// =============================================================================================================

#include "fang.h"
#include "fcamera.h"

// =============================================================================================================

// Forward declarations.

// =============================================================================================================

FCLASS_ALIGN_PREFIX class CCamBarterInfo
{
public:
	// Constructors:	
	CCamBarterInfo() { }
	CCamBarterInfo(f32 fHalfFOV, CFXfm &xfmBarterPos) : m_fHalfFOV(fHalfFOV), m_xfmBarterPos(xfmBarterPos) {}

	f32 m_fHalfFOV;				// in Radians
	CFXfm m_xfmBarterPos;	// Xfm that places the barter droids.

	FCLASS_STACKMEM_ALIGN(CCamBarterInfo);
} FCLASS_ALIGN_SUFFIX;

// =============================================================================================================

FCLASS_ALIGN_PREFIX class CCamBarter : public CFCameraMan
{
public:
	CCamBarter();

	static BOOL InitSystem();
	static void UninitSystem();

	BOOL Init(CCamBarterInfo *pCamInfo);
	void Reset();
	void Work();

	void MoveToFinalPos();
	void MoveToStartPos();

private:
	CCamBarterInfo *m_pCamInfo;

	//////////////////////////////////////////////////////////////////////
	//
	CFXfm m_xfmCamStart;

	CFQuatA m_qStart;
	CFVec3A m_vecStart;

	CFQuatA m_qFinal;
	CFVec3A m_vecFinal;
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	//
	f32 m_fCurTimePos;
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	//
	enum CamState_e
	{
		CAMSTATE_OUT,
		CAMSTATE_MOVINGIN,
		CAMSTATE_IN,
		CAMSTATE_MOVINGOUT,

		CAMSTATE_COUNT
	};
	CamState_e m_eState;
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	//
	void _Decisions();
	void _Work();
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	//
	void _MoveToFinalPos();
	void _GotoFinalPos();
	void _MoveToStartPos();
	void _GotoStartPos();
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Helper functions.
	void _CalcCamPosOri(f32 fWeight1, f32 fWeight2);
	//
	//////////////////////////////////////////////////////////////////////

	FCLASS_STACKMEM_ALIGN(CCamBarter);
} FCLASS_ALIGN_SUFFIX;

#endif
