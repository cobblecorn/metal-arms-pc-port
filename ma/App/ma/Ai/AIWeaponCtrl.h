//////////////////////////////////////////////////////////////////////////////////////
// AIWeaponCtrl.h - 
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
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 07/08/02 MacKellar   Created
// 10/29/02	MacKellar	Added Burst Time, removed hard-coded spew defaults
//
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _AIWEAPONCTRL_H_
#define _AIWEAPONCTRL_H_ 1


class CAIBuilder;
class CAIBrain;

FCLASS_NOALIGN_PREFIX class CAIWeaponCtrl
{
public:
	CAIWeaponCtrl(void);
	~CAIWeaponCtrl(void);

	void InitFromBuilder(CAIBuilder* pBuilder, CAIBrain* pBrain, u32 uWeaponIndex );
	void Reset(void);
	void Work(void);
	void NotifyFire(void);

	enum
	{
		CTRLFLAG_PREFERS_STOP_AND_SHOOT				= 0x01,
		CTRLFLAG_FIRE_DELAY_MEANS_TIMED_BURSTS		= 0x02,	  //fire continuously during each burst
		STATUSFLAG_BURSTING							= 0x04,
		STATUSFLAG_HAPPYTRIGGER_ON					= 0x08,
		STATUSFLAG_JUST_FIRED						= 0x10
	};

	BOOL IsBursting(void);
	BOOL IsReady(BOOL bTriggerHappy);
	FINLINE f32 GetAccuracy(void)				{ return m_fAccuracy;}
	FINLINE BOOL CanAbortBursts(void)			{ return IsBursting() && !(m_uCtrlFlags & CTRLFLAG_FIRE_DELAY_MEANS_TIMED_BURSTS);}
	FINLINE BOOL GetCtrlFlag_StopAndShoot(void)	{ return m_uCtrlFlags & CTRLFLAG_PREFERS_STOP_AND_SHOOT;}
	FINLINE BOOL DidJustFire(void)				{ return m_uCtrlFlags &	STATUSFLAG_JUST_FIRED;}
	BOOL IsTimeToStartBurst(void);		//is it time for a burst to start?

	//configurable vbls
	f32 m_fAccuracy;
	f32 m_fBurstDelay;								// how long between bursts
	f32 m_fBurstDelayBonus;
	f32 m_fFireDelay;								// how long between fires while bursting (or, length of a continuous fire burst if CTRLFLAG_FIRE_DELAY_MEANS_TIMED_BURSTS)
	f32 m_fFireDelayBonus;
	f32 m_fTriggerHappyFireDelay;					// overrides m_fFireDelay if triggerhappy bit is passed 
	f32 m_fTriggerHappyFireBonus;
	u8 m_uNumFiresPerBurst;
	u8 m_uNumFiresPerBurstBonus;					// how many fires during a burst

	//working vbls
	u8 m_uCtrlFlags;								// configureable and working :)
	u8 m_uNumFiresLeftThisBurst;
	f32 m_fBurstDelayTimeOut;
	f32 m_fFireTimeOut;
	f32 m_fBurstStartTime;

	FCLASS_STACKMEM_NOALIGN(CAIWeaponCtrl);
} FCLASS_NOALIGN_SUFFIX;


#endif

