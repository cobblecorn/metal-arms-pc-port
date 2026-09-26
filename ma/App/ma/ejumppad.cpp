//////////////////////////////////////////////////////////////////////////////////////
// ejumppad.cpp - Jumps ya from point to point
//
// Author: Michael Scholz
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
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

#include "ejumppad.h"
#include "ItemInst.h"
#include "bot.h"
#include "fclib.h"
#include "FCheckPoint.h"
#include "FScriptSystem.h"
#include "hud2.h"
#include "fresload.h"
#include "meshtypes.h"
#include "fsound.h"
#include "AI/AIGameUtils.h"
#include "botfx.h"
#include "fstringtable.h"

#define _JUMPPAD_PROP_FILENAME "jumppad.csv"

u32				CEJumpPad::m_uSystemFlags		= 0;
u32				CEJumpPad::m_nClassClientCount	= 0;
CEJumpPad::JumpPadProps_t CEJumpPad::m_JumpPadProps;

static CEJumpPadBuilder _EJumpPadDropBuilder;
static cchar* JumpPad_pszSoundEffectBank = "Jumppad";


const FGameData_TableEntry_t CEJumpPad::m_aJumpPadPropVocab[] =
{
//	CFSoundGroup* pActivateSnd;
	FGAMEDATA_VOCAB_SOUND_GROUP,

	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

const FGameDataMap_t CEJumpPad::m_aJumpPadPropMapTable[] =
{
	"JumpPad",
	m_aJumpPadPropVocab,
	sizeof(m_JumpPadProps),
	(void *)&m_JumpPadProps,

	NULL
};

void CEJumpPadBuilder::SetDefaults(u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType)
{
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS(CMeshEntityBuilder, ENTITY_BIT_JUMPPAD, pszEntityType);

	pszActivateSoundGroup = NULL;
	pszTarget = NULL;
	fHangTime = 0.0f;
	fDragPercent = 0.0f;
	eEffect = CEJumpPad::JUMPPAD_INVALID;
}

BOOL CEJumpPadBuilder::InterpretTable()
{
	FGameDataTableHandle_t hCurTable = CEntityParser::m_hTable;
	FGameData_VarType_e eVarType = FGAMEDATA_VAR_TYPE_STRING;

	if( !fclib_stricmp(CEntityParser::m_pszTableName, "target" ))
	{
		CEntityParser::Interpret_String( &pszTarget );
	}
	else if( !fclib_stricmp(CEntityParser::m_pszTableName, "sound" ))
	{
		CEntityParser::Interpret_String( &pszActivateSoundGroup );
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "hangtime"))
	{
		CEntityParser::Interpret_F32(&fHangTime);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "slowdown"))
	{
		CEntityParser::Interpret_F32(&fDragPercent);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "effect"))
	{
		CEntityParser::Interpret_U32((u32*)&eEffect);
		if (eEffect >= CEJumpPad::JUMPPAD_EFFECTCOUNT)
			eEffect = (CEJumpPad::JumpPadEffect_e)(CEJumpPad::JUMPPAD_EFFECTCOUNT-1);
	}
	else
	{
		return (CMeshEntityBuilder::InterpretTable());
	}
	return TRUE;
}

BOOL CEJumpPadBuilder::PostInterpretFixup()
{
	if(!CMeshEntityBuilder::PostInterpretFixup())
	{
		goto _ExitWithError;
	}

	return(TRUE);

_ExitWithError:
	return(FALSE);
}


BOOL CEJumpPad::ClassHierarchyLoadSharedResources( void ) 
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
   	FMATH_SETBITMASK(m_uSystemFlags, JUMPPAD_SYS_INITIALIZED);

	FResFrame_t ResFrame;
	ResFrame = fres_GetFrame();
	
	fang_MemZero( &m_JumpPadProps, sizeof(m_JumpPadProps) );

	// Load the sound effects bank for this bot...
	if( !fresload_Load( FSNDFX_RESTYPE, JumpPad_pszSoundEffectBank) )
	{
		DEVPRINTF( "CEJumpPad::ClassHierarchyLoadSharedResources(): Could not load sound effect bank '%s'\n", JumpPad_pszSoundEffectBank);
	}

	// Read the user properties for all EUK levels of this weapon...
	if ( !fgamedata_ReadFileUsingMap( m_aJumpPadPropMapTable, _JUMPPAD_PROP_FILENAME ) )
	{
		DEVPRINTF( "CEJumpPad::ClassHierarchyLoadSharedResources(): Could not read user properties from file '%s'.\n", _JUMPPAD_PROP_FILENAME );
		goto _ExitWithError;
	}
	
	return TRUE;

	// Error:
_ExitWithError:
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

void CEJumpPad::ClassHierarchyUnloadSharedResources( void ) 
{
	FASSERT( m_nClassClientCount > 0 );

	--m_nClassClientCount;

	if( m_nClassClientCount > 0 ) 
	{
		return;
	}

	CMeshEntity::ClassHierarchyUnloadSharedResources();
	
	FMATH_CLEARBITMASK(m_uSystemFlags, JUMPPAD_SYS_INITIALIZED);
}

// =============================================================================================================

CEJumpPad::CEJumpPad()
{

}

// =============================================================================================================

CEJumpPad::~CEJumpPad()
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


// =============================================================================================================

void CEJumpPad::ClassHierarchyDestroy()
{
	FASSERT( IsInitialized() );
	// These are set to NULL in Clear() below.
	CMeshEntity::ClassHierarchyDestroy();
}

// =============================================================================================================

BOOL CEJumpPad::ClassHierarchyBuild()
{
	FASSERT(IsInitialized());
	FASSERT(!IsCreated());
	FASSERT(FWorld_pWorld);

	// Get a frame.
	FResFrame_t hResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object.
	CEJumpPadBuilder *pBuilder = (CEJumpPadBuilder *)(GetLeafClassBuilder());

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
	if (pBuilder->pszTarget)
	{
		// copy the string into the "Main" string table and store the pointer
		m_pszTarget = CFStringTable::AddString( "Main", pBuilder->pszTarget );
	}


	if (pBuilder->pszActivateSoundGroup)
	{
		m_pActivateSnd = CFSoundGroup::RegisterGroup(pBuilder->pszActivateSoundGroup);
		if (!m_pActivateSnd)
		{
			DEVPRINTF("CEJumpPad::ClassHierarchyBuild() Sound not found %s using default",pBuilder->pszActivateSoundGroup);
		}
		else
		{
			m_pActivateSnd = m_JumpPadProps.pActivateSnd;
		}
	}
	else
	{
		m_pActivateSnd = m_JumpPadProps.pActivateSnd;
	}

	if (pBuilder->eEffect > JUMPPAD_INVALID)
	{
		m_eEffect = pBuilder->eEffect;
	}

	m_fHangTime = pBuilder->fHangTime;
	m_fDragPercent = 1.0f - pBuilder->fDragPercent;
	FMATH_CLAMP_UNIT_FLOAT(m_fDragPercent);

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame(hResFrame);
	return(FALSE);
}

BOOL CEJumpPad::ClassHierarchyBuilt( void ) 
{
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	if( !CMeshEntity::ClassHierarchyBuilt() ) 
	{
		goto _ExitWithError;
	}

	EnableOurWorkBit();

	return TRUE;

_ExitWithError:
	Destroy();
	return FALSE;
}

CEntityBuilder *CEJumpPad::GetLeafClassBuilder()
{
	return(&_EJumpPadDropBuilder);
}

void CEJumpPad::ClassHierarchyResolveEntityPointerFixups()
{
	CMeshEntity::ClassHierarchyResolveEntityPointerFixups();

	if( m_pszTarget ) 
	{
		m_pTarget = Find( m_pszTarget );

		if( m_pTarget ) 
		{
			// everything is okay;	
		} 
		else 
		{
			DEVPRINTF( "CEntity::ClassHierarchyResolveEntityPointerFixups(): Entity '%s' at Max ( %.2f, %.2f, %.2f )\n", Name(), m_MtxToWorld.m_vPos.x, m_MtxToWorld.m_vPos.z, m_MtxToWorld.m_vPos.y );
			DEVPRINTF( "                                                     Cannot find 'target' entity '%s'.\n", m_pszTarget );
			goto _ExitWithError;
		}
	}
	else
	{
			DEVPRINTF( "CEntity::ClassHierarchyResolveEntityPointerFixups(): Entity '%s' at Max ( %.2f, %.2f, %.2f )\n", Name(), m_MtxToWorld.m_vPos.x, m_MtxToWorld.m_vPos.z, m_MtxToWorld.m_vPos.y );
			DEVPRINTF( "                                                     No 'target' entity declared?\n");
			goto _ExitWithError;
	}
	EnableAutoWork(TRUE);
	return;


_ExitWithError:
	EnableAutoWork(FALSE);
}

void CEJumpPad::ClassHierarchyWork()
{
	FASSERT( IsInitialized() );

	CMeshEntity::ClassHierarchyWork();

	if( !IsOurWorkBitSet() )
	{
		return;
	}

	if( !(GetMeshInst()->GetVolumeFlags() & FVIS_VOLUME_IN_ACTIVE_LIST))
	{
		// We're not part of any active volume, therefore no workie;
		return;
	}
	
	
	_StateWork();
}

void CEJumpPad::_SetDefaults()
{
	m_eCurState = JUMPPAD_EXISTS;
	m_pszTarget = NULL;
	m_pTarget = NULL;
	m_pActivateSnd = NULL;
	m_fHangTime = 0.0f;
	m_fDragPercent = 1.0f;
	m_eEffect = JUMPPAD_HIGHJUMP;

	SetTargetable(FALSE);
	SetActionable(FALSE);
	SetCollisionFlag(TRUE);
}

void CEJumpPad::_SetState(JumpPadState_e eNewState)
{
	// transition out opportunities
	if (m_eCurState == JUMPPAD_EXISTS)
	{
	}

	m_eCurState = eNewState;
	// transition in opportunities
	if (m_eCurState == JUMPPAD_EXISTS)
	{
	}
}

void CEJumpPad::_StateWork()
{
	switch (m_eCurState)
	{
	case JUMPPAD_EXISTS:
		for(CEntity* pChildEntity=GetFirstChild(); pChildEntity; pChildEntity=GetNextChild(pChildEntity) ) 
		{
			if ( (pChildEntity->TypeBits() & ENTITY_BIT_BOT) &&	
				!(pChildEntity->TypeBits() & ENTITY_BIT_VEHICLE) && 
				!(pChildEntity->TypeBits() & ENTITY_BIT_SITEWEAPON) )
			{
				 CBot* pBot = (CBot*)pChildEntity;
				if (pBot->CanUseJumpPad())
					_BoostBot(pBot);
			}
		}
		break;
	}
}

void CEJumpPad::_BoostBot(CBot* pBot)
{
	CFVec3A vPadToBotXZ;
	vPadToBotXZ.x = pBot->MtxToWorld()->m_vPos.x - m_MtxToWorld.m_vPos.x;
	vPadToBotXZ.y = 0.f;
	vPadToBotXZ.z = pBot->MtxToWorld()->m_vPos.z - m_MtxToWorld.m_vPos.z;

	// compute the target to be similar XZ offset to launch position
	CFVec3A vTargetLanding;
	vTargetLanding.x = m_pTarget->MtxToWorld()->m_vPos.x + vPadToBotXZ.x;
	vTargetLanding.y = m_pTarget->MtxToWorld()->m_vPos.y;
	vTargetLanding.z = m_pTarget->MtxToWorld()->m_vPos.z + vPadToBotXZ.z;

	CFVec3A vFlightTraj;
	if (m_fHangTime > 0.0f)
	{
		aiutils_ComputeTrajectoryFromAirTime(vFlightTraj,pBot->MtxToWorld()->m_vPos,vTargetLanding,m_fHangTime,-pBot->m_pBotInfo_Gen->fGravity);
		pBot->m_Velocity_WS.Set(vFlightTraj);
	}
	else if (m_fHangTime < 0.0f)
	{
		pBot->Relocate_Xlat_WS(&vTargetLanding);
		pBot->m_Velocity_WS.Set(0.0f,0.0f,0.0f);
	}
	else
	{
		CFVec3A vFlightTraj;
		m_fHangTime = aiutils_ComputeTrajectoryFromXZVel(vFlightTraj,pBot->MtxToWorld()->m_vPos,vTargetLanding,pBot->m_pBotInfo_Walk->fMaxXlatVelocity,-pBot->m_pBotInfo_Gen->fGravity);
		pBot->m_Velocity_WS.Set(vFlightTraj);
	}
	pBot->WS2MS(pBot->m_Velocity_MS,pBot->m_Velocity_WS);
	pBot->VelocityHasChanged();
	pBot->SetJumpPadBoost();
	pBot->m_uLandingVelocityOverride = (u16)(m_fDragPercent * (f32) ((u16)-1));
	pBot->PlaySound(m_pActivateSnd);
	switch (m_eEffect)
	{
	default:
	case JUMPPAD_NOEFFECT:
		break;
	case JUMPPAD_HIGHJUMP:
		{
		CBotPowerupFx* pFX = pBot->GetPowerupFx();
		if (pFX)
		{
			if (m_fHangTime > 0.0f)
			{
				pFX->s_uActivePickupCount++;
				pFX->_StartHiJumpFx(m_fHangTime,1.0f, 1.0f);
			}
		}
		}
		break;
	}
}

void CEJumpPad::CheckpointSaveSelect( s32 nCheckpoint )
{
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}

BOOL CEJumpPad::CheckpointSave( void )
{
	// Save parent class data...
	CMeshEntity::CheckpointSave();
	CFCheckPoint::SaveData( (u32&) m_eCurState );
	return TRUE;
}

void CEJumpPad::CheckpointRestore( void )
{
	// Load parent class data...
	CMeshEntity::CheckpointRestore();

	JumpPadState_e				eCurState;
	CFCheckPoint::LoadData( (u32&) eCurState );
	_SetState(eCurState);
}

void CEJumpPad::ClassHierarchyAddToWorld( void )
{
	CMeshEntity::ClassHierarchyAddToWorld();
}

void CEJumpPad::ClassHierarchyRemoveFromWorld( void )
{
	CMeshEntity::ClassHierarchyRemoveFromWorld();
}

void CEJumpPad::ClassHierarchyRelocated( void *pIdentifier )
{
	CMeshEntity::ClassHierarchyRelocated( pIdentifier );
}
