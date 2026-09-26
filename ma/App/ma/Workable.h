//////////////////////////////////////////////////////////////////////////////////////
// Workable.h - Workable object class for Mettle Arms.
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
// 03/26/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _WORKABLE_H_
#define _WORKABLE_H_ 1

#include "fang.h"

const u32 CWorkable_uWorkListMaxSize = 100;

//// This abstract base class provides a simple way for 'work' objects to get added
//     to the game, have their work functions called, and have their systems initialized.
//   To use, simply derive your work object from this class, add appropriate calls to your
//     own InitSystem(), Reset(), and UninitSystem() methods in the respective methods of
//     this class.
//   Whenever you need an object to get its work function called, simply call the inherited
//     Activate() method, and to terminate it's entry in the work list call the inherited
//     Deactivate() method.
//   Note that it is the *object's* responsibility to turn on and off it's work function.
//   The system will happily keep calling the work function of any objects that are in its
//     list even if they are done doing their work.
class CWorkable
{
public:
	CWorkable() { m_nWorkIdx = -1; }

	// To be overridden in derived classes to contain the desired behavior.
	virtual void Work() = NULL;

	// These methods add and remove from the Actor work list respectively.
	void Activate() { m_nWorkIdx = AddToWorkList(this); }
	void Deactivate() { RemoveFromWorkList(m_nWorkIdx); m_nWorkIdx = -1; }

	static BOOL InitSystem();
	static void UninitSystem();
	static BOOL LevelInit();
	static void LevelUninit();
	static void Reset();
	static void SystemWork();

protected:

private:
	// Used to record where in the work list a given CWorkable object is so that
	//   it can be removed more quickly.
	s32 m_nWorkIdx;

	//// Work-list related functions and data.
	//
	static void ClearWorkList();
	static s32 AddToWorkList(CWorkable *pWorkable);
	static void RemoveFromWorkList(s32 nWorkListIdx);
	static void RemoveFromWorkList(CWorkable *pWorkable);

	static CWorkable *m_apWorkList[CWorkable_uWorkListMaxSize];
	static u32 m_uWLSize;
	//
	////

	static BOOL m_bSystemInitted;
};
//
////


#endif
