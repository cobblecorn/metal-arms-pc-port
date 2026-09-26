//////////////////////////////////////////////////////////////////////////////////////
// FVec3Obj.h - System for CFVec3 motion objects in Fang.
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
#ifndef _FVEC3OBJ_H_
#define _FVEC3OBJ_H_ 1

#include "FMotionObj.h"

// =============================================================================================================

//// Abstract base class defining interface for CFVec3 motion objects.
//
FCLASS_ALIGN_PREFIX class CFVec3AObj : public CFMotionObj
{
public:
	static BOOL ModuleStartup();
	static void ModuleShutdown();
	static BOOL LevelInit();
	static void LevelUninit();

	virtual void Work() = 0;

	virtual void SetTimePos(f32 fNewTime) = 0;
	virtual f32 GetTimePos() { return(0.0f); }

	/////////////////////////////////////////////////////////////
	// Methods that turn on or off extended functionality (and
	//   calculations).
	// Note that all of these keep a count of the number of on
	//   and off requests so that two users could share an object
	//   and not have to communicate directly to each other about
	//   when the extra calculations can be turned on or off.

		void SetTangVel(BOOL bAddRequest) { if(bAddRequest) { ++m_uTangVelReqCnt; } else { FASSERT(m_uTangVelReqCnt > 0); --m_uTangVelReqCnt; } }

	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// Methods for obtaining data from the CFVec3 object.
	// Note that some of these methods require that they be 
	//   enabled via a call to one of the methods above.

		// Returns the current value of the CFVec3 object.
		CFVec3A &GetValue() { FASSERT(IsInitted()); return(m_vecValue); }

		// Returns the current velocity vector.
		CFVec3A &GetVel() { FASSERT(m_uTangVelReqCnt > 0); return(m_vecVel); }

		// Returns a unit geometric tangent vector (note that it is possible that this is *not* just 
		//   an unitized version of the velocity returned above.  This could be the case if the 
		//   velocity at that particular point in time is zero).
		CFVec3A &GetTang() { FASSERT(m_uTangVelReqCnt > 0); return(m_vecTang); }

		// Returns 
		CFVec3A &GetAccel() { return(m_vecAccel); }

	//
	/////////////////////////////////////////////////////////////
protected:
	CFVec3AObj() { m_uTangVelReqCnt = 0; }

	u8 m_uTangVelReqCnt, m_uCurvatureReqCnt;

	CFVec3A m_vecValue;
	CFVec3A m_vecTang, m_vecVel;
	CFVec3A m_vecAccel;
} FCLASS_ALIGN_SUFFIX;
//
////

// =============================================================================================================

#endif
