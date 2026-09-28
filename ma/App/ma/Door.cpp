//////////////////////////////////////////////////////////////////////////////////////
// Door.cpp - Door object.
//
// Author: Justin Link
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
// Date		Who			Description
// --------	----------	--------------------------------------------------------------
// 04/22/02	Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "Door.h"
#include "floop.h"
#include "bot.h"
#include "meshtypes.h"
#include "FPointPath2.h"
#include "FV3OLine1.h"
#include "FScriptSystem.h"
#include "fclib.h"
#include "Fsound.h"
#include "gstring.h"
#include "fsndfx.h"
#include "fvis.h"
#include "FCheckPoint.h"
#include "gameloop.h"
#include "AI\AIGameutils.h" //debug render stuff
#include "AI\AIBrain.h"
#include "AI\AIMover.h"
#include "fdraw.h"

const cchar* _kpszDamLiftDamageProfile = "RocketL1";//"DamLift"
const cchar* _kpszDamLiftSoundGroup = "DamLiftSound";

// Default sounds
#define _DOOR_OPEN	"SOM_MOpen"
#define _DOOR_LATCH "SOM_MLatc"
#define _DOOR_LOOP	"SOM_MLoop"
#define _DOOR_CLOSE "SOM_MClos"
#define _DOOR_SLAM	"SOM_MSlam"
#define _LIFT_START	"SOM_MLiIn"
#define _LIFT_STOP	"SOM_MLiSp"
#define _LIFT_LOOP	"SOM_MLiLp"

FCLASS_ALIGN_PREFIX class CEDoorBuilder : public CMeshEntityBuilder
{
public:
	FINLINE CEDoorBuilder() {}

	virtual void SetDefaults(u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType);

	// Ways the door as a whole can move.
	cchar *m_apszPathType[2];
	cchar *m_apszPathData[2];
	CFVec3A m_vecDoorMove;
	f32 m_fRotateY, m_fRotateZ;

	// Direction the bones move.
	CFVec3A m_avecBoneTrans[CDoorEntity::NUM_DOORBONES];

	// Time for the door to change states each way.
	f32 m_afDoorMoveTime[CDoorEntity::NUM_DOORBONES];

	// Various flags.
	BOOL m_bUseAccelMap;
	BOOL m_bIgnoreSteam;
	BOOL m_bStartLocked;
	BOOL m_bAIAccess;
	BOOL m_bPlayerAccess;
	BOOL m_bStartOpen;
	BOOL m_bIsPortal;
	u32 m_uBehaviorCtrlFlags;

	//
	f32 m_fAutoCloseDelay;

	// Events to trigger.
	cchar *m_apszEventName[2];

	// Enum'd stuff.
	CDoorEntity::UserType_e m_eUserType;
	CDoorEntity::BoneMoveType_e m_eBoneMoveType;
	CDoorEntity::DoorMoveType_e m_eDoorMoveType;
	CDoorEntity::DoorBehavior_e m_eBehavior;

	f32 m_afActiveRadius[2];

	// Info for AI.
	cchar *m_apszAISwitchName[2];

	// Used for BOTH a lift and a door
	cchar *m_pszDoorOpenStart;
	cchar *m_pszDoorOpenStop;
	cchar *m_pszDoorOpenLoop;

	cchar *m_pszDoorCloseStart;
	cchar *m_pszDoorCloseStop;
	cchar *m_pszDoorCloseLoop;

protected:
	virtual BOOL InterpretTable();
	virtual BOOL PostInterpretFixup();

	FCLASS_STACKMEM_ALIGN(CEDoorBuilder);
} FCLASS_ALIGN_SUFFIX;





#define _DOOR_RADIUS_AUDIO_SCALE ( 4.0f )

// =============================================================================================================
const f32 CDoorEntity::m_afDefaultActiveRadius[2] = { 15.0f, 20.0f };
const cchar *_apszBoneNames[2] = { "door1", "door2" };

// =============================================================================================================

BOOL CDoorEntity::m_bSystemInitialized = FALSE;
u32 CDoorEntity::s_uAccessFlags;
u32 CDoorEntity::s_uFrontBackFilter;
CFVec3A CDoorEntity::s_vecDoorZ, CDoorEntity::s_vecDoorPos;
CDoorEntity *CDoorEntity::m_pDoorListHead;
static CEDoorBuilder _EDoorBuilder;
CDoorEntity *s_pCollLift = NULL;
CBot* s_pCollLiftBlocker = NULL;
CFCollInfo s_CollLiftInfo;
// =============================================================================================================

BOOL CDoorEntity::InitSystem()
{
	FASSERT(!m_bSystemInitialized);

	m_bSystemInitialized = TRUE;

	m_pDoorListHead = NULL;

	return(TRUE);
}

// =============================================================================================================

void CDoorEntity::UninitSystem()
{
	if(m_bSystemInitialized)
	{
		m_pDoorListHead = NULL;
		m_bSystemInitialized = FALSE;
	}
}

// =============================================================================================================

BOOL CDoorEntity::InitLevel()
{
	// Do *NOT* set m_pDoorListHead to NULL because the DoorEntities for this level have already been created and
	//   added to the list.

	s_CollLiftInfo.nCollTestType			= FMESH_COLLTESTTYPE_SPHERE;
	s_CollLiftInfo.nStopOnFirstOfCollMask   = FCOLL_MASK_NONE;
	s_CollLiftInfo.bFindClosestImpactOnly	= FALSE;
	s_CollLiftInfo.nCollMask				= FCOLL_MASK_COLLIDE_WITH_NPCS | FCOLL_MASK_COLLIDE_WITH_PLAYER;
	s_CollLiftInfo.nResultsLOD				= FCOLL_LOD_HIGHEST;
	s_CollLiftInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
	s_CollLiftInfo.pSphere_WS				= NULL;
	s_CollLiftInfo.pTag						= NULL;

	return(TRUE);
}

// =============================================================================================================
void CDoorEntity::UninitLevel()
{
	m_pDoorListHead = NULL;
}

// =============================================================================================================

BOOL CDoorEntity::AI_IsOpen(const CFVec3A& FromWhere)
{
	if (m_eUserType == USERTYPE_DOOR)
	{
		return ((m_eBehavior == DOOR_BEHAVIOR_DOOR) && !IsLocked()) || (m_eState == DOORSTATE_ONE);	  //DOOR_BEHAVIOR_DOOR will always open, so might as well consider them open
	}
	else if (m_eUserType == USERTYPE_LIFT)
	{
		if (!m_bUnoccupiedLastFrame)
		{
		   return FALSE;   //one person lift.  Don't ever consider it open if it isn't unoccupied
		}

		f32 fDist1 = FromWhere.DistSq(m_avecAIPts[0]);
		f32 fDist2 = FromWhere.DistSq(m_avecAIPts[1]);
		if (fDist1 < fDist2)
		{  //closer to ai point 0 then aipoint 1
			return (m_eState == DOORSTATE_ZERO);
		}
		else
		{
			return (m_eState == DOORSTATE_ONE);
		}
	}
	return FALSE;
}

// =============================================================================================================

BOOL CDoorEntity::AI_IsOpening(const CFVec3A& FromWhere)
{
	if (m_eUserType == USERTYPE_DOOR)
	{
		return (m_eState == DOORSTATE_ONE || m_eState == DOORSTATE_ZEROTOONE);
	}
	else if (m_eUserType == USERTYPE_LIFT)
	{
		f32 fDist1 = FromWhere.DistSq(m_avecAIPts[0]);
		f32 fDist2 = FromWhere.DistSq(m_avecAIPts[1]);
		if (fDist1 < fDist2)
		{  //closer to ai point 0 then aipoint 1
			return (m_eState == DOORSTATE_ZERO || m_eState == DOORSTATE_ONETOZERO);
		}
		else
		{
			return (m_eState == DOORSTATE_ONE || m_eState == DOORSTATE_ZEROTOONE);
		}
	}
	return FALSE;
}

// =============================================================================================================

f32 CDoorEntity::AI_GetUnitTargetPos(const CFVec3A &vecPos)
{
	CFVec3A vecTemp;
	f32 afDist[2];

	FASSERT(m_eUserType==USERTYPE_LIFT);

	//////////////////////////////////////////////////////////////////////
	// Compute the distances to the two AI points.
	u32 uAIPtIdx;
	for(uAIPtIdx = 0; uAIPtIdx < 2; ++uAIPtIdx)
	{
		vecTemp = vecPos;
		vecTemp.Sub(m_avecAIPts[uAIPtIdx]);
		afDist[uAIPtIdx] = vecTemp.MagSq();
	}
	//
	//////////////////////////////////////////////////////////////////////

	if(afDist[0] <= afDist[1])
	{
		return(0.0f);
	}

	return(1.0f);
}

// =============================================================================================================

CDoorEntity *CDoorEntity::AI_FindWithNearestPad(const CFVec3A &vecPos, UserType_e eUserType)
{
	CDoorEntity *pClosest = NULL;
	f32 fClosestDistSq = 0.0f, fNewDistSq;
	CFVec3A vecDiff;

	CDoorEntity *pCurDoor = m_pDoorListHead;

	while(pCurDoor != NULL)
	{
		if(pCurDoor->m_eUserType == eUserType)
		{
			//////////////////////////////////////////////////////////////////////
			// Check each AI pt of this door.
			u32 uAIPtIdx;
			for(uAIPtIdx = 0; uAIPtIdx < pCurDoor->m_uNumAIPts; ++uAIPtIdx)
			{
				vecDiff = vecPos;
				vecDiff.Sub(pCurDoor->m_avecAIPts[uAIPtIdx]);
				fNewDistSq = vecDiff.MagSq();

				if((pClosest == NULL) || (fNewDistSq < fClosestDistSq))
				{
					pClosest = pCurDoor;
					fClosestDistSq = vecDiff.MagSq();
				}
			}
			//
			//////////////////////////////////////////////////////////////////////
		}

		pCurDoor = pCurDoor->m_pDoorListNext;
	}

	return(pClosest);
}

void CDoorEntity::AI_HoldAtEndLiftFor(f32 fSecs)
{
	m_eCurGotoReason = GOTOREASON_PICKUP;
	m_fPickupCntDn = fSecs;
}
	 
// =============================================================================================================
/*
void CDoorEntity::AI_Wait(f32 fWaitTime)
{
	m_fWa
}
*/
// =============================================================================================================

CDoorEntity::CDoorEntity()
{
	// Clear all pointers to NULL...
	Clear();
}

// =============================================================================================================

CDoorEntity::~CDoorEntity()
{
	if(IsSystemInitialized() && IsCreated())
	{
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld(TRUE);
		ClassHierarchyDestroy();
	}
}

// =============================================================================================================

BOOL CDoorEntity::Create()
{
	FASSERT_NOW;
	FASSERT(m_bSystemInitialized);
	FASSERT(!IsCreated());

	// Get pointer to the leaf class's builder object...
	CEDoorBuilder *pBuilder = (CEDoorBuilder *)(GetLeafClassBuilder());

	// If we're the leaf class, set the builder object' defaults...
	if(pBuilder == &_EDoorBuilder)
	{
		pBuilder->SetDefaults(0, 0, "door");
	}

	Clear();

	/////////////////////////////////////////////////////////////
	// Set up the door parameters.
	// TODO: This should be dealing with the builder object.
		m_anMeshIdx[0] = 0;
		m_anMeshIdx[1] = 0;

		m_eBehavior = DOOR_BEHAVIOR_DOOR;

		m_avecBoneTrans[0].Set(-5.0f, 0.0f, 0.0f);
		m_avecBoneTrans[1].Set(5.0f, 0.0f, 0.0f);

		m_afPosTime[0] = 0.5f;
		m_afPosTime[1] = 0.5f;

		m_afUnitPosRate[0] = (1.0f / m_afPosTime[0]);
		m_afUnitPosRate[1] = (-1.0f / m_afPosTime[1]);

		m_eBoneMoveType = MOVETYPE_BONETRANS;
		m_bUseAccelMap = TRUE;

		m_uAccessFlags = (DOOR_ACCESS_PLAYER | DOOR_ACCESS_NPC);

		m_apV3OPath[0] = NULL;
		m_apV3OPath[1] = NULL;
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// Set up the mesh entity.
		const char *pszMeshNames[2] = { "gowrdoor001", "" };
		if(!CMeshEntity::Create(1, pszMeshNames, 0, NULL, "JustinTest", NULL))
		{
			return(FALSE);
		}

		// TODO: Change this to a switch.
		if(m_eBoneMoveType == MOVETYPE_BONETRANS)
		{
			// Needs to be done after subclass creation.
			m_fClosedRadius = GetBoundingSphere_WS().m_fRadius;
			m_fRadiusDelta = FMATH_MAX(m_avecBoneTrans[0].Mag(), m_avecBoneTrans[1].Mag());
		}

		// Needs to be done after sub class creation,
		FindSteamBoneIndices();

		// These should get set via the builder.
		SetLockState(FALSE);
//		SetCollisionFlag(TRUE);
		SetTargetable(FALSE);
		SnapToPos(0);
		EnableAutoWork(TRUE);

	m_fPickupCntDn		= 0.0f;
		//
	/////////////////////////////////////////////////////////////

	return(TRUE);
}

// =============================================================================================================

void CDoorEntity::ClassHierarchyDestroy()
{
	// These are set to NULL in Clear() below.
	fdelete(m_apV3OPath[0]);
	fdelete(m_apV3OPath[1]);
	fdelete(m_pManMtx);
	fdelete(m_pRestMtx);
	fdelete(m_pAnimCombiner);

	StopDoorLoopSounds();

	Clear();

	CMeshEntity::ClassHierarchyDestroy();
}

// =============================================================================================================

BOOL CDoorEntity::ClassHierarchyBuild()
{
	FASSERT(m_bSystemInitialized);
	FASSERT(!IsCreated());
	FASSERT(FWorld_pWorld);

	// Get a frame.
	FResFrame_t hResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object.
	CEDoorBuilder *pBuilder = (CEDoorBuilder *)(GetLeafClassBuilder());

	// Build our parent entity class.
	if(!CMeshEntity::ClassHierarchyBuild())
	{
		// Parent class could not be built.
		goto _ExitWithError;
	}

	// Set any defaults in our class.
	_SetDefaults();

	/////////////////////////////////////////////////////////////
	// Initialize our entity from our builder object.
	// If the builder has a path, fnew it. (x2)
	// If the builder refers to a mesh, get it. (x2)
		/////////////////////////////////////////////////////////////
		//  Set up things that AI will need.
		m_eUserType = pBuilder->m_eUserType;
		switch(m_eUserType)
		{
			case USERTYPE_DOOR:
			{
				m_uNumAIPts = 0;
				m_avecAIPts[m_uNumAIPts++].Set(m_MtxToWorld.m_vPos);
				FASSERT(m_uNumAIPts == 1);

				break;
			}
			case USERTYPE_LIFT:
			{
				m_uNumAIPts = 0;
				m_avecAIPts[m_uNumAIPts++].Set(m_MtxToWorld.m_vPos);
				m_avecAIPts[m_uNumAIPts].Set(m_MtxToWorld.m_vPos);
				m_avecAIPts[m_uNumAIPts++].Add(pBuilder->m_vecDoorMove);

				FASSERT(m_uNumAIPts == 2);
				break;
			}
		}
		//
		/////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////
		// Set up the movement type, possibly overriding if there is
		//   no available animation.
		m_eBoneMoveType = pBuilder->m_eBoneMoveType;
		if(pBuilder->m_eBoneMoveType == MOVETYPE_ANIMATION)
		{
			if((UserAnim_GetCount() == 0) || (UserAnim_GetCurrentInst() == NULL))
			{
				m_eBoneMoveType = MOVETYPE_NONE;
			}
		}
		//
		/////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////
		// Set up the unit pos variables.
		if(pBuilder->m_afDoorMoveTime[0] == 0.0f)
		{
			// The user did not specify a time in the user properties.
			switch(m_eBoneMoveType)
			{
				case MOVETYPE_BONETRANS:
				{
					// Bone movement time defaults to 0.5s.
					m_afPosTime[0] = 0.5f;

					break;
				}
				case MOVETYPE_ANIMATION:
				{
					// Animation time defaults to the length of the animation.
					UserAnim_Select(0);
					m_afPosTime[0] = UserAnim_GetCurrentInst()->GetTotalTime();
					m_afAnimSpeedMult[0] = 1.0f;

					break;
				}
				case MOVETYPE_NONE:
				{
					m_afPosTime[0] = 0.5f;
					break;
				}
				default:
				{
					FASSERT_NOW;
				}
			}
		}
		else
		{
			m_afPosTime[0] = pBuilder->m_afDoorMoveTime[0];

			// There was a time specified, let's use it.
			switch(m_eBoneMoveType)
			{
				case MOVETYPE_BONETRANS:
				{
					// Bone movement has nothing extra special to do here.

					break;
				}
				case MOVETYPE_ANIMATION:
				{
					// Calculate the rate at which we need to progress through the animation relative
					//   to the total time we want it to take.
					m_afAnimSpeedMult[0] = UserAnim_GetCurrentInst()->GetTotalTime() / m_afPosTime[0];

					break;
				}
				case MOVETYPE_NONE:
				{
					break;
				}
				default:
				{
					FASSERT_NOW;
				}
			}
		}
		m_afUnitPosRate[0] = 1.0f / m_afPosTime[0];
		if(pBuilder->m_afDoorMoveTime[1] == 0.0f)
		{
			// They didn't specify anything for the second time, so we just copy
			//   the first time.
			m_afPosTime[1] = m_afPosTime[0];
			m_afUnitPosRate[1] = -m_afUnitPosRate[0];
			m_afAnimSpeedMult[1] = m_afAnimSpeedMult[0];
		}
		else
		{
			// They did specify something for the second time.
			m_afPosTime[1] = pBuilder->m_afDoorMoveTime[1];
			m_afUnitPosRate[1] = -1.0f / m_afPosTime[1];

			switch(m_eBoneMoveType)
			{
				case MOVETYPE_BONETRANS:
				{
					// Bone movement has nothing extra special to do here.

					break;
				}
				case MOVETYPE_ANIMATION:
				{
					// Calculate the rate at which we need to progress through the animation relative
					//   to the total time we want it to take.
					m_afAnimSpeedMult[1] = UserAnim_GetCurrentInst()->GetTotalTime() / m_afPosTime[1];

					break;
				}
				case MOVETYPE_NONE:
				{
					break;
				}
				default:
				{
					FASSERT_NOW;
				}
			}
		}
		//
		/////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////
		// Check to see what type of bone movement is requested, and
		//   initialize accordingly.
		switch(m_eBoneMoveType)
		{
			case MOVETYPE_NONE:
			{
				break;
			}
			case MOVETYPE_BONETRANS:
			{
				m_avecBoneTrans[0] = pBuilder->m_avecBoneTrans[0];
				m_avecBoneTrans[1] = pBuilder->m_avecBoneTrans[1];

				// Create a manual matrix animation object.
				m_pManMtx = fnew CFAnimManMtx;
				if(m_pManMtx == NULL)
				{
					DEVPRINTF("DoorEntity::ClassHierarchyBuild() : Could not create CFAnimManMtx.\n");
					goto _ExitWithError;
				}
				if(!m_pManMtx->Create(2, _apszBoneNames))
				{
					DEVPRINTF("DoorEntity::ClassHierarchyBuild() : Could not Create() CFAnimManMtx.\n");
					goto _ExitWithError;
				}

				// Create the mesh rest animation object.
				m_pRestMtx = fnew CFAnimMeshRest;
				if(m_pRestMtx == NULL)
				{
					DEVPRINTF("DoorEntity::ClassHierarchyBuild() : Could not create CFAnimMeshRest.\n");
					goto _ExitWithError;
				}
				if(!m_pRestMtx->Create(GetMeshInst()->m_pMesh))
				{
					DEVPRINTF("DoorEntity::ClassHierarchyBuild() : Could not Create() CFAnimMeshRest.\n");
					goto _ExitWithError;
				}

				// Create an animation combiner object.
				m_pAnimCombiner = fnew CFAnimCombiner;
				if(m_pAnimCombiner == NULL)
				{
					DEVPRINTF("DoorEntity::ClassHierarchyBuild() : Could not create CFAnimCombiner.\n");
					goto _ExitWithError;
				}
				m_pAnimCombiner->CreateSummer(GetMeshInst());
				m_pAnimCombiner->AttachToTap(0, m_pManMtx);
				m_pAnimCombiner->AttachToTap(1, m_pRestMtx);

				DriveMeshWithAnim(TRUE);
				UserAnim_Select(-1);
				SetExternalCombiner(0, m_pAnimCombiner);

				// Calculate what we need to do to adjust the bounding sphere.
				m_fClosedRadius = GetBoundingSphere_WS().m_fRadius;
				m_fRadiusDelta = FMATH_MAX(m_avecBoneTrans[0].Mag(), m_avecBoneTrans[1].Mag());

				break;
			}
			case MOVETYPE_ANIMATION:
			{
				break;
			}
		}
		//
		/////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////
		// Check to see what kind of door movement is requested, and
		//   initialize accordingly.
		m_eDoorMoveType = pBuilder->m_eDoorMoveType;
		switch(m_eDoorMoveType)
		{
			case DOORMOVETYPE_NONE:
			{
				break;
			}
			case DOORMOVETYPE_LINE:
			{
				m_vecDoorMove = pBuilder->m_vecDoorMove;
				m_fRotateY = pBuilder->m_fRotateY;
				m_fRotateZ = pBuilder->m_fRotateZ;

				break;
			}
			case DOORMOVETYPE_VECOBJ:
			{
				break;
			}
		}
		m_mtxDoorInitPos = GetMeshInst()->m_Xfm.m_MtxF;
		//
		/////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////
		// Get the mesh index array set up.
		FASSERT(GetMeshCount() > 0);
		m_anMeshIdx[0] = 0;

		FASSERT(GetMeshCount() < 2);
		if(GetMeshCount() == 2)
		{
			// Special closed mesh.
			m_anMeshIdx[1] = 1;
		}
		else
		{
			// Same open and closed meshes.
			m_anMeshIdx[1] = 0;
		}
		//
		/////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////
		// Get various flags set up.
		m_bUseAccelMap = pBuilder->m_bUseAccelMap;
		m_bIgnoreSteam = pBuilder->m_bIgnoreSteam;

		if(!m_bIgnoreSteam)
		{
			FindSteamBoneIndices();
		}

		m_bIsPortal = pBuilder->m_bIsPortal;

		if(m_bIsPortal)
		{
			f32 fDistToPortal;

			CFVec3A vecPos;
			vecPos.Set(GetBoundingSphere_WS().m_Pos);
			m_pPortal = fvis_GetNearestPortal(&vecPos, &fDistToPortal);
			f32 fBoundSphereRad = GetBoundingSphere_WS().m_fRadius;
			if(fDistToPortal > fBoundSphereRad * 2.f)
			{
				DEVPRINTF("CDoorEntity::ClassHierarchyBuild() : Door '%s' flagged as portal but closest portal is too far (%f feet).\n", Name(), fDistToPortal);
				m_pPortal = NULL;
				m_bIsPortal = FALSE;
			}
			else
			{
				GetMeshInst()->SetTraverseClosedPortalsFlag( TRUE );
			}
		}
		//
		/////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////
		// Get the access parameters set up.
		m_uAccessFlags = DOOR_ACCESS_NONE;
		if(pBuilder->m_bPlayerAccess)
		{
			m_uAccessFlags |= DOOR_ACCESS_PLAYER;
		}
		else
		{
			m_uAccessFlags &= ~DOOR_ACCESS_PLAYER;
		}

		if(pBuilder->m_bAIAccess)
		{
			m_uAccessFlags |= DOOR_ACCESS_NPC;
		}
		else
		{
			m_uAccessFlags &= ~DOOR_ACCESS_NPC;
		}
		//
		/////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////
		// Get the brain set up.
		m_eBehavior = pBuilder->m_eBehavior;

		m_uBehaviorCtrlFlags = pBuilder->m_uBehaviorCtrlFlags;
		m_fAutoCloseDelay = pBuilder->m_fAutoCloseDelay;
		//
		/////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////
		// Get the special events set up.
		m_anEvent[0] = CFScriptSystem::GetEventNumFromName(pBuilder->m_apszEventName[0]);
		m_anEvent[1] = CFScriptSystem::GetEventNumFromName(pBuilder->m_apszEventName[1]);
		//
		/////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////
		// Get the radii for the spheres set up properly.
		u32 uCurRadIdx;
		for(uCurRadIdx = 0; uCurRadIdx < 2; ++uCurRadIdx)
		{
			if(pBuilder->m_afActiveRadius[uCurRadIdx] == 0.0f)
			{
				m_afActiveRadius[uCurRadIdx] = m_afDefaultActiveRadius[uCurRadIdx];
			}
			else
			{
				m_afActiveRadius[uCurRadIdx] = pBuilder->m_afActiveRadius[uCurRadIdx];
			}
		}
		//
		/////////////////////////////////////////////////////////////

		// These should get set via the builder.
		SetLockState(pBuilder->m_bStartLocked);
		m_bStartOpen = pBuilder->m_bStartOpen;

//		SetCollisionFlag(TRUE);
		SetTargetable(FALSE);
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// Add ourselves to the master door list (so that we can be found by
	//   AI queries).
	m_pDoorListNext = m_pDoorListHead;
	m_pDoorListHead = this;
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// Record the name of the switch that was referenced (for AI).
	{
		for (s32 i = 0; i < 2; i++)
		{
			m_apAISwitch[i] = (CEntity *)(pBuilder->m_apszAISwitchName[i]);
		}
	}
	//
	/////////////////////////////////////////////////////////////
	// Lifts and doors both use this set
	if( pBuilder->m_pszDoorOpenStart && fclib_stricmp( pBuilder->m_pszDoorOpenStart, "none" ) ) {
		m_hSoundOpenStart = fsndfx_GetFxHandle( pBuilder->m_pszDoorOpenStart );

		if ( !m_hSoundOpenStart ) {
			DEVPRINTF( "CDoorEntity::ClassHierarchyBuild(): Could not find sound '%s'.\n", pBuilder->m_pszDoorOpenStart );
		}
	}

	if( pBuilder->m_pszDoorOpenStop && fclib_stricmp( pBuilder->m_pszDoorOpenStop, "none" ) ) {
		m_hSoundOpenStop = fsndfx_GetFxHandle( pBuilder->m_pszDoorOpenStop );

		if ( !m_hSoundOpenStop ) {
			DEVPRINTF( "CDoorEntity::ClassHierarchyBuild(): Could not find sound '%s'.\n", pBuilder->m_pszDoorOpenStop );
		}
	}

	if( pBuilder->m_pszDoorOpenLoop && fclib_stricmp( pBuilder->m_pszDoorOpenLoop, "none" ) ) {
		m_hSoundOpenLoop = fsndfx_GetFxHandle( pBuilder->m_pszDoorOpenLoop );

		if ( !m_hSoundOpenLoop ) {
			DEVPRINTF( "CDoorEntity::ClassHierarchyBuild(): Could not find sound '%s'.\n", pBuilder->m_pszDoorOpenLoop );
		}
	}
	
	// Only doors use this set of sounds
	if (m_eUserType == CDoorEntity::USERTYPE_DOOR) {
		if( pBuilder->m_pszDoorCloseStart && fclib_stricmp( pBuilder->m_pszDoorCloseStart, "none" ) ) {
			m_hSoundCloseStart = fsndfx_GetFxHandle( pBuilder->m_pszDoorCloseStart );

			if ( !m_hSoundCloseStart ) {
				DEVPRINTF( "CDoorEntity::ClassHierarchyBuild(): Could not find sound '%s'.\n", pBuilder->m_pszDoorCloseStart );
			}
		}

		if( pBuilder->m_pszDoorCloseStop && fclib_stricmp( pBuilder->m_pszDoorCloseStop, "none" ) ) {
			m_hSoundCloseStop = fsndfx_GetFxHandle( pBuilder->m_pszDoorCloseStop );

			if ( !m_hSoundCloseStop ) {
				DEVPRINTF( "CDoorEntity::ClassHierarchyBuild(): Could not find sound '%s'.\n", pBuilder->m_pszDoorCloseStop );
			}
		}

		if( pBuilder->m_pszDoorCloseLoop && fclib_stricmp( pBuilder->m_pszDoorCloseLoop, "none" ) ) {
			m_hSoundCloseLoop = fsndfx_GetFxHandle( pBuilder->m_pszDoorCloseLoop );

			if ( !m_hSoundCloseLoop ) {
				DEVPRINTF( "CDoorEntity::ClassHierarchyBuild(): Could not find sound '%s'.\n", pBuilder->m_pszDoorCloseLoop );
			}
		}
	}

	m_bUnoccupiedLastFrame = TRUE;

	return(TRUE);

_ExitWithError:
	Destroy();
	fres_ReleaseFrame(hResFrame);
	return(FALSE);
}

// =============================================================================================================

BOOL CDoorEntity::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CMeshEntity::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	EnableOurWorkBit();
//	SnapToPos(0);
	SnapToPos(m_bStartOpen);

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

// =============================================================================================================

CEntityBuilder *CDoorEntity::GetLeafClassBuilder()
{
	return(&_EDoorBuilder);
}

// =============================================================================================================

void CDoorEntity::ClassHierarchyResolveEntityPointerFixups()
{
	FASSERT(IsCreated());

	CMeshEntity::ClassHierarchyResolveEntityPointerFixups();

	for (s32 i = 0; i < 2; i++)
	{
		if(m_apAISwitch[i] != NULL)
		{
			m_apAISwitch[i] = CEntity::Find((cchar *)(m_apAISwitch[i]));
		}
	}

	// Check path data, if any path data is referenced, get a hold of the data entities for the paths.
}

// =============================================================================================================
BOOL bDoorWorkEnabled = TRUE;
void CDoorEntity::ClassHierarchyWork()
{
	// Note: don't check the vis flags if drawing is not enabled, because we
	// may be skipping through a cut scene that requires a door to operate
	CFWorldMesh *pWM = GetMeshInst();
	if( !bDoorWorkEnabled || (Gameloop_bDrawEnabled && !(pWM->GetVolumeFlags() & FVIS_VOLUME_IN_ACTIVE_LIST)) )
	{
		// We're not part of any active volume, let's not do any work.
		return;
	}

	CMeshEntity::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}
	
	if (m_eUserType == USERTYPE_LIFT &&
		(m_eState == DOORSTATE_ZERO || m_eState == DOORSTATE_ONE) && 
		m_fPickupCntDn > 0.0f)
	{
		m_fPickupCntDn -= FLoop_fPreviousLoopSecs;

		if(m_fPickupCntDn < 0.0f)
		{
			m_fPickupCntDn = 0.0f;
			m_eCurGotoReason = GOTOREASON_UNKNOWN;
			m_bUnoccupiedLastFrame = TRUE; //re-evaluate who is on me
		}
	}

	//
	// Doors can decide to change states for various reasons
	//
	StateTransitions();

	//
	// move bones, update animations etc
	//
	StateWork();

	if( m_pSoundOpenLoopEmitter ) {
		m_pSoundOpenLoopEmitter->SetPosition( &MtxToWorld()->m_vPos );
	}

	// Only doors can close
	if( m_eUserType == CDoorEntity::USERTYPE_DOOR && m_pSoundCloseLoopEmitter ) {
		m_pSoundCloseLoopEmitter->SetPosition( &MtxToWorld()->m_vPos );
	}
}

// =============================================================================================================
// =============================================================================================================
// =============================================================================================================
BOOL CDoorEntity::ForceGotoPos(u32 uNewPos, GotoReason_e eGotoReason)
{
	// NKM - If we try to force to the state we are already at, don't do it.
	// What in the world does the return value mean?
	if( m_eState == uNewPos ) {
		return FALSE; 
	}

	switch(uNewPos)
	{
		case 0:
		{
			// We have to calculate a new m_fUnitTimePos if they are using the acceleration map.
			if((m_bUseAccelMap) && (m_eState != DOORSTATE_ONETOZERO))
			{
				// JUSTIN: Are these parameters backward?
//				ConvertUnitPos(0, 1);
			}
			if(m_eBoneMoveType == MOVETYPE_ANIMATION)
			{
				UserAnim_SetSpeedMult(-m_afAnimSpeedMult[1]);
				UserAnim_Pause(FALSE);
				FASSERT(!UserAnim_IsPaused());
			}
			m_fTimeOpen = 0.0f;

			if (m_eState == DOORSTATE_ONE)
			{
				// Lift starting to move from an up state
				if (m_eUserType == CDoorEntity::USERTYPE_LIFT) {
					// Play the opening / starting sound for the lift
					fsndfx_Play3D( m_hSoundOpenStart, &MtxToWorld()->m_vPos, GetBoundingSphere_WS().m_fRadius * _DOOR_RADIUS_AUDIO_SCALE );

					StopDoorLoopSounds();

					// Start the loop sound if we have one
					if( m_hSoundOpenLoop ) {
						m_pSoundOpenLoopEmitter = FSNDFX_ALLOCNPLAY3D( 
							m_hSoundOpenLoop, 
							&MtxToWorld()->m_vPos, 
							GetBoundingSphere_WS().m_fRadius * 5.f, 1.0f, 1.0f, FAudio_EmitterDefaultPriorityLevel, TRUE );
					}
				} else {
					// Door is starting to close
					// Play the closing sound for the door
					fsndfx_Play3D( m_hSoundCloseStart, &MtxToWorld()->m_vPos, GetBoundingSphere_WS().m_fRadius * _DOOR_RADIUS_AUDIO_SCALE );

					StopDoorLoopSounds();

					// Start the loop sound if we have one
					if( m_hSoundCloseLoop ) {
						// Start the loop sound
						m_pSoundCloseLoopEmitter = FSNDFX_ALLOCNPLAY3D( 
							m_hSoundCloseLoop, 
							&MtxToWorld()->m_vPos, 
							GetBoundingSphere_WS().m_fRadius * 5.f, 1.0f, 1.0f, FAudio_EmitterDefaultPriorityLevel, TRUE );
					}
				}
			}

			m_eState = DOORSTATE_ONETOZERO;

			break;
		}
		case 1:
		{
			if((m_bUseAccelMap)  && (m_eState != DOORSTATE_ZEROTOONE))
			{
				// JUSTIN: Are these parameters backward?
				// We have to calculate a new m_fUnitTimePos if they are using the acceleration map.
	//			ConvertUnitPos(1, 0);
			}
			if(m_eBoneMoveType == MOVETYPE_ANIMATION)
			{
				UserAnim_SetSpeedMult(m_afAnimSpeedMult[0]);
				UserAnim_Pause(FALSE);
				FASSERT(!UserAnim_IsPaused());
			}

			if(m_eState == DOORSTATE_ZERO)
			{
				if(m_bIsPortal)
				{
					FASSERT(m_pPortal != NULL);
					m_pPortal->SetOpenState(TRUE);
				}
				
				// Either the lift is starting or the door is opening
				// Play the opening / starting sound
				fsndfx_Play3D( m_hSoundOpenStart, &MtxToWorld()->m_vPos, GetBoundingSphere_WS().m_fRadius * _DOOR_RADIUS_AUDIO_SCALE );

				StopDoorLoopSounds();

				// Start the loop sound if we have one
				if( m_hSoundOpenLoop ) {
					m_pSoundOpenLoopEmitter = FSNDFX_ALLOCNPLAY3D( 
						m_hSoundOpenLoop, 
						&MtxToWorld()->m_vPos, 
						GetBoundingSphere_WS().m_fRadius * 5.f, 1.0f, 1.0f, FAudio_EmitterDefaultPriorityLevel, TRUE );
				}

				SpawnSteam();
			}


			m_eState = DOORSTATE_ZEROTOONE;

			break;
		}
	}

	m_eCurGotoReason = eGotoReason;
	if(eGotoReason == GOTOREASON_PICKUP)
	{
		m_fPickupCntDn = 1.0f;	   //removed this since AI bots will always set it when they need
	}
	return TRUE;
}


BOOL CDoorEntity::GotoPos(u32 uNewPos, GotoReason_e eGotoReason)
{
	FASSERT(m_bSystemInitialized);
	FASSERT(uNewPos < 2);

	if((m_eState == DOORSTATE_ONETOZERO) || (m_eState == DOORSTATE_ZEROTOONE))
	{
		// We will ignore requests to move if we are already moving.
		return FALSE;	   //request denied.. whoever called this lift, had better keep calling if they really want it to come.
	}

//	if((m_eCurGotoReason == GOTOREASON_PICKUP) && (eGotoReason != GOTOREASON_DESTINATION) && (m_fPickupCntDn > 0.0f))
	if (m_eUserType == CDoorEntity::USERTYPE_LIFT)
	{
		if ((m_fPickupCntDn > 0.0f))
		{
			// We will ignore requests to move if we are currently still waiting to pick someone up.
			return FALSE;	   //request denied.. whoever called this lift, had better keep calling if they really want it to come.
		}
	}

	return ForceGotoPos(uNewPos, eGotoReason);
}

// =============================================================================================================
// updates position and orientation of a line-type door
BOOL CDoorEntity::_UpdateLineDoorPosition( BOOL bCheckCollision)
{
	BOOL bMoved = FALSE;
	CFMtx43A mtxMS2WS;
	mtxMS2WS = m_mtxDoorInitPos;

	BOOL bRelocate = FALSE;
	if(m_fRotateY != 0.0f)
	{
		CFMtx43A::m_RotY.SetRotationY(m_fRotateY * m_fUnitPosMapped);
		mtxMS2WS.Mul(CFMtx43A::m_RotY);
		bRelocate = TRUE;
	}

	if(m_fRotateZ != 0.0f)
	{
		CFMtx43A::m_RotZ.SetRotationZ(m_fRotateZ * m_fUnitPosMapped);
		mtxMS2WS.Mul(CFMtx43A::m_RotZ);
		bRelocate = TRUE;
	}

	if (!(m_vecDoorMove.x == 0.0f && m_vecDoorMove.y == 0.0f && m_vecDoorMove.z == 0.0f))
	{
		CFVec3A vecTemp = m_vecDoorMove;
		vecTemp.Mul(m_fUnitPosMapped);
		mtxMS2WS.m_vPos.Add(vecTemp);

		s_pCollLiftBlocker = NULL;
		if (bCheckCollision)
		{


			//do collision test here
			// Create a sphere to check for intersecting robots.

			// Add a dummy tracker to the world
			CFWorldUser UserTracker;
			UserTracker.SetTraverseClosedPortalsFlag( TRUE );
			UserTracker.MoveTracker(GetBoundingSphere_WS());
			s_pCollLift = this;
			BOOL bValidBotFound = !UserTracker.FindIntersectingTrackers(LiftPathBlockedCallback, FWORLD_TRACKERTYPE_MESH);

			UserTracker.RemoveFromWorld();
		}

		if (s_pCollLiftBlocker)
		{
		}
		else
		{
			bRelocate = TRUE;
		}


		s_pCollLiftBlocker = NULL;
		s_pCollLift = NULL;
	}

	if (bRelocate)
	{
		Relocate_RotXlatFromUnitMtx_WS(&mtxMS2WS);
		bMoved = TRUE;
	}

	return !bMoved;
}

// =============================================================================================================

void CDoorEntity::SnapToPos(u32 uNewPos)
{
	FASSERT(m_bSystemInitialized);
	FASSERT(uNewPos < 2);

	SelectMesh(m_anMeshIdx[uNewPos]);
	switch(uNewPos)
	{
		case DOORSTATE_ZERO:				// Going to the 'closed' state.
		{
			m_fUnitPos = 0.0f;
			m_fUnitPosMapped = 0.0f;
			if(m_eBoneMoveType == MOVETYPE_ANIMATION)
			{
				UserAnim_GetCurrentInst()->UpdateUnitTime(0.0f);
				UserAnim_Pause(TRUE);
			}
			else if( m_eDoorMoveType == DOORMOVETYPE_LINE )
			{
				_UpdateLineDoorPosition(NO_COLLISION_TEST);
			}

			if(m_bIsPortal)
			{
				FASSERT(m_pPortal != NULL);
				m_pPortal->SetOpenState(FALSE);
			}

			// Door close or lift stop
			if (m_eUserType == CDoorEntity::USERTYPE_DOOR) {
				// Larger radius so we can hear it
				fsndfx_Play3D( m_hSoundCloseStop, &MtxToWorld()->m_vPos, GetBoundingSphere_WS().m_fRadius * _DOOR_RADIUS_AUDIO_SCALE );
			
				StopDoorLoopSounds();
			} else {
				fsndfx_Play3D( m_hSoundOpenStop, &MtxToWorld()->m_vPos, GetBoundingSphere_WS().m_fRadius * _DOOR_RADIUS_AUDIO_SCALE );

				StopDoorLoopSounds();
			}

			m_eState = DOORSTATE_ZERO;

			break;
		}
		case DOORSTATE_ONE:				// Going to the 'open' state.
		{
			m_fUnitPos = 1.0f;
			m_fUnitPosMapped = 1.0f;
			m_fTimeOpen = 0.0f;
			if(m_bIsPortal)
			{
				FASSERT(m_pPortal != NULL);
				m_pPortal->SetOpenState(TRUE);
			}
			else if( m_eDoorMoveType == DOORMOVETYPE_LINE )
			{
				_UpdateLineDoorPosition(NO_COLLISION_TEST);
			}


			if(m_eBoneMoveType == MOVETYPE_ANIMATION)
			{
				UserAnim_GetCurrentInst()->UpdateUnitTime(1.0f);
				UserAnim_Pause(TRUE);
			}
			m_eState = DOORSTATE_ONE;

			// Door close or lift stop
			if (m_eUserType == CDoorEntity::USERTYPE_DOOR) {
				// Larger radius so we can hear it
				fsndfx_Play3D( m_hSoundOpenStop, &MtxToWorld()->m_vPos, GetBoundingSphere_WS().m_fRadius * _DOOR_RADIUS_AUDIO_SCALE );
			
				StopDoorLoopSounds();
			} else {
				fsndfx_Play3D( m_hSoundOpenStop, &MtxToWorld()->m_vPos, GetBoundingSphere_WS().m_fRadius * _DOOR_RADIUS_AUDIO_SCALE );
			
				StopDoorLoopSounds();
			}


			break;
		}
		default:
		{
			FASSERT_NOW;
		}
	}

	switch(m_eBoneMoveType)
	{
		case MOVETYPE_NONE:
		{
			break;
		}
		case MOVETYPE_BONETRANS:
		{
			DoBoneMovement();
			break;
		}
		case MOVETYPE_ANIMATION:
		{
			DoAnimMovement();
			break;
		}
		default:
		{
			FASSERT_NOW;
		}
	}
}

// =============================================================================================================

void CDoorEntity::SetLockState(BOOL bLocked)
{
	m_bLocked = bLocked;
}

// =============================================================================================================

f32 CDoorEntity::GetUnitPos()
{
	FASSERT(m_bSystemInitialized);

	return(m_fUnitPos);
}

// =============================================================================================================

f32 CDoorEntity::GetUnitPosMapped()
{
	FASSERT(m_bSystemInitialized);

	return(m_fUnitPosMapped);
}

// =============================================================================================================

// This should zero out everything and make this guy a boring lump of nothing.
void CDoorEntity::Clear()
{
	m_apV3OPath[0] = NULL;
	m_apV3OPath[1] = NULL;

	m_pManMtx = NULL;
	m_pRestMtx = NULL;
	m_pAnimCombiner = NULL;

	for (s32 i = 0; i < 2; i++)
	{
		m_apAISwitch[i] = NULL;
	}

	m_hSoundOpenStart = FSNDFX_INVALID_FX_HANDLE;
	m_hSoundOpenStop = FSNDFX_INVALID_FX_HANDLE;
	m_hSoundOpenLoop = FSNDFX_INVALID_FX_HANDLE;
	m_pSoundOpenLoopEmitter = NULL;

	m_hSoundCloseStart = FSNDFX_INVALID_FX_HANDLE;
	m_hSoundCloseStop = FSNDFX_INVALID_FX_HANDLE;
	m_hSoundCloseLoop = FSNDFX_INVALID_FX_HANDLE;
	m_pSoundCloseLoopEmitter = NULL;
}

// =============================================================================================================

void CDoorEntity::_SetDefaults()
{
	m_afActiveRadius[0] = m_afDefaultActiveRadius[0];
	m_afActiveRadius[1] = m_afDefaultActiveRadius[1];

	m_pPortal = NULL;
}

// =============================================================================================================

// 
void CDoorEntity::DoBoneMovement()
{
	FASSERT(m_bSystemInitialized);

	if((m_fUnitPos == 1.0f) && (m_anMeshIdx[1] == -1))
	{
		// We're in state 1, but it uses a special mesh for this state.
		// Hence, we don't bother trying to apply the translation to the
		//   bones.
		return;
	}

	CFVec3A vecTemp;
	CFMtx43A mtxTemp;

	/////////////////////////////////////////////////////////////
	// Get each bone of the door into the right position.
	u32 uCurBoneIdx;
	for(uCurBoneIdx = 0; uCurBoneIdx < 2; ++uCurBoneIdx)
	{
		// Calculate the displacement vector.
		vecTemp = m_avecBoneTrans[uCurBoneIdx];
		vecTemp.Mul(m_fUnitPosMapped);

		mtxTemp.Identity();
		mtxTemp.m_vPos = vecTemp;
		m_pManMtx->UpdateFrame(uCurBoneIdx, mtxTemp);
	}
	//
	/////////////////////////////////////////////////////////////

	CFMeshInst *pMI = GetMeshInst();
	pMI->m_BoundSphere_MS.m_fRadius = (m_fClosedRadius + m_fRadiusDelta * m_fUnitPosMapped) * pMI->m_Xfm.m_fScaleR;
	// JUSTIN: This line may not be necessary.
	Relocate_RotXlatScaleFromScaledMtx_WS(&pMI->m_Xfm.m_MtxF, pMI->m_Xfm.m_fScaleF);
}

// =============================================================================================================

// 
void CDoorEntity::DoAnimMovement()
{
	FASSERT(m_bSystemInitialized);
}


// =============================================================================================================

// uPosIdx = 0 -> rest pos, 1 -> other pos
// uFrontBackFilter = 1 -> front only, 2 -> back only, 0 -> both sides.
//BOOL CDoorEntity::_ValidBotNearby(u32 uPosIdx, f32 fRadius, u32 uFrontBackFilter)
BOOL CDoorEntity::_ValidBotNearby(CFVec3A &vecPos, f32 fRadius, u32 uFrontBackFilter)
{
	// Create a sphere to check for intersecting robots.
	CFSphere sphTest;
	sphTest.m_Pos = vecPos.v3;
	sphTest.m_fRadius = fRadius;

	// Add a dummy tracker to the world
	CFWorldUser UserTracker;
	UserTracker.SetTraverseClosedPortalsFlag( TRUE );
	UserTracker.MoveTracker(sphTest);

	// Set static parameters to be accessed by the callback function.
	s_uAccessFlags = m_uAccessFlags;
	s_uFrontBackFilter = uFrontBackFilter;
	s_vecDoorPos = MtxToWorld()->m_vPos;
	s_vecDoorZ = MtxToWorld()->m_vZ;

	BOOL bValidBotFound = !UserTracker.FindIntersectingTrackers(BotCheckCallback, FWORLD_TRACKERTYPE_MESH);

	UserTracker.RemoveFromWorld();

	return(bValidBotFound);
}

// =============================================================================================================

void CDoorEntity::StateTransitions()
{
	if(m_bLocked)
	{
		return;
	}

	/////////////////////////////////////////////////////////////
	switch(m_eBehavior)
	{
		case DOOR_BEHAVIOR_DOOR:
		{
			DoBehaviorDoor();
			break;
		}
		case DOOR_BEHAVIOR_OSCILLATE:
		{
			DoBehaviorOscillate();
			break;
		}
		case DOOR_BEHAVIOR_ELEVATOR:
		{
			DoBehaviorElevator();
			break;
		}
		case DOOR_BEHAVIOR_OTHER:
		{
			DoBehaviorOther();
			break;
		}
		default:
		{
			FASSERT_NOW;
		}
	}
}

// =============================================================================================================

void CDoorEntity::StateWork()
{
	
	//
	//	If moving, or animating, update the m_fUnitPos
	//	- still call, even if not, so that m_fUnitMapped always stays in synch
	//
	AdvanceUnitPos();

	/////////////////////////////////////////////////////////////
	// Work that does not get skipped when the door is locked.
	// ^ That doesn't really mean anything, ignore it.

	BOOL bPathBlocked = FALSE;
	switch(m_eState)
	{
		case DOORSTATE_ZEROTOONE:
		{
			switch(m_eDoorMoveType)
			{
				case DOORMOVETYPE_LINE:
				{
					bPathBlocked = _UpdateLineDoorPosition(
						(m_uBehaviorCtrlFlags & DOOR_BEHAVIOR_CTRL_NO_LIFT_BLOCK_CHECK) ? NO_COLLISION_TEST : DO_COLLISION_TEST);
					break;
				}
				case DOORMOVETYPE_VECOBJ:
				{
					//pgm: have no Idea what this code is. Don't actually think it ever gets executed
					FASSERT(m_apV3OPath[0] != NULL);
					CFMeshInst *pMI = GetMeshInst();
					CFMtx43A mtxMS2WS = pMI->m_Xfm.m_MtxF;
					mtxMS2WS.m_vPos = m_apV3OPath[0]->GetValue();
					Relocate_RotXlatScaleFromScaledMtx_WS(&mtxMS2WS, pMI->m_Xfm.m_fScaleF);

					break;
				}
			}
			break;
		}
		case DOORSTATE_ONETOZERO:
		{
			switch(m_eDoorMoveType)
			{
				case DOORMOVETYPE_LINE:
				{
					bPathBlocked = _UpdateLineDoorPosition(
						(m_uBehaviorCtrlFlags & DOOR_BEHAVIOR_CTRL_NO_LIFT_BLOCK_CHECK) ? NO_COLLISION_TEST : DO_COLLISION_TEST);
					break;
				}
				case DOORMOVETYPE_VECOBJ:
				{
					//pgm: have no Idea what this code is. Don't actually think it ever gets executed
					FASSERT(m_apV3OPath[1] != NULL);
					CFMeshInst *pMI = GetMeshInst();
					CFMtx43A mtxMS2WS = pMI->m_Xfm.m_MtxF;
					mtxMS2WS.m_vPos = m_apV3OPath[1]->GetValue();
					Relocate_RotXlatScaleFromScaledMtx_WS(&mtxMS2WS, pMI->m_Xfm.m_fScaleF);

					break;
				}
			}
			break;
		}
	}
	//
	/////////////////////////////////////////////////////////////

	if (bPathBlocked)
	{
		if (m_eState == DOORSTATE_ZEROTOONE)
		{
			AdvanceUnitPos(UNDO_LAST_ADVANCE);
			ForceGotoPos(DOORSTATE_ZERO, GOTOREASON_PATHBLOCKED);
		}
		else if (m_eState == DOORSTATE_ONETOZERO)
		{
			AdvanceUnitPos(UNDO_LAST_ADVANCE);
			ForceGotoPos(DOORSTATE_ONE, GOTOREASON_PATHBLOCKED);
		}
	}

	/////////////////////////////////////////////////////////////
	// Work that can get skipped if the door is locked.
	// ^ That doesn't really mean anything, ignore it.
	switch(m_eState)
	{
		case DOORSTATE_ZERO:
		{
			break;
		}
		case DOORSTATE_ONE:
		{
			break;
		}
		case DOORSTATE_ZEROTOONE:
		{
			StateMovement();
			break;
		}
		case DOORSTATE_ONETOZERO:
		{
			StateMovement();
			break;
		}
		default:
		{
			FASSERT_NOW;
		}
	}
	//
	/////////////////////////////////////////////////////////////
}

void CDoorEntity::AdvanceUnitPos(BOOL bUndoLastAdvance)
{
	f32 fDeltaTime = FLoop_fPreviousLoopSecs;
	if (bUndoLastAdvance)
	{
		fDeltaTime = -FLoop_fPreviousLoopSecs;
	}

	switch(m_eState)
	{
		case DOORSTATE_ZEROTOONE:
		{
			m_fUnitPos += (m_afUnitPosRate[0] * fDeltaTime);
			if(m_fUnitPos >= 1.0f)
			{
				SnapToPos(DOORSTATE_ONE);	//pgm:  SnapToPos does a bunch of crap, then sets m_eState to DOORSTATE_ONE
			}
			break;
		}
		case DOORSTATE_ONETOZERO:
		{
			m_fUnitPos += (m_afUnitPosRate[1] * fDeltaTime);
			if(m_fUnitPos <= 0.0f)
			{
				SnapToPos(DOORSTATE_ZERO);	//pgm: SnapToPos does a bunch of crap, then sets m_eState to DOORSTATE_ZERO

			}
			break;
		}
	}

	/////////////////////////////////////////////////////////////
	// At this point, m_fUnitPos has been determined for this frame.
	// We will now map it to a new unit position.
	if(m_bUseAccelMap)
	{
		switch(m_eState)
		{
			case DOORSTATE_ZEROTOONE:
			{
				m_fUnitPosMapped = fmath_UnitLinearToSCurve( m_fUnitPos );
				break;
			}
			case DOORSTATE_ONETOZERO:
			{
				m_fUnitPosMapped = fmath_UnitLinearToSCurve( m_fUnitPos );
				break;
			}
			case DOORSTATE_ZERO:
			{
				FASSERT(m_fUnitPos == 0.0f); //pgm added this out of curiousity
				m_fUnitPosMapped = 0.0f;
				break;
			}
			case DOORSTATE_ONE:
			{
				FASSERT(m_fUnitPos == 1.0f); //pgm added this out of curiousity
				m_fUnitPosMapped = 1.0f;
				break;
			}
			default:
			{
				FASSERT_NOW;
			}
		}
	}
	else
	{
		m_fUnitPosMapped = m_fUnitPos;
	}
}


// =============================================================================================================

// Transitions for when DoorBehaviorType is set to 'DOOR'.
void CDoorEntity::DoBehaviorDoor()
{
	switch(m_eState)
	{
		case DOORSTATE_ZERO:
		{
			if(_ValidBotNearby(MtxToWorld()->m_vPos, m_afActiveRadius[0], CHECK_FOR_BOTS_IN_FRONTANDBACK))
			{
				GotoPos(DOORSTATE_ONE, GOTOREASON_DESTINATION);
			}
			break;
		}
		case DOORSTATE_ONE:
		{
			if(!_ValidBotNearby(MtxToWorld()->m_vPos, m_afActiveRadius[0], CHECK_FOR_BOTS_IN_FRONTANDBACK))
			{
				GotoPos(DOORSTATE_ZERO, GOTOREASON_PICKUP);
			}
			break;
		}
		case DOORSTATE_ZEROTOONE:
		{
			break;
		}
		case DOORSTATE_ONETOZERO:
		{   //since we're an automatic door, make sure we start opening up even if we're closing down
			if(_ValidBotNearby(MtxToWorld()->m_vPos, m_afActiveRadius[0], CHECK_FOR_BOTS_IN_FRONTANDBACK))
			{
				ForceGotoPos(DOORSTATE_ONE, GOTOREASON_PATHBLOCKED);
			}
			break;
		}
		default:
		{
			break;
		}
	}
}

// =============================================================================================================

void CDoorEntity::DoBehaviorOscillate()
{
	switch(m_eState)
	{
		case DOORSTATE_ZERO:
		{
			GotoPos(DOORSTATE_ONE, GOTOREASON_DESTINATION);
			break;
		}
		case DOORSTATE_ONE:
		{
			GotoPos(DOORSTATE_ZERO, GOTOREASON_DESTINATION);
			break;
		}
		case DOORSTATE_ZEROTOONE:
		{
			break;
		}
		case DOORSTATE_ONETOZERO:
		{
			break;
		}
		default:
		{
			break;
		}
	}
}

// =============================================================================================================

void CDoorEntity::DoBehaviorOther()
{
	
	switch(m_eState)
	{
		case DOORSTATE_ZERO:
		{
			//pgm: are we a door that might need to open from the front due to a bot being there
			if (m_uBehaviorCtrlFlags & DOOR_BEHAVIOR_CTRL_OPENFRONT)
			{
				AutoOpenFront();
			}


			//pgm: are we a door that might need to open from the back due to a bot being there
			if (m_uBehaviorCtrlFlags & DOOR_BEHAVIOR_CTRL_OPENBACK)
			{
				AutoOpenBack();
			}
			break;
		}
		case DOORSTATE_ONE:
		{
			//pgm: are we an open door that might autoclose now?
			if (m_uBehaviorCtrlFlags & DOOR_BEHAVIOR_CTRL_AUTOCLOSE)
			{
				AutoClose();
			}
			break;
		}
		case DOORSTATE_ZEROTOONE:
		{
			break;
		}
		case DOORSTATE_ONETOZERO:
		{
			if ((m_eUserType == USERTYPE_DOOR) && (m_eBehavior == DOOR_BEHAVIOR_DOOR))
			{
				if(_ValidBotNearby(MtxToWorld()->m_vPos, m_afActiveRadius[0], CHECK_FOR_BOTS_IN_FRONTANDBACK))
				{
					ForceGotoPos(DOORSTATE_ONE, GOTOREASON_PATHBLOCKED);
				}
			}
			break;
		}
		default:
		{
			break;
		}
	}
}


// =============================================================================================================
void CDoorEntity::DoBehaviorElevator()
{
	switch(m_eState)
	{
		case DOORSTATE_ZERO:
		{
			if(m_bUnoccupiedLastFrame)
			{
				// Check to see if someone has entered our smaller region, and if so, move them to 1.
				if(_ValidBotNearby(m_avecAIPts[0], m_afActiveRadius[0], CHECK_FOR_BOTS_IN_FRONTANDBACK))
				{
					// Somebody entered our inner region.
					GotoPos(DOORSTATE_ONE, GOTOREASON_DESTINATION);
					m_bUnoccupiedLastFrame = FALSE;
					break;
				}
				// or, maybe someone has requested a pickup in the outter radius of the other end
				else if (!(m_uBehaviorCtrlFlags & DOOR_BEHAVIOR_CTRL_DISABLE_AUTO_CALL0) &&
						 _ValidBotNearby(m_avecAIPts[1], m_afActiveRadius[1], CHECK_FOR_BOTS_IN_FRONTANDBACK)) 
				{
					GotoPos(DOORSTATE_ONE, GOTOREASON_PICKUP);
					m_bUnoccupiedLastFrame = FALSE;
					break;
				}
			}
			else
			{
				if(!_ValidBotNearby(m_avecAIPts[0], m_afActiveRadius[0], CHECK_FOR_BOTS_IN_FRONTANDBACK))
				{
					m_bUnoccupiedLastFrame = TRUE;
				}
			}
			break;
		}
		case DOORSTATE_ONE:
		{
			if(m_bUnoccupiedLastFrame)
			{
				// Check to see if someone has entered our smaller region, and if so, move them to 1.
				if(_ValidBotNearby(m_avecAIPts[1], m_afActiveRadius[0], CHECK_FOR_BOTS_IN_FRONTANDBACK))
				{
					// Somebody entered our inner region.
					if (GotoPos(DOORSTATE_ZERO, GOTOREASON_DESTINATION))
					{
						m_bUnoccupiedLastFrame = FALSE;
					}
					break;
				}
				else if(!(m_uBehaviorCtrlFlags & DOOR_BEHAVIOR_CTRL_DISABLE_AUTO_CALL1) &&
						_ValidBotNearby(m_avecAIPts[0], m_afActiveRadius[1], CHECK_FOR_BOTS_IN_FRONTANDBACK))
				{
					if((m_eCurGotoReason != GOTOREASON_PICKUP) || (m_fPickupCntDn <= 0.0f))	  //lift related vbl
					{
						// Somebody needs to be picked up.
						if (GotoPos(DOORSTATE_ZERO, GOTOREASON_PICKUP))
						{
							m_bUnoccupiedLastFrame = FALSE;
						}
					}
					break;
				}
			}
			else
			{
				if(!_ValidBotNearby(m_avecAIPts[1], m_afActiveRadius[0], CHECK_FOR_BOTS_IN_FRONTANDBACK))
				{
					m_bUnoccupiedLastFrame = TRUE;
				}
			}
			break;
		}
		case DOORSTATE_ZEROTOONE:
		{
			break;
		}
		case DOORSTATE_ONETOZERO:
		{
			break;
		}
		default:
		{
			break;
		}
	}
}

// =============================================================================================================

void CDoorEntity::AutoClose()
{
	m_fTimeOpen += FLoop_fPreviousLoopSecs;

	switch(m_eState)
	{
		case DOORSTATE_ZERO:
		{
			break;
		}
		case DOORSTATE_ONE:
		{
			if(m_fTimeOpen >= m_fAutoCloseDelay)
			{
				// Don't close on any bot that might be there.
				if(!_ValidBotNearby(m_avecAIPts[1], m_afActiveRadius[0], CHECK_FOR_BOTS_IN_FRONTANDBACK))
				{
					GotoPos(0, GOTOREASON_DESTINATION);
				}
			}
			break;
		}
		case DOORSTATE_ZEROTOONE:
		{
			break;
		}
		case DOORSTATE_ONETOZERO:
		{
			break;
		}
		default:
		{
			break;
		}
	}
}

// =============================================================================================================

void CDoorEntity::AutoOpenFront()
{

	switch(m_eState)
	{
		case DOORSTATE_ZERO:
		{
			if(_ValidBotNearby(m_avecAIPts[0], m_afActiveRadius[0], CHECK_FOR_BOTS_IN_FRONT))
			{
				GotoPos(1, GOTOREASON_DESTINATION);
			}
			break;
		}
		case DOORSTATE_ONE:
		{
			break;
		}
		case DOORSTATE_ZEROTOONE:
		{
			break;
		}
		case DOORSTATE_ONETOZERO:
		{
			break;
		}
		default:
		{
			break;
		}
	}
}

// =============================================================================================================

void CDoorEntity::AutoOpenBack()
{
	switch(m_eState)
	{
		case DOORSTATE_ZERO:
		{
			if(_ValidBotNearby(m_avecAIPts[0], m_afActiveRadius[0], CHECK_FOR_BOTS_IN_BACK))
			{
				GotoPos(1, GOTOREASON_DESTINATION);
			}
			break;
		}
		case DOORSTATE_ONE:
		{
			break;
		}
		case DOORSTATE_ZEROTOONE:
		{
			break;
		}
		case DOORSTATE_ONETOZERO:
		{
			break;
		}
		default:
		{
			break;
		}
	}
}

// =============================================================================================================

void CDoorEntity::StateMovement()
{
	switch(m_eBoneMoveType)
	{
		case MOVETYPE_NONE:
		{
			break;
		}
		case MOVETYPE_BONETRANS:
		{
			DoBoneMovement();
			break;
		}
		case MOVETYPE_ANIMATION:
		{
			DoAnimMovement();
			break;
		}
		default:
		{
			FASSERT_NOW;
		}
	}
}


BOOL CDoorEntity::IsOccupied(void)
{
	return _ValidBotNearby(MtxToWorld()->m_vPos, m_afActiveRadius[0], CHECK_FOR_BOTS_IN_FRONTANDBACK);
}


// =============================================================================================================
void CDoorEntity::SpawnSteam()
{
	CFVec3A vecPosDir, vecNegDir, vecPos;
	u32 uCurBoneIdx;
	CFMeshInst *pMI = GetMeshInst();

	// If there's any steam work to be done...
	if(m_uNumSteamBones != 0)
	{
		// Let's get to doing it.
		vecPosDir = m_MtxToWorld.m_vRight;
		vecNegDir = vecPosDir;
		vecNegDir.Mul(-1.0f);

		// Once for each steam bone.
		for(uCurBoneIdx = 0; uCurBoneIdx < m_uNumSteamBones; ++uCurBoneIdx)
		{
			vecPos = pMI->m_pMesh->pBoneArray[m_anSteamBoneIdx[uCurBoneIdx]].AtRestBoneToParentMtx.m_vPos;
			m_MtxToWorld.MulPoint(vecPos);
//			fpart_SpawnParticleEmitter(FPART_TYPE_STEAM_PUFF, 0.6f, vecPos, &vecPosDir, FALSE);
//			fpart_SpawnParticleEmitter(FPART_TYPE_STEAM_PUFF, 0.6f, vecPos, &vecNegDir, FALSE);
		}
	}
}

// =============================================================================================================

void CDoorEntity::FindSteamBoneIndices()
{
	CFMeshInst *pMI;

	SelectMesh(0);
	pMI = GetMeshInst();

	m_uNumSteamBones = 0;

	char szBoneName[7] = "steamx";

	u8 uBoneNameIdx;
	for(uBoneNameIdx = 1; uBoneNameIdx <= 5; ++uBoneNameIdx)
	{
		szBoneName[5] = (char)(uBoneNameIdx) + '0';
		m_anSteamBoneIdx[m_uNumSteamBones] = pMI->FindBone(szBoneName);
		if(m_anSteamBoneIdx[m_uNumSteamBones] != -1)
		{
			++m_uNumSteamBones;
		}
	}
}

// =============================================================================================================

void CDoorEntity::ConvertUnitPos(u32 uOldState, u32 uNewState)
{
	m_fUnitPos = ConvertUnitPos(uOldState, uNewState, m_fUnitPos);
}

// =============================================================================================================

f32 CDoorEntity::ConvertUnitPos(u32 uOldState, u32 uNewState, f32 fOldUnitPos)
{
	FASSERT(uOldState < 2);
	FASSERT(uNewState < 2);

	if(uOldState == uNewState)
	{
		// For some reason they're not really converting anything.
		return(fOldUnitPos);
	}

	if((!m_bUseAccelMap) || (fOldUnitPos == 0.0f) || (fOldUnitPos == 1.0f))
	{
		return(fOldUnitPos);
//		return(1.0f - fOldUnitPos);
	}

	switch(uOldState)
	{
		case 0:
		{
			FASSERT(uNewState == 1);
			// This fancy mapping to a new unit pos is necessary if they are using the acceleration map.
			return(fmath_Sqrt(1.0f - (fOldUnitPos - 1.0f) * (fOldUnitPos - 1.0f)));

			break;
		}
		case 1:
		{
			FASSERT(uNewState == 0);
			return(1.0f - fmath_Sqrt((1.0f - fOldUnitPos * fOldUnitPos)));

			break;
		}
		default:
		{
			FASSERT_NOW;
		}
	}

	FASSERT_NOW;
	return(0.0f);		// This is just to supress warnings, it should be impossible to get here.
}

// =============================================================================================================

BOOL CDoorEntity::BotCheckCallback(CFWorldTracker *pTracker, FVisVolume_t *pVolume )
{
	FASSERT(m_bSystemInitialized);

	CFWorldMesh *pWorldMesh = (CFWorldMesh *)pTracker;
	CEntity *pEntity;
	CBot *pBot;

	// See if the world mesh that we found is an entity...
	if(pWorldMesh->m_nUser == MESHTYPES_ENTITY)
	{
		// It's an entity, let's check if it's a bot...
		pEntity = (CEntity *)(pWorldMesh->m_pUser);
		if((pEntity->TypeBits() & ENTITY_BIT_BOT) == 0)
		{
			// It's not a bot.
			return(TRUE);
		}

		pBot = (CBot *)(pEntity);
		if((pBot->m_nOwnerPlayerIndex != -1) && ((s_uAccessFlags & DOOR_ACCESS_PLAYER) == 0))
		{
			return(TRUE);
		}
		if((pBot->m_nOwnerPlayerIndex == -1) && ((s_uAccessFlags & DOOR_ACCESS_NPC) == 0))
		{
			return(TRUE);
		}
		if (pBot->IsSleeping() ||
			pBot->IsDeadOrDying() ||
			// Port: aibrainman_Create() may legitimately return NULL (see CEntity::Create()), so a
			// brainless NPC is treated like one that is not following a path.
			(!pBot->IsPlayerBot() && (!pBot->AIBrain() || !pBot->AIBrain()->GetAIMover()->IsFollowingPath())))
		{
			return(TRUE);
		}

		switch(s_uFrontBackFilter)
		{
			case 0:
			{
				break;
			}
			case 1:
			{
				// We should only respond if they're in front of the door.
				CFVec3A vecDoorMinusBot = pBot->MtxToWorld()->m_vPos;
				vecDoorMinusBot.Sub(s_vecDoorPos);
				vecDoorMinusBot.Unitize();
				f32 fDot = vecDoorMinusBot.Dot(s_vecDoorZ);

				if(fDot >= 0.0f)
				{
					// They're on the wrong side.
					return(TRUE);
				}

				break;
			}
			case 2:
			{
				// We should only respond if they're in back of the door.
				CFVec3A vecDoorMinusBot = pBot->MtxToWorld()->m_vPos;
				vecDoorMinusBot.Sub(s_vecDoorPos);
				vecDoorMinusBot.Unitize();
				f32 fDot = vecDoorMinusBot.Dot(s_vecDoorZ);

				if(fDot <= 0.0f)
				{
					// They're on the wrong side.
					return(TRUE);
				}

				break;
			}
			default:
			{
				FASSERT_NOW;
				break;
			}
		}

		return(FALSE);
	}

	return(TRUE);
//	return(FALSE);
}


BOOL CDoorEntity::LiftPathBlockedCallback(CFWorldTracker *pTracker, FVisVolume_t *pVolume )
{
	FASSERT(m_bSystemInitialized);

	CFWorldMesh *pWorldMesh = (CFWorldMesh *)pTracker;
	CEntity *pEntity;
	CBot *pBot;

	// See if the world mesh that we found is an entity...
	if(pWorldMesh->m_nUser == MESHTYPES_ENTITY)
	{
		// It's an entity, let's check if it's a bot...
		pEntity = (CEntity *)(pWorldMesh->m_pUser);
		if(!(pEntity->TypeBits() & ENTITY_BIT_BOT))
		{
			// It's not a bot.
			return(TRUE);
		}

		pBot = (CBot *)(pEntity);
		if(pBot->IsSleeping() || pBot->IsDeadOrDying())
		{
			return(TRUE);
		}

		if((pBot->m_nOwnerPlayerIndex != -1))
		{
			 //player bot
		}
		else 
		{
			//npc bot
		}

		//
		// Is that bot blocking my way
		//
		if (pBot->GetStickyEntity() != s_pCollLift)
		{
			CFVec3A LiftPos_WS = s_pCollLift->GetBoundingSphere_WS().m_Pos;
			CFVec3A DeltaToBot;
			DeltaToBot.Sub(pBot->MtxToWorld()->m_vPos, LiftPos_WS);

			//Going up, or down?
			CFVec3A AxisMoveDir = CFVec3A::m_Null;
			if (s_pCollLift->m_vecDoorMove.y > 0.0f)
			{
				AxisMoveDir.y = 1.0f;
			}
			else
			{
				AxisMoveDir.y = -1.0f;
			}

			if (s_pCollLift->m_eState == DOORSTATE_ONETOZERO)
			{
				AxisMoveDir.y *= -1.0f;
			}

			if (AxisMoveDir.y < 0.0f && DeltaToBot.Dot(AxisMoveDir) > 0.0f)
			{
				CFSphere BotBounds;
				BotBounds.m_fRadius = pBot->m_fCollCylinderHeight_WS*0.5f;
				BotBounds.m_Pos = pBot->MtxToWorld()->m_vPos.v3;
				BotBounds.m_Pos.y += BotBounds.m_fRadius;
				s_CollLiftInfo.pSphere_WS = &BotBounds;
				if (s_pCollLift->GetMeshInst()->CollideWithMeshTris(&s_CollLiftInfo))
				{
					s_pCollLiftBlocker = pBot;



					// Damage the bot
					CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

					if( pDamageForm ) {
						// Fill out the form...
						pDamageForm->m_nDamageLocale	= CDamageForm::DAMAGE_LOCALE_AMBIENT;
						pDamageForm->m_nDamageDelivery	= CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
						pDamageForm->m_pDamageProfile	= CDamage::FindDamageProfile(_kpszDamLiftDamageProfile);//"RocketL1");//"DamLift");
						pDamageForm->m_Damager.pWeapon	= NULL;
						pDamageForm->m_Damager.pBot		= NULL;
						pDamageForm->m_Damager.nDamagerPlayerIndex = -1;
						pDamageForm->m_pDamageeEntity	= pBot;
						
						CDamage::SubmitDamageForm( pDamageForm );
					}


					CFSoundGroup *pGroup = CFSoundGroup::RegisterGroup(_kpszDamLiftSoundGroup);
					if (pGroup)
					{
						CFSoundGroup::PlaySound(pGroup , FALSE, &(pBot->MtxToWorld()->m_vPos));
					}

					//in my way.
					return(FALSE);		//fals means fworld can stop iterating through all trackers
				}

			}
			
		}
	}
	return(TRUE);
}


void CDoorEntity::CheckpointSaveSelect( s32 nCheckpoint ) {
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}

// saves state for checkpoint
BOOL CDoorEntity::CheckpointSave( void )
{
	// save base class data
	CEntity::CheckpointSave();

	// save door class data.
	// order must match load order below
	CFCheckPoint::SaveData( m_fPickupCntDn );
	CFCheckPoint::SaveData( (u32&) m_eCurGotoReason );
	CFCheckPoint::SaveData( m_bUnoccupiedLastFrame );
	CFCheckPoint::SaveData( m_fTimeOpen );
	CFCheckPoint::SaveData( m_fUnitPos );
	CFCheckPoint::SaveData( m_fUnitPosMapped );
	CFCheckPoint::SaveData( (u32&) m_eState );
	CFCheckPoint::SaveData( m_bLocked );
	CFCheckPoint::SaveData( m_bStartOpen );

	return TRUE;
}

// loads state for checkpoint
void CDoorEntity::CheckpointRestore( void )
{
	ActionState_e ePreState = m_eState;

	// load base class data
	CEntity::CheckpointRestore();

	// load door class data.
	// order must match save order above
	CFCheckPoint::LoadData( m_fPickupCntDn );
	CFCheckPoint::LoadData( (u32&) m_eCurGotoReason );
	CFCheckPoint::LoadData( m_bUnoccupiedLastFrame );
	CFCheckPoint::LoadData( m_fTimeOpen );
	CFCheckPoint::LoadData( m_fUnitPos );
	CFCheckPoint::LoadData( m_fUnitPosMapped );
	CFCheckPoint::LoadData( (u32&) m_eState );
	CFCheckPoint::LoadData( m_bLocked );
	CFCheckPoint::LoadData( m_bStartOpen );

	if( ( m_eState == DOORSTATE_ZERO && ePreState != DOORSTATE_ZERO ) ||
		( m_eState == DOORSTATE_ONE && ePreState != DOORSTATE_ONE ) )
	{
		SnapToPos( m_eState );
	}
}

void CDoorEntity::StopDoorLoopSounds( void ) {
	if( m_pSoundOpenLoopEmitter ) {
		m_pSoundOpenLoopEmitter->Destroy();
		m_pSoundOpenLoopEmitter = NULL;
	}

	if( m_pSoundCloseLoopEmitter ) {
		m_pSoundCloseLoopEmitter->Destroy();
		m_pSoundCloseLoopEmitter = NULL;
	}
}

char* _apszUserTypes[] = 
{
	"USERTYPE_DOOR",
	"USERTYPE_LIFT",
};

char* _apszBehaviorTypes[] =
{
	"BEHAVIOR_DOOR",			// Door goes to DOORSTATE_ONE when a bot is
	"BEHAVIOR_OSCILLATE",		// Continually switches its position back
	"BEHAVIOR_ELEVATOR",		// If someone enters larger active region, go to
	"BEHAVIOR_OTHER",			// Doesn't take any action of its own
};

void CDoorEntity::DebugRender(void)
{
	CFVec3A ScreenPos;
	f32 fLineHeight = 0.02f;
	CFVec3A Origin;
	CFVec3A DrawThisPt;



	if (aiutils_WSToScreenPos(GetBoundingSphere_WS().m_Pos,  &ScreenPos))
	{
		if(GetMeshInst() && !(GetMeshInst()->GetVolumeFlags() & FVIS_VOLUME_IN_ACTIVE_LIST))
		{
			ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "D");
		}
		else
		{
			ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "~w1%s,%s", Name() ? Name():"NONAME", _apszUserTypes[m_eUserType]);
			ScreenPos.y += fLineHeight;
			ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "~w1%s", _apszBehaviorTypes[m_eBehavior]);
			ScreenPos.y += fLineHeight;

	//		fdraw_SolidLine( &(Origin.v3), &(DrawThisPt.v3), &aiutils_paDebugColors[uColor]);


			if (m_eUserType == USERTYPE_DOOR)
			{
				fdraw_FacetedWireSphere( &(MtxToWorld()->m_vPos.v3), m_afActiveRadius[ACTIVE_REGION_INNER], 1, 1, aiutils_paDebugColors+AICOLOR_RED);
			}
			else if (m_eUserType == USERTYPE_LIFT)
			{
				fdraw_FacetedWireSphere( &(m_avecAIPts[LIFTSTOP_ZERO].v3), m_afActiveRadius[ACTIVE_REGION_INNER], 1, 1, aiutils_paDebugColors+AICOLOR_WHITE);

				fdraw_FacetedWireSphere( &(m_avecAIPts[LIFTSTOP_ZERO].v3), m_afActiveRadius[ACTIVE_REGION_INNER], 5, 5, aiutils_paDebugColors+AICOLOR_RED);
				fdraw_FacetedWireSphere( &(m_avecAIPts[LIFTSTOP_ZERO].v3), m_afActiveRadius[ACTIVE_REGION_OUTER], 5, 5, aiutils_paDebugColors+AICOLOR_RED);

				fdraw_FacetedWireSphere( &(m_avecAIPts[LIFTSTOP_ONE].v3), m_afActiveRadius[ACTIVE_REGION_INNER], 5, 5, aiutils_paDebugColors+AICOLOR_BLUE);
				fdraw_FacetedWireSphere( &(m_avecAIPts[LIFTSTOP_ONE].v3), m_afActiveRadius[ACTIVE_REGION_OUTER], 5, 5, aiutils_paDebugColors+AICOLOR_BLUE);
			}
		}
	}
}

// =============================================================================================================
// =============================================================================================================
// =============================================================================================================

void CEDoorBuilder::SetDefaults(u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType)
{
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS(CMeshEntityBuilder, ENTITY_BIT_DOOR, pszEntityType);

	if(!fclib_stricmp(pszEntityType, "lift"))
	{
		m_apszPathType[0] = "line";
		m_apszPathType[1] = NULL;

		m_eUserType = CDoorEntity::USERTYPE_LIFT;
	}
	else
	{
		m_apszPathType[0] = NULL;
		m_apszPathType[1] = NULL;

		m_eUserType = CDoorEntity::USERTYPE_DOOR;
	}
	m_apszPathData[0] = NULL;
	m_apszPathData[1] = NULL;
	
	m_vecDoorMove.Set(0.0f, 0.0f, 0.0f);

	m_avecBoneTrans[0].Set(0.0f, 0.0f, 0.0f);
	m_avecBoneTrans[1].Set(0.0f, 0.0f, 0.0f);

	m_afDoorMoveTime[0] = 0.0f;
	m_afDoorMoveTime[1] = 0.0f;

	m_bUseAccelMap = TRUE;
	m_bIgnoreSteam = FALSE;
	m_bAIAccess = TRUE;
	m_bPlayerAccess = TRUE;
	m_bStartLocked = FALSE;
	m_bStartOpen = FALSE;

	m_apszEventName[0] = NULL;
	m_apszEventName[1] = NULL;

	m_eBoneMoveType = CDoorEntity::MOVETYPE_NONE;		//builder default
	m_eDoorMoveType = CDoorEntity::DOORMOVETYPE_NONE;  //builder default
	m_eBehavior = CDoorEntity::DOOR_BEHAVIOR_DOOR;	   //builder default

//	m_fActiveRadius = 0.0f;
	m_afActiveRadius[0] = 0.0f;
	m_afActiveRadius[1] = 0.0f;

	m_fRotateY = 0.0f;
	m_fRotateZ = 0.0f;

	m_uBehaviorCtrlFlags = 0;
	m_bIsPortal = FALSE;
	m_fAutoCloseDelay = 1.0f;

	for (s32 i = 0; i < 2; i++)
	{
		m_apszAISwitchName[i] = NULL;
	}

	if( m_eUserType == CDoorEntity::USERTYPE_DOOR ) {
		m_pszDoorOpenStart = _DOOR_OPEN;
		m_pszDoorOpenStop = _DOOR_LATCH;
		m_pszDoorOpenLoop = _DOOR_LOOP;
	} else {
		m_pszDoorOpenStart = _LIFT_START;	
		m_pszDoorOpenStop = _LIFT_STOP;
		m_pszDoorOpenLoop = _LIFT_LOOP;
	}

	m_pszDoorCloseStart = _DOOR_CLOSE;
	m_pszDoorCloseStop = _DOOR_SLAM;
	m_pszDoorCloseLoop = _DOOR_LOOP;
}

// =============================================================================================================

BOOL CEDoorBuilder::InterpretTable()
{
	FGameDataTableHandle_t hCurTable = CEntityParser::m_hTable;
	FGameData_VarType_e eVarType;

//	DEVPRINTF("*** Found table named '%s' for a door.\n", CEntityParser::m_pszTableName);
	if(fclib_stricmp(CEntityParser::m_pszTableName, "type") == 0)
	{
		cchar *pszValue;
		CEntityParser::Interpret_String(&pszValue);

		if(fclib_stricmp(pszValue, "door") == 0)
		{
			m_eUserType = CDoorEntity::USERTYPE_DOOR;
		}
		else if(fclib_stricmp(pszValue, "lift") == 0)
		{
			m_eUserType = CDoorEntity::USERTYPE_LIFT;
		}
		else
		{
			FASSERT_NOW;
			return(TRUE);
		}
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "door1move"))
	{
		if(fgamedata_GetNumFields(hCurTable) != 3)
		{
			DEVPRINTF("CEDoorBuilder::InterpretTable() : Wrong number of entries in 'door1move' table.\n");
			return(TRUE);
		}
		m_avecBoneTrans[CDoorEntity::DOORBONE_ZERO].x = *(const f32 *)fgamedata_GetPtrToFieldData(hCurTable, 0, eVarType);
		m_avecBoneTrans[CDoorEntity::DOORBONE_ZERO].z = *(const f32 *)fgamedata_GetPtrToFieldData(hCurTable, 1, eVarType);
		m_avecBoneTrans[CDoorEntity::DOORBONE_ZERO].y = *(const f32 *)fgamedata_GetPtrToFieldData(hCurTable, 2, eVarType);

		m_eBoneMoveType = CDoorEntity::MOVETYPE_BONETRANS;
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "door2move"))
	{
		if(fgamedata_GetNumFields(hCurTable) != 3)
		{
			DEVPRINTF("CEDoorBuilder::InterpretTable() : Wrong number of entries in 'door2move' table.\n");
			return(TRUE);
		}
		m_avecBoneTrans[CDoorEntity::DOORBONE_ONE].x = *(const f32 *)fgamedata_GetPtrToFieldData(hCurTable, 0, eVarType);
		m_avecBoneTrans[CDoorEntity::DOORBONE_ONE].z = *(const f32 *)fgamedata_GetPtrToFieldData(hCurTable, 1, eVarType);
		m_avecBoneTrans[CDoorEntity::DOORBONE_ONE].y = *(const f32 *)fgamedata_GetPtrToFieldData(hCurTable, 2, eVarType);

		m_eBoneMoveType = CDoorEntity::MOVETYPE_BONETRANS;
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "movetime1"))
	{
		CEntityParser::Interpret_F32(&m_afDoorMoveTime[CDoorEntity::DOORBONE_ZERO]);
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "movetime2"))
	{
		CEntityParser::Interpret_F32(&m_afDoorMoveTime[CDoorEntity::DOORBONE_ONE]);
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "anim"))
	{
		if(fgamedata_GetNumFields(hCurTable) > 1)
		{
			DEVPRINTF("CEDoorBuilder::InterpretTable() : Too many entries in 'anim' table.\n");
			return(TRUE);
		}
		m_eBoneMoveType = CDoorEntity::MOVETYPE_ANIMATION;
		// We can go ahead and let this fall through to CMeshEntity.
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "mesh"))
	{
		if(fgamedata_GetNumFields(hCurTable) > 2)
		{
			DEVPRINTF("CEDoorBuilder::InterpretTable() : Too many entries in 'mesh' table.\n");
			return(TRUE);
		}
		// We can go ahead and let this fall through to CMeshEntity.
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "useaccel"))
	{
		cchar *pszValue;
		CEntityParser::Interpret_String(&pszValue);
		if(!fclib_stricmp(pszValue, "true"))
		{
			m_bUseAccelMap = TRUE;
		}
		else if(!fclib_stricmp(pszValue, "false"))
		{
			m_bUseAccelMap = FALSE;
		}
		else
		{
			DEVPRINTF("CEDoorBuilder::InterpretTable() : Invalid value '%s' for 'useaccel' table.\n", pszValue);
			return(TRUE);
		}
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "ignoresteam"))
	{
		cchar *pszValue;
		CEntityParser::Interpret_String(&pszValue);
		if(!fclib_stricmp(pszValue, "true"))
		{
			m_bIgnoreSteam = TRUE;
		}
		else if(!fclib_stricmp(pszValue, "false"))
		{
			m_bIgnoreSteam = FALSE;
		}
		else
		{
			DEVPRINTF("CEDoorBuilder::InterpretTable() : Invalid value '%s' for 'ignoresteam' table.\n", pszValue);
			return(TRUE);
		}
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "aiaccess"))
	{
		cchar *pszValue;
		CEntityParser::Interpret_String(&pszValue);
		if(!fclib_stricmp(pszValue, "true"))
		{
			m_bAIAccess = TRUE;
		}
		else if(!fclib_stricmp(pszValue, "false"))
		{
			m_bAIAccess = FALSE;
		}
		else
		{
			DEVPRINTF("CEDoorBuilder::InterpretTable() : Invalid value '%s' for 'aiaccess' table.\n", pszValue);
			return(TRUE);
		}
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "playeraccess"))
	{
		cchar *pszValue;
		CEntityParser::Interpret_String(&pszValue);
		if(!fclib_stricmp(pszValue, "true"))
		{
			m_bPlayerAccess = TRUE;
		}
		else if(!fclib_stricmp(pszValue, "false"))
		{
			m_bPlayerAccess = FALSE;
		}
		else
		{
			DEVPRINTF("CEDoorBuilder::InterpretTable() : Invalid value '%s' for 'playeraccess' table.\n", pszValue);
			return(TRUE);
		}
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "pathtype1"))
	{
		cchar *pszValue;
		CEntityParser::Interpret_String(&pszValue);
		if(!fclib_stricmp(pszValue, "line"))
		{
			m_apszPathType[0] = pszValue;
		}
		else
		{
			DEVPRINTF("CEDoorBuilder::InterpretTable() : Unrecognized type '%s' for 'pathtype1' table.\n", pszValue);
			return(TRUE);
		}
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "pathdata1"))
	{
		CEntityParser::Interpret_String(&m_apszPathData[0]);
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "pathtype2"))
	{
		cchar *pszValue;
		CEntityParser::Interpret_String(&pszValue);
		if(!fclib_stricmp(pszValue, "line"))
		{
			m_apszPathType[0] = pszValue;
		}
		else
		{
			DEVPRINTF("CEDoorBuilder::InterpretTable() : Unrecognized type '%s' for 'pathtype2' table.\n", pszValue);
			return(TRUE);
		}
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "pathdata2"))
	{
		CEntityParser::Interpret_String(&m_apszPathData[1]);
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "eventopen"))
	{
		CEntityParser::Interpret_String(&m_apszEventName[0]);
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "eventclose"))
	{
		CEntityParser::Interpret_String(&m_apszEventName[1]);
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "behavtype") ||	// misspelling remains for backwards compatibility
			!fclib_stricmp(CEntityParser::m_pszTableName, "behavetype"))
	{
		cchar *pszValue;
		CEntityParser::Interpret_String(&pszValue);
		if(!fclib_stricmp(pszValue, "other"))
		{
			m_eBehavior = CDoorEntity::DOOR_BEHAVIOR_OTHER;
		}
		else if(!fclib_stricmp(pszValue, "door"))
		{
			m_eBehavior = CDoorEntity::DOOR_BEHAVIOR_DOOR;
		}
		else if(!fclib_stricmp(pszValue, "oscillate"))
		{
			m_eBehavior = CDoorEntity::DOOR_BEHAVIOR_OSCILLATE;
		}
		else if(!fclib_stricmp(pszValue, "elevator"))
		{
			m_eBehavior = CDoorEntity::DOOR_BEHAVIOR_ELEVATOR;
		}
		else
		{
			DEVPRINTF("CEDoorBuilder::InterpretTable() : Unrecognized value '%s' for 'behavetype' table.\n", pszValue);
			return(TRUE);
		}
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "displace"))
	{
		if(fgamedata_GetNumFields(hCurTable) != 3)
		{
			DEVPRINTF("CEDoorBuilder::InterpretTable() : Wrong number of entries in 'displace' table.\n");
			return(TRUE);
		}
		m_vecDoorMove.x = *(const f32 *)fgamedata_GetPtrToFieldData(hCurTable, 0, eVarType);
		m_vecDoorMove.z = *(const f32 *)fgamedata_GetPtrToFieldData(hCurTable, 1, eVarType);
		m_vecDoorMove.y = *(const f32 *)fgamedata_GetPtrToFieldData(hCurTable, 2, eVarType);

		m_eDoorMoveType = CDoorEntity::DOORMOVETYPE_LINE;	//userprop "displace" 
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "rotatey"))
	{
		// NOTE: Y and Z and swapped for the artists.
		CEntityParser::Interpret_F32(&m_fRotateZ);
		m_fRotateZ = FMATH_DEG2RAD(m_fRotateZ);

		m_eDoorMoveType = CDoorEntity::DOORMOVETYPE_LINE;	//userprop "rotatey" 
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "rotatez"))
	{
		// NOTE: Y and Z and swapped for the artists.	  
		CEntityParser::Interpret_F32(&m_fRotateY);
		m_fRotateY = FMATH_DEG2RAD(m_fRotateY);

		m_eDoorMoveType = CDoorEntity::DOORMOVETYPE_LINE;	 //userprop "rotatez" 
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "activeradius"))
	{
		CEntityParser::Interpret_F32(&m_afActiveRadius[0]);
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "activeradius2"))
	{
		CEntityParser::Interpret_F32(&m_afActiveRadius[1]);	 //this is for elevators. It is the "call" radius.
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "startlocked"))
	{
		cchar *pszValue;
		CEntityParser::Interpret_String(&pszValue);
		if(!fclib_stricmp(pszValue, "true"))
		{
			m_bStartLocked = TRUE;
		}
		else if(!fclib_stricmp(pszValue, "false"))
		{
			m_bStartLocked = FALSE;
		}
		else
		{
			DEVPRINTF("CEDoorBuilder::InterpretTable() : Invalid value '%s' for 'startlocked' table.\n", pszValue);
			return(TRUE);
		}
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "aiswitch1") || !fclib_stricmp(CEntityParser::m_pszTableName, "aiswitch"))
	{
		CEntityParser::Interpret_String(&(m_apszAISwitchName[0]));
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "aiswitch2"))
	{
		CEntityParser::Interpret_String(&(m_apszAISwitchName[1]));
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "autoclose"))
	{
		CEntityParser::Interpret_Flag(&m_uBehaviorCtrlFlags, CDoorEntity::DOOR_BEHAVIOR_CTRL_AUTOCLOSE);
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "autoclosedelay"))
	{
		m_uBehaviorCtrlFlags |= CDoorEntity::DOOR_BEHAVIOR_CTRL_AUTOCLOSE;
		CEntityParser::Interpret_F32(&m_fAutoCloseDelay);
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "autoopenfront"))
	{
		CEntityParser::Interpret_Flag(&m_uBehaviorCtrlFlags, CDoorEntity::DOOR_BEHAVIOR_CTRL_OPENFRONT);
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "autoopenback"))
	{
		CEntityParser::Interpret_Flag(&m_uBehaviorCtrlFlags, CDoorEntity::DOOR_BEHAVIOR_CTRL_OPENBACK);
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "disablecall0"))
	{
		CEntityParser::Interpret_Flag(&m_uBehaviorCtrlFlags, CDoorEntity::DOOR_BEHAVIOR_CTRL_DISABLE_AUTO_CALL0);
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "disablecall1"))
	{
		CEntityParser::Interpret_Flag(&m_uBehaviorCtrlFlags, CDoorEntity::DOOR_BEHAVIOR_CTRL_DISABLE_AUTO_CALL1);
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "noliftblockchecking"))
	{
		CEntityParser::Interpret_Flag(&m_uBehaviorCtrlFlags, CDoorEntity::DOOR_BEHAVIOR_CTRL_NO_LIFT_BLOCK_CHECK);
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "startopen"))
	{
		CEntityParser::Interpret_Flag((u32 *)(&m_bStartOpen), 1);
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "isportal"))
	{
		CEntityParser::Interpret_Flag((u32 *)(&m_bIsPortal), 1);
		return(TRUE);
	} else if( !fclib_stricmp(CEntityParser::m_pszTableName, "open_start" ) || 
			 !fclib_stricmp(CEntityParser::m_pszTableName, "lift_start" ) ) {
		// open_start and lift_start are the same thing, lift is used to make things clear
		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			CEntityParser::Interpret_String( &m_pszDoorOpenStart );
		}
		return TRUE;
	} else if( !fclib_stricmp(CEntityParser::m_pszTableName, "open_stop" ) || 
			 !fclib_stricmp(CEntityParser::m_pszTableName, "lift_stop" ) || 
			 !fclib_stricmp(CEntityParser::m_pszTableName, "open_close" )) {
		// !!Nate open_close is here because I sent out a bad e-mail where I said open_close was a valid command
		//        and it was placed in a few worlds.  
		// open_stop and lift_stop are the same thing, lift is used to make things clear
		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			CEntityParser::Interpret_String( &m_pszDoorOpenStop );
		}
		return TRUE;
	} else if( !fclib_stricmp(CEntityParser::m_pszTableName, "open_loop" ) || 
			 !fclib_stricmp(CEntityParser::m_pszTableName, "lift_loop" ) ) {
		// open_loop and lift_loop are the same thing, lift is used to make things clear
		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			CEntityParser::Interpret_String( &m_pszDoorOpenLoop );
		}
		return TRUE;
	} else if( !fclib_stricmp(CEntityParser::m_pszTableName, "close_start" ) ) {
		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			CEntityParser::Interpret_String( &m_pszDoorCloseStart );
		}
		return TRUE;
	} else if( !fclib_stricmp(CEntityParser::m_pszTableName, "close_stop" ) ) {
		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			CEntityParser::Interpret_String( &m_pszDoorCloseStop );
		}
		return TRUE;
	} else if( !fclib_stricmp(CEntityParser::m_pszTableName, "close_loop" ) ) {
		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			CEntityParser::Interpret_String( &m_pszDoorCloseLoop );
		}
		return TRUE;
	}

	return(CMeshEntityBuilder::InterpretTable());
}

// =============================================================================================================

BOOL CEDoorBuilder::PostInterpretFixup()
{
	s32 i;

	if(!CMeshEntityBuilder::PostInterpretFixup())
	{
		goto _ExitWithError;
	}

	for (i = 0; i < 2; i++)
	{
		if (m_apszAISwitchName[i] != NULL)
		{
			m_apszAISwitchName[i] = gstring_Main.AddString(m_apszAISwitchName[i]);
		}
	}

	return(TRUE);

_ExitWithError:
	return(FALSE);
}

// =============================================================================================================

// =============================================================================================================

// =============================================================================================================

// =============================================================================================================
