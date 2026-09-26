//////////////////////////////////////////////////////////////////////////////////////
// FMotionObj.h - System for objects that encapsulate motion for Fang.
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
#ifndef _FMOTIONOBJ_H_
#define _FMOTIONOBJ_H_ 1

#include "fmath.h"

const u32 CFMotionObj_uWorkListSize = 100;

// =============================================================================================================

//// Abstract base class defining the interface for a motion object.
//
class CFMotionObj
{
public:
	// Work function that gets called once per frame when the object is in the system work list.
	virtual void Work() = NULL;

	// Resets the behavior of this motion object and optionally starts it again as well.
	virtual void Reset(BOOL bAutoStart = TRUE) = NULL;

	// Ensures that the object is added to the work list (and thus progressing through its motion).
	void Start() { FASSERT(IsInitted()); if(!IsWorking()) { m_uFlags &= ~(FLAG_DONE); AddToWorkList(this); } }

	// Freezes the object in its place.
	void Pause() { FASSERT(IsInitted()); if(IsWorking()) { RemoveFromWorkList(this); } }

	// Same as Pause() except that the object considers itself done (as if it had completed its motion normally).
	// Note that this does not jump an object to the end of its path.
	// Also, DERIVED OBJECTS MUST CALL THIS WHEN THEY ARE ENDING THEIR MOTION!
	void End();

	// Sets the object to be fNewTime seconds into its motion.
	// Whether or not an object is currently working is unaffected.
	// It is an error to set this timer to outside the time domain of the object.
	virtual void SetTimePos(f32 fNewTime) = NULL;

//	void SetEndEvent(s32 nEndEvent) { m_nEndEvent = nEndEvent; }
	void SetEndEventEnable(BOOL bEnable) { m_bEndEventEnabled = bEnable; }

//	BOOL IsActive() { return((m_uFlags & FLAG_ACTIVE) != 0); }
	BOOL IsInitted() { return((m_uFlags & FLAG_INITTED) != 0); }
	BOOL IsWorking() { return((m_uFlags & FLAG_WORKING) != 0); }
	BOOL IsDone() { return((m_uFlags & FLAG_DONE) != 0); }

	/////////////////////////////////////////////////////////////
	// Fang system functions.
	static BOOL ModuleStartup();
	static void ModuleShutdown();
	static BOOL LevelInit();
	static void LevelUninit();
	static void ModuleWork();
	//
	/////////////////////////////////////////////////////////////

	static s32 AddToWorkList(CFMotionObj *poSO);
	static void RemoveFromWorkList(CFMotionObj *poSO);
	static void RemoveFromWorkList(s32 nSOIdx);
	static BOOL IsInWorkList( const CFMotionObj &roSO );

protected:
	enum Flags_e
	{
//		FLAG_ACTIVE		=	1 << 0,		// Active means that is has been removed from the pool.
		FLAG_INITTED	=	1 << 1,
		FLAG_WORKING	=	1 << 2,
		FLAG_DONE		=	1 << 3
	};

	CFMotionObj() { m_uFlags = 0; m_bEndEventEnabled = FALSE; }

	static BOOL m_bModuleInitted;
	static u32 m_uWLSize;
	static CFMotionObj *m_apWorkList[CFMotionObj_uWorkListSize];
	static s32 m_nPathEndedEvent;

	BOOL m_bEndEventEnabled;
	u32 m_uFlags;

};
//
////

// =============================================================================================================

// =============================================================================================================


// =============================================================================================================

#endif
