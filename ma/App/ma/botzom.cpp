//////////////////////////////////////////////////////////////////////////////////////
// botzom.cpp - Zombie bot.
//
// Author: Steve Ranck     
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
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 01/27/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "botzom.h"
#include "fanim.h"
#include "fresload.h"
#include "floop.h"
#include "fsndfx.h"
#include "fclib.h"
#include "FCheckPoint.h"
#include "meshtypes.h"
#include "ProTrack.h"
#include "weapon.h"
#include "fworld_coll.h"
#include "sound.h"
#include "botzom_part.h"
#include "eparticlepool.h"
#include "ai/aienviro.h"
#include "fsound.h"
#include "player.h"

#include <vector>

#define _BOTINFO_FILENAME		"b_zom"
#define _SOUND_BANK_NAME		"ZombieBot"


#define _ZOMBOT_MESH_NAME		"GRZAzombA00"




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotZomBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CBotZomBuilder _BotZomBuilder;


void CBotZomBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_BOTZOM, pszEntityType );

	// Override defaults set by our parent classes...
	m_nEC_HealthContainerCount = CBotZom::m_BotInfo_Gen.nBatteryCount;
	m_pszEC_ArmorProfile = CBotZom::m_BotInfo_Gen.pszArmorProfile;

	m_bStartFormed = FALSE;

	// This class of bots shows up on Glitch's radar...
	FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_SHOWS_UP_ON_RADAR );
}


BOOL CBotZomBuilder::PostInterpretFixup( void ) {
	if( !CBotBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	return TRUE;

	// Error:
_ExitWithError:
	return FALSE;
}


BOOL CBotZomBuilder::InterpretTable( void ) {
	if( !fclib_stricmp( CEntityParser::m_pszTableName, "ZombieStartFormed" ) ) {
		// ZombieStartFormed on/off

		CEntityParser::Interpret_BOOL( &m_bStartFormed );
		return TRUE;
	}

	return CBotBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotZom
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CBotZom::m_bSystemInitialized = FALSE;
u32 CBotZom::m_nBotClassClientCount = 0;

CBotZom::BotInfo_Gen_t CBotZom::m_BotInfo_Gen;
CBotZom::BotInfo_MountAim_t CBotZom::m_BotInfo_MountAim;
CBotZom::BotInfo_Walk_t CBotZom::m_BotInfo_Walk;
CBotZom::BotInfo_Jump_t CBotZom::m_BotInfo_Jump;
CBotZom::BotInfo_Weapon_t CBotZom::m_BotInfo_Weapon;
CBotZom::BotInfo_Zom_t CBotZom::m_BotInfo_Zom;

FMesh_t *CBotZom::m_pZomBotMesh;
CBotAnimStackDef CBotZom::m_AnimStackDef;
CFVec3A CBotZom::m_GroinVecY_WS;
CFCollInfo CBotZom::m_CollInfo;
CFMtx43A CBotZom::m_TempAnimMtx;

CFTexInst CBotZom::m_StreamerTexInst;

struct _ZomSlashHit_t {
	CEntity *pEntity;
	CFWorldMesh *pWorldMesh;
	FCollImpact_t Impact;
	BOOL bApplyDamage;
};

// Damage and debris effects can add or remove mesh trackers. Collect hits during the world-list
// traversal, then apply them after fworld_CollideWithTrackers has finished walking those lists.
static std::vector<_ZomSlashHit_t> _aZomSlashHits;

cchar *CBotZom::m_apszStreamerBones[] = {
	"L_Finger_1_Tip",
	"L_Finger_2_Tip",
	"Thumb_Tip",
	NULL
};


BOOL CBotZom::InitSystem( void ) {

	FASSERT( !m_bSystemInitialized );

	m_nBotClassClientCount = 0;

	m_bSystemInitialized = TRUE;

	return TRUE;
}


void CBotZom::UninitSystem( void ) {
	if( m_bSystemInitialized ) {
		m_bSystemInitialized = FALSE;
	}
}


BOOL CBotZom::ClassHierarchyLoadSharedResources( void ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( m_nBotClassClientCount != 0xffffffff );

	++m_nBotClassClientCount;

	if( !CBot::ClassHierarchyLoadSharedResources() ) {
		// Bail now since parent class has already called
		// ClassHierarchyUnloadSharedResources() and decremented
		// our client counter...

		return FALSE;
	}

	if( m_nBotClassClientCount > 1 ) {
		// Resources already loaded...

		return TRUE;
	}

	// Resources not yet loaded...

	FResFrame_t ResFrame = fres_GetFrame();

	// Load the sound effects bank for this bot...
	if( !fresload_Load( FSNDFX_RESTYPE, _SOUND_BANK_NAME ) ) {
		DEVPRINTF( "CBotZom::ClassHierarchyLoadSharedResources(): Could not load sound effect bank '%s'\n", _SOUND_BANK_NAME );
	}

	if( !ReadBotInfoFile( m_aGameDataMap, _BOTINFO_FILENAME ) ) {
		goto _ExitWithError;
	}

	if( !_BuildAnimStackDef() ) {
		goto _ExitWithError;
	}

	// Load streamer texture...
	if( m_BotInfo_Zom.pSlashStreamerTexDef == NULL ) {
		DEVPRINTF( "CBotZom::ClassHierarchyLoadSharedResources(): Trouble loading streamer texture.\n" );
		goto _ExitWithError;
	}

	m_StreamerTexInst.SetTexDef( m_BotInfo_Zom.pSlashStreamerTexDef );

	// Load mesh resource...
	m_pZomBotMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, _ZOMBOT_MESH_NAME );
	if( m_pZomBotMesh == NULL ) {
		DEVPRINTF( "CBotZom::ClassHierarchyLoadSharedResources(): Could not find mesh '%s'\n", _ZOMBOT_MESH_NAME );
		goto _ExitWithError;
	}

	// Create ZombieBot part groups...
	if( !CBotZomPartGroup::CreateGroups( m_BotInfo_Zom.nPartGroupCount, m_pZomBotMesh ) ) {
		DEVPRINTF( "CBotZom::ClassHierarchyLoadSharedResources(): Could not create ZombieBot part groups.\n" );
		goto _ExitWithError;
	}

	return TRUE;

	// Error:
_ExitWithError:
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( ResFrame );

	return FALSE;
}


void CBotZom::ClassHierarchyUnloadSharedResources( void ) {
	FASSERT( m_nBotClassClientCount > 0 );

	--m_nBotClassClientCount;

	if( m_nBotClassClientCount == 0 ) {
		m_AnimStackDef.Destroy();

		CBotZomPartGroup::DestroyGroups();

		m_pZomBotMesh = NULL;
	}

	CBot::ClassHierarchyUnloadSharedResources();
}


CBotZom::CBotZom() : CBot() {
	m_pInventory = NULL;
	m_pWorldMesh = NULL;
}


CBotZom::~CBotZom() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CBotZom::Create( s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName, BOOL bStartInPieces ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	if( !ClassHierarchyLoadSharedResources() ) {
		// Failure! (resources have already been cleaned up, so we can bail now)
		return FALSE;
	}

	// Get pointer to the leaf class's builder object...
	CBotZomBuilder *pBuilder = (CBotZomBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_BotZomBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOTZOM );
	}

	// Set our builder parameters...

	pBuilder->m_bStartFormed = !bStartInPieces;

	// Create an entity...
	return CBot::Create( &m_BotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}


BOOL CBotZom::ClassHierarchyBuild( void ) {
	FMeshInit_t MeshInit;
	s32 nBoneIndex;
	cchar * pszMeshFilename = NULL;

	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CBotZomBuilder *pBuilder = (CBotZomBuilder *)GetLeafClassBuilder();

	// Set input parameters for CBot creation...
	pBuilder->m_pBotDef = &m_BotDef;

	// Build parent class...
	if( !CBot::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	// Initialize from builder object...

	if( pBuilder->m_bStartFormed ) {
		FMATH_SETBITMASK( m_nZomFlags, ZOMFLAG_START_FORMED );
	}

	// Create world mesh...
	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL ) {
		DEVPRINTF( "CBotZom::ClassHierarchyBuild(): Not enough memory to create CFWorldMesh.\n" );
		goto _ExitWithError;
	}

	// Init the world mesh...
	MeshInit.pMesh = m_pZomBotMesh;
	MeshInit.nFlags = 0;
	MeshInit.fCullDist = FMATH_MAX_FLOAT;
	MeshInit.Mtx.Set( pBuilder->m_EC_Mtx_WS );
	m_pWorldMesh->Init( &MeshInit );

	m_pWorldMesh->m_nUser = MESHTYPES_ENTITY;
	m_pWorldMesh->m_pUser = this;
	m_pWorldMesh->SetUserTypeBits( TypeBits() );
	m_pWorldMesh->m_nFlags &= ~(FMESHINST_FLAG_DONT_DRAW | FMESHINST_FLAG_NOCOLLIDE);
	m_pWorldMesh->SetCollisionFlag( TRUE );
	m_pWorldMesh->SetLineOfSightFlag( FALSE );

	m_pWorldMesh->UpdateTracker();
	m_pWorldMesh->RemoveFromWorld();

	// Find interesting bones...
	m_nBoneIndexGroin = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_GROIN] );
	m_nBoneIndexTorso = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_TORSO] );
	m_nBreathParticleBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_NECK_F] );

	if( m_nBoneIndexGroin==-1 || m_nBoneIndexTorso==-1 ) {
		DEVPRINTF( "CBotZom::ClassHierarchyCreate(): Could not find groin or torso bone in mesh.\n" );
		goto _ExitWithError;
	}

	if( m_BotInfo_Zom.pszClawDamageSphereBoneName ) {
		m_nBoneIndexDamageSphere = m_pWorldMesh->FindBone( m_BotInfo_Zom.pszClawDamageSphereBoneName );
	}

	if( m_nBoneIndexDamageSphere == -1 ) {
		DEVPRINTF( "CBotZom::ClassHierarchyCreate(): Bone to attach claw damage sphere was either not specified or was not found.\n" );
		goto _ExitWithError;
	}

	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_L_TOE] );
	if( nBoneIndex >= 0 ) {
		m_pLeftFootDustPos_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;

		nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_R_TOE] );
		if( nBoneIndex >= 0 ) {
			m_pRightFootDustPos_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
		} else {
			m_pLeftFootDustPos_WS = NULL;
		}
	}

	// Build our animation stack and load animations...
	if( !m_Anim.Create( &m_AnimStackDef, m_pWorldMesh ) ) {
		DEVPRINTF( "CBotZom::ClassHierarchyBuild(): Trouble creating m_Anim.\n" );
		goto _ExitWithError;
	}

	if( !m_AnimManFrameAim.Create( m_AnimStackDef.m_nBoneCount, m_AnimStackDef.m_apszBoneNameTable ) ) {
		DEVPRINTF( "CBotZom::ClassHierarchyCreate(): Could not create aim animation summer.\n" );
		goto _ExitWithError;
	}

	_ResetAnimSystem();

	m_Anim.m_pAnimCombiner->SetBoneCallback( &_AnimBoneCallback );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_GROIN] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_TORSO] );

	m_apWeapon[0] = NULL;
	m_apWeapon[1] = NULL;

	SetMaxHealth();

	// Find approx eye point...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CBotZom::ClassHierarchyBuild(): Could not locate approx eye point bone '%s'.\n", m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
		m_pApproxEyePoint_WS = &m_MtxToWorld.m_vPos;
	} else {
		m_pApproxEyePoint_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
	}

	// Find gaze direction...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_NECK_F ] );
	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CBotZom::ClassHierarchyBuild(): Could not locate gaze bone '%s'.\n", m_apszBoneNameTable[ BONE_NECK_F ] );
		m_pGazeDir_WS = &m_MtxToWorld.m_vFront;
	} else {
		m_pGazeDir_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vFront;
	}

	m_fBreathParticleTimer = fmath_RandomFloatRange( m_BotInfo_Zom.fBreathTimeMin, m_BotInfo_Zom.fBreathTimeMax );

	FMATH_CLEARBITMASK( m_pWorldMesh->m_nFlags, FMESHINST_FLAG_NOBONES );

	// Initialize matrix palette (this is important for attached entities)...
	AtRestMatrixPalette();

	// Create tag points...
	if( !TagPoint_CreateFromBoneArray( m_anTagPointBoneNameIndexArray, m_apszBoneNameTable ) ) {
		goto _ExitWithError;
	}

	m_hTexLayerLightning = m_pWorldMesh->GetTexLayerHandle( 50 );
	m_hTexLayerEye = m_pWorldMesh->GetTexLayerHandle( 51 );

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );

	return FALSE;
}


BOOL CBotZom::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CBot::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CBotZom::ClassHierarchyResolveEntityPointerFixups( void ) {
	CBot::ClassHierarchyResolveEntityPointerFixups();

	if( m_nZomFlags & ZOMFLAG_START_FORMED ) {
		// Start this Zombie Bot formed...

		_InitAlive();

		// Tell AI that we're awake...
		m_nSleepState = BOTSLEEPSTATE_NONE;
	} else {
		// Start this Zombie Bot in pieces...

		_InitInPieces();

		// Tell AI that we're sleeping...
		m_nSleepState = BOTSLEEPSTATE_DOZE_LOOP;
	}
}


void CBotZom::ClassHierarchyDestroy( void ) {
	m_Anim.Destroy();

	fdelete( m_pWorldMesh );
	m_pWorldMesh = NULL;

	CBot::ClassHierarchyDestroy();

	_KillStreamer();
}


void CBotZom::_InitInPieces( void ) {
	_InitCommon();
	_ConfigureBotToPieces();
	_ScatterPieces();

	m_fEmissiveIntensity = 0.0f;
	_UpdateEmissiveIntensity();

	m_pWorldMesh->SetShadowIntensityAdjust( 0.0f );
}


void CBotZom::_InitAlive( void ) {
	FMATH_SETBITMASK( m_nZomFlags, ZOMFLAG_DISABLE_AI_WAKE_UP );

	_InitCommon();
	_ConfigureBotToAlive( TRUE );

	m_fEmissiveIntensity = 1.0f;
	_UpdateEmissiveIntensity();

	m_pWorldMesh->SetShadowIntensityAdjust( 1.0f );
}


void CBotZom::_InitCommon( void ) {

	if( m_pPartGroup ) {
		m_pPartGroup->UnassignGroup();
	}

	m_nZomFlags &= (ZOMFLAG_START_FORMED | ZOMFLAG_DISABLE_AI_WAKE_UP);

	m_nHitEntityCount = 0;
	m_hStreamer = FXSTREAMER_INVALID_HANDLE;
	
	// Disable auto-wake timer...
	_SetAutoWakeupTimer( -1.0f );

	// Reset power state...
	m_nPowerState = POWERSTATE_POWERED_UP;
	m_fPowerOffOnTime = 0.0f;
}


CEntityBuilder *CBotZom::GetLeafClassBuilder( void ) {
	return &_BotZomBuilder;
}


void CBotZom::_ClearDataMembers( void ) {

	// Init data members...

	m_pPartGroup = NULL;
	m_nSlashState = SLASH_STATE_NONE;
	m_nZomMode = ZOMMODE_ALIVE;
	m_nZomFlags = ZOMFLAG_NONE;

	m_hTexLayerLightning = FMESH_TEXLAYERHANDLE_INVALID;
	m_hTexLayerEye = FMESH_TEXLAYERHANDLE_INVALID;
	m_fEmissiveIntensity = 1.0f;
	m_fEmissiveAnimAngle = 0.0f;

	_SetAutoWakeupTimer( -1.0f );

	m_fReformNormHealth = NormHealth();

	m_nRemainingDeathCount = m_BotInfo_Zom.nDeathCount;

	m_pMoveIdentifier = &m_nZomFlags;

	m_fCollCylinderRadius_WS = 2.0f;
	m_fCollCylinderHeight_WS = 6.0f;

	m_pBotInfo_Gen = &m_BotInfo_Gen;
	m_pBotInfo_MountAim = &m_BotInfo_MountAim;
	m_pBotInfo_Walk = &m_BotInfo_Walk;
	m_pBotInfo_Jump = &m_BotInfo_Jump;
	m_pBotInfo_Weapon = &m_BotInfo_Weapon;

	m_fGravity = m_pBotInfo_Gen->fGravity;
	m_fMaxFlatSurfaceSpeed_WS = m_pBotInfo_Walk->fMaxXlatVelocity * m_fRunMultiplier;

	m_fMountPitchMax_WS = m_pBotInfo_MountAim->fMountPitchDownLimit;
	m_fMountPitchMin_WS = m_pBotInfo_MountAim->fMountPitchUpLimit;

	m_anAnimStackIndex[ASI_STAND] = ANIMTAP_STAND;

	m_anAnimStackIndex[ASI_WALK] = ANIMTAP_WALK;
	m_anAnimStackIndex[ASI_RUN] = ANIMTAP_RUN;

	m_anAnimStackIndex[ASI_FALL] = ANIMCONTROL_JUMP_FLY;

	m_pnEnableBoneNameIndexTableForSummer_Normal = m_anEnableBoneNameIndexTableForSummer_Normal;
	m_pnEnableBoneNameIndexTableForSummer_TetherShock = NULL;

	m_hStreamer = FXSTREAMER_INVALID_HANDLE;
	
	m_fFormingVolume = 0.0f;
	m_pAudioEmitterForming = NULL;

	m_nBoneIndexGroin = -1;
	m_nBoneIndexTorso = -1;
	m_nBoneIndexDamageSphere = -1;

	m_nHitEntityCount = 0;

	m_nBreathParticleBoneIndex = -1;
	m_fBreathParticleTimer = 0.0f;
}


void CBotZom::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CBot::ClassHierarchyAddToWorld();

	m_pWorldMesh->UpdateTracker();
}


void CBotZom::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	m_pWorldMesh->RemoveFromWorld();

	CBot::ClassHierarchyRemoveFromWorld();

	_KillStreamer();
}


void CBotZom::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	FASSERT( IsCreated() );
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );

	apTrackerSkipList[nTrackerSkipListCount++] = m_pWorldMesh;
}


const CFVec3A *CBotZom::GetApproxEyePoint( void ) const {
	FASSERT( IsCreated() );

	return m_pApproxEyePoint_WS;
}


void CBotZom::_UpdateEmissiveIntensity( void ) {
	if( m_hTexLayerLightning != FMESH_TEXLAYERHANDLE_INVALID ) {
		m_pWorldMesh->LayerAlpha_Set( m_hTexLayerLightning, m_fEmissiveIntensity );
	}

	if( m_hTexLayerEye != FMESH_TEXLAYERHANDLE_INVALID ) {
		m_pWorldMesh->LayerAlpha_Set( m_hTexLayerEye, m_fEmissiveIntensity * fmath_PositiveSin(m_fEmissiveAnimAngle) );
	}
}


void CBotZom::ClassHierarchyWork( void ) {
	FASSERT( m_bSystemInitialized );

	CBot::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}

	if( m_nPossessionPlayerIndex >= 0 ) {
		// Player-controlled ZombieBot...

		if( m_bControls_Human ) {
			CHumanControl *pHumanControl = (CHumanControl *)Controls();

			if( !(m_nZomFlags & ZOMFLAG_WAIT_FOR_FIRE2_RELEASE) ) {
				if( pHumanControl->m_fFire2 > 0.0f ) {
					FMATH_SETBITMASK( m_nZomFlags, ZOMFLAG_WAIT_FOR_FIRE2_RELEASE );

					if( m_nZomMode == ZOMMODE_ALIVE ) {
						CollapseIntoDebris();
						_SetAutoWakeupTimer( -1.0f );
					} else if( m_nZomMode == ZOMMODE_IN_PIECES ) {
						StartForming();
					}
				}
			} else {
				if( pHumanControl->m_fFire2 == 0.0f ) {
					FMATH_CLEARBITMASK( m_nZomFlags, ZOMFLAG_WAIT_FOR_FIRE2_RELEASE );
				}
			}
		}
	}

	if( m_nZomMode != ZOMMODE_IN_PIECES ) {
		// Execute usual bot work stuff...
		_StandardBotWork();
	}

	if( m_pPartGroup ) {
		// Do part group work...
		m_pPartGroup->Work();

		// Move any attached entities...
		RelocateAllChildren();

		if( m_nZomMode == ZOMMODE_CRUMBLING ) {
			// We need to do a few special things for crumbling mode...

			if( m_pPartGroup ) {
				// Part group still active...

				if( m_pPartGroup->m_nRoutedPartsCount == CBotZomPartGroup::m_nPartCount ) {
					// No parts are using the bot animations any longer...

					_ConfigureBotToPieces();
				}
			} else {
				// Part group is done...

				_ConfigureBotToPieces();
			}
		}

		if( m_nZomMode != ZOMMODE_ALIVE ) {
			// Update the bot's bounding sphere based on the location of the bones...
			CBotZomPartGroup::UpdateBotBoundingSphere( this );
		}
	}

	f32 fDeltaTime = FMATH_MIN( FLoop_fPreviousLoopSecs, (1.0f/30.0f) );
	BOOL bNeedShadowWorkNextFrame;

	// Update emissiveness:
	m_fEmissiveAnimAngle += (FMATH_2PI * 1.25f) * fDeltaTime;
	while( m_fEmissiveAnimAngle >= FMATH_2PI ) {
		m_fEmissiveAnimAngle -= FMATH_2PI;
	}

	if( m_nZomMode == ZOMMODE_ALIVE ) {
		if( m_fEmissiveIntensity < 1.0f ) {
			m_fEmissiveIntensity += FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMAX( m_fEmissiveIntensity, 1.0f );
		}
	} else {
		if( m_fEmissiveIntensity > 0.0f ) {
			m_fEmissiveIntensity -= FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fEmissiveIntensity, 0.0f );
		}
	}

	_UpdateEmissiveIntensity();
	_FormingSoundWork();
	_HandleParticles();

	bNeedShadowWorkNextFrame = _ShadowWork();

	if( m_nPossessionPlayerIndex < 0 ) {
		// NPC bot...

		if( m_pPartGroup == NULL ) {
			// No part group...

			if( m_nZomMode == ZOMMODE_IN_PIECES ) {
				// We're in pieces...

				if( m_fSecsUntilAutoWakeup <= 0.0f ) {
					// No wakeup alarm set...

					if( m_fEmissiveIntensity <= 0.0f ) {
						// Emissive materials are completely off...

						if( m_pAudioEmitterForming == NULL ) {
							// Not playing formation sound...

							if( m_fPowerOffOnTime <= 0.0f ) {
								// No power-on timer...

								if( !bNeedShadowWorkNextFrame ) {
									// Not animating our shadow intensity...

									_KillFormingSound();
									_KillStreamer();
									DisableOurWorkBit();
								}
							}
						}
					}
				}
			}
		}
	}

	if( m_fPowerOffOnTime > 0.0f ) {
		m_fPowerOffOnTime -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fPowerOffOnTime, 0.0f );

		// Time is up! Power up the bot...
		if( (m_fPowerOffOnTime == 0.0f) && !IsDead() ) {
			Power( TRUE );
		}
	}

	if( m_nZomMode != ZOMMODE_FORMING ) {
		if( m_pAudioEmitterForming ) {
			_KillFormingSound();
		}
	}

	if( m_nZomMode == ZOMMODE_IN_PIECES ) {
		// Advance wake-up timer when in pieces...

		if( m_fSecsUntilAutoWakeup >= 0.0f ) {
			// Timer is enabled...

			m_fSecsUntilAutoWakeup -= FLoop_fPreviousLoopSecs;

			if( m_fSecsUntilAutoWakeup <= 0.0f ) {
				// Time to wake up the bot...

				if( StartForming() ) {
					// Bot has started forming...

					m_fSecsUntilAutoWakeup = -1.0f;
				} else {
					// Could not wake bot. Try again later...

					m_fSecsUntilAutoWakeup = 0.5f;
				}
			}
		}
	}

	if( IsDeadOrDying() ) {
		if( !m_pPartGroup ) {
			FMATH_CLEARBITMASK( m_uBotDeathFlags, BOTDEATHFLAG_WALKING_DEAD );
		}

		DeathWork();
	}
}


// Returns TRUE if we need to be called next frame.
BOOL CBotZom::_ShadowWork( void ) {
	BOOL bNeedWorkNextFrame;
	f32 fShadowIntensity;

	bNeedWorkNextFrame = FALSE;
	fShadowIntensity = m_pWorldMesh->GetShadowIntensityAdjust();

	if( (m_nZomMode == ZOMMODE_ALIVE) || ((m_nZomMode == ZOMMODE_FORMING) && !m_pPartGroup) ) {
		// Fade in shadow...

		if( fShadowIntensity < 1.0f ) {
			bNeedWorkNextFrame = TRUE;

			fShadowIntensity += FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMAX( fShadowIntensity, 1.0f );

			m_pWorldMesh->SetShadowIntensityAdjust( fShadowIntensity );
		}
	} else {
		// Fade out shadow...

		if( fShadowIntensity > 0.0f ) {
			bNeedWorkNextFrame = TRUE;

			fShadowIntensity -= FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( fShadowIntensity, 0.0f );

			m_pWorldMesh->SetShadowIntensityAdjust( fShadowIntensity );
		}
	}

	return bNeedWorkNextFrame;
}


void CBotZom::_StandardBotWork( void ) {
	const CFVec3A *pSpherePos_WS;
	CFVec3A TempVec3A, PrevSpherePos_WS;

	ParseControls();

	if( (m_nZomMode != ZOMMODE_ALIVE) || (m_nZomMode != ZOMMODE_CRUMBLING) ) {
		m_ImpulseVelocity_WS.Zero();
		FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_VELOCITY_IMPULSE_VALID );
	}

	// Save a copy of the previous frame's info...
	m_MountPrevPos_WS = m_MountPos_WS;
	m_nPrevState = m_nState;

	// Apply velocity impulses that were accumulated last frame...
	m_Velocity_WS.Add( m_ImpulseVelocity_WS );

	// Update position...
	TempVec3A.Mul( m_Velocity_WS, FLoop_fPreviousLoopSecs );
	m_MountPos_WS.Add( TempVec3A );
	m_fMaxDeltaJumpVelY_WS = m_pBotInfo_Jump->fVerticalVelocityJump1 + m_pBotInfo_Jump->fVerticalVelocityJump2;

	// Handle pitch and yaw...
	HandlePitchMovement();
	HandleYawMovement();

	// receive and process head look
	// commands from entity control
	HandleHeadLook();

	// Rotate model space velocity to account for yaw change...
	WS2MS( m_Velocity_MS, m_Velocity_WS );

	// If an external velocity impulse was applied above, recompute our velocities
	// and speeds...
	if( m_nBotFlags & BOTFLAG_VELOCITY_IMPULSE_VALID ) {
		VelocityHasChanged();

		m_ImpulseVelocity_WS.Zero();
		FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_VELOCITY_IMPULSE_VALID );
	}

	// Update translation analog stick vectors from raw controller data...
	ComputeXlatStickInfo();

	// Collide with the world below our feet...
	PROTRACK_BEGINBLOCK("Coll");
		HandleCollision();
	PROTRACK_ENDBLOCK();//"Coll"

	// Move and animate our bot...
	switch( m_nState ) {
	case STATE_GROUND:
		PROTRACK_BEGINBLOCK("GroundXlat");
			HandleGroundTranslation();
			_HandleJumping();
			_HandleHopping();
		PROTRACK_ENDBLOCK();//"GroundXlat");

		PROTRACK_BEGINBLOCK("GroundAnim");
			HandleGroundAnimations();
		PROTRACK_ENDBLOCK();//"GroundAnim");

		break;

	case STATE_AIR:
		PROTRACK_BEGINBLOCK("AirXlat");
			HandleAirTranslation();
			_HandleDoubleJump();
		PROTRACK_ENDBLOCK();//"AirXlat");

		PROTRACK_BEGINBLOCK("AirAnim");
			if( HandleAirAnimations() ) {
				_EnterFlyMode();
			}
		PROTRACK_ENDBLOCK();//"AirAnim");

		break;
	}

	PROTRACK_BEGINBLOCK("CommonAnim");
		_HandleJumpAnimations();
		HandleHipsAnimation();
	PROTRACK_ENDBLOCK();//"CommonAnim");
 
	PROTRACK_BEGINBLOCK("Weapon");
		_HandleWeaponFiring();
		_HandleWeaponAnimations();
	PROTRACK_ENDBLOCK();//"WeaponAnim");

	_HandleFormingAnim();

	if( m_nZomFlags & ZOMFLAG_SLASHING ) {
		pSpherePos_WS = &m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIndexDamageSphere ]->m_vPos;
		PrevSpherePos_WS = *pSpherePos_WS;
	}

	_UpdateMatrices();

	if( m_nZomFlags & ZOMFLAG_SLASHING ) {
		_PerformSlashDamage( &PrevSpherePos_WS, pSpherePos_WS );
	}
}


void CBotZom::_HandleParticles( void ) {
	if( m_nZomMode == ZOMMODE_ALIVE ) {
		if( m_fSpeed_WS < 10.0f ) {
			if( m_BotInfo_Zom.hBreathParticle != FPARTICLE_INVALID_HANDLE ) {
				if( m_nBreathParticleBoneIndex >= 0 ) {
					m_fBreathParticleTimer -= FLoop_fPreviousLoopSecs;
					if( m_fBreathParticleTimer <= 0.0f ) {
						m_fBreathParticleTimer = fmath_RandomFloatRange( m_BotInfo_Zom.fBreathTimeMin, m_BotInfo_Zom.fBreathTimeMax );

						CEParticle *pEParticle;

						pEParticle = eparticlepool_GetEmitter();
						if( pEParticle ) {
							CFMtx43A Mtx;

							pEParticle->StartEmission( m_BotInfo_Zom.hBreathParticle, 1.0f, fmath_RandomFloatRange( 0.2f, 0.3f ) );

							_ComputeBreathMatrix( &Mtx );

							pEParticle->Relocate_RotXlatFromScaledMtx_WS_NewScale_WS( &Mtx, Mtx.m_vX.Mag(), 1.0f, FALSE );
							pEParticle->Attach_ToParent_WS( this, m_apszBoneNameTable[BONE_NECK_F], FALSE );

							_PlayBreathPuffSound( &Mtx.m_vPos );
						}
					}
				}
			}
		}
	}
}


void CBotZom::_ComputeBreathMatrix( CFMtx43A *pMtx ) {
	FASSERT( m_nBreathParticleBoneIndex >= 0 );

	CFVec3A DisplacementVec_BS;
	CFQuatA RotQuat;
	CFMtx43A *pBoneMtx;

	DisplacementVec_BS.Set( 0.0f, -0.25f, 0.25f );
	pBoneMtx = m_pWorldMesh->GetBoneMtxPalette()[m_nBreathParticleBoneIndex];

	RotQuat.BuildQuat( pBoneMtx->m_vX, FMATH_DEG2RAD( 20.0f ) );

	RotQuat.MulPoint( pMtx->m_vX, pBoneMtx->m_vX );
	RotQuat.MulPoint( pMtx->m_vY, pBoneMtx->m_vY );
	RotQuat.MulPoint( pMtx->m_vZ, pBoneMtx->m_vZ );

	pBoneMtx->MulPoint( pMtx->m_vPos, DisplacementVec_BS );
}


void CBotZom::_HandleFormingAnim( void ) {
	// Advance forming animation...
	if( m_nZomFlags & ZOMFLAG_PLAY_FORM_ANIM ) {
		f32 fAnimUnitTime, fAnimUnitControl;

		DeltaTime( ANIMTAP_FORM, 0.35f * FLoop_fPreviousLoopSecs, TRUE );
		fAnimUnitTime = GetUnitTime( ANIMTAP_FORM );

		if( fAnimUnitTime >= 0.5f ) {
			fAnimUnitControl = GetControlValue( ANIMCONTROL_FORM );

			fAnimUnitControl -= 1.0f * FLoop_fPreviousLoopSecs;
			if( fAnimUnitControl <= 0.0f ) {
				fAnimUnitControl = 0.0f;

				FMATH_CLEARBITMASK( m_nZomFlags, ZOMFLAG_PLAY_FORM_ANIM );

				MobilizeBot();
			}

			SetControlValue( ANIMCONTROL_FORM, fAnimUnitControl );
		}
	}

	if( m_nZomMode == ZOMMODE_FORMING ) {
		if( m_pPartGroup == NULL ) {
			if( _DoesNormalBoundSphereIntersectAnotherBot() ) {
				// We can't start forming because another bot's bounding
				// sphere is occupying the same space as ours...

				_FadeOutFormingSound();

				CollapseIntoDebris();

				if( NormHealth() > 0.0f ) {
					_SetAutoWakeupTimer( m_BotInfo_Zom.fReformInterruptionStayAsleepSecs );
				}
			} else {
				// It's safe to turn on bot/bot collisions...

				_ConfigureBotToAlive( !(m_nZomFlags & ZOMFLAG_PLAY_FORM_ANIM) );

				m_fBreathParticleTimer = 0.0f;

				if( !(m_nZomFlags & ZOMFLAG_LOWER_FORMING_VOLUME) ) {
					_FadeOutFormingSound();
				}
			}
		}
	}
}


void CBotZom::_PerformSlashDamage( const CFVec3A *pPrevSpherePos_WS, const CFVec3A *pCurrentSpherePos_WS ) {
	CFTrackerCollideProjSphereInfo TrackerCollideInfo;

	m_CollInfo.nCollTestType = FMESH_COLLTESTTYPE_PROJSPHERE;
	m_CollInfo.nCollMask = FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES;
	m_CollInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
	m_CollInfo.nTrackerUserTypeBitsMask = -1;
	m_CollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_CHECK_ALL;
	m_CollInfo.bFindClosestImpactOnly = FALSE;
	m_CollInfo.bCullBacksideCollisions = FALSE;
	m_CollInfo.bCalculateImpactData = TRUE;
	m_CollInfo.ProjSphere.Init( pPrevSpherePos_WS, pCurrentSpherePos_WS, m_BotInfo_Zom.fClawDamageSphereRadius * m_fScaleToWorld );

	TrackerCollideInfo.pProjSphere = &m_CollInfo.ProjSphere;
	TrackerCollideInfo.pCallback = _SlashHitTrackerCallback;
	TrackerCollideInfo.nTrackerTypeBits = FWORLD_TRACKERTYPEBIT_MESH;
	TrackerCollideInfo.nTrackerUserTypeBitsMask = -1;
	TrackerCollideInfo.bIgnoreCollisionFlag = FALSE;
	TrackerCollideInfo.nTrackerSkipCount = 0;
	TrackerCollideInfo.ppTrackerSkipList = NULL;

	_aZomSlashHits.clear();
	m_pCollBot = this;
	fworld_CollideWithTrackers( &TrackerCollideInfo );

	for( size_t i = 0; i < _aZomSlashHits.size(); ++i ) {
		const _ZomSlashHit_t &Hit = _aZomSlashHits[i];
		_SpawnSlashImpactEffects( &Hit.Impact );
		if( Hit.bApplyDamage )
			_InflictSlashDamage( Hit.pEntity, Hit.pWorldMesh, &Hit.Impact );
	}
	_aZomSlashHits.clear();

	fcoll_Clear();

	if( fworld_CollideWithWorldTris( &m_CollInfo ) ) {
		_SpawnSlashImpactEffects( &FColl_aImpactBuf[0] );

		if( !(m_nZomFlags & ZOMFLAG_SLASH_HIT_TERRAIN) ) {
			// Haven't hit any terrain yet...

			_InflictSlashDamage( NULL, NULL, &FColl_aImpactBuf[0] );

			FMATH_SETBITMASK( m_nZomFlags, ZOMFLAG_SLASH_HIT_TERRAIN );
		}
	}
}


BOOL CBotZom::_SlashHitTrackerCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) {
	CBotZom *pBotZom = (CBotZom *)m_pCollBot;
	CFWorldMesh *pWorldMesh = (CFWorldMesh *)pTracker;

	if( pTracker->m_nUser != MESHTYPES_ENTITY ) {
		// Tracker is a non-entity...

		fcoll_Clear();

		if( pWorldMesh->CollideWithMeshTris( &m_CollInfo ) ) {
			// Defer effects and damage until the tracker traversal completes; both can change its lists.
			_ZomSlashHit_t Hit;
			Hit.pEntity = NULL;
			Hit.pWorldMesh = pWorldMesh;
			Hit.Impact = FColl_aImpactBuf[0];
			Hit.bApplyDamage = !(pBotZom->m_nZomFlags & ZOMFLAG_SLASH_HIT_TERRAIN);
			_aZomSlashHits.push_back( Hit );

			if( Hit.bApplyDamage )
				FMATH_SETBITMASK( pBotZom->m_nZomFlags, ZOMFLAG_SLASH_HIT_TERRAIN );
		}

		return TRUE;
	}

	// This tracker is an entity...

	CEntity *pEntity = (CEntity *)pTracker->m_pUser;

	if( pEntity == (CEntity *)pBotZom ) {
		// We hit ourselves...
		return TRUE;
	}

	if( pEntity->TypeBits() & ENTITY_BIT_BOTZOM ) {
		// Entity is another CBotZom...

		if( pBotZom->m_nPossessionPlayerIndex < 0 ) {
			// AI is controlling the attacking ZombieBot...

			return TRUE;
		}
	}

	fcoll_Clear();

	if( !pWorldMesh->CollideWithMeshTris( &m_CollInfo ) ) {
		// We did not hit any triangles on the entity...
		return TRUE;
	}

	// Record the hit for processing once tracker-list traversal is complete.
	_ZomSlashHit_t Hit;
	Hit.pEntity = pEntity;
	Hit.pWorldMesh = pWorldMesh;
	Hit.Impact = FColl_aImpactBuf[0];
	Hit.bApplyDamage = FALSE;

	if( pBotZom->m_nHitEntityCount < HIT_ENTITY_MAX_COUNT ) {
		u32 i;
		for( i = 0; i < pBotZom->m_nHitEntityCount; ++i ) {
			if( pBotZom->m_apHitEntity[i] == pEntity )
				break;
		}

		if( i == pBotZom->m_nHitEntityCount ) {
			Hit.bApplyDamage = TRUE;

			// Track the owner for weapon hits so the same bot cannot be damaged twice by its weapon mesh.
			if( pEntity->TypeBits() & ENTITY_BIT_WEAPON ) {
				CWeapon *pWeapon = (CWeapon *)pEntity;
				if( pWeapon->GetOwner() && pWeapon->GetBotDamageBoneIndex() >= 0 )
					pEntity = pWeapon->GetOwner();
			}
			pBotZom->m_apHitEntity[ pBotZom->m_nHitEntityCount++ ] = pEntity;
		}
	}

	_aZomSlashHits.push_back( Hit );

	return TRUE;
}


// Applies a velocity impulse in model space. The impulse will be applied first thing next frame.
void CBotZom::ApplyVelocityImpulse_MS( const CFVec3A &rVelocityImpulseVec_MS ) {
	if( m_nZomMode == ZOMMODE_ALIVE ) {
		if( !IsPlayerOutOfBody() ) {
			CFVec3A VelocityImpulseVec_WS;

			m_ImpulseVelocity_WS.Add( MS2WS( VelocityImpulseVec_WS, rVelocityImpulseVec_MS ) );
			FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_VELOCITY_IMPULSE_VALID );
		}
	}
}


// Applies a velocity impulse in world space. The impulse will be applied first thing next frame.
void CBotZom::ApplyVelocityImpulse_WS( const CFVec3A &rVelocityImpulseVec_WS ) {
	if( m_nZomMode == ZOMMODE_ALIVE ) {
		if( !IsPlayerOutOfBody() ) {
			m_ImpulseVelocity_WS.Add( rVelocityImpulseVec_WS );
			FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_VELOCITY_IMPULSE_VALID );
		}
	}
}


void CBotZom::_InflictSlashDamage( CEntity *pHitEntity, CFWorldMesh *pHitWorldMesh, const FCollImpact_t *pCollImpact ) {
	if( pHitEntity ) {
		// Inflict damage to the entity...

		// Get an empty damage form...
		CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

		if( pDamageForm ) {
			// Fill out the form...

			CFVec3A FireUnitDir_WS;
			f32 fMag2;

			FireUnitDir_WS.Sub( m_CollInfo.ProjSphere.m_vCenterEnd_WS, m_CollInfo.ProjSphere.m_vCenterStart_WS );

			fMag2 = FireUnitDir_WS.MagSq();
			if( fMag2 >= 0.000001f ) {
				FireUnitDir_WS.Mul( fmath_InvSqrt( fMag2 ) );
			} else {
				FireUnitDir_WS = CFVec3A::m_UnitAxisY;
			}

			pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
			pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
			pDamageForm->m_pDamageProfile = m_BotInfo_Zom.pDamageProfileSlash;
			pDamageForm->m_Damager.pWeapon = NULL;
			pDamageForm->m_Damager.pBot = this;
			pDamageForm->m_Damager.nDamagerPlayerIndex = m_nPossessionPlayerIndex;
			pDamageForm->m_pDamageeEntity = pHitEntity;
			pDamageForm->InitTriDataFromCollImpact( pHitWorldMesh, pCollImpact, &FireUnitDir_WS );

			CDamage::SubmitDamageForm( pDamageForm );
		}

		if( pHitEntity->TypeBits() & ENTITY_BIT_BOT ) {
			CBot *pHitBot = (CBot *)pHitEntity;

			CFVec3A ImpulseVec_WS;
			f32 fMag2;

			ImpulseVec_WS.Sub( pHitBot->MtxToWorld()->m_vPos, MtxToWorld()->m_vPos );
			ImpulseVec_WS.y = 0.0f;
			fMag2 = ImpulseVec_WS.MagSq();

			if( fMag2 > 0.00001f ) {
				ImpulseVec_WS.Mul( fmath_InvSqrt(fMag2) * 30.0f );

				pHitBot->ApplyVelocityImpulse_WS( ImpulseVec_WS );
			}
		}
	}

	if( pHitEntity && !pHitEntity->IsInvincible() ) {
		// Hit a damageable entity...
		PlaySound( m_BotInfo_Zom.pSoundGroupHitDamageable );
	} else {
		// Hit an invincible entity...
		PlaySound( m_BotInfo_Zom.pSoundGroupHitNonDamageable );
	}
}


void CBotZom::_SpawnSlashImpactEffects( const FCollImpact_t *pCollImpact ) {
	const CGCollMaterial *pMaterial = CGColl::GetMaterial( pCollImpact );

	if( pMaterial->CanDrawParticle( CGCollMaterial::PARTICLE_TYPE_SPARKS_BURST ) ) {
		pMaterial->DrawParticle(
			CGCollMaterial::PARTICLE_TYPE_SPARKS_BURST,
			&pCollImpact->ImpactPoint,
			&pCollImpact->UnitFaceNormal,
			1.0f
		);
	}

	if( pMaterial->CanDrawParticle( CGCollMaterial::PARTICLE_TYPE_BITS ) ) {
		pMaterial->DrawParticle(
			CGCollMaterial::PARTICLE_TYPE_BITS,
			&pCollImpact->ImpactPoint,
			&pCollImpact->UnitFaceNormal,
			1.0f
		);
	}

	if( pMaterial->HasLooseDust() ) {
		if( pMaterial->CanDrawParticle( CGCollMaterial::PARTICLE_TYPE_DUST ) ) {
			pMaterial->DrawParticle(
				CGCollMaterial::PARTICLE_TYPE_DUST,
				&pCollImpact->ImpactPoint,
				&pCollImpact->UnitFaceNormal,
				1.0f
			);
		}
	}

	if( pMaterial->CanDrawDebris( CGCollMaterial::DEBRIS_GROUP_MEDIUM ) ) {
		CFDebrisSpawner DebrisSpawner;
		DebrisSpawner.InitToDefaults();

		DebrisSpawner.m_Mtx.m_vPos = pCollImpact->ImpactPoint;
		DebrisSpawner.m_Mtx.m_vZ = pCollImpact->UnitFaceNormal;
		DebrisSpawner.m_nEmitterType = CFDebrisSpawner::EMITTER_TYPE_POINT;
		DebrisSpawner.m_pDebrisGroup = pMaterial->m_apDebrisGroup[ CGCollMaterial::DEBRIS_GROUP_MEDIUM ];
		DebrisSpawner.m_fSpawnerAliveSecs = 0.0f;
		DebrisSpawner.m_fMinSpeed = 10.0f;
		DebrisSpawner.m_fMaxSpeed = 20.0f;
		DebrisSpawner.m_fUnitDirSpread = 0.2f;
		DebrisSpawner.m_fScaleMul = 1.0f;
		DebrisSpawner.m_fGravityMul = 1.0f;
		DebrisSpawner.m_fRotSpeedMul = 1.0f;
		DebrisSpawner.m_nMinDebrisCount = 1;
		DebrisSpawner.m_nMaxDebrisCount = 3;

		CGColl::SpawnDebris( &DebrisSpawner );
	}
}


void CBotZom::_HandleDoubleJump( void ) {
	if( m_bControls_Jump && m_Velocity_WS.y>=0.0f ) {
		if( m_nJumpState==BOTJUMPSTATE_LAUNCH || m_nJumpState==BOTJUMPSTATE_AIR ) {
			_StartDoubleJump();
		}
	}
}


BOOL CBotZom::Hop( BOOL bHopLeft ) {
	if( m_nPowerState != POWERSTATE_POWERED_UP ) {
		// We're not powered up...
		return FALSE;
	}

	if( (m_nRemainingDeathCount == 0) || (NormHealth() <= 0.0f) ) {
		// We're dead...
		return FALSE;
	}

	if( m_nZomMode != ZOMMODE_ALIVE ) {
		return FALSE;
	}

	if( m_nState != STATE_GROUND ) {
		return FALSE;
	}

	if( m_nMoveState != BOTMOVESTATE_NONE ) {
		return FALSE;
	}

	if( m_nJumpState != BOTJUMPSTATE_NONE ) {
		return FALSE;
	}

	CFVec3A Impulse_WS;

	Impulse_WS = MtxToWorld()->m_vRight;

	if( bHopLeft ) {
		Impulse_WS.Negate();
	}

	Impulse_WS.Add( MtxToWorld()->m_vUp );
	Impulse_WS.Mul( m_pBotInfo_Walk->fHopLRImpulseMag );

	_StartVelocityJump( &Impulse_WS );

	// Initialize the parts in the group...
	if( !CBotZomPartGroup::InitState_Hopping( this ) ) {
		// No free part groups...
		return FALSE;
	}

	return TRUE;
}


void CBotZom::_HandleHopping( void ) {
	if( m_nControlsBot_Flags & (CBotControl::FLAG_HOP_LEFT | CBotControl::FLAG_HOP_RIGHT) ) {
		// Start hopping...

		if( m_nControlsBot_Flags & CBotControl::FLAG_HOP_LEFT ) {
			Hop( TRUE );
		} else {
			Hop( FALSE );
		}
	}
}


void CBotZom::_HandleJumping( void ) {
	if( !m_bControlsBot_JumpVec ) {
		if( m_bControlsBot_Jump2 ) {
			_StartSingleJump();
			_StartDoubleJump();
		} else if( m_bControls_Jump ) {
			_StartSingleJump();
		}
	} else {
		// Velocity jump specified...
		_StartVelocityJump( &m_ControlsBot_JumpVelocity_WS );
	}

	if( m_nPrevState == STATE_AIR ) {
		if( m_nJumpState != BOTJUMPSTATE_NONE ) {
			_JumpLanded();
		}
	}
}


void CBotZom::_EnterFlyMode( void ) {
	m_bPlayLandAnim = FALSE;
	m_bPlayLandAnimLower = FALSE;

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );

	m_nJumpState = BOTJUMPSTATE_AIR;
	SetJumping();
}


void CBotZom::_StartVelocityJump( const CFVec3A *pJumpVelocity_WS ) {
	CFVec3A JumpVelocity_WS = *pJumpVelocity_WS;
	FMATH_CLAMP( JumpVelocity_WS.y, 0.0f, m_fMaxDeltaJumpVelY_WS );

	_StartSingleJump( &JumpVelocity_WS );

	if( pJumpVelocity_WS->y >= m_pBotInfo_Jump->fVerticalVelocityJump1 ) {
		_StartDoubleJump( FALSE );
	}
}


void CBotZom::_StartSingleJump( const CFVec3A *pJumpVelocity_WS ) {
	if( pJumpVelocity_WS == NULL ) {
		m_Velocity_WS.y += m_pBotInfo_Jump->fVerticalVelocityJump1 * m_fJumpMultiplier;
		m_Velocity_MS.y += m_pBotInfo_Jump->fVerticalVelocityJump1 * m_fJumpMultiplier;
	} else {
		m_Velocity_WS.x = pJumpVelocity_WS->x;
		m_Velocity_WS.y += pJumpVelocity_WS->y;
		m_Velocity_WS.z = pJumpVelocity_WS->z;
		WS2MS( m_Velocity_MS, m_Velocity_WS );
		VelocityHasChanged();
	}

	m_bPlayLandAnim = FALSE;
	m_bPlayLandAnimLower = FALSE;

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );

	ZeroTime( ANIMTAP_JUMP_LAUNCH );

	m_fUnitJump1Jump2Blend = 0.0f;
	m_nJumpState = BOTJUMPSTATE_LAUNCH;
	SetJumping();
}


void CBotZom::_StartDoubleJump( BOOL bAddToVelocityY ) {
	return;
}


void CBotZom::_JumpLanded( void ) {
	MakeFootImpactDust( TRUE, TRUE, m_pBotInfo_Gen->fUnitDustKickup, &m_FeetToGroundCollImpact.UnitFaceNormal );

	m_nJumpState = BOTJUMPSTATE_NONE;
	ClearJumping();
	m_fFlipPitch = 0.0f;
	m_bPlayLandAnim = TRUE;

	ZeroTime( ANIMTAP_JUMP_LAND_UPPER );

	m_fMaxLandUnitBlend = GetControlValue( ANIMCONTROL_JUMP_FLY );
	if( m_fMaxLandUnitBlend == 0.0f ) {
		m_fMaxLandUnitBlend = 1.0f;
	}

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, m_fMaxLandUnitBlend );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );

	if ( m_nPossessionPlayerIndex >= 0 ) {
		// Don't play scuff sounds when landing from a jump...

		if( m_pBotInfo_Sound && m_nSurfaceTypeOn != SURFACE_TYPE_NONE ) {
			fsndfx_Play2D(
				m_pBotInfo_Sound->ahStepSound[m_nSurfaceTypeOn][ fmath_RandomRange( 0, ( BOTSOUND_SCUFF_SOUND_START - 1 ) ) ],
				m_pBotInfo_Walk->fJumpLandedSoundUnitVolume_2D * m_fStepVolumeScale
			);
		}

		// Tell the AI that this sound happened...
		AIEnviro_BoostPlayerSoundTo( m_nPossessionPlayerIndex, 35.0f );
	} else {
		if( m_pBotInfo_Sound && m_nSurfaceTypeOn != SURFACE_TYPE_NONE ) {
			// Don't play scuff sounds when landing from a jump...

			fsndfx_Play3D(
				m_pBotInfo_Sound->ahStepSound[m_nSurfaceTypeOn][ fmath_RandomRange( 0, ( BOTSOUND_SCUFF_SOUND_START - 1 ) ) ],
				&MtxToWorld()->m_vPos,
				m_pBotInfo_Walk->fJumpLandedSoundRadius_3D,
				1.0f,
				m_pBotInfo_Walk->fJumpLandedSoundUnitVolume_3D
			);
		}
	}

	if( m_fSpeedXZ_WS == 0.0f ) {
		SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, m_fMaxLandUnitBlend );
		m_bPlayLandAnimLower = TRUE;
	} else {
		SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
		m_bPlayLandAnimLower = FALSE;
	}
}


void CBotZom::_HandleJumpAnimations( void ) {
	f32 fTemp, fUnitTime;

	if( m_nState == STATE_GROUND ) {
		// We're on the ground...
		if( m_fUnitFlyBlend > 0.0f ) {
			m_fUnitFlyBlend = 0.0f;
			_JumpLanded();
		}

		// Handle landing animation...
		if( m_bPlayLandAnim ) {
			if( m_fSpeedXZ_WS > 0.0f ) {
				fTemp = m_pBotInfo_Jump->fMovingLandAnimSpeedMult;
			} else {
				fTemp = 1.0f;
			}

			if( !DeltaTime( ANIMTAP_JUMP_LAND_UPPER, FLoop_fPreviousLoopSecs * fTemp ) ) {
				// Still playing landing animation...

				f32 fUnitTime = 1.01f - GetUnitTime(ANIMTAP_JUMP_LAND_UPPER);
				fUnitTime = m_fMaxLandUnitBlend * fmath_UnitLinearToSCurve( fUnitTime );
				SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, fUnitTime );

				if( m_bPlayLandAnimLower ) {
					SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, fUnitTime );
				}
			} else {
				// Done playing landing animation...

				m_bPlayLandAnim = FALSE;
				m_bPlayLandAnimLower = FALSE;

				SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );
			}
		} else {
			if( m_nJumpState == BOTJUMPSTATE_NONE ) {
				SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
			}
		}
	} else {
		// We're in the air...

		if( m_nJumpState != BOTJUMPSTATE_NONE ) {
			switch( m_nJumpState ) {
			case BOTJUMPSTATE_LAUNCH:
				if( !DeltaTime( ANIMTAP_JUMP_LAUNCH ) ) {
					fUnitTime = 6.0f * fmath_UnitLinearToSCurve( GetUnitTime( ANIMTAP_JUMP_LAUNCH ) );
					FMATH_CLAMPMAX( fUnitTime, 1.0f );
					SetControlValue( ANIMCONTROL_JUMP_LAUNCH, fUnitTime );
				} else {
					SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );
					SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
					ZeroTime( ANIMTAP_JUMP_FLY );
					m_nJumpState = BOTJUMPSTATE_AIR;
					SetJumping();
				}

				break;

			case BOTJUMPSTATE_AIR:
			case BOTJUMPSTATE_AIR2:
				DeltaTime( ANIMTAP_JUMP_FLY );
				break;

			default:
				FASSERT_NOW;
			}
		}
	}
}


void CBotZom::_InitStartSwingData( void ) {
	m_nHitEntityCount = 0;
	FMATH_CLEARBITMASK( m_nZomFlags, (ZOMFLAG_SLASH_HIT_TERRAIN | ZOMFLAG_SLASHING) );
}


void CBotZom::_StartSlashMode( void ) {
	FMATH_SETBITMASK( m_nZomFlags, ZOMFLAG_SLASHING );
	_StartStreamer();

	PlaySound( m_BotInfo_Zom.pSoundGroupSlash );
}


void CBotZom::_StopSlashMode( void ) {
	FMATH_CLEARBITMASK( m_nZomFlags, ZOMFLAG_SLASHING );
	_KillStreamer();
}


void CBotZom::_HandleSlashingSweetRange( f32 fPrevUnitTime, f32 fCurrentUnitTime, f32 fUnitTimeSweetStart, f32 fUnitTimeSweetEnd ) {
	if( fPrevUnitTime<fUnitTimeSweetStart && fCurrentUnitTime>=fUnitTimeSweetStart ) {
		_StartSlashMode();
	}

	if( fPrevUnitTime<fUnitTimeSweetEnd && fCurrentUnitTime>=fUnitTimeSweetEnd ) {
		_StopSlashMode();
	}
}


void CBotZom::_AttachAnimAndSetBoneMaskToUpperBody( AnimTap_e nTapIndex, Anim_e nAnimIndex ) {
	AttachAnim( nTapIndex, &m_Anim.m_pLoadedBaseAnimInst[ nAnimIndex ] );
	UpdateBoneMask( nTapIndex, m_aBoneEnableIndices_UpperBody, TRUE );

	if( nTapIndex == ANIMTAP_SLASH_FB ) {
		AttachAnim( ANIMTAP_SLASH_FB_LOWER, &m_Anim.m_pLoadedBaseAnimInst[ nAnimIndex ] );
		UpdateBoneMask( ANIMTAP_SLASH_FB_LOWER, m_aBoneEnableIndices_LowerBody, TRUE );
	} else if( nTapIndex == ANIMTAP_SLASH_D ) {
		AttachAnim( ANIMTAP_SLASH_D_LOWER, &m_Anim.m_pLoadedBaseAnimInst[ nAnimIndex ] );
		UpdateBoneMask( ANIMTAP_SLASH_D_LOWER, m_aBoneEnableIndices_LowerBody, TRUE );
	}
}


void CBotZom::_HandleFiringAnimations( void ) {
	f32 fUnitControlValue, fPrevUnitTime, fCurrentUnitTime, fDeltaTime;

	if( m_nRemainingDeathCount == 1 ) {
		// This is our last life. Make us twice as mean!
		fDeltaTime = 2.0f * FLoop_fPreviousLoopSecs;
	} else {
		// Normal meanness...
		fDeltaTime = FLoop_fPreviousLoopSecs;
	}

	switch( m_nSlashState ) {
	case SLASH_STATE_NONE:
		break;

	case SLASH_STATE_F_FROM_REST:
	case SLASH_STATE_B_FROM_REST:
		// Blend in the animation...
		fUnitControlValue = GetControlValue( ANIMCONTROL_SLASH_FB );

		if( fUnitControlValue < 1.0f ) {
			fUnitControlValue += m_pBotInfo_Weapon->fFire1BlendInSpeed * fDeltaTime;
			FMATH_CLAMPMAX( fUnitControlValue, 1.0f );
			SetControlValue( ANIMCONTROL_SLASH_FB, fUnitControlValue );
			SetControlValue( ANIMCONTROL_SLASH_FB_LOWER, fUnitControlValue );
		}

		// Advance the animation...
		if( DeltaTime( ANIMTAP_SLASH_FB, fDeltaTime, TRUE ) ) {
			// Animation complete...

			if( fUnitControlValue == 1.0f ) {
				// We're done with this state...

				if( m_nSlashState == SLASH_STATE_F_FROM_REST ) {
					m_nSlashState = SLASH_STATE_F;
					_AttachAnimAndSetBoneMaskToUpperBody( ANIMTAP_SLASH_FB, ANIM_SLASH_F );
				} else {
					m_nSlashState = SLASH_STATE_B;
					_AttachAnimAndSetBoneMaskToUpperBody( ANIMTAP_SLASH_FB, ANIM_SLASH_B );
				}

				ZeroTime( ANIMTAP_SLASH_FB );
			}
		}

		break;

	case SLASH_STATE_F:
	case SLASH_STATE_B:
		fPrevUnitTime = GetUnitTime( ANIMTAP_SLASH_FB );

		// Advance the animation...
		if( !DeltaTime( ANIMTAP_SLASH_FB, fDeltaTime, TRUE ) ) {
			// Animation still in progress...

			fCurrentUnitTime = GetUnitTime( ANIMTAP_SLASH_FB );

			if( m_nSlashState == SLASH_STATE_F ) {
				_HandleSlashingSweetRange( fPrevUnitTime, fCurrentUnitTime, 0.33f, 0.72f );
			} else {
				_HandleSlashingSweetRange( fPrevUnitTime, fCurrentUnitTime, 0.33f, 0.66f );
			}
		} else {
			// Animation complete...

			_StopSlashMode();

			if( m_fControls_Fire1 == 0.0f ) {
				// No more slashing...

				if( m_nSlashState == SLASH_STATE_F ) {
					m_nSlashState = SLASH_STATE_F_TO_REST;
					_AttachAnimAndSetBoneMaskToUpperBody( ANIMTAP_SLASH_FB, ANIM_SLASH_F_TO_REST );
				} else {
					m_nSlashState = SLASH_STATE_B_TO_REST;
					_AttachAnimAndSetBoneMaskToUpperBody( ANIMTAP_SLASH_FB, ANIM_SLASH_B_TO_REST );
				}

				ZeroTime( ANIMTAP_SLASH_FB );

				break;
			}

			// Start another slash...

			_InitStartSwingData();

			if( fmath_RandomChance( 0.65f ) ) {
				// Start another slash...

				if( m_nSlashState == SLASH_STATE_F ) {
					m_nSlashState = SLASH_STATE_B;
					_AttachAnimAndSetBoneMaskToUpperBody( ANIMTAP_SLASH_FB, ANIM_SLASH_B );
				} else {
					m_nSlashState = SLASH_STATE_F;
					_AttachAnimAndSetBoneMaskToUpperBody( ANIMTAP_SLASH_FB, ANIM_SLASH_F );
				}

				ZeroTime( ANIMTAP_SLASH_FB );
			} else {
				// Start a down slash...

				m_nSlashState = SLASH_STATE_D_FROM_FB;

				_AttachAnimAndSetBoneMaskToUpperBody( ANIMTAP_SLASH_D, ANIM_SLASH_D_FROM_REST );
				ZeroTime( ANIMTAP_SLASH_D );
			}
		}

		break;

	case SLASH_STATE_F_TO_REST:
	case SLASH_STATE_B_TO_REST:
		// Blend out the animation...
		fUnitControlValue = GetControlValue( ANIMCONTROL_SLASH_FB );

		if( fUnitControlValue > 0.0f ) {
			fUnitControlValue -= m_pBotInfo_Weapon->fFire1BlendOutSpeed * fDeltaTime;
			FMATH_CLAMPMIN( fUnitControlValue, 0.0f );
			SetControlValue( ANIMCONTROL_SLASH_FB, fUnitControlValue );
			SetControlValue( ANIMCONTROL_SLASH_FB_LOWER, fUnitControlValue );
		}

		// Advance the animation...
		if( DeltaTime( ANIMTAP_SLASH_FB, fDeltaTime, TRUE ) ) {
			// Animation complete...

			if( fUnitControlValue == 0.0f ) {
				// We're done with this state...

				m_nSlashState = SLASH_STATE_NONE;
			}
		}

		break;

	case SLASH_STATE_D_FROM_REST:
	case SLASH_STATE_D_FROM_FB:
		// Blend in the downward slash animation...
		fUnitControlValue = GetControlValue( ANIMCONTROL_SLASH_D );

		if( fUnitControlValue < 1.0f ) {
			fUnitControlValue += m_pBotInfo_Weapon->fFire1BlendInSpeed * fDeltaTime;
			FMATH_CLAMPMAX( fUnitControlValue, 1.0f );
			SetControlValue( ANIMCONTROL_SLASH_D, fUnitControlValue );
			SetControlValue( ANIMCONTROL_SLASH_D_LOWER, fUnitControlValue );
		}

		// Advance the animation...
		if( DeltaTime( ANIMTAP_SLASH_D, fDeltaTime, TRUE ) ) {
			// Animation complete...

			if( fUnitControlValue == 1.0f ) {
				// We're done with this state...

				SetControlValue( ANIMCONTROL_SLASH_FB, 0.0f );
				SetControlValue( ANIMCONTROL_SLASH_FB_LOWER, 0.0f );

				m_nSlashState = SLASH_STATE_D;

				_AttachAnimAndSetBoneMaskToUpperBody( ANIMTAP_SLASH_D, ANIM_SLASH_D );
				ZeroTime( ANIMTAP_SLASH_D );
			}
		}

		break;

	case SLASH_STATE_D:
		fPrevUnitTime = GetUnitTime( ANIMTAP_SLASH_D );

		// Advance the animation...
		if( !DeltaTime( ANIMTAP_SLASH_D, fDeltaTime, TRUE ) ) {
			// Animation still in progress...

			fCurrentUnitTime = GetUnitTime( ANIMTAP_SLASH_D );
			_HandleSlashingSweetRange( fPrevUnitTime, fCurrentUnitTime, 0.30f, 0.66f );
		} else {
			// Animation complete...

			_StopSlashMode();

			if( m_fControls_Fire1 == 0.0f ) {
				// No more slashing...

				m_nSlashState = SLASH_STATE_D_TO_REST;

				_AttachAnimAndSetBoneMaskToUpperBody( ANIMTAP_SLASH_D, ANIM_SLASH_D_TO_REST );
				ZeroTime( ANIMTAP_SLASH_D );

				break;
			}

			// Start another slash...

			_InitStartSwingData();

			if( fmath_RandomChance( 0.65f ) ) {
				// Start a forward slash...

				m_nSlashState = SLASH_STATE_D_TO_F;
				_AttachAnimAndSetBoneMaskToUpperBody( ANIMTAP_SLASH_FB, ANIM_SLASH_F_FROM_REST );
			} else {
				// Start a backward slash...

				m_nSlashState = SLASH_STATE_D_TO_B;
				_AttachAnimAndSetBoneMaskToUpperBody( ANIMTAP_SLASH_FB, ANIM_SLASH_B_FROM_REST );
			}

			SetControlValue( ANIMTAP_SLASH_FB, 1.0f );
			SetControlValue( ANIMTAP_SLASH_FB_LOWER, 1.0f );
			ZeroTime( ANIMTAP_SLASH_FB );
		}

		break;

	case SLASH_STATE_D_TO_REST:
		// Blend out the animation...
		fUnitControlValue = GetControlValue( ANIMCONTROL_SLASH_D );

		if( fUnitControlValue > 0.0f ) {
			fUnitControlValue -= m_pBotInfo_Weapon->fFire1BlendOutSpeed * fDeltaTime;
			FMATH_CLAMPMIN( fUnitControlValue, 0.0f );
			SetControlValue( ANIMCONTROL_SLASH_D, fUnitControlValue );
			SetControlValue( ANIMCONTROL_SLASH_D_LOWER, fUnitControlValue );
		}

		// Advance the animation...
		if( DeltaTime( ANIMTAP_SLASH_D, fDeltaTime, TRUE ) ) {
			// Animation complete...

			if( fUnitControlValue == 0.0f ) {
				// We're done with this state...

				m_nSlashState = SLASH_STATE_NONE;
			}
		}

		break;

	case SLASH_STATE_D_TO_F:
	case SLASH_STATE_D_TO_B:
		// Blend out the downward slash animation...
		fUnitControlValue = GetControlValue( ANIMCONTROL_SLASH_D );

		if( fUnitControlValue > 0.0f ) {
			fUnitControlValue -= m_pBotInfo_Weapon->fFire1BlendOutSpeed * fDeltaTime;
			FMATH_CLAMPMIN( fUnitControlValue, 0.0f );
			SetControlValue( ANIMCONTROL_SLASH_D, fUnitControlValue );
			SetControlValue( ANIMCONTROL_SLASH_D_LOWER, fUnitControlValue );
		}

		// Advance the forward or backward slash animation...
		if( DeltaTime( ANIMTAP_SLASH_FB, fDeltaTime, TRUE ) ) {
			// Animation complete...

			if( fUnitControlValue == 0.0f ) {
				// We're done with this state...

				if( m_nSlashState == SLASH_STATE_D_TO_F ) {
					m_nSlashState = SLASH_STATE_F;
					_AttachAnimAndSetBoneMaskToUpperBody( ANIMTAP_SLASH_FB, ANIM_SLASH_F );
				} else {
					m_nSlashState = SLASH_STATE_B;
					_AttachAnimAndSetBoneMaskToUpperBody( ANIMTAP_SLASH_FB, ANIM_SLASH_B );
				}

				ZeroTime( ANIMTAP_SLASH_FB );
			}
		}

		break;

	};
}


void CBotZom::_StartStreamer( void ) {
	
	m_hStreamer = CFXStreamerEmitter::SpawnFromMeshBones(
						15,
						m_pWorldMesh,
						m_apszStreamerBones,
						m_BotInfo_Zom.fSlashStreamerAlpha,
						&m_StreamerTexInst,
						m_BotInfo_Zom.fSlashStreamerWidth,
						60.0f,
						CFXStreamerEmitter::USE_UP_AXIS
					);
}


void CBotZom::_KillStreamer( void ) {

	CFXStreamerEmitter::EnableStreamerEmission( m_hStreamer, FALSE );
}


void CBotZom::_KillFormingSound( void ) {
	if( m_pAudioEmitterForming ) {
		m_pAudioEmitterForming->Destroy();
		m_pAudioEmitterForming = NULL;
	}

	m_fFormingVolume = 0.0f;

	FMATH_CLEARBITMASK( m_nZomFlags, ZOMFLAG_LOWER_FORMING_VOLUME );
}


void CBotZom::_StartFormingSound( void ) {
	_KillFormingSound();

	m_pAudioEmitterForming = AllocAndPlaySound( m_BotInfo_Zom.pSoundGroupAssemble );
}


void CBotZom::_FadeOutFormingSound( void ) {
	if( m_pAudioEmitterForming ) {
		FMATH_SETBITMASK( m_nZomFlags, ZOMFLAG_LOWER_FORMING_VOLUME );
	}
}


void CBotZom::_FormingSoundWork( void ) {
	if( m_nZomFlags & ZOMFLAG_LOWER_FORMING_VOLUME ) {
		FASSERT( m_pAudioEmitterForming );

		m_fFormingVolume -= 1.5f * FLoop_fPreviousLoopSecs;

		if( m_fFormingVolume > 0.0f ) {
			m_pAudioEmitterForming->SetVolume( m_fFormingVolume );
		} else {
			_KillFormingSound();
		}
	}
}


void CBotZom::_PlayClatterSound( const CFVec3A *pPos_WS, f32 fVolMultiplier ) {
	PlaySound( m_BotInfo_Zom.pSoundGroupClatter, fVolMultiplier );
}


void CBotZom::_PlayCrumbleApartSound( const CFVec3A *pPos_WS, f32 fVolMultiplier ) {
	PlaySound( m_BotInfo_Zom.pSoundGroupCrumbleApart, fVolMultiplier );
}


void CBotZom::_PlayBlowApartSound( const CFVec3A *pPos_WS, f32 fVolMultiplier ) {
	PlaySound( m_BotInfo_Zom.pSoundGroupBlownApart, fVolMultiplier );
}


void CBotZom::_PlayBreathPuffSound( const CFVec3A *pPos_WS, f32 fVolMultiplier ) {
	PlaySound( m_BotInfo_Zom.pSoundGroupBreathPuff, fVolMultiplier );
}


void CBotZom::_HandleAimAnimations( void ) {
	f32 fUnitAimPitch;
	CFAnimFrame TorsoQuat, HeadQuat;

	// Compute the amount we're aiming up/down (up=1, down=0, middle=0.5)...
	fUnitAimPitch = fmath_Div( m_fMountPitch_WS - m_fMountPitchMin_WS, m_fMountPitchMax_WS - m_fMountPitchMin_WS );
	FMATH_CLAMP_UNIT_FLOAT( fUnitAimPitch );

	// Compute the head and torso quaternions...
	TorsoQuat.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( fUnitAimPitch, FMATH_DEG2RAD(-60.0f), FMATH_DEG2RAD(30.0f) ), CFVec3A::m_Null );
	HeadQuat.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( fUnitAimPitch, FMATH_DEG2RAD(-20.0f), FMATH_DEG2RAD(20.0f) ), CFVec3A::m_Null );

	// Apply head and torso quaternions...
	m_AnimManFrameAim.UpdateFrame( BONE_NECK_F, HeadQuat );
	m_AnimManFrameAim.UpdateFrame( BONE_TORSO, TorsoQuat );
}


void CBotZom::_HandleWeaponAnimations( void ) {
	_HandleFiringAnimations();
	_HandleAimAnimations();
}


void CBotZom::_UpdateMatrices( void ) {
	CFMtx43A EntityMtxToWorld;

	CFMtx43A::m_XlatRotY.SetRotationY( m_fMountYaw_WS + m_fLegsYaw_MS );
	CFMtx43A::m_XlatRotY.m_vPos = m_MountPos_WS;

	m_pWorldMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_XlatRotY );

	m_pWorldMesh->UpdateTracker();

	// Update bone matrix palette...
	PROTRACK_BEGINBLOCK("ComputeMtxPal");
		CBotZomPartGroup::ComputeBotMtxPalette( this );
	PROTRACK_ENDBLOCK();//"ComputeMtxPal");

	m_GazeUnitVec_WS.ReceiveUnit( *m_pGazeDir_WS );

	EntityMtxToWorld.Identity();
	EntityMtxToWorld.SetRotationYXZ( m_fMountYaw_WS, m_fMountPitch_WS, 0.0f );
	EntityMtxToWorld.m_vPos = m_MountPos_WS;
	Relocate_RotXlatFromUnitMtx_WS( &EntityMtxToWorld, TRUE, m_pMoveIdentifier );
}


void CBotZom::_HandleWeaponFiring( void ) {
	if( m_nSlashState == SLASH_STATE_NONE ) {
		// Not currently slashing...

		if( m_fControls_Fire1 > 0.0f ) {
			// Slash request...

			_InitStartSwingData();

			if( fmath_RandomChance( 0.75f ) ) {
				// Forward/backward slash...

				if( fmath_RandomChance( 0.65f ) ) {
					// Forward slash...

					m_nSlashState = SLASH_STATE_F_FROM_REST;
					_AttachAnimAndSetBoneMaskToUpperBody( ANIMTAP_SLASH_FB, ANIM_SLASH_F_FROM_REST );
				} else {
					// Backward slash...

					m_nSlashState = SLASH_STATE_B_FROM_REST;
					_AttachAnimAndSetBoneMaskToUpperBody( ANIMTAP_SLASH_FB, ANIM_SLASH_B_FROM_REST );
				}

				ZeroTime( ANIMTAP_SLASH_FB );
			} else {
				// Down slash...

				m_nSlashState = SLASH_STATE_D_FROM_REST;
				_AttachAnimAndSetBoneMaskToUpperBody( ANIMTAP_SLASH_D, ANIM_SLASH_D_FROM_REST );

				ZeroTime( ANIMTAP_SLASH_D );
			}
		}
	}
}


void CBotZom::_AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	// Compute the frame...
	if( nBoneIndex == (u32)((CBotZom *)m_pCollBot)->m_nBoneIndexGroin ) {
		// Groin...

		rNewMtx.Mul( rParentMtx, rBoneMtx );
		m_GroinVecY_WS = rNewMtx.m_vUp;
	} else if( nBoneIndex == (u32)((CBotZom *)m_pCollBot)->m_nBoneIndexTorso ) {
		// Torso...

		CFMtx43A WorldMtx;
		CFQuatA Quat;
		CBotZom *pBotZom = (CBotZom *)m_pCollBot;

		Quat.BuildQuat( m_GroinVecY_WS, -pBotZom->m_fLegsYaw_MS );
		WorldMtx.Mul( rParentMtx, rBoneMtx );

		rNewMtx.m_vPos = WorldMtx.m_vPos;
		Quat.MulPoint( rNewMtx.m_vRight, WorldMtx.m_vRight );
		Quat.MulPoint( rNewMtx.m_vUp, WorldMtx.m_vUp );
		Quat.MulPoint( rNewMtx.m_vFront, WorldMtx.m_vFront );
	} else {
		// All other bones...

		rNewMtx.Mul( rParentMtx, rBoneMtx );
	}
}


void CBotZom::CheckpointSaveSelect( s32 nCheckpoint ) {
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}


BOOL CBotZom::CheckpointSave( void ) {
	// Save parent class data...
	CBot::CheckpointSave();

	// Save bot class data. Order must match load order below!
	CFCheckPoint::SaveData( m_nZomFlags );
	CFCheckPoint::SaveData( m_nZomMode );
	CFCheckPoint::SaveData( m_nRemainingDeathCount );

	return TRUE;
}


void CBotZom::CheckpointRestore( void ) {
	if( m_pPartGroup ) {
		m_pPartGroup->UnassignGroup();
	}

	// Load parent class data...
	FASSERT( m_hSaveData[CFCheckPoint::GetCheckPoint()] != CFCheckPoint::NULL_HANDLE );
	CBot::CheckpointRestore();

	CFCheckPoint::LoadData( m_nZomFlags );
	CFCheckPoint::LoadData( m_nZomMode );
	CFCheckPoint::LoadData( m_nRemainingDeathCount );

	_ResetAnimSystem();

	m_nSlashState = SLASH_STATE_NONE;

	if( m_nRemainingDeathCount > 0 ) {
		// This bot has another life...

		if( m_nSleepState == BOTSLEEPSTATE_NONE ) {
			// Bot is awake...

			_InitAlive();
		} else {
			// Bot is asleep...

			_InitInPieces();
			m_nSleepState = BOTSLEEPSTATE_DOZE_LOOP;
		}

		SetNormHealth( m_fReformNormHealth );
	} else {
		// This bot is dead...

		SetNormHealth( 0.0f );
	}
}


void CBotZom::_ResetAnimSystem( void ) {
	m_Anim.Reset();

	AttachAnim( ANIMTAP_AIM_SUMMER, &m_AnimManFrameAim );
	m_AnimManFrameAim.Reset();
	UpdateBoneMask( ANIMTAP_AIM_SUMMER, m_aBoneEnableIndices_AimSummer, TRUE );

	SetControlValue( ANIMCONTROL_STAND, 1.0f );
	UpdateUnitTime( ANIMTAP_STAND, fmath_RandomFloat() );

	EnableControlSmoothing( ANIMCONTROL_FORM );
}


void CBotZom::InflictDamageResult( const CDamageResult *pDamageResult ) {
	FASSERT( IsCreated() );

	if( m_nZomMode != ZOMMODE_ALIVE ) {
		// Not alive, so don't deal damage...

		if( pDamageResult->m_pDamageData->m_nDamageLocale == CDamageForm::DAMAGE_LOCALE_AMBIENT ) {
			// Nothing to do for ambient damage...
			return;
		}

		if( (pDamageResult->m_fDeltaHitpoints <= 0.0f) || (pDamageResult->m_fImpulseMag <= 0.0f) ) {
			return;
		}

		switch( pDamageResult->m_pDamageData->m_nDamageLocale ) {
		case CDamageForm::DAMAGE_LOCALE_IMPACT:
			if( m_nZomMode == ZOMMODE_FORMING ) {
				CollapseIntoDebris();
			} else {
				ReactToImpact( pDamageResult );
			}

			break;

		case CDamageForm::DAMAGE_LOCALE_BLAST:
			ReactToBlast( pDamageResult, FALSE );
			break;
		};

		if( m_nSleepState == BOTSLEEPSTATE_NONE ) {
			if( NormHealth() > 0.0f ) {
				_SetAutoWakeupTimer( m_BotInfo_Zom.fReformInterruptionStayAsleepSecs );
			}
		}

		return;
	}

	// Bot is alive...

	f32 fCurrentNormHealth = NormHealth();

	if( fCurrentNormHealth <= 0.0f ) {
		return;
	}

	if( (fCurrentNormHealth - pDamageResult->m_fDeltaHitpoints) > 0.0f ) {
		// Bot will still be alive after we apply this damage...

		CBot::InflictDamageResult( pDamageResult );

		FASSERT( NormHealth() > 0.0f );

		switch( pDamageResult->m_pDamageData->m_nDamageLocale ) {
		case CDamageForm::DAMAGE_LOCALE_IMPACT:
			ReactToImpact( pDamageResult );
			break;

		case CDamageForm::DAMAGE_LOCALE_BLAST:
			ReactToBlast( pDamageResult, TRUE );
			break;
		};

		return;
	}

	// Death blow...

	FASSERT( m_nRemainingDeathCount > 0 );

	if( m_nRemainingDeathCount > 1 ) {
		// Fake our death...

		BOOL bDeathBlowSuccess;

		if( pDamageResult->m_pDamageData->m_nDamageLocale == CDamageForm::DAMAGE_LOCALE_BLAST ) {
			bDeathBlowSuccess = ReactToBlast( pDamageResult, FALSE );
		} else {
			bDeathBlowSuccess = CrumbleIntoDebris();
		}

		if( bDeathBlowSuccess == FALSE ) {
			// Unsuccessful in setting death blow mode. Ignore damage request...
			return;
		}

		if( pDamageResult->m_pDamageData->m_nDamageLocale == CDamageForm::DAMAGE_LOCALE_BLAST ) {
			_PlayBlowApartSound( &m_MtxToWorld.m_vPos );
		} else {
			_PlayCrumbleApartSound( &m_MtxToWorld.m_vPos );
		}

		_SetAutoWakeupTimer( m_BotInfo_Zom.fDeathStayAsleepSecs );

		// Restore health...
		SetNormHealth( m_fReformNormHealth );
	} else {
		// This bot is dead for real...

		if( CBotZomPartGroup::InitState_FinalDeathExplosion( this ) ) {
			// Death explosion spawned...

			_ConfigureBotToPieces();
			FMATH_SETBITMASK( m_uBotDeathFlags, BOTDEATHFLAG_WALKING_DEAD );
		}

		CBot::InflictDamageResult( pDamageResult );
		FASSERT( NormHealth() <= 0.0f );

		_SetAutoWakeupTimer( -1.0f );
	}

	--m_nRemainingDeathCount;
}


void CBotZom::Squish( const CBot* pSquisher ) {
	FASSERT( IsCreated() );

	if( m_nZomMode != ZOMMODE_ALIVE ) {
		// Not alive, so don't deal damage...

		CollapseIntoDebris();

		if( m_nSleepState == BOTSLEEPSTATE_NONE ) {
			if( NormHealth() > 0.0f ) {
				_SetAutoWakeupTimer( m_BotInfo_Zom.fReformInterruptionStayAsleepSecs );
			}
		}

		return;
	}

	// Bot is alive...

	f32 fCurrentNormHealth = NormHealth();

	if( fCurrentNormHealth <= 0.0f ) {
		CollapseIntoDebris();
		return;
	}

	if( m_nRemainingDeathCount > 1 ) {
		// Fake our death...

		CollapseIntoDebris();

		_SetAutoWakeupTimer( m_BotInfo_Zom.fDeathStayAsleepSecs );

		// Restore health...
		SetNormHealth( m_fReformNormHealth );
	} else {
		// This bot is dead for real...

		if( CBotZomPartGroup::InitState_FinalDeathExplosion( this ) ) {
			// Death explosion spawned...

			_ConfigureBotToPieces();
			FMATH_SETBITMASK( m_uBotDeathFlags, BOTDEATHFLAG_WALKING_DEAD );
		}

		_SetAutoWakeupTimer( -1.0f );

		SetNormHealth( 0.0f );

		// Make sure the bot is not already dead
		if ( !IsDeadOrDying() ) {
			Die();

			// Player probably isn't in a zombie, but just in case...
			if ( pSquisher ) {
				s32 nPlayerIndex = -1;
				if (pSquisher->Recruit_IsRecruited())
					nPlayerIndex = pSquisher->Recruit_GetRecruiter();
				if (nPlayerIndex < 0)
					nPlayerIndex = pSquisher->m_nPossessionPlayerIndex;
				if (nPlayerIndex >= 0)
					Player_aPlayer[nPlayerIndex].CreditKill( m_nPossessionPlayerIndex, this );
			}
		}
	}

	--m_nRemainingDeathCount;
}


// Configures various bot states and variables so the
// bot will be in its alive mode.
void CBotZom::_ConfigureBotToAlive( BOOL bMobilizeBot ) {
	m_nZomMode = ZOMMODE_ALIVE;

	UseNormalTestForBlastDamage();
	SetTargetable( TRUE );
	DontIgnoreBotVBotCollision();

	if( bMobilizeBot ) {
		MobilizeBot();
	} else {
		ImmobilizeBot();
	}

	// Restore our bounding sphere...
	m_pWorldMesh->m_BoundSphere_MS = m_pWorldMesh->m_pMesh->BoundSphere_MS;

	EnableOurWorkBit();
}


// Configures various bot states and variables so the
// bot will be in its formation mode.
void CBotZom::_ConfigureBotToForming( void ) {
	m_nZomMode = ZOMMODE_FORMING;

	UseSphereTestForBlastDamage();
	SetTargetable( FALSE );
	IgnoreBotVBotCollision();
	ImmobilizeBot();
	ZeroVelocity();

	EnableOurWorkBit();
}


// Configures various bot states and variables so the
// bot will be in its in-pieces mode.
void CBotZom::_ConfigureBotToPieces( void ) {
	m_nZomMode = ZOMMODE_IN_PIECES;

	UseSphereTestForBlastDamage();
	SetTargetable( FALSE );
	IgnoreBotVBotCollision();
	ImmobilizeBot();
	ZeroVelocity();

	EnableOurWorkBit();
}


void CBotZom::_SetAutoWakeupTimer( f32 fAutoWakeupSecs ) {
	m_fSecsUntilAutoWakeup = fAutoWakeupSecs;

	if( fAutoWakeupSecs >= 0.0f ) {
		EnableOurWorkBit();
	}
}


// Called by the ZombieBot part group system to inform this bot
// that he's about to lose his group.
void CBotZom::LosingPartGroup( void ) {
	FASSERT( IsCreated() );

	if( m_pPartGroup == NULL ) {
		// Uhhh, we don't have a part group...
		return;
	}

	// We have a part group...

	if( NormHealth() <= 0.0f ) {
		// We're dead...
		return;
	}

	// We're alive...

	_InitAlive();
}


// We don't support nap jerks. This function always returns FALSE.
BOOL CBotZom::NapJerk( BOOL bAllowAutoWakeup ) {
	FASSERT( IsCreated() );

	return FALSE;
}


BOOL CBotZom::Sleep( void ) {
	FASSERT( IsCreated() );

	if( m_nSleepState == BOTSLEEPSTATE_DOZE_LOOP ) {
		// Already sleeping...
		return TRUE;
	}

	if( !CollapseIntoDebris() ) {
		return FALSE;
	}

	m_nSleepState = BOTSLEEPSTATE_DOZE_LOOP;
	_SetAutoWakeupTimer( -1.0f );

	FMATH_CLEARBITMASK( m_nZomFlags, ZOMFLAG_DISABLE_AI_WAKE_UP );

	return FALSE;
}


// Awakens the bot.
//
// Returns TRUE if the bot has started to awaken.
// Returns FALSE if the bot could not be awakened at this time.
BOOL CBotZom::WakeUp( void ) {
	FASSERT( IsCreated() );

	if( (m_nRemainingDeathCount == 0) || (NormHealth() <= 0.0f) ) {
		// We're dead...
		return FALSE;
	}

	if( m_nZomFlags & ZOMFLAG_DISABLE_AI_WAKE_UP ) {
		// AI is not allowed to wake us up...
		return FALSE;
	}

	if( StartForming() ) {
		// Forming successful...

		FMATH_SETBITMASK( m_nZomFlags, ZOMFLAG_DISABLE_AI_WAKE_UP );

		return TRUE;
	}

	// Forming unsuccessful...

	return FALSE;
}


// Forms the bot from a pile into its intact state.
//
// Returns TRUE if the operation was successful.
// Returns FALSE if the collapse could not be initiated.
BOOL CBotZom::StartForming( void ) {
	FASSERT( IsCreated() );

	if( m_nPowerState != POWERSTATE_POWERED_UP ) {
		// We're not powered up...
		return FALSE;
	}

	if( (m_nRemainingDeathCount == 0) || (NormHealth() <= 0.0f) ) {
		// We're dead...
		return FALSE;
	}

	if( (m_nZomMode == ZOMMODE_FORMING) || (m_nZomMode == ZOMMODE_ALIVE) ) {
		// Waking or already awake...
		return TRUE;
	}

	if( _DoesNormalBoundSphereIntersectAnotherBot() ) {
		// We can't start forming because another bot's bounding
		// sphere is occupying the same space as ours...
		return FALSE;
	}

	// Initialize the parts in the group...
	if( !CBotZomPartGroup::InitState_Forming( this ) ) {
		// No free part groups...
		return FALSE;
	}

	// Start our forming animation...
	UpdateTime( ANIMTAP_FORM, 0.0f );
	SetControlValue( ANIMCONTROL_FORM, 1.0f );
	FMATH_SETBITMASK( m_nZomFlags, ZOMFLAG_PLAY_FORM_ANIM );

	_ConfigureBotToForming();

	_StartFormingSound();

	// Tell AI we're awake...
	m_nSleepState = BOTSLEEPSTATE_NONE;

	return TRUE;
}


// Spreads the bot into a bunch of hovering pieces.
//
// Returns TRUE if the operation was successful.
// Returns FALSE if the collapse could not be initiated.
BOOL CBotZom::SpreadIntoHoveringPieces( void ) {
	FASSERT( IsCreated() );

	// Initialize the parts in the group...
	if( !CBotZomPartGroup::InitState_EMP( this ) ) {
		// No free part groups...
		return FALSE;
	}

	_ConfigureBotToPieces();

	return TRUE;
}


// Collapses the entire bot into a pile of debris.
//
// Returns TRUE if the operation was successful.
// Returns FALSE if the collapse could not be initiated.
BOOL CBotZom::CollapseIntoDebris( void ) {
	FASSERT( IsCreated() );

	if( m_nZomMode == ZOMMODE_IN_PIECES ) {
		// Already in pieces...

		return TRUE;
	}

	// Initialize the parts in the group...
	if( !CBotZomPartGroup::InitState_Collapse( this ) ) {
		// No free part groups...
		return FALSE;
	}

	_ConfigureBotToPieces();

	return TRUE;
}


// Crumbles the entire bot into a pile of debris.
//
// Returns TRUE if the operation was successful.
// Returns FALSE if the collapse could not be initiated.
BOOL CBotZom::CrumbleIntoDebris( void ) {
	FASSERT( IsCreated() );

	if( (m_nZomMode == ZOMMODE_CRUMBLING) || (m_nZomMode == ZOMMODE_IN_PIECES) ) {
		// Already crumbling or in pieces...

		return TRUE;
	}

	if( m_nZomMode == ZOMMODE_FORMING ) {
		// If we're forming, just collapse the bot...

		return CollapseIntoDebris();
	}

	// Bot is alive...

	FASSERT( m_nZomMode == ZOMMODE_ALIVE );

	// Initialize the parts in the group...
	if( !CBotZomPartGroup::InitState_StagedCollapse( this ) ) {
		// No free part groups...
		return FALSE;
	}

	m_nZomMode = ZOMMODE_CRUMBLING;

	return TRUE;
}


// Blasts the entire bot into a pile of debris or, if the
// bot is alive and bIfAliveUseRecoil is TRUE, performs a
// recoil animation on the affected parts which returns the
// parts to the bot's body when the recoil is done.
//
// Returns TRUE if the operation was successful.
// Returns FALSE if the collapse could not be initiated.
BOOL CBotZom::ReactToBlast( const CDamageResult *pDamageResult, BOOL bIfAliveUseRecoil ) {
	FASSERT( IsCreated() );

	if( pDamageResult->m_pDamageData->m_nDamageLocale != CDamageForm::DAMAGE_LOCALE_BLAST ) {
		return FALSE;
	}

	if( (m_nZomMode == ZOMMODE_ALIVE) && bIfAliveUseRecoil ) {
		// Initialize the parts in the group for a recoil animation...

		if( !CBotZomPartGroup::InitState_RecoilFromBlast( this, pDamageResult ) ) {
			// No free part groups...
			return FALSE;
		}
	} else {
		// Initialize the parts in the group to blast into debris...

		if( !CBotZomPartGroup::InitState_DebrisFromBlast( this, pDamageResult ) ) {
			// No free part groups...
			return FALSE;
		}

		_ConfigureBotToPieces();
	}

	return TRUE;
}


// Reacts the bot to an impact. If the bot is alive, the hit part
// is recoiled and returned to the bot. If the bot is forming, it
// collapses into debris. Otherwise, the hit part is turned into
// debris and reacts to the impact.
//
// Returns TRUE if the operation was successful.
// Returns FALSE if the collapse could not be initiated.
BOOL CBotZom::ReactToImpact( const CDamageResult *pDamageResult ) {
	FASSERT( IsCreated() );

	if( pDamageResult->m_pDamageData->m_nDamageLocale != CDamageForm::DAMAGE_LOCALE_IMPACT ) {
		return FALSE;
	}

	if( m_nZomMode == ZOMMODE_ALIVE ) {
		// Initialize the parts in the group for a recoil animation...

		if( !CBotZomPartGroup::InitState_RecoilFromImpact( this, pDamageResult ) ) {
			// No free part groups...
			return FALSE;
		}

		return TRUE;
	}

	if( m_nZomMode == ZOMMODE_FORMING ) {
		// If we're forming, just collapse into debris...

		return CollapseIntoDebris();
	}

	// We're either crumbling or in pieces...

	FASSERT( (m_nZomMode == ZOMMODE_CRUMBLING) || (m_nZomMode == ZOMMODE_IN_PIECES) );

	// Initialize the parts in the group...
	if( !CBotZomPartGroup::InitState_DebrisFromImpact( this, pDamageResult ) ) {
		// No free part groups...
		return FALSE;
	}

	_ConfigureBotToPieces();

	return TRUE;
}


void CBotZom::ResetBotToScatteredPieces( void ) {
	FASSERT( IsCreated() );

	if( NormHealth() <= 0.0f ) {
		// Bot is not alive...
		return;
	}

	// Bot is alive...

	_InitInPieces();
}


void CBotZom::Power( BOOL bPowerUp, f32 fPowerOffTime, f32 fPowerOffOnSpeedMult ) {
	FASSERT( IsCreated() );

	if( bPowerUp ) {
		// Caller wishes to power-up the bot...

		if( m_nPowerState == POWERSTATE_POWERED_DOWN ) {
			m_nPowerState = POWERSTATE_POWERED_UP;

			m_fPowerOffOnTime = 0.0f;

			StartForming();
		}
	} else {
		// Caller wishes to power-down the bot...

		if( m_nSleepState == BOTSLEEPSTATE_NONE ) {
			if( m_nPowerState == POWERSTATE_POWERED_UP ) {
				m_nPowerState = POWERSTATE_POWERED_DOWN;

				SpreadIntoHoveringPieces();
			}

			m_fPowerOffOnTime = fPowerOffTime;
		}
	}
}


void CBotZom::Power_SetState( BOOL bPowerUp, f32 fPowerOffTime ) {
	Power( bPowerUp, fPowerOffTime );
}



#define _START_RAY_HEIGHT_ABOVE_BOT		10.0f
#define _MAX_PART_PLACEMENT_DIST_XZ		40.0f
#define _MAX_PART_PLACEMENT_DIST_Y		40.0f
#define _TRY_DELTA_YAW					FMATH_DEG2RAD( 10.0f )
#define _TRY_DELTA_DIST_XZ				5.0f


void CBotZom::_ScatterPieces( void ) {
	CFVec3A Pos_WS, UnitFaceNormal_WS, StartRayPos_WS, OrigX, OrigZ;
	const FMesh_t *pMesh;
	const FMeshBone_t *pBone;
	u32 nBoneIndex, nBoneCount;

	StartRayPos_WS = m_MtxToWorld.m_vPos;
	StartRayPos_WS.y += _START_RAY_HEIGHT_ABOVE_BOT;

	pMesh = m_pWorldMesh->m_pMesh;
	nBoneCount = pMesh->nBoneCount;

	// Step through all bones...
	for( nBoneIndex=0; nBoneIndex<nBoneCount; ++nBoneIndex ) {
		pBone = &pMesh->pBoneArray[nBoneIndex];

		if( pBone->nFlags & FMESH_BONEFLAG_VOIDBONE ) {
			// No geo on this bone...
			continue;
		}

		// This bone has geo on it...

		_PickRandomStartingPlaceForPiece( &StartRayPos_WS, &Pos_WS, &UnitFaceNormal_WS );

		m_pWorldMesh->GetBoneMtxPalette()[ nBoneIndex ]->m_vPos = Pos_WS;
		m_pWorldMesh->GetBoneMtxPalette()[ nBoneIndex ]->UnitMtxFromUnitVec( &UnitFaceNormal_WS );

		OrigX = m_pWorldMesh->GetBoneMtxPalette()[ nBoneIndex ]->m_vX;
		OrigZ = m_pWorldMesh->GetBoneMtxPalette()[ nBoneIndex ]->m_vZ;

		m_pWorldMesh->GetBoneMtxPalette()[ nBoneIndex ]->m_vX = OrigZ;
		m_pWorldMesh->GetBoneMtxPalette()[ nBoneIndex ]->m_vZ.ReceiveNegative( OrigX );
	}

	CBotZomPartGroup::UpdateBotBoundingSphere( this );
}


void CBotZom::_PickRandomStartingPlaceForPiece( const CFVec3A *pStartRayPos_WS, CFVec3A *pPos_WS, CFVec3A *pUnitFaceNormal_WS ) {
	f32 fDeltaAngle, fStartAngleOnXZPlane;

	// Pick random direction on the XZ plane...
	fStartAngleOnXZPlane = fmath_RandomFloatRange( 0.0f, FMATH_2PI );

	if( _FindPieceStartingPlaceAlongRayXZ( pStartRayPos_WS, fStartAngleOnXZPlane, pPos_WS, pUnitFaceNormal_WS ) ) {
		// This is a great spot for the piece...
		return;
	}

	// Try alternate angles...

	for( fDeltaAngle=_TRY_DELTA_YAW; fDeltaAngle<FMATH_PI; fDeltaAngle+=_TRY_DELTA_YAW ) {
		if( _FindPieceStartingPlaceAlongRayXZ( pStartRayPos_WS, fStartAngleOnXZPlane + fDeltaAngle, pPos_WS, pUnitFaceNormal_WS ) ) {
			// This is a great spot for the piece...
			return;
		}

		if( _FindPieceStartingPlaceAlongRayXZ( pStartRayPos_WS, fStartAngleOnXZPlane - fDeltaAngle, pPos_WS, pUnitFaceNormal_WS ) ) {
			// This is a great spot for the piece...
			return;
		}
	}

	// Couldn't find a good place for the piece...

	*pPos_WS = m_MtxToWorld.m_vPos;
	*pUnitFaceNormal_WS = CFVec3A::m_UnitAxisY;
}


// Returns TRUE and fills pPos_WS and pUnitFaceNormal_WS if a good spot was found.
BOOL CBotZom::_FindPieceStartingPlaceAlongRayXZ( const CFVec3A *pStartRayPos_WS, f32 fAngleOnXZPlane, CFVec3A *pPos_WS, CFVec3A *pUnitFaceNormal_WS ) {
	f32 fMaxTryDistXZ, fInitialTryDistXZ, fTryRayDistXZ;
	CFVec3A RayDirXZ, StartRayPos_WS, EndRayPos_WS;
	FCollImpact_t *pCollImpact;

	RayDirXZ.x = fmath_Sin( fAngleOnXZPlane );
	RayDirXZ.y = 0.0f;
	RayDirXZ.z = fmath_Cos( fAngleOnXZPlane );

	// Cast ray out radially from start ray position, along random XZ direction...
	EndRayPos_WS.Mul( RayDirXZ, _MAX_PART_PLACEMENT_DIST_XZ ).Add( *pStartRayPos_WS );

	m_CollInfo.nCollTestType = FMESH_COLLTESTTYPE_RAY;
	m_CollInfo.nCollMask = FCOLL_MASK_COLLIDE_WITH_NPCS;
	m_CollInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
	m_CollInfo.nTrackerUserTypeBitsMask = -1;
	m_CollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_NONE;
	m_CollInfo.bFindClosestImpactOnly = TRUE;
	m_CollInfo.bCullBacksideCollisions = TRUE;
	m_CollInfo.bCalculateImpactData = TRUE;
	m_CollInfo.Ray.Init( pStartRayPos_WS, &EndRayPos_WS );

	CFTrackerCollideRayInfo TrackerCollideInfo;
	TrackerCollideInfo.StartPoint_WS = *pStartRayPos_WS;
	TrackerCollideInfo.EndPoint_WS = EndRayPos_WS;
	TrackerCollideInfo.bComputeIntersection = FALSE;
	TrackerCollideInfo.bSort = TRUE;
	TrackerCollideInfo.pCallback = _PickRandomPlaceTrackerCallback1;
	TrackerCollideInfo.nTrackerTypeBits = FWORLD_TRACKERTYPEBIT_MESH;
	TrackerCollideInfo.nTrackerUserTypeBitsMask = -1;
	TrackerCollideInfo.bIgnoreCollisionFlag = FALSE;
	TrackerCollideInfo.nTrackerSkipCount = 0;
	TrackerCollideInfo.ppTrackerSkipList = NULL;

	m_pCollBot = this;

	fcoll_Clear();
	fworld_CollideWithTrackers( &TrackerCollideInfo );
	fworld_CollideWithWorldTris( &m_CollInfo );

	pCollImpact = fcoll_FindClosest();
	if( pCollImpact ) {
		// Our ray hit something...

		fMaxTryDistXZ = pCollImpact->fImpactDistInfo * _MAX_PART_PLACEMENT_DIST_XZ;
	} else {
		// Our ray didn't hit anything...

		fMaxTryDistXZ = _MAX_PART_PLACEMENT_DIST_XZ;
	}

	if( fMaxTryDistXZ > 0.5f ) {
		fMaxTryDistXZ -= 0.4f;
	} else {
        fMaxTryDistXZ *= 0.99f;
	}

	fInitialTryDistXZ = fmath_RandomFloat() * fMaxTryDistXZ;

	if( _TryRayAtGivenDistXZ( pStartRayPos_WS, &RayDirXZ, fInitialTryDistXZ, pPos_WS, pUnitFaceNormal_WS ) ) {
		// This is a great spot for the piece...
		return TRUE;
	}

	// Try places closer to the bot...

	for( fTryRayDistXZ = (fInitialTryDistXZ - _TRY_DELTA_DIST_XZ); fTryRayDistXZ >= 0.1f; fTryRayDistXZ -= _TRY_DELTA_DIST_XZ ) {
		if( _TryRayAtGivenDistXZ( pStartRayPos_WS, &RayDirXZ, fTryRayDistXZ, pPos_WS, pUnitFaceNormal_WS ) ) {
			// This is a great spot for the piece...
			return TRUE;
		}
	}

	// Try places farther from the bot...

	for( fTryRayDistXZ = (fInitialTryDistXZ + _TRY_DELTA_DIST_XZ); fTryRayDistXZ <= fMaxTryDistXZ; fTryRayDistXZ += _TRY_DELTA_DIST_XZ ) {
		if( _TryRayAtGivenDistXZ( pStartRayPos_WS, &RayDirXZ, fTryRayDistXZ, pPos_WS, pUnitFaceNormal_WS ) ) {
			// This is a great spot for the piece...
			return TRUE;
		}
	}

	// There are no good places along this ray...

	return FALSE;
}


BOOL CBotZom::_PickRandomPlaceTrackerCallback1( CFWorldTracker *pTracker, FVisVolume_t *pVolume, const CFVec3 *pIntersectionPoint_WS, f32 fUnitDistToIntersection ) {
	CFWorldMesh *pWorldMesh = (CFWorldMesh *)pTracker;

	if( !(pWorldMesh->m_nFlags & FMESHINST_FLAG_STATIC) ) {
		// Dynamic mesh...
		return TRUE;
	}

	// Static mesh...

	if( !pWorldMesh->CollideWithMeshTris( &m_CollInfo ) ) {
		// Our ray didn't hit any tris on this mesh...
		return TRUE;
	}

	// Our ray hit a triangle on this mesh...

	return FALSE;
}


BOOL CBotZom::_TryRayAtGivenDistXZ( const CFVec3A *pStartRayPos_WS, const CFVec3A *pRayUnitDirXZ, f32 fRayDistXZ, CFVec3A *pPos_WS, CFVec3A *pUnitFaceNormal_WS ) {
	CFVec3A StartVerticalRayPos_WS, EndVerticalRayPos_WS, SaveRayStart, SaveRayEnd;
	FCollImpact_t *pCollImpact;

	// Save original ray...
	SaveRayStart = m_CollInfo.Ray.vStart_WS;
	SaveRayEnd = m_CollInfo.Ray.vEnd_WS;

	StartVerticalRayPos_WS.Mul( *pRayUnitDirXZ, fRayDistXZ ).Add( *pStartRayPos_WS );
	EndVerticalRayPos_WS = StartVerticalRayPos_WS;
	EndVerticalRayPos_WS.y -= _MAX_PART_PLACEMENT_DIST_Y;

	m_CollInfo.Ray.Init( &StartVerticalRayPos_WS, &EndVerticalRayPos_WS );

	CFTrackerCollideRayInfo TrackerCollideInfo;
	TrackerCollideInfo.StartPoint_WS = StartVerticalRayPos_WS;
	TrackerCollideInfo.EndPoint_WS = EndVerticalRayPos_WS;
	TrackerCollideInfo.bComputeIntersection = FALSE;
	TrackerCollideInfo.bSort = TRUE;
	TrackerCollideInfo.pCallback = _PickRandomPlaceTrackerCallback2;
	TrackerCollideInfo.nTrackerTypeBits = FWORLD_TRACKERTYPEBIT_MESH;
	TrackerCollideInfo.nTrackerUserTypeBitsMask = -1;
	TrackerCollideInfo.bIgnoreCollisionFlag = FALSE;
	TrackerCollideInfo.nTrackerSkipCount = 0;
	TrackerCollideInfo.ppTrackerSkipList = NULL;

	fcoll_Clear();
	fworld_CollideWithTrackers( &TrackerCollideInfo );
	fworld_CollideWithWorldTris( &m_CollInfo );

	// Restore original ray...
	m_CollInfo.Ray.Init( &SaveRayStart, &SaveRayEnd );

	pCollImpact = fcoll_FindClosest();
	if( pCollImpact == NULL ) {
		// Our ray didn't hit anything...

		return FALSE;
	}

	// Our ray hit a triangle...

	*pPos_WS = pCollImpact->ImpactPoint;
	*pUnitFaceNormal_WS = pCollImpact->UnitFaceNormal;

	return TRUE;
}


BOOL CBotZom::_PickRandomPlaceTrackerCallback2( CFWorldTracker *pTracker, FVisVolume_t *pVolume, const CFVec3 *pIntersectionPoint_WS, f32 fUnitDistToIntersection ) {
	CFWorldMesh *pWorldMesh = (CFWorldMesh *)pTracker;

	if( !(pWorldMesh->m_nFlags & FMESHINST_FLAG_STATIC) ) {
		// Dynamic mesh...
		return TRUE;
	}

	// Static mesh...

	if( !pWorldMesh->CollideWithMeshTris( &m_CollInfo ) ) {
		// Our ray didn't hit any tris on this mesh...
		return TRUE;
	}

	// Our ray hit a triangle on this mesh...

	FASSERT( FColl_nImpactCount );

	if( FColl_aImpactBuf[ FColl_nImpactCount - 1 ].UnitFaceNormal.y < 0.707f ) {
		// Too steep of an angle for part to sit on...
		return TRUE;
	}

	return FALSE;
}


// Returns TRUE if another bot's collision sphere occupies the same space as ours.
BOOL CBotZom::_DoesNormalBoundSphereIntersectAnotherBot( void ) {
	CFVec3A PrevSphereCenter_WS;
	f32 fCollSphereY;

	m_pCollBot = this;
	FWorld_nTrackerSkipListCount = 0;
	AppendTrackerSkipList();

	fCollSphereY = ComputeCollSphereY();

	m_CollSphere.m_Pos.x = m_pBotInfo_Gen->fCollSphere1X_MS + m_MountPos_WS.x;
	m_CollSphere.m_Pos.y = fCollSphereY + m_MountPos_WS.y - 1.0f;
	m_CollSphere.m_Pos.z = m_pBotInfo_Gen->fCollSphere1Z_MS + m_MountPos_WS.z;
	m_CollSphere.m_fRadius = m_pBotInfo_Gen->fCollSphere1Radius_MS;

	PrevSphereCenter_WS.x = m_CollSphere.m_Pos.x;
	PrevSphereCenter_WS.y = fCollSphereY + m_MountPrevPos_WS.y - 1.0f;
	PrevSphereCenter_WS.z = m_CollSphere.m_Pos.z;

	CFWorldUser UserTracker;
	UserTracker.MoveTracker( m_CollSphere );

	if( !UserTracker.FindIntersectingTrackers( _TrackerCollisionCallback, FWORLD_TRACKERTYPE_MESH ) ) {
		// Our bot's collision sphere intersects another bot's collision sphere...

		return TRUE;
	}

	return FALSE;
}


// returning FALSE aborts the collision test.
BOOL CBotZom::_TrackerCollisionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) {
	if( !pTracker->IsCollisionFlagSet() ) {
		// Not collidable...
		return TRUE;
	}

	u32 i;

	for( i=0; i<FWorld_nTrackerSkipListCount; i++ ) {
		if( pTracker == FWorld_apTrackerSkipList[i] ) {
			return TRUE;
		}
	}

	if( pTracker->m_nUser != MESHTYPES_ENTITY ) {
		// Not an entity...
		return TRUE;
	}

	if( !(((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_BOT) ) {
		// Not a bot...
		return TRUE;
	}

	CBot *pBot = (CBot *)pTracker->m_pUser;

	if( pBot->m_pBotInfo_Gen == NULL ) {
		// Not collision sphere info...
		return TRUE;
	}

	if( pBot->TypeBits() & ENTITY_BIT_BOTZOM ) {
		// This is a ZombieBot...

		CBotZom *pBotZom = (CBotZom *)pBot;

		if( pBotZom->m_nZomMode == ZOMMODE_IN_PIECES ) {
			// Ignore collision with ZombieBots that are in pieces...
			return TRUE;
		}
	}

	CFSphere HisCollSphere_WS;

	HisCollSphere_WS.m_Pos.x = pBot->m_pBotInfo_Gen->fCollSphere1X_MS + pBot->MtxToWorld()->m_vPos.x;
	HisCollSphere_WS.m_Pos.y = pBot->ComputeCollSphereY() + pBot->MtxToWorld()->m_vPos.y - 1.0f;
	HisCollSphere_WS.m_Pos.z = pBot->m_pBotInfo_Gen->fCollSphere1Z_MS + pBot->MtxToWorld()->m_vPos.z;
	HisCollSphere_WS.m_fRadius = pBot->m_pBotInfo_Gen->fCollSphere1Radius_MS;

	if( !m_CollSphere.IsIntersecting( HisCollSphere_WS ) ) {
		// No collision...
		return TRUE;
	}

	// This bot's collision sphere intersects ours...

	return FALSE;
}

