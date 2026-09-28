//////////////////////////////////////////////////////////////////////////////////////
// ESwitch.cpp - Switch object.
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
// 05/08/02	Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "ESwitch.h"
#include "floop.h"
#include "bot.h"
#include "meshtypes.h"
#include "FPointPath2.h"
#include "FV3OLine1.h"
#include "fresload.h"
#include "fclib.h"
#include "FScriptSystem.h"
#include "Door.h"
#include "gstring.h"
#include "ItemInst.h"
#include "fsndfx.h"
#include "TalkSystem2.h"
#include "AI/AiApi.h"
#include "AI/AIBTATable.h"
#include "BotTalkInst.h"




FCLASS_ALIGN_PREFIX class CESwitchBuilder : public CMeshEntityBuilder
{
public:
	FINLINE CESwitchBuilder() {}

	virtual void SetDefaults(u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType);

	cchar *m_apszDoorNames[4];
	CESwitch::DoorAction_e m_aeDoorActions[4];
	BOOL m_bTriggerEvent;
	BOOL m_bRequireChip;
	BOOL m_bMilOnly;

protected:
	virtual BOOL InterpretTable();
	virtual BOOL PostInterpretFixup();

	FCLASS_STACKMEM_ALIGN(CESwitchBuilder);
} FCLASS_ALIGN_SUFFIX;


extern BOOL AIBrain_TalkModeCB( u32 uTalkModeCBControl, void *pvData1, void *pvData2 );



// =============================================================================================================

BOOL CESwitch::m_bSystemInitialized = FALSE;
s32 CESwitch::m_nSwitchEvent = -1;
FSndFx_FxHandle_t CESwitch::m_hLockedSound;
FSndFx_FxHandle_t CESwitch::m_hSwitchTriggeredSound;

static CESwitchBuilder _ESwitchBuilder;

// =============================================================================================================

BOOL CESwitch::InitSystem()
{
	FASSERT(!m_bSystemInitialized);

	m_bSystemInitialized = TRUE;

	return(TRUE);
}

// =============================================================================================================

void CESwitch::UninitSystem()
{
	if(m_bSystemInitialized)
	{
		m_bSystemInitialized = FALSE;
	}
}

// =============================================================================================================

BOOL CESwitch::InitLevel()
{
	m_nSwitchEvent = CFScriptSystem::GetEventNumFromName("switch");

	if(m_nSwitchEvent == -1)
	{
		DEVPRINTF("CESwitch::InitLevel() : Could not find 'switch' event.\n");
	}

	m_hLockedSound = fsndfx_GetFxHandle( "Door Locked" );
	if ( !m_hLockedSound) {
		DEVPRINTF( "CSwitchEntity::InitLevel(): Could not find sound '%s'.\n", "Door Locked" );
	}

	m_hSwitchTriggeredSound = fsndfx_GetFxHandle( "Door Chip" );
	if ( !m_hSwitchTriggeredSound) {
		DEVPRINTF( "CSwitchEntity::InitLevel(): Could not find sound '%s'.\n", "Door Chip" );
	}

	return(TRUE);
}

// =============================================================================================================

void CESwitch::UninitLevel()
{
}

// =============================================================================================================

CESwitch::CESwitch()
{
	// ClearDoorPtrs all pointers to NULL...
	ClearDoorPtrs();
}

// =============================================================================================================

CESwitch::~CESwitch()
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

BOOL CESwitch::ActionNearby(CEntity *pEntity)
{
	CEntity::ActionNearby(pEntity);

	if (m_eCurState == SWITCHSTATE_ACTIVATING || m_eCurState == SWITCHSTATE_WAITINGFOROKAY)
	{
		// It's already being activated.  We don't need to do anything more.
		return(TRUE);
	}

	if((m_bRequireChip) && (!pEntity->TypeBits() & ENTITY_BIT_BOT))
	{
		// They're not a bot, and this requires a chip, so they're not allowed to pass.
		return(FALSE);
	}


	if( m_bRequireChip )
	{
		CBot *pBot = (CBot *)(pEntity);

		if( (pBot->m_pInventory == NULL) || (pBot->m_pInventory->m_aoItems[INVPOS_CHIP].m_nClipAmmo <= 0) )
		{
			MakeActivateSounds(FALSE);
			return(FALSE);
		}
		else
		{
			pBot->m_pInventory->m_aoItems[INVPOS_CHIP].m_nClipAmmo--;
		}
	}

	m_pInitEntity = pEntity;

	if ( m_pInitEntity->TypeBits() & ENTITY_BIT_BOT)
	{	
		_StartActivate();
	}
	else
	{
		_DoSwitchActions();
	}

	return(TRUE);
}

// =============================================================================================================

void CESwitch::ClassHierarchyDestroy()
{
	// These are set to NULL in ClearDoorPtrs() below.
	ClearDoorPtrs();

	CMeshEntity::ClassHierarchyDestroy();
}

// =============================================================================================================

BOOL CESwitch::ClassHierarchyBuild()
{
	FASSERT(m_bSystemInitialized);
	FASSERT(!IsCreated());
	FASSERT(FWorld_pWorld);

	// Get a frame.
	FResFrame_t hResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object.
	CESwitchBuilder *pBuilder = (CESwitchBuilder *)(GetLeafClassBuilder());

	// Build our parent entity class.
	if(!CMeshEntity::ClassHierarchyBuild())
	{
		// Parent class could not be built.
		goto _ExitWithError;
	}

	// Set any defaults in our class.
	_SetDefaults();

	/////////////////////////////////////////////////////////////
	// Load the animation for the button press.
	{
		FAnim_t *pAnim = (FAnim_t *)(fresload_Load(FANIM_RESNAME, "ARDGchip_01"));
		if(pAnim == NULL)
		{
			DEVPRINTF("CESwitch::ClassHierarchyBuild() : Could not load button press animation.\n");
		}
		else
		{
			m_pSwitchAnim = fnew CFAnimInst;
			if(m_pSwitchAnim == NULL)
			{
				DEVPRINTF("CESwitch::ClassHierarchyBuild() : Out of memory.\n");
				return(FALSE);
			}
			if(!m_pSwitchAnim->Create(pAnim))
			{
				DEVPRINTF("CESwitch::ClassHierarchyBuild() : Something went wrong creating CFAnimInst.\n");
				return(FALSE);
			}
		}
	}
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// Initialize our entity from our builder object.
	u32 uCurDoorIdx;
	for(uCurDoorIdx = 0; uCurDoorIdx < 4; ++uCurDoorIdx)
	{
		if(pBuilder->m_apszDoorNames[uCurDoorIdx] != NULL)
		{
			m_apDoor[uCurDoorIdx] = (CDoorEntity *)(pBuilder->m_apszDoorNames[uCurDoorIdx]);
			m_aeDoorActions[uCurDoorIdx] = pBuilder->m_aeDoorActions[uCurDoorIdx];
		}
	}

	m_bTriggerEvent = pBuilder->m_bTriggerEvent;
	m_bRequireChip = pBuilder->m_bRequireChip;
	m_bMilOnly = pBuilder->m_bMilOnly;

	//
	/////////////////////////////////////////////////////////////

	SetCollisionFlag(TRUE);
	SetTargetable(FALSE);
	SetActionable(TRUE);

	EnableAutoWork(FALSE);

	return(TRUE);

_ExitWithError:
	Destroy();
	fres_ReleaseFrame(hResFrame);
	return(FALSE);
}

// =============================================================================================================

BOOL CESwitch::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CMeshEntity::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	EnableOurWorkBit();

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

// =============================================================================================================

BOOL CESwitch::CanOpenDoor(CDoorEntity* pDoor)
{
	if (!IsLocked() && !m_bRequireChip)   //chips are like key to switches
	{
		for (s32 i = 0; i < 4; i++)
		{
			if (m_apDoor[i] == pDoor && (m_aeDoorActions[i] == DOORACTION_OPEN))
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}


// =============================================================================================================

CEntityBuilder *CESwitch::GetLeafClassBuilder()
{
	return(&_ESwitchBuilder);
}

// =============================================================================================================

void CESwitch::ClassHierarchyResolveEntityPointerFixups()
{
	FASSERT(IsCreated());

	CMeshEntity::ClassHierarchyResolveEntityPointerFixups();

	u32 uCurDoorIdx;
	for(uCurDoorIdx = 0; uCurDoorIdx < 4; ++uCurDoorIdx)
	{
		if(m_apDoor[uCurDoorIdx] != NULL)
		{
			m_apDoor[uCurDoorIdx] = (CDoorEntity *)(CEntity::Find((char *)(m_apDoor[uCurDoorIdx])));
			if(m_apDoor[uCurDoorIdx] == NULL)
			{
				DEVPRINTF("CESwitch::ClassHierarchyResolveEntityPointerFixups() : Unknown door is referenced by a switch.\n");
				continue;
			}
			FASSERT(m_apDoor[uCurDoorIdx]->TypeBits() & ENTITY_BIT_DOOR);
		}
	}
}

// =============================================================================================================

void CESwitch::ClassHierarchyWork()
{
	CMeshEntity::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}

	StateTransitions();
	StateWork();
}

// =============================================================================================================

// This should zero out everything and make this guy a boring lump of nothing.
void CESwitch::ClearDoorPtrs()
{
	u32 uCurDoorIdx;
	for(uCurDoorIdx = 0; uCurDoorIdx < 4; ++uCurDoorIdx)
	{
		m_apDoor[uCurDoorIdx] = NULL;
	}
}

// =============================================================================================================

void CESwitch::_SetDefaults()
{
	CESwitch::ClearDoorPtrs();

	m_bHasBeenActivated = FALSE;
	m_pInitEntity = NULL;
	m_eCurState = SWITCHSTATE_IDLE;
}

// =============================================================================================================

void CESwitch::_DoSwitchActions()
{
	CDoorEntity *pCurDE;

	u32 uCurDoorIdx;
	for(uCurDoorIdx = 0; uCurDoorIdx < 4; ++uCurDoorIdx)
	{
		pCurDE = m_apDoor[uCurDoorIdx];
		if(pCurDE != NULL)
		{
			switch(m_aeDoorActions[uCurDoorIdx])
			{
				case DOORACTION_OPEN:
				{
					pCurDE->GotoPos(1, CDoorEntity::GOTOREASON_PICKUP);
					break;
				}
				case DOORACTION_CLOSE:
				{
					pCurDE->GotoPos(0, CDoorEntity::GOTOREASON_PICKUP);
					break;
				}
				case DOORACTION_UNLOCK:
				{
					pCurDE->SetLockState(FALSE);
					break;
				}
				case DOORACTION_LOCK:
				{
					pCurDE->SetLockState(TRUE);
					break;
				}
				case DOORACTION_TOGGLE:
				{
					CDoorEntity::ActionState_e eDoorActionState = pCurDE->GetState();
					switch(eDoorActionState)
					{
						case CDoorEntity::DOORSTATE_ZERO:
						case CDoorEntity::DOORSTATE_ONETOZERO:
						{
							pCurDE->GotoPos(1, CDoorEntity::GOTOREASON_PICKUP);
							break;
						}
						case CDoorEntity::DOORSTATE_ONE:
						case CDoorEntity::DOORSTATE_ZEROTOONE:
						{
							pCurDE->GotoPos(0, CDoorEntity::GOTOREASON_PICKUP);
							break;
						}
					}
				}
			}
		}
	}
	
	if(m_bTriggerEvent)
	{
		CFScriptSystem::TriggerEvent(m_nSwitchEvent, 0, (cell)(this), (cell)(m_pInitEntity));
	}
	m_bHasBeenActivated = TRUE;
}

// =============================================================================================================

void CESwitch::_StartActivate()
{
	FASSERT(m_pInitEntity->TypeBits() & ENTITY_BIT_BOT);
	CBot *pBot = (CBot *)(m_pInitEntity);
	cchar *pszBTAName;

	pszBTAName = AIBTA_EventToBTAName("***_SWTC", pBot->AIBrain());
	if (pszBTAName !=NULL)
	{
		if( pBot->m_nPossessionPlayerIndex == -1 && pBot->AIBrain()) {
			// It's an AI bot.
			if( !ai_IsTalking( pBot->AIBrain() ) )
			{
				ai_TalkModeBegin(pBot->AIBrain(),
								AIBrain_TalkModeCB,
								(void*) pszBTAName,
								pBot,
								30,
								TRUE,	//Stop before starting the BTA
								TRUE,	//Turn To Look At pSwitch->Guid() object before starting the BTA
								Guid());
			}
		} else {
			pBot->StartBotTalkByName( pszBTAName );
		}
	}

	m_eCurState = SWITCHSTATE_ACTIVATING;
	EnableAutoWork(TRUE);

}


void CESwitch::MakeActivateSounds(BOOL bWorked)
{
	if (!bWorked)
	{
		fsndfx_Play3D( m_hLockedSound, &MtxToWorld()->m_vPos, 50.f, 1.f );
	}
	else
	{
		fsndfx_Play3D( m_hSwitchTriggeredSound, &MtxToWorld()->m_vPos, 50.f, 1.f );
	}
}

// =============================================================================================================

void CESwitch::_EndActivate()
{
	EnableAutoWork(FALSE);


//	if(pBot->m_nPossessionPlayerIndex == -1)
//	{
//		ai_TalkModeEnd(pBot->AIBrain());  //findfix: somehow, you need to make sure that this switch is definitely the thing that made the bot talk 
		m_eCurState = SWITCHSTATE_WAITINGFOROKAY;
//	}
//	else
//	{
		m_eCurState = SWITCHSTATE_IDLE;
//	}
}

// =============================================================================================================

void CESwitch::StateTransitions()
{
	FASSERT(m_pInitEntity->TypeBits() & ENTITY_BIT_BOT);
	switch(m_eCurState)
	{
		case SWITCHSTATE_ACTIVATING:
		{
			FASSERT(m_pInitEntity != NULL);
			CBot *pBot = (CBot *)(m_pInitEntity);
			CBotTalkInst *pBotTalk = pBot->m_pActiveTalkInst;
			if( pBotTalk ) {
				if( pBotTalk->GetCurrentUnitTime() > 0.5f ) {
					_EndActivate();
					_DoSwitchActions();
				}
			} else {
				// no bottalk playing.  That's a bad situation, but don't want to stop the switch from activating
				DEVPRINTF( "CESwitch::StateTransitions():  Switch activating without a bottalk playing\n" );
				_EndActivate();
				_DoSwitchActions();
			}
			break;

			//m_pSwitchAnim->DeltaTime(FLoop_fPreviousLoopSecs, TRUE);

			//f32 fAnimUnitTime = m_pSwitchAnim->GetUnitTime();
			//if(fAnimUnitTime <= 0.1f)
			//{
			//	pBot->UserAnim_SetControlValue(0, fAnimUnitTime * 10.0f);
			//}
			//else if(fAnimUnitTime >= 0.9f)
			//{
			//	pBot->UserAnim_SetControlValue(0, (1.0f - fAnimUnitTime) * 10.0f);
			//}
			//else
			//{
			//	pBot->UserAnim_SetControlValue(0, 1.0f);
			//}

			//if(fAnimUnitTime >= 0.99f)
			//{
			//	_EndActivate();
			//	_DoSwitchActions();

			//	break;
			//}
			//break;
		}
		default:
		{
			FASSERT_NOW;
			break;
		}
	}
}

// =============================================================================================================

void CESwitch::StateWork()
{
}


void CESwitch::CheckpointSaveSelect( s32 nCheckpoint )
{
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}


BOOL CESwitch::CheckpointSave( void )
{
	CMeshEntity::CheckpointSave();

	CFCheckPoint::SaveData( m_bHasBeenActivated );
	return TRUE;
}


void CESwitch::CheckpointRestore( void )
{
	CMeshEntity::CheckpointRestore();

	CFCheckPoint::LoadData( m_bHasBeenActivated );
}



// =============================================================================================================
// =============================================================================================================
// =============================================================================================================

void CESwitchBuilder::SetDefaults(u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType)
{
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS(CMeshEntityBuilder, ENTITY_BIT_SWITCH, pszEntityType);

	u32 uCurDoorIdx;
	for(uCurDoorIdx = 0; uCurDoorIdx < 4; ++uCurDoorIdx)
	{
		m_apszDoorNames[uCurDoorIdx] = NULL;
	}
	m_bTriggerEvent = TRUE;	// designers request that event triggering be on by default
	m_bRequireChip = FALSE;
	m_bMilOnly  = FALSE;
}

// =============================================================================================================

BOOL CESwitchBuilder::InterpretTable()
{
	FGameDataTableHandle_t hCurTable = CEntityParser::m_hTable;
	FGameData_VarType_e eVarType;

	if(!fclib_stricmp(CEntityParser::m_pszTableName, "doorname"))
	{
		if(CEntityParser::m_nFieldCount > 4)
		{
			DEVPRINTF("CESwitchBuilder::InterpretTable() : Too many entries in 'doorname' table (max = 4).\n");
		}
		CEntityParser::Interpret_StringArray(m_apszDoorNames, 4);
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "dooraction"))
	{
		eVarType = FGAMEDATA_VAR_TYPE_STRING;

		cchar *pszCurAction;
		u32 uCurEntry, uNumEntries = CEntityParser::m_nFieldCount;
		if(uNumEntries > 4)
		{
			DEVPRINTF("CESwitchBuilder::InterpretTable() : Too many entries in 'dooraction' table (max = 4).\n");
			uNumEntries = 4;
		}
		for(uCurEntry = 0; uCurEntry < uNumEntries; ++uCurEntry)
		{
			pszCurAction = (cchar *)(fgamedata_GetPtrToFieldData(hCurTable, uCurEntry, eVarType));
			if(!fclib_stricmp(pszCurAction, "open"))
			{
				m_aeDoorActions[uCurEntry] = CESwitch::DOORACTION_OPEN;
			}
			else if(!fclib_stricmp(pszCurAction, "close"))
			{
				m_aeDoorActions[uCurEntry] = CESwitch::DOORACTION_CLOSE;
			}
			else if(!fclib_stricmp(pszCurAction, "unlock"))
			{
				m_aeDoorActions[uCurEntry] = CESwitch::DOORACTION_UNLOCK;
			}
			else if(!fclib_stricmp(pszCurAction, "lock"))
			{
				m_aeDoorActions[uCurEntry] = CESwitch::DOORACTION_LOCK;
			}
			else if(!fclib_stricmp(pszCurAction, "toggle"))
			{
				m_aeDoorActions[uCurEntry] = CESwitch::DOORACTION_TOGGLE;
			}
			else
			{
				FASSERT_NOW;
			}
		}
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "triggerevent"))
	{
		cchar *pszValue;
		CEntityParser::Interpret_String(&pszValue);
		if(!fclib_stricmp(pszValue, "true"))
		{
			m_bTriggerEvent = TRUE;
		}
		else if(!fclib_stricmp(pszValue, "false"))
		{
			m_bTriggerEvent = FALSE;
		}
		else
		{
			DEVPRINTF("CESwitchBuilder::InterpretTable() : Invalid value '%s' for 'triggerevent' table.\n", pszValue);
			return(TRUE);
		}
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "reqchip"))
	{
		cchar *pszValue;
		CEntityParser::Interpret_String(&pszValue);
		if(!fclib_stricmp(pszValue, "true"))
		{
			m_bRequireChip = TRUE;
		}
		else if(!fclib_stricmp(pszValue, "false"))
		{
			m_bRequireChip = FALSE;
		}
		else
		{
			DEVPRINTF("CESwitchBuilder::InterpretTable() : Invalid value '%s' for 'triggerevent' table.\n", pszValue);
			return(TRUE);
		}
		return(TRUE);
	}
	else if (!_stricmp(CEntityParser::m_pszTableName, "useby"))
	{
		cchar *pszValue;
		CEntityParser::Interpret_String(&pszValue);
		if(!fclib_stricmp(pszValue, "mil"))
		{
			m_bMilOnly = TRUE;
		}
		else
		{
			DEVPRINTF("CESwitchBuilder::InterpretTable() : Invalid value '%s' for 'useby' table. mil\n", pszValue);
			return(TRUE);
		}
		return(TRUE);
	}

	return (CMeshEntityBuilder::InterpretTable());;
}

// =============================================================================================================

BOOL CESwitchBuilder::PostInterpretFixup()
{
	if(!CMeshEntityBuilder::PostInterpretFixup())
	{
		goto _ExitWithError;
	}

	u32 uCurDoorIdx;
	for(uCurDoorIdx = 0; uCurDoorIdx < 4; ++uCurDoorIdx)
	{
		if(m_apszDoorNames[uCurDoorIdx] != NULL)
		{
			m_apszDoorNames[uCurDoorIdx] = gstring_Main.AddString(m_apszDoorNames[uCurDoorIdx]);
		}
	}

	return(TRUE);

_ExitWithError:
	return(FALSE);
}
