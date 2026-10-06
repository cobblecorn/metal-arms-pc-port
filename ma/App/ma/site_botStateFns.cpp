//////////////////////////////////////////////////////////////////////////////////////
// site_BotStateFns.cpp -
//
// Author: Michael Scholz
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
// 11/12/02 MScholz     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "site_BotWeapon.h"
#include "site_FloorSentry.h"
#include "fworld_coll.h"
#include "ftext.h"
#include "gamepad.h"
#include "meshtypes.h"
#include "AI\AIEnviro.h"
#include "player.h"
#include "eparticlepool.h"
#include "MultiplayerMgr.h"
#include "vehicle.h"
#include "miscdraw.h"
#include "sas_user.h"

#define	REACHEDGOAL_YAWSIN 0.018f
// My choice is that all access to m_eSiteWeaponState are via this call

void CBotSiteWeapon::_SetSiteWeaponState(CBotSiteWeapon::State_e NewState)
{
	CBotSiteWeapon::State_e OldState = m_pData->m_eSiteWeaponState; 

	if (NewState == OldState) // transitional behavior unnecessary, return
		return;

	if (OldState == SITE_PATROLLING) // when leaving patrol mode, faster movement possible
	{
		m_RotateYAxis.SetMaxVelocity(m_pData->m_fMaxYawVelocity);
		m_RotateXAxis.SetMaxVelocity(m_pData->m_fMaxYawVelocity);
	}

	else if (OldState == SITE_ATTACKING) // when leaving attack mode, pretend to have just beeped
	{
		f32 fTimeNow = FLoop_nTotalLoopTicks*FLoop_fSecsPerTick;
		m_fLastBeepTime = fTimeNow;
	}

	else if (OldState == SITE_POWER_UP) 
	{
		m_RotateXAxis.SetValue(m_pData->m_fPitchWS);
	}

	else if (OldState == SITE_POWERING_UP_POSSESSED) 
	{
		m_RotateXAxis.SetValue(m_pData->m_fPitchWS);
		// Two things can happen, either we re-set this variable in "if (NewState == SITE_POSSESSED)"
		// or, we've interrupted the power up process, and this value is correct
		m_nPossessionPlayerIndex = -1;
	}

	else if (OldState == SITE_POSSESSED) 
	{
		// need to do this because some site wepons can be "driven" by players without going through the cbot::Possess() interface
        m_nPossessionPlayerIndex = -1;	   

		CBotSiteWeaponBuilder *pBuilder = (CBotSiteWeaponBuilder *)GetLeafClassBuilder();
		if ( m_pBotDef->m_nSubClass  == BOTSUBCLASS_SITEWEAPON_PILLBOX)		
			m_pBotDef = &pBuilder->m_AmbientPillBoxBotDef;
		if ( m_pBotDef->m_nSubClass  == BOTSUBCLASS_SITEWEAPON_RATGUN)		
			m_pBotDef = &pBuilder->m_AmbientRatGunBotDef;

		PlaySound(m_pSounds[SITESND_DISPOSSESS]);

		m_pData->m_pEnemyTarget=NULL;
		m_bDamageTaken = FALSE;
		
		_RotateBaseNoise(FALSE); // need to switch to 3d sounds
		if (m_pData->m_pDriverBot && //pgm added this to stop a crash.
			m_pData->m_pDriverBot->IsInWorld())
		{
			// if driver's spotlight was on when entering vehicle, turn it back on now.
			if( m_uVehicleFlags & VEHICLE_FLAG_DRIVER_LIGHT_ON )
			{
				m_pData->m_pDriverBot->SetSpotLightOn();
			}

			// if driver was casting a shadow before entering vehicle, turn it back on now.
			if( m_uVehicleFlags & VEHICLE_FLAG_DRIVER_HAS_SHADOW )
			{
				m_pData->m_pDriverBot->m_pWorldMesh->m_nFlags |= FMESHINST_FLAG_CAST_SHADOWS;
			}
		}
	}

	if ( (NewState == SITE_POWERING_UP_POSSESSED) ||
		 (NewState == SITE_POSSESSED) )
	{
		// need to do this because some site wepons can be "driven" by players without going through the cbot::Possess() interface
		m_nPossessionPlayerIndex = m_pData->m_pDriverBot->m_nPossessionPlayerIndex;
	}

	m_pData->m_eSiteWeaponState = NewState; // sets the new state 
	m_pData->m_fTimeSinceStateChanged = 0.0f; // reset state change counter
	m_pGun->CueNewState();

	if (NewState == SITE_IDLING) // only pillboxes / ratguns idle, except in multiplayer
	{							
		if ( (m_pBotDef->m_nSubClass != BOTSUBCLASS_SITEWEAPON_PILLBOX) && 
			 (m_pBotDef->m_nSubClass != BOTSUBCLASS_SITEWEAPON_RATGUN) )
		{
			_SetSiteWeaponState( MultiplayerMgr.IsSinglePlayer() ? SITE_PATROLLING : SITE_POWER_DOWN);
		}
	}

	else if (NewState == SITE_PATROLLING) // when entering patrol, reset our target
	{									// when entering patrol mode, renormalize the yaw parameter
		f32 yawValue  = m_RotateYAxis.GetValue();
		while(yawValue > FMATH_2PI)
			yawValue -= (FMATH_2PI);

		while(yawValue < -FMATH_2PI)
			yawValue += (FMATH_2PI);

		m_RotateYAxis.SetValue(yawValue);
		m_nPowerState = POWERSTATE_POWERED_UP;
		m_RotateYAxis.SetMaxVelocity(m_pData->m_fMaxYawVelocityPatrol);
		m_RotateXAxis.SetMaxVelocity(m_pData->m_fMaxYawVelocityPatrol);
		m_RotateYAxis.SetTarget(m_SiteWeaponAI.fPatrolMinYaw);
		m_RotateXAxis.SetTarget(m_SiteWeaponAI.fPatrolPitch);
	}

	else if (NewState == SITE_TARGETING) // targeting beeps;
	{
		f32 fTimeNow = FLoop_nTotalLoopTicks*FLoop_fSecsPerTick;
		if (fTimeNow >= (m_fLastBeepTime+2.0f))
		{
			PlaySound(m_pSounds[SITESND_LOCK]);
			m_fLastBeepTime = fTimeNow;
		}
	}

	else if (NewState == SITE_SEARCHING) 
	{
		m_bSearchPointReached = FALSE;
	}

	else if (NewState == SITE_DEFENDING) // defend needs target, follow the damage back to the source 
	{
		m_TargetedPoint_WS.Add(m_vImpactPoint_WS,m_vUnitDirToDamageSource.Mul(100.f));
	}
	
	else if (NewState == SITE_POSSESSED) // to do, actually place glitch on the sentry
	{
		if (m_pGun->IsOwnedByPlayer() == FALSE)
		{
			m_RotateYAxis.SetMaxVelocity(m_pData->m_fMaxYawVelocity);
			m_RotateXAxis.SetMaxVelocity(m_pData->m_fMaxYawVelocity);
		}
		
		m_nPossessionPlayerIndex = m_pData->m_pDriverBot->m_nPossessionPlayerIndex;	 //need to do this so that CBot::ParseControls works. even though, we're not using the "possess" interface
		
		CBotSiteWeaponBuilder *pBuilder = (CBotSiteWeaponBuilder *)GetLeafClassBuilder();
		if (m_pBotDef->m_nSubClass==BOTSUBCLASS_SITEWEAPON_FLOORSENTRY)
		{
			// Single player: always recruit
			if (MultiplayerMgr.IsSinglePlayer())
			{
				if( m_nRecruitID >= 0 ) 
				{
					CWeaponRecruiter::UnrecruitBot(this, FALSE);
				}

				CWeaponRecruiter::RecruitBot(this,m_pData->m_pDriverBot,&this->m_MtxToWorld.m_vPos);
				// m_pBotDef = ((m_pData->m_pDriverBot->m_pBotDef->m_nRace == BOTRACE_DROID) ? &pBuilder->m_DroidFloorSentryBotDef : &pBuilder->m_MilFloorSentryBotDef);
			}
			else
			{
				// Multiplayer: if the bot was recruited by someone else, unrecruit it.
				//              otherwise, just leave the recruit state alone
				if ( Recruit_IsRecruited() && (Recruit_GetRecruiter() != m_pData->m_pDriverBot->m_nPossessionPlayerIndex) )
					Unrecruit();
			}
		}
		else if ( ( m_pBotDef->m_nSubClass  == BOTSUBCLASS_SITEWEAPON_PILLBOX) ||
				  (m_pBotDef->m_nSubClass != BOTSUBCLASS_SITEWEAPON_RATGUN) )
		{
#ifdef ATTACK_PILLBOX_WHEN_OCCUPIED
			m_pBotDef = ((m_pData->m_pDriverBot->m_pBotDef->m_nRace == BOTRACE_DROID) ? &pBuilder->m_DroidPillBoxBotDef : &pBuilder->m_AmbientPillBoxBotDef);
#endif
		}
		m_pData->m_pEnemyTarget = NULL;
		m_pData->m_pEnemyTargetDistance2 = FMATH_MAX_FLOAT;
		PlaySound(m_pSounds[SITESND_POSSESS]);
		_RotateBaseNoise(FALSE); // need to switch to 2d sounds

		// remember if driver's light was on, then turn it off while in vehicle
		if( m_pData->m_pDriverBot->IsSpotLightOn() )
		{
			m_uVehicleFlags |= VEHICLE_FLAG_DRIVER_LIGHT_ON;
			m_pData->m_pDriverBot->SetSpotLightOff(TRUE);
		}
		else
		{
			m_uVehicleFlags &= ~VEHICLE_FLAG_DRIVER_LIGHT_ON;
		}

		// remember if driver was casting a shadow, then turn it off while in vehicle
		if( m_pData->m_pDriverBot->m_pWorldMesh->m_nFlags & FMESHINST_FLAG_CAST_SHADOWS )
		{
			m_uVehicleFlags |= VEHICLE_FLAG_DRIVER_HAS_SHADOW;
			m_pData->m_pDriverBot->m_pWorldMesh->m_nFlags &= ~FMESHINST_FLAG_CAST_SHADOWS;
		}
		else
		{
			m_uVehicleFlags &= ~VEHICLE_FLAG_DRIVER_HAS_SHADOW;
		}
	}

	else if (NewState == SITE_ENTERING_POSSESSED) 
	{
		if ( (m_pBotDef->m_nSubClass != BOTSUBCLASS_SITEWEAPON_PILLBOX) &&
			 (m_pBotDef->m_nSubClass != BOTSUBCLASS_SITEWEAPON_RATGUN) )
		{
			_SetSiteWeaponState(SITE_POWERING_UP_POSSESSED);
		}
		if (!m_pData->m_pDriverBot->IsCapableOfAimingPB()) // no anim to even play... 
			_SetSiteWeaponState(SITE_POWERING_UP_POSSESSED);
//		if( m_pGun->IsOwnedByPlayer() == TRUE ) // don't play anim in possessed cases
//			_SetSiteWeaponState(SITE_POWERING_UP_POSSESSED);
	}

	else if (NewState == SITE_POWERING_UP_POSSESSED) 
	{
		m_nPowerState = POWERSTATE_POWERING_UP;
		if ( (m_pBotDef->m_nSubClass != BOTSUBCLASS_SITEWEAPON_PILLBOX) &&
			 (m_pBotDef->m_nSubClass != BOTSUBCLASS_SITEWEAPON_RATGUN) )
		{
			PlaySound(m_pSounds[SITESND_POWERUP]);
		}
		// need to do this because some site wepons can be "driven" by players without going through the cbot::Possess() interface
		m_nPossessionPlayerIndex = m_pData->m_pDriverBot->m_nPossessionPlayerIndex;
	}

	else if (NewState == SITE_POWER_UP) 
	{
		m_nPowerState = POWERSTATE_POWERING_UP;
		PlaySound(m_pSounds[SITESND_POWERUP]);
	}

	else if (NewState == SITE_POWER_DOWN) 
	{
		m_nPowerState = POWERSTATE_POWERED_DOWN;
		m_RotateYAxis.SetVelocity(0.0f);

		_RotateBaseNoise(FALSE); 
		
		PlaySound(m_pSounds[SITESND_POWERDOWN]);
	}

	else if (NewState == SITE_DEAD) 
	{
		if (m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_FLOORSENTRY)
		{
			if( m_nRecruitID >= 0 ) 
			{
				CWeaponRecruiter::UnrecruitBot(this, FALSE);
			}
			//m_pBotDef = &(((CBotSiteWeaponBuilder*)GetLeafClassBuilder())->m_MilFloorSentryBotDef);
		}
		SetTargetable(FALSE);

		m_RotateYAxis.SetVelocity(0.0f);

		_RotateBaseNoise(FALSE); 

		PlaySound(m_pSounds[SITESND_DEATH]);

		if( !m_pDeathInfo )
		{
			return;
		}
		m_nExplosionsLeftInDeath = m_pDeathInfo->nExplosions - 1; // 'cause the biggie we did already
		m_fTimeToNextExplosion=fmath_RandomFloatRange(m_pDeathInfo->fMinTimeBetweenExpl,m_pDeathInfo->fMaxTimeBetweenExpl);
	}
}

void CBotSiteWeapon::_CallSiteStateFunction(void)
{
	m_pData->m_fTimeSinceStateChanged += FLoop_fPreviousLoopSecs; // increment our time in this state

	// CFColorRGBA col;
	// col.Red();
	// col.fAlpha = 0.5f;
	// fdraw_DevSphere(&m_TargetedPoint_WS,4.0f,&col,6,6,6);

	switch (m_pData->m_eSiteWeaponState)
	{
	case SITE_PATROLLING:
//		if (m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_FLOORSENTRY)
//		ftext_DebugPrintf(0.5f,0.4f,"PATROLLING");
		_ai_patrolling();
		m_pGun->Work();
		return;
	case SITE_TURNING:
//		if (m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_FLOORSENTRY)
//		ftext_DebugPrintf(0.5f,0.4f,"TURNING");
		_ai_turning();
		m_pGun->Work();
		return;
	case SITE_TARGETING:
//		if (m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_FLOORSENTRY)
//		ftext_DebugPrintf(0.5f,0.4f,"TARGETING");
		_ai_targeting();
		m_pGun->Work();
		return;
	case SITE_ATTACKING:
//		if (m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_FLOORSENTRY)
//		ftext_DebugPrintf(0.5f,0.4f,"ATTACKING");
		_ai_attacking();
		m_pGun->Work();
		return;
	case SITE_STRAFING:
//		if (m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_FLOORSENTRY)
//		ftext_DebugPrintf(0.5f,0.4f,"STRAFING");
		_ai_strafing();
		m_pGun->Work();
		return;
	case SITE_SEARCHING:
//		if (m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_FLOORSENTRY)
//		ftext_DebugPrintf(0.5f,0.4f,"SEARCHING");
		_ai_searching();
		m_pGun->Work();
		return;
	case SITE_DEFENDING:
//		if (m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_FLOORSENTRY)
//		ftext_DebugPrintf(0.5f,0.4f,"DEFENDING");
		_ai_defending();
		m_pGun->Work();
		return;
	
	case SITE_CONFUSION:
		//ftext_DebugPrintf(0.5f,0.4f,"CONFUSED");
		_ai_confusion();
		m_pGun->Work();
		return;
	
	case SITE_IDLING:
		//ftext_DebugPrintf(0.5f,0.4f,"IDLING");
		_Do_Idling();
		m_pGun->Work();
		return;
	case SITE_ENTERING_POSSESSED:
		m_pGun->Work(); // working the gun first to make the power up cam look right
		_Do_Entering_Possessed();
		return;
	case SITE_POWERING_UP_POSSESSED:
//		if (m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_FLOORSENTRY)
//		ftext_DebugPrintf(0.5f,0.4f,"POSSESSED_POWER_UP");
		m_pGun->Work(); // working the gun first to make the power up cam look right
		_Do_Powering_Up_Possessed();
		return;
	case SITE_POSSESSED:
//		if (m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_FLOORSENTRY)
//		ftext_DebugPrintf(0.5f,0.4f,"POSSESSED");
		_Do_Possessed();
		return;
	case SITE_POWER_UP:
//		if (m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_FLOORSENTRY)
//		ftext_DebugPrintf(0.5f,0.4f,"POWER UP");
		_Do_Power_Up();
		m_pGun->Work();
		return;
	case SITE_POWER_DOWN:
//		if (m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_FLOORSENTRY)
//		ftext_DebugPrintf(0.5f,0.4f,"POWERED DOWN");
		_Do_Power_Down();
		m_pGun->Work();
		return;
	case SITE_DEAD:
//		if (m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_FLOORSENTRY)
//		ftext_DebugPrintf(0.5f,0.4f,"DEAD");
		_Do_Dead();
		m_pGun->Work();
		return;
	default:
		FASSERT(!"Bad state in FSM");
	}
}
f32 CBotSiteWeapon::_aihelper_aim_at_m_TargetedPoint()
{
	// Aim the yaw axis via helper function here
	f32 fTargetAngleOffsetPercent = _TurnToTargetAndFocusRadar2d(m_TargetedPoint_WS,&m_pData->m_fDistanceToTarget);

	// Aim the pitch axis here
	f32 heightDifference = m_pData->m_vAimingOrigin_WS.y - m_TargetedPoint_WS.y;
	f32 fPitchTarget = fmath_Atan(heightDifference,m_pData->m_fDistanceToTarget);

	if (fPitchTarget < m_pData->m_fMinimumPitchWS)
		fPitchTarget= m_pData->m_fMinimumPitchWS;
	if (fPitchTarget > m_pData->m_fMaximumPitchWS)
		fPitchTarget = m_pData->m_fMaximumPitchWS;
	m_RotateXAxis.SetTarget(fPitchTarget);

#if 0//(SAS_ACTIVE_USER == SAS_USER_SCHOLZ)
#if !FANG_PRODUCTION_BUILD
	CFVec3A	vStart,vEnd;
	vStart = m_pData->m_vAimingOrigin_WS;
	vEnd.ReceiveRotationX(CFVec3A::m_UnitAxisZ,m_RotateXAxis.GetTarget());
	vEnd.RotateY(m_RotateYAxis.GetTarget());
	
	vEnd.Mul(m_pData->m_fDistanceToTarget + 10.0f);
	f32 fDist1 = vEnd.Mag();
	vEnd.Add(vStart);
	CDebugDraw::Line(vStart,vEnd);
	
	vStart = m_pData->m_vAimingOrigin_WS;
	CFVec3A vUnitMuzzleDir;
	vUnitMuzzleDir.ReceiveUnit(m_pData->m_pMtxAimOrientation_WS->m_vFront);
	vEnd.Mul(vUnitMuzzleDir,m_pData->m_fDistanceToTarget + 10.0f);
	vEnd.Add(vStart);
	CDebugDraw::Line(vStart,vEnd,&FColor_MotifRed,&FColor_MotifRed);
	
	CDebugDraw::Line(m_pData->m_vAimingOrigin_WS, m_TargetedPoint_WS, &FColor_MotifBlack,&FColor_MotifBlack);
#endif
#endif

	// return the yaw angle offset
	return fTargetAngleOffsetPercent;
}

BOOL CBotSiteWeapon::_aihelper_can_see_you(CBot* pYOU)
{
	if (!pYOU)
	{
		return FALSE;
	}
	
	BOOL bCanSeeTarget = FALSE;
	
	FWorld_nTrackerSkipListCount = 0;
	pYOU->AppendTrackerSkipList();	//Some trackers to ignore in the LOS test.
	this->AppendTrackerSkipList();	//Some trackers to ignore in the LOS test.
	
	CFVec3A EyeLoc;
	EyeLoc.x = m_MtxToWorld.m_vPos.x;
	EyeLoc.y = m_MtxToWorld.m_vPos.y + m_fFixedEyeOffset;
	EyeLoc.z = m_MtxToWorld.m_vPos.z;

	// check next tagpoints for LOS to enemy
	s32 nTagPointCount = pYOU->GetTagPointCount();
	s32 nTagPointCheck = FMATH_MIN(m_nLastTagPointInspected-1,nTagPointCount-1);
	if (nTagPointCheck < 0)
	{
		nTagPointCheck = nTagPointCount-1;
	}
	CFVec3A ProposeTargetPoint = *(pYOU->GetTagPoint((u32)nTagPointCheck));

//	CDebugDraw::Line(EyeLoc,ProposeTargetPoint);

	if (!fworld_IsLineOfSightObstructed( &EyeLoc, &ProposeTargetPoint, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList))
	{
		bCanSeeTarget=TRUE;
		// although it works to LOS all the tagpoints, lets always aim between tag 0 and the ground
		m_TargetedPoint_WS.Add(*pYOU->GetTagPoint(0),pYOU->MtxToWorld()->m_vPos).Mul(0.5);
	}
	else // only search tagpoints when target is not LOS
	{
		m_nLastTagPointInspected = nTagPointCheck;
	}
	return bCanSeeTarget;
}

void CBotSiteWeapon::_ai_patrolling(void)
{
	// update my radar when on patrol 
	f32 rotAmount = FLoop_fPreviousLoopSecs * m_SiteWeaponAI.fRadarVelocity;
	m_vRadarDirectionWS.RotateY(rotAmount);

	// search out noises as low priority
	AIEnviro_DetectSoundCollisions(MtxToWorld()->m_vPos,this,1.0f,_SoundCollisionCB,this);

	_aihelper_SearchForTarget(); // look for enemies

	

	if (m_pData->m_pEnemyTarget != NULL) // enemy sighted! turn to target to attack!
	{
		_SetSiteWeaponState(SITE_TURNING);
	}
	else if (m_bDamageTaken)
	{
		m_bDamageTaken = FALSE;
		_SetSiteWeaponState(SITE_DEFENDING);
	}
	else // all is quiet, continue patrol 
	{
		if (m_pData->m_fYawWS <= m_SiteWeaponAI.fPatrolMinYaw+CSTEP_EPSILON)
		{
			m_RotateYAxis.SetTarget(m_SiteWeaponAI.fPatrolMaxYaw);
		}
		else if (m_pData->m_fYawWS >= m_SiteWeaponAI.fPatrolMaxYaw-CSTEP_EPSILON)
		{
			m_RotateYAxis.SetTarget(m_SiteWeaponAI.fPatrolMinYaw);
		}
	}
	
	_MoveXYAxes(); // update my yaw/pitch variables
}

void CBotSiteWeapon::_ai_turning(void)
{
	if (m_bDamageTaken)
	{
		m_bDamageTaken = FALSE;
		_SetSiteWeaponState(SITE_DEFENDING);
	}
	
	f32 fYawAngleOffset = _aihelper_aim_at_m_TargetedPoint(); // shared code for aiming

	_aihelper_SearchForTarget();
	if (m_pData->m_pEnemyTarget != NULL)
	{
		if (fYawAngleOffset < m_SiteWeaponAI.fTargetingYawSin)
		{
			// switch to target mode;
			_SetSiteWeaponState(SITE_TARGETING);
		}
	}
	else
	{
		if (fYawAngleOffset < REACHEDGOAL_YAWSIN)
		{
			// switch to search mode;
			_SetSiteWeaponState(SITE_SEARCHING);
		}
	}
	_MoveXYAxes();// update my yaw/pitch variables
}

void CBotSiteWeapon::_ai_targeting(void)
{
	_aihelper_SearchForTarget();
	
	f32 fYawAngleOffset = _aihelper_aim_at_m_TargetedPoint(); // shared code for aiming
	
	if (m_pData->m_pEnemyTarget != NULL) // I can see you
	{
		if (fYawAngleOffset < m_SiteWeaponAI.fSpewFireYawSin)
		{
			// switch to target mode;
			_SetSiteWeaponState(SITE_ATTACKING);
		}
		if (fYawAngleOffset > m_SiteWeaponAI.fTargetingYawSin)
		{
			_SetSiteWeaponState(SITE_TURNING);
		}
	}
	else
	{
		_SetSiteWeaponState(SITE_TURNING);
	}
	_MoveXYAxes();// update my yaw/pitch variables
}

void CBotSiteWeapon::_ai_attacking(void)
{
	_aihelper_SearchForTarget();
	f32 fYawAngleOffset = _aihelper_aim_at_m_TargetedPoint(); // shared code for aiming
	
	if (m_pData->m_pEnemyTarget != NULL)
	{
		if (fYawAngleOffset < m_SiteWeaponAI.fSpewFireYawSin)
		{
			if (m_pData->m_fTimeSinceStateChanged >= m_SiteWeaponAI.fAttackDelayTime)
			{
				m_fControls_Fire1=1.0f;
				if (m_bAimRockets && (fYawAngleOffset < m_SiteWeaponAI.fRocketFireYawSin))
				{
					m_fControls_Fire2=1.0f;
				}
			}
		}
		else
		{
			// switch to target mode;
			_SetSiteWeaponState(SITE_TARGETING);
		}
	}
	else
	{
		// switch to strafe mode;
		_SetSiteWeaponState(SITE_STRAFING);
	}
	_MoveXYAxes();// update my yaw/pitch variables
}

void CBotSiteWeapon::_ai_strafing(void)
{
	if (m_bDamageTaken)
	{
		m_bDamageTaken = FALSE;
		_SetSiteWeaponState(SITE_DEFENDING);
	}

	if (m_pData->m_fTimeSinceStateChanged > m_fStrafingTime)
	{
		_SetSiteWeaponState(SITE_SEARCHING);
	}
	else
	{
		m_fControls_Fire1=1.0f;
	}
}

void CBotSiteWeapon::_ai_searching(void)
{
	_aihelper_SearchForTarget(); // look for enemies
	f32 fYawAngleOffset = _aihelper_aim_at_m_TargetedPoint(); // shared code for aiming
	if (m_pData->m_pEnemyTarget)
	{
		_SetSiteWeaponState(SITE_TURNING); 
	}
	
	if (m_bSearchPointReached==FALSE)
	{
		m_pData->m_fTimeSinceStateChanged = 0.0f; // zero'ing the state change counter, don't want to go on the clock
												  // until after we've lined up the target;
		if (fYawAngleOffset < REACHEDGOAL_YAWSIN)
		{
			m_bSearchPointReached = TRUE;
			m_fSearchingInitialYaw = m_pData->m_fYawWS +.15f;
			m_fSearchingInitialPitch = m_pData->m_fPitchWS;
			m_fSearchTheta = 0.0f;
			_aihelper_SearchPoint(3.0f,0.20f,0.08f);
		}
		else
		{
			_MoveXYAxes(); // update my yaw/pitch variables
		}
	}
	else
	{
		_aihelper_SearchPoint(3.0f,0.20f,0.08f);
	}

	if (m_pData->m_fTimeSinceStateChanged >= m_fSearchingTime)
	{
		_SetSiteWeaponState(SITE_PATROLLING);
	}
}

void CBotSiteWeapon::_aihelper_SearchPoint(f32 fAngularVelocity,f32 fYawRadius,f32 fPitchRadius)
{
	m_fSearchTheta += FLoop_fPreviousLoopSecs*fAngularVelocity;
	if (m_fSearchTheta > FMATH_2PI)
		m_fSearchTheta -=FMATH_2PI;

	f32 fUnitYawOffset;
	f32 fUnitPitchOffset;
	fmath_SinCos(m_fSearchTheta,&fUnitPitchOffset,&fUnitYawOffset);

		m_pData->m_fYawWS	= m_fSearchingInitialYaw   - fYawRadius   * fUnitYawOffset;
		m_pData->m_fPitchWS = m_fSearchingInitialPitch - fPitchRadius * fUnitPitchOffset;
		m_RotateYAxis.SetValue(m_pData->m_fYawWS);
		m_RotateXAxis.SetValue(m_pData->m_fPitchWS);

	FMATH_CLAMP(m_pData->m_fYawWS, m_pData->m_fMinimumYawWS, m_pData->m_fMaximumYawWS);
	FMATH_CLAMP(m_pData->m_fPitchWS, m_pData->m_fMinimumPitchWS, m_pData->m_fMaximumPitchWS);

	m_pData->m_MtxAimY_MS.SetRotationY( (m_pData->m_fYawWS-m_pData->m_fYawAdjustWS2MS) );
	m_pData->m_MtxAimX_MS.SetRotationX( (m_pData->m_fPitchWS-m_pData->m_fPitchAdjustWS2MS));
}


void CBotSiteWeapon::_ai_defending(void)
{
	_aihelper_SearchForTarget(); // look for enemies
	if (m_pData->m_pEnemyTarget)
	{
		_SetSiteWeaponState(SITE_TURNING); 
		return;
	}

	// turn to damage source direction
	f32 fYawAngleOffset = _aihelper_aim_at_m_TargetedPoint(); // shared code for aiming

	if (fYawAngleOffset < REACHEDGOAL_YAWSIN)
	{
		if (m_pData->m_fTimeSinceStateChanged <= m_fDefendingTime)
		{
			if (fmath_RandomChance(.15f))
			{
				m_fControls_Fire1=1.0f;
			}

			if (m_bAimRockets && fmath_RandomChance(.33f))
			{
				m_fControls_Fire2=1.0f;	
			}
		}
		else
		{
			_SetSiteWeaponState(SITE_SEARCHING);
		}
	}
	

	_MoveXYAxes();// update my yaw/pitch variables
}

void CBotSiteWeapon::_ai_confusion(void)
{}

void CBotSiteWeapon::_Do_Possessed(void)
{
	FASSERT(m_pData->m_pDriverBot);
	if (CVehicle::IsDriverUseless(m_pData->m_pDriverBot)) // then pop him;
	{
		SetSiteWeaponDriver(NULL,FALSE);
		return;
	}

	CEntityControl*  pControls = Controls();
	if (pControls->Type() == CEntityControl::TYPE_HUMAN) // human controls 
	{
		ComputeHumanTargetPoint_WS(&m_TargetedPoint_WS, NULL);

		// simplified aiming model, since the accelerated was so problematic. 
		// instantaneous velocity change, 1.0 is full man, -1.0 is -full max
		m_fControls_RotateCW = m_fControls_RotateCW;
		FMATH_BIPOLAR_CLAMPMAX(m_fControls_RotateCW,1.0f);

		m_fControls_AimDown =m_fControls_AimDown;
		FMATH_BIPOLAR_CLAMPMAX(m_fControls_AimDown,1.0f);

		f32 fYawVelocity = m_pData->m_fMaxYawVelocityPossess * m_fControls_RotateCW;
		f32 fYawDelta = fYawVelocity * FLoop_fPreviousLoopSecs +Player_aPlayer[ m_nPossessionPlayerIndex ].m_fYawAdjust;
		f32 fPitchVelocity = m_pData->m_fMaxPitchVelocityPossess * m_fControls_AimDown;
#if defined(MA_PC_INPUT)
		if( m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_RATGUN ) {
			// ParseControls already applies the player's inversion preference.
			const f32 fSensitivity = Player_aPlayer[m_nPossessionPlayerIndex].ComputeLookSensitivityMultiplier();
			fYawVelocity *= fSensitivity;
			fPitchVelocity = FMATH_MIN(m_pData->m_fMaxPitchVelocityPossess,
				m_pData->m_fMaxYawVelocityPossess) * m_fControls_AimDown * fSensitivity;
			fYawDelta = fYawVelocity * FLoop_fPreviousLoopSecs + Player_aPlayer[m_nPossessionPlayerIndex].m_fYawAdjust;
		}
#endif
		f32 fPitchDelta = fPitchVelocity * FLoop_fPreviousLoopSecs + Player_aPlayer[ m_nPossessionPlayerIndex ].m_fPitchAdjust;
#if defined(MA_PC_INPUT)
		// Mouse look turns manned guns directly, as it turns the player's bot.
		fYawDelta += TakeMouseLookDelta( FALSE );
		fPitchDelta += TakeMouseLookDelta( TRUE );
#endif
		m_pData->m_fYawWS += fYawDelta;
		while(m_pData->m_fYawWS-m_pData->m_fYawAdjustWS2MS > FMATH_PI)
			m_pData->m_fYawWS -= (FMATH_2PI);

		while(m_pData->m_fYawWS-m_pData->m_fYawAdjustWS2MS < -FMATH_PI)
			m_pData->m_fYawWS += (FMATH_2PI);

		m_pData->m_fPitchWS += fPitchDelta;
		FMATH_CLAMP(m_pData->m_fYawWS, m_pData->m_fMinimumYawWS, m_pData->m_fMaximumYawWS);

		if ( m_pBotDef->m_nSubClass  != BOTSUBCLASS_SITEWEAPON_RATGUN)		
		{
			FMATH_CLAMP(m_pData->m_fPitchWS, m_pData->m_fMinimumPitchWS, m_pData->m_fMaximumPitchWS);
		}
		else
		{
			f32 fAbsYaw = fmath_Abs(m_pData->m_fYawWS - m_pData->m_fYawAdjustWS2MS);
			if (fAbsYaw < m_BotInfo_RatGunXtra.fMinCapBlendAngle)
			{
				FMATH_CLAMP(m_pData->m_fPitchWS, m_pData->m_fMinimumPitchWS, m_BotInfo_RatGunXtra.fMaxPitchCapped);
			}
			else if (fAbsYaw > m_BotInfo_RatGunXtra.fMaxCapBlendAngle)
			{
				FMATH_CLAMP(m_pData->m_fPitchWS, m_pData->m_fMinimumPitchWS, m_pData->m_fMaximumPitchWS);
			}
			else
			{
				f32 fBlendYawRange = m_BotInfo_RatGunXtra.fMaxCapBlendAngle-m_BotInfo_RatGunXtra.fMinCapBlendAngle;
				f32 fBlendPitchRange = m_pData->m_fMaximumPitchWS - m_BotInfo_RatGunXtra.fMaxPitchCapped;
				f32 fUnitCap = fmath_Div(fAbsYaw-m_BotInfo_RatGunXtra.fMinCapBlendAngle,fBlendYawRange);
				FMATH_CLAMP(m_pData->m_fPitchWS, m_pData->m_fMinimumPitchWS, m_BotInfo_RatGunXtra.fMaxPitchCapped +fUnitCap * fBlendPitchRange);
			}
		}

		m_pData->m_MtxAimY_MS.SetRotationY(m_pData->m_fYawWS - m_pData->m_fYawAdjustWS2MS);
		m_pData->m_MtxAimX_MS.SetRotationX(m_pData->m_fPitchWS);

		if ( (fmath_Abs(fYawVelocity) > 0.02) || (fmath_Abs(fPitchVelocity) > 0.02) )
		{
			_RotateBaseNoise(TRUE,fmath_Abs(m_fControls_RotateCW));
		}
		else
		{
			_RotateBaseNoise(TRUE,fmath_Abs(m_fControls_RotateCW));
		}
	}
	else 
	{
		// Bot controls...
		CBotControl *pBotControl = (CBotControl*)pControls;
		if ( pBotControl->m_nFlags & CBotControl::FLAG_AIM_AT_TARGET_POINT )
		{
			m_TargetedPoint_WS = pBotControl->m_TargetPoint_WS;
			_aihelper_aim_at_m_TargetedPoint();
			_MoveXYAxes();// update my yaw/pitch variables
			FMATH_CLAMP(m_pData->m_fYawWS, m_pData->m_fMinimumYawWS, m_pData->m_fMaximumYawWS);
			FMATH_CLAMP(m_pData->m_fPitchWS, m_pData->m_fMinimumPitchWS, m_pData->m_fMaximumPitchWS);
		}
		else
		{
			m_pData->m_fYawWS   += 5.0f * m_fControls_RotateCW * FLoop_fPreviousLoopSecs;		
			m_pData->m_fPitchWS += 5.0f * m_fControls_AimDown *  FLoop_fPreviousLoopSecs;		
			FMATH_CLAMP(m_pData->m_fYawWS, m_pData->m_fMinimumYawWS, m_pData->m_fMaximumYawWS);
			FMATH_CLAMP(m_pData->m_fPitchWS, m_pData->m_fMinimumPitchWS, m_pData->m_fMaximumPitchWS);
			
			m_pData->m_MtxAimY_MS.SetRotationY(m_pData->m_fYawWS - m_pData->m_fYawAdjustWS2MS);
			m_pData->m_MtxAimX_MS.SetRotationX(m_pData->m_fPitchWS);
	
			m_RotateYAxis.SetValue(m_pData->m_fYawWS);
			m_RotateYAxis.SetVelocity(0.0f);
			m_RotateXAxis.SetValue(m_pData->m_fPitchWS);
			m_RotateXAxis.SetVelocity(0.0f);

			f32 fAbsRotate = fmath_Abs(m_fControls_RotateCW);
			f32 fAbsPitch = fmath_Abs(m_fControls_AimDown);
			if ( (fAbsRotate > 0.10f) || (fAbsRotate > 0.10f) ) 
				_RotateBaseNoise(TRUE,fAbsRotate);
			else
				_RotateBaseNoise(TRUE,fAbsRotate);

		}
	}
	FASSERT (m_pData->m_fPitchWS >= m_pData->m_fMinimumPitchWS);
	FASSERT (m_pData->m_fPitchWS <= m_pData->m_fMaximumPitchWS);
	FASSERT (  m_pData->m_fYawWS >= m_pData->m_fMinimumYawWS);
	FASSERT (  m_pData->m_fYawWS <= m_pData->m_fMaximumYawWS);
	
	m_pGun->Work();
}

void CBotSiteWeapon::_Do_Entering_Possessed(void)
{	
	FASSERT(m_pData->m_pDriverBot);
	if (m_pData->m_fTimeSinceStateChanged >= m_pData->m_fTimeToEnter)
		_SetSiteWeaponState(SITE_POWERING_UP_POSSESSED);
}

void CBotSiteWeapon::_Do_Powering_Up_Possessed(void)
{
	if (m_pData->m_fPitchWS == 0.0f || m_pBotDef->m_nSubClass  == BOTSUBCLASS_SITEWEAPON_RATGUN)
	{
		_SetSiteWeaponState(SITE_POSSESSED);
	}
}

void CBotSiteWeapon::_Do_Power_Up(void)
{
	if (m_pData->m_fPitchWS == 0.0f || m_pBotDef->m_nSubClass  == BOTSUBCLASS_SITEWEAPON_RATGUN)
	{
		_SetSiteWeaponState(SITE_PATROLLING);
	}
}

void CBotSiteWeapon::_Do_Power_Down(void)
{
	// If we were turned off on a timer...
	if( m_fPowerOffOnTime > 0.0f ) 
	{
		m_fPowerOffOnTime -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fPowerOffOnTime, 0.0f );

		// Time is done, turn the bot back on
		if( m_fPowerOffOnTime == 0.0f && !IsDead() ) 
		{
			Power( TRUE, 0 );
		}
	}
}

BOOL CBotSiteWeapon::_SoundCollisionCB(CAISound* pSound, void* pData)
{
	CBotSiteWeapon* pThis = (CBotSiteWeapon*)pData;
	
	if ( (pSound->m_uSoundType == AISOUNDTYPE_GRENADE) )				// a grenade bouncing sound
	{
		pThis->m_TargetedPoint_WS = pSound->m_Origin;
		pThis->_SetSiteWeaponState(CBotSiteWeapon::SITE_SEARCHING);
		return TRUE;
	}
	return FALSE;
}

void CBotSiteWeapon::_Do_Dead(void)
{
	if (m_nExplosionsLeftInDeath > 0)
	{
		m_fTimeToNextExplosion -= FLoop_fPreviousLoopSecs;
		if (m_fTimeToNextExplosion < 0.0f)
		{
			CFVec3A vPoint,vNormal;
			BOOL bPtFound = GetRandomPointOnMeshSurface(vPoint,vNormal,m_pGun->m_pDeadMesh);
			if (!bPtFound)
			{
				return;
			}

			FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();
			if( hSpawner == FEXPLOSION_INVALID_HANDLE ) 
			{
				return;
			}
			
			CEParticle *pEParticle = eparticlepool_GetEmitter();
			if (!pEParticle)
				return;
			
			FExplosionSpawnParams_t SpawnParams;
			SpawnParams.InitToDefaults();
			SpawnParams.Pos_WS.Set(vPoint);
			SpawnParams.UnitDir.Set(vNormal);
			FASSERT(m_pDeathInfo);
			
			CExplosion2::SpawnExplosion( hSpawner, m_pDeathInfo->hExplosion, &SpawnParams );

			CFMtx43A ParticleMtx;
			CFVec3A ParticleUnitDir_WS;

			ParticleMtx.UnitMtxFromUnitVec( &vNormal);
			ParticleMtx.m_vPos = vPoint;
			pEParticle->StartEmission( m_pDeathInfo->hSmoke, 1.0f, m_pDeathInfo->fSecSmoke, TRUE );
			pEParticle->Attach_UnitMtxToParent_PS(this);
			pEParticle->Relocate_RotXlatFromUnitMtx_WS(&ParticleMtx, FALSE );

			m_fTimeToNextExplosion=fmath_RandomFloatRange(m_pDeathInfo->fMinTimeBetweenExpl,m_pDeathInfo->fMaxTimeBetweenExpl);
			m_nExplosionsLeftInDeath--;
		}
	}
}


void CBotSiteWeapon::_Do_Idling(void)
{

}
