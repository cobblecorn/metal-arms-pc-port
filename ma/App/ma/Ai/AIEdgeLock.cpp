//////////////////////////////////////////////////////////////////////////////////////
// AIEdgeLock.cpp - 
//
// Author: Pat MacKellar 
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
// 04/02/02 MacKellar   Created.
// 12/13/02
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "flinklist.h"
#include "AIEdgeLock.h"
#include "AInodepools.h"
#include "flinklist.h"
#include "ftl.h"

//====================
// private definitions
class CEdgeLock
{
public:
	inline CEdgeLock(void) : m_uGUID(0), m_uVertA(0), m_uVertB(0) {};
	inline CEdgeLock(u16 uVertA, u16 uVertB, u32 uGUID) : m_uGUID(uGUID), m_uVertA(uVertA), m_uVertB(uVertB) {};
	inline ~CEdgeLock(void) {};
	inline BOOL CompareGUID( const CEdgeLock& v ) const { return (v.m_uGUID == m_uGUID); }
	inline BOOL CompareResource( const CEdgeLock& v ) const { return ( ((v.m_uVertA==m_uVertA) && (v.m_uVertB==m_uVertB)) || ((v.m_uVertB==m_uVertA) && (v.m_uVertA==m_uVertB))); }
	inline BOOL operator == ( const CEdgeLock& v ) const { return CompareGUID(v) && CompareResource(v); }
	inline u32 GetGUID(void) const { return m_uGUID;}
	u16 m_uVertA;
	u16 m_uVertB;
	u32 m_uGUID;

	static CNiList<CEdgeLock*>* s_pEdgeLockList;
	static CNiBank<CEdgeLock>* s_pAllocBank;
	static CNiLocker<CEdgeLock>* s_pLocker;
	static BOOL InitSystem(void);
	static BOOL UninitSystem(void);
	static void FreeAll(void);
	
	FCLASS_STACKMEM_NOALIGN(CEdgeLock);
};
CNiList<CEdgeLock*>* CEdgeLock::s_pEdgeLockList = NULL;
CNiBank<CEdgeLock>* CEdgeLock::s_pAllocBank = NULL;
CNiLocker<CEdgeLock>* CEdgeLock::s_pLocker = NULL;
const s32 knEdgeLockBankSize = 50;
CEdgeLock *  CNiIterator<class CEdgeLock *>::s_ReturnError;  //iterator template requires you to put this here

//=================
// public variables

//==================
// private variables

//===================
// private prototypes

//=================
// public functions
BOOL CEdgeLock::InitSystem(void)
{
	FASSERT(!s_pAllocBank);
	s_pAllocBank = APE_NEW CNiBank<CEdgeLock>(aimain_pNodePool, knEdgeLockBankSize, APE_NEW CEdgeLock[knEdgeLockBankSize]);
	s_pEdgeLockList = APE_NEW CNiList<CEdgeLock*>(aimain_pNodePool);
	s_pLocker = APE_NEW CNiLocker<CEdgeLock>(s_pEdgeLockList, s_pAllocBank);
	return (s_pEdgeLockList && s_pAllocBank && s_pLocker);
}


void CEdgeLock::FreeAll(void)
{
	CNiIterator<CEdgeLock *> it = s_pEdgeLockList->Begin();
	while (it.IsValid())
	{
		s_pAllocBank->Recycle(it.Get());
		it.Next();
	}
	if (s_pEdgeLockList)
	{
		s_pEdgeLockList->RemoveAll();
	}
}


BOOL CEdgeLock::UninitSystem(void)
{
	if (s_pEdgeLockList)
	{
		FASSERT(s_pEdgeLockList->Size()==0);
	}

	FreeAll();
	CNiIterator<CEdgeLock *> it = s_pEdgeLockList->Begin();
	while (it.IsValid())
	{
		s_pAllocBank->Recycle(it.Get());
		it.Next();
	}
	if (s_pEdgeLockList)
	{
		s_pEdgeLockList->RemoveAll();
	}
	APE_DELETE(s_pEdgeLockList); s_pEdgeLockList = NULL;
	APE_DELETE(s_pAllocBank);	s_pAllocBank = NULL;
	APE_DELETE(s_pLocker);		s_pLocker = NULL;
	return TRUE;
}

//
//
//	Mech's can 
//
//
//====================
// private definitions
class CBot;
class CMechSeatLock
{
public:
	inline CMechSeatLock(void) : m_uGUID(0), m_pMech(NULL), m_uSeatId(0) {};
	inline CMechSeatLock(CBot* pVehicle, u16 uVertB, u32 uGUID) : m_uGUID(uGUID), m_pMech(pVehicle), m_uSeatId(uVertB) {};
	inline ~CMechSeatLock(void) {};
	inline BOOL CompareGUID( const CMechSeatLock& v ) const { return (v.m_uGUID == m_uGUID); }
	inline BOOL CompareResource( const CMechSeatLock& v ) const { return ((v.m_pMech==m_pMech) && (v.m_uSeatId==m_uSeatId));}
	inline BOOL operator == ( const CMechSeatLock& v ) const { return CompareGUID(v) && CompareResource(v); }
	inline u32 GetGUID(void) const { return m_uGUID;}
	CBot* m_pMech;
	u16 m_uSeatId;
	u16 m_uGUID;

	static CNiList<CMechSeatLock*>* s_pMechSeatLockList;
	static CNiBank<CMechSeatLock>* s_pAllocBank;
	static CNiLocker<CMechSeatLock>* s_pLocker;
	static BOOL InitSystem(void);
	static BOOL UninitSystem(void);
	static void FreeAll(void);
	
	FCLASS_STACKMEM_NOALIGN(CMechSeatLock);
};
CNiList<CMechSeatLock*>* CMechSeatLock::s_pMechSeatLockList = NULL;
CNiBank<CMechSeatLock>* CMechSeatLock::s_pAllocBank = NULL;
CNiLocker<CMechSeatLock>* CMechSeatLock::s_pLocker = NULL;
const s32 knMechSeatLockBankSize = 50;
CMechSeatLock *  CNiIterator<class CMechSeatLock *>::s_ReturnError;  //iterator template requires you to put this here

//=================
// public variables

//==================
// private variables

//===================
// private prototypes

//=================
// public functions
BOOL CMechSeatLock::InitSystem(void)
{
	FASSERT(!s_pAllocBank);
	s_pAllocBank = APE_NEW CNiBank<CMechSeatLock>(aimain_pNodePool, knMechSeatLockBankSize, APE_NEW CMechSeatLock[knMechSeatLockBankSize]);
	s_pMechSeatLockList = APE_NEW CNiList<CMechSeatLock*>(aimain_pNodePool);
	s_pLocker = APE_NEW CNiLocker<CMechSeatLock>(s_pMechSeatLockList, s_pAllocBank);
	return (s_pMechSeatLockList && s_pAllocBank && s_pLocker);
}


BOOL CMechSeatLock::UninitSystem(void)
{
	if (s_pMechSeatLockList)
	{
		FASSERT(s_pMechSeatLockList->Size()==0);
	}

	FreeAll();

	APE_DELETE(s_pMechSeatLockList); s_pMechSeatLockList = NULL;
	APE_DELETE(s_pAllocBank);	s_pAllocBank = NULL;
	APE_DELETE(s_pLocker);		s_pLocker = NULL;
	return TRUE;
}


void CMechSeatLock::FreeAll(void)
{
	CNiIterator<CMechSeatLock *> it = s_pMechSeatLockList->Begin();
	while (it.IsValid())
	{
		s_pAllocBank->Recycle(it.Get());
		it.Next();
	}
	if (s_pMechSeatLockList)
	{
		s_pMechSeatLockList->RemoveAll();
	}

}

//==================
// private functions

ResLockRetVal_e ResLock_RequestEdgeLock(BOOL bGetInLine, u16 uVertA, u16 uVertB, u32 uGUID)
{
	CEdgeLock test(uVertA, uVertB, uGUID);
	return (ResLockRetVal_e) CEdgeLock::s_pLocker->RequestLock(test, bGetInLine);
}

BOOL ResLock_FreeEdgeLock(u16 uVertA, u16 uVertB, u32 uGUID)
{
	CEdgeLock test(uVertA, uVertB, uGUID);
	return CEdgeLock::s_pLocker->FreeLock(test);
}

BOOL ResLock_FreeAllEdgeLocks(u32 uGUID)
{
	return CEdgeLock::s_pLocker->FreeAllLocks(uGUID);
}

BOOL ResLock_IsEdgeLocked(u16 uVertA, u16 uVertB, u32 *puWhoGUID)
{
	CEdgeLock test(uVertA, uVertB, 0);
	return CEdgeLock::s_pLocker->IsLocked(test, puWhoGUID);
}

BOOL ResLock_HasEdgeLocked(u32 uGUID, u16* puPlaceInQueue)
{
	CEdgeLock test(0, 0, uGUID);
	return CEdgeLock::s_pLocker->HasAnyLocked(test, puPlaceInQueue);
}

//
//
//	Seats in Vehicles can be locked
//
//
ResLockRetVal_e ResLock_RequestMechSeatLock(BOOL bGetInLine, CBot* pVehicle, u8 uSeatId, u32 uGUID)
{
	CMechSeatLock test(pVehicle, uSeatId, uGUID);
	return (ResLockRetVal_e) CMechSeatLock::s_pLocker->RequestLock(test, bGetInLine);
}

BOOL ResLock_FreeMechSeatLock(CBot* pVehicle, u8 uSeatId, u32 uGUID)
{
	CMechSeatLock test(pVehicle, uSeatId, uGUID);
	return CMechSeatLock::s_pLocker->FreeLock(test);
}

BOOL ResLock_FreeAllMechSeatLocks(u32 uGUID)
{
	return CMechSeatLock::s_pLocker->FreeAllLocks(uGUID);
}

BOOL ResLock_IsMechSeatLocked(CBot* pVehicle, u8 uSeatId, u32 *puWhoGUID /*= NULL*/)			//Is it locked by anyone? If so, who
{
	CMechSeatLock test(pVehicle, uSeatId, 0);
	return CMechSeatLock::s_pLocker->IsLocked(test, puWhoGUID);
}

BOOL ResLock_HasMechSeatLocked(u32 uGUID, u16* puPlaceInQueue/*= NULL*/)		//If uGUID has a lock, what is his place in line?
{
	CMechSeatLock test(0, 0, uGUID);
	return CMechSeatLock::s_pLocker->HasAnyLocked(test, puPlaceInQueue);
}


BOOL ResLock_InitSystem(void)
{
	return CEdgeLock::InitSystem() && CMechSeatLock::InitSystem();
}


BOOL ResLock_UninitSystem(void)
{
	return CEdgeLock::UninitSystem() && CMechSeatLock::UninitSystem();
}

void ResLock_CheckpointRestore(void)
{
	CMechSeatLock::FreeAll();
}
