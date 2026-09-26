#include "fang.h"
#include "ftext.h"
#include "fxfm.h"
#include "fvis.h"
#include "fclib.h"
#include "AIBrainMan.h"
#include "AIBrain.h"
#include "AIBrainMems.h"
#include "AIBotMover.h"
#include "AICarMover.h"
#include "AIEdgeLock.h"
#include "AI3DBotMover.h"
#include "AINodePools.h"
#include "AIThoughtsGeneric.h"
#include "AIThoughtsGround.h"
#include "AIThoughts3DBot.h"
#include "AIBuilder.h"
#include "AICorrosive.h"
#include "AIZombieBoss.h"
#include "AiApi.h"
#include "AIGameUtils.h"
#include "AIPatrolPath.h"
#include "AIThought.h"
#include "AIGroup.h"
#include "../Damage.h"
#include "../Entity.h"
#include "../EntityControl.h"
#include "../Player.h"
#include "../Protrack.h"
#include "../ESpline.h"
#include "../Vehicle.h"

//
//	Table of all Thought Classes that have beeen implemented
//	  Brains can hook them up in any reasonable fashion in order to implement
//    the CBrain Thought Types.
//
struct _ThoughtClassSpecification
{
	s32 nHowMany;						   //How many thought class objects should be pre-allocated and placed in the pool at level-load
	ThoughtBank_InitFunc* pInitFunc;
	ThoughtBank_CleanupFunc* pCleanupFunc;
	ThoughtBank_AccessFunc* pAccessFunc;
} 
_ThoughtClassSpec [NUM_THOUGHTCLASSES] = 
{
	50,	CGenericWait::InitBank,		CGenericWait::CleanupBank,		CGenericWait::BankAccess,		//CLASS_CGENERICWAIT
	50,	CGenericWander::InitBank,	CGenericWander::CleanupBank,	CGenericWander::BankAccess,		//CLASS_CGENERICWANDER
	50,	CGenericPatrol::InitBank,	CGenericPatrol::CleanupBank,	CGenericPatrol::BankAccess,		//CLASS_CGENERICPATROL
	35,	CGroundCombat::InitBank,	CGroundCombat::CleanupBank,		CGroundCombat::BankAccess,		//CLASS_CGROUNDCOMBAT
	75,	CGenericGoto::InitBank,		CGenericGoto::CleanupBank,		CGenericGoto::BankAccess,		//CLASS_CGENERICGOTO
	35,	CGenericFaceIt::InitBank,	CGenericFaceIt::CleanupBank,	CGenericFaceIt::BankAccess,		//CLASS_CGENERICFACEIT
	35,	CGenericTalkTo::InitBank,	CGenericTalkTo::CleanupBank,	CGenericTalkTo::BankAccess,		//CLASS_CGENERICTALKTO
	50,	CGenericFollow::InitBank,	CGenericFollow::CleanupBank,	CGenericFollow::BankAccess,		//CLASS_CGENERICFOLLOW
	10,	CGroundSearch::InitBank,	CGroundSearch::CleanupBank,		CGroundSearch::BankAccess,		//CLASS_CGROUNDSEARCH
	10,	CPillWait::InitBank,		CPillWait::CleanupBank,			CPillWait::BankAccess,			//CLASS_CPILLWAIT
	1,	CCorrosiveCombat::InitBank,	CCorrosiveCombat::CleanupBank,	CCorrosiveCombat::BankAccess,	//CLASS_CCORROSIVECOMBAT
	1,	CZombieBossCombat::InitBank,CZombieBossCombat::CleanupBank,	CZombieBossCombat::BankAccess,	//CLASS_CZOMBIEBOSSCOMBAT
	5,  CScoutAlert::InitBank,		CScoutAlert::CleanupBank,		CScoutAlert::BankAccess,		//CLASS_CSCOUTALERT
};

//
//	This is a table that defines some Default brain types
//	 The idea is that someday, custom brain types could be defined outside of code through the scripting language or .csv data parsing system
//
struct _BrainSpec
{
	s32 anStateSpec[CAIBrain::NUM_THOUGHT_TYPES];
}
_BrainSpec [NUM_BRAINTYPES] = 
{
	//BRAINTYPE_GENERICBIPED
	{
		CLASS_CGENERICWAIT,						//   TT_WAIT 
		CLASS_CGENERICWANDER,					//   TT_WANDER,
		CLASS_CGENERICPATROL,					//   TT_PATROL
		CLASS_CGENERICFOLLOW,					//   TT_FOLLOW
		CLASS_CGROUNDCOMBAT,					//   TT_COMBAT,
		NO_THOUGHT_CONNECTED,					//   TT_SEARCH,
		CLASS_CGENERICGOTO,						//   TT_GOTO,
		CLASS_CGENERICFACEIT,					//   TT_FACEIT,
		CLASS_CGENERICTALKTO,					//   TT_TALK,
	},												 
	//BRAINTYPE_GENERICTREADED						 
	{												 
		CLASS_CGENERICWAIT,						//   TT_WAIT 	 
		CLASS_CGENERICWANDER,					//   TT_WANDER,	 
		CLASS_CGENERICPATROL,					//   TT_PATROL
		CLASS_CGENERICFOLLOW,					//   TT_FOLLOW
		NO_THOUGHT_CONNECTED,					//   TT_COMBAT,
		NO_THOUGHT_CONNECTED,					//   TT_SEARCH,
		CLASS_CGENERICGOTO,						//   TT_GOTO,
		CLASS_CGENERICFACEIT,					//   TT_FACEIT,
		NO_THOUGHT_CONNECTED,					//   TT_TALK,
	},
	//BRAINTYPE_GENERICHOVERCRAFT
	{
		CLASS_CGENERICWAIT,						//  TT_WAIT 
		CLASS_CGENERICWANDER,					//  TT_WANDER,
		CLASS_CGENERICPATROL,					//  TT_PATROL
		CLASS_CGENERICFOLLOW,					//  TT_FOLLOW
		CLASS_CGROUNDCOMBAT,//CLASS_CCORROSIVECOMBAT,					//   TT_COMBAT,
		NO_THOUGHT_CONNECTED,					//  TT_SEARCH
		CLASS_CGENERICGOTO,						//  TT_GOTO,
		CLASS_CGENERICFACEIT,					//  TT_FACEIT,
		CLASS_CGENERICTALKTO,					//  TT_TALK,
	},
	//BRAINTYPE_PILLBOX
	{
		CLASS_CPILLWAIT,					 //   TT_WAIT 
		NO_THOUGHT_CONNECTED,				 //   TT_WANDER,
		NO_THOUGHT_CONNECTED,				 //   TT_PATROL 
		NO_THOUGHT_CONNECTED,				 //   TT_FOLLOW 
		NO_THOUGHT_CONNECTED,				 //   TT_COMBAT,
		NO_THOUGHT_CONNECTED,				 //   TT_SEARCH,
		NO_THOUGHT_CONNECTED,				 //   TT_GOTO,
		CLASS_CGENERICFACEIT,				 //   TT_FACEIT,
		NO_THOUGHT_CONNECTED,				 //  TT_TALK,
	},
	//BRAINTYPE_SCOUT
	{
		CLASS_CGENERICWAIT,					 //   TT_WAIT 
		CLASS_CGENERICWANDER,				 //   TT_WANDER,
		CLASS_CGENERICPATROL,				 //   TT_PATROL 
		CLASS_CGENERICFOLLOW,				 //   TT_FOLLOW 
		CLASS_CSCOUTALERT,				//   TT_COMBAT,
		CLASS_CGROUNDSEARCH,				//  TT_SEARCH
		CLASS_CGENERICGOTO,					//  TT_GOTO,
		CLASS_CGENERICFACEIT,				//  TT_FACEIT,
		NO_THOUGHT_CONNECTED,				//  TT_TALK,
	},
	//BRAINTYPE_MORTAR
	{
		NO_THOUGHT_CONNECTED,				//  TT_WAIT 
		NO_THOUGHT_CONNECTED,				//  TT_WANDER,
		NO_THOUGHT_CONNECTED,				//  TT_PATROL 
		NO_THOUGHT_CONNECTED,				//  TT_FOLLOW 
		NO_THOUGHT_CONNECTED,				//  TT_COMBAT,
		NO_THOUGHT_CONNECTED,				//  TT_SEARCH
		NO_THOUGHT_CONNECTED,				//  TT_GOTO,
		NO_THOUGHT_CONNECTED,				//  TT_FACEIT,
		NO_THOUGHT_CONNECTED,				//  TT_TALK,
	},
	//BRAINTYPE_CORROSIVE
	{
		CLASS_CGENERICWAIT,						//   TT_WAIT 
		CLASS_CGENERICWANDER,					//   TT_WANDER,
		CLASS_CGENERICPATROL,					//   TT_PATROL
		NO_THOUGHT_CONNECTED,					//   TT_FOLLOW
		CLASS_CCORROSIVECOMBAT,					//   TT_COMBAT,
		NO_THOUGHT_CONNECTED,					//   TT_SEARCH,
		CLASS_CGENERICGOTO,						//   TT_GOTO,
		CLASS_CGENERICFACEIT,					//   TT_FACEIT,
		CLASS_CGENERICTALKTO,					//   TT_TALK,
	},												 
	//BRAINTYPE_ZOMBIEBOSS
	{
		CLASS_CGENERICWAIT,						//   TT_WAIT 
		CLASS_CGENERICWANDER,					//   TT_WANDER,
		CLASS_CGENERICPATROL,					//   TT_PATROL
		NO_THOUGHT_CONNECTED,					//   TT_FOLLOW
		CLASS_CZOMBIEBOSSCOMBAT,				//   TT_COMBAT,
		NO_THOUGHT_CONNECTED,					//   TT_SEARCH,
		CLASS_CGENERICGOTO,						//   TT_GOTO,
		CLASS_CGENERICFACEIT,					//   TT_FACEIT,
		CLASS_CGENERICTALKTO,					//   TT_TALK,
	}
};

//
//	aibrainman data
//
CNiList<CAIBrain*>* _pActiveBrains = NULL;
const CAIBrain* CNiIterator<const CAIBrain *>::s_ReturnError;
CNiList<CAIBrain*>* _pPendingDeactivates = NULL;

//
//   Debug data
//
u32	_uTotalBrainsAllocated = 0;
u32	_uTotalBrainsInWorld = 0;
u32	_uActiveBrains = 0;
u32 _uVisibleBrains = 0;
static 	f32 _kfDebugRenderHeightSpread = 0.0f;
CAIBrain* _pClosestBrain = NULL;
extern f32 fDebugDisplayF32;

BOOL aibrainman_InitSys(void)
{
	s32 i;
	for (i = 0; i < NUM_THOUGHTCLASSES; i++)
	{
		if (_ThoughtClassSpec[i].pInitFunc)
		{
			if (!_ThoughtClassSpec[i].pInitFunc(_ThoughtClassSpec[i].nHowMany))
			{
				aibrainman_UninitSys();
				return FALSE;
			}
		}
	}

	FASSERT(_pActiveBrains == NULL);
	_pActiveBrains = APE_NEW CNiList<CAIBrain*>(aimain_pNodePool);
	if (!_pActiveBrains)
	{
		aibrainman_UninitSys();
		return FALSE;
	}

	FASSERT(_pPendingDeactivates == NULL);
	_pPendingDeactivates = APE_NEW CNiList<CAIBrain*>(aimain_pNodePool);
	if (!_pPendingDeactivates)
	{
		aibrainman_UninitSys();
		return FALSE;
	}
	
	return TRUE;
	
}


void aibrainman_DoDeactivations(void)
{
	if (_pPendingDeactivates)
	{
		CNiIterator<CAIBrain*> it = _pPendingDeactivates->Begin();
		while (it.IsValid())
		{
			CAIBrain* pBrain = it.Get();
			BOOL bDidIt = _pActiveBrains->Remove(it.Get());
			FASSERT(bDidIt);
			it.Next();
		}
		_pPendingDeactivates->RemoveAll();
	}
}


void aibrainman_UninitSys(void)
{

	aibrainman_DoDeactivations();

	CNiIterator<CAIBrain*> it = _pActiveBrains->Begin();
	CAIBrain* pTmp = it.Get();
	FASSERT(!_pActiveBrains || _pActiveBrains->Size()==0); //else, not all bots we're removed from the world.
	APE_DELETE(_pActiveBrains); _pActiveBrains = NULL;

	FASSERT(!_pPendingDeactivates || _pPendingDeactivates->Size()==0); //else, not all bots we're removed from the world.
	APE_DELETE(_pPendingDeactivates); _pPendingDeactivates = NULL;

	for (s32 i = 0; i < NUM_THOUGHTCLASSES; i++)
	{
		if (_ThoughtClassSpec[i].pCleanupFunc)
		{
			_ThoughtClassSpec[i].pCleanupFunc();
		}
	}
}


static void _UpdateLODFlags(void)
{
	_uTotalBrainsInWorld = 0;
	_uActiveBrains = 0;
	_uVisibleBrains = 0;


	CNiIterator<CAIBrain*> it = _pActiveBrains->Begin();
	while (it.IsValid())
	{
		CAIBrain* pBrain = it.Get();

		_uTotalBrainsInWorld++;

		pBrain->ClearLODActiveAndVisible();

		BOOL bIn = FALSE;
		CEntity* pEntity = pBrain->GetAIMover()->GetEntity();
		if (pEntity->TypeBits() & ENTITY_BIT_BOT)
		{
			 
			if (((CBot*) pEntity)->m_pWorldMesh->GetVolumeFlags() & FVIS_VOLUME_IN_ACTIVE_LIST)
			{
				bIn = TRUE;
				pBrain->SetActiveAndVisibleBits(CAIBrain::LOD_ACTIVE);
			}
			if (((CBot*) pEntity)->m_pWorldMesh->GetVolumeFlags() & FVIS_VOLUME_IN_VISIBLE_LIST)
			{
				bIn = TRUE;
				pBrain->SetActiveAndVisibleBits(CAIBrain::LOD_VISIBLE);
			}
			if (!(pBrain->GetLODFlags() & (FVIS_VOLUME_IN_ACTIVE_LIST | FVIS_VOLUME_IN_VISIBLE_LIST)))
			{
				if ((pBrain->IsGoal(pBrain->GetCurThought()) &&
					!(pBrain->GetCurThoughtPtr()->m_uThoughtFlags & CAIThought::THOUGHTFLAG_LOD_TREAT_GOAL_WITH_JOB_LOD_RULES)) ||
					pBrain->IsGoalChangePending())
				{
					bIn = TRUE;
					pBrain->SetActiveAndVisibleBits(CAIBrain::LOD_ON_GOAL_OVERRIDE);
				}
				else if ( pBrain->IsFollowing() && 
						  (aiutils_IsPlayer(pBrain->GetLeader()->GetAIMover()->GetEntity()) || CAIBrain::IsGoal(pBrain->GetLeader()->GetCurThought())))

				{
					bIn = TRUE;
					pBrain->SetActiveAndVisibleBits(CAIBrain::LOD_ON_GOAL_OVERRIDE);
				}
			}
		  	
			_uVisibleBrains+= bIn;

		}
		else
		{
			FWorld_nTrackerSkipListCount = 0;
			pBrain->GetAIMover()->GetEntity()->AppendTrackerSkipList();

			for (u32 i = 0; i < FWorld_nTrackerSkipListCount; i++)
			{
				if (FWorld_apTrackerSkipList[i]->GetVolumeFlags() & FVIS_VOLUME_IN_ACTIVE_LIST)
				{
					bIn = TRUE;
					pBrain->SetActiveAndVisibleBits(CAIBrain::LOD_ACTIVE);
				}
				if (FWorld_apTrackerSkipList[i]->GetVolumeFlags() & FVIS_VOLUME_IN_VISIBLE_LIST)
				{
					bIn = TRUE;
					pBrain->SetActiveAndVisibleBits(CAIBrain::LOD_VISIBLE);
				}
				if (!(pBrain->GetLODFlags() & (FVIS_VOLUME_IN_ACTIVE_LIST | FVIS_VOLUME_IN_VISIBLE_LIST)) && pBrain->IsGoal(pBrain->GetCurThought()))
				{
					bIn = TRUE;
					pBrain->SetActiveAndVisibleBits(CAIBrain::LOD_ON_GOAL_OVERRIDE);
				}
			}

			_uVisibleBrains+= bIn;
		}

		pBrain->ClearHearingLOD();
		if (((u32) pBrain->GetGUID() & 1) == (FVid_nFrameCounter & 1))
		{
			pBrain->ForceHearingLODOnThisFrame();
		}

		it.Next();
	}
}

void aibrainman_Work(void)
{
	FASSERT(_pActiveBrains);

	aibrainman_DoDeactivations();
	_UpdateLODFlags();
	
	
	//all AIBotMovers BeginFrame
	PROTRACK_BEGINBLOCK("MOVERS");
	CNiIterator<CAIBrain*> it = _pActiveBrains->Begin();
	while (it.IsValid())
	{
		CAIBrain* pBrain = it.Get();

		if (pBrain->IsLODActive())
		{
			pBrain->GetAIMover()->BeginFrame();
		}
		it.Next();
	}
	PROTRACK_ENDBLOCK();//"MOVERS");

	//all brains think (possibly giving orders to botmovers)
	PROTRACK_BEGINBLOCK("BRAINS");
	it = _pActiveBrains->Begin();
	while (it.IsValid())
	{
		CAIBrain* pBrain = it.Get();
		if (pBrain->IsLODActive())
		{
			pBrain->Work();
		}
		it.Next();
	}
	PROTRACK_ENDBLOCK();//"BRAINS");

	//all AIBotMovers apply directions into BotLayer
	PROTRACK_BEGINBLOCK("MOVERS");
	it = _pActiveBrains->Begin();
	while (it.IsValid())
	{
		CAIBrain* pBrain = it.Get();
		if (pBrain->IsLODActive())
		{
			pBrain->GetAIMover()->EndFrame();
		}
		it.Next();
	}
	PROTRACK_ENDBLOCK();//"MOVERS");

	u8 uUpdateStage = 0;
	while (uUpdateStage < 2)
	{
		//all bots that are being driven by brains, do their thing
		PROTRACK_BEGINBLOCK("ENTITY_WORK");
		it = _pActiveBrains->Begin();
		while (it.IsValid())
		{
			CAIBrain* pBrain = it.Get();
			BOOL bOverrideOneFrame  = pBrain->GetLODFlags() & CAIBrain::LOD_OVERRIDE_FOR_ONE_WORK;
			pBrain->ClearLODOverrideForOneWork();

			if (bOverrideOneFrame ||
					(pBrain->IsLODActive() &&
					!pBrain->GetFlag_DisableEntityWork() &&
					pBrain->GetAIMover()->GetEntity()->GetClassHierarchyWorkMask() &&
					(pBrain->GetAIMover()->GetEntity()->GetParent() && pBrain->GetAIMover()->GetEntity()->GetParent()->AIBrain()) == uUpdateStage))
			{
				pBrain->GetAIMover()->GetEntity()->Work();
			}

			it.Next();
		}
		PROTRACK_ENDBLOCK();//"ENTITY_WORK");

		uUpdateStage++;
	}

	aibrainman_DoDeactivations();


	//
	// Test Code for only allowing certain named bots to be created
	//
return;	   //disable test code


	it = _pActiveBrains->Begin();
	while (it.IsValid())
	{
		CAIBrain* pBrain = it.Get();
		//only let bots named "xxx come through here
		CEntity* pEntity = pBrain->GetAIMover()->GetEntity();
		if (
//			pBrain->GetGUID() == 2 ||
			pBrain->GetGUID() == 25 ||
//			pBrain->GetGUID() == 27 ||
//			(pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_VEHICLE) ||
//			(pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTTITAN) ||
//			(pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTMINER) ||
//			(pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTJUMPER) ||
//			(pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTMINER) ||
//			(pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTELITEGUARD) ||
			(pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTSCOUT) ||
//			(pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_SITEWEAPON) ||
//			(pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTPRED) ||
//			(pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTPROBE) ||
//			(pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTSNARQ) ||
//			(pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_VEHICLESENTINEL) ||
//			(pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTCORROSIVE) ||
//			(pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTZOM) ||
//			(pEntity->Name() && fclib_strnicmp(pEntity->Name(), "xa_", 3)==0) ||
//			(pEntity->Name() && fclib_stricmp(pEntity->Name(), "xa_autobot_grunt1")==0)	||
//			(pEntity->Name() && fclib_stricmp(pEntity->Name(), "washer")==0)	||
//		(pEntity->Name() && fclib_stricmp(pEntity->Name(), "TankGrunt")==0)	||
//			(pEntity->Name() && fclib_stricmp(pEntity->Name(), "GlitchBuddy")==0) ||
//			(pEntity->Name() && fclib_stricmp(pEntity->Name(), "rat_glitch")==0) ||
//			(pEntity->Name() && fclib_strnicmp(pEntity->Name(), "TB_", fclib_strlen("TB_"))==0) ||
//			(pEntity->Name() && fclib_stricmp(pEntity->Name(), "gleep")==0) ||
//			(pEntity->Name() && fclib_stricmp(pEntity->Name(), "glop")==0) ||
//			(pEntity->Name() && fclib_stricmp(pEntity->Name(), "hubpred03")==0) ||
//			(pEntity->Name() && fclib_stricmp(pEntity->Name(), "testpred")==0) ||
//			(pEntity->Name() && fclib_stricmp(pEntity->Name(), "r3")==0)||
//			(pEntity->Name() && fclib_stricmp(pEntity->Name(), "AI04")==0) ||
//			(pEntity->Name() && fclib_stricmp(pEntity->Name(), "hubgr01")==0) ||
//			(pEntity->Name() && fclib_stricmp(pEntity->Name(), "csv04")==0) ||
			0
			)
		{
			//keep for testing
		}
		else
		{
			pEntity->RemoveFromWorld();
		}
		it.Next();
	}

}

#if FANG_PRODUCTION_BUILD	
void aibrainman_Render(u32 uRenderBits)
{

}
#else //FANG_PRODUCTION_BUILD	

cchar* pszAITextColor[2] =
{
	"~c99999999",
	"~c00000099"
};
u32 _uCurAIMenuColor = 0;

void aibrainman_Render(u32 uRenderBits)
{
	CFVec3A CamPos;
	CFVec3A origin;
	CFVec3A DrawThisPt;
	CAIBrain* pClosestBrainThisFrame = NULL;
	f32 fClosestBrainDist = 0.0f;
	CNiIterator<CAIBrain*> it;
	u8 uColor = 0;
		
	
	if (!aiutils_GetCamPos(&CamPos))
	{
		return;
	}

	if (uRenderBits & AIBM_DISPLAY_ACTIVE_LIST)
	{
		ftext_DebugPrintf( 0.5f, 0.1f, "~w1%s Total AI %02d",pszAITextColor[_uCurAIMenuColor], _uTotalBrainsInWorld);
		ftext_DebugPrintf( 0.5f, 0.115f, "~w1%s Active AI %02d",pszAITextColor[_uCurAIMenuColor], _uActiveBrains);
		ftext_DebugPrintf( 0.5f, 0.130f, "~w1%sActive",pszAITextColor[_uCurAIMenuColor]);
		ftext_DebugPrintf( 0.6f, 0.130f, "~w1%sOut",pszAITextColor[_uCurAIMenuColor]);
		u32 uActiveCounter = 0;
		u32 uVisibleCounter = 0;
		u32 uInActiveCounter = 0;

		it = _pActiveBrains->Begin();
		while (it.IsValid())
		{
			CAIBrain* pBrain = it.Get();

			if (pBrain->IsLODActive())
			{
				ftext_DebugPrintf( 0.5f, 0.145f+0.015f*(f32)uActiveCounter, "~w1%s%02d",pszAITextColor[_uCurAIMenuColor], pBrain->GetGUID());//pBrain->GetAIMover()->GetEntity()->Guid());
				uActiveCounter++;
			}
			else
			{
				ftext_DebugPrintf( 0.6f, 0.145f+0.015f*(f32)uInActiveCounter, "~w1%s%02d",pszAITextColor[_uCurAIMenuColor], pBrain->GetGUID());//pBrain->GetAIMover()->GetEntity()->Guid());
				uInActiveCounter++;
			}

			it.Next();
		}
	}

	if (1)//uRenderBits & AIBM_DISPLAY_PLAYER_BRAIN)
	{
		CEntity* pPlayerEntity = Player_aPlayer[0].m_pEntityCurrent;
		if (pPlayerEntity && pPlayerEntity->AIBrain() && !pPlayerEntity->AIBrain()->GetFlag_Active())
		{
			CAIBrain* pBrain = pPlayerEntity->AIBrain();
			FViewport_t * pView = fviewport_GetActive();
			CFVec3A ScreenPos;
			ScreenPos.z = -1.0f;
			f32 fLineHeight = 0.02f;

			CFVec3A Point_VS;
			Point_VS = pBrain->GetLoc();
			FXfm_pView->m_MtxF.MulPoint(Point_VS);
			BOOL bInView = FALSE;
			if (Point_VS.z > 0.0f)
			{
				CFVec3A loc = pBrain->GetLoc();
				loc.y+=pBrain->GetAIMover()->GetHeight();

				fviewport_ComputeUnitOrtho3DScreenPoint_WS(pView, &(FXfm_pView->m_MtxF), &loc, &ScreenPos);
				if (ScreenPos.z >0.0f && ScreenPos.x < 1.0f && ScreenPos.x > -1.0f && ScreenPos.y < 1.0f && ScreenPos.y > -1.0f)
				{
					ScreenPos.x = 0.5f+0.5f*ScreenPos.x;
					ScreenPos.y = (0.375f -ScreenPos.y*0.375f);
					bInView = TRUE;
				}

				//
				// draw the PLAYER BOT visibility rays
				//
				u8* puaVisRays = aigroup_GetVisRaysAt(pPlayerEntity);
				for (u8 i = 0; i < 8; i++)
				{
					origin = pBrain->GetAIMover()->GetEyePos();
					DrawThisPt = CGenericWait::VisRayToLookAtLoc(i, origin, pBrain->GetAIMover()->GetRadiusXZ()+_PLAYERBRAIN_VISRAYSCAN_LOS_DIST);
					u8 auScannerStatusToColorMap[3] = {AICOLOR_BLUE, AICOLOR_RED, AICOLOR_GREEN};
					uColor = auScannerStatusToColorMap[puaVisRays[i]];
					fdraw_SolidLine( &(origin.v3), &(DrawThisPt.v3), &aiutils_paDebugColors[uColor]);
				}

				//draw how bright engine is saying it is near the player
				DrawThisPt = *(pPlayerEntity->GetTagPoint(0));
				DrawThisPt.y+=2.0f;
				f32 fBrightness = aigroup_GetLightAt(pPlayerEntity);
				CFColorRGBA tmp;
				tmp.fBlue = 1.0f;
				tmp.fRed = fBrightness;
				tmp.fGreen = fBrightness;
				tmp.fAlpha = 1.0f;
				fdraw_FacetedWireSphere( &(DrawThisPt.v3), 1.0f, 1, 1, &tmp);


//				ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "~w1%s%s%02d", pszAITextColor[_uCurAIMenuColor],"team:", ((CBot*) pBrain->GetAIMover()->GetEntity())->GetTeam());
//				ScreenPos.y -=fLineHeight;

//				ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "~w1%s%s%02d", pszAITextColor[_uCurAIMenuColor],"race:", pBrain->GetRace());
//				ScreenPos.y -=fLineHeight;

				pBrain->DebugRender(ScreenPos.x, ScreenPos.y);
			}

		/*
			if (pBrain->m_pFormation)
			{
				pBrain->m_pFormation->DebugRender();
			}
		*/

		}
	}

	if (uRenderBits & AIBM_DISPLAY_ALLNPCBRAINMASK)
	{
		it = _pActiveBrains->Begin();
		while (it.IsValid())
		{
			s32 nLod = 0;

			CAIBrain* pBrain = it.Get();
			it.Next();
			CAIMover* pMover = pBrain->GetAIMover();

			if (((CBot*) pMover->GetEntity())->GetCurMech())
			{   //skip drivers and passengers
//				continue;  
			}

			if ((pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOT) && ((CBot*) pMover->GetEntity())->IsDead())
			{
				continue;  //skip dead dudes
			}

			FViewport_t * pView = fviewport_GetActive();
			CFVec3A ScreenPos;
			ScreenPos.z = -1.0f;
			f32 fLineHeight = 0.02f;

			CFVec3A Point_VS;
			Point_VS = pBrain->GetLoc();
			FXfm_pView->m_MtxF.MulPoint(Point_VS);
			BOOL bInView = FALSE;
			if (Point_VS.z > 0.0f)
			{
				CFVec3A loc = pBrain->GetLoc();
				loc.y+=pBrain->GetAIMover()->GetHeight();

				fviewport_ComputeUnitOrtho3DScreenPoint_WS(pView, &(FXfm_pView->m_MtxF), &loc, &ScreenPos);
				if (ScreenPos.z >0.0f && ScreenPos.x < 1.0f && ScreenPos.x > -1.0f && ScreenPos.y < 1.0f && ScreenPos.y > -1.0f)
				{
					ScreenPos.x = 0.5f+0.5f*ScreenPos.x;
					ScreenPos.y = (0.375f -ScreenPos.y*0.375f);
					bInView = TRUE;
				}
			}

			if (!bInView)	//let last frames closest guy through
			{
				continue;
			}
			f32 fCamDist2 = CamPos.DistSq(pBrain->GetLoc());
			if (!pClosestBrainThisFrame || fCamDist2 < fClosestBrainDist)
			{
				pClosestBrainThisFrame = pBrain;
				fClosestBrainDist = fCamDist2;
			}

			nLod = 0;

			f32 fLodDist = 100.0f;
			f32 fLodDist2 = 250.0f;

			if (_pActiveBrains->Size() < 10)
			{
				fLodDist = 250.0f;
				fLodDist2  =  3000.0f;
			}
			if (fCamDist2 < fLodDist*fLodDist)
			{
				nLod = 2;
			}
			else if (fCamDist2 < fLodDist2*fLodDist2)
			{
				nLod = 1;
			}

			if (pBrain == _pClosestBrain)
			{
				nLod = 2;
			}

			if (!nLod)
			{
				continue;
			}

			if (nLod == 1)
			{
				ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "~w1%s%02d",pszAITextColor[_uCurAIMenuColor], pBrain->GetGUID());
				ScreenPos.y -=fLineHeight;
			}
			else
			{

				if (uRenderBits & AIBM_DISPLAY_BRAIN_STATES)
				{
					cchar* pszJob = CAIBrain::ThoughtTypeToString(pBrain->GetJobThought());
					cchar* pszGoal = "";
					if (pBrain->GetCurThought() != pBrain->GetJobThought())
					{
						pszGoal = CAIBrain::ThoughtTypeToString(pBrain->GetCurThought());
					}
					ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "~w1%s%02d,%s,%s%s%s%s",pszAITextColor[_uCurAIMenuColor],
										pBrain->GetGUID(),
										pszJob,
										pszGoal,
										pBrain->HasTalkThought() ? ",TALK" : "",
										pBrain->IsFollowing() ? ",FOLLOW" : "",
										pBrain->GetReactPtr() ? ",REACT": "");
					ScreenPos.y -=fLineHeight;


			//		ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "~w1%s%s%02d", pszAITextColor[_uCurAIMenuColor],"team:", ((CBot*) pBrain->GetAIMover()->GetEntity())->GetTeam());
			//		ScreenPos.y -=fLineHeight;

			//		ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "~w1%s%s%02d", pszAITextColor[_uCurAIMenuColor],"race:", pBrain->GetRace());
			//		ScreenPos.y -=fLineHeight;
				}
				
				if (uRenderBits & AIBM_DISPLAY_BRAIN_NAME)
				{
					cchar* pName = "NoName";
					if (pBrain && pBrain->GetAIMover() && pBrain->GetAIMover()->GetEntity() && pBrain->GetAIMover()->GetEntity()->Name())
					{
						pName = pBrain->GetAIMover()->GetEntity()->Name();
					}
					ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "~w1%s%02d,%s",pszAITextColor[_uCurAIMenuColor], pBrain->GetGUID(), pName);
					ScreenPos.y -=fLineHeight;
				}
				
				if (uRenderBits & AIBM_DISPLAY_KNOWLEDGE_INFO)
				{
					pBrain->DebugRender(ScreenPos.x, ScreenPos.y);
				}
		 
			
				//draw the current path goal, if there is one
				if (uRenderBits & AIBM_DISPLAY_CURTHOUGHT_DATA)
				{
					CGenericWait* pLookAtMgr = NULL;

					if (pBrain->GetCurThought() == CAIBrain::TT_WANDER && pBrain->GetCurThoughtPtr())
					{
						DrawThisPt = ((CGenericWander*) pBrain->GetCurThoughtPtr())->m_WanderGoal;
						fdraw_FacetedWireSphere( &(DrawThisPt.v3), 2.0f, 2, 2, &aiutils_paDebugColors[AICOLOR_RED]);

						origin = pBrain->GetLoc();
						origin.y += pBrain->GetAIMover()->GetHeight();
						fdraw_SolidLine( &(origin.v3), &(DrawThisPt.v3), &aiutils_paDebugColors[AICOLOR_RED]);
						DrawThisPt.y += pBrain->GetAIMover()->GetHeight();
					}
					if (pBrain->GetCurThought() == CAIBrain::TT_WAIT && pBrain->GetCurThoughtPtr()->SupportsClassInterface(CLASS_CGENERICWAIT))
					{
						pLookAtMgr = (CGenericWait*) pBrain->GetCurThoughtPtr();
					}
					else if (pBrain->GetCurThought() == CAIBrain::TT_FACEIT && pBrain->GetCurThoughtPtr())
					{
						CGenericFaceIt* pFaceIt = (CGenericFaceIt*) pBrain->GetCurThoughtPtr();

						//where are we being told to look?
						if (pFaceIt->m_pLookAtObj)
						{
							DrawThisPt = *(pFaceIt->m_pLookAtObj->GetTagPoint(0));
						}
						else
						{
							DrawThisPt = pFaceIt->m_LookAtLoc;
						}
						fdraw_FacetedWireSphere( &(DrawThisPt.v3), 2.0f, 2, 2, &aiutils_paDebugColors[AICOLOR_RED]);
					}
					else if (pBrain->GetCurThought() == CAIBrain::TT_GOTO && pBrain->GetCurThoughtPtr())
					{
						DrawThisPt = ((CGenericGoto*) pBrain->GetCurThoughtPtr())->m_GoalLoc;
						fdraw_FacetedWireSphere( &(DrawThisPt.v3), 2.0f, 2, 2, &aiutils_paDebugColors[AICOLOR_RED]);
						origin = pBrain->GetLoc();
						origin.y += pBrain->GetAIMover()->GetHeight();
						fdraw_SolidLine( &(origin.v3), &(DrawThisPt.v3), &aiutils_paDebugColors[AICOLOR_RED]);
						DrawThisPt.y += pBrain->GetAIMover()->GetHeight();
					}
					else if (pBrain->GetCurThought() == CAIBrain::TT_COMBAT && pBrain->GetCurThoughtPtr())
					{
						if (pBrain->GetCurThoughtPtr()->SupportsClassInterface(CLASS_CGROUNDCOMBAT))
						{
							CGroundCombat* pT = (CGroundCombat*) pBrain->GetCurThoughtPtr();

							if (pT->m_pLookAtMgr)
							{
								pLookAtMgr = pT->m_pLookAtMgr;
							}

							//Display the current rule state
							ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "%s%s",pszAITextColor[_uCurAIMenuColor], pT->GetRuleSetName());
							ScreenPos.y -=fLineHeight;

							//Display the current rule state
							ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "%s%s",pszAITextColor[_uCurAIMenuColor], pT->GetRuleSetStateName());
							ScreenPos.y -=fLineHeight;

							//Display the current tactic
							ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "%s%s",pszAITextColor[_uCurAIMenuColor], pT->GetTacticName());
							ScreenPos.y -=fLineHeight;

							//draw the last known enemy position
							origin = pMover->GetEyePos();
							if (pT->m_pEnemy)
							{
								DrawThisPt = pT->m_LastEnemyMark;
							}
							else
							{//no enemy
								DrawThisPt = origin;	 
								DrawThisPt.y += 5.0f;
							}
							uColor = AICOLOR_YELLOW;		 //No LOS right now
							fdraw_SolidLine( &(origin.v3), &(DrawThisPt.v3), &aiutils_paDebugColors[uColor]);
							fdraw_FacetedWireSphere( &(DrawThisPt.v3), 1.0f, 3, 3, &aiutils_paDebugColors[uColor]);

						   /*
							char* apszDodgeLabels[] = {"DODGE_IN","DODGE_OUT"};
							ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "%s%s", pszAITextColor[_uCurAIMenuColor], apszDodgeLabels[pT->m_uDodgeState]);
							ScreenPos.y -=fLineHeight;

							ScreenPos.x-=0.25f;
							ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "%sDa: %d",pszAITextColor[_uCurAIMenuColor], pT->m_AttackPtSearch.m_nCurDir);
							ScreenPos.y +=fLineHeight;
							ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "%sdx: %d",pszAITextColor[_uCurAIMenuColor], pT->m_AttackPtSearch.m_nCurRangeXZ);
							ScreenPos.y +=fLineHeight;
							ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "%sdy: %d",pszAITextColor[_uCurAIMenuColor], pT->m_AttackPtSearch.m_nCurRangeY);
							ScreenPos.y +=fLineHeight;
							 */
						}
						else if (pBrain->GetCurThoughtPtr()->SupportsClassInterface(CLASS_CSCOUTALERT))
						{
							CScoutAlert* pS = (CScoutAlert*) pBrain->GetCurThoughtPtr();

							//Display the current scout state
							extern cchar* apszScoutStages[];
							ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "%s%s",pszAITextColor[_uCurAIMenuColor], apszScoutStages[pS->m_uScoutStage]);
							ScreenPos.y -=fLineHeight;
						}
						

					}
					else if (pBrain->GetCurThought() == CAIBrain::TT_PATROL && pBrain->GetCurThoughtPtr())
					{
						//nothing to display... entire path is the goal
					}
				
				
					if (pBrain->GetCurFollowerThoughtPtr())
					{
						if (pBrain->GetCurFollowerThoughtPtr()->SupportsClassInterface(CLASS_CGENERICFOLLOW))
						{
							CGenericFollow* pFollow = (CGenericFollow*) pBrain->GetCurFollowerThoughtPtr();
							if (pFollow)
							{
								if (pFollow->m_pLookAtMgr)
								{
									pLookAtMgr = pFollow->m_pLookAtMgr;
								}
							}
						}
					}
				

					if (pLookAtMgr)
					{
						if (pLookAtMgr->m_uWaitFlags & CGenericWait::WAIT_VISRAYS_ENABLED)
						{

							//draw the scanner lookat rays
							for (u8 i = 0; i < 8; i++)
							{
								origin = pMover->GetEyePos();
								DrawThisPt = pLookAtMgr->VisRayToLookAtLoc(i, origin, pMover->GetRadiusXZ()+_WAIT_VISRAYSCAN_LOS_DIST);
								u8 auScannerStatusToColorMap[3] = {AICOLOR_BLUE, AICOLOR_RED, AICOLOR_GREEN};
								uColor = auScannerStatusToColorMap[pLookAtMgr->m_auScannerStatus[i]];
								fdraw_SolidLine( &(origin.v3), &(DrawThisPt.v3), &aiutils_paDebugColors[uColor]);

								//draw a sphere on the end of the current visray during a visray scan2
								if (!pLookAtMgr->IsVisRayScanPaused() && i == pLookAtMgr->m_uCurScan)
								{
									fdraw_FacetedWireSphere( &(DrawThisPt.v3), 1.0f, 1, 1, &aiutils_paDebugColors[AICOLOR_GREEN]);
								}
							}
						}
					}
				}
				
				
				if (uRenderBits & AIBM_DISPLAY_WEAPON_INFO)
				{
					if (pBrain->GetCurThought() == CAIBrain::TT_COMBAT && pBrain->GetCurThoughtPtr())
					{
						if (pBrain->GetCurThoughtPtr()->SupportsClassInterface(CLASS_CGROUNDCOMBAT))
						{
							CGroundCombat* pT = (CGroundCombat*) pBrain->GetCurThoughtPtr();
							CBot* pBot = (CBot*) pBrain->GetAIMover()->GetEntity();
							
							if (((CBotControl*) pBot->Controls())->m_nFlags & CBotControl::FLAG_AIM_AT_TARGET_POINT)
							{
								if (pBrain->GetAIMover()->HasTargetLock())
								{
	    							ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "%sLock: REQ,LOCK",pszAITextColor[_uCurAIMenuColor]);
								}
								else
								{
	    							ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "%sLock: REQ,FAIL",pszAITextColor[_uCurAIMenuColor]);
								}
							}
							else
							{
								ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "%sLock: N/A",pszAITextColor[_uCurAIMenuColor]);
							}
							ScreenPos.y -=fLineHeight;

							//Display the current rule state
							if (!pBrain->GetAIMover()->GetNumWeaponCtrls())
							{

								ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "%sNo Weaps",pszAITextColor[_uCurAIMenuColor]);
								ScreenPos.y -=fLineHeight;
							}
							else
							{

								for (u32 w = 0; w < pBrain->GetAIMover()->GetNumWeaponCtrls(); w++)
								{
									if (pBrain->GetAIMover()->GetWeaponCtrlPtr(w))
									{
										ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "%sBURST: %s",pszAITextColor[_uCurAIMenuColor], pBrain->GetAIMover()->GetWeaponCtrlPtr(w)->IsBursting() ? "BURSTING": "NOT");
									}
									ScreenPos.y -=fLineHeight;
									if (pT->m_uAttackFlags & CGroundCombat::FLAG_TRIGGERHAPPY)
									{
										if (pBrain->GetAIMover()->GetWeaponCtrlPtr(w))
										{
											ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "%sREADY: %s",pszAITextColor[_uCurAIMenuColor], pBrain->GetAIMover()->GetWeaponCtrlPtr(w)->IsReady(pT->m_uAttackFlags & CGroundCombat::FLAG_TRIGGERHAPPY) ? "THAPY": "NOT");
										}
										ScreenPos.y -=fLineHeight;
									}
									else
									{
										if (pBrain->GetAIMover()->GetWeaponCtrlPtr(w))
										{
											ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "%sREADY: %s",pszAITextColor[_uCurAIMenuColor], pBrain->GetAIMover()->GetWeaponCtrlPtr(w)->IsReady(pT->m_uAttackFlags & CGroundCombat::FLAG_TRIGGERHAPPY) ? "YES": "NOT");
										}
										ScreenPos.y -=fLineHeight;
									}
									if (pBrain->GetAIMover()->GetWeaponCtrlPtr(w))
									{
										ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "%sFIRED: %s",pszAITextColor[_uCurAIMenuColor], pBrain->GetAIMover()->GetWeaponCtrlPtr(w)->DidJustFire() ? "FIRED": "");
									}
									ScreenPos.y -=fLineHeight;
								}
							}

							//draw current aim at pt
							uColor = AICOLOR_CYAN;
							DrawThisPt = pT->m_LastAim;
							fdraw_FacetedWireSphere( &(DrawThisPt.v3), 1.0f, 1, 1, &aiutils_paDebugColors[uColor]);
						}
					}

				}


				if (uRenderBits & AIBM_DISPLAY_PATROLPATHS)
				{
					if (pBrain->GetCurThought() == CAIBrain::TT_PATROL && pBrain->GetCurThoughtPtr())
					{
			//				if (pBrain->GetAIMover()->GetCurRoomId() == player_GetRoomId(PLAYER_ZERO))
						{
							if (((CGenericPatrol*) pBrain->GetCurThoughtPtr())->m_pPatrolPath)
							{	
								((CGenericPatrol*) pBrain->GetCurThoughtPtr())->m_pPatrolPath->GetAIPath()->DebugRender(0);
							}
						}
			//				if (pBrain->GetAIMover()->GetCurRoomId() == player_GetRoomId(PLAYER_ZERO))
						{
							
							CESpline* pSpline = ((CGenericPatrol*) pBrain->GetCurThoughtPtr())->m_pPatrolPts;
							if (pSpline)
							{
								BOOL bLinkLast = 0;
								CFVec3 LastLoc;
								f32 fHeight = 0.0f;

								for (u16 i = 0; i < pSpline->PointCount(); i++)
								{
									CFVec3 drawLoc = pSpline->PointArray()[i].v3;
									if (!bLinkLast)
									{
										fHeight = drawLoc.y;
									}
		//							drawLoc.y = fHeight;
									fdraw_FacetedWireSphere( &(drawLoc), 2.0f, 1, 1 , &aiutils_paDebugColors[AICOLOR_RED]);
									if (bLinkLast)
									{
										fdraw_SolidLine(&drawLoc, &(LastLoc), &aiutils_paDebugColors[AICOLOR_RED]);
									}
									LastLoc = drawLoc;
									bLinkLast = TRUE;
									fHeight+= _kfDebugRenderHeightSpread;
								}
							}
						}
					}
				}

				if (uRenderBits & AIBM_DISPLAY_NEXT_WAYPOINT)
				{
					if (pBrain->GetCurThoughtPtr() && pBrain->GetAIMover()->m_PathWalker.IsValid())
					{
						if (pBrain->GetAIMover()->m_PathWalker.GetCurLoc(&DrawThisPt))
						{
							fdraw_FacetedWireSphere( &(DrawThisPt.v3), 2.0f, 1, 1, &aiutils_paDebugColors[AICOLOR_GREEN]);

							origin = pBrain->GetLoc();
						//	origin.y += pBrain->GetAIMover()->GetHeight();
							fdraw_SolidLine( &(origin.v3), &(DrawThisPt.v3), &aiutils_paDebugColors[AICOLOR_GREEN]);
						
							pBrain->GetAIMover()->m_pPath->GetEndOfPath(&DrawThisPt);
						//	origin.y+=1.0f-2.0f*fmath_RandomFloat();
							fdraw_SolidLine( &(origin.v3), &(DrawThisPt.v3), &aiutils_paDebugColors[AICOLOR_RED]);
							fdraw_FacetedWireSphere( &(DrawThisPt.v3), 2.0f, 1, 1, &aiutils_paDebugColors[AICOLOR_RED]);
						
						}


					}
				}

				if (uRenderBits & AIBM_DISPLAY_MOVER_INFO)
				{
					origin = pBrain->GetLoc();
					origin.y += aigraph_kfSurfaceOffset;

					//
					// Steering Forces and Avoidance Collision
					//

					CFVec3A tip;
					tip.x = pBrain->GetAIMover()->GetTorsoLookAtXZ().z;
					tip.z = -pBrain->GetAIMover()->GetTorsoLookAtXZ().x;
					tip.y = 0.f;
					tip.Mul(20.0f * pBrain->GetAIMover()->m_Controls.GetEntityControl()->m_fRotateCW);
					tip.Add(origin);

					fdraw_SolidLine( &(origin.v3), &(tip.v3) );
		 
					//draw steering sensor
			//		origin.y+=1.5f;
					s32 nColor = 1;
					if (pBrain->GetAIMover()->m_pAvoidThis)
					{
						nColor = 0;
					}
				
					CFCylnA testCyln;
					if (pBrain->GetAIMover()->CalcAvoidanceTestCyln(&testCyln))
					{
						fdraw_FacetedCylinder( &(testCyln.m_Origin.v3), &(testCyln.m_Axis.v3), testCyln.m_fWidth, testCyln.m_fHeight, &aiutils_paDebugColors[nColor], 2 );
					}

					//draw bounding sphere
					fdraw_FacetedCylinder( &(origin.v3), &(CFVec3A::m_UnitAxisY.v3), pBrain->GetAIMover()->GetRadiusXZ(), pBrain->GetAIMover()->GetHeight(), &aiutils_paDebugColors[nColor], 2 );
				
					//
					// draw Head Look Input
					//
					if (((CBot*) pBrain->GetAIMover()->GetEntity())->HeadIsLooking())
					{
						DrawThisPt = ((CBot*) pBrain->GetAIMover()->GetEntity())->GetHeadLookLocation();
						fdraw_FacetedWireSphere( &(DrawThisPt.v3), 1.0f, 1, 1, &aiutils_paDebugColors[AICOLOR_BLUE]);
					}

					DrawThisPt = pBrain->GetAIMover()->GetEyeLookAt();
					DrawThisPt.Mul(5.0f);
					DrawThisPt.Add(pBrain->GetAIMover()->GetEyePos());
					fdraw_SolidLine( &(pBrain->GetAIMover()->GetEyePos().v3), &(DrawThisPt.v3), &aiutils_paDebugColors[AICOLOR_MAGENTA]);
		
				}


				if (uRenderBits & AIBM_DISPLAY_HEARING)
				{
					//How alert is unit
					ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "~w1%s%s",pszAITextColor[_uCurAIMenuColor], CAIBrain::s_apszBrainReactionStateStrings[pBrain->m_uReactionState]);
					ScreenPos.y -=fLineHeight;

					ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "~w1%s%s:%f",pszAITextColor[_uCurAIMenuColor], "ALERT", pBrain->m_fBrainAlertUnit);
					ScreenPos.y -=fLineHeight;

					if (pBrain->IsSightMarkValid())
					{
						ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "~c99990099~w1%s%s:%f",pszAITextColor[_uCurAIMenuColor], "SIGHT_MARK:", aiutils_GetCurTimeSecs()-pBrain->m_fSightMarkTime);
						ScreenPos.y -=fLineHeight;

						pBrain->GetSightMark(&DrawThisPt);
						fdraw_FacetedWireSphere( &(DrawThisPt.v3), 1.0f, 1, 2, &aiutils_paDebugColors[AICOLOR_YELLOW]);
					}
					
					if (pBrain->IsSoundMarkValid())
					{
						ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "~c00999999~w1%s:%f", "SOUND_MARK:", aiutils_GetCurTimeSecs()-pBrain->m_fSoundMarkTime);
						ScreenPos.y -=fLineHeight;
						DrawThisPt = pBrain->GetSoundMarkLoc();
						fdraw_FacetedWireSphere( &(DrawThisPt.v3), 1.0f, 1, 2, &aiutils_paDebugColors[AICOLOR_CYAN]);
					}

					//draw Hearing Scale
					origin = *(pMover->GetEntity()->GetTagPoint(0));
					fdraw_FacetedWireSphere( &(origin.v3), pBrain->GetHearingMag(), &aiutils_paDebugColors[AICOLOR_MAGENTA], 2 );

				}

				if (uRenderBits & AIBM_DISPLAY_VISION)
				{
					CAIMemoryMedium* pVisCheckMem = NULL;
					if (pBrain->GetKnowledge().CanRememberAny(MEMORY_MEDIUM_VISCHECK, (CAIMemorySmall **) &pVisCheckMem))
					{
						CEntity* pOtherEntity = CEntity::Find(pVisCheckMem->m_uMediumData); //lookup by GUID
						if (pOtherEntity)
						{
							CFVec3A DeltaToTarg = *(pOtherEntity->GetTagPoint(0));
							DeltaToTarg.Sub(pBrain->GetAIMover()->GetEyePos());
//							DeltaToTarg.Unitize();
							DeltaToTarg.Mul(pVisCheckMem->m_fMediumData); //focus
							DeltaToTarg.Add(pBrain->GetAIMover()->GetEyePos());
							fdraw_FacetedWireSphere( &(DeltaToTarg.v3), 2.4f, &aiutils_paDebugColors[AICOLOR_MAGENTA], 2 );

							
							ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "~w1%s%3.3f",pszAITextColor[_uCurAIMenuColor], fDebugDisplayF32);
							ScreenPos.y -=fLineHeight;
						}
					}

				}
			}
		}
	}

	_pClosestBrain = pClosestBrainThisFrame;
}
#endif //FANG_PRODUCTION_BUILD	

BOOL aibrainman_IsActive(CAIBrain* pBrain)
{
	FASSERT(_pActiveBrains);
	return _pActiveBrains->Find(pBrain);
}


BOOL aibrainman_Activate(CAIBrain* pBrain)
{
	if (pBrain  && !pBrain->GetFlag_Active())
	{
		if (pBrain->GetAIMover()->GetEntity() &&
			(!(pBrain->GetAIMover()->GetEntity()->IsInWorld() ||
			pBrain->GetAIMover()->GetEntity()->IsMarkedForWorldRemove())))
		{  //must safeguard because this can get called in worldremove.
			return FALSE;
		}


		if (_pPendingDeactivates->Find(pBrain))
		{
			_pPendingDeactivates->Remove(pBrain);
		}
		
		FASSERT(pBrain->GetAIMover() && pBrain->GetAIMover()->GetEntity());
		FASSERT(_pActiveBrains);

		BOOL bOK = FALSE;
		if (_pActiveBrains->Find(pBrain))
		{
			bOK = TRUE;
		}
		else if (_pActiveBrains->PushHead(pBrain))
		{
			bOK = TRUE;
		}

		if (bOK)
		{
		   pBrain->SetFlag_Active();

			if (pBrain->GetAIMover() && pBrain->GetAIMover()->GetEntity()) 
			{	//brain manager will be calling entity work, and will be needing to control the object
				if (pBrain->GetAIMover()->GetEntity()->HasHumanOrBotControls() &&
					pBrain->GetAIMover()->GetEntity()->Controls() != pBrain->GetAIMover()->m_Controls.GetEntityControl() )
				{
					cchar* pszName = pBrain->GetAIMover()->GetEntity()->Name();
					if (!pszName)
					{
						pszName = "NO_NAME";
					}
					DEVPRINTF("Note! AIBrainman taking control of entity: %s\n", pszName);
				}
				pBrain->GetAIMover()->GetEntity()->SetControls( pBrain->GetAIMover()->m_Controls.GetEntityControl());
			}
			return TRUE;  //
		}
		else
		{
			FASSERT(0);
		}
	}
	return FALSE;  //could not activate
}


BOOL aibrainman_Deactivate(CAIBrain* pBrain, BOOL bClearBrain /*= TRUE*/)
{
	if (pBrain && _pActiveBrains->Find(pBrain))
	{
		if (_pPendingDeactivates->Find(pBrain))
		{
			return FALSE;
		}

		FASSERT(pBrain->GetFlag_Active());

		pBrain->ClearFlag_Active();

		if (bClearBrain)
		{
			pBrain->ClearBrain();
		}
		if (pBrain->GetAIMover()->m_Controls.GetEntityControl() == pBrain->GetAIMover()->GetEntity()->Controls())
		{
			pBrain->GetAIMover()->GetEntity()->SetControls(NULL);
		}
		_pPendingDeactivates->PushHead(pBrain);
		return TRUE;
	}
	return FALSE;  //
}


CAIBrain* aibrainman_AllocBrainAndMover(u32 nBrainType, u32 nMoverClass)
{
	CAIMover* pMover = NULL;
	BOOL bFailed = FALSE;
	//
	//  Create the appropriate mover object
	//
	switch (nMoverClass)
	{
		case MOVERCLASS_XZ:
			pMover = APE_NEW CAIBotMover();
			break;
		case MOVERCLASS_3D:
			pMover = APE_NEW CAI3DBotMover();
			break;
		case MOVERCLASS_CAR:
			pMover = APE_NEW CAICarMover();
			break;
		case MOVERCLASS_XZANDHOVER:
			pMover = APE_NEW CAIHoverBotMover();
			break;
		default:
			DEVPRINTF("aibrainman_AllocBrainAndMover() : Unknown MoverClass\n");
			break;
	}

	//
	//  Create a BotBrain with the thought types connected
	//
	CAIBrain* pBrain = APE_NEW CAIBrain();
	if (pBrain && pMover)
	{
		
		pBrain->Create(pMover);  //doesn't allocate memory.  Just initializes pBrain

		for (s16 j = 0; j < CAIBrain::NUM_THOUGHT_TYPES && !bFailed; j++)
		{
			if (_BrainSpec[nBrainType].anStateSpec[j] != NO_THOUGHT_CONNECTED)
			{
				FASSERT(_BrainSpec[nBrainType].anStateSpec[j] < NUM_THOUGHTCLASSES);
				pBrain->ConnectThought(j, _ThoughtClassSpec[_BrainSpec[nBrainType].anStateSpec[j]].pAccessFunc);	//allocates a pool of thoughts that this brain can pull from!
			}
		}
	}
	else
	{
		bFailed = TRUE;
	}

	if (bFailed)
	{
		DEVPRINTF("aibrainman_AllocBrainAndMover() : Failed\n");
	}

	return pBrain;
}


void aibrainman_Delete(CAIBrain* pBrain)
{
	APE_DELETE(pBrain->GetAIMover());
	APE_DELETE(pBrain);
}


CNiIterator<CAIBrain*> aibrainman_ActiveListBegin(void)
{
	return _pActiveBrains->Begin();
}


BOOL _GetDefaultBrainAndMoverTypeForEntity(CEntity* pEntity,  s8* pnBrainType, s8* pnMoverType)
{
	*pnBrainType = -1;
	*pnMoverType = -1;

	if (pEntity->TypeBits() & ENTITY_BIT_BOTPRED || pEntity->TypeBits() & ENTITY_BIT_BOTPROBE)
	{
		*pnBrainType = BRAINTYPE_GENERICHOVERCRAFT;
		*pnMoverType = MOVERCLASS_3D;
	}
	else if ((((CBot*)pEntity)->IsPillBox() || ((CBot*)pEntity)->IsRatGun()))
	{
		*pnBrainType = BRAINTYPE_PILL;
		*pnMoverType = MOVERCLASS_XZ;
	}
	else if ((pEntity->TypeBits() & ENTITY_BIT_VEHICLERAT))
	{
		*pnBrainType = BRAINTYPE_GENERICBIPED;
		*pnMoverType = MOVERCLASS_CAR;
	}
	else if ( pEntity->TypeBits() & ENTITY_BIT_BOTSCOUT )
	{
		*pnBrainType = BRAINTYPE_SCOUT;
		*pnMoverType = MOVERCLASS_XZ;
	}
	else if ( pEntity->TypeBits() & ENTITY_BIT_BOTMORTAR )
	{
		*pnBrainType = BRAINTYPE_MORTAR;
		*pnMoverType = MOVERCLASS_XZ;
	}
	else if ( pEntity->TypeBits() & (ENTITY_BIT_BOTJUMPER | ENTITY_BIT_BOTSNARQ) )
	{
		*pnBrainType = BRAINTYPE_GENERICBIPED;
		*pnMoverType = MOVERCLASS_XZANDHOVER;
	}
	else if (!(pEntity->TypeBits() & ENTITY_BIT_SITEWEAPON) &&
			 !(pEntity->TypeBits() & ENTITY_BIT_BOTSWARMER))
	{
		*pnBrainType = BRAINTYPE_GENERICBIPED;
		*pnMoverType = MOVERCLASS_XZ;
	}

	return (*pnBrainType !=-1 && *pnMoverType !=-1);
}


CAIBrain* aibrainman_Create(CEntity* pEntity, CEntityBuilder* pBuilder /* NULL*/)
{
	CAIBrain* pBrain = NULL;
	CAIBuilder* pAIBuilder = NULL;
	s8 nBrainType = -1;
	s8 nMoverType = -1;

	if (pBuilder)
	{
		pAIBuilder = pBuilder->m_pAIBuilder;
	}

	_GetDefaultBrainAndMoverTypeForEntity(pEntity, &nBrainType, &nMoverType);

	if (pAIBuilder && pAIBuilder->m_nBrainType != -1)
	{	//auto brain type
		nBrainType = pAIBuilder->m_nBrainType;

		if (nBrainType < 0 || nBrainType >= NUM_BRAINTYPES)
		{
			DEVPRINTF("aiBrainman_Create() : Unknown Brain Type (%d) requested by Builder\n", nBrainType);
		}
	}

	if (pAIBuilder && pAIBuilder->m_nMoverType != -1)
	{	//auto Mover type
		nMoverType = pAIBuilder->m_nMoverType;

		if (nMoverType < 0 || nMoverType >= NUM_MOVERCLASSES)
		{
			DEVPRINTF("aiBrainman_Create() : Unknown Mover Type (%d) requested by Builder\n", nMoverType);
		}

	}

	if (nBrainType != -1  && nMoverType != -1)
	{
		pBrain = aibrainman_AllocBrainAndMover(nBrainType, nMoverType);
		if (pBrain && pBrain->GetAIMover())
		{
			pBrain->GetAIMover()->Create(pEntity, pBrain->GetGUID());
			if (pAIBuilder)
			{
				pBrain->InitFromBuilder(pBuilder);
			}
		}
		else
		{
			DEVPRINTF("aibrainman_Create() : Could not allocate an AIBrain/AIMover combo\n");
			pBrain = NULL;
		}
	}
	return pBrain;
}


void aibrainman_Destroy(CEntity* pEntity)
{
	if (pEntity && pEntity->AIBrain())
	{
		aibrainman_Delete(pEntity->AIBrain());
		pEntity->SetAIBrain(NULL);
	}
}


void aibrainman_AddToWorld(CEntity* pEntity, BOOL bActivate /* = TRUE*/)
{
	if (pEntity && pEntity->AIBrain())
	{
		pEntity->AIBrain()->AddToWorld();
		if (bActivate)
		{
			aibrainman_Activate(pEntity->AIBrain());
		}
	}
}

void aibrainman_RemoveFromWorld(CEntity* pEntity)
{
	if (pEntity && pEntity->AIBrain())
	{
		aibrainman_Deactivate(pEntity->AIBrain());
		pEntity->AIBrain()->RemoveFromWorld();
	}
}

void aibrainman_CheckpointSave(CEntity* pEntity)
{
	if( pEntity->AIBrain() )
	{
		pEntity->AIBrain()->CheckpointSave();
	}

}


void aibrainman_CheckpointRestoreSys(void)
{
	aibrainman_DoDeactivations();
	FASSERT(!_pActiveBrains || _pActiveBrains->Size()==0); //else, not all bots we're removed from the world.
}

void aibrainman_CheckpointRestore(CEntity* pEntity)
{
	aibrainman_DoDeactivations();
	if( pEntity->AIBrain() )
	{
		pEntity->AIBrain()->CheckpointRestore();
	}

}

void aibrainman_CheckpointRestorePostamble(CEntity* pEntity)	//postamble to the restore
{
	if( pEntity->AIBrain() )
	{
		pEntity->AIBrain()->CheckpointRestorePostamble();
	}
}

//Some player clicked "Action" on this guy!
void aibrainman_NotifyActionEnable(CAIBrain* pBrain, CEntity* pEntity)
{
	if (pBrain &&
		pEntity &&
		pEntity->TypeBits() & ENTITY_BIT_BOT &&
		pBrain->GetAIMover()->GetEntity()->IsInWorld())
	{
		pBrain->NotifyActionEnable(pEntity);
	}
}


//Damage was done to a brain
void aibrainman_DamagedNotify(CAIBrain* pBrain, const CDamageResult *pDamageResult)
{
	if (pBrain &&
		pDamageResult &&
		(pBrain->GetAIMover()->GetEntity()->IsInWorld() || (Player_aPlayer[0].m_pEntityCurrent && (pBrain == Player_aPlayer[0].m_pEntityCurrent->AIBrain()))))
	{
		pBrain->NotifyDamaged(pDamageResult);
	}
}


//Damage was dealt by a brain
void aibrainman_DamageDealtNotify(CAIBrain* pBrain, const CDamageResult *pDamageResult)
{
	if (pBrain &&
		pDamageResult && 
		(pBrain->GetAIMover()->GetEntity()->IsInWorld() || (Player_aPlayer[0].m_pEntityCurrent && (pBrain == Player_aPlayer[0].m_pEntityCurrent->AIBrain()))))
	{
		pBrain->NotifyDamageDealt(pDamageResult);
	}
}


//A bot was Rammed By Another
void aibrainman_RammedByNotify(CAIBrain* pBrain, CEntity* pDamager)
{
	if (pBrain &&
		pDamager &&
		(pBrain->GetAIMover()->GetEntity()->IsInWorld() || aiutils_IsPlayer(pBrain->GetAIMover()->GetEntity())))
	{
		pBrain->NotifyRammedBy(pDamager);
	}
}


BOOL aibrainman_ConfigurePlayerBotBrain(CAIBrain* pBrain, u32 uPlayerId)
{
	FASSERT( uPlayerId >= 0 && uPlayerId < MAX_PLAYERS);
	if (pBrain)
	{
		ai_TurnOffPerceptor(pBrain, AI_PERCEPTOR_EYES);
		ai_TurnOffPerceptor(pBrain, AI_PERCEPTOR_EARS);
	//	ai_TurnOffPerceptor(pBrain, AI_PERCEPTOR_TOUCH);	//don't turn touch off, since NPC's are (cheating a bit) using player's brain to record total damage per sec 
		ai_TurnOffPerceptor(pBrain, AI_PERCEPTOR_RADIO);
		ai_IgnorePerceptor(pBrain, AI_PERCEPTOR_EYES);
		ai_IgnorePerceptor(pBrain, AI_PERCEPTOR_EARS);
		ai_IgnorePerceptor(pBrain, AI_PERCEPTOR_TOUCH);
		ai_IgnorePerceptor(pBrain, AI_PERCEPTOR_RADIO);
		aibrainman_Deactivate(pBrain, DONT_CLEAR_BRAIN);	//de-activate it, so that brainman doesn't call work on it
		pBrain->ForceFailureOnAllThoughtsExceptJob();

		if (Player_aPlayer[uPlayerId].m_pEntityOrig &&
			Player_aPlayer[uPlayerId].m_pEntityOrig->AIBrain())
		{
			if ( (pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_VEHICLE) ||
				 (pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_SITEWEAPON) )
			{
				pBrain->SetRace(Player_aPlayer[uPlayerId].m_pEntityOrig->AIBrain()->GetRace());
			}
		}
		return TRUE;
	}
	return FALSE;
}


BOOL aibrainman_ConfigurePostPossessionBotBrain(CAIBrain* pBrain)
{
	if (pBrain)
	{
		ai_TurnOnPerceptor(pBrain, AI_PERCEPTOR_EYES);
		ai_TurnOnPerceptor(pBrain, AI_PERCEPTOR_EARS);
		ai_TurnOnPerceptor(pBrain, AI_PERCEPTOR_TOUCH);
		ai_TurnOnPerceptor(pBrain, AI_PERCEPTOR_RADIO);

		ai_UsePerceptor(pBrain, AI_PERCEPTOR_EYES);
		ai_UsePerceptor(pBrain, AI_PERCEPTOR_EARS);
		ai_UsePerceptor(pBrain, AI_PERCEPTOR_TOUCH);
		ai_UsePerceptor(pBrain, AI_PERCEPTOR_RADIO);

		if (pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_SITEWEAPON)
		{
			// I guess this matches the above action, should only be on ratguns and pillboxs
			pBrain->SetRace(AIRACE_AMBIENT);
		}
		
		aibrainman_Activate(pBrain);	//re-activate it, since player had disabled it

		return TRUE;
	}
	return FALSE;
}


BOOL aibrainman_ConfigureNPCVehicleBrain(CAIBrain* pVehicleBrain, CAIBrain* pDriverBrain)
{
	CVehicle* pVehicleBot = NULL;
	if (pVehicleBrain->GetAIMover()->GetEntity() && pVehicleBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_VEHICLE)
	{
		pVehicleBot = (CVehicle*) pVehicleBrain->GetAIMover()->GetEntity();
	}
	FASSERT(pVehicleBot);

	if (!pVehicleBot)
	{
		return FALSE;
	}

	//set race of vehicle to match race of driver
	if( (pVehicleBot->LeafTypeBit() & ENTITY_BIT_VEHICLERAT) && pVehicleBrain->GetAttackRuleSet() == CAIBrain::ATTACKRULESET_RAT0 )
	{
		// no perceptors for racing Rats
		ai_TurnOffPerceptor(pVehicleBrain, AI_PERCEPTOR_EYES);
		ai_TurnOffPerceptor(pVehicleBrain, AI_PERCEPTOR_EARS);
		ai_TurnOffPerceptor(pVehicleBrain, AI_PERCEPTOR_TOUCH);
		ai_TurnOffPerceptor(pVehicleBrain, AI_PERCEPTOR_RADIO);
	}
	else
	{	//Loader and sentinel brains do the thinking for themselves, Driver is really just along for the ride....
		ai_TurnOnPerceptor(pVehicleBrain, AI_PERCEPTOR_EYES);
		ai_TurnOnPerceptor(pVehicleBrain, AI_PERCEPTOR_EARS);
		ai_TurnOnPerceptor(pVehicleBrain, AI_PERCEPTOR_TOUCH);
		ai_TurnOnPerceptor(pVehicleBrain, AI_PERCEPTOR_RADIO);

		ai_UsePerceptor(pVehicleBrain, AI_PERCEPTOR_EYES);
		ai_UsePerceptor(pVehicleBrain, AI_PERCEPTOR_EARS);
		ai_UsePerceptor(pVehicleBrain, AI_PERCEPTOR_TOUCH);
		ai_UsePerceptor(pVehicleBrain, AI_PERCEPTOR_RADIO);
	}

	//take on the the attack goal of the the driver. Only if it has an enemy. 
	//no enemy means that the driver is probably just using the attack state to get into the vehicle
	if (pDriverBrain)
	{
		if (pDriverBrain->GetCurThought() == CAIBrain::TT_COMBAT &&
			pDriverBrain->GetCurThoughtPtr() && 
			pDriverBrain->GetCurThoughtPtr()->SupportsClassInterface(CLASS_CGROUNDCOMBAT))
		{
			CGroundCombat* pDriversGroundCombatThought = (CGroundCombat* ) pDriverBrain->GetCurThoughtPtr();
			u32 uGUID = 0;
			if (pDriversGroundCombatThought->m_pEnemy )
			{
				uGUID = pDriversGroundCombatThought->m_pEnemy->Guid();
				ai_AssignGoal_Attack(pVehicleBrain, uGUID, 0);
			}
			else if( pVehicleBrain->GetAttackRuleSet() == CAIBrain::ATTACKRULESET_RAT0 )
			{
				// Rat vehicles always attack
				ai_AssignGoal_Attack(pVehicleBrain, 0, 0);
			}
//			ai_AssignJob_Wander(pVehicleBrain, 0.0f,FALSE, 0,0,0,0,0,NULL);
		}

		//take on the race of the driver
		pVehicleBrain->SetRace(pDriverBrain->GetRace());
	}
	return TRUE;
}


BOOL aibrainman_ConfigurePostNPCVehicleBrain(CAIBrain* pBrain)
{
	//No jobs or goals.
	//No perception
	ai_TurnOffPerceptor(pBrain, AI_PERCEPTOR_EYES);
	ai_TurnOffPerceptor(pBrain, AI_PERCEPTOR_EARS);
	ai_TurnOffPerceptor(pBrain, AI_PERCEPTOR_TOUCH);	//don't turn touch off, since NPC's are (cheating a bit) using player's brain to record total damage per sec 
	ai_TurnOffPerceptor(pBrain, AI_PERCEPTOR_RADIO);
	ai_IgnorePerceptor(pBrain, AI_PERCEPTOR_EYES);
	ai_IgnorePerceptor(pBrain, AI_PERCEPTOR_EARS);
	ai_IgnorePerceptor(pBrain, AI_PERCEPTOR_TOUCH);
	ai_IgnorePerceptor(pBrain, AI_PERCEPTOR_RADIO);
	aibrainman_Activate(pBrain);	//re-activate it, so that brainman does  call work on it
	pBrain->ForceFailureOnAllThoughtsExceptJob();
	pBrain->SetRace(AIRACE_AMBIENT);
	return TRUE;
}

// some code that could be used to turn player's original bot's eyes and ears on
// and manage his scanner rays 
BOOL aibrainman_PlayerBotBrainWork(CAIBrain* pBrain, u32 uPlayerId)
{

	if (pBrain && !pBrain->GetFlag_Active())
	{
		FASSERT(pBrain->GetAIMover()->GetEntity()->IsInWorld());
		//be sure to only turn these on when you need them.  
		//then, turn them off right away, so that they won't ever get left on 
		pBrain->ForceHearingLODOnThisFrame();
		ai_TurnOnPerceptor(pBrain, AI_PERCEPTOR_EYES);
		ai_TurnOnPerceptor(pBrain, AI_PERCEPTOR_EARS);
		ai_TurnOnPerceptor(pBrain, AI_PERCEPTOR_TOUCH);
		ai_TurnOnPerceptor(pBrain, AI_PERCEPTOR_RADIO);
		pBrain->DoPerceptionWork();
		pBrain->DoKnowledgeWork();
		pBrain->DoUpdateMovementTrackingWork();
		if (pBrain->GetFormation())
		{
			pBrain->GetFormation()->Work();
		}

		ai_TurnOffPerceptor(pBrain, AI_PERCEPTOR_EYES);
		ai_TurnOffPerceptor(pBrain, AI_PERCEPTOR_EARS);
	//	ai_TurnOffPerceptor(pBrain, AI_PERCEPTOR_TOUCH);
		ai_TurnOffPerceptor(pBrain, AI_PERCEPTOR_RADIO);
		pBrain->ClearHearingLOD();
		
		pBrain->GetAIMover()->BeginFrame();
		if (pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOT && 
			((CBot*) pBrain->GetAIMover()->GetEntity())->GetCurMech() && 
			!((CBot*) pBrain->GetAIMover()->GetEntity())->GetCurMech()->IsAutoWorkEnabled() && 
			!(((CBot*) pBrain->GetAIMover()->GetEntity())->GetCurMech()->AIBrain() && 
				((CBot*) pBrain->GetAIMover()->GetEntity())->GetCurMech()->AIBrain()->GetFlag_DisableEntityWork()))
		{
			((CBot*) pBrain->GetAIMover()->GetEntity())->GetCurMech()->Work();
		}

		//
		// Player emits a rage sound incase any other friendlies are in the area
		//
/*		CAIMemoryLarge* pDamageByMem = NULL;
		if (pBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_DAMAGED_BY, (CAIMemorySmall**) &pDamageByMem) &&
			pDamageByMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY)
		{
			AIEnviro_AddSound(pBrain->GetLoc(),
								40.0f,					  //40 feet of range
								5.0f,
								AISOUNDTYPE_RAGE,
								0,
								pDamageByMem->m_pEntity);
		}
  */

		return TRUE;
	}
	return FALSE;
		
}


ThoughtBank_AccessFunc* aithought_ThoughtClassTypeToAccessFunc(u32 uAIThoughtClassType)
{
	FASSERT(uAIThoughtClassType > 0 && uAIThoughtClassType < NUM_THOUGHTCLASSES);
	return _ThoughtClassSpec[uAIThoughtClassType].pAccessFunc;
}


void aibrainman_IterateActiveBrains(ActiveBrainCBFunc* pCBFunc, void* pData)
{
	CNiIterator<CAIBrain*> it = _pActiveBrains->Begin();
	while (it.IsValid())
	{
		CAIBrain* pBrain = it.Get();
		if (pBrain->IsLODActive() &&
			!((pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOT) && 
			(((CBot*) pBrain->GetAIMover()->GetEntity())->IsDeadOrDying() || !((CBot*) pBrain->GetAIMover()->GetEntity())->Power_IsPoweredUp())))
		{
			if (pCBFunc(pBrain, pData) == AIBRAINMAN_STOP_ITERATION)		//If CB returns TRUE, it means ABORT!
			{
				return;
			}
		}
		it.Next();
	}
}


void aibrainman_CountActiveBrains(void)
{
}


BOOL _StopAttacking(CAIBrain* pBrain, void* pData)
{
	if (pBrain->GetCurThought() == CAIBrain::TT_COMBAT)
	{
		FASSERT(pBrain->GetCurThoughtPtr());
		pBrain->GetCurThoughtPtr()->m_uThoughtFlags |= CAIThought::THOUGHTFLAG_GOAL_FAILED;
	}
	return AIBRAINMAN_CONTINUE_ITERATION;
}


//restore all AI to their Jobs
void aibrainman_StopAttacking(void)
{
	aibrainman_IterateActiveBrains(_StopAttacking, NULL);
}


void aibrainman_NotifyMechEject(CAIBrain* pBrain)	//this brain got ejected from its mech
{
	if (pBrain)
	{
		if (pBrain->GetMechLock())
		{
			FASSERT(ResLock_HasMechSeatLocked(pBrain->GetGUID()));
			ResLock_FreeMechSeatLock(pBrain->GetMechLock(), 0, pBrain->GetGUID());
			FASSERT(!ResLock_HasMechSeatLocked((u32) pBrain->GetGUID()));
			pBrain->ClearMechLock();
		}
		else
		{
			FASSERT(!ResLock_HasMechSeatLocked(pBrain->GetGUID()));
		}
	}
}


void aibrainman_SetLODOverrideForOneWork(CAIBrain* pBrain) 
{
	if (pBrain) 
	{
		pBrain->SetLODOverrideForOneWork();
	}
}


void aibrainman_SetDisableEntityWorkFlag(CAIBrain* pBrain)
{
	if (pBrain) 
	{
		pBrain->SetFlag_DisableEntityWork();
	}
}


void aibrainman_ClearDisableEntityWorkFlag(CAIBrain* pBrain)
{
	if (pBrain) 
	{
		pBrain->ClearFlag_DisableEntityWork();
	}
}


BOOL aibrainman_GetDisableEntityWorkFlag(CAIBrain* pBrain)
{
	if (pBrain) 
	{
		return pBrain->GetFlag_DisableEntityWork();
	}
	return FALSE;
}


