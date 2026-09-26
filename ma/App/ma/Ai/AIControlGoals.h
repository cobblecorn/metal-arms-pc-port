//////////////////////////////////////////////////////////////////////////////////////
// AIControlGoals.h - 
//
//
//	  Wrap and smooth out the input that the AI feeds the entity controller
//	  - kinda does a frame-rate indepentent box filter on input
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
#ifndef _AICONTROLGOALS_H_
#define _AICONTROLGOALS_H_ 1

#include "../EntityControl.h"

class CAIControlGoals
{
public:
	FINLINE CAIControlGoals(void)					{ Reset(); };

	void ResetGoals(void);
	void ResetAverages(void);
	void ResetButtons(void);
	void Reset(void);
	void ApplyGoals(void);	  //Work function... smooths imput to get goal, then feeds the entity controller

	//movement/orientation goals
	void SetDXGoal(f32 fGoal);
	void SetDYGoal(f32 fGoal);
	void SetDZGoal(f32 fGoal);
	void SetAimDownGoal(f32 fGoal);
	void SetRotateCWGoal(f32 fGoal);
	void SetQuickRotateCWGoal(f32 fGoal);

	//actions
    void Jump(void);
    void JumpHigh(void);
    void JumpVec(const CFVec3A& InitialVel);
	void Fire(void);
	void Fire2(void);
	void Action(void);
	void Panic(void)								{ m_Controls.m_nFlags |= CBotControl::FLAG_PANIC; }
	void Alert(void)								{ m_Controls.m_nFlags |= CBotControl::FLAG_ALERT; }
	void HopLeft(void)								{ m_Controls.m_nFlags |= CBotControl::FLAG_HOP_LEFT; }
	void HopRight(void)								{ m_Controls.m_nFlags |= CBotControl::FLAG_HOP_RIGHT; }
	void RollLeft(void)								{ m_Controls.m_nFlags |= CBotControl::FLAG_ROLL_LEFT; }
	void RollRight(void)							{ m_Controls.m_nFlags |= CBotControl::FLAG_ROLL_RIGHT; }
	void Startle(void)								{ m_Controls.m_nFlags |= CBotControl::FLAG_HOP_VERT; }
	void Special1(void)								{ m_Controls.m_nButtons |= CBotControl::BUTTONFLAG_SPECIAL1; }
	void Special2(void)								{ m_Controls.m_nButtons |= CBotControl::BUTTONFLAG_SPECIAL2; }
	BOOL IsSpecial2(void)							{ return ( m_Controls.m_nButtons & CBotControl::BUTTONFLAG_SPECIAL2 ); }
	void QuickTurn(void);
	void InitQuickTurn(f32 fRads);
	void AimAt(const CFVec3A& Pt_WS);


	FINLINE f32 GetDXGoal(void)						{ return m_fDXGoal;}
	FINLINE f32 GetDYGoal(void)						{ return m_fDYGoal;}
	FINLINE f32 GetDZGoal(void)						{ return m_fDZGoal;}
	FINLINE f32 GetAimDownGoal(void)				{ return m_fAimDownGoal;}
	FINLINE f32 GetRotateCWGoal(void)				{ return m_fRotateCWGoal;}
	FINLINE  CBotControl* GetEntityControl(void)	{ return &m_Controls;}
	FINLINE BOOL IsHopFlagSet(void)					{ return (m_Controls.m_nFlags & CBotControl::FLAG_HOP_LEFT || 
																m_Controls.m_nFlags & CBotControl::FLAG_HOP_RIGHT);}
	FINLINE BOOL IsRollFlagSet(void)				{ return (m_Controls.m_nFlags & CBotControl::FLAG_ROLL_LEFT || 
																m_Controls.m_nFlags & CBotControl::FLAG_ROLL_RIGHT);}
	enum
	{
		AICONTROLFLAG_NONE = 0x0000,
		AICONTROLFLAG_SLAM_ROTATECW_AVERAGE = 0x0001,
		AICONTROLFLAG_SLAM_DX_AVERAGE = 0x0002,
		AICONTROLFLAG_SLAM_DY_AVERAGE = 0x0004,
		AICONTROLFLAG_SLAM_DZ_AVERAGE = 0x0008,
		AICONTROLFLAG_SLAM_AVERAGE_WHEN_ZERO = 0x0010,
	};
	FINLINE void SetFlag_Slam_RotateCW_Average(void) { m_uFlags |= AICONTROLFLAG_SLAM_ROTATECW_AVERAGE;}
	FINLINE void SetFlag_Slam_DX_Average(void) { m_uFlags |= AICONTROLFLAG_SLAM_DX_AVERAGE;}
	FINLINE void SetFlag_Slam_DY_Average(void) { m_uFlags |= AICONTROLFLAG_SLAM_DY_AVERAGE;}
	FINLINE void SetFlag_Slam_DZ_Average(void) { m_uFlags |= AICONTROLFLAG_SLAM_DZ_AVERAGE;}
	FINLINE void SetFlag_Slam_AverageWhenZero(void) { m_uFlags |= AICONTROLFLAG_SLAM_AVERAGE_WHEN_ZERO;}


protected:

	f32 m_fDXGoal;
	f32 m_fDYGoal;
	f32 m_fDZGoal;
	f32 m_fAimDownGoal;
	f32 m_fRotateCWGoal;
	
	CBotControl m_Controls;	 //Note, this is an aligned structure that is part of a Non-Aligned class

	enum 
	{
		NUM_SAMPLES_TO_AVG = 8,
	};
	f32 m_afDXAvg[NUM_SAMPLES_TO_AVG];
	f32 m_afDYAvg[NUM_SAMPLES_TO_AVG];
	f32 m_afDZAvg[NUM_SAMPLES_TO_AVG];
	f32 m_afAimDownAvg[NUM_SAMPLES_TO_AVG];
	f32 m_afRotateCWAvg[NUM_SAMPLES_TO_AVG];

	f32 AddSampleAndGetAverage(f32 fNewSample, f32* paSampleHistory);  //a fancy function that is like a frame-rate independent box-filter
	f32 m_afSampleTimes[NUM_SAMPLES_TO_AVG];							//findfix: might be able to make th...
	f32 m_fOoSampleTimeTotal;											//averaging data static if we knew that all
	u16 m_nCurSample;													//AI controlgoals would be Updated at the same frequence!
	u16 m_nNumSamples;
	u8 m_uFlags;

	FCLASS_STACKMEM_NOALIGN(CAIControlGoals);
};


#endif

