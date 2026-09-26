//////////////////////////////////////////////////////////////////////////////////////
// AIMover.inl - 
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
// 06/11/02 MacKellar   Created.
// 09/01.02 MacKellar   Started making this class assume it was driving a bot.  
//                      (but only in limited easy way that would be easy to change if 
//                       necessary) AIBotMover is the real hard-core cbot class wrapper.
//////////////////////////////////////////////////////////////////////////////////////
#include "../Entity.h"
#include "../Bot.h"
#include "aigameutils.h"

const CFVec3A& CAIMover::GetLoc(void) const
{
	return m_pEntity->MtxToWorld()->m_vPos;
}


const CFVec3A& CAIMover::GetEarLoc(void) const
{
	return m_pEntity->MtxToWorld()->m_vPos;
}

f32 CAIMover::GetRadius(void) const
{
	FASSERT(m_pEntity->TypeBits() & ENTITY_BIT_BOT);
	return ((CBot*) m_pEntity)->m_fCollCylinderHeight_WS; //findfix: does this make sense?
}


f32	CAIMover::GetRadiusXZ(void) const
{
	FASSERT(m_pEntity->TypeBits() & ENTITY_BIT_BOT);
/*
f32 fRadXZ;
	if ((CBot*) m_pEntity)->GetCurMech())
	{
	   fRadXZ = ((CBot*) m_pEntity)->GetCurMech())->m_fCollCylinderRadius_WS;
	}
	else
	{
		fRadXZ = ((CBot*) m_pEntity)->m_fCollCylinderRadius_WS;

	}
	return fRadXZ;
*/
	if (((CBot*) m_pEntity)->GetCurMech())
	{
	   return ((CBot*) m_pEntity)->GetCurMech()->m_fCollCylinderRadius_WS;
	}
	return ((CBot*) m_pEntity)->m_fCollCylinderRadius_WS;
}


f32	CAIMover::GetHeight(void) const
{
	FASSERT(m_pEntity->TypeBits() & ENTITY_BIT_BOT);
	return ((CBot*) m_pEntity)->m_fCollCylinderHeight_WS;
}


f32	CAIMover::GetTallHeight(void) const
{
	FASSERT(m_pEntity->TypeBits() & ENTITY_BIT_BOT);
	f32 fHeight;
	((CBot*) m_pEntity)->IsStoopable(NULL, &fHeight);
	return fHeight;
}

f32	CAIMover::GetMinHeight(void) const
{
	FASSERT(m_pEntity->TypeBits() & ENTITY_BIT_BOT);
	f32 fMinHeight;
	((CBot*) m_pEntity)->IsStoopable(&fMinHeight, NULL);
	return fMinHeight;
}


FINLINE const CFVec3A& CAIMover::GetMoveDirXZ(void)
{
	FASSERT(m_pEntity->TypeBits() & ENTITY_BIT_BOT);
	return ((CBot*) m_pEntity)->m_UnitVelocityXZ_WS;
}


FINLINE f32 CAIMover::GetMoveSpeedXZ(void)
{
	FASSERT(m_pEntity->TypeBits() & ENTITY_BIT_BOT);
	return ((CBot*) m_pEntity)->m_fSpeedXZ_WS;
}
	

FINLINE f32 CAIMover::GetMoveSpeed(void)
{
	FASSERT(m_pEntity->TypeBits() & ENTITY_BIT_BOT);
	return ((CBot*) m_pEntity)->m_fSpeed_WS;
}

FINLINE BOOL CAIMover::HasTargetLock(void)
{
	FASSERT(m_pEntity->TypeBits() & ENTITY_BIT_BOT);
	return ((CBot*) m_pEntity)->HasTargetLock();
}


FINLINE BOOL CAIMover::IsDeadOrDying(void)
{
	return ((CBot*) m_pEntity)->IsDeadOrDying();
}


FINLINE BOOL CAIMover::IsSleeping(void)
{
	FASSERT(m_pEntity->TypeBits() & ENTITY_BIT_BOT);
	return (((CBot*) GetEntity())->IsSleeping() &&
			!((CBot*) GetEntity())->IsWaking());
}


FINLINE const CFVec3A&	CAIMover::GetTorsoLookAt(void)
{
	FASSERT(m_pEntity->TypeBits() & ENTITY_BIT_BOT);
	if (((CBot*) GetEntity())->GetAISteerMtx())
	{
		return ((CBot*) GetEntity())->GetAISteerMtx()->m_vFront;
	}
	return GetEntity()->MtxToWorld()->m_vFront;

}


FINLINE CBot* CAIMover::GetCurMech(void)
{
	FASSERT(m_pEntity && m_pEntity->TypeBits() & ENTITY_BIT_BOT);
	return ((CBot*) m_pEntity)->GetCurMech();
}

FINLINE void CAIMover::SetHeadLookGoal_WS(const CFVec3A& Pt_WS)
{
	if (CanSee())
	{
		m_uMoverFlags |= MOVERFLAG_RECEIVED_EXTERNAL_HEADLOOK_GOAL;
		m_HeadLookGoal_WS = Pt_WS;

//		FASSERT(aiutils_IsInBoundsOfWorld(Pt_WS));
//		if (!aiutils_IsInBoundsOfWorld(Pt_WS))
//		{
//			int i = 3;
//			FASSERT(0);
//		}
	}
}
