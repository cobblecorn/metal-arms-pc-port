//////////////////////////////////////////////////////////////////////////////////////
// botEliteGuard.cpp -
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
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 01/23/02 Scholz       Created.
//////////////////////////////////////////////////////////////////////////////////////


#include "fang.h"
#include "BotEliteGuard.h"
#include "BotScout.h"
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
#include "potmark.h"
#include "reticle.h"
#include "player.h"
#include "level.h"
#include "ProTrack.h"
#include "fsound.h"
#include "meshentity.h"
#include "botpart.h"
#include "Ai\AIEnviro.h"
#include "Hud2.h"
#include "zipline.h"
#include "fworld_coll.h"
#include "weapon_staff.h"
#include "fdecal.h"
#include "game.h"

#define _BOTINFO_FILENAME		"b_elitegrd"
#define _BOTPART_FILENAME		"bp_elitegrd"
#define _STREAMER_TEXTURE_NAME	"TFMEStream1"

#define _BEAM_TEX_0_NAME					( "TRMUtrans01" )
#define _BEAM_TEX_1_NAME					( "TRMUtrans02" )
#define _BEAM_TEX_2_NAME					( "TRMUtrans03" )
#define _BEAM_TEX_FRAME_RATE				( 30.0f )			// rate at which BEAMmission texture frames animate (frames/sec)
#define _BEAM_TEX_SCROLL_RATE				( 2.0f )			// rate at which BEAMmission texture coords scroll (units/sec)
static const f32 _fEGPrimaryLaserFireBaseSpeed			= 124.0f;
static const f32 _fEGPrimaryLaserFireBaseScatterSpeed	= 8.0f;

#define _LASER_TEX_NAME						(("TFMQshot_01"))
#define _MELEE_SPHERE_RADIUS				( 2.0f )
#define _ATTACK_AIM_TIME					( 1.0f )			// time elite stays in the attack pose

static cchar *_pszGuardMeshFilenameRoot = "GRMEelite";
static cchar*		   _apszGuardMeshFilenames[] = {"GRMEelite00"};
static cchar*		   _BotEliteGuard_pszSoundEffectBank = "elite";
static CBotEliteGuard* _pThisBot = NULL;

static cchar* _pszSoundGroupTable[CBotEliteGuard::ELITESND_COUNT] = 
{
	"SRMEattack",	//	ELITESND_SWIPEATTACK
	"SRMEattimp",	//	ELITESND_SWIPEIMPACT
	"SRMEstafire",//ELITESND_PRIMARY_FIRE
};
//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotEliteGuardBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************
typedef struct 
{
	cchar* pszDamageProfile;
	f32 fSpeed;
	f32 fProjectileMeshScale;
} ELiteRocketInfo_t;

static ELiteRocketInfo_t  _ELiteRocketInfo;

typedef struct 
{
	cchar* pszDamageProfile;
	f32 fSpeed;
	f32 fProjectileMeshScale;
} ELiteLaserInfo_t;
static ELiteLaserInfo_t  _ELiteLaserInfo;


static CBotEliteGuardBuilder *_pBotEliteGuardBuilder;
static CDamageProfile *_pEliteRocketDamageProfile = NULL;
static CDamageProfile *_pEliteLaserDamageProfile = NULL;
static FDecalDefHandle_t _hDecalDefCopy = FDECALDEF_INVALID_HANDLE;

static void _EGTracerBuildTrackerSkipList(void*);

#define _NUM_EG_BEAMS 6
static CBotScout::TransBeam_t _aEGBeam[ _NUM_EG_BEAMS ];
static s32 _nNumEGBeamsThisFrame = 0;
#define NUM_BEAM_TEX 3								// number of transmission effect textures
static CFTexInst _aEGBeamTexInst[NUM_BEAM_TEX];	// texture for transmission effect

static CFTexInst _TracerTexInstPri;
static TracerDef_t _TracerDefPri;			// Tracer definition
static TracerGroupHandle_t _hTracerGroupPri;	// Our primary fire tracer group

//f32 fTempDiveTicker;
// CFVec3A vTempDiveBall(0.0f,0.0f,0.0f);


#define DOT_XZ(arg0,arg1) ((arg0.x * arg1.x) + (arg0.z * arg1.z))

void CBotEliteGuardBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType )
{	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_BOTELITEGUARD, pszEntityType );

	// Override defaults set by our parent classes...
	m_nEC_HealthContainerCount = CBotEliteGuard::m_BotInfo_Gen.nBatteryCount;
	m_pszEC_ArmorProfile = CBotEliteGuard::m_BotInfo_Gen.pszArmorProfile;

	m_anNPCWeaponType[0] = _NPC_WEAPON_STAFF;			
	m_anNPCWeaponType[1] = _NPC_WEAPON_HAND;		

	// This class of bots can be recruited...
	FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_CLASS_CAN_BE_RECRUITED );

	// This class of bots shows up on Glitch's radar...
	FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_SHOWS_UP_ON_RADAR );
}


BOOL CBotEliteGuardBuilder::PostInterpretFixup( void )
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


BOOL CBotEliteGuardBuilder::InterpretTable( void )
{
	return CBotBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotEliteGuard
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************


BOOL 	CBotEliteGuard::m_bSystemInitialized	= FALSE;
u32		CBotEliteGuard::m_nBotClassClientCount	= 0;

CBotEliteGuard::BotInfo_Gen_t CBotEliteGuard::m_BotInfo_Gen;
CBotEliteGuard::BotInfo_MountAim_t CBotEliteGuard::m_BotInfo_MountAim;
CBotEliteGuard::BotInfo_Walk_t CBotEliteGuard::m_BotInfo_Walk;
CBotEliteGuard::BotInfo_Jump_t CBotEliteGuard::m_BotInfo_Jump;
CBotEliteGuard::EliteInfoDive_t CBotEliteGuard::m_DefaultEliteInfoDive;
CBotEliteGuard::EliteInfoStaff_t CBotEliteGuard::m_DefaultEliteInfoStaff;

CBotAnimStackDef CBotEliteGuard::m_AnimStackDef;

CFVec3A			CBotEliteGuard::m_GroinVecY_WS;
CBotPartPool*	CBotEliteGuard::m_pPartPool;

s32			CBotEliteGuard::m_nBoneIndexPrimaryFire = -1;
s32			CBotEliteGuard::m_nBoneIndexStaff		= -1;
s32			CBotEliteGuard::m_nBoneIndexHead		= -1;
s32			CBotEliteGuard::m_nBoneIndexTorso		= -1;
s32			CBotEliteGuard::m_nBoneIndexGroin		= -1;

CFTexInst   CBotEliteGuard::m_StreamerTexInst;
CFSoundGroup* CBotEliteGuard::m_pSounds[ELITESND_COUNT];
const CDamageProfile* CBotEliteGuard::m_pDamageProfileSlash=NULL;

BOOL CBotEliteGuard::InitSystem( void )
{
	FASSERT( !m_bSystemInitialized );
	
	_pBotEliteGuardBuilder = fnew CBotEliteGuardBuilder;
	if( !_pBotEliteGuardBuilder ) {
		return FALSE;
	}
	m_bSystemInitialized = TRUE;
	m_nBotClassClientCount = 0;

	_ELiteRocketInfo.pszDamageProfile = "EliteRocketDam";
	_ELiteRocketInfo.fSpeed = 80.0f;
	_ELiteRocketInfo.fProjectileMeshScale = 1.0f;;

	
	_ELiteLaserInfo.pszDamageProfile = "EliteLaserDam";
	_ELiteLaserInfo.fSpeed = 80.0f;
	_ELiteLaserInfo.fProjectileMeshScale = 1.0f;;

	return TRUE;
}

BOOL CBotEliteGuard::ClassHierarchyLoadSharedResources( void ) 
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

	//default for the CSV defaults
	m_BotInfo_Walk.fHopLRImpulseMag = 12.0f;
	m_BotInfo_Walk.fHopAnimLandPct = 0.75f;
	m_BotInfo_Walk.fApproxHopLRDist = 3.0f;

	m_BotInfo_Walk.fRollStickForceWhileRolling = 1.0f;
	m_BotInfo_Walk.fRollLRImpulseMagXZ = 23.0f;
	m_BotInfo_Walk.fRollLRImpulseMagY = 6.0f;
	m_BotInfo_Walk.fRollAnimLandPct = 0.85f;
	m_BotInfo_Walk.fApproxRollLRDist= 15.0f;

	if( !ReadBotInfoFile( m_aGameDataMap, _BOTINFO_FILENAME ) )
	{
		goto _ExitLSRWithError;
	}

	_pEliteRocketDamageProfile = CDamage::FindDamageProfile( _ELiteRocketInfo.pszDamageProfile );
	_pEliteLaserDamageProfile = CDamage::FindDamageProfile( _ELiteLaserInfo.pszDamageProfile );
	m_pDamageProfileSlash = CDamage::FindDamageProfile( m_DefaultEliteInfoStaff.pszDamageProfileSlash );
	_hDecalDefCopy = m_DefaultEliteInfoStaff.hDecalDef;

	if( !_BuildAnimStackDef() )
	{
		goto _ExitLSRWithError;
	}

	m_StreamerTexInst.SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, _STREAMER_TEXTURE_NAME ) );
	if( !m_StreamerTexInst.GetTexDef() ) 
	{
		goto _ExitLSRWithError;
	}

	// Load the sound effects bank for this bot...
	if( !fresload_Load( FSNDFX_RESTYPE, _BotEliteGuard_pszSoundEffectBank) )
	{
		DEVPRINTF( "CBotEliteGuard::ClassHierarchyLoadSharedResources(): Could not load sound effect bank '%s'\n", _BotEliteGuard_pszSoundEffectBank);
	}

	for (EliteGuardSound_e iSndIdx = (EliteGuardSound_e)0; iSndIdx < ELITESND_COUNT; iSndIdx = (EliteGuardSound_e)(int(iSndIdx)+1))
	{
		m_pSounds[iSndIdx] = CFSoundGroup::RegisterGroup( _pszSoundGroupTable[iSndIdx] );
		if (!m_pSounds[iSndIdx])
			DEVPRINTF( "CBotEliteGuard::ClassHierarchyLoadSharedResources(): Could not load sound effect '%s'\n", _pszSoundGroupTable[iSndIdx]);
	}

	_nNumEGBeamsThisFrame = 0;

	_aEGBeamTexInst[0].SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, _BEAM_TEX_0_NAME ) );
	if( _aEGBeamTexInst[0].GetTexDef() == NULL )
	{
		DEVPRINTF( "CBotEG Load Shared Resources: Could not load beam texture %s.\n", _BEAM_TEX_0_NAME );
		goto _ExitLSRWithError;
	}

	_aEGBeamTexInst[1].SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, _BEAM_TEX_1_NAME ) );
	if( _aEGBeamTexInst[1].GetTexDef() == NULL )
	{
		DEVPRINTF( "CBotEG Load Shared Resources: Could not load beam texture %s.\n", _BEAM_TEX_1_NAME );
		goto _ExitLSRWithError;
	}

	_aEGBeamTexInst[2].SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, _BEAM_TEX_2_NAME ) );
	if( _aEGBeamTexInst[2].GetTexDef() == NULL )
	{
		DEVPRINTF( "CBotEG Load Shared Resources: Could not load beam texture %s.\n", _BEAM_TEX_2_NAME );
		goto _ExitLSRWithError;
	}

	_TracerDefPri.pUser						= NULL;
	_TracerDefPri.pFcnKillCallback			= _EGTracerKilledCallbackPrimary;
	_TracerDefPri.pFcnBuildSkipList			= _EGTracerBuildTrackerSkipList;
	_TracerDefPri.fWidth_WS					= 0.5f;
	_TracerDefPri.fLength_WS				= 2.0f;
	_TracerDefPri.fSpeed_WS					= 124.0f;
	_TracerDefPri.fMaxTailDist_WS			= 300.0f;
	_TracerDefPri.fBeginDeathUnitFade_WS	= 0.25f;
	_TracerDefPri.uFlags					= TRACERFLAG_DRAW_ADDITIVE | TRACERFLAG_THICK_PROJECTILE;
	_TracerDefPri.ColorRGBA.OpaqueWhite();

	_hTracerGroupPri = TRACER_NULLGROUPHANDLE;
	_TracerTexInstPri.SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, _LASER_TEX_NAME ) );
	if( _TracerTexInstPri.GetTexDef() == NULL )
	{
		DEVPRINTF( "CBotEG Load Shared Resources: Could not load laser texture %s.\n", _LASER_TEX_NAME);
		goto _ExitLSRWithError;
	}
	_hTracerGroupPri = tracer_CreateGroup( &_TracerTexInstPri,  260 );

	if( (_hTracerGroupPri == TRACER_NULLGROUPHANDLE) ) {
		DEVPRINTF( "CBotEG::ClassHierarchyLoadSharedResources(): Could not create tracer group.\n" );
		goto _ExitLSRWithError;
	}


	return TRUE;
	// Error:
_ExitLSRWithError:
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

void CBotEliteGuard::UninitSystem( void )
{
	if( m_bSystemInitialized ) {
		m_bSystemInitialized = FALSE;
		fdelete( _pBotEliteGuardBuilder );
		_pBotEliteGuardBuilder = NULL;
	}	
}

void CBotEliteGuard::ClassHierarchyUnloadSharedResources( void ) 
{
	FASSERT( m_nBotClassClientCount > 0 );

	--m_nBotClassClientCount;

	if( m_nBotClassClientCount > 0 ) 
	{
		return;
	}

	tracer_DestroyGroup( _hTracerGroupPri );
	_hTracerGroupPri = TRACER_NULLGROUPHANDLE;


	m_AnimStackDef.Destroy();

	CBot::ClassHierarchyUnloadSharedResources();
}

CBotEliteGuard::CBotEliteGuard() : CBot()
{

}


CBotEliteGuard::~CBotEliteGuard()
{
	if( IsSystemInitialized() && IsCreated() )
	{
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld(TRUE);
		ClassHierarchyDestroy();
	}
}


BOOL CBotEliteGuard::Create( s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName )
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
	CBotEliteGuardBuilder *pBuilder = (CBotEliteGuardBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == _pBotEliteGuardBuilder )
	{
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOTELITEGUARD );
	}

	// Set our builder parameters...

	// Create an entity...
	return CBot::Create( &m_BotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}


void CBotEliteGuard::ClassHierarchyDestroy( void )
{	// Delete the items that we had instantiated for us...
	m_Anim.Destroy();

	_StreamerOn(1.0f);
	
	fdelete( m_pWorldMesh );
	m_pWorldMesh = NULL;

	fdelete( m_apWeapon[0] );
	m_apWeapon[0] = NULL;
	
	for (EliteGuardSound_e iSndIdx = (EliteGuardSound_e)0; iSndIdx < ELITESND_COUNT; iSndIdx = (EliteGuardSound_e)(int(iSndIdx)+1))
	{
		if (m_pSoundEmitters[iSndIdx])
		{
			m_pSoundEmitters[iSndIdx]->Destroy();
			m_pSoundEmitters[iSndIdx] = NULL;
		}
	}

	CBot::ClassHierarchyDestroy();
}


BOOL CBotEliteGuard::ClassHierarchyBuild( void )
{	FMesh_t *pMesh;
	FMeshInit_t MeshInit;
	s32 nBoneIndex;
	cchar * pszMeshFilename = NULL;
	s32 sLeftHandSecondaryFire;			//CPS 4.7.03
	
	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CBotEliteGuardBuilder *pBuilder = (CBotEliteGuardBuilder *)GetLeafClassBuilder();

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
	pszMeshFilename = _apszGuardMeshFilenames[0];
	if ( pBuilder->m_uMeshVersionOverride >0 && pBuilder->m_uMeshVersionOverride < sizeof(_apszGuardMeshFilenames)/sizeof(char*) )
	{
		pszMeshFilename = _apszGuardMeshFilenames[pBuilder->m_uMeshVersionOverride];
	}
	pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pszMeshFilename);
	if( pMesh == NULL )
	{
		DEVPRINTF( "CBotEliteGuard::ClassHierarchyBuild(): Could not find mesh '%s'\n", pszMeshFilename );
		goto _ExitWithError;
	}

	// Create world mesh...
	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL )
	{
		DEVPRINTF( "CBotEliteGuard::ClassHierarchyBuild(): Not enough memory to create CFWorldMesh.\n" );
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
		DEVPRINTF( "CBotEliteGuard::ClassHierarchyBuild(): Trouble creating m_Anim.\n" );
		goto _ExitWithError;
	}

	SetControlValue( ANIMCONTROL_STAND, 1.0f );
	UpdateUnitTime( ANIMTAP_STAND, fmath_RandomFloat() );

	m_nBoneIndexPrimaryFire = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_PRIMARY_FIRE] );
	m_nBoneIndexStaff = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_STAFF] );
	m_nBoneIndexHead = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_HEAD] );
	m_nBoneIndexTorso = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_TORSO] );
	m_nBoneIndexGroin = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_GROIN] );
	
	if( (m_nBoneIndexTorso < 0)  || (m_nBoneIndexPrimaryFire < 0) || (m_nBoneIndexStaff < 0) || (m_nBoneIndexHead < 0) || (m_nBoneIndexGroin <0) )
	{
		DEVPRINTF( "CBotEliteGuard::ClassHierarchyBuild(): Trouble retrieving bone indices.\n" );
		goto _ExitWithError;
	}

	m_pStaffUnitDir = &m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexStaff]->m_vUp;

	m_Anim.m_pAnimCombiner->SetBoneCallback( &_AnimBoneCallback );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_GROIN] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_TORSO] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_HEAD] );

	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_L_FOOT_BACK] );
	if( nBoneIndex >= 0 )
	{
		m_pLeftFootDustPos_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;

		nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_R_FOOT_BACK] );
		if( nBoneIndex >= 0 )
		{
			m_pRightFootDustPos_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
		} else
		{
			m_pLeftFootDustPos_WS = NULL;
		}
	}

	if( !m_AnimManFrameAim.Create( m_AnimStackDef.m_nBoneCount, m_AnimStackDef.m_apszBoneNameTable ) )
	{
		DEVPRINTF( "CBotEliteGuard::ClassHierarchyCreate(): Could not create aim animation summer.\n" );
		goto _ExitWithError;
	}
	AttachAnim( ANIMTAP_AIM_SUMMER, &m_AnimManFrameAim );
	m_AnimManFrameAim.Reset();
	UpdateBoneMask( ANIMTAP_AIM_SUMMER, m_aBoneEnableIndices_AimSummer, TRUE );

	// Set primary weapon.........
	m_apWeapon[0] = CBotBuilder::AllocNPCWeaponOfType(pBuilder->m_anNPCWeaponType[0]);

	if( m_apWeapon[0] == NULL ) 
	{
		DEVPRINTF( "CBotEliteGuard::ClassHierarchyBuild() : Error instantiating CWeapon %d.\n", 0 );
		goto _ExitWithError;
	}
	if (!CBotBuilder::CreateNPCWeaponOfType(m_apWeapon[0], pBuilder->m_anNPCWeaponType[0]))
	{
		DEVPRINTF( "CBotEliteGuard::ClassHierarchyBuild() : Error Creating CWeapon %d.\n", 0 );
		goto _ExitWithError;
	}
	m_apWeapon[0]->SetClipAmmo( CWeapon::INFINITE_AMMO );
	m_apWeapon[0]->SetReserveAmmo( CWeapon::INFINITE_AMMO );
	m_apWeapon[0]->EnableAutoWork( FALSE );
	m_apWeapon[0]->SetOwner(this);
	m_apWeapon[0]->SetDesiredState( CWeapon::STATE_DEPLOYED );

	/*
	if (3<m_apWeapon[0]->GetUpgradeLevelCount())
	{
	m_apWeapon[0]->SetUpgradeLevel(3);	  //3 is special level for npc
	}
	else
	{
	m_apWeapon[0]->SetUpgradeLevel(0);	  //3 is special level for npc
	}
	*/

	// Make primary weapon pass along damage to upper-right arm...
	m_apWeapon[0]->SetBotDamageBoneIndex( m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_R_ARM_LOWER] ) );

	SetMaxHealth();

	if( m_nPossessionPlayerIndex >= 0 )
	{
		Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.SetNormOrigin( 0.0f, 0.23f );
	}

	// Find approx eye point...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
	if( nBoneIndex < 0 )
	{
		DEVPRINTF( "CBotEliteGuard::ClassHierarchyBuild(): Could not locate approx eye point bone '%s'.\n", m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
		m_pApproxEyePoint_WS = &m_MtxToWorld.m_vPos;
	}
	else
	{
		m_pApproxEyePoint_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
	}

	// Find gaze direction...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_HEAD ] );
	if( nBoneIndex < 0 )
	{
		DEVPRINTF( "CBotEliteGuard::ClassHierarchyBuild(): Could not locate gaze bone '%s'.\n", m_apszBoneNameTable[ BONE_HEAD ] );
		m_pGazeDir_WS = &m_MtxToWorld.m_vFront;
	}
	else
	{
		m_pGazeDir_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vFront;
	}

	// Find torso mtx...
	m_pAISteerMtx = &m_MtxToWorld;

	// Initialize matrix palette (this is important for attached entities)...
	AtRestMatrixPalette();

	// Create tag points...
	if( !TagPoint_CreateFromBoneArray( m_anTagPointBoneNameIndexArray, m_apszBoneNameTable ) )
	{
		goto _ExitWithError;
	}

	if( !fclib_strnicmp( _pszGuardMeshFilenameRoot, pszMeshFilename, fclib_strlen(_pszGuardMeshFilenameRoot)) )
	{
		if( !m_pPartMgr->Create( this, &m_pPartPool, _BOTPART_FILENAME, PART_INSTANCE_COUNT_PER_TYPE, LIMB_TYPE_COUNT ) )
		{
			goto _ExitWithError;
		}
	}

	// Make invincible limbs...
	MakeLimbInvincible( LIMB_CODE_HEAD, LIMB_TYPE_HEAD, pBuilder );
//	MakeLimbInvincible( LIMB_CODE_ARMS, LIMB_TYPE_LEFT_ARM, pBuilder );
//	MakeLimbInvincible( LIMB_CODE_ARMS, LIMB_TYPE_RIGHT_ARM, pBuilder );
//	MakeLimbInvincible( LIMB_CODE_TORSO, LIMB_TYPE_TORSO, pBuilder );


	// Set up data port stuff...
	DataPort_SetupTetherShockInfo();

	SetBotFlag_Enemy();

//CPS 4.7.03	s32 sLeftHandSecondaryFire = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_L_MIDDLEFINGERB] );
	sLeftHandSecondaryFire = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_L_MIDDLEFINGERB] );
	if( sLeftHandSecondaryFire < 0 )
	{
		DEVPRINTF( "CBotEliteGuard::ClassHierarchyBuild: Bone '%s' doesn't exist in object '%s'.\n", "L_Hand", Name() );
		m_pLeftHandSecondaryFireBonePos = &m_MtxToWorld.m_vPos;
	}
	else
	{
		m_pLeftHandSecondaryFireBonePos = &m_pWorldMesh->GetBoneMtxPalette()[sLeftHandSecondaryFire]->m_vPos;
	}


	// initialize melee data
	MeleeInit( &m_MeleeData, _MELEE_SPHERE_RADIUS, m_pDamageProfileSlash );
	MeleeInit( &m_MeleeData2, _MELEE_SPHERE_RADIUS, m_pDamageProfileSlash, NULL, &m_MeleeData, MELEE_MAX_ENTITIES_PER_SWING ); 

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


BOOL CBotEliteGuard::ClassHierarchyBuilt( void )
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


CEntityBuilder *CBotEliteGuard::GetLeafClassBuilder( void )
{
	return _pBotEliteGuardBuilder;
}


void CBotEliteGuard::_ClearDataMembers( void )
{
	// Init data members...

	m_pMoveIdentifier = &m_StaffCollSphereWS;

	m_fCollCylinderRadius_WS = 3.9f;
	m_fCollCylinderHeight_WS = 7.9f;

	m_pBotInfo_Gen = &m_BotInfo_Gen;
	m_pBotInfo_MountAim = &m_BotInfo_MountAim;
	m_pBotInfo_Walk = &m_BotInfo_Walk;
	m_pBotInfo_Jump = &m_BotInfo_Jump;
	m_pEliteInfo_Dive = &m_DefaultEliteInfoDive;
	m_pEliteInfo_Staff = &m_DefaultEliteInfoStaff;

	m_fGravity = m_pBotInfo_Gen->fGravity;
	m_fMaxFlatSurfaceSpeed_WS = m_pBotInfo_Walk->fMaxXlatVelocity * m_fRunMultiplier;

	m_fMountPitchMax_WS = m_pBotInfo_MountAim->fMountPitchDownLimit;
	m_fMountPitchMin_WS = m_pBotInfo_MountAim->fMountPitchUpLimit;

	m_anAnimStackIndex[ASI_STAND] = ANIMTAP_STAND;
	m_anAnimStackIndex[ASI_STAND_ALERT] = ANIMTAP_STAND_ALERT;

	m_anAnimStackIndex[ASI_WALK] = ANIMTAP_WALK;
	m_anAnimStackIndex[ASI_WALK_ALERT] = ANIMTAP_WALK_ALERT;
	m_anAnimStackIndex[ASI_RUN] = ANIMTAP_RUN;
	m_anAnimStackIndex[ASI_RUN_PANIC] = -1;

	m_anAnimStackIndex[ASI_FALL] = ANIMTAP_JUMP_FLY;

	m_anAnimStackIndex[ASI_HOP_LEFT] = -1;
	m_anAnimStackIndex[ASI_HOP_RIGHT] = -1;
	m_anAnimStackIndex[ASI_STARTLE] = -1;
	m_anAnimStackIndex[ASI_ROLL_LEFT] = ANIMTAP_DIVEROLL;
	m_anAnimStackIndex[ASI_ROLL_RIGHT] = ANIMTAP_DIVEROLL;

	m_anAnimStackIndex[ASI_DOZE_LOOP] = -1;
	m_anAnimStackIndex[ASI_NAPJERK] = -1;
	m_anAnimStackIndex[ASI_WAKE] = -1;

	m_anAnimStackIndex[ASI_RELOAD_CLIP_EJECT_OLD] = -1;
	m_anAnimStackIndex[ASI_RELOAD_CLIP_GRAB_NEW] = -1;
	m_anAnimStackIndex[ASI_RELOAD_CLIP_INSERT_NEW] = -1;
	m_anAnimStackIndex[ASI_RELOAD_CLIP_SLAPIN_NEW] = -1;

	m_anAnimStackIndex[ASI_RELOAD_WITH_LEFT_GRAB_AMMO] = -1;
	m_anAnimStackIndex[ASI_RELOAD_WITH_LEFT_DROP_IN] = -1;
	m_anAnimStackIndex[ASI_RELOAD_WITH_LEFT_FINISH] = -1;
	m_anAnimStackIndex[ASI_RELOAD_SINGLE_ARM] = -1;
	m_anAnimStackIndex[ASI_RELOAD_SINGLE_BODY] = -1;

	m_anAnimStackIndex[ASI_SNEAK] = -1;

	m_anAnimStackIndex[ASI_RC_TETHERED]		= ANIMTAP_RC_TETHERED;
	m_anAnimStackIndex[ASI_RC_POWER_DOWN]	= ANIMTAP_RC_POWER_DOWN;
	m_anAnimStackIndex[ASI_RC_POWER_UP]		= ANIMTAP_RC_POWER_UP;
	m_anAnimStackIndex[ASI_STOOP]			= -1;

	m_anAnimStackIndex[ASI_AIM_PILLBOX]		= -1;

	m_pnEnableBoneNameIndexTableForSummer_Normal	  =	 m_anEnableBoneNameIndexTableForSummer_Normal;
	m_pnEnableBoneNameIndexTableForSummer_TetherShock =  m_anEnableBoneNameIndexTableForSummer_TetherShock;

	// mike start
	m_eStaffFireState = STAFF_FIRE_NONE;
	m_eStaffSwingState = STAFF_SWING_NONE;
	m_fDiveTargetYaw_WS = 0.f;
	m_fDiveYaw_WS = 0.f;
	m_hStreamerEmitter = FXSTREAMER_INVALID_HANDLE;
	//m_vecDiveCameraLookat.Set(0.f,-5,0.f);
	m_vecDiveCameraLookat.Set(m_pEliteInfo_Dive->fCamEnterLookAtX,m_pEliteInfo_Dive->fCamEnterLookAtY,m_pEliteInfo_Dive->fCamEnterLookAtZ);
	//m_vecDiveCameraPos.Set(0.0f,0.0f,-12.f);
	m_vecDiveCameraPos.Set(m_pEliteInfo_Dive->fCamEnterFollowPosX,m_pEliteInfo_Dive->fCamEnterFollowPosY,m_pEliteInfo_Dive->fCamEnterFollowPosZ);

	m_fBeamTransTexFrame = 0.0f;
	m_fBeamTransTexScroll = 0.0f;
	
	fang_MemSet(m_pSoundEmitters,0,sizeof(m_pSoundEmitters[0])*ELITESND_COUNT);

	m_fMaxLandUnitBlendLower = 1.0f;
	m_bFireLaserThisFrame = FALSE;

}


void CBotEliteGuard::ClassHierarchyAddToWorld( void )
{
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CBot::ClassHierarchyAddToWorld();
	m_pWorldMesh->UpdateTracker();

	// Primary must be attached
	m_apWeapon[0]->Attach_UnitMtxToParent_PS( this, m_apszBoneNameTable[BONE_PRIMARY_FIRE] );
	m_apWeapon[0]->AddToWorld();
	FASSERT(m_apWeapon[0]->IsInWorld());

	// ensure that no previous sound handles remain resident
	fang_MemSet(m_pSoundEmitters,0,sizeof(m_pSoundEmitters[0])*ELITESND_COUNT);

	m_fStaffRocketLoadTimer = 0.0f;
	m_fSecsUntilNextSound = 0.0f;
	m_fAttackAimTimer = 0.0f;
}


void CBotEliteGuard::ClassHierarchyRemoveFromWorld( void )
{
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	for (EliteGuardSound_e iSndIdx = (EliteGuardSound_e)0; iSndIdx < ELITESND_COUNT; iSndIdx = (EliteGuardSound_e)(int(iSndIdx)+1))
	{
		_PlaySnd(iSndIdx, FALSE);
	}

	m_pWorldMesh->RemoveFromWorld();
	CBot::ClassHierarchyRemoveFromWorld();
}


void CBotEliteGuard::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList)
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


const CFVec3A *CBotEliteGuard::GetApproxEyePoint( void ) const
{
	FASSERT( IsCreated() );

	return m_pApproxEyePoint_WS;
}


void CBotEliteGuard::ClassHierarchyWork()
{
	CFVec3A TempVec3A;

	FASSERT( m_bSystemInitialized );

	CBot::ClassHierarchyWork();

	if( !IsOurWorkBitSet() )
	{
		return;
	}

	//pgm put these here while tuning up the elite guards anim and movement stuff

	//this->m_pBotInfo_Walk->fSneakVelocity = 3.5f;
	//this->m_pBotInfo_Walk->fMinWalkVelocity = 4.0f;
	//this->m_pBotInfo_Walk->fMinRunBlendVelocity = 10.0f;
	//this->m_pBotInfo_Walk->fMaxRunBlendVelocity = 25.0f;
	//this->m_pBotInfo_Walk->fMaxXlatVelocity = 28.0f;
	//this->m_pBotInfo_Walk->fNormVelocityStepSize  = 2.0f;
	//this->m_pBotInfo_Walk->fMaxSneakStickMag = 0.25f;

	//this->m_pBotInfo_Walk->fDeltaFeetAnimPerFootSneak  = 0.15f;
	//this->m_pBotInfo_Walk->fDeltaFeetAnimPerFootWalk = 0.1f;
	//this->m_pBotInfo_Walk->fDeltaFeetAnimPerFootRun = 0.04f;
	//this->m_pBotInfo_Walk->fDeltaFeetAnimPerRadian = 2.5f;

	//this->m_pBotInfo_Walk->fIdleToWalkDeltaSpeed = 8.0f;
	//this->m_pBotInfo_Walk->fIdleToSneakDeltaSpeed = 8.0f;
	//this->m_pBotInfo_Walk->fAnimLeftFootDownUnitTime = 1.1f;
	//this->m_pBotInfo_Walk->fAnimRightFootDownUnitTime = 1.1f;

	//this->m_pBotInfo_Walk->fSwivelHipsReturnSpeedThreshold = 0.0f;

	//this->m_pBotInfo_Walk->fSwivelHipsDeltaRadiansPerSec = 6.0f;
	//this->m_pBotInfo_Walk->fHipYawSlackWhileStoppedThreshold = 1.57f;
	//this->m_pBotInfo_Walk->fHipFlipHysteresisRads = 0.5f;

	//this->m_pBotInfo_Walk->fInvMaxXlatVelocity = 1.0f / this->m_pBotInfo_Walk->fMaxXlatVelocity;
	//this->m_pBotInfo_Walk->fSneakNormVelocity = m_pBotInfo_Walk->fSneakVelocity / m_pBotInfo_Walk->fMaxXlatVelocity;
	//this->m_pBotInfo_Walk->fMinWalkNormVelocity = m_pBotInfo_Walk->fMinWalkVelocity / m_pBotInfo_Walk->fMaxXlatVelocity;
	//this->m_pBotInfo_Walk->fMinRunBlendNormVelocity = m_pBotInfo_Walk->fMinRunBlendVelocity / m_pBotInfo_Walk->fMaxXlatVelocity;
	//this->m_pBotInfo_Walk->fMaxRunBlendNormVelocity = m_pBotInfo_Walk->fMaxRunBlendVelocity / m_pBotInfo_Walk->fMaxXlatVelocity;
	//this->m_pBotInfo_Walk->fOOMaxWalkRunBlendTime  = 3.5;


	if( IsPlayerBot() ) {
		EndForcedAlert();
	}

	m_fStaffRocketLoadTimer-=FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN( m_fCableGrabTime, 0.0f );

	m_fCableGrabTime -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN( m_fCableGrabTime, 0.0f );

	Power_Work();
	DataPort_Work();

	if( DataPort_IsBeingShocked() )
	{
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
	
//	CFVec3A collision;collision.Set(m_CollSphere.m_Pos);
//	fdraw_DevSphere(&collision,m_CollSphere.m_fRadius);

	_HandleStaffCollision();
	PROTRACK_ENDBLOCK();//"Coll"

	State_e myState = m_nState;
	if (m_nState == STATE_GROUND)
	{
		if (IsDiveJumping())
		{
			myState = STATE_AIR;
			if (m_nPrevState == STATE_AIR)
			{
//				m_Velocity_WS.x = m_Velocity_WS.x * m_pEliteInfo_Dive->fLandingSpeedMult;
//				m_Velocity_WS.y = m_Velocity_WS.y;
//				m_Velocity_WS.z = m_Velocity_WS.z * m_pEliteInfo_Dive->fLandingSpeedMult;

				CFVec3A tmp;
				tmp = m_UnitVelocityXZ_WS;
				tmp.y = 0.0f;
				tmp.Mul(m_pBotInfo_Walk->fMaxXlatVelocity);
				tmp.x = fmath_Abs(tmp.x);
				tmp.z = fmath_Abs(tmp.z);
				FMATH_BIPOLAR_CLAMPMAX(m_Velocity_WS.x, tmp.x);
				FMATH_BIPOLAR_CLAMPMAX(m_Velocity_WS.z, tmp.z);
				

				WS2MS( m_Velocity_MS, m_Velocity_WS );
				VelocityHasChanged();
			}
		}
	}
	// Move and animate our bot...
	switch( myState )
	{
	case STATE_GROUND:
		PROTRACK_BEGINBLOCK("GroundXlat");
		HandleGroundTranslation();
		_HandleJumping();
		_HandleHopRollStartle();
		PROTRACK_ENDBLOCK();//"GroundXlat");

		PROTRACK_BEGINBLOCK("GroundAnim");
		HandleGroundAnimations();
		PROTRACK_ENDBLOCK();//"GroundAnim");
		break;

	case STATE_AIR:
		PROTRACK_BEGINBLOCK("AirXlat");
		HandleAirTranslation();
		_HandleCableTranslation();
		_HandleCableAndDoubleJump();
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
	_HandleCableAnimations();
	HandleHipsAnimation();
	PROTRACK_ENDBLOCK();//"CommonAnim");

	PROTRACK_BEGINBLOCK("Weapon");
	
    HandleTargeting();
//	_HandleStaffAttacking();
	_HandleStaffAnimations();
	_HandleFiringAnimations();
	_HandleAimAnimations();
	PROTRACK_ENDBLOCK();//"WeaponAnim");

	_UpdateMatrices();

	PROTRACK_BEGINBLOCK("WeaponWork");
	_HandleWeaponWork();
	_HandleStaffWork();

	if( m_bFireLaserThisFrame ) {
		if( _FirePrimaryLaser( m_TargetedPoint_WS ) ) {
			JustFired();  // No more disguise...
			m_fStaffRocketLoadTimer = 0.5f;
		}
	}
	PROTRACK_ENDBLOCK();//"WeaponWork");


	if( m_pCableHook && m_pCableHook->m_bAttached ) 
	{
		m_pCableHook->m_Pos_Draw_WS = *m_pLeftHandSecondaryFireBonePos;
	}


	if (IsDeadOrDying())
	{
		DeathWork();
	}
}


void CBotEliteGuard::_HandleHopRollStartle( void ) 
{
	if (!m_bControls_Human &&
		m_nMoveState == BOTMOVESTATE_NONE &&
		m_nJumpState == BOTJUMPSTATE_NONE &&
		!IsDiveJumping()) {

		if (m_nControlsBot_Flags & CBotControl::FLAG_ROLL_LEFT)	{

			f32 fDiveYaw = m_fMountYaw_WS - FMATH_HALF_PI;
			_StartDive(fDiveYaw);
		}
		else if (m_nControlsBot_Flags & CBotControl::FLAG_ROLL_RIGHT) {
			f32 fDiveYaw = m_fMountYaw_WS + FMATH_HALF_PI;
			_StartDive(fDiveYaw);
		}
		else
		{
			HandleHopRollStartleGeneric();
		}
	}

	

}

BOOL CBotEliteGuard::GrabCable( CEZipLine *pCable ) 
{
	if( pCable == m_pCablePrev && m_fCableGrabTime != 0.0f ) 
	{
		return FALSE;
	}

	m_pCableHook = pCable->AttachHook( &m_MountPos_WS, &m_Velocity_WS, this );

	if( m_pCableHook == NULL ) 
	{
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
	Summer_UpdateBoneMask( m_apszBoneNameTable[BONE_L_HAND], TRUE );
	
	f32 fDot = ( m_MountUnitFrontXZ_WS.x * m_pCableHook->m_pCable->m_UnitVec12XZ.x ) + 
				( m_MountUnitFrontXZ_WS.z * m_pCableHook->m_pCable->m_UnitVec12XZ.y) ;
	f32 fZipYaw = fmath_Atan( m_pCableHook->m_pCable->m_UnitVec12XZ.y, m_pCableHook->m_pCable->m_UnitVec12XZ.x );

	if( fDot >= 0.0f ) {
		m_pCableHook->m_fYawSnapOffset = fZipYaw;
	} else {
		m_pCableHook->m_fYawSnapOffset = fZipYaw - FMATH_PI;
	}

	CFMtx43A::m_RotZ.SetRotationZ( -m_fMountYaw_WS + m_pCableHook->m_fYawSnapOffset - FMATH_HALF_PI );
	*m_Anim.m_pAnimManMtx->GetFrameAndFlagAsChanged( BONE_L_HAND ) = CFMtx43A::m_RotZ;

	ZipLineVerletSetup( );

	return TRUE;
}

void CBotEliteGuard::ReleaseCable( void ) 
{
	SetControlValue( ANIMCONTROL_CABLE_GRASP, 0.0f );

	m_pCableHook->m_pCable->DetachHook( m_pCableHook, this );
	m_pCablePrev = m_pCableHook->m_pCable;

	m_fCableGrabTime = BOT_ZIPLINE_GRAB_TIME;
	m_pCableHook = NULL;

	*m_Anim.m_pAnimManMtx->GetFrameAndFlagAsChanged( BONE_L_HAND ) = CFMtx43A::m_IdentityMtx;
	Summer_UpdateBoneMask( m_apszBoneNameTable[BONE_L_HAND], FALSE );
}

void CBotEliteGuard::_HandleCableAndDoubleJump( void ) 
{
	if( m_nJumpState != BOTJUMPSTATE_CABLE ) 
	{
		if( m_bControls_Jump && !m_pPartMgr->IsBackBroken() && ((m_nBotFlags2 & BOTFLAG2_CAN_DOUBLE_JUMP) || (m_Velocity_WS.y >= 0.0f)) ) {
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

void CBotEliteGuard::_HandleCableTranslation( void ) 
{
	if( m_nJumpState == BOTJUMPSTATE_CABLE ) 
	{
		if( m_pCableHook && m_pCableHook->m_bAttached ) 
		{
			// Update the hook hand brake value
			m_pCableHook->m_fInputBrakeAmount = m_fControls_Fire2;

			m_Velocity_MS = m_pCableHook->m_Vel_WS;

			CFVec3A Pos_WS_Diff;

			Pos_WS_Diff.Sub( m_pCableHook->m_Pos_WS, *m_pLeftHandSecondaryFireBonePos );
			m_MountPos_WS.Add( Pos_WS_Diff );

			// Don't spin the hand
			CFMtx43A::m_RotZ.SetRotationZ( -m_fMountYaw_WS + m_pCableHook->m_fYawSnapOffset - FMATH_HALF_PI );
			*m_Anim.m_pAnimManMtx->GetFrameAndFlagAsChanged( BONE_L_HAND ) = CFMtx43A::m_RotZ;

			m_Velocity_WS = m_pCableHook->m_Vel_WS;

			// Update velocities...
			WS2MS( m_Velocity_MS, m_Velocity_WS );
			VelocityHasChanged();

			if( m_bControlsBot_Jump2 ) 
			{
				_StartSingleJump();
				_StartDoubleJump();

				ReleaseCable();
			} 
			else if( m_bControls_Jump ) 
			{
				_StartSingleJump();

				// Push off the cable in the direction we are holding
				CFVec3A Dir;

				Dir = m_XlatStickUnitVecXZ_WS;
				Dir.Mul(-10.0f);

				ApplyVelocityImpulse_MS(Dir);
				
				ReleaseCable();
			}
		}
		else 
		{
			_StartSingleJump();
			ReleaseCable();
		}
	}
}


void CBotEliteGuard::_HandleCableAnimations( void ) 
{
	if( m_fUnitCableBlend>0.0f && m_nJumpState!=BOTJUMPSTATE_CABLE ) 
	{
		m_fUnitCableBlend -= FLoop_fPreviousLoopSecs * m_fTuckUntuckAnimTimeMult;	// SER Change this!
		FMATH_CLAMPMIN( m_fUnitCableBlend, 0.0f );
		SetControlValue( ANIMCONTROL_CABLE_GRASP, m_fUnitCableBlend );
	}
}

void CBotEliteGuard::_HandleJumping( void )
{
	if( !m_bControlsBot_JumpVec )
	{
		if( !m_bControls_Human && m_bControlsBot_Jump2 )   //only for AI control
		{
			_StartSingleJump();
			_StartDoubleJump();
		}
		else if( m_bControls_Human && m_bControls_Jump )  //only for human control
		{
			_StartSingleJump();
			FMATH_SETBITMASK( m_nBotFlags2, BOTFLAG2_CAN_DOUBLE_JUMP );
		}
#if 0 // no more dive for human
		else 
		{

			if( m_bControls_Human & !!((CHumanControl*)Controls())->m_nPadFlagsSelect1)
			{

				f32 fDiveYaw  = 0.0f;
				const CFVec3A& rDiveVec = GetDiveUnitDir();
				f32 fDiveDotVelWS = rDiveVec.Dot(m_UnitVelocityXZ_WS);
				if ( (fDiveDotVelWS >= 0.0f) && (DOT_XZ(rDiveVec,m_MountUnitFrontXZ_WS) > m_pEliteInfo_Dive->fDiveMinCosHalfAngle)) // in front of us
				{
					fDiveYaw = fmath_Atan( rDiveVec.x, rDiveVec.z ); // where on the circle we're yawing to;

					_StartDive(fDiveYaw);		//when human player dive, the direction is the direction of the legs.
				}
			}
		}
#endif
	}
	else
	{
		// Velocity jump specified...
		_StartVelocityJump( &m_ControlsBot_JumpVelocity_WS );
	}

	if( m_nPrevState == STATE_AIR )
	{
		if ( m_nJumpState != BOTJUMPSTATE_NONE )
		{
			_JumpLanded();
		}
	}
	
	// If we hit ground while on a cable, detach
	if( m_pCableHook ) 
	{
		ReleaseCable();
	}

}



void CBotEliteGuard::_EnterFlyMode( void )
{
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
	m_fUnitFlyBlend = 1.0f;
}


void CBotEliteGuard::_StartVelocityJump( const CFVec3A *pJumpVelocity_WS )
{
	CFVec3A JumpVelocity_WS = *pJumpVelocity_WS;
	FMATH_CLAMP( JumpVelocity_WS.y, 0.0f, m_pBotInfo_Jump->fVerticalVelocityJump1 + m_pBotInfo_Jump->fVerticalVelocityJump2);

	_StartSingleJump( &JumpVelocity_WS );

	if( pJumpVelocity_WS->y >= m_pBotInfo_Jump->fVerticalVelocityJump1 )
	{
		_StartDoubleJump( FALSE );
	}
}


void CBotEliteGuard::_StartSingleJump( const CFVec3A *pJumpVelocity_WS )
{	
	if (m_nJumpState == BOTJUMPSTATE_LAUNCH) 
	{
		_StartDoubleJump();
		return;
	}

	if( !pJumpVelocity_WS ) {
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
	m_fUnitFlyBlend = 0.f;
	m_fUnitJump1Jump2Blend = 0.0f;
	m_nJumpState = BOTJUMPSTATE_LAUNCH;
	SetJumping();

	f32 fPlayJumpLaunchFromUnitTime = 0.0f;
	
	if( m_fSpeedXZ_WS != 0.0f )
		fPlayJumpLaunchFromUnitTime =0.0f;

	UpdateUnitTime( ANIMTAP_JUMP_LAUNCH, fPlayJumpLaunchFromUnitTime );

	SetDoubleJumpTimer( m_pBotInfo_Gen->fDoubleJumpTime );
}

void CBotEliteGuard::_StartDive(f32 fDiveYaw_WS )
{
	m_fDiveYaw_WS = m_fMountYaw_WS + m_fLegsYaw_MS;
	m_fDiveTargetYaw_WS = fDiveYaw_WS;
	if ((m_fMountYaw_WS - m_fDiveTargetYaw_WS) > FMATH_PI)
	{
		m_fDiveTargetYaw_WS += FMATH_2PI;
		FASSERT((m_fMountYaw_WS - m_fDiveTargetYaw_WS) <= FMATH_PI);
	}
	else if ((m_fMountYaw_WS - m_fDiveTargetYaw_WS) < -FMATH_PI)
	{
		m_fDiveTargetYaw_WS -= FMATH_2PI;
		FASSERT((m_fMountYaw_WS - m_fDiveTargetYaw_WS) >= -FMATH_PI);
	}


	//
	// slam velocity to a set amount for diving.
	//
	CFVec3A DiveDir;
	DiveDir.Set(CFVec3A::m_UnitAxisZ);
	DiveDir.RotateY(m_fDiveTargetYaw_WS);
	FMATH_CLAMPMIN(m_Velocity_WS.y,m_pEliteInfo_Dive->fMinVerticalVel);
	FMATH_CLAMPMIN(m_Velocity_MS.y,m_pEliteInfo_Dive->fMinVerticalVel);
    m_Velocity_WS.y += m_pEliteInfo_Dive->fBoostVertical;
	m_Velocity_MS.y += m_pEliteInfo_Dive->fBoostVertical;
    m_Velocity_WS.x = (m_pBotInfo_Walk->fMaxXlatVelocity + m_pEliteInfo_Dive->fBoostForward) * DiveDir.x;
	m_Velocity_MS.x = (m_pBotInfo_Walk->fMaxXlatVelocity + m_pEliteInfo_Dive->fBoostForward) * DiveDir.x;
	m_Velocity_WS.z = (m_pBotInfo_Walk->fMaxXlatVelocity + m_pEliteInfo_Dive->fBoostForward) * DiveDir.z;
	m_Velocity_MS.z = (m_pBotInfo_Walk->fMaxXlatVelocity + m_pEliteInfo_Dive->fBoostForward) * DiveDir.z;

	VelocityHasChanged();

	
	//prepare animations for diving state
	m_fLegsYaw_MS = 0.0f;
	SetControlValue(ANIMCONTROL_JUMP_TUCK_STAFF,0.0f);
	SetControlValue(ANIMCONTROL_JUMP_TUCK,0.0f);
	SetControlValue(ANIMCONTROL_JUMP_UNTUCK,0.0f);
	ZeroTime(ANIMTAP_JUMP_TUCK_STAFF);
	m_nJumpState = BOTJUMPSTATE_DIVE_ENTER;
	SetCameraOverride(TRUE,m_pEliteInfo_Dive->fCameraOverrideEnterTime,FALSE,m_vecDiveCameraLookat,m_vecDiveCameraPos);
	
//		vTempDiveBall.Mul(rDiveVec,EstimatedDiveDistance()).Add(m_MtxToWorld.m_vPos);
//		fTempDiveTicker = (float) ((double)FLoop_nTotalLoopTicks*(double)FLoop_fSecsPerTick);

}


void CBotEliteGuard::_StartDoubleJump( BOOL bAddToVelocityY ) {
	if ( !HaveTimeToDoubleJump() ) 
	{
		return;
	}

	StartDoubleJumpGeneric(bAddToVelocityY,
							ANIMTAP_JUMP_TUCK,
							ANIMTAP_JUMP_UNTUCK,
							ANIMCONTROL_JUMP_LAUNCH,
							ANIMCONTROL_JUMP_FLY,
							ANIMCONTROL_JUMP_TUCK);

	StopAiming();
}


void CBotEliteGuard::_JumpLanded( void ) {
	MakeFootImpactDust( TRUE, TRUE, m_pBotInfo_Gen->fUnitDustKickup, &m_FeetToGroundCollImpact.UnitFaceNormal );

	m_nJumpState = BOTJUMPSTATE_NONE;
	ClearJumping();
	m_fFlipPitch = 0.0f;
	m_bPlayLandAnim = TRUE;

	ZeroTime( ANIMTAP_JUMP_LAND_UPPER );

	m_fMaxLandUnitBlend = GetControlValue( ANIMCONTROL_JUMP_FLY );
	if( m_fMaxLandUnitBlend == 0.0f )	{
		m_fMaxLandUnitBlend = 1.0f;
	}

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, m_fMaxLandUnitBlend );
	SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, m_fMaxLandUnitBlend );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_TUCK, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_UNTUCK, 0.0f );


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
		{			// Don't play scuff sounds when landing from a jump
			fsndfx_Play3D(
				m_pBotInfo_Sound->ahStepSound[m_nSurfaceTypeOn][ fmath_RandomRange( 0, ( BOTSOUND_SCUFF_SOUND_START - 1 ) ) ],
				&MtxToWorld()->m_vPos,
				m_pBotInfo_Walk->fJumpLandedSoundRadius_3D,
				1.0f,
				m_pBotInfo_Walk->fJumpLandedSoundUnitVolume_3D
				);
		}
	}

	if( m_fSpeedXZ_WS < m_pBotInfo_Walk->fMinRunBlendVelocity )//m_pBotInfo_Walk->fMinWalkVelocity ) 
	{
		SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, m_fMaxLandUnitBlend );
		m_bPlayLandAnimLower = TRUE;
		m_fMaxLandUnitBlendLower = 1.0f;
	}
	else
	{
		// from Grunt
		m_fMaxLandUnitBlendLower = 1.0f - fmath_Div( m_fSpeedXZ_WS - m_pBotInfo_Walk->fMinRunBlendVelocity, m_pBotInfo_Walk->fMinRunBlendVelocity );
		FMATH_CLAMP_UNIT_FLOAT( m_fMaxLandUnitBlendLower );
		SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, m_fMaxLandUnitBlendLower );
		m_bPlayLandAnimLower = TRUE;
		//

		// from Titan
		//m_bPlayLandAnimLower	= TRUE;
		//m_fMaxLandUnitBlend		= 1.0f - fmath_Div( m_fSpeedXZ_WS - m_pBotInfo_Walk->fMinWalkVelocity, m_pBotInfo_Walk->fMaxXlatVelocity );
		//FMATH_CLAMPMAX( m_fMaxLandUnitBlend, m_fUnitFlyBlend );
		//SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, m_fMaxLandUnitBlend );
	}

	if( m_pStickyEntity && m_pStickyEntity->GetVerlet() )
	{
		CFVec3A ImpulseVec_WS;

		ImpulseVec_WS.Set( 0.0f, -m_pBotInfo_Gen->fPhysForce_Land, 0.0f );
		m_pStickyEntity->GetVerlet()->ApplyImpulse( &m_MtxToWorld.m_vPos, &ImpulseVec_WS );
	}
}


void CBotEliteGuard::_HandleJumpAnimations( void ) {
	f32 fTemp, fUnitTime;

	if ( IsDiveJumping())	{
		 return _HandleDiveAnimations();
	}

	if( m_nState == STATE_GROUND ) {
		// We're on the ground...
		if( m_fUnitFlyBlend > 0.0f ) {
			m_fUnitFlyBlend = 0.0f;
			_JumpLanded();
			FMATH_CLEARBITMASK( m_nBotFlags2, BOTFLAG2_CAN_DOUBLE_JUMP );
		}

		// Handle landing animation...
		if( m_bPlayLandAnim ) {
			if( m_fSpeedXZ_WS > m_pBotInfo_Walk->fMinRunBlendVelocity ) {
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
					FMATH_CLAMPMAX( fUnitTime, m_fMaxLandUnitBlendLower );
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

				//m_fUnitJump1Jump2Blend -= m_pBotInfo_Jump->fUntuckToFlyBlendSpeed * FLoop_fPreviousLoopSecs;
				//FMATH_CLAMPMIN( m_fUnitJump1Jump2Blend, 0.0f );
				//SetControlValue( ANIMCONTROL_JUMP_UNTUCK, m_fUnitJump1Jump2Blend );
				
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
						SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );
						SetJumping();
					} else {
						// Time to start untuck animation...
						SetControlValue( ANIMCONTROL_JUMP_TUCK, 0.0f );
						SetControlValue( ANIMCONTROL_JUMP_UNTUCK, 1.0f );
						ZeroTime( ANIMTAP_JUMP_UNTUCK );
						m_nJumpState = BOTJUMPSTATE_UNTUCK;
						SetJumping();
					}
				} else {
					f32 fVal = GetUnitTime( ANIMTAP_JUMP_TUCK ) * 2.0f;
					FMATH_CLAMP_MAX1( fVal );
					SetControlValue( ANIMCONTROL_JUMP_TUCK, fVal);
					SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );
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
					SetControlValue( ANIMCONTROL_JUMP_UNTUCK, 0.0f );
					ZeroTime( ANIMTAP_JUMP_FLY );
					m_fFlipPitch = 0.0f;
					m_fUnitJump1Jump2Blend = 1.0f;
					m_nJumpState = BOTJUMPSTATE_AIR;
					SetJumping();
				} else {
					SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );
					f32 fVal = (1.0f - GetUnitTime( ANIMTAP_JUMP_UNTUCK )) * 4.0f;
					FMATH_CLAMP_MAX1( fVal );
					SetControlValue( ANIMCONTROL_JUMP_UNTUCK, fVal );
					
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
	
 /*
	if (m_nJumpState == BOTJUMPSTATE_LAUNCH) {
//
//		if( !DeltaTime( ANIMTAP_JUMP_LAUNCH ) )
//		{
//			// means that at 1/3d pt. of animation, blend is at 100%	
//			f32 fUnitTime = 3.0f * fmath_UnitLinearToSCurve( GetUnitTime( ANIMTAP_JUMP_LAUNCH ) );
//			FMATH_CLAMPMAX( fUnitTime, 1.0f );
//			SetControlValue( ANIMCONTROL_JUMP_LAUNCH, fUnitTime );
//		}
//
		if(!DeltaTime( ANIMTAP_JUMP_LAUNCH, FLoop_fPreviousLoopSecs, TRUE )) 
		{
			m_fUnitFlyBlend = GetControlValue(ANIMCONTROL_JUMP_LAUNCH);
			// if launches plays from 0-1, we get full control at midpoint, from .80-1 it's at 40%
			m_fUnitFlyBlend  = m_fUnitFlyBlend + 5.0f * GetOOTotalTime(ANIMTAP_JUMP_LAUNCH) * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMAX( m_fUnitFlyBlend, 1.0f );
			SetControlValue( ANIMCONTROL_JUMP_LAUNCH, m_fUnitFlyBlend );
			FASSERT_UNIT_FLOAT( m_fUnitFlyBlend );
		}
 		else
		{
			m_Velocity_WS.y += m_pBotInfo_Jump->fVerticalVelocityJump1 * m_fJumpMultiplier;
			m_Velocity_MS.y += m_pBotInfo_Jump->fVerticalVelocityJump1 * m_fJumpMultiplier;
			SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
			SetControlValue( ANIMCONTROL_JUMP_FLY, m_fUnitFlyBlend );
			ZeroTime( ANIMTAP_JUMP_FLY );
			m_nJumpState = BOTJUMPSTATE_AIR;
			SetJumping();
		}
	}

	if( m_nState == STATE_GROUND )
	{
		// We're on the ground...
//		if( m_fUnitFlyBlend > 0.0f )
//		{
//			m_fUnitFlyBlend = 0.0f;
//			_JumpLanded();
//		}

		// Handle landing animation...
		if( m_bPlayLandAnim )
		{
			f32 fTemp;
			if( m_fSpeedXZ_WS > 0.0f )
			{
				fTemp = m_pBotInfo_Jump->fMovingLandAnimSpeedMult;
			}
			else
			{
				fTemp = 1.0f;
			}
			
			if(!DeltaTime( ANIMTAP_JUMP_LAND_UPPER, FLoop_fPreviousLoopSecs * fTemp ))
			{
				// Still playing landing animation...

				f32 fUnitTime		= 1.01f - GetUnitTime(ANIMTAP_JUMP_LAND_UPPER);
				f32 fUnitTimeSmooth = fmath_UnitLinearToSCurve( fUnitTime );
				f32 fUnitTimeCapped = m_fMaxLandUnitBlend * fUnitTimeSmooth;
				SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, fUnitTimeSmooth );
				if( m_bPlayLandAnimLower )
				{
					SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, fUnitTimeCapped );
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
				SetControlValue( ANIMCONTROL_JUMP_TUCK, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_UNTUCK, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_TUCK_STAFF, 0.0f);
			}
		}
		else
		{
			if( m_nJumpState == BOTJUMPSTATE_NONE )
			{
				SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_TUCK, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_UNTUCK, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_TUCK_STAFF, 0.0f);
			}
		}
	}
	else
	{
		// We're in the air...
		if( m_nJumpState != BOTJUMPSTATE_NONE )
		{
			if( m_nJumpState==BOTJUMPSTATE_TUCK || m_nJumpState==BOTJUMPSTATE_FLIPPING || m_nJumpState==BOTJUMPSTATE_UNTUCK )
			{
				m_fFlipPitch += m_fFlipPitchVelocity * FLoop_fPreviousLoopSecs;
			}
			switch( m_nJumpState )
			{
			case BOTJUMPSTATE_LAUNCH:
				if( !DeltaTime( ANIMTAP_JUMP_LAUNCH ) )
				{
					// means that at 1/3d pt. of animation, blend is at 100%	
					f32 fUnitTime = 3.0f * fmath_UnitLinearToSCurve( GetUnitTime( ANIMTAP_JUMP_LAUNCH ) );
					FMATH_CLAMPMAX( fUnitTime, 1.0f );
					SetControlValue( ANIMCONTROL_JUMP_LAUNCH, fUnitTime );
				}
				else
				{
					SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
					SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );
					ZeroTime( ANIMTAP_JUMP_FLY );
					m_nJumpState = BOTJUMPSTATE_AIR;
					SetJumping();
				}
				break;

			case BOTJUMPSTATE_AIR:
				DeltaTime( ANIMTAP_JUMP_FLY );
				m_fUnitFlyBlend = GetControlValue(ANIMCONTROL_JUMP_FLY);
				// if control less than 1.0f, make sure it's 100% before the anim plays out
				m_fUnitFlyBlend  = m_fUnitFlyBlend + 5.0f * GetOOTotalTime(ANIMTAP_JUMP_FLY) * FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMAX( m_fUnitFlyBlend, 1.0f );
				SetControlValue( ANIMCONTROL_JUMP_FLY, m_fUnitFlyBlend );

				m_fUnitJump1Jump2Blend -= m_pBotInfo_Jump->fUntuckToFlyBlendSpeed * FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMIN( m_fUnitJump1Jump2Blend, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_UNTUCK, m_fUnitJump1Jump2Blend );
				if( CanAttachToZipLine() ) 
				{
					CFSphere Sphere;
					Sphere.m_Pos = (*m_pLeftHandSecondaryFireBonePos).v3;
					Sphere.m_fRadius = FMATH_MAX(m_fCollCylinderHeight_WS, m_fCollCylinderRadius_WS);

					CEZipLine *pZipHit = CEZipLine::GetZipCollision( Sphere );

					if( pZipHit ) 
					{
						GrabCable( pZipHit );
					}
				}

				break;

			case BOTJUMPSTATE_TUCK:
				if( DeltaTime( ANIMTAP_JUMP_TUCK, FLoop_fPreviousLoopSecs * m_fTuckUntuckAnimTimeMult, TRUE ) )
				{
					// Tuck animation completed...

					m_fDiveYaw_WS  = m_fDiveTargetYaw_WS;

					if( m_fUntuckStartCountdownSecs > 0.0f )
					{
						m_nJumpState = BOTJUMPSTATE_FLIPPING;
						SetJumping();
					} else
					{
						// Time to start untuck animation...
						SetControlValue( ANIMCONTROL_JUMP_TUCK, 0.0f );
						SetControlValue( ANIMCONTROL_JUMP_UNTUCK, 1.0f );
						ZeroTime( ANIMTAP_JUMP_UNTUCK );
						m_nJumpState = BOTJUMPSTATE_UNTUCK;
						SetJumping();
					}
				}
				else
				{
					m_fDiveYaw_WS = FMATH_FPOT(GetUnitTime(ANIMTAP_JUMP_TUCK), m_fMountYaw_WS + m_fLegsYaw_MS, m_fDiveTargetYaw_WS);
				}

				break;

			case BOTJUMPSTATE_FLIPPING:
				m_fUntuckStartCountdownSecs -= FLoop_fPreviousLoopSecs;

				if( m_fUntuckStartCountdownSecs <= 0.0f )
				{
					// Time to start untuck animation...
					SetControlValue( ANIMCONTROL_JUMP_TUCK, 0.0f );
					SetControlValue( ANIMCONTROL_JUMP_UNTUCK, 1.0f );
					ZeroTime( ANIMTAP_JUMP_UNTUCK );
					m_nJumpState = BOTJUMPSTATE_UNTUCK;
					SetJumping();
				}

				break;

			case BOTJUMPSTATE_UNTUCK:
				if( DeltaTime( ANIMTAP_JUMP_UNTUCK, FLoop_fPreviousLoopSecs * m_fTuckUntuckAnimTimeMult, TRUE ) )
				{
					// Untuck animation completed...
					m_fDiveYaw_WS  = m_fDiveTargetYaw_WS;
					SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );
					ZeroTime( ANIMTAP_JUMP_FLY );
					m_fFlipPitch = 0.0f;
					m_fUnitJump1Jump2Blend = 1.0f;
					m_nJumpState = BOTJUMPSTATE_AIR;
					SetJumping();
				}
				else
				{
					m_fDiveYaw_WS = FMATH_FPOT(GetUnitTime(ANIMTAP_JUMP_UNTUCK), m_fDiveTargetYaw_WS,m_fMountYaw_WS + m_fLegsYaw_MS);
				}
				break;
				///////////////////////////////////////
				///////////////////////////////////////

			case BOTJUMPSTATE_CABLE:
				DeltaTime( ANIMTAP_JUMP_FLY );
				DeltaTime( ANIMTAP_CABLE_GRASP, FLoop_fPreviousLoopSecs, TRUE );

				if( m_fUnitCableBlend < 1.0f )
				{
					m_fUnitCableBlend = 1.0f;
					SetControlValue( ANIMCONTROL_CABLE_GRASP, 1.0f );
				}
				break;

			default:
				FASSERT_NOW;
			}
		}
	}
*/

}

void CBotEliteGuard::_HandleDiveAnimations(void)
{

//	CFColorRGBA cyan,blue,green,red,yello;
//	cyan.OpaqueCyan();
//	blue.OpaqueBlue();
//	green.OpaqueGreen();
//	red.OpaqueRed();
//	yello.OpaqueYellow();

	CFVec3A Step;
    Step.Sub(m_MtxToWorld.m_vPos, m_vPreviousPos);
	m_vPreviousPos = m_MtxToWorld.m_vPos;

	CFVec3A Head;
	CFVec3A Gnd;
	Head.Set(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexGroin]->m_vPos);
	Gnd.Set(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexGroin]->m_vFront);
	const f32  fLineLength = 100.f;
	Gnd.Mul(fLineLength);
	Gnd.Add(Head);
	f32 fInvStepSize;
	if (Step.MagSq() > 0.0004f)
	{
		fInvStepSize = Step.InvMag();
	}
	else 
	{
		fInvStepSize = 9999.f;
	}
/*	
	Head.Set(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexGroin]->m_vPos);
	f32 fInvStepSize;
	if (Step.MagSq() > 0.0004f)
	{
		fInvStepSize = Step.InvMag();
		Gnd.Mul(Step,fInvStepSize);
	}
	else 
	{
		fInvStepSize = 9999.f;
		Gnd.x = 0.0f;
		Gnd.y = -1.0f;
		Gnd.z = 0.0f;
	}
	const f32  fLineLength = 100.f;
	Gnd.Mul(fLineLength);
	Gnd.Add(Head);
*/
//	fdraw_DevLine(&Head,&Gnd, &blue, &cyan);
	f32 fFlipPitchDeg =0.f;
	f32 fBlendAmount = 0.f;
	switch (m_nJumpState)
	{
	case BOTJUMPSTATE_DIVE_ENTER:
		fBlendAmount = GetControlValue(ANIMCONTROL_JUMP_TUCK_STAFF) + m_pEliteInfo_Dive->fEnterBlendVel * FLoop_fPreviousLoopSecs;
		FMATH_CLAMP_UNIT_FLOAT(fBlendAmount);
		SetControlValue(ANIMCONTROL_JUMP_TUCK_STAFF, fBlendAmount);
		DeltaTime(ANIMTAP_JUMP_TUCK_STAFF, FLoop_fPreviousLoopSecs * m_pEliteInfo_Dive->fEnterAnimVel, TRUE);
		
		m_fFlipPitch += m_pEliteInfo_Dive->fEnterPitchVel * FLoop_fPreviousLoopSecs;
		fFlipPitchDeg = FMATH_RAD2DEG(m_fFlipPitch);
		if (fFlipPitchDeg > m_pEliteInfo_Dive->fAngleExitEnter)
		{
			m_nJumpState = BOTJUMPSTATE_DIVE_FLY;
			SetControlValue(ANIMCONTROL_JUMP_TUCK_STAFF, 1.f);
			m_fDiveYaw_WS = m_fDiveTargetYaw_WS;
		}
		m_fDiveYaw_WS = FMATH_FPOT( fmath_Div(fFlipPitchDeg,m_pEliteInfo_Dive->fAngleExitEnter), m_fMountYaw_WS + m_fLegsYaw_MS , m_fDiveTargetYaw_WS);
//		fdraw_DevLine(&Head,&Gnd,&yello,&yello);
		break;

	case BOTJUMPSTATE_DIVE_FLY:
		{
			f32 fFlyPitch = m_pEliteInfo_Dive->fFlyPitchVelMin;;
			const u32 nTrackerSkipCount = 1;
			const CFWorldTracker* pSkipList[nTrackerSkipCount] = {m_pWorldMesh};
			FCollImpact_t CollImpact;
			f32 fNumSteps = 0.f;
			if( !fworld_FindClosestImpactPointToRayStart( &CollImpact, &Head, &Gnd,nTrackerSkipCount,(const CFWorldTracker * const *)&pSkipList) )
			{
				// Our ray didn't hit anything...
				if (fFlipPitchDeg >= m_pEliteInfo_Dive->fAngleExitFly) 
				{
					SetControlValue(ANIMCONTROL_JUMP_TUCK, 0.f);
					ZeroTime(ANIMTAP_JUMP_TUCK);
					m_nJumpState = BOTJUMPSTATE_DIVE_TUCK;
				}
			} 
			else 
			{
				// Our ray hit something...
				Gnd.Set(CollImpact.ImpactPoint);
//				fdraw_DevSphere(&Gnd,.45f,&red);
				f32 fDistanceToGround;
				fDistanceToGround = Head.Dist(Gnd);
				fNumSteps = fDistanceToGround * fInvStepSize;
				fFlyPitch = m_pEliteInfo_Dive->fFlyPitchVelMax;
				fFlipPitchDeg = FMATH_RAD2DEG(m_fFlipPitch);
					if ( ((m_Velocity_WS.y < 0.0f) && (fNumSteps < m_pEliteInfo_Dive->fTuckDistance))
						|| (fFlipPitchDeg >= m_pEliteInfo_Dive->fAngleExitFly) )
					{
						SetControlValue(ANIMCONTROL_JUMP_TUCK, 0.f);
						ZeroTime(ANIMTAP_JUMP_TUCK);
						m_nJumpState = BOTJUMPSTATE_DIVE_TUCK;
					}
			}
		m_fFlipPitch += fFlyPitch * FLoop_fPreviousLoopSecs;
		//FMATH_CLAMPMAX(m_fFlipPitch, FMATH_DEG2RAD(m_pEliteInfo_Dive->fAngleExitFly));
		}
//		fdraw_DevLine(&Head,&Gnd,&red,&red);
		break;

	case BOTJUMPSTATE_DIVE_TUCK:
		{
		DeltaTime(ANIMTAP_JUMP_TUCK, FLoop_fPreviousLoopSecs * m_pEliteInfo_Dive->fTuckAnimVel, TRUE);
		fBlendAmount = GetControlValue(ANIMCONTROL_JUMP_TUCK) + m_pEliteInfo_Dive->fTuckBlendVel * FLoop_fPreviousLoopSecs;
		FMATH_CLAMP_UNIT_FLOAT(fBlendAmount);
		SetControlValue(ANIMCONTROL_JUMP_TUCK, fBlendAmount);
		
		f32 fTuckPitchVel = FMATH_FPOT(fBlendAmount,m_pEliteInfo_Dive->fTuckPitchVel,m_pEliteInfo_Dive->fLandPitchVel);
		m_fFlipPitch += fTuckPitchVel * FLoop_fPreviousLoopSecs;
		if (FMATH_RAD2DEG(m_fFlipPitch) > m_pEliteInfo_Dive->fAngleExitTuck)
		{
			SetControlValue(ANIMCONTROL_JUMP_TUCK, 0.f);
			SetControlValue(ANIMCONTROL_JUMP_UNTUCK, 1.f);
			ZeroTime(ANIMTAP_JUMP_UNTUCK);
			m_nJumpState = BOTJUMPSTATE_DIVE_LAND;
		}
//		fdraw_DevLine(&Head,&Gnd,&green,&green);
		break;
		}
	case BOTJUMPSTATE_DIVE_LAND:
		{
		f32 fLandPitchVel = FMATH_FPOT(FMATH_RAD2DEG(m_fFlipPitch)/m_pEliteInfo_Dive->fAngleExitLand,m_pEliteInfo_Dive->fLandPitchVel,m_pEliteInfo_Dive->fExitPitchVel);
		m_fFlipPitch += fLandPitchVel * FLoop_fPreviousLoopSecs;
		if (FMATH_RAD2DEG(m_fFlipPitch) > m_pEliteInfo_Dive->fAngleExitLand)
		{
			m_nJumpState = BOTJUMPSTATE_DIVE_EXIT;
			SetControlValue(ANIMCONTROL_JUMP_TUCK_STAFF,0.0f);
			SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
			SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
			SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
			SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );

			if ((m_fMountYaw_WS + m_fLegsYaw_MS - m_fDiveTargetYaw_WS) > FMATH_PI)
			{
				m_fDiveTargetYaw_WS += FMATH_2PI;
				FASSERT((m_fMountYaw_WS + m_fLegsYaw_MS - m_fDiveTargetYaw_WS) <= FMATH_PI);
			}
			else if ((m_fMountYaw_WS + m_fLegsYaw_MS - m_fDiveTargetYaw_WS) < -FMATH_PI)
			{
				m_fDiveTargetYaw_WS -= FMATH_2PI;
				FASSERT((m_fMountYaw_WS + m_fLegsYaw_MS - m_fDiveTargetYaw_WS) >= -FMATH_PI);
			}
		}
//		fdraw_DevLine(&Head,&Gnd,&cyan,&cyan);
		break;
		}
	case BOTJUMPSTATE_DIVE_EXIT:
		m_fFlipPitch += m_pEliteInfo_Dive->fExitPitchVel * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMAX(m_fFlipPitch, m_pEliteInfo_Dive->fAngleExitExit);
        if (DeltaTime(ANIMTAP_JUMP_UNTUCK, FLoop_fPreviousLoopSecs * m_pEliteInfo_Dive->fExitAnimVel, TRUE))
		{
			SetControlValue( ANIMCONTROL_JUMP_TUCK, 0.0f );
			m_fFlipPitch = 0.0f;
			UpdateUnitTime(ANIMTAP_STAND, 45.0f / 150.0f);
			ClearJumping();
			m_nJumpState = BOTJUMPSTATE_AIR;
			SetCameraOverride(FALSE,m_pEliteInfo_Dive->fCameraOverrideExitTime,FALSE);
//			f32 fDiveTime = (float) ((double)FLoop_nTotalLoopTicks*(double)FLoop_fSecsPerTick) - fTempDiveTicker;
//			DEVPRINTF("% 2.4f secs\n",fDiveTime);
		}
		else
		{
			fBlendAmount = m_pEliteInfo_Dive->fExitBlendVel * fmath_PositiveCos( FMATH_PI * (GetUnitTime(ANIMTAP_JUMP_UNTUCK) ));
			FMATH_CLAMP_UNIT_FLOAT(fBlendAmount);
			SetControlValue(ANIMCONTROL_JUMP_UNTUCK, fBlendAmount);
			UpdateUnitTime(ANIMTAP_STAND, 45.0f / 150.0f);
		}
		
		m_fDiveYaw_WS = FMATH_FPOT( GetUnitTime(ANIMTAP_JUMP_UNTUCK), m_fDiveTargetYaw_WS, m_fMountYaw_WS + m_fLegsYaw_MS );
//		fdraw_DevLine(&Head,&Gnd,&blue,&blue);
		break;
	
	default:
		FASSERT_NOW;
		break;
	}
}

void CBotEliteGuard::_HandleStaffWork( void )
{
	if( IsSwinging() ) {
		CFVec3A vMeleeAttackPos;

		_CalcMeleeAttackPos( 0, &vMeleeAttackPos );
		MeleeSwing( &m_MeleeData, &vMeleeAttackPos );

		_CalcMeleeAttackPos( 1, &vMeleeAttackPos );
		MeleeSwing( &m_MeleeData2, &vMeleeAttackPos );
	}

	/*
	if (m_vFirePosPrev.DistSq(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]->m_vPos) > 0.01f)
	{
		m_vFireUnitDir.Set(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]->m_vPos);
		m_vFireUnitDir.Sub(m_vFirePosPrev);
		m_vFireUnitDir.Unitize();
		m_vFirePosPrev.Set(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]->m_vPos);
	}

	CFVec3A vStaffCollSphereWS(m_StaffCollSphereWS.m_Pos);
//	fdraw_DevSphere(&vStaffCollSphereWS.v3,m_StaffCollSphereWS.m_fRadius);
	
	CFVec3A vStaffPos;
	vStaffPos.Set( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]->m_vPos);

	CFColorRGBA red;red.OpaqueRed();
	CFColorRGBA white;white.OpaqueWhite();
	CFColorRGBA blue;blue.OpaqueBlue();
	CFColorRGBA yellow;yellow.OpaqueYellow();
	CFVec3A vPrimaryFirePos;
	vPrimaryFirePos.Set( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]->m_vPos);

//	fdraw_DevSphere(&vPrimaryFirePos,1.0f,&blue,6,4,6);
//	fdraw_DevSphere(&m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexStaff]->m_vPos,1.0f,&blue,6,4,6);
//	CFVec3A vStaffEnd;
//	vStaffEnd.Mul(m_vFireUnitDir,5.0f).Add(vStaffPos);
//	fdraw_DevLine(&vStaffPos,&vStaffEnd,&blue);

	if (!IsSwinging())
	{
		// no swinging means no swinging collision
		return;
	}
	
	m_nHitEntityCount = 0; // haven't connected yet
	_pThisBot = this;
	FWorld_nTrackerSkipListCount = 0;
	this->AppendTrackerSkipList();
	fworld_FindTrackersIntersectingSphere(	&m_StaffCollSphereWS,
											FWORLD_TRACKERTYPE_MESH,
											CBotEliteGuard::_SwingStrikeCallback,
											FWorld_nTrackerSkipListCount, 
											FWorld_apTrackerSkipList,										
											m_pWorldMesh,
											MESHTYPES_ENTITY,
											ENTITY_BIT_BOT|ENTITY_BIT_WEAPON|ENTITY_BIT_MESHENTITY);
	_pThisBot = NULL;
	*/
}

void CBotEliteGuard::UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup )
{
/*	
	static const u8 *__apnUserAnimBoneMaskIndex[UABONEMASK_COUNT] =
	{
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
*/
}


void CBotEliteGuard::_HandleFiringAnimations( void ) 
{
	u32 uFireAnimControlId;
	u32 uFireAnimTapId;

	if( IsAlertOn() ) {
		uFireAnimControlId = ANIMCONTROL_FIRE_UPPER;
		uFireAnimTapId = ANIMTAP_FIRE_UPPER;

	} else {
		uFireAnimControlId = ANIMCONTROL_FIRE_STAFF;
		uFireAnimTapId = ANIMTAP_FIRE_STAFF;
	}

	if( m_nBotFlags & BOTFLAG_PLAY_FIRE1_ANIM ) {
		SetControlValue( uFireAnimControlId, 1.0f );
		m_fAttackAimTimer = _ATTACK_AIM_TIME;
		
		if( DeltaTime( uFireAnimTapId, FLoop_fPreviousLoopSecs, TRUE ) ) {
			UpdateUnitTime( uFireAnimTapId, 0.0f );
			FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM );
		}
	} else {
		m_fAttackAimTimer -= FLoop_fPreviousLoopSecs;
		if( m_fAttackAimTimer < 0.0f ) {
			f32 fVal = GetControlValue( ANIMCONTROL_FIRE_STAFF );
			if( fVal > 0.0f ) {
				fVal -= FLoop_fPreviousLoopSecs;
				FMATH_CLAMP_MIN0( fVal );
				SetControlValue( ANIMCONTROL_FIRE_STAFF, fVal );
			}

			fVal = GetControlValue( ANIMCONTROL_FIRE_UPPER );
			if( fVal > 0.0f ) {
				fVal -= FLoop_fPreviousLoopSecs;
				FMATH_CLAMP_MIN0( fVal );
				SetControlValue( ANIMCONTROL_FIRE_UPPER, fVal );
			}

		}
	}
	
	

	//if (IsAlertOn())
	//{

	//	if( m_nBotFlags & BOTFLAG_PLAY_FIRE1_ANIM )
	//	{
	//		SetControlValue( uFireAnimControlId, 1.0f );

	//		if( DeltaTime( uFireAnimTapId, FLoop_fPreviousLoopSecs, TRUE ) )
	//		{
	//			FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM );
	////			ZeroTime( uFireAnimTapId );
	//		}
	//	}
	//}

	//if( !(m_nBotFlags & BOTFLAG_PLAY_FIRE1_ANIM ))
	//{
	//	SetControlValue( uFireAnimTapId, 0.0f);
	//}


}

void CBotEliteGuard::_HandleAimAnimations( void )
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

	// Apply head and torso quaternions...
	m_AnimManFrameAim.UpdateFrame( BONE_HEAD, HeadQuat );
	m_AnimManFrameAim.UpdateFrame( BONE_TORSO, TorsoQuat );
	// Compute the head quaternion...
}


void CBotEliteGuard::_UpdateMatrices( void ) {
	CFMtx43A FlipMtx, MeshMtx, NewMtx, EntityMtxToWorld;

	CFMtx43A::m_XlatRotY.SetRotationY( m_fMountYaw_WS + m_fLegsYaw_MS );

	CFMtx43A::m_XlatRotY.m_vPos = m_MountPos_WS;

//	f32 fDelta = m_pBotInfo_MountAim->fMountYawBaseRevsPerSec * FLoop_fPreviousLoopSecs * FMATH_2PI;
	
	// Update xfm...
	//	floop_SetTimeScale(0.2f); 
	if (m_fFlipPitch)
	{
//		floop_SetTimeScale(m_pEliteInfo_Dive->fTimeScale); 
		if( IsDiveJumping() ) 
		{


			m_fLegsYaw_MS = (0.f - m_fLegsYaw_MS)*0.25f;  //while diving, legs yaw approaches zero rapidly


			if (m_fFlipPitch < FMATH_HALF_PI)
			{
				CFMtx43A::m_XlatRotX.m_vPos.Set( 0.0f,2.0f,m_fFlipPitch/FMATH_HALF_PI*6.0f);//m_pBotInfo_Jump->fFlipOriginX, m_pBotInfo_Jump->fFlipOriginY, m_pBotInfo_Jump->fFlipOriginZ );
			}
			else if (m_fFlipPitch < FMATH_PI)
			{
				CFMtx43A::m_XlatRotX.m_vPos.Set( 0.0f,2.0f,((FMATH_PI-m_fFlipPitch)/FMATH_HALF_PI)*6.0f);//m_pBotInfo_Jump->fFlipOriginX, m_pBotInfo_Jump->fFlipOriginY, m_pBotInfo_Jump->fFlipOriginZ );
			}
			else
			{
				CFMtx43A::m_XlatRotX.m_vPos.Set( 0.0f,2.0f,0.0f);//m_pBotInfo_Jump->fFlipOriginX, m_pBotInfo_Jump->fFlipOriginY, m_pBotInfo_Jump->fFlipOriginZ );
			}

			CFMtx43A::m_XlatRotY.SetRotationY( m_fDiveYaw_WS );
			FASSERT(CFMtx43A::m_XlatRotY.m_vUp.y == 1.0f);
			if( m_fFlipPitch >= 0.0f ) 
			{
				CFMtx43A::m_XlatRotX.SetRotationX( fmath_UnitLinearToSCurve(m_fFlipPitch * (1.0f/FMATH_2PI))  * FMATH_2PI );
			}
			else 
			{
				CFMtx43A::m_XlatRotX.SetRotationX( -fmath_UnitLinearToSCurve(m_fFlipPitch * (1.0f/FMATH_2PI)) * FMATH_2PI );
			}
			FASSERT(CFMtx43A::m_XlatRotX.m_vRight.x == 1.0f);

			CFMtx43A::m_Xlat.m_vPos.ReceiveNegative( CFMtx43A::m_XlatRotX.m_vPos );

			FlipMtx.Mul( CFMtx43A::m_XlatRotX, CFMtx43A::m_Xlat );

			MeshMtx.Mul( CFMtx43A::m_XlatRotY, FlipMtx );
			m_pWorldMesh->m_Xfm.BuildFromMtx( MeshMtx );
		} 
		else
		{

			//regular double jump code that most all bots with flips in their double jumps use
			CFMtx43A::m_XlatRotX.m_vPos.Set( m_pBotInfo_Jump->fFlipOriginX, m_pBotInfo_Jump->fFlipOriginY, m_pBotInfo_Jump->fFlipOriginZ );

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

	}
	else if( m_pCableHook ) // We are on a zipline
	{
		CFMtx43A RotMtx;

		ZipLineVerletWork( m_pLeftHandSecondaryFireBonePos, &RotMtx );

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
		m_pCollBot = this;
		m_Anim.m_pAnimCombiner->ComputeMtxPalette( TRUE );
		m_pCollBot = NULL;
	PROTRACK_ENDBLOCK();//"ComputeMtxPal");

	if( (m_pPartMgr == NULL) || !m_pPartMgr->IsCreated() || (m_pPartMgr->GetLimbState( LIMB_TYPE_HEAD ) == CBotPartMgr::LIMB_STATE_INTACT) ) {
		m_GazeUnitVec_WS.ReceiveUnit( *m_pGazeDir_WS );
	} else {
		m_GazeUnitVec_WS = m_MtxToWorld.m_vFront;
	}

	EntityMtxToWorld.Identity();
	EntityMtxToWorld.SetRotationYXZ( m_fMountYaw_WS, m_fMountPitch_WS, 0.0f );
	EntityMtxToWorld.m_vPos = m_MountPos_WS;
	if (!m_pCurMech)
	{
		Relocate_RotXlatFromUnitMtx_WS( &EntityMtxToWorld, TRUE, m_pMoveIdentifier );
	}
}


void CBotEliteGuard::_HandleStaffAnimations( void )
{
	BOOL bTryToSwing = FALSE;
	BOOL bStaffTriggerDown  = FALSE;
	m_bFireLaserThisFrame = FALSE;
	
	if ( IsDiveJumping() && 
		(m_nJumpState != BOTJUMPSTATE_DIVE_EXIT) )
	{
		if (m_eStaffSwingState != STAFF_SWING_NONE)
		{
			SetControlValue(ANIMCONTROL_SWING_CHARGE_UPPER,0.0f);
			SetControlValue(ANIMCONTROL_SWING_FORWARD_UPPER,0.0f);		
			SetControlValue(ANIMCONTROL_SWING_BACK_UPPER,0.0f);			
			SetControlValue(ANIMCONTROL_SWING_DONE_UPPER,0.0f);			
			SetControlValue(ANIMCONTROL_SWING_CHARGE_LOWER,0.0f);			
			SetControlValue(ANIMCONTROL_SWING_FORWARD_LOWER,0.0f);		
			SetControlValue(ANIMCONTROL_SWING_BACK_LOWER,0.0f);			
			SetControlValue(ANIMCONTROL_SWING_DONE_LOWER,0.0f);	
			if( CFXStreamerEmitter::IsValidHandle( m_hStreamerEmitter ) )
			{
				CFXStreamerEmitter::ReturnEmitterHandle(m_hStreamerEmitter);
				m_hStreamerEmitter = FXSTREAMER_INVALID_HANDLE;
			} 
			_PlaySnd(ELITESND_SWIPEATTACK, FALSE );
			m_eStaffSwingState = STAFF_SWING_NONE;
		}
		if (m_eStaffFireState != STAFF_FIRE_NONE)
		{
			SetControlValue(ANIMCONTROL_FIRE_CHARGE_UPPER,0.0f);				
			SetControlValue(ANIMCONTROL_FIRE_UPPER,0.0f);						
			SetControlValue(ANIMCONTROL_FIRE_DONE_UPPER,0.0f);				
			SetControlValue(ANIMCONTROL_FIRE_CHARGE_LOWER,0.0f);				
			SetControlValue(ANIMCONTROL_FIRE_LOWER,0.0f);						
			SetControlValue(ANIMCONTROL_FIRE_DONE_LOWER,0.0f);
			((CWeaponStaff*)m_apWeapon[0])->ResetUnitCharge();
			m_eStaffFireState = STAFF_FIRE_NONE;
		}
	}
	else
	{
		// button is pressed, and we're not cabling
		if (m_fControls_Fire2 > 0.1f && !m_pCableHook)
		{
			bTryToSwing = TRUE;
		}
		if (!bTryToSwing && (m_fControls_Fire1 > 0.1f))
		{
			bStaffTriggerDown = TRUE;
		}

	}



	switch(m_eStaffSwingState)
	{
	case STAFF_SWING_NONE: // opportunity to get a state
		if (bTryToSwing)
		{
			if ( (m_eStaffFireState != STAFF_FIRE_FIRING) )
			{
				JustFired();  // No more disguise...

				ZeroTime(ANIMTAP_SWING_CHARGE_UPPER);
				m_eStaffSwingState = STAFF_SWING_CHARGING;
			}
		}
		break;

	case STAFF_SWING_CHARGING:
		if( !DeltaTime(ANIMTAP_SWING_CHARGE_UPPER,FLoop_fPreviousLoopSecs * 2.5f,TRUE) )// means we're still charging
		{
			// means that at 1/3d pt. of animation, blend is at 100%
			f32 fUnitTime = 3.0f * fmath_UnitLinearToSCurve( GetUnitTime( ANIMTAP_SWING_CHARGE_UPPER) );
			FMATH_CLAMPMAX( fUnitTime, 1.0f );
			SetControlValue( ANIMCONTROL_SWING_CHARGE_UPPER, fUnitTime);
			SetControlValue( ANIMCONTROL_SWING_CHARGE_LOWER, fUnitTime);
		}
		else
		{
			SetControlValue( ANIMCONTROL_SWING_CHARGE_UPPER, 0.0f);
			SetControlValue( ANIMCONTROL_SWING_CHARGE_LOWER, 0.0f);
			SetControlValue( ANIMCONTROL_SWING_FORWARD_UPPER, 1.0f);
			SetControlValue( ANIMCONTROL_SWING_FORWARD_LOWER, 1.0f);
			ZeroTime(ANIMTAP_SWING_FORWARD_UPPER);
			m_eStaffSwingState = STAFF_SWING_FORWARD;
			_PlaySnd(ELITESND_SWIPEATTACK, TRUE);

			CFVec3A vMeleeAttackPos;
			_CalcMeleeAttackPos( 0, &vMeleeAttackPos );
			MeleeStartAttack( &m_MeleeData, &vMeleeAttackPos );
			
			_CalcMeleeAttackPos( 1, &vMeleeAttackPos );
			MeleeStartAttack( &m_MeleeData2, &vMeleeAttackPos );
		}
		break;

	case STAFF_SWING_FORWARD:
		if( !DeltaTime(ANIMTAP_SWING_FORWARD_UPPER,FLoop_fPreviousLoopSecs * m_pEliteInfo_Staff->fSwingSpeedForward,TRUE))
		{
			f32 fUnitTime = GetUnitTime(ANIMTAP_SWING_FORWARD_UPPER);
			if ( (fUnitTime > m_pEliteInfo_Staff->fSwingSwingForwardStreamerOn) &&
				 (fUnitTime < m_pEliteInfo_Staff->fSwingSwingForwardStreamerFadeOff) )
			{
				_StreamerOn(0.0f);
			}
			else if ( (fUnitTime > m_pEliteInfo_Staff->fSwingSwingForwardStreamerFadeOff) &&
					  (fUnitTime < m_pEliteInfo_Staff->fSwingSwingForwardStreamerOff) )
			{
				f32 fFadeRange = m_pEliteInfo_Staff->fSwingSwingForwardStreamerOff - m_pEliteInfo_Staff->fSwingSwingForwardStreamerFadeOff;
				if (fFadeRange!=0.0f)
				{
					f32 fFade = fmath_Div(fUnitTime-m_pEliteInfo_Staff->fSwingSwingForwardStreamerFadeOff,fFadeRange);
					_StreamerOn(fFade);
				}
			}
			else
				_StreamerOn(1.0f);
		}
		else
		{
			_StreamerOn(1.0f);
			SetControlValue( ANIMCONTROL_SWING_FORWARD_UPPER, 0.0f);
			SetControlValue( ANIMCONTROL_SWING_FORWARD_LOWER, 0.0f);
			SetControlValue( ANIMCONTROL_SWING_BACK_UPPER, 1.0f);
			SetControlValue( ANIMCONTROL_SWING_BACK_LOWER, 1.0f);
			ZeroTime(ANIMTAP_SWING_BACK_UPPER);
			m_eStaffSwingState = STAFF_SWING_BACK;

			CFVec3A vMeleeAttackPos;
			_CalcMeleeAttackPos( 0, &vMeleeAttackPos );
			MeleeStartAttack( &m_MeleeData, &vMeleeAttackPos );

			_CalcMeleeAttackPos( 1, &vMeleeAttackPos );
			MeleeStartAttack( &m_MeleeData2, &vMeleeAttackPos );
		}
		break;

	case STAFF_SWING_BACK:
		if( !DeltaTime(ANIMTAP_SWING_BACK_UPPER,FLoop_fPreviousLoopSecs * m_pEliteInfo_Staff->fSwingSpeedBackward,TRUE) )
		{
			f32 fUnitTime = GetUnitTime(ANIMTAP_SWING_BACK_UPPER);
			if ( (fUnitTime > m_pEliteInfo_Staff->fSwingSwingBackwardStreamerOn) &&
				(fUnitTime < m_pEliteInfo_Staff->fSwingSwingBackwardStreamerFadeOff) )
			{
				_StreamerOn(0.0f);
			}
			else if ( (fUnitTime > m_pEliteInfo_Staff->fSwingSwingBackwardStreamerFadeOff) &&
					  (fUnitTime < m_pEliteInfo_Staff->fSwingSwingBackwardStreamerOff) )
			{
				f32 fFadeRange = m_pEliteInfo_Staff->fSwingSwingBackwardStreamerOff - m_pEliteInfo_Staff->fSwingSwingBackwardStreamerFadeOff;
				if (fFadeRange!=0.0f)
				{
					f32 fFade = fmath_Div(fUnitTime-m_pEliteInfo_Staff->fSwingSwingBackwardStreamerFadeOff,fFadeRange);
					_StreamerOn(fFade);
				}
			}
			else
				_StreamerOn(1.0f);
		}
		else
		{
			_StreamerOn(1.0f);
			SetControlValue( ANIMCONTROL_SWING_BACK_UPPER, 0.0f);
			SetControlValue( ANIMCONTROL_SWING_BACK_LOWER, 0.0f);
			SetControlValue( ANIMCONTROL_SWING_DONE_UPPER, 1.0f);
			SetControlValue( ANIMCONTROL_SWING_DONE_LOWER, 1.0f);
			ZeroTime(ANIMTAP_SWING_DONE_UPPER);
			m_eStaffSwingState = STAFF_SWING_DONE;
		}
		break;

	case STAFF_SWING_DONE:
		if (bTryToSwing) // return to swinging state
		{
			m_eStaffSwingState = STAFF_SWING_DONE_UNDO;
		}
		else // blend out
		{
			if (!DeltaTime(ANIMTAP_SWING_DONE_UPPER,FLoop_fPreviousLoopSecs, TRUE))
			{
				f32 fUnitTime = GetUnitTime( ANIMTAP_SWING_DONE_UPPER );
				fUnitTime = 1.0f - fUnitTime;
				fUnitTime = fmath_UnitLinearToSCurve( fUnitTime );
				SetControlValue( ANIMCONTROL_SWING_DONE_UPPER, fUnitTime);
				SetControlValue( ANIMCONTROL_SWING_DONE_LOWER, fUnitTime);
			}
			else
			{
				SetControlValue( ANIMCONTROL_SWING_DONE_UPPER, 0.0f);
				SetControlValue( ANIMCONTROL_SWING_DONE_LOWER, 0.0f);
				m_eStaffSwingState= STAFF_SWING_NONE;
			}
		}
		break;
	case STAFF_SWING_DONE_UNDO:
		{		
			DeltaTime(ANIMTAP_SWING_DONE_UPPER, FLoop_fPreviousLoopSecs * -2.5f, TRUE);
			f32 fUnitTime = GetUnitTime( ANIMTAP_SWING_DONE_UPPER );
			fUnitTime = 1.01f - fUnitTime;
			fUnitTime = fmath_UnitLinearToSCurve( fUnitTime );
			SetControlValue( ANIMCONTROL_SWING_DONE_UPPER, fUnitTime);
			SetControlValue( ANIMCONTROL_SWING_DONE_LOWER, fUnitTime);

			if (GetUnitTime(ANIMTAP_SWING_DONE_UPPER)==0.0f)
			{
				SetControlValue( ANIMCONTROL_SWING_DONE_UPPER, 0.0f);
				SetControlValue( ANIMCONTROL_SWING_DONE_LOWER, 0.0f);
				SetControlValue( ANIMCONTROL_SWING_FORWARD_UPPER, 1.0f);
				SetControlValue( ANIMCONTROL_SWING_FORWARD_LOWER, 1.0f);
				ZeroTime(ANIMTAP_SWING_FORWARD_UPPER);
				m_eStaffSwingState = STAFF_SWING_FORWARD;
				_PlaySnd(ELITESND_SWIPEATTACK, TRUE);

				CFVec3A vMeleeAttackPos;
				_CalcMeleeAttackPos( 0, &vMeleeAttackPos );
				MeleeStartAttack( &m_MeleeData, &vMeleeAttackPos );

				_CalcMeleeAttackPos( 1, &vMeleeAttackPos );
				MeleeStartAttack( &m_MeleeData2, &vMeleeAttackPos );
			}
		}
		break;


	default:
		FASSERT(!"Invalid state reached");
	}// end switch(m_eStaffAnimCurrentState)


	if (m_fControls_Fire1 > 0.9f)
	{
		BOOL bJustFired = FALSE;

		if( _IsFingerOnTrigger() ) {
			m_bFireLaserThisFrame = TRUE;
			FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM );

			// keep the target point at least min dist away
			if( m_MountPos_WS.DistSq( m_TargetedPoint_WS ) <  10.0f * 10.0f )
			{
				m_TargetedPoint_WS.Sub( m_MountPos_WS );
				m_TargetedPoint_WS.Unitize().Mul( 10.0f );
				m_TargetedPoint_WS.Add( m_MountPos_WS );
			}
		}
		else
		{
			if (this->IsAlertOn())
			{	

				if (m_fUnitAlert >= 0.89f &&

					m_fStaffRocketLoadTimer <= 0.0f)
				{

					if( (m_nControlsBot_Flags & CBotControl::FLAG_AIM_AT_TARGET_POINT) && 
						(m_nBotFlags & BOTFLAG_TARGET_LOCKED) && 
						(m_MountPos_WS.DistSq( m_TargetedPoint_WS ) > 10.0f * 10.0f ) )
					{

						
			//			if (_FireRocket())
			//			if (_FireBeamThisFrame(m_TargetedPoint_WS))
						if (_FirePrimaryLaser(m_TargetedPoint_WS))
						{
							bJustFired = TRUE;
							m_fStaffRocketLoadTimer = 0.1f;// + fmath_RandomFloat()*0.2f;
						}
						
					} 
					else
					{
						//BLIND FIRE?
					}
				}
			}
		}

		if (bJustFired)
		{
			JustFired();  // No more disguise...

			FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM );
			ZeroTime( ANIMTAP_FIRE_UPPER );
		}
	}
	
	if (0)//findfix... pgm removed staff fire.
	{

		switch(m_eStaffFireState)
		{
		case STAFF_FIRE_NONE: // opportunity to get a state
			if (bStaffTriggerDown)
			{
				if ( (m_eStaffSwingState != STAFF_SWING_CHARGING) &&
					(m_eStaffSwingState != STAFF_SWING_FORWARD) &&
					(m_eStaffSwingState != STAFF_SWING_BACK) &&
					(m_eStaffSwingState != STAFF_SWING_DONE_UNDO) )
				{
					ZeroTime(ANIMTAP_FIRE_CHARGE_UPPER);
					m_eStaffFireState = STAFF_FIRE_CHARGING;
				}
			}
			break;

		case STAFF_FIRE_CHARGING:
			if (bStaffTriggerDown) // blend forward
			{
				if( !DeltaTime(ANIMTAP_FIRE_CHARGE_UPPER,FLoop_fPreviousLoopSecs,TRUE) )// means we're still charging
				{
					// means that at 1/3d pt. of animation, blend is at 100%
					f32 fUnitTime = 3.0f * fmath_UnitLinearToSCurve( GetUnitTime( ANIMTAP_FIRE_CHARGE_UPPER ) );
					FMATH_CLAMPMAX( fUnitTime, 1.0f );
					SetControlValue( ANIMCONTROL_FIRE_CHARGE_UPPER, fUnitTime);
					SetControlValue( ANIMCONTROL_FIRE_CHARGE_LOWER, fUnitTime);
				}
				else
				{
					SetControlValue( ANIMCONTROL_FIRE_CHARGE_UPPER, 1.0f);
					SetControlValue( ANIMCONTROL_FIRE_CHARGE_LOWER, 1.0f);
					m_eStaffFireState = STAFF_FIRE_CHARGED;
				}
			}
			else // blend out
			{
				DeltaTime( ANIMTAP_FIRE_CHARGE_UPPER, -FLoop_fPreviousLoopSecs, TRUE);
				f32 fUnitTime = 3.0f * fmath_UnitLinearToSCurve( GetUnitTime( ANIMTAP_FIRE_CHARGE_UPPER ) );
				FMATH_CLAMPMAX( fUnitTime, 1.0f );
				SetControlValue( ANIMCONTROL_FIRE_CHARGE_UPPER, fUnitTime);
				SetControlValue( ANIMCONTROL_FIRE_CHARGE_LOWER, fUnitTime);
				if (GetUnitTime(ANIMTAP_FIRE_CHARGE_UPPER)==0.0f)
				{
					m_eStaffFireState = STAFF_FIRE_NONE;
				}
			}
			break;

		case STAFF_FIRE_CHARGED:
			if (bStaffTriggerDown) // accumulate charge
			{
				m_apWeapon[0]->TriggerWork(1.0f,0.0f,&m_TargetedPoint_WS);
			}
			else
			{
				BOOL bFired = m_apWeapon[0]->TriggerWork(0.0f,0.0f,&m_TargetedPoint_WS);
				if (bFired)
				{
					SetControlValue( ANIMCONTROL_FIRE_UPPER, 1.0f);
					SetControlValue( ANIMCONTROL_FIRE_LOWER, 1.0f);
					ZeroTime(ANIMTAP_FIRE_UPPER);
					SetControlValue( ANIMCONTROL_FIRE_CHARGE_UPPER, 0.0f);
					SetControlValue( ANIMCONTROL_FIRE_CHARGE_LOWER, 0.0f);
					m_eStaffFireState = STAFF_FIRE_FIRING;
				}
				else
				{
					m_eStaffFireState = STAFF_FIRE_CHARGING;
				}
			}
		break;

		case STAFF_FIRE_FIRING:
			m_apWeapon[0]->TriggerWork(0.0f,0.0f,&m_TargetedPoint_WS);
			if (DeltaTime(ANIMTAP_FIRE_UPPER,FLoop_fPreviousLoopSecs,FALSE))
			{
				SetControlValue( ANIMCONTROL_FIRE_DONE_UPPER, 1.0f);
				SetControlValue( ANIMCONTROL_FIRE_DONE_LOWER, 1.0f);
				ZeroTime(ANIMTAP_FIRE_DONE_UPPER);
				SetControlValue( ANIMCONTROL_FIRE_UPPER, 0.0f);
				SetControlValue( ANIMCONTROL_FIRE_LOWER, 0.0f);
				m_eStaffFireState = STAFF_FIRE_DONE;
			}
			break;

		case STAFF_FIRE_DONE:
			if (bStaffTriggerDown) // return to firing state
			{
				DeltaTime(ANIMTAP_FIRE_DONE_UPPER, -FLoop_fPreviousLoopSecs, TRUE);
				f32 fUnitTime = GetUnitTime( ANIMTAP_FIRE_DONE_UPPER );
				fUnitTime = 1.01f - fUnitTime;
				fUnitTime = fmath_UnitLinearToSCurve( fUnitTime );
				SetControlValue( ANIMCONTROL_FIRE_DONE_UPPER, fUnitTime);
				SetControlValue( ANIMCONTROL_FIRE_DONE_LOWER, fUnitTime);
				
				if (GetUnitTime(ANIMTAP_FIRE_DONE_UPPER)==0.0f)
				{
					SetControlValue( ANIMCONTROL_FIRE_DONE_UPPER, 0.0f);
					SetControlValue( ANIMCONTROL_FIRE_DONE_LOWER, 0.0f);
					SetControlValue( ANIMCONTROL_FIRE_CHARGE_UPPER, 1.0f);
					SetControlValue( ANIMCONTROL_FIRE_CHARGE_LOWER, 1.0f);
					DeltaTime(ANIMTAP_FIRE_DONE_UPPER, 99.f, TRUE); // any clampable large val;
					m_eStaffFireState = STAFF_FIRE_CHARGING;
				}
			}
			else // blend out
			{
				if (!DeltaTime(ANIMTAP_FIRE_DONE_UPPER,FLoop_fPreviousLoopSecs, TRUE))
				{
				f32 fUnitTime = GetUnitTime( ANIMTAP_FIRE_DONE_UPPER );
				fUnitTime = 1.01f - fUnitTime;
				fUnitTime = fmath_UnitLinearToSCurve( fUnitTime );
				SetControlValue( ANIMCONTROL_FIRE_DONE_UPPER, fUnitTime);
				SetControlValue( ANIMCONTROL_FIRE_DONE_LOWER, fUnitTime);
				}
				else
				{
					SetControlValue( ANIMCONTROL_FIRE_DONE_UPPER, 0.0f);
					SetControlValue( ANIMCONTROL_FIRE_DONE_LOWER, 0.0f);
					m_eStaffFireState = STAFF_FIRE_NONE;
				}
			}
			break;
		default:
			FASSERT(!"Invalid state reached");
		}// end switch(m_eStaffAnimCurrentState)
	}
}

void CBotEliteGuard::_AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx )
{
	CBotEliteGuard *pBotEliteGuard = (CBotEliteGuard *)m_pCollBot;

	if( pBotEliteGuard->m_pPartMgr &&
		pBotEliteGuard->m_pPartMgr->IsCreated() &&
		pBotEliteGuard->m_pPartMgr->AnimBoneCallbackFunctionHandler( nBoneIndex, rNewMtx, rParentMtx, rBoneMtx ) )
	{
		return;
	}
	else if( nBoneIndex == (u32)((CBotEliteGuard *)m_pCollBot)->m_nBoneIndexGroin )
	{
		// Groin
		rNewMtx.Mul( rParentMtx, rBoneMtx );
		m_GroinVecY_WS = rNewMtx.m_vUp;
	}
	else if( nBoneIndex == (u32)((CBotEliteGuard *)m_pCollBot)->m_nBoneIndexTorso )
	{		// Torso
		CFMtx43A WorldMtx;
		CFQuatA Quat;

		Quat.BuildQuat( m_GroinVecY_WS, pBotEliteGuard->m_fAimDeltaYaw_MS - pBotEliteGuard->m_fLegsYaw_MS );
		WorldMtx.Mul( rParentMtx, rBoneMtx );

		rNewMtx.m_vPos = WorldMtx.m_vPos;
		Quat.MulPoint( rNewMtx.m_vRight, WorldMtx.m_vRight );
		Quat.MulPoint( rNewMtx.m_vUp, WorldMtx.m_vUp );
		Quat.MulPoint( rNewMtx.m_vFront, WorldMtx.m_vFront );
	}
	else
	{
		// Head
		m_pCollBot->HeadLookUpdate( rNewMtx, rParentMtx, rBoneMtx, FALSE );
	}
}


BOOL CBotEliteGuard::_InventoryCallback( CInventoryCallbackReason_e eReason, CInventory *pInventory, u32 nHandIndex, u32 nWeaponIndex )
{	CBotEliteGuard *pBotEliteGuard = (CBotEliteGuard *)pInventory->m_pUser;

	switch( eReason )
	{
	case IREASON_WEAPONCHANGE:
		FASSERT_NOW;

		break;

	case IREASON_RELOAD:
		if( nHandIndex == 0 )
		{
			pBotEliteGuard->m_abReloadRequest[0] = TRUE;
		}

		break;

	case IREASON_WEAPONUPGRADED:
		FASSERT_NOW;

		break;

	}

	return TRUE;
}

void CBotEliteGuard::CheckpointSaveSelect( s32 nCheckpoint )
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

void 	CBotEliteGuard::_HandleStaffCollision(void)
{
	if( IsSwinging() ) {
		CFVec3A vStaffMultDir;
		const CFMtx43A* pPrimaryFireMtx = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire];
		vStaffMultDir.Mul(pPrimaryFireMtx->m_vUp,m_pEliteInfo_Staff->fStreamerLength);
		m_mtxStreamerWS.Set(*pPrimaryFireMtx);
		m_mtxStreamerWS.m_vPos.Sub(vStaffMultDir);
	}
}
//#if 0
//	CFVec3A vPrimaryFirePos;
//	vPrimaryFirePos.Set( pPrimaryFireMtx->m_vPos);
//
//	CFVec3A vStaffCollSphereWS;
//	vStaffCollSphereWS.Set(m_CollSphere.m_Pos);
//	vStaffCollSphereWS.Sub(vPrimaryFirePos).Unitize();
//	vStaffCollSphereWS.Mul(m_CollSphere.m_fRadius - ((CWeaponStaff*)m_apWeapon[0])->GetChargeRadius());
//	vStaffCollSphereWS.Add(vPrimaryFirePos);
//
//	m_StaffCollSphereWS.m_Pos.x = vStaffCollSphereWS.x;
//	m_StaffCollSphereWS.m_Pos.y = vStaffCollSphereWS.y;
//	m_StaffCollSphereWS.m_Pos.z = vStaffCollSphereWS.z;
//	m_StaffCollSphereWS.m_fRadius = m_pBotInfo_Gen->fCollSphere1Radius_MS;
//
////	CFVec3A collision;collision.Set(m_StaffCollSphereWS.m_Pos);
////	fdraw_DevSphere(&collision,m_StaffCollSphereWS.m_fRadius);
//
//	m_CollInfo.nCollTestType = FMESH_COLLTESTTYPE_SPHERE;
//	m_CollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_NONE;
//	m_CollInfo.bFindClosestImpactOnly = TRUE;
//	m_CollInfo.nCollMask = (m_nPossessionPlayerIndex < 0) ? FCOLL_MASK_COLLIDE_WITH_NPCS : FCOLL_MASK_COLLIDE_WITH_PLAYER;
//	m_CollInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
//	m_CollInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
//	m_CollInfo.pSphere_WS = &m_StaffCollSphereWS;
//	m_CollInfo.pTag = NULL;
//
//	if (!IsSwinging()) {
//		return;
//	}
//
//	fcoll_Clear();
//	fworld_CollideWithWorldTris( &m_CollInfo, m_pWorldMesh );
//
//	if( FColl_nImpactCount > 0 )
//		fcoll_Sort( FALSE );
//
//	// Collide with walls...
//	for(u32 i=0; i<FColl_nImpactCount; i++ ) 
//	{
//		FCollImpact_t *pImpact = FColl_apSortedImpactBuf[i];
//
//		if( pImpact->UnitFaceNormal.y < 0.5f ) 
//		{
//			f32 fDot = m_Velocity_WS.Dot( pImpact->UnitFaceNormal );
//			if( fDot > 0.0f ) 
//			{
//				continue;
//			}
//
//			fDot = m_Velocity_WS.Dot( pImpact->PushUnitVec );
//			if( fDot > 0.0f ) 
//			{
//				continue;
//			}
//
//			f32 fDiffY = pImpact->PushUnitVec.y - pImpact->UnitFaceNormal.y;
//			if( fmath_Abs( fDiffY ) > 0.001f ) 
//			{
//				if( pImpact->PushUnitVec.y >= 0.0f ) 
//				{
//					continue;
//				}
//			}
///*
////pgm removed this since it cause entire bot to slide back 
//	CFVec3A vPushUnitVec;
//			vPushUnitVec.Sub(m_MountPos_WS,pImpact->ImpactPoint);
//			vPushUnitVec.Unitize();
//
//			FMATH_CLAMPMAX(pImpact->fImpactDistInfo, m_pBotInfo_Gen->fCollSphere1Radius_MS);
//			CFVec3A PushAmount_WS;
//			PushAmount_WS.x = vPushUnitVec.x * pImpact->fImpactDistInfo;
//			PushAmount_WS.y = 0.f;//pImpact->PushUnitVec.y * pImpact->fImpactDistInfo;
//			PushAmount_WS.z = vPushUnitVec.z * pImpact->fImpactDistInfo;
//		
//			if (PushAmount_WS.Mag() > m_pBotInfo_Gen->fCollSphere1Radius_MS)
//			{
//				PushAmount_WS = PushAmount_WS;
//			}
//			m_MountPos_WS.Add( PushAmount_WS );
//
//*/
////			if (IsSwinging())
////			{
//				_SpawnSlashImpactEffects(pImpact);
////			}
//			break;
//		} // end if wall
//	} // end impact checks
//
//#endif
//}
//
//BOOL CBotEliteGuard::_SwingStrikeCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) 
//{
//#if 0
//	FASSERT (_pThisBot->m_pWorldMesh != pTracker); // don't bother with self
//	if (_pThisBot->m_pWorldMesh == pTracker	)
//	{
//		return TRUE;//pgm: guard keeps hitting himself, can't figure out why. will put this for safety
//	}
//
//	CFWorldMesh *pWorldMesh = (CFWorldMesh *)pTracker;
//	if( !pWorldMesh->IsCollisionFlagSet() ) 
//	{
//		// Not collidable...
//		return TRUE;
//	}
//	
//	if( pTracker->m_nUser == MESHTYPES_ENTITY ) 
//	{
//		if( ((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_MESHENTITY ) 
//		{
//			return _SwingStruckMeshEntity( pTracker );
//		}
//		else if ( ((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_BOT ) 
//		{
//			return _SwingStruckBot( pTracker );
//		}
//		else if ( ((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_WEAPON ) 
//		{
//			// Entity is a CWeapon...
//			CWeapon *pWeapon = (CWeapon *)pTracker->m_pUser;
//			if( pWeapon->GetOwner() ) 
//			{
//				// Weapon has an owner...
//				if( pWeapon->GetBotDamageBoneIndex() >= 0 ) 
//				{
//					// Weapon will pass damage onto its owner...
//					_SwingStruckBot( pWeapon->GetOwner()->m_pWorldMesh );
//				}
//			}
//		}
//	}
//#endif
//	return TRUE;
//}

//BOOL CBotEliteGuard::_SwingStruckBot( CFWorldTracker *pTracker ) 
//{
//#if 0
//	if(!  (((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_BOT )) 
//	{
//		DEVPRINTF("Somebody thought this guy's a bot, but not true.\n");
//		return TRUE;
//	}
//	
//	CBot *pBot = (CBot *)pTracker->m_pUser;
//	if (pBot->IsDeadOrDying())
//	{
//		// you're dead to me, really
//		return TRUE;
//	}
//	m_CollInfo.nCollTestType = FMESH_COLLTESTTYPE_PROJSPHERE;
//	m_CollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_CHECK_ALL; // if we push per frame, one collide per frame should be okay?
//	m_CollInfo.bFindClosestImpactOnly = FALSE;
//	m_CollInfo.bFindClosestImpactOnly=FALSE;			
//	m_CollInfo.bCullBacksideCollisions=TRUE;			
//	m_CollInfo.bCalculateImpactData = TRUE;
//	m_CollInfo.nCollMask = FCOLL_MASK_COLLIDE_WITH_NPCS | FCOLL_MASK_COLLIDE_WITH_PLAYER;
//	m_CollInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
//	m_CollInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
//	
//	CFVec3A* pStaffBone   = &_pThisBot->m_pWorldMesh->GetBoneMtxPalette()[CBotEliteGuard::m_nBoneIndexStaff]->m_vPos;
//	CFVec3A* pPriFireBone = &_pThisBot->m_pWorldMesh->GetBoneMtxPalette()[CBotEliteGuard::m_nBoneIndexPrimaryFire]->m_vPos;
//
//	m_CollInfo.ProjSphere.Init(pStaffBone->x,pStaffBone->y,pStaffBone->z,pPriFireBone->x,pPriFireBone->y,pPriFireBone->z,1.0f);
//	m_CollInfo.pTag = pTracker;
//	
//	fcoll_Clear();
//	CFWorldMesh *pWorldMesh = (CFWorldMesh *)pTracker;
//	if( pWorldMesh->CollideWithMeshTris( &m_CollInfo ) ) 
//	{
//		// We hit a triangle on the entity...
//		_pThisBot->_SpawnSlashImpactEffects(&FColl_aImpactBuf[0]);
//		
//		if( _pThisBot->m_nHitEntityCount >= HIT_ENTITY_MAX_COUNT ) 
//		{
//			// Cannot damage any more entities with this slash...
//			return TRUE;
//		}
//		
//		for(s32 i=0; i<_pThisBot->m_nHitEntityCount; ++i ) 
//		{
//			if( _pThisBot->m_apHitEntity[i] == pBot ) 
//			{
//				// Already issued damage to this entity during this slash...
//				return TRUE;
//			}
//		}
//		
//		_pThisBot->_InflictSlashDamage( pBot, pWorldMesh, &FColl_aImpactBuf[0] );
//
//		// Now, add this entity to our hit-entity buffer to remember that we've already damaged it...
//		_pThisBot->m_apHitEntity[ _pThisBot->m_nHitEntityCount++ ] = pBot;
//	}
//	return TRUE;
//#endif 
//	return TRUE;
//}

//BOOL CBotEliteGuard::_SwingStruckMeshEntity( CFWorldTracker *pTracker ) 
//{
//#if 0
//	// Don't collide with weapons that are attached to bots...
//	if( ((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_WEAPON ) 
//	{
//		if( ((CWeapon *)pTracker->m_pUser)->GetOwner() ) 
//		{
//			return TRUE;
//		}
//	}
//
//	m_CollInfo.nCollTestType = FMESH_COLLTESTTYPE_PROJSPHERE;
//	m_CollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_CHECK_ALL; 
//	m_CollInfo.bFindClosestImpactOnly = FALSE;
//	m_CollInfo.bFindClosestImpactOnly=FALSE;			
//	m_CollInfo.bCullBacksideCollisions=TRUE;			
//	m_CollInfo.bCalculateImpactData = FALSE;
//	m_CollInfo.nCollMask = FCOLL_MASK_COLLIDE_WITH_NPCS | FCOLL_MASK_COLLIDE_WITH_PLAYER;
//	m_CollInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
//	m_CollInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
//	
//	CFVec3A* pStaffBone   = &_pThisBot->m_pWorldMesh->GetBoneMtxPalette()[CBotEliteGuard::m_nBoneIndexStaff]->m_vPos;
//	CFVec3A* pPriFireBone = &_pThisBot->m_pWorldMesh->GetBoneMtxPalette()[CBotEliteGuard::m_nBoneIndexPrimaryFire]->m_vPos;
//
//	m_CollInfo.ProjSphere.Init(pStaffBone->x,pStaffBone->y,pStaffBone->z,pPriFireBone->x,pPriFireBone->y,pPriFireBone->z,1.0f);
//	m_CollInfo.pTag = pTracker;
//	
//	fcoll_Clear();
//	CFWorldMesh *pWorldMesh = (CFWorldMesh *)pTracker;
//	if( pWorldMesh->CollideWithMeshTris( &m_CollInfo ) ) 
//	{
//		_pThisBot->_InflictSlashDamage( (CEntity *) pTracker->m_pUser, pWorldMesh, &FColl_aImpactBuf[0] );
//
//		_pThisBot->_SpawnSlashImpactEffects( &FColl_aImpactBuf[0] );
//		return FALSE; //pgm: findfix: gotta stop doing collision after this since InflictSlashDamage butchers the skip list. and stuff
//	}
//	return TRUE;
//#endif
//	return TRUE;
//}

void CBotEliteGuard::_StreamerOn( f32 fFadeOut) 
{
	if (fFadeOut==0.0f)
	{
		if( CFXStreamerEmitter::IsValidHandle( m_hStreamerEmitter ) ) // it's on 
		{
		}
		else
		{
			// grab a streamer emitter
			CFVec3 vSpawnPt(0.f,m_pEliteInfo_Staff->fStreamerLength*0.5f,0.0f);
			m_hStreamerEmitter = CFXStreamerEmitter::SpawnFromMtx43A( 50, &m_mtxStreamerWS,
																	m_pEliteInfo_Staff->fStreamerAlpha,
																	&m_StreamerTexInst,
																	&vSpawnPt,
																	m_pEliteInfo_Staff->fStreamerLength,
																	60.0f,
																	CFXStreamerEmitter::USE_UP_AXIS);
			if( m_hStreamerEmitter == FXSTREAMER_INVALID_HANDLE )
			{
				DEVPRINTF( "CBotEliteGuard::_StreamerOn(): trouble starting streamer emitter.\n" );
			}
		}
	}
	else if (fFadeOut==1.0f)
	{
		if( CFXStreamerEmitter::IsValidHandle( m_hStreamerEmitter ) ) 
		{
			CFXStreamerEmitter::EnableStreamerEmission(m_hStreamerEmitter,FALSE);
			m_hStreamerEmitter = FXSTREAMER_INVALID_HANDLE;
		}
	}
	else
	{
		if( CFXStreamerEmitter::IsValidHandle( m_hStreamerEmitter ) ) 
		{
			CFXStreamerEmitter::NearingEnd(m_hStreamerEmitter,fFadeOut);
		}
	}
}

void CBotEliteGuard::ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS )
{
	// sighting along an animated point causes model flicker,
	//pApproxMuzzlePoint_WS->Set(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]->m_vPos);

	// is the groin bone adequate for this purpose?
	pApproxMuzzlePoint_WS->Set(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexGroin]->m_vPos);
}

void CBotEliteGuard::_SpawnSlashImpactEffects( const FCollImpact_t *pCollImpact ) 
{
	const CGCollMaterial *pMaterial = CGColl::GetMaterial( pCollImpact );

	if( pMaterial->CanDrawParticle( CGCollMaterial::PARTICLE_TYPE_SPARKS_BURST ) ) 
	{
		pMaterial->DrawParticle(
			CGCollMaterial::PARTICLE_TYPE_SPARKS_BURST,
			&pCollImpact->ImpactPoint,
			&pCollImpact->UnitFaceNormal,
			1.0f
		);
	}

	if( pMaterial->CanDrawParticle( CGCollMaterial::PARTICLE_TYPE_BITS ) ) 
	{
		pMaterial->DrawParticle(
			CGCollMaterial::PARTICLE_TYPE_BITS,
			&pCollImpact->ImpactPoint,
			&pCollImpact->UnitFaceNormal,
			1.0f
		);
	}

	if( pMaterial->HasLooseDust() ) 
	{
		if( pMaterial->CanDrawParticle( CGCollMaterial::PARTICLE_TYPE_DUST ) ) 
		{
			pMaterial->DrawParticle(
				CGCollMaterial::PARTICLE_TYPE_DUST,
				&pCollImpact->ImpactPoint,
				&pCollImpact->UnitFaceNormal,
				1.0f
			);
		}
	}

	if( pMaterial->CanDrawDebris( CGCollMaterial::DEBRIS_GROUP_MEDIUM ) ) 
	{
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

void CBotEliteGuard::Possess( s32 nPlayerIndex, f32 fPossessionArmorModifier ) 
{
	CBot::Possess( nPlayerIndex, fPossessionArmorModifier );

	// set up our HUD
	CHud2* pHud = CHud2::GetHudForPlayer(nPlayerIndex);
	pHud->SetDrawFlags( CHud2::DRAW_RIGHT_AMMOBOX | CHud2::DRAW_BATTERIES | CHud2::DRAW_RADAR | CHud2::DRAW_SELFDESTRUCT | CHud2::DRAW_ENABLEOVERRIDE );
	
	CFColorRGBA startColor, endColor;
	startColor.OpaqueBlue();
	endColor.OpaqueWhite();
	pHud->OverrideAmmoData( CHud2::RIGHT, CHud2::OVERRIDE_METER_MIL, ((CWeaponStaff*)m_apWeapon[0])->GetUnitChargePtr(), TRUE, &startColor, &endColor );
}

void CBotEliteGuard::_HandleWeaponWork( void ) 
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


void CBotEliteGuard::MeleeCollision( CEntity *pEntity, FCollImpact_t *pImpact, BOOL bFirstCollision ) {
	CBot::MeleeCollision( pEntity, pImpact, bFirstCollision );
	_SpawnSlashImpactEffects( pImpact );

	if( pEntity ) {
		FASSERT( pImpact->pTag );
		// hit an entity, do some damage
		// find the worldmesh that was hit		

		//DEVPRINTF( "Swing (time %0.2f) hit entity %x\n", (FLoop_nRealTotalLoopTicks / (f32)FLoop_nTicksPerSec ), pEntity );

		_InflictSlashDamage( pEntity, (CFWorldMesh*)pImpact->pTag, pImpact );
	}

	if( bFirstCollision ) {
		_PlaySnd(ELITESND_SWIPEIMPACT,TRUE);
	}
}


void CBotEliteGuard::_InflictSlashDamage( CEntity *pHitEntity, CFWorldMesh *pHitWorldMesh, const FCollImpact_t *pCollImpact ) 
{
	FASSERT(pHitEntity); // don't call me with nulls;

	//// Inflict damage to the entity...

	//// Get an empty damage form...
	//CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

	CFVec3A vToMesh;
	vToMesh.Sub( pCollImpact->ImpactPoint, m_MountPos_WS );
	f32 fMag2 = vToMesh.MagSq();
	if( fMag2 > 0.01f ) {
		vToMesh.Mul( fmath_InvSqrt( fMag2 ) );
	} else {
		return;
	}

	//if( pDamageForm ) 
	//{
	//	// Fill out the form...
	//	pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
	//	pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
	//	pDamageForm->m_pDamageProfile = m_pDamageProfileSlash;
	//	pDamageForm->m_Damager.pWeapon = NULL;
	//	pDamageForm->m_Damager.pBot = this;
	//	pDamageForm->m_Damager.nDamagerPlayerIndex = m_nPossessionPlayerIndex;
	//	pDamageForm->m_pDamageeEntity = pHitEntity;

	//	pDamageForm->InitTriDataFromCollImpact( pHitWorldMesh, pCollImpact, &vToMesh );
	//	CDamage::SubmitDamageForm( pDamageForm );
	//}

	if( pHitEntity->TypeBits() & ENTITY_BIT_BOT ) 
	{
		CBot *pHitBot = (CBot *)pHitEntity;

		// Bump the bot a bit backwards
		CFVec3A Impulse = vToMesh;
		Impulse.Mul( m_pEliteInfo_Staff->fSwingSwingBackwardStreamerOff );
		pHitBot->ApplyVelocityImpulse_WS( Impulse );
	}
}

//#if 0
//BOOL CBotEliteGuard::NotifyBotCollision( CBot *pBot ) 
//{
//	CFSphere HisCollSphere_WS;
//	CFVec3A HisToMySphereCenter_WS;
//
//	FASSERT( pBot != NULL );
//	HisCollSphere_WS.m_Pos.x = pBot->m_pBotInfo_Gen->fCollSphere1X_MS + pBot->MtxToWorld()->m_vPos.x;
//	HisCollSphere_WS.m_Pos.y = 0.f;
//	HisCollSphere_WS.m_Pos.z = pBot->m_pBotInfo_Gen->fCollSphere1Z_MS + pBot->MtxToWorld()->m_vPos.z;
//	HisCollSphere_WS.m_fRadius = pBot->m_pBotInfo_Gen->fCollSphere1Radius_MS;
//
//	HisToMySphereCenter_WS.x = m_CollSphere.m_Pos.x - HisCollSphere_WS.m_Pos.x;
//	HisToMySphereCenter_WS.y = 0.f;
//	HisToMySphereCenter_WS.z = m_CollSphere.m_Pos.z - HisCollSphere_WS.m_Pos.z;
//	
//	f32 fDistFromHisSphereToMine_WS = HisToMySphereCenter_WS.Mag();
//	f32 fIntersectDist = (HisCollSphere_WS.m_fRadius + m_CollSphere.m_fRadius) - fDistFromHisSphereToMine_WS;
//	if( fIntersectDist < 0.0f ) 
//	{
//		return FALSE;
//	}
//	else if ( fDistFromHisSphereToMine_WS > 0.0f ) 
//	{
//		HisToMySphereCenter_WS.Unitize();
//		HisToMySphereCenter_WS.Mul( fIntersectDist );
//	}
//	else
//	{
//		HisToMySphereCenter_WS.Set(CFVec3A::m_UnitAxisX);
//	}
//
//	this->AdjustForCollision( &HisToMySphereCenter_WS );
//	return FALSE;
//}
//#endif


void CBotEliteGuard::_PlaySnd(EliteGuardSound_e eSnd, BOOL bPlay)
{
	if (eSnd >= ELITESND_COUNT)
	{
		DEVPRINTF("_PlaySnd() attempted sound past limit\n");
		return;
	}
	if (!m_pSounds[eSnd])
	{
		DEVPRINTF("_PlaySnd() attempted sound not loaded\n");
		return;
	}

	if (m_pSoundEmitters[eSnd])
	{
		m_pSoundEmitters[eSnd]->Destroy();
		m_pSoundEmitters[eSnd]=NULL;
	}

	if (bPlay)
	{
		m_pSoundEmitters[eSnd] = AllocAndPlaySound(m_pSounds[eSnd]);
	}
}

void CBotEliteGuard::Die( BOOL bSpawnDeathEffects/*=TRUE*/, BOOL bSpawnGoodies )
{
	CBot::Die(bSpawnDeathEffects, bSpawnGoodies);
	((CWeaponStaff*)m_apWeapon[0])->ResetUnitCharge();		
}

BOOL CBotEliteGuard::CanAttachToZipLine( void )
{
	if( m_eStaffFireState != STAFF_FIRE_NONE ) 
	{
		return FALSE;
	}

	if( m_eStaffSwingState != STAFF_SWING_NONE ) 
	{
		return FALSE;
	}
	
	return CBot::CanAttachToZipLine();
}

f32	CBotEliteGuard::EstimatedDiveDistance( void )
{
	f32 fEstimatedDiveVelocity = m_pBotInfo_Walk->fMaxXlatVelocity + m_pEliteInfo_Dive->fBoostForward;

	// I rough guessed one, then measured times from .84 to 1.14, with release builds hovering near 1, so one for now...
	f32 fEstimatedDiveTime =  1.0f;

	f32 fEstimatedDiveDistance = fEstimatedDiveVelocity * fEstimatedDiveTime;

	return fEstimatedDiveDistance;
}


void CBotEliteGuard::_FireBeamThisFrame( const CFVec3A & TargetPt_WS )
{
	// animate transmission texture frame
	m_fBeamTransTexFrame += FLoop_fPreviousLoopSecs * _BEAM_TEX_FRAME_RATE;
	if( m_fBeamTransTexFrame >= (f32) NUM_BEAM_TEX )
	{
		m_fBeamTransTexFrame -= (f32) NUM_BEAM_TEX;
	}

	// animate transmission texture scroll rate
	m_fBeamTransTexScroll += FLoop_fPreviousLoopSecs * _BEAM_TEX_SCROLL_RATE;
	if( m_fBeamTransTexScroll >= 1.0f )
	{
		m_fBeamTransTexScroll -= 1.0f;
	}

	// submit transmission beams for drawing
	if( _nNumEGBeamsThisFrame < _NUM_EG_BEAMS )
	{
		_aEGBeam[ _nNumEGBeamsThisFrame ].vStart.Set( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]->m_vPos );
		_aEGBeam[ _nNumEGBeamsThisFrame ].vEnd.Set( TargetPt_WS );
		_aEGBeam[ _nNumEGBeamsThisFrame ].vColor.Set( 0.8f, 0.8f, 1.0f, 0.52539f );
		_aEGBeam[ _nNumEGBeamsThisFrame ].fTexFrame = m_fBeamTransTexFrame;
		_aEGBeam[ _nNumEGBeamsThisFrame ].fTexScroll = m_fBeamTransTexScroll;
		++_nNumEGBeamsThisFrame;

		if( _nNumEGBeamsThisFrame < _NUM_EG_BEAMS )
		{
			_aEGBeam[ _nNumEGBeamsThisFrame ].vStart.Set( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]->m_vPos );
			_aEGBeam[ _nNumEGBeamsThisFrame ].vEnd.Set( TargetPt_WS );
			_aEGBeam[ _nNumEGBeamsThisFrame ].vColor.Set( 1.0f, 1.0f, 0.8f, 0.52539f );
			_aEGBeam[ _nNumEGBeamsThisFrame ].fTexFrame = m_fBeamTransTexFrame;
			_aEGBeam[ _nNumEGBeamsThisFrame ].fTexScroll = m_fBeamTransTexScroll;
			++_nNumEGBeamsThisFrame;
		}
	}
}



BOOL CBotEliteGuard::_FireRocket( void ) {
	// Get a free projectile...
	return FALSE;

	CEProj *pProj = CEProjPool::GetProjectileFromFreePool( CWeapon::m_ahProjPool[CWeapon::PROJ_POOL_TYPE_SWARMER_ROCKET] );
	if( pProj == NULL ) {
		// No more projectiles...
		return FALSE;
	}

	CFVec3A vUnitFireDir;

	// calculate fire direction
	if( m_pPartMgr && m_pPartMgr->IsCreated() && 
		(m_pPartMgr->GetComponentStatus( CBotPartMgr::COMPONENT_TYPE_PRIMARY ) == CBotPartMgr::COMPONENT_STATUS_ALL_FULLY_OPERATIONAL) )
	{
/*
		f32 fMag2;
		vUnitFireDir.Sub( m_vFiredRocketAtPoint, m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]->m_vPos );
		fMag2 = vUnitFireDir.MagSq();
		if( fMag2 > 0.01f ) {
			vUnitFireDir.Mul( fmath_InvSqrt( fMag2 ) );
		}
		else
*/
		{
			vUnitFireDir = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]->m_vFront;
		}
	} else {
		vUnitFireDir = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]->m_vFront;
	}

	_InitProjectile( pProj, &m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]->m_vPos, &vUnitFireDir, FALSE );

	pProj->Launch();

/*

	// flash of light
	CFVec3A vFlashPos = CFMtx43A::m_Temp.m_vFront;
	vFlashPos.Mul( 2.0f );
	vFlashPos.Add( CFMtx43A::m_Temp.m_vPos );

	CMuzzleFlash::AddFlash_CardPosterXY(
				CWeapon::m_ahMuzzleFlashGroup[CWeapon::MUZZLEFLASH_TYPE_BALL_POSTER_XY_1],
				vFlashPos,
				2.0f,
				0.75f
				);

	m_bRocketReactionOn		 = TRUE;
	m_fRocketReactionTarget	+= 1.0f;

	if( m_hRocketExplosionFire ) {
		FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();

		if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
			FExplosionSpawnParams_t SpawnParams;

			SpawnParams.InitToDefaults();

			SpawnParams.uFlags |= FEXPLOSION_SPAWN_EXPLOSION_FOLLOW_UNIT_DIR;

			SpawnParams.Pos_WS = pProj->MtxToWorld()->m_vPos;
			SpawnParams.UnitDir = vUnitFireDir;
			SpawnParams.uSurfaceType = 0;

			CExplosion2::SpawnExplosion( hSpawner, m_hRocketExplosionFire, &SpawnParams );
		}
	}

	PlaySound( m_BotInfo_Titan.pSndGrpRocketLauncher );
	if( m_nPossessionPlayerIndex >= 0 ) {
		fforce_Kill( &m_hForce );
		fforce_Play( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROCKET_THRUST_LIGHT, &m_hForce );
	}

	////add some sound...
	//if( m_nPossessionPlayerIndex < 0 ) {
	//	fsndfx_Play3D( m_BotInfo_TitanSounds.hSndRLFire, &vFlashPos, m_BotInfo_Titan.fSoundRadius );
	//} else {
	//	fsndfx_Play2D( m_BotInfo_TitanSounds.hSndRLFire );
	//	fforce_Kill( &m_hForce );
	//	fforce_Play( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROCKET_THRUST_LIGHT, &m_hForce );
	//}

	//draw some power
	m_fPowerDraw += _POWERDRAW_ROCKET;

*/
	return TRUE;
}



void CBotEliteGuard::_BuildRocketSkipList( CEProj *pProj ) {
	CBot* pBot = pProj->GetDamagerBot();
	pBot->AppendTrackerSkipList();
}


void CBotEliteGuard::_InitProjectile( CEProj *pProj, const CFVec3A *pPos_WS, const CFVec3A *pUnitDir_WS, BOOL bNoDynamicLight ) {
	CFMtx43A ProjMtx;

	ProjMtx.UnitMtxFromUnitVec( pUnitDir_WS );
	ProjMtx.m_vPos = *pPos_WS;

	pProj->Init();
	pProj->SetDamager( NULL, this );
	pProj->SetDamageProfile( _pEliteRocketDamageProfile );
	pProj->SetSkipListCallback( _BuildRocketSkipList );
	pProj->SetMaxDistCanTravel( 400.0f );
	pProj->SetLinSpeed( _ELiteRocketInfo.fSpeed );
	pProj->SetLinUnitDir_WS( pUnitDir_WS );
//	pProj->SmokeTrailOn( &m_SmokeTrailAttrib );
//	pProj->SetExplosionGroup( m_hRocketExplosionImpact );

	pProj->Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &ProjMtx, _ELiteRocketInfo.fProjectileMeshScale );

/*
	CFWorldLightItem *pWorldLightItem = CLightPool::GetFromFreePool();
	if( pWorldLightItem ) {
		pWorldLightItem->m_Light.InitOmniLight( &ProjMtx.m_vPos.v3, m_BotInfo_Titan.fLightRadius );

		FMATH_SETBITMASK( pWorldLightItem->m_Light.m_nFlags, FLIGHT_FLAG_PER_PIXEL | FLIGHT_FLAG_CORONA | FLIGHT_FLAG_CORONA_WORLDSPACE );

		if( bNoDynamicLight ) {
			FMATH_SETBITMASK( pWorldLightItem->m_Light.m_nFlags, FLIGHT_FLAG_CORONA_ONLY );
		}

		pWorldLightItem->m_Light.m_pCoronaTex = &m_CoronaTexInst;
		pWorldLightItem->m_Light.m_fCoronaScale = m_BotInfo_Titan.fCoronaScale;
		pWorldLightItem->m_Light.m_fDeltaScale = m_BotInfo_Titan.fCoronaUnitScreenspaceScale;

		pWorldLightItem->m_Light.SetColor( m_BotInfo_Titan.fCoronaColorRed, m_BotInfo_Titan.fCoronaColorGreen, m_BotInfo_Titan.fCoronaColorBlue );
		pWorldLightItem->m_Light.SetMotif( fcolor_GetRandomMotif(FCOLORMOTIF_ROCKET0) );
		pProj->SetAttachedLight( pWorldLightItem );
	}
*/
}


//BOOL CBotTitan::_InitRocketSmokeTrail( void ) {
//	FTexDef_t *pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, m_BotInfo_Titan.pszRocketSmokeTex );
//	if( pTexDef == NULL ) {
//		DEVPRINTF( "CWeaponRocket::_RL1_SetSmokeTrailAttributes(): Could not load smoke trail texture.\n" );
//		return FALSE;
//	}
//
//	// Rocket smoke:
//	m_SmokeTrailAttrib.nFlags = SMOKETRAIL_FLAG_NONE;
//	m_SmokeTrailAttrib.pTexDef = NULL;
//
//	m_SmokeTrailAttrib.fScaleMin_WS = 0.5f;
//	m_SmokeTrailAttrib.fScaleMax_WS = 0.75f;
//	m_SmokeTrailAttrib.fScaleSpeedMin_WS = 3.0f;
//	m_SmokeTrailAttrib.fScaleSpeedMax_WS = 5.0f;
//	m_SmokeTrailAttrib.fScaleAccelMin_WS = -0.5f;
//	m_SmokeTrailAttrib.fScaleAccelMax_WS = -0.75f;
//
//	m_SmokeTrailAttrib.fXRandSpread_WS = 0.2f;
//	m_SmokeTrailAttrib.fYRandSpread_WS = 0.4f;
//	m_SmokeTrailAttrib.fDistBetweenPuffs_WS = 0.4f;
//	m_SmokeTrailAttrib.fDistBetweenPuffsRandSpread_WS = 0.05f;
//
//	m_SmokeTrailAttrib.fYSpeedMin_WS = 0.5f;
//	m_SmokeTrailAttrib.fYSpeedMax_WS = 1.0f;
//	m_SmokeTrailAttrib.fYAccelMin_WS = -0.2f;
//	m_SmokeTrailAttrib.fYAccelMax_WS = -0.5f;
//
//	m_SmokeTrailAttrib.fUnitOpaqueMin_WS = 0.7f;
//	m_SmokeTrailAttrib.fUnitOpaqueMax_WS = 1.0f;
//	m_SmokeTrailAttrib.fUnitOpaqueSpeedMin_WS = -0.5f;
//	m_SmokeTrailAttrib.fUnitOpaqueSpeedMax_WS = -0.9f;
//	m_SmokeTrailAttrib.fUnitOpaqueAccelMin_WS = 0.0f;
//	m_SmokeTrailAttrib.fUnitOpaqueAccelMax_WS = 0.0f;
//
//	m_SmokeTrailAttrib.StartColorRGB.Set( 1.0f, 0.5f, 0.25f );
//	m_SmokeTrailAttrib.EndColorRGB.Set( 0.75f, 0.75f, 0.75f );
//	m_SmokeTrailAttrib.fStartColorUnitIntensityMin = 1.0f;
//	m_SmokeTrailAttrib.fStartColorUnitIntensityMax = 0.9f;
//	m_SmokeTrailAttrib.fEndColorUnitIntensityMin = 0.8f;
//	m_SmokeTrailAttrib.fEndColorUnitIntensityMax = 1.0f;
//
//	m_SmokeTrailAttrib.fColorUnitSliderSpeedMin = 3.5f;
//	m_SmokeTrailAttrib.fColorUnitSliderSpeedMax = 4.5f;
//	m_SmokeTrailAttrib.fColorUnitSliderAccelMin = 0.0f;
//	m_SmokeTrailAttrib.fColorUnitSliderAccelMax = 0.0f;
//
//	return TRUE;
//}


void BotEliteGuard_InitLevel( void )
{
	_nNumEGBeamsThisFrame = 0;

}

void BotEliteGuard_DrawBeams( void )
{
	s32 nIndex;

	FASSERT(_nNumEGBeamsThisFrame >= 0 && _nNumEGBeamsThisFrame < _NUM_EG_BEAMS);
	if( _nNumEGBeamsThisFrame >= _NUM_EG_BEAMS )
	{
		_nNumEGBeamsThisFrame = _NUM_EG_BEAMS - 1;   //whoops.
	}
	for( nIndex = 0; nIndex < _nNumEGBeamsThisFrame; nIndex++ )
	{
		CBotScout::DrawTransmissionBeam(_aEGBeam[nIndex].vStart, 
										_aEGBeam[nIndex].vEnd, 
										_aEGBeam[nIndex].vColor, 
										_aEGBeam[nIndex].fTexFrame, 
										_aEGBeam[nIndex].fTexScroll,
										&(_aEGBeamTexInst[(s32) _aEGBeam[nIndex].fTexFrame]));
	}
	_nNumEGBeamsThisFrame = 0;
}

static void _EGTracerBuildTrackerSkipList( void *pThis ) {
	if( pThis ) {
		((CBotEliteGuard*) pThis)->AppendTrackerSkipList();
	}
}


void CBotEliteGuard::_EGTracerKilledCallbackPrimary( TracerDef_t *pTracerDef, TracerKillReason_e nKillReason, const FCollImpact_t *pImpact ) {
	CFVec3A vtmp;

	if( nKillReason == TRACER_KILLREASON_HIT_GEO ) {
		FASSERT( pTracerDef->pUser != NULL );
		
		CBotEliteGuard* pShooter = (CBotEliteGuard*)(pTracerDef->pUser);
		const CGCollMaterial *pCollMaterial = CGColl::GetMaterial( pImpact->nUserType );

		//pCollMaterial->DrawAllDrawableParticles(
		//	&pImpact->ImpactPoint,
		//	&pImpact->UnitFaceNormal,
		//	TRUE,
		//	fmath_RandomFloatRange( 0.0f, 0.5f ),
		//	fmath_RandomFloatRange( 0.0f, 0.75f ),
		//	fmath_RandomFloatRange( 0.0f, 0.25f )
		//);

		fparticle_SpawnEmitter( pShooter->m_DefaultEliteInfoStaff.hLaserParticle, &(pImpact->ImpactPoint.v3), &(pImpact->UnitFaceNormal.v3), NULL, 0.1f );

		vtmp.Mul( pImpact->UnitFaceNormal, 0.25f ).Add( pImpact->ImpactPoint );

//		CMuzzleFlash::AddFlash_CardPosterXY( m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_BALL_POSTER_XY_1], vtmp, 5.0f, 1.0f );

		if( pCollMaterial->IsPockMarkEnabled() ) {
			CFDecal::Create( _hDecalDefCopy, pImpact, &pTracerDef->UnitDir_WS );
		}
		

		if( pImpact->pTag != NULL ) {
			// hit something interesting...

			if( ((CFWorldTracker*)(pImpact->pTag))->GetType() == FWORLD_TRACKERTYPE_MESH ) {
				CFWorldMesh *pHitWorldMesh = (CFWorldMesh*)(pImpact->pTag);

				if( pHitWorldMesh->m_nUser == MESHTYPES_ENTITY ) {
					// it was an entity, damage it...
					CEntity *pHitEntity = (CEntity*)(pHitWorldMesh->m_pUser);

					CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();
					if( pDamageForm ) {
						pDamageForm->m_nDamageLocale	= CDamageForm::DAMAGE_LOCALE_IMPACT;
						pDamageForm->m_nDamageDelivery	= CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
						pDamageForm->m_pDamageProfile	= _pEliteLaserDamageProfile;
						pDamageForm->m_Damager			= pTracerDef->Damager;
						pDamageForm->m_pDamageeEntity	= pHitEntity;

						pDamageForm->InitTriDataFromCollImpact( pHitWorldMesh, pImpact, &pTracerDef->UnitDir_WS );

						CDamage::SubmitDamageForm( pDamageForm );

					}
				}
			}
		}
		
		if( fmath_RandomFloat() < 0.5f ) {
			CFVec3A vDir = pTracerDef->UnitDir_WS;
			pTracerDef->UnitDir_WS.ReceiveReflection( vDir, pImpact->UnitFaceNormal );
			pTracerDef->TailPos_WS = pImpact->ImpactPoint;
			vDir = pTracerDef->UnitDir_WS;
			fmath_ScatterUnitVec( &pTracerDef->UnitDir_WS, &vDir, 0.25f );

			pTracerDef->fMaxTailDist_WS *= fmath_RandomFloatRange( 0.1f, 0.25f );
			pTracerDef->fSpeed_WS = _fEGPrimaryLaserFireBaseSpeed * fmath_RandomFloatRange( 0.5f, 0.75f );
			tracer_NewTracer( _hTracerGroupPri, pTracerDef, 0.0f, FALSE, FALSE, &CDamageForm::m_TempDamager );
		}

		// make an AI impact sound in case any AI are listening
		if( pShooter->m_nPossessionPlayerIndex > -1 && pShooter->m_fSecsUntilNextSound <= 0.0f) {
			AIEnviro_AddSound( pImpact->ImpactPoint, AIEnviro_fLaserImpactAudibleRange, 0.3f, AISOUNDTYPE_WEAPON, 0, pShooter );
			pShooter->m_fSecsUntilNextSound = 0.5f;
		}
	}
}


BOOL CBotEliteGuard::_FirePrimaryLaser( const CFVec3A& TargetedPoint_WS ) {
	CFVec3A vFirePos;
	FASSERT( m_apWeapon[0] );
	

//	_apMuzzleSmokeParticles[uFireArm]->EnableEmission( TRUE );

	// Now create the tracer
	u32 uNumTracerThisFire = 20;
	f32 fHalfSpreadFeet = 3.0f;
	CFVec3A FireDirUnit;

	_TracerDefPri.pUser		= this;
	//FireDirUnit.Sub(TargetedPoint_WS, m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]->m_vPos);

	//m_apWeapon[0]->ComputeFireUnitDir( &FireDirUnit, &(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]->m_vPos), 
	//									&(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexStaff]->m_vUp), &TargetedPoint_WS, FALSE );

	//TMP
	FireDirUnit.Sub( TargetedPoint_WS, m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]->m_vPos );
	if( FireDirUnit.SafeUnitAndMag( FireDirUnit ) <= 0.0f ) {
		return FALSE;
	}
	////


	f32 fFocusDist = 0.0f;
	fFocusDist = TargetedPoint_WS.Dist( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]->m_vPos );
	BOOL bDidFire = FALSE;
	if (fFocusDist > 0.0f)
	{
		CFQuatA Quat;
		Quat.BuildQuat(FireDirUnit,FMATH_QUARTER_PI+FMATH_PI*fmath_RandomFloat());//m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]->m_vFront, FMATH_DEG2RAD(360/3));//30.0f-GetBrain()->GetInfreqCycleRandom()*10.0f));//s_pWD->pBot->m_pBotInfo_MountAim->fAllowableLockAngleCos
		Quat.BuildMtx( CFMtx43A::m_Temp );

		ConstructDamagerData();//fills in CDamageForm::m_TempDamager

		f32 fRadSpread = fmath_Atan(fHalfSpreadFeet, fFocusDist);
		FireDirUnit.RotateY(fRadSpread*fmath_RandomFloat());

		_TracerDefPri.UnitDir_WS = FireDirUnit;

		for (u32 i = 0; i < uNumTracerThisFire; i++)
		{

			_TracerDefPri.TailPos_WS.x = fmath_RandomFloat();
			_TracerDefPri.TailPos_WS.y = fmath_RandomFloat();
			_TracerDefPri.TailPos_WS.z = fmath_RandomFloat();
			_TracerDefPri.TailPos_WS.Mul(0.25f);
			_TracerDefPri.TailPos_WS.Add(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]->m_vPos);
			
			
			//tweak the speed a bit every frame
			_TracerDefPri.fSpeed_WS = _fEGPrimaryLaserFireBaseSpeed+fmath_RandomFloat()*_fEGPrimaryLaserFireBaseScatterSpeed;

			_TracerDefPri.fLength_WS = fmath_RandomFloatRange( 5.0f, 10.0f );


			
//			CFMtx43A::m_Temp.MulDir(_TracerDefPri.UnitDir_WS);
			fmath_ScatterUnitVec( &_TracerDefPri.UnitDir_WS, &FireDirUnit, fRadSpread );



			if( tracer_NewTracer( _hTracerGroupPri, &_TracerDefPri, 0.2f, FALSE, FALSE, &CDamageForm::m_TempDamager ) )
			{
				bDidFire = TRUE;
			}
		}

		if (bDidFire)
		{
			CMuzzleFlash::AddFlash_Mesh3D( CWeapon::m_ahMuzzleFlashGroup[ CWeapon::MUZZLEFLASH_TYPE_3D_ENERGY_WHITE ],
												_TracerDefPri.TailPos_WS,
												FireDirUnit,
												2.0f+fmath_RandomFloat()*1.5f,//m_BotInfo_Loader.fMuzzleFlashMeshScale * fRandomScale,
												fmath_RandomFloat() * FMATH_2PI );	// rotation

			CMuzzleFlash::MuzzleLight( &_TracerDefPri.TailPos_WS, 
										8.0f);//m_BotInfo_Loader.fMuzzleLightRadius );

			PlaySound( m_pSounds[ELITESND_PRIMARY_FIRE], 1.0f, 0.8f, -1.0f );

				//do some force feedback work
		/*
				if( IsOwnedByPlayer() ) {
					fforce_Kill( &m_hForce );
					fforce_Play( Player_aPlayer[GetOwner()->m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROUGH_RUMBLE_MED, &m_hForce );
				}
		*/
		}
	}

	return bDidFire;
}


f32 CBotEliteGuard::GetApproxPrimaryWeaponSpeed(void)
{
	return _fEGPrimaryLaserFireBaseSpeed;
}

BOOL CBotEliteGuard::IsDiveJumping(void)
{
	return (
			( m_nJumpState == BOTJUMPSTATE_DIVE_ENTER ) ||
			( m_nJumpState == BOTJUMPSTATE_DIVE_FLY )  ||
			( m_nJumpState == BOTJUMPSTATE_DIVE_TUCK ) ||
			( m_nJumpState == BOTJUMPSTATE_DIVE_LAND ) ||
			( m_nJumpState == BOTJUMPSTATE_DIVE_EXIT ) );
}
BOOL CBotEliteGuard::IsSwinging(void)
{
	return (
			( m_eStaffSwingState == STAFF_SWING_FORWARD) || 
			( m_eStaffSwingState == STAFF_SWING_BACK) );
}
const CFVec3A& CBotEliteGuard::GetDiveUnitDir(void)
{
	if (m_fSpeedXZ_WS > 0.0f ) // when moving
	{
		if (m_XlatStickUnitVecXZ_WS.MagSqXZ() > 0.5f) // unit means length one, don't trust less than that
		{
			return m_XlatStickUnitVecXZ_WS; // return stick
		}
		return m_UnitVelocityXZ_WS; // return velocity
	}
	return m_MountUnitFrontXZ_WS; // all other cases return the look dir
}


void CBotEliteGuard::_CalcMeleeAttackPos( u32 i, CFVec3A *pvMeleeAttackPos ) {
	FASSERT( pvMeleeAttackPos );


	pvMeleeAttackPos->Mul( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]->m_vUp, -2.0f + (((f32)i) * -2.0f * _MELEE_SPHERE_RADIUS) );
	
	pvMeleeAttackPos->Add( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]->m_vPos );
}


BOOL CBotEliteGuard::_IsFingerOnTrigger( void ) {
	if( (m_fFlipPitch == 0.0f) || (fmath_Abs(m_fLegsYaw_MS) > (0.8f * FMATH_HALF_PI)) ) {
		if( m_nPossessionPlayerIndex >= 0 )
		{
			if (m_fStaffRocketLoadTimer <= 0.0f)
			{
				if( m_eStaffSwingState == STAFF_SWING_NONE ) {
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}