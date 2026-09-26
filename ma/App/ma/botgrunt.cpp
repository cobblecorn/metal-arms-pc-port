//////////////////////////////////////////////////////////////////////////////////////
// botgrunt.cpp - 
//
// Author: Steve Ranck     
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
// 02/18/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
 
#include "fang.h"
#include "botgrunt.h"
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
#include "collectable.h"
#include "item.h"
#include "botglitch.h"
#include "vehicle.h"
#include "site_botWeapon.h"
#include "game.h"
#include "letterbox.h"

#define _BOTINFO_FILENAME		"b_grunt"
#define _BOTPART_FILENAME		"bp_grunt"
//#define _SOUND_BANK_NAME		"grunt"

#define _HEADLIGHT_TEXTURE_ID	( 1 )

static cchar *_pszGruntMeshFilenameRoot = "GRMGgrunt";
static cchar *_apszGruntMeshFilenames[] = { "GRMGgrunt00", "GRMGgrunt01" };  

#define _ROCKET1TORSOTWIST_ANGLE		( FMATH_DEG2RAD( 36.5f ) )


#define _TINT_R_LASER		( 118	/ 255.0f )
#define _TINT_G_LASER		( 124	/ 255.0f )
#define _TINT_B_LASER		( 255	/ 255.0f )

#define _TINT_R_SPEW		( 185	/ 255.0f )
#define _TINT_G_SPEW		( 59	/ 255.0f )
#define _TINT_B_SPEW		( 38	/ 255.0f )

#define _TINT_R_FLAMER		( 255 	/ 255.0f )
#define _TINT_G_FLAMER		( 255 	/ 255.0f )
#define _TINT_B_FLAMER		( 255 	/ 255.0f )

#define _TINT_R_ROCKET		( 85 	/ 255.0f )
#define _TINT_G_ROCKET		( 157 	/ 255.0f )
#define _TINT_B_ROCKET		( 63 	/ 255.0f )

#define _TINT_R_RIVET		( 64 	/ 255.0f )
#define _TINT_G_RIVET		( 54 	/ 255.0f )
#define _TINT_B_RIVET		( 42 	/ 255.0f )

#define _TINT_R_HAND		( 161 	/ 255.0f )
#define _TINT_G_HAND		( 52 	/ 255.0f )
#define _TINT_B_HAND		( 104 	/ 255.0f )


static const CFColorRGB _TintColorLaser		= CFColorRGB( _TINT_R_LASER,	_TINT_G_LASER,	_TINT_B_LASER	);
static const CFColorRGB _TintColorSPEW		= CFColorRGB( _TINT_R_SPEW,		_TINT_G_SPEW,	_TINT_B_SPEW	);
static const CFColorRGB _TintColorFlamer	= CFColorRGB( _TINT_R_FLAMER,	_TINT_G_FLAMER,	_TINT_B_FLAMER	);
static const CFColorRGB _TintColorRocket	= CFColorRGB( _TINT_R_ROCKET,	_TINT_G_ROCKET,	_TINT_B_ROCKET	);
static const CFColorRGB _TintColorRivet		= CFColorRGB( _TINT_R_RIVET,	_TINT_G_RIVET,	_TINT_B_RIVET	);
static const CFColorRGB _TintColorHand		= CFColorRGB( _TINT_R_HAND,		_TINT_G_HAND,	_TINT_B_HAND	);




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotGruntBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CBotGruntBuilder _BotGruntBuilder;


void CBotGruntBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_BOTGRUNT, pszEntityType );

	// Override defaults set by our parent classes...
	m_nEC_HealthContainerCount = CBotGrunt::m_BotInfo_Gen.nBatteryCount;
	m_pszEC_ArmorProfile = CBotGrunt::m_BotInfo_Gen.pszArmorProfile;

	m_anNPCWeaponType[0] = _NPC_WEAPON_LASER;		// NPC grunts default to a laser
	m_anNPCWeaponType[1] = _NPC_WEAPON_GRENADE;		// NPC grunts default to a grenade

	u32 i;
	for( i = 0; i < CBotGrunt::ANIM_BASE_COUNT; i++ ) {
		m_apszAnimFileOverrides[i] = NULL;
	}

	// This class of bots can be recruited...
	FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_CLASS_CAN_BE_RECRUITED );

	// This class of bots shows up on Glitch's radar...
	FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_SHOWS_UP_ON_RADAR );

	m_bAllowWeaponDrop = TRUE;
}


const CFColorRGB *CBotGruntBuilder::_GetGruntColor( NPCWeapon_e nNPCWeaponType ) {
	switch( nNPCWeaponType ) {
	case _NPC_WEAPON_HAND:
		return &_TintColorHand;

	case _NPC_WEAPON_SPEW:
		return &_TintColorSPEW;

	case _NPC_WEAPON_FLAMER:
		return &_TintColorFlamer;

	case _NPC_WEAPON_RIVET:
		return &_TintColorRivet;

	case _NPC_WEAPON_ROCKET:
		return &_TintColorRocket;
	};

	return &_TintColorLaser;
}


BOOL CBotGruntBuilder::PostInterpretFixup( void ) {
	if( !CBotBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	u32 i;
	for (i = 0; i < CBotGrunt::ANIM_BASE_COUNT; i++)
	{
		if( m_apszAnimFileOverrides[i] && !(m_apszAnimFileOverrides[i] = gstring_Main.AddString( m_apszAnimFileOverrides[i] )) ) {
			goto _ExitWithError;
		}
	}

	// set mesh tint based on weapon selected...
	m_MeshTintColor = *_GetGruntColor( m_anNPCWeaponType[0] );

	return TRUE;

	// Error:
_ExitWithError:
	return FALSE;
}


BOOL CBotGruntBuilder::InterpretTable( void ) {
	cchar *pszString;

	//this is legacy because grunt used to have something called gruntweapon, but
	//now any bot can get it's weapons configured via the NPCWeapon0 and NPCWeapon1 tags. 
	//see bot.cpp
	if( !fclib_stricmp( CEntityParser::m_pszTableName, "GruntWeapon" ) ) {

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
				for (i = 0; i < CBotGrunt::ANIM_BASE_COUNT; i++)
				{
					if (!m_apszAnimFileOverrides[i])
					{	//override a grunt anim
						m_apszAnimFileOverrides[i] = pszAnimRepName;
					}
				}
			}
			else
			{
				for (i = 0; i < CBotGrunt::ANIM_BASE_COUNT; i++)
				{
					if (!fclib_stricmp( pszTapName, CBotGrunt::m_apszBaseTapNameTable[i] ) )
					{	//override a grunt anim
						m_apszAnimFileOverrides[i] = pszAnimRepName;
					}
				}
			}
		}
		return TRUE;
	}

	if( !fclib_stricmp( CEntityParser::m_pszTableName, "dropweapon" ) ) {
		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			cchar *pszValue;

			if( CEntityParser::Interpret_String( &pszValue ) ) {
				CEntityParser::CompareBoolString( pszValue, &m_bAllowWeaponDrop );
			}
		}
	}

	return CBotBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotGrunt
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CBotGrunt::m_bSystemInitialized = FALSE;
u32 CBotGrunt::m_nBotClassClientCount = 0;

CBotGrunt::BotInfo_Gen_t CBotGrunt::m_BotInfo_Gen;
CBotGrunt::BotInfo_MountAim_t CBotGrunt::m_BotInfo_MountAim;
CBotGrunt::BotInfo_Walk_t CBotGrunt::m_BotInfo_Walk;
CBotGrunt::BotInfo_Jump_t CBotGrunt::m_BotInfo_Jump;
CBotGrunt::BotInfo_Weapon_t CBotGrunt::m_BotInfo_Weapon;
CBotGrunt::BotInfo_Grunt_t CBotGrunt::m_BotInfo_Grunt;

CBotAnimStackDef CBotGrunt::m_AnimStackDef;

CFAnimFrame CBotGrunt::m_Rocket1TorsoTwistQuat;
CFAnimFrame CBotGrunt::m_Rocket1TorsoUntwistQuat;

CFVec3A CBotGrunt::m_GroinVecY_WS;


CBotPartPool *CBotGrunt::m_pPartPool;
CBotGrunt*   CBotGrunt::m_pCBGrunt = NULL;







BOOL CBotGrunt::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );

	m_nBotClassClientCount = 0;

	m_bSystemInitialized = TRUE;

	m_Rocket1TorsoTwistQuat.BuildQuat( CFVec3A::m_UnitAxisY, _ROCKET1TORSOTWIST_ANGLE, CFVec3A::m_Null );
	m_Rocket1TorsoUntwistQuat.BuildQuat( CFVec3A::m_UnitAxisY, -_ROCKET1TORSOTWIST_ANGLE, CFVec3A::m_Null );

	return TRUE;
}

void CBotGrunt::UninitSystem( void ) {
	if( m_bSystemInitialized ) {
		m_AnimStackDef.Destroy();
		m_bSystemInitialized = FALSE;
	}
}


BOOL CBotGrunt::ClassHierarchyLoadSharedResources( void ) {
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
	/*if( !fresload_Load( FSNDFX_RESTYPE, _SOUND_BANK_NAME ) ) {
		DEVPRINTF( "CBotGrunt::ClassHierarchyLoadSharedResources(): Could not load sound effect bank '%s'\n", _SOUND_BANK_NAME );
	}*/

	if( !ReadBotInfoFile( m_aGameDataMap, _BOTINFO_FILENAME ) ) {
		goto _ExitWithError;
	}

	if( !_BuildAnimStackDef() ) {
		goto _ExitWithError;
	}

	return TRUE;

	// Error:
_ExitWithError:
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( ResFrame );

	return FALSE;
}


void CBotGrunt::ClassHierarchyUnloadSharedResources( void ) {
	FASSERT( m_nBotClassClientCount > 0 );

	--m_nBotClassClientCount;

	if( m_nBotClassClientCount == 0 ) {
		m_AnimStackDef.Destroy();
	}

	CBot::ClassHierarchyUnloadSharedResources();
}


CBotGrunt::CBotGrunt() : CBot() {
	m_pInventory = NULL;
	m_pWorldMesh = NULL;
}


CBotGrunt::~CBotGrunt() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CBotGrunt::Create( s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName, u32 nMeshVersionOverride, CBotBuilder::NPCWeapon_e nWeapon ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	if( !ClassHierarchyLoadSharedResources() ) {
		// Failure! (resources have already been cleaned up, so we can bail now)
		return FALSE;
	}

	// Get pointer to the leaf class's builder object...
	CBotGruntBuilder *pBuilder = (CBotGruntBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_BotGruntBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOTGRUNT );
	}

	// Set our builder parameters...

	pBuilder->m_uMeshVersionOverride = nMeshVersionOverride;

	switch( nWeapon ) {
	case CBotBuilder::_NPC_WEAPON_HAND:
		pBuilder->m_anNPCWeaponType[0] = CBotBuilder::_NPC_WEAPON_HAND;
		break;

	case CBotBuilder::_NPC_WEAPON_SPEW:
		pBuilder->m_anNPCWeaponType[0] = CBotBuilder::_NPC_WEAPON_SPEW;
		break;

	case CBotBuilder::_NPC_WEAPON_FLAMER:
		pBuilder->m_anNPCWeaponType[0] = CBotBuilder::_NPC_WEAPON_FLAMER;
		break;

	case CBotBuilder::_NPC_WEAPON_RIVET:
		pBuilder->m_anNPCWeaponType[0] = CBotBuilder::_NPC_WEAPON_RIVET;
		break;

	case CBotBuilder::_NPC_WEAPON_ROCKET:
		pBuilder->m_anNPCWeaponType[0] = CBotBuilder::_NPC_WEAPON_ROCKET;
		break;

	default:
		pBuilder->m_anNPCWeaponType[0] = CBotBuilder::_NPC_WEAPON_LASER;
		break;
	};

	pBuilder->m_MeshTintColor = *CBotGruntBuilder::_GetGruntColor( nWeapon );

	// Create an entity...
	return CBot::Create( &m_BotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}


void CBotGrunt::ClassHierarchyDestroy( void ) {
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


BOOL CBotGrunt::ClassHierarchyBuild( void ) {
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
	CBotGruntBuilder *pBuilder = (CBotGruntBuilder *)GetLeafClassBuilder();

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
	pszMeshFilename = _apszGruntMeshFilenames[0];
	if ( pBuilder->m_uMeshVersionOverride >0 && pBuilder->m_uMeshVersionOverride < sizeof(_apszGruntMeshFilenames)/sizeof(char*) ) {
		pszMeshFilename = _apszGruntMeshFilenames[pBuilder->m_uMeshVersionOverride];
	}

	if( pBuilder->m_pszMeshReplacement ) {
	   pszMeshFilename = pBuilder->m_pszMeshReplacement;
	}

	pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pszMeshFilename);
	if( pMesh == NULL ) {
		DEVPRINTF( "CBotGrunt::ClassHierarchyBuild(): Could not find mesh '%s'\n", pszMeshFilename );
		goto _ExitWithError;
	}

	// Create world mesh...
	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL ) {
		DEVPRINTF( "CBotGrunt::ClassHierarchyBuild(): Not enough memory to create CFWorldMesh.\n" );
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
	m_pWorldMesh->SetMeshTint( pBuilder->m_MeshTintColor.fRed, pBuilder->m_MeshTintColor.fGreen, pBuilder->m_MeshTintColor.fBlue );

	m_pWorldMesh->UpdateTracker();
	m_pWorldMesh->RemoveFromWorld();

	for( i = 0; i < ANIM_BASE_COUNT; i++ ) {
		if( !pBuilder->m_apszAnimFileOverrides[i] ) {
			pBuilder->m_apszAnimFileOverrides[i] = m_apszBaseAnimNameTable[i];
		}
	}

	m_AnimStackDef.m_apszBaseAnimNameTable = pBuilder->m_apszAnimFileOverrides;

	// Build our animation stack and load animations...
	if( !m_Anim.Create( &m_AnimStackDef, m_pWorldMesh ) ) {
		DEVPRINTF( "CBotGrunt::ClassHierarchyBuild(): Trouble creating m_Anim.\n" );
		goto _ExitWithError;
	}


	SetControlValue( ANIMCONTROL_STAND, 1.0f );
	UpdateUnitTime( ANIMTAP_STAND, fmath_RandomFloat() );
	EnableControlSmoothing( ANIMCONTROL_AIM_ROCKET1 );
//	EnableControlSmoothing( ANIMCONTROL_AIM_ROCKET23 );
	EnableControlSmoothing( ANIMCONTROL_FIRE_2_UPPER );
	EnableControlSmoothing( ANIMCONTROL_FIRE_2_LOWER );

	m_nBoneIndexGroin = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_GROIN] );
	m_nBoneIndexTorso = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_TORSO] );
	m_nBoneIndexPriFire = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ATTACHPOINT_PRIMARY] );

	if( (m_nBoneIndexGroin < 0) || (m_nBoneIndexTorso < 0) || (m_nBoneIndexPriFire < 0) ) {
		DEVPRINTF( "CBotGrunt::ClassHierarchyBuild(): Trouble retrieving bone indices.\n" );
	//	goto _ExitWithError;
	}

	m_Anim.m_pAnimCombiner->SetBoneCallback( &_AnimBoneCallback );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_GROIN] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_TORSO] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_HEAD] );

	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_LEFT_FOOT] );
	if( nBoneIndex >= 0 ) {
		m_pLeftFootDustPos_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;

		nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_RIGHT_FOOT] );
		if( nBoneIndex >= 0 ) {
			m_pRightFootDustPos_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
		} else {
			m_pLeftFootDustPos_WS = NULL;
		}
	}

	if( !m_AnimManFrameAim.Create( m_AnimStackDef.m_nBoneCount, m_AnimStackDef.m_apszBoneNameTable ) ) {
		DEVPRINTF( "CBotGrunt::ClassHierarchyCreate(): Could not create aim animation summer.\n" );
		goto _ExitWithError;
	}
	AttachAnim( ANIMTAP_AIM_SUMMER, &m_AnimManFrameAim );
	m_AnimManFrameAim.Reset();
	UpdateBoneMask( ANIMTAP_AIM_SUMMER, m_aBoneEnableIndices_AimSummer, TRUE );

	// Set primary and secondary weapon.........
	if (m_nBoneIndexPriFire > -1)
	{
		for (i = 0; i < 2; i++) {
			FASSERT( pBuilder->m_anNPCWeaponType[i] < CBotBuilder::_NPC_WEAPON_COUNT );

			if( m_anWeaponUpgradeLevel[pBuilder->m_anNPCWeaponType[i]] == -1 ) {
				m_apWeapon[i] = NULL;
				continue;
			}

			m_apWeapon[i] = CBotBuilder::AllocNPCWeaponOfType(pBuilder->m_anNPCWeaponType[i]);
			if( m_apWeapon[i] == NULL ) {
				DEVPRINTF( "CBotGrunt::ClassHierarchyBuild() : Error instantiating CWeapon %d.\n", i );
				goto _ExitWithError;
			}
			
			m_apWeapon[i]->SetSingleMeshForEUKs( m_anWeaponUpgradeLevel[pBuilder->m_anNPCWeaponType[i]] );
			
			if (!CBotBuilder::CreateNPCWeaponOfType(m_apWeapon[i], pBuilder->m_anNPCWeaponType[i]))
			{
				DEVPRINTF( "CBotGrunt::ClassHierarchyBuild() : Error Creating CWeapon %d.\n", i );
				goto _ExitWithError;
			}

			m_apWeapon[i]->SetUpgradeLevel( (u32)m_anWeaponUpgradeLevel[pBuilder->m_anNPCWeaponType[i]] );

//			m_apWeapon[i]->OverrideMaxReserveAmmo( CWeapon::INFINITE_AMMO );
			m_apWeapon[i]->SetReserveAmmo( CWeapon::INFINITE_AMMO );
			m_apWeapon[i]->SetClipAmmo( CWeapon::INFINITE_AMMO );

			m_apWeapon[i]->EnableAutoWork( FALSE );
			m_apWeapon[i]->RemoveFromWorld();
			m_apWeapon[i]->SetOwner(this);
			m_apWeapon[i]->SetDesiredState( CWeapon::STATE_DEPLOYED );
		}
	}

		// Primary must be attached
	if (m_apWeapon[0])
	{
		m_apWeapon[0]->Attach_UnitMtxToParent_PS( this, m_apszBoneNameTable[BONE_ATTACHPOINT_PRIMARY] );
		// Make primary weapon pass along damage to upper-right arm...
		m_apWeapon[0]->SetBotDamageBoneIndex( m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_RIGHT_ARM_UPPER] ) );

		// Don't give the EUK level!
		CCollectable::NotifyCollectableUsedInWorld( CCollectable::ClassifyBotWeapon( m_apWeapon[0] ), TRUE );

		if( m_apWeapon[0]->m_pInfo->nStanceType == CWeapon::STANCE_TYPE_TWO_HANDED_TORSO_TWIST ) {
			SetControlValue( ANIMCONTROL_AIM_ROCKET1, 1.0f );
			m_anAnimStackIndex[ASI_STAND_ALERT] = -1;
		}
	}

	


	SetMaxHealth();

	_WeaponOrUpgradeLevelMayHaveChanged();

	if( m_nPossessionPlayerIndex >= 0 ) {
		Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.SetNormOrigin( 0.0f, 0.23f );
	}

	// Find approx eye point...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CBotGrunt::ClassHierarchyBuild(): Could not locate approx eye point bone '%s'.\n", m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
		m_pApproxEyePoint_WS = &m_MtxToWorld.m_vPos;
	} else {
		m_pApproxEyePoint_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
	}

	// Find gaze direction...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_HEAD ] );
	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CBotGrunt::ClassHierarchyBuild(): Could not locate gaze bone '%s'.\n", m_apszBoneNameTable[ BONE_HEAD ] );
		m_pGazeDir_WS = &m_MtxToWorld.m_vFront;
	} else {
		m_pGazeDir_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vFront;
	}
	
	m_pAISteerMtx = &m_MtxToWorld;

	// Initialize matrix palette (this is important for attached entities)...
	AtRestMatrixPalette();

	// Create tag points...
	if( !TagPoint_CreateFromBoneArray( m_anTagPointBoneNameIndexArray, m_apszBoneNameTable ) ) {
		goto _ExitWithError;
	}

	if( !fclib_strnicmp( _pszGruntMeshFilenameRoot, pszMeshFilename, fclib_strlen(_pszGruntMeshFilenameRoot)) ) {
		if( !m_pPartMgr->Create( this, &m_pPartPool, _BOTPART_FILENAME, PART_INSTANCE_COUNT_PER_TYPE, LIMB_TYPE_COUNT ) ) {
			goto _ExitWithError;
		}

		// Apply debris chunk tint so it matches the color of our Grunt...
		m_pPartMgr->SetDebrisChunkTintColor( &pBuilder->m_MeshTintColor );
	}

	// Make invincible limbs...
	MakeLimbInvincible( LIMB_CODE_HEAD, LIMB_TYPE_HEAD, pBuilder );
	MakeLimbInvincible( LIMB_CODE_ARMS, LIMB_TYPE_LEFT_ARM, pBuilder );
	MakeLimbInvincible( LIMB_CODE_ARMS, LIMB_TYPE_RIGHT_ARM, pBuilder );
	MakeLimbInvincible( LIMB_CODE_TORSO, LIMB_TYPE_TORSO, pBuilder );

	// Set up data port stuff...
	DataPort_SetupTetherShockInfo();

	SetBotFlag_Enemy();

	nLeftHandSecondaryFire = m_pWorldMesh->FindBone( "Secondary_Fire" );

	if( nLeftHandSecondaryFire < 0 ) {
		DEVPRINTF( "CBotGrunt::ClassHierarchyBuild: Bone '%s' doesn't exist in object '%s'.\n", "Secondary_Fire", Name() );
		m_pLeftHandSecondaryFireBonePos = &m_MtxToWorld.m_vPos;
	} else {
		m_pLeftHandSecondaryFireBonePos = &m_pWorldMesh->GetBoneMtxPalette()[nLeftHandSecondaryFire]->m_vPos;
	}

	m_pSpotLight = m_pWorldMesh->GetAttachedLightByID( _HEADLIGHT_TEXTURE_ID );

	if( !pBuilder->m_bAllowWeaponDrop ) {
		SetDroppedWeapon();
	}

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


BOOL CBotGrunt::ClassHierarchyBuilt( void ) {
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


CEntityBuilder *CBotGrunt::GetLeafClassBuilder( void ) {
	return &_BotGruntBuilder;
}


void CBotGrunt::_ClearDataMembers( void ) {
	// Init data members...

	m_pMoveIdentifier = &m_nBoneIndexGroin;

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

	m_nGruntFlags = GRUNTFLAG_NONE;

	m_anAnimStackIndex[ASI_STAND] = ANIMTAP_STAND;
	m_anAnimStackIndex[ASI_STAND_ALERT] = ANIMTAP_STAND_ALERT;

	m_anAnimStackIndex[ASI_WALK] = ANIMTAP_WALK;
	m_anAnimStackIndex[ASI_WALK_ALERT] = ANIMTAP_WALK_ALERT;
	m_anAnimStackIndex[ASI_RUN] = ANIMTAP_RUN;
	m_anAnimStackIndex[ASI_RUN_PANIC] = ANIMTAP_RUN_PANIC;

	m_anAnimStackIndex[ASI_FALL] = ANIMCONTROL_JUMP_FLY;

	m_anAnimStackIndex[ASI_HOP_LEFT] = ANIMTAP_HOP_LEFT;
	m_anAnimStackIndex[ASI_HOP_RIGHT] = ANIMTAP_HOP_RIGHT;
	m_anAnimStackIndex[ASI_STARTLE] = ANIMTAP_STARTLE;
	m_anAnimStackIndex[ASI_ROLL_LEFT] = ANIMTAP_ROLL_LEFT;
	m_anAnimStackIndex[ASI_ROLL_RIGHT] = ANIMTAP_ROLL_RIGHT;

	m_anAnimStackIndex[ASI_DOZE_LOOP] = ANIMTAP_DOZE_LOOP;
	m_anAnimStackIndex[ASI_NAPJERK] = ANIMTAP_NAPJERK;
	m_anAnimStackIndex[ASI_WAKE] = ANIMTAP_WAKE;

	m_anAnimStackIndex[ASI_RELOAD_CLIP_EJECT_OLD] = ANIMCONTROL_RELOAD_CLIP_EJECT_OLD;
	m_anAnimStackIndex[ASI_RELOAD_CLIP_GRAB_NEW] = ANIMCONTROL_RELOAD_CLIP_GRAB_NEW;
	m_anAnimStackIndex[ASI_RELOAD_CLIP_INSERT_NEW] = ANIMCONTROL_RELOAD_CLIP_INSERT_NEW;
	m_anAnimStackIndex[ASI_RELOAD_CLIP_SLAPIN_NEW] = ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW;

	m_anAnimStackIndex[ASI_SNEAK] = ANIMTAP_TURN_IN_PLACE;

	m_anAnimStackIndex[ASI_RC_TETHERED]		= ANIMTAP_RC_TETHERED;
	m_anAnimStackIndex[ASI_RC_POWER_DOWN]	= ANIMTAP_RC_POWER_DOWN;
	m_anAnimStackIndex[ASI_RC_POWER_UP]		= ANIMTAP_RC_POWER_UP;
	m_anAnimStackIndex[ASI_STOOP]			= -1;

	m_anAnimStackIndex[ASI_AIM_PILLBOX]		= ANIMTAP_AIM_PILLBOX;

	m_anAnimStackIndex[ASI_RELOAD_WITH_LEFT_GRAB_AMMO]	= ANIMTAP_RELOAD_ROCKET1_GRAB_AMMO;
	m_anAnimStackIndex[ASI_RELOAD_WITH_LEFT_DROP_IN]	= ANIMTAP_RELOAD_ROCKET1_DROP_IN;
	m_anAnimStackIndex[ASI_RELOAD_WITH_LEFT_FINISH]		= ANIMTAP_RELOAD_ROCKET1_FINISH;

	m_pnEnableBoneNameIndexTableForSummer_Normal = m_anEnableBoneNameIndexTableForSummer_Normal;
	m_pnEnableBoneNameIndexTableForSummer_TetherShock = m_anEnableBoneNameIndexTableForSummer_TetherShock;
	
	m_fGasParticleTimer = fmath_RandomFloatRange( m_BotInfo_Grunt.fGasTimeMin, m_BotInfo_Grunt.fGasTimeMax );
}


void CBotGrunt::ClassHierarchyAddToWorld( void ) {
	u32 i;

	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CBot::ClassHierarchyAddToWorld();
	m_pWorldMesh->UpdateTracker();

//	m_uBotDeathFlags |= BOTDEATHFLAG_PLAYDEATHANIM | BOTDEATHFLAG_COMEAPART | BOTDEATHFLAG_AUTOPERSISTAFTERDEATH;
	for( i=0; i<2; i++ ) {
		if( m_apWeapon[i] ) {
			m_apWeapon[i]->Attach_UnitMtxToParent_PS( this, m_apszBoneNameTable[BONE_ATTACHPOINT_PRIMARY] );
			m_apWeapon[i]->AddToWorld();
			m_apWeapon[i]->ResetToState( CWeapon::STATE_DEPLOYED );
		}
	}
}


void CBotGrunt::ClassHierarchyRemoveFromWorld( void ) {
	u32 i;

	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	for( i=0; i<2; i++ ) {
		if( m_apWeapon[i] ) {
			m_apWeapon[i]->RemoveFromWorld();
		}
	}

//	FreeAllParticleEmitterLocks(this);
	m_nGruntFlags &=~GRUNTFLAG_TERMINAL_STATE_ON;

	m_pWorldMesh->RemoveFromWorld();
	CBot::ClassHierarchyRemoveFromWorld();
}


void CBotGrunt::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
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


const CFVec3A *CBotGrunt::GetApproxEyePoint( void ) const {
	FASSERT( IsCreated() );

	return m_pApproxEyePoint_WS;
}


void CBotGrunt::ClassHierarchyWork() {
	CFVec3A TempVec3A;

	FASSERT( m_bSystemInitialized );

	CBot::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}

	m_fCableGrabTime -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN( m_fCableGrabTime, 0.0f );	

	Power_Work();
	DataPort_Work();

	if( DataPort_IsBeingShocked() ) {
		_UpdateMatrices();
		if (IsDeadOrDying()) {
			DeathWork();
		}

		return;
	}

	ParseControls();

	//funitcurhack++
	if( IsPlayerBot() ) {
		if( (((CHumanControl*)Controls())->m_nPadFlagsSelect1 & GAMEPAD_BUTTON_1ST_PRESS_MASK) && !IsReloading() ) {
			m_abReloadRequest[0] = TRUE;
		}
	}

	// Save a copy of the previous frame's info...
	m_MountPrevPos_WS = m_MountPos_WS;
	m_nPrevState = m_nState;

	// Apply velocity impulses that were accumulated last frame...
	BOOL bImpulseApplied = HandleVelocityImpulses();
	//if (m_nBotFlags & BOTFLAG_GLUED_TO_PARENT)
	//{
	//	m_ImpulseVelocity_WS.Zero();
	//}
	//m_Velocity_WS.Add( m_ImpulseVelocity_WS );

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

	// if we're glued to parent and force panic is on (trapped in loader)  don't translate or do animation work. Just be panicked
	if( (m_nBotFlags & BOTFLAG_GLUED_TO_PARENT) && (m_nBotFlags & BOTFLAG_FORCED_PANIC) ) {
		DeltaTime( ANIMTAP_RUN_PANIC );

		f32 fVal = GetControlValue( ANIMCONTROL_RUN_PANIC );
		if( fVal < 1.0f ) {
			fVal += FLoop_fPreviousLoopSecs * 5.0f;
			FMATH_CLAMP_MAX1( fVal );
			SetControlValue( ANIMCONTROL_RUN_PANIC, fVal );
		}
		
	} else {
		// Move and animate our bot...
		switch( m_nState ) {
		case STATE_GROUND:
			PROTRACK_BEGINBLOCK("GroundXlat");
				HandleGroundTranslation();
				_HandleJumping();
				HandleHopRollStartleGeneric();
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
	PROTRACK_ENDBLOCK();//"WeaponWork");

	// Call the work function for our botfx system...
	_HandleParticles();

	if( m_pCableHook && m_pCableHook->m_bAttached ) {
		m_pCableHook->m_Pos_Draw_WS = *m_pLeftHandSecondaryFireBonePos;
	}

	UpdateSpotLight();

	if (IsDeadOrDying()) {
		DeathWork();
	}
}


void CBotGrunt::_HandleParticles( void ) {
	if( !IsDead() ) {
		if( m_fSpeed_WS < 10.0f ) {
			if( m_BotInfo_Grunt.hGasParticle != FPARTICLE_INVALID_HANDLE ) {
				if( m_nBoneIndexGroin >= 0 ) {
					m_fGasParticleTimer -= FLoop_fPreviousLoopSecs;
					if( m_fGasParticleTimer <= 0.0f ) {
						m_fGasParticleTimer = fmath_RandomFloatRange( m_BotInfo_Grunt.fGasTimeMin, m_BotInfo_Grunt.fGasTimeMax );

						CEParticle *pEParticle;

						pEParticle = eparticlepool_GetEmitter();
						if( pEParticle ) {
							CFMtx43A Mtx;

							pEParticle->StartEmission( m_BotInfo_Grunt.hGasParticle, 1.0f, 0.3f );

							_ComputeGasMatrix( &Mtx );

							pEParticle->Relocate_RotXlatFromScaledMtx_WS_NewScale_WS( &Mtx, Mtx.m_vX.Mag(), 1.0f, FALSE );
							pEParticle->Attach_ToParent_WS( this, m_apszBoneNameTable[BONE_GROIN], FALSE );

							PlaySound( m_BotInfo_Grunt.pSoundGroupGas );
						}
					}
				}
			}
		}
	}
}


void CBotGrunt::_ComputeGasMatrix( CFMtx43A *pMtx ) {
	FASSERT( m_nBoneIndexGroin >= 0 );

	CFVec3A DisplacementVec_BS;
	CFQuatA RotQuat;
	CFMtx43A *pBoneMtx;

	DisplacementVec_BS.Set( 0.0f, -0.2f, -0.2f );
	pBoneMtx = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexGroin];

	RotQuat.BuildQuat( pBoneMtx->m_vX, FMATH_DEG2RAD( 170.0f ) );

	RotQuat.MulPoint( pMtx->m_vX, pBoneMtx->m_vX );
	RotQuat.MulPoint( pMtx->m_vY, pBoneMtx->m_vY );
	RotQuat.MulPoint( pMtx->m_vZ, pBoneMtx->m_vZ );

	pBoneMtx->MulPoint( pMtx->m_vPos, DisplacementVec_BS );
}


BOOL CBotGrunt::GrabCable( CEZipLine *pCable ) {
	if( pCable == m_pCablePrev && m_fCableGrabTime != 0.0f ) {
		return FALSE;
	}

	if( m_pPartMgr && m_pPartMgr->GetLimbState( LIMB_TYPE_LEFT_ARM ) != CBotPartMgr::LIMB_STATE_INTACT ) {
		return FALSE;
	}

	m_pCableHook = pCable->AttachHook( &m_MountPos_WS, &m_Velocity_WS, this );

	if( m_pCableHook == NULL ) {
		return FALSE;
	}

//	m_MountPos_WS.y += 1.0f;
//	m_Velocity_WS.y += m_pBotInfo_Jump->fVerticalVelocityJump1;
//	m_Velocity_MS.y += m_pBotInfo_Jump->fVerticalVelocityJump1;

	m_nJumpState = BOTJUMPSTATE_CABLE;
	SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );
	SetControlValue( ANIMCONTROL_JUMP_TUCK, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_UNTUCK, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_CABLE_GRASP, 1.0f );
	UpdateTime( ANIMCONTROL_CABLE_GRASP, 1.0f );
	m_fUnitCableBlend = 1.0f;

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

	ZipLineVerletSetup( );

	return TRUE;
}

void CBotGrunt::ReleaseCable( void ) {
	SetControlValue( ANIMCONTROL_CABLE_GRASP, 0.0f );

	m_pCableHook->m_pCable->DetachHook( m_pCableHook, this );
	m_pCablePrev = m_pCableHook->m_pCable;
	m_fCableGrabTime = BOT_ZIPLINE_GRAB_TIME;
	m_pCableHook = NULL;

	*m_Anim.m_pAnimManMtx->GetFrameAndFlagAsChanged( BONE_LEFT_HAND ) = CFMtx43A::m_IdentityMtx;
	Summer_UpdateBoneMask( m_apszBoneNameTable[BONE_LEFT_HAND], FALSE );
}

void CBotGrunt::_HandleCableAndDoubleJump( void ) {
	if( m_nJumpState != BOTJUMPSTATE_CABLE ) {
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


void CBotGrunt::_HandleCableTranslation( void ) {
	if( m_nJumpState == BOTJUMPSTATE_CABLE ) {
		if( m_pCableHook->m_bAttached ) {
			// If our limb got blown off, detach
			if( m_pPartMgr && m_pPartMgr->GetLimbState( LIMB_TYPE_LEFT_ARM ) != CBotPartMgr::LIMB_STATE_INTACT ) {
				_StartSingleJump();
				ReleaseCable();

				return;
			}

			// Update the hook hand brake value
			m_pCableHook->m_fInputBrakeAmount = m_fControls_Fire2;

			/*CFVec3A Pos_WS_Diff;
			
			Pos_WS_Diff.Sub( m_pCableHook->m_Pos_WS, *m_pLeftHandSecondaryFireBonePos );
			m_MountPos_WS.Add( Pos_WS_Diff );*/

			// Don't spin the hand
			CFMtx43A::m_RotZ.SetRotationZ( -m_fMountYaw_WS + m_pCableHook->m_fYawSnapOffset - FMATH_HALF_PI );
			*m_Anim.m_pAnimManMtx->GetFrameAndFlagAsChanged( BONE_LEFT_HAND ) = CFMtx43A::m_RotZ;

			m_Velocity_WS.Zero();
			m_Velocity_MS.Zero();

			/*m_Velocity_WS = m_pCableHook->m_Vel_WS;

			// Update velocities...
			WS2MS( m_Velocity_MS, m_Velocity_WS );
			VelocityHasChanged();*/

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


void CBotGrunt::_HandleCableAnimations( void ) {
	if( m_fUnitCableBlend>0.0f && m_nJumpState!=BOTJUMPSTATE_CABLE ) {
		m_fUnitCableBlend -= FLoop_fPreviousLoopSecs * m_fTuckUntuckAnimTimeMult;	// SER Change this!
		FMATH_CLAMPMIN( m_fUnitCableBlend, 0.0f );
		SetControlValue( ANIMCONTROL_CABLE_GRASP, m_fUnitCableBlend );
	}
}


void CBotGrunt::_HandleJumping( void ) {
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


void CBotGrunt::_EnterFlyMode( void ) {
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


void CBotGrunt::_StartVelocityJump( const CFVec3A *pJumpVelocity_WS ) {
	CFVec3A JumpVelocity_WS = *pJumpVelocity_WS;
	FMATH_CLAMP( JumpVelocity_WS.y, 0.0f, m_fMaxDeltaJumpVelY_WS );

	_StartSingleJump( &JumpVelocity_WS );

	if( pJumpVelocity_WS->y >= m_pBotInfo_Jump->fVerticalVelocityJump1 ) {
		_StartDoubleJump( FALSE );
	}
}


void CBotGrunt::_StartSingleJump( const CFVec3A *pJumpVelocity_WS ) {
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

void CBotGrunt::_StartDoubleJump( BOOL bAddToVelocityY ) {
	if( !HaveTimeToDoubleJump() ) {
		return;
	}

	StartDoubleJumpGeneric(bAddToVelocityY,
							ANIMTAP_JUMP_TUCK,
							ANIMTAP_JUMP_UNTUCK,
							ANIMCONTROL_JUMP_LAUNCH,
							ANIMCONTROL_JUMP_FLY,
							ANIMCONTROL_JUMP_TUCK);
	DetachAllSwarmers();
}


void CBotGrunt::_JumpLanded( void ) {
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


void CBotGrunt::_HandleJumpAnimations( void ) {
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


void CBotGrunt::_HandleWeaponWork( void ) {
	if( m_apWeapon[0] ) {
		m_apWeapon[0]->Work();
	}
	if( m_apWeapon[1] ) {
		m_apWeapon[1]->Work();
	}
}


// Called by the weapon when it's EUK upgrade level has changed...
void CBotGrunt::NotifyWeaponUpgraded( CWeapon *pUpgradedWeapon ) {
	if( pUpgradedWeapon != m_apWeapon[0] ) {
		// Not our current weapon...
		return;
	}

	// The EUK upgrade level of our currently-selected primary weapon
	// has changed...

	_WeaponOrUpgradeLevelMayHaveChanged();
}


void CBotGrunt::UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup ) {
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

#define _ALERT_FIRE_THRESHOLD		( 0.75f )

void CBotGrunt::_HandleFiringAnimations( void ) {
	f32 fUnitFireControlValue;
	BOOL bLimbDamaged;

	// Primary fire...
	u32 uFireAnimControlId = ANIMCONTROL_FIRE_1;
	u32 uFireAnimTapId = ANIMTAP_FIRE_1;

	if( m_apWeapon[0]->m_pInfo->nStanceType == CWeapon::STANCE_TYPE_TWO_HANDED_TORSO_TWIST ) {
		uFireAnimControlId = ANIMCONTROL_FIRE_ROCKET1;
		uFireAnimTapId = ANIMTAP_FIRE_ROCKET1;
	} else {
		uFireAnimControlId = ANIMCONTROL_FIRE_1;
		uFireAnimTapId = ANIMTAP_FIRE_1;
		if ( IsAlertOn() && (m_fUnitAlert > _ALERT_FIRE_THRESHOLD) ) {
			uFireAnimControlId = ANIMCONTROL_FIRE_1_ALERT;
			uFireAnimTapId = ANIMTAP_FIRE_1_ALERT;
			SetControlValue( ANIMCONTROL_FIRE_1, 0.0f);
		}
		else {
			SetControlValue( ANIMCONTROL_FIRE_1_ALERT, 0.0f);
		}
	}

	if( m_nBotFlags & BOTFLAG_PLAY_FIRE1_ANIM ) {
		SetControlValue( uFireAnimControlId, 1.0f );
		//fUnitFireControlValue = GetControlValue( uFireAnimControlId );

		//if( fUnitFireControlValue < 1.0f ) {
		//	//fUnitFireControlValue += m_pBotInfo_Weapon->fFire1BlendInSpeed * FLoop_fPreviousLoopSecs;
		//	//FMATH_CLAMPMAX( fUnitFireControlValue, 1.0f );
		//	//SetControlValue( uFireAnimControlId, fUnitFireControlValue );
		//}

		if( DeltaTime( uFireAnimTapId, FLoop_fPreviousLoopSecs, TRUE ) ) {
			FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM );
//			ZeroTime( uFireAnimTapId );
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
		SetControlValue( ANIMCONTROL_FIRE_2_LOWER, fUnitFireControlValue );

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

			bLimbDamaged = FALSE;

			// Attach ammo...
			if( m_pPartMgr && m_pPartMgr->IsCreated() ) {
				if( m_pPartMgr->GetLimbState( LIMB_TYPE_LEFT_ARM ) == CBotPartMgr::LIMB_STATE_DANGLING ) {
					bLimbDamaged = TRUE;
				}
			}

			m_apWeapon[1]->Throwable_ThrowGrenade_MountAimDirection( -1.0f, bLimbDamaged );
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


void CBotGrunt::_HandleAimAnimations( void ) {
	f32 fUnitAimPitch;
	CFAnimFrame TorsoQuat, HeadQuat;

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
		// Rocket launcher...
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
			TorsoTwistQuat.BuildQuat( CFVec3A::m_UnitAxisY, fmath_UnitLinearToSCurve(m_fUnitTorsoTwistBlend) * _ROCKET1TORSOTWIST_ANGLE, CFVec3A::m_Null );
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


void CBotGrunt::_HandleWeaponAnimations( void ) {
	HandleWeaponReloadingAnimations();
	_HandleFiringAnimations();
	_HandleAimAnimations();
}


void CBotGrunt::_WeaponOrUpgradeLevelMayHaveChanged( void ) {
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

void CBotGrunt::_UpdateMatrices( void ) {
	CFMtx43A FlipMtx, MeshMtx, NewMtx, EntityMtxToWorld;

	CFMtx43A::m_XlatRotY.SetRotationY( m_fMountYaw_WS + m_fLegsYaw_MS );
	CFMtx43A::m_XlatRotY.m_vPos = m_MountPos_WS;

	// Update xfm...
	if( m_fFlipPitch ) 
	{
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
		ComputeMtxPalette( TRUE );
	PROTRACK_ENDBLOCK();//"ComputeMtxPal");

	if( (m_pPartMgr == NULL) || !m_pPartMgr->IsCreated() || (m_pPartMgr->GetLimbState( LIMB_TYPE_HEAD ) == CBotPartMgr::LIMB_STATE_INTACT) ) {
		m_GazeUnitVec_WS.ReceiveUnit( *m_pGazeDir_WS );
	} else {
		m_GazeUnitVec_WS = m_MtxToWorld.m_vFront;
	}

	EntityMtxToWorld.Identity();
	EntityMtxToWorld.SetRotationYXZ( m_fMountYaw_WS, m_fMountPitch_WS, 0.0f );
	EntityMtxToWorld.m_vPos = m_MountPos_WS;
	if (!(m_nBotFlags & BOTFLAG_GLUED_TO_PARENT)) {
		Relocate_RotXlatFromUnitMtx_WS( &EntityMtxToWorld, TRUE, m_pMoveIdentifier );
	} else {
		RelocateAllChildren();
	}
}


// Returns TRUE if Grunt's finger is on his primary weapon's trigger.
// In other words, TRUE is returned if Grunt is in a state where he
// can pull the trigger.
BOOL CBotGrunt::_IsFingerOnTrigger1( void ) const {
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

				// check to see if we're throwing a grenade
				if( m_nThrowState != THROWSTATE_NONE ) {
					bFingerIsOnTrigger = FALSE;
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
BOOL CBotGrunt::IsFingerOnTrigger2( void ) const {
	BOOL bFingerIsOnTrigger = FALSE;

	if( m_nThrowState == THROWSTATE_NONE ) {
		// Not currently throwing a secondary grenade...

		if( m_nWSState==WS_STATE_NONE || m_nWSState==WS_STATE_RELEASING ) {
			// We're not switching or reloading the weapon...

			//if( m_apWeapon[0]->m_pInfo->nGrip == CWeapon::GRIP_RIGHT_ARM ) {
			//	// Primary weapon is not two-handed...

				if( m_nJumpState != BOTJUMPSTATE_CABLE ) {
					// We're not on a cable...

					if( !(m_apWeapon[0]->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_LEFT_HAND_RELOADS) || (m_nWRState == WR_STATE_NONE) ) {
						// Primary weapon is either auto-reloading, or is not currently reloading...

						bFingerIsOnTrigger = TRUE;
					}
				}
			//}
		}
	}

	return bFingerIsOnTrigger;
}


void CBotGrunt::_HandleWeaponFiring( void ) {
	f32 fTrig1, fTrig2;
	u32 nFireBitsPrimary;

	fTrig1 = m_fControls_Fire1;
	fTrig2 = m_fControls_Fire2;

	if( !_IsFingerOnTrigger1() ) {
		fTrig1 = 0.0f;
	}

	if( !IsFingerOnTrigger2() ) {
		fTrig2 = 0.0f;
	}

	u32 uRightArmLimbState = CBotPartMgr::LIMB_STATE_INTACT;
	u32 uLeftArmLimbState = CBotPartMgr::LIMB_STATE_INTACT;

	if( m_pPartMgr && m_pPartMgr->IsCreated() ) {
		uRightArmLimbState = m_pPartMgr->GetLimbState( LIMB_TYPE_RIGHT_ARM );
		uLeftArmLimbState = m_pPartMgr->GetLimbState( LIMB_TYPE_LEFT_ARM );
	}

	switch( uRightArmLimbState ) {
		case CBotPartMgr::LIMB_STATE_DANGLING: {
			CFVec3A vMuzzlePt;
			CFVec3A vTgtPt;
			m_apWeapon[0]->ComputeMuzzlePoint_WS( &vMuzzlePt );
			vTgtPt.Mul( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPriFire]->m_vFront, 1000.0f );
			vTgtPt.Add( vMuzzlePt );
			nFireBitsPrimary = m_apWeapon[0]->TriggerWork( fTrig1, fTrig2, &vTgtPt /*&(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPriFire]->m_vFront)*/, NULL );
			break;
		}

		case CBotPartMgr::LIMB_STATE_REMOVED:	//don't fire if limb removed
			nFireBitsPrimary = m_apWeapon[0]->TriggerWork( 0.f, 0.f, &m_TargetedPoint_WS, NULL );
//			nFireBitsPrimary = 0;
			break;

		default:
			nFireBitsPrimary = m_apWeapon[0]->TriggerWork( fTrig1, fTrig2, &m_TargetedPoint_WS, NULL );
			break;
	}


	if( nFireBitsPrimary ) {
		// We fired the primary weapon...
		JustFired();   // No more disguise...

		FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM );
		ZeroTime( ANIMTAP_FIRE_1 );
		ZeroTime( ANIMTAP_FIRE_1_ALERT );
		ZeroTime( ANIMTAP_FIRE_ROCKET1 );
	}

	if( m_apWeapon[1] ) {
		if( (m_apWeapon[1]->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_THROWABLE) && (uLeftArmLimbState != CBotPartMgr::LIMB_STATE_REMOVED) ) {
			// Secondary is throwable...

			if( m_apWeapon[1]->Throwable_TriggerWork( fTrig2 ) ) {
				// We can throw the grenade...

				m_nThrowState = THROWSTATE_REACHING_FOR_AMMO;
				ZeroTime( ANIMTAP_FIRE_2_UPPER );

				JustFired();	// No more disguise...
			}
		}
	}
}


void CBotGrunt::_AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {

	if( !m_pCBGrunt ) {
		rNewMtx.Mul( rParentMtx, rBoneMtx );
		return;
	}

	FASSERT(m_pCBGrunt && m_pCBGrunt->TypeBits() & ENTITY_BIT_BOTGRUNT);

	if( m_pCBGrunt->m_pPartMgr &&
		m_pCBGrunt->m_pPartMgr->IsCreated() &&
		m_pCBGrunt->m_pPartMgr->AnimBoneCallbackFunctionHandler( nBoneIndex, rNewMtx, rParentMtx, rBoneMtx ) ) {
		return;
	} else if( nBoneIndex == (u32)((CBotGrunt *)m_pCBGrunt)->m_nBoneIndexGroin ) {
		// Groin
		rNewMtx.Mul( rParentMtx, rBoneMtx );
		m_GroinVecY_WS = rNewMtx.m_vUp;
	} else if( nBoneIndex == (u32)((CBotGrunt *)m_pCBGrunt)->m_nBoneIndexTorso ) {
		// Torso
		CFMtx43A WorldMtx;
		CFQuatA Quat;
		CBotGrunt *pBotGrunt = (CBotGrunt *)m_pCBGrunt;

		Quat.BuildQuat( m_GroinVecY_WS, pBotGrunt->m_fAimDeltaYaw_MS - pBotGrunt->m_fLegsYaw_MS );
		WorldMtx.Mul( rParentMtx, rBoneMtx );

		rNewMtx.m_vPos = WorldMtx.m_vPos;
		Quat.MulPoint( rNewMtx.m_vRight, WorldMtx.m_vRight );
		Quat.MulPoint( rNewMtx.m_vUp, WorldMtx.m_vUp );
		Quat.MulPoint( rNewMtx.m_vFront, WorldMtx.m_vFront );	
	} else {
		// Head
		m_pCBGrunt->HeadLookUpdate( rNewMtx, rParentMtx, rBoneMtx, FALSE );
	}
}


BOOL CBotGrunt::_InventoryCallback( CInventoryCallbackReason_e eReason, CInventory *pInventory, u32 nHandIndex, u32 nWeaponIndex ) {
	CBotGrunt *pBotGrunt = (CBotGrunt *)pInventory->m_pUser;

	switch( eReason ) {
	case IREASON_WEAPONCHANGE:
		FASSERT_NOW;

		break;

	case IREASON_RELOAD:
		if( nHandIndex == 0 ) {
			pBotGrunt->m_abReloadRequest[0] = TRUE;
		}

		break;

	case IREASON_WEAPONUPGRADED:
		FASSERT_NOW;

		break;

	}

	return TRUE;
}


void CBotGrunt::CheckpointSaveSelect( s32 nCheckpoint ) {
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


void CBotGrunt::Die( BOOL bSpawnDeathEffects/*=TRUE*/, BOOL bSpawnGoodies ) {
	CBot::Die( bSpawnDeathEffects, bSpawnGoodies );

	// If we are allowed to drop a weapon and we have a weapon and we don't have a goodie bag, then try to drop it.
	if( !DroppedWeapon() && m_apWeapon[0] && !m_pGoodieBag ) {
		CPlayer *pPlayer = &Player_aPlayer[0];
		CBot *pBot = (CBot *) pPlayer->m_pEntityOrig;
		CWeapon::WeaponType_e eType = m_apWeapon[0]->Type();
		
		FASSERT( pBot );

		// Only throw out weapons for Glitch
		if( !( pBot->TypeBits() & ENTITY_BIT_BOTGLITCH ) ) {
			return;
		}

		CBotGlitch *pGlitch = (CBotGlitch *) pBot;

		// Only throw out a weapon if Glitch has it as well, but thrown the EUK level that Glitch has.
		u32 i;
		s8 j;
		s32 nEUK = -1;

		for( i = 0; i < 2; ++i ) {
			for( j = 0; j < pGlitch->m_WeaponInv[i].m_nWeaponInvCount; ++j ) {
				if( pGlitch->m_WeaponInv[i].m_apWeapon[j] && pGlitch->m_WeaponInv[i].m_apWeapon[j]->Type() == eType ) {
					nEUK = pGlitch->m_WeaponInv[i].m_apWeapon[j]->GetUpgradeLevel();
					break;
				}
			}
		}

		if( nEUK != -1 ) {
			CFMtx43A Mtx;
			CFVec3A Vel = CFVec3A::m_UnitAxisY;
			CollectableType_e eType = CCollectable::ClassifyBotWeapon( m_apWeapon[ 0 ], (u32) nEUK );

			if( eType != COLLECTABLE_UNKNOWN ) {
				Vel.Mul( 30.0f );
				Mtx.Identity();
				Mtx.m_vPos = m_MountPos_WS;
				Mtx.m_vPos.Add( CFVec3A::m_UnitAxisY );

				CCollectable::PlaceIntoWorld( eType, &Mtx, &Vel, 1.0f );
			}

			SetDroppedWeapon();
		}
	}
}

void CBotGrunt::DrawText( void )
{
	cwchar* wszFormat = (CPlayer::m_nPlayerCount > 1) ? L"~f9~C92929299~w0~al~o1%ls" : L"~f1~C92929299~w0~al~s1.00%ls";

	CVehicle* pVehicleNearby = NULL;

	// don't draw text while letterbox is up
	if( letterbox_GetUnitSlideOnAmount() != 0.0f ) {
		return;
	}

	// print "enter vehicle" text
	if( m_pActionableEntityNearby && 
		m_nPossessionPlayerIndex >= 0 &&
		!IsDeadOrDying() )
	{
		if( (m_pActionableEntityNearby->TypeBits() & ENTITY_BIT_VEHICLE) )
		{
			pVehicleNearby = (CVehicle*)m_pActionableEntityNearby;

			if( pVehicleNearby && m_pDrivingVehicle == NULL && !pVehicleNearby->IsDeadOrDying() )
			{
				if( pVehicleNearby->IsUpsideDown() )
				{
					ftext_Printf( 0.22f, 0.58f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_PRESS_Y_TO_FLIP_VEHICLE_OVER ] );
				}
				else if( !pVehicleNearby->IsPlayerDriveable() )
				{
					ftext_Printf( 0.22f, 0.58f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_VEHICLE_IS_LOCKED ] );
				}
				else if( pVehicleNearby->CanOccupyStation( this, CVehicle::STATION_DRIVER ) == CVehicle::STATION_STATUS_EMPTY )
				{
					ftext_Printf( 0.22f, 0.58f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_PRESS_Y_TO_DRIVE_VEHICLE ] );
				}
				else if( pVehicleNearby->CanOccupyStation( this, CVehicle::STATION_GUNNER ) == CVehicle::STATION_STATUS_EMPTY )
				{
					ftext_Printf( 0.22f, 0.58f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_PRESS_Y_TO_OPERATE_GUN ] );
				}
			}
		}
		else if((m_pActionableEntityNearby->TypeBits() & ENTITY_BIT_SITEWEAPON))
		{
			CBotSiteWeapon* pSiteWeapon = (CBotSiteWeapon*)m_pActionableEntityNearby;

			if (!pSiteWeapon->IsDeadOrDying() && 
				pSiteWeapon->m_pBotDef->m_nSubClass != BOTSUBCLASS_SITEWEAPON_RATGUN ) {
				
				if (((pSiteWeapon->IsStationObstructed()==NULL) || (pSiteWeapon->IsStationObstructed()==this)) && 
					(pSiteWeapon->CanOccupyStation(this)) )
				{
						ftext_Printf( 0.22f, 0.58f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_PRESS_Y_TO_OPERATE_GUN ] );
				}
			}
		}
	}
}


void CBotGrunt::ResetToNeutral( void ) {
	CBot::ResetToNeutral();

	SetControlValue( ANIMCONTROL_FIRE_2_UPPER, 0.0f );
	SetControlValue( ANIMCONTROL_FIRE_2_LOWER, 0.0f );
}