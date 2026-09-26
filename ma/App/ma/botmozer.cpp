//////////////////////////////////////////////////////////////////////////////////////
// botmozer.cpp -
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
// 02/25/03 Scholz       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "botmozer.h"
#include "fanim.h"
#include "fresload.h"
#include "floop.h"
#include "ftext.h"
#include "ItemInst.h"
#include "ItemRepository.h"
#include "fforce.h"
#include "meshtypes.h"
#include "reticle.h"
#include "player.h"
#include "level.h"
#include "fsound.h"
#include "meshentity.h"
#include "Hud2.h"
#include "fworld_coll.h"
#include "weapon_rivet.h"
#include "protrack.h"
#include "Ai\AIEnviro.h"
#include "splat.h"
#include "fxshockwave.h"
#include "botpart.h"

#define _BOTINFO_FILENAME		"b_mozer"
#define _BOTPART_FILENAME		"bp_mozer"
#define _STREAMER_TEXTURE_NAME	"TFDCswing01"

#define _STOMPCAM_POSX				0.0f
#define _STOMPCAM_POSY				-5.0f
#define _STOMPCAM_POSZ				-5.0f

#define _STOMPCAM_LOOKATX			0.0f
#define _STOMPCAM_LOOKATY			-5.0f
#define _STOMPCAM_LOOKATZ			0.0f		

#define _STOMPCAM_TRANSON			2.0f
#define _STOMPCAM_TRANSOFF			1.0f
#define _STOMPCAM_HOLDTIME			0.5f

#define _RECOIL_ROT_RIGHT_SHOULDER		( FMATH_DEG2RAD( 10.0f) )			// y axis now
#define _RECOIL_ROT_TORSO				( FMATH_DEG2RAD( 15.0f ) )
#define _RECOIL_ROT_LEFT_ELBOW			( FMATH_DEG2RAD( 5.0f ) )
#define _RECOIL_ROT_DUMMY				( FMATH_DEG2RAD( -2.0f ) )

#define _FIRE_ANIM_ON_TIME				( 0.1f )
#define _FIRE_ANIM_OO_ON_TIME			( 1.0f / _FIRE_ANIM_ON_TIME )
#define _FIRE_ANIM_HOLD_TIME			( 0.66f )
#define _FIRE_ANIM_READY_TIME			( 5.0f )
#define _FIRE_ANIM_OFF_TIME				( 1.0f )
#define _FIRE_ANIM_OO_OFF_TIME			( 1.0f / _FIRE_ANIM_OFF_TIME )
#define _FIRE_READY_BLEND				( 0.33f )

#define _ARM_SLERP_RATE					( 20.0f )
#define _RECOIL_ON_SPEED				( 15.0f )
#define _RECOIL_OFF_SPEED				( 2.0f )

#define _MELEE_RADIUS					( 2.5f )

static cchar*		   _apszMozerMeshFilenames[] = {"GRDCcolos00"};
static cchar*		   _BotMozer_pszSoundEffectBank = "mozer";
static CBotMozer*		_pThisBot = NULL;


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotMozerBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CBotMozerBuilder _BotMozerBuilder;


void CBotMozerBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType )
{	
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_BOTMOZER, pszEntityType );

	// Override defaults set by our parent classes...
	m_nEC_HealthContainerCount = CBotMozer::m_BotInfo_Gen.nBatteryCount;
	m_pszEC_ArmorProfile = CBotMozer::m_BotInfo_Gen.pszArmorProfile;
	m_anNPCWeaponType[0] = _NPC_WEAPON_RIVET;		

	// This class of bots shows up on Glitch's radar...
	FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_SHOWS_UP_ON_RADAR );
}


BOOL CBotMozerBuilder::PostInterpretFixup( void )
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


BOOL CBotMozerBuilder::InterpretTable( void )
{
	return CBotBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotMozer
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL	CBotMozer::m_bSystemInitialized		= FALSE;
u32		CBotMozer::m_nBotClassClientCount	= 0;

CBotMozer::BotInfo_Gen_t		CBotMozer::m_BotInfo_Gen;
CBotMozer::BotInfo_MountAim_t	CBotMozer::m_BotInfo_MountAim;
CBotMozer::BotInfo_Walk_t		CBotMozer::m_BotInfo_Walk;
CBotMozer::BotInfo_Jump_t		CBotMozer::m_BotInfo_Jump;
CBotMozer::BotInfo_Weapon_t		CBotMozer::m_BotInfo_Weapon;
CBotMozer::BotInfo_Mozer_t		CBotMozer::m_BotInfo_Mozer;
CBotMozer::BotInfo_Girder_t		CBotMozer::m_BotInfo_Girder;

CBotAnimStackDef CBotMozer::m_AnimStackDef;

CFVec3A			CBotMozer::m_vecStompCameraLookat;
CFVec3A			CBotMozer::m_vecStompCameraPos;

s32				CBotMozer::m_nBoneIndexSecondaryFire	= -1;
s32				CBotMozer::m_nBoneIndexGirder			= -1;
s32				CBotMozer::m_nBoneIndexHead				= -1;
s32				CBotMozer::m_nBoneIndexTorso			= -1;
s32				CBotMozer::m_nBoneIndexGroin			= -1;
s32				CBotMozer::m_nBoneIndexArmLower			= -1;
s32				CBotMozer::m_nBoneIndexArmElbow			= -1;
s32				CBotMozer::m_nBoneIndexL_Leg_Upper		= -1;
s32				CBotMozer::m_nBoneIndexR_Leg_Upper		= -1;
CFVec3A			CBotMozer::m_vGroinToGroinPivot;
CFTexInst   	CBotMozer::m_StreamerTexInst;
CBotPartPool*	CBotMozer::m_pPartPool					= NULL;

CFSoundGroup* CBotMozer::m_pSounds[MOZERSND_COUNT];

static cchar* _pszSoundGroupTable[CBotMozer::MOZERSND_COUNT] = 
{
	"SRDZswinout",//		MOZERSND_SWIPEFORWARD,
	"SRDZswinbac",	//		MOZERSND_SWIPEBACK,
	"SRDZbash",	//		MOZERSND_SWIPEIMPACT,
};

const CDamageProfile* CBotMozer::m_pDamageProfileSlash=NULL;

CBot::BotInfo_Idle_t*	CBotMozer::m_BotInfo_Idle=NULL;				// Idle bot info
u32				CBotMozer::m_nBotInfo_Idles=0;			// Idle bot info count

BOOL CBotMozer::InitSystem( void )
{
	FASSERT( !m_bSystemInitialized );
	m_bSystemInitialized = TRUE;
	m_nBotClassClientCount = 0;

	//set up the stomp cam data
	m_vecStompCameraPos.Set( _STOMPCAM_POSX, _STOMPCAM_LOOKATY, _STOMPCAM_POSZ );
	m_vecStompCameraLookat.Set( _STOMPCAM_LOOKATX, _STOMPCAM_LOOKATY, _STOMPCAM_LOOKATZ );

	return TRUE;
}

BOOL CBotMozer::ClassHierarchyLoadSharedResources( void ) 
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

	if( !ReadBotIdleFile( &m_BotInfo_Idle,&m_nBotInfo_Idles, _BOTINFO_FILENAME ) ) 
	{
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


	m_pDamageProfileSlash = CDamage::FindDamageProfile( m_BotInfo_Girder.pszDamageProfileSlash );

	if( !_BuildAnimStackDef() )
	{
		goto _ExitInitSystemWithError;
	}

	m_StreamerTexInst.SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, _STREAMER_TEXTURE_NAME ) );
	if( !m_StreamerTexInst.GetTexDef() )
	{
		goto _ExitInitSystemWithError;
	}
	// Load the sound effects bank for this bot...
	if( !fresload_Load( FSNDFX_RESTYPE, _BotMozer_pszSoundEffectBank) )
	{
		DEVPRINTF( "CBotMozer::ClassHierarchyLoadSharedResources(): Could not load sound effect bank '%s'\n", _BotMozer_pszSoundEffectBank);
	}
	for (MozerSound_e iSndIdx = (MozerSound_e)0; iSndIdx < MOZERSND_COUNT; iSndIdx = (MozerSound_e)(int(iSndIdx)+1))
	{
		m_pSounds[iSndIdx] = CFSoundGroup::RegisterGroup( _pszSoundGroupTable[iSndIdx] );
		if (!m_pSounds[iSndIdx])
			DEVPRINTF( "CBotMozer::ClassHierarchyLoadSharedResources(): Could not load sound effect '%s'\n", _pszSoundGroupTable[iSndIdx]);
	}


	return TRUE;
	// Error:
_ExitInitSystemWithError:
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

void CBotMozer::UninitSystem( void )
{
	m_bSystemInitialized = FALSE;
}

void CBotMozer::ClassHierarchyUnloadSharedResources( void ) 
{
	FASSERT( m_nBotClassClientCount > 0 );

	--m_nBotClassClientCount;

	if( m_nBotClassClientCount > 0 ) 
	{
		return;
	}

	m_AnimStackDef.Destroy();

	CBot::ClassHierarchyUnloadSharedResources();
}	

CBotMozer::CBotMozer() : CBot()
{

}


CBotMozer::~CBotMozer()
{
	if( IsSystemInitialized() && IsCreated() )
	{
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld(TRUE);
		ClassHierarchyDestroy();
	}
}


BOOL CBotMozer::Create( s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName )
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
	CBotMozerBuilder *pBuilder = (CBotMozerBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_BotMozerBuilder )
	{
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOTMOZER );
	}

	// Set our builder parameters...

	// Create an entity...
	return CBot::Create( &m_BotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}


void CBotMozer::ClassHierarchyDestroy( void )
{	// Delete the items that we had instantiated for us...
	fforce_Kill( &m_hForce );

	m_Anim.Destroy();

	_StreamerOn(1.0f);

	fdelete( m_pWorldMesh );
	m_pWorldMesh = NULL;

	fdelete( m_apWeapon[0] );
	m_apWeapon[0] = NULL;

	for (MozerSound_e iSndIdx = (MozerSound_e)0; iSndIdx < MOZERSND_COUNT; iSndIdx = (MozerSound_e)(int(iSndIdx)+1))
	{
		if (m_pSoundEmitters[iSndIdx])
		{
			m_pSoundEmitters[iSndIdx]->Destroy();
			m_pSoundEmitters[iSndIdx] = NULL;
		}
	}

	CBot::ClassHierarchyDestroy();
}


BOOL CBotMozer::ClassHierarchyBuild( void )
{	FMesh_t *pMesh;
	FMeshInit_t MeshInit;
	u32 i;
	s32 nBoneIndex;
	cchar * pszMeshFilename = NULL;

	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CBotMozerBuilder *pBuilder = (CBotMozerBuilder *)GetLeafClassBuilder();

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
	pszMeshFilename = _apszMozerMeshFilenames[0];
	if ( pBuilder->m_uMeshVersionOverride >0 && pBuilder->m_uMeshVersionOverride < sizeof(_apszMozerMeshFilenames)/sizeof(char*) )
	{
		pszMeshFilename = _apszMozerMeshFilenames[pBuilder->m_uMeshVersionOverride];
	}
	if( pBuilder->m_pszMeshReplacement ) {
	   pszMeshFilename = pBuilder->m_pszMeshReplacement;
	}
	pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pszMeshFilename);
	if( pMesh == NULL )
	{
		DEVPRINTF( "CBotMozer::ClassHierarchyBuild(): Could not find mesh '%s'\n", pszMeshFilename );
		goto _ExitWithError;
	}

	// Create world mesh...
	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL )
	{
		DEVPRINTF( "CBotMozer::ClassHierarchyBuild(): Not enough memory to create CFWorldMesh.\n" );
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
		DEVPRINTF( "CBotMozer::ClassHierarchyBuild(): Trouble creating m_Anim.\n" );
		goto _ExitWithError;
	}

	SetControlValue( ANIMCONTROL_STAND, 1.0f );
	UpdateUnitTime( ANIMTAP_STAND, fmath_RandomFloat() );

	m_nBoneIndexSecondaryFire = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_SECONDARY_FIRE] );
	m_nBoneIndexGirder = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_GIRDER] );
	m_nBoneIndexHead = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_HEAD] );
	m_nBoneIndexTorso = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_TORSO] );
	m_nBoneIndexGroin = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_GROIN] );
	m_nBoneIndexArmLower= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_GUN] );
	m_nBoneIndexArmElbow= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_R_ARM_ELBOW] );
	m_nBoneIndexL_Leg_Upper= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_L_LEG_UPPER] );
	m_nBoneIndexR_Leg_Upper= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_R_LEG_UPPER] );

	if( (m_nBoneIndexTorso < 0)  || (m_nBoneIndexSecondaryFire < 0) || (m_nBoneIndexGirder < 0) || (m_nBoneIndexHead < 0) || (m_nBoneIndexGroin <0) || (m_nBoneIndexArmLower<0) || (m_nBoneIndexArmElbow<0) || (m_nBoneIndexL_Leg_Upper<0)||(m_nBoneIndexR_Leg_Upper<0))
	{
		DEVPRINTF( "CBotMozer::ClassHierarchyBuild(): Trouble retrieving bone indices.\n" );
		goto _ExitWithError;
	}
	
	{
		AtRestMatrixPalette(); // set up my bones for following computations
		
		CFVec3A vLeftHip_GroinSpace,vRightHip_GroinSpace;
		vLeftHip_GroinSpace.Sub(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexL_Leg_Upper]->m_vPos,
						 m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexGroin]->m_vPos);
		vRightHip_GroinSpace.Sub(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexR_Leg_Upper]->m_vPos,
						 m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexGroin]->m_vPos);
		
		// the groin's pivot point (from the groin) is the average of these two vectors;
		m_vGroinToGroinPivot.Add(vLeftHip_GroinSpace,vRightHip_GroinSpace).Mul(0.5f);
	}
	
	m_pGirderUnitDir = &m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexGirder]->m_vUp;
	
	m_Anim.m_pAnimCombiner->SetBoneCallback( &_AnimBoneCallback );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_HEAD] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_TORSO] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_GROIN] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_L_LEG_UPPER] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_R_LEG_UPPER] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_GUN] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_R_ARM_ELBOW] );

	EnableControlSmoothing( ANIMTAP_JUMP_LAUNCH );
	EnableControlSmoothing( ANIMTAP_JUMP_FLY );
	EnableControlSmoothing( ANIMTAP_JUMP_LAND_LOWER );
	EnableControlSmoothing( ANIMTAP_JUMP_LAND_UPPER );
	EnableControlSmoothing( ANIMTAP_JUMP2_FLY );
	EnableControlSmoothing( ANIMTAP_JUMP2_LAND );

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

	if( !m_AnimManFrameRecoil.Create( ANIMBONE_RECOIL_COUNT, m_apszRecoilBoneNameTable ) ) {
		DEVPRINTF( "CBotGlitch::ClassHierarchyCreate(): Could not create recoil animation summer.\n" );
		goto _ExitWithError;
	}

	AttachAnim( ANIMTAP_RECOIL_SUMMER, &m_AnimManFrameRecoil );
	m_AnimManFrameRecoil.Reset();
	UpdateBoneMask( ANIMTAP_RECOIL_SUMMER, m_aBoneEnableIndices_RecoilSummer, TRUE );
	SetControlValue( ANIMCONTROL_RECOIL_SUMMER, 0.0f );


	// set up part mgr...
	if( !m_pPartMgr->Create( this, &m_pPartPool, _BOTPART_FILENAME, PART_INSTANCE_COUNT_PER_TYPE, LIMB_TYPE_COUNT ) ) {
		goto _ExitWithError;
	}


	if( m_nPossessionPlayerIndex >= 0 )
	{
		m_pInventory = CPlayer::GetInventory(m_nOwnerPlayerIndex,TRUE); // gets the possession inventory
		if( m_pInventory == NULL ) 
		{
			// What is going on here?!  This shouldn't happen!
			DEVPRINTF( "CBotBlink::_InitInventory() : Could not get inventory from CPlayer.\n" );
			return FALSE;
		}

		m_pInventory->SetToMozer();
		m_pInventory->m_pUser = this;
		m_pInventory->m_pfcnCallback = _InventoryCallback;
		CHud2::GetHudForPlayer(m_nPossessionPlayerIndex)->SetHudMode(HUDMODE_MOZER);
		CWeapon *pWeap;
		s32 uCurWpnIdx;
		// Set primary and secondary weapon.........
		for( i=0; i<1; ++i )
		{
			// step through primary weapon
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
				pWeap->SetClipAmmo( 5 );
				pWeap->SetReserveAmmo( (u16)CWeapon::INFINITE_AMMO );

				// give weapon a pointer back to its associated ItemInst object
				pWeap->SetItemInst( &m_pInventory->m_aoWeapons[i][uCurWpnIdx] );

				m_apWeapon[i] = pWeap;
			}
		}
		m_pInventory->m_aoWeapons[0][0].SetAmmoDisplayType(CItemInst::AMMODISPLAY_NUMERIC,CItemInst::AMMODISPLAY_NONE);
	}
	else
	{
		for (i = 0; i < 1; i++) 
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
			m_apWeapon[i]->SetClipAmmo( 5 );
			m_apWeapon[i]->SetReserveAmmo( (u16)CWeapon::INFINITE_AMMO );

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

	// Make primary weapon pass along damage to upper-right arm...
	m_apWeapon[0]->SetBotDamageBoneIndex( m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_R_ARM_ELBOW] ) );

	SetMaxHealth();

	if( m_nPossessionPlayerIndex >= 0 )
	{
		Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.SetNormOrigin( 0.0f, 0.28f );
	}

	// Find approx eye point...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
	if( nBoneIndex < 0 )
	{
		DEVPRINTF( "CBotMozer::ClassHierarchyBuild(): Could not locate approx eye point bone '%s'.\n", m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
		m_pApproxEyePoint_WS = &m_MtxToWorld.m_vPos;
	}
	else
	{
		m_pApproxEyePoint_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
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
		DEVPRINTF( "CBotMozer::ClassHierarchyBuild(): Could not locate gaze bone '%s'.\n", m_apszBoneNameTable[ BONE_HEAD ] );
		m_pGazeDir_WS = &m_MtxToWorld.m_vFront;
	}
	else
	{
		m_pGazeDir_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vFront;
	}

	m_pAISteerMtx = &m_MtxToWorld;

	//MeleeInit( &m_MeleeData, _MELEE_RADIUS, m_pDamageProfileSlash, &(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexGirder]->m_vPos) );
	MeleeInit( &m_MeleeData, _MELEE_RADIUS, m_pDamageProfileSlash, &(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexGirder]->m_vPos) );
	MeleeInit( &m_MeleeData2, _MELEE_RADIUS, m_pDamageProfileSlash, NULL, &m_MeleeData );


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


BOOL CBotMozer::ClassHierarchyBuilt( void )
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


CEntityBuilder *CBotMozer::GetLeafClassBuilder( void )
{
	return &_BotMozerBuilder;
}


void CBotMozer::_ClearDataMembers( void )
{
	// Init data members...
	m_pMoveIdentifier = &m_fUnitAimPitch;

	m_fCollCylinderHeight_WS					= 13.5f;
	m_fCollCylinderRadius_WS					= 5.5f;

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
	m_anAnimStackIndex[ASI_STAND_ALERT] = -1;

	m_anAnimStackIndex[ASI_WALK] = ANIMTAP_WALK;
	m_anAnimStackIndex[ASI_WALK_ALERT] = -1;
	m_anAnimStackIndex[ASI_RUN] = ANIMTAP_RUN;
	m_anAnimStackIndex[ASI_RUN_PANIC] = -1;

	m_anAnimStackIndex[ASI_FALL] = ANIMCONTROL_JUMP_FLY;

	m_anAnimStackIndex[ASI_HOP_LEFT] = -1;
	m_anAnimStackIndex[ASI_HOP_RIGHT] = -1;
	m_anAnimStackIndex[ASI_STARTLE] = -1;
	m_anAnimStackIndex[ASI_ROLL_LEFT] = -1;
	m_anAnimStackIndex[ASI_ROLL_RIGHT] = -1;

	m_anAnimStackIndex[ASI_DOZE_LOOP] = -1;
	m_anAnimStackIndex[ASI_NAPJERK] = -1;
	m_anAnimStackIndex[ASI_WAKE] = -1;

	m_anAnimStackIndex[ASI_RELOAD_CLIP_EJECT_OLD]	= ANIMCONTROL_RELOAD_CLIP_EJECT_OLD;
	m_anAnimStackIndex[ASI_RELOAD_CLIP_GRAB_NEW]	= ANIMCONTROL_RELOAD_CLIP_GRAB_NEW;
	m_anAnimStackIndex[ASI_RELOAD_CLIP_INSERT_NEW]	= ANIMCONTROL_RELOAD_CLIP_INSERT_NEW;
	m_anAnimStackIndex[ASI_RELOAD_CLIP_SLAPIN_NEW]	= ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW;

	m_anAnimStackIndex[ASI_RELOAD_WITH_LEFT_GRAB_AMMO] = -1;
	m_anAnimStackIndex[ASI_RELOAD_WITH_LEFT_DROP_IN] = -1;
	m_anAnimStackIndex[ASI_RELOAD_WITH_LEFT_FINISH] = -1;
	m_anAnimStackIndex[ASI_RELOAD_SINGLE_ARM] = -1;
	m_anAnimStackIndex[ASI_RELOAD_SINGLE_BODY] = -1;

	m_anAnimStackIndex[ASI_SNEAK]			= -1;

	m_anAnimStackIndex[ASI_RC_TETHERED]		= -1;
	m_anAnimStackIndex[ASI_RC_POWER_DOWN]	= -1;
	m_anAnimStackIndex[ASI_RC_POWER_UP]		= -1;
	m_anAnimStackIndex[ASI_STOOP]			= -1;

	m_anAnimStackIndex[ASI_AIM_PILLBOX]		= -1;

	m_pnEnableBoneNameIndexTableForSummer_Normal	  =	 m_anEnableBoneNameIndexTableForSummer_Normal;

	// titan vars
	m_bDoubleJump				= FALSE;
//	m_uSquishedBotDuringJump	= -1;

	// glitch vars
	m_fUnitRecoil = 0.0f;
	m_fUnitRecoilTarget = 0.0f;
	m_fFireAnimSpeedMult = 1.0f;
	m_fUnitFireCtlUpper = 0.0f;
	m_fUnitFireCtlLower = 0.0f;
	m_fFireTimer = 1000.0f;

	// mike start
	m_fUnitAimPitch = 0.0f;
	m_eGirderSwingState = GIRDER_SWING_NONE;
	m_hStreamerEmitter = FXSTREAMER_INVALID_HANDLE;
	//m_nHitEntityCount = 0;
	fang_MemSet(m_pSoundEmitters,0,sizeof(m_pSoundEmitters[0])*MOZERSND_COUNT);
	fforce_NullHandle( &m_hForce );

	m_pPlayerDeathData = &m_PlayerDamageData;
}


void CBotMozer::ClassHierarchyAddToWorld( void )
{
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CBot::ClassHierarchyAddToWorld();
	m_pWorldMesh->UpdateTracker();

	// Primary must be attached
	m_apWeapon[0]->Attach_UnitMtxToParent_PS( this, m_apszBoneNameTable[BONE_ATTACHPOINT_PRIMARY] );
	m_apWeapon[0]->AddToWorld();
	FASSERT(m_apWeapon[0]->IsInWorld());
	m_apWeapon[0]->ResetToState( CWeapon::STATE_DEPLOYED );
}


void CBotMozer::ClassHierarchyRemoveFromWorld( void )
{
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	for (MozerSound_e iSndIdx = (MozerSound_e)0; iSndIdx < MOZERSND_COUNT; iSndIdx = (MozerSound_e)(int(iSndIdx)+1))
	{
		_PlaySnd(iSndIdx, FALSE);
	}
	
	m_apWeapon[0]->RemoveFromWorld();
	m_pWorldMesh->RemoveFromWorld();

	for (MozerSound_e iSndIdx = (MozerSound_e)0; iSndIdx < MOZERSND_COUNT; iSndIdx = (MozerSound_e)(int(iSndIdx)+1))
	{
		_PlaySnd(iSndIdx, FALSE);
	}

	CBot::ClassHierarchyRemoveFromWorld();
}


void CBotMozer::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList)
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


const CFVec3A *CBotMozer::GetApproxEyePoint( void ) const
{
	FASSERT( IsCreated() );

	return m_pApproxEyePoint_WS;
}


void CBotMozer::ClassHierarchyWork()
{
	CFVec3A TempVec3A;

	FASSERT( m_bSystemInitialized );

	CBot::ClassHierarchyWork();

	if( !IsOurWorkBitSet() )
	{
		return;
	}

	Power_Work();
	ParseControls();

	// Save a copy of the previous frame's info...
	m_MountPrevPos_WS = m_MountPos_WS;
	m_nPrevState = m_nState;

	// Apply velocity impulses that were accumulated last frame...
	m_Velocity_WS.Add( m_ImpulseVelocity_WS );

	// Update position...
	TempVec3A.Mul( m_Velocity_WS, FLoop_fPreviousLoopSecs );

	BOOL bImpulseApplied = HandleVelocityImpulses();m_MountPos_WS.Add( TempVec3A );
	
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
	m_pBotToSquishThisFrame = NULL;
	HandleCollision();
	if( m_pBotToSquishThisFrame ) {
		m_pBotToSquishThisFrame->Die();
	}

//	CFVec3A collision;collision.Set(m_CollSphere.m_Pos);
//	fdraw_DevSphere(&collision,m_CollSphere.m_fRadius);

	//_HandleGirderCollision();
	PROTRACK_ENDBLOCK();//"Coll"

	// Move and animate our bot...
	switch( m_nState ) 
	{
	case STATE_GROUND:
		PROTRACK_BEGINBLOCK("GroundXlat");
			HandleGroundTranslation();
			_HandleJumping();
		PROTRACK_ENDBLOCK();//"GroundXlat");

		PROTRACK_BEGINBLOCK("GroundAnim");
			HandleGroundAnimations();
			HandleSleeping();
		PROTRACK_ENDBLOCK();//"GroundAnim");
		break;

	case STATE_AIR:
		PROTRACK_BEGINBLOCK("AirXlat");
			HandleAirTranslation();
			_CheckForDoubleJump();
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
	//_HandleGirderAttacking();
	_HandleGirderAnimations();
	PROTRACK_ENDBLOCK();//"WeaponAnim");

	_pThisBot = this;
	_UpdateMatrices();
	_pThisBot = NULL;

	PROTRACK_BEGINBLOCK("WeaponWork");
	_HandleWeaponWork();
	_HandleGirderWork();
	PROTRACK_ENDBLOCK();//"WeaponWork");


	if (IsDeadOrDying())
	{
		DeathWork();
	}
}


void CBotMozer::_HandleJumping( void )
{
	if( !m_bControlsBot_JumpVec ) 
	{
		if( m_bControlsBot_Jump2 ) 
		{
			m_vJumpVelocity		= m_Velocity_WS;
			m_vJumpVelocity.y	= m_pBotInfo_Jump->fVerticalVelocityJump2;
			_StartSingleJump();
			_StartSingleJump();
		}
		else if( m_bControls_Jump ) 
		{
			m_vJumpVelocity		= m_Velocity_WS;
			m_vJumpVelocity.y	= m_pBotInfo_Jump->fVerticalVelocityJump1;
			_StartSingleJump();
		}
	} 
	else
	{
		m_vJumpVelocity = m_ControlsBot_JumpVelocity_WS;
//		FMATH_CLAMP( m_ControlsBot_JumpVelocity_WS.y, 0.0f, m_fMaxDeltaJumpVelY_WS );
		_StartSingleJump();
	}
}


void CBotMozer::_EnterFlyMode( void ) 
{
	m_nJumpState	= BOTJUMPSTATE_AIR;
	SetJumping();
	m_fUnitFlyBlend = 1.0f;
	m_bDoubleJump	= FALSE;
}


BOOL CBotMozer::NotifyBotCollision( CBot *pBot ) {
	FASSERT( pBot );

	
	if( (m_nJumpState == BOTJUMPSTATE_AIR2) && (pBot->IsSquishy()) ) {
		m_pBotToSquishThisFrame = pBot;
		//pBot->Die();
		return FALSE;
	} else {
		return TRUE;		// let bot collision handle the collision normally
	}
}


void CBotMozer::_HandleLanding( void ) 
{
	MakeFootImpactDust( TRUE, TRUE, m_fUnitFlyBlend, &m_FeetToGroundCollImpact.UnitFaceNormal );

	ClearJumping();
	m_fGravityMultiplier	= 1.0f;

	//are we doing the stomp jump?
	if( m_nJumpState == BOTJUMPSTATE_AIR2 ) 
	{
		m_nJumpState	= BOTJUMPSTATE_LAND2;
		
		ZeroTime( ANIMTAP_JUMP2_LAND );
		SetControlValue( ANIMCONTROL_JUMP2_LAND,	1.0f );
		
		CFXShockwave::AddShockwave( m_MountPos_WS, 3.0f, 2.5f, ENTITY_BIT_BOT, m_BotInfo_Mozer.fMaxShockwaveImpulse, m_BotInfo_Mozer.fMinShockwaveImpulse );

		//if( m_uSquishedBotDuringJump != -1 ) 
		//{
		//	//if( m_uSquishedBotDuringJump == BOTCLASS_MINER ) 
		//	//{
		//	//	CSplat::AddSplat( CSplat::SPLATTYPE_SQUISHED_GLITCH, 6.0f, m_MountPos_WS, m_MountUnitFrontXZ_WS );
		//	//} 
		//	//else if( m_uSquishedBotDuringJump == BOTCLASS_GRUNT ) 
		//	//{
		//	//	CSplat::AddSplat( CSplat::SPLATTYPE_SQUISHED_GRUNT, 6.0f, m_MountPos_WS, m_MountUnitFrontXZ_WS );
		//	//}

		//	//do the sparks thing
		//	// fparticle_SpawnEmitter( m_hSparksPartDef, m_MountPos_WS.v3, &CFVec3A::m_UnitAxisY.v3, 1.0f );

		//	m_uSquishedBotDuringJump = -1;
		//}

		ZeroTime( ANIMTAP_WALK );
		m_fUnitWalkBlend = 0.0f;
				
		//stick
		m_Velocity_WS.x			= 0.0f;
		m_Velocity_WS.z			= 0.0f;
		VelocityHasChanged();

		//if we're player controlled, shake our camera
		if( m_nPossessionPlayerIndex >= 0 ) 
		{
			fcamera_GetCameraByIndex( m_nPossessionPlayerIndex )->ShakeCamera( 0.25f, 0.25f );
			fforce_Kill( &m_hForce );
			fforce_Play( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROCKET_THRUST_HEAVY, &m_hForce );
		}

		// transition the camera back to normal
		SetCameraOverride(FALSE,1.5f);

		//play the stomp jump sound
		if( m_nPossessionPlayerIndex < 0 ) 
		{ 
//			fsndfx_Play3D( m_BotInfo_TitanSounds.hSndStomp, &m_MountPos_WS, m_BotInfo_Titan.fSoundRadius );
		}
		else
		{
//			fsndfx_Play2D( m_BotInfo_TitanSounds.hSndStomp );
		}

		if( m_pStickyEntity && m_pStickyEntity->GetVerlet() ) 
		{
			CFVec3A ImpulseVec_WS;

			ImpulseVec_WS.Set( 0.0f, -2.0f*m_pBotInfo_Gen->fPhysForce_Land, 0.0f );
			m_pStickyEntity->GetVerlet()->ApplyImpulse( &m_MtxToWorld.m_vPos, &ImpulseVec_WS );
		}
	} 
	else if( (m_nJumpState != BOTJUMPSTATE_LAND) && (m_nJumpState != BOTJUMPSTATE_LAND2) ) 
	{
		m_nJumpState	= BOTJUMPSTATE_LAND;
		ZeroTime( ANIMTAP_JUMP_LAND_UPPER );
		SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, m_fUnitFlyBlend );

		if( m_nPossessionPlayerIndex > -1 ) 
		{
			fforce_Kill( &m_hForce );
			fforce_Play( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROCKET_THRUST_LIGHT, &m_hForce );
		}

		// if we're moving slower than walk, just play the lower body jump anim, otherwise blend it in
		if( m_fSpeedXZ_WS < m_pBotInfo_Walk->fMinWalkVelocity ) 
		{
            SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, m_fUnitFlyBlend );
			m_bPlayLandAnimLower = TRUE;
		} 
		else 
		{
			//stick a little bit
			m_Velocity_WS.Mul( 0.75f );
			VelocityHasChanged();
	
			SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
			m_bPlayLandAnimLower	= TRUE;
//			m_bPlayLandAnimLower	= TRUE;
//			m_fMaxLandUnitBlend		= 1.0f - fmath_Div( m_fSpeedXZ_WS - m_pBotInfo_Walk->fMinWalkVelocity, m_pBotInfo_Walk->fMaxXlatVelocity );
			m_fMaxLandUnitBlend = 1.0f;
			FMATH_CLAMPMAX( m_fMaxLandUnitBlend, m_fUnitFlyBlend );
			SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, m_fMaxLandUnitBlend );
		}

		if( m_pStickyEntity && m_pStickyEntity->GetVerlet() ) 
		{
			CFVec3A ImpulseVec_WS;

			ImpulseVec_WS.Set( 0.0f, m_pBotInfo_Gen->fPhysForce_Land, 0.0f );
			m_pStickyEntity->GetVerlet()->ApplyImpulse( &m_MtxToWorld.m_vPos, &ImpulseVec_WS );
		}
	}

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH,	0.0f );
	SetControlValue( ANIMCONTROL_JUMP_FLY,		0.0f );
	SetControlValue( ANIMCONTROL_JUMP2_FLY,		0.0f );


	if ( m_nPossessionPlayerIndex >= 0 )	
	{
		if( m_pBotInfo_Sound && m_nSurfaceTypeOn != SURFACE_TYPE_NONE) 
		{
			// Don't play scuff sounds when landing from a jump
			fsndfx_Play2D(
				m_pBotInfo_Sound->ahStepSound[m_nSurfaceTypeOn][ fmath_RandomRange( 0, ( BOTSOUND_SCUFF_SOUND_START - 1 ) ) ],
				m_pBotInfo_Walk->fJumpLandedSoundUnitVolume_2D * m_fStepVolumeScale
			);
		}
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
}


void CBotMozer::_StartSingleJump( const CFVec3A *pJumpVelocity_WS ) 
{
	// titan can only jump after completing landing
	if( m_nJumpState != BOTJUMPSTATE_LAUNCH && m_nJumpState != BOTJUMPSTATE_AIR && m_nJumpState != BOTJUMPSTATE_NONE ) 
	{
		return;	
	}

	if( IsJumping() && (m_nJumpState == BOTJUMPSTATE_LAUNCH) ) 
	{ // we were already jumping, do the stomp thing
		m_bDoubleJump		= TRUE;

		// start the camera transition to watch the landing:
		SetCameraOverride(TRUE,0.25f,TRUE,m_vecStompCameraLookat,m_vecStompCameraPos);
		m_fGravityMultiplier	= 3.0f;

	} 
	else 
	{
		ZeroTime( ANIMTAP_JUMP_LAUNCH );
		ZeroTime( ANIMTAP_JUMP_FLY );
		ZeroTime( ANIMTAP_JUMP_LAND_UPPER );
		ZeroTime( ANIMTAP_JUMP2_FLY );
		ZeroTime( ANIMTAP_JUMP2_LAND );
		m_bDoubleJump = FALSE;
		m_nJumpState	= BOTJUMPSTATE_LAUNCH;
		SetJumping();
		f32 fPlayJumpLaunchFromUnitTime = 0.0f;

		if( m_fSpeedXZ_WS > m_BotInfo_Walk.fMinWalkVelocity )
			fPlayJumpLaunchFromUnitTime =0.8f;

		UpdateUnitTime( ANIMTAP_JUMP_LAUNCH , fPlayJumpLaunchFromUnitTime );
	}

	SetDoubleJumpTimer( m_pBotInfo_Gen->fDoubleJumpTime );
}

void CBotMozer::_CheckForDoubleJump( void ) 
{
	if( !HaveTimeToDoubleJump() )
	{
		return;
	}

	if( m_bControls_Jump ) 
	{
		m_bDoubleJump		= TRUE;

		// start the camera transition to watch the landing:
		SetCameraOverride(TRUE,0.25f,TRUE,m_vecStompCameraLookat,m_vecStompCameraPos);
		m_fGravityMultiplier	= 3.0f;
		m_Velocity_MS.x = 0.0f;
		m_Velocity_MS.z = 0.0f;
		m_Velocity_WS.x = 0.0f;
		m_Velocity_WS.z = 0.0f;
	}
}

#define _LAUNCH_TAKEOFF_THRESHOLD		( 0.66f )

void CBotMozer::_HandleJumpAnimations( void ) 
{
	f32 fUnitTime;

	switch( m_nJumpState ) 
	{
		case BOTJUMPSTATE_LAUNCH:
			fUnitTime = GetUnitTime( ANIMTAP_JUMP_LAUNCH );
			if( !DeltaTime( ANIMTAP_JUMP_LAUNCH, FLoop_fPreviousLoopSecs, TRUE )) 
			{
				m_fUnitFlyBlend = GetControlValue(ANIMCONTROL_JUMP_LAUNCH);
				// if launches plays from 0-1, we get full control at midpoint, from .80-1 it's at 40%
				m_fUnitFlyBlend  = m_fUnitFlyBlend + 5.0f * GetOOTotalTime(ANIMTAP_JUMP_LAUNCH) * FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMAX( m_fUnitFlyBlend, 1.0f );
				SetControlValue( ANIMCONTROL_JUMP_LAUNCH, m_fUnitFlyBlend );
				FASSERT_UNIT_FLOAT( m_fUnitFlyBlend );

				if( (fUnitTime < _LAUNCH_TAKEOFF_THRESHOLD) && (GetUnitTime( ANIMTAP_JUMP_LAUNCH ) >= _LAUNCH_TAKEOFF_THRESHOLD) ) {
					m_Velocity_WS.y += m_pBotInfo_Jump->fVerticalVelocityJump1 * m_fJumpMultiplier;
					m_Velocity_MS.y += m_pBotInfo_Jump->fVerticalVelocityJump1 * m_fJumpMultiplier;
				}
			}
			else
			{
				// all done
				//m_Velocity_WS.y += m_pBotInfo_Jump->fVerticalVelocityJump1 * m_fJumpMultiplier;
				//m_Velocity_MS.y += m_pBotInfo_Jump->fVerticalVelocityJump1 * m_fJumpMultiplier;
				SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
				SetJumping();

				SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );

				if( m_bDoubleJump ) 
				{ //stomp jump requested
					ZeroTime( ANIMTAP_JUMP2_FLY );	
					SetControlValue( ANIMCONTROL_JUMP2_FLY, 1.0f );
					m_nJumpState  = BOTJUMPSTATE_AIR2;
					m_bDoubleJump = FALSE;
					m_Velocity_WS = m_vJumpVelocity;
					m_vJumpVelocity.y	= m_pBotInfo_Jump->fVerticalVelocityJump2* m_fJumpMultiplier;
					VelocityHasChanged();
				} 
				else 
				{	//normal jump requested
					ZeroTime( ANIMTAP_JUMP_FLY );	
					SetControlValue( ANIMCONTROL_JUMP_FLY, m_fUnitFlyBlend );
					m_nJumpState = BOTJUMPSTATE_AIR;
					m_Velocity_WS = m_vJumpVelocity;
					VelocityHasChanged();
				}
			}
			break;

		case BOTJUMPSTATE_AIR:
			if( m_fUnitFlyBlend < 1.0f ) 
			{
                m_fUnitFlyBlend += FLoop_fPreviousLoopSecs;
				FMATH_CLAMP_MAX1( m_fUnitFlyBlend );
				SetControlValue( ANIMCONTROL_JUMP_FLY, m_fUnitFlyBlend );
			}
			DeltaTime( ANIMTAP_JUMP_FLY );

			if( m_bDoubleJump ) 
			{
				ZeroTime( ANIMTAP_JUMP2_FLY );
				m_nJumpState = BOTJUMPSTATE_AIR2;
				SetControlValue( ANIMCONTROL_JUMP2_FLY, 1.0f );
			}

			if( m_nState == STATE_GROUND ) 
			{
				_HandleLanding();
			}
			break;

		case BOTJUMPSTATE_AIR2:
			DeltaTime( ANIMTAP_JUMP2_FLY, FLoop_fPreviousLoopSecs, TRUE );

			if( m_nState == STATE_GROUND ) 
			{
				_HandleLanding();
			}
			break;

		case BOTJUMPSTATE_LAND:
			if( m_fUnitFlyBlend > 0.0f ) 
			{
				DeltaTime( ANIMTAP_JUMP_LAND_UPPER, FLoop_fPreviousLoopSecs, TRUE );
				m_fUnitFlyBlend -= FLoop_fPreviousLoopSecs;
				FMATH_CLAMP_UNIT_FLOAT( m_fUnitFlyBlend );
				SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, m_fUnitFlyBlend );
				if( m_bPlayLandAnimLower ) 
				{
					fUnitTime = GetControlValue( ANIMCONTROL_JUMP_LAND_LOWER );
					fUnitTime -= FLoop_fPreviousLoopSecs;
					FMATH_CLAMP_MIN0( fUnitTime );
					SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, fUnitTime );
				}
			} 
			else 
			{
				// jump over
				m_nJumpState = BOTJUMPSTATE_NONE;
				SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
			}
			break;

		case BOTJUMPSTATE_LAND2:
			fUnitTime = GetUnitTime( ANIMTAP_JUMP2_LAND );
			if( !DeltaTime( ANIMTAP_JUMP2_LAND, FLoop_fPreviousLoopSecs, TRUE ) ) 
			{

				if( fUnitTime > 0.8f ) {
					SetControlValue( ANIMCONTROL_JUMP2_LAND, 1.0f - (fUnitTime - 0.8f) * 5.0f );
				}

//				fUnitTime = (1.0f - fUnitTime) * 1.0f;
//				FMATH_CLAMP_UNIT_FLOAT( fUnitTime );
//				SetControlValue( ANIMCONTROL_JUMP2_LAND, fUnitTime );

				m_Velocity_WS.x			= 0.0f;
				m_Velocity_WS.z			= 0.0f;
				VelocityHasChanged();

			}
			else
			{
				m_fUnitFlyBlend = 0.0f;
				m_nJumpState	= BOTJUMPSTATE_NONE;
				SetControlValue( ANIMCONTROL_JUMP2_LAND, 0.0f );
			}
			break;

		default:
			if( m_nState == STATE_GROUND && m_fUnitFlyBlend > 0.0f ) 
			{
				_HandleLanding();
			}

			//// blend out of landing animations if some are still active
			//fUnitTime = GetControlValue( ANIMCONTROL_JUMP2_LAND );
			//if( fUnitTime > 0.0f ) {
			//	fUnitTime -= FLoop_fPreviousLoopSecs * 8.0f;
			//	FMATH_CLAMP_MIN0( fUnitTime );
			//	SetControlValue( ANIMCONTROL_JUMP2_LAND, fUnitTime );
			//}
			break;
	}
}


void CBotMozer::_HandleGirderWork( void ) {
	if( IsAttackingWithMelee() ) {
		CFVec3A vTmp;
		_CalcGirderMeleePos( &vTmp );
		MeleeSwing( &m_MeleeData );
		MeleeSwing( &m_MeleeData2, &vTmp );
		
		CFVec3A vGirderMultDir;
		const CFMtx43A* pSecondaryFireMtx = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexSecondaryFire];
		vGirderMultDir.Mul(pSecondaryFireMtx->m_vUp,m_BotInfo_Girder.fStreamerLength);
		m_mtxStreamerWS.Set(*pSecondaryFireMtx);
		m_mtxStreamerWS.m_vPos.Sub(vGirderMultDir);
	}
}


#if 0
void CBotMozer::_HandleGirderWork( void )
{
	if (m_vFirePosPrev.DistSq(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexSecondaryFire]->m_vPos) > 0.01f)
	{
		m_vFireUnitDir.Set(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexSecondaryFire]->m_vPos);
		m_vFireUnitDir.Sub(m_vFirePosPrev);
		m_vFireUnitDir.Unitize();
		m_vFirePosPrev.Set(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexSecondaryFire]->m_vPos);
	}

//	CFVec3A vGirderCollSphereWS(m_GirderCollSphereWS.m_Pos);
//	fdraw_DevSphere(&vGirderCollSphereWS,m_GirderCollSphereWS.m_fRadius);

	//ME - doesn't seem to be used for anything
	CFVec3A vGirderPos;
	vGirderPos.Set( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexGirder]->m_vPos);

//	CFColorRGBA red;red.OpaqueRed();
//	CFColorRGBA white;white.OpaqueWhite();
//	CFColorRGBA blue;blue.OpaqueBlue();
//	CFColorRGBA yellow;yellow.OpaqueYellow();
	
	//ME - doesn't seem to be used for anything
	CFVec3A vSecondaryFirePos;
	vSecondaryFirePos.Set( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexSecondaryFire]->m_vPos);

//	fdraw_DevSphere(&vPrimaryFirePos,1.0f,&blue,6,4,6);
//	fdraw_DevSphere(&m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexGirder]->m_vPos,1.0f,&blue,6,4,6);
//	CFVec3A vGirderEnd;
//	vGirderEnd.Mul(m_vFireUnitDir,5.0f).Add(vGirderPos);
//	fdraw_DevLine(&vGirderPos,&vGirderEnd,&blue);

	if (!IsSwinging())
//	if( !IsAttackingWithMelee() )
	{
		// no swinging means no swinging collision
		return;
	}
/*
	CFColorRGBA red;red.OpaqueRed();
	CFColorRGBA green;green.OpaqueGreen();
	CFColorRGBA blue;blue.OpaqueBlue();
	CFColorRGBA cyan;cyan.OpaqueCyan();
	CFVec3A vGird,vStreamOn,vStreamOff,vFire;
	vGird = vGirderPos;
	vStreamOn = m_mtxStreamerWS.m_vPos;
	vFire = vSecondaryFirePos;
	vStreamOff.Mul(m_mtxStreamerWS.m_vUp, m_BotInfo_Girder.fStreamerLength).Add(m_mtxStreamerWS.m_vPos);
	fdraw_DevSphere(&vGird,1.0f,&red);
	fdraw_DevSphere(&vStreamOn,1.0f,&green);
	fdraw_DevSphere(&vStreamOff,1.0f,&cyan);
	fdraw_DevSphere(&vFire,1.0f,&blue);
*/
	//m_nHitEntityCount = 0; // haven't connected yet

	_pThisBot = this;
	FWorld_nTrackerSkipListCount = 0;
	this->AppendTrackerSkipList();
	
	m_pMeleeBotToDamageThisFrame = NULL;
	fworld_FindTrackersIntersectingSphere(	&m_GirderCollSphereWS,
											FWORLD_TRACKERTYPE_MESH,
											_SwingStrikeCallback,
											FWorld_nTrackerSkipListCount,
											FWorld_apTrackerSkipList,
											m_pWorldMesh,
											MESHTYPES_ENTITY,
											ENTITY_BIT_BOT|ENTITY_BIT_WEAPON|ENTITY_BIT_MESHENTITY);

	if( m_pMeleeBotToDamageThisFrame ) {
		_InflictSlashDamage( m_pMeleeBotToDamageThisFrame, m_pMeleeBotToDamageThisFrame->m_pWorldMesh, &FColl_aImpactBuf[0] );

	}

	_pThisBot = NULL;
}
#endif

void CBotMozer::UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup )
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

void CBotMozer::_UpdateMatrices( void ) 
{
	// calculate unit aim pitch for the bone callbacks
	m_fUnitAimPitch = fmath_Div( m_fMountPitch_WS - m_fMountPitchMin_WS, m_fMountPitchMax_WS - m_fMountPitchMin_WS );
	FMATH_CLAMP_UNIT_FLOAT( m_fUnitAimPitch );
	
	CFMtx43A	EntityMtxToWorld;
	CFMtx43A::m_XlatRotY.SetRotationY( m_fMountYaw_WS + m_fLegsYaw_MS );
	CFMtx43A::m_XlatRotY.m_vPos = m_MountPos_WS;

	m_pWorldMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_XlatRotY );
	m_pWorldMesh->UpdateTracker();

	PROTRACK_BEGINBLOCK("ComputeMtxPal");
		ComputeMtxPalette( TRUE );
	PROTRACK_ENDBLOCK();//"ComputeMtxPal");

	m_GazeUnitVec_WS.ReceiveUnit( *m_pGazeDir_WS );

	EntityMtxToWorld.Identity();
	EntityMtxToWorld.SetRotationYXZ( m_fMountYaw_WS, m_fMountPitch_WS, 0.0f );
	EntityMtxToWorld.m_vPos = m_MountPos_WS;
	Relocate_RotXlatFromUnitMtx_WS( &EntityMtxToWorld, TRUE, m_pMoveIdentifier );
}

void CBotMozer::_HandleWeaponFiring( void ) 
{
	f32 fTrig1;
	u32 nFireBitsPrimary;

	fTrig1 = ( IsSwinging() ? 0.0f : m_fControls_Fire1); 

	nFireBitsPrimary = m_apWeapon[0]->TriggerWork( fTrig1, 0.0f, &m_TargetedPoint_WS );

	if( nFireBitsPrimary ) 
	{
		// We fired the primary weapon...
		JustFired();
		m_fFireTimer = 0.0f;
		FASSERT( m_apWeapon[0] != NULL );
		FASSERT_UNIT_FLOAT( m_apWeapon[0]->m_pInfo->fUnitRecoil );
		_Recoil( m_apWeapon[0]->m_pInfo->fUnitRecoil );

		FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM );
		ZeroTime( ANIMTAP_FIRE_1_UPPER );
	} 
	else 
	{
		// Primary weapon not fired...
	}
}

void CBotMozer::_HandleWeaponAnimations(void)
{
	HandleWeaponReloadingAnimations();
	_HandleFiringAnimations();
	_HandleRecoil();
}

void CBotMozer::_HandleFiringAnimations( void ) 
{
	// Primary fire...
	if( m_nBotFlags & BOTFLAG_PLAY_FIRE1_ANIM ) 
	{
		// blend in primary fire animation(s)
		if( m_fUnitFireCtlUpper < 1.0f ) 
		{
			m_fUnitFireCtlUpper = 1.0f;
			m_fUnitFireCtlLower = 1.0f;

			SetControlValue( ANIMCONTROL_FIRE_1_UPPER, m_fUnitFireCtlUpper );
//			SetControlValue( ANIMCONTROL_FIRE_1_LOWER, m_fUnitFireCtlLower );
		}

		FASSERT( m_apWeapon[0] && m_apWeapon[0]->m_pInfo->nStanceType == CWeapon::STANCE_TYPE_STANDARD );
		
		// playing one handed fire animation
		m_fFireTimer += FLoop_fPreviousLoopSecs;

		// hold the pose until time is up, unless clip is empty
		if( (m_fFireTimer > _FIRE_ANIM_HOLD_TIME) || m_apWeapon[0]->IsClipEmpty() ) 
		{
			FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM );
		}
	} 
	else 
	{
		m_fFireTimer += FLoop_fPreviousLoopSecs;
		FASSERT( m_apWeapon[0] && (m_apWeapon[0]->m_pInfo->nStanceType == CWeapon::STANCE_TYPE_STANDARD) );
		
		// handle animations for standard one handed weapons
		if( (m_fFireTimer < _FIRE_ANIM_READY_TIME) ) 
		{
			if( m_fUnitFireCtlUpper > _FIRE_READY_BLEND ) 
			{
				m_fUnitFireCtlUpper -= FLoop_fPreviousLoopSecs * _FIRE_ANIM_OO_OFF_TIME;
				FMATH_CLAMPMIN( m_fUnitFireCtlUpper, _FIRE_READY_BLEND );
				SetControlValue( ANIMCONTROL_FIRE_1_UPPER, m_fUnitFireCtlUpper );
			}
		} 
		else 
		{
			// blend out primary fire (upper)
			if( m_fUnitFireCtlUpper > 0.0f ) 
			{
				// if control is above _FIRE_READY_BLEND, we are probably going into a different animation.  Blend out quickly
				if( m_fUnitFireCtlUpper > _FIRE_READY_BLEND ) 
				{
					m_fUnitFireCtlUpper -= FLoop_fPreviousLoopSecs * _FIRE_ANIM_OO_ON_TIME;        
					FMATH_CLAMP_MIN0( m_fUnitFireCtlUpper );
					SetControlValue( ANIMCONTROL_FIRE_1_UPPER, m_fUnitFireCtlUpper );
				}
				else
				{
					m_fUnitFireCtlUpper -= FLoop_fPreviousLoopSecs * _FIRE_ANIM_OO_OFF_TIME;
					FMATH_CLAMP_MIN0( m_fUnitFireCtlUpper );
					SetControlValue( ANIMCONTROL_FIRE_1_UPPER, m_fUnitFireCtlUpper );
				}
			}

			// blend out primary fire (lower)
			if( !DeltaTime( ANIMTAP_FIRE_1_UPPER, FLoop_fPreviousLoopSecs, TRUE  ) ) 
			{
			}
			else 
			{
				if( m_fUnitFireCtlLower > 0.0f ) 
				{
					m_fUnitFireCtlLower -= FLoop_fPreviousLoopSecs * _FIRE_ANIM_OO_OFF_TIME;
					FMATH_CLAMP_MIN0( m_fUnitFireCtlLower );
//					SetControlValue( ANIMCONTROL_FIRE_1_LOWER, m_fUnitFireCtlLower );
				}
			}
		}
	}
}

#define _MELEE_WINDUP_RATE	( 2.5f )


void CBotMozer::_HandleGirderAnimations( void )
{
	BOOL bTryToSwing = FALSE;

	// button is pressed, and we're not cabling
	if (m_fControls_Fire2 > 0.1f && !m_pCableHook)
	{
		bTryToSwing = TRUE;
	}

//	floop_SetTimeScale(m_BotInfo_Girder.fTimeScale); 
	//if (m_eGirderSwingState == GIRDER_SWING_NONE)
	//	floop_SetTimeScale(1.0f); 

	switch(m_eGirderSwingState)
	{
	case GIRDER_SWING_NONE: // opportunity to get a state
		if (bTryToSwing)
		{
			ZeroTime(ANIMTAP_SWING_WINDUP_UPPER);
			m_eGirderSwingState = GIRDER_SWING_WINDUP;
		}
		break;

	case GIRDER_SWING_WINDUP:
		if( !DeltaTime(ANIMTAP_SWING_WINDUP_UPPER,FLoop_fPreviousLoopSecs * _MELEE_WINDUP_RATE,TRUE) )// means we're still charging
		{
			// means that at 1/3d pt. of animation, blend is at 100%
			f32 fUnitTime = 3.0f * fmath_UnitLinearToSCurve( GetUnitTime( ANIMTAP_SWING_WINDUP_UPPER) );
			FMATH_CLAMPMAX( fUnitTime, 1.0f );
			SetControlValue( ANIMCONTROL_SWING_WINDUP_UPPER, fUnitTime);
			SetControlValue( ANIMCONTROL_SWING_WINDUP_LOWER, fUnitTime);
		}
		else
		{
			SetControlValue( ANIMCONTROL_SWING_WINDUP_UPPER, 0.0f);
			SetControlValue( ANIMCONTROL_SWING_WINDUP_LOWER, 0.0f);
			SetControlValue( ANIMCONTROL_SWING_FORWARD_UPPER, 1.0f);
			SetControlValue( ANIMCONTROL_SWING_FORWARD_LOWER, 1.0f);
			ZeroTime(ANIMTAP_SWING_FORWARD_UPPER);
			m_eGirderSwingState = GIRDER_SWING_FORWARD;
			_PlaySnd(MOZERSND_SWIPEFORWARD, TRUE);
			//_StartGirderSwing();
			MeleeStartAttack( &m_MeleeData );
			CFVec3A vTmp;
			_CalcGirderMeleePos( &vTmp );
			MeleeStartAttack( &m_MeleeData2, &vTmp );
		}
		break;

	case GIRDER_SWING_FORWARD:
		if( !DeltaTime(ANIMTAP_SWING_FORWARD_UPPER,FLoop_fPreviousLoopSecs * m_BotInfo_Girder.fSwingSpeedForward,TRUE))
		{
			f32 fUnitTime = GetUnitTime(ANIMTAP_SWING_FORWARD_UPPER);
			if ( (fUnitTime > m_BotInfo_Girder.fSwingSwingForwardStreamerOn) &&
				 (fUnitTime < m_BotInfo_Girder.fSwingSwingForwardStreamerFadeOff) )
			{
				_StreamerOn(0.0f);
			}
			else if ( (fUnitTime > m_BotInfo_Girder.fSwingSwingForwardStreamerFadeOff) &&
					  (fUnitTime < m_BotInfo_Girder.fSwingSwingForwardStreamerOff) )
			{
				f32 fFadeRange = m_BotInfo_Girder.fSwingSwingForwardStreamerOff - m_BotInfo_Girder.fSwingSwingForwardStreamerFadeOff;
				if (fFadeRange!=0.0f)
				{
					f32 fFade = fmath_Div(fUnitTime-m_BotInfo_Girder.fSwingSwingForwardStreamerFadeOff,fFadeRange);
					_StreamerOn(fFade);
				}
			}
			else
				_StreamerOn(1.0f);
		}
		else
		{
			SetControlValue( ANIMCONTROL_SWING_FORWARD_UPPER, 0.0f);
			SetControlValue( ANIMCONTROL_SWING_FORWARD_LOWER, 0.0f);
			_StreamerOn(1.0f);

			if (bTryToSwing)
			{
				SetControlValue( ANIMCONTROL_SWING_BACK_UPPER, 1.0f);
				SetControlValue( ANIMCONTROL_SWING_BACK_LOWER, 1.0f);
				ZeroTime(ANIMTAP_SWING_BACK_UPPER);
				m_eGirderSwingState = GIRDER_SWING_BACK;
				_PlaySnd(MOZERSND_SWIPEBACK, TRUE);
				//_StartGirderSwing();
				MeleeStartAttack( &m_MeleeData );
				CFVec3A vTmp;
				_CalcGirderMeleePos( &vTmp );
				MeleeStartAttack( &m_MeleeData2, &vTmp );
			}
			else
			{
				SetControlValue( ANIMCONTROL_SWING_FORWARD_DONE_UPPER, 1.0f);
				SetControlValue( ANIMCONTROL_SWING_FORWARD_DONE_LOWER, 1.0f);
				ZeroTime(ANIMTAP_SWING_FORWARD_DONE_UPPER);
				m_eGirderSwingState = GIRDER_SWING_FORWARD_DONE;
			}
		}
		break;
	
	case GIRDER_SWING_FORWARD_DONE:
		if (bTryToSwing) // return to swinging state
		{
			m_eGirderSwingState = GIRDER_SWING_FORWARD_DONE_UNDO;
		}
		else // blend out
		{
			if (!DeltaTime(ANIMTAP_SWING_FORWARD_DONE_UPPER,FLoop_fPreviousLoopSecs, TRUE))
			{
				f32 fUnitTime = GetUnitTime( ANIMTAP_SWING_FORWARD_DONE_UPPER );
				fUnitTime = 1.0f - fUnitTime;
				fUnitTime = fmath_UnitLinearToSCurve( fUnitTime );
				SetControlValue( ANIMCONTROL_SWING_FORWARD_DONE_UPPER, fUnitTime);
				SetControlValue( ANIMCONTROL_SWING_FORWARD_DONE_LOWER, fUnitTime);
			}
			else
			{
				SetControlValue( ANIMCONTROL_SWING_FORWARD_DONE_UPPER, 0.0f);
				SetControlValue( ANIMCONTROL_SWING_FORWARD_DONE_LOWER, 0.0f);
				m_eGirderSwingState= GIRDER_SWING_NONE;
			}
		}
		break;
		
	case GIRDER_SWING_BACK:
		if( !DeltaTime(ANIMTAP_SWING_BACK_UPPER,FLoop_fPreviousLoopSecs * m_BotInfo_Girder.fSwingSpeedBackward,TRUE) )
		{
			f32 fUnitTime = GetUnitTime(ANIMTAP_SWING_BACK_UPPER);
			if ( (fUnitTime > m_BotInfo_Girder.fSwingSwingBackwardStreamerOn) &&
				(fUnitTime < m_BotInfo_Girder.fSwingSwingBackwardStreamerFadeOff) )
			{
				_StreamerOn(0.0f);
			}
			else if ( (fUnitTime > m_BotInfo_Girder.fSwingSwingBackwardStreamerFadeOff) &&
					  (fUnitTime < m_BotInfo_Girder.fSwingSwingBackwardStreamerOff) )
			{
				f32 fFadeRange = m_BotInfo_Girder.fSwingSwingBackwardStreamerOff - m_BotInfo_Girder.fSwingSwingBackwardStreamerFadeOff;
				if (fFadeRange!=0.0f)
				{
					f32 fFade = fmath_Div(fUnitTime-m_BotInfo_Girder.fSwingSwingBackwardStreamerFadeOff,fFadeRange);
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

			if (bTryToSwing)
			{
				SetControlValue( ANIMCONTROL_SWING_FORWARD_UPPER, 1.0f);
				SetControlValue( ANIMCONTROL_SWING_FORWARD_LOWER, 1.0f);
				ZeroTime(ANIMTAP_SWING_FORWARD_UPPER);
				m_eGirderSwingState = GIRDER_SWING_FORWARD;
				_PlaySnd(MOZERSND_SWIPEFORWARD, TRUE);
			}
			else
			{
				SetControlValue( ANIMCONTROL_SWING_BACK_DONE_UPPER, 1.0f);
				SetControlValue( ANIMCONTROL_SWING_BACK_DONE_LOWER, 1.0f);
				ZeroTime(ANIMTAP_SWING_BACK_DONE_UPPER);
				m_eGirderSwingState = GIRDER_SWING_BACK_DONE;
			}
		}
		break;

	case GIRDER_SWING_BACK_DONE:
		if (bTryToSwing) // return to swinging state
		{
			m_eGirderSwingState = GIRDER_SWING_BACK_DONE_UNDO;
		}
		else // blend out
		{
			if (!DeltaTime(ANIMTAP_SWING_BACK_DONE_UPPER,FLoop_fPreviousLoopSecs, TRUE))
			{
				f32 fUnitTime = GetUnitTime( ANIMTAP_SWING_BACK_DONE_UPPER );
				fUnitTime = 1.0f - fUnitTime;
				fUnitTime = fmath_UnitLinearToSCurve( fUnitTime );
				SetControlValue( ANIMCONTROL_SWING_BACK_DONE_UPPER, fUnitTime);
				SetControlValue( ANIMCONTROL_SWING_BACK_DONE_LOWER, fUnitTime);
			}
			else
			{
				SetControlValue( ANIMCONTROL_SWING_BACK_DONE_UPPER, 0.0f);
				SetControlValue( ANIMCONTROL_SWING_BACK_DONE_LOWER, 0.0f);
				m_eGirderSwingState= GIRDER_SWING_NONE;
			}
		}
		break;
	case GIRDER_SWING_BACK_DONE_UNDO:
		{
			DeltaTime(ANIMTAP_SWING_BACK_DONE_UPPER, -FLoop_fPreviousLoopSecs, TRUE);
			f32 fUnitTime = GetUnitTime( ANIMTAP_SWING_BACK_DONE_UPPER );
			fUnitTime = 1.01f - fUnitTime;
			fUnitTime = fmath_UnitLinearToSCurve( fUnitTime );
			SetControlValue( ANIMCONTROL_SWING_BACK_DONE_UPPER, fUnitTime);
			SetControlValue( ANIMCONTROL_SWING_BACK_DONE_LOWER, fUnitTime);

			if (GetUnitTime(ANIMTAP_SWING_BACK_DONE_UPPER)==0.0f)
			{
				SetControlValue( ANIMCONTROL_SWING_BACK_DONE_UPPER, 0.0f);
				SetControlValue( ANIMCONTROL_SWING_BACK_DONE_LOWER, 0.0f);
				SetControlValue( ANIMCONTROL_SWING_FORWARD_UPPER, 1.0f);
				SetControlValue( ANIMCONTROL_SWING_FORWARD_LOWER, 1.0f);
				ZeroTime(ANIMTAP_SWING_FORWARD_UPPER);
				m_eGirderSwingState = GIRDER_SWING_FORWARD;
				_PlaySnd(MOZERSND_SWIPEFORWARD, TRUE);
			}
		}
		break;
	case GIRDER_SWING_FORWARD_DONE_UNDO:
		{
			DeltaTime(ANIMTAP_SWING_FORWARD_DONE_UPPER, -FLoop_fPreviousLoopSecs, TRUE);
			f32 fUnitTime = GetUnitTime( ANIMTAP_SWING_FORWARD_DONE_UPPER );
			fUnitTime = 1.01f - fUnitTime;
			fUnitTime = fmath_UnitLinearToSCurve( fUnitTime );
			SetControlValue( ANIMCONTROL_SWING_FORWARD_DONE_UPPER, fUnitTime);
			SetControlValue( ANIMCONTROL_SWING_FORWARD_DONE_LOWER, fUnitTime);

			if (GetUnitTime(ANIMTAP_SWING_FORWARD_DONE_UPPER)==0.0f)
			{
				SetControlValue( ANIMCONTROL_SWING_FORWARD_DONE_UPPER, 0.0f);
				SetControlValue( ANIMCONTROL_SWING_FORWARD_DONE_LOWER, 0.0f);
				SetControlValue( ANIMCONTROL_SWING_BACK_UPPER, 1.0f);
				SetControlValue( ANIMCONTROL_SWING_BACK_LOWER, 1.0f);
				ZeroTime(ANIMTAP_SWING_BACK_UPPER);
				m_eGirderSwingState = GIRDER_SWING_BACK;
				_PlaySnd(MOZERSND_SWIPEBACK, TRUE);
			}
		}
		break;


	default:
		FASSERT(!"Invalid state reached");
	}// end switch(m_eGirderAnimCurrentState)
}

void CBotMozer::_AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx )
{
	if (!_pThisBot) // entity is updating the mtx palette;
	{
		rNewMtx.Mul( rParentMtx, rBoneMtx );
		return;
	}

	FASSERT( _pThisBot );
	FASSERT( _pThisBot->TypeBits() & ENTITY_BIT_BOTMOZER );

	CBotMozer* pBotMozer = ((CBotMozer*)_pThisBot);

	if( pBotMozer->m_pPartMgr->AnimBoneCallbackFunctionHandler( nBoneIndex, rNewMtx, rParentMtx, rBoneMtx ) ) {
		return;
	}

	if ( nBoneIndex == CBotMozer::m_nBoneIndexHead ) 
	{
		// Head
		CFQuatA HeadQuat;
		HeadQuat.BuildQuat( rBoneMtx.m_vRight, FMATH_FPOT( pBotMozer->m_fUnitAimPitch, FMATH_DEG2RAD(-25), FMATH_DEG2RAD(40)) );
		
		CFMtx43A NewBoneMtx;
		NewBoneMtx.m_vPos = rBoneMtx.m_vPos;
		HeadQuat.MulPoint( NewBoneMtx.m_vRight, rBoneMtx.m_vRight );
		HeadQuat.MulPoint( NewBoneMtx.m_vUp, rBoneMtx.m_vUp );
		HeadQuat.MulPoint( NewBoneMtx.m_vFront, rBoneMtx.m_vFront );
		
		rNewMtx.Mul( rParentMtx, NewBoneMtx );
	}
	else if ( nBoneIndex == CBotMozer::m_nBoneIndexTorso )
	{
		// Torso
		CFQuatA TorsoQuat;
		f32 fLegsYaw = pBotMozer->m_fLegsYaw_MS;
		TorsoQuat.BuildQuat(rBoneMtx.m_vUp,-fLegsYaw);

		if( !pBotMozer->IsReverseFacingForward() ) 
		{
			// Rotate torso when in reverse facing mode...
			CFQuatA RevQuat;
			RevQuat.BuildQuat( rBoneMtx.m_vUp, fmath_UnitLinearToSCurve( pBotMozer->m_fReverseFacingYaw_MS * ( 1.0f / FMATH_DEG2RAD( 180.0f ) ) ) * FMATH_DEG2RAD( 180.0f ) );
			TorsoQuat.Mul( RevQuat );
		}


		CFQuatA PitchQuat;
		PitchQuat.BuildQuat( rBoneMtx.m_vRight, FMATH_FPOT( pBotMozer->m_fUnitAimPitch, FMATH_DEG2RAD(-5.0f), FMATH_DEG2RAD(15.0f)) );
		TorsoQuat.Mul( PitchQuat );

		CFMtx43A NewBoneMtx;
		NewBoneMtx.m_vPos = rBoneMtx.m_vPos;
		TorsoQuat.MulPoint( NewBoneMtx.m_vRight, rBoneMtx.m_vRight );
		TorsoQuat.MulPoint( NewBoneMtx.m_vUp, rBoneMtx.m_vUp );
		TorsoQuat.MulPoint( NewBoneMtx.m_vFront, rBoneMtx.m_vFront );
		
		rNewMtx.Mul( rParentMtx, NewBoneMtx );
	}
	else if ( nBoneIndex == CBotMozer::m_nBoneIndexGroin ) 
	{
        // Groin
		//CFMtx43A GroinMtx;
		//GroinMtx.m_vRight = rBoneMtx.m_vRight;
		//GroinMtx.m_vUp = rBoneMtx.m_vUp;
		//GroinMtx.m_vFront = rBoneMtx.m_vFront;
		//		
		//GroinMtx.m_vPos.ReceiveNegative(CBotMozer::m_vGroinToGroinPivot);

		//GroinMtx.RotateX(FMATH_FPOT( pBotMozer->m_fUnitAimPitch, FMATH_DEG2RAD(-25), FMATH_DEG2RAD(20)));
		//
		//GroinMtx.m_vPos.Add(CBotMozer::m_vGroinToGroinPivot);
		//GroinMtx.m_vPos.Add(rBoneMtx.m_vPos);
		//rNewMtx.Mul( rParentMtx, GroinMtx );
		rNewMtx.Mul( rParentMtx, rBoneMtx );
	}
	else if ( ( nBoneIndex == CBotMozer::m_nBoneIndexL_Leg_Upper ) || ( nBoneIndex == CBotMozer::m_nBoneIndexR_Leg_Upper ) )
	{
		//// Groin counter to hips
		//CFMtx43A HipsMtx;
		//HipsMtx.m_vRight = rBoneMtx.m_vRight;
		//HipsMtx.m_vUp = rBoneMtx.m_vUp;
		//HipsMtx.m_vFront = rBoneMtx.m_vFront;

		//HipsMtx.RotateX(FMATH_FPOT( pBotMozer->m_fUnitAimPitch, FMATH_DEG2RAD(25), FMATH_DEG2RAD(-20)));
		//
		//HipsMtx.m_vPos = rBoneMtx.m_vPos; 
		//rNewMtx.Mul( rParentMtx, HipsMtx );
		rNewMtx.Mul( rParentMtx, rBoneMtx );
	}
	else if ( nBoneIndex == CBotMozer::m_nBoneIndexArmElbow )
	{
		((CBotMozer*)_pThisBot)->_AimElbow( rNewMtx, rParentMtx, rBoneMtx );
	}
	else if ( nBoneIndex == CBotMozer::m_nBoneIndexArmLower )
	{
		((CBotMozer*)_pThisBot)->_AimLowerArm( rNewMtx, rParentMtx, rBoneMtx );
	}
}

BOOL CBotMozer::_InventoryCallback( CInventoryCallbackReason_e eReason, CInventory *pInventory, u32 nHandIndex, u32 nWeaponIndex )
{	CBotMozer *_pThisBot = (CBotMozer *)pInventory->m_pUser;

	switch( eReason )
	{
	case IREASON_WEAPONCHANGE:
		FASSERT_NOW;

		break;

	case IREASON_RELOAD:
		if( nHandIndex == 0 )
		{
			_pThisBot->m_abReloadRequest[0] = TRUE;
		}

		break;

	case IREASON_WEAPONUPGRADED:
		FASSERT_NOW;

		break;

	}

	return TRUE;
}



void CBotMozer::CheckpointSaveSelect( s32 nCheckpoint )
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

BOOL CBotMozer::CheckpointSave( void )
{
	// save parent class data
	CBot::CheckpointSave();

	return TRUE;
}

void CBotMozer::CheckpointRestore( void )
{
	// load parent class data
	FASSERT( m_hSaveData[CFCheckPoint::GetCheckPoint()] != CFCheckPoint::NULL_HANDLE );
	CBot::CheckpointRestore();

	m_Anim.Reset();
	SetControlValue( ANIMCONTROL_STAND, 1.0f );

	AttachAnim( ANIMTAP_RECOIL_SUMMER, &m_AnimManFrameRecoil );
	m_AnimManFrameRecoil.Reset();
	UpdateBoneMask( ANIMTAP_RECOIL_SUMMER, m_aBoneEnableIndices_RecoilSummer, TRUE );
	_ResetRecoilSummer();
	SetControlValue( ANIMCONTROL_RECOIL_SUMMER, 0.0f );
}


void CBotMozer::MeleeCollision( CEntity *pEntity, FCollImpact_t *pImpact, BOOL bFirstHit ) {
	CBot::MeleeCollision( pEntity, pImpact, bFirstHit );

	_SpawnSlashImpactEffects( pImpact );

	if( bFirstHit ) {
		_PlaySnd(MOZERSND_SWIPEIMPACT,TRUE);
	}

	if( pEntity && pEntity->TypeBits() & ENTITY_BIT_BOT )
	{
		CBot *pHitBot = (CBot *)pEntity;

		// Bump the bot a bit backwards
		CFVec3A vImpulse;
		f32 fMag2;
		vImpulse.Sub( pHitBot->MtxToWorld()->m_vPos, m_MtxToWorld.m_vPos );
		fMag2 = vImpulse.MagSq();
		if( fMag2 > 0.01f ) {
			fMag2 = fmath_InvSqrt( fMag2 ) * m_BotInfo_Girder.fSwingPush;
			if( pHitBot->m_pBotInfo_Gen ) {
				fMag2 = fmath_Div( fMag2, pHitBot->m_pBotInfo_Gen->fBotCollisionMass );
				vImpulse.Mul( fMag2 );
				pHitBot->ApplyVelocityImpulse_WS( vImpulse );
			}
		}
	}
}


//#if 0
//void 	CBotMozer::_HandleGirderCollision(void)
//{
//
//	// ME - this stuff seems to want to be set every frame...
//	CFVec3A vGirderMultDir;
//	const CFMtx43A* pSecondaryFireMtx = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexSecondaryFire];
//	vGirderMultDir.Mul(pSecondaryFireMtx->m_vUp,m_BotInfo_Girder.fStreamerLength);
//	m_mtxStreamerWS.Set(*pSecondaryFireMtx);
//	m_mtxStreamerWS.m_vPos.Sub(vGirderMultDir);
//
//	CFVec3A vSecondaryFirePos;
//	vSecondaryFirePos.Set( pSecondaryFireMtx->m_vPos);
//
//	CFVec3A vGirderCollSphereWS;
//	vGirderCollSphereWS.Set(m_CollSphere.m_Pos);
//	vGirderCollSphereWS.Sub(vSecondaryFirePos).Unitize();
//	vGirderCollSphereWS.Mul(m_CollSphere.m_fRadius);
//	vGirderCollSphereWS.Add(vSecondaryFirePos);
//
//	m_GirderCollSphereWS.m_Pos.x = vGirderCollSphereWS.x;
//	m_GirderCollSphereWS.m_Pos.y = vGirderCollSphereWS.y;
//	m_GirderCollSphereWS.m_Pos.z = vGirderCollSphereWS.z;
//	m_GirderCollSphereWS.m_fRadius = m_pBotInfo_Gen->fCollSphere1Radius_MS;
//	
////	CFVec3A collision;collision.Set(m_GirderCollSphereWS.m_Pos);
////	fdraw_DevSphere(&collision,m_GirderCollSphereWS.m_fRadius);
//
//	m_CollInfo.nCollTestType = FMESH_COLLTESTTYPE_SPHERE;
//	m_CollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_NONE;
//	m_CollInfo.bFindClosestImpactOnly = TRUE;
//	m_CollInfo.nCollMask = (m_nPossessionPlayerIndex < 0) ? FCOLL_MASK_COLLIDE_WITH_NPCS : FCOLL_MASK_COLLIDE_WITH_PLAYER;
//	m_CollInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
//	m_CollInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
//	m_CollInfo.pSphere_WS = &m_GirderCollSphereWS;
//	m_CollInfo.pTag = NULL;
//
//	// ME - seems like we don't have to do this collision if we're not swinging...
//	if( !IsAttackingWithMelee() ) {
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
///* pgm took this out since it causes bot to magically slide back and quite often pops him righ out of the world
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
//}
//
//BOOL CBotMozer::_SwingStrikeCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume )
//{
//	for ( u32 uIndex = 0; uIndex < FWorld_nTrackerSkipListCount; uIndex++ )
//	{
//		if ( pTracker == FWorld_apTrackerSkipList[uIndex] )
//		{
//			// Tracker is in the skip list, can't collide with this guy
//			return TRUE;
//		}
//	}
//
//	FASSERT (_pThisBot->m_pWorldMesh != pTracker); // don't bother with self
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
//	return TRUE;
//}
//
//BOOL CBotMozer::_SwingStruckBot( CFWorldTracker *pTracker )
//{
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
//	CFVec3A* pGirderBone   = &_pThisBot->m_pWorldMesh->GetBoneMtxPalette()[CBotMozer::m_nBoneIndexGirder]->m_vPos;
//	CFVec3A* pSecndFireBone = &_pThisBot->m_pWorldMesh->GetBoneMtxPalette()[CBotMozer::m_nBoneIndexSecondaryFire]->m_vPos;
//
//	m_CollInfo.ProjSphere.Init(pGirderBone->x,pGirderBone->y,pGirderBone->z,pSecndFireBone->x,pSecndFireBone->y,pSecndFireBone->z,1.0f);
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
//		//_pThisBot->_InflictSlashDamage( pBot, pWorldMesh, &FColl_aImpactBuf[0] );
//		_pThisBot->m_pMeleeBotToDamageThisFrame = pBot;
//
//		// Now, add this entity to our hit-entity buffer to remember that we've already damaged it...
//		_pThisBot->m_apHitEntity[ _pThisBot->m_nHitEntityCount++ ] = pBot;
//
//		// ok, we hit one.  Good enough
//		return FALSE;
//	}
//	return TRUE;
//}
//
//BOOL CBotMozer::_SwingStruckMeshEntity( CFWorldTracker *pTracker )
//{
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
//	CFVec3A* pGirderBone   = &_pThisBot->m_pWorldMesh->GetBoneMtxPalette()[CBotMozer::m_nBoneIndexGirder]->m_vPos;
//	CFVec3A* pSecndFireBone = &_pThisBot->m_pWorldMesh->GetBoneMtxPalette()[CBotMozer::m_nBoneIndexSecondaryFire]->m_vPos;
//
//	m_CollInfo.ProjSphere.Init(pGirderBone->x,pGirderBone->y,pGirderBone->z,pSecndFireBone->x,pSecndFireBone->y,pSecndFireBone->z,1.0f);
//	m_CollInfo.pTag = pTracker;
//
//	fcoll_Clear();
//	CFWorldMesh *pWorldMesh = (CFWorldMesh *)pTracker;
//	if( pWorldMesh->CollideWithMeshTris( &m_CollInfo ) )
//	{
//		_pThisBot->_InflictSlashDamage( (CEntity *) pTracker->m_pUser, pWorldMesh, &FColl_aImpactBuf[0] );
//		_pThisBot->_SpawnSlashImpactEffects( &FColl_aImpactBuf[0] );
//	}
//	return TRUE;
//}
//#endif

void CBotMozer::_StreamerOn( f32 fFadeOut)
{
	if (fFadeOut==0.0f)
	{
		if( CFXStreamerEmitter::IsValidHandle( m_hStreamerEmitter ) ) // it's on
		{
		}
		else
		{
			// grab a streamer emitter
			CFVec3 vSpawnPt(0.f,m_BotInfo_Girder.fStreamerLength*0.5f,0.0f);
			m_hStreamerEmitter = CFXStreamerEmitter::SpawnFromMtx43A( 50, &m_mtxStreamerWS,
			m_BotInfo_Girder.fStreamerAlpha,
			&m_StreamerTexInst,
			&vSpawnPt,
			m_BotInfo_Girder.fStreamerLength,
			60.0f,
			CFXStreamerEmitter::USE_UP_AXIS);
			if( m_hStreamerEmitter == FXSTREAMER_INVALID_HANDLE )
			{
				DEVPRINTF( "CBotMozer::_StreamerOn(): trouble starting streamer emitter.\n" );
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

void CBotMozer::ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS )
{
	// sighting along an animated point causes model flicker,
	//pApproxMuzzlePoint_WS->Set(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexSecondaryFire]->m_vPos);

	pApproxMuzzlePoint_WS->Set(m_MtxToWorld.m_vPos);
}

void CBotMozer::_SpawnSlashImpactEffects( const FCollImpact_t *pCollImpact )
{
	if (IsPlayerBot())
	{
		// Generate camera shake...
		CFCamera *pCamera = fcamera_GetCameraByIndex( m_nPossessionPlayerIndex );
		pCamera->ShakeCamera( 0.05f, 0.15f );

		// Feel the shwing
		fforce_Kill( &m_hForce );
		fforce_Play( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ARROW_THUNK_MED, &m_hForce );
	}

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

void CBotMozer::_HandleWeaponWork( void )
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

void CBotMozer::_InflictSlashDamage( CEntity *pHitEntity, CFWorldMesh *pHitWorldMesh, const FCollImpact_t *pCollImpact )
{
	FASSERT(pHitEntity); // don't call me with nulls;

	DEVPRINTF( "CBotMozer::_InflictSlashDamage on %x\n", pHitEntity );

	// Inflict damage to the entity...

	// Get an empty damage form...
	CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

	if( pDamageForm )
	{
		// Fill out the form...
		pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
		pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
		pDamageForm->m_pDamageProfile = m_pDamageProfileSlash;
		pDamageForm->m_Damager.pWeapon = NULL;
		pDamageForm->m_Damager.pBot = this;
		pDamageForm->m_Damager.nDamagerPlayerIndex = m_nPossessionPlayerIndex;
		pDamageForm->m_pDamageeEntity = pHitEntity;
		pDamageForm->InitTriDataFromCollImpact( pHitWorldMesh, pCollImpact, &_pThisBot->m_vFireUnitDir );

		CDamage::SubmitDamageForm( pDamageForm );
	}

	if( pHitEntity->TypeBits() & ENTITY_BIT_BOT )
	{
		CBot *pHitBot = (CBot *)pHitEntity;

		// Bump the bot a bit backwards
		CFVec3A vImpulse;
		f32 fMag2;
		vImpulse.Sub( pHitBot->MtxToWorld()->m_vPos, _pThisBot->m_MtxToWorld.m_vPos );
		fMag2 = vImpulse.MagSq();
		if( fMag2 > 0.01f ) {
			fMag2 = fmath_InvSqrt( fMag2 ) * m_BotInfo_Girder.fSwingPush;
			if( pHitBot->m_pBotInfo_Gen ) {
				fMag2 = fmath_Div( fMag2, pHitBot->m_pBotInfo_Gen->fBotCollisionMass );
			}

			vImpulse.Mul( fMag2 );
			pHitBot->ApplyVelocityImpulse_WS( vImpulse );
		}

		//CFVec3A Impulse = _pThisBot->m_vFireUnitDir;
		//Impulse.Mul( m_BotInfo_Girder.fSwingSwingBackwardStreamerOff );
		//pHitBot->ApplyVelocityImpulse_WS( Impulse );
	}
}

void CBotMozer::_PlaySnd(MozerSound_e eSnd, BOOL bPlay)
{
	if (eSnd >= MOZERSND_COUNT)
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


void CBotMozer::Die( BOOL bSpawnDeathEffects/*=TRUE*/, BOOL bSpawnGoodies )
{
	CBot::Die( FALSE );
}


void CBotMozer::_AimLowerArm( CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	f32 fSlerp;
	f32 fMag2;

	rNewMtx.Mul( rParentMtx, rBoneMtx );
	// return;

	if( (GetControlValue( ANIMCONTROL_FIRE_1_UPPER ) == 0.0f) ) {
		return;
	}

	if( m_fUnitFireCtlUpper >= _FIRE_READY_BLEND ) {
		CFVec3A vAim;
		fSlerp = _ARM_SLERP_RATE * FLoop_fPreviousLoopSecs;

		vAim.Sub( m_TargetedPoint_WS, rNewMtx.m_vPos );
		if( vAim.MagSq() > 0.001f ) {
			rNewMtx.m_vFront = vAim.PlanarProjection( rNewMtx.m_vRight );

			fMag2 = rNewMtx.m_vFront.MagSq();
			if( fMag2 > 0.01f ) {
				rNewMtx.m_vFront.Mul( fmath_InvSqrt( fMag2 ) );
				rNewMtx.m_vUp.Cross( rNewMtx.m_vFront, rNewMtx.m_vRight );
			} else {
                rNewMtx.m_vFront.Cross( rNewMtx.m_vRight, rNewMtx.m_vUp );
			}
		}
	} else {
		fSlerp = 1.0f - fmath_Div( GetControlValue( ANIMCONTROL_FIRE_1_UPPER ), _FIRE_READY_BLEND );
	}

	// if fired very recently, go directly there, otherwise move toward correct position
	if( m_nBotFlags & BOTFLAG_PLAY_FIRE1_ANIM ) {
		m_qRightLowerArm.BuildQuat( rNewMtx );
	} else {
		CFQuatA qTo;
		FMATH_CLAMP_UNIT_FLOAT( fSlerp );

		qTo.BuildQuat( rNewMtx );
		m_qRightLowerArm.ReceiveSlerpOf( fSlerp, m_qRightLowerArm, qTo );
		m_qRightLowerArm.BuildMtx33( rNewMtx );
	}
}


void CBotMozer::_AimElbow( CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	f32 fSlerp;
	f32 fMag2;

	rNewMtx.Mul( rParentMtx, rBoneMtx );
	//return;

	if( (GetControlValue( ANIMCONTROL_FIRE_1_UPPER ) == 0.0f) ) {
		return;
	}

	if( m_fUnitFireCtlUpper >= _FIRE_READY_BLEND ) {
		CFVec3A vAim;
		fSlerp = _ARM_SLERP_RATE * FLoop_fPreviousLoopSecs;

		vAim.Sub( m_TargetedPoint_WS, rNewMtx.m_vPos );
		if( vAim.MagSq() > 0.001f ) {
			rNewMtx.m_vFront = vAim.PlanarProjection( rNewMtx.m_vUp );

			fMag2 = rNewMtx.m_vFront.MagSq();
			if( fMag2 > 0.01f ) {
				rNewMtx.m_vFront.Mul( fmath_InvSqrt( fMag2 ) );
				rNewMtx.m_vRight.Cross( rNewMtx.m_vUp, rNewMtx.m_vFront );
			} else {
				rNewMtx.m_vFront.Cross( rNewMtx.m_vRight, rNewMtx.m_vUp );
			}
		}
	} else {
		fSlerp = 1.0f - fmath_Div( GetControlValue( ANIMCONTROL_FIRE_1_UPPER ), _FIRE_READY_BLEND );
	}

	if( m_nBotFlags & BOTFLAG_PLAY_FIRE1_ANIM ) {
		m_qRightElbow.BuildQuat( rNewMtx );
	} else {
		CFQuatA qTo;
		FMATH_CLAMP_UNIT_FLOAT( fSlerp );

		qTo.BuildQuat( rNewMtx );
		m_qRightElbow.ReceiveSlerpOf( fSlerp, m_qRightElbow, qTo );
		m_qRightElbow.BuildMtx33( rNewMtx );
	}
}


void CBotMozer::_ResetRecoilSummer( void ) 
{
	CFAnimFrame qFrame;

	m_AnimManFrameRecoil.Reset();

	FASSERT( m_apWeapon[0] && m_apWeapon[0]->m_pInfo->nStanceType == CWeapon::STANCE_TYPE_STANDARD );

	qFrame.BuildQuat( CFVec3A::m_UnitAxisY, _RECOIL_ROT_RIGHT_SHOULDER, CFVec3A::m_Null );
	m_AnimManFrameRecoil.UpdateFrame( ANIMBONE_RECOIL_R_ARM_UPPER, qFrame );

	CFVec3A vTorsoAxis;
	vTorsoAxis.Set( -1.0f, 2.0f, 0.0f );
	vTorsoAxis.Unitize();

	qFrame.BuildQuat( vTorsoAxis, _RECOIL_ROT_TORSO, CFVec3A::m_Null );
	m_AnimManFrameRecoil.UpdateFrame( ANIMBONE_RECOIL_TORSO, qFrame );

}


void CBotMozer::_Recoil( f32 fRecoilAmount ) {
	FASSERT_UNIT_FLOAT( fRecoilAmount );

	if( m_fUnitRecoil + fRecoilAmount > m_fUnitRecoilTarget ) {
		m_fUnitRecoilTarget = m_fUnitRecoil + fRecoilAmount;
		FMATH_CLAMPMAX( m_fUnitRecoilTarget, FMATH_MIN( 1.0f, 1.5f * fRecoilAmount ) );
	}
}


void CBotMozer::_HandleRecoil( void ) {
	// calculate our dummy rotation
	f32 fSin, fCos;
	CFVec3A vAxis;
	CFAnimFrame qFrame;

	fmath_SinCos( m_fAimDeltaYaw_MS - m_fLegsYaw_MS, &fSin, &fCos );
	vAxis.Set( fCos, 0.0f, -fSin );

	qFrame.BuildQuat( vAxis, _RECOIL_ROT_DUMMY, CFVec3A::m_Null );
	m_AnimManFrameRecoil.UpdateFrame( ANIMBONE_RECOIL_DUMMY, qFrame );

	if( m_fUnitRecoil <= m_fUnitRecoilTarget ) {

		// don't recoil till fire animation is at 100%
		if( GetControlValue( ANIMCONTROL_FIRE_1_UPPER ) < 1.0f ) {
			return;
		}

        // we're recoiling
		m_fUnitRecoil += FLoop_fPreviousLoopSecs * _RECOIL_ON_SPEED;
		
		if( m_fUnitRecoil >= m_fUnitRecoilTarget ) {		// got there, now let's head back
			m_fUnitRecoil = m_fUnitRecoilTarget;
			m_fUnitRecoilTarget = 0.0f;
		}

		SetControlValue( ANIMCONTROL_RECOIL_SUMMER, m_fUnitRecoil );

	} else if( m_fUnitRecoil > m_fUnitRecoilTarget ) {
		// recovering
		m_fUnitRecoil -= FLoop_fPreviousLoopSecs * _RECOIL_OFF_SPEED;
		FMATH_CLAMP_MIN0( m_fUnitRecoil );
		SetControlValue( ANIMCONTROL_RECOIL_SUMMER, m_fUnitRecoil );
	}
}


void CBotMozer::_CalcGirderMeleePos( CFVec3A *pvTmp ) {
	pvTmp->Mul( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexSecondaryFire]->m_vUp, -2.5f );
	pvTmp->Add( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexSecondaryFire]->m_vPos );
}
