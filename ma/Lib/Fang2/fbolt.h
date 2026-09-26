//////////////////////////////////////////////////////////////////////////////////////
// fbolt.h - Draws lightning bolts.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 05/01/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FBOLT_H_
#define _FBOLT_H_ 1

#include "fang.h"
#include "ftex.h"
#include "fcolor.h"
#include "fdraw.h"



FCLASS_NOALIGN_PREFIX class CFBolt {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FCLASS_ALIGN_PREFIX class CFBoltData {
	public:
		CFVec3A m_StartPos_WS;
		CFVec3A m_EndPos_WS;

		CFTexInst m_TexInst;
		CFColorRGBA m_Color;

		u32 m_nBoltCount;
		u32 m_nMaxJointCount;
		f32 m_fDistBetweenJoints;
		f32 m_fTrumpetRadius;
		f32 m_fMinBoltThickness;
		f32 m_fMaxBoltThickness;
		f32 m_fMinBoltDisplacement;
		f32 m_fMaxBoltDisplacement;
		f32 m_fUnitEmitterPinch;			// Amount to pinch the bolt thickness toward the emitter end (1=no pinch, 0=max pinch)
		f32 m_fUnitDistPinch;				// How far away from the emitter to stop the pinching (0=at the emitter, 1=at the target)


		FCLASS_STACKMEM_NOALIGN( CFBoltData );
	} FCLASS_NOALIGN_SUFFIX;




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static CFVec3A m_UnitDir_WS;
	static CFVec3A m_RightUnitVec_WS;
	static CFVec3A m_CurveUnitVec_WS;
	static CFVec3A m_BasePos_WS;
	static CFVec3A m_BaseDeltaVec_WS;
	static CFVec3A m_CurvedBasePos_WS;
	static CFVec3A m_DisplacementVec_WS;
	static CFVec3A m_VtxPos_WS;
	static CFQuatA m_RotQuat;
	static u32 m_nJointCount;
	static u32 m_nJointCountMinusOne;
	static f32 m_fCurveStep;
	static f32 m_fOOUnitDistPinch;
	static FDrawVtx_t *m_pVtxArray;
	static CFBoltData *m_pBoltData;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static void Draw( CFBoltData *pBoltData );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static void _DrawOneBolt( void );


	FCLASS_STACKMEM_NOALIGN( CFBolt );
} FCLASS_NOALIGN_SUFFIX;




#endif

