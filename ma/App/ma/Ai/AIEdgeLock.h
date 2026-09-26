//////////////////////////////////////////////////////////////////////////////////////
// AIEdgeLock.h - 
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
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _AIEDGELOCK_H_
#define _AIEDGELOCK_H_ 1

#include "fang.h"

class CBot;

//
//
//	Edges on the graph can be locked and unlocked for various reasons.
//	   - clients pass a unique 32 bit vale (GUID) that their locks can be later referenced by
//

enum ResLockRetVal_e
{
	RESLOCKRETVAL_REQUEST_DENIED = 0,
	RESLOCKRETVAL_LOCK_ACQUIRED,
	RESLOCKRETVAL_REQUEST_ADDED_TO_QUEUE,
};
#define RESLOCK_LOCK_OR_WAIT TRUE
#define RESLOCK_LOCK_ONLY FALSE

ResLockRetVal_e ResLock_RequestEdgeLock(BOOL bGetInQueue, u16 uVertA, u16 uVertB, u32 uGUID);
BOOL ResLock_FreeEdgeLock(u16 uVertA, u16 uVertB, u32 uGUID);
BOOL ResLock_FreeAllEdgeLocks(u32 nGUID);
BOOL ResLock_IsEdgeLocked(u16 uVertA, u16 uVertB, u32 *puWhoGUID = NULL);						//Is it locked by anyone? If so, who
BOOL ResLock_HasEdgeLocked(u32 uGUID, u16* puPlaceInQueue = NULL);		//If uGUID has a lock, what is his place in line?


//
//
//	Seats in Mechs can be locked
//
//
ResLockRetVal_e ResLock_RequestMechSeatLock(BOOL bGetInLine, CBot* pVehicle, u8 uSeatId, u32 uGUID);
BOOL ResLock_FreeMechSeatLock(CBot* pVehicle, u8 uSeatId, u32 uGUID);
BOOL ResLock_FreeAllMechSeatLocks(u32 uGUID);
BOOL ResLock_IsMechSeatLocked(CBot* pVehicle, u8 uSeatId, u32 *puWhoGUID = NULL);			//Is it locked by anyone? If so, who
BOOL ResLock_HasMechSeatLocked(u32 uGUID, u16* puPlaceInQueue= NULL);		//If uGUID has a lock, what is his place in line?

BOOL ResLock_InitSystem(void);
BOOL ResLock_UninitSystem(void);
void ResLock_CheckpointRestore(void);



#endif

