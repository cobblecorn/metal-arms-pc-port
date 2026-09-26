//////////////////////////////////////////////////////////////////////////////////////
// AIMover.cpp - 
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
// 04/15/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "AIMover.h"
#include "AINodePools.h"
#include "AIGameUtils.h"
#include "AIRooms.h"
#include "AIBrain.h"
#include "../Entity.h"
#include "../Bot.h" //findfix: remove this ASA CEntity has its own methods for getting Bounds, Location and Orientation
#include "../BotTalkInst.h"
#include "../BottalkData.h"
#include "../BotPart.h"
#include "../Vehicle.h"

#if !FANG_ENABLE_INLINE_CODE
#include "AIMover.inl"
#endif

//====================
// private definitions

//=================
// public variables
CNiList<class CEntity *> CAIMover::m_AvoidEntityList;

//==================
// private variables

//===================
// private prototypes

//=================
// public functions

//Create
CAIMover::CAIMover(void)
:	m_pEntity(NULL),
	m_nRoomId(0),
	m_fForwardGoalSpeed(1.0f),
	m_fForwardMaxAcc(0.0f),
	m_fForwardMaxDcc(0.0f),
	m_nClientFeedback(0),
	m_fLinkStartTime(0.0f),
	m_fLinkEstimateTime(0.0f),
	m_pAvoidThis(NULL),
	m_fLastAvoidSteeringChange(0.0f),
	m_fLastOverrideSteeringChange(0.0f),
	m_fTorsoAlignmentSpeedConstraint(0.25f),
	m_fTorsoAlignmentVelConstraint(0.35f),
	m_uMoverFlags(MOVERFLAG_TORSO_ALIGNMENT_CONSTRAINT_IGNORABLE),
	m_pPath(NULL),
	m_fStuckForSecs(0.0f),
	m_fMaxXZTurnSpeedPct(1.0f),
	m_uNumWeaponCtrls(0),
	m_paWeaponCtrl(NULL),
	m_uMoverCaps(MOVERCAP_CAN_ALL),
	m_uLastGraphVertId(0),
	m_uLastGraphEdgeId(0)
{
	m_AvoidEntityList.SetNodePool(aimain_pNodePool);
	m_HeadLookGoal_WS.Zero();
	m_TorsoLookAtXZ.Zero();
}


//Destroy
CAIMover::~CAIMover(void)
{
	RemoveFromWorld();
	//Why wasn't cleanup called?
	FASSERT(m_AvoidEntityList.Size()==0);
	APE_ARRAYDELETE(m_paWeaponCtrl);
	m_paWeaponCtrl = NULL;
}


void CAIMover::Create(CEntity* pEntity, u16 uGUID)
{
	FASSERT(pEntity);
	m_pEntity = pEntity;
	m_pEntity->SetControls(m_Controls.GetEntityControl());
}


void CAIMover::AddToWorld(void)
{
	FASSERT(GetEntity());


	m_nRoomId = airooms_FindRoomID(GetLoc());
	m_pAvoidThis = NULL;
	m_fLastAvoidSteeringChange = 0;
	m_fLastOverrideSteeringChange = 0;
	m_nClientFeedback = FEEDBACK_NONE;
	m_uMoverFlags &= ~MOVERFLAG_STUCK_WHILE_AVOIDING_LAST_FRAME;
	m_uMoverCaps = MOVERCAP_CAN_ALL;

	m_fForwardGoalSpeed = 0.0f;
	m_fForwardMaxAcc = 0.0f;
	m_fForwardMaxDcc = 0.0f;
	m_fForwardGoalSpeed = 20.0f;//findfix?
	for (u32 i = 0; i < GetNumWeaponCtrls(); i++)
	{
		m_paWeaponCtrl[i].Reset();
	}
	m_uLastGraphVertId = 0;
	m_uLastGraphEdgeId = 0;
}


void CAIMover::RemoveFromWorld(void)
{
	m_AvoidEntityList.RemoveAll();
}

void CAIMover::BeginFrame(void)
{
	m_uMoverFlags &=~ MOVERFLAG_RECEIVED_EXTERNAL_HEADLOOK_GOAL;
	UpdateMoverCaps();
}


void CAIMover::UpdateMoverCaps(void)
{
	m_uMoverCaps = MOVERCAP_CAN_ALL;
	if (m_pEntity && m_pEntity->TypeBits() & ENTITY_BIT_BOT)
	{
		CBot* pBot = (CBot*) m_pEntity;
		if (pBot->IsLimping() || pBot->HasNoLegs() || pBot->IsBackBroken() )
		{
			m_uMoverCaps &=~(MOVERCAP_CAN_JUMP);
		}

		if (pBot->HasNoLegs() || pBot->GetMaxFlatSurfaceSpeed() <= 0.01f)
		{
			m_uMoverCaps &=~(MOVERCAP_CAN_MOVE_XZ|MOVERCAP_CAN_MOVE_Y);
		}

		if (pBot->GetPartMgr() && pBot->GetPartMgr()->IsCreated() && pBot->GetPartMgr()->GetComponentStatus(CBotPartMgr::COMPONENT_TYPE_EYES) == CBotPartMgr::COMPONENT_STATUS_NONE_OPERATIONAL)
		{
			m_uMoverCaps &=~(MOVERCAP_CAN_SEE);
		}

		if (pBot->GetPartMgr() && pBot->GetPartMgr()->IsCreated() && pBot->GetPartMgr()->GetComponentStatus(CBotPartMgr::COMPONENT_TYPE_PRIMARY) == CBotPartMgr::COMPONENT_STATUS_NONE_OPERATIONAL)
		{
			m_uMoverCaps &=~(MOVERCAP_CAN_FIRE_PRIMARY);
		}

		if (pBot->GetPartMgr() && pBot->GetPartMgr()->IsCreated() && pBot->GetPartMgr()->GetComponentStatus(CBotPartMgr::COMPONENT_TYPE_SECONDARY) == CBotPartMgr::COMPONENT_STATUS_NONE_OPERATIONAL)
		{
			m_uMoverCaps &=~(MOVERCAP_CAN_FIRE_SECONDARY);
		}

		if (pBot->GetPartMgr() && pBot->GetPartMgr()->IsCreated() && pBot->GetPartMgr()->GetComponentStatus(CBotPartMgr::COMPONENT_TYPE_PRIMARY) == CBotPartMgr::COMPONENT_STATUS_SOME_PARTIALLY_OPERATIONAL)
		{
			m_uMoverCaps &=~(MOVERCAP_CAN_AIM_PRIMARY);
		}

		if (pBot->GetPartMgr() && pBot->GetPartMgr()->IsCreated() && pBot->GetPartMgr()->GetComponentStatus(CBotPartMgr::COMPONENT_TYPE_SECONDARY) == CBotPartMgr::COMPONENT_STATUS_SOME_PARTIALLY_OPERATIONAL)
		{
			m_uMoverCaps &=~(MOVERCAP_CAN_AIM_SECONDARY);
		}
	}
}

void CAIMover::StopAndClearCmds(void)
{
	m_Controls.ResetGoals();
	m_Controls.ResetButtons();
}

void CAIMover::SetForwardGoalSpeedPctCapable(f32 fScale)
{
	FASSERT(m_pEntity->TypeBits() & ENTITY_BIT_BOT);
	m_fForwardGoalSpeed = fScale*((CBot*) m_pEntity)->m_fMaxFlatSurfaceSpeed_WS;
	FMATH_CLAMP(m_fForwardGoalSpeed, 0.0f, ((CBot*) m_pEntity)->m_fMaxFlatSurfaceSpeed_WS);
}


void CAIMover::SetForwardGoalSpeed(f32 fGoalSpeed)
{ 
	FASSERT(m_pEntity->TypeBits() & ENTITY_BIT_BOT);
	m_fForwardGoalSpeed = fGoalSpeed;
	FMATH_CLAMP(m_fForwardGoalSpeed, 0.0f, ((CBot*) m_pEntity)->m_fMaxFlatSurfaceSpeed_WS);
}


f32 CAIMover::CalcSpeedPctFromAction(f32 fGoalSpeed)
{
	FMATH_CLAMP(fGoalSpeed, 0.0f, ((CBot*) m_pEntity)->m_fMaxFlatSurfaceSpeed_WS);
	return fGoalSpeed / ((CBot*) m_pEntity)->m_fMaxFlatSurfaceSpeed_WS;
}


void CAIMover::SetMovementStyle( f32 fAlgnmntVelConstraint, f32 fAlgnmntSpeedConstraint, BOOL bCanOverrideConstraint)
{
	m_fTorsoAlignmentSpeedConstraint = fAlgnmntSpeedConstraint;
	m_fTorsoAlignmentSpeedConstraint = fAlgnmntVelConstraint;

	m_uMoverFlags &=~CAIMover::MOVERFLAG_TORSO_ALIGNMENT_CONSTRAINT_IGNORABLE ;
	if (bCanOverrideConstraint)
	{
		m_uMoverFlags |= CAIMover::MOVERFLAG_TORSO_ALIGNMENT_CONSTRAINT_IGNORABLE ;
	}
}


BOOL CAIMover::IsTalkingWithOnlyAudio(void)
{
	FASSERT(m_pEntity->TypeBits() & ENTITY_BIT_BOT);
	return (BOOL)(  ((CBot*) m_pEntity)->m_pActiveTalkInst &&
					((CBot*) m_pEntity)->m_pActiveTalkInst->GetTalkData() &&
					!((CBot*) m_pEntity)->m_pActiveTalkInst->GetTalkData()->DrivesBones());
}


BOOL CAIMover::IsTalkingWithAnim(void)
{
	FASSERT(m_pEntity->TypeBits() & ENTITY_BIT_BOT);
	return (BOOL)(  ((CBot*) m_pEntity)->m_pActiveTalkInst &&
					((CBot*) m_pEntity)->m_pActiveTalkInst->GetTalkData() &&
					((CBot*) m_pEntity)->m_pActiveTalkInst->GetTalkData()->DrivesBones());
}


BOOL CAIMover::IsTalkingWithLowerBodyAnim(void)
{
	FASSERT(m_pEntity->TypeBits() & ENTITY_BIT_BOT);
	return (BOOL)(  ((CBot*) m_pEntity)->m_pActiveTalkInst &&
					((CBot*) m_pEntity)->m_pActiveTalkInst->GetTalkData() &&
					((CBot*) m_pEntity)->m_pActiveTalkInst->GetTalkData()->DrivesLowerBody());
}


const CFVec3A &CAIMover:: GetEyeLookAt(void)
{
	if (m_pEntity->TypeBits() & ENTITY_BIT_BOT)
	{
		return ((CBot*) m_pEntity)->m_GazeUnitVec_WS;
	}
	return m_pEntity->MtxToWorld()->m_vFront;
}


const CFVec3A & CAIMover::GetEyePos(void)
{
	if (CanSee() && (m_pEntity->TypeBits() & ENTITY_BIT_BOT))
	{
		FASSERT(((CBot*) m_pEntity)->m_pApproxEyePoint_WS);
		return *((CBot*) m_pEntity)->m_pApproxEyePoint_WS;
	}
	return GetEntity()->MtxToWorld()->m_vPos;
}


void CAIMover::GetPathFindingOrigin(CFVec3A* pOrigin)
{
	FASSERT(pOrigin && GetEntity()->MtxToWorld());
	*pOrigin = GetEntity()->MtxToWorld()->m_vPos;
	pOrigin->y+=GetHeight()*0.5f;
	return;
}


f32 _CFShereDistance(const CFSphere& a, const CFSphere& b)
{
	f32 fDist = (b.m_Pos - a.m_Pos).Mag();
	fDist -= a.m_fRadius;
	fDist -= b.m_fRadius;
	if (fDist < 0.0f)
	{
		fDist = 0.0f;
	}

	return fDist;
}


f32 AIMover_DistApartXZ(const CAIMover* pA, const CAIMover* pB)
{
	return _CFShereDistance(CFSphere(pA->GetLoc().v3, pA->GetRadiusXZ()), CFSphere(pB->GetLoc().v3, pB->GetRadiusXZ()));
}


f32 AIMover_DistApart(const CAIMover* pA, const CAIMover* pB)
{
	return _CFShereDistance(CFSphere(pA->GetLoc().v3, pA->GetRadius()), CFSphere(pB->GetLoc().v3, pB->GetRadius()));
}


void CAIMover::RequestMechEntry(CEntity* pMech, BOOL bProximityCheck /*= MECH_ENTRY_PROXIMITYCHECK */)
{
	FASSERT(!GetCurMech());

	if (pMech->TypeBits() & ENTITY_BIT_SITEWEAPON)
	{
		if (((CBot*) pMech)->IsPillBox())
		{
			pMech->ActionNearby(GetEntity());
		}
		else if (((CBot*) pMech)->IsRatGun() && pMech->GetParent() && pMech->GetParent()->TypeBits() & ENTITY_BIT_VEHICLERAT )
		{
			((CVehicle*) pMech->GetParent())->EnterStation((CBot*) GetEntity(), CVehicle::STATION_GUNNER, bProximityCheck, TRUE /*bImmediately*/);
		}
	}
	else if (pMech->TypeBits() & ENTITY_BIT_VEHICLE)
	{
		((CVehicle*)pMech)->EnterStation((CBot*) GetEntity(), CVehicle::STATION_DRIVER, bProximityCheck, TRUE /*bImmediately*/);
	}

	if (GetCurMech())
	{  //didn't gain access, release lock and try using a "blind" attack order to get into the vehicle
		int i = 3;

	}
	else
	{
		//Didn't get in
		int i = 4;
	}


	FASSERT(!GetCurMech() || GetCurMech() == pMech);
}


void CAIMover::RequestMechExit(CEntity* pMech)
{
	pMech->ActionNearby(GetEntity());
}


BOOL CAIMover::CanDriveMech(void)
{
	return CanSee() && 	CanFirePrimary() && CanFireSecondary() && CanAimPrimary();
}


BOOL CAIMover::WakeUp(void)
{
	FASSERT(m_pEntity && m_pEntity->TypeBits() & ENTITY_BIT_BOT);
	return ((CBot*) m_pEntity)->WakeUp();
}
