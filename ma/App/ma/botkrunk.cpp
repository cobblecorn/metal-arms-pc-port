//////////////////////////////////////////////////////////////////////////////////////
// botkrunk.cpp -
//
// Author: Michael Starich
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
// 1/27/03 Starich       Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "botkrunk.h"
#include "fanim.h"
#include "fresload.h"
#include "floop.h"
#include "gamepad.h"
#include "ftimer.h"
#include "ftext.h"
#include "ItemInst.h"
#include "ItemRepository.h"
#include "fforce.h"
#include "meshtypes.h"
#include "weapons.h"
#include "reticle.h"
#include "player.h"
#include "level.h"
#include "ProTrack.h"
#include "fsndfx.h"
#include "meshentity.h"
#include "Ai\AIEnviro.h"
#include "Hud2.h"
#include "eparticlepool.h"
#include "gcoll.h"
#include "botpart.h"


static cchar *_pszBotInfoFilename = "b_krunk";
static cchar *_apszKrunkMeshFilenames[] = {
	"GRDKkrunk00"
};

static cchar *_pszSoundEffectBank = "Krunk";
static cchar *_pszFlameParticleFile = "flame1";

#define _BOTPART_FILENAME		"bp_krunk"

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotKrunkBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************
static CBotKrunkBuilder _BotKrunkBuilder;


void CBotKrunkBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_KRUNK, pszEntityType );

	// Override defaults set by our parent classes...
	m_nEC_HealthContainerCount = CBotKrunk::m_BotInfo_Gen.nBatteryCount;
	m_pszEC_ArmorProfile = CBotKrunk::m_BotInfo_Gen.pszArmorProfile;

	m_anNPCWeaponType[0] = _NPC_WEAPON_TETHER;
	m_anNPCWeaponType[1] = _NPC_WEAPON_EMP;

	// This class of bots shows up on Glitch's radar...
	FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_SHOWS_UP_ON_RADAR );
}

BOOL CBotKrunkBuilder::PostInterpretFixup( void ) {

	if( !CBotBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	return TRUE;

_ExitWithError:
	return FALSE;
}

BOOL CBotKrunkBuilder::InterpretTable( void ) {
	return CBotBuilder::InterpretTable();
}



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotKrunk
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

// static Krunk Vars:
BOOL CBotKrunk::m_bSystemInitialized = FALSE;
u32 CBotKrunk::m_nBotClassClientCount = 0;
CBotKrunk::BotInfo_Gen_t 		CBotKrunk::m_BotInfo_Gen;
CBotKrunk::BotInfo_MountAim_t 	CBotKrunk::m_BotInfo_MountAim;
CBotKrunk::BotInfo_Walk_t 		CBotKrunk::m_BotInfo_Walk;
CBotKrunk::BotInfo_Jump_t 		CBotKrunk::m_BotInfo_Jump;
CBotKrunk::BotInfo_Weapon_t 	CBotKrunk::m_BotInfo_Weapon;
CBotKrunk::BotInfo_Idle_t*		CBotKrunk::m_BotInfo_Idle=NULL;
u32								CBotKrunk::m_nBotInfo_Idles=0;
CBotKrunk::_KrunkFootsteps_t	CBotKrunk::m_KrunkFootStepSounds;
CBotAnimStackDef 				CBotKrunk::m_AnimStackDef;
CFVec3A 						CBotKrunk::m_GroinVecY_WS;
FParticle_DefHandle_t			CBotKrunk::m_hFlameParticleDef;
CBotPartPool*					CBotKrunk::m_pPartPool=NULL;

BOOL CBotKrunk::InitSystem( void ) {
	
	FASSERT( !m_bSystemInitialized );

	m_nBotClassClientCount = 0;

	m_bSystemInitialized = TRUE;

	return TRUE;
}

void CBotKrunk::UninitSystem( void ) {

	if( m_bSystemInitialized ) {
		m_bSystemInitialized = FALSE;
	}
}

BOOL CBotKrunk::ClassHierarchyLoadSharedResources( void ) {
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
	u32 i;
	FResFrame_t ResFrame = fres_GetFrame();

	//default for the CSV defaults
	m_BotInfo_Walk.fHopLRImpulseMag = 12.0f;
	m_BotInfo_Walk.fHopAnimLandPct = 0.75f;
	m_BotInfo_Walk.fApproxHopLRDist = 3.0f;

	m_BotInfo_Walk.fRollStickForceWhileRolling = 1.0f;
	m_BotInfo_Walk.fRollLRImpulseMagXZ = 23.0f;
	m_BotInfo_Walk.fRollLRImpulseMagY = 6.0f;
	m_BotInfo_Walk.fRollAnimLandPct = 0.85f;
	m_BotInfo_Walk.fApproxRollLRDist= 15.0f;

	if( !ReadBotInfoFile( m_aGameDataMap, _pszBotInfoFilename ) ) {
		goto _ExitInitSystemWithError;
	}
//	if( !ReadBotIdleFile( &m_BotInfo_Idle,&m_nBotInfo_Idles, _pszBotInfoFilename ) ) {
//		goto _ExitInitSystemWithError;
//	}
	ReadBotIdleFile( &m_BotInfo_Idle,&m_nBotInfo_Idles, _pszBotInfoFilename );// allow there to be no idle table

	if( m_nBotInfo_Idles ) {
		m_apszIdleAnimNameTable = (cchar**)fres_AllocAndZero( sizeof(cchar*) * m_nBotInfo_Idles );
		if( !m_apszIdleAnimNameTable ) {
			goto _ExitInitSystemWithError;
		}
		
		m_apnEnableBoneNameIndexTableForEachIdleTap = (const u8**)fres_AllocAndZero( sizeof(u8*) * m_nBotInfo_Idles );
		if( !m_apnEnableBoneNameIndexTableForEachIdleTap ) {
			goto _ExitInitSystemWithError;
		}

		for( u32 uIndex=0; uIndex < m_nBotInfo_Idles; uIndex++ ) {
			m_apszIdleAnimNameTable[uIndex] = m_BotInfo_Idle[uIndex].pszIdleName;
			m_apnEnableBoneNameIndexTableForEachIdleTap[uIndex]= m_aBoneEnableIndices_FullBody;
		}
	}

	if( !_BuildAnimStackDef() ) {
		goto _ExitInitSystemWithError;
	}

	fang_MemZero( &m_KrunkFootStepSounds, sizeof( m_KrunkFootStepSounds ) );

	// Load the sound effects bank for this bot...
	if( !fresload_Load( FSNDFX_RESTYPE, _pszSoundEffectBank) ) {
		DEVPRINTF( "CBotKrunk::ClassHierarchyLoadSharedResources(): Could not load sound effect bank '%s'\n", _pszSoundEffectBank);
	}

	// fixup the peg leg footsteps
	i = 0;
	m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_CONCRETE][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_METAL][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_METAL_GRATE][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_GLASS][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_COMPOSITE][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_ELECTRONICS][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_WATER][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_GOOP][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_ACID][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_FORCE_FIELD][i] = fsndfx_GetFxHandle( "KC_MetalStep01" );
	m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_DIRT][i] = fsndfx_GetFxHandle( "KC_DirtStep01" );
	m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_JUNK][i] = fsndfx_GetFxHandle( "KC_JunkStep01" );
	m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_ROCK][i] = fsndfx_GetFxHandle( "KC_RockStep01" );
	i = 1;
	m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_CONCRETE][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_METAL][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_METAL_GRATE][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_GLASS][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_COMPOSITE][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_ELECTRONICS][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_WATER][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_GOOP][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_ACID][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_FORCE_FIELD][i] = fsndfx_GetFxHandle( "KC_MetalStep02" );
	m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_DIRT][i] = fsndfx_GetFxHandle( "KC_DirtStep02" );
	m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_JUNK][i] = fsndfx_GetFxHandle( "KC_JunkStep02" );
	m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_ROCK][i] = fsndfx_GetFxHandle( "KC_RockStep02" );
	i = 2;
	m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_CONCRETE][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_METAL][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_METAL_GRATE][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_GLASS][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_COMPOSITE][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_ELECTRONICS][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_WATER][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_GOOP][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_ACID][i] = 
		m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_FORCE_FIELD][i] = fsndfx_GetFxHandle( "KC_MetalStep03" );
	m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_DIRT][i] = fsndfx_GetFxHandle( "KC_DirtStep03" );
	m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_JUNK][i] = fsndfx_GetFxHandle( "KC_JunkStep03" );
	m_KrunkFootStepSounds.ahStepSound[SURFACE_TYPE_ROCK][i] = fsndfx_GetFxHandle( "KC_RockStep03" );

	// load the flame particle file
	m_hFlameParticleDef = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, _pszFlameParticleFile );
	if( m_hFlameParticleDef == FPARTICLE_INVALID_HANDLE ) {
		DEVPRINTF( "CBotKrunk::ClassHierarchyLoadSharedResources(): Could not find particle definition '%s'.\n", _pszFlameParticleFile );
		goto _ExitInitSystemWithError;
	}
	
	return TRUE;

	// Error:
_ExitInitSystemWithError:
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( ResFrame );

	return FALSE;
}

void CBotKrunk::ClassHierarchyUnloadSharedResources( void ) {
	FASSERT( m_nBotClassClientCount > 0 );

	--m_nBotClassClientCount;

	if( m_nBotClassClientCount == 0 ) {
		m_AnimStackDef.Destroy();

		fang_MemZero( &m_KrunkFootStepSounds, sizeof( m_KrunkFootStepSounds ) );
		m_hFlameParticleDef = FPARTICLE_INVALID_HANDLE;	
	}

	CBot::ClassHierarchyUnloadSharedResources();
}

CBotKrunk::CBotKrunk() : CBot() {
	m_pInventory = NULL;
	m_pWorldMesh = NULL;
}

CBotKrunk::~CBotKrunk() {

	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}

BOOL CBotKrunk::Create( s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName,
					   const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	if( !ClassHierarchyLoadSharedResources() ) {
		// Failure! (resources have already been cleaned up, so we can bail now)
		return FALSE;
	}

	// Get pointer to the leaf class's builder object...
	CBotKrunkBuilder *pBuilder = (CBotKrunkBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_BotKrunkBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_KRUNK );
	}

	// Set our builder parameters...

	// Create an entity...
	return CBot::Create( &m_BotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}

void CBotKrunk::ClassHierarchyDestroy( void ) { 
	
	// Delete the items that we had instantiated for us...
	fdelete( m_apWeapon[0] );
	m_apWeapon[0] = NULL;

	fdelete( m_apWeapon[1] );
	m_apWeapon[1] = NULL;

	m_Anim.Destroy();

	fdelete( m_pWorldMesh );
	m_pWorldMesh = NULL;

	CBot::ClassHierarchyDestroy();
}

BOOL CBotKrunk::ClassHierarchyBuild( void ) {
	FMesh_t *pMesh;
	FMeshInit_t MeshInit;
	u32 i;
	s32 nBoneIndex;
	cchar * pszMeshFilename = NULL;

	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CBotKrunkBuilder *pBuilder = (CBotKrunkBuilder *)GetLeafClassBuilder();

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

	// Load mesh resource...
	pszMeshFilename = _apszKrunkMeshFilenames[0];
	if( pBuilder->m_uMeshVersionOverride >0 &&
		pBuilder->m_uMeshVersionOverride < sizeof(_apszKrunkMeshFilenames)/sizeof(char*) ) {
		pszMeshFilename = _apszKrunkMeshFilenames[pBuilder->m_uMeshVersionOverride];
	}
	if( pBuilder->m_pszMeshReplacement ) {
	   pszMeshFilename = pBuilder->m_pszMeshReplacement;
	}
	pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pszMeshFilename );
	if( pMesh == NULL ) {
		DEVPRINTF( "CBotKrunk::ClassHierarchyBuild(): Could not find mesh '%s'\n", pszMeshFilename );
		goto _ExitWithError;
	}

	// Create world mesh...
	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL ) {
		DEVPRINTF( "CBotKrunk::ClassHierarchyBuild(): Not enough memory to create CFWorldMesh.\n" );
		goto _ExitWithError;
	}

	// Init the world mesh...
	MeshInit.pMesh = pMesh;
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

	// Build our animation stack and load animations...
	if( !m_Anim.Create( &m_AnimStackDef, m_pWorldMesh ) ) {
		DEVPRINTF( "CBotKrunk::ClassHierarchyBuild(): Trouble creating m_Anim.\n" );
		goto _ExitWithError;
	}
	for( i=0; i < m_nBotInfo_Idles; i++ ) {
		m_Anim.IdleAnim_EnableControlSmoothing( i );
	}
	SetControlValue( ANIMCONTROL_STAND, 1.0f );
	UpdateUnitTime( ANIMTAP_STAND, fmath_RandomFloat() );

	m_nBoneIndexGroin = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_GROIN] );
	m_nBoneIndexTorso = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_TORSO] );
	m_Anim.m_pAnimCombiner->SetBoneCallback( &_AnimBoneCallback );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_GROIN] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_TORSO] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_HEAD] );

	// grab the dust emitter bones
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_L_FOOT] );
	if( nBoneIndex >= 0 ) {
		m_pLeftFootDustPos_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;

		nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_R_FOOT_DUMMY] );
		if( nBoneIndex >= 0 ) {
			m_pRightFootDustPos_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
		} else {
			m_pLeftFootDustPos_WS = NULL;
		}
	}

	// find the torch bone
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_TORCHDUMMY] );
	if( nBoneIndex >= 0 ) {
        m_pTorchMtx = m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex];
	}
	m_fSecsUntilNextBurner = 0.0f;

	// create the animation stack
	if( !m_AnimManFrameAim.Create( m_AnimStackDef.m_nBoneCount, m_AnimStackDef.m_apszBoneNameTable ) ) {
		DEVPRINTF( "CBotKrunk::ClassHierarchyCreate(): Could not create aim animation summer.\n" );
		goto _ExitWithError;
	}
	AttachAnim( ANIMTAP_AIM_SUMMER, &m_AnimManFrameAim );
	m_AnimManFrameAim.Reset();
	UpdateBoneMask( ANIMTAP_AIM_SUMMER, m_aBoneEnableIndices_AimSummer, TRUE );

	if( m_nPossessionPlayerIndex >= 0 )
	{
		Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.SetNormOrigin( 0.0f, 0.23f );
		m_pInventory = CPlayer::GetInventory(m_nOwnerPlayerIndex,TRUE); // gets the possession inventory
		if( m_pInventory == NULL ) 
		{
			// What is going on here?!  This shouldn't happen!
			DEVPRINTF( "CBotBlink::_InitInventory() : Could not get inventory from CPlayer.\n" );
			return FALSE;
		}

		m_pInventory->SetToKrunk();
		m_pInventory->m_pUser = this;
		m_pInventory->m_pfcnCallback = _InventoryCallback;
		CHud2::GetHudForPlayer(m_nPossessionPlayerIndex)->SetHudMode(HUDMODE_KRUNK);
		CWeapon *pWeap;
		s32 uCurWpnIdx;
		// Set primary and secondary weapon.........
		for( i=0; i<2; ++i )
		{
			// step through primary and secondary weapons
			for( uCurWpnIdx=0; uCurWpnIdx < m_pInventory->m_auNumWeapons[i]; ++uCurWpnIdx )
			{
				// step through all weapons in ItemInst inventory
				// create a weapon object from the ItemInst parameters
				pWeap = m_pInventory->m_aoWeapons[i][uCurWpnIdx].MakeWeapon();
				if( pWeap == NULL )
				{
					continue;
				}
				// set weapon states
				pWeap->EnableAutoWork( FALSE );
				pWeap->RemoveFromWorld();
				pWeap->SetOwner(this);

				if (pWeap->GetUpgradeLevelCount() > 3)
				{
					pWeap->SetUpgradeLevel(3);	  //3 is special level for npc
				}
				else
				{
					pWeap->SetUpgradeLevel(0);	  //3 is special level for npc
				}

				// give weapon a pointer back to its associated ItemInst object
				pWeap->SetItemInst( &m_pInventory->m_aoWeapons[i][uCurWpnIdx] );

				m_apWeapon[i] = pWeap;
			}
		}
		m_pInventory->m_aoWeapons[0][0].SetAmmoDisplayType( CItemInst::AMMODISPLAY_NONE,CItemInst::AMMODISPLAY_NONE );
	}
	else
	{
		for (i = 0; i < 2; i++) 
		{
			m_apWeapon[i] = CBotBuilder::AllocNPCWeaponOfType(pBuilder->m_anNPCWeaponType[i]);
			if( m_apWeapon[i] == NULL ) {
				DEVPRINTF( "CBotGrunt::ClassHierarchyBuild() : Error instantiating CWeapon %d.\n", i );
				goto _ExitWithError;
			}
			if (!CBotBuilder::CreateNPCWeaponOfType(m_apWeapon[i], pBuilder->m_anNPCWeaponType[i]))
			{
				DEVPRINTF( "CBotGrunt::ClassHierarchyBuild() : Error Creating CWeapon %d.\n", i );
				goto _ExitWithError;
			}

			m_apWeapon[i]->EnableAutoWork( FALSE );
			m_apWeapon[i]->RemoveFromWorld();
			m_apWeapon[i]->SetOwner(this);

			if (m_apWeapon[i]->GetUpgradeLevelCount() > 3)
			{
				m_apWeapon[i]->SetUpgradeLevel(3);	  //3 is special level for npc
			}
			else
			{
				m_apWeapon[i]->SetUpgradeLevel(0);	  //3 is special level for npc
			}
			m_apWeapon[i]->SetClipAmmo( 50 );
			m_apWeapon[i]->SetReserveAmmo( (u16)CWeapon::INFINITE_AMMO );
		}
	}
	
	//primary must be attached
	m_apWeapon[0]->Attach_UnitMtxToParent_PS( this, m_apszBoneNameTable[BONE_ATTACHPOINT_PRIMARY] );
	
	SetMaxHealth();

	// set up part mgr...
	if( !m_pPartMgr->Create( this, &m_pPartPool, _BOTPART_FILENAME, PART_INSTANCE_COUNT_PER_TYPE, LIMB_TYPE_COUNT ) ) {
		goto _ExitWithError;
	}


	// Find approx eye point...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CBotKrunk::ClassHierarchyBuild(): Could not locate approx eye point bone '%s'.\n", m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
		m_pApproxEyePoint_WS = &m_MtxToWorld.m_vPos;
	} else {
		m_pApproxEyePoint_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
	}

	// Find gaze direction...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_HEAD ] );
	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CBotKrunk::ClassHierarchyBuild(): Could not locate gaze bone '%s'.\n", m_apszBoneNameTable[ BONE_HEAD ] );
		m_pGazeDir_WS = &m_MtxToWorld.m_vFront;
	} else {
		m_pGazeDir_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vFront;
	}

	// Initialize matrix palette (this is important for attached entities)...
	AtRestMatrixPalette();

	// Create tag points...
	if( !TagPoint_CreateFromBoneArray( m_anTagPointBoneNameIndexArray, m_apszBoneNameTable ) ) {
		goto _ExitWithError;
	}

	// set up player damage/death
	m_PlayerDamageData.uFirstDamageLimb	= -1;
	m_PlayerDamageData.uNumDamageLimbs	= 0;
	m_PlayerDamageData.uFirstDeathLimb	= LIMB_TYPE_HEAD;
	m_PlayerDamageData.uNumDeathLimbs	= LIMB_TYPE_NUM_DEATH_LIMBS;
	m_PlayerDamageData.fDeathTimer		= 0.0f;
	m_PlayerDamageData.fNextDeathEvent	= 0.0f;

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

BOOL CBotKrunk::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CBot::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	EnableOurWorkBit();

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

CEntityBuilder *CBotKrunk::GetLeafClassBuilder( void ) {
	return &_BotKrunkBuilder;
}

void CBotKrunk::_ClearDataMembers( void ) {
	// Init data members...

	m_pMoveIdentifier = &m_nBoneIndexGroin;

	m_fCollCylinderRadius_WS = 2.0f;
	m_fCollCylinderHeight_WS = 4.0f;

	m_pBotInfo_Gen = &m_BotInfo_Gen;
	m_pBotInfo_MountAim = &m_BotInfo_MountAim;
	m_pBotInfo_Walk = &m_BotInfo_Walk;
	m_pBotInfo_Jump = &m_BotInfo_Jump;
	m_pBotInfo_Weapon = &m_BotInfo_Weapon;
	m_pBotInfo_Idle = m_BotInfo_Idle;
	m_nCountIdles = m_nBotInfo_Idles;

	m_fGravity = m_pBotInfo_Gen->fGravity;
	m_fMaxFlatSurfaceSpeed_WS = m_pBotInfo_Walk->fMaxXlatVelocity * m_fRunMultiplier;

	m_fMountPitchMax_WS = m_pBotInfo_MountAim->fMountPitchDownLimit;
	m_fMountPitchMin_WS = m_pBotInfo_MountAim->fMountPitchUpLimit;

	m_anAnimStackIndex[ASI_STAND]							= ANIMTAP_STAND;
	m_anAnimStackIndex[ASI_WALK]							= ANIMTAP_WALK;
	m_anAnimStackIndex[ASI_RUN]								= ANIMTAP_RUN;
	m_anAnimStackIndex[ASI_FALL]							= ANIMCONTROL_JUMP_FLY;
	m_anAnimStackIndex[ASI_RELOAD_TETHER_PREPARE_TO_PUMP]	= -1;
	m_anAnimStackIndex[ASI_RELOAD_TETHER_PUMP]				= -1;
	m_anAnimStackIndex[ASI_RELOAD_TETHER_GRAB_AMMO]			= ANIMCONTROL_RELOAD_CLIP_GRAB_NEW;
	m_anAnimStackIndex[ASI_RELOAD_TETHER_DROP_IN]			= ANIMCONTROL_RELOAD_CLIP_INSERT_NEW;
	m_anAnimStackIndex[ASI_RELOAD_TETHER_FINISH]			= ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW;
	m_anAnimStackIndex[ASI_DISASSEMBLE]						= ANIMCONTROL_DISASSEMBLE;
	m_anAnimStackIndex[ASI_ASSEMBLE]						= ANIMCONTROL_ASSEMBLE;
	
	m_pnEnableBoneNameIndexTableForSummer_Normal = m_anEnableBoneNameIndexTableForSummer_Normal;
	m_pnEnableBoneNameIndexTableForSummer_TetherShock = NULL;

	m_fUnitTetherShockVibration = 0.0f;

	m_pTorchMtx = NULL;
	m_fSecsUntilNextBurner = 0.0f;

	// shouldn't be hard-coded...
	m_pBotInfo_Jump->fFlipOriginX = 0.0f;
	m_pBotInfo_Jump->fFlipOriginY = 3.8f;
	m_pBotInfo_Jump->fFlipOriginZ = 0.47f;

	m_bInPieces = FALSE;
	m_bFallDown = FALSE;
	m_bFallAnimDone = TRUE;
	m_bOnGround = FALSE;
	m_fTimeInPieces = 0.0f;
	m_fUnitPiecesBlend = 0.0f;
	m_bColWithPieces = FALSE;

	m_pPlayerDeathData = &m_PlayerDamageData;

}

void CBotKrunk::ClassHierarchyAddToWorld( void ) {
	u32 i;

	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CBot::ClassHierarchyAddToWorld();
	m_pWorldMesh->UpdateTracker();

	m_uBotDeathFlags |= BOTDEATHFLAG_PLAYDEATHANIM | BOTDEATHFLAG_COMEAPART | BOTDEATHFLAG_AUTOPERSISTAFTERDEATH;
	for( i=0; i<2; i++ ) {
		if( m_apWeapon[i] ) {
			if (m_nPossessionPlayerIndex < 0) // NPC HACK DON"T DRWA THE TETHER
				m_apWeapon[i]->DrawEnable(FALSE);
			else
			{
				m_apWeapon[i]->Attach_UnitMtxToParent_PS( this, m_apszBoneNameTable[BONE_ATTACHPOINT_PRIMARY] );
				m_apWeapon[i]->AddToWorld();
				m_apWeapon[i]->ResetToState( CWeapon::STATE_DEPLOYED );
			}
		}
	}	
}

void CBotKrunk::ClassHierarchyRemoveFromWorld( void ) {
	u32 i;

	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );
	
	for( i=0; i<2; i++ ) {
		if( m_apWeapon[i] ) {
			m_apWeapon[i]->RemoveFromWorld();
		}
	}

	m_pWorldMesh->RemoveFromWorld();
	CBot::ClassHierarchyRemoveFromWorld();
}

void CBotKrunk::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	FASSERT( IsCreated() );
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = m_pWorldMesh;

	if( m_apWeapon[0] ) {
		m_apWeapon[0]->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}

	if( m_pDataPortMeshEntity ) {
		m_pDataPortMeshEntity->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}
}

const CFVec3A *CBotKrunk::GetApproxEyePoint( void ) const {
	FASSERT( IsCreated() );

	return m_pApproxEyePoint_WS;
}

void CBotKrunk::ClassHierarchyWork() {
	CFVec3A TempVec3A;

	FASSERT( m_bSystemInitialized );

	CBot::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}

	ParseControls();

	// Save a copy of the previous frame's info...
	m_MountPrevPos_WS = m_MountPos_WS;
	m_nPrevState = m_nState;

	// Apply velocity impulses that were accumulated last frame...
	BOOL bImpulseApplied = HandleVelocityImpulses();

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
	if( bImpulseApplied ) {
		VelocityHasChanged();
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
//			HandleHopRollStartleGeneric();
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
		HandleTargeting();
		_HandleWeaponFiring();
		_HandleWeaponAnimations();
	PROTRACK_ENDBLOCK();//"WeaponAnim");

	_UpdateMatrices();

	PROTRACK_BEGINBLOCK("WeaponWork");
		_HandleWeaponWork();
	PROTRACK_ENDBLOCK();//"WeaponWork");

	BotInPiecesWork();
	
	if( IsDeadOrDying() ) {
		DeathWork();
	}

	_SpawnBurners();
}

void CBotKrunk::_HandleDoubleJump( void ) {

	if( m_bControls_Jump && m_Velocity_WS.y >= 0.0f ) {

		if( m_nJumpState==BOTJUMPSTATE_LAUNCH || m_nJumpState==BOTJUMPSTATE_AIR ) {
			_StartDoubleJump();
			if( m_nPossessionPlayerIndex >= 0 ) {
//				fsndfx_Play2D( CBotKrunk::GetSloshSndHandle(CBotKrunk::SND_BOOST),1.0f);
			} else {
//				fsndfx_Play3D( CBotKrunk::GetSndHandle(CBotKrunk::SND_BOOST), &MtxToWorld()->m_vPos, 100);
			}
		}
	}
}

void CBotKrunk::_HandleJumping( void ) {

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

void CBotKrunk::_EnterFlyMode( void ) {
	m_bPlayLandAnim = FALSE;
	m_bPlayLandAnimLower = FALSE;

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );

	m_nJumpState = BOTJUMPSTATE_AIR;
	SetJumping();
}

void CBotKrunk::_StartVelocityJump( const CFVec3A *pJumpVelocity_WS ) {
	CFVec3A JumpVelocity_WS = *pJumpVelocity_WS;
	FMATH_CLAMP( JumpVelocity_WS.y, 0.0f, m_fMaxDeltaJumpVelY_WS );

	_StartSingleJump( &JumpVelocity_WS );

	if( pJumpVelocity_WS->y >= m_pBotInfo_Jump->fVerticalVelocityJump1 ) {
		_StartDoubleJump( FALSE );
	}
}

void CBotKrunk::_StartSingleJump( const CFVec3A *pJumpVelocity_WS ) {

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
	SetDoubleJumpTimer( m_pBotInfo_Gen->fDoubleJumpTime );
}

void CBotKrunk::_StartDoubleJump( BOOL bAddToVelocityY ) {
	if( !HaveTimeToDoubleJump() ) {
		return;
	}

//	f32 fOOTuckUntuckAnimSpeedMult;
//	f32 fJumpSecs, fAnimSecs, fVelY;

	if( bAddToVelocityY ) {
		m_Velocity_WS.y += m_pBotInfo_Jump->fVerticalVelocityJump2 * m_fJumpMultiplier;
		m_Velocity_MS.y += m_pBotInfo_Jump->fVerticalVelocityJump2 * m_fJumpMultiplier;
	}
/*
	FASSERT( m_pBotInfo_Jump->fTuckUntuckAnimSpeedMult != 0.f );
	fOOTuckUntuckAnimSpeedMult = fmath_Inv( m_pBotInfo_Jump->fTuckUntuckAnimSpeedMult );

	fAnimSecs = GetTotalTime( ANIMTAP_JUMP_BOOST ) * fOOTuckUntuckAnimSpeedMult;

	fVelY = m_pBotInfo_Jump->fVerticalVelocityJump2;

	fJumpSecs = fmath_Div( -2.0f * fVelY, m_pBotInfo_Gen->fGravity );

	if( fAnimSecs <= fJumpSecs ) 
	{
		m_fTuckUntuckAnimTimeMult = 1.0f;
		m_fUntuckStartCountdownSecs = fJumpSecs - (fTuckSecs + fUntuckSecs);
	}
	else
	{
		m_fTuckUntuckAnimTimeMult = fmath_Div( fAnimSecs, fJumpSecs );
		m_fUntuckStartCountdownSecs = 0.0f;
	}
*/
	m_nJumpState = BOTJUMPSTATE_AIR2;
	SetJumping();
}

void CBotKrunk::_JumpLanded( void ) {
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
	
	if( m_nPossessionPlayerIndex >= 0 ) {
		// Don't play scuff sounds when landing from a jump
		if( m_pBotInfo_Sound && m_nSurfaceTypeOn != SURFACE_TYPE_NONE ) {
			fsndfx_Play2D(
				m_pBotInfo_Sound->ahStepSound[m_nSurfaceTypeOn][ fmath_RandomRange( 0, ( BOTSOUND_SCUFF_SOUND_START - 1 ) ) ],
				m_pBotInfo_Walk->fJumpLandedSoundUnitVolume_2D * m_fStepVolumeScale
			);
		}
		//tell the AI that this sound happened
		AIEnviro_BoostPlayerSoundTo( m_nPossessionPlayerIndex, 35.0f );
	} else {
		if( m_pBotInfo_Sound && m_nSurfaceTypeOn != SURFACE_TYPE_NONE ) {
			// Don't play scuff sounds when landing from a jump
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

	if( m_pStickyEntity && m_pStickyEntity->GetVerlet() ) {
		CFVec3A ImpulseVec_WS;

		ImpulseVec_WS.Set( 0.0f, -m_pBotInfo_Gen->fPhysForce_Land, 0.0f );
		m_pStickyEntity->GetVerlet()->ApplyImpulse( &m_MtxToWorld.m_vPos, &ImpulseVec_WS );
	}
}

void CBotKrunk::_HandleJumpAnimations( void ) {
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
				// in the air from a single jump
				DeltaTime( ANIMTAP_JUMP_FLY );
				break;

			case BOTJUMPSTATE_AIR2:
				// in the air from a double jump
				fUnitTime = GetUnitTime( ANIMTAP_JUMP_LAUNCH );
				if( fUnitTime < 0.99f ) {
					// still launching, quickly move to flying
					if( DeltaTime( ANIMTAP_JUMP_LAUNCH, FLoop_fPreviousLoopSecs, TRUE ) ) {
						// done launching
						SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );
						SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
						ZeroTime( ANIMTAP_JUMP_FLY );
					}
				} else {
					DeltaTime( ANIMTAP_JUMP_FLY );
				}
				break;

			default:
				FASSERT_NOW;
				break;
			}
		}
	}
}

void CBotKrunk::_HandleWeaponWork( void ) {

	if( m_apWeapon[0] ) {
		m_apWeapon[0]->Work();
	}

	if( m_apWeapon[1] ) {
		m_apWeapon[1]->Work();
	}
}

void CBotKrunk::_HandleFiringAnimations( void ) {
	f32 fUnitFireControlValue;

	// Primary fire...
	u32 uFireAnimControlId = ANIMCONTROL_FIRE_1;
	u32 uFireAnimTapId = ANIMTAP_FIRE_1;
		
	if( m_nBotFlags & BOTFLAG_PLAY_FIRE1_ANIM ) {
		fUnitFireControlValue = GetControlValue( uFireAnimControlId );

		if( fUnitFireControlValue < 1.0f ) {
			fUnitFireControlValue += m_pBotInfo_Weapon->fFire1BlendInSpeed * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMAX( fUnitFireControlValue, 1.0f );
			SetControlValue( uFireAnimControlId, fUnitFireControlValue );
		}

		if( !(m_nBotFlags & BOTFLAG_PLAY_TETHER_VIBRATE) ) {
			// Playing standard fire anim...
			if( DeltaTime( uFireAnimTapId, FLoop_fPreviousLoopSecs, TRUE ) ) {
				FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM );
			}
		} else {
			// Playing special tether zap vibration...

			f32 fUnitAnim;

			m_fUnitTetherShockVibration += FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMAX( m_fUnitTetherShockVibration, 1.0f );

			//fUnitAnim = 0.05f*fmath_RandomFloat() + 0.25f*m_fUnitTetherShockVibration;
			fUnitAnim = 0.40f + (fmath_RandomFloat() * 0.35f) + (0.25f * m_fUnitTetherShockVibration);
			FMATH_CLAMP( fUnitAnim, 0.0f, 1.0f );

			UpdateUnitTime( uFireAnimTapId, fUnitAnim );
		}
	} else {
		fUnitFireControlValue = GetControlValue( uFireAnimControlId );

		if( fUnitFireControlValue > 0.0f ) {
			fUnitFireControlValue -= m_pBotInfo_Weapon->fFire1BlendOutSpeed * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( fUnitFireControlValue, 0.0f );
			SetControlValue( uFireAnimControlId, fUnitFireControlValue );
		}
	}
		
	// Secondary fire...
	if( m_nThrowState != THROWSTATE_NONE ) {
		f32 fUnitTime;

		if( DeltaTime( ANIMTAP_FIRE_2, 2.0f * FLoop_fPreviousLoopSecs, TRUE ) ) {
			fUnitTime = 1.0f;
			fUnitFireControlValue = 0.0f;
		} else {
			fUnitTime = GetUnitTime( ANIMTAP_FIRE_2 );

			if( fUnitTime < m_pBotInfo_Weapon->fThrowBlendInUnitTime ) {
				fUnitFireControlValue = fUnitTime * m_pBotInfo_Weapon->fOOThrowBlendInUnitTime;
			} else if( fUnitTime > m_pBotInfo_Weapon->fThrowBlendOutUnitTime ) {
				fUnitFireControlValue = 1.01f - (fUnitTime - m_pBotInfo_Weapon->fThrowBlendOutUnitTime) * m_pBotInfo_Weapon->fOOInvThrowBlendOutUnitTime;
			} else {
				fUnitFireControlValue = 1.0f;
			}
		}

		SetControlValue( ANIMCONTROL_FIRE_2, fUnitFireControlValue );

		switch( m_nThrowState ) {

		case THROWSTATE_REACHING_FOR_AMMO:
			if( fUnitTime <= 0.17f ) {
				break;
			}

			// Attach ammo...
			m_apWeapon[1]->Throwable_AttachGrenadeToOwnerBotBone( m_apszBoneNameTable[BONE_L_HAND_DUMMY] );
			m_nThrowState = THROWSTATE_WINDING_UP;

			// Fall through to next state...

		case THROWSTATE_WINDING_UP:
			if( fUnitTime <= 0.60f ) {
				break;
			}

			// Attach ammo...
			m_apWeapon[1]->Throwable_ThrowGrenade_MountAimDirection();
			m_nThrowState = THROWSTATE_RECOVERING;

			// Fall through to next state...

		case THROWSTATE_RECOVERING:
			if( fUnitTime >= 1.0f ) {
				m_nThrowState = THROWSTATE_NONE;
			}

			break;
		};
	}
}

void CBotKrunk::_HandleAimAnimations( void ) {
	f32 fUnitAimPitch;
	CFAnimFrame TorsoQuat, HeadQuat;

	// Compute the amount we're aiming up/down (up=1, down=0, middle=0.5)...
	fUnitAimPitch = fmath_Div( m_fAimPitch_WS - m_fMountPitchMin_WS, m_fMountPitchMax_WS - m_fMountPitchMin_WS );
	FMATH_CLAMP_UNIT_FLOAT( fUnitAimPitch );

	// Update the torso-twist blend value...
	if( m_fUnitTorsoTwistBlend > 0.0f ) {
		m_fUnitTorsoTwistBlend -= 2.0f * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fUnitTorsoTwistBlend, 0.0f );
	}

	// Compute the head and torso quaternions...
	TorsoQuat.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( fUnitAimPitch, FMATH_DEG2RAD(-30.0f), FMATH_DEG2RAD(30.0f) ), CFVec3A::m_Null );

	if( m_fUnitTorsoTwistBlend == 0.0f ) {
		// No torso twist...
		// Torso quaternion has already been computed.
		// Compute head quaternion...

		HeadQuat.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( fUnitAimPitch, FMATH_DEG2RAD(-20.0f), FMATH_DEG2RAD(20.0f) ), CFVec3A::m_Null );
	} else {
		// We must introduce some rocket-launcher-L1 torso twist...

		if( m_fUnitTorsoTwistBlend == 1.0f ) {
			// 100% torso twist...

			// Compute the head quaternion...
		} else {
			// Partial torso twist...

			// Compute the torso twist quaternion...
			CFAnimFrame TorsoTwistQuat;
			TorsoTwistQuat.BuildQuat( CFVec3A::m_UnitAxisY, fmath_UnitLinearToSCurve(m_fUnitTorsoTwistBlend) * FMATH_DEG2RAD(55.0f), CFVec3A::m_Null );
			TorsoQuat.Mul( TorsoTwistQuat );

			// Compute the head quaternion...
			TorsoTwistQuat.Negate();
			CFAnimFrame TempQuat;
			TempQuat.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( fUnitAimPitch, FMATH_DEG2RAD(20.0f), FMATH_DEG2RAD(-20.0f) ), CFVec3A::m_Null );
			HeadQuat.Mul( TempQuat, TorsoTwistQuat );
		}
	}

	// Apply head and torso quaternions...
	m_AnimManFrameAim.UpdateFrame( BONE_HEAD, HeadQuat );
	m_AnimManFrameAim.UpdateFrame( BONE_TORSO, TorsoQuat );

	// Compute the head quaternion...

}

void CBotKrunk::_HandleWeaponAnimations( void ) {
	if( m_nWRState == WR_STATE_TETHER_GRAB_AMMO ) {
		// blend the grab animation in
		f32 fUnitTime = 6.0f * fmath_UnitLinearToSCurve( GetUnitTime( ASI_RELOAD_TETHER_GRAB_AMMO ) );
		FMATH_CLAMPMAX( fUnitTime, 1.0f );
		SetControlValue( ASI_RELOAD_TETHER_GRAB_AMMO, fUnitTime );
	}
	HandleWeaponReloadingAnimations();
	_HandleFiringAnimations();
	_HandleAimAnimations();
}

void CBotKrunk::_UpdateMatrices( void ) {
	CFMtx43A FlipMtx, MeshMtx, NewMtx, EntityMtxToWorld;

	CFMtx43A::m_XlatRotY.SetRotationY( m_fMountYaw_WS + m_fLegsYaw_MS );
	CFMtx43A::m_XlatRotY.m_vPos = m_MountPos_WS;

	// Update xfm...
	if( m_fFlipPitch ) {
		CFMtx43A::m_XlatRotX.m_vPos.Set( m_pBotInfo_Jump->fFlipOriginX, 
			m_pBotInfo_Jump->fFlipOriginY, 
			m_pBotInfo_Jump->fFlipOriginZ );

		if( m_fFlipPitch >= 0.0f ) {
			CFMtx43A::m_XlatRotX.SetRotationX( fmath_UnitLinearToSCurve( m_fFlipPitch * (1.0f/FMATH_2PI) ) * FMATH_2PI );
		} else {
			CFMtx43A::m_XlatRotX.SetRotationX( fmath_UnitLinearToSCurve( m_fFlipPitch * (-1.0f/FMATH_2PI) ) * -FMATH_2PI );
		}

		CFMtx43A::m_Xlat.m_vPos.ReceiveNegative( CFMtx43A::m_XlatRotX.m_vPos );

		FlipMtx.Mul( CFMtx43A::m_XlatRotX, CFMtx43A::m_Xlat );

		MeshMtx.Mul( CFMtx43A::m_XlatRotY, FlipMtx );
		m_pWorldMesh->m_Xfm.BuildFromMtx( MeshMtx );

		CFMtx43A::m_XlatRotY.SetRotationY( m_fMountYaw_WS );
	} else {
		m_pWorldMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_XlatRotY );
	}

	m_pWorldMesh->UpdateTracker();

	PROTRACK_BEGINBLOCK("ComputeMtxPal");
		// Update bone matrix palette...
		m_pCollBot = this;
		m_Anim.m_pAnimCombiner->ComputeMtxPalette( TRUE );
	PROTRACK_ENDBLOCK();//"ComputeMtxPal");

	m_GazeUnitVec_WS.ReceiveUnit( *m_pGazeDir_WS );

	EntityMtxToWorld.Identity();
	EntityMtxToWorld.SetRotationYXZ( m_fMountYaw_WS, m_fMountPitch_WS, 0.0f );
	EntityMtxToWorld.m_vPos = m_MountPos_WS;
	Relocate_RotXlatFromUnitMtx_WS( &EntityMtxToWorld, TRUE, m_pMoveIdentifier );
}

// Returns TRUE if Krunk's finger is on his primary weapon's trigger.
// In other words, TRUE is returned if Krunk is in a state where he
// can pull the trigger.
BOOL CBotKrunk::_IsFingerOnTrigger1( void ) const
{
	BOOL bFingerIsOnTrigger = FALSE;

	if( m_nWSState==WS_STATE_NONE || m_nWSState==WS_STATE_RELEASING ) {
		// We're not switching or reloading the weapon...

		if( (m_fFlipPitch == 0.0f) || (fmath_Abs(m_fLegsYaw_MS) > (0.8f * FMATH_HALF_PI)) ) {
			// We're not flipping...

			switch( m_apWeapon[0]->m_pInfo->nGrip ) {

			case CWeapon::GRIP_RIGHT_ARM:
				bFingerIsOnTrigger = TRUE;
				break;

			case CWeapon::GRIP_BOTH_ARMS:
				if( m_nJumpState == BOTJUMPSTATE_NONE ) {
					f32 fLandBlend = GetControlValue( ANIMCONTROL_JUMP_LAND_UPPER );

					if( fLandBlend == 0.0f ) {
						if( m_nState == STATE_GROUND ) {
							bFingerIsOnTrigger = TRUE;
						} else {
							if( m_fUnitFlyBlend == 0.0f ) {
								bFingerIsOnTrigger = TRUE;
							}
						}
					}
				}
				break;

			default:
				FASSERT_NOW;
			};
		}
	}

	return bFingerIsOnTrigger;
}

// Returns TRUE if Krunk's finger is on his secondary weapon's trigger.
// In other words, TRUE is returned if Krunk is in a state where he
// can pull the trigger.
BOOL CBotKrunk::_IsFingerOnTrigger2( void ) const
{
	BOOL bFingerIsOnTrigger = FALSE;

	if( m_nThrowState == THROWSTATE_NONE ) {
		// Not currently throwing a secondary grenade...

		if( m_nWSState==WS_STATE_NONE || m_nWSState==WS_STATE_RELEASING ) {
			// We're not switching or reloading the weapon...

			if( m_apWeapon[0]->m_pInfo->nGrip == CWeapon::GRIP_RIGHT_ARM ) {
				// Primary weapon is not two-handed...

				if( m_nJumpState != BOTJUMPSTATE_CABLE ) {
					// We're not on a cable...

					if( !(m_apWeapon[0]->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_LEFT_HAND_RELOADS) || (m_nWRState == WR_STATE_NONE) ) {
						// Primary weapon is either auto-reloading, or is not currently reloading...
						bFingerIsOnTrigger = TRUE;
					}
				}
			}
		}
	}
	return bFingerIsOnTrigger;
}

void CBotKrunk::_HandleWeaponFiring( void ) {
	f32 fTrig1, fTrig2;
	u32 nFireBitsPrimary;

	fTrig1 = m_fControls_Fire1;
	fTrig2 = m_fControls_Fire2;

	if( !_IsFingerOnTrigger1() ) {
		fTrig1 = 0.0f;
	}

	if( !_IsFingerOnTrigger2() ) {
		fTrig2 = 0.0f;
	}

	nFireBitsPrimary = m_apWeapon[0]->TriggerWork( fTrig1, fTrig2, &m_TargetedPoint_WS, NULL );

	if( nFireBitsPrimary ) {
		// We fired the primary weapon...
		JustFired();   

		FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM );
		FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_PLAY_TETHER_VIBRATE );
		ZeroTime( ANIMTAP_FIRE_1 );

	} else {
		// Primary weapon not fired...

		// Do special tether zapping...
		FASSERT( m_apWeapon[0]->Type() == CWeapon::WEAPON_TYPE_TETHER );

		CWeaponTether *pTether = (CWeaponTether *)m_apWeapon[0];

		if( !(m_nBotFlags & BOTFLAG_PLAY_FIRE1_ANIM) ) {
			if( pTether->IsTetherFiringZapPulse() ) {
				FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM | BOTFLAG_PLAY_TETHER_VIBRATE );
				ZeroTime( ANIMTAP_FIRE_1 );
				m_fUnitTetherShockVibration = 0.0f;
			}
		} else {
			if( (m_nBotFlags & BOTFLAG_PLAY_TETHER_VIBRATE) && !pTether->IsTetherFiringZapPulse() ) {
				FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM | BOTFLAG_PLAY_TETHER_VIBRATE );
			}
		}
	}

	if( m_apWeapon[1] ) {
		if( m_apWeapon[1]->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_THROWABLE ) {
			// Secondary is throwable...
			if( m_apWeapon[1]->Throwable_TriggerWork( fTrig2 ) ) {
				// We can throw the grenade...

				m_nThrowState = THROWSTATE_REACHING_FOR_AMMO;
				ZeroTime( ANIMTAP_FIRE_2 );

				JustFired();  //disguise, if there was any is gone!
			}
		}
	}
}

void CBotKrunk::_AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	FASSERT( m_pCollBot && (m_pCollBot->TypeBits() & ENTITY_BIT_KRUNK) );

	if( ((CBotKrunk*)m_pCollBot)->m_pPartMgr->AnimBoneCallbackFunctionHandler( nBoneIndex, rNewMtx, rParentMtx, rBoneMtx ) ) {
		return;
	}

	if( ((CBotKrunk *)m_pCollBot)->BotInPieces_InPiecesAnimBoneCB( rNewMtx, rParentMtx, rBoneMtx ) ) {
		return;
	}

	if( nBoneIndex == (u32)((CBotKrunk *)m_pCollBot)->m_nBoneIndexGroin ) {
		// Groin
		rNewMtx.Mul( rParentMtx, rBoneMtx );
		m_GroinVecY_WS = rNewMtx.m_vUp;
	} else if( nBoneIndex == (u32)((CBotKrunk *)m_pCollBot)->m_nBoneIndexTorso ) {
		// Torso
		CFMtx43A WorldMtx;
		CFQuatA Quat;
		CBotKrunk *pBotKrunk = (CBotKrunk *)m_pCollBot;

		Quat.BuildQuat( m_GroinVecY_WS, pBotKrunk->m_fAimDeltaYaw_MS - pBotKrunk->m_fLegsYaw_MS );
		WorldMtx.Mul( rParentMtx, rBoneMtx );

		rNewMtx.m_vPos = WorldMtx.m_vPos;
		Quat.MulPoint( rNewMtx.m_vRight, WorldMtx.m_vRight );
		Quat.MulPoint( rNewMtx.m_vUp, WorldMtx.m_vUp );
		Quat.MulPoint( rNewMtx.m_vFront, WorldMtx.m_vFront );
	} else {
		// Head
		m_pCollBot->HeadLookUpdate( rNewMtx, rParentMtx, rBoneMtx, FALSE );
	}

	// This must be last
	((CBotKrunk *)m_pCollBot)->BotInPieces_RecoverAnimBoneCB( rNewMtx, rParentMtx, rBoneMtx );
}

BOOL CBotKrunk::_InventoryCallback( CInventoryCallbackReason_e eReason, 
								   CInventory *pInventory,
								   u32 nHandIndex,
								   u32 nWeaponIndex ) {
	CBotKrunk *pBotKrunk = (CBotKrunk *)pInventory->m_pUser;

	switch( eReason ) {

	case IREASON_WEAPONCHANGE:
		FASSERT_NOW;// ???????
		break;

	case IREASON_RELOAD:
		if( nHandIndex == 0 ) {
			pBotKrunk->m_abReloadRequest[0] = TRUE;
		}
		break;

	case IREASON_WEAPONUPGRADED:
		FASSERT_NOW;// ???????
		break;
	}

	return TRUE;
}

void CBotKrunk::SpawnDeathEffects( void ) {
	//note! patm put this code here just so that we know when a krunk is killed.
	//probably, someday there will be a death anim, or ragdoll, or propper spewage of krunk chunks or something.


	if( m_uBotDeathFlags & BOTDEATHFLAG_PLAYDEATHANIM ) {
		//if this guy is going to do a death anim, no chunks
		return;
	} else if (m_uBotDeathFlags & BOTDEATHFLAG_COMEAPART ) {

	}
}

void CBotKrunk::CheckpointSaveSelect( s32 nCheckpoint ) {

	// save bot's weapons first
	if( m_apWeapon[0] ) {
		m_apWeapon[0]->CheckpointSaveSelect( nCheckpoint );
	}

	if( m_apWeapon[1] ) {
		m_apWeapon[1]->CheckpointSaveSelect( nCheckpoint );
	}

	// then save self
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}

void CBotKrunk::InflictDamage( CDamageData *pDamageData ) {
	CBot* pWhoHitMe = pDamageData->m_Damager.pBot; // when hit by a possessed bot, 

	if( pWhoHitMe==this ) { // don't take damage from self
		return;
	}

	CBot::InflictDamage( pDamageData );
}

// Krunks Left leg will use the regular bot footstep, but
// the Right peg leg will use a custom footstep bank
void CBotKrunk::PlayFootSound( BOOL bLeft, BOOL bRight, SurfaceType_e nSurfaceType ) {
	u32 uSndIndex;

	FASSERT( nSurfaceType >= SURFACE_TYPE_NONE && nSurfaceType < SURFACE_TYPE_COUNT );

	if( !bLeft && !bRight ) {
		m_fNextScuffSound -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fNextScuffSound, 0.0f );
		return;
	}

	if( bLeft ) {
		// the left foot is a regular foot, just call the base class
		CBot::PlayFootSound( bLeft, bRight, nSurfaceType );
	} else {
		m_fNextScuffSound -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fNextScuffSound, 0.0f );

		// the right foot is a peg leg
		if( m_nPossessionPlayerIndex >= 0 ) {
			if( m_pBotInfo_Sound && (nSurfaceType != SURFACE_TYPE_NONE) ) {
				if( m_fNextScuffSound == 0.0f ) {
					m_fNextScuffSound = fmath_RandomFloatRange( BOT_SOUND_SCUFF_MIN_TIME, BOT_SOUND_SCUFF_MAX_TIME );
				}
				uSndIndex = fmath_RandomChoice( ANIM_SOUNDS_PER_SURFACE );

				fsndfx_Play2D(
					m_KrunkFootStepSounds.ahStepSound[nSurfaceType][ uSndIndex ],
					m_fClampedNormSpeedAlongSurface_WS * m_fStepVolumeScale * m_pBotInfo_Walk->fFootstepSoundUnitVolume_2D );			
				
				AIEnviro_BoostPlayerSoundTo(m_nPossessionPlayerIndex, 19.0f*m_fClampedNormSpeedAlongSurface_WS*m_fClampedNormSpeedAlongSurface_WS);
			}
		} else {
			if( m_pBotInfo_Sound && (nSurfaceType != SURFACE_TYPE_NONE) ) {
				
				if( m_fNextScuffSound == 0.0f ) {
					m_fNextScuffSound = fmath_RandomFloatRange( BOT_SOUND_SCUFF_MIN_TIME, BOT_SOUND_SCUFF_MAX_TIME );
				}
				uSndIndex = fmath_RandomChoice( ANIM_SOUNDS_PER_SURFACE );

				fsndfx_Play3D(
					m_KrunkFootStepSounds.ahStepSound[nSurfaceType][ uSndIndex ],
					&MtxToWorld()->m_vPos,
					m_pBotInfo_Walk->fFootstepSoundRadius_3D,
					1.0f,
					m_fClampedNormSpeedAlongSurface_WS * m_pBotInfo_Walk->fFootstepSoundUnitVolume_3D );
			}
		}
	}
}
void CBotKrunk::AboutToPossessBot( CBot *pBotToBePossessed ) {
	FASSERT( m_pInventory );	

	// copy our current inventory before calling the base class
	fang_MemCopy( &m_InventoryCache, m_pInventory, sizeof( CInventory ) );

	// call the base class
	CBot::AboutToPossessBot( pBotToBePossessed );
}	

void CBotKrunk::HaveUnPossessedBot( CBot *pBotBeingLost ) {
	FASSERT( m_pInventory );

	// restore the cached inventory
	fang_MemCopy( m_pInventory, &m_InventoryCache, sizeof( CInventory ) );

	// call the base class 
	CBot::HaveUnPossessedBot( pBotBeingLost );	
}

void CBotKrunk::_SpawnBurners( void ) {

	if( m_hFlameParticleDef == FPARTICLE_INVALID_HANDLE ||
		!m_pTorchMtx ||
		m_fSpeed_WS >= 3.0f ) {
		return;
	}

	m_fSecsUntilNextBurner -= FLoop_fPreviousLoopSecs;
	if( m_fSecsUntilNextBurner > 0.0f ) {
		return;
	}

	m_fSecsUntilNextBurner = fmath_RandomFloatRange( 0.35f, 1.85f );

	FCollImpact_t CollImpact;
	CFVec3A RayStart, RayEnd;
	f32 fMag2;
	
	RayStart.Mul( m_pTorchMtx->m_vRight, 0.25f );// move the start point into the torch
	RayStart.Add( m_pTorchMtx->m_vPos );
	RayEnd.Mul( m_pTorchMtx->m_vRight, -1.25f );
	RayEnd.Add( RayStart );

	FWorld_nTrackerSkipListCount = 0;
	AppendTrackerSkipList();

	if( !fworld_FindClosestImpactPointToRayStart( &CollImpact, &RayStart, &RayEnd,
		FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList,
		TRUE, m_pWorldMesh, -1, FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES ) ) {
		// No collision...
		return;
	}

	// We hit something...
	const CGCollMaterial *pCollMaterial = CGColl::GetMaterial( &CollImpact );

	if( !pCollMaterial->CanCatchFire() ) {
		// Cannot catch fire...
		return;
	}

	CFMtx43A Mtx;
	f32 fRandSecs;
		
	Mtx.m_vPos = CollImpact.ImpactPoint;
	Mtx.m_vFront = CollImpact.UnitFaceNormal;
	Mtx.m_vRight.CrossYWithVec( Mtx.m_vFront );
	fMag2 = Mtx.m_vRight.MagSq();
	if( fMag2 > 0.0001f ) {
		Mtx.m_vRight.Mul( fmath_InvSqrt(fMag2) );
	} else {
		Mtx.m_vRight.UnitCross( CFVec3A::m_UnitAxisX, Mtx.m_vFront );
	}
	Mtx.m_vUp.UnitCross( Mtx.m_vFront, Mtx.m_vRight );

	// grab a particle emitter
	CEParticle *pEPart = eparticlepool_GetEmitter();
	if( !pEPart ) {
		return;
	}

	fRandSecs = fmath_RandomFloatRange( 0.75f, 2.0f );

	pEPart->AddToWorld();
	pEPart->Relocate_RotXlatFromUnitMtx_WS( &Mtx );
	pEPart->StartEmission( m_hFlameParticleDef, fmath_RandomFloatRange( 0.10f, 0.5f ), fRandSecs, TRUE );

	if( CollImpact.pTag ) {
		// Hit an object...

		CFWorldMesh *pWorldMesh = (CFWorldMesh *)CollImpact.pTag;

		// See if the world mesh that we found is an entity...
		if( pWorldMesh->m_nUser == MESHTYPES_ENTITY ) {
			// It's an Entity!

			CEntity *pHitEntity = (CEntity *)pWorldMesh->m_pUser;

			pEPart->Attach_ToParent_WS( pHitEntity );
		}
	}
}
