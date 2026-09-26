//////////////////////////////////////////////////////////////////////////////////////
// botslosh.cpp -
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
// 12/18/02 Scholz       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "botslosh.h"
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
#include "fsound.h"
#include "meshentity.h"
#include "Ai\AIEnviro.h"
#include "Hud2.h"
#include "botpart.h"

#define _BOTINFO_FILENAME		"b_slosh"
#define _BOTPART_FILENAME		"bp_slosh"
#define _BOOST_PARTICLE_NAME	"PRDSjetfire"

static cchar* _apszSloshMeshFilenames[] = {"GRDSSlosh00"};

CFSoundGroup* CBotSlosh::m_pSounds[SLOSHSND_COUNT];

cchar* BotSlosh_pszSoundEffectBank = "Slosh";

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotSloshBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CBotSloshBuilder _BotSloshBuilder;


void CBotSloshBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType )
{
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_BOTCHEMBOT, pszEntityType );

	// Override defaults set by our parent classes...
	m_nEC_HealthContainerCount = CBotSlosh::m_BotInfo_Gen.nBatteryCount;
	m_pszEC_ArmorProfile = CBotSlosh::m_BotInfo_Gen.pszArmorProfile;

	m_anNPCWeaponType[0] = _NPC_WEAPON_FLAMER;		
	m_anNPCWeaponType[1] = _NPC_WEAPON_MAGMABOMB;	

	// This class of bots shows up on Glitch's radar...
	FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_SHOWS_UP_ON_RADAR );
}


BOOL CBotSloshBuilder::PostInterpretFixup( void )
{
	if( !CBotBuilder::PostInterpretFixup() )
	{
		goto _ExitWithError;
	}

	return TRUE;

	// Error:
_ExitWithError:
	return FALSE;
}


BOOL CBotSloshBuilder::InterpretTable( void )
{
	return CBotBuilder::InterpretTable();
}





//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotSlosh
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL							CBotSlosh::m_bSystemInitialized		= FALSE;
u32								CBotSlosh::m_nBotClassClientCount	= 0;

CBotSlosh::BotInfo_Gen_t 		CBotSlosh::m_BotInfo_Gen;
CBotSlosh::BotInfo_MountAim_t 	CBotSlosh::m_BotInfo_MountAim;
CBotSlosh::BotInfo_Walk_t 		CBotSlosh::m_BotInfo_Walk;
CBotSlosh::BotInfo_Jump_t 		CBotSlosh::m_BotInfo_Jump;
CBotSlosh::BotInfo_Weapon_t 	CBotSlosh::m_BotInfo_Weapon;
CBotSlosh::BotInfo_Idle_t*		CBotSlosh::m_BotInfo_Idle=NULL;
u32								CBotSlosh::m_nBotInfo_Idles=0;

CBotAnimStackDef 				CBotSlosh::m_AnimStackDef;
CFVec3A 						CBotSlosh::m_GroinVecY_WS;
CBotPartPool*					CBotSlosh::m_pPartPool				= NULL;

BOOL CBotSlosh::InitSystem( void )
{
	FASSERT( !m_bSystemInitialized );
	m_bSystemInitialized = TRUE;
	m_nBotClassClientCount = 0;
	return TRUE;
}
static cchar* _pszSoundGroupTable[CBotSlosh::SLOSHSND_COUNT] = 
{
	"SRDSfartbur",	//	SLOSHSND_BOOST
};

BOOL CBotSlosh::ClassHierarchyLoadSharedResources( void ) 
{
	FASSERT( m_bSystemInitialized );
	FASSERT( m_nBotClassClientCount != 0xffffffff );

	++m_nBotClassClientCount;

	if( !CBot::ClassHierarchyLoadSharedResources() ) 
	{
		// Bail now since parent class has already called
		// ClassHierarchyUnloadSharedResources() and decremented
		// our client counter...
		return FALSE;
	}

	if( m_nBotClassClientCount > 1 ) 
	{
		// Resources already loaded...
		return TRUE;
	}

	FResFrame_t ResFrame;
	ResFrame = fres_GetFrame();

	if( !ReadBotInfoFile( m_aGameDataMap, _BOTINFO_FILENAME ) )
	{
		goto _ExitInitSystemWithError;
	}

	if( !ReadBotIdleFile( &m_BotInfo_Idle,&m_nBotInfo_Idles, _BOTINFO_FILENAME ) ) {
		goto _ExitInitSystemWithError;
	}
	if (m_nBotInfo_Idles)
	{
		m_apszIdleAnimNameTable = (cchar**) fres_Alloc( sizeof(cchar*) * m_nBotInfo_Idles);
		if (!m_apszIdleAnimNameTable)
			goto _ExitInitSystemWithError;
		
		m_apnEnableBoneNameIndexTableForEachIdleTap = (const u8**) fres_Alloc( sizeof(u8*) * m_nBotInfo_Idles);
		if (!m_apnEnableBoneNameIndexTableForEachIdleTap)
			goto _ExitInitSystemWithError;

		for (u32 uIndex=0;uIndex<m_nBotInfo_Idles;uIndex++)
		{
			m_apszIdleAnimNameTable[uIndex] = m_BotInfo_Idle[uIndex].pszIdleName;
			m_apnEnableBoneNameIndexTableForEachIdleTap[uIndex]= m_aBoneEnableIndices_FullBody;
		}
	}

	CSloshTank::InitSystem();

	if( !_BuildAnimStackDef() )
	{
		goto _ExitInitSystemWithError;
	}
	
	// Particle effects
	m_hBoostParticleDef = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, _BOOST_PARTICLE_NAME );
		
	if( m_hBoostParticleDef == FPARTICLE_INVALID_HANDLE ) 
	{
		DEVPRINTF( "CBostSlosh::ClassHierarchyBuild(): Could not find particle definition '%s'.\n", _BOOST_PARTICLE_NAME );
	}

	// Load the sound effects bank for this bot...
	if( !fresload_Load( FSNDFX_RESTYPE, BotSlosh_pszSoundEffectBank) )
	{
		DEVPRINTF( "CBotSlosh::ClassHierarchyLoadSharedResources(): Could not load sound effect bank '%s'\n", BotSlosh_pszSoundEffectBank);
	}

	for (BotSloshSound_e iSndIdx = (BotSloshSound_e)0; iSndIdx < SLOSHSND_COUNT; iSndIdx = (BotSloshSound_e)(int(iSndIdx)+1))
	{
		m_pSounds[iSndIdx] = CFSoundGroup::RegisterGroup( _pszSoundGroupTable[iSndIdx] );
		if (!m_pSounds[iSndIdx])
			DEVPRINTF( "CBotSlosh::ClassHierarchyLoadSharedResources(): Could not load sound effect '%s'\n", _pszSoundGroupTable[iSndIdx]);
	}

	return TRUE;

	// Error:
_ExitInitSystemWithError:
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

void CBotSlosh::UninitSystem( void )
{
	m_bSystemInitialized = FALSE;
}

void CBotSlosh::ClassHierarchyUnloadSharedResources( void ) 
{
	FASSERT( m_nBotClassClientCount > 0 );

	--m_nBotClassClientCount;

	if( m_nBotClassClientCount > 0 ) 
	{
		return;
	}
	m_AnimStackDef.Destroy();

	CSloshTank::UninitSystem();

	CBot::ClassHierarchyUnloadSharedResources();
}

CBotSlosh::CBotSlosh() : CBot()
{
	m_pInventory = NULL;
	m_pWorldMesh = NULL;
}


CBotSlosh::~CBotSlosh()
{
	if( IsSystemInitialized() && IsCreated() )
	{
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CBotSlosh::Create( s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName )
{
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	if( !ClassHierarchyLoadSharedResources() ) 
	{
		// Failure! (resources have already been cleaned up, so we can bail now)
		return FALSE;
	}

	// Get pointer to the leaf class's builder object...
	CBotSloshBuilder *pBuilder = (CBotSloshBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_BotSloshBuilder )
	{
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOTCHEMBOT );
	}

	// Set our builder parameters...

	// Create an entity...
	return CBot::Create( &m_BotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}


void CBotSlosh::ClassHierarchyDestroy( void )
{
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


BOOL CBotSlosh::ClassHierarchyBuild( void )
{
	FMesh_t *pMesh;
	FMeshInit_t MeshInit;
	u32 i;
	s32 nBoneIndex;
	cchar * pszMeshFilename = NULL;
	u32 uAttachedLights;			//CPS 4.7.03
	
	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CBotSloshBuilder *pBuilder = (CBotSloshBuilder *)GetLeafClassBuilder();

	// Set input parameters for CBot creation...
	pBuilder->m_pBotDef = &m_BotDef;

	// Build parent class...
	if( !CBot::ClassHierarchyBuild() )
	{
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	// Initialize from builder object...

	// Load mesh resource...
	pszMeshFilename = _apszSloshMeshFilenames[0];
	if ( pBuilder->m_uMeshVersionOverride >0 && pBuilder->m_uMeshVersionOverride < sizeof(_apszSloshMeshFilenames)/sizeof(char*) )
	{
		pszMeshFilename = _apszSloshMeshFilenames[pBuilder->m_uMeshVersionOverride];
	}
	if( pBuilder->m_pszMeshReplacement ) {
	   pszMeshFilename = pBuilder->m_pszMeshReplacement;
	}
	pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pszMeshFilename);
	if( pMesh == NULL )
	{
		DEVPRINTF( "CBotSlosh::ClassHierarchyBuild(): Could not find mesh '%s'\n", pszMeshFilename );
		goto _ExitWithError;
	}

	// Create world mesh...
	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL )
	{
		DEVPRINTF( "CBotSlosh::ClassHierarchyBuild(): Not enough memory to create CFWorldMesh.\n" );
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
	if( !m_Anim.Create( &m_AnimStackDef, m_pWorldMesh ) )
	{
		DEVPRINTF( "CBotSlosh::ClassHierarchyBuild(): Trouble creating m_Anim.\n" );
		goto _ExitWithError;
	}
	for (u32 uIndex=0;uIndex<m_nBotInfo_Idles;uIndex++)	
	{
		m_Anim.IdleAnim_EnableControlSmoothing(uIndex);
	}


	SetControlValue( ANIMCONTROL_STAND, 1.0f );
	UpdateUnitTime( ANIMTAP_STAND, fmath_RandomFloat() );


	m_nBoneIndexGroin = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_GROIN] );
	m_nBoneIndexTorso = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_TORSO] );
	m_Anim.m_pAnimCombiner->SetBoneCallback( &_AnimBoneCallback );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_GROIN] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_TORSO] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_HEAD] );

	if( !m_AnimManFrameAim.Create( m_AnimStackDef.m_nBoneCount, m_AnimStackDef.m_apszBoneNameTable ) )
	{
		DEVPRINTF( "CBotSlosh::ClassHierarchyCreate(): Could not create aim animation summer.\n" );
		goto _ExitWithError;
	}
	AttachAnim( ANIMTAP_AIM_SUMMER, &m_AnimManFrameAim );
	m_AnimManFrameAim.Reset();
	UpdateBoneMask( ANIMTAP_AIM_SUMMER, m_aBoneEnableIndices_AimSummer, TRUE );

	if( m_nPossessionPlayerIndex >= 0 )
	{
		Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.SetNormOrigin( 0.18f, 0.23f );
		m_pInventory = CPlayer::GetInventory(m_nOwnerPlayerIndex,TRUE); // gets the possession inventory
		if( m_pInventory == NULL ) 
		{
			// What is going on here?!  This shouldn't happen!
			DEVPRINTF( "CBotBlink::_InitInventory() : Could not get inventory from CPlayer.\n" );
			return FALSE;
		}

		m_pInventory->SetToSlosh();
		m_pInventory->m_pUser = this;
		m_pInventory->m_pfcnCallback = _InventoryCallback;
		CHud2::GetHudForPlayer(m_nPossessionPlayerIndex)->SetHudMode(HUDMODE_SLOSH);
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

				if (3<pWeap->GetUpgradeLevelCount())
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
		m_pInventory->m_aoWeapons[0][0].SetAmmoDisplayType(CItemInst::AMMODISPLAY_METER,CItemInst::AMMODISPLAY_NONE);
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

			if (3<m_apWeapon[i]->GetUpgradeLevelCount())
			{
				m_apWeapon[i]->SetUpgradeLevel(3);	  //3 is special level for npc
			}
			else
			{
				m_apWeapon[i]->SetUpgradeLevel(0);	  //3 is special level for npc
			}
		}
	}

	m_apWeapon[0]->SetClipAmmo( 100 );
	m_apWeapon[0]->SetReserveAmmo( (u16)CWeapon::INFINITE_AMMO );

	// Make primary weapon pass along damage to upper-right arm...
	m_apWeapon[0]->SetBotDamageBoneIndex( m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_R_ARM_LOWER] ) );
	SetMaxHealth();

	// Find approx eye point...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
	if( nBoneIndex < 0 )
	{
		DEVPRINTF( "CBotSlosh::ClassHierarchyBuild(): Could not locate approx eye point bone '%s'.\n", m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
		m_pApproxEyePoint_WS = &m_MtxToWorld.m_vPos;
	}
	else
	{
		m_pApproxEyePoint_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
	}

	// Initialize matrix palette (this is important for attached entities)...
	AtRestMatrixPalette();

	// set up part mgr...
	if( !m_pPartMgr->Create( this, &m_pPartPool, _BOTPART_FILENAME, PART_INSTANCE_COUNT_PER_TYPE, LIMB_TYPE_COUNT ) ) {
		goto _ExitWithError;
	}


	// Create tag points...
	if( !TagPoint_CreateFromBoneArray( m_anTagPointBoneNameIndexArray, m_apszBoneNameTable ) )
	{
		goto _ExitWithError;
	}

	// Find gaze direction...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_HEAD ] );
	if( nBoneIndex < 0 )
	{
		DEVPRINTF( "CBotSlosh::ClassHierarchyBuild(): Could not locate gaze bone '%s'.\n", m_apszBoneNameTable[ BONE_HEAD ] );
		m_pGazeDir_WS = &m_MtxToWorld.m_vFront;
	}
	else
	{
		m_pGazeDir_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vFront;
	}

	m_pAISteerMtx = &m_MtxToWorld;

	// Find left booster pos
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_ATTACHPOINT_JET_L ] );
	if( nBoneIndex < 0 )
	{
		DEVPRINTF( "CBotSlosh::ClassHierarchyBuild(): Could not locate bone '%s'.\n", m_apszBoneNameTable[ BONE_ATTACHPOINT_JET_L ] );
		m_pmtxLeftBooster_WS = NULL;
	}
	else
	{
		m_pmtxLeftBooster_WS = m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex];
	}
	
	// Find right booster pos
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_ATTACHPOINT_JET_R ] );
	if( nBoneIndex < 0 )
	{
		DEVPRINTF( "CBotSlosh::ClassHierarchyBuild(): Could not locate bone '%s'.\n", m_apszBoneNameTable[ BONE_ATTACHPOINT_JET_R ] );
		m_pmtxRightBooster_WS = NULL;
	}
	else
	{
		m_pmtxRightBooster_WS = m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex];
	}

//CPS 4.7.03	u32 uAttachedLights = m_pWorldMesh->GetAttachedLightCount();
	uAttachedLights = m_pWorldMesh->GetAttachedLightCount();
	FASSERT(uAttachedLights==1);
	m_pTankLight = m_pWorldMesh->GetFirstAttachedLight();
	if (!m_pTankLight)
		goto _ExitWithError;

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


BOOL CBotSlosh::ClassHierarchyBuilt( void )
{
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CBot::ClassHierarchyBuilt() )
	{
		goto _ExitWithError;
	}

	EnableOurWorkBit();

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


CEntityBuilder *CBotSlosh::GetLeafClassBuilder( void )
{
	return &_BotSloshBuilder;
}


void CBotSlosh::_ClearDataMembers( void )
{
	// Init data members...

	m_pMoveIdentifier = &m_nBoneIndexGroin;

	m_fCollCylinderRadius_WS = 3.0f;
	m_fCollCylinderHeight_WS = 7.0f;

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

	m_anAnimStackIndex[ASI_STAND] = ANIMTAP_STAND;
	m_anAnimStackIndex[ASI_STAND_ALERT] = -1;// ANIMTAP_STAND_ALERT;

	m_anAnimStackIndex[ASI_WALK] = ANIMTAP_WALK;
	m_anAnimStackIndex[ASI_WALK_ALERT] = -1;// ANIMTAP_WALK_ALERT;
	m_anAnimStackIndex[ASI_RUN] = ANIMTAP_RUN;
	m_anAnimStackIndex[ASI_RUN_PANIC] = -1;// ANIMTAP_RUN_PANIC;

	m_anAnimStackIndex[ASI_FALL] = ANIMCONTROL_JUMP_FLY;

	m_anAnimStackIndex[ASI_HOP_LEFT]				= -1;// ANIMTAP_HOP_LEFT;
	m_anAnimStackIndex[ASI_HOP_RIGHT]				= -1;// ANIMTAP_HOP_RIGHT;
	m_anAnimStackIndex[ASI_STARTLE]					= -1;// ANIMTAP_STARTLE;
	m_anAnimStackIndex[ASI_ROLL_LEFT]				= -1;// ANIMTAP_ROLL_LEFT;
	m_anAnimStackIndex[ASI_ROLL_RIGHT]				= -1;// ANIMTAP_ROLL_RIGHT;

	m_anAnimStackIndex[ASI_DOZE_LOOP]				= -1;// ANIMTAP_DOZE_LOOP;
	m_anAnimStackIndex[ASI_NAPJERK]					= -1;// ANIMTAP_NAPJERK;
	m_anAnimStackIndex[ASI_WAKE]					= -1;// ANIMTAP_WAKE;

	m_anAnimStackIndex[ASI_RELOAD_CLIP_EJECT_OLD]	= ANIMCONTROL_RELOAD_CLIP_EJECT_OLD;
	m_anAnimStackIndex[ASI_RELOAD_CLIP_GRAB_NEW]	= ANIMCONTROL_RELOAD_CLIP_GRAB_NEW;
	m_anAnimStackIndex[ASI_RELOAD_CLIP_INSERT_NEW]	= ANIMCONTROL_RELOAD_CLIP_INSERT_NEW;
	m_anAnimStackIndex[ASI_RELOAD_CLIP_SLAPIN_NEW]	= ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW;

	m_anAnimStackIndex[ASI_SNEAK]					= ANIMTAP_SNEAK;	//ANIMTAP_TURN_IN_PLACE;

	m_anAnimStackIndex[ASI_RC_TETHERED]				= -1;// ANIMTAP_RC_TETHERED;
	m_anAnimStackIndex[ASI_RC_POWER_DOWN]			= -1;// ANIMTAP_RC_POWER_DOWN;
	m_anAnimStackIndex[ASI_RC_POWER_UP]				= -1;// ANIMTAP_RC_POWER_UP;
	m_anAnimStackIndex[ASI_STOOP]					= -1;

	m_anAnimStackIndex[ASI_AIM_PILLBOX]				= -1;//ANIMTAP_AIM_PILLBOX;

	m_pnEnableBoneNameIndexTableForSummer_Normal = m_anEnableBoneNameIndexTableForSummer_Normal;
	m_pnEnableBoneNameIndexTableForSummer_TetherShock = NULL;//m_anEnableBoneNameIndexTableForSummer_TetherShock;

	m_pfTankFillPercent = NULL;
	m_pPlayerDeathData = &m_PlayerDamageData;
}


void CBotSlosh::ClassHierarchyAddToWorld( void )
{
	u32 i;

	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CBot::ClassHierarchyAddToWorld();
	m_pWorldMesh->UpdateTracker();

	m_uBotDeathFlags |= BOTDEATHFLAG_PLAYDEATHANIM | BOTDEATHFLAG_COMEAPART | BOTDEATHFLAG_AUTOPERSISTAFTERDEATH;
	for( i=0; i<2; i++ )
	{
		if( m_apWeapon[i] )
		{
			m_apWeapon[i]->Attach_UnitMtxToParent_PS( this, m_apszBoneNameTable[BONE_R_ARM_LOWER] );
			m_apWeapon[i]->AddToWorld();
			m_apWeapon[i]->ResetToState( CWeapon::STATE_DEPLOYED );
		}
	}
	this->m_pTank = (CSloshTank::GetTank());
	m_pfTankFillPercent = this->m_pTank->GetTankPercentPtr();
	// Find tank pos
	s32 nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_ATTACHPOINT_FLUID ] );
	if( nBoneIndex < 0 )
	{
		DEVPRINTF( "CBotSlosh::ClassHierarchyBuild(): Could not locate bone '%s'.\n", m_apszBoneNameTable[ BONE_ATTACHPOINT_FLUID ] );
		m_pTank->m_pmtxFluid_WS = &CFMtx43A::m_IdentityMtx;
	}
	else
	{
		m_pTank->m_pmtxFluid_WS = m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex];
	}

	m_pTank->m_vPendentVerletCurr = m_pTank->m_pmtxFluid_WS->m_vPos;
	m_pTank->m_vPendentVerletPrev = m_pTank->m_pmtxFluid_WS->m_vPos;
}


void CBotSlosh::ClassHierarchyRemoveFromWorld( void )
{
	u32 i;

	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );
	
	if( m_pTank ) {
		CSloshTank::PutTankBack(m_pTank);
		m_pTank = NULL;
	}
	

	for( i=0; i<2; i++ )
	{
		if( m_apWeapon[i] )
		{
			m_apWeapon[i]->RemoveFromWorld();
		}
	}

	m_pWorldMesh->RemoveFromWorld();
	CBot::ClassHierarchyRemoveFromWorld();
}


void CBotSlosh::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList)
{
	FASSERT( IsCreated() );
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = m_pWorldMesh;

	if( m_apWeapon[0] )
	{
		m_apWeapon[0]->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}

	if( m_pDataPortMeshEntity )
	{
		m_pDataPortMeshEntity->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}
}


const CFVec3A *CBotSlosh::GetApproxEyePoint( void ) const
{
	FASSERT( IsCreated() );

	return m_pApproxEyePoint_WS;
}


void CBotSlosh::ClassHierarchyWork()
{
	CFVec3A TempVec3A;

	FASSERT( m_bSystemInitialized );

	CBot::ClassHierarchyWork();

	if( !IsOurWorkBitSet() )
	{
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
	if( bImpulseApplied )
	{
		VelocityHasChanged();
	}

	// Update translation analog stick vectors from raw controller data...
	ComputeXlatStickInfo();

	// Collide with the world below our feet...
	PROTRACK_BEGINBLOCK("Coll");
		HandleCollision();
	PROTRACK_ENDBLOCK();//"Coll"

	// Move and animate our bot...
	switch( m_nState ) 
	{
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
			if( HandleAirAnimations() ) 
			{
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
	
	if (IsDeadOrDying()) 
	{
		DeathWork();
	}
}

void CBotSlosh::_HandleDoubleJump( void ) 
{
	if( m_bControls_Jump ) 
	{
		if( m_nJumpState==BOTJUMPSTATE_LAUNCH || m_nJumpState==BOTJUMPSTATE_AIR ) 
		{
			_StartDoubleJump();
		}
	}
}

void CBotSlosh::_HandleJumping( void )
{
	if( !m_bControlsBot_JumpVec )
	{
		if( m_bControlsBot_Jump2 )
		{
			_StartSingleJump();
			_StartDoubleJump();
		}
		else if( m_bControls_Jump )
		{
			_StartSingleJump();
		}
	}
	else
	{
		// Velocity jump specified...
		_StartVelocityJump( &m_ControlsBot_JumpVelocity_WS );
	}

	if( m_nPrevState == STATE_AIR )
	{
		if( m_nJumpState != BOTJUMPSTATE_NONE )
		{
			_JumpLanded();
		}
	}
}

void CBotSlosh::_EnterFlyMode( void ) 
{
	m_bPlayLandAnim = FALSE;
	m_bPlayLandAnimLower = FALSE;

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );
	m_nJumpState = BOTJUMPSTATE_AIR;
	SetJumping();
}


void CBotSlosh::_StartVelocityJump( const CFVec3A *pJumpVelocity_WS )
{
	CFVec3A JumpVelocity_WS = *pJumpVelocity_WS;
	FMATH_CLAMP( JumpVelocity_WS.y, 0.0f, m_pBotInfo_Jump->fVerticalVelocityJump1 + m_pBotInfo_Jump->fVerticalVelocityJump2 );

	_StartSingleJump( &JumpVelocity_WS );

	if( pJumpVelocity_WS->y >= m_pBotInfo_Jump->fVerticalVelocityJump1 )
	{
		_StartDoubleJump( FALSE );
	}
}


void CBotSlosh::_StartSingleJump( const CFVec3A *pJumpVelocity_WS )
{
	if( pJumpVelocity_WS == NULL ) {
		m_Velocity_WS.y += m_pBotInfo_Jump->fVerticalVelocityJump1 * m_fJumpMultiplier;
		m_Velocity_MS.y += m_pBotInfo_Jump->fVerticalVelocityJump1 * m_fJumpMultiplier;
	}
	else
	{
		m_Velocity_WS.x = pJumpVelocity_WS->x;
		m_Velocity_WS.y += pJumpVelocity_WS->y;
		m_Velocity_WS.z = pJumpVelocity_WS->z;
		WS2MS( m_Velocity_MS, m_Velocity_WS );
		VelocityHasChanged();
	}

	m_bPlayLandAnim = FALSE;
	m_bPlayLandAnimLower = FALSE;

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );

	ZeroTime( ANIMTAP_JUMP_LAUNCH );

	m_fUnitJump1Jump2Blend = 0.0f;
	m_nJumpState = BOTJUMPSTATE_LAUNCH;
	SetJumping();
	SetDoubleJumpTimer( m_pBotInfo_Gen->fDoubleJumpTime );
}

void CBotSlosh::_StartDoubleJump( BOOL bAddToVelocityY )
{
	if( !HaveTimeToDoubleJump() ) {
		return;
	}

	FParticle_EmitterHandle_t hFartLeft = fparticle_SpawnEmitter( m_hBoostParticleDef, &m_pmtxLeftBooster_WS->m_vPos.v3,  &m_pmtxLeftBooster_WS->m_vUp.v3); 
	FParticle_EmitterHandle_t hFartRight = fparticle_SpawnEmitter( m_hBoostParticleDef, &m_pmtxRightBooster_WS->m_vPos.v3, &m_pmtxRightBooster_WS->m_vUp.v3); 
	fparticle_SetDirection(hFartLeft, &m_pmtxLeftBooster_WS->m_vUp.v3);
	fparticle_SetDirection(hFartRight, &m_pmtxRightBooster_WS->m_vUp.v3);

//	f32 fOOTuckUntuckAnimSpeedMult;
//	f32 fJumpSecs, fAnimSecs, fVelY;

	if( bAddToVelocityY )
	{
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
	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_BOOST, 1.0f );
	ZeroTime( ANIMTAP_JUMP_BOOST );

	m_nJumpState = BOTJUMPSTATE_AIR2;
	SetJumping();

	// NKM - Remove all attached Swarmers
	DetachAllSwarmers();

	PlaySound(m_pSounds[SLOSHSND_BOOST]);
}


void CBotSlosh::_JumpLanded( void )
{
	m_nJumpState = BOTJUMPSTATE_NONE;
	ClearJumping();
	m_fFlipPitch = 0.0f;
	m_bPlayLandAnim = TRUE;

	ZeroTime( ANIMTAP_JUMP_LAND_UPPER );

	m_fMaxLandUnitBlend = GetControlValue( ANIMCONTROL_JUMP_FLY );
	if( m_fMaxLandUnitBlend == 0.0f )
	{
		m_fMaxLandUnitBlend = 1.0f;
	}

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, m_fMaxLandUnitBlend );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_BOOST, 0.0f );

	if ( m_nPossessionPlayerIndex >= 0 )
	{
		// Don't play scuff sounds when landing from a jump
		if( m_pBotInfo_Sound && m_nSurfaceTypeOn != SURFACE_TYPE_NONE)
		{
			fsndfx_Play2D(
				m_pBotInfo_Sound->ahStepSound[m_nSurfaceTypeOn][ fmath_RandomRange( 0, ( BOTSOUND_SCUFF_SOUND_START - 1 ) ) ],
				m_pBotInfo_Walk->fJumpLandedSoundUnitVolume_2D * m_fStepVolumeScale
			);
		}
		//tell the AI that this sound happened
		AIEnviro_BoostPlayerSoundTo(m_nPossessionPlayerIndex, 35.0f);
	}
	else
	{
		if( m_pBotInfo_Sound && m_nSurfaceTypeOn != SURFACE_TYPE_NONE)
		{
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

	if( m_fSpeedXZ_WS == 0.0f ) 
	{
		SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, m_fMaxLandUnitBlend );
		m_bPlayLandAnimLower = TRUE;
	} 
	else 
	{
		SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
		m_bPlayLandAnimLower = FALSE;
	}
}


void CBotSlosh::_HandleJumpAnimations( void )
{
	f32 fTemp, fUnitTime;

	if( m_nState == STATE_GROUND )
	{
		// We're on the ground...
		if( m_fUnitFlyBlend > 0.0f )
		{
			m_fUnitFlyBlend = 0.0f;
			_JumpLanded();
		}

		// Handle landing animation...
		if( m_bPlayLandAnim )
		{
			if( m_fSpeedXZ_WS > 0.0f )
			{
				fTemp = m_pBotInfo_Jump->fMovingLandAnimSpeedMult;
			}
			else
			{
				fTemp = 1.0f;
			}

			if( !DeltaTime( ANIMTAP_JUMP_LAND_UPPER, FLoop_fPreviousLoopSecs * fTemp ) )
			{
				// Still playing landing animation...

				f32 fUnitTime = 1.01f - GetUnitTime(ANIMTAP_JUMP_LAND_UPPER);
				fUnitTime = m_fMaxLandUnitBlend * fmath_UnitLinearToSCurve( fUnitTime );

				SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, fUnitTime );
				if( m_bPlayLandAnimLower ) 
				{
					SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, fUnitTime );
				}
			}
			else
			{
				// Done playing landing animation...

				m_bPlayLandAnim = FALSE;
				m_bPlayLandAnimLower = FALSE;

				SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );
			}
		}
		else
		{
			if( m_nJumpState == BOTJUMPSTATE_NONE )
			{
				SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
			}
		}
	}
	else
	{
		// We're in the air...

		if( m_nJumpState != BOTJUMPSTATE_NONE )
		{
			switch( m_nJumpState )
			{
			case BOTJUMPSTATE_LAUNCH:
				if( !DeltaTime( ANIMTAP_JUMP_LAUNCH ) )
				{
					fUnitTime = 6.0f * fmath_UnitLinearToSCurve( GetUnitTime( ANIMTAP_JUMP_LAUNCH ) );
					FMATH_CLAMPMAX( fUnitTime, 1.0f );
					SetControlValue( ANIMCONTROL_JUMP_LAUNCH, fUnitTime );
				}
				else
				{
					SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );
					SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
					ZeroTime( ANIMTAP_JUMP_FLY );
					m_nJumpState = BOTJUMPSTATE_AIR;
					SetJumping();
				}

				break;

			case BOTJUMPSTATE_AIR:
				DeltaTime( ANIMTAP_JUMP_FLY );
				break;

			case BOTJUMPSTATE_AIR2:
				if( DeltaTime( ANIMTAP_JUMP_BOOST ) )
				{
					SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );
					SetControlValue( ANIMCONTROL_JUMP_BOOST, 0.0f );
					// ZeroTime( ANIMTAP_JUMP_FLY );
					m_nJumpState = BOTJUMPSTATE_AIR;
				}
				break;
///////////////////////////////////////
///////////////////////////////////////
				default:
				FASSERT_NOW;
			}
		}
	}
}


void CBotSlosh::_HandleWeaponWork( void )
{
	if( m_apWeapon[0] )
	{
		m_apWeapon[0]->Work();
	}
	if( m_apWeapon[1] )
	{
		m_apWeapon[1]->Work();
	}
}


void CBotSlosh::_HandleFiringAnimations( void )
{
	f32 fUnitFireControlValue;

	/* no specific primary firing animation 

	// Primary fire...
	u32 uFireAnimControlId = ANIMCONTROL_FIRE_1;
	u32 uFireAnimTapId = ANIMTAP_FIRE_1;
	if (IsAlertOn())
	{
		uFireAnimControlId = ANIMCONTROL_FIRE_1_ALERT;
		uFireAnimTapId = ANIMTAP_FIRE_1_ALERT;
		SetControlValue( ANIMCONTROL_FIRE_1, 0.0f);
	}
	else
	{
		SetControlValue( ANIMCONTROL_FIRE_1_ALERT, 0.0f);
	}

	if( m_nBotFlags & BOTFLAG_PLAY_FIRE1_ANIM )
	{
		fUnitFireControlValue = GetControlValue( uFireAnimControlId );

		if( fUnitFireControlValue < 1.0f )
		{
			fUnitFireControlValue += m_pBotInfo_Weapon->fFire1BlendInSpeed * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMAX( fUnitFireControlValue, 1.0f );
			SetControlValue( uFireAnimControlId, fUnitFireControlValue );
		}

		if( DeltaTime( uFireAnimTapId ) )
		{
			FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM );
		}
	}
	else
	{
		fUnitFireControlValue = GetControlValue( uFireAnimControlId );

		if( fUnitFireControlValue > 0.0f )
		{
			fUnitFireControlValue -= m_pBotInfo_Weapon->fFire1BlendOutSpeed * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( fUnitFireControlValue, 0.0f );
			SetControlValue( uFireAnimControlId, fUnitFireControlValue );
		}
	}
	*/

	// Secondary fire...
	if( m_nThrowState != THROWSTATE_NONE )
	{
		//floop_SetTimeScale(1./60.);
		f32 fUnitTime;

		if( DeltaTime( ANIMTAP_FIRE_2, 2.0f * FLoop_fPreviousLoopSecs, TRUE ) )
		{
			fUnitTime = 1.0f;
			fUnitFireControlValue = 0.0f;
		}
		else
		{
			fUnitTime = GetUnitTime( ANIMTAP_FIRE_2 );

			if( fUnitTime < m_pBotInfo_Weapon->fThrowBlendInUnitTime )
			{
				fUnitFireControlValue = fUnitTime * m_pBotInfo_Weapon->fOOThrowBlendInUnitTime;
			}
			else if( fUnitTime > m_pBotInfo_Weapon->fThrowBlendOutUnitTime )
			{
				fUnitFireControlValue = 1.01f - (fUnitTime - m_pBotInfo_Weapon->fThrowBlendOutUnitTime) * m_pBotInfo_Weapon->fOOInvThrowBlendOutUnitTime;
			}
			else
			{
				fUnitFireControlValue = 1.0f;
			}
		}


		SetControlValue( ANIMCONTROL_FIRE_2, fUnitFireControlValue );

		switch( m_nThrowState )
		{
		case THROWSTATE_REACHING_FOR_AMMO:
			if( fUnitTime <= 0.25f )
			{
				break;
			}

			// Attach ammo...
			m_apWeapon[1]->Throwable_AttachGrenadeToOwnerBotBone( "Secondary_Fire" );
			m_nThrowState = THROWSTATE_WINDING_UP;

			// Fall through to next state...

		case THROWSTATE_WINDING_UP:
			if( fUnitTime <= 0.56f )
			{
				break;
			}

			// Attach ammo...
			m_apWeapon[1]->Throwable_ThrowGrenade_MountAimDirection();
			m_nThrowState = THROWSTATE_RECOVERING;

			// Fall through to next state...

		case THROWSTATE_RECOVERING:
			if( fUnitTime >= 1.0f )
			{
				m_nThrowState = THROWSTATE_NONE;
			}

			break;
		};
	}
	else
	{
//		floop_SetTimeScale(1.);
	}
	
}


void CBotSlosh::_HandleAimAnimations( void )
{
	f32 fUnitAimPitch;
	CFAnimFrame TorsoQuat, HeadQuat;

	// Compute the amount we're aiming up/down (up=1, down=0, middle=0.5)...
	fUnitAimPitch = fmath_Div( m_fMountPitch_WS - m_fMountPitchMin_WS, m_fMountPitchMax_WS - m_fMountPitchMin_WS );
	FMATH_CLAMP_UNIT_FLOAT( fUnitAimPitch );

	// Update the torso-twist blend value...
	if( m_fUnitTorsoTwistBlend > 0.0f )
	{
		m_fUnitTorsoTwistBlend -= 2.0f * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fUnitTorsoTwistBlend, 0.0f );
	}


	// Compute the head and torso quaternions...
	TorsoQuat.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( fUnitAimPitch, FMATH_DEG2RAD(-30.0f), FMATH_DEG2RAD(30.0f) ), CFVec3A::m_Null );

	if( m_fUnitTorsoTwistBlend == 0.0f )
	{
		// No torso twist...
		// Torso quaternion has already been computed.
		// Compute head quaternion...

		HeadQuat.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( fUnitAimPitch, FMATH_DEG2RAD(-20.0f), FMATH_DEG2RAD(20.0f) ), CFVec3A::m_Null );

	}
	else
	{
		// We must introduce some rocket-launcher-L1 torso twist...

		if( m_fUnitTorsoTwistBlend == 1.0f )
		{
			// 100% torso twist...

			// Compute the head quaternion...
		}
		else
		{
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


void CBotSlosh::_HandleWeaponAnimations( void )
{
	HandleWeaponReloadingAnimations();
	_HandleFiringAnimations();
	_HandleAimAnimations();
}


void CBotSlosh::_UpdateMatrices( void )
{
	CFMtx43A FlipMtx, MeshMtx, NewMtx, EntityMtxToWorld;

	CFMtx43A::m_XlatRotY.SetRotationY( m_fMountYaw_WS + m_fLegsYaw_MS );
	CFMtx43A::m_XlatRotY.m_vPos = m_MountPos_WS;

	// Update xfm...
	if( m_fFlipPitch )
	{
		CFMtx43A::m_XlatRotX.m_vPos.Set( m_pBotInfo_Jump->fFlipOriginX, m_pBotInfo_Jump->fFlipOriginY, m_pBotInfo_Jump->fFlipOriginZ );

		if( m_fFlipPitch >= 0.0f )
		{
			CFMtx43A::m_XlatRotX.SetRotationX( fmath_UnitLinearToSCurve( m_fFlipPitch * (1.0f/FMATH_2PI) ) * FMATH_2PI );
		}
		else
		{
			CFMtx43A::m_XlatRotX.SetRotationX( fmath_UnitLinearToSCurve( m_fFlipPitch * (-1.0f/FMATH_2PI) ) * -FMATH_2PI );
		}

		CFMtx43A::m_Xlat.m_vPos.ReceiveNegative( CFMtx43A::m_XlatRotX.m_vPos );

		FlipMtx.Mul( CFMtx43A::m_XlatRotX, CFMtx43A::m_Xlat );

		MeshMtx.Mul( CFMtx43A::m_XlatRotY, FlipMtx );
		m_pWorldMesh->m_Xfm.BuildFromMtx( MeshMtx );

		CFMtx43A::m_XlatRotY.SetRotationY( m_fMountYaw_WS );
	}
	else
	{
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


// Returns TRUE if Grunt's finger is on his primary weapon's trigger.
// In other words, TRUE is returned if Grunt is in a state where he
// can pull the trigger.
BOOL CBotSlosh::_IsFingerOnTrigger1( void ) const
{
	BOOL bFingerIsOnTrigger = FALSE;

	if( m_nWSState==WS_STATE_NONE || m_nWSState==WS_STATE_RELEASING )
	{
		// We're not switching or reloading the weapon...

		if( (m_fFlipPitch == 0.0f) || (fmath_Abs(m_fLegsYaw_MS) > (0.8f * FMATH_HALF_PI)) ) {
			// We're not flipping...

			switch( m_apWeapon[0]->m_pInfo->nGrip )
			{
			case CWeapon::GRIP_RIGHT_ARM:
				bFingerIsOnTrigger = TRUE;
				break;

			case CWeapon::GRIP_BOTH_ARMS:
				if( m_nJumpState == BOTJUMPSTATE_NONE )
				{
					f32 fLandBlend = GetControlValue( ANIMCONTROL_JUMP_LAND_UPPER );

					if( fLandBlend == 0.0f )
					{
						if( m_nState == STATE_GROUND )
						{
							bFingerIsOnTrigger = TRUE;
						}
						else
						{
							if( m_fUnitFlyBlend == 0.0f )
							{
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


// Returns TRUE if Grunt's finger is on his secondary weapon's trigger.
// In other words, TRUE is returned if Grunt is in a state where he
// can pull the trigger.
BOOL CBotSlosh::_IsFingerOnTrigger2( void ) const
{
	BOOL bFingerIsOnTrigger = FALSE;

	if( m_nThrowState == THROWSTATE_NONE )
	{
		// Not currently throwing a secondary grenade...

		if( m_nWSState==WS_STATE_NONE || m_nWSState==WS_STATE_RELEASING )
		{
			// We're not switching or reloading the weapon...

			if( m_apWeapon[0]->m_pInfo->nGrip == CWeapon::GRIP_RIGHT_ARM )
			{
				// Primary weapon is not two-handed...

				if( m_nJumpState != BOTJUMPSTATE_CABLE )
				{
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


void CBotSlosh::_HandleWeaponFiring( void )
{
	f32 fTrig1, fTrig2;
	u32 nFireBitsPrimary;

	fTrig1 = m_fControls_Fire1;
	fTrig2 = m_fControls_Fire2;

	if( !_IsFingerOnTrigger1() )
	{
		fTrig1 = 0.0f;
	}

	if( !_IsFingerOnTrigger2() )
	{
		fTrig2 = 0.0f;
	}

	nFireBitsPrimary = m_apWeapon[0]->TriggerWork( fTrig1, fTrig2, &m_TargetedPoint_WS, NULL );

	// don't update the tank if we're dying.  It's gone
	if( m_pTank ) {
		f32 fTankFillPercent = fmath_Div((f32)m_apWeapon[0]->GetClipAmmo(),(f32)m_apWeapon[0]->GetMaxClipAmmo()) ;
		FMATH_CLAMP_UNIT_FLOAT(fTankFillPercent);
		if ( (m_nWRState == WR_STATE_CLIP_SLAPIN_NEW) && (m_pTank->m_eTankState == TR_STATE_ACTIVE) )
		{
			m_pTank->m_eTankState = TR_STATE_REFILLING;
		}
		m_pTank->UpdateTank(fTankFillPercent);

		m_pTankLight->m_Light.SetIntensity(*m_pfTankFillPercent);
	}


	if( nFireBitsPrimary )
	{
		// We fired the primary weapon...
		JustFired();

//		FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM );
//		ZeroTime( ANIMTAP_FIRE_1 );
//		ZeroTime( ANIMTAP_FIRE_1_ALERT );

		JustFired();  // No more disguise...
	}

	if( m_apWeapon[1] )
	{
		if( m_apWeapon[1]->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_THROWABLE ) 
		{
			// Secondary is throwable...
			if( m_apWeapon[1]->Throwable_TriggerWork( fTrig2 ) ) 
			{
				// We can throw the grenade...

				m_nThrowState = THROWSTATE_REACHING_FOR_AMMO;
				ZeroTime( ANIMTAP_FIRE_2 );

				JustFired();  // No more disguise...
			}
		}
	}
}



void CBotSlosh::_AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx )
{
	FASSERT( m_pCollBot && (m_pCollBot->TypeBits() & ENTITY_BIT_BOTCHEMBOT) );

	if( ((CBotSlosh *)m_pCollBot)->m_pPartMgr->AnimBoneCallbackFunctionHandler( nBoneIndex, rNewMtx, rParentMtx, rBoneMtx ) ) {
		return;
	}

	if( nBoneIndex == (u32)((CBotSlosh *)m_pCollBot)->m_nBoneIndexGroin )
	{
		// Groin
		rNewMtx.Mul( rParentMtx, rBoneMtx );
		m_GroinVecY_WS = rNewMtx.m_vUp;
	}
	else if( nBoneIndex == (u32)((CBotSlosh *)m_pCollBot)->m_nBoneIndexTorso )
	{
		// Torso
		CFMtx43A WorldMtx;
		CFQuatA Quat;
		CBotSlosh *pBotGrunt = (CBotSlosh *)m_pCollBot;

		Quat.BuildQuat( m_GroinVecY_WS, 0/*pBotGrunt->m_fAimDeltaYaw_MS*/ - pBotGrunt->m_fLegsYaw_MS );
		WorldMtx.Mul( rParentMtx, rBoneMtx );

		rNewMtx.m_vPos = WorldMtx.m_vPos;
		Quat.MulPoint( rNewMtx.m_vRight, WorldMtx.m_vRight );
		Quat.MulPoint( rNewMtx.m_vUp, WorldMtx.m_vUp );
		Quat.MulPoint( rNewMtx.m_vFront, WorldMtx.m_vFront );
	}
	else
	{
//		// Head
//		m_pCollBot->HeadLookUpdate( rNewMtx, rParentMtx, rBoneMtx, FALSE );
		rNewMtx.Mul( rParentMtx, rBoneMtx );
	}
}


BOOL CBotSlosh::_InventoryCallback( CInventoryCallbackReason_e eReason, CInventory *pInventory, u32 nHandIndex, u32 nWeaponIndex )
{
	CBotSlosh *pBotSlosh = (CBotSlosh *)pInventory->m_pUser;

	switch( eReason )
	{
	case IREASON_WEAPONCHANGE:
		FASSERT_NOW;

		break;

	case IREASON_RELOAD:
		if( nHandIndex == 0 )
		{
			pBotSlosh->m_abReloadRequest[0] = TRUE;
		}

		break;

	case IREASON_WEAPONUPGRADED:
		FASSERT_NOW;

		break;

	}

	return TRUE;
}



void CBotSlosh::SpawnDeathEffects(void)
{
	//note! patm put this code here just so that we know when a grunt is killed.
	//probably, someday there will be a death anim, or ragdoll, or propper spewage of grunt chunks or something.


	if (m_uBotDeathFlags & BOTDEATHFLAG_PLAYDEATHANIM)
	{
		//if this guy is going to do a death anim, no chunks
		return;
	}
	else if (m_uBotDeathFlags & BOTDEATHFLAG_COMEAPART )
	{

	}
}

void CBotSlosh::CheckpointSaveSelect( s32 nCheckpoint ) 
{
	// save bot's weapons first
	if( m_apWeapon[0] )
	{
		m_apWeapon[0]->CheckpointSaveSelect( nCheckpoint );
	}

	if( m_apWeapon[1] )
	{
		m_apWeapon[1]->CheckpointSaveSelect( nCheckpoint );
	}

	// then save self
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}


void CBotSlosh::InflictDamage( CDamageData *pDamageData)
{
	CBot* pWhoHitMe = pDamageData->m_Damager.pBot; // when hit by a possessed bot, 

	if (pWhoHitMe==this) // don't take damage from self
		return;

	CBot::InflictDamage(pDamageData);
}


void CBotSlosh::Die( BOOL bSpawnDeathEffects/*=TRUE*/, BOOL bSpawnGoodies/*=TRUE*/ ) {
    FASSERT( IsCreated() );

	CBot::Die( bSpawnDeathEffects, bSpawnGoodies );
}


void CBotSlosh::DeathWork( void ) {
	CBot::DeathWork();

	if( m_pPartMgr->GetLimbState( LIMB_TYPE_TANK ) == CBotPartMgr::LIMB_STATE_REMOVED ) {
		if( m_pTank ) {
			CSloshTank::PutTankBack( m_pTank );
			m_pTank = NULL;
		}
	}
}













