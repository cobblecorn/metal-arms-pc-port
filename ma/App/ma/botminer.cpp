//////////////////////////////////////////////////////////////////////////////////////
// botminer.cpp - 
//
// Author: Mike Elliott
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
// 02/26/03 Elliott		Cut & pasted from botgrunt.cpp
//////////////////////////////////////////////////////////////////////////////////////
 
#include "fang.h"
#include "botminer.h"
#include "fanim.h"
#include "fresload.h"
#include "floop.h"
#include "gamepad.h"
#include "ftimer.h"
#include "ftext.h"
#include "fclib.h"
#include "ItemInst.h"
#include "ItemRepository.h"
#include "fforce.h"
#include "gstring.h"
#include "meshtypes.h"
#include "weapons.h"
#include "reticle.h"
#include "player.h"
#include "level.h"
#include "ProTrack.h"
#include "fsndfx.h"
#include "meshentity.h"
#include "zipline.h"
#include "botpart.h"
#include "Ai\AIEnviro.h"
#include "eparticlepool.h"



#define _BOTINFO_FILENAME		"b_Miner"
#define _BOTPART_FILENAME		"bp_Miner"
#define _HEADLIGHT_TEXTURE_ID	( 1 )
#define _BUDDY_ROCKET_EUK		( 5 )

static char* _pszMinerMeshFilenameRoot = "GRDMminer";
static cchar* _apszMinerMeshFilenames[] = {"GRDMminer00", "GRDMminer00"};	 //"Gom_ctytrs1", "GRMUScout00",   
cchar* BotMiner_pszSoundEffectBank = "Miner";


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotMinerBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CBotMinerBuilder _BotMinerBuilder;


void CBotMinerBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_BOTMINER, pszEntityType );

	// Override defaults set by our parent classes...
	m_nEC_HealthContainerCount = CBotMiner::m_BotInfo_Gen.nBatteryCount;
	m_pszEC_ArmorProfile = CBotMiner::m_BotInfo_Gen.pszArmorProfile;

	m_anNPCWeaponType[0] = _NPC_WEAPON_LASER;		// NPC Miners default to a laser
	m_anNPCWeaponType[1] = _NPC_WEAPON_HAND;		// NPC Miners default to a hand

	m_nNPCStoredWeaponType = _NPC_WEAPON_HAND;		// NPC Miners default to a rocket launcher baby!

	u32 i;
	for (i = 0; i < CBotMiner::ANIM_BASE_COUNT; i++)
	{
		m_apszAnimFileOverrides[i] = NULL;
	}

	// This class of bots shows up on Glitch's radar...
	FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_SHOWS_UP_ON_RADAR );
}


BOOL CBotMinerBuilder::PostInterpretFixup( void ) {
	if( !CBotBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	u32 i;
	for (i = 0; i < CBotMiner::ANIM_BASE_COUNT; i++)
	{
		if( m_apszAnimFileOverrides[i] && !(m_apszAnimFileOverrides[i] = gstring_Main.AddString( m_apszAnimFileOverrides[i] )) ) {
			goto _ExitWithError;
		}
	}

	return TRUE;

	// Error:
_ExitWithError:
	return FALSE;
}


BOOL CBotMinerBuilder::InterpretTable( void ) {
	cchar *pszString;

	//this is legacy because Miner used to have something called Minerweapon, but
	//now any bot can get it's weapons configured via the NPCWeapon0 and NPCWeapon1 tags. 
	//see bot.cpp
	if( !fclib_stricmp( CEntityParser::m_pszTableName, "Weapon" ) ) {

		if( CEntityParser::Interpret_String( &pszString ) ) {
			if( !fclib_stricmp( pszString, "Laser" ) ) {
				m_anNPCWeaponType[0] = _NPC_WEAPON_LASER;
			} else if( !fclib_stricmp( pszString, "Spew" ) ) {
				m_anNPCWeaponType[0] = _NPC_WEAPON_SPEW;
			} else {
				CEntityParser::Error_InvalidParameterValue();
			}
		}

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "StoredWeapon" ) ) {
		cchar *pszString = NULL;
		if( CEntityParser::Interpret_String( &pszString ) ) {
			if (!fclib_stricmp( pszString, "HAND")) {
				m_nNPCStoredWeaponType = _NPC_WEAPON_HAND;
			} else if (!fclib_stricmp( pszString, "LASER")) {
				m_nNPCStoredWeaponType = _NPC_WEAPON_LASER;
			} else if (!fclib_stricmp( pszString, "SPEW")) {
				m_nNPCStoredWeaponType = _NPC_WEAPON_SPEW;
			} else if (!fclib_stricmp( pszString, "FLAMER")) {
				m_nNPCStoredWeaponType = _NPC_WEAPON_FLAMER;
			} else if (!fclib_stricmp( pszString, "BLASTER")) {						  
				m_nNPCStoredWeaponType = _NPC_WEAPON_BLASTER;							  
			} else if (!fclib_stricmp( pszString, "RIVET")) {						  
				m_nNPCStoredWeaponType = _NPC_WEAPON_RIVET;
			} else if (!fclib_stricmp( pszString, "ROCKET")) {						  
				m_nNPCStoredWeaponType = _NPC_WEAPON_ROCKET;							  
			} else {																  
				CEntityParser::Error_InvalidParameterValue();						  
			}																		  
		}																			  
		return TRUE;																  
	}

	if (CEntityParser::m_nFieldCount == 2 && !fclib_strnicmp( CEntityParser::m_pszTableName, "ANIM_REP", fclib_strlen("ANIM_REP") ) )
	{
		FGameDataTableHandle_t hCurTable = CEntityParser::m_hTable;
		FGameData_VarType_e uFieldType = FGAMEDATA_VAR_TYPE_COUNT;
		cchar *pszTapName = (cchar *)(fgamedata_GetPtrToFieldData(hCurTable, 0, uFieldType));
		if (pszTapName && uFieldType != FGAMEDATA_VAR_TYPE_STRING)
		{
			pszTapName = NULL;
		}
		cchar *pszAnimRepName = (cchar *)(fgamedata_GetPtrToFieldData(hCurTable, 1, uFieldType));
		if (pszAnimRepName && uFieldType != FGAMEDATA_VAR_TYPE_STRING)
		{
			pszAnimRepName = NULL;
		}
		u32 i;
		if (pszTapName && pszAnimRepName)
		{
			if (!fclib_stricmp(pszTapName, "UBER_ANIM"))
			{
				for (i = 0; i < CBotMiner::ANIM_BASE_COUNT; i++)
				{
					if (!m_apszAnimFileOverrides[i])
					{	//override a Miner anim
						m_apszAnimFileOverrides[i] = pszAnimRepName;
					}
				}
			}
			else
			{
				for (i = 0; i < CBotMiner::ANIM_BASE_COUNT; i++)
				{
					if (!fclib_stricmp( pszTapName, CBotMiner::m_apszBaseTapNameTable[i] ) )
					{	//override a Miner anim
						m_apszAnimFileOverrides[i] = pszAnimRepName;
					}
				}
			}
		}
		return TRUE;
	}

	return CBotBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotMiner
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CBotMiner::m_bSystemInitialized = FALSE;

CBotMiner::BotInfo_Gen_t CBotMiner::m_BotInfo_Gen;
CBotMiner::BotInfo_MountAim_t CBotMiner::m_BotInfo_MountAim;
CBotMiner::BotInfo_Walk_t CBotMiner::m_BotInfo_Walk;
CBotMiner::BotInfo_Jump_t CBotMiner::m_BotInfo_Jump;
CBotMiner::BotInfo_Weapon_t CBotMiner::m_BotInfo_Weapon;

CBotAnimStackDef CBotMiner::m_AnimStackDef;

CFAnimFrame CBotMiner::m_Rocket1TorsoTwistQuat;
CFAnimFrame CBotMiner::m_Rocket1TorsoUntwistQuat;

CFVec3A CBotMiner::m_GroinVecY_WS;
CBotPartPool *CBotMiner::m_pPartPool;
CBotMiner*   CBotMiner::m_pCBMiner = NULL;





BOOL CBotMiner::InitSystem( void ) {
	FResFrame_t ResFrame;

	FASSERT( !m_bSystemInitialized );

	ResFrame = fres_GetFrame();

	m_bSystemInitialized = TRUE;

	//default for the CSV defaults
	m_BotInfo_Walk.fHopLRImpulseMag = 12.0f;
	m_BotInfo_Walk.fHopAnimLandPct = 0.75f;
	m_BotInfo_Walk.fApproxHopLRDist = 3.0f;

	m_BotInfo_Walk.fRollStickForceWhileRolling = 1.0f;
	m_BotInfo_Walk.fRollLRImpulseMagXZ = 23.0f;
	m_BotInfo_Walk.fRollLRImpulseMagY = 6.0f;
	m_BotInfo_Walk.fRollAnimLandPct = 0.85f;
	m_BotInfo_Walk.fApproxRollLRDist= 15.0f;

	if( !ReadBotInfoFile( m_aGameDataMap, _BOTINFO_FILENAME ) ) {
		goto _ExitInitSystemWithError;
	}

	if( !_BuildAnimStackDef() ) {
		goto _ExitInitSystemWithError;
	}

	m_Rocket1TorsoTwistQuat.BuildQuat( CFVec3A::m_UnitAxisY, FMATH_DEG2RAD(55.0f), CFVec3A::m_Null );
	m_Rocket1TorsoUntwistQuat.BuildQuat( CFVec3A::m_UnitAxisY, FMATH_DEG2RAD(-55.0f), CFVec3A::m_Null );

	//m_uNumDeathChunkMeshs = sizeof(_pszMinerDeathChunkMeshNames)/sizeof(cchar*);
	//m_papDeathChunkMeshs = (FMesh_t **) fres_Alloc(sizeof(FMesh_t*) * m_uNumDeathChunkMeshs );
	//if (!m_papDeathChunkMeshs) {
	//	DEVPRINTF("CGotBrunt::InitSystem() : No Mem for DeathChunks'\n" );
	//	goto _ExitInitSystemWithError;
	//}

	//for(i = 0; i < m_uNumDeathChunkMeshs; i++) {
	//	m_papDeathChunkMeshs[i] = (FMesh_t *) fresload_Load(FMESH_RESTYPE, _pszMinerDeathChunkMeshNames[i]);
	//	if(m_papDeathChunkMeshs[i] == NULL)	{
	//		DEVPRINTF("CGotBrunt::InitSystem() : Could not load death chunk mesh '%s' .\n", _pszMinerDeathChunkMeshNames[i]);
	//		goto _ExitInitSystemWithError;
	//	}
	//}

	InitParticleEmitterLock();

	return TRUE;

	// Error:
_ExitInitSystemWithError:
	UninitSystem();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

void CBotMiner::UninitSystem( void ) {
	if( m_bSystemInitialized ) {
		m_AnimStackDef.Destroy();
		//m_uNumDeathChunkMeshs = 0;
		//m_papDeathChunkMeshs = NULL;
		m_bSystemInitialized = FALSE;
	}
}

void* CBotMiner::m_ahParticleEmitter[NUM_PARTICLE_EMITTER_LOCKS];		
CBot* CBotMiner::m_apParticleEmitterLock[NUM_PARTICLE_EMITTER_LOCKS];	
void* CBotMiner::m_ahParticleEmitterDef[NUM_PARTICLE_EMITTER_LOCKS];			// Particle def to be used for particleEmitters
cchar* CBotMiner::m_apszParticleDefNames[] = {"smokey","smoke1","sparky02"};

BOOL CBotMiner::InitParticleEmitterLock(void)
{
	u32 i;
	for ( i=0; i < NUM_PARTICLE_EMITTER_LOCKS; i++)
	{
		m_ahParticleEmitter[i] = NULL;						// Shared classwide is the emitter for Particles coming from the miner when he is in terminal panic
		m_apParticleEmitterLock[i] = NULL;					// the grunt who has the Particle emitter locked
	}

	u32 uNumParticleDefs = sizeof(m_apszParticleDefNames)/ sizeof(cchar*);
	u32 j = fmath_RandomChoice(uNumParticleDefs);
	for (i = 0; i < NUM_PARTICLE_EMITTER_LOCKS ; i++)
	{
		m_ahParticleEmitterDef[i] = fresload_Load( FPARTICLE_RESTYPE, m_apszParticleDefNames[j] );
		if( m_ahParticleEmitterDef[i] == FPARTICLE_INVALID_HANDLE ) {
			DEVPRINTF( "CBotMiner::InitParticleEmitterLock(): Could not find particle definition '%s'.\n", m_apszParticleDefNames[j] );
		}
		j++;
		if (j >= uNumParticleDefs)
		{
			j=0;
		}
	}
	return TRUE;
}

void CBotMiner::UninitParticleEmitterLock(void)
{
}


//static
BOOL CBotMiner::RequestParticleEmitterLock(CBot* pWho, u8* pnLockId)
{
	FASSERT(pnLockId);
	if (pnLockId)
	{
		for (u8 i = 0;i < NUM_PARTICLE_EMITTER_LOCKS; i++)
		{
			if (m_apParticleEmitterLock[i] == NULL)
			{
				m_apParticleEmitterLock[i] = pWho;
				*pnLockId = i;
				return TRUE;
			}
		}
	}
	return FALSE;
}

//static
void CBotMiner::FreeParticleEmitterLock(void* pEmitter, CBot* pWho)
{
	for (u32 i = 0;i < NUM_PARTICLE_EMITTER_LOCKS; i++)
	{
		if (m_apParticleEmitterLock[i] == pWho)
		{
			if (m_ahParticleEmitter[i])
			{
				fparticle_StopEmitter(m_ahParticleEmitter[i]);
				m_ahParticleEmitter[i] = NULL;
				m_apParticleEmitterLock[i] = NULL;
				return;
			}
		}
	}
}


//static
void CBotMiner::FreeAllParticleEmitterLocks(CBot* pWho)
{
	for (u32 i = 0;i < NUM_PARTICLE_EMITTER_LOCKS; i++)
	{
		if (m_apParticleEmitterLock[i] == pWho)
		{
			if (m_ahParticleEmitter[i])
			{
				m_ahParticleEmitter[i] = NULL;
			}
			m_apParticleEmitterLock[i] = NULL;
		}
	}
}



BOOL CBotMiner::HasParticleEmitterLock(CBot* pWho)
{
	for (u32 i=0; i < NUM_PARTICLE_EMITTER_LOCKS; i++)
	{
		if (m_apParticleEmitterLock[i] == pWho)
		{
			return TRUE;
		}
	}
	return FALSE;
}


CBotMiner::CBotMiner() : CBot() {
	m_pInventory = NULL;
	m_pWorldMesh = NULL;
}


CBotMiner::~CBotMiner() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CBotMiner::Create( s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get pointer to the leaf class's builder object...
	CBotMinerBuilder *pBuilder = (CBotMinerBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_BotMinerBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOTMINER );
	}

	// Set our builder parameters...

	// Create an entity...
	return CBot::Create( &m_BotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}


void CBotMiner::ClassHierarchyDestroy( void ) {
	// Delete the items that we had instantiated for us...
	fdelete( m_apWeapon[0] );
	m_apWeapon[0] = NULL;

	fdelete( m_apWeapon[1] );
	m_apWeapon[1] = NULL;

	if( m_pStoredWeapon ) {
		fdelete( m_pStoredWeapon );
		m_pStoredWeapon = NULL;
	}

	m_Anim.Destroy();

	fdelete( m_pWorldMesh );
	m_pWorldMesh = NULL;

	CBot::ClassHierarchyDestroy();
}


BOOL CBotMiner::ClassHierarchyBuild( void ) {
	FMesh_t *pMesh;
	FMeshInit_t MeshInit;
	u32 i;
	s32 nBoneIndex, nLeftHandSecondaryFire;
	cchar * pszMeshFilename = NULL;

	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();


	// Get pointer to the leaf class's builder object...
	CBotMinerBuilder *pBuilder = (CBotMinerBuilder *)GetLeafClassBuilder();

	// Set input parameters for CBot creation...
	pBuilder->m_pBotDef = &m_BotDef;

#if !FANG_WINGC	// retail has no Miner bank (main.dol names none for this bot); its sounds come from the level's banks
	// Load the sound effects bank for this bot...
	if( !fresload_Load( FSNDFX_RESTYPE, BotMiner_pszSoundEffectBank) ) {
		DEVPRINTF( "CBotMiner::ClassHierarchyBuild(): Could not load sound effect bank '%s'\n", BotMiner_pszSoundEffectBank);
	}
#endif

	// Build parent class...
	if( !CBot::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	// Initialize from builder object...

	// Load mesh resource...
	pszMeshFilename = _apszMinerMeshFilenames[0];
	if ( pBuilder->m_uMeshVersionOverride >0 && pBuilder->m_uMeshVersionOverride < sizeof(_apszMinerMeshFilenames)/sizeof(char*) ) {
		pszMeshFilename = _apszMinerMeshFilenames[pBuilder->m_uMeshVersionOverride];
	}

	if( pBuilder->m_pszMeshReplacement ) {
	   pszMeshFilename = pBuilder->m_pszMeshReplacement;
	}

	pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pszMeshFilename);
	if( pMesh == NULL ) {
		DEVPRINTF( "CBotMiner::ClassHierarchyBuild(): Could not find mesh '%s'\n", pszMeshFilename );
		goto _ExitWithError;
	}

	// Create world mesh...
	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL ) {
		DEVPRINTF( "CBotMiner::ClassHierarchyBuild(): Not enough memory to create CFWorldMesh.\n" );
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


	for (i = 0; i < ANIM_BASE_COUNT; i++)
	{
		if (!pBuilder->m_apszAnimFileOverrides[i])
		{
			pBuilder->m_apszAnimFileOverrides[i] = m_apszBaseAnimNameTable[i];
		}
	}
	m_AnimStackDef.m_apszBaseAnimNameTable = pBuilder->m_apszAnimFileOverrides;

	// Build our animation stack and load animations...
	if( !m_Anim.Create( &m_AnimStackDef, m_pWorldMesh ) ) {
		DEVPRINTF( "CBotMiner::ClassHierarchyBuild(): Trouble creating m_Anim.\n" );
		goto _ExitWithError;
	}


	SetControlValue( ANIMCONTROL_STAND, 1.0f );
	UpdateUnitTime( ANIMTAP_STAND, fmath_RandomFloat() );
//	EnableControlSmoothing( ANIMCONTROL_AIM_ROCKET1 );
//	EnableControlSmoothing( ANIMCONTROL_AIM_ROCKET23 );
//	EnableControlSmoothing( ANIMCONTROL_FIRE_2_UPPER );

	m_nBoneIndexGroin = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_GROIN] );
	m_nBoneIndexTorso = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_TORSO] );
	m_nBoneIndexPriFire = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ATTACHPOINT_PRIMARY] );

	if( (m_nBoneIndexGroin < 0) || (m_nBoneIndexTorso < 0) || (m_nBoneIndexPriFire < 0) ) {
		DEVPRINTF( "CBotMiner::ClassHierarchyBuild(): Trouble retrieving bone indices.\n" );
	//	goto _ExitWithError;
	}

	m_Anim.m_pAnimCombiner->SetBoneCallback( &_AnimBoneCallback );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_GROIN] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_TORSO] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_HEAD] );

	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_LEFT_HEEL] );
	if( nBoneIndex >= 0 ) {
		m_pLeftFootDustPos_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;

		nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_RIGHT_HEEL] );
		if( nBoneIndex >= 0 ) {
			m_pRightFootDustPos_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
		} else {
			m_pLeftFootDustPos_WS = NULL;
		}
	}

	if( !m_AnimManFrameAim.Create( m_AnimStackDef.m_nBoneCount, m_AnimStackDef.m_apszBoneNameTable ) ) {
		DEVPRINTF( "CBotMiner::ClassHierarchyCreate(): Could not create aim animation summer.\n" );
		goto _ExitWithError;
	}
	AttachAnim( ANIMTAP_AIM_SUMMER, &m_AnimManFrameAim );
	m_AnimManFrameAim.Reset();
	UpdateBoneMask( ANIMTAP_AIM_SUMMER, m_aBoneEnableIndices_AimSummer, TRUE );

	// Set primary and secondary weapon.........
	if (m_nBoneIndexPriFire > -1)
	{
		for (i = 0; i < 2; i++) {
			m_apWeapon[i] = CBotBuilder::AllocNPCWeaponOfType(pBuilder->m_anNPCWeaponType[i]);
			if( m_apWeapon[i] == NULL ) {
				DEVPRINTF( "CBotMiner::ClassHierarchyBuild() : Error instantiating CWeapon %d.\n", i );
				goto _ExitWithError;
			}
			if (!CBotBuilder::CreateNPCWeaponOfType(m_apWeapon[i], pBuilder->m_anNPCWeaponType[i]))
			{
				DEVPRINTF( "CBotMiner::ClassHierarchyBuild() : Error Creating CWeapon %d.\n", i );
				goto _ExitWithError;
			}


			if( pBuilder->m_anNPCWeaponType[i] == CBotBuilder::_NPC_WEAPON_HAND ) {
				m_apWeapon[i]->SetUpgradeLevel( 1 );	// hand euk 1 is the miner hand
			} 
			else if ( pBuilder->m_anNPCWeaponType[i] == CBotBuilder::_NPC_WEAPON_ROCKET )
			{
				m_apWeapon[i]->SetUpgradeLevel( _BUDDY_ROCKET_EUK );
			}
			else if (3<m_apWeapon[i]->GetUpgradeLevelCount())
			{
				m_apWeapon[i]->SetUpgradeLevel(0);	  //3 is special level for npc
			}
			else
			{
				m_apWeapon[i]->SetUpgradeLevel(0);	  //3 is special level for npc
			}


			if (pBuilder->m_anNPCWeaponType[i] == CBotBuilder::_NPC_WEAPON_SPEW)
			{
				m_apWeapon[i]->SetClipAmmo( 30 );
				m_apWeapon[i]->OverrideMaxReserveAmmo(CWeapon::INFINITE_AMMO );
				m_apWeapon[i]->SetReserveAmmo( CWeapon::INFINITE_AMMO );
			}
			else if (pBuilder->m_anNPCWeaponType[i] == CBotBuilder::_NPC_WEAPON_FLAMER)
			{
				m_apWeapon[i]->SetClipAmmo( 200 );
				m_apWeapon[i]->SetReserveAmmo( CWeapon::INFINITE_AMMO );
			} 
			else if (pBuilder->m_anNPCWeaponType[i] == CBotBuilder::_NPC_WEAPON_ROCKET)
			{
				m_apWeapon[i]->SetReserveAmmo( CWeapon::INFINITE_AMMO );
				m_apWeapon[i]->SetClipAmmo( CWeapon::INFINITE_AMMO );
			}

			m_apWeapon[i]->EnableAutoWork( FALSE );
			m_apWeapon[i]->RemoveFromWorld();
			m_apWeapon[i]->SetOwner(this);
			m_apWeapon[i]->SetDesiredState( CWeapon::STATE_DEPLOYED );
		}
	}

	// build the weapon stored in backpack
	m_pInitialPrimaryWeapon = m_apWeapon[0];

	m_pStoredWeapon = CBotBuilder::AllocNPCWeaponOfType(pBuilder->m_nNPCStoredWeaponType);
	if( m_pStoredWeapon ) {
		if (!CBotBuilder::CreateNPCWeaponOfType(m_pStoredWeapon, pBuilder->m_nNPCStoredWeaponType))
		{
			DEVPRINTF( "CBotMiner::ClassHierarchyBuild() : Error Creating stored CWeapon\n" );
			goto _ExitWithError;
		}
		if( pBuilder->m_nNPCStoredWeaponType == CBotBuilder::_NPC_WEAPON_HAND ) {
			m_pStoredWeapon->SetUpgradeLevel( 1 );	// hand euk 1 is the miner hand
		} else if( pBuilder->m_nNPCStoredWeaponType == CBotBuilder::_NPC_WEAPON_ROCKET ) {
			m_pStoredWeapon->SetUpgradeLevel( _BUDDY_ROCKET_EUK );
		}
		else if (3<m_pStoredWeapon->GetUpgradeLevelCount())
		{
			m_pStoredWeapon->SetUpgradeLevel(0);	  //3 is special level for npc
		}
		else
		{
			m_pStoredWeapon->SetUpgradeLevel(0);	  //3 is special level for npc
		}

		if (pBuilder->m_nNPCStoredWeaponType == CBotBuilder::_NPC_WEAPON_SPEW)
		{
			m_pStoredWeapon->SetClipAmmo( 30 );
			m_pStoredWeapon->OverrideMaxReserveAmmo(CWeapon::INFINITE_AMMO );
			m_pStoredWeapon->SetReserveAmmo( CWeapon::INFINITE_AMMO );
		}
		else if (pBuilder->m_nNPCStoredWeaponType == CBotBuilder::_NPC_WEAPON_FLAMER)
		{
			m_pStoredWeapon->SetClipAmmo( 200 );
			m_pStoredWeapon->SetReserveAmmo( CWeapon::INFINITE_AMMO );
		}
		else if (pBuilder->m_nNPCStoredWeaponType == CBotBuilder::_NPC_WEAPON_ROCKET)
		{
			m_pStoredWeapon->SetReserveAmmo( CWeapon::INFINITE_AMMO );
			m_pStoredWeapon->SetClipAmmo( CWeapon::INFINITE_AMMO );
		}

		m_pStoredWeapon->EnableAutoWork( FALSE );
		m_pStoredWeapon->RemoveFromWorld();
		m_pStoredWeapon->SetOwner(this);
		m_pStoredWeapon->SetDesiredState( CWeapon::STATE_STOWED );	
	}

	m_bInitialWeaponActive		= TRUE;	
	m_bInitialWeaponRequested	= TRUE;

		// Primary must be attached
	if (m_apWeapon[0])
	{
		m_apWeapon[0]->Attach_UnitMtxToParent_PS( this, m_apszBoneNameTable[BONE_ATTACHPOINT_PRIMARY] );
		// Make primary weapon pass along damage to upper-right arm...
		m_apWeapon[0]->SetBotDamageBoneIndex( m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_RIGHT_ARM_UPPER] ) );
	}


	SetMaxHealth();

	_WeaponOrUpgradeLevelMayHaveChanged();

	if( m_nPossessionPlayerIndex >= 0 ) {
		Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.SetNormOrigin( 0.0f, 0.23f );
	}

	// Find approx eye point...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CBotMiner::ClassHierarchyBuild(): Could not locate approx eye point bone '%s'.\n", m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
		m_pApproxEyePoint_WS = &m_MtxToWorld.m_vPos;
	} else {
		m_pApproxEyePoint_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
	}

	// Find gaze direction...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_HEAD ] );
	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CBotMiner::ClassHierarchyBuild(): Could not locate gaze bone '%s'.\n", m_apszBoneNameTable[ BONE_HEAD ] );
		m_pGazeDir_WS = &m_MtxToWorld.m_vFront;
	} else {
		m_pGazeDir_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vFront;
	}
	
	m_pAISteerMtx = &m_MtxToWorld; //findfix: remove this code if botMiner ever gets a torso matrix that isn't inverted. (Ai controlled ones will aim and look at things better)

	// Initialize matrix palette (this is important for attached entities)...
	AtRestMatrixPalette();

	// Create tag points...
	if( !TagPoint_CreateFromBoneArray( m_anTagPointBoneNameIndexArray, m_apszBoneNameTable ) ) {
		goto _ExitWithError;
	}
	
	//if (!fclib_strnicmp(_pszMinerMeshFilenameRoot,  pszMeshFilename, fclib_strlen(_pszMinerMeshFilenameRoot)))
	//{
	//	if( !m_pPartMgr->Create( this, &m_pPartPool, _BOTPART_FILENAME, PART_INSTANCE_COUNT_PER_TYPE, LIMB_TYPE_COUNT ) ) {
	//		FASSERT(!m_pPartMgr->IsCreated());
	//	//	goto _ExitWithError;
	//	}
	//}

	// Set up data port stuff...
//	DataPort_SetupTetherShockInfo();

	SetBotFlag_Enemy();

	nLeftHandSecondaryFire = m_pWorldMesh->FindBone( "Secondary_Fire" );

	if( nLeftHandSecondaryFire < 0 ) {
		DEVPRINTF( "CBotMiner::ClassHierarchyBuild: Bone '%s' doesn't exist in object '%s'.\n", "L_Hand", Name() );
		m_pLeftHandSecondaryFireBonePos = &m_MtxToWorld.m_vPos;
	} else {
		m_pLeftHandSecondaryFireBonePos = &m_pWorldMesh->GetBoneMtxPalette()[nLeftHandSecondaryFire]->m_vPos;
	}

	m_pSpotLight = m_pWorldMesh->GetAttachedLightByID( _HEADLIGHT_TEXTURE_ID );

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


BOOL CBotMiner::ClassHierarchyBuilt( void ) {
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


CEntityBuilder *CBotMiner::GetLeafClassBuilder( void ) {
	return &_BotMinerBuilder;
}


void CBotMiner::_ClearDataMembers( void ) {
	// Init data members...

	m_pMoveIdentifier = &m_nMinerFlags;

	m_fCollCylinderRadius_WS = 2.0f;
	m_fCollCylinderHeight_WS = 4.0f;

	m_pBotInfo_Gen = &m_BotInfo_Gen;
	m_pBotInfo_MountAim = &m_BotInfo_MountAim;
	m_pBotInfo_Walk = &m_BotInfo_Walk;
	m_pBotInfo_Jump = &m_BotInfo_Jump;
	m_pBotInfo_Weapon = &m_BotInfo_Weapon;

	m_fGravity = m_pBotInfo_Gen->fGravity;
	m_fMaxFlatSurfaceSpeed_WS = m_pBotInfo_Walk->fMaxXlatVelocity * m_fRunMultiplier;

	m_fMountPitchMax_WS = m_pBotInfo_MountAim->fMountPitchDownLimit;
	m_fMountPitchMin_WS = m_pBotInfo_MountAim->fMountPitchUpLimit;

	m_nMinerFlags = MINERFLAG_NONE;

	m_anAnimStackIndex[ASI_STAND] = ANIMTAP_STAND;
	m_anAnimStackIndex[ASI_STAND_ALERT] = -1; //ANIMTAP_STAND_ALERT;

	m_anAnimStackIndex[ASI_WALK] = ANIMTAP_WALK;
	m_anAnimStackIndex[ASI_WALK_ALERT] = -1; //ANIMTAP_WALK_ALERT;
	m_anAnimStackIndex[ASI_RUN] = ANIMTAP_RUN;
	m_anAnimStackIndex[ASI_RUN_PANIC] = -1; //ANIMTAP_RUN_PANIC;

	m_anAnimStackIndex[ASI_FALL] = ANIMCONTROL_JUMP_FLY;

	m_anAnimStackIndex[ASI_HOP_LEFT] = -1; //ANIMTAP_HOP_LEFT;
	m_anAnimStackIndex[ASI_HOP_RIGHT] = -1; //ANIMTAP_HOP_RIGHT;
	m_anAnimStackIndex[ASI_STARTLE] = -1; //ANIMTAP_STARTLE;
	m_anAnimStackIndex[ASI_ROLL_LEFT] = -1; //ANIMTAP_ROLL_LEFT;
	m_anAnimStackIndex[ASI_ROLL_RIGHT] = -1; //ANIMTAP_ROLL_RIGHT;

	m_anAnimStackIndex[ASI_DOZE_LOOP] = -1; //ANIMTAP_DOZE_LOOP;
	m_anAnimStackIndex[ASI_NAPJERK] = -1; //ANIMTAP_NAPJERK;
	m_anAnimStackIndex[ASI_WAKE] = -1; //ANIMTAP_WAKE;

	m_anAnimStackIndex[ASI_RELOAD_CLIP_EJECT_OLD] = ANIMCONTROL_RELOAD_CLIP_EJECT_OLD;
	m_anAnimStackIndex[ASI_RELOAD_CLIP_GRAB_NEW] = ANIMCONTROL_RELOAD_CLIP_GRAB_NEW;
	m_anAnimStackIndex[ASI_RELOAD_CLIP_INSERT_NEW] = ANIMCONTROL_RELOAD_CLIP_INSERT_NEW;
	m_anAnimStackIndex[ASI_RELOAD_CLIP_SLAPIN_NEW] = ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW;
	
	m_anAnimStackIndex[ASI_RELOAD_WITH_LEFT_GRAB_AMMO]	= ANIMTAP_RELOAD_ROCKET1_GRAB_AMMO;
	m_anAnimStackIndex[ASI_RELOAD_WITH_LEFT_DROP_IN]	= ANIMTAP_RELOAD_ROCKET1_DROP_IN;
	m_anAnimStackIndex[ASI_RELOAD_WITH_LEFT_FINISH]		= ANIMTAP_RELOAD_ROCKET1_FINISH;
//	m_anAnimStackIndex[ASI_RELOAD_SINGLE_ARM] = ANIMTAP_RELOAD_SINGLE_ARM;
//	m_anAnimStackIndex[ASI_RELOAD_SINGLE_BODY] = ANIMTAP_RELOAD_SINGLE_BODY;

	m_anAnimStackIndex[ASI_SNEAK] = -1; //ANIMTAP_TURN_IN_PLACE;

	m_anAnimStackIndex[ASI_RC_TETHERED]		= -1; //ANIMTAP_RC_TETHERED;
	m_anAnimStackIndex[ASI_RC_POWER_DOWN]	= ANIMTAP_RC_POWER_DOWN;
	m_anAnimStackIndex[ASI_RC_POWER_UP]		= ANIMTAP_RC_POWER_UP;
	m_anAnimStackIndex[ASI_STOOP]			= -1;

	m_anAnimStackIndex[ASI_AIM_PILLBOX]		= ANIMTAP_AIM_PILLBOX;

	m_pnEnableBoneNameIndexTableForSummer_Normal = m_anEnableBoneNameIndexTableForSummer_Normal;
	m_pnEnableBoneNameIndexTableForSummer_TetherShock = m_anEnableBoneNameIndexTableForSummer_TetherShock;

	// shouldn't be hard-coded...
	// it's not anymore
	//m_pBotInfo_Jump->fFlipOriginX = 0.0f;
	//m_pBotInfo_Jump->fFlipOriginY = 3.8f;
	//m_pBotInfo_Jump->fFlipOriginZ = 0.47f;
	
}


void CBotMiner::ClassHierarchyAddToWorld( void ) {
//	u32 i;

	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CBot::ClassHierarchyAddToWorld();
	m_pWorldMesh->UpdateTracker();

	m_uBotDeathFlags |= BOTDEATHFLAG_PLAYDEATHANIM | BOTDEATHFLAG_COMEAPART | BOTDEATHFLAG_AUTOPERSISTAFTERDEATH;
	
	if( m_apWeapon[0] ) {
		m_apWeapon[0]->Attach_UnitMtxToParent_PS( this, m_apszBoneNameTable[BONE_ATTACHPOINT_PRIMARY] );
		m_apWeapon[0]->AddToWorld();
		m_apWeapon[0]->ResetToState( CWeapon::STATE_DEPLOYED );
	}

	//if( m_apWeapon[1] ) {
	//	m_apWeapon[1]->RemoveFromWorld();
	//	m_apWeapon[2]->ResetToState( CWeapon::STATE_STOWED );
	//}

	/*for( i=0; i<2; i++ ) {
		if( m_apWeapon[i] ) {
			m_apWeapon[i]->Attach_UnitMtxToParent_PS( this, m_apszBoneNameTable[BONE_ATTACHPOINT_PRIMARY] );
			m_apWeapon[i]->AddToWorld();
			m_apWeapon[i]->ResetToState( CWeapon::STATE_DEPLOYED );
		}
	}*/

}


void CBotMiner::ClassHierarchyRemoveFromWorld( void ) {
	u32 i;

	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	for( i=0; i<2; i++ ) {
		if( m_apWeapon[i] ) {
			if ( m_apWeapon[i]->IsInWorld() ) {
				m_apWeapon[i]->RemoveFromWorld();
			}
		}
	}

	FreeAllParticleEmitterLocks(this);
	m_nMinerFlags &=~MINERFLAG_TERMINAL_STATE_ON;

	m_pWorldMesh->RemoveFromWorld();
	CBot::ClassHierarchyRemoveFromWorld();
}


void CBotMiner::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
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


const CFVec3A *CBotMiner::GetApproxEyePoint( void ) const {
	FASSERT( IsCreated() );

	return m_pApproxEyePoint_WS;
}


void CBotMiner::ClassHierarchyWork() {
	CFVec3A TempVec3A;

	FASSERT( m_bSystemInitialized );

	CBot::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}

	m_fCableGrabTime -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN( m_fCableGrabTime, 0.0f );	

	UpdateSpotLight();
	Power_Work();
	DataPort_Work();

	if( DataPort_IsBeingShocked() ) {
		_UpdateMatrices();
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
			HandleSleeping();
		PROTRACK_ENDBLOCK();//"GroundAnim");
		break;

	case STATE_AIR:
		PROTRACK_BEGINBLOCK("AirXlat");
			HandleAirTranslation();
			_HandleCableTranslation();
			_HandleCableAndDoubleJump();
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
		_HandleCableAnimations();
		HandleHipsAnimation();
	PROTRACK_ENDBLOCK();//"CommonAnim");

	PROTRACK_BEGINBLOCK("Weapon");
	if (m_apWeapon[0])
	{
		HandleTargeting();
		_HandleWeaponFiring();
		_HandleWeaponAnimations();
	}
	PROTRACK_ENDBLOCK();//"WeaponAnim");

	_UpdateMatrices();

	PROTRACK_BEGINBLOCK("WeaponWork");
		_HandleWeaponWork();
		_HandleWeaponSwitching();
	PROTRACK_ENDBLOCK();//"WeaponWork");

	// Call the work function for our botfx system...
	//_HandleParticles();

	if( m_pCableHook && m_pCableHook->m_bAttached ) {
		m_pCableHook->m_Pos_Draw_WS = *m_pLeftHandSecondaryFireBonePos;
	}

	if (IsDeadOrDying()) {
		DeathWork();
	}
}


BOOL CBotMiner::GrabCable( CEZipLine *pCable ) {
	if( pCable == m_pCablePrev && m_fCableGrabTime != 0.0f ) {
		return FALSE;
	}

	if( m_pPartMgr && 
		m_pPartMgr->IsCreated() &&
		m_pPartMgr->GetLimbState( LIMB_TYPE_LEFT_ARM ) != CBotPartMgr::LIMB_STATE_INTACT ) {
		return FALSE;
	}

	m_pCableHook = pCable->AttachHook( &m_MountPos_WS, &m_Velocity_WS, this );

	if( m_pCableHook == NULL ) {
		return FALSE;
	}

	m_MountPos_WS.y += 1.0f;
	m_Velocity_WS.y += m_pBotInfo_Jump->fVerticalVelocityJump1;
	m_Velocity_MS.y += m_pBotInfo_Jump->fVerticalVelocityJump1;

	m_nJumpState = BOTJUMPSTATE_CABLE;
	SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );
	SetControlValue( ANIMCONTROL_JUMP_TUCK, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_UNTUCK, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_CABLE_GRASP, 1.0f );
	UpdateTime( ANIMCONTROL_CABLE_GRASP, 1.0f );

	SetJumping();

	// Update the hook hand brake value
	m_pCableHook->m_fInputBrakeAmount = m_fControls_Fire2;

	// Don't let the hand spin
	Summer_UpdateBoneMask( m_apszBoneNameTable[BONE_LEFT_HAND], TRUE );
	
	f32 fDot = ( m_MountUnitFrontXZ_WS.x * m_pCableHook->m_pCable->m_UnitVec12XZ.x ) + 
				( m_MountUnitFrontXZ_WS.z * m_pCableHook->m_pCable->m_UnitVec12XZ.y) ;
	f32 fZipYaw = fmath_Atan( m_pCableHook->m_pCable->m_UnitVec12XZ.y, m_pCableHook->m_pCable->m_UnitVec12XZ.x );

	if( fDot >= 0.0f ) {
		m_pCableHook->m_fYawSnapOffset = fZipYaw;
	} else {
		m_pCableHook->m_fYawSnapOffset = fZipYaw - FMATH_PI;
	}

	CFMtx43A::m_RotZ.SetRotationZ( -m_fMountYaw_WS + m_pCableHook->m_fYawSnapOffset - FMATH_HALF_PI );
	*m_Anim.m_pAnimManMtx->GetFrameAndFlagAsChanged( BONE_LEFT_HAND ) = CFMtx43A::m_RotZ;

//	ZipLineVerletSetup( );

	return TRUE;
}

void CBotMiner::ReleaseCable( void ) {
	SetControlValue( ANIMCONTROL_CABLE_GRASP, 0.0f );

	m_pCableHook->m_pCable->DetachHook( m_pCableHook, this );
	m_pCablePrev = m_pCableHook->m_pCable;
	m_fCableGrabTime = BOT_ZIPLINE_GRAB_TIME;
	m_pCableHook = NULL;

	*m_Anim.m_pAnimManMtx->GetFrameAndFlagAsChanged( BONE_LEFT_HAND ) = CFMtx43A::m_IdentityMtx;
	Summer_UpdateBoneMask( m_apszBoneNameTable[BONE_LEFT_HAND], FALSE );
}

void CBotMiner::_HandleCableAndDoubleJump( void ) {
	if( m_nJumpState != BOTJUMPSTATE_CABLE ) {
		if( m_bControls_Jump && ((m_nBotFlags2 & BOTFLAG2_CAN_DOUBLE_JUMP) || (m_Velocity_WS.y >= 0.0f)) ) {
			if( m_nJumpState==BOTJUMPSTATE_LAUNCH || m_nJumpState==BOTJUMPSTATE_AIR ) {
				if( m_nBotFlags2 & BOTFLAG2_CAN_DOUBLE_JUMP ) {
					// Flip and boost...
					_StartDoubleJump( TRUE );
				} else {
					// Just flip...
					_StartDoubleJump( FALSE );
				}
			}
		}
	}
}


void CBotMiner::_HandleCableTranslation( void ) {
	if( m_nJumpState == BOTJUMPSTATE_CABLE ) {
		if( m_pCableHook->m_bAttached ) {
			// Update the hook hand brake value
			m_pCableHook->m_fInputBrakeAmount = m_fControls_Fire2;

			m_Velocity_MS = m_pCableHook->m_Vel_WS;

			CFVec3A Pos_WS_Diff;

			Pos_WS_Diff.Sub( m_pCableHook->m_Pos_WS, *m_pLeftHandSecondaryFireBonePos );
			m_MountPos_WS.Add( Pos_WS_Diff );

			// Don't spin the hand
			CFMtx43A::m_RotZ.SetRotationZ( -m_fMountYaw_WS + m_pCableHook->m_fYawSnapOffset - FMATH_HALF_PI );
			*m_Anim.m_pAnimManMtx->GetFrameAndFlagAsChanged( BONE_LEFT_HAND ) = CFMtx43A::m_RotZ;

			m_Velocity_WS = m_pCableHook->m_Vel_WS;

			// Update velocities...
			WS2MS( m_Velocity_MS, m_Velocity_WS );
			VelocityHasChanged();

			if( m_bControlsBot_Jump2 ) {
				_StartSingleJump();
				_StartDoubleJump();

				ReleaseCable();
			} else if( m_bControls_Jump ) {
				_StartSingleJump();

				// Push off the cable in the direction we are holding
				CFVec3A Dir;

				Dir = m_XlatStickUnitVecXZ_WS;
				Dir.Mul(-10.0f);

				ApplyVelocityImpulse_MS(Dir);
				
				ReleaseCable();
			}
		} else {
			_StartSingleJump();
			ReleaseCable();
		}
	}
}


void CBotMiner::_HandleCableAnimations( void ) {
	if( m_fUnitCableBlend>0.0f && m_nJumpState!=BOTJUMPSTATE_CABLE ) {
		m_fUnitCableBlend -= FLoop_fPreviousLoopSecs * m_fTuckUntuckAnimTimeMult;	// SER Change this!
		FMATH_CLAMPMIN( m_fUnitCableBlend, 0.0f );
		SetControlValue( ANIMCONTROL_CABLE_GRASP, m_fUnitCableBlend );
	}
}


void CBotMiner::_HandleJumping( void ) {
	if( !m_bControlsBot_JumpVec ) {
		if( m_bControlsBot_Jump2 ) {
			_StartSingleJump();
			_StartDoubleJump();
		} else if( m_bControls_Jump ) {
			_StartSingleJump();

			FMATH_SETBITMASK( m_nBotFlags2, BOTFLAG2_CAN_DOUBLE_JUMP );
		}
	} else {
		// Velocity jump specified...
		_StartVelocityJump( &m_ControlsBot_JumpVelocity_WS );
	}

	if( m_nPrevState == STATE_AIR ) {
		if( m_nJumpState != BOTJUMPSTATE_NONE ) {
			_JumpLanded();
		}

		// If we hit ground while on a cable, detach
		if( m_pCableHook ) {
			ReleaseCable();
		}
	}
}


void CBotMiner::_EnterFlyMode( void ) {
	m_bPlayLandAnim = FALSE;
	m_bPlayLandAnimLower = FALSE;

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );
	SetControlValue( ANIMCONTROL_JUMP_TUCK, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_UNTUCK, 0.0f );

	m_nJumpState = BOTJUMPSTATE_AIR;
	SetJumping();
}


void CBotMiner::_StartVelocityJump( const CFVec3A *pJumpVelocity_WS ) {
	CFVec3A JumpVelocity_WS = *pJumpVelocity_WS;
	FMATH_CLAMP( JumpVelocity_WS.y, 0.0f, m_fMaxDeltaJumpVelY_WS );

	_StartSingleJump( &JumpVelocity_WS );

	if( pJumpVelocity_WS->y >= m_pBotInfo_Jump->fVerticalVelocityJump1 ) {
		_StartDoubleJump( FALSE );
	}
}


void CBotMiner::_StartSingleJump( const CFVec3A *pJumpVelocity_WS ) {
	if( pJumpVelocity_WS == NULL ) {
		m_Velocity_WS.y += m_pBotInfo_Jump->fVerticalVelocityJump1 * m_fJumpMultiplier;
		m_Velocity_MS.y += m_pBotInfo_Jump->fVerticalVelocityJump1 * m_fJumpMultiplier;
	} else {
		m_Velocity_WS.x = pJumpVelocity_WS->x;
		m_Velocity_WS.y = pJumpVelocity_WS->y;
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
	SetControlValue( ANIMCONTROL_JUMP_TUCK, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_UNTUCK, 0.0f );


	ZeroTime( ANIMTAP_JUMP_LAUNCH );

	m_fUnitJump1Jump2Blend = 0.0f;
	m_nJumpState = BOTJUMPSTATE_LAUNCH;
	SetJumping();
	SetDoubleJumpTimer( m_pBotInfo_Gen->fDoubleJumpTime );
}

void CBotMiner::_StartDoubleJump( BOOL bAddToVelocityY ) {
	if( !HaveTimeToDoubleJump() ) {
		return;
	}

	StartDoubleJumpGeneric(bAddToVelocityY,
							ANIMTAP_JUMP_TUCK,
							ANIMTAP_JUMP_UNTUCK,
							ANIMCONTROL_JUMP_LAUNCH,
							ANIMCONTROL_JUMP_FLY,
							ANIMCONTROL_JUMP_TUCK);
}


void CBotMiner::_JumpLanded( void ) {
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
	SetControlValue( ANIMCONTROL_JUMP_TUCK, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_UNTUCK, 0.0f );


	if ( m_nPossessionPlayerIndex >= 0 )	{
		// Don't play scuff sounds when landing from a jump
		if( m_pBotInfo_Sound && m_nSurfaceTypeOn != SURFACE_TYPE_NONE) {
			fsndfx_Play2D(
				m_pBotInfo_Sound->ahStepSound[m_nSurfaceTypeOn][ fmath_RandomRange( 0, ( BOTSOUND_SCUFF_SOUND_START - 1 ) ) ],
				m_pBotInfo_Walk->fJumpLandedSoundUnitVolume_2D * m_fStepVolumeScale
			);
		}
		//tell the AI that this sound happened
		AIEnviro_BoostPlayerSoundTo(m_nPossessionPlayerIndex, 35.0f);
	} else {
		if( m_pBotInfo_Sound && m_nSurfaceTypeOn != SURFACE_TYPE_NONE) {
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


void CBotMiner::_HandleJumpAnimations( void ) {
	f32 fTemp, fUnitTime;

	if( m_nState == STATE_GROUND ) {
		// We're on the ground...
		if( m_fUnitFlyBlend > 0.0f ) {
			m_fUnitFlyBlend = 0.0f;
			_JumpLanded();
			FMATH_CLEARBITMASK( m_nBotFlags2, BOTFLAG2_CAN_DOUBLE_JUMP );
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
				SetControlValue( ANIMCONTROL_JUMP_TUCK, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_UNTUCK, 0.0f );

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
			if( m_nJumpState==BOTJUMPSTATE_TUCK || m_nJumpState==BOTJUMPSTATE_FLIPPING || m_nJumpState==BOTJUMPSTATE_UNTUCK ) {
				m_fFlipPitch += m_fFlipPitchVelocity * FLoop_fPreviousLoopSecs;
			}

			switch( m_nJumpState ) {
			case BOTJUMPSTATE_LAUNCH:
				if( !DeltaTime( ANIMTAP_JUMP_LAUNCH ) ) {
					fUnitTime = 3.0f * fmath_UnitLinearToSCurve( GetUnitTime( ANIMTAP_JUMP_LAUNCH ) );
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
				DeltaTime( ANIMTAP_JUMP_FLY );

				m_fUnitJump1Jump2Blend -= m_pBotInfo_Jump->fUntuckToFlyBlendSpeed * FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMIN( m_fUnitJump1Jump2Blend, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_UNTUCK, m_fUnitJump1Jump2Blend );
				
				if( CanAttachToZipLine() ) {
					CFSphere Sphere;

					//Sphere.m_Pos = m_MountPos_WS.v3;
					Sphere.m_Pos = (*m_pLeftHandSecondaryFireBonePos).v3;
                   	Sphere.m_fRadius = FMATH_MAX(m_fCollCylinderHeight_WS, m_fCollCylinderRadius_WS);

					CEZipLine *pZipHit = CEZipLine::GetZipCollision( Sphere );

					if( pZipHit ) {
						GrabCable( pZipHit );
					}
				}

				break;

			case BOTJUMPSTATE_TUCK:
				if( DeltaTime( ANIMTAP_JUMP_TUCK, FLoop_fPreviousLoopSecs * m_fTuckUntuckAnimTimeMult, TRUE ) ) {
					// Tuck animation completed...

					if( m_fUntuckStartCountdownSecs > 0.0f ) {
						m_nJumpState = BOTJUMPSTATE_FLIPPING;
						SetJumping();
					} else {
						// Time to start untuck animation...
						SetControlValue( ANIMCONTROL_JUMP_TUCK, 0.0f );
						SetControlValue( ANIMCONTROL_JUMP_UNTUCK, 1.0f );
						ZeroTime( ANIMTAP_JUMP_UNTUCK );
						m_nJumpState = BOTJUMPSTATE_UNTUCK;
						SetJumping();
					}
				}

				break;

			case BOTJUMPSTATE_FLIPPING:
				m_fUntuckStartCountdownSecs -= FLoop_fPreviousLoopSecs;

				if( m_fUntuckStartCountdownSecs <= 0.0f ) {
					// Time to start untuck animation...
					SetControlValue( ANIMCONTROL_JUMP_TUCK, 0.0f );
					SetControlValue( ANIMCONTROL_JUMP_UNTUCK, 1.0f );
					ZeroTime( ANIMTAP_JUMP_UNTUCK );
					m_nJumpState = BOTJUMPSTATE_UNTUCK;
					SetJumping();
				}

				break;

			case BOTJUMPSTATE_UNTUCK:
				if( DeltaTime( ANIMTAP_JUMP_UNTUCK, FLoop_fPreviousLoopSecs * m_fTuckUntuckAnimTimeMult, TRUE ) ) {
					// Untuck animation completed...
					SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );
					ZeroTime( ANIMTAP_JUMP_FLY );
					m_fFlipPitch = 0.0f;
					m_fUnitJump1Jump2Blend = 1.0f;
					m_nJumpState = BOTJUMPSTATE_AIR;
					SetJumping();
				}
				break;
///////////////////////////////////////
///////////////////////////////////////

			case BOTJUMPSTATE_CABLE:
				DeltaTime( ANIMTAP_JUMP_FLY );
				DeltaTime( ANIMTAP_CABLE_GRASP, FLoop_fPreviousLoopSecs, TRUE );

				if( m_fUnitCableBlend < 1.0f ) {
					m_fUnitCableBlend = 1.0f;
					SetControlValue( ANIMCONTROL_CABLE_GRASP, 1.0f );
				}

				break;

			default:
				FASSERT_NOW;
			}
		}
	}
}


void CBotMiner::_HandleWeaponWork( void ) {
	if( m_apWeapon[0] ) {
		m_apWeapon[0]->Work();
	}
	if( m_apWeapon[1] ) {
		m_apWeapon[1]->Work();
	}
}


// Called by the weapon when it's EUK upgrade level has changed...
void CBotMiner::NotifyWeaponUpgraded( CWeapon *pUpgradedWeapon ) {
	if( pUpgradedWeapon != m_apWeapon[0] ) {
		// Not our current weapon...
		return;
	}

	// The EUK upgrade level of our currently-selected primary weapon
	// has changed...

	_WeaponOrUpgradeLevelMayHaveChanged();
}


void CBotMiner::UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup ) {
	static const u8 *__apnUserAnimBoneMaskIndex[UABONEMASK_COUNT] = {
		m_aBoneEnableIndices_UserAnim_UpperBody,
		m_aBoneEnableIndices_UserAnim_LowerBody,
		m_aBoneEnableIndices_UserAnim_UpperTorso,
		m_aBoneEnableIndices_UserAnim_LowerTorso,
		m_aBoneEnableIndices_UserAnim_LeftArm,
		m_aBoneEnableIndices_UserAnim_RightArm,
		m_aBoneEnableIndices_UserAnim_Head
	};

	FASSERT( IsCreated() );
	FASSERT( nBoneMaskGroup>=0 && nBoneMaskGroup<UABONEMASK_COUNT );

	CBot::UserAnim_BatchUpdateTapBoneMask( __apnUserAnimBoneMaskIndex[nBoneMaskGroup], m_apszBoneNameTable );
}


void CBotMiner::_HandleFiringAnimations( void ) {
	f32 fUnitFireControlValue;
	u32 uFireAnimControlId = ANIMCONTROL_FIRE_1;
	u32 uFireAnimTapId = ANIMTAP_FIRE_1;

	// Primary fire...
	if( m_apWeapon[0]->m_pInfo->nStanceType == CWeapon::STANCE_TYPE_TWO_HANDED_TORSO_TWIST ) {
		uFireAnimControlId = ANIMCONTROL_FIRE_ROCKET1;
		uFireAnimTapId = ANIMTAP_FIRE_ROCKET1;
	} else {
		uFireAnimControlId = ANIMCONTROL_FIRE_1;
		uFireAnimTapId = ANIMTAP_FIRE_1;
	}
	
	//if (IsAlertOn()) {
	//	uFireAnimControlId = ANIMCONTROL_FIRE_1_ALERT;
	//	uFireAnimTapId = ANIMTAP_FIRE_1_ALERT;
	//	SetControlValue( ANIMCONTROL_FIRE_1, 0.0f);
	//}
	//else {
	//	SetControlValue( ANIMCONTROL_FIRE_1_ALERT, 0.0f);
	//}

	if( m_nBotFlags & BOTFLAG_PLAY_FIRE1_ANIM ) {
		fUnitFireControlValue = GetControlValue( uFireAnimControlId );

		if( fUnitFireControlValue < 1.0f ) {
			fUnitFireControlValue += m_pBotInfo_Weapon->fFire1BlendInSpeed * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMAX( fUnitFireControlValue, 1.0f );
			SetControlValue( uFireAnimControlId, fUnitFireControlValue );
		}

		if( DeltaTime( uFireAnimTapId ), FLoop_fPreviousLoopSecs, TRUE ) {
			FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM );
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

		if( DeltaTime( ANIMTAP_FIRE_2_UPPER, 2.0f * FLoop_fPreviousLoopSecs, TRUE ) ) {
			fUnitTime = 1.0f;
			fUnitFireControlValue = 0.0f;
		} else {
			fUnitTime = GetUnitTime( ANIMTAP_FIRE_2_UPPER );

			if( fUnitTime < m_pBotInfo_Weapon->fThrowBlendInUnitTime ) {
				fUnitFireControlValue = fUnitTime * m_pBotInfo_Weapon->fOOThrowBlendInUnitTime;
			} else if( fUnitTime > m_pBotInfo_Weapon->fThrowBlendOutUnitTime ) {
				fUnitFireControlValue = 1.01f - (fUnitTime - m_pBotInfo_Weapon->fThrowBlendOutUnitTime) * m_pBotInfo_Weapon->fOOInvThrowBlendOutUnitTime;
			} else {
				fUnitFireControlValue = 1.0f;
			}
		}

		SetControlValue( ANIMCONTROL_FIRE_2_UPPER, fUnitFireControlValue );

		switch( m_nThrowState ) {
		case THROWSTATE_REACHING_FOR_AMMO:
			if( fUnitTime <= 0.25f ) {
				break;
			}

			// Attach ammo...
			m_apWeapon[1]->Throwable_AttachGrenadeToOwnerBotBone( "Secondary_Fire" );
			m_nThrowState = THROWSTATE_WINDING_UP;

			// Fall through to next state...

		case THROWSTATE_WINDING_UP:
			if( fUnitTime <= 0.63f ) {
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


void CBotMiner::_HandleAimAnimations( void ) {
	f32 fUnitAimPitch, fCtl;
	CFAnimFrame TorsoQuat, HeadQuat;

	if( m_apWeapon[0]->m_pInfo->nStanceType == CWeapon::STANCE_TYPE_TWO_HANDED_TORSO_TWIST ) {
		fCtl = GetControlValue( ANIMCONTROL_AIM_ROCKET1 );
		if( fCtl < 1.0f ) {
			fCtl += FLoop_fPreviousLoopSecs;
			FMATH_CLAMP_MAX1( fCtl );
			SetControlValue( ANIMCONTROL_AIM_ROCKET1, fCtl );
		}
	} else {
		fCtl = GetControlValue( ANIMCONTROL_AIM_ROCKET1 );
		if( fCtl > 0.0f ) {
			fCtl -= FLoop_fPreviousLoopSecs;
			FMATH_CLAMP_MIN0( fCtl );
			SetControlValue( ANIMCONTROL_AIM_ROCKET1, fCtl );
		}
	}


#if 0
	if( m_apWeapon[0]->Type() != CWeapon::WEAPON_TYPE_ROCKET_LAUNCHER ) {
		// Not a rocket launcher. Make sure that we're not using
		// either rocket launcher aim anims...

		fControlVal = GetControlValue( ANIMCONTROL_AIM_ROCKET1 );
		if( fControlVal > 0.0f ) {
			fControlVal -= 2.0f * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( fControlVal, 0.0f );
			SetControlValue( ANIMCONTROL_AIM_ROCKET1, fControlVal );
		}

		fControlVal = GetControlValue( ANIMCONTROL_AIM_ROCKET23 );
		if( fControlVal > 0.0f ) {
			fControlVal -= 2.0f * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( fControlVal, 0.0f );
			SetControlValue( ANIMCONTROL_AIM_ROCKET23, fControlVal );
		}
	}
#endif

	// Compute the amount we're aiming up/down (up=1, down=0, middle=0.5)...
	fUnitAimPitch = fmath_Div( m_fAimPitch_WS - m_fMountPitchMin_WS, m_fMountPitchMax_WS - m_fMountPitchMin_WS );
	FMATH_CLAMP_UNIT_FLOAT( fUnitAimPitch );

	// Update the torso-twist blend value...
	if( m_apWeapon[0]->Type() != CWeapon::WEAPON_TYPE_ROCKET_LAUNCHER ) {
		// Not a rocket launcher...

		if( m_fUnitTorsoTwistBlend > 0.0f ) {
			m_fUnitTorsoTwistBlend -= 2.0f * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fUnitTorsoTwistBlend, 0.0f );
		}
	} else {
		// L1 rocket launcher...

		UpdateUnitTime( ANIMTAP_AIM_ROCKET1, FMATH_FPOT( fUnitAimPitch, 0.1f, 0.9f ) );

		if( m_fUnitTorsoTwistBlend < 1.0f ) {
			m_fUnitTorsoTwistBlend += 2.0f * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMAX( m_fUnitTorsoTwistBlend, 1.0f );
		}
#if 0
		if( m_apWeapon[0]->GetUpgradeLevel() != 0 ) {
			// L2 or L3 rocket launcher...

			UpdateUnitTime( ANIMTAP_AIM_ROCKET23, FMATH_FPOT( fUnitAimPitch, 0.1f, 0.9f ) );

			if( m_fUnitTorsoTwistBlend > 0.0f ) {
				m_fUnitTorsoTwistBlend -= 2.0f * FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMIN( m_fUnitTorsoTwistBlend, 0.0f );
			}
		} else {
			// L1 rocket launcher...

			UpdateUnitTime( ANIMTAP_AIM_ROCKET1, FMATH_FPOT( fUnitAimPitch, 0.1f, 0.9f ) );

			if( m_fUnitTorsoTwistBlend < 1.0f ) {
				m_fUnitTorsoTwistBlend += 2.0f * FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMAX( m_fUnitTorsoTwistBlend, 1.0f );
			}
		}
#endif
	}

	// Compute the head and torso quaternions...
	TorsoQuat.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( fUnitAimPitch, FMATH_DEG2RAD(30.0f), FMATH_DEG2RAD(-30.0f) ), CFVec3A::m_Null );

	if( m_fUnitTorsoTwistBlend == 0.0f ) {
		// No torso twist...
		// Torso quaternion has already been computed.
		// Compute head quaternion...

		HeadQuat.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( fUnitAimPitch, FMATH_DEG2RAD(20.0f), FMATH_DEG2RAD(-20.0f) ), CFVec3A::m_Null );

	} else {
		// We must introduce some rocket-launcher-L1 torso twist...

		if( m_fUnitTorsoTwistBlend == 1.0f ) {
			// 100% torso twist...

			// Compute the torso twist quaternion...
			TorsoQuat.Mul( m_Rocket1TorsoTwistQuat );

			// Compute the head quaternion...
			CFAnimFrame TempQuat;
			TempQuat.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( fUnitAimPitch, FMATH_DEG2RAD(20.0f), FMATH_DEG2RAD(-20.0f) ), CFVec3A::m_Null );
			HeadQuat.Mul( TempQuat, m_Rocket1TorsoUntwistQuat );
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


void CBotMiner::_HandleWeaponAnimations( void ) {
	HandleWeaponReloadingAnimations();
	_HandleFiringAnimations();
	_HandleAimAnimations();
}


void CBotMiner::_WeaponOrUpgradeLevelMayHaveChanged( void ) {
#if 0
	if( m_nWRState != WR_STATE_NONE ) {
		// Snap out of any weapon reloading animation...

		m_nWRState = WR_STATE_NONE;
		SetControlValue( ANIMCONTROL_RELOAD_ROCKET1_GRAB_AMMO, 0.0f );
		SetControlValue( ANIMCONTROL_RELOAD_ROCKET1_DROP_IN, 0.0f );
		SetControlValue( ANIMCONTROL_RELOAD_ROCKET1_FINISH, 0.0f );
		SetControlValue( ANIMCONTROL_RELOAD_SINGLE_ARM, 0.0f );
		SetControlValue( ANIMCONTROL_RELOAD_SINGLE_BODY, 0.0f );
	}

	if( m_apWeapon[0]->Type() == CWeapon::WEAPON_TYPE_ROCKET_LAUNCHER ) {
		// Rocket launcher...

		if( m_apWeapon[0]->GetUpgradeLevel() != 0 ) {
			// L2 or L3 rocket launcher...

			if( GetAttachedAnim( ANIMTAP_FIRE_1 ) != &m_Anim.m_pLoadedBaseAnimInst[ ANIM_FIRE_ROCKET23 ] ) {
				AttachAnim( ANIMTAP_FIRE_1, &m_Anim.m_pLoadedBaseAnimInst[ ANIM_FIRE_ROCKET23 ] );
				UpdateBoneMask( ANIMTAP_FIRE_1, m_aBoneEnableIndices_FireRocket23, TRUE );
			}
		} else {
			// L1 rocket launcher...

			if( GetAttachedAnim( ANIMTAP_FIRE_1 ) != &m_Anim.m_pLoadedBaseAnimInst[ ANIM_FIRE_ROCKET1 ] ) {
				AttachAnim( ANIMTAP_FIRE_1, &m_Anim.m_pLoadedBaseAnimInst[ ANIM_FIRE_ROCKET1 ] );
				UpdateBoneMask( ANIMTAP_FIRE_1, m_aBoneEnableIndices_FireRocket1, TRUE );
			}
		}
	} else {
		// Not a rocket launcher...

		if( GetAttachedAnim( ANIMTAP_FIRE_1 ) != &m_Anim.m_pLoadedBaseAnimInst[ ANIM_FIRE_PRIMARY ] ) {
			AttachAnim( ANIMTAP_FIRE_1, &m_Anim.m_pLoadedBaseAnimInst[ ANIM_FIRE_PRIMARY ] );
			UpdateBoneMask( ANIMTAP_FIRE_1, m_aBoneEnableIndices_TapFire1, TRUE );
		}
	}
#endif
}


CFMtx43A *CBotMiner::ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName ) {
	s32 nBoneIndex;

	if( pszAttachBoneName == NULL ) {
		// No bone to attach to...
		return NULL;
	}

	nBoneIndex = m_pWorldMesh->FindBone( pszAttachBoneName );

	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CBotMiner::ClassHierarchyAttachChild(): Bone '%s' doesn't exist in object '%s'.\n", pszAttachBoneName, Name() );
		return NULL;
	}

	return m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex];
}


void CBotMiner::_UpdateMatrices( void ) {
	CFMtx43A FlipMtx, MeshMtx, NewMtx, EntityMtxToWorld;

	CFMtx43A::m_XlatRotY.SetRotationY( m_fMountYaw_WS + m_fLegsYaw_MS );
	CFMtx43A::m_XlatRotY.m_vPos = m_MountPos_WS;

	// Update xfm...
	if( m_fFlipPitch ) 
	{
		CFMtx43A::m_XlatRotX.m_vPos.Set( m_pBotInfo_Jump->fFlipOriginX, m_pBotInfo_Jump->fFlipOriginY, m_pBotInfo_Jump->fFlipOriginZ );
//		CFMtx43A::m_XlatRotX.m_vPos.Set( 0.0f, 2.337f, 0.593f );

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
	} 
	else if( m_pCableHook ) // We are on a zipline
	{
		CFMtx43A RotMtx;

		RotMtx.Identity();
		//ZipLineVerletWork( m_pLeftHandSecondaryFireBonePos, &RotMtx );

		// Build the matrix to rotate about our hand position
		RotMtx.m_vPos.Set( m_ZipHandPos_MS );
		CFMtx43A::m_Xlat.m_vPos.ReceiveNegative( RotMtx.m_vPos );
		FlipMtx.Mul( RotMtx, CFMtx43A::m_Xlat );

		// Rotate the offset matrix by our yaw
		CFMtx43A::m_XlatRotY.m_vPos.Zero();
		FlipMtx.Mul( CFMtx43A::m_XlatRotY );

		CFMtx43A::m_Xlat.m_vPos = m_MountPos_WS;
		MeshMtx.Mul( CFMtx43A::m_Xlat, FlipMtx );
		m_pWorldMesh->m_Xfm.BuildFromMtx( MeshMtx );
	} 
	else if ( m_pCurMech && m_pCurMech->m_pBotDef->m_nSubClass != BOTSUBCLASS_SITEWEAPON_PILLBOX ) // driving guy
	{
		m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld );
	}
	else
	{
		m_pWorldMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_XlatRotY );
	}


	m_pWorldMesh->UpdateTracker();

	PROTRACK_BEGINBLOCK("ComputeMtxPal");
		// Update bone matrix palette...
		ComputeMtxPalette( TRUE );
	PROTRACK_ENDBLOCK();//"ComputeMtxPal");

	if( (m_pPartMgr == NULL) || !m_pPartMgr->IsCreated() || (m_pPartMgr->GetLimbState( LIMB_TYPE_HEAD ) == CBotPartMgr::LIMB_STATE_INTACT) ) {
		m_GazeUnitVec_WS.ReceiveUnit( *m_pGazeDir_WS );

		// because the miner model is backwards, apparently
		m_GazeUnitVec_WS.Negate();
	} else {
		m_GazeUnitVec_WS = m_MtxToWorld.m_vFront;
	}

	EntityMtxToWorld.Identity();
	EntityMtxToWorld.SetRotationYXZ( m_fMountYaw_WS, m_fMountPitch_WS, 0.0f );
	EntityMtxToWorld.m_vPos = m_MountPos_WS;
	if (!m_pCurMech)
		Relocate_RotXlatFromUnitMtx_WS( &EntityMtxToWorld, TRUE, m_pMoveIdentifier );
}


// Returns TRUE if Miner's finger is on his primary weapon's trigger.
// In other words, TRUE is returned if Miner is in a state where he
// can pull the trigger.
BOOL CBotMiner::_IsFingerOnTrigger1( void ) const {
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


// Returns TRUE if Miner's finger is on his secondary weapon's trigger.
// In other words, TRUE is returned if Miner is in a state where he
// can pull the trigger.
BOOL CBotMiner::_IsFingerOnTrigger2( void ) const {
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


void CBotMiner::_HandleWeaponFiring( void ) {
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

	u32 uRightArmLimbState = CBotPartMgr::LIMB_STATE_COUNT;
	if (m_pPartMgr && m_pPartMgr->IsCreated())
	{
		uRightArmLimbState = m_pPartMgr->GetLimbState( LIMB_TYPE_RIGHT_ARM );
	}

	switch( uRightArmLimbState ) {
		case CBotPartMgr::LIMB_STATE_DANGLING: {
			CFVec3A vMuzzlePt;
			CFVec3A vTgtPt;
			m_apWeapon[0]->ComputeMuzzlePoint_WS( &vMuzzlePt );
			vTgtPt.Mul( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPriFire]->m_vFront, 1000.0f );
			vTgtPt.Add( vMuzzlePt );
			nFireBitsPrimary = m_apWeapon[0]->TriggerWork( fTrig1, fTrig2, &vTgtPt, NULL );
			break;
		}

		case CBotPartMgr::LIMB_STATE_REMOVED:	//don't fire if limb removed
			nFireBitsPrimary = m_apWeapon[0]->TriggerWork( 0.f, 0.f, &m_TargetedPoint_WS, NULL );
			nFireBitsPrimary = 0;
			break;

		default:
			nFireBitsPrimary = m_apWeapon[0]->TriggerWork( fTrig1, fTrig2, &m_TargetedPoint_WS, NULL );
			break;
	}


	if( nFireBitsPrimary ) {
		// We fired the primary weapon...
		JustFired();  // No more disguise...

		FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM );
		ZeroTime( ANIMTAP_FIRE_1 );
//		ZeroTime( ANIMTAP_FIRE_1_ALERT );

	}

	if( m_apWeapon[1] ) {
		if( m_apWeapon[1]->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_THROWABLE && 
			(uRightArmLimbState == CBotPartMgr::LIMB_STATE_INTACT) ) {
			// Secondary is throwable...

			if( m_apWeapon[1]->Throwable_TriggerWork( fTrig2 ) ) {
				// We can throw the grenade...

				m_nThrowState = THROWSTATE_REACHING_FOR_AMMO;
				ZeroTime( ANIMTAP_FIRE_2_UPPER );

				JustFired();  // No more disguise...
			}
		}
	}
}


void CBotMiner::_AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {

	if( !m_pCBMiner ) {
		rNewMtx.Mul( rParentMtx, rBoneMtx );
		return;
	}

	FASSERT(m_pCBMiner && m_pCBMiner->TypeBits() & ENTITY_BIT_BOTMINER);

	if( m_pCBMiner->m_pPartMgr &&
		m_pCBMiner->m_pPartMgr->IsCreated() &&
		m_pCBMiner->m_pPartMgr->AnimBoneCallbackFunctionHandler( nBoneIndex, rNewMtx, rParentMtx, rBoneMtx ) ) {
		return;
	} else if( nBoneIndex == (u32)((CBotMiner *)m_pCBMiner)->m_nBoneIndexGroin ) {
		// Groin
		rNewMtx.Mul( rParentMtx, rBoneMtx );
		m_GroinVecY_WS = rNewMtx.m_vUp;
	} else if( nBoneIndex == (u32)((CBotMiner *)m_pCBMiner)->m_nBoneIndexTorso ) {
		// Torso
		CFMtx43A WorldMtx;
		CFQuatA Quat;
		CBotMiner *pBotMiner = (CBotMiner *)m_pCBMiner;

		Quat.BuildQuat( m_GroinVecY_WS, pBotMiner->m_fAimDeltaYaw_MS - pBotMiner->m_fLegsYaw_MS );
		WorldMtx.Mul( rParentMtx, rBoneMtx );

		rNewMtx.m_vPos = WorldMtx.m_vPos;
		Quat.MulPoint( rNewMtx.m_vRight, WorldMtx.m_vRight );
		Quat.MulPoint( rNewMtx.m_vUp, WorldMtx.m_vUp );
		Quat.MulPoint( rNewMtx.m_vFront, WorldMtx.m_vFront );	
	} else {
		// Head
		m_pCBMiner->HeadLookUpdate( rNewMtx, rParentMtx, rBoneMtx, TRUE );
	}
}


BOOL CBotMiner::_InventoryCallback( CInventoryCallbackReason_e eReason, CInventory *pInventory, u32 nHandIndex, u32 nWeaponIndex ) {
	CBotMiner *pBotMiner = (CBotMiner *)pInventory->m_pUser;

	switch( eReason ) {
	case IREASON_WEAPONCHANGE:
		FASSERT_NOW;

		break;

	case IREASON_RELOAD:
		if( nHandIndex == 0 ) {
			pBotMiner->m_abReloadRequest[0] = TRUE;
		}

		break;

	case IREASON_WEAPONUPGRADED:
		FASSERT_NOW;

		break;

	}

	return TRUE;
}



#if 0
void _MinerDeathDebris_StateChangeCBFunc(CDebris* pDebris, u32 uStateChange)
{
}


void CBotMiner::SpawnDeathEffects(void)
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

		//make one big debris chunk that looks just like the original model.
		CFVec3A DebrisVelocity;
		CFVec3A TempVec1;
		CFVec3A TempVec2;
		CFVec3A UnitDir;
		
		DebrisInitSettings_t PrimaryChunkSetting;

		PrimaryChunkSetting.bSampleDynamicLights = TRUE;
		PrimaryChunkSetting.fMinRotVel = 0.15f;
		PrimaryChunkSetting.fMaxRotVel = 0.25f;
		PrimaryChunkSetting.fCullDist2 = (300.0f * 300.0f);
		PrimaryChunkSetting.fLinearFrictionPerSec = 40.0f;
		PrimaryChunkSetting.fRotFrictionPerSec = 20.0f;
		PrimaryChunkSetting.fGravityPerSec = 40.0f;
		PrimaryChunkSetting.fShrinkPerSec = 1000.0f;
		PrimaryChunkSetting.pStateChangeCBFunc = _MinerDeathDebris_StateChangeCBFunc;

		UnitDir.Set(0.0f, 1.0f, 0.0f);

		DebrisVelocity = UnitDir;
		DebrisVelocity.Mul( fmath_RandomFloatRange( 12.5f, 21.5f ));
		DebrisVelocity.x += fmath_RandomBipolarUnitFloat() * 5.75f;
		DebrisVelocity.y += fmath_RandomFloatRange( 0.85f, 7.0f );
		DebrisVelocity.z += fmath_RandomBipolarUnitFloat() * 5.75f;

		TempVec1.Set( MtxToWorld()->m_vPos );
		TempVec2.Set( DebrisVelocity );
		
		FMesh_t* pMesh = this->m_pWorldMesh->m_pMesh;
		CDebris::CreateDebris(	&TempVec1,		//initial pos
								&TempVec2, 		//initial vel
								&(pMesh), 1,	//array of meshes?
								1.0f,			//initial scale
								&PrimaryChunkSetting);		//other PrimaryChunkSetting
	} else if( m_uBotDeathFlags & BOTDEATHFLAG_COMEAPART_FAST ) {
		DebrisInitSettings_t SecondaryChunkSetting;

		SecondaryChunkSetting.bSampleDynamicLights = TRUE;
		SecondaryChunkSetting.fMinRotVel = 0.65f;
		SecondaryChunkSetting.fMaxRotVel = 1.15f;
		SecondaryChunkSetting.fCullDist2 = (85.0f * 85.0f);
		SecondaryChunkSetting.fLinearFrictionPerSec = 40.0f;
		SecondaryChunkSetting.fRotFrictionPerSec = 20.0f;
		SecondaryChunkSetting.fGravityPerSec = 40.0f;
		SecondaryChunkSetting.fShrinkPerSec = 0.075f;
		SecondaryChunkSetting.pStateChangeCBFunc = NULL;

		CFVec3A DebrisVelocity;
		CFVec3A DebrisPos;

		DebrisPos.Set( MtxToWorld()->m_vPos );
		DebrisPos.y += 2.0f;
	
//		fpart_SpawnParticleEmitter(FPART_TYPE_SMOKE_CLOUD, 0.7f, DebrisPos);

		for(u32 i=0; i < 6; i++ ) 
		{
			
			DebrisVelocity.x = fmath_RandomBipolarUnitFloat() * 15.75f;
			DebrisVelocity.y = fmath_RandomBipolarUnitFloat() * 0.10f;
			DebrisVelocity.z = fmath_RandomBipolarUnitFloat() * 15.75f;

			DebrisVelocity.Mul( fmath_RandomFloatRange( 12.5f, 21.5f ));

			CDebris::CreateDebris(	&(DebrisPos),		//initial pos
										&(DebrisVelocity), 	//initial vel
										&(CBotMiner::m_papDeathChunkMeshs[i]), 1,	//array of pointers to meshes
										1.0f,	//initial scale
										&SecondaryChunkSetting);		//other PrimaryChunkSetting
		}
	}
}
#endif


void CBotMiner::DebugDraw( CBotMiner *pMiner ) {
	if( !(pMiner->TypeBits() & ENTITY_BIT_BOTMINER) )
		return;

	fdraw_FacetedWireSphere( &(pMiner->m_MountPos_WS.v3), 3.0f, 4, 4, &FColor_MotifBlue );
}


void CBotMiner::CheckpointSaveSelect( s32 nCheckpoint ) {
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


//////////////////
//WEAPON SWITCHING

BOOL CBotMiner::SwitchPrimaryWeapon( BOOL bSwitchToInitial ) {
	// if request == current weapon state, don't do anything
	if( bSwitchToInitial == m_bInitialWeaponActive ) {
		return FALSE;
	}

	m_bInitialWeaponRequested = bSwitchToInitial;
	return TRUE;
}


BOOL CBotMiner::IsPrimaryWeaponInitial( void ) {
	return m_bInitialWeaponActive;
}


BOOL CBotMiner::SwitchingWeapons( void ) const {
	// more to come...
	return !((m_nWSState == WS_STATE_NONE) &&
			(m_bInitialWeaponActive == m_bInitialWeaponRequested));
}


void CBotMiner::_HandleWeaponSwitching( void ) {
	f32 fUnitTime;

	switch( m_nWSState ) {
		case WS_STATE_NONE:
			if( m_bInitialWeaponActive != m_bInitialWeaponRequested ) {
				// request != current situation, switch weapons
				_BeginWeaponSwitch();
			}
			break;

		case WS_STATE_GRABBING:
			if( !DeltaTime( m_nWSASI, -FLoop_fPreviousLoopSecs * m_pBotInfo_Weapon->fSwitchTimeScale, TRUE ) ) {
				fUnitTime = fmath_UnitLinearToSCurve( GetUnitTime( m_nWSASI ) );
				SetControlValue( m_nWSASI, 1.01f - fUnitTime );
			} else {
				// done releasing
				m_nWSState = WS_STATE_STOWING;
				SetControlValue( ANIMCONTROL_WEAPON_SWITCH, 1.0f );

				ZeroTime( ANIMTAP_WEAPON_SWITCH );
				SetControlValue( m_nWSASI, 0.0f );

				m_apWeapon[0]->Attach_UnitMtxToParent_PS_NewScale_PS( this, m_apszBoneNameTable[BONE_SECONDARY_FIRE], &CFMtx43A::m_IdentityMtx, 1.0f, TRUE );
			}
			break;

		case WS_STATE_STOWING:
		case WS_STATE_RETRIEVING:
			if( !DeltaTime( ANIMTAP_WEAPON_SWITCH, FLoop_fPreviousLoopSecs * m_pBotInfo_Weapon->fSwitchTimeScale, TRUE ) ) {
                if( m_nWSState==WS_STATE_STOWING && GetUnitTime( ANIMTAP_WEAPON_SWITCH )>=0.5f ) {
					// hit the halfway point.  Switch weapons.
					m_apWeapon[0]->ResetToState( CWeapon::STATE_STOWED );
					
					m_bInitialWeaponActive = !m_bInitialWeaponActive;
					_SwitchPrimaryWeapon();
					
					m_apWeapon[0]->ResetToState( CWeapon::STATE_STOWED );
					m_apWeapon[0]->Attach_UnitMtxToParent_PS_NewScale_PS( this, m_apszBoneNameTable[BONE_SECONDARY_FIRE], &CFMtx43A::m_IdentityMtx, 1.0f, TRUE );

					m_nWSState = WS_STATE_RETRIEVING;
				}
			} else {
				// done retrieving
                
				if( m_nWSState == WS_STATE_STOWING ) {
					FASSERT_NOW;
				}

				m_nWSASI = ANIMCONTROL_WEAPON_SWITCH_RELEASE_SINGLE;
				ZeroTime( m_nWSASI );
				SetControlValue( m_nWSASI, 1.0f );
				SetControlValue( ANIMCONTROL_WEAPON_SWITCH, 0.0f );

				m_apWeapon[0]->Attach_UnitMtxToParent_PS_NewScale_PS( this, m_apszBoneNameTable[BONE_ATTACHPOINT_PRIMARY], &CFMtx43A::m_IdentityMtx, 1.0f, TRUE );
				m_nWSState = WS_STATE_RELEASING;
			}
			break;

		case WS_STATE_RELEASING:
			if( !DeltaTime( m_nWSASI, FLoop_fPreviousLoopSecs * m_pBotInfo_Weapon->fSwitchTimeScale * 0.5f, TRUE ) ) {
				fUnitTime = fmath_UnitLinearToSCurve( GetUnitTime( m_nWSASI ) );
				SetControlValue( m_nWSASI, 1.01f - fUnitTime );
			} else {
				// done releasing
				m_apWeapon[0]->SetDesiredState( CWeapon::STATE_DEPLOYED );
				SetControlValue( m_nWSASI, 0.0f );
				m_nWSState = WS_STATE_NONE;
				
			}
			break;

		default:
			FASSERT_NOW;
			break;
	}
}


void CBotMiner::_BeginWeaponSwitch( void ) {
	FASSERT( m_bInitialWeaponActive != m_bInitialWeaponRequested );

	// cancel reload requests
	m_abReloadRequest[0] = FALSE;
	m_nWSASI = ANIMTAP_WEAPON_SWITCH_RELEASE_SINGLE;
	m_nWSState = WS_STATE_GRABBING;
	SetControlValue( m_nWSASI, 0.0f );
	UpdateUnitTime( m_nWSASI, 1.0f );

	m_apWeapon[0]->SetDesiredState( CWeapon::STATE_STOWED );
}


void CBotMiner::_SwitchPrimaryWeapon( void ) {
	m_apWeapon[0]->RemoveFromWorld();

	CWeapon *pWpn = m_apWeapon[0];
	m_apWeapon[0] = m_pStoredWeapon;
	m_pStoredWeapon = pWpn;

	m_apWeapon[0]->AddToWorld();

	FASSERT( m_bInitialWeaponRequested == m_bInitialWeaponActive );
}


void CBotMiner::CheckpointRestore( void ) {
	BOOL bInitialWpnActive;

	CBot::CheckpointRestore();

	CFCheckPoint::LoadData( bInitialWpnActive );


	// make sure the correct weapon is out
	if( bInitialWpnActive ) {
		m_bInitialWeaponActive = TRUE;
		m_bInitialWeaponRequested = TRUE;

		if( m_pInitialPrimaryWeapon != m_apWeapon[0] ) {
			_SwitchPrimaryWeapon();
		}
	} else {
		m_bInitialWeaponActive = FALSE;
		m_bInitialWeaponRequested = FALSE;
		
		if( m_pInitialPrimaryWeapon == m_apWeapon[0] ) {
			_SwitchPrimaryWeapon();
		}
	}

	m_apWeapon[0]->ResetToState( CWeapon::STATE_DEPLOYED );
	m_apWeapon[0]->Attach_UnitMtxToParent_PS_NewScale_PS( this, m_apszBoneNameTable[BONE_ATTACHPOINT_PRIMARY], &CFMtx43A::m_IdentityMtx, 1.0f, TRUE );

	// reset weapon switching animations and state
	m_nWSState = WS_STATE_RELEASING;
	SetControlValue( ANIMCONTROL_WEAPON_SWITCH, 0.0f );
	m_nWSASI = ANIMTAP_WEAPON_SWITCH_RELEASE_SINGLE;
	ZeroTime( m_nWSASI );
	SetControlValue( m_nWSASI, 1.0f );
}


BOOL CBotMiner::CheckpointSave( void ) {
	BOOL brv = CBot::CheckpointSave();

	CFCheckPoint::SaveData( m_bInitialWeaponActive );

	return brv;
}

CWeapon *CBotMiner::SetStoredWeapon( CWeapon *pWeapon ) 
{ 
	CWeapon *pOldWpn = m_pStoredWeapon;
	m_pStoredWeapon = pWeapon; 

	return pOldWpn;
}

