//////////////////////////////////////////////////////////////////////////////////////
// AIZombieBoss.h -header for Boss AI
//
// Author: Michael Scholz
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
// 04/05/03 Scholz       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _AIZOMBIEBOSS_H
#define _AIZOMBIEBOSS_H 1

#include "AIThoughtsGround.h"

class CBotZombieBoss;


//
//  CZombieBossCombat
//
///////////////////////
//
// Parameters 
//
//	ParamPackSize: Large
//
//	CAIMemoryLarge::m_pEntity:	CEntity *m_pEnemy
//	 - f32      CloseEnoughRadius
//	 - f32      Time
//
///////////////////////

FCLASS_ALIGN_PREFIX class CZombieBossCombat : public CGroundCombat
{
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	struct
	{
		CBotZombieBoss* pZombieBoss;			// who I am
		CBot* pEnemyBot;						// my anti-guy
		f32	  fBotDistanceToGlitch;				
		f32	  fTimeSpentOnPath;					// how long I been going here?

		f32	  fLurchTimeCounter;				

		u8	  uCurrPathReason;
		u8	  uNextPathReason;
	}m_zb;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	void HandlePathing(BOOL bStop=FALSE);
	f32 DistanceToEnemy(void);
	void SetSpeed(u8 uSpeed);
	
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	CZombieBossCombat(void);
	~CZombieBossCombat(void);
	virtual void Init(CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack = NULL);
	virtual void Cleanup(void);
	virtual void Work(void);

	// RTTI
	virtual BOOL SupportsClassInterface(const s32 nInterfaceId);
	virtual s32 GetClassInterface(void);

	static BOOL InitBank(s32 nHowMany);
	static void CleanupBank(void);
	static CAIThought* BankAccess(CAIThought*);
	static CNiBank<CZombieBossCombat> *s_pAllocBank;

	FCLASS_STACKMEM_ALIGN(CZombieBossCombat);

} FCLASS_ALIGN_SUFFIX;

#endif