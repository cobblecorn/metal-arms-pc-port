//////////////////////////////////////////////////////////////////////////////////////
// bottitan.cpp - 
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
// 10/14/02 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "bottitan.h"
#include "meshtypes.h"
#include "player.h"
#include "ProTrack.h"
#include "meshentity.h"
#include "weapon_chaingun.h"
#include "weapon.h"
#include "fcamera.h"
#include "gamecam.h"
#include "fworld_coll.h"
#include "fxshockwave.h"
#include "fclib.h"
#include "fresload.h"
#include "FCheckPoint.h"
#include "splat.h"
#include "eshield.h"
#include "explosion.h"
#include "damage.h"
#include "hud2.h"
#include "botpart.h"
#include "LightPool.h"
//#include "gamepad.h"		// Remove this at some point
#include "sas_user.h"

#define _STEVETEST		0


#if _STEVETEST
	#include "meshentity.h"
	#include "fverlet.h"

	static CMeshEntity *_pMeshEntity;
#endif



#define _BOTINFO_FILENAME				"b_titan"
#define _BOTPART_FILENAME				"bp_titan"

static cchar _apszTitanMeshFilename[]	= "GRMTtitan00";
cchar* BotTitan_pszSoundEffectBank		= "titan";
cchar* BotHeavy_pszSoundEffectBank		= "Bots_Heavy";
cchar* Weapons_pszSoundEffectBank		= "Weapons";

#define _NPC_TITAN_CHAINGUN_UPGRADE_LEVEL 1

#define _DEBUG_DISABLE_SHIELD		0

#define	_TEXLAYER_POWERPLANT		1
#define _TEXLAYER_LIGHT				38
#define _TEXLAYER_ROCKET			80

#define _FIRE_ANIM_TRANSITION_TIME	(1.0f/0.25f)
#define _MAX_ELBOW_AIMANGLE			(0.5f * FMATH_QUARTER_PI)
#define _MAX_ARM_AIMANGLE			(0.75f*FMATH_HALF_PI)
#define _MAX_ARM_ROT_SPEED			((2.0f*FMATH_QUARTER_PI))
#define _STOOP_MINHEIGHT			9.0f

#define _STOMPCAM_POSX				0.0f
#define _STOMPCAM_POSY				-5.0f
#define _STOMPCAM_POSZ				-5.0f

#define _STOMPCAM_LOOKATX			0.0f
#define _STOMPCAM_LOOKATY			-5.0f
#define _STOMPCAM_LOOKATZ			0.0f		

#define _STOMPCAM_TRANSON			2.0f
#define _STOMPCAM_TRANSOFF			1.0f
#define _STOMPCAM_HOLDTIME			0.5f



#define _ROCKET_REACT_ANGLE			FMATH_DEG2RAD( 5.0f )
#define _ROCKET_OO_REACT_ON_TIME	(1.0f/0.1f)
#define _ROCKET_OO_REACT_OFF_TIME	1.0f

//these are used by the power plant.  Each action causes a certain draw of power which goes in a pool and is drained at the recovery speed
#define _POWERDRAW_JUMP2			15.0f
#define _POWERDRAW_JUMP1			5.0f
#define	_POWERDRAW_RUN				3.0f
#define _POWERDRAW_GUNS				5.0f
#define _POWERDRAW_ROCKET			5.0f
#define _POWERDRAW_MAX_POOL			50.0f

#define _POWERDRAW_REC_LAG			0.15f		// max recovery is _POWERDRAW_MAX_POOL * _POWERDRAW_REC_LAG, plant targets current draw * _POWERDRAW_REC_LAG
#define _POWERDRAW_RECOVERY			4.0f
#define _POWERDRAW_REC_CPS			0.5f		// rate the recovery can change per second

#define _LIGHT_OFF					2
#define _LIGHT_ON					1

#define _AIM_OO_ON_TIME				2.0f
#define _AIM_OO_OFF_TIME			1.0f
#define _AIM_SPEED					500.0f

#define _FIRE_THRESHOLD				( 0.25f )


// Stoop Summer Bones:
typedef enum {
	_STOOP_BONE_TORSO,				//"Torso",
	_STOOP_BONE_REACTOR,			//"Reactor",
	_STOOP_BONE_LLEG_UPPER,			//"L_Leg_Upper",
	_STOOP_BONE_RLEG_UPPER,			//"R_Leg_Upper",
	_STOOP_BONE_LLEG_LOWER,			//"L_Leg_Lower",
	_STOOP_BONE_RLEG_LOWER,			//"R_Leg_Lower",
	_STOOP_BONE_GROIN,				//"Groin",
	_STOOP_BONE_ROCKETLAUNCHER,		//"RocketLauncher",
	_STOOP_BONE_LARM_UPPER,			//"L_Arm_Upper",
	_STOOP_BONE_RARM_UPPER,			//"R_Arm_Upper",
	_STOOP_BONE_RFOOT_FRONT,		//"R_Foot_Front",		
	_STOOP_BONE_RFOOT_BACK,			//"R_Foot_Back",	
	_STOOP_BONE_RHEEL,				//"R_Heel",
	_STOOP_BONE_LFOOT_FRONT,		//"L_Foot_Front",		
	_STOOP_BONE_LFOOT_BACK,			//"L_Foot_Back",	
	_STOOP_BONE_LHEEL,				//"L_Heel",
	_STOOP_BONE_SPOTLIGHT,			//"Spotlight",
	_STOOP_BONE_HEAD,				//"Head",					

	_STOOP_BONE_COUNT
};


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotTitanBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CBotTitanBuilder _BotTitanBuilder;


void CBotTitanBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_BOTTITAN, pszEntityType );

	// Override defaults set by our parent classes...
	m_nEC_HealthContainerCount = CBotTitan::m_BotInfo_Gen.nBatteryCount;
	m_pszEC_ArmorProfile = CBotTitan::m_BotInfo_Gen.pszArmorProfile;
	m_bEnableLight	= FALSE;
	m_bEnableShield	= FALSE;

	// This class of bots can be recruited...
	FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_CLASS_CAN_BE_RECRUITED );

	// This class of bots shows up on Glitch's radar...
	FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_SHOWS_UP_ON_RADAR );
}


BOOL CBotTitanBuilder::PostInterpretFixup( void ) {
	if( !CBotBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	return TRUE;

	// Error:
_ExitWithError:
	return FALSE;
}


BOOL CBotTitanBuilder::InterpretTable( void ) {

	cchar *pszString;

	if( !fclib_stricmp( CEntityParser::m_pszTableName, "SpotLight" ) ) {
		if( CEntityParser::Interpret_String( &pszString ) ) {
			if( !fclib_stricmp( pszString, "On" ) ) {
				m_bEnableLight = TRUE;
			} else if( !fclib_stricmp( pszString, "Off" ) ) {
				m_bEnableLight = FALSE;
			} else {
				CEntityParser::Error_InvalidParameterValue();
			}
		}

		return TRUE;
	}

	if( !fclib_stricmp( CEntityParser::m_pszTableName, "Shield" ) ) {
		if( CEntityParser::Interpret_String( &pszString ) ) {
			if( !fclib_stricmp( pszString, "On" ) ) {
				m_bEnableShield = TRUE;
			} else if( !fclib_stricmp( pszString, "Off" ) ) {
				m_bEnableShield = FALSE;
			} else {
				CEntityParser::Error_InvalidParameterValue();
			}
		}

		return TRUE;
	}

	return CBotBuilder::InterpretTable();
}



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotTitan
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************



BOOL CBotTitan::m_bSystemInitialized	= FALSE;
u32	 CBotTitan::m_nBotClassClientCount	= 0;
BOOL CBotTitan::m_bSoundsInitialized	= FALSE;

CBotTitan::BotInfo_Gen_t			CBotTitan::m_BotInfo_Gen;
CBotTitan::BotInfo_MountAim_t		CBotTitan::m_BotInfo_MountAim;
CBotTitan::BotInfo_Walk_t			CBotTitan::m_BotInfo_Walk;
CBotTitan::BotInfo_Jump_t			CBotTitan::m_BotInfo_Jump;
CBotTitan::BotInfo_Weapon_t			CBotTitan::m_BotInfo_Weapon;
CBotTitan::BotInfo_Titan_t			CBotTitan::m_BotInfo_Titan;
CBotTitan::BotInfo_TitanSounds_t	CBotTitan::m_BotInfo_TitanSounds;

FExplosion_GroupHandle_t	CBotTitan::m_hDebrisExplosion		= FEXPLOSION_INVALID_HANDLE;
FExplosion_GroupHandle_t	CBotTitan::m_hRocketExplosionImpact	= FEXPLOSION_INVALID_HANDLE;
FExplosion_GroupHandle_t	CBotTitan::m_hRocketExplosionFire	= FEXPLOSION_INVALID_HANDLE;

SmokeTrailAttrib_t			CBotTitan::m_SmokeTrailAttrib;
CBotAnimStackDef			CBotTitan::m_AnimStackDef;
CFVec3A						CBotTitan::m_GroinVecY_WS;
CFVec3A						CBotTitan::m_vecStompCameraLookat;
CFVec3A						CBotTitan::m_vecStompCameraPos;
CBotTitan*					CBotTitan::m_pCBTitan				= NULL;

FMesh_t*					CBotTitan::m_pTitanMesh				= NULL;
//FParticle_DefHandle_t		CBotTitan::m_hSparksPartDef			= FPARTICLE_INVALID_HANDLE;

CFTexInst					CBotTitan::m_CoronaTexInst;


CBotPartPool *CBotTitan::m_pPartPool;


BOOL CBotTitan::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );

	m_nBotClassClientCount = 0;

	m_bSystemInitialized = TRUE;

	//Initialize the rocket smoke trail data
#if 0
	m_SmokeTrailAttrib.nFlags = SMOKETRAIL_FLAG_NONE;
	m_SmokeTrailAttrib.pTexDef = NULL;

	m_SmokeTrailAttrib.fScaleMin_WS = 0.5f;
	m_SmokeTrailAttrib.fScaleMax_WS = 0.75f;
	m_SmokeTrailAttrib.fScaleSpeedMin_WS = 3.0f;
	m_SmokeTrailAttrib.fScaleSpeedMax_WS = 5.0f;
	m_SmokeTrailAttrib.fScaleAccelMin_WS = -0.5f;
	m_SmokeTrailAttrib.fScaleAccelMax_WS = -0.75f;

	m_SmokeTrailAttrib.fXRandSpread_WS = 0.2f;
	m_SmokeTrailAttrib.fYRandSpread_WS = 0.4f;
	m_SmokeTrailAttrib.fDistBetweenPuffs_WS = 0.4f;
	m_SmokeTrailAttrib.fDistBetweenPuffsRandSpread_WS = 0.05f;

	m_SmokeTrailAttrib.fYSpeedMin_WS = 0.5f;
	m_SmokeTrailAttrib.fYSpeedMax_WS = 1.0f;
	m_SmokeTrailAttrib.fYAccelMin_WS = -0.2f;
	m_SmokeTrailAttrib.fYAccelMax_WS = -0.5f;

	m_SmokeTrailAttrib.fUnitOpaqueMin_WS = 0.7f;
	m_SmokeTrailAttrib.fUnitOpaqueMax_WS = 1.0f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMin_WS = -0.5f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMax_WS = -0.9f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMin_WS = 0.0f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMax_WS = 0.0f;

	m_SmokeTrailAttrib.StartColorRGB.Set( 1.0f, 0.5f, 0.25f );
	m_SmokeTrailAttrib.EndColorRGB.Set( 0.75f, 0.75f, 0.75f );
	m_SmokeTrailAttrib.fStartColorUnitIntensityMin = 1.0f;
	m_SmokeTrailAttrib.fStartColorUnitIntensityMax = 0.9f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMin = 0.8f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMax = 1.0f;

	m_SmokeTrailAttrib.fColorUnitSliderSpeedMin = 3.5f;
	m_SmokeTrailAttrib.fColorUnitSliderSpeedMax = 4.5f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMin = 0.0f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMax = 0.0f;
#else
	m_SmokeTrailAttrib.nFlags = SMOKETRAIL_FLAG_NONE;
	m_SmokeTrailAttrib.pTexDef = NULL;

	m_SmokeTrailAttrib.fScaleMin_WS = 0.5f;
	m_SmokeTrailAttrib.fScaleMax_WS = 0.6f;
	m_SmokeTrailAttrib.fScaleSpeedMin_WS = 1.2f;
	m_SmokeTrailAttrib.fScaleSpeedMax_WS = 1.5f;
	m_SmokeTrailAttrib.fScaleAccelMin_WS = -1.0f;
	m_SmokeTrailAttrib.fScaleAccelMax_WS = -1.5f;

	m_SmokeTrailAttrib.fXRandSpread_WS = 0.2f;
	m_SmokeTrailAttrib.fYRandSpread_WS = 0.2f;
	m_SmokeTrailAttrib.fDistBetweenPuffs_WS = 0.4f;
	m_SmokeTrailAttrib.fDistBetweenPuffsRandSpread_WS = 0.1f;

	m_SmokeTrailAttrib.fYSpeedMin_WS = 0.5f;
	m_SmokeTrailAttrib.fYSpeedMax_WS = 1.0f;
	m_SmokeTrailAttrib.fYAccelMin_WS = -0.2f;
	m_SmokeTrailAttrib.fYAccelMax_WS = -0.5f;

	m_SmokeTrailAttrib.fUnitOpaqueMin_WS = 0.5f;
	m_SmokeTrailAttrib.fUnitOpaqueMax_WS = 0.7f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMin_WS = -0.5f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMax_WS = -0.9f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMin_WS = 0.0f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMax_WS = 0.0f;

	m_SmokeTrailAttrib.StartColorRGB.Set( 1.0f, 0.5f, 0.25f );
	m_SmokeTrailAttrib.EndColorRGB.White();
	m_SmokeTrailAttrib.fStartColorUnitIntensityMin = 1.0f;
	m_SmokeTrailAttrib.fStartColorUnitIntensityMax = 0.9f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMin = 0.2f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMax = 0.6f;

	m_SmokeTrailAttrib.fColorUnitSliderSpeedMin = 3.5f;
	m_SmokeTrailAttrib.fColorUnitSliderSpeedMax = 4.5f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMin = 0.0f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMax = 0.0f;
#endif
	 

	//set up the stomp cam data
	m_vecStompCameraPos.Set( _STOMPCAM_POSX, _STOMPCAM_LOOKATY, _STOMPCAM_POSZ );
	m_vecStompCameraLookat.Set( _STOMPCAM_LOOKATX, _STOMPCAM_LOOKATY, _STOMPCAM_LOOKATZ );

	return TRUE;
}


BOOL CBotTitan::ClassHierarchyLoadSharedResources( void ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( m_nBotClassClientCount != 0xffffffff );

	m_nBotClassClientCount++;

	if( !CBot::ClassHierarchyLoadSharedResources() ) {
		// Bail now since parent class has already called
		// ClassHierarchyUnloadSharedResources() and decremented
		// our client counter...
		return FALSE;
	}

	if( m_nBotClassClientCount > 1 ) {
		// resources already loaded
		return TRUE;
	}

	FResFrame_t ResFrame;
	ResFrame = fres_GetFrame();

	// SOUNDS
	// Load the sound effects bank for this bot...
	if( !fresload_Load( FSNDFX_RESTYPE, BotTitan_pszSoundEffectBank) ) {
		DEVPRINTF( "CBotTitan::ClassHierarchyLoadSharedResources(): Could not load sound effect bank '%s'\n", BotTitan_pszSoundEffectBank);
	}

	// load the heavy bots too, for servo & power up/down sounds
	if( !fresload_Load( FSNDFX_RESTYPE, BotHeavy_pszSoundEffectBank) ) {
		DEVPRINTF( "CBotTitan::ClassHierarchyLoadSharedResources(): Could not load sound effect bank '%s'\n", BotHeavy_pszSoundEffectBank);
	}

	// and the weapons bank for the rocket launcher
	if( !fresload_Load( FSNDFX_RESTYPE, Weapons_pszSoundEffectBank) ) {
		DEVPRINTF( "CBotTitan::ClassHierarchyLoadSharedResources(): Could not load sound effect bank '%s'\n", Weapons_pszSoundEffectBank);
	}

	if( !ReadBotInfoFile( m_aGameDataMap, _BOTINFO_FILENAME ) ) {
		goto _ExitWithError;
	}

	if( !_BuildAnimStackDef() ) {
		goto _ExitWithError;
	}

	// Load mesh resource...
	m_pTitanMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, _apszTitanMeshFilename);
	if( m_pTitanMesh == NULL ) {
		DEVPRINTF( "CBotTitan::ClassHierarchyBuild(): Could not find mesh '%s'\n", _apszTitanMeshFilename );
		goto _ExitWithError;
	}

	// load texture for rocket smoke
	m_SmokeTrailAttrib.pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, m_BotInfo_Titan.pszRocketSmokeTex );
	if( m_SmokeTrailAttrib.pTexDef == NULL ) {
		DEVPRINTF( "CBotTitan::ClassHierarchyLoadSharedResources(): Could not load smoke trail texture.\n" );
		return FALSE;
	}

	////init our particle def handles
	//m_hSparksPartDef = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, m_BotInfo_Titan.pszParticleSparks );
	//if( m_hSparksPartDef == FPARTICLE_INVALID_HANDLE ) {
	//	DEVPRINTF( "CBotTitan::ClassHierarchyLoadSharedResources(): Couldn't load sparks particle %s\n", m_BotInfo_Titan.pszParticleSparks );
	//	goto _ExitWithError;
	//}

	m_hDebrisExplosion = CExplosion2::GetExplosionGroup( m_BotInfo_Titan.pszDebrisPileMeshExplosion );
	if( m_hDebrisExplosion == FEXPLOSION_INVALID_HANDLE ) {
		DEVPRINTF( "CBotTitan::ClassHierarchyLoadSharedResources():  Error getting explosion group %s\n", m_BotInfo_Titan.pszDebrisPileMeshExplosion );
		goto _ExitWithError;
	}

	m_hRocketExplosionImpact = CExplosion2::GetExplosionGroup( m_BotInfo_Titan.pszRocketExplosionImpact );
	if( m_hRocketExplosionImpact == FEXPLOSION_INVALID_HANDLE ) {
		DEVPRINTF( "CBotTitan::ClassHierarchyLoadSharedResources():  Error getting explosion group %s\n", m_BotInfo_Titan.pszRocketExplosionImpact );
		goto _ExitWithError;
	}

	m_hRocketExplosionFire = CExplosion2::GetExplosionGroup( m_BotInfo_Titan.pszRocketExplosionFire );
	if( m_hRocketExplosionFire == FEXPLOSION_INVALID_HANDLE ) {
		DEVPRINTF( "CBotTitan::ClassHierarchyLoadSharedResources():  Error getting explosion group %s\n", m_BotInfo_Titan.pszRocketExplosionFire );
		goto _ExitWithError;
	}

	//m_pRocketDamageProfile	 = CDamage::FindDamageProfile( m_BotInfo_Titan.pszRocketDamageProfile );
	//m_pFootstepDamageProfile = CDamage::FindDamageProfile( m_BotInfo_Titan.pszFootstepDamageProfile );

	// Load corona texture...
	m_CoronaTexInst.SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, "TF_1corona3" ) );

	if( !fgamedata_ReadFileUsingMap( m_aSoundDataMap, _BOTINFO_FILENAME ) ) {
		return FALSE;
	}

	return TRUE;

	// Error:
_ExitWithError:
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CBotTitan::ClassHierarchyUnloadSharedResources( void ) {
	FASSERT( m_nBotClassClientCount > 0 );

	m_nBotClassClientCount--;

	if( m_nBotClassClientCount > 0 ) {
		return;
	}

	m_hDebrisExplosion = FEXPLOSION_INVALID_HANDLE;
	m_hRocketExplosionImpact = FEXPLOSION_INVALID_HANDLE;
	m_hRocketExplosionFire = FEXPLOSION_INVALID_HANDLE;

	m_AnimStackDef.Destroy();

	m_pTitanMesh = NULL;

	m_SmokeTrailAttrib.pTexDef = NULL;
	
	CBot::ClassHierarchyUnloadSharedResources();
}

void CBotTitan::UninitSystem( void ) {
	if( m_bSystemInitialized ) {
		m_AnimStackDef.Destroy();
		m_bSystemInitialized = FALSE;
	}
}


CBotTitan::CBotTitan() : CBot() {
	m_pInventory	= NULL;
	m_pWorldMesh	= NULL;
	m_pShield		= NULL;
}


CBotTitan::~CBotTitan() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}



BOOL CBotTitan::Create( s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName, BOOL bInstallShield ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	if( !ClassHierarchyLoadSharedResources() ) {
		// Failure! (resources have already been cleaned up, so we can bail now)
		return FALSE;
	}


	// Get pointer to the leaf class's builder object...
	CBotTitanBuilder *pBuilder = (CBotTitanBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_BotTitanBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOTTITAN );
	}

	// Set our builder parameters...

	pBuilder->m_bEnableShield = bInstallShield;

	// Create an entity...
	return CBot::Create( &m_BotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}


void CBotTitan::_ClearDataMembers( void ) {
	// Init data members...

	m_fCollCylinderRadius_WS = 6.0f;
	m_fCollCylinderHeight_WS = 12.0f;
	m_fStoopMinHeight = _STOOP_MINHEIGHT;
	m_fStoopMaxHeight = m_fCollCylinderHeight_WS;
	m_bStooping = FALSE;

	m_pBotInfo_Gen		= &m_BotInfo_Gen;
	m_pBotInfo_MountAim	= &m_BotInfo_MountAim;
	m_pBotInfo_Walk		= &m_BotInfo_Walk;
	m_pBotInfo_Jump		= &m_BotInfo_Jump;
	m_pBotInfo_Weapon	= &m_BotInfo_Weapon;

	m_fGravity			= m_pBotInfo_Gen->fGravity;

	m_fMaxFlatSurfaceSpeed_WS	= m_pBotInfo_Walk->fMaxXlatVelocity * m_fRunMultiplier;

	m_fMountPitchMax_WS = m_pBotInfo_MountAim->fMountPitchDownLimit;
	m_fMountPitchMin_WS = m_pBotInfo_MountAim->fMountPitchUpLimit;

	m_hPlantTexLayer	= NULL;
	m_hLightTexLayer	= NULL;

	m_anAnimStackIndex[ASI_STAND]				= ANIMTAP_STAND;
	m_anAnimStackIndex[ASI_STAND_ALERT]			= -1;
	m_anAnimStackIndex[ASI_WALK]				= ANIMTAP_WALK;
	m_anAnimStackIndex[ASI_WALK_ALERT]			= -1;
	m_anAnimStackIndex[ASI_RUN]					= ANIMTAP_RUN;
	m_anAnimStackIndex[ASI_RUN_PANIC]			= -1;
	m_anAnimStackIndex[ASI_FALL]				= ANIMTAP_JUMP_FLY;
	m_anAnimStackIndex[ASI_RC_TETHERED]			= ANIMTAP_RC_TETHERED;
	m_anAnimStackIndex[ASI_RC_POWER_DOWN]		= ANIMTAP_RC_POWER_DOWN;
	m_anAnimStackIndex[ASI_RC_POWER_UP]			= ANIMTAP_RC_POWER_UP;
	m_anAnimStackIndex[ASI_STOOP]				= ANIMTAP_STOOP_SUMMER;
	m_anAnimStackIndex[ASI_STAND_LIMP_LEFT]		= ANIMTAP_STAND_LIMP_LEFT;
	m_anAnimStackIndex[ASI_STAND_LIMP_RIGHT]	= ANIMTAP_STAND_LIMP_RIGHT;
	m_anAnimStackIndex[ASI_LIMP_LEFT]			= ANIMTAP_LIMP_LEFT;
	m_anAnimStackIndex[ASI_LIMP_RIGHT]			= ANIMTAP_LIMP_RIGHT;

	//m_pnEnableBoneNameIndexTableForSummer_Normal		= m_anEnableBoneNameIndexTableForSummer_Normal;
	m_pnEnableBoneNameIndexTableForSummer_Normal		= m_anEnableBoneNameIndexTableForSummer_TetherShock;
	m_pnEnableBoneNameIndexTableForSummer_TetherShock	= m_anEnableBoneNameIndexTableForSummer_TetherShock;

	//m_bAimGuns	= FALSE;
	//m_fGunAimTimer	= 1.0f;

	m_fLastAimAngle_LElbow		= 0.0f;
	m_fLastAimAngle_RElbow		= 0.0f;
	m_fLastAimAngle_LArm		= 0.0f;
	m_fLastAimAngle_RArm		= 0.0f;

	m_bDoubleJump				= FALSE;
	m_bStooping					= FALSE;
	m_fStompCameraTransition	= 0.0f;

	//rocket stuff
	m_fRocketReaction			= 0.0f;
	m_fRocketReactionTarget		= 0.0f;
	m_bRocketReactionOn			= FALSE;	

	m_fRocketLoadTimer			= 1.0f;
	m_bRocketTriggerDown		= FALSE;
	m_uRocketsToFire			= 0;	
	m_fRocketFireTimer			= 0.0f;
	m_fRocketAimTimer			= 1.0f;
	m_bFireRocketThisFrame		= FALSE;

	m_pLight					= NULL;
	m_bLightEnabled				= FALSE;

	m_fPowerPlantUnitCycle		= 0.0f;
	m_fPowerDraw				= 0.0f;
	m_fShieldSoundTimer			= 0.0f;

	m_pReactorEmitter			= NULL;

	m_fShieldHealth				= 0.0f;
	m_fChaingunHeat				= 0.0f;
	
	m_pMoveIdentifier			= &m_fStompCameraTransition;
	m_bShieldSuspended			= FALSE;

	fforce_NullHandle( &m_hForce );
}


//Class Hierarchy stuff


void CBotTitan::ClassHierarchyDestroy( void ) {
	// Delete the items that we had instantiated for us...

#if _STEVETEST
	fdelete( _pMeshEntity );
	_pMeshEntity = NULL;
#endif

	WeaponsDestroy();

	m_Anim.Destroy();

	fforce_Kill( &m_hForce );

	fdelete( m_pWorldMesh );
	m_pWorldMesh = NULL;

	if( m_pShield != NULL ) {
		fdelete( m_pShield );
		m_pShield = NULL;
	}

	if( m_pLight != NULL ) {
		fdelete( m_pLight );
		m_pLight = NULL;
	}


	CBot::ClassHierarchyDestroy();
}



BOOL CBotTitan::ClassHierarchyBuild( void ) {
	FMeshInit_t MeshInit;
//	u32 i;
	s32 nBoneIndex;
	cchar * pszMeshFilename = NULL;

	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CBotTitanBuilder *pBuilder = (CBotTitanBuilder *)GetLeafClassBuilder();

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

	// Create world mesh...
	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL ) {
		DEVPRINTF( "CBotTitan::ClassHierarchyBuild(): Not enough memory to create CFWorldMesh.\n" );
		goto _ExitWithError;
	}

	// Init the world mesh...
	MeshInit.pMesh		= m_pTitanMesh;
	MeshInit.nFlags		= 0;
	MeshInit.fCullDist  = FMATH_MAX_FLOAT;
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

	// Create debris world mesh...
	if( m_pBotInfo_Gen->pDebrisPileMesh ) {
		m_pNoLegsDebrisPileWorldMesh = fnew CFWorldMesh;
		if( m_pNoLegsDebrisPileWorldMesh == NULL ) {
			DEVPRINTF( "CBotTitan::ClassHierarchyBuild(): Not enough memory to allocate m_pNoLegsDebrisPileWorldMesh.\n" );
			goto _ExitWithError;
		}

		FMeshInit_t MeshInit;
		MeshInit.fCullDist = 300.0f;
		MeshInit.Mtx.Identity();
		MeshInit.nFlags = FMESHINST_FLAG_NOCOLLIDE;
		MeshInit.pMesh = m_pBotInfo_Gen->pDebrisPileMesh;
		m_pNoLegsDebrisPileWorldMesh->Init( &MeshInit );
		m_pNoLegsDebrisPileWorldMesh->SetCollisionFlag( FALSE );
		m_pNoLegsDebrisPileWorldMesh->SetLineOfSightFlag( FALSE );
		m_pNoLegsDebrisPileWorldMesh->RemoveFromWorld();
	}

	// get tex layer handle for powerplant

	m_hPlantTexLayer = m_pWorldMesh->GetTexLayerHandle( _TEXLAYER_POWERPLANT );
	if( m_hPlantTexLayer == FMESH_TEXLAYERHANDLE_INVALID ) {
		DEVPRINTF( "CBotTitan::ClassHierarchyBuild():  Error getting tex layer handle for powerplant, ID %d\n", _TEXLAYER_POWERPLANT );
	} else {
		m_pWorldMesh->AnimTC_ApplyToShader( m_hPlantTexLayer, TRUE );
		m_pWorldMesh->AnimTC_AnimateScroll( m_hPlantTexLayer, TRUE );
		m_pWorldMesh->AnimTC_SetScrollRate( m_hPlantTexLayer, CFVec2::m_UnitAxisY );
		m_pWorldMesh->LayerAlpha_Set( m_hPlantTexLayer, 1.0f );
	}

	m_hLightTexLayer = m_pWorldMesh->GetTexLayerHandle( _TEXLAYER_LIGHT );
	if( m_hLightTexLayer == FMESH_TEXLAYERHANDLE_INVALID ) {
		DEVPRINTF( "CBotTitan::ClassHierarchyBuild():  Error getting tex layer handle for light, ID %d\n", _TEXLAYER_LIGHT );
	} else {
		m_pWorldMesh->TexFlip_AnimateFlip( m_hLightTexLayer, FALSE );
	}

	m_hRocketTexLayer = m_pWorldMesh->GetTexLayerHandle( _TEXLAYER_ROCKET );
	if( m_hRocketTexLayer < 0 ) {
		DEVPRINTF( "CBotTitan::ClassHierarchyBuild():  Error getting tex layer handle for powerplant, ID %d\n", _TEXLAYER_POWERPLANT );
		goto _ExitWithError;	
	}
	m_pWorldMesh->AnimTC_AnimateScroll( m_hRocketTexLayer, FALSE );


//	// get tex layer handles for rocket launcher indicator lights
//	for( i=0; i<ROCKET_NUM_CHAMBERS; i++ ) {
//		m_ahRocketTexLayers[i] = m_pWorldMesh->GetTexLayerHandle( m_auRocketTexLayerIDs[i] );
//		if( m_ahRocketTexLayers[i] < 0 ) {
//			DEVPRINTF( "CBotTitan::ClassHierarchyBuild():  Error getting tex layer handle for powerplant, ID %d\n", _TEXLAYER_POWERPLANT );
////			goto _ExitWithError;	
//		}
//		m_pWorldMesh->AnimTC_AnimateScroll( m_ahRocketTexLayers[i], FALSE );
//	}

	// Build our animation stack and load animations...
	if( !m_Anim.Create( &m_AnimStackDef, m_pWorldMesh ) ) {
		DEVPRINTF( "CBotTitan::ClassHierarchyBuild(): Trouble creating m_Anim.\n" );
		goto _ExitWithError;
	}

	SetControlValue( ANIMCONTROL_STAND, 1.0f );
	UpdateUnitTime( ANIMTAP_STAND, fmath_RandomFloat() );

	m_nBoneIndexGroin			= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_GROIN] );
	m_nBoneIndexTorso			= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_TORSO] );
	m_nBoneIndexRArm			= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_R_ARM_LOWER] );
	m_nBoneIndexRElbow			= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_R_ELBOW] );
	m_nBoneIndexLArm			= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_L_ARM_LOWER] );
	m_nBoneIndexLElbow			= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_L_ELBOW] );
	m_nBoneIndexRocketLauncher	= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ROCKETLAUNCHER] ); 
	m_nBoneIndexRocketFire		= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_SECONDARY_FIRE] ); 
	m_nBoneIndexLight			= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_SPOTLIGHT] );
	m_nBoneIndexRightToe		= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_R_TOE] );
	m_nBoneIndexRightHeel		= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_R_HEEL] );
	m_nBoneIndexLeftToe			= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_L_TOE] );
	m_nBoneIndexLeftHeel		= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_L_HEEL] );
	m_nBoneIndexReactor			= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_REACTOR] );

	m_Anim.m_pAnimCombiner->SetBoneCallback( &_AnimBoneCallback );

	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_GROIN] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_TORSO] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_HEAD] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_R_ARM_LOWER] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_R_ELBOW] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_L_ARM_LOWER] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_L_ELBOW] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ROCKETLAUNCHER] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_SPOTLIGHT] );

	EnableControlSmoothing( ANIMTAP_JUMP_LAUNCH );
	EnableControlSmoothing( ANIMTAP_JUMP_FLY );
	EnableControlSmoothing( ANIMTAP_JUMP_LAND_LOWER );
	EnableControlSmoothing( ANIMTAP_JUMP_LAND_UPPER );
	EnableControlSmoothing( ANIMTAP_JUMP2_FLY );
	EnableControlSmoothing( ANIMTAP_JUMP2_LAND );

	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_L_HEEL] );
	if( nBoneIndex >= 0 ) {
		m_pLeftFootDustPos_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;

		nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_R_HEEL] );
		if( nBoneIndex >= 0 ) {
			m_pRightFootDustPos_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
		} else {
			m_pLeftFootDustPos_WS = NULL;
		}
	}

	// set up the manual animation for our aiming summer
	if( !m_AnimManFrameAim.Create( m_AnimStackDef.m_nBoneCount, m_AnimStackDef.m_apszBoneNameTable ) ) {
		DEVPRINTF( "CBotTitan::ClassHierarchyCreate(): Could not create aim animation summer.\n" );
		goto _ExitWithError;
	}
	AttachAnim( ANIMTAP_AIM_SUMMER, &m_AnimManFrameAim );
	m_AnimManFrameAim.Reset();
	UpdateBoneMask( ANIMTAP_AIM_SUMMER, m_aBoneEnableIndices_AimSummer, TRUE );

	if( !m_AnimManMtxStoop.Create( _STOOP_BONE_COUNT, m_papszStoopSummerBones ) ) {// m_AnimStackDef.m_nBoneCount, m_AnimStackDef.m_apszBoneNameTable ) ) {
		DEVPRINTF( "CBotTitan::ClassHierarchyBuild(): Could not create stoop animation summer.\n" );
		goto _ExitWithError;
	}
	AttachAnim( ANIMTAP_STOOP_SUMMER, &m_AnimManMtxStoop );
	m_AnimManMtxStoop.Reset();

	if( !_InitStoopAnimation() ) {
		DEVPRINTF( "CBotTitan::ClassHierarchyBuild(): Could not initialize stoop animation summer.\n" );
		goto _ExitWithError;
	}

	SetMaxHealth();

	if( m_nPossessionPlayerIndex >= 0 ) {
		Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.SetNormOrigin( 0.0f, 0.23f );
	}

	// Find approx eye point...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CBotTitan::ClassHierarchyBuild(): Could not locate approx eye point bone '%s'.\n", m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
		m_pApproxEyePoint_WS = &m_MtxToWorld.m_vPos;
	} else {
		m_pApproxEyePoint_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
	}

	// Find gaze direction...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_HEAD ] );
	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CBotTitan::ClassHierarchyBuild(): Could not locate gaze bone '%s'.\n", m_apszBoneNameTable[ BONE_HEAD ] );
		m_pGazeDir_WS = &m_MtxToWorld.m_vFront;
	} else {
		m_pGazeDir_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vFront;
	}

	m_pAISteerMtx = &m_MtxToWorld;

	//Initialize matrix palette (this is important for attached entities)...
	AtRestMatrixPalette();

	// set up weapons
	if( !WeaponsCreate( 0, CWeapon::WEAPON_TYPE_CHAINGUN, 1, _NPC_TITAN_CHAINGUN_UPGRADE_LEVEL ) ) {
		DEVPRINTF( "CBotTitan::ClassHierarchyBuild(): Could not create chainguns\n" );
		goto _ExitWithError;
	}

	FASSERT( m_apWeapon[0] );
	FASSERT( m_aapDupWeapons[0] );

	// Make weapons pass along damage to their appropriate arms...
	m_apWeapon[0]->SetBotDamageBoneIndex( m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_R_ARM_UPPER] ) );
	m_aapDupWeapons[0][0]->SetBotDamageBoneIndex( m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_L_ARM_UPPER] ) );

	//this gets done in addtoworld
//	m_apWeapon[0]->Attach_UnitMtxToParent_PS( this, m_apszBoneNameTable[BONE_R_ARM_LOWER] );
//	m_aapDupWeapons[0][0]->Attach_UnitMtxToParent_PS( this, m_apszBoneNameTable[BONE_L_ARM_LOWER] );

	((CWeaponChaingun*)m_aapDupWeapons[0][0])->SetRotation( FALSE );

	((CWeaponChaingun*)m_apWeapon[0])->OverrideLights( TRUE, TRUE, &m_vChaingunMidpoint_WS );
	((CWeaponChaingun*)m_aapDupWeapons[0][0])->OverrideLights( FALSE );

	((CWeaponChaingun*)m_apWeapon[0])->OverrideSounds( TRUE, TRUE, &m_vChaingunMidpoint_WS );
	((CWeaponChaingun*)m_aapDupWeapons[0][0])->OverrideSounds( FALSE );

	if( !m_pPartMgr->Create( this, &m_pPartPool, _BOTPART_FILENAME, PART_INSTANCE_COUNT_PER_TYPE, LIMB_TYPE_COUNT ) ) {
		goto _ExitWithError;
	}

	// Make invincible limbs...
	MakeLimbInvincible( LIMB_CODE_HEAD, LIMB_TYPE_HEAD, pBuilder );
	MakeLimbInvincible( LIMB_CODE_ARMS, LIMB_TYPE_LEFT_ARM, pBuilder );
	MakeLimbInvincible( LIMB_CODE_ARMS, LIMB_TYPE_RIGHT_ARM, pBuilder );
	MakeLimbInvincible( LIMB_CODE_ROCKET_LAUNCHER, LIMB_TYPE_ROCKET_LAUNCHER, pBuilder );
	MakeLimbInvincible( LIMB_CODE_SPOTLIGHT, LIMB_TYPE_SPOTLIGHT, pBuilder );
	MakeLimbInvincible( LIMB_CODE_MISC, LIMB_TYPE_RIBS1, pBuilder );
	MakeLimbInvincible( LIMB_CODE_MISC, LIMB_TYPE_RIBS2, pBuilder );
	MakeLimbInvincible( LIMB_CODE_LEGS, LIMB_TYPE_LEFT_LEG, pBuilder );
	MakeLimbInvincible( LIMB_CODE_LEGS, LIMB_TYPE_RIGHT_LEG, pBuilder );

	// Set up data port stuff...
	DataPort_SetupTetherShockInfo();

	fforce_NullHandle( &m_hForce );

#if !_DEBUG_DISABLE_SHIELD

	if( pBuilder->m_bEnableShield ) {
		m_pShield = fnew CEShield;
		if( !m_pShield->Create( "TitanShield" ) ) {
			DEVPRINTF( "CBotTitan::ClassHierarchyBuild():  Error creating shield\n" );
			goto _ExitWithError;
		} else {
			ShieldInit_t shieldInit;

			shieldInit.uShieldFlags			= 0;
			shieldInit.fShieldRechargeTime	= m_BotInfo_Titan.fShieldRechargeTime;
			shieldInit.fShieldRechargeDelay	= m_BotInfo_Titan.fShieldRechargeDelay;
	//		shieldInit.fShieldScale			= m_BotInfo_Titan.fShieldScale;
			shieldInit.fShieldScale			= 1.0f;
			shieldInit.pArmorProfile		= CDamage::FindArmorProfile( m_BotInfo_Titan.pszShieldArmorProfile );

			m_pShield->Init( this, &shieldInit );

			m_pShield->RemoveFromWorld();
		}
	}
#endif

	m_pLight = fnew CMeshEntity();
	if( m_pLight == NULL ) {
		DEVPRINTF( "CBotTitan::ClassHierarchyBuild(): Could not allocate spotlight\n" );
		goto _ExitWithError;
	}

	if( !m_pLight->Create( m_BotInfo_Titan.pszSpotlightMesh ) ) {
		DEVPRINTF( "CBotTitan::ClassHierarchyBuild(): Could not create spotlight\n" );
		goto _ExitWithError;
	}
	m_pLight->SetMeshInstFlags( FMESHINST_FLAG_POSTER_Y );

	// Create tag points...
	if( !TagPoint_CreateFromBoneArray( m_anTagPointBoneNameIndexArray, m_apszBoneNameTable ) ) {
		goto _ExitWithError;
	}

	_EnableLight( pBuilder->m_bEnableLight );

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}



void CBotTitan::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

//	m_uBotDeathFlags |= BOTDEATHFLAG_PLAYDEATHANIM;
//	m_uBotDeathFlags |= BOTDEATHFLAG_AUTOPERSISTAFTERDEATH;

	CBot::ClassHierarchyAddToWorld();
	m_pWorldMesh->UpdateTracker();

	WeaponsAddToWorld( 0 );

	if( m_apWeapon[0] ) {
		m_apWeapon[0]->Attach_UnitMtxToParent_PS( this, m_apszBoneNameTable[BONE_R_ARM_LOWER] );
	}

	if( m_aapDupWeapons[0][0] ) {
		m_aapDupWeapons[0][0]->Attach_UnitMtxToParent_PS( this, m_apszBoneNameTable[BONE_L_ARM_LOWER] );
	}

	//m_fGunAimTimer			 = 1.0f;	//not aiming guns

	if( m_pShield ) {
		m_pShield->AddToWorld();
		m_pShield->Attach_UnitMtxToParent_PS( this, m_apszBoneNameTable[BONE_GROIN] );
		m_pShield->EnableShield( TRUE );
	}

	// enable the power plant sound
	if( m_pReactorEmitter == NULL ) {
		CFMtx43A *pReactorMtx = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexReactor];
		m_pReactorEmitter = FSNDFX_ALLOCNPLAY3D( m_BotInfo_TitanSounds.hSndReactor, &pReactorMtx->m_vPos, 50.0f, 1.0f, m_BotInfo_Titan.fReactorSoundLevel, FAudio_EmitterDefaultPriorityLevel, TRUE );
	}

	FASSERT( m_pLight );
	if( m_bLightEnabled ) {
		m_pLight->AddToWorld();
	}
}


void CBotTitan::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	m_pWorldMesh->RemoveFromWorld();
	CBot::ClassHierarchyRemoveFromWorld();

	WeaponsRemoveFromWorld( 0 );

	fforce_Kill( &m_hForce );

	if( m_pShield ) {
		m_pShield->RemoveFromWorld();
	}

	FASSERT( m_pLight != NULL );
	m_pLight->RemoveFromWorld();

	//	m_pWorldMesh->TexFlip_SetFlipPage( m_hLightTexLayer, _LIGHT_OFF );
	
	if( m_pReactorEmitter ) {
		m_pReactorEmitter->Destroy();
		m_pReactorEmitter = NULL;
	}

	// reset the chainguns so they share lights and sounds.
	// note, if the bot part system doesn't reset in the bot layer, this shouldn't happen
	((CWeaponChaingun*)m_apWeapon[0])->OverrideLights( TRUE, TRUE, &m_vChaingunMidpoint_WS );
	((CWeaponChaingun*)m_aapDupWeapons[0][0])->OverrideLights( FALSE );

	((CWeaponChaingun*)m_apWeapon[0])->OverrideSounds( TRUE, TRUE, &m_vChaingunMidpoint_WS );
	((CWeaponChaingun*)m_aapDupWeapons[0][0])->OverrideSounds( FALSE );

}


void CBotTitan::HandleTargeting( void ) {
	BOOL bAiming;
	
	CBot::HandleTargeting();

	// if human controlled, aim always, otherwise, check to make sure that 1:  AI wants us to aim and 2:  CBot::HandleTargetting() is satisfied 
	// that our target is in a reasonable position.
    if( m_nPossessionPlayerIndex >= 0 ) {
		bAiming = TRUE;

		// keep the target point at least min dist away
		if( m_MountPos_WS.DistSq( m_TargetedPoint_WS ) <  10.0f * 10.0f ) {
			m_TargetedPoint_WS.Sub( m_MountPos_WS );
			m_TargetedPoint_WS.Unitize().Mul( 10.0f );
			m_TargetedPoint_WS.Add( m_MountPos_WS );
		}
	} else {
		if( (m_nControlsBot_Flags & CBotControl::FLAG_AIM_AT_TARGET_POINT) && 
			(m_nBotFlags & BOTFLAG_TARGET_LOCKED) && 
			(m_MountPos_WS.DistSq( m_TargetedPoint_WS ) > 10.0f * 10.0f ) ) {
			bAiming = TRUE;
		} else {
			bAiming = FALSE;
			if( m_vLastValidTargetPt_WS.MagSq() == 0.0f ) {
				m_vLastValidTargetPt_WS.Sub( m_TargetedPoint_WS, m_MountPos_WS );
			}

			if( m_fUnitAim > 0.0f ) {
				m_TargetedPoint_WS.Add( m_MountPos_WS, m_vLastValidTargetPt_WS );
			}
		}
	}

	// move in/out of aiming state as required
	if( bAiming ) {
		m_fUnitAim += _AIM_OO_ON_TIME * FLoop_fPreviousLoopSecs;
		m_vLastValidTargetPt_WS.Zero();
	} else {
		m_fUnitAim -= _AIM_OO_OFF_TIME * FLoop_fPreviousLoopSecs;
	}
	FMATH_CLAMP_UNIT_FLOAT( m_fUnitAim );

	// if not 100% aimed, we are not ready to fire, clear the flag
	if( m_nBotFlags & BOTFLAG_TARGET_LOCKED ) {
		if( m_fUnitAim < 1.0f ) {
			FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_TARGET_LOCKED );
		}
	}
}



CEntityBuilder *CBotTitan::GetLeafClassBuilder( void ) {
	return &_BotTitanBuilder;
}


BOOL CBotTitan::ClassHierarchyBuilt( void ) {
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


void CBotTitan::UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup ) {
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



void CBotTitan::ClassHierarchyWork() {
	CFVec3A TempVec3A;

	FASSERT( m_bSystemInitialized );

	CBot::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}

	Power_Work();
	DataPort_Work();

	if( DataPort_IsBeingShocked() ) {
		_UpdateMatrices();
		WeaponsWork(0);

		if( m_pNoLegsDebrisPileWorldMesh && m_pNoLegsDebrisPileWorldMesh->IsAddedToWorld() ) {
			_PositionDebrisPile();
		}

		return;
	}

	ParseControls();

	// Save a copy of the previous frame's info...
	m_MountPrevPos_WS	= m_MountPos_WS;
	m_nPrevState		= m_nState;

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
	m_pBotToSquishThisFrame = NULL;
		HandleCollision();
	if( m_pBotToSquishThisFrame ) {
		m_pBotToSquishThisFrame->Die();
	}
	PROTRACK_ENDBLOCK();//"Coll"

	// Move and animate our bot...
	switch( m_nState ) {
	case STATE_GROUND:
		PROTRACK_BEGINBLOCK("GroundXlat");
			HandleGroundTranslation();
			_HandleJumping();
//			HandleHopRollStartleGeneric();		// titans don't hop
		PROTRACK_ENDBLOCK();//"GroundXlat");

		PROTRACK_BEGINBLOCK("GroundAnim");
			HandleGroundAnimations();
			HandleStooping();
		PROTRACK_ENDBLOCK();//"GroundAnim");

		// if we're on the ground, make sure the shield is enabled
		if( m_bShieldSuspended ) {
			SuspendShield( FALSE );
		}
		break;

	case STATE_AIR:
		PROTRACK_BEGINBLOCK("AirXlat");
			HandleAirTranslation();
//			_HandleCableTranslation();
//			_HandleCableAndDoubleJump();
			_CheckForDoubleJump();
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
//		_HandleCableAnimations();
		HandleHipsAnimation();
	PROTRACK_ENDBLOCK();//"CommonAnim");

//	static CFVec3A _vecTargetPos;

	PROTRACK_BEGINBLOCK("Weapon");
		HandleTargeting();
		_HandleWeaponFiring();
		_HandleFiringAnimations();
		_HandleAimingAnimations();

		//rockets
		_HandleRocketWork();
//		_HandleWeaponAnimations();
	PROTRACK_ENDBLOCK();//"WeaponAnim");

	_UpdateMatrices();

	PROTRACK_BEGINBLOCK("WeaponWork");
		_HandleWeaponWork();
	PROTRACK_ENDBLOCK();//"WeaponWork");

	if( m_pShield ) {
		if( (m_pPartMgr->GetLimbState( LIMB_TYPE_RIBS1 ) == CBotPartMgr::LIMB_STATE_REMOVED) &&
			(m_pPartMgr->GetLimbState( LIMB_TYPE_RIBS2 ) == CBotPartMgr::LIMB_STATE_REMOVED) ) {
			m_pShield->EnableShield( FALSE );
		}
		m_pShield->Work();
	}

	// update our power plant
	_HandlePowerPlant();
	

	if (IsDeadOrDying()) {
		DeathWork();
	}

	// see whether we should limit our top speed
	if( m_bFiringGuns || m_bStooping ) {
		m_fUnitTopSpeedLimiter = m_BotInfo_Titan.fUnitTopSpeedWhileFiring;
	} else {
		m_fUnitTopSpeedLimiter = 1.0f;
	}

	// handle the spotlight
	if( m_bLightEnabled ) {
		FASSERT( m_pLight != NULL );
		//decide whether or not the light should be on
		if( m_fSpeed_WS > 0.01f || (m_nPossessionPlayerIndex >= 0) ) {
			m_fLightTimer = m_BotInfo_Titan.fLightInactivityTime;
		} else {
			m_fLightTimer -= FLoop_fPreviousLoopSecs;
		}

		if( (m_uLifeCycleState == BOTLIFECYCLE_DEAD) && (m_fLightTimer > 0.0f) ) {
			m_fLightTimer = 0.0f;
		}

		if( (m_fLightTimer < 0.0f) && (m_fLightTimer > -0.5f) ) {
			m_pLight->GetMeshInst()->SetMeshAlpha( fmath_UnitLinearToSCurve( 1.0f + m_fLightTimer * 2.0f ) );
		} else if( m_fLightTimer <= -1.0f ) {
			_EnableLight( FALSE );
		} else {
			if( !m_pLight->IsInWorld() ) {
				_EnableLight( TRUE );
			}
		}
	}
	
	_CheckServoSounds();

	// used for the shield splash sound
	m_fShieldSoundTimer -= FLoop_fPreviousLoopSecs;

	// grab these so we can display their values on the HUD
	if( m_nPossessionPlayerIndex > -1 ) {
		if( m_pShield ) {
			m_fShieldHealth = m_pShield->NormHealth();
		} else {
			m_fShieldHealth = 0.0f;
		}

		if( m_apWeapon[0] ) {
            m_fChaingunHeat = ((CWeaponChaingun*)m_apWeapon[0])->GetUnitHeat();
		} else {
			m_fChaingunHeat = 0.0f;
		}
	}

	if( m_pNoLegsDebrisPileWorldMesh && m_pNoLegsDebrisPileWorldMesh->IsAddedToWorld() ) {
		_PositionDebrisPile();
	}


	if( m_bDoubleJump && (m_nJumpState == BOTJUMPSTATE_NONE) ) {
		m_bDoubleJump = FALSE;
		SetCameraOverride( FALSE, 1.5f );
	}

////TEST STUFF FOR LIMPING
//#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
//	if( m_nPossessionPlayerIndex >= 0 ) {
//		if( m_fControls_Fire1 > 0.01f ) {
//			GiveBotLimp( FALSE );
//		} 
//
//		if( m_fControls_Fire2 > 0.01f ) {
//			GiveBotLimp( TRUE );
//		} 
//	}
//#endif

//	//TEST STUFF FOR SHIELD
//#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
//	if( m_fControls_Fire1 > 0.01f ) {
//		m_pShield->DropShield();
//	}
//	
//	if( m_fControls_Fire2 > 0.01f ) {
//		m_pShield->RechargeNow();
//	}
//#endif

////TEST STUFF FOR STOOPING
//#if 0 //SAS_ACTIVE_USER == SAS_USER_ELLIOTT
//	if( m_fControls_Fire1 > 0.01f ) {
//		m_bStooping = TRUE;
//	} else {
//		m_bStooping = FALSE;
//	}
//
//	if( m_nOwnerPlayerIndex == 0 ) {
//		u32 uTmpBone = m_pWorldMesh->FindBone( "RocketCartridge" );
//		CFMtx43A *pTmpMtx = m_pWorldMesh->GetBoneMtxPalette()[uTmpBone];
//	
//		ftext_DebugPrintf( 0.2f, 0.7f, "~w1Stoop?  %d.  cyl = %0.2f, Alt = %0.2f", (m_fControls_Fire2 > 0.1f), m_fCollCylinderHeight_WS, pTmpMtx->m_vPos.y );
//	}
//#endif
}



void CBotTitan::_UpdateMatrices( void ) {
	CFMtx43A	EntityMtxToWorld;

	CFMtx43A::m_XlatRotY.SetRotationY( m_fMountYaw_WS + m_fLegsYaw_MS );
	CFMtx43A::m_XlatRotY.m_vPos = m_MountPos_WS;

	m_pWorldMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_XlatRotY );
	m_pWorldMesh->UpdateTracker();

	PROTRACK_BEGINBLOCK("ComputeMtxPal");
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
	Relocate_RotXlatFromUnitMtx_WS( &EntityMtxToWorld, TRUE, m_pMoveIdentifier );
}


void CBotTitan::_AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	FASSERT( m_pCBTitan != NULL );
	FASSERT( m_pCBTitan->TypeBits() & ENTITY_BIT_BOTTITAN );
	if( !m_pCBTitan ) {
		rNewMtx.Mul( rParentMtx, rBoneMtx );
		return;
	}

	if( m_pCBTitan->m_pPartMgr->AnimBoneCallbackFunctionHandler( nBoneIndex, rNewMtx, rParentMtx, rBoneMtx ) ) {
		return;
	} else if( nBoneIndex == (u32)(m_pCBTitan->m_nBoneIndexGroin) ) {
		// Groin
		rNewMtx.Mul( rParentMtx, rBoneMtx );
		m_GroinVecY_WS = rNewMtx.m_vUp;
	} else if( nBoneIndex == (u32)m_pCBTitan->m_nBoneIndexTorso ) {
		// Torso
		CFMtx43A WorldMtx;
		CFQuatA Quat;

		Quat.BuildQuat( m_GroinVecY_WS, -m_pCBTitan->m_fLegsYaw_MS );
		WorldMtx.Mul( rParentMtx, rBoneMtx );

		rNewMtx.m_vPos = WorldMtx.m_vPos;
		Quat.MulPoint( rNewMtx.m_vRight,	WorldMtx.m_vRight );
		Quat.MulPoint( rNewMtx.m_vUp,		WorldMtx.m_vUp );
		Quat.MulPoint( rNewMtx.m_vFront,	WorldMtx.m_vFront );
	
	} else if( nBoneIndex == (u32)m_pCBTitan->m_nBoneIndexRArm ) {
		if( m_pCBTitan->m_fUnitAim > 0.0f ) {
			_HandleArmAim( rNewMtx, rParentMtx, rBoneMtx, m_pCBTitan->m_TargetedPoint_WS, m_pCBTitan->m_fUnitAim, &m_pCBTitan->m_fLastAimAngle_RArm );
	   } else {
		   rNewMtx.Mul( rParentMtx, rBoneMtx );
	   }
	} else if( nBoneIndex == (u32)m_pCBTitan->m_nBoneIndexLArm ) {
		if( m_pCBTitan->m_fUnitAim > 0.0f ) {
			_HandleArmAim( rNewMtx, rParentMtx, rBoneMtx, m_pCBTitan->m_TargetedPoint_WS, m_pCBTitan->m_fUnitAim, &m_pCBTitan->m_fLastAimAngle_LArm );
		} else {
		   rNewMtx.Mul( rParentMtx, rBoneMtx );
		}
	} else if( nBoneIndex == (u32)m_pCBTitan->m_nBoneIndexRElbow ) {
		if( m_pCBTitan->m_fUnitAim > 0.0f ) {
			_HandleElbowAim( rNewMtx, rParentMtx, rBoneMtx, m_pCBTitan->m_TargetedPoint_WS, m_pCBTitan->m_fUnitAim, &m_pCBTitan->m_fLastAimAngle_RElbow );
		} else {
			rNewMtx.Mul( rParentMtx, rBoneMtx );
		}
	} else if( nBoneIndex == (u32)m_pCBTitan->m_nBoneIndexLElbow ) {
		if( m_pCBTitan->m_fUnitAim > 0.0f ) {
			_HandleElbowAim( rNewMtx, rParentMtx, rBoneMtx, m_pCBTitan->m_TargetedPoint_WS, m_pCBTitan->m_fUnitAim, &m_pCBTitan->m_fLastAimAngle_LElbow );
		} else {
			rNewMtx.Mul( rParentMtx, rBoneMtx );
		}
	} else if( nBoneIndex == (u32)m_pCBTitan->m_nBoneIndexLight ) {
		//aim the light the same direction as the head
		CFMtx43A::m_Temp.Mul( rParentMtx, rBoneMtx );
		rNewMtx.m_vPos = CFMtx43A::m_Temp.m_vPos;
		m_pCBTitan->m_HeadLookCurrentQuat.MulPoint( rNewMtx.m_vRight,	CFMtx43A::m_Temp.m_vRight );
		m_pCBTitan->m_HeadLookCurrentQuat.MulPoint( rNewMtx.m_vUp,		CFMtx43A::m_Temp.m_vUp );
		m_pCBTitan->m_HeadLookCurrentQuat.MulPoint( rNewMtx.m_vFront,	CFMtx43A::m_Temp.m_vFront );
	} else if( nBoneIndex == (u32)m_pCBTitan->m_nBoneIndexRocketLauncher ) {
		 m_pCBTitan->_HandleRocketLauncherAim( rNewMtx, rParentMtx, rBoneMtx );
	} else {
		// Head
		m_pCBTitan->HeadLookUpdate( rNewMtx, rParentMtx, rBoneMtx, FALSE );
	}
}



void CBotTitan::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	FASSERT( IsCreated() );
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );

	apTrackerSkipList[nTrackerSkipListCount++] = m_pWorldMesh;

	if( m_pShield ) {
		m_pShield->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}

	if( m_apWeapon[0] ) {
		m_apWeapon[0]->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}

	if( m_aapDupWeapons[0][0] ) {
		m_aapDupWeapons[0][0]->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}

	if( m_pDataPortMeshEntity ) {
		m_pDataPortMeshEntity->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}
}

const CFVec3A *CBotTitan::GetApproxEyePoint( void ) const {
	FASSERT( IsCreated() );

	return m_pApproxEyePoint_WS;
}


BOOL CBotTitan::NotifyBotCollision( CBot *pBot ) {
	FASSERT( pBot != NULL );

	if( (m_nJumpState == BOTJUMPSTATE_AIR2) ) {
		if( pBot->IsSquishy() ) {
			m_pBotToSquishThisFrame = pBot;
		}
		return FALSE;
	} else {
		return TRUE;
	}
}


void CBotTitan::_HandleJumping( void ) {
	if( m_bStooping || (m_fCollCylinderHeight_WS < m_fStoopMaxHeight) || IsLimping() ) { 
		return;
	}

	if( !m_bControlsBot_JumpVec ) {
		if( m_bControlsBot_Jump2 ) {
			m_vJumpVelocity		= m_Velocity_WS;
			m_vJumpVelocity.y	= m_pBotInfo_Jump->fVerticalVelocityJump2;
			_StartSingleJump();
			_StartSingleJump();
		} else if( m_bControls_Jump ) {
			m_vJumpVelocity		= m_Velocity_WS;
			m_vJumpVelocity.y	= m_pBotInfo_Jump->fVerticalVelocityJump1;
			_StartSingleJump();
		}
	} else {
		m_vJumpVelocity = m_ControlsBot_JumpVelocity_WS;
//		FMATH_CLAMP( m_ControlsBot_JumpVelocity_WS.y, 0.0f, m_fMaxDeltaJumpVelY_WS );
		_StartSingleJump();
	}
}


void CBotTitan::_EnterFlyMode( void ) {
	m_nJumpState	= BOTJUMPSTATE_AIR;
	SetJumping();
	m_fUnitFlyBlend = 1.0f;
	m_bDoubleJump	= FALSE;
}


void CBotTitan::_HandleLanding( void ) {
	MakeFootImpactDust( TRUE, TRUE, m_fUnitFlyBlend, &m_FeetToGroundCollImpact.UnitFaceNormal );

	ClearJumping();
	m_fGravityMultiplier	= 1.0f;
	m_bDoubleJump = FALSE;

	//if( m_pShield ) {
	//	m_pShield->EnablePushOutBots( TRUE );
	//	SuspendShield( FALSE );
	//}

	CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

	//are we doing the stomp jump?
	if( m_nJumpState == BOTJUMPSTATE_AIR2 ) {
		m_nJumpState	= BOTJUMPSTATE_LAND2;
		
		ZeroTime( ANIMTAP_JUMP2_LAND );
		SetControlValue( ANIMCONTROL_JUMP2_LAND,	1.0f );

		JustFired();  
		CFXShockwave::AddShockwave( m_MountPos_WS, 3.0f, 2.5f, ENTITY_BIT_BOT, m_BotInfo_Titan.fMaxShockwaveImpulse, m_BotInfo_Titan.fMinShockwaveImpulse );
		
		ZeroTime( ANIMTAP_WALK );
		m_fUnitWalkBlend = 0.0f;
				
		//stick
		m_Velocity_WS.x			= 0.0f;
		m_Velocity_WS.z			= 0.0f;
		VelocityHasChanged();

		//if we're player controlled, shake our camera
		if( m_nPossessionPlayerIndex >= 0 ) {
			fcamera_GetCameraByIndex( m_nPossessionPlayerIndex )->ShakeCamera( 0.25f, 0.25f );
			fforce_Kill( &m_hForce );
			fforce_Play( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROCKET_THRUST_HEAVY, &m_hForce );
		}

		// transition the camera back to normal
		SetCameraOverride(FALSE,1.5f);

		//play the stomp jump sound
		if( m_nPossessionPlayerIndex < 0 ) { 
			fsndfx_Play3D( m_BotInfo_TitanSounds.hSndStomp, &m_MountPos_WS, m_BotInfo_Titan.fSoundRadius );
		} else {
			fsndfx_Play2D( m_BotInfo_TitanSounds.hSndStomp );
		}

		if( m_pStickyEntity && m_pStickyEntity->GetVerlet() ) {
			CFVec3A ImpulseVec_WS;

			ImpulseVec_WS.Set( 0.0f, -2.0f*m_pBotInfo_Gen->fPhysForce_Land, 0.0f );
			m_pStickyEntity->GetVerlet()->ApplyImpulse( &m_MtxToWorld.m_vPos, &ImpulseVec_WS );
		}

		// do stomp damage
		if( pDamageForm ) {
			pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_BLAST;
			pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS;
			pDamageForm->m_pDamageProfile = m_BotInfo_Titan.pStompDamageProfile;
			pDamageForm->m_Epicenter_WS = m_MountPos_WS;
			pDamageForm->m_Damager.pBot = this;
			pDamageForm->m_Damager.pWeapon = NULL;
			pDamageForm->m_Damager.pEntity = this;

			CDamage::SubmitDamageForm( pDamageForm );
		}
		
	} else if( (m_nJumpState != BOTJUMPSTATE_LAND) && (m_nJumpState != BOTJUMPSTATE_LAND2) ) {
		m_nJumpState	= BOTJUMPSTATE_LAND;
		ZeroTime( ANIMTAP_JUMP_LAND_UPPER );
		SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, m_fUnitFlyBlend );

		if( m_nPossessionPlayerIndex > -1 ) {
			fforce_Kill( &m_hForce );
			fforce_Play( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROCKET_THRUST_LIGHT, &m_hForce );
		}

		// if we're moving slower than walk, just play the lower body jump anim, otherwise blend it in
		if( m_fSpeedXZ_WS < m_pBotInfo_Walk->fMinWalkVelocity ) {
            SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, m_fUnitFlyBlend );
			m_bPlayLandAnimLower = TRUE;


		} else {
			//stick a little bit
			m_Velocity_WS.Mul( 0.75f );
			VelocityHasChanged();
	
			SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
			m_bPlayLandAnimLower	= TRUE;
			m_fMaxLandUnitBlend		= 1.0f - fmath_Div( m_fSpeedXZ_WS - m_pBotInfo_Walk->fMinWalkVelocity, m_pBotInfo_Walk->fMaxXlatVelocity );
			FMATH_CLAMPMAX( m_fMaxLandUnitBlend, m_fUnitFlyBlend );
			SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, m_fMaxLandUnitBlend );
		}

		if( m_pStickyEntity && m_pStickyEntity->GetVerlet() ) {
			CFVec3A ImpulseVec_WS;

			ImpulseVec_WS.Set( 0.0f, m_pBotInfo_Gen->fPhysForce_Land, 0.0f );
			m_pStickyEntity->GetVerlet()->ApplyImpulse( &m_MtxToWorld.m_vPos, &ImpulseVec_WS );
		}

		// do stomp damage
		if( pDamageForm ) {
			pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_BLAST;
			pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS;
			pDamageForm->m_pDamageProfile = m_BotInfo_Titan.pFootstepDamageProfile;
			pDamageForm->m_Epicenter_WS = m_MountPos_WS;
			pDamageForm->m_Damager.pBot = this;
			pDamageForm->m_Damager.pWeapon = NULL;
			pDamageForm->m_Damager.pEntity = this;

			CDamage::SubmitDamageForm( pDamageForm );
		}
	}

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH,	0.0f );
	SetControlValue( ANIMCONTROL_JUMP_FLY,		0.0f );
	SetControlValue( ANIMCONTROL_JUMP2_FLY,		0.0f );


	if ( m_nPossessionPlayerIndex >= 0 )	{
		if( m_pBotInfo_Sound && m_nSurfaceTypeOn != SURFACE_TYPE_NONE) {
			// Don't play scuff sounds when landing from a jump
			fsndfx_Play2D(
				m_pBotInfo_Sound->ahStepSound[m_nSurfaceTypeOn][ fmath_RandomRange( 0, ( BOTSOUND_SCUFF_SOUND_START - 1 ) ) ],
				m_pBotInfo_Walk->fJumpLandedSoundUnitVolume_2D * m_fStepVolumeScale
			);
		}
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

	if( HasNoLegs() && !IsDeadOrDying() ) {
		if( m_pNoLegsDebrisPileWorldMesh && !m_pNoLegsDebrisPileWorldMesh->IsAddedToWorld() ) {
			// Add debris pile to world...

			_PositionDebrisPile();
		}
	}
}


void CBotTitan::_PositionDebrisPile( void ) {
	CFMtx43A::m_Xlat.m_vPos.Set( m_MtxToWorld.m_vPos.x, m_MtxToWorld.m_vPos.y + m_pBotInfo_Gen->fNoLegsDeltaCollSphereY_MS, m_MtxToWorld.m_vPos.z );

	m_pNoLegsDebrisPileWorldMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_Xlat, FALSE );

	m_pNoLegsDebrisPileWorldMesh->UpdateTracker();
}


void CBotTitan::_DestroyDebrisPile( void ) {
	FASSERT( m_pNoLegsDebrisPileWorldMesh );

	// cover it up with an explosion

	FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();

	if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
		FExplosionSpawnParams_t SpawnParams;

		SpawnParams.InitToDefaults();

		SpawnParams.uFlags			= FEXPLOSION_SPAWN_NONE;
		SpawnParams.Pos_WS			= m_MtxToWorld.m_vPos;
		SpawnParams.UnitDir			= m_MtxToWorld.m_vUp;
		SpawnParams.uSurfaceType	= 0;
		SpawnParams.pDamageProfile	= NULL;
		SpawnParams.pDamager		= NULL;

		CExplosion2::SpawnExplosion( hSpawner, m_hDebrisExplosion, &SpawnParams );
	}

	m_pNoLegsDebrisPileWorldMesh->RemoveFromWorld();

}


void CBotTitan::_HandleJumpAnimations( void ) {
	f32 fUnitTime;

	switch( m_nJumpState ) {
		case BOTJUMPSTATE_LAUNCH:
			if( m_fUnitFlyBlend < 1.0f ) {
				DeltaTime( ANIMTAP_JUMP_LAUNCH, FLoop_fPreviousLoopSecs, TRUE );
				m_fUnitFlyBlend = GetUnitTime( ANIMTAP_JUMP_LAUNCH );
				SetControlValue( ANIMCONTROL_JUMP_LAUNCH, fmath_UnitLinearToSCurve( m_fUnitFlyBlend ) );
				FASSERT_UNIT_FLOAT( m_fUnitFlyBlend );
			} else {
				// all done
				SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );

				if( m_bDoubleJump ) { //stomp jump requested
					ZeroTime( ANIMTAP_JUMP2_FLY );	
					SetControlValue( ANIMCONTROL_JUMP2_FLY, 1.0f );
					m_nJumpState  = BOTJUMPSTATE_AIR2;
					m_bDoubleJump = FALSE;
					m_Velocity_WS = m_vJumpVelocity;
					VelocityHasChanged();

					//if( m_pShield ) {
					//	//m_pShield->DropShield();
					//	m_pShield->EnablePushOutBots( FALSE );		//stop pushing bots away so we can stomp em
					//	SuspendShield( TRUE );
					//}

					m_fPowerDraw += _POWERDRAW_JUMP2;
				} else {	//normal jump requested
					ZeroTime( ANIMTAP_JUMP_FLY );	
					SetControlValue( ANIMCONTROL_JUMP_FLY, m_fUnitFlyBlend );
					m_nJumpState  = BOTJUMPSTATE_AIR;
					m_Velocity_WS = m_vJumpVelocity;
					VelocityHasChanged();
					
					m_fPowerDraw += _POWERDRAW_JUMP1;
				}
			}
			break;

		case BOTJUMPSTATE_AIR:
			if( m_fUnitFlyBlend < 1.0f ) {
                m_fUnitFlyBlend += FLoop_fPreviousLoopSecs;
				FMATH_CLAMP_MAX1( m_fUnitFlyBlend );
				SetControlValue( ANIMCONTROL_JUMP_FLY, m_fUnitFlyBlend );
			}
			DeltaTime( ANIMTAP_JUMP_FLY );

			if( m_bDoubleJump ) {
				ZeroTime( ANIMTAP_JUMP2_FLY );
				m_nJumpState = BOTJUMPSTATE_AIR2;
				SetControlValue( ANIMCONTROL_JUMP2_FLY, 1.0f );
			}

			if( m_nState == STATE_GROUND ) {
				_HandleLanding();
			}
			break;

		case BOTJUMPSTATE_AIR2:
			DeltaTime( ANIMTAP_JUMP2_FLY, FLoop_fPreviousLoopSecs, TRUE );

			//make sure our shield doesn't come back up
			if( m_pShield ) {
				m_pShield->NotifyBotDamage();
			}

			if( m_nState == STATE_GROUND ) {
				_HandleLanding();
			}
			break;

		case BOTJUMPSTATE_LAND:
			if( m_fUnitFlyBlend > 0.0f ) {
				DeltaTime( ANIMTAP_JUMP_LAND_UPPER, FLoop_fPreviousLoopSecs, TRUE );
				m_fUnitFlyBlend -= FLoop_fPreviousLoopSecs;
				FMATH_CLAMP_UNIT_FLOAT( m_fUnitFlyBlend );
				SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, m_fUnitFlyBlend );
				if( m_bPlayLandAnimLower ) {
					fUnitTime = GetControlValue( ANIMCONTROL_JUMP_LAND_LOWER );
					fUnitTime -= FLoop_fPreviousLoopSecs;
					FMATH_CLAMP_MIN0( fUnitTime );
					SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, fUnitTime );
				}
			} else {
				// jump over
				m_nJumpState = BOTJUMPSTATE_NONE;
				SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
			}
			break;

		case BOTJUMPSTATE_LAND2:
			fUnitTime = GetUnitTime( ANIMTAP_JUMP2_LAND );
			if( !DeltaTime( ANIMTAP_JUMP2_LAND, FLoop_fPreviousLoopSecs, TRUE ) ) {
				fUnitTime = (1.0f - fUnitTime) * 8.0f;
				FMATH_CLAMP_UNIT_FLOAT( fUnitTime );
				SetControlValue( ANIMCONTROL_JUMP2_LAND, fUnitTime );

				m_Velocity_WS.x			= 0.0f;
				m_Velocity_WS.z			= 0.0f;
				VelocityHasChanged();
			} else {
				m_fUnitFlyBlend = 0.0f;
				m_nJumpState	= BOTJUMPSTATE_NONE;
				SetControlValue( ANIMCONTROL_JUMP2_LAND, 0.0f );
			}
			break;

		default:
			if( m_nState == STATE_GROUND && m_fUnitFlyBlend > 0.0f ) {
				_HandleLanding();
			}
			break;
	}
}


void CBotTitan::_StartSingleJump( const CFVec3A *pJumpVelocity_WS ) {
	// titan can only jump after completing landing
	if( m_nJumpState != BOTJUMPSTATE_LAUNCH && m_nJumpState != BOTJUMPSTATE_AIR && m_nJumpState != BOTJUMPSTATE_NONE ) {
		return;	
	}

	if( IsJumping() && !m_pPartMgr->IsBackBroken() && (m_nJumpState == BOTJUMPSTATE_LAUNCH) ) { // we were already jumping, do the stomp thing
		m_bDoubleJump		= TRUE;

		// start the camera transition to watch the landing:
		SetCameraOverride(TRUE,0.25f,TRUE,m_vecStompCameraLookat,m_vecStompCameraPos);
		m_fGravityMultiplier	= m_BotInfo_Titan.fStompGravityMultiplier;

	} else {
		ZeroTime( ANIMTAP_JUMP_LAUNCH );
		ZeroTime( ANIMTAP_JUMP_FLY );
		ZeroTime( ANIMTAP_JUMP_LAND_UPPER );
		ZeroTime( ANIMTAP_JUMP2_FLY );
		ZeroTime( ANIMTAP_JUMP2_LAND );
		m_bDoubleJump = FALSE;
	}

	m_nJumpState	= BOTJUMPSTATE_LAUNCH;
	SetJumping();
	SetDoubleJumpTimer( m_pBotInfo_Gen->fDoubleJumpTime );
}


void CBotTitan::_HandleWeaponFiring( void ) {
	f32 fCtlValue;
	BOOL bFiredWeapon = FALSE;

	FASSERT( m_apWeapon[0] && m_apWeapon[0]->IsCreated() );
	FASSERT( m_aapDupWeapons[0][0] && m_aapDupWeapons[0][0]->IsCreated() );

	if( m_pPartMgr->GetLimbState( LIMB_TYPE_RIGHT_ARM ) != CBotPartMgr::LIMB_STATE_REMOVED ) {
		bFiredWeapon = m_apWeapon[0]->TriggerWork( m_fControls_Fire1, m_fControls_Fire2, &m_TargetedPoint_WS, NULL );
	} else {
		m_apWeapon[0]->TriggerWork( 0.0f, 0.0f, &m_TargetedPoint_WS, NULL );

		// the titan's chain guns share lights & sounds when both are there.  If they aren't both there, that needs to be disabled.
		((CWeaponChaingun*)m_aapDupWeapons[0][0])->OverrideLights( TRUE, FALSE );
		((CWeaponChaingun*)m_aapDupWeapons[0][0])->OverrideSounds( TRUE, FALSE );
	}

	if( m_pPartMgr->GetLimbState( LIMB_TYPE_LEFT_ARM ) != CBotPartMgr::LIMB_STATE_REMOVED ) {
		bFiredWeapon |= m_aapDupWeapons[0][0]->TriggerWork( m_fControls_Fire1, m_fControls_Fire2, &m_TargetedPoint_WS, NULL );
	} else {
		m_aapDupWeapons[0][0]->TriggerWork( 0.0f, 0.0f, &m_TargetedPoint_WS, NULL );

		// the titan's chain guns share lights & sounds when both are there.  If they aren't both there, that needs to be disabled.
		((CWeaponChaingun*)m_apWeapon[0])->OverrideLights( TRUE, FALSE );
		((CWeaponChaingun*)m_apWeapon[0])->OverrideSounds( TRUE, FALSE );

	}

	if( bFiredWeapon ) {
		m_bFiringGuns = TRUE;
		// if we're firing, draw some power
		m_fPowerDraw += 10.0f * FLoop_fPreviousLoopSecs;

		JustFired();

		// blend in fire animation
		fCtlValue = GetControlValue( ANIMCONTROL_FIRE_1 );
		UpdateUnitTime( ANIMTAP_FIRE_1, fmath_RandomChance(0.5) ? 1.0f : 0.75f );
		
		if( fCtlValue < 1.0f ) {
			fCtlValue += FLoop_fPreviousLoopSecs * _FIRE_ANIM_TRANSITION_TIME;
			FMATH_CLAMP_MAX1( fCtlValue );
			SetControlValue( ANIMCONTROL_STAND, 1.0f - fCtlValue );
			SetControlValue( ANIMCONTROL_FIRE_1, fCtlValue );
		}
	} else {
		m_bFiringGuns = FALSE;

		fCtlValue = GetControlValue( ANIMCONTROL_FIRE_1 );
		if( fCtlValue > 0.0f ) {
			fCtlValue -= FLoop_fPreviousLoopSecs;
			FMATH_CLAMP_MIN0( fCtlValue );

			m_fUnitTopSpeedLimiter = 1.0f;

			SetControlValue( ANIMCONTROL_FIRE_1, fCtlValue );
			SetControlValue( ANIMCONTROL_STAND, 1.0f - fCtlValue );
		}
	}





#if 0
	if( !m_apWeapon[0] ) {
		return;
	}

	if( m_pPartMgr->GetLimbState( LIMB_TYPE_RIGHT_ARM ) == CBotPartMgr::LIMB_STATE_REMOVED ) {
		fTrig1 = 0.0f;
	} else {
		fTrig1 = m_fControls_Fire1;
	}

	if( m_apWeapon[0]->TriggerWork( fTrig1, fTrig2, &m_TargetedPoint_WS, NULL ) ) {
		m_bFiringGuns = TRUE;
		// if we're firing, draw some power
		m_fPowerDraw += 10.0f * FLoop_fPreviousLoopSecs;

		JustFired();

		fCtlValue = GetControlValue( ANIMCONTROL_FIRE_1 );
		UpdateUnitTime( ANIMTAP_FIRE_1, fmath_RandomChance(0.5) ? 1.0f : 0.75f );
		
		if( fCtlValue < 1.0f ) {
			//we are firing our chainguns
			fCtlValue += FLoop_fPreviousLoopSecs * _FIRE_ANIM_TRANSITION_TIME;
			FMATH_CLAMP_MAX1( fCtlValue );
			SetControlValue( ANIMCONTROL_STAND, 1.0f - fCtlValue );
			SetControlValue( ANIMCONTROL_FIRE_1, fCtlValue );
		}
	} else {
		m_bFiringGuns = FALSE;

		fCtlValue = GetControlValue( ANIMCONTROL_FIRE_1 );
		if( fCtlValue > 0.0f ) {
			fCtlValue -= FLoop_fPreviousLoopSecs;
			FMATH_CLAMP_MIN0( fCtlValue );

			m_fUnitTopSpeedLimiter = 1.0f;

			SetControlValue( ANIMCONTROL_FIRE_1, fCtlValue );
			SetControlValue( ANIMCONTROL_STAND, 1.0f - fCtlValue );
		}
	}

	// now do the dup chaingun.  First make sure arm is still attached...
	if( m_pPartMgr->GetLimbState( LIMB_TYPE_LEFT_ARM ) == CBotPartMgr::LIMB_STATE_REMOVED ) {
		fTrig1 = 0.0f;
	}

	if( m_aapDupWeapons[0][0] ) {
		CFVec3A vecMuzzlePt, vecFireUnitDir;

//		m_aapDupWeapons[0][0]->ComputeMuzzlePoint_WS( &vecMuzzlePt );
//		vecFireUnitDir.Sub( m_TargetedPoint_WS, vecMuzzlePt ).Unitize();

		if (m_aapDupWeapons[0][0]->TriggerWork( fTrig1, fTrig2, &m_TargetedPoint_WS, NULL ))
		{
			JustFired();
		}
	}
#endif
}


void CBotTitan::_HandleWeaponWork( void ) {
	CFVec3A vWpn1Pt;

	// calculate the midpoint of our chaingun muzzles

	m_apWeapon[0]->ComputeMuzzlePoint_WS( &m_vChaingunMidpoint_WS );
	m_aapDupWeapons[0][0]->ComputeMuzzlePoint_WS( &vWpn1Pt );
	vWpn1Pt.Sub( m_vChaingunMidpoint_WS );
	vWpn1Pt.Mul( 0.5f );
	m_vChaingunMidpoint_WS.Add( vWpn1Pt );

	WeaponsWork( 0 );
}


void CBotTitan::_HandleFiringAnimations( void ) {
}

void CBotTitan::_CheckForDoubleJump( void ) {
	if( !HaveTimeToDoubleJump() ) {
		return;
	}

	if( m_bControls_Jump && 
		!m_pPartMgr->IsBackBroken() && 
		m_nPossessionPlayerIndex < 0 ) {

		m_bDoubleJump		= TRUE;

		// start the camera transition to watch the landing:
		SetCameraOverride(TRUE,0.25f,TRUE,m_vecStompCameraLookat,m_vecStompCameraPos);
		m_fGravityMultiplier	= m_BotInfo_Titan.fStompGravityMultiplier;
		if( m_pShield ) {
			//m_pShield->DropShield();
			m_pShield->EnablePushOutBots( FALSE );		//stop pushing bots away so we can stomp em
			SuspendShield( TRUE );

		}
	}
}


void CBotTitan::_HandleAimingAnimations( void ) {
	f32 fUnitAimPitch;
	f32 fctlvalue;
	CFAnimFrame torsoQuatFrame;
	f32 fTwistAngle;

	fctlvalue = GetControlValue( ANIMCONTROL_AIM_SUMMER );
	if( m_bStooping ) {
		if( fctlvalue > 0.0f ) {
			fctlvalue -= FLoop_fPreviousLoopSecs;
			FMATH_CLAMP_MIN0(fctlvalue);
			SetControlValue( ANIMCONTROL_AIM_SUMMER, fctlvalue );
		}
	} else {
		// handle all torso twist activities
		CFVec3A vRotAxis;
		if( fctlvalue < 1.0f ) {
			fctlvalue += FLoop_fPreviousLoopSecs;
			FMATH_CLAMP_MAX1(fctlvalue);
			SetControlValue( ANIMCONTROL_AIM_SUMMER, fctlvalue );
		}

		fUnitAimPitch = fmath_Div( m_fAimPitch_WS - m_fMountPitchMin_WS, m_fMountPitchMax_WS - m_fMountPitchMin_WS );
		FMATH_CLAMP_UNIT_FLOAT( fUnitAimPitch );

		// figure out the angle to twist torso.  Increase it if we've just fired a rocket
		fTwistAngle =  FMATH_FPOT( fUnitAimPitch, FMATH_DEG2RAD(-20.0f), FMATH_DEG2RAD(20.0f) );
		fTwistAngle -= _ROCKET_REACT_ANGLE * m_fRocketReaction;

		// if we're reacting to rocket fire, adjust our rot axis so the right shoulder is jerking back
		fctlvalue = FMATH_DEG2RAD(-30.0f) * m_fRocketReaction;
		FMATH_CLAMP( fctlvalue, FMATH_DEG2RAD(-30.0f), 0.0f );
		vRotAxis.ReceiveRotationZ( CFVec3A::m_UnitAxisX, fctlvalue );
		
		// do the animation thing
		torsoQuatFrame.BuildQuat( vRotAxis, fTwistAngle, CFVec3A::m_Null );
		m_AnimManFrameAim.UpdateFrame( BONE_TORSO, torsoQuatFrame );
	}

	////decide whether or not we should aim the guns
	//if(  (m_nState == STATE_GROUND && m_nJumpState == BOTJUMPSTATE_NONE) && 
	//	 ((m_nOwnerPlayerIndex >=0) || (m_nPossessionPlayerIndex >= 0) || ((m_nControlsBot_Flags & CBotControl::FLAG_AIM_AT_TARGET_POINT) && (m_nBotFlags & BOTFLAG_TARGET_LOCKED))) ) {

	//	m_fGunAimTimer -= FLoop_fPreviousLoopSecs * 1.0f;
	//	FMATH_CLAMPMIN( m_fGunAimTimer, -1.0f);
	//} else {
	//	m_fGunAimTimer += FLoop_fPreviousLoopSecs * 10.0f;
	//	FMATH_CLAMPMAX( m_fGunAimTimer, 1.0f);
	//}
}



void CBotTitan::_HandleArmAim( CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx, const CFVec3A &rAimPt, f32 fSlerp, f32 *pfLastAngle ) {
	f32			fDotCos, fDotSin, fAngle;
	CFVec3A		vecAim;
	CFQuatA		qtAim;
	
	CFMtx43A::m_Temp.Mul( rParentMtx, rBoneMtx );

	vecAim = rAimPt;
	vecAim.Sub( CFMtx43A::m_Temp.m_vP );
	vecAim.Unitize();

	fDotCos = vecAim.Dot( CFMtx43A::m_Temp.m_vUp );
	fDotSin = vecAim.Dot( CFMtx43A::m_Temp.m_vFront );
	fAngle	= fmath_Atan( fDotCos, fDotSin );

	if( fAngle > *pfLastAngle ) {
		*pfLastAngle += FLoop_fPreviousLoopSecs * _MAX_ARM_ROT_SPEED;
		FMATH_CLAMPMAX(*pfLastAngle, fAngle );
	} else {
		*pfLastAngle -= FLoop_fPreviousLoopSecs * _MAX_ARM_ROT_SPEED;
		FMATH_CLAMPMIN(*pfLastAngle, fAngle );
	}

	FMATH_CLAMP( *pfLastAngle, -_MAX_ARM_AIMANGLE, _MAX_ARM_AIMANGLE );

	qtAim.BuildQuat( CFMtx43A::m_Temp.m_vRight, -*pfLastAngle );
    
	rNewMtx.m_vPos = CFMtx43A::m_Temp.m_vPos;

	if( fSlerp < 1.0f ) {
		CFQuatA qtAimTmp = qtAim;
		CFQuatA qtBase;

		qtBase.BuildQuat( CFMtx43A::m_Temp.m_vRight, 0.0f );

		FMATH_CLAMP_UNIT_FLOAT(fSlerp );
		qtAim.ReceiveSlerpOf( fSlerp, qtBase, qtAimTmp );
	}

	qtAim.MulPoint( rNewMtx.m_vRight,	CFMtx43A::m_Temp.m_vRight	);
	qtAim.MulPoint( rNewMtx.m_vUp,		CFMtx43A::m_Temp.m_vUp		);
	qtAim.MulPoint( rNewMtx.m_vFront,	CFMtx43A::m_Temp.m_vFront	);

	
}

#define _MIN_ELBOW_ANGLE .05f
#define _MAX_ELBOW_ANGLE 0.5f

void CBotTitan::_HandleElbowAim( CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx, const CFVec3A &rAimPt, f32 fSlerp, f32 *pfLastAngle ) {
	f32			fDotCos, fDotSin, fAngle;
	CFVec3A		vecAim;
	CFQuatA		qtAim;
	
	CFMtx43A::m_Temp.Mul( rParentMtx, rBoneMtx );

	vecAim = rAimPt;
	vecAim.Sub( CFMtx43A::m_Temp.m_vP );
	vecAim.Unitize();

	fDotCos = vecAim.Dot( CFMtx43A::m_Temp.m_vRight );
	fDotSin = vecAim.Dot( CFMtx43A::m_Temp.m_vFront );
	fAngle  = fmath_Atan( fDotCos, fDotSin );

	if( fAngle > *pfLastAngle ) {
		*pfLastAngle += FLoop_fPreviousLoopSecs * _MAX_ARM_ROT_SPEED;
		FMATH_CLAMPMAX(*pfLastAngle, fAngle );
	} else {
		*pfLastAngle -= FLoop_fPreviousLoopSecs * _MAX_ARM_ROT_SPEED;
		FMATH_CLAMPMIN(*pfLastAngle, fAngle );
	}


	FMATH_CLAMP( *pfLastAngle, -_MAX_ELBOW_AIMANGLE, _MAX_ELBOW_AIMANGLE );

	qtAim.BuildQuat( CFMtx43A::m_Temp.m_vUp, *pfLastAngle );
    
	rNewMtx.m_vPos = CFMtx43A::m_Temp.m_vPos;

	if( fSlerp < 1.0f ) {
		CFQuatA qtAimTmp = qtAim;
		CFQuatA qtBase;

		qtBase.BuildQuat( CFMtx43A::m_Temp.m_vUp, 0.0f );

		FASSERT( fSlerp <= 1.0f );
		FMATH_CLAMP_UNIT_FLOAT(fSlerp );

		qtAim.ReceiveSlerpOf( fSlerp, qtBase, qtAimTmp );
	}

	qtAim.MulPoint( rNewMtx.m_vRight,	CFMtx43A::m_Temp.m_vRight	);
	qtAim.MulPoint( rNewMtx.m_vUp,		CFMtx43A::m_Temp.m_vUp		);
	qtAim.MulPoint( rNewMtx.m_vFront,	CFMtx43A::m_Temp.m_vFront	);
}


void CBotTitan::_HandleRocketLauncherAim( CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	if( m_fCollCylinderHeight_WS < m_fStoopMaxHeight ) { //must be stooping
		f32 fSlerp = m_fStoopMaxHeight - m_fCollCylinderHeight_WS;
		CFMtx43A::m_Temp = rBoneMtx;
		CFMtx43A::m_Temp.SetRotationY( 0.0f );
		CFMtx43A::m_Temp.SetRotationZ( 0.0f );

		if( fSlerp <= 1.0 ) {
			CFQuatA	 qtfrom, qtto, qtslerped;
		
			qtfrom.BuildQuat( rBoneMtx.m_vFront, rBoneMtx.m_vFront );
			qtto.BuildQuat( rBoneMtx.m_vFront, CFMtx43A::m_Temp.m_vFront );
			qtslerped.ReceiveSlerpOf( fmath_UnitLinearToSCurve( fSlerp ), qtfrom, qtto );

			qtslerped.MulPoint( CFMtx43A::m_Temp.m_vRight,	rBoneMtx.m_vRight	);
			qtslerped.MulPoint( CFMtx43A::m_Temp.m_vUp,		rBoneMtx.m_vUp		);
			qtslerped.MulPoint( CFMtx43A::m_Temp.m_vFront,	rBoneMtx.m_vFront	);
		} 

		rNewMtx.Mul( rParentMtx, CFMtx43A::m_Temp );

	} else if( m_fRocketAimTimer < 1.0f ) {				// we're aiming our rocket launcher
		CFMtx43A::m_Temp.Mul( rParentMtx, rBoneMtx );

		//not sure this is the fastest way to do this...
		CFVec3A vecAim;
		CFQuatA qtAim, qtStart, qtFinal;
		f32 fSlerp;

//		vecAim.Sub( m_TargetedPoint_WS, CFMtx43A::m_Temp.m_vPos );
		vecAim.Sub( m_vFiredRocketAtPoint, CFMtx43A::m_Temp.m_vPos );
		vecAim.Unitize();

//		FASSERT( vecAim.Dot( CFMtx43A::m_Temp.m_vFront ) > 0.8f );

		fSlerp = 1.0f - m_fRocketAimTimer;
		FMATH_CLAMP_UNIT_FLOAT( fSlerp );

		qtAim.BuildQuat( CFMtx43A::m_Temp.m_vFront, vecAim );
		qtStart.BuildQuat( CFMtx43A::m_Temp.m_vFront, CFMtx43A::m_Temp.m_vFront );

		FASSERT_UNIT_FLOAT( fSlerp );

		qtFinal.ReceiveSlerpOf( fmath_UnitLinearToSCurve( fSlerp ), qtStart, qtAim );

		rNewMtx.m_vPos = CFMtx43A::m_Temp.m_vPos;
		qtFinal.MulPoint( rNewMtx.m_vRight,	CFMtx43A::m_Temp.m_vRight	);
		qtFinal.MulPoint( rNewMtx.m_vUp,	CFMtx43A::m_Temp.m_vUp		);
		qtFinal.MulPoint( rNewMtx.m_vFront,	CFMtx43A::m_Temp.m_vFront	);
	} else {
		rNewMtx.Mul(rParentMtx, rBoneMtx );
	}
}


void CBotTitan::CheckpointSaveSelect( s32 nCheckpoint ) {
	// save bot's weapons first
	if( m_apWeapon[0] )
	{
		m_apWeapon[0]->CheckpointSaveSelect( nCheckpoint );
	}

	if( m_aapDupWeapons[0][0] ) {
		m_aapDupWeapons[0][0]->CheckpointSaveSelect( nCheckpoint );
	}

	if( m_apWeapon[1] )
	{
		m_apWeapon[1]->CheckpointSaveSelect( nCheckpoint );
	}

	// then save self
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}

BOOL CBotTitan::CheckpointSave( void ) {
	f32 fShieldHealth;
	CBot::CheckpointSave();
	
	if( m_pShield ) {
		fShieldHealth = m_pShield->NormHealth();
	} else {
		fShieldHealth = 0.0f;
	}
    
	CFCheckPoint::SaveData( fShieldHealth );
	CFCheckPoint::SaveData( m_bLightEnabled );

	return TRUE;
}


void CBotTitan::CheckpointRestore( void ) {
	f32 fShieldHealth;
	CBot::CheckpointRestore();
	
	CFCheckPoint::LoadData( fShieldHealth );
	CFCheckPoint::LoadData( m_bLightEnabled );
    
	if( m_pShield ) { 
		m_pShield->SetShieldHealth( fShieldHealth );
	}

	_EnableLight( m_bLightEnabled );

	// set our chaingun states
	if( m_uLifeCycleState == BOTLIFECYCLE_DEAD ) {
		if( m_apWeapon[0] ) {
			m_apWeapon[0]->SetDisabledFlag( TRUE );
		}

		if( m_aapDupWeapons[0][0] ) {
			m_aapDupWeapons[0][0]->SetDisabledFlag( TRUE );
		}
	} else {
		if( m_apWeapon[0] ) {
			m_apWeapon[0]->SetDisabledFlag( FALSE );
		}

		if( m_aapDupWeapons[0][0] ) {
			m_aapDupWeapons[0][0]->SetDisabledFlag( FALSE );
		}
	}

	// clear some control values
	SetControlValue( ANIMCONTROL_JUMP2_FLY, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP2_LAND, 0.0f  );
}



BOOL CBotTitan::_InitStoopAnimation( void ) {

	//TORSO
	CFMtx43A::m_Temp.Identity();
	CFMtx43A::m_Temp.SetRotationX( FMATH_DEG2RAD(55.0f) );
	CFMtx43A::m_Temp.m_vPos.y -= 0.5f;
	m_AnimManMtxStoop.UpdateFrame( _STOOP_BONE_TORSO, CFMtx43A::m_Temp );

	//HEAD & LIGHT
	CFMtx43A::m_RotX.SetRotationX( FMATH_DEG2RAD(-45.0f) );
	m_AnimManMtxStoop.UpdateFrame( _STOOP_BONE_HEAD, CFMtx43A::m_RotX );
	CFMtx43A::m_RotX.SetRotationX( FMATH_DEG2RAD(-55.0f) );
	m_AnimManMtxStoop.UpdateFrame( _STOOP_BONE_SPOTLIGHT, CFMtx43A::m_RotX );

	//REACTOR
	CFMtx43A::m_Temp.Identity();
	CFMtx43A::m_RotX.SetRotationX( FMATH_DEG2RAD(15.0f));
	CFMtx43A::m_Temp.m_vPos.y -= 1.25f;
	CFMtx43A::m_Temp.Mul( CFMtx43A::m_RotX );
	m_AnimManMtxStoop.UpdateFrame( _STOOP_BONE_REACTOR, CFMtx43A::m_Temp );


	//R & L LEG_UPPER
	CFMtx43A::m_RotX.SetRotationX( FMATH_DEG2RAD(13.0f) );
	m_AnimManMtxStoop.UpdateFrame( _STOOP_BONE_LLEG_UPPER, CFMtx43A::m_RotX );
	m_AnimManMtxStoop.UpdateFrame( _STOOP_BONE_RLEG_UPPER, CFMtx43A::m_RotX );


	//R & L LEG_LOWER
	CFMtx43A::m_RotX.SetRotationX( FMATH_DEG2RAD(-36.0f) );
	m_AnimManMtxStoop.UpdateFrame( _STOOP_BONE_LLEG_LOWER, CFMtx43A::m_RotX );
	m_AnimManMtxStoop.UpdateFrame( _STOOP_BONE_RLEG_LOWER, CFMtx43A::m_RotX );

	//GROIN
	CFMtx43A::m_Temp.Identity();
	CFMtx43A::m_Temp.m_vPos.y -= 1.0f;
	m_AnimManMtxStoop.UpdateFrame( _STOOP_BONE_GROIN, CFMtx43A::m_Temp );

	//ROCKETLAUNCHER
	CFMtx43A::m_RotX.SetRotationX( FMATH_DEG2RAD(-60.0f) );
	m_AnimManMtxStoop.UpdateFrame( _STOOP_BONE_ROCKETLAUNCHER, CFMtx43A::m_RotX );


	//R & L ARM_UPPER
	CFMtx43A::m_RotX.SetRotationX( FMATH_DEG2RAD( -25.0f ) );
	m_AnimManMtxStoop.UpdateFrame( _STOOP_BONE_LARM_UPPER, CFMtx43A::m_RotX );
	m_AnimManMtxStoop.UpdateFrame( _STOOP_BONE_RARM_UPPER, CFMtx43A::m_RotX );
	
		
	//FEETT
	CFMtx43A::m_RotX.SetRotationX( FMATH_DEG2RAD( 20.0f ) );
	m_AnimManMtxStoop.UpdateFrame( _STOOP_BONE_LFOOT_FRONT, CFMtx43A::m_RotX );
	m_AnimManMtxStoop.UpdateFrame( _STOOP_BONE_RFOOT_FRONT, CFMtx43A::m_RotX );

	CFMtx43A::m_RotX.SetRotationX( FMATH_DEG2RAD( 34.0f ) );
	m_AnimManMtxStoop.UpdateFrame( _STOOP_BONE_LFOOT_BACK, CFMtx43A::m_RotX );
	m_AnimManMtxStoop.UpdateFrame( _STOOP_BONE_RFOOT_BACK, CFMtx43A::m_RotX );

	CFMtx43A::m_RotX.SetRotationX( FMATH_DEG2RAD( -5.0f ) );
	m_AnimManMtxStoop.UpdateFrame( _STOOP_BONE_LHEEL, CFMtx43A::m_RotX );
	m_AnimManMtxStoop.UpdateFrame( _STOOP_BONE_RHEEL, CFMtx43A::m_RotX );

	EnableControlSmoothing( ANIMCONTROL_STOOP_SUMMER );

	return TRUE;
}


void CBotTitan::_HandleRocketWork( void ) {
	CFVec2	vTex;
	vTex.x = 0.0f;
	s32 nPrevReloadTime;
	f32 fFireControl;

#if 0
	//no firing if our rocket launcher is gone
	if( (m_pPartMgr->GetLimbState( LIMB_TYPE_ROCKET_LAUNCHER ) == CBotPartMgr::LIMB_STATE_REMOVED) ||
		((m_nPossessionPlayerIndex >= 0) && (m_fControls_Fire1 > _FIRE_THRESHOLD) ) ) {
		fFireControl = 0.0f;
	} else {
		fFireControl = m_fControls_Fire2;
	}
#else
	//no firing if our rocket launcher is gone
	if( (m_pPartMgr->GetLimbState( LIMB_TYPE_ROCKET_LAUNCHER ) == CBotPartMgr::LIMB_STATE_REMOVED) ) {
		fFireControl = 0.0f;
	} else {
		fFireControl = m_fControls_Fire2;
	}
#endif

	//deal with the lights
	vTex.y = m_fRocketLoadTimer;
	FMATH_CLAMP_UNIT_FLOAT( vTex.y );
	vTex.y = FMATH_FPOT( vTex.y, 1.0f, 0.1f );
	m_pWorldMesh->AnimTC_SetScrollST( m_hRocketTexLayer, vTex );


//	for( u32 i=0; i<ROCKET_NUM_CHAMBERS; i++ ) {
//		vTex.y = m_fRocketLoadTimer - (f32)i;
//		FMATH_CLAMP_UNIT_FLOAT( vTex.y );
//		vTex.y = FMATH_FPOT( vTex.y, 1.0f, 0.1f );
////		m_pWorldMesh->AnimTC_SetScrollST( m_ahRocketTexLayers[i], vTex );
//
//	}

	//handle reloading
	nPrevReloadTime = fmath_FloatToS32( m_fRocketLoadTimer );
	m_fRocketLoadTimer += FLoop_fRealPreviousLoopSecs * m_BotInfo_Titan.fOORocketReloadTime;
	
//	if( m_bRocketTriggerDown ) {
//		FMATH_CLAMPMAX( m_fRocketLoadTimer, 3.0f );
//	} else {
		FMATH_CLAMPMAX( m_fRocketLoadTimer, 1.0f );
//	}

	// if we've loaded a new rocket, play the sound
	if( (nPrevReloadTime >= 0) && (nPrevReloadTime != fmath_FloatToS32( m_fRocketLoadTimer )) ) {
		if( m_nPossessionPlayerIndex >= 0 ) { 
			fsndfx_Play2D( m_BotInfo_TitanSounds.hSndRLReady );
		}
	}

	//AI override
	if( m_bFireRocketThisFrame ) {
		m_uRocketsToFire = 1;
		m_bFireRocketThisFrame = FALSE;
	}

	//fire
	//if( (m_fRocketLoadTimer == 3.0f) || (m_bRocketTriggerDown && (fFireControl < 0.1f ) && (m_fRocketLoadTimer >= 1.0f)) ) {
	if( m_fRocketLoadTimer == 1.0f && (fFireControl > 0.1f) ) {
		//fire rocket(s)
		m_uRocketsToFire = fmath_FloatToU32( m_fRocketLoadTimer);
		m_fRocketFireTimer = m_fRocketAimTimer * m_BotInfo_Titan.fRocketAimTime;		// don't fire the rocket until we're fully aimed

		if( m_uRocketsToFire == 1 ) {
			m_fRocketLoadTimer = 0.0f;
		} else {
			m_fRocketLoadTimer = -m_BotInfo_Titan.fRocketFullReloadTime;
		}
	}

	if( fFireControl > 0.1f || (m_uRocketsToFire > 0) ) {
		//getting ready to fire
		m_bRocketTriggerDown = TRUE;
		m_fRocketAimTimer -= FLoop_fPreviousLoopSecs * m_BotInfo_Titan.fOORocketAimTime;
	} else {
		m_bRocketTriggerDown = FALSE;
		m_fRocketAimTimer += FLoop_fPreviousLoopSecs * m_BotInfo_Titan.fOORocketAimOffTime;
	}
	FMATH_BIPOLAR_CLAMPMAX( m_fRocketAimTimer, 1.0f );



	// now fire if any are requested
	if( m_uRocketsToFire > 0 ) {	
		m_fRocketFireTimer -= FLoop_fPreviousLoopSecs;
		if( m_fRocketFireTimer < 0.0f ) {
			_FireRocket();
			m_fRocketFireTimer = m_BotInfo_Titan.fRocketRippleDelay;
			m_uRocketsToFire--;
		}
	}
	
	if( (m_nPossessionPlayerIndex >= 0) || (m_uRocketsToFire == 0) ) {
		// else {
//		if( (m_nPossessionPlayerIndex >= 0) || (m_nBotFlags & BOTFLAG_TARGET_LOCKED) ) {
			m_vFiredRocketAtPoint = m_TargetedPoint_WS;
//		}
	}

	//now handle titan's reaction to the rocketlaunch
	if( m_bRocketReactionOn ) {
		m_fRocketReaction += FLoop_fPreviousLoopSecs * _ROCKET_OO_REACT_ON_TIME;
		FMATH_CLAMPMAX( m_fRocketReaction, m_fRocketReactionTarget );
		if( m_fRocketReaction == m_fRocketReactionTarget ) {
			m_bRocketReactionOn		= FALSE;
		}
	} else {
		m_fRocketReaction -= FLoop_fPreviousLoopSecs * _ROCKET_OO_REACT_OFF_TIME;
		FMATH_CLAMP_MIN0( m_fRocketReaction );
		m_fRocketReactionTarget = m_fRocketReaction;
	}	
}


void CBotTitan::_BuildRocketSkipList( CEProj *pProj ) {
	CBotTitan *pBot = (CBotTitan*)pProj->GetDamagerBot();
	pBot->AppendTrackerSkipList();
}



void CBotTitan::_FireRocket( void ) {
	// Get a free projectile...

	CEProj *pProj = CEProjPool::GetProjectileFromFreePool( CWeapon::m_ahProjPool[CWeapon::PROJ_POOL_TYPE_SWARMER_ROCKET] );
	if( pProj == NULL ) {
		// No more projectiles...
		return;
	}
	JustFired();

	CFVec3A vUnitFireDir;

	// calculate fire direction
	if( m_pPartMgr && m_pPartMgr->IsCreated() && 
		(m_pPartMgr->GetComponentStatus( CBotPartMgr::COMPONENT_TYPE_SECONDARY ) == CBotPartMgr::COMPONENT_STATUS_ALL_FULLY_OPERATIONAL) ) {
		f32 fMag2;
		vUnitFireDir.Sub( m_vFiredRocketAtPoint, m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexRocketFire]->m_vPos );
		fMag2 = vUnitFireDir.MagSq();
		if( fMag2 > 0.01f ) {
			vUnitFireDir.Mul( fmath_InvSqrt( fMag2 ) );
		} else {
			vUnitFireDir = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexRocketFire]->m_vFront;
		}
	} else {
		vUnitFireDir = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexRocketFire]->m_vFront;
	}

	_InitProjectile( pProj, &m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexRocketFire]->m_vPos, &vUnitFireDir, FALSE );
	pProj->Launch();

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
}


void CBotTitan::_InitProjectile( CEProj *pProj, const CFVec3A *pPos_WS, const CFVec3A *pUnitDir_WS, BOOL bNoDynamicLight ) {
	CFMtx43A ProjMtx;

	ProjMtx.UnitMtxFromUnitVec( pUnitDir_WS );
	ProjMtx.m_vPos = *pPos_WS;

	pProj->Init();
	pProj->SetDamager( NULL, this );

	if( IsPlayerBot() ) {
		pProj->SetDamageProfile( m_BotInfo_Titan.pPCRocketDamageProfile );
	} else {
		pProj->SetDamageProfile( m_BotInfo_Titan.pNPCRocketDamageProfile );
	}
	
	pProj->SetSkipListCallback( _BuildRocketSkipList );
	pProj->SetMaxDistCanTravel( m_BotInfo_Titan.fRocketRange );
	pProj->SetLinSpeed( m_BotInfo_Titan.fRocketSpeed );
	pProj->SetLinUnitDir_WS( pUnitDir_WS );
	pProj->SmokeTrailOn( &m_SmokeTrailAttrib );
	pProj->SetExplosionGroup( m_hRocketExplosionImpact );
	pProj->LoopingSoundOn( m_BotInfo_Titan.pSndGrpRocketLoop );

	if( m_pShield ) {
		CEProjExt::CEProj_Swarmer_Params_t SwarmerParams;
		SwarmerParams.pMervStaticParams = &m_BotInfo_Titan.MervStaticParams;
		SwarmerParams.fSwarmAmplitude = 0.0f;
		SwarmerParams.fSwarmingBlendInInvDist = 0.0f;
		SwarmerParams.fSwarmingSpiralSpeed = 0.0f;
		pProj->SetSwarmerParams( &SwarmerParams );
	}

	pProj->Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &ProjMtx, m_BotInfo_Titan.fProjectileMeshScale );

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
}


#if 0
BOOL CBotTitan::_RocketDetonateCallback( CEProj *pProj, CEProj::Event_e nEvent, const FCollImpact_t *pImpact ) {
	if( pImpact ) {
		//if( nEvent == CEProj::EVENT_HIT_GEO ) {
		//	explosion_Spawn(
		//		EXPLOSION_TYPE_ROCKET,
		//		pImpact->ImpactPoint,
		//		0.5f,
		//		&pImpact->UnitFaceNormal,
		//		TRUE,
		//		100.0f
		//	);
		//}

		// Get an empty damage form...
		CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

		if( pDamageForm ) {
			// Fill out the form...

			pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_BLAST;
			pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS;
			pDamageForm->m_pDamageProfile = pProj->GetDamageProfile();
			pDamageForm->m_Epicenter_WS = pProj->MtxToWorld()->m_vPos;
			pDamageForm->m_Damager = *pProj->GetDamager();

			CDamage::SubmitDamageForm( pDamageForm );
		}

		// create an explosion
		FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();

		if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
			FExplosionSpawnParams_t SpawnParams;

			SpawnParams.InitToDefaults();

			SpawnParams.uFlags			= FEXPLOSION_SPAWN_NONE;
			SpawnParams.Pos_WS			= pImpact->ImpactPoint;
			SpawnParams.UnitDir			= pImpact->UnitFaceNormal;
			SpawnParams.uSurfaceType	= pImpact->nUserType;;
			SpawnParams.pDamageProfile	= NULL;
			SpawnParams.pDamager		= NULL;

			CExplosion2::SpawnExplosion( hSpawner, m_hRocketExplosionImpact, &SpawnParams );
		}
	}

	return FALSE;
}
#endif


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

f32 CBotTitan::GetRocketTravelSpeed( void ) {
	return m_BotInfo_Titan.fRocketSpeed;
}


void CBotTitan::_EnableLight( BOOL bEnable ) {
	FASSERT( m_pLight != NULL );
	if( bEnable ) {
		m_bLightEnabled = TRUE;
		
		m_pLight->AddToWorld();
		m_pLight->Attach_UnitMtxToParent_PS( this, m_apszBoneNameTable[BONE_ATTACHPOINT_LIGHT] );
		m_pLight->GetMeshInst()->SetMeshAlpha( 1.0f );
		m_pWorldMesh->TexFlip_SetFlipPage( m_hLightTexLayer, _LIGHT_ON );
	} else {
		m_pLight->RemoveFromWorld();
		m_pWorldMesh->TexFlip_SetFlipPage( m_hLightTexLayer, _LIGHT_OFF );
	}

}


void CBotTitan::SetGunsHeatupPerSecond( f32 fVal ) {
	if( m_apWeapon[0] ) {
		((CWeaponChaingun*)m_apWeapon[0])->SetHeatupPerSecond( fVal );
	}

	if( m_aapDupWeapons[0][0] ) {
		((CWeaponChaingun*)m_aapDupWeapons[0][0])->SetHeatupPerSecond( fVal );
	}
}


void CBotTitan::SetGunsCooldownPerSecond( f32 fVal ) {
	if( m_apWeapon[0] ) {
		((CWeaponChaingun*)m_apWeapon[0])->SetCooldownPerSecond( fVal );
	}

	if( m_aapDupWeapons[0][0] ) {
		((CWeaponChaingun*)m_aapDupWeapons[0][0])->SetCooldownPerSecond( fVal );
	}
}


void CBotTitan::SetGunsCooldownOverheatPenalty( f32 fVal ) {
	if( m_apWeapon[0] ) {
		((CWeaponChaingun*)m_apWeapon[0])->SetCooldownOverheatPenalty( fVal );
	}

	if( m_aapDupWeapons[0][0] ) {
		((CWeaponChaingun*)m_aapDupWeapons[0][0])->SetCooldownOverheatPenalty( fVal );
	}
}


void CBotTitan::DeathWork( void ) {
	if( m_uLifeCycleState == BOTLIFECYCLE_DEAD ) {
		if( m_pShield && m_pShield->IsInWorld() ) {
			m_pShield->EnableShield( FALSE );
			m_pShield->RemoveFromWorld();
		}

		if( m_apWeapon[0] ) {
			m_apWeapon[0]->SetDisabledFlag( TRUE );
		}

		if( m_aapDupWeapons[0][0] ) {
			m_aapDupWeapons[0][0]->SetDisabledFlag( TRUE );
		}

		if( m_pNoLegsDebrisPileWorldMesh ) {
			_DestroyDebrisPile();
		}

	}
	CBot::DeathWork();
}


void CBotTitan::InflictDamage( CDamageData *pDamageData ) {
	if( m_pShield ) {
		m_pShield->NotifyBotDamage();
	}

	if( !pDamageData ||
		// This test is handled in CBot::InflictDamageResult() and is not needed here.
		//(pDamageData->m_Damager.pBot == this) ||
		(m_pShield && (m_pShield->NormHealth() > 0.0f) && (pDamageData->m_nDamageLocale != CDamageForm::DAMAGE_LOCALE_AMBIENT)) ) {
		return;
	}

	CBot::InflictDamage( pDamageData );
}


void CBotTitan::_HandlePowerPlant( void ) {
	f32		fLastUnitTexPos;
	CFVec2	vTex;

	//add in some power draw for our movement
	m_fPowerDraw += _POWERDRAW_RUN * fmath_Div( m_fSpeedXZ_WS, m_fMaxFlatSurfaceSpeed_WS ) * FLoop_fPreviousLoopSecs;

	if( m_uLifeCycleState == BOTLIFECYCLE_DEAD ) {
		vTex.Set( 0.0f, 0.32f );
		m_pWorldMesh->AnimTC_SetScrollST( m_hPlantTexLayer, vTex );
		if( m_pReactorEmitter ) {
			m_pReactorEmitter->Destroy();
			m_pReactorEmitter = NULL;
		}
		return;
	}

	if( Power_IsPoweringDown() || Power_IsPoweredDown() ) {
		m_fPowerDraw = 0.0f;
	}

	fLastUnitTexPos = m_fPowerPlantUnitCycle;

	if( m_fPowerPlantRecRate > (m_fPowerDraw * _POWERDRAW_REC_LAG ) )
		m_fPowerPlantRecRate -= _POWERDRAW_REC_CPS * FLoop_fPreviousLoopSecs;

	if( m_fPowerPlantRecRate < (m_fPowerDraw * _POWERDRAW_REC_LAG ) ) {
		m_fPowerPlantRecRate += _POWERDRAW_REC_CPS * FLoop_fPreviousLoopSecs;
	}
	FMATH_CLAMP( m_fPowerPlantRecRate, m_BotInfo_Titan.fMinPowerCycle, m_BotInfo_Titan.fMaxPowerCycle );

	m_fPowerPlantUnitCycle += m_fPowerPlantRecRate * FLoop_fPreviousLoopSecs;

	if( m_fPowerPlantUnitCycle >= 1.0f ) {
		if( Power_IsPoweredDown() ) {
			// we've just wrapped and we're powered down, so quit cycling
			m_fPowerPlantUnitCycle = 1.0f;
		} else {
			m_fPowerDraw -= 2.0f;
			m_fPowerPlantUnitCycle -= 1.0f;
		}
	}

	vTex.Set( 0.0f, 0.32f + m_fPowerPlantUnitCycle );
	if( vTex.y > 1.0f ) {
		vTex.y -= 1.0f;
	}
	
	m_pWorldMesh->AnimTC_SetScrollST( m_hPlantTexLayer, vTex );
	FMATH_CLAMP( m_fPowerDraw, 0.0f, _POWERDRAW_MAX_POOL );

	if( m_pReactorEmitter ) {
		f32 fUnitRate = fmath_Div( (m_fPowerPlantRecRate - m_BotInfo_Titan.fMinPowerCycle), (m_BotInfo_Titan.fMaxPowerCycle - m_BotInfo_Titan.fMinPowerCycle ) );
		CFMtx43A *pReactorMtx = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexReactor];
		m_pReactorEmitter->SetFrequencyFactor( 1.0f + (0.1f*fUnitRate) );
		m_pReactorEmitter->SetPosition( &pReactorMtx->m_vPos );
	}
}


void CBotTitan::NotifyShieldStatus( u32 uStatus, const CFVec3A &vPos ) {
	//play the sound.  It's always 3D
	switch( uStatus ) {
		case SHIELD_NOTIFY_IMPACT:
			if( m_fShieldSoundTimer < 0.0f ) {
				fsndfx_Play3D( m_BotInfo_TitanSounds.hSndShieldHit, &vPos, m_BotInfo_Titan.fSoundRadius, 1.0f, m_BotInfo_Titan.fShieldImpactSoundVolume  );
				m_fShieldSoundTimer = m_BotInfo_Titan.fShieldImpactSoundInterval;
			}
			break;

		case SHIELD_NOTIFY_UP:
			if( m_nPossessionPlayerIndex < 0 ) {
				fsndfx_Play3D( m_BotInfo_TitanSounds.hSndShieldUp, &vPos, m_BotInfo_Titan.fSoundRadius, 1.0f, m_BotInfo_Titan.fShieldImpactSoundVolume  );
			} else {
                fsndfx_Play2D( m_BotInfo_TitanSounds.hSndShieldUp );
			}
			break;

		case SHIELD_NOTIFY_DOWN:
			if( m_nPossessionPlayerIndex < 0 ) {
				fsndfx_Play3D( m_BotInfo_TitanSounds.hSndShieldDown, &vPos, m_BotInfo_Titan.fSoundRadius, 1.0f, m_BotInfo_Titan.fShieldImpactSoundVolume  );
			} else {
				fsndfx_Play2D( m_BotInfo_TitanSounds.hSndShieldDown );
			}
			break;
	}
}


void CBotTitan::NotifyFootDown( BOOL bLeftFoot, BOOL bRightFoot, f32 fUnitMag ) {
	CBot::NotifyFootDown( bLeftFoot, bRightFoot, fUnitMag );

//	if( fUnitMag > 0.1f ) {
	if( fUnitMag > 0.0f ) {
		// Get an empty damage form...
		CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

		if( pDamageForm ) {
			// Fill out the form...
			pDamageForm->m_nDamageLocale	= CDamageForm::DAMAGE_LOCALE_BLAST;
			pDamageForm->m_nDamageDelivery	= CDamageForm::DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS;
			pDamageForm->m_pDamageProfile	= m_BotInfo_Titan.pFootstepDamageProfile;
			pDamageForm->m_Epicenter_WS		= m_MountPos_WS;
			pDamageForm->m_Damager.pBot		= this;
			pDamageForm->m_Damager.nDamagerPlayerIndex = m_nPossessionPlayerIndex;
			pDamageForm->m_fNormIntensity	= fmath_Sqrt( fUnitMag );

			// submitting footdown damage form for titan mag = 
			CDamage::SubmitDamageForm( pDamageForm );
		}

		//if( m_nPossessionPlayerIndex >= 0 ) {
		//	CFCamera *pCamera = gamecam_GetActiveCamera();
		//	if( pCamera ) {
		//		pCamera->ShakeCamera( fUnitMag * m_BotInfo_Titan.fUnitInPossessionCamShake, 0.1f );
		//	}
		//}
	}
}


void CBotTitan::Power( BOOL bPowerUp, f32 fPowerOffTime, f32 fPowerOffOnSpeedMult ) {
	CBot::Power( bPowerUp, fPowerOffTime, fPowerOffOnSpeedMult );

	if( m_nPowerState == POWERSTATE_POWERING_UP ) {
		if( m_nPossessionPlayerIndex < 0 ) {
			fsndfx_Play3D( m_BotInfo_TitanSounds.hSndPowerUp, &m_MountPos_WS, m_BotInfo_Titan.fSoundRadius );			
		} else {
			fsndfx_Play2D( m_BotInfo_TitanSounds.hSndPowerUp );
		}
	} else if( m_nPowerState == POWERSTATE_WAITING_FOR_IMMOBILIZED ) {
		if( m_nPossessionPlayerIndex < 0 ) {
			fsndfx_Play3D( m_BotInfo_TitanSounds.hSndPowerDown, &m_MountPos_WS, m_BotInfo_Titan.fSoundRadius );
		} else {
			fsndfx_Play2D( m_BotInfo_TitanSounds.hSndPowerDown );
		}
	}
}


void CBotTitan::Possess( s32 nPlayerIndex, f32 fPossessionArmorModifier ) {
	CBot::Possess( nPlayerIndex, fPossessionArmorModifier );

	// set up our HUD
	CHud2* pHud = CHud2::GetHudForPlayer(nPlayerIndex);
	pHud->SetDrawFlags( CHud2::DRAW_AMMOBOXES | CHud2::DRAW_BATTERIES | CHud2::DRAW_RADAR | CHud2::DRAW_SELFDESTRUCT | CHud2::DRAW_WEAPONBOXES | CHud2::DRAW_ENABLEOVERRIDE );
	CFColorRGBA startColor, endColor;
	startColor.OpaqueYellow();
	endColor.OpaqueRed();
	pHud->OverrideAmmoData( CHud2::RIGHT, CHud2::OVERRIDE_METER_MIL, &m_fChaingunHeat, TRUE, &startColor, &endColor );
	startColor.OpaqueWhite();
	endColor.OpaqueBlue();
	pHud->OverrideAmmoData( CHud2::LEFT, CHud2::OVERRIDE_METER_MIL, &m_fShieldHealth, TRUE, &startColor, &endColor );
	pHud->OverrideWeaponBox( TRUE, CHud2::LEFT, "RLauncher L1", TRUE );
	pHud->OverrideWeaponBox( TRUE, CHud2::RIGHT, "Blaster L1" );

	//set the chainguns back to default...
	if( m_apWeapon[0] ) {
		((CWeaponChaingun*)m_apWeapon[0])->RestoreDefaultValues();
	}

	if( m_aapDupWeapons[0][0] ) {
		((CWeaponChaingun*)m_aapDupWeapons[0][0])->RestoreDefaultValues();
	}

	_EnableLight( TRUE );
}



void CBotTitan::_CheckServoSounds( void ) {	
#if 0
#define __SERVO1		0.1f
#define __SERVO2		0.6f
	BOOL bPlayServoSound = FALSE;
	if( m_fClampedNormSpeedAlongSurface_WS < 0.1f ) {
		return;
	}

	ftext_DebugPrintf( 0.2f, 0.3f, "~w1m_fUnitFeetAnim = %0.2f, speed = %0.2f", m_fUnitFeetAnim, m_fClampedNormSpeedAlongSurface_WS );
	f32	 fVolume = 1.0f - (m_fClampedNormSpeedAlongSurface_WS);

	if( fVolume <= 0.25f ) {
		return;
	}

	if( !IsMovingBackwards() ) {
		if( ((m_fLastUnitFeetAnim < __SERVO1) && (m_fUnitFeetAnim > __SERVO1)) ) {
			bPlayServoSound = TRUE;
		} else if( ((m_fLastUnitFeetAnim < __SERVO2) && (m_fUnitFeetAnim > __SERVO2)) ) {
			bPlayServoSound = TRUE;
		}
	} else {
		if( ((m_fLastUnitFeetAnim > __SERVO1) && (m_fUnitFeetAnim + 1.0f < __SERVO1)) ) {
			bPlayServoSound = TRUE;
		} else if( ((m_fLastUnitFeetAnim > __SERVO2) && (m_fUnitFeetAnim + 1.0f < __SERVO2)) ) {
			bPlayServoSound = TRUE;
		}
	}

	if( bPlayServoSound ) {
		
	// if we're taking a step, play a servo sound
		/*if( m_fClampedNormSpeedAlongSurface_WS > 0.1f ) */{
			if( m_nPossessionPlayerIndex < 0 ) {
				fsndfx_Play3D( m_BotInfo_Titan.hSndServo, &m_MountPos_WS, m_BotInfo_Titan.fSoundRadius, 1.0f, fVolume );
			} else {
				fsndfx_Play2D( m_BotInfo_Titan.hSndServo, fVolume );
			}
		}
	}

#undef __SERV01
#undef __SERV02
#endif
}

void CBotTitan::DebugDraw( CBotTitan *pTitan ) {
	//CFSphere sphere;
	//sphere.m_Pos.Set( pTitan->m_pBotInfo_Gen->fCollSphere1X_MS, pTitan->m_pBotInfo_Gen->fCollSphere1Y_MS, pTitan->m_pBotInfo_Gen->fCollSphere1Z_MS );
	//sphere.m_Pos.Add( pTitan->m_MountPos_WS.v3 );
	//sphere.m_fRadius = pTitan->m_pBotInfo_Gen->fCollSphere1Radius_MS;

	//fdraw_FacetedWireSphere( &(sphere.m_Pos), sphere.m_fRadius, 4, 4, &FColor_MotifRed );

//	fdraw_FacetedWireSphere( &pTitan->m_TargetedPoint_WS.v3, 4.0f, 4, 4, &FColor_MotifRed );
//	fdraw_FacetedWireSphere( &pTitan->m_ControlBot_TargetPoint_WS.v3, 3.0f, 4, 4, &FColor_MotifBlue );
//	fdraw_FacetedWireSphere( &pTitan->m_vActualAimPt_WS.v3, 3.5f, 4, 4, &FColor_MotifGreen );
	
}


void CBotTitan::SuspendShield( BOOL bSuspend ) { 
	FASSERT( IsCreated() );

	m_bShieldSuspended = bSuspend;
	if( m_pShield ) {
		m_pShield->Suspend( bSuspend );
	}
}