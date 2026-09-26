#include "fang.h"
#include "flinklist.h"
#include "fScriptSystem.h"
#include "AITactics.h"
#include "AIThoughtsGround.h"
#include "AIThoughtsGeneric.h"
#include "AIMain.h"
#include "AIBrainman.h"
#include "AIBrain.h"
#include "AIMover.h"
#include "AIBotMover.h"
#include "AIGraph.h"
#include "AINodePools.h"
#include "AIRooms.h"
#include "ApeNew.h"
#include "AiApi.h"
#include "AiBrainUtils.h"
#include "AIGameUtils.h"
#include "AIPatrolPath.h"
#include "AIEnviro.h"
#include "AIBTAtable.h"
#include "AIGroup.h"
#include "AIFSM.h"
#include "AIEdgeLock.h"

#include "../alarmsys.h"
#include "../Bot.h"
#include "../BotProbe.h"
#include "../BotTalkInst.h"
#include "../BotTalkData.h"
#include "../Bottitan.h"
#include "../Entity.h"
#include "../espline.h"
#include "../GString.h"
#include "../Player.h"
#include "../Site_BotWeapon.h"
#include "../Vehicle.h"
#include "../Weapon.h"
#include "../VehicleLoader.h"

extern BOOL AIBrain_TalkModeCB( u32 uTalkModeCBControl, void *pvData1, void *pvData2 );


BOOL _LocalPoiSearchWork(CGroundCombat* pT, BOOL *pbNewPtReached)
{
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	u8 uAttackPtSlot =0;
	CFVec3A PoiAttackPt;
	if (pbNewPtReached)
	{
		*pbNewPtReached = FALSE;
	}

	if ((pT->m_uAttackFlags & CGroundCombat::FLAG_STRATEGIC_ATTACK_PT_REACHED) &&
			(pT->m_StrategicAttackPt.DistSq(_pWD->pMover->GetLoc()) > 3.0f*3.0f ||
			(pT->m_pLockedPoi && !pT->m_pLockedPoi->IsLocVisible(aimain_pAIGraph, _pWD->EnemyMarkTagPos))))
	{
		pT->m_uAttackFlags &= ~CGroundCombat::FLAG_STRATEGIC_ATTACK_PT_REACHED;
		pT->ReleaseLockedPoi();
	}

	//if
	//		No Path ready to a new attack pt
	//      
	//then
	//
	//		try to find a new local attack pt
	//
	if (!(pT->m_uAttackFlags & CGroundCombat::FLAG_STRATEGIC_ATTACK_PT_REACHED) &&
		!pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS))
	{
		CGraphPoi* pPoi = NULL;
		if ((pT->*(_pWD->pFindLocalPoi_Func))(&PoiAttackPt,
					&pPoi,
					_pWD->EnemyMarkTagPos,
					_pWD->pMover->GetLoc(),
					0.0f,
					30.0f,
					0.0f,
					TRUE))
		{
			pT->LockPoi(pPoi);
			pT->RequestSearch(PoiAttackPt, CGroundCombat::SEARCHREASON_NEWATTACKPOS, 5.0f);
		}
		else 
		{
			return FALSE;
		}
	}

	//if
	//		an attack pt path fails || 
	//      bot is stuck
	//then
	//		clear the search slot
	//
	if (pT->IsSearchFailed(CGroundCombat::SEARCHREASON_NEWATTACKPOS) ||
		_pWD->pMover->IsStuck(2.2f))
	{
		if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackPtSlot))
		{
			pT->FreeRequestSlot(uAttackPtSlot);
		}
		pT->ReleaseLockedPoi();
		return FALSE;
	}


	//if    
	//		just completed an attackpt path
	//then
	//		clear the search slot
	//
	if (pT->m_bPathJustCompleted &&
		pT->m_nLastPathReason == CGroundCombat::SEARCHREASON_NEWATTACKPOS)
	{
		if (pbNewPtReached)
		{
			*pbNewPtReached = TRUE;
		}
		pT->m_StrategicAttackPt = _pWD->pMover->GetLoc();
		pT->m_uAttackFlags |= CGroundCombat::FLAG_STRATEGIC_ATTACK_PT_REACHED;

		if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackPtSlot))
		{
			pT->FreeRequestSlot(uAttackPtSlot);
		}
	}

	//if
	//		an attack pt path is ready && 
	//      not currently following attack pt
	//then
	//		follow the new tc path

	if (!pT->IsFollowingPath(TRUE, CGroundCombat::SEARCHREASON_NEWATTACKPOS) &&
		pT->IsSearchDone(CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackPtSlot))
	{
		u8 uAssignPathFlags = 0;
		if (pT->GetBrain()->GetMechLock() && !((CBot*) _pWD->pMover->GetEntity())->GetCurMech())
		{
			uAssignPathFlags |= CAIMover::ASSIGNPATHFLAG_ENTERMECH_PATH;
		}

		pT->Attack_AssignPath(uAttackPtSlot, uAssignPathFlags);
	}

	return TRUE;
}




BOOL _GAStandGroundReset(CGroundCombat* pT)
{
	pT->ResetCommonTacticVbls();

	pT->ResetDodge();

	pT->m_AttackPtSearch.InitEnemyRel();

	pT->m_uAttackFlags &=~ CGroundCombat::FLAG_STRATEGIC_ATTACK_PT_REACHED;
	pT->ReleaseLockedPoi();

	return TRUE;
}


void _GAStandGroundSetParams(	CGroundCombat* pT,
								u8 uDodgeTimeMin,
								u8 uDodgeTimeMax,
								u8 uDodgeOutTimeMin,
								u8 uDodgeOutTimeMax,
								u8 uDamageDodgeTimeMin,
								u8 uEnemyRelAttackPtSearchDelayTimeMin,
								u8 uEnemyRelAttackPtSearchDelayTimeMax,
								u8 uInitialStandGroundState)
{
	pT->m_uDodgeTimeMin = uDodgeTimeMin;
	pT->m_uDodgeTimeMax = uDodgeTimeMax;
	pT->m_uDodgeOutTimeMin = uDodgeOutTimeMin;
	pT->m_uDodgeOutTimeMax = uDodgeOutTimeMax;
	pT->m_uDamageDodgeTimeMin = uDamageDodgeTimeMin;
	pT->m_uStandGroundState = uInitialStandGroundState;

	if (pT->m_uStandGroundState == CGroundCombat::STANDGROUNDSTATE_VEHICLESEARCH &&
		pT->GetBrain()->GetMechUseageCtrl() == CAIBrain::MECH_USEAGE_CTRL_NEVER)
	{
		pT->m_uStandGroundState = CGroundCombat::STANDGROUNDSTATE_LOCALATTACKPT;
	}

	pT->m_AttackPtSearch.SetAttackPtSearchParams(	uEnemyRelAttackPtSearchDelayTimeMin,	//		u8 uRepeatTimeMin,
													uEnemyRelAttackPtSearchDelayTimeMax,	//		u8 uRepeatTimeMax,
													1,										//		s8 nDirInc,
													0,										//		u8 uDirMin,
													CGroundCombat::NUM_RELATIVE_DIRS,		//		u8 uDirMax,
													1,										//		s8 nRangeXZInc,
													0,//NOTE. StandGround sets this range every frame to match range to enemy mark.			//		u8 uRangeXZMin,
													0,//NOTE. StandGround sets this range every												//		u8 uRangeXZMax,
													0,										//		s8 nRangeYInc,
													0,										//		u8 uRangeYMin,
													0);										//		u8 uRangeYMax
}


BOOL _GAStandGroundInit(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;

	pT->ResetCommonTacticVbls();

	_GAStandGroundReset(pT);

	if (_pWD->pMover->GetCurMech() && pT->GetBrain()->GetMechLock() != _pWD->pMover->GetCurMech())
	{
		FASSERT(0);
	}

	return CFSM::INITSTATE_DONE;
}


BOOL _GAStandGroundUninit(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;

	pT->ResetDodge();

	u8 uAttackSearchSlot;
	if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackSearchSlot))
	{
		pT->FreeRequestSlot(uAttackSearchSlot);
	}
	return CFSM::INITSTATE_DONE; 
}


BOOL _GAStandGroundWork(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;

	if (_pWD->pMover->GetCurMech() && pT->GetBrain()->GetMechLock() != _pWD->pMover->GetCurMech())
	{
		FASSERT(0);
	}
	BOOL bOKToMakeEnemyRelAttackPtPaths  = FALSE;

	if (!(pT->m_uAttackFlags & CGroundCombat::FLAG_STRATEGIC_ATTACK_PT_REACHED))
	{
		// let the dodge tactic make local moves using dodge tactic
		// passing FALSE means don't even try to make moves if working on a new attack pos
		pT->DoDodgeWork(!(pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS)));	
	}

	if (pT->m_uStandGroundState == CGroundCombat::STANDGROUNDSTATE_VEHICLESEARCH)
	{
		if (pT->GetBrain()->GetMechUseageCtrl() != CAIBrain::MECH_USEAGE_CTRL_NEVER)
		{
			if ((_pWD->uStayPutDist == 0xffff || (pT->m_uAttackFlags & CGroundCombat::FLAG_STAYPUTBARRIER_BROKEN)))
			{
				if (!pT->GetBrain()->GetMechLock() && _pWD->pMover->CanDriveMech())
				{
					CBot* pNearbyVehicle = NULL;
					
					if (pT->GetBrain()->GetMechUseageTypeFlag() & CAIBrain::MECH_USEAGE_TYPE_FLAG_PILL)
					{
						pNearbyVehicle = aigroup_FindOpenPillBox(pT->GetBrain(), (f32) pT->GetBrain()->GetMechUseageRadius());
					}
					if (!pNearbyVehicle && pT->GetBrain()->GetMechUseageTypeFlag() & CAIBrain::MECH_USEAGE_TYPE_FLAG_LOADER)
					{
						pNearbyVehicle = aigroup_FindOpenLoader(pT->GetBrain(), (f32) pT->GetBrain()->GetMechUseageRadius());
					}
					if (!pNearbyVehicle && pT->GetBrain()->GetMechUseageTypeFlag() & CAIBrain::MECH_USEAGE_TYPE_FLAG_RAT)
					{
						pNearbyVehicle = aigroup_FindOpenRat(pT->GetBrain(), (f32) pT->GetBrain()->GetMechUseageRadius());
					}
					if (!pNearbyVehicle && pT->GetBrain()->GetMechUseageTypeFlag() & CAIBrain::MECH_USEAGE_TYPE_FLAG_SENTINEL)
					{
						pNearbyVehicle = aigroup_FindOpenSentinel(pT->GetBrain(), (f32) pT->GetBrain()->GetMechUseageRadius());
					}
					if (!pNearbyVehicle && pT->GetBrain()->GetMechUseageTypeFlag() & CAIBrain::MECH_USEAGE_TYPE_FLAG_RATGUN)
					{
						pNearbyVehicle = aigroup_FindOpenRatGun(pT->GetBrain(), (f32) pT->GetBrain()->GetMechUseageRadius());
					}

					if (pNearbyVehicle && ResLock_RequestMechSeatLock(RESLOCK_LOCK_ONLY, pNearbyVehicle, 0, pT->GetBrain()->GetGUID()))
					{
						pT->GetBrain()->SetMechLock(pNearbyVehicle, 0);
					}
					else
					{
						 //couldn't lock a nearby vehicle. Give up.
						pT->m_uStandGroundState = CGroundCombat::STANDGROUNDSTATE_LOCALATTACKPT;  

						if (pT->GetBrain()->GetMechUseageCtrl() == CAIBrain::MECH_USEAGE_CTRL_INIT)
						{
							pT->GetBrain()->SetFlag_MechUseageCtrlInitFailed();
						}
					}
				}
				else if (pT->GetBrain()->GetMechLock())
				{
					if (_pWD->pMover->GetCurMech() == pT->GetBrain()->GetMechLock())
					{
						pT->m_uAttackFlags |= CGroundCombat::FLAG_STRATEGIC_ATTACK_PT_REACHED;
					}
					else 
					{
						if (!pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS))
						{
							u8 uSeatId = 0;
							CBot* pNearbyVehicle = pT->GetBrain()->GetMechLock(&uSeatId);
							CFVec3A EntryPos_WS;
							f32 fFudge = 0.0f;
		//					pNearbySite->GetSiteAIData()->m_pAIGraphPoi->GetLocation(aimain_pAIGraph, &tmp);
							if (pNearbyVehicle->TypeBits() & ENTITY_BIT_SITEWEAPON)
							{
								EntryPos_WS = pNearbyVehicle->MtxToWorld()->m_vPos;		//findfix: get real gunner position.
								EntryPos_WS .y+=3.3f;
								fFudge = 0.0f;
							}
							else if (pNearbyVehicle->TypeBits() & ENTITY_BIT_VEHICLELOADER)
							{
								EntryPos_WS = pNearbyVehicle->MtxToWorld()->m_vPos;		//findfix: get real driver position.
								fFudge = 3.0f;
							}
							else if (pNearbyVehicle->TypeBits() & ENTITY_BIT_VEHICLESENTINEL)
							{
								EntryPos_WS = pNearbyVehicle->MtxToWorld()->m_vFront;		//findfix: get real driver position.
								EntryPos_WS.Mul(-(pNearbyVehicle->m_fCollCylinderRadius_WS+5.0f));
								EntryPos_WS.Add(pNearbyVehicle->MtxToWorld()->m_vPos);
								fFudge = 3.0f;
							}
							else if (pNearbyVehicle->TypeBits() & ENTITY_BIT_VEHICLERAT)
							{
								if (uSeatId == 0)	 //driver
								{
									EntryPos_WS = pNearbyVehicle->MtxToWorld()->m_vFront;		//findfix: get real driver position.
									EntryPos_WS.Mul(5.0f);
									EntryPos_WS.Add(pNearbyVehicle->MtxToWorld()->m_vPos);
									fFudge = 5.0f;
								}
								else
								{					//gunner
									EntryPos_WS = pNearbyVehicle->MtxToWorld()->m_vFront;		//findfix: get real gunner position.
									EntryPos_WS.Mul(-(pNearbyVehicle->m_fCollCylinderRadius_WS+5.0f));
									EntryPos_WS.Add(pNearbyVehicle->MtxToWorld()->m_vPos);
									fFudge = 3.0f;
								}
							}

							pT->RequestSearch(EntryPos_WS, CGroundCombat::SEARCHREASON_NEWATTACKPOS, 0.0f);
						}
					
						
						BOOL bTmp;
						if (!_LocalPoiSearchWork(pT, &bTmp))   //this function sits and waits for thepath to the NEWATTACKPOS to be completed
						{   //failure means path could not be acquired, or bot didn't make it all the way.
							//when NEWATTACKPOS is reached,	 CGroundCombat::FLAG_STRATEGIC_ATTACK_PT_REACHED is set
							//if (pT->m_nPossessionPlayerIndexOfEnemy !=-1)		//hmmmm, found this condition one day and not sure why It was here.... takingit out
							{
								ResLock_FreeMechSeatLock(pT->GetBrain()->GetMechLock(), 0, pT->GetBrain()->GetGUID());
								FASSERT(!ResLock_HasMechSeatLocked((u32) pT->GetBrain()->GetGUID()));
								pT->GetBrain()->ClearMechLock();
								pT->m_uStandGroundState = CGroundCombat::STANDGROUNDSTATE_LOCALATTACKPT;   //Quit trying to find a vehicle to drive

								if (pT->GetBrain()->GetMechUseageCtrl() == CAIBrain::MECH_USEAGE_CTRL_INIT)
								{
									pT->GetBrain()->SetFlag_MechUseageCtrlInitFailed();
								}

								return FALSE;
							}
						}
					}
				}
				else
				{
				}
			}
		}
		else
		{
			pT->m_uStandGroundState = CGroundCombat::STANDGROUNDSTATE_LOCALATTACKPT;	 //Quit trying to find a vehicle to drive
		}
	}
	else if (pT->m_uStandGroundState == CGroundCombat::STANDGROUNDSTATE_LOCALATTACKPT)
	{
		if ((_pWD->uStayPutDist == 0xffff || (pT->m_uAttackFlags & CGroundCombat::FLAG_STAYPUTBARRIER_BROKEN)))
		{
			BOOL bTmp;
			if (!_LocalPoiSearchWork(pT, &bTmp))
			{
				if (pT->m_nPossessionPlayerIndexOfEnemy > -1)
				{	//go to	searching for an enemy relative attack pt
					pT->m_uStandGroundState = CGroundCombat::STANDGROUNDSTATE_ENEMYRELATTACKPT;
					return FALSE;
				}
			}
		}
	}
	else if (pT->m_uStandGroundState == CGroundCombat::STANDGROUNDSTATE_ENEMYRELATTACKPT)
	{

		// if
		//		StayPutDist has been broken &&
		// then
		//		Stand Ground also may choose to move to a new enemy relative attack pt
		if ((_pWD->uStayPutDist == 0xffff || (pT->m_uAttackFlags & CGroundCombat::FLAG_STAYPUTBARRIER_BROKEN)))
		{
			//this AttackPtSearch is just doing one range loop around the enemy. so, set range min and range max equal
			if (_pWD->fDistToEnemyMarkXZ>0.0f && _pWD->fDistToEnemyMarkXZ< 250.0f)
			{
				pT->m_AttackPtSearch.m_uRangeXZMin = (u8) _pWD->fDistToEnemyMarkXZ;
				pT->m_AttackPtSearch.m_uRangeXZMax = (u8) _pWD->fDistToEnemyMarkXZ+5;
			}
			else
			{
				pT->m_AttackPtSearch.m_uRangeXZMin = 0;
				pT->m_AttackPtSearch.m_uRangeXZMax = 0;
			}
			_EnemyRelAttackPtWork(pT, OK_TOREQUESTENEMYRELATTACKPTPATHS, SEARCH_IS_SUCCESS_ON_PATHCOMPLETE);


			if (pT->GetBrain()->GetMechUseageCtrl() != CAIBrain::MECH_USEAGE_CTRL_NEVER &&
				((_pWD->uNowTime - pT->m_uTacticInitTime) & 7 )==7 && 
				_pWD->uLastFrameNowTime != _pWD->uNowTime &&
				!pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS))
			{
				pT->m_uStandGroundState = CGroundCombat::STANDGROUNDSTATE_VEHICLESEARCH;

			}
		}
	}

	return FALSE;
}

BOOL _GANoneInit(u32 uControl, void* pParam1, void* pParam2)
{
	return CFSM::INITSTATE_DONE;
}

BOOL _GANoneWork(u32 uControl, void* pParam1, void* pParam2)
{
	return FALSE;
}

BOOL _GANoneUninit(u32 uControl, void* pParam1, void* pParam2)
{
	return CFSM::INITSTATE_DONE; 
}

void _GARangeSetParams(	CGroundCombat* pT,
					    u8 uDodgeTimeMin,
						u8 uDodgeTimeMax,
						u8 uDodgeOutTimeMin,
						u8 uDodgeOutTimeMax,
						u8 uDamageDodgeTimeMin,
						f32 fRangeMin,
						f32 fRangeMax, 
						u8 uEnemyRelAttackPtSearchUsesage,	 //0 = NEVER, 1 = TIMER_BASED, 2 = WHEN_LOS_IS_LOST
						f32 fEraptLosIsLostWaitTime)

{
	pT->m_uDodgeTimeMin = uDodgeTimeMin;
	pT->m_uDodgeTimeMax = uDodgeTimeMax;
	pT->m_uDodgeOutTimeMin = uDodgeOutTimeMin;
	pT->m_uDodgeOutTimeMax = uDodgeOutTimeMax;
	pT->m_uDamageDodgeTimeMin = uDamageDodgeTimeMin;
	pT->m_fRangeMin = fRangeMin;
	pT->m_fRangeMax = fRangeMax;
	pT->m_uRangeTactic_EraptUseageCtrl = uEnemyRelAttackPtSearchUsesage;
	pT->m_fEraptLosIsLostWaitTime = fEraptLosIsLostWaitTime;

	if (pT->m_uRangeTactic_EraptUseageCtrl != CGroundCombat::ERAPTUSEAGECTRL_NEVER)
	{
		pT->m_AttackPtSearch.SetAttackPtSearchParams(	2,// u8 uRepeatTimeMin,
														4,// u8 uRepeatTimeMax,
														1,// u8 uDirInc,
														0,// u8 uDirMin,
														3,// u8 uDirMax,
														5,// s8 nRangeXZInc
														fRangeMin < 255.0f ? (u8) fRangeMin : 255, // u8 uRangeXZMax
														fRangeMax < 255.0f ? (u8) fRangeMax : 255, // u8 uRangeXZMin
														0,
														0,
														0);
	}
}


BOOL _GARangeInit(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;

		pT->ResetCommonTacticVbls();

	pT->ResetDodge();

	if (pT->m_uRangeTactic_EraptUseageCtrl != CGroundCombat::ERAPTUSEAGECTRL_NEVER)
	{
		pT->m_AttackPtSearch.InitEnemyRel();
	}

	pT->m_uRangePathFailureCount = 0;
	pT->m_uRangeTacticSmartEraptDecisionTimeOut = 0;
	return CFSM::INITSTATE_DONE;
}


BOOL _GARangeUninit(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	pT->ResetDodge();

	u8 uAttackSearchSlot;
	if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_RANGE, &uAttackSearchSlot))
	{
		pT->FreeRequestSlot(uAttackSearchSlot);
	}

	if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackSearchSlot))
	{
		pT->FreeRequestSlot(uAttackSearchSlot);
	}

	return CFSM::INITSTATE_DONE; 
}



BOOL _GARangeWork(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	u8 uAttackSearchSlot = 0;
	BOOL bRangePathInProgress = pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_RANGE);
	BOOL bAttackPathInProgress = pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS);
	BOOL bSmartErapt = FALSE;
	
	//
	//  let the dodge tactic make local moves using (passing of the param means don't even try to make moves if working on a new attack pos
	//
	BOOL bOkToRequestDodgePaths = !(bRangePathInProgress || bAttackPathInProgress);
	pT->DoDodgeWork(bOkToRequestDodgePaths);

	if (pT->m_pEnemy)
	{

		//
		//    Is it a good time for a non time-based ERAPT?
		//		- and has stayputdist been broken
		if (pT->m_uRangeTactic_EraptUseageCtrl & CGroundCombat::ERAPTUSEAGECTRL_SMART &&
			_pWD->fDistToEnemyMarkXZ > _pWD->pMover->GetRadiusXZ()*2.0f &&
			(_pWD->uStayPutDist == 0xffff || (pT->m_uAttackFlags & CGroundCombat::FLAG_STAYPUTBARRIER_BROKEN)))
		{
			if (pT->m_uRangeTacticSmartEraptDecisionTimeOut == 0)
			{
				if (pT->m_fTimeWithoutLOS > pT->m_fEraptLosIsLostWaitTime)
				{
					bSmartErapt = TRUE;
				}
				else if (pT->m_fTimeWithLOS > 0.25f && pT->m_fTimeWithoutTargetLock > pT->m_fEraptLosIsLostWaitTime)
				{
					bSmartErapt = TRUE;
				} 
				else if (pT->m_fTimeWithLOS > 3.0f && pT->m_fTimeWithoutTargetLock > 3.0f)
				{
					bSmartErapt = TRUE;
				}

				if (bSmartErapt)
				{
					if (_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTPRED)
					{
						pT->m_uRangeTacticSmartEraptDecisionTimeOut = aiutils_GetTimeOutRandMinMax((u8)50, (u8)100);
					}
					else
					{
						pT->m_uRangeTacticSmartEraptDecisionTimeOut = aiutils_GetTimeOutRandMinMax((u8)5, (u8)10);
					}
				}
			}

			if (pT->m_uRangeTacticSmartEraptDecisionTimeOut > _pWD->uNowTime)
			{
				bSmartErapt = TRUE;
			}

		}

		
		BOOL bOkToRequestEnemyRelAttackPtPaths = FALSE;
		if (pT->m_uRangeTacticSmartEraptDecisionTimeOut > _pWD->uNowTime)
		{
			_EnemyRelAttackPtWork(pT, !bRangePathInProgress, NOT_SEARCH_IS_SUCCESS_ON_PATHCOMPLETE);	 //can we find another point to attack from?

			//when is an erapt pt good.....
			if (pT->m_fTimeWithLOS > 0.25f)
			{
				pT->m_AttackPtSearch.AdvanceEnemyRel(CAttackPtSearch::BOOL_LAST_SEARCH_SUCCEEDED);
				pT->m_uRangeTacticSmartEraptDecisionTimeOut = 0;
			}
		}
		else if (pT->m_uRangeTactic_EraptUseageCtrl & CGroundCombat::ERAPTUSEAGECTRL_TIMER_BASED)
		{
			_EnemyRelAttackPtWork(pT, !bRangePathInProgress, SEARCH_IS_SUCCESS_ON_PATHCOMPLETE);	 //can we find another point to attack from?
		}
		
		
		//if 
		//	   EnemyDist > Max Range
		//	   have seen enemy at the mark recently
		//     not working on a range path already
		//then
		//	   Request a path to move toward last known enemy position
		if (_pWD->fDistToEnemyMarkXZ > pT->m_fRangeMax &&
			(pT->m_uRangeTactic_EraptUseageCtrl == CGroundCombat::ERAPTUSEAGECTRL_NEVER  || (pT->m_fTimeWithoutLOS < 1.0f)) &&
			!pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_RANGE))
		{
			CFVec3A AdvancePt;
			BOOL bRequested = FALSE;
			if (pT->m_uRangePathFailureCount < 5)
			{
				if ((pT->*(_pWD->pFindRangeAcquirePos_Func))(&AdvancePt, _pWD->EnemyMark, 0.0f))
				{
					bRequested = pT->RequestSearch(AdvancePt, CGroundCombat::SEARCHREASON_RANGE, 1.0f+((f32)pT->m_uRangePathFailureCount));
				}
			}
			else if (pT->m_uRangePathFailureCount < 10 && _pWD->pFindAdvancePos_Func)
			{
				f32 fScanDist = _pWD->fDistToEnemyMarkXZ;
				fScanDist -= _pWD->pMover->GetRadiusXZ();
				if (fScanDist > 0.0f)
				{


					if ((pT->*(_pWD->pFindAdvancePos_Func))(&AdvancePt,
															_pWD->pMover->GetLoc(),
															_pWD->DeltaToEnemyMarkUnit,
															fmath_Div(fScanDist, 5.0f)*(f32)(10 - pT->m_uRangePathFailureCount),
											 				0.0f)) //yOff



					{
						bRequested = pT->RequestSearch(AdvancePt, CGroundCombat::SEARCHREASON_RANGE, 5.0f);
					}
				}
			}

			if (!bRequested)
			{
				pT->m_uRangePathFailureCount++;
				if (pT->m_uRangePathFailureCount>60 ||
					pT->m_fEnemyMarkLastSmallGridChangeTime < 0.1f)
				{
					pT->m_uRangePathFailureCount = 0;
				}
			}
		}

		//if 
		//	   EnemyDist < Min Range
		//	   have seen enemy at the mark recently
		//     not working on a range path already
		//then
		//	   Request a path to move away from last known enemy position
		if (_pWD->fDistToEnemyMarkXZ < pT->m_fRangeMin &&
			(pT->m_uRangeTactic_EraptUseageCtrl == CGroundCombat::ERAPTUSEAGECTRL_NEVER  || !pT->IsFollowingPath(TRUE, CGroundCombat::SEARCHREASON_NEWATTACKPOS)) &&
			!pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_RANGE))
		{
			CFVec3A RetreatPt;
			BOOL bRequested = FALSE;
			if (pT->m_uRangePathFailureCount < 3)
			{
				if ((pT->*(_pWD->pFindRetreatPos_Func))(	&RetreatPt,
														_pWD->pMover->GetLoc(),
														_pWD->DeltaToEnemyMarkUnit,
														(pT->m_fRangeMin - _pWD->fDistToEnemyMarkXZ)+5.0f-pT->m_uRangePathFailureCount,
														0.25f*pT->m_uRangePathFailureCount)) 
				{
					bRequested = pT->RequestSearch(RetreatPt, CGroundCombat::SEARCHREASON_RANGE, 0.25f*pT->m_uRangePathFailureCount);
				}
			}
			else if (pT->m_uRangePathFailureCount < 12)
			{
				u8 uDir = pT->m_uRangePathFailureCount % CGroundCombat::NUM_RELATIVE_DIRS;
				f32 fRange = pT->m_fRangeMin;
				if (pT->m_uRangePathFailureCount > 8)
				{
					fRange *= 2.0f;
				}

				if ((pT->*(_pWD->pFindCurRelPos_Func))(	&RetreatPt,					//CFVec3A *pResult,
														_pWD->EnemyMark,			//	const CFVec3A& EnemyPos,
														_pWD->pMover->GetLoc(),		//	const CFVec3A& CurPos,
														pT->m_fRangeMin,			//	f32 fLocalDist,	
														0.0f, //yOff				//	f32 fYOff,	
														uDir,						//	u32 uLocalDir,	
														FALSE,						//	BOOL bCheckLOSToEnemy,	
														pT->m_fRangeMin,			//	f32 fMinDistToEnemy,	
														pT->m_fRangeMax))			//	f32 fMaxDistToEnemy);	
				{																												
					bRequested = pT->RequestSearch(RetreatPt, CGroundCombat::SEARCHREASON_RANGE, 5.0f);										
				}																												
			}
			
			if (!bRequested)
			{
				pT->m_uRangePathFailureCount++;
				if (pT->m_uRangePathFailureCount>60 ||
					pT->m_fEnemyMarkLastSmallGridChangeTime < 0.1f)
				{
					pT->m_uRangePathFailureCount = 0;
				}
			}
		}
	}


	//if 
	//		just completed a r-a-p (range-acquisition-path)
	//then
	//		release the r-a-p path slot
	if (pT->m_bPathJustCompleted &&
		pT->m_nLastPathReason == CGroundCombat::SEARCHREASON_RANGE)
	{
		if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_RANGE, &uAttackSearchSlot))
		{
			pT->FreeRequestSlot(uAttackSearchSlot);
		}
	}
	
	//if
	//		a r-a-p fails 
	//then
	//		reset clear the search slot
	if (pT->IsSearchFailed(CGroundCombat::SEARCHREASON_RANGE))
	{
		if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_RANGE, &uAttackSearchSlot))
		{
			pT->FreeRequestSlot(uAttackSearchSlot);
		}
		pT->m_uRangePathFailureCount++;
	}

	
	//if
	//	  Is Stuck ||
	//			EnemyDist > Min Range &&
	//			EnemyDist < Max Range &&
	//			Following a r-a-p
	//then
	//    stop following the r-a-p &&
	//    clear the r-a-p slot
	if (_pWD->pMover->IsStuck(2.2f) ||
		(	pT->IsFollowingPath(TRUE, CGroundCombat::SEARCHREASON_RANGE) &&
			_pWD->fDistToEnemyMarkXZ > pT->m_fRangeMin &&	  //need decision latency here.
			_pWD->fDistToEnemyMarkXZ < pT->m_fRangeMax-2.0f*_pWD->pMover->GetRadiusXZ() ) )
	{
		if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_RANGE, &uAttackSearchSlot))
		{
			pT->FreeRequestSlot(uAttackSearchSlot);
		}
	}
	
	//if 
	//		not following r-a-p path &&
	//		one is available
	//then
	//		follow it										  
	if (!pT->IsFollowingPath(TRUE, CGroundCombat::SEARCHREASON_RANGE) &&
		pT->IsSearchDone(CGroundCombat::SEARCHREASON_RANGE, &uAttackSearchSlot))
	{
		pT->m_uRangePathFailureCount = 0;
		pT->Attack_AssignPath(uAttackSearchSlot);
	}

	return FALSE;
}


BOOL _GARange3DInit(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	pT->ResetCommonTacticVbls();

	pT->ResetDodge();

	if (pT->m_uRangeTactic_EraptUseageCtrl != CGroundCombat::ERAPTUSEAGECTRL_NEVER)
	{
		pT->m_AttackPtSearch.InitEnemyRel();
	}

	pT->m_uRangePathFailureCount = 0;
	return CFSM::INITSTATE_DONE;
}


BOOL _GARange3DUninit(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	pT->ResetDodge();

	u8 uAttackSearchSlot;
	if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_RANGE, &uAttackSearchSlot))
	{
		pT->FreeRequestSlot(uAttackSearchSlot);
	}

	if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackSearchSlot))
	{
		pT->FreeRequestSlot(uAttackSearchSlot);
	}

	return CFSM::INITSTATE_DONE; 
}


BOOL _GARange3DWork(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	u8 uAttackSearchSlot = 0;
	BOOL bRangePathInProgress = pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_RANGE);
	BOOL bAttackPathInProgress = pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS);

	//  let the dodge tactic make local moves using (passing of the param means don't even try to make moves if working on a new attack pos
	BOOL bOkToRequestDodgePaths = !(bRangePathInProgress || bAttackPathInProgress);
	pT->DoDodgeWork(bOkToRequestDodgePaths);
	
	// if 
	//		No RangePath In progress
	//		StayPutDist has been broken &&
	//		No LOS for ** secs &&
	// then
	//		start a range attack pt search
	BOOL bOkToRequestEnemyRelAttackPtPaths = FALSE;
	if (pT->m_uRangeTactic_EraptUseageCtrl & CGroundCombat::ERAPTUSEAGECTRL_SMART)
	{
		if (!bRangePathInProgress &&
			(_pWD->uStayPutDist == 0xffff || (pT->m_uAttackFlags & CGroundCombat::FLAG_STAYPUTBARRIER_BROKEN)) &&
			(pT->m_uAttackFlags & CGroundCombat::FLAG_CANTFINDENEMY) &&
			(aiutils_GetCurTimeSecsU16() - pT->m_uLostEnemyTime > pT->m_fEraptLosIsLostWaitTime))
		{
			bOkToRequestEnemyRelAttackPtPaths = TRUE;
		}
		_EnemyRelAttackPtWork(pT, bOkToRequestEnemyRelAttackPtPaths, NOT_SEARCH_IS_SUCCESS_ON_PATHCOMPLETE);	 //can we find another point to attack from?

		if (pT->m_fTimeWithLOS > 0.25f)
		{
			pT->m_AttackPtSearch.AdvanceEnemyRel(CAttackPtSearch::BOOL_LAST_SEARCH_SUCCEEDED);
		}

	}
	else if (pT->m_uRangeTactic_EraptUseageCtrl & CGroundCombat::ERAPTUSEAGECTRL_TIMER_BASED)
	{
		_EnemyRelAttackPtWork(pT, !bRangePathInProgress, SEARCH_IS_SUCCESS_ON_PATHCOMPLETE);	 //can we find another point to attack from?
	}

	//if 
	//	   EnemyDist > Max Range
	//	   have seen enemy at the mark recently
	//     not working on a range path already
	//then
	//	   Request a path to move toward last known enemy position
	if (_pWD->fDistToEnemyMark > pT->m_fRangeMax+_pWD->pMover->GetRadiusXZ() &&
		//(pT->m_uRangeTactic_EraptUseageCtrl == CGroundCombat::ERAPTUSEAGECTRL_NEVER  || (pT->m_fTimeWithoutLOS < 1.0f)) &&
		!pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_RANGE))
	{
		CFVec3A AdvancePt;
		if (pT->m_uRangePathFailureCount < 5)
		{
			AdvancePt = _pWD->EnemyMark;
			AdvancePt.y +=1.0f;
			pT->RequestSearch(AdvancePt, CGroundCombat::SEARCHREASON_RANGE, 5.0f);
		}
		else if (pT->m_uRangePathFailureCount < 10 && _pWD->pFindAdvancePos_Func)
		{
			if ((pT->*(_pWD->pFindAdvancePos_Func))(&AdvancePt,
													_pWD->pMover->GetLoc(),
													_pWD->pMover->GetEntity()->MtxToWorld()->m_vFront,
													fmath_Div(_pWD->fDistToEnemyMark, 5.0f)*(f32)(10 - pT->m_uRangePathFailureCount),
													0.0f)) //yOff



			{
				pT->RequestSearch(AdvancePt, CGroundCombat::SEARCHREASON_RANGE, 5.0f);
			}
		}
		else
		{
			pT->m_uRangePathFailureCount++;
			if (pT->m_uRangePathFailureCount>60 ) 
			{
				pT->m_uRangePathFailureCount = 0;
			}
		}
	}

	//if 
	//	   EnemyDist < Min Range
	//	   have seen enemy at the mark recently
	//     not working on a range path already
	//then
	//	   Request a path to move away from last known enemy position
	if (_pWD->fDistToEnemyMarkXZ < pT->m_fRangeMin &&
	//	(pT->m_uRangeTactic_EraptUseageCtrl == CGroundCombat::ERAPTUSEAGECTRL_NEVER  || (pT->m_fTimeWithoutLOS < 1.0f)) &&
		!pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_RANGE))
	{
		CFVec3A RetreatPt;
		if ((pT->*(_pWD->pFindRetreatPos_Func))(	&RetreatPt,
												_pWD->pMover->GetLoc(),
												_pWD->pMover->GetEntity()->MtxToWorld()->m_vFront,
												(pT->m_fRangeMin - _pWD->fDistToEnemyMarkXZ)+5.0f,
												0.0f)) //yOff
		{
			RetreatPt.y = _pWD->EnemyMark.y+1.0f;
			pT->RequestSearch(RetreatPt, CGroundCombat::SEARCHREASON_RANGE, 5.0f);
		}
	}



	//if 
	//		just completed a r-a-p (range-acquisition-path)
	//then
	//		release the r-a-p path slot
	if (pT->m_bPathJustCompleted &&
		pT->m_nLastPathReason == CGroundCombat::SEARCHREASON_RANGE)
	{
		if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_RANGE, &uAttackSearchSlot))
		{
			pT->FreeRequestSlot(uAttackSearchSlot);
		}
	}
	
	//if
	//		a r-a-p fails 
	//then
	//		reset clear the search slot
	if (pT->IsSearchFailed(CGroundCombat::SEARCHREASON_RANGE))
	{
		if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_RANGE, &uAttackSearchSlot))
		{
			pT->FreeRequestSlot(uAttackSearchSlot);
		}
		pT->m_uRangePathFailureCount++;
	}

	
	//if
	//	  Is Stuck ||
	//			EnemyDist > Min Range &&
	//			EnemyDist < Max Range &&
	//			Following a r-a-p
	//then
	//    stop following the r-a-p &&
	//    clear the r-a-p slot
	if (_pWD->pMover->IsStuck(2.2f) ||
		(	pT->IsFollowingPath(TRUE, CGroundCombat::SEARCHREASON_RANGE) &&
			_pWD->fDistToEnemyMarkXZ > pT->m_fRangeMin &&	  //need decision latency here.
			_pWD->fDistToEnemyMark < pT->m_fRangeMax-_pWD->pMover->GetRadiusXZ() ) )
	{
		if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_RANGE, &uAttackSearchSlot))
		{
			pT->FreeRequestSlot(uAttackSearchSlot);
		}
	}
	
	//if 
	//		not following r-a-p path &&
	//		one is available
	//then
	//		follow it										  
	if (!pT->IsFollowingPath(TRUE, CGroundCombat::SEARCHREASON_RANGE) &&
		pT->IsSearchDone(CGroundCombat::SEARCHREASON_RANGE, &uAttackSearchSlot))
	{
		pT->m_uRangePathFailureCount = 0;
		pT->Attack_AssignPath(uAttackSearchSlot);
	}

	return FALSE;
}


void _GACircleReset(CGroundCombat* pT)
{
	u8 uAttackSearchSlot;
	if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackSearchSlot))
	{
		pT->FreeRequestSlot(uAttackSearchSlot);
	}
}


BOOL _GACircleInit(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	pT->ResetCommonTacticVbls();

	_GACircleReset(pT);
	pT->m_AttackPtSearch.m_nCurDir = pT->GetBrain()->GetInfreqCycleRandom() > 0.5f;
	return CFSM::INITSTATE_DONE;
}


BOOL _GACircleUninit(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	u8 uAttackSearchSlot;
	if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackSearchSlot))
	{
		pT->FreeRequestSlot(uAttackSearchSlot);
	}
	return CFSM::INITSTATE_DONE; 
}


BOOL _GACircleWork(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	u8 uCircleSlot;


	//if
	//  need a path to do something
	//then
	//	request it
  	if (!(pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS)))
	{

		CFVec3A NewCirclePt;
		f32 fCircleRadius = _pWD->fDistToEnemyMark;

		if (_pWD->pEnemyPoi)
		{	//use range info stored inside of the enemy poi to decide on a good circle strafe distance
			fCircleRadius = _pWD->pEnemyPoi->GetMin360Range();
			if (fCircleRadius > _pWD->fDistToEnemyMark)
			{
				fCircleRadius = _pWD->fDistToEnemyMark;
			}
		}

		f32 _fMinCircleRadius = 5.0f;
		if ((pT->*(_pWD->pFindCurRelPos_Func))(&NewCirclePt,
											_pWD->EnemyMark,
											_pWD->pMover->GetLoc(),
											5.0f,		//fDist
											0.0f,		//fYOff
											pT->m_AttackPtSearch.m_nCurDir,
											DONT_CHECK_LOS_TO_ENEMY_MARK,
											_fMinCircleRadius,
											fCircleRadius+2.0f))
		{
			pT->RequestSearch(NewCirclePt, CGroundCombat::SEARCHREASON_NEWATTACKPOS, 5.0f);
		}
		else
		{
			pT->m_AttackPtSearch.m_nCurDir = 1 - pT->m_AttackPtSearch.m_nCurDir;
		}
	}

	//if 
	//		just completed a c-p (circle-path)
	//then
	//		release the c-p slot
	if (pT->m_bPathJustCompleted &&
		pT->m_nLastPathReason == CGroundCombat::SEARCHREASON_NEWATTACKPOS)
	{
		if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uCircleSlot))
		{
			pT->FreeRequestSlot(uCircleSlot);
		}
	}

	//if
	//		a r-a-p fails 
	//then
	//		reset clear the search slot
	if (pT->IsSearchFailed(CGroundCombat::SEARCHREASON_NEWATTACKPOS))
	{
		if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uCircleSlot))
		{
			pT->FreeRequestSlot(uCircleSlot);
		}
		pT->m_AttackPtSearch.m_nCurDir = 1 - pT->m_AttackPtSearch.m_nCurDir;
	}

	//if 
	//		not following r-a-p path &&
	//		one is available
	//then
	//		follow it
	if (!pT->IsFollowingPath(TRUE, CGroundCombat::SEARCHREASON_NEWATTACKPOS) &&
		pT->IsSearchDone(CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uCircleSlot))
	{
		pT->Attack_AssignPath(uCircleSlot);
	}

	
	if (pT->IsFollowingPath())
	{
		pT->m_uAttackFlags |= CGroundCombat::FLAG_TRIGGEROVERRIDE_STOPSHOOT;
	}

	
	return FALSE;
}


void _GAPeekABooReset(CGroundCombat* pT)
{
	u8 uTakeCoverSlot;
	u8 uLeaveCoverSlot;
	u8 uRetreatSlot;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;

	pT->m_uPeekABooState = CGroundCombat::TCSTATE_EXPOSED;
	pT->m_ExposedPos = _pWD->pMover->GetLoc();		//a place where I was exposed to the EnemyMark
	
	_InitCoverPtSearch(&(pT->m_CoverPtSearch), (pT->m_uAttackFlags & CGroundCombat::FLAG_PANIC_ON));
	
	pT->m_AttackPtSearch.InitLocal();

	if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_TAKINGCOVER, &uTakeCoverSlot))
	{
		pT->FreeRequestSlot(uTakeCoverSlot);
	}

	if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_LEAVINGCOVER, &uLeaveCoverSlot))
	{
		pT->FreeRequestSlot(uLeaveCoverSlot);
	}

	if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_RETREAT, &uRetreatSlot))
	{
		pT->FreeRequestSlot(uRetreatSlot);
	}
}


BOOL _GAPeekABooInit(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;

	pT->ResetCommonTacticVbls();
	_GAPeekABooReset(pT);
	return CFSM::INITSTATE_DONE;
}


void _GAPeekABooSetParams(CGroundCombat* pT,
						  u8 uAttackPtTimeMin,				//timer for when to come out of cover!                           
						  u8 uAttackPtTimeMax,				                                                                 
						  u8 uAttackFromCoverTimeMin,		//timer for how long to stay out of cover under normal conditions
						  u8 uAttackFromCoverTimeMax)
{
	pT->m_AttackPtSearch.m_uRepeatTimeMin = uAttackPtTimeMin;
	pT->m_AttackPtSearch.m_uRepeatTimeMax = uAttackPtTimeMax;
	pT->m_uAttackFromCoverTimeMin = uAttackFromCoverTimeMin;
	pT->m_uAttackFromCoverTimeMax = uAttackFromCoverTimeMax;
}


BOOL _GAPeekABooUninit(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	u8 uTakeCoverSlot;
	u8 uLeaveCoverSlot;
	u8 uRetreatSlot;

	if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_TAKINGCOVER, &uTakeCoverSlot))
	{
		pT->FreeRequestSlot(uTakeCoverSlot);
	}

	if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_LEAVINGCOVER, &uLeaveCoverSlot))
	{
		pT->FreeRequestSlot(uLeaveCoverSlot);
	}

	if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_RETREAT, &uRetreatSlot))
	{
		pT->FreeRequestSlot(uRetreatSlot);
	}

	if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uRetreatSlot))
	{
		pT->FreeRequestSlot(uRetreatSlot);
	}
	
	return CFSM::INITSTATE_DONE; 
}


BOOL _GAPeekABooWork(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	u16 uCoverForTime = 5;
	u8 uTakeCoverSlot;
	u8 uLeaveCoverSlot;
	u8 uRetreatSlot;
	

	//if
	//      there is a take cover path working or available &&
	//		takeCover position stops being cover 
	//then
	//		state = exposed
	//		reset cur path
	//		remove any leaveCover path in search queue
	//		remove any takeCover path in search queue
	if (pT->m_uPeekABooState == CGroundCombat::TCSTATE_ATTACK_FROM_COVER)	 //when attacking from cover, how long to stay exposed for (under normal conditions, ie. if not getting shot, or encroached apon, etc)
	{
		BOOL bBailOutOfAttackFromCover = FALSE;
		if (pT->m_uAttackFromCoverTimeOut < _pWD->uNowTime)
		{
			bBailOutOfAttackFromCover = TRUE;
		}
		if ((pT->m_uAttackFlags & CGroundCombat::FLAG_HASTARGETLOS) &&
			(_pWD->fDistToEnemyMark < 7.0f))
		{
			bBailOutOfAttackFromCover = TRUE;
		}
		else if (pT->m_fDamageTakenPerSec > 0.15f)   //bail out of attack from cover
		{
			bBailOutOfAttackFromCover = TRUE;
		}
		
		if (bBailOutOfAttackFromCover)
		{
			pT->m_uPeekABooState = CGroundCombat::TCSTATE_EXPOSED;
			if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_LEAVINGCOVER, &uLeaveCoverSlot))
			{
				pT->FreeRequestSlot(uLeaveCoverSlot);
			}
			pT->m_AttackPtSearch.AdvanceLocal(CAttackPtSearch::BOOL_LAST_SEARCH_SUCCEEDED);
		}
	}


	if ((pT->m_uPeekABooState == CGroundCombat::TCSTATE_COVER) &&
		(	(!pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_LEAVINGCOVER) &&
				((pT->m_uAttackFlags & CGroundCombat::FLAG_HASTARGETLOS) ||
				(_pWD->pDamagedByMem && (_pWD->pDamagedByMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY))	)
			) ||
			(	pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_LEAVINGCOVER) &&
				(pT->m_uAttackFlags & CGroundCombat::FLAG_HASTARGETLOS) &&
				(_pWD->fDistToEnemyMark < 7.0f)
			)
		))
	{
	   _GAPeekABooReset(pT);
	}


	//if
	//   stuck
	//then
	//   reset: try to find cover
	if (_pWD->pMover->IsStuck(1.2f))
	{
	   _GAPeekABooReset(pT);
	}


	//if 
	//		state == EXPOSED  &&
	//		there is no t-c-p path in the works or in use   &&
	//		it is time to take cover
	//then
	//    find a cover position
	//    and request a path from ExposePos to cover position
	if ((pT->m_uPeekABooState == CGroundCombat::TCSTATE_EXPOSED) &&
		!pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_TAKINGCOVER, &uTakeCoverSlot))	//note! CoverPtTimeOut is reset if state goes to exposed involuntarily
	{
		CFVec3A CoverPos;
		if ((pT->*(_pWD->pFindCoverPos_Func))(&CoverPos,
											_pWD->EnemyMark,
											_pWD->pMover->GetLoc(),
											pT->m_CoverPtSearch.m_fCoverSearchAngle,
											0.0f,
											pT->m_CoverPtSearch.m_fCoverSearchRange))
		{
			pT->RequestSearch(CoverPos, CGroundCombat::SEARCHREASON_TAKINGCOVER, 2.0f);
			if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_TAKINGCOVER, &uTakeCoverSlot))
			{
				//special path that should avoid the mark en-route to new attack point
				pT->m_aSearchQuery[uTakeCoverSlot].SetAvoidPt(_pWD->EnemyMark, 10.5f);
				pT->m_aSearchQuery[uTakeCoverSlot].SetParams(pT->m_aSearchQuery[uTakeCoverSlot].GetParams() | CSearchQuery::SEARCHPARAM_AVOIDPT_PATH);
			}
		}
		else
		{
			_AdvanceCoverPtSearch(&(pT->m_CoverPtSearch), CCoverPtSearch::BOOL_LAST_SEARCH_FAILED, (pT->m_uAttackFlags & CGroundCombat::FLAG_PANIC_ON));
		}
	}
	

	//if 
	//	  state == EXPOSED &&
	//	  there is no retreat path in the works, or in use
	//then
	//    request a retreat path
	if ((pT->m_uPeekABooState == CGroundCombat::TCSTATE_EXPOSED) &&
		!pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_RETREAT, &uRetreatSlot))
	{
		CFVec3A RetreatPos;
		f32 _fDefaultRetreatDist = 5.0f;
		CFVec3A DeltaToEnemyUnit;
		if (_pWD->fDistToEnemyMark > 0.001f)
		{
			DeltaToEnemyUnit = _pWD->DeltaToEnemyMarkUnit;
		}
		else
		{
			DeltaToEnemyUnit.Set(_pWD->pMover->GetEyeLookAt());
		}

		if ((pT->*(_pWD->pFindRetreatPos_Func))(&RetreatPos,
											_pWD->pMover->GetLoc(),
											DeltaToEnemyUnit,
											_fDefaultRetreatDist,
											0.0f))
		{
			pT->RequestSearch(RetreatPos, CGroundCombat::SEARCHREASON_RETREAT, 2.0f);
		}
	}
	
	
	//if
	//
	//	  state = takingCover &&
	//	  not currently working on a l-c-p path &&
	//	  the local area hasn't been completely, unsuccessfully scanned for a good l-c-p &&
	//    time to leave cover for attacking purposes
	//
	//then
	//	  come out of hiding
	//
  	if ((pT->m_uPeekABooState == CGroundCombat::TCSTATE_COVER) && 
		!pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_LEAVINGCOVER) &&
		pT->m_AttackPtSearch.m_uFailureCount < 16 &&
		pT->m_AttackPtSearch.IsTimeForNewSearch())
	{
		if ((pT->*(_pWD->pFindCurRelPos_Func))(&(pT->m_ExposedPos), _pWD->EnemyMark, _pWD->pMover->GetLoc(), pT->m_AttackPtSearch.m_nCurRangeXZ, pT->m_AttackPtSearch.m_nCurRangeY, pT->m_AttackPtSearch.m_nCurDir, CHECK_LOS_TO_ENEMY_MARK, -1.0f, -1.0f))
		{
			pT->RequestSearch(pT->m_ExposedPos, CGroundCombat::SEARCHREASON_LEAVINGCOVER, 2.0f);
		}
		else
		{
			pT->m_AttackPtSearch.AdvanceLocal(CAttackPtSearch::BOOL_LAST_SEARCH_FAILED);
		}
	}

	//if
	//		just completed a TC path
	//then
	//		state = TCSTATE_COVER
	if (pT->m_bPathJustCompleted &&
		pT->m_nLastPathReason == CGroundCombat::SEARCHREASON_TAKINGCOVER)
	{
		pT->m_uPeekABooState = CGroundCombat::TCSTATE_COVER;

		_AdvanceCoverPtSearch(&(pT->m_CoverPtSearch), CCoverPtSearch::BOOL_LAST_SEARCH_SUCCEEDED, (pT->m_uAttackFlags & CGroundCombat::FLAG_PANIC_ON));

		if (pT->m_pLookAtMgr)
		{
    		pT->m_pLookAtMgr->ResetVisRayScan();		 //should do this whenever we're gonna be stopped for a while
		}
		
		if (_pWD->pBot && pT->m_pLookAtMgr)
		{
			CFVec3A LookAtPt;
			LookAtPt.Set(((CBot*) _pWD->pMover->GetEntity())->m_VelocityXZ_WS);
			LookAtPt.Mul(-1.0f);
			LookAtPt.Add(_pWD->pMover->GetLoc());
			pT->m_pLookAtMgr->ForceLookAtWhileScanning(LookAtPt);	   //try to look at the place I just came from
		}

		if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_TAKINGCOVER, &uTakeCoverSlot))
		{
			pT->FreeRequestSlot(uTakeCoverSlot);
		}

	}


	//if    
	//		just completed an l-c-p
	//then
	//      state = TCSTATE_ATTACK_FROM_COVER
	if (pT->m_bPathJustCompleted &&
		pT->m_nLastPathReason == CGroundCombat::SEARCHREASON_LEAVINGCOVER)
	{
		pT->m_uPeekABooState = CGroundCombat::TCSTATE_ATTACK_FROM_COVER;
		pT->m_uAttackFromCoverTimeOut = aiutils_GetTimeOutRandMinMax(pT->m_uAttackFromCoverTimeMin, pT->m_uAttackFromCoverTimeMax);

		//reset the retreat path now that we're at have a new location
		if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_LEAVINGCOVER, &uLeaveCoverSlot))
		{
			pT->FreeRequestSlot(uLeaveCoverSlot);
		}
	}


	//if    
	//		just completed a Retreat path
	//then
	//      reset the retreat path position
	if (pT->m_bPathJustCompleted &&
		pT->m_nLastPathReason == CGroundCombat::SEARCHREASON_RETREAT)
	{
		//reset the retreat path now that we're at have a new location
		if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_RETREAT, &uRetreatSlot))
		{
			pT->FreeRequestSlot(uRetreatSlot);
		}
	}

	
	//if
	//		a t-c-p fails 
	//then
	//		reset clear the search slot
	if (pT->IsSearchFailed(CGroundCombat::SEARCHREASON_TAKINGCOVER))
	{
		if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_TAKINGCOVER, &uTakeCoverSlot))
		{
			pT->FreeRequestSlot(uTakeCoverSlot);
		}
		_AdvanceCoverPtSearch(&(pT->m_CoverPtSearch), CCoverPtSearch::BOOL_LAST_SEARCH_FAILED, (pT->m_uAttackFlags & CGroundCombat::FLAG_PANIC_ON));
	}

	
	//if
	//		a l-c-p fails 
	//then
	//		reset clear the search slot
	if (pT->IsSearchFailed(CGroundCombat::SEARCHREASON_LEAVINGCOVER))
	{
		if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_LEAVINGCOVER, &uLeaveCoverSlot))
		{
			pT->FreeRequestSlot(uLeaveCoverSlot);
		}
		pT->m_AttackPtSearch.AdvanceLocal(CAttackPtSearch::BOOL_LAST_SEARCH_FAILED);
	}


	//if
	//		a retreat path fails 
	//then
	//		clear the t-c-p search slot
	if (pT->IsSearchFailed(CGroundCombat::SEARCHREASON_RETREAT))
	{
		if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_RETREAT, &uRetreatSlot))
		{
			pT->FreeRequestSlot(uRetreatSlot);
		}
	}


	//if
	//      state = exposed &&
	//		a t-c-p path is ready && 
	//      not currently following t-c-p  &&
	//		o.k to play peek aboo
	//then
	//		follow the new tc path
	if ((pT->m_uPeekABooState == CGroundCombat::TCSTATE_EXPOSED) &&
		!pT->IsFollowingPath(TRUE, CGroundCombat::SEARCHREASON_TAKINGCOVER) &&
		pT->IsSearchDone(CGroundCombat::SEARCHREASON_TAKINGCOVER, &uTakeCoverSlot))
	{
		u32 uAssignPathFlags = 0;			  
		if ((_pWD->pBot && (_pWD->pBot->IsAlertOn() || !_pWD->pBot->IsCapableofAlert())) && fmath_RandomChoice(100) < 90)
		{
			uAssignPathFlags |= CAIMover::ASSIGNPATHFLAG_CANHOP;
		}
		pT->Attack_AssignPath(uTakeCoverSlot, uAssignPathFlags);
	}


	//if
	//      state = cover &&
	//      not currently following l-c-p path &&
	//		a l-c-p path is ready && 
	//		o.k to play peek aboo
	//then
	//		follow the new tc path
	if ((pT->m_uPeekABooState == CGroundCombat::TCSTATE_COVER) &&
		!pT->IsFollowingPath(TRUE, CGroundCombat::SEARCHREASON_LEAVINGCOVER) &&
		pT->IsSearchDone(CGroundCombat::SEARCHREASON_LEAVINGCOVER, &uLeaveCoverSlot) &&
		pT->m_AttackPtSearch.IsTimeForNewSearch())	//note AttackPtTimeOutMax < AttackPtTimeOutMax means never willfully come out of cover
	{
		u32 uAssignPathFlags = 0;
		if ((_pWD->pBot && (_pWD->pBot->IsAlertOn() || !_pWD->pBot->IsCapableofAlert())) && fmath_RandomChoice(100) < 90)
		{
			if (pT->m_AttackPtSearch.m_uSearchType == CAttackPtSearch::ATTACKPT_SEARCHTYPE_LOCAL_ROLLTO)
			{
				uAssignPathFlags |= CAIMover::ASSIGNPATHFLAG_CANROLLALONG;
			}
			else 
			{
				uAssignPathFlags |= CAIMover::ASSIGNPATHFLAG_CANHOP;
			}
		}
		pT->Attack_AssignPath(uLeaveCoverSlot, uAssignPathFlags);
	}


	//if
	//      state == exposed &&
	//		not currently going anywhere &&
	//		a retreat path is ready && 
	//      don't have a l-c-p or a t-c-path ready
	//then
	//		follow the new retreat path
	if ((pT->m_uPeekABooState == CGroundCombat::TCSTATE_EXPOSED) &&
		!pT->IsFollowingPath() &&
		pT->IsSearchDone(CGroundCombat::SEARCHREASON_RETREAT, &uRetreatSlot) &&
		!pT->IsSearchDone(CGroundCombat::SEARCHREASON_LEAVINGCOVER, &uLeaveCoverSlot) &&
		!pT->IsSearchDone(CGroundCombat::SEARCHREASON_TAKINGCOVER, &uTakeCoverSlot))
	{
		pT->Attack_AssignPath(uRetreatSlot);
	}


	// if
	//	   leaving cover 
	// then
	//	   happy trigger  (also means look at mark)
	if (pT->IsFollowingPath(TRUE, CGroundCombat::SEARCHREASON_LEAVINGCOVER) || 
		(pT->m_uPeekABooState == CGroundCombat::TCSTATE_ATTACK_FROM_COVER && (pT->m_fTimeWithoutLOS < 1)))
	{
		pT->m_uAttackFlags |= CGroundCombat::FLAG_TRIGGERHAPPY;	 //will be reset every frame
		pT->m_uAttackFlags |= CGroundCombat::FLAG_TORSO_DIR_ENEMY;
	}

	
	//if
	//     state == cover &&
	//     I'm not moving &&
	//     it's been a while since seeing the player &&
	//     not been startled in awhile
	//then
	//     to a startled turn-jump
	if (pT->m_uPeekABooState == CGroundCombat::TCSTATE_COVER &&
		!pT->IsFollowingPath() &&
		_pWD->uNowTime - pT->m_uLastPeekABooSupriseTime > 5 &&
		_pWD->pHeardMem && 
		(_pWD->pHeardMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY) &&
		pT->m_fTimeWithoutLOS > 5.0f &&
		!((CBot*) _pWD->pMover->GetEntity())->IsJumping()) 
	{
		pT->m_uLastPeekABooSupriseTime = _pWD->uNowTime;
		_pWD->pMover->m_Controls.Jump();
		CFVec3A JumpDir;
		JumpDir.Set(_pWD->pHeardMem->m_EntityLoc);
		JumpDir.Sub(_pWD->pMover->GetLoc());
		JumpDir.y = 0.0f;
		f32 fMag = JumpDir.SafeUnitAndMagXZ(JumpDir);
		if (fMag > 0.0f)
		{
			JumpDir.Mul(1.0f/fMag*0.25f);
			JumpDir.y = 1.0f;
			JumpDir.Mul(20.0f);
			_pWD->pMover->m_Controls.JumpVec(JumpDir);
		}
	}
	
	return FALSE;
}


void _GASearchSetParams(CGroundCombat* pT)
{
}


BOOL _GASearchInit(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;

	pT->ResetCommonTacticVbls();
	FASSERT(!pT->m_pSearchTactic);
	pT->m_pSearchTactic = (CGroundSearch*) CGroundSearch::BankAccess(NULL);
	if (pT->m_pSearchTactic)
	{	//initialize/configure
		pT->m_pSearchTactic->Init(pT->GetBrain(), CAIThought::THOUGHTFLAG_GOAL_HEAD_PRIORITY, NULL);
		pT->m_pSearchTactic->SetLookAtMgr(pT->m_pLookAtMgr);
	}
	return CFSM::INITSTATE_DONE;
}


BOOL _GASearchWork(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;

	if (pT->m_pSearchTactic)
	{
		pT->m_pSearchTactic->Work();
	}

	return FALSE;
}


BOOL _GASearchUninit(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;

	if (pT->m_pSearchTactic)
	{
		pT->m_pSearchTactic->Cleanup();
		CGroundSearch::BankAccess(pT->m_pSearchTactic);
		pT->m_pSearchTactic = NULL;
	}
	return CFSM::INITSTATE_DONE; 
}
