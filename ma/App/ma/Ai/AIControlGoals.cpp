//////////////////////////////////////////////////////////////////////////////////////
// AIControlGoals.cpp - 
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
// 04/16/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "AIControlGoals.h"
#include "floop.h"
#include "gamepad.h"
//====================
// private definitions

//=================
// public variables

//==================
// private variables

//===================
// private prototypes

//==================
// private functions

//=================
// public functions
void FloatApproachGoal(f32 *fVal, f32 *fLastSpeed, const f32 fGoal, const f32 fMaxSpeed, const f32 fMaxAcc, const f32 fDeltaTime)
{
	f32 fToGo = fGoal - *fVal;

	if (fDeltaTime <=0.0f)
	{
		return;
	}

	if (FMATH_IS_CLOSE_TO_ZERO(fToGo))
	{
		*fVal = fGoal;
		*fLastSpeed = 0.0f;
		return;
	}

	f32 fDir = FMATH_FSIGN(fToGo);
	f32 fIdealSpeed = fToGo / fDeltaTime;	 //as if we could get there in one step of fDeltaTime!
	FMATH_CLAMP(fIdealSpeed, -fMaxSpeed, fMaxSpeed);   //clamped by max Speed
	f32 fSpeedChangeIdeal = (fIdealSpeed - *fLastSpeed)/fDeltaTime;	  //as if we could get there in one step of fDeltaTime
//	FMATH_CLAMP(fSpeedChangeIdeal, -fMaxAcc, fMaxAcc);
	*fLastSpeed += fSpeedChangeIdeal*fDeltaTime;
	*fVal += (*fLastSpeed)*fDeltaTime;
}


void CAIControlGoals::ResetGoals(void)
{
	m_fDXGoal = 0.0f;
	m_fDYGoal = 0.0f;
	m_fDZGoal = 0.0f;
	m_fAimDownGoal = 0.0f;
	m_fRotateCWGoal = 0.0f;
	m_nNumSamples = 0;
	fang_MemZero( m_afSampleTimes, sizeof( f32 ) * NUM_SAMPLES_TO_AVG );
	m_uFlags = m_uFlags & AICONTROLFLAG_SLAM_AVERAGE_WHEN_ZERO;
}


void CAIControlGoals::ResetAverages(void)
{
	m_nCurSample = 0;
	m_nNumSamples = 0;

	for (s32 i = 0; i < NUM_SAMPLES_TO_AVG; i++)
	{
		m_afDXAvg[i] = 0.0f;
		m_afDYAvg[i] = 0.0f;
		m_afDZAvg[i] = 0.0f;
		m_afAimDownAvg[i] = 0.0f;
		m_afRotateCWAvg[i] = 0.0f;
	}
}

	
void CAIControlGoals::ResetButtons(void)
{
	m_Controls.m_nButtons = 0;
	m_Controls.m_fFire1 = 0.0f;
	m_Controls.m_fFire2 = 0.0f;
	m_Controls.m_nFlags &= ~CBotControl::FLAG_PANIC;
	m_Controls.m_nFlags &= ~CBotControl::FLAG_ALERT;
	m_Controls.m_nFlags &= ~CBotControl::FLAG_NEW_HEADLOOK_LOCATION	;
	m_Controls.m_nFlags &= ~CBotControl::FLAG_USE_HEADLOOK;
	m_Controls.m_nFlags &= ~CBotControl::FLAG_HOP_LEFT;
	m_Controls.m_nFlags &= ~CBotControl::FLAG_HOP_RIGHT;
	m_Controls.m_nFlags &= ~CBotControl::FLAG_HOP_VERT;
	m_Controls.m_nFlags &= ~CBotControl::FLAG_ROLL_LEFT;
	m_Controls.m_nFlags &= ~CBotControl::FLAG_ROLL_RIGHT;
	m_Controls.m_nFlags &= ~CBotControl::FLAG_QUICKROTATE_XZ;
	m_Controls.m_nFlags &= ~CBotControl::FLAG_AIM_AT_TARGET_POINT;
	m_Controls.m_nFlags &= ~CBotControl::BUTTONFLAG_SPECIAL1;
	//findfix: should CBotControl::BUTTONFLAG_SPECIAL2 be here as well?
}


void CAIControlGoals::Reset(void)
{
	ResetAverages();
	ResetGoals();
	ResetButtons();
}


void CAIControlGoals::AimAt(const CFVec3A& Pt_WS)
{
	m_Controls.m_TargetPoint_WS = Pt_WS;
	m_Controls.m_nFlags |= CBotControl::FLAG_AIM_AT_TARGET_POINT;
}


f32 CAIControlGoals::AddSampleAndGetAverage(f32 fNewSample, f32* paSampleHistory) 
{
	f32 fAvg = 0.0f;
	paSampleHistory[m_nCurSample] = fNewSample;
	for (s32 i = 0; i < m_nNumSamples; i++)
	{
		fAvg += paSampleHistory[i]*(m_afSampleTimes[i]*m_fOoSampleTimeTotal);
	}
	return fAvg;
}


void CAIControlGoals::ApplyGoals(void)
{
	m_afSampleTimes[m_nCurSample] = FLoop_fPreviousLoopSecs;
	m_nNumSamples++;
	if (m_nNumSamples > NUM_SAMPLES_TO_AVG)
	{
		m_nNumSamples = NUM_SAMPLES_TO_AVG;
	}

	m_fOoSampleTimeTotal = 0.0f;
	for (s32 i = 0; i < m_nNumSamples; i++)
	{
		m_fOoSampleTimeTotal += m_afSampleTimes[i];
	}
	if (m_fOoSampleTimeTotal > 0.0f)
	{
		m_fOoSampleTimeTotal = 1.0f/m_fOoSampleTimeTotal;
	}
	else
	{
		m_fOoSampleTimeTotal = 0.01f;
	}
	m_nCurSample++;
	m_nCurSample&=7;

	m_Controls.m_XlatNormSpeedXZ_WS.x = AddSampleAndGetAverage(m_fDXGoal, m_afDXAvg);
	m_Controls.m_XlatNormSpeedXZ_WS.z = AddSampleAndGetAverage(m_fDZGoal, m_afDZAvg);
	m_Controls.m_fFlyUp = AddSampleAndGetAverage(m_fDYGoal, m_afDYAvg);
	m_Controls.m_fAimDown = AddSampleAndGetAverage(m_fAimDownGoal, m_afAimDownAvg);
	m_Controls.m_fRotateCW = AddSampleAndGetAverage(m_fRotateCWGoal, m_afRotateCWAvg);//m_fRotateCWGoal*2.0f;//


	if (fmath_Abs(m_Controls.m_fAimDown) < 0.01f)
	{
		m_Controls.m_fAimDown = 0.0f;
	}
	if (fmath_Abs(m_Controls.m_fRotateCW) < 0.01f)
	{ //findfix: shouldn't lower level take care of this?
		m_Controls.m_fRotateCW = 0.0f;
	}
	else
	{
		FMATH_CLAMP(m_Controls.m_fRotateCW, -1.0f, 1.0f);
	}


	if (fmath_Abs(m_Controls.m_fFlyUp) < 0.001)
	{
		m_Controls.m_fFlyUp = 0.0f;
	}
	else
	{
		FMATH_CLAMP(m_Controls.m_fFlyUp, -1.0f, 1.0f);
	}

	FASSERT(fmath_Fcheck(m_Controls.m_XlatNormSpeedXZ_WS.x) == FMATH_FCHECK_RESULT_OK);
	FASSERT(fmath_Fcheck(m_Controls.m_XlatNormSpeedXZ_WS.z) == FMATH_FCHECK_RESULT_OK);
	FASSERT(fmath_Fcheck(m_Controls.m_fRotateCW) == FMATH_FCHECK_RESULT_OK);
	FASSERT(fmath_Fcheck(m_Controls.m_fAimDown) == FMATH_FCHECK_RESULT_OK);

}


void CAIControlGoals::Jump(void)
{
	m_Controls.m_nButtons |= CBotControl::BUTTONFLAG_JUMP1;
}


void CAIControlGoals::JumpHigh(void)
{
	m_Controls.m_nButtons |= CBotControl::BUTTONFLAG_JUMP2;
}


void CAIControlGoals::Action(void)
{
	m_Controls.m_nButtons |= CBotControl::BUTTONFLAG_ACTION;
}


void CAIControlGoals::JumpVec(const CFVec3A& InitialVel)
{
	m_Controls.m_nButtons |= CBotControl::BUTTONFLAG_JUMPVEC;
	m_Controls.m_JumpVelocity_WS = InitialVel;
}


void CAIControlGoals::QuickTurn(void)
{
	m_Controls.m_nFlags |= CBotControl::FLAG_QUICKROTATE_XZ;
}

void CAIControlGoals::InitQuickTurn(f32 fRads)
{
	m_Controls.m_fQuickRotateXZRads = fRads;
}


void CAIControlGoals::Fire(void)
{
	m_Controls.m_fFire1 = 1.0f;
}


void CAIControlGoals::Fire2(void)
{
	m_Controls.m_fFire2 = 1.0f;
}


void CAIControlGoals::SetDXGoal(f32 fGoal)
{

	if (m_uFlags & AICONTROLFLAG_SLAM_DX_AVERAGE ||
		(m_uFlags & AICONTROLFLAG_SLAM_AVERAGE_WHEN_ZERO && fGoal == 0.0f && m_fDXGoal != 0.0f))
	{
		for (s32 i = 0; i < NUM_SAMPLES_TO_AVG; i++)
		{
			m_afDXAvg[i] = fGoal;
		}
		m_uFlags &=~AICONTROLFLAG_SLAM_DX_AVERAGE;
	}

	m_fDXGoal = fGoal;
}


void CAIControlGoals::SetDYGoal(f32 fGoal)
{
	if (m_uFlags & AICONTROLFLAG_SLAM_DY_AVERAGE ||
		(m_uFlags & AICONTROLFLAG_SLAM_AVERAGE_WHEN_ZERO && fGoal == 0.0f && m_fDYGoal != 0.0f))
	{
		for (s32 i = 0; i < NUM_SAMPLES_TO_AVG; i++)
		{
			m_afDYAvg[i] = fGoal;
		}
		m_uFlags &=~AICONTROLFLAG_SLAM_DY_AVERAGE;
	}
	m_fDYGoal = fGoal;
}


void CAIControlGoals::SetDZGoal(f32 fGoal)
{
	if (m_uFlags & AICONTROLFLAG_SLAM_DZ_AVERAGE ||
		(m_uFlags & AICONTROLFLAG_SLAM_AVERAGE_WHEN_ZERO && fGoal == 0.0f && m_fDZGoal != 0.0f))
	{
		for (s32 i = 0; i < NUM_SAMPLES_TO_AVG; i++)
		{
			m_afDZAvg[i] = fGoal;
		}
		m_uFlags &=~AICONTROLFLAG_SLAM_DZ_AVERAGE;
	}
	m_fDZGoal = fGoal;
}


void CAIControlGoals::SetRotateCWGoal(f32 fGoal)
{
	if (m_uFlags & AICONTROLFLAG_SLAM_ROTATECW_AVERAGE ||
		(m_uFlags & AICONTROLFLAG_SLAM_AVERAGE_WHEN_ZERO && fGoal == 0.0f && m_fRotateCWGoal != 0.0f))
	{
		for (s32 i = 0; i < NUM_SAMPLES_TO_AVG; i++)
		{
			m_afRotateCWAvg[i] = fGoal;
		}
		m_uFlags &=~AICONTROLFLAG_SLAM_ROTATECW_AVERAGE;
	}
	m_fRotateCWGoal = fGoal;
}


void CAIControlGoals::SetAimDownGoal(f32 fGoal)
{
	m_fAimDownGoal = fGoal;
}