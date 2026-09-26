//////////////////////////////////////////////////////////////////////////////////////
// AIWeaponCtrl.cpp - 
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
// 07/08/02 MacKellar   Created.
// 03/05/03 MacKellar	Changed Burst Timer to be a Burst Delay Timer
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "AIWeaponCtrl.h"
#include "AIBuilder.h"
#include "AIGameUtils.h"


CAIWeaponCtrl::CAIWeaponCtrl(void)
:	m_fBurstDelay(0.8f),
	m_fFireDelay(0.0f),	//0 means every frame
	m_uNumFiresPerBurst(3),
	m_fAccuracy(1.0f),
	m_fBurstDelayTimeOut(0.0f),
	m_fFireTimeOut(0.0f),
	m_uCtrlFlags(0),
	m_fFireDelayBonus(0.0f),
	m_fBurstDelayBonus(0.0f),
	m_uNumFiresPerBurstBonus(0)
{
	//no bursting while trigger happy
	//this is  
	m_fTriggerHappyFireDelay = 0.0f;
	m_fTriggerHappyFireBonus  =0.03f;
}


CAIWeaponCtrl::~CAIWeaponCtrl(void)
{
}


void CAIWeaponCtrl::InitFromBuilder(CAIBuilder* pAIBuilder, CAIBrain* pBrain, u32 uWeaponIndex)
{
	m_uCtrlFlags = 0;
	if (uWeaponIndex == 1)
	{
		m_fBurstDelay = pAIBuilder->m_fBurstDelay2;
		m_fFireDelay = pAIBuilder->m_fFireDelay2;	//0 means every frame
		m_uNumFiresPerBurst = (u8) pAIBuilder->m_uNumFiresPerBurst2;
		m_fAccuracy = pAIBuilder->m_fAccuracy2;

		m_fFireDelayBonus = pAIBuilder->m_fFireDelayBonus2;
		m_fBurstDelayBonus = pAIBuilder->m_fBurstDelayBonus2;
		m_uNumFiresPerBurstBonus = (u8)pAIBuilder->m_uNumFiresPerBurstBonus2;

		if (pAIBuilder->m_uTimedBursts2)
		{
			m_uCtrlFlags |= CTRLFLAG_FIRE_DELAY_MEANS_TIMED_BURSTS;
		}
		if (pAIBuilder->m_uPreferStopAndShoot)
		{
			m_uCtrlFlags |= CTRLFLAG_PREFERS_STOP_AND_SHOOT;
		}
	}
	else
	{
		m_fBurstDelay = pAIBuilder->m_fBurstDelay;
		m_fFireDelay = pAIBuilder->m_fFireDelay;	//0 means every frame
		m_uNumFiresPerBurst = (u8) pAIBuilder->m_uNumFiresPerBurst;
		m_fAccuracy = pAIBuilder->m_fAccuracy;

		m_fFireDelayBonus = pAIBuilder->m_fFireDelayBonus;
		m_fBurstDelayBonus = pAIBuilder->m_fBurstDelayBonus;
		m_uNumFiresPerBurstBonus = (u8)pAIBuilder->m_uNumFiresPerBurstBonus;
		if (pAIBuilder->m_uTimedBursts)
		{
			m_uCtrlFlags |= CTRLFLAG_FIRE_DELAY_MEANS_TIMED_BURSTS;
		}
		if (pAIBuilder->m_uPreferStopAndShoot)
		{
			m_uCtrlFlags |= CTRLFLAG_PREFERS_STOP_AND_SHOOT;
		}
	}

	Reset();
}


void CAIWeaponCtrl::Reset(void)
{
	m_fBurstDelayTimeOut = 0.0f;
	m_fFireTimeOut = 0.0f;
	m_uNumFiresLeftThisBurst = 0;
	m_fBurstStartTime = 0.0f;
	m_uCtrlFlags &= ~STATUSFLAG_BURSTING;
	m_uCtrlFlags &= ~STATUSFLAG_HAPPYTRIGGER_ON;
}


BOOL CAIWeaponCtrl::IsBursting(void)
{
	return (m_uCtrlFlags & STATUSFLAG_BURSTING);
}


BOOL CAIWeaponCtrl::IsTimeToStartBurst(void)
{
	return (m_fBurstDelayTimeOut < aiutils_FTotalLoopSecs());
}


void CAIWeaponCtrl::Work(void)
{
	m_uCtrlFlags &= ~STATUSFLAG_JUST_FIRED;

	if (IsBursting() &&
		(((m_uCtrlFlags & CTRLFLAG_FIRE_DELAY_MEANS_TIMED_BURSTS) && (m_fFireTimeOut < aiutils_FTotalLoopSecs())) ||
		 (!(m_uCtrlFlags & CTRLFLAG_FIRE_DELAY_MEANS_TIMED_BURSTS) && (m_uNumFiresLeftThisBurst ==0))))
	{
		
		//just finished bursting, so start the delay...
		m_uCtrlFlags &= ~STATUSFLAG_BURSTING;

		m_fBurstDelayTimeOut = aiutils_FTotalLoopSecs() + m_fBurstDelay + fmath_RandomFloat() * m_fBurstDelayBonus;
	}
}


BOOL CAIWeaponCtrl::IsReady(BOOL bTriggerHappy)
{
	BOOL bPermaBurst = FALSE;
	if (bTriggerHappy)
	{
		bPermaBurst = (	!(m_uCtrlFlags & STATUSFLAG_HAPPYTRIGGER_ON) ||
						m_fFireTimeOut < aiutils_FTotalLoopSecs());
		m_uCtrlFlags |= STATUSFLAG_HAPPYTRIGGER_ON;
	}
	else
	{
		m_uCtrlFlags &= ~STATUSFLAG_HAPPYTRIGGER_ON;
	}

	return	(bPermaBurst ||		  //Perma burst is so that during  triggerhappy mode, burst delays won't limit readyness.
			(!IsBursting() && IsTimeToStartBurst()) ||	   //waiting to begin a burst
			(IsBursting() && (m_uCtrlFlags & CTRLFLAG_FIRE_DELAY_MEANS_TIMED_BURSTS) && (m_fFireTimeOut > aiutils_FTotalLoopSecs())) ||	//bursting and on a timed burst that still has time remaining
			(IsBursting() && !(m_uCtrlFlags & CTRLFLAG_FIRE_DELAY_MEANS_TIMED_BURSTS) && (m_fFireTimeOut < aiutils_FTotalLoopSecs()) && (m_uNumFiresLeftThisBurst > 0))); //bursting and on a fire count burst and there are burts left
}


void CAIWeaponCtrl::NotifyFire(void)
{
	m_uCtrlFlags |= STATUSFLAG_JUST_FIRED;

	if (!IsBursting())   //pgm removed this so that bots can force a burst to start && IsTimeToStartBurst()
	{
		//start bursting!!!!!
		m_uCtrlFlags |= STATUSFLAG_BURSTING;
		
		m_fBurstStartTime = aiutils_FTotalLoopSecs();
		if (m_uCtrlFlags & CTRLFLAG_FIRE_DELAY_MEANS_TIMED_BURSTS)
		{
			m_fFireTimeOut = m_fBurstStartTime + m_fFireDelay + fmath_RandomFloat()*m_fFireDelayBonus;
		}
		else
		{
			m_uNumFiresLeftThisBurst = m_uNumFiresPerBurst+(u8)fmath_RandomChoice(m_uNumFiresPerBurstBonus);
		}
	}
	else
	{
		if (m_uCtrlFlags & CTRLFLAG_FIRE_DELAY_MEANS_TIMED_BURSTS)
		{
		}
		else
		{
			if (m_uCtrlFlags & STATUSFLAG_HAPPYTRIGGER_ON)
			{
				m_fFireTimeOut = aiutils_FTotalLoopSecs() + m_fTriggerHappyFireDelay + fmath_RandomFloat()*m_fTriggerHappyFireBonus;
			}
			else
			{
				m_fFireTimeOut = aiutils_FTotalLoopSecs() + m_fFireDelay + fmath_RandomFloat()*m_fFireDelayBonus;
			}

			if (m_uNumFiresLeftThisBurst > 0)
			{
				m_uNumFiresLeftThisBurst--;
			}
		}
	}
}
