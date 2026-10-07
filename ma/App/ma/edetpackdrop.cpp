//////////////////////////////////////////////////////////////////////////////////////
// EDetPackDrop.cpp - blows stuff up
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
// Date		Who			Description
// --------	----------	--------------------------------------------------------------
// 02.07.03	Scholz		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "EDetPackDrop.h"
#include "player.h"
#include "ItemInst.h"
#include "bot.h"
#include "fclib.h"
#include "FCheckPoint.h"
#include "FScriptSystem.h"
#include "hud2.h"
#include "fresload.h"
#include "meshtypes.h"
#include "fsound.h"
#include "fstringtable.h"

// =============================================================================================================
#define _DET_PROP_FILENAME "detpack.csv"

static s32 _nTEXLAYER_ID	= 1;
static s32 _nLIGHT_ID		= 1;

u32		CEDetPackDrop::m_uSystemFlags = 0;
u32		CEDetPackDrop::m_nClassClientCount	= 0;

FExplosion_GroupHandle_t	CEDetPackDrop::m_hExplosionGroup = FEXPLOSION_INVALID_HANDLE;
CEDetPackDrop::_DetProps_t	CEDetPackDrop::m_DefaultDetProps;

static CEDetPackDropBuilder _EDetPackDropBuilder;

// This table describes to fgamedata how our user property table is to be interpreted:
const FGameData_TableEntry_t CEDetPackDrop::m_aDetPropVocab[] =
{
//		cchar*				pszExplosion;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

//		f32					fCountDownTime;
	FGAMEDATA_VOCAB_F32_UNBOUND,

//		FSndFx_FxHandle_t	hSoundActivate;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_TO_SOUND_GROUP,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

//		FSndFx_FxHandle_t	hSoundTick;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_TO_SOUND_GROUP,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

//		FSndFx_FxHandle_t	hSoundTock;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_TO_SOUND_GROUP,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//		cchar*				pszFormatStr;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,
//		cchar*				pszFormatStr;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

//		f32					fSlowBeepInterval;
	FGAMEDATA_VOCAB_F32_UNBOUND,
//		f32					fFastBeepInterval;
	FGAMEDATA_VOCAB_F32_UNBOUND,
//		f32					fSlowToFastSwitch;
	FGAMEDATA_VOCAB_F32_UNBOUND,
//		f32					fGameoverTime;
	FGAMEDATA_VOCAB_F32_UNBOUND,
//		f32					fAlphaMin;
	FGAMEDATA_VOCAB_F32_UNBOUND,
//		f32					fAlphaMax;
	FGAMEDATA_VOCAB_F32_UNBOUND,
//		f32					fAlphaPeriod;
	FGAMEDATA_VOCAB_F32_UNBOUND,

	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

const FGameDataMap_t CEDetPackDrop::m_aDetPropMapTable[] =
{
	"DetPack1",
	m_aDetPropVocab,
	sizeof(m_DefaultDetProps),
	(void *)&m_DefaultDetProps,
	
	NULL
};


// =============================================================================================================
BOOL CEDetPackDrop::ClassHierarchyLoadSharedResources( void ) 
{
	FASSERT( m_nClassClientCount != 0xffffffff );

	++m_nClassClientCount;

	if( !CMeshEntity::ClassHierarchyLoadSharedResources() ) 
	{
		// Bail now since parent class has already called
		// ClassHierarchyUnloadSharedResources() and decremented
		// our client counter...
		return FALSE;
	}

	if( m_nClassClientCount > 1 ) 
	{
		// Resources already loaded...
		return TRUE;
	}
	FASSERT(!IsInitialized());
   	FMATH_SETBITMASK(m_uSystemFlags, DETPACK_SYS_INITIALIZED);

	FResFrame_t ResFrame;
	ResFrame = fres_GetFrame();
	
	fang_MemZero( &m_DefaultDetProps, sizeof(m_DefaultDetProps) );

	// Read the user properties for all EUK levels of this weapon...
	if ( !fgamedata_ReadFileUsingMap( m_aDetPropMapTable, _DET_PROP_FILENAME ) )
	{
		DEVPRINTF( "CEDetPackDrop::ClassHierarchyLoadSharedResources(): Could not read user properties from file '%s'.\n", _DET_PROP_FILENAME );
		goto _ExitWithError;
	}

	return TRUE;

	// Error:
_ExitWithError:
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

void CEDetPackDrop::ClassHierarchyUnloadSharedResources( void ) 
{
	FASSERT( m_nClassClientCount > 0 );

	--m_nClassClientCount;

	if( m_nClassClientCount > 0 ) 
	{
		return;
	}

	CMeshEntity::ClassHierarchyUnloadSharedResources();
	
	FMATH_CLEARBITMASK(m_uSystemFlags, DETPACK_SYS_INITIALIZED);
}

CEDetPackDrop::CEDetPackDrop()
{

}

// =============================================================================================================

CEDetPackDrop::~CEDetPackDrop()
{
	if(IsInitialized() && IsCreated())
	{
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld(TRUE);
		ClassHierarchyDestroy();
	}
}

// =============================================================================================================

BOOL CEDetPackDrop::ActionNearby(CEntity *pEntity)
{
	CEntity::ActionNearby(pEntity);

	if (!pEntity->TypeBits() & ENTITY_BIT_BOT)
	{
		DEVPRINTF("Don't know who you are, but you're not touching me!\n");
		return FALSE;
	}
	if (m_eCurState != DETDROP_EXISTS)
	{
		// not in preactivate state
		return FALSE;
	}
	m_pActionBot = (CBot*)pEntity;

	if (m_pActionBot->m_pInventory == NULL)
	{
		DEVPRINTF("Bot no inventory, no detpack activation!\n");
		return FALSE;
	}

	if (m_pActionBot->m_pInventory->m_aoItems[INVPOS_DETPACK].m_nClipAmmo <= 0)
	{
		DEVPRINTF("Bot no detpack, no detpack activation!\n");
		// play click?
		return FALSE;
	}
	else
	{
		// activating detpack!
		_SetState(DETDROP_ACTIVATED);
		return(TRUE);
	}

	// never get here
	return(FALSE);
}

// =============================================================================================================

void CEDetPackDrop::ClassHierarchyDestroy()
{
	// These are set to NULL in Clear() below.
	CMeshEntity::ClassHierarchyDestroy();
}

// =============================================================================================================

BOOL CEDetPackDrop::ClassHierarchyBuild()
{
	FASSERT(IsInitialized());
	FASSERT(!IsCreated());
	FASSERT(FWorld_pWorld);

	// Get a frame.
	FResFrame_t hResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object.
	CEDetPackDropBuilder *pBuilder = (CEDetPackDropBuilder *)(GetLeafClassBuilder());

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

	if (pBuilder->m_pszMasterDetPackName)
	{
		// copy the string into the "Main" string table and store the pointer
		m_pszMasterDetPack = CFStringTable::AddString( "Main", pBuilder->m_pszMasterDetPackName );
	}

	if (pBuilder->m_pszActivateSound)
	{
		m_DetProps.pSoundActivate = CFSoundGroup::RegisterGroup( pBuilder->m_pszActivateSound );
		if (!m_DetProps.pSoundActivate)
		{
			DEVPRINTF("CEDetPackDrop::ClassHierarchyBuild() failed on sound %s", pBuilder->m_pszActivateSound);
		}
	}

	if (pBuilder->m_pszExplosion)
	{
		m_DetProps.pszExplosion = pBuilder->m_pszExplosion;
	}

	m_DetProps.fCountDownTime = pBuilder->m_fCountDownTime;
	m_DetProps.fAlphaFrequency =  pBuilder->m_fAlphaFreq;
	m_DetProps.fAlphaMin = pBuilder->m_fAlphaMin;
	m_DetProps.fAlphaMax = pBuilder->m_fAlphaMax;
	m_DetProps.fSlowToFastSwitch = pBuilder->m_fFasterBeep;

	//
	/////////////////////////////////////////////////////////////
	m_hTexLayer = GetMeshInst()->GetTexLayerHandle( _nTEXLAYER_ID );
	if( m_hTexLayer == FMESH_TEXLAYERHANDLE_INVALID ) 
	{
		DEVPRINTF( "CEDetPackDrop::ClassHierarchyBuild():  Error getting tex layer handle ID %d\n", _nTEXLAYER_ID );
	} 
	m_pWorldLight = GetMeshInst()->GetAttachedLightByID(_nLIGHT_ID); 
	if( m_pWorldLight == NULL ) 
	{
		DEVPRINTF( "CEDetPackDrop::ClassHierarchyBuild():  Error getting light handle ID %d\n", _nLIGHT_ID);
	} 
	
	m_hExplosionGroup = CExplosion2::GetExplosionGroup( m_DetProps.pszExplosion );
	return(TRUE);

_ExitWithError:
	Destroy();
	fres_ReleaseFrame(hResFrame);
	return(FALSE);
}

// =============================================================================================================

BOOL CEDetPackDrop::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	if( !CMeshEntity::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	EnableOurWorkBit();

	return TRUE;

_ExitWithError:
	Destroy();
	return FALSE;
}

// =============================================================================================================


// =============================================================================================================

CEntityBuilder *CEDetPackDrop::GetLeafClassBuilder()
{
	return(&_EDetPackDropBuilder);
}

// =============================================================================================================

void CEDetPackDrop::ClassHierarchyResolveEntityPointerFixups()
{
	CMeshEntity::ClassHierarchyResolveEntityPointerFixups();

	if( m_pszMasterDetPack  ) 
	{
		CEntity* pEntity = Find( m_pszMasterDetPack );
		if (!pEntity)
		{
			DEVPRINTF( "CEntity::ClassHierarchyResolveEntityPointerFixups(): Entity '%s' at Max ( %.2f, %.2f, %.2f )\n", Name(), m_MtxToWorld.m_vPos.x, m_MtxToWorld.m_vPos.z, m_MtxToWorld.m_vPos.y );
			DEVPRINTF( "                                                     Cannot find 'target' entity '%s'.\n", m_pszMasterDetPack );
			goto _SelfParented;
		}
		
		if (! (pEntity->TypeBits() & ENTITY_BIT_DETPACKDROP ) )
		{
			DEVPRINTF( "CEntity::ClassHierarchyResolveEntityPointerFixups(): Entity '%s' at Max ( %.2f, %.2f, %.2f )\n", Name(), m_MtxToWorld.m_vPos.x, m_MtxToWorld.m_vPos.z, m_MtxToWorld.m_vPos.y );
			DEVPRINTF( "entity '%s' is not of type detpack, failure\n", m_pszMasterDetPack );
			goto _SelfParented;
		}
		
		if (this != pEntity) // if I'm not my own parent, some other detpack must be. 
		{
			m_pMasterDetPack = (CEDetPackDrop*) pEntity;
			m_pMasterDetPack->m_uInactiveChildCount ++;
			return;
		}
	}

_SelfParented:
	m_pMasterDetPack = this;
	m_uInactiveChildCount++;
}


// =============================================================================================================

void CEDetPackDrop::ClassHierarchyWork()
{
	CMeshEntity::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) 
	{
		return;
	}
	
	_StateWork();
}

// =============================================================================================================

void CEDetPackDrop::_SetDefaults()
{
	// master values
	m_pMasterDetPack = NULL;
	m_uInactiveChildCount = 0; 
	m_fDetonateTimeLeft = 0.0f;						
	m_fGameOverTimeLeft = 0.0f;						
	m_fTimeUntilNextBeep= 0.0f;					

	// instance values
	m_pActionBot = NULL;
	m_pfTimeRemainingToKablooie = NULL;
	m_Damager.pEntity = this;
	m_Damager.pWeapon = NULL;
	m_Damager.pBot   = NULL;
	m_fUnitAlphaOverride = 0.0f;
	m_pWorldLight = NULL;
	fang_MemCopy(&m_DetProps,&m_DefaultDetProps,sizeof(m_DefaultDetProps));
	m_fAlphaOverrideFreqMult = m_DefaultDetProps.fAlphaFrequency;
	SetCollisionFlag(TRUE); // since we only action against collideables
	SetTargetable(FALSE);
	SetActionable(TRUE);
	EnableAutoWork(TRUE);
	m_eCurState=DETDROP_EXISTS;
}


// =============================================================================================================

void CEDetPackDrop::_SetState(DetState_e eNewState)
{
	// transition out opportunities
	if (m_eCurState == DETDROP_EXISTS) 
	{
        
	}
	else if (m_eCurState == DETDROP_ACTIVATED)  
	{
	}
	else if (m_eCurState == DETDROP_COUNTDOWN)  
	{
	}
	else if (m_eCurState == DETDROP_EXPLODED)  
	{
	}
	
	m_eCurState = eNewState;
	// transition in opportunities
	if (m_eCurState == DETDROP_EXISTS) 
	{
	}
	else if (m_eCurState == DETDROP_ACTIVATED)  
	{
		SelectMesh(ACTIVATED_MESH);
		m_pfTimeRemainingToKablooie	= &m_pMasterDetPack->m_fDetonateTimeLeft;
		m_pActionBot->m_pInventory->m_aoItems[INVPOS_DETPACK].m_nClipAmmo--;
		m_pMasterDetPack->m_uInactiveChildCount--;
		FASSERT(m_pMasterDetPack->m_uInactiveChildCount != -1);

		if (m_pMasterDetPack->m_uInactiveChildCount==0 ) // hey, last one!
		{
			m_pMasterDetPack->m_fDetonateTimeLeft = m_pMasterDetPack->m_DetProps.fCountDownTime;
			m_pMasterDetPack->m_fTimeUntilNextBeep = m_pMasterDetPack->m_DetProps.fFastBeepInterval;
		}

		// we're alive now!
		EnableAutoWork(TRUE);
		m_pActionBot->PlaySound(m_DetProps.pSoundActivate);
		CFScriptSystem::TriggerEvent(CFScriptSystem::GetEventNumFromName("detpack"), DETPACK_DROPPED, (u32) this,(u32) m_pActionBot);
	}
	else if (m_eCurState == DETDROP_COUNTDOWN)  
	{
		CFScriptSystem::TriggerEvent(CFScriptSystem::GetEventNumFromName("detpack"), DETPACK_COUNTDOWN, (u32) this,  (u32) m_pActionBot);
	}
	else if (m_eCurState == DETDROP_EXPLODED)  
	{
		_ClearHudTimer();
		FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();

		if( hSpawner != FEXPLOSION_INVALID_HANDLE ) 
		{
			FExplosionSpawnParams_t SpawnParams;
			SpawnParams.InitToDefaults();
			m_Damager.pBot		= m_pActionBot;
			SpawnParams.pDamager= &m_Damager;
			SpawnParams.Pos_WS  = m_MtxToWorld.m_vPos.v3;
			SpawnParams.UnitDir = m_MtxToWorld.m_vUp.v3;
            CExplosion2::SpawnExplosion( hSpawner, m_hExplosionGroup, &SpawnParams );
		}
		
		CFScriptSystem::TriggerEvent(CFScriptSystem::GetEventNumFromName("detpack"), DETPACK_BOOM, (u32) this, (u32) m_pActionBot);
		
		if (this==m_pMasterDetPack)
		{
			m_fGameOverTimeLeft = m_DetProps.fGameOverTime;
		}
	}
}

// =============================================================================================================

void CEDetPackDrop::_ClearHudTimer( void )
{
#if FANG_WINGC
	if( this != m_pMasterDetPack ) return;
	// Possession can move the planter away from the HUD that received this
	// countdown. Clear only this pack's pointer on every possible owner.
	for( s32 n=0; n<CPlayer::m_nPlayerCount; ++n ) {
		CHud2::GetHudForPlayer(n)->ClearIconTimerDrawIfOwned(
			CHud2::ICON_TIMER_TYPE_DETPACK, &m_fDetonateTimeLeft );
	}
#endif
}

void CEDetPackDrop::_StateWork()
{
	switch (m_eCurState)
	{
	case DETDROP_EXISTS:
		{
			f32 fAlphaRange = m_DetProps.fAlphaMax - m_DetProps.fAlphaMin;
			m_fUnitAlphaOverride += FLoop_fPreviousLoopSecs * m_fAlphaOverrideFreqMult;
						
			if (m_fUnitAlphaOverride < 0.0f)
			{
				m_fUnitAlphaOverride =0.0f;
				m_fAlphaOverrideFreqMult =- m_fAlphaOverrideFreqMult;
			}
			else if (m_fUnitAlphaOverride > 1.0f)
			{
				m_fUnitAlphaOverride =1.0f;
				m_fAlphaOverrideFreqMult =- m_fAlphaOverrideFreqMult;
			}
				 
			f32 fAlpha = m_DetProps.fAlphaMin + (m_fUnitAlphaOverride * fAlphaRange);
			GetMeshInst()->LayerAlpha_Set( m_hTexLayer, fAlpha );
			m_pWorldLight->m_Light.SetIntensity(fAlpha);
			break;
		}
	case DETDROP_ACTIVATED:
		FASSERT(m_pfTimeRemainingToKablooie);
		if (*m_pfTimeRemainingToKablooie > 0.0f)
		{
			_SetState(DETDROP_COUNTDOWN);
		}
		break;

	case DETDROP_COUNTDOWN:
		if (this==m_pMasterDetPack) // only master det pack gets to count down;
		{
			FASSERT(m_fDetonateTimeLeft > 0.0f);
			m_fDetonateTimeLeft -= FLoop_fPreviousLoopSecs;
			FMATH_CLAMP_MIN0(m_fDetonateTimeLeft);
			if (m_fDetonateTimeLeft > 0.0f)
			{
				if (m_pActionBot->m_nPossessionPlayerIndex >= 0)
				{
					CHud2* pHud = CHud2::GetHudForPlayer(m_pActionBot->m_nPossessionPlayerIndex);
					if (m_fDetonateTimeLeft < 10.0f)
					{
						pHud->SetIconTimerDraw( CHud2::ICON_TIMER_TYPE_DETPACK, TRUE, &m_fDetonateTimeLeft );
					}
					else
					{
						pHud->SetIconTimerDraw( CHud2::ICON_TIMER_TYPE_DETPACK, TRUE, &m_fDetonateTimeLeft );
					}
				}

				m_fTimeUntilNextBeep -= FLoop_fPreviousLoopSecs;
				if (m_fTimeUntilNextBeep < 0.0f)
				{
					if (m_fDetonateTimeLeft < m_DetProps.fSlowToFastSwitch)
						m_fTimeUntilNextBeep = m_DetProps.fFastBeepInterval;
					else
						m_fTimeUntilNextBeep = m_DetProps.fSlowBeepInterval;

					if (m_uSystemFlags & DETPACK_SYS_ALT_BEEP)
					{
						FMATH_CLEARBITMASK(m_uSystemFlags, DETPACK_SYS_ALT_BEEP);
						m_pActionBot->PlaySound(m_DetProps.pSoundTock);
					}
					else
					{
						FMATH_SETBITMASK(m_uSystemFlags, DETPACK_SYS_ALT_BEEP);
						m_pActionBot->PlaySound(m_DetProps.pSoundTick);
					}
				}
			}
		}
		
		if (*m_pfTimeRemainingToKablooie <= 0.0f)
		{
			_SetState(DETDROP_EXPLODED);
		}
		break;

	case DETDROP_EXPLODED:
		this->DrawEnable(FALSE);
		if (this==m_pMasterDetPack) // only master det pack gets to count down;
		{
			FASSERT(m_fGameOverTimeLeft > 0.0f);
			m_fGameOverTimeLeft -= FLoop_fPreviousLoopSecs;
			if (m_fGameOverTimeLeft <= 0.0f)
			{
                EnableAutoWork(FALSE);
				this->RemoveFromWorld(TRUE);
				_SetState(DETDROP_NOEXIST);
			}
			else
			{
#if !FANG_WINGC
				if (m_pActionBot->m_nPossessionPlayerIndex >= 0)
				{
					CHud2* pHud = CHud2::GetHudForPlayer(m_pActionBot->m_nPossessionPlayerIndex);
					pHud->SetIconTimerDraw( CHud2::ICON_TIMER_TYPE_DETPACK, TRUE, NULL );
				}
#endif
			}
		}
		else
		{
			EnableAutoWork(FALSE);
			this->RemoveFromWorld(TRUE);
			_SetState(DETDROP_NOEXIST);
		}
		break;
	}
}



// =============================================================================================================

void CEDetPackDropBuilder::SetDefaults(u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType)
{
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS(CMeshEntityBuilder, ENTITY_BIT_DETPACKDROP, pszEntityType);

	m_pszActivateSound = NULL;
	m_pszMasterDetPackName = NULL;
	m_fCountDownTime = CEDetPackDrop::m_DefaultDetProps.fCountDownTime;
	m_pszExplosion = CEDetPackDrop::m_DefaultDetProps.pszExplosion;
	m_fCountDownTime = CEDetPackDrop::m_DefaultDetProps.fCountDownTime;
	m_fAlphaFreq = CEDetPackDrop::m_DefaultDetProps.fAlphaFrequency;
	m_fAlphaMin = CEDetPackDrop::m_DefaultDetProps.fAlphaMin;
	m_fAlphaMax = CEDetPackDrop::m_DefaultDetProps.fAlphaMax;
	m_fFasterBeep = CEDetPackDrop::m_DefaultDetProps.fSlowToFastSwitch;
}

// =============================================================================================================

BOOL CEDetPackDropBuilder::InterpretTable()
{
	FGameDataTableHandle_t hCurTable = CEntityParser::m_hTable;
	FGameData_VarType_e eVarType = FGAMEDATA_VAR_TYPE_STRING;
	
	if( !fclib_stricmp(CEntityParser::m_pszTableName, "activate_sound" ))
	{
		CEntityParser::Interpret_String( &m_pszActivateSound );
	}
	else if( !fclib_stricmp(CEntityParser::m_pszTableName, "master" ))
	{
		CEntityParser::Interpret_String( &m_pszMasterDetPackName );
	}
	else if( !fclib_stricmp(CEntityParser::m_pszTableName, "explosion" ))
	{
		CEntityParser::Interpret_String( &m_pszExplosion );
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "countdown_time"))
	{
		CEntityParser::Interpret_F32(&m_fCountDownTime);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "alpha_freq"))
	{
		CEntityParser::Interpret_F32(&m_fAlphaFreq);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "alpha_min"))
	{
		CEntityParser::Interpret_F32(&m_fAlphaMin);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "alpha_max"))
	{
		CEntityParser::Interpret_F32(&m_fAlphaMax);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "faster_beep"))
	{
		CEntityParser::Interpret_F32(&m_fFasterBeep);
	}
	else
	{
		return (CMeshEntityBuilder::InterpretTable());
	}
	return TRUE;
}

// =============================================================================================================

BOOL CEDetPackDropBuilder::PostInterpretFixup()
{
	if(!CMeshEntityBuilder::PostInterpretFixup())
	{
		goto _ExitWithError;
	}

	return(TRUE);

_ExitWithError:
	return(FALSE);
}

void CEDetPackDrop::CheckpointSaveSelect( s32 nCheckpoint ) 
{
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}

BOOL CEDetPackDrop::CheckpointSave( void )
{
	// Save parent class data...
	CMeshEntity::CheckpointSave();

	CFCheckPoint::SaveData( (u32&) m_eCurState );
	if (this==m_pMasterDetPack)
	{
		CFCheckPoint::SaveData( (u32&) m_uInactiveChildCount );
		CFCheckPoint::SaveData( (u32&) m_fDetonateTimeLeft );	
		CFCheckPoint::SaveData( (u32&) m_fGameOverTimeLeft );
		CFCheckPoint::SaveData( (u32&) m_fTimeUntilNextBeep);
	}
	return TRUE;
}

void CEDetPackDrop::CheckpointRestore( void )
{
	_ClearHudTimer();
	// Load parent class data...
	CMeshEntity::CheckpointRestore();
	
	DetState_e				eCurState;
	CFCheckPoint::LoadData( (u32&) eCurState );

	if (this==m_pMasterDetPack)
	{
		CFCheckPoint::LoadData( (u32&) m_uInactiveChildCount );
		CFCheckPoint::LoadData( (u32&) m_fDetonateTimeLeft );	
		CFCheckPoint::LoadData( (u32&) m_fGameOverTimeLeft );
		CFCheckPoint::LoadData( (u32&) m_fTimeUntilNextBeep);
	}
	
	_SetState(eCurState);
}

void CEDetPackDrop::ClassHierarchyAddToWorld( void )
{
	CMeshEntity::ClassHierarchyAddToWorld();
}

void CEDetPackDrop::ClassHierarchyRemoveFromWorld( void )
{
	_ClearHudTimer();
	CMeshEntity::ClassHierarchyRemoveFromWorld();
#if !FANG_WINGC
	if (this==m_pMasterDetPack)
	{
		if (m_pActionBot && m_pActionBot->m_nPossessionPlayerIndex >= 0)
		{
			CHud2* pHud = CHud2::GetHudForPlayer(m_pActionBot->m_nPossessionPlayerIndex);
			pHud->SetIconTimerDraw( CHud2::ICON_TIMER_TYPE_DETPACK, FALSE );
		}
	}
#endif
}

void CEDetPackDrop::ClassHierarchyRelocated( void *pIdentifier ) 
{
	CMeshEntity::ClassHierarchyRelocated( pIdentifier );
}
