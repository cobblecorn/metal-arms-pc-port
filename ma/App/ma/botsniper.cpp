//////////////////////////////////////////////////////////////////////////////////////
// botsniper.cpp - Sniper
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
// 03/11/03 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "botsniper.h"

#include "fanim.h"
#include "botanim.h"
#include "fmesh.h"
#include "fresload.h"
#include "meshtypes.h"
#include "protrack.h"
#include "meshentity.h"
#include "reticle.h"
#include "player.h"
#include "muzzleflash.h"
#include "weapon.h"
#include "potmark.h"
#include "Ai\AIEnviro.h"
//#include "fpad.h"
#include "botpart.h"
#include "gamecam.h"
#include "weapon_scope.h"
#include "grapple.h"
#include "gamepad.h"
#include "frenderer.h"
#include "fvtxpool.h"

#include "picklevel.h" //METMP 

#define	_BOTINFO_FILENAME	( "b_sniper" )
#define _MESH_NAME			( "grmnsnipr00" )
#define _TRACER_TEX			( "TFMJshot001" )
#define _LASERSIGHT_TEX		( "twdm_lit01" )

#define _RETICLE_Y			( 0.2f )
#define _SCOPE_EUK			( 1 )	// GameCube w_scope.csv has valid levels 0 and 1.
#define _GRAPPLE_FLY_SPEED	( 300.0f )
#define _BULLETTRAIL_TEX	( "TFMNsmoke01" )

static CBotSniperBuilder _BotSniperBuilder;

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotSniper
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************


BOOL CBotSniper::m_bSystemInitialized = FALSE;
u32 CBotSniper::m_nBotClassClientCount = 0;

CBotSniper::BotInfo_Gen_t			CBotSniper::m_BotInfo_Gen;
CBotSniper::BotInfo_MountAim_t		CBotSniper::m_BotInfo_MountAim;
CBotSniper::BotInfo_Walk_t			CBotSniper::m_BotInfo_Walk;
CBotSniper::BotInfo_Jump_t			CBotSniper::m_BotInfo_Jump;
CBotSniper::BotInfo_Weapon_t		CBotSniper::m_BotInfo_Weapon;

CBotSniper::BotInfo_Sniper_t		CBotSniper::m_BotInfo_Sniper = {
	2.0f,		// f32 fTracerWidth;
	5000.0f,	// f32 fTracerLength;
	3000.0f,	// f32 fTracerSpeed;
	5,			// f32 uMaxTracers;
	2.0f,		// f32 fReloadTime;

	0.394f,		// f32 fWallClingPosX
	7.68f,		// f32 fWallClingPosY
	0.0f,		// f32 fWallClingPosZ
};

CBotPartPool* CBotSniper::m_pPartPool = NULL;
CBotSniper* CBotSniper::m_pCBSniper = NULL;
CFVec3A CBotSniper::m_GroinVecY_WS;
CFVec3A CBotSniper::m_vGrappleClingPosWall_MS;

CBotAnimStackDef CBotSniper::m_AnimStackDef;

TracerGroupHandle_t CBotSniper::m_hTracerGroup = TRACER_NULLGROUPHANDLE;	
TracerDef_t			CBotSniper::m_TracerDef;	
CFTexInst			CBotSniper::m_TracerTexInst;

BulletTrail_t*		CBotSniper::m_paBulletTrails = NULL;
#if FANG_WINGC
CDamageProfile*		CBotSniper::m_pPortShotDamageNPC = NULL;
CDamageProfile*		CBotSniper::m_pPortShotDamagePlayer = NULL;
#endif
u32					CBotSniper::m_uNumActiveBulletTrails = 0;
CFTexInst			CBotSniper::m_BulletTrailTex;

CFVec3A				CBotSniper::m_avLaserPt[MAX_LASER_SIGHTS*4];
u32					CBotSniper::m_uNumSightsActive = 0;					
CFTexInst			CBotSniper::m_LaserSightTex;




//////////////////////
//STATIC FNs

BOOL CBotSniper::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );

	m_bSystemInitialized = TRUE;

	m_nBotClassClientCount = 0;

	m_vGrappleClingPosWall_MS.Set( 0.394f, 7.68f, 0.0f );
	m_vGrappleClingPosWall_MS.Negate();

	m_paBulletTrails = NULL;

	return TRUE;
}


void CBotSniper::UninitSystem( void ) {
	if( !m_bSystemInitialized ) {
		return;
	}

	FASSERT( m_nBotClassClientCount == 0 );

	m_bSystemInitialized = FALSE;
}


CBotSniper::CBotSniper() : CBot() {
	m_pInventory	= NULL;
	m_pWorldMesh	= NULL;
}


CBotSniper::~CBotSniper() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CBotSniper::Create( s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	if( !ClassHierarchyLoadSharedResources() ) {
		// Failure! (resources have already been cleaned up, so we can bail now)
		return FALSE;
	}

	// Get pointer to the leaf class's builder object...
	CBotSniperBuilder *pBuilder = (CBotSniperBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_BotSniperBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOTSNIPER );
	}

	// Create an entity...
	return CBot::Create( &m_BotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}


void CBotSniper::_ClearDataMembers( void ) {
	m_fCollCylinderHeight_WS	= 8.0f;
	m_fCollCylinderRadius_WS	= 4.0f;

	m_pBotInfo_Gen		= &m_BotInfo_Gen;
	m_pBotInfo_MountAim	= &m_BotInfo_MountAim;
	m_pBotInfo_Walk		= &m_BotInfo_Walk;
	m_pBotInfo_Jump		= &m_BotInfo_Jump;
	m_pBotInfo_Weapon	= &m_BotInfo_Weapon;

	m_fMaxFlatSurfaceSpeed_WS	= m_pBotInfo_Walk->fMaxXlatVelocity * m_fRunMultiplier;
	m_fMountPitchMax_WS			= m_pBotInfo_MountAim->fMountPitchDownLimit;
	m_fMountPitchMin_WS			= m_pBotInfo_MountAim->fMountPitchUpLimit;
	m_pMoveIdentifier			= &m_fFireTimer;

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
	m_anAnimStackIndex[ASI_STOOP]				= -1; //ANIMTAP_STOOP_SUMMER;
	m_anAnimStackIndex[ASI_STAND_LIMP_LEFT]		= -1; //ANIMTAP_STAND_LIMP_LEFT;
	m_anAnimStackIndex[ASI_STAND_LIMP_RIGHT]	= -1; //ANIMTAP_STAND_LIMP_RIGHT;
	m_anAnimStackIndex[ASI_LIMP_LEFT]			= -1; //ANIMTAP_LIMP_LEFT;
	m_anAnimStackIndex[ASI_LIMP_RIGHT]			= -1; //ANIMTAP_LIMP_RIGHT;
	m_anAnimStackIndex[ASI_HOP_LEFT]			= -1;
	m_anAnimStackIndex[ASI_HOP_RIGHT]			= -1;
	m_anAnimStackIndex[ASI_STARTLE]				= -1;
	m_anAnimStackIndex[ASI_ROLL_LEFT]			= -1;
	m_anAnimStackIndex[ASI_ROLL_RIGHT]			= -1;
	m_anAnimStackIndex[ASI_DOZE_LOOP]			= -1; //ANIMTAP_SLEEP;
	m_anAnimStackIndex[ASI_NAPJERK]				= -1; //ANIMTAP_SLEEP;
	m_anAnimStackIndex[ASI_WAKE]				= -1; //ANIMTAP_WAKE;

	m_pnEnableBoneNameIndexTableForSummer_Normal		= m_anEnableBoneNameIndexTableForSummer_Normal;
	m_pnEnableBoneNameIndexTableForSummer_TetherShock	= m_anEnableBoneNameIndexTableForSummer_Normal;

	m_apWeapon[1] = NULL;
	fforce_NullHandle( &m_hForce );
	m_eSnipeState = SNIPESTATE_NONE;

	m_fFireTimer = 0.0f;
#if FANG_WINGC
	m_fPortDiagTimer = 0.0f;
	m_bPortDiagDeathLogged = FALSE;
#endif
	m_fUnitAim = 0.0f;

	m_bGrappleRecoilOn = FALSE;
	m_bValidGrapplePos = FALSE;
	m_eGrappleState = GRAPPLESTATE_NONE;
	m_fGrappleRecoil = 0.0f;
	m_fModelPitch = FMATH_HALF_PI;

	m_pGrapple = NULL;
}


/////////////////////
//CLASS HIERARCHY FNs

BOOL CBotSniper::ClassHierarchyLoadSharedResources( void ) {
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
		// Resources already loaded...
		return TRUE;
	}

	// Resources not yet loaded...
	FResFrame_t frame = fres_GetFrame();
	FTexDef_t *pTexDef;
    
	if( !ReadBotInfoFile( m_aGameDataMap, _BOTINFO_FILENAME ) ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyLoadSharedResources():  Error parsing %s\n", _BOTINFO_FILENAME );
		goto _ExitWithError;
	}

	// Retail GameCube b_sniper's MountAim table has 11 fields. The shared bot
	// vocabulary has a final allowable-lock-angle field that this table omits.
	// Other retail bot tables use 20 degrees for that field, so use the same
	// threshold while leaving the retail row itself untouched.
	{
		static const s8 anMountAimFieldMap11[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, -1 };
		static const s8 anMountAimFieldMap12[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
		FMemFrame_t hMemFrame = fmem_GetFrame();
		FGameDataFileHandle_t hFile = fgamedata_LoadFileToFMem( _BOTINFO_FILENAME );
		if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
			DEVPRINTF( "CBotSniper::ClassHierarchyLoadSharedResources(): Could not load %s.csv for MountAim compatibility.\n", _BOTINFO_FILENAME );
			fmem_ReleaseFrame( hMemFrame );
			goto _ExitWithError;
		}

		FGameDataTableHandle_t hMountAim = fgamedata_GetFirstTableHandle( hFile, "MountAim" );
		if( hMountAim == FGAMEDATA_INVALID_TABLE_HANDLE ) {
			DEVPRINTF( "CBotSniper::ClassHierarchyLoadSharedResources(): Could not find MountAim in %s.csv.\n", _BOTINFO_FILENAME );
			fmem_ReleaseFrame( hMemFrame );
			goto _ExitWithError;
		}

		u32 nMountAimFields = fgamedata_GetNumFields( hMountAim );
		const s8 *panMountAimFieldMap = NULL;
		if( nMountAimFields == 11 ) {
			m_BotInfo_MountAim.fAllowableLockAngleCos = fmath_Cos( FMATH_DEG2RAD( 20.0f ) );
			panMountAimFieldMap = anMountAimFieldMap11;
		} else if( nMountAimFields == 12 ) {
			panMountAimFieldMap = anMountAimFieldMap12;
		} else {
			DEVPRINTF( "CBotSniper::ClassHierarchyLoadSharedResources(): MountAim in %s.csv has %u fields; expected 11 or 12.\n",
					   _BOTINFO_FILENAME, nMountAimFields );
			fmem_ReleaseFrame( hMemFrame );
			goto _ExitWithError;
		}

		if( !fgamedata_GetTableDataRemapped( hMountAim, m_aBotInfoVocab_MountAim, panMountAimFieldMap, 12,
															&m_BotInfo_MountAim, sizeof( m_BotInfo_MountAim ) ) ) {
			DEVPRINTF( "CBotSniper::ClassHierarchyLoadSharedResources(): Could not parse MountAim in %s.csv.\n", _BOTINFO_FILENAME );
			fmem_ReleaseFrame( hMemFrame );
			goto _ExitWithError;
		}
		fmem_ReleaseFrame( hMemFrame );
	}

	// Build animation stack...
	if( !_BuildAnimStackDef() ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyLoadSharedResources():  Error creating bot anim stack def\n" );
		goto _ExitWithError;
	}

	// initialize tracers...
	FASSERT( m_hTracerGroup == TRACER_NULLGROUPHANDLE );
    
	m_TracerDef.pUser = NULL;
	m_TracerDef.pFcnKillCallback = _TracerKilledCallback;
	m_TracerDef.pFcnBuildSkipList = _TracerBuildTrackerSkipList;
	m_TracerDef.fWidth_WS = m_BotInfo_Sniper.fTracerWidth;
	m_TracerDef.fLength_WS = m_BotInfo_Sniper.fTracerLength;
	m_TracerDef.fSpeed_WS = m_BotInfo_Sniper.fTracerSpeed;
	m_TracerDef.fMaxTailDist_WS = m_BotInfo_MountAim.fMaxTargetRange;
	m_TracerDef.fBeginDeathUnitFade_WS = 0.25f;
	m_TracerDef.uFlags = 0;
	m_TracerDef.ColorRGBA.OpaqueWhite();

	pTexDef = (FTexDef_t*)(fresload_Load(FTEX_RESNAME, _TRACER_TEX ) );
	if( pTexDef == NULL ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyLoadSharedResources() : Could not load tracer texture '%s'.\n", _TRACER_TEX );
		goto _ExitWithError;
	} else {
		m_TracerTexInst.SetTexDef( pTexDef );
	}
	m_hTracerGroup = tracer_CreateGroup( &m_TracerTexInst, m_BotInfo_Sniper.uMaxTracers );
	if( m_hTracerGroup == TRACER_NULLGROUPHANDLE ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyLoadSharedResources() : Could not create tracer group.\n" );
		goto _ExitWithError;
	}

#if FANG_WINGC
	// The cut Sniper's tracer never delivered damage (its kill callback was empty) and retail has no
	// Sniper damage profile: use the Rivet Gun's, L2 for enemy Snipers and L3 when a player drives one.
	m_pPortShotDamageNPC = CDamage::FindDamageProfile( "RivetL2" );
	m_pPortShotDamagePlayer = CDamage::FindDamageProfile( "RivetL3" );
#endif

	// load and init bullet trails
	FASSERT( m_paBulletTrails == NULL );
	m_paBulletTrails = (BulletTrail_t*)fres_Alloc( sizeof( BulletTrail_t ) * m_BotInfo_Sniper.uMaxTracers );
	if( !m_paBulletTrails ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyLoadSharedResources() : Out of memory creating bullet trails.\n", _TRACER_TEX );
		goto _ExitWithError;
	}

	for( u32 i=0; i<m_BotInfo_Sniper.uMaxTracers; i++ ) {
		m_paBulletTrails[i].uTimeCreated = 0;
	}
	m_uNumActiveBulletTrails = 0;

	pTexDef = (FTexDef_t*)fresload_Load( FTEX_RESNAME, _BULLETTRAIL_TEX );
	if( !pTexDef ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyLoadSharedResources() : Could not load bullet trail texture '%s'.\n", _BULLETTRAIL_TEX );
		goto _ExitWithError;
	} else {
		m_BulletTrailTex.SetTexDef( pTexDef );
	}

	// laser sights...
	m_uNumSightsActive = 0;	

	pTexDef = (FTexDef_t*)fresload_Load( FTEX_RESNAME, _LASERSIGHT_TEX );
	if( !pTexDef ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyLoadSharedResources() : Could not load laser sight texture '%s'.\n", _LASERSIGHT_TEX );
		goto _ExitWithError;
	} else {
		m_LaserSightTex.SetTexDef( pTexDef );
	}



	return TRUE;

	// Error:
_ExitWithError:
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( frame );

	return FALSE;
}


void CBotSniper::ClassHierarchyUnloadSharedResources( void ) {
	FASSERT( m_nBotClassClientCount > 0 );

	m_nBotClassClientCount--;

	if( m_nBotClassClientCount > 0 ) {
		return;
	}

	if( m_hTracerGroup != TRACER_NULLGROUPHANDLE ) {
		tracer_DestroyGroup( m_hTracerGroup );
		m_hTracerGroup = TRACER_NULLGROUPHANDLE;
	}

	m_AnimStackDef.Destroy();

	// all snipers are gone, so make sure this is set back to 0
	m_uNumActiveBulletTrails = 0;
	if( m_paBulletTrails ) {
		m_paBulletTrails = NULL;
	}

	m_uNumSightsActive = 0;
	
	CBot::ClassHierarchyUnloadSharedResources();
}


BOOL CBotSniper::ClassHierarchyBuild( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	FMesh_t *pMesh;
	FMeshInit_t meshinit;
	FResFrame_t frame;

	s32 nBoneIdx;

	frame = fres_GetFrame();

	// get pointer to leaf class's builder object...
	CBotSniperBuilder *pBuilder = (CBotSniperBuilder*)GetLeafClassBuilder();

	// set input params for CBot creation...
	pBuilder->m_pBotDef = &m_BotDef;

	// build parent...
	if( !CBot::ClassHierarchyBuild() ) {
		goto _ExitWithError;
	}

	// set defaults...
	_ClearDataMembers();

	// init from builder...
	// nothing yet

	// load mesh...
	pMesh = (FMesh_t*)fresload_Load( FMESH_RESTYPE, _MESH_NAME );
	if( pMesh == NULL ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyBuild(): Could not find mesh '%s'\n", _MESH_NAME );
		goto _ExitWithError;
	}

	// create worldmesh...
	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyBuild(): Unable to allocate CFWorldMesh\n" );
		goto _ExitWithError;
	}

	// init worldmesh...
	meshinit.pMesh = pMesh;
	meshinit.fCullDist = FMATH_MAX_FLOAT;
	meshinit.Mtx.Set( pBuilder->m_EC_Mtx_WS );
	meshinit.nFlags = 0;
	m_pWorldMesh->Init( &meshinit );

	m_pWorldMesh->m_nUser = MESHTYPES_ENTITY;
	m_pWorldMesh->m_pUser = this;
	m_pWorldMesh->SetUserTypeBits( TypeBits() );
	m_pWorldMesh->UpdateTracker();
	m_pWorldMesh->RemoveFromWorld();


	// build animation stack...
	if( !m_Anim.Create( &m_AnimStackDef, m_pWorldMesh ) ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyBuild(): Unable to create animation stack\n" );
		goto _ExitWithError;
	}

	SetControlValue( ANIMCONTROL_STAND, 1.0f );
	UpdateUnitTime( ANIMTAP_STAND, fmath_RandomFloat() );

	// get bone indices & set up callbacks...
	m_nBoneIdxGroin = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_GROIN] );
	if( m_nBoneIdxGroin < 0 ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[BONE_GROIN] );
		goto _ExitWithError;
	}

	m_nBoneIdxTorso = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_TORSO] );
	if( m_nBoneIdxTorso < 0 ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[BONE_TORSO] );
		goto _ExitWithError;
	}

	m_nBoneIdxHead = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_HEAD] );
	if( m_nBoneIdxHead < 0 ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[BONE_HEAD] );
		goto _ExitWithError;
	}

	m_nBoneIdxPriFire = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_GUN_BARREL] );
	if( m_nBoneIdxPriFire < 0 ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[BONE_GUN_BARREL] );
		goto _ExitWithError;
	}

	m_nBoneIdxGrapple = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_GRAPPLE_ATTACH] );
	if( m_nBoneIdxGrapple < 0 ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[BONE_GRAPPLE_ATTACH] );
		goto _ExitWithError;
	}

	m_nBoneIdxRib1 = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_RIB1] );
	if( m_nBoneIdxRib1 < 0 ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[BONE_RIB1] );
		goto _ExitWithError;
	}

	m_nBoneIdxRib2 = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_RIB2] );
	if( m_nBoneIdxRib2 < 0 ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[BONE_RIB2] );
		goto _ExitWithError;
	}

	m_nBoneIdxRib3 = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_RIB3] );
	if( m_nBoneIdxRib3 < 0 ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[BONE_RIB3] );
		goto _ExitWithError;
	}

	m_nBoneIdxRib4 = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_RIB4] );
	if( m_nBoneIdxRib4 < 0 ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[BONE_RIB4] );
		goto _ExitWithError;
	}
	
	m_nBoneIdxRightArm = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ARM_UPPER_R] );
	if( m_nBoneIdxRightArm < 0 ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[BONE_ARM_UPPER_R] );
		goto _ExitWithError;
	}


	// set up bone callbacks
	m_Anim.m_pAnimCombiner->DisableAllBoneCallbacks();
	m_Anim.m_pAnimCombiner->SetBoneCallback( _AnimBoneCallback );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_GROIN] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_TORSO] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_HEAD] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_RIB1] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_RIB2] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_RIB3] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_RIB4] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ARM_UPPER_R] );

	EnableControlSmoothing(	ANIMCONTROL_JUMP_LAUNCH );
	EnableControlSmoothing(	ANIMCONTROL_JUMP_FLY );
	EnableControlSmoothing(	ANIMCONTROL_JUMP_LAND_UPPER );
	EnableControlSmoothing(	ANIMCONTROL_JUMP_LAND_LOWER );

	// set up the summer
	if( !m_AnimManFrameSummer.Create( ANIMBONE_COUNT, m_apszSummerBoneNameTable ) ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyBuild(): Could not create recoil animation summer.\n" );
		goto _ExitWithError;
	}

	AttachAnim( ANIMTAP_SUMMER, &m_AnimManFrameSummer );
	m_AnimManFrameSummer.Reset();
	UpdateBoneMask( ANIMTAP_SUMMER, m_aBoneEnableIndices_Summer, TRUE );
	SetControlValue( ANIMCONTROL_SUMMER, 0.0f );

	SetMaxHealth();

	if( m_nPossessionPlayerIndex >= 0 ) {
		Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.SetNormOrigin( 0.0f, _RETICLE_Y );
		Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.SetType( CReticle::TYPE_DROID_STANDARD );
		Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.EnableDraw( TRUE );
	}

	// initialize part pool
	//if( !m_pPartMgr->Create( this, &m_pPartPool, _BOTPART_FILENAME, PART_INSTANCE_COUNT_PER_TYPE, LIMB_TYPE_COUNT ) ) {
	//	FASSERT_NOW;
	//	goto _ExitWithError;
	//}

	// Find approx eye point...
	nBoneIdx = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
	if( nBoneIdx < 0 ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyBuild(): Could not locate approx eye point bone '%s'.\n", m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
		m_pApproxEyePoint_WS = &m_MtxToWorld.m_vPos;
	} else {
		m_pApproxEyePoint_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIdx]->m_vPos;
	}

	// Find gaze direction...
	nBoneIdx = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_HEAD ] );
	if( nBoneIdx < 0 ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyBuild(): Could not locate gaze bone '%s'.\n", m_apszBoneNameTable[ BONE_HEAD ] );
		m_pGazeDir_WS = &m_MtxToWorld.m_vFront;
	} else {
		m_pGazeDir_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIdx]->m_vFront;
	}

	m_pAISteerMtx = &m_MtxToWorld;

	// initialize mtx palette...
	AtRestMatrixPalette();

	// Create tag points...
	if( !TagPoint_CreateFromBoneArray( m_anTagPointBoneNameIndexArray, m_apszBoneNameTable ) ) {
		goto _ExitWithError;
	}

	// Set up data port stuff...
	DataPort_SetupTetherShockInfo();

	// create the scope.  It needs to be weapon 1 (that's where bot-layer code expects it to be)
	m_apWeapon[1] = fnew CWeaponScope();
	if( !m_apWeapon[1] ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyBuild():  Could not allocate CWeaponScope.\n" );
		goto _ExitWithError;
	}
	((CWeaponScope*)m_apWeapon[1])->Create();
	((CWeaponScope*)m_apWeapon[1])->SetUpgradeLevel( _SCOPE_EUK );
	((CWeaponScope*)m_apWeapon[1])->EnableAutoWork( FALSE );
	((CWeaponScope*)m_apWeapon[1])->RemoveFromWorld();
	((CWeaponScope*)m_apWeapon[1])->SetOwner( this );


	// init the grappling hook
	m_pGrapple = fnew CGrapple;
	if( !m_pGrapple ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyBuild():  Out of memory creating grapple.\n" );
		goto _ExitWithError;
	}

	if( !m_pGrapple->Create( this, m_apszBoneNameTable[BONE_GRAPPLE_ATTACH] ) ) {
		DEVPRINTF( "CBotSniper::ClassHierarchyBuild():  Couldn't create grapple.\n" );
		goto _ExitWithError;
	}

	m_pGrapple->RemoveFromWorld();

	return TRUE;

	// Error:
_ExitWithError:
	FASSERT_NOW;
	Destroy();
	fres_ReleaseFrame( frame );
	return FALSE;
}


void CBotSniper::ClassHierarchyDestroy( void ) {
	m_Anim.Destroy();

	if( m_apWeapon[1] ) {
		m_apWeapon[1]->Destroy();
		fdelete( m_apWeapon[1] );
		m_apWeapon[1] = NULL;
	}

	if( m_pWorldMesh ) {
		fdelete( m_pWorldMesh );
		m_pWorldMesh = NULL;
	}

	if( m_pGrapple ) {
		m_pGrapple->Destroy();
		fdelete( m_pGrapple );
		m_pGrapple = NULL;
	}

	CBot::ClassHierarchyDestroy();
}


void CBotSniper::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CBot::ClassHierarchyAddToWorld();
	
	m_pWorldMesh->UpdateTracker();

	m_apWeapon[1]->AddToWorld();
	m_pGrapple->AddToWorld();
}


void CBotSniper::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	fforce_Kill( &m_hForce );

	m_pWorldMesh->RemoveFromWorld();

	CBot::ClassHierarchyRemoveFromWorld();
	m_apWeapon[1]->RemoveFromWorld();
	m_pGrapple->RemoveFromWorld();
}


BOOL CBotSniper::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t frame = fres_GetFrame();

	if( !CBot::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	EnableOurWorkBit();

	return TRUE;

_ExitWithError:
	FASSERT_NOW;
	Destroy();
	fres_ReleaseFrame( frame );
	return FALSE;
}


CEntityBuilder* CBotSniper::GetLeafClassBuilder( void ) {
	return &_BotSniperBuilder;
}


void CBotSniper::ClassHierarchyWork( void ) {
	FASSERT( IsSystemInitialized() );

	CFVec3A vTmp;

	CBot::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}

	PROTRACK_BEGINBLOCK( "Sniper" );

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	////////////////////////////////////////
	//DBG STUFF

	m_BotInfo_Walk.fMaxXlatVelocity				= 35.0f;
	m_BotInfo_Walk.fNormVelocityStepSize		= 2.0f;

	m_BotInfo_Walk.fMinWalkVelocity				= 4.0f;
	m_BotInfo_Walk.fDeltaFeetAnimPerFootWalk	= 0.084f;
	m_BotInfo_Walk.fDeltaFeetAnimPerFootRun		= 0.04f;
	m_BotInfo_Walk.fMinRunBlendVelocity			= 10.0f;
	m_BotInfo_Walk.fMaxRunBlendVelocity			= 25.0f;


	m_BotInfo_Walk.fInvMaxXlatVelocity	= 1.0f/m_BotInfo_Walk.fMaxXlatVelocity;
	m_pBotInfo_Walk->fMinWalkNormVelocity = m_pBotInfo_Walk->fMinWalkVelocity / m_BotInfo_Walk.fMaxXlatVelocity;
	m_BotInfo_Walk.fMinRunBlendNormVelocity = m_pBotInfo_Walk->fMinRunBlendVelocity / m_BotInfo_Walk.fMaxXlatVelocity;
	m_BotInfo_Walk.fMaxRunBlendNormVelocity = m_pBotInfo_Walk->fMaxRunBlendVelocity / m_BotInfo_Walk.fMaxXlatVelocity;


	ftext_DebugPrintf( 0.5f, 0.2f, "~w1speed: %0.2f", m_fSpeed_WS );
	ftext_DebugPrintf( 0.5f, 0.23f, "~w1walk: %0.2f", GetControlValue( ANIMCONTROL_WALK ) );
	ftext_DebugPrintf( 0.5f, 0.26f, "~w1run: %0.2f", GetControlValue( ANIMCONTROL_RUN ) );

	//DBG
	////////////////////////////////////////
#endif

	Power_Work();
	DataPort_Work();

	ParseControls();
	if( m_eGrappleState == GRAPPLESTATE_FLYING ) {
		if( m_fControls_Fire2 > 0.25f ) {
			_BreakGrappleLine();
		}
		ZeroControls();
	}

	// save a copy of previous frame info...
	m_MountPrevPos_WS	= m_MountPos_WS;
	m_nPrevState		= m_nState;

	// add in any velocity impulse...
	BOOL bImpulseApplied = HandleVelocityImpulses();

	// update position...
	vTmp.Mul( m_Velocity_WS, FLoop_fPreviousLoopSecs );
	m_MountPos_WS.Add( vTmp );

	// pitch & yaw...
	if( IsClinging() ) {
		_HandleClingingYaw();
		_HandleClingingPitch();
	} else {
		HandlePitchMovement();
		HandleYawMovement();
	}

	HandleHeadLook();

	WS2MS( m_Velocity_MS, m_Velocity_WS );

	if( bImpulseApplied ) {
		VelocityHasChanged();
	}

	// Update translation analog stick vectors from raw controller data...
	ComputeXlatStickInfo();

	m_fVerticalVelocity = m_Velocity_WS.y;
	// Collide with the world below our feet...
	PROTRACK_BEGINBLOCK("Coll");
		HandleCollision();
	PROTRACK_ENDBLOCK();//"Coll"

	if( m_eGrappleState != GRAPPLESTATE_CLINGING ) {
		switch( m_nState ) {
			case STATE_GROUND:
				PROTRACK_BEGINBLOCK("GroundXlat");
					HandleGroundTranslation();
					_HandleJumping();
				PROTRACK_ENDBLOCK();//"GroundXlat");

				PROTRACK_BEGINBLOCK("GroundAnim");
					HandleGroundAnimations();
				PROTRACK_ENDBLOCK();//"GroundAnim");
				break;


			case STATE_AIR:
				if( IsSleeping() ) {
					break;
				}
				PROTRACK_BEGINBLOCK("AirXlat");
					HandleAirTranslation();
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
		HandleHipsAnimation();
	PROTRACK_ENDBLOCK();//"CommonAnim");

	PROTRACK_BEGINBLOCK("Weapon");
		HandleTargeting();
		_CalcLaserPt();
		m_apWeapon[1]->Work();
	PROTRACK_ENDBLOCK();//"WeaponAnim");

	PROTRACK_BEGINBLOCK("Grapple");
	_GrappleWork();
	PROTRACK_ENDBLOCK();//"Grapple");

	_UpdateMatrices();

	_WeaponWork();
	_HandleFireAnimation();

	

	if( IsDeadOrDying() ) {
		DeathWork();
	}

#if FANG_WINGC
	// -port-diag: where each recovered Sniper is and how it is doing, every 5 s, and when it goes down.
	if( Fang_bPortDiag ) {
		m_fPortDiagTimer -= FLoop_fPreviousLoopSecs;
		if( m_fPortDiagTimer <= 0.0f || (IsDeadOrDying() && !m_bPortDiagDeathLogged) ) {
			m_fPortDiagTimer = 1.0f;
			if( IsDeadOrDying() ) {
				m_bPortDiagDeathLogged = TRUE;
			}
			DEVPRINTF( "PORT-SNIPER '%s' %s at (%.0f, %.0f, %.0f) vel=(%.0f, %.0f, %.0f) state=%d grapple=%d fire=%.1f/%.1f health=%.2f\n",
					   Name() ? Name() : "?", IsDeadOrDying() ? "down" : "alive", m_MountPos_WS.x, m_MountPos_WS.y, m_MountPos_WS.z,
					   m_Velocity_WS.x, m_Velocity_WS.y, m_Velocity_WS.z, (int)m_nState, (int)m_eGrappleState,
					   m_fControls_Fire1, m_fControls_Fire2, NormHealth() );
		}
	}
#endif

	_HandleWeaponAnimations();
	_HandleSummerAnimations();
	m_pGrapple->Work();

	_DebugAnimData();

	PROTRACK_ENDBLOCK(); //"Sniper"
}


void CBotSniper::AppendTrackerSkipList( u32& nTrackerSkipListCount, CFWorldTracker **apTrackerSkipList ) {
	FASSERT( IsCreated() );
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );

	apTrackerSkipList[nTrackerSkipListCount++] = m_pWorldMesh;

	if( m_pDataPortMeshEntity ) {
		m_pDataPortMeshEntity->AppendTrackerSkipList( nTrackerSkipListCount, apTrackerSkipList );
	}
}


const CFVec3A* CBotSniper::GetApproxEyePoint( void ) const {
	FASSERT( IsCreated() );

	return m_pApproxEyePoint_WS;
}


void CBotSniper::UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup ) {
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

void CBotSniper::_EnterSnipingMode( void ) {
	m_eSnipeState = SNIPESTATE_BEGINNING;
}

void CBotSniper::_ExitSnipingMode( void ) {
	m_nScopeState = SCOPESTATE_NONE;
	SetControlValue( ANIMCONTROL_FIRE, 0.0f );
	SetControlValue( ANIMCONTROL_FIRE_LOWER, 0.0f );
	
	SetControlValue( ANIMCONTROL_FIRE_END, 1.0f );
	SetControlValue( ANIMCONTROL_FIRE_END_LOWER, 1.0f );
	
	UpdateUnitTime( ANIMTAP_FIRE_END, 0.0f );

	m_eSnipeState = SNIPESTATE_ENDING;
}

#define _FIRE_START_BLEND_RATE		( 3.0f )
#define _FIRE_START_ANIM_RATE		( 1.0f )
#define _FIRE_END_BLEND_RATE		( 3.0f )
#define _FIRE_END_ANIM_RATE			( 1.0f )
//#define _SNIPE_SCOPE_ZOOM_THESHOLD	( .98f )

void CBotSniper::_HandleWeaponAnimations( void ) {
	
	switch( m_eSnipeState ) {

		case SNIPESTATE_NONE:
			// if we're still in our exit mode, blend out
			if( m_fUnitAim > 0.0f ) {
				m_fUnitAim -= FLoop_fPreviousLoopSecs * _FIRE_END_BLEND_RATE;
				FMATH_CLAMP_MIN0( m_fUnitAim );
				SetControlValue( ANIMCONTROL_FIRE_END, m_fUnitAim );
				SetControlValue( ANIMCONTROL_FIRE_END_LOWER, m_fUnitAim );
			}

			if( m_bControls_Action ) {
				_EnterSnipingMode();
			}

//			FASSERT( GetControlValue( ANIMCONTROL_FIRE ) == 0.0f );
//			FASSERT( GetControlValue( ANIMCONTROL_FIRE_START ) == 0.0f );
			break;

		case SNIPESTATE_BEGINNING:
			if( m_fUnitAim < 1.0f ) {
				m_fUnitAim += FLoop_fPreviousLoopSecs * _FIRE_START_BLEND_RATE;
				FMATH_CLAMP_MAX1( m_fUnitAim );
				SetControlValue( ANIMCONTROL_FIRE_START, m_fUnitAim );
				SetControlValue( ANIMCONTROL_FIRE_START_LOWER, m_fUnitAim );
			}

			//if( GetUnitTime( ANIMTAP_FIRE_START ) > _SNIPE_SCOPE_ZOOM_THESHOLD ) {
			//	((CWeaponScope*)m_apWeapon[1])->EnterZoomMode();
			//	m_nScopeState = SCOPESTATE_LOOKING_THROUGH_SCOPE;
			//}

			if( DeltaTime( ANIMTAP_FIRE_START, FLoop_fPreviousLoopSecs * _FIRE_START_ANIM_RATE, TRUE ) ) {
				m_eSnipeState = SNIPESTATE_SNIPING;
				SetControlValue( ANIMCONTROL_FIRE_START, 0.0f );
				SetControlValue( ANIMCONTROL_FIRE_START_LOWER, 0.0f );
				SetControlValue( ANIMCONTROL_FIRE, 1.0f );
				SetControlValue( ANIMCONTROL_FIRE_LOWER, 1.0f );
				UpdateUnitTime( ANIMTAP_FIRE, 0.0f );
				UpdateUnitTime( ANIMTAP_FIRE_START, 0.0f );

				//METMP
				((CWeaponScope*)m_apWeapon[1])->EnterZoomMode( 1 );
				m_nScopeState = SCOPESTATE_LOOKING_THROUGH_SCOPE;
			}
			break;

		case SNIPESTATE_SNIPING:
			if( m_bControls_Action ) {
				((CWeaponScope*)m_apWeapon[1])->LeaveZoomMode();
				_ExitSnipingMode();
			}

			break;

		case SNIPESTATE_ENDING:
			if( DeltaTime( ANIMTAP_FIRE_END, FLoop_fPreviousLoopSecs * _FIRE_END_ANIM_RATE, TRUE ) ) {
				m_eSnipeState = SNIPESTATE_NONE;
			}
			break;
	}
}


void CBotSniper::_UpdateMatrices( void ) {
	if( m_pDrivingVehicle ) {
		m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld );
		ComputeMtxPalette( TRUE );
		return;
	}
	
	if( IsFlyingOnGrapple() ) {
		m_mtxGrappleFly.m_vPos = m_MountPos_WS;
		m_pWorldMesh->m_Xfm.BuildFromMtx( m_mtxGrappleFly );
		m_pWorldMesh->UpdateTracker();

/*
		CFMtx43A::m_Temp.SetRotationYXZ( m_fMountYaw_WS, FMATH_HALF_PI - m_fModelPitch, 0.0f );
		CFMtx43A::m_Temp.m_vPos = m_MountPos_WS;
		m_pWorldMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_Temp );
		m_pWorldMesh->UpdateTracker();
*/
	} else if( m_eGrappleState != GRAPPLESTATE_CLINGING ) {
		CFMtx43A::m_XlatRotY.SetRotationY( m_fMountYaw_WS + m_fLegsYaw_MS );
		CFMtx43A::m_XlatRotY.m_vPos = m_MountPos_WS;
		m_pWorldMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_XlatRotY );
		m_pWorldMesh->UpdateTracker();
	}

		PROTRACK_BEGINBLOCK("ComputeMtxPal");
			ComputeMtxPalette( TRUE );
		PROTRACK_ENDBLOCK();//"ComputeMtxPal");

		m_GazeUnitVec_WS.ReceiveUnit( *m_pGazeDir_WS );


	if( m_eGrappleState != GRAPPLESTATE_CLINGING ) {
		CFMtx43A::m_Temp.Identity();
		CFMtx43A::m_Temp.SetRotationYXZ( m_fMountYaw_WS, m_fMountPitch_WS, 0.0f );
		CFMtx43A::m_Temp.m_vPos = m_MountPos_WS;
		Relocate_RotXlatFromUnitMtx_WS( &(CFMtx43A::m_Temp), TRUE, m_pMoveIdentifier );
	}
//	if( (m_pPartMgr == NULL) || !m_pPartMgr->IsCreated() || (m_pPartMgr->GetLimbState( LIMB_TYPE_HEAD ) == CBotPartMgr::LIMB_STATE_INTACT) ) {
		m_GazeUnitVec_WS.ReceiveUnit( *m_pGazeDir_WS );
//	} else {
//		m_GazeUnitVec_WS = m_MtxToWorld.m_vFront;
//	}

}


void CBotSniper::_AnimBoneCallback( u32 uBoneIdx, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	FASSERT( m_pCBSniper );
	FASSERT( m_pCBSniper->TypeBits() & ENTITY_BIT_BOTSNIPER );

	if( !m_pCBSniper ) {
		rNewMtx.Mul( rParentMtx, rBoneMtx );
		return;
	}

	// give part mgr first shot
//	if( m_pCBSniper->m_pPartMgr->AnimBoneCallbackFunctionHandler( uBoneIdx, rNewMtx, rParentMtx, rBoneMtx ) ) {
//		return;

	// Groin
	/*} else */
	if( uBoneIdx == m_pCBSniper->m_nBoneIdxGroin ) {
		rNewMtx.Mul( rParentMtx, rBoneMtx );
		m_GroinVecY_WS = rNewMtx.m_vUp;
	
	} else if( uBoneIdx == m_pCBSniper->m_nBoneIdxRightArm ) {
		m_pCBSniper->_AimRightArm( rNewMtx, rParentMtx, rBoneMtx );

	// Torso
	} else if( uBoneIdx == m_pCBSniper->m_nBoneIdxTorso ) {
		if( m_pCBSniper->IsClinging() ) {
			rNewMtx.Mul( rParentMtx, rBoneMtx );
		} else {
			CFMtx43A	mtxWorld;
			CFQuatA		q;

			q.BuildQuat( m_GroinVecY_WS, -(m_pCBSniper->m_fLegsYaw_MS /** 1.0f*/) );
			mtxWorld.Mul( rParentMtx, rBoneMtx );

			rNewMtx.m_vPos = mtxWorld.m_vPos;
			q.MulPoint( rNewMtx.m_vRight, mtxWorld.m_vRight );
			q.MulPoint( rNewMtx.m_vUp, mtxWorld.m_vUp );
			q.MulPoint( rNewMtx.m_vFront, mtxWorld.m_vFront );
		}
	// rib 1
	} else if( uBoneIdx == m_pCBSniper->m_nBoneIdxRib1 ) {
		if( m_pCBSniper->IsClinging() ) {
			rNewMtx.Mul( rParentMtx, rBoneMtx );
		} else {
			CFMtx43A	mtxWorld;
			CFQuatA		q;

			q.BuildQuat( m_GroinVecY_WS, -(m_pCBSniper->m_fLegsYaw_MS * 0.8f) );
			mtxWorld.Mul( rParentMtx, rBoneMtx );

			rNewMtx.m_vPos = mtxWorld.m_vPos;
			q.MulPoint( rNewMtx.m_vRight, mtxWorld.m_vRight );
			q.MulPoint( rNewMtx.m_vUp, mtxWorld.m_vUp );
			q.MulPoint( rNewMtx.m_vFront, mtxWorld.m_vFront );
		}
	// rib 2
	} else if( uBoneIdx == m_pCBSniper->m_nBoneIdxRib2 ) {
		if( m_pCBSniper->IsClinging() ) {
			rNewMtx.Mul( rParentMtx, rBoneMtx );
		} else {
			CFMtx43A	mtxWorld;
			CFQuatA		q;

			q.BuildQuat( m_GroinVecY_WS, -(m_pCBSniper->m_fLegsYaw_MS * 0.6f) );
			mtxWorld.Mul( rParentMtx, rBoneMtx );

			rNewMtx.m_vPos = mtxWorld.m_vPos;
			q.MulPoint( rNewMtx.m_vRight, mtxWorld.m_vRight );
			q.MulPoint( rNewMtx.m_vUp, mtxWorld.m_vUp );
			q.MulPoint( rNewMtx.m_vFront, mtxWorld.m_vFront );
		}
	// rib 3
	} else if( uBoneIdx == m_pCBSniper->m_nBoneIdxRib3 ) {
		if( m_pCBSniper->IsClinging() ) {
			rNewMtx.Mul( rParentMtx, rBoneMtx );
		} else {
			CFMtx43A	mtxWorld;
			CFQuatA		q;

			q.BuildQuat( m_GroinVecY_WS, -(m_pCBSniper->m_fLegsYaw_MS * 0.4f) );
			mtxWorld.Mul( rParentMtx, rBoneMtx );

			rNewMtx.m_vPos = mtxWorld.m_vPos;
			q.MulPoint( rNewMtx.m_vRight, mtxWorld.m_vRight );
			q.MulPoint( rNewMtx.m_vUp, mtxWorld.m_vUp );
			q.MulPoint( rNewMtx.m_vFront, mtxWorld.m_vFront );
		}	
	// rib 4
	} else if( uBoneIdx == m_pCBSniper->m_nBoneIdxRib4 ) {
		if( m_pCBSniper->IsClinging() ) {
			rNewMtx.Mul( rParentMtx, rBoneMtx );
		} else {
			CFMtx43A	mtxWorld;
			CFQuatA		q;

			q.BuildQuat( m_GroinVecY_WS, -(m_pCBSniper->m_fLegsYaw_MS * 0.2f) );
			mtxWorld.Mul( rParentMtx, rBoneMtx );

			rNewMtx.m_vPos = mtxWorld.m_vPos;
			q.MulPoint( rNewMtx.m_vRight, mtxWorld.m_vRight );
			q.MulPoint( rNewMtx.m_vUp, mtxWorld.m_vUp );
			q.MulPoint( rNewMtx.m_vFront, mtxWorld.m_vFront );
		}
	// Head
	} else if( uBoneIdx == m_pCBSniper->m_nBoneIdxHead ) {
		m_pCBSniper->HeadLookUpdate( rNewMtx, rParentMtx, rBoneMtx, FALSE );

	// Unknown bone, not good...
	} else {
		FASSERT_NOW;
		rNewMtx.Mul( rParentMtx, rBoneMtx );
	}
}


#define _RECOIL_UPPER_MIN	FMATH_DEG2RAD( -30.0f )
#define _RECOIL_UPPER_MAX	FMATH_DEG2RAD( 50.0f )
#define _RECOIL_LOWER_MIN	FMATH_DEG2RAD( 10.0f )
#define _RECOIL_LOWER_MAX	FMATH_DEG2RAD( -60.0f )
#define _RECOIL_TORSO_MIN	FMATH_DEG2RAD( 15.0f )
#define _RECOIL_TORSO_MAX	FMATH_DEG2RAD( -30.0f )


/////////////////////////
//ANIMATION / SUMMER
void CBotSniper::_HandleSummerAnimations( void ) {
	if( IsClinging() ) {
		SetControlValue( ANIMCONTROL_SUMMER, 0.0f );
		return;
	} else {
		SetControlValue( ANIMCONTROL_SUMMER, 1.0f );
	}


	CFAnimFrame animframe;
	CFAnimFrame qRot;
	CFVec3A vTorsoRecoilAxis;
	vTorsoRecoilAxis.Set( 0.7071f, 0.7071f, 0.0f );

	SetControlValue( ANIMCONTROL_SUMMER, 1.0f );

	//m_fGrappleRecoil = 0.0f;

	static f32 fVal = 0.0f;
	static f32 fVal2 = 0.0f;
//	static f32 _RECOIL_TORSO_MIN = 0.0f;
//	static f32 _RECOIL_TORSO_MAX = 0.0f;

	//gamepad_AxisSlider( m_fGrappleRecoil,
	//					m_fGrappleRecoil,
	//					-1.0f,
	//					1.0f,
	//					0.01f,
	//					1.0f,
	//					GAMEPAD_SLIDER_AXIS_RIGHT_ANALOG_Y,
	//					1 );
	//
	//gamepad_AxisSlider( fVal,
	//					fVal,
	//					-FMATH_PI,
	//					FMATH_PI,
	//					FMATH_2PI * 0.01f,
	//					FMATH_QUARTER_PI,
	//					GAMEPAD_SLIDER_AXIS_RIGHT_ANALOG_X,
	//					1 );


	/*ME*///Add some code in here to clamp recoil to control value of acgrapple
	if( m_fGrappleRecoil >= 0.0f ) {
		animframe.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( fmath_UnitLinearToSCurve( m_fGrappleRecoil ), 0.0f, _RECOIL_UPPER_MAX ), CFVec3A::m_Null );
		m_AnimManFrameSummer.UpdateFrame( ANIMBONE_ARM_UPPER_L, animframe );

		animframe.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( m_fGrappleRecoil, 0.0f, _RECOIL_LOWER_MAX ), CFVec3A::m_Null );
		m_AnimManFrameSummer.UpdateFrame( ANIMBONE_ARM_LOWER_L, animframe );

		qRot.BuildQuat( vTorsoRecoilAxis, FMATH_FPOT( m_fGrappleRecoil, 0.0f, _RECOIL_TORSO_MAX ), CFVec3A::m_Null );
	} else {
		animframe.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( -m_fGrappleRecoil, 0.0f, _RECOIL_UPPER_MIN ), CFVec3A::m_Null );
		m_AnimManFrameSummer.UpdateFrame( ANIMBONE_ARM_UPPER_L, animframe );

		animframe.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( -m_fGrappleRecoil, 0.0f, _RECOIL_LOWER_MIN ), CFVec3A::m_Null );
		m_AnimManFrameSummer.UpdateFrame( ANIMBONE_ARM_LOWER_L, animframe );

		qRot.BuildQuat( vTorsoRecoilAxis, FMATH_FPOT( -m_fGrappleRecoil, 0.0f, _RECOIL_TORSO_MIN ), CFVec3A::m_Null );
	}


	// Aim the big gun
	//CFVec3A vArmToTgt;
	//CFVec3A vAxis;

	//const CFMtx43A *pArmMtx = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxRightArm];
	//vArmToTgt.Sub( m_TargetedPoint_WS, pArmMtx->m_vPos ).Negate();
	//f32 fMag2 = vArmToTgt.MagSq();
	//if( fMag2 > 0.0001f ) {
	//	vArmToTgt.Mul( fmath_InvSqrt( fMag2 ) );

	//	vAxis.Cross( pArmMtx->m_vUp, vArmToTgt );
	//	f32 fSin = vAxis.SafeUnitAndMag( vAxis );
	//	f32 fAngle;
	//	if( fSin > 0.01f ) {
	//		fAngle = fmath_Atan( fSin, pArmMtx->m_vUp.Dot( vArmToTgt ) );
	//	} else {
	//		fAngle = 0.0f;
	//	}
	//	animframe.BuildQuat( vAxis, fAngle, CFVec3A::m_Null );

	//} else {
	//	animframe.Identity();
	//	animframe.m_Pos.Zero();
	//}

	//m_AnimManFrameSummer.UpdateFrame( ANIMBONE_ARM_UPPER_R, animframe );
		
    
	// Twist torso and head
	f32 fUnitAimPitch = fmath_Div( m_fMountPitch_WS - m_fMountPitchMin_WS, m_fMountPitchMax_WS - m_fMountPitchMin_WS );
	///*ME*/FASSERT_UNIT_FLOAT( fUnitAimPitch );
	FMATH_CLAMP_UNIT_FLOAT( fUnitAimPitch );

	animframe.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( fUnitAimPitch, FMATH_DEG2RAD( -45.0f ), FMATH_DEG2RAD( 45.0f ) ), CFVec3A::m_Null );
	if( m_fGrappleRecoil != 0.0f ) {
		animframe.Mul( qRot );
	}
	m_AnimManFrameSummer.UpdateFrame( ANIMBONE_TORSO, animframe );

	animframe.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( fUnitAimPitch, FMATH_DEG2RAD( -25.0f ), FMATH_DEG2RAD( 25.0f ) ), CFVec3A::m_Null );
	m_AnimManFrameSummer.UpdateFrame( ANIMBONE_HEAD, animframe );

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	ftext_DebugPrintf( 0.25f, 0.6f, "~w1U:  %0.2f, L: %0.2f", m_fGrappleRecoil, FMATH_RAD2DEG( fVal ) );
#endif
	
}

void CBotSniper::_AimRightArm( CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	rNewMtx.Mul( rParentMtx, rBoneMtx );

	if( m_fUnitAim == 0.0f ) {
		m_qArmAim.BuildQuat( rNewMtx );
		return;
	}

	// we're at least aiming a little bit
	CFQuatA qTo;

	if( m_fUnitAim == 1.0f ) {
		CFVec3A vArmToTgt;

		vArmToTgt.Sub( m_TargetedPoint_WS, rNewMtx.m_vPos );
		f32 fMag2 = vArmToTgt.MagSq();
		if( fMag2 > 0.0001f ) {
			vArmToTgt.Mul( fmath_InvSqrt( fMag2 ) );
			rNewMtx.UnitMtxFromUnitVec( &vArmToTgt );

			CFVec3A vTmp;
			vTmp = rNewMtx.m_vUp;
			rNewMtx.m_vUp = rNewMtx.m_vFront.Negate();
			rNewMtx.m_vFront = vTmp;
		}

		qTo.BuildQuat( rNewMtx );
		m_qArmAim.ReceiveSlerpOf( 10.0f * FLoop_fPreviousLoopSecs, m_qArmAim, qTo );		

	} else {
		// blending out of firing pose.  Blend back to normal
		qTo.BuildQuat( rNewMtx );
		m_qArmAim.ReceiveSlerpOf( fmath_UnitLinearToSCurve( 1.0f - m_fUnitAim ), m_qArmAim, qTo );
	}

	m_qArmAim.BuildMtx33( rNewMtx );
}


/////////////////////////
//JUMPING

void CBotSniper::_HandleJumping( void ) {
	if( !m_bControlsBot_JumpVec ) {
//		if( m_bControlsBot_Jump2 ) {
//			_StartSingleJump();
//			_StartDoubleJump();
//		} else
		if( m_bControls_Jump ) {
			_StartSingleJump();
		}
	} else {
		// Velocity jump specified...
		_StartSingleJump( &m_ControlsBot_JumpVelocity_WS );
	}

	if( m_nPrevState == STATE_AIR ) {
		if( m_nJumpState != BOTJUMPSTATE_NONE ) {
			_JumpLanded();
		}
	}
}


void CBotSniper::_EnterFlyMode( void ) {
	m_bPlayLandAnim			= FALSE;
	m_bPlayLandAnimLower	= FALSE;

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_RECOVER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );

	m_nJumpState = BOTJUMPSTATE_AIR;
	SetJumping();
}


#define _BIG_LANDING_THRESHOLD (-75.0f)

void CBotSniper::_JumpLanded( void ) {
	MakeFootImpactDust( TRUE, TRUE, m_pBotInfo_Gen->fUnitDustKickup, &m_FeetToGroundCollImpact.UnitFaceNormal );

	m_nJumpState				= BOTJUMPSTATE_NONE;
	ClearJumping();
	m_bPlayLandAnim 			= TRUE;
	m_bPlayLandRecoverAnimation = FALSE;

	ZeroTime( ANIMTAP_JUMP_LAND_UPPER );
	ZeroTime( ANIMTAP_JUMP_LAND_RECOVER );

	m_fMaxLandUnitBlend = GetControlValue( ANIMCONTROL_JUMP_FLY );
	if( m_fMaxLandUnitBlend == 0.0f ) {
		m_fMaxLandUnitBlend = 1.0f;
	}

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );

	//f32 fVVel = (m_MountPrevPos_WS.y - m_MountPos_WS.y) * FLoop_fPreviousLoopOOSecs;
	//DEVPRINTF( "vvel = %0.2f\n", m_fVerticalVelocity );
	// check whether we're playing the big landing or the little one
	if( (m_fMaxLandUnitBlend == 1.0f) && (m_fVerticalVelocity < _BIG_LANDING_THRESHOLD) ) {
		SetControlValue( ANIMCONTROL_JUMP_LAND_RECOVER, 1.0f );
		m_bPlayLandRecoverAnimation = TRUE;
	} else {
		SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, m_fMaxLandUnitBlend );
		
		if( m_fSpeedXZ_WS == 0.0f ) {
			SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, m_fMaxLandUnitBlend );
			m_bPlayLandAnimLower = TRUE;
		} else {
			SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
			m_bPlayLandAnimLower = FALSE;
		}
	}

	// play sound...
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

	if( m_pStickyEntity && m_pStickyEntity->GetVerlet() ) {
		CFVec3A ImpulseVec_WS;

		ImpulseVec_WS.Set( 0.0f, -m_pBotInfo_Gen->fPhysForce_Land, 0.0f );
		m_pStickyEntity->GetVerlet()->ApplyImpulse( &m_MtxToWorld.m_vPos, &ImpulseVec_WS );
	}
}


void CBotSniper::_StartSingleJump( const CFVec3A *pJumpVelocity_WS ) {
	if( pJumpVelocity_WS == NULL ) {
		m_Velocity_MS.y += m_pBotInfo_Jump->fVerticalVelocityJump1 * m_fJumpMultiplier;
		MS2WS( m_Velocity_WS, m_Velocity_MS );
	} else {
		m_Velocity_WS.x = pJumpVelocity_WS->x;
		m_Velocity_WS.y = pJumpVelocity_WS->y;
		m_Velocity_WS.z = pJumpVelocity_WS->z;
		WS2MS( m_Velocity_MS, m_Velocity_WS );
	}
		
	VelocityHasChanged();

	m_bPlayLandAnim		 = FALSE;
	m_bPlayLandAnimLower = FALSE;

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_RECOVER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );	

	ZeroTime( ANIMTAP_JUMP_LAUNCH );

	m_nJumpState = BOTJUMPSTATE_LAUNCH;
	SetJumping();
}


void CBotSniper::_HandleJumpAnimations( void ) {
	f32 fTemp, fUnitTime;
	BOOL bAnimFinished;

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
			
			if( m_bPlayLandRecoverAnimation ) {
				bAnimFinished = DeltaTime( ANIMTAP_JUMP_LAND_RECOVER, FLoop_fPreviousLoopSecs * fTemp );
				ZeroVelocity();
			} else {
				bAnimFinished = DeltaTime( ANIMTAP_JUMP_LAND_UPPER, FLoop_fPreviousLoopSecs * fTemp );
			}

			if( !bAnimFinished ) {
				// Still playing landing animation...

				f32 fUnitTime = 1.01f - GetUnitTime(ANIMTAP_JUMP_LAND_UPPER);
				fUnitTime = m_fMaxLandUnitBlend * fmath_UnitLinearToSCurve( fUnitTime );

				if( m_bPlayLandRecoverAnimation ) {
					SetControlValue( ANIMCONTROL_JUMP_LAND_RECOVER, fUnitTime );
				} else {
					SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, fUnitTime );
				}
				
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
				SetControlValue( ANIMCONTROL_JUMP_LAND_RECOVER, 0.0f );
			}
		} else {
			if( m_nJumpState == BOTJUMPSTATE_NONE ) {
				SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_LAND_RECOVER, 0.0f );
			}
		}
	} else {
		// We're in the air...

		if( m_nJumpState != BOTJUMPSTATE_NONE ) {

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
					break;

				default:
					FASSERT_NOW;
					break;
			}
		}
	}
}



/////////////////////////
//WEAPONS


void CBotSniper::_WeaponWork( void ) {
	if( (m_fControls_Fire1 > 0.25f) && (m_fFireTimer <= 0.0f) ) {
        // fire

		CFVec3A vMuzzlePt;
		CFVec3A vFireUnitDir;

		vMuzzlePt = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxPriFire]->m_vPos;
		vFireUnitDir.Sub( m_TargetedPoint_WS, vMuzzlePt );
		f32 fMag2 = vFireUnitDir.MagSq();
		if( fMag2 < 0.001f ) {
			vFireUnitDir = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxPriFire]->m_vFront;
		} else {
			vFireUnitDir.Mul( fmath_InvSqrt( fMag2 ) );
		}


		if( m_uNumActiveBulletTrails < m_BotInfo_Sniper.uMaxTracers - 1 ) {
			m_TracerDef.nUser = m_uNumActiveBulletTrails;
			m_paBulletTrails[m_uNumActiveBulletTrails].fUnitLifespan = 0.0f;
			m_paBulletTrails[m_uNumActiveBulletTrails].pTracer = NULL;		// useless, don't know how to get this
			m_paBulletTrails[m_uNumActiveBulletTrails].uTimeCreated = FLoop_nTotalLoopTicks; // useful at all?
			m_paBulletTrails[m_uNumActiveBulletTrails].vStartPoint = vMuzzlePt;
			m_paBulletTrails[m_uNumActiveBulletTrails].vEndPoint = vMuzzlePt;
			for( u32 i=0; i<_BULLETTRAIL_NODES; i++ ) {
				m_paBulletTrails[m_uNumActiveBulletTrails].afTopVelocities[i] = fmath_RandomFloatRange( 0.66f, 1.33f );
				m_paBulletTrails[m_uNumActiveBulletTrails].afBottomVelocities[i] = fmath_RandomFloatRange( 0.66f, 1.33f );
			}

			m_uNumActiveBulletTrails++;
		} else {
			m_TracerDef.nUser = -1;
		}

		m_TracerDef.pUser = this;
		m_TracerDef.TailPos_WS = vMuzzlePt;
		m_TracerDef.UnitDir_WS = vFireUnitDir;
		m_TracerDef.fWidth_WS = 2.0f;
		m_TracerDef.fLength_WS = 2000.0f;
		m_TracerDef.fSpeed_WS = 2500.0f;
		m_TracerDef.fMaxTailDist_WS = 2000.0f;
		m_TracerDef.fBeginDeathUnitFade_WS = 0.99f;
		m_TracerDef.uFlags = 0;
		m_TracerDef.ColorRGBA.OpaqueWhite();

		ConstructDamagerData();
		if( !tracer_NewTracer( m_hTracerGroup, &m_TracerDef, 0.2f, FALSE, FALSE, &CDamageForm::m_TempDamager ) ) {
			return;
		}

		tracer_AddExtraParamsToLastTracer( _TracerMovedCB, (void*)(m_uNumActiveBulletTrails-1) );

		// successfully fired a tracer
		m_fFireTimer = m_BotInfo_Sniper.fReloadTime;
#if FANG_WINGC
		if( Fang_bPortDiag ) {
			DEVPRINTF( "PORT-SNIPER '%s' fired from (%.0f, %.0f, %.0f) toward (%.0f, %.0f, %.0f)%s\n", Name() ? Name() : "?",
					   vMuzzlePt.x, vMuzzlePt.y, vMuzzlePt.z, m_TargetedPoint_WS.x, m_TargetedPoint_WS.y, m_TargetedPoint_WS.z,
					   m_nPossessionPlayerIndex >= 0 ? " (player)" : "" );
		}
#endif
		
		// kick off the firing animation
		UpdateUnitTime( ANIMTAP_FIRE, 0.001f );
	}

	if( m_fFireTimer > 0.0f ) {
		m_fFireTimer -= FLoop_fPreviousLoopSecs;
	}
}


#define _FIRE_SPEED		( 1.0f )

void CBotSniper::_HandleFireAnimation( void ) {
	f32 fTime = GetUnitTime( ANIMTAP_FIRE );
	if( fTime > 0.0f ) {
		fTime += FLoop_fPreviousLoopSecs * _FIRE_SPEED;
		if( DeltaTime( ANIMTAP_FIRE, FLoop_fPreviousLoopSecs * _FIRE_SPEED, TRUE ) ) {
			UpdateUnitTime( ANIMTAP_FIRE, 0.0f );
		}
	}
}

void CBotSniper::_TracerKilledCallback( TracerDef_t *pTracerDef, TracerKillReason_e nKillReason, const FCollImpact_t *pImpact ) {
	///*ME*/DEVPRINTF( "Boom\n" );
#if FANG_WINGC
	// PC port: deliver the shot's damage (the same impact path CBotSnarq's tracer uses).
	if( nKillReason != TRACER_KILLREASON_HIT_GEO || !pImpact || !pTracerDef->pUser ) {
		return;
	}
	CBotSniper *pSniper = (CBotSniper *)pTracerDef->pUser;

	const CGCollMaterial *pCollMaterial = CGColl::GetMaterial( pImpact->nUserType );
	pCollMaterial->DrawAllDrawableParticles( &pImpact->ImpactPoint, &pImpact->UnitFaceNormal, TRUE,
											 fmath_RandomFloatRange( 0.2f, 0.4f ), fmath_RandomFloatRange( 0.25f, 0.75f ), 0.0f );

	CDamageProfile *pProfile = pSniper->m_nPossessionPlayerIndex >= 0 ? m_pPortShotDamagePlayer : m_pPortShotDamageNPC;
	CEntity *pEntity = CGColl::ExtractEntity( pImpact );
	if( pEntity && pProfile ) {
		CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();
		if( pDamageForm ) {
			pDamageForm->m_nDamageLocale	= CDamageForm::DAMAGE_LOCALE_IMPACT;
			pDamageForm->m_nDamageDelivery	= CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
			pDamageForm->m_pDamageProfile	= pProfile;
			pDamageForm->m_Damager.pWeapon	= NULL;
			pDamageForm->m_Damager.pBot		= pSniper;
			pDamageForm->m_Damager.pEntity	= pSniper;
			pDamageForm->m_Damager.nDamagerPlayerIndex = pSniper->m_nPossessionPlayerIndex;
			pDamageForm->m_pDamageeEntity	= pEntity;
			pDamageForm->InitTriDataFromCollImpact( (CFWorldMesh *)pImpact->pTag, pImpact, &pTracerDef->UnitDir_WS );
			CDamage::SubmitDamageForm( pDamageForm );
		}
	}
#endif
}


void CBotSniper::_TracerBuildTrackerSkipList( void *pUser ) {
	FASSERT( pUser && ((CEntity*)pUser)->TypeBits() & ENTITY_BIT_BOTSNIPER );
	FASSERT( ((CEntity*)pUser)->IsCreated() );

	((CBotSniper*)pUser)->AppendTrackerSkipList();
}


void CBotSniper::_TracerMovedCB( void* pUserData, const CFVec3A& NewPos_WS, BOOL bDied ) {
	u32 uTrailID = (u32)pUserData;
	if( !m_paBulletTrails || uTrailID >= m_BotInfo_Sniper.uMaxTracers ) {
		return;
	}

	m_paBulletTrails[uTrailID].vEndPoint = NewPos_WS;
}


///////////////////////////
//GRAPPLE

#define _GRAPPLE_MIN_DIST_SQ	( 1.0f * 1.0f )
#define _GRAPPLE_MIN_ANGLE_COS	( 0.1f )



/*ME*/static CFVec3A _vDbgPt0;
/*ME*/static CFVec3A _vDbgPt1;
/*ME*/static CFVec3A _vDbgPt2;
/*ME*/static CFVec3A _vDbgPt3;

BOOL CBotSniper::_CheckGrapplePos( void ) {
///*ME*/	return FALSE;
	f32 fMag2;
	f32 fVal;

	m_vGrappleTgtPos.Zero();

	m_vGrappleTgtPos = m_TargetedPoint_WS;


	// calculate start point for raycasts
	CFVec3A vStartPt;
	CFVec3A vUnitToBot;
	CFVec3A vRayPt;

	vUnitToBot.Sub( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxGrapple]->m_vPos, m_vGrappleTgtPos );
	fMag2 = vUnitToBot.MagSq();
	if( fMag2 < _GRAPPLE_MIN_DIST_SQ ) {
		m_bValidGrapplePos = FALSE;
		return FALSE;
	}

	vUnitToBot.Mul( fmath_InvSqrt( fMag2 ) );

	vRayPt.Sub( m_vGrappleTgtPos, vUnitToBot );
	vStartPt.Mul( vUnitToBot, 5.0f ).Add( m_vGrappleTgtPos );

	//ok now do the first raycast
	FCollImpact_t impact;

	if( !fworld_FindClosestImpactPointToRayStart( &impact, &vStartPt, &vRayPt, 0, NULL, TRUE, NULL, FCOLL_USER_TYPE_BITS_ALL ) ) {
		DEVPRINTF( "No initial coll, that's strange\n" );
		m_bValidGrapplePos = FALSE;
		return FALSE;
	}

	m_vGrappleTgtNormal = impact.UnitFaceNormal;

	// check the normal.  Should be more than oh... .25
	fVal = vUnitToBot.Dot( impact.UnitFaceNormal );
	
	if( fVal < _GRAPPLE_MIN_ANGLE_COS ) {
		DEVPRINTF( "Dot too high\n" );
		m_bValidGrapplePos = FALSE;
		return FALSE;
	}

	// now get some new collision points
	//pt 1...
	CFVec3A vUnitRay;
	CFVec3A vTestPt;
	CFVec3A vNormal = impact.UnitFaceNormal;


	if( FMATH_FABS( impact.UnitFaceNormal.y ) < 0.1f ) {
		vUnitRay.CrossVecWithY( impact.UnitFaceNormal );
		vUnitRay.Unitize();
	} else {
		vUnitRay.Cross( impact.UnitFaceNormal, CFVec3A::m_UnitAxisX );
		vUnitRay.Unitize();
	}

	vTestPt.Mul( vUnitRay, 3.0f ).Add( vRayPt );
	_vDbgPt0 = vTestPt;
	if( fworld_FindClosestImpactPointToRayStart( &impact,
		&vStartPt, &vTestPt, 0, NULL, TRUE, NULL, FCOLL_USER_TYPE_BITS_ALL ) ) {
		if( impact.UnitFaceNormal.Dot( vNormal ) < 0.85f ) {
			DEVPRINTF( "Corner 0 dot failed\n" );
			m_bValidGrapplePos = FALSE;
			return FALSE;
		}

		_vDbgPt0 = vTestPt;
	} else {
		_vDbgPt0.Zero();
		DEVPRINTF( "Corner 0 failed\n" );
		m_bValidGrapplePos = FALSE;
		return FALSE;
	}


	vTestPt.Mul( vUnitRay, -3.0f ).Add( vRayPt );
	if( fworld_FindClosestImpactPointToRayStart( &impact, &vStartPt, &vTestPt, 0, NULL, TRUE, NULL, FCOLL_USER_TYPE_BITS_ALL ) ) {
		_vDbgPt1 = vTestPt;
		if( impact.UnitFaceNormal.Dot( vNormal ) < 0.85f ) {
			DEVPRINTF( "Corner 1 dot failed\n" );
			m_bValidGrapplePos = FALSE;
			return FALSE;
		}
	} else {
		_vDbgPt1.Zero();
		DEVPRINTF( "Corner 1 failed\n" );
		m_bValidGrapplePos = FALSE;
		return FALSE;
	}

	vUnitRay.Cross( impact.UnitFaceNormal );

	vTestPt.Mul( vUnitRay, 3.0f ).Add( vRayPt );
	if( fworld_FindClosestImpactPointToRayStart( &impact, &vStartPt, &vTestPt, 0, NULL, TRUE, NULL, FCOLL_USER_TYPE_BITS_ALL ) ) {
		_vDbgPt2 = vTestPt;
		if( impact.UnitFaceNormal.Dot( vNormal ) < 0.85f ) {
			DEVPRINTF( "Corner 2 dot failed\n" );
			m_bValidGrapplePos = FALSE;
			return FALSE;
		}
	} else {
		_vDbgPt2.Zero();
		DEVPRINTF( "Corner 2 failed\n" );
		m_bValidGrapplePos = FALSE;
		return FALSE;
	}

	vTestPt.Mul( vUnitRay, -3.0f ).Add( vRayPt );
	if( fworld_FindClosestImpactPointToRayStart( &impact, &vStartPt, &vTestPt, 0, NULL, TRUE, NULL, FCOLL_USER_TYPE_BITS_ALL ) ) {
		_vDbgPt3 = vTestPt;
		if( impact.UnitFaceNormal.Dot( vNormal ) < 0.85f ) {
			DEVPRINTF( "Corner 3 dot failed\n" );
			m_bValidGrapplePos = FALSE;
			return FALSE;
		}
	} else {
		_vDbgPt3.Zero();
		DEVPRINTF( "Corner 3 failed\n" );
		m_bValidGrapplePos = FALSE;
		return FALSE;
	}

	m_bValidGrapplePos = TRUE;
	return m_bValidGrapplePos;
}


void CBotSniper::_GrappleStopAiming( void ) {
	m_eGrappleState = GRAPPLESTATE_NONE;
	m_bValidGrapplePos = FALSE;
}


#define _GRAPPLE_AIM_BLENDIN_RATE		( 4.0f )
#define _GRAPPLE_AIM_BLENDOUT_RATE		( 1.0f )
#define _GRAPPLE_FLY_BLENDIN_TIME		( 0.2f ) 
#define _GRAPPLE_FLY_BLENDIN_RATE		( 1.0f / _GRAPPLE_FLY_BLENDIN_TIME )
#define _GRAPPLE_FLY_BLENDOUT_RATE		( 2.0f )


void CBotSniper::_FireGrapple( void ) {
	m_eGrappleState = GRAPPLESTATE_FIRING;
	m_bGrappleRecoilOn = TRUE;
	m_pGrapple->Fire( m_vGrappleTgtPos );
}


void CBotSniper::_GrappleTakeoff( void ) {
	m_eGrappleState = GRAPPLESTATE_LAUNCHING;

	m_fTimeToFlyOnCable = fmath_Div( m_vGrappleTgtPos.Dist( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxGrapple]->m_vPos ), _GRAPPLE_FLY_SPEED );
	m_fOOTimeToFlyOnCable = fmath_Inv( m_fTimeToFlyOnCable );
	m_fUnitCableFly = 0.0f;

	// calculate flight orientation matrix.  Dist to grapple point cannot = 0
	CFVec3A vToGrapple, vTmp;
	
	// calculate quat for flight
	vToGrapple.Sub( m_vGrappleTgtPos, m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxGrapple]->m_vPos );
	CFMtx43A::m_Temp.UnitMtxFromNonUnitVec( &vToGrapple );
	vTmp = CFMtx43A::m_Temp.m_vUp;
	CFMtx43A::m_Temp.m_vUp = CFMtx43A::m_Temp.m_vFront;
	CFMtx43A::m_Temp.m_vFront.ReceiveNegative( vTmp );
	m_qGrappleFlyOrientation.BuildQuat( CFMtx43A::m_Temp );

	// initialize the matrix for first frame
	m_mtxGrappleFly = m_pWorldMesh->m_Xfm.m_MtxF;

	// calculate our destination matrix...
	vTmp = m_vGrappleTgtNormal;
	vTmp.Negate();

	m_mtxGrappleClingPosition.UnitMtxFromUnitVec( &vTmp );
	m_mtxGrappleClingPosition.m_vPos = m_vGrappleTgtPos;
	m_mtxGrappleClingPosition.MulPoint( m_mtxGrappleClingPosition.m_vPos, m_vGrappleClingPosWall_MS );

	m_fGrappleFlyYaw	= fmath_Atan( vToGrapple.x, vToGrapple.z );
	m_fGrappleFlyPitch	= fmath_Atan( vToGrapple.y, vToGrapple.MagXZ() );
	m_qGrappleLastOrientation.BuildQuat( m_mtxGrappleFly );
	
	m_qGrappleLandOrientation.BuildQuat( m_mtxGrappleClingPosition );

	// should switch to our dest animation here too
	if( m_vGrappleTgtNormal.y > -0.45f ) {
		// hanging from the wall
	} else {
		// hanging from the ceiling
	}

	//if( GetAttachedAnim( ANIMTAP_FIRE_1_UPPER ) != &m_Anim.m_pLoadedBaseAnimInst[ ANIM_FIRE_ROCKET1 ] ) {
	//AttachAnim( ANIMTAP_FIRE_1_UPPER, &m_Anim.m_pLoadedBaseAnimInst[ ANIM_FIRE_ROCKET1 ] );

}


#define _GRAPPLE_LANDING_DIST		( 75.0f )

void CBotSniper::_FlyAlongCable( void ) {
	CFVec3A vToTgt;
	
	vToTgt.Sub( m_vGrappleTgtPos, m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxGrapple]->m_vPos );
	f32 fDist = vToTgt.SafeUnitAndMag( vToTgt );
	if( fDist < _GRAPPLE_LANDING_DIST ) {
		m_fGrappleLandRate = fmath_Div( _GRAPPLE_FLY_SPEED, fDist );
		m_eGrappleState = GRAPPLESTATE_LANDING;
	}

	m_Velocity_WS.Mul( vToTgt, _GRAPPLE_FLY_SPEED );
	WS2MS( m_Velocity_MS, m_Velocity_WS );
	VelocityHasChanged();

	m_fUnitCableFly += FLoop_fPreviousLoopSecs * m_fOOTimeToFlyOnCable;
	if( m_fUnitCableFly >= 1.0f ) {
		m_fUnitCableFly = 1.0f;
		_ClingToWall();
	}
}


#define _START_CLING_POSE_DIST		( 50.0f )
#define _START_CLING_POSE_DIST_SQ	( _START_CLING_POSE_DIST * _START_CLING_POSE_DIST )
#define _TAKEOFF_ORIENT_TIME		( 0.5f )
#define _TAKEOFF_ORIENT_OO_TIME		( 1.0f / _TAKEOFF_ORIENT_TIME )


void CBotSniper::_CalcLaunchOrientation( void ) {
	f32 fVal = GetControlValue( ANIMCONTROL_GRAPPLE_FLY );
	FASSERT_UNIT_FLOAT( fVal );
	
	m_qGrappleLastOrientation.ReceiveSlerpOf( fVal, m_qGrappleLastOrientation, m_qGrappleFlyOrientation );
	m_qGrappleLastOrientation.BuildMtx33( m_mtxGrappleFly );
}


void CBotSniper::_CalcLandingOrientation( void ) {
	f32 fVal;

	fVal = GetControlValue( ANIMCONTROL_GRAPPLE_HANG );
	fVal += FLoop_fPreviousLoopSecs * m_fGrappleLandRate;
	FMATH_CLAMP_MAX1( fVal );
	SetControlValue( ANIMCONTROL_GRAPPLE_HANG, fVal );
	m_qGrappleLastOrientation.ReceiveSlerpOf( fVal, m_qGrappleLastOrientation, m_qGrappleLandOrientation );
	m_qGrappleLastOrientation.BuildMtx33( m_mtxGrappleFly );
}


void CBotSniper::_BreakGrappleLine( void ) {
	m_pGrapple->Break();
	m_eGrappleState = GRAPPLESTATE_NONE;
	((CWeaponScope*)m_apWeapon[1])->LeaveZoomMode();
	m_nScopeState = SCOPESTATE_NONE;
	m_fModelPitch = FMATH_HALF_PI;
	//_ExitSnipingMode();
}


void CBotSniper::_ClingToWall( void ) {
	// check the normal of the surfact to decide what to do
	if( m_mtxGrappleClingPosition.m_vFront.y < -0.7071f ) {
		_BreakGrappleLine();
		ZeroVelocity();
		SetControlValue( ANIMCONTROL_JUMP_LAND_RECOVER, 1.0f );
		UpdateUnitTime( ANIMTAP_JUMP_LAND_RECOVER, 0.0f );
		return;
	}


	m_eGrappleState = GRAPPLESTATE_CLINGING;
	ZeroVelocity();

	CFVec3A vTmp;

	vTmp = m_vGrappleTgtNormal;
	vTmp.Negate();
	CFMtx43A::m_Temp.UnitMtxFromUnitVec( &vTmp );

	m_pGrapple->GetDistToGrapple( &vTmp );
	vTmp.Add( m_MtxToWorld.m_vPos );

	CFMtx43A::m_Temp.m_vPos = vTmp;

	//Relocate_RotXlatFromUnitMtx_WS( &CFMtx43A::m_Temp );
	Relocate_RotXlatFromUnitMtx_WS( &m_mtxGrappleClingPosition );

	m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld );

	// Store new position...
	m_MountPos_WS = m_MtxToWorld.m_vPos;
	m_MountPrevPos_WS = m_MountPos_WS;

	UpdateBoneMask( ANIMTAP_FIRE_START, m_aBoneEnableIndices_UserAnim_RightArm, TRUE );
	UpdateBoneMask( ANIMTAP_FIRE, m_aBoneEnableIndices_UserAnim_RightArm, TRUE );
	UpdateBoneMask( ANIMTAP_FIRE_END, m_aBoneEnableIndices_UserAnim_RightArm, TRUE );

	//_EnterSnipingMode();
}


#define _GRAPPLE_RECOIL_RELAX_RATE		( 1.0f )
#define _GRAPPLE_RECOIL_RATE			( 15.0f )
#define _GRAPPLE_RECOIL_TIGHTEN_RATE	( 5.0f )
#define _GRAPPLE_RECOIL_FLY_RATE		( 10.0f )


void CBotSniper::_GrappleWork( void ) {
	///*ME*/return;
	f32 fVal;

	switch( m_eGrappleState ) {
		case GRAPPLESTATE_NONE:

			// if any grapple states are active, blend them out
			fVal = GetControlValue( ANIMCONTROL_GRAPPLE );
			if( fVal > 0.0f ) {
				fVal -= FLoop_fPreviousLoopSecs * _GRAPPLE_AIM_BLENDOUT_RATE;
				FMATH_CLAMP_MIN0( fVal );

				FMATH_BIPOLAR_CLAMPMAX( m_fGrappleRecoil, fVal );
				UpdateUnitTime( ANIMTAP_GRAPPLE, 1.0f - fVal );
				SetControlValue( ANIMCONTROL_GRAPPLE, fVal );
				SetControlValue( ANIMCONTROL_GRAPPLE_LOWER, fVal );
			}
			
			fVal = GetControlValue( ANIMCONTROL_GRAPPLE_FLY );
			if( fVal > 0.0f ) {
				fVal -= FLoop_fPreviousLoopSecs * _GRAPPLE_FLY_BLENDOUT_RATE;
				FMATH_CLAMP_MIN0( fVal );
				SetControlValue( ANIMCONTROL_GRAPPLE_FLY, fVal );
			}

			fVal = GetControlValue( ANIMCONTROL_GRAPPLE_HANG );
			if( fVal > 0.0f ) {
				fVal -= FLoop_fPreviousLoopSecs * _GRAPPLE_FLY_BLENDOUT_RATE;
				FMATH_CLAMP_MIN0( fVal );
				SetControlValue( ANIMCONTROL_GRAPPLE_HANG, fVal );
			}

			if( !m_pGrapple->IsReloading() && (m_fControls_Fire2 > 0.25f) ) {
				UpdateUnitTime( ANIMTAP_GRAPPLE, 0.0f );
				m_eGrappleState = GRAPPLESTATE_AIMING;
			}
			break;

		case GRAPPLESTATE_AIMING:
			_CheckGrapplePos();

			// blend in grapple aim animation
			fVal = GetControlValue( ANIMCONTROL_GRAPPLE );
			if( fVal < 1.0f ) {
				fVal += FLoop_fPreviousLoopSecs * _GRAPPLE_AIM_BLENDIN_RATE;
				FMATH_CLAMP_MAX1( fVal );
				SetControlValue( ANIMCONTROL_GRAPPLE, fVal );
				SetControlValue( ANIMCONTROL_GRAPPLE_LOWER, fVal );
			}

			if( m_fControls_Fire2 < 0.25f ) {
				_FireGrapple();			// sets state to FIRING-
			}
			break;

		case GRAPPLESTATE_FIRING:

			if( m_pGrapple->GetState() == CGrapple::STATE_TIGHT ) {
				_GrappleTakeoff();		// sets state to FLYING
				////METMP
//				if( m_fControls_Fire2 > 0.2f ) {
//					_BreakGrappleLine();
//				}
				//////
			}

			if( m_pGrapple->GetState() == CGrapple::STATE_INACTIVE ) {
				_BreakGrappleLine();	// sets state to NONE
			}

			if( m_bGrappleRecoilOn ) {
				if( m_fGrappleRecoil < 1.0f ) {
					m_fGrappleRecoil += FLoop_fPreviousLoopSecs * _GRAPPLE_RECOIL_RATE;
					FMATH_CLAMP_MAX1( m_fGrappleRecoil );
				} else {
					m_bGrappleRecoilOn = FALSE;
				}
			} else {
				if( m_fGrappleRecoil > 0.0f ) {
					m_fGrappleRecoil -= FLoop_fPreviousLoopSecs * _GRAPPLE_RECOIL_RELAX_RATE; //_GRAPPLE_RECOIL_TIGHTEN_RATE;
					FMATH_CLAMP_MIN0( m_fGrappleRecoil );
				}
			}
			break;

		case GRAPPLESTATE_LAUNCHING:
			_FlyAlongCable();					// handle translation up cable
			_CalcLaunchOrientation();
			_HandleCableAnimations();			// blend into flight animation, will also check states	MEFIX:  should be handlelaunchanims
			DeltaTime( ANIMTAP_GRAPPLE_FLY );
			break;

		case GRAPPLESTATE_FLYING:
			_FlyAlongCable();					// handle translation up cable
			DeltaTime( ANIMTAP_GRAPPLE_FLY );

			// for now, don't do anything else
			if( m_fControls_Fire2 > 0.25f ) {
				_BreakGrappleLine();	// sets state to NONE 
			}

			#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
				ftext_DebugPrintf( 0.2f, 0.5f, "~w1Yaw: %0.2f, Pitch: %0.2f", m_fGrappleFlyYaw, m_fGrappleFlyPitch );
			#endif
			break;

		case GRAPPLESTATE_LANDING:
			_FlyAlongCable();					// handle translation up cable, will also check states
			_CalcLandingOrientation();
			DeltaTime( ANIMTAP_GRAPPLE_FLY );
			break;

		case GRAPPLESTATE_CLINGING:
			if( m_fControls_Fire2 > 0.25f ) {
				_BreakGrappleLine();
			}
			break;

		case GRAPPLESTATE_RELOADING:

	
		default:
			FASSERT_NOW;
	}
}


void CBotSniper::_HandleCableAnimations( void ) {
	f32 fVal;
	// recoil forward
	if( m_fGrappleRecoil > -1.0f ) {
		m_fGrappleRecoil -= FLoop_fPreviousLoopSecs * _GRAPPLE_RECOIL_TIGHTEN_RATE;
		FMATH_CLAMPMIN( m_fGrappleRecoil, -1.0f );
	}

	// deal with the fly animation
	fVal = GetControlValue( ANIMCONTROL_GRAPPLE_FLY );
	if( fVal < 1.0f ) {
		fVal += FLoop_fPreviousLoopSecs * _GRAPPLE_FLY_BLENDIN_RATE;
		FMATH_CLAMP_MAX1( fVal );

		SetControlValue( ANIMCONTROL_GRAPPLE_FLY, fVal );

//		_BuildGrappleFlightMtx( fVal );

		// get rid of leg yaw while we're at it
		if( fVal < 1.0f ) {
		} else {
			m_fLegsYaw_MS = 0.0f;
		}
	} else { 
		m_eGrappleState = GRAPPLESTATE_FLYING;
	}
}


#define _WALL_YAW_RANGE		( FMATH_HALF_PI )

void CBotSniper::_HandleClingingYaw( void ) {
	f32 fCenterYaw = fmath_Atan( m_vGrappleTgtNormal.x, m_vGrappleTgtNormal.z );
	f32 fMinYaw = fCenterYaw - FMATH_HALF_PI;
	f32 fMaxYaw = fCenterYaw + FMATH_HALF_PI;
	

	HandleYawMovement();
	f32 fTYaw = m_fMountYaw_WS;

	if( fMinYaw < -FMATH_PI ) {
		fMinYaw += FMATH_2PI;
	}

	if( fMaxYaw > FMATH_PI ) {
		fMaxYaw -= FMATH_2PI;
	}
	f32 fDYaw = m_fMountYaw_WS - fCenterYaw;
	if( fDYaw < -FMATH_PI ) {
		fDYaw += FMATH_2PI;
	}
	if( fDYaw > FMATH_PI ) {
		fDYaw -= FMATH_2PI;
	}

	if( FMATH_FABS( fDYaw ) > _WALL_YAW_RANGE ) {
		FMATH_BIPOLAR_CLAMPMAX( fDYaw, _WALL_YAW_RANGE );
		ChangeMountYaw( fCenterYaw + fDYaw );
	}

	m_fUnitWallYaw = fmath_Div( fDYaw + _WALL_YAW_RANGE, 2.0f * _WALL_YAW_RANGE );
	FMATH_CLAMP_UNIT_FLOAT( m_fUnitWallYaw );
	UpdateUnitTime( ANIMTAP_GRAPPLE_HANG, m_fUnitWallYaw );
	
//#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
//	ftext_DebugPrintf( 0.1f, 0.6f, "~w1CY: %0.2f, MY: %0.2f, DY: %0.2f, AY: %0.2f", fCenterYaw, m_fMountYaw_WS, fDYaw, fTYaw );
//	ftext_DebugPrintf( 0.1f, 0.63f, "~w1 UY:  %0.2f", m_fUnitWallYaw );
//	//ftext_DebugPrintf( 0.1f, 0.6f, "~w1CY: %0.2f, NY: %0.2f, XY: %0.2f", fCenterYaw, fMinYaw, fMaxYaw );
//	//ftext_DebugPrintf( 0.1f, 0.63f, "~w1 MY:  %0.2f", m_fMountYaw_WS );
//#endif
}





void CBotSniper::_HandleClingingPitch( void ) {
	HandlePitchMovement();
}

#define _BULLETTRAIL_LIFESPAN		( 5.0f )
#define _BULLETTRAIL_OO_LIFESPAN	( 1.0f / _BULLETTRAIL_LIFESPAN )
#define _BULLETTRAIL_VERTICES		( _BULLETTRAIL_NODES * 2 )
#define _BULLETTRAIL_RISE_SPEED		( 0.5f )

void CBotSniper::DrawEffects( void ) {
	PROTRACK_BEGINBLOCK( "SniperFX" );
	// do the bullet trails...
	BOOL bDrawingTrails = FALSE;
	FDrawVtx_t *pVtxPool;
	CFXfm xfm;

	PROTRACK_BEGINBLOCK( "Bullets" );
		if( m_uNumActiveBulletTrails > 0 ) {
			
			for( u32 i=0; i<m_uNumActiveBulletTrails; i++ ) {
				if( m_paBulletTrails[i].fUnitLifespan == 1.0f ) {
					m_uNumActiveBulletTrails--;
					m_paBulletTrails[i] = m_paBulletTrails[m_uNumActiveBulletTrails];
					i--;
				} else {
					if( !bDrawingTrails ) {
						frenderer_SetDefaultState();
						frenderer_Push( FRENDERER_DRAW, NULL );
						pVtxPool = fvtxpool_GetArray( _BULLETTRAIL_VERTICES );	// will check below whether we got some
						bDrawingTrails = TRUE;
						xfm.BuildFromMtx( CFMtx43A::m_IdentityMtx );
						xfm.PushModel();
						fdraw_Depth_EnableWriting( FALSE );
						fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
						fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST );
						fdraw_SetCullDir( FDRAW_CULLDIR_NONE );
						fdraw_SetTexture( &m_BulletTrailTex );
					}

					m_paBulletTrails[i].vStartPoint.y +=  _BULLETTRAIL_RISE_SPEED * FLoop_fPreviousLoopSecs * m_paBulletTrails[i].afTopVelocities[0];
					m_paBulletTrails[i].vEndPoint.y +=  _BULLETTRAIL_RISE_SPEED * FLoop_fPreviousLoopSecs * m_paBulletTrails[i].afBottomVelocities[0];

					m_paBulletTrails[i].fUnitLifespan += FLoop_fPreviousLoopSecs * _BULLETTRAIL_OO_LIFESPAN;
					FMATH_CLAMP_MAX1( m_paBulletTrails[i].fUnitLifespan );
					_DrawBulletTrail( &(m_paBulletTrails[i]), pVtxPool );
				}
			}

			if( bDrawingTrails ) {
				if( pVtxPool ) {
					fvtxpool_ReturnArray( pVtxPool );
				}
				CFXfm::PopModel();
				frenderer_Pop();
			}
		}
	PROTRACK_ENDBLOCK(); //"Bullets"

	PROTRACK_BEGINBLOCK( "Lasers" );
		_DrawLaserSights();
	PROTRACK_ENDBLOCK(); //Lasers

	PROTRACK_BEGINBLOCK( "Grapple" );
		CGrapple::DrawAll();
	PROTRACK_ENDBLOCK();//( "Grapple" )
	
	PROTRACK_ENDBLOCK(); //"SniperFX"
}

#define _BULLETTRAIL_INITIAL_ALPHA 0.33f

void CBotSniper::_DrawBulletTrail( const BulletTrail_t *pBTrail, FDrawVtx_t *pVtxArray) {
	if( !pVtxArray ) {
		// don't have any verts to use, just quit
		return;
	}
	u32 i;
    f32 fLength;
	CFVec3A vAxis;
	CFVec3A vDrawAxis;
	CFVec3A vTmp;
	vAxis.Sub( pBTrail->vEndPoint, pBTrail->vStartPoint );
	fLength = vAxis.SafeUnitAndMag( vAxis );
	f32 fDistPerNode = fmath_Div( fLength, (f32)(_BULLETTRAIL_NODES-1) );

	if( fLength < 0.01f ) {
		return;
	}
	CFMtx43A::m_Temp.UnitMtxFromUnitVec( &vAxis );
	
	//calculate the first two points
	FDrawVtx_t *pVtx = pVtxArray;
	CFVec3A vPt;

	CFCamera *pCurCam = gamecam_GetActiveCamera();
	if( !pCurCam ) {
		return;		// no camera, if a vertex is drawn in the forest, and no one's there to see it...
	}

	vTmp.Sub( pBTrail->vStartPoint, pCurCam->GetFinalXfm()->m_MtxR.m_vPos );
	vDrawAxis.Cross( vTmp, vAxis );
    
	f32 fMag2 = vDrawAxis.MagSq();
	if( fMag2 < 0.0001f ) {
		// wouldn't be able to see it anyway
		return;
	}

	vDrawAxis.Mul( fmath_InvSqrt( fMag2 ) );
	//METMP
	CFMtx43A::m_Temp.m_vUp = vDrawAxis;

	f32 fSpreadFactor = 0.25f + fmath_UnitLinearToSCurve( pBTrail->fUnitLifespan ) * 2.0f;
	f32 fAlpha = _BULLETTRAIL_INITIAL_ALPHA - (_BULLETTRAIL_INITIAL_ALPHA * fmath_Sqrt( pBTrail->fUnitLifespan ));
	FMATH_CLAMP_UNIT_FLOAT( fAlpha );

	// along up axis...
	for( i=0; i<_BULLETTRAIL_NODES; i++ ) {
		vPt.Mul( vAxis, fDistPerNode * (f32)i ).Add( pBTrail->vStartPoint );
        
		pVtx->Pos_MS.Mul( CFMtx43A::m_Temp.m_vUp.v3, fSpreadFactor * pBTrail->afTopVelocities[i] );
		pVtx->Pos_MS.Add( vPt.v3 );
		pVtx->ColorRGBA.OpaqueWhite();
		if( (i == 0) || (i==_BULLETTRAIL_NODES-1) ) {
			pVtx->ColorRGBA.fAlpha = 0.0f;
		} else {
			pVtx->ColorRGBA.fAlpha = fAlpha;
		}
		pVtx->ST.Set( (f32)i * 10.0f, 0.0f );
		pVtx++;

		pVtx->Pos_MS.Mul( CFMtx43A::m_Temp.m_vUp.v3, -(fSpreadFactor * pBTrail->afTopVelocities[i]) );
		pVtx->Pos_MS.Add( vPt.v3 );
		pVtx->ColorRGBA.OpaqueWhite();

		if( (i == 0) || (i==_BULLETTRAIL_NODES-1) ) {
			pVtx->ColorRGBA.fAlpha = 0.0f;
		} else {
			pVtx->ColorRGBA.fAlpha = fAlpha;
		}

		pVtx->ST.Set( (f32)i * 10.0f, 1.0f );
		pVtx++;
	}

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, pVtxArray, _BULLETTRAIL_NODES * 2 );        
}

void CBotSniper::_DrawLaserSights( void ) {
	CFXfm xfm;
	FDrawVtx_t *pVtxArray;

	if( m_uNumSightsActive == 0 ) {
		return;
	}

	pVtxArray = fvtxpool_GetArray( m_uNumSightsActive * 4 );
	if( !pVtxArray ) {
		m_uNumSightsActive = 0;
		return;
	}

	frenderer_SetDefaultState();
	frenderer_Push( FRENDERER_DRAW, NULL );
	xfm.BuildFromMtx( CFMtx43A::m_IdentityMtx );
	xfm.PushModel();
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST );
	fdraw_SetCullDir( FDRAW_CULLDIR_NONE );
	fdraw_SetTexture( &m_LaserSightTex );

	FDrawVtx_t *pVtx = pVtxArray;
	for( u32 i=0; i<m_uNumSightsActive; i++ ) {
		pVtx->Pos_MS = m_avLaserPt[i*4].v3;
		pVtx->ColorRGBA.OpaqueRed();
		pVtx->ST.Set( 1.0f, 1.0f );
		pVtx++;

		pVtx->Pos_MS = m_avLaserPt[i*4+1].v3;
		pVtx->ColorRGBA.OpaqueRed();
		pVtx->ST.Set( 1.0f, 0.0f );
		pVtx++;

		pVtx->Pos_MS = m_avLaserPt[i*4+2].v3;
		pVtx->ColorRGBA.OpaqueRed();
		pVtx->ST.Set( 0.0f, 1.0f );
		pVtx++;

		pVtx->Pos_MS = m_avLaserPt[i*4+3].v3;
		pVtx->ColorRGBA.OpaqueRed();
		pVtx->ST.Set( 0.0f, 0.0f );
		pVtx++;

		fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, &pVtxArray[i*4], 4 );
	}

	m_uNumSightsActive = 0;

	fvtxpool_ReturnArray( pVtxArray );
	CFXfm::PopModel();
	frenderer_Pop();
}


#define _LASER_SIGHT_SIZE		( 2.0f )
#define _LASER_SIGHT_PUSH		( 0.25f )
#define _LASER_SIGHT_SCALE		( 0.0057f )

void CBotSniper::_CalcLaserPt( void ) {
	// based on target point.  Find the normal, then calculate the four corners
	// only do this in scope mode
	// PC port: this checked m_uNumActiveBulletTrails, so the sight count (reset only when the sights
	// are drawn) ran past m_avLaserPt[] every frame and overwrote other statics (tracers, damage forms).
	if( m_uNumSightsActive >= MAX_LASER_SIGHTS ) {
		return;
	}

	u32 uPtIdx = 4 * m_uNumSightsActive;
	FCollImpact_t impact;
	CFVec3A vRayStart, vRayEnd;

	vRayStart.Mul( m_TargetLockUnitVec_WS, -1.0f ).Add( m_TargetedPoint_WS );
	vRayEnd.Add( m_TargetLockUnitVec_WS, m_TargetedPoint_WS );
	CFVec3A vPushedImpactPt;

	if( fworld_FindClosestImpactPointToRayStart( &impact, &vRayStart, &vRayEnd, 0, NULL, TRUE, NULL, FCOLL_USER_TYPE_BITS_ALL ) ) {
		CFMtx43A::m_Temp.UnitMtxFromUnitVec( &(impact.UnitFaceNormal) );

		// scale based on distance
		CFCamera *pCamera;

		pCamera = gamecam_GetActiveCamera();
		
		FASSERT( pCamera );

		f32 fDistFromCamera;
		f32 fScaleFactor;
		fDistFromCamera = impact.ImpactPoint.Dist(  pCamera->GetFinalXfm()->m_MtxR.m_vPos );
		fScaleFactor = fDistFromCamera * _LASER_SIGHT_SCALE;

		vPushedImpactPt.Mul( CFMtx43A::m_Temp.m_vFront, _LASER_SIGHT_PUSH ).Add( impact.ImpactPoint );
		//m_avLaserPt[uPtIdx++].Mul( CFMtx43A::m_Temp.m_vRight, _LASER_SIGHT_SIZE ).Add( vPushedImpactPt );
		//m_avLaserPt[uPtIdx++].Mul( CFMtx43A::m_Temp.m_vUp, _LASER_SIGHT_SIZE ).Add( vPushedImpactPt );
		//m_avLaserPt[uPtIdx++].Mul( CFMtx43A::m_Temp.m_vUp, -_LASER_SIGHT_SIZE ).Add( vPushedImpactPt );
		//m_avLaserPt[uPtIdx++].Mul( CFMtx43A::m_Temp.m_vRight, -_LASER_SIGHT_SIZE ).Add( vPushedImpactPt );
		m_avLaserPt[uPtIdx++].Mul( CFMtx43A::m_Temp.m_vRight, fScaleFactor ).Add( vPushedImpactPt );
		m_avLaserPt[uPtIdx++].Mul( CFMtx43A::m_Temp.m_vUp, fScaleFactor ).Add( vPushedImpactPt );
		m_avLaserPt[uPtIdx++].Mul( CFMtx43A::m_Temp.m_vUp, -fScaleFactor ).Add( vPushedImpactPt );
		m_avLaserPt[uPtIdx++].Mul( CFMtx43A::m_Temp.m_vRight, -fScaleFactor ).Add( vPushedImpactPt );

		m_uNumSightsActive++;
	}
}



/////////////////////////////////
//DEBUG

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	static f32 _fDbgTime = 1.0f;
#endif

void CBotSniper::_DebugAnimData( void ) {
//#if 0
#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	u32 uCtr = 0;

	if( m_bControls_Human && ((CHumanControl*)Controls())->m_nPadFlagsSelect1 & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
		picklevel_Start();
	//}
		//if( _fDbgTime == 1.0f ) {
		//	_fDbgTime = 0.1f;
		//} else {
		//	_fDbgTime = 1.0f;
		//}
	}

//	floop_SetTimeScale( _fDbgTime );


	for( u32 i=0; i<ANIMCONTROL_BASE_COUNT; i++ ) {
		if( GetControlValue( i ) != 0.0f ) {
			ftext_DebugPrintf( 0.4f, 0.33f + (0.03f * uCtr++), "~w1ctl:  %s, %0.2f", m_apszBaseControlNameTable[i], GetControlValue( i ) );
		}
	}

	//f32 fModelYaw = fmath_Atan( m_MtxToWorld.m_vFront.x, m_MtxToWorld.m_vFront.z );
	//			
	//ftext_DebugPrintf( 0.1f, 0.5f, "~w1TY: %0.2f, MY: %0.2f", m_fMountYaw_WS, fModelYaw );
	//SetControlValue( ANIMCONTROL_AIM_SUMMER, 1.0f ) ;
	//SetControlValue( ANIMCONTROL_RECOIL_SUMMER, 0.0f );

	//TEMP
	//if( m_bControls_Human && ((CHumanControl*)Controls())->m_nPadFlagsAction ) {
	//	Power( FALSE, 5.0f );
	////	picklevel_Start();
	//}


#endif
}


void CBotSniper::DebugDraw( CBotSniper *pSniper ) {
	if( !pSniper || !(pSniper->TypeBits() & ENTITY_BIT_BOTSNIPER) ) {
		return;
	}
/*
	CFVec3A vTmp;
	vTmp.Mul( pSniper->MtxToWorld()->m_vFront, 1000.0f ).Add( pSniper->m_MtxToWorld.m_vPos );
	fdraw_SolidLine( &pSniper->m_MtxToWorld.m_vPos.v3, &vTmp.v3, &FColor_MotifRed );

	vTmp = pSniper->m_vGrappleClingPosWall_MS;
	CFMtx43A mTmp;
	mTmp = pSniper->m_MtxToWorld;
	//mTmp.m_vPos.Zero();
	mTmp.MulPoint( vTmp );
	//vTmp.Add( pSniper->m_MtxToWorld.m_vPos, vTmp );
	fdraw_FacetedWireSphere( &(vTmp.v3), 1.0f, 4, 4, &FColor_MotifRed );

	fdraw_FacetedWireSphere( &pSniper->m_TargetedPoint_WS.v3, 4.0f, 4, 4, &FColor_MotifRed );
	vTmp = pSniper->m_pWorldMesh->GetBoneMtxPalette()[pSniper->m_nBoneIdxRightArm]->m_vPos;
	fdraw_SolidLine( &vTmp.v3, &pSniper->m_TargetedPoint_WS.v3, &FColor_MotifRed );

	vTmp = pSniper->m_pWorldMesh->GetBoneMtxPalette()[pSniper->m_nBoneIdxRightArm]->m_vUp;
	vTmp.Negate().Mul( 1000.0f ).Add( pSniper->m_pWorldMesh->GetBoneMtxPalette()[pSniper->m_nBoneIdxRightArm]->m_vPos );
	fdraw_SolidLine( &pSniper->m_pWorldMesh->GetBoneMtxPalette()[pSniper->m_nBoneIdxRightArm]->m_vPos.v3, &vTmp.v3, &FColor_MotifBlue );
*/
	
	//for( u32 i=0; i<pSniper->m_uNumPtsInGrappleLine; i++ ) {
	//	fdraw_FacetedWireSphere( &(pSniper->m_avGrappleLinePts[i].v3), 1.0f, 4, 4, &FColor_MotifRed );
	//}

	//if( pSniper->m_uNumPtsInGrappleLine > 0 ) {
	//	fdraw_FacetedWireSphere( &(pSniper->m_vGrapplePos.v3), 4.0f, 4, 4, &FColor_MotifBlue );
	//}



	//if( pSniper->m_bValidGrapplePos ) {
	//	fdraw_FacetedWireSphere( &pSniper->m_vGrappleTgtPos.v3, 3.0f, 4, 4, &FColor_MotifBlue );
	//}

	//fdraw_FacetedWireSphere( &_vDbgPt0.v3, 3.0f, 4, 4, &FColor_MotifRed );
	//fdraw_FacetedWireSphere( &_vDbgPt1.v3, 3.0f, 4, 4, &FColor_MotifRed );
	//fdraw_FacetedWireSphere( &_vDbgPt2.v3, 3.0f, 4, 4, &FColor_MotifRed );
	//fdraw_FacetedWireSphere( &_vDbgPt3.v3, 3.0f, 4, 4, &FColor_MotifRed );
}



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotSniperBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

void CBotSniperBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_BOTSNIPER, pszEntityType );

	// Override defaults set by our parent classes...
	m_nEC_HealthContainerCount = CBotSniper::m_BotInfo_Gen.nBatteryCount;
	m_pszEC_ArmorProfile = CBotSniper::m_BotInfo_Gen.pszArmorProfile;
#if FANG_WINGC
	// The retail b_sniper gen table (cut content, shared with the Swarmer Queen's) names the
	// "None_Im_Invincible" armor profile, so a revived Sniper could never be killed. Use the Grunt's.
	if( !m_pszEC_ArmorProfile || !fclib_stricmp( m_pszEC_ArmorProfile, "None_Im_Invincible" ) ) {
		m_pszEC_ArmorProfile = "Grunt";
	}
#endif
}


BOOL CBotSniperBuilder::PostInterpretFixup( void ) {
	if( !CBotBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	return TRUE;

	// Error:
_ExitWithError:
	FASSERT_NOW;
	return FALSE;
}


BOOL CBotSniperBuilder::InterpretTable( void ) {
	return CBotBuilder::InterpretTable();
}

























//////////////////////////////////////////////////
//DELETED
#if 0
















void CBotSniper::_BuildGrappleFlightMtx( f32 fUnitTime ) {
	//f32 fMag2;
	//CFMtx43A::m_Temp.m_vUp.Sub( m_vGrappleTgtPos, m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxGrapple]->m_vPos );
	//fMag2 = CFMtx43A::m_Temp.m_vUp.MagSq();
	//if( fMag2 < 0.01f ) {
	//	return;
	//}

	//CFMtx43A::m_Temp.m_vUp.Mul( fmath_InvSqrt( fMag2 ) );
	//CFMtx43A::m_Temp.m_vRight.CrossYWithVec( CFMtx43A::m_Temp.m_vUp );
	//fMag2 = CFMtx43A::m_Temp.m_vRight.MagSq();
	//if( fMag2 < 0.0001f ) {
	//	// this shouldn't ever happen
	//	return;
	//}

	//CFQuatA qFrom, qTo;


	//CFMtx43A::m_Temp.m_vRight.Mul( fMag2 );
	//CFMtx43A::m_Temp.m_vFront.Cross( CFMtx43A::m_Temp.m_vRight, CFMtx43A::m_Temp.m_vUp );
	//qTo.BuildQuat( CFMtx43A::m_Temp );

	//// build the matrix for where the bot wants to be
	//CFMtx43A::m_XlatRotY.SetRotationY( m_fMountYaw_WS + m_fLegsYaw_MS );
	//qFrom.BuildQuat( CFMtx43A::m_XlatRotY );


	//

	//qTo.ReceiveSlerpOf( fUnitTime, qFrom, qTo );

	//qTo.BuildMtx33( m_mtxGrappleFly );




	//
	//// swing mount yaw around so bot is headed in the correct direction
	//if( fVal < 1.0f ) {
	//	f32 fD = m_fMountYaw_WS - m_fGrappleFlyYaw;
	//	if( fD > FMATH_PI ) {
	//		fD = (fD-FMATH_2PI);
	//	} else if ( fD < -FMATH_PI ) {
	//		fD = (fD+FMATH_2PI);
	//	}
	//	fD = fmath_Div( fD, 1.0f - fVal ) * FLoop_fPreviousLoopSecs * _GRAPPLE_FLY_BLENDIN_RATE;
	//	m_fMountYaw_WS -= fD;
	//} else {
	//	m_fMountYaw_WS = m_fGrappleFlyYaw;
	//}

	//// adjust model pitch so bot is angled correctly
	//if( fVal < 1.0f ) {
	//	f32 fD = m_fModelPitch - m_fGrappleFlyPitch;
	//	fD = fmath_Div( fD, 1.0f - fVal ) * FLoop_fPreviousLoopSecs * _GRAPPLE_FLY_BLENDIN_RATE;
	//	m_fModelPitch -= fD;
	//} else {
	//	m_fModelPitch = m_fGrappleFlyPitch;
	//}

	//// get rid of leg yaw while we're at it
	//if( fVal < 1.0f ) {
	//} else {
	//	m_fLegsYaw_MS = 0.0f;
	//}

	//m_mtxGrappleFly.SetRotationYXZ( m_fMountYaw_WS, FMATH_HALF_PI - m_fModelPitch, 0.0f );
	//CFMtx43A::m_Temp.m_vPos = m_MountPos_WS;
	//m_pWorldMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_Temp );
	//m_pWorldMesh->UpdateTracker();

	//	m_mtxGrappleFly
}







































#endif

















