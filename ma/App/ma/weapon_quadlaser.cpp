//////////////////////////////////////////////////////////////////////////////////////
// weapon_quadlaser.cpp - Predator's Quad Laser weapon.
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
// 12/30/02 Elliott		Created.
//////////////////////////////////////////////////////////////////////////////////////


#include "fang.h"
#include "weapon_quadlaser.h"
#include "bot.h"
#include "fforce.h"
#include "potmark.h"
#include "fworld_coll.h"
#include "fresload.h"
#include "meshtypes.h"
#include "AI/AIEnviro.h"
#include "eparticle.h"
#include "player.h"


#define _USER_PROP_FILENAME		"w_quadlaser.csv"
#define _TRACERS_IN_POOL		30

#define _AIM_ON_SEG0			0.1f
#define _AIM_ON_SEG1			0.0f
#define _AIM_ON_SEG2			0.7f

#define _AIM_OFF_SEG0			0.75f
#define _AIM_OFF_SEG1			0.75f
#define _AIM_OFF_SEG2			1.0f

#define _OO_AIM_INTERVAL_SEG0	( 1.0f / (_AIM_OFF_SEG0 - _AIM_ON_SEG0) )
#define _OO_AIM_INTERVAL_SEG1	( 1.0f / (_AIM_OFF_SEG1 - _AIM_ON_SEG1) )
#define _OO_AIM_INTERVAL_SEG2	( 1.0f / (_AIM_OFF_SEG2 - _AIM_ON_SEG2) )

#define _AIM_SEG0_XROT_TL		FMATH_DEG2RAD( -30.0f )
#define _AIM_SEG0_XROT_TR		FMATH_DEG2RAD( 30.0f )
#define _AIM_SEG0_XROT_BL		FMATH_DEG2RAD( 30.0f )
#define _AIM_SEG0_XROT_BR		FMATH_DEG2RAD( -30.0f )

#define _AIM_ELBOW_ANGLE_MAX	FMATH_DEG2RAD( -60.0f )
#define _AIM_ELBOW_ANGLE_MIN	FMATH_DEG2RAD( -45.0f )

#define _AIM_MAX_DISTSQ			(250.0f * 250.0f)
#define _AIM_MIN_DISTSQ			(75.0f * 75.0f)

#define _RECOIL_OO_ON_TIME		(1.0f/0.05f)
#define _RECOIL_OO_OFF_TIME		(1.0f/0.25f)
#define _RECOIL_ANGLE_0			FMATH_DEG2RAD( -25.0f )
#define _RECOIL_ANGLE_1			FMATH_DEG2RAD( -5.0f )
#define _RECOIL_SEC_MOD			0.33f							// modifies how much recoil is produced for secondary fire

#define	_MAX_CHARGE				20
#define _OO_MAX_CHARGE			(1.0f/(f32)_MAX_CHARGE)

#define _AIM_SPEED				10000.0f							// the speed that each arm's aim point can travel in world space each second at max range

#define _SHOTS_PER_SOUND_PRI	3
#define _SHOTS_PER_SOUND_SEC	2

#define _SMOKE_TEXTURE			"tfp1smoke01"

static CWeaponQuadLaserBuilder _WeaponQuadLaserBuilder;

///////////////////////////
//STATIC DATA

BOOL CWeaponQuadLaser::m_bSystemInitialized	= FALSE;

_TracerDef_s			CWeaponQuadLaser::m_aTracerDefPri[EUK_COUNT_QUADLASER];
_TracerDef_s			CWeaponQuadLaser::m_aTracerDefSec[EUK_COUNT_QUADLASER];

TracerGroupHandle_t		CWeaponQuadLaser::m_hTracerGroupPri[EUK_COUNT_QUADLASER];
TracerGroupHandle_t		CWeaponQuadLaser::m_hTracerGroupSec[EUK_COUNT_QUADLASER];


CFTexInst				CWeaponQuadLaser::m_aTracerTexInstPri[EUK_COUNT_QUADLASER];
CFTexInst				CWeaponQuadLaser::m_aTracerTexInstSec[EUK_COUNT_QUADLASER];

FParticle_DefHandle_t	CWeaponQuadLaser::m_ahFireParticleDef[EUK_COUNT_QUADLASER];
SmokeTrailAttrib_t		CWeaponQuadLaser::m_SmokeTrailAttrib;
u32						CWeaponQuadLaser::m_uWpnClassClientCount = 0;

CWeaponQuadLaser::_UserProps_t CWeaponQuadLaser::m_aUserProps[EUK_COUNT_QUADLASER];

const FGameData_TableEntry_t CWeaponQuadLaser::m_aUserPropVocab[] = {
	//cchar *pszMeshName;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//f32 fOORoundsPerSecPrimary;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_OO_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_100,

	//f32 fOORoundsPerSecSecondary;		
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_OO_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_100,

	//f32 fOOAimSpreadTime;				
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_OO_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_100,

	//f32 fAimRecoveryTime;				
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_OO_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_100,

	//f32 fMaxShotSpread;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_100000,

	//cchar *pszFireParticle;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//cchar *pszPriTracerTex;				
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//cchar *pszSecTracerTex;				
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//f32	fMaxLiveRange;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_100000,

	//f32 fPriTracerLength;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	//f32 fPriTracerWidth;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	//f32 fPriTracerSpeed;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	//f32 fSecTracerLength;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	//f32 fSecTracerWidth;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	//f32 fSecTracerSpeed;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	//f32 fMinTargetAssistDist;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	//f32 fMaxTargetAssistDist;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	//f32 fRechargeTime;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	//f32	fPrimaryDischargeTime;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	//f32 fSecondaryDischargeTime;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	//f32 fSecondaryFireRechargeTime;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_OO_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ) * 2,
	F32_DATATABLE_0,
	F32_DATATABLE_100000,


	//f32 fSecondaryFireBarrageTime;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_OO_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ) * 2,
	F32_DATATABLE_0,
	F32_DATATABLE_100000,


	FGAMEDATA_VOCAB_DAMAGE,			// pDamageProfile1
	FGAMEDATA_VOCAB_DAMAGE,			// pDamag eProfile2
	FGAMEDATA_VOCAB_EXPLODE_GROUP,	// hExplosionGroup
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupFire1
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupFire2

	FGAMEDATA_VOCAB_DECAL_DEF,		// hDecalDef


	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameDataMap_t CWeaponQuadLaser::m_aUserPropMapTable[] = {

	"QuadLaserPredator",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[0],

	"QuadLaserPredator_P",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[1],

	NULL
};


////////////////////////////
//Static FNs

BOOL CWeaponQuadLaser::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );

	m_bSystemInitialized = TRUE;
	m_uWpnClassClientCount = 0;

	_InitSmokeTrail();

	return TRUE;

}


void CWeaponQuadLaser::UninitSystem( void ) {
	if( IsSystemInitialized() ) {
		m_bSystemInitialized = FALSE;
	}
}


BOOL CWeaponQuadLaser::ClassHierarchyLoadSharedResources( void ) {
	u32 i;
	FASSERT( m_bSystemInitialized );
	FASSERT( m_uWpnClassClientCount != 0xffffffff );

	m_uWpnClassClientCount++;

	if( !CWeapon::ClassHierarchyLoadSharedResources() ) {
		// Bail now since parent class has already called
		// ClassHierarchyUnloadSharedResources() and decremented
		// our client counter...

		return FALSE;
	}

	if( m_uWpnClassClientCount > 1 ) {
		// Resources already loaded...

		return TRUE;
	}

	for( i=0; i<EUK_COUNT_QUADLASER; i++ ) {
		m_hTracerGroupPri[i] = TRACER_NULLGROUPHANDLE;
		m_hTracerGroupSec[i] = TRACER_NULLGROUPHANDLE;
	}

	// Resources not yet loaded...
	Info_t		*pInfo;
	FTexDef_t	*pTexDef;

	FResFrame_t ResFrame = fres_GetFrame();

	// Read the user properties for all EUK levels of this weapon...
	if( !fgamedata_ReadFileUsingMap( m_aUserPropMapTable, _USER_PROP_FILENAME ) ) {
		DEVPRINTF( "WeaponQuadLaser::ClassHierarchyLoadSharedResources(): Couldn't load user props from %s\n", _USER_PROP_FILENAME );
		goto _ExitWithError;
	}

	// Do this for each EUK level...
	for( i=0; i<EUK_COUNT_QUADLASER; i++ ) {
		m_aTracerDefPri[i].pUser					= NULL;
		m_aTracerDefPri[i].pFcnKillCallback			= _TracerKilledCallbackPrimary;
		m_aTracerDefPri[i].pFcnBuildSkipList		= _TracerBuildTrackerSkipList;
		m_aTracerDefPri[i].fWidth_WS				= m_aUserProps[i].fPriTracerWidth;
		m_aTracerDefPri[i].fLength_WS				= m_aUserProps[i].fPriTracerLength;
		m_aTracerDefPri[i].fSpeed_WS				= m_aUserProps[i].fPriTracerSpeed;
		m_aTracerDefPri[i].fMaxTailDist_WS			= m_aUserProps[i].fMaxLiveRange;
		m_aTracerDefPri[i].fBeginDeathUnitFade_WS	= 0.25f;
		m_aTracerDefPri[i].uFlags					= TRACERFLAG_DRAW_ADDITIVE | TRACERFLAG_THICK_PROJECTILE;
		m_aTracerDefPri[i].ColorRGBA.OpaqueWhite();

		m_aTracerDefSec[i].pUser					= NULL;
		m_aTracerDefSec[i].pFcnKillCallback			= _TracerKilledCallbackSecondary;
		m_aTracerDefSec[i].pFcnBuildSkipList		= _TracerBuildTrackerSkipList;
		m_aTracerDefSec[i].fWidth_WS				= m_aUserProps[i].fSecTracerWidth;
		m_aTracerDefSec[i].fLength_WS				= m_aUserProps[i].fSecTracerLength;
		m_aTracerDefSec[i].fSpeed_WS				= m_aUserProps[i].fSecTracerSpeed;
		m_aTracerDefSec[i].fMaxTailDist_WS			= m_aUserProps[i].fMaxLiveRange;
		m_aTracerDefSec[i].fBeginDeathUnitFade_WS	= 0.25f;
		m_aTracerDefSec[i].uFlags					= TRACERFLAG_THICK_PROJECTILE;
		m_aTracerDefSec[i].ColorRGBA.OpaqueWhite();

		pTexDef = (FTexDef_t *)(fresload_Load(FTEX_RESNAME, m_aUserProps[i].pszPriTracerTex));
		if(pTexDef == NULL) {
				DEVPRINTF("CWeaponQuadLaser::ClassHierarchyLoadSharedResources() : Could not load tracer texture '%s'.\n", m_aUserProps[i].pszPriTracerTex);
		} else {
			m_aTracerTexInstPri[i].SetTexDef( pTexDef );
		}

		pTexDef = (FTexDef_t *)(fresload_Load(FTEX_RESNAME, m_aUserProps[i].pszSecTracerTex));
		if(pTexDef == NULL) {
				DEVPRINTF("CWeaponQuadLaser::ClassHierarchyLoadSharedResources() : Could not load tracer texture '%s'.\n", m_aUserProps[i].pszSecTracerTex);
		} else {
			m_aTracerTexInstSec[i].SetTexDef( pTexDef );
		}

		m_hTracerGroupPri[i]	= tracer_CreateGroup( &m_aTracerTexInstPri[i],  8 );
		m_hTracerGroupSec[i]	= tracer_CreateGroup( &m_aTracerTexInstSec[i], _TRACERS_IN_POOL );

		if( (m_hTracerGroupPri[i] == TRACER_NULLGROUPHANDLE) ||
			(m_hTracerGroupSec[i] == TRACER_NULLGROUPHANDLE) ) {
			DEVPRINTF( "CWeaponQuadLaser::ClassHierarchyLoadSharedResources(): Could not create tracer group.\n" );
			goto _ExitWithError;
		}

		// load particles
		m_ahFireParticleDef[i] = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, m_aUserProps[i].pszFireParticle );
		if( m_ahFireParticleDef[i] == FPARTICLE_INVALID_HANDLE ) {
			DEVPRINTF( "WeaponQuadLaser::ClassHierarchyLoadSharedResources(): Couldn't load fire particle %s\n", m_aUserProps[i].pszFireParticle );
			goto _ExitWithError;
		}

		// Fill out our global info data...
		pInfo = &m_aaInfo[WEAPON_TYPE_QUADLASER][i];

		// set up pInfo...
		pInfo->nGrip				= GRIP_EMBEDDED;
		pInfo->nReloadType			= RELOAD_TYPE_FULLYAUTOMATIC;
		pInfo->nStanceType			= STANCE_TYPE_STANDARD;
		pInfo->fMinTargetAssistDist = m_aUserProps[i].fMinTargetAssistDist;
		pInfo->fMaxTargetAssistDist = m_aUserProps[i].fMaxTargetAssistDist;
		pInfo->fMaxLiveRange		= m_aUserProps[i].fMaxLiveRange;
		pInfo->nClipAmmoMax			= _MAX_CHARGE;
		pInfo->nReserveAmmoMax		= 0;
		pInfo->nInfoFlags			= INFOFLAG_THICK_TARGETING;
		pInfo->nReticleType			= CReticle::TYPE_QUADLASER;
	}


	// load smoke trail texture
	m_SmokeTrailAttrib.pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, _SMOKE_TEXTURE );
	if( m_SmokeTrailAttrib.pTexDef == NULL ) {
		DEVPRINTF( "CWeaponQuadLaser::ClassHierarchyLoadSharedResources(): Could not load smoke trail texture.\n" );
		return FALSE;
	}


	return TRUE;
	
	// Error:
_ExitWithError:
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( ResFrame );

	return FALSE;

}

void CWeaponQuadLaser::ClassHierarchyUnloadSharedResources( void ) {
	FASSERT( m_uWpnClassClientCount > 0 );

	--m_uWpnClassClientCount;

	if( m_uWpnClassClientCount > 0 ) {
		return;
	}


	u32 i;

	for( i=0; i<EUK_COUNT_QUADLASER; i++ ) {
		tracer_DestroyGroup( m_hTracerGroupPri[i] );
		tracer_DestroyGroup( m_hTracerGroupSec[i] );
		m_hTracerGroupPri[i] = TRACER_NULLGROUPHANDLE;
		m_hTracerGroupSec[i] = TRACER_NULLGROUPHANDLE;
	}

	CWeapon::ClassHierarchyUnloadSharedResources();
}



/////////////////////////
//VIRTUAL FNs

CEntityBuilder* CWeaponQuadLaser::GetLeafClassBuilder( void ) {
	return &_WeaponQuadLaserBuilder;
}


BOOL CWeaponQuadLaser::ClassHierarchyBuild( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	u32 i;

	FResFrame_t resframe = fres_GetFrame();
	
	// Get pointer to the leaf class's builder object...
	CWeaponQuadLaserBuilder *pBuilder = (CWeaponQuadLaserBuilder *)GetLeafClassBuilder();

	// Build parent class...
	if( !CWeapon::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	for( i=0; i<EUK_COUNT_QUADLASER; i++ ) {
		_ResourceData_t *pResourceData;
		Info_t			*pInfo;
        
		pResourceData = &m_aResourceData[i];
		pInfo		  = &m_aaInfo[WEAPON_TYPE_QUADLASER][i];

		// if required, create the worldmesh
		if( m_aUserProps[i].pszMeshName[0] != NULL ) {
			// currently, doesn't happen, but if it did, load & init a mesh
		}
	}

	m_pResourceData = &m_aResourceData[m_nUpgradeLevel];
	m_pUserProps	= &m_aUserProps[m_nUpgradeLevel];
	
	fforce_NullHandle( &m_hForce );

	////Initialize our muzzle smoke particles
	for( i=0; i<ARM_COUNT; i++ ) {
		m_apMuzzleSmokeParticles[i] = fnew CEParticle();
		if( m_apMuzzleSmokeParticles[i] == NULL ) {
			DEVPRINTF( "CWeaponQuadLaser::ClassHierarchyBuild(): Could not allocate muzzle smoke particles.\n" );
			goto _ExitWithError;
		}
		m_apMuzzleSmokeParticles[i]->Create();
	}

	SetClipAmmo( _MAX_CHARGE );

	
	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( resframe );
	return FALSE;
}


BOOL CWeaponQuadLaser::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CWeapon::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	EnableOurWorkBit();

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CWeaponQuadLaser::ClassHierarchyDestroy( void ) {
	u32 i;

	fforce_Kill( &m_hForce );

	// Destroy ourselves first...
	for(i=0; i<EUK_COUNT_QUADLASER; ++i)
	{
		if( m_aResourceData[i].m_pWorldMesh ) {
			fdelete( m_aResourceData[i].m_pWorldMesh );
			m_aResourceData[i].m_pWorldMesh = NULL;
		}
	}

	for( i=0; i<ARM_COUNT; i++ ) {
		if( m_apMuzzleSmokeParticles[i] != NULL ) {
			fdelete( m_apMuzzleSmokeParticles[i] );
			m_apMuzzleSmokeParticles[i] = NULL;
		}
	}

	_ClearDataMembers();

	// Now destroy our parent...
	CWeapon::ClassHierarchyDestroy();
}

void CWeaponQuadLaser::ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel ) {
	FASSERT( !IsTransitionState( CurrentState() ) );

	_ResourceData_t *pOldResourceData = &m_aResourceData[nPreviousUpgradeLevel];
	Info_t *pOldInfo = &m_aaInfo[WEAPON_TYPE_LASER][nPreviousUpgradeLevel];

	// Set up new level...
	m_pResourceData = &m_aResourceData[ m_nUpgradeLevel ];



	// Remove old mesh from world...
	if( pOldResourceData->m_pWorldMesh ) {
		pOldResourceData->m_pWorldMesh->RemoveFromWorld();

		if( IsInWorld() && m_pResourceData->m_pWorldMesh ) {
			m_pResourceData->m_pWorldMesh->m_Xfm = pOldResourceData->m_pWorldMesh->m_Xfm;
			m_pResourceData->m_pWorldMesh->UpdateTracker();
		}
	}
	
}



// Switches immediately to state CWeapon::CurrentState().
void CWeaponQuadLaser::ClassHierarchyResetToState( void ) {
	CWeapon::ClassHierarchyResetToState();
}


void CWeaponQuadLaser::KillRumble( void ) {
	FASSERT( IsCreated() );

	fforce_Kill( &m_hForce );
}


u32 CWeaponQuadLaser::TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pProjUnitDir_WS, const CFVec3A *pBuddyFirePos_WS/* = NULL*/ ) {
	FASSERT( IsCreated() );
	
	if( (fUnitTriggerVal2 > 0.25f) && (m_eSecondaryStatus == SECONDARY_READY) ) {
		if( !m_bTrigger2Down ) {
			// this is the first frame of secondary fire, initialize everything
			_StartSecondaryFire();
			m_fFireTimer		= 0.0f;
			m_uSoundCounter		= 0;
		}
		m_fUnitRecoilAdjust	= _RECOIL_SEC_MOD;
        m_bTrigger2Down		= TRUE;
		m_bTrigger1Down		= FALSE;
	} else {
		if( (m_eSecondaryStatus != SECONDARY_FIRING) && fUnitTriggerVal1 > 0.25f ) {
			if( !m_bTrigger1Down ) {
				m_fFireTimer		= 0.0f;
				m_uSoundCounter		= 0;
			}
			m_bTrigger1Down		= TRUE;
			m_fUnitRecoilAdjust = 1.0f;
		} else {
			m_bTrigger1Down = FALSE;
		}

		m_bTrigger2Down = FALSE;
	}

	return 0;
}

void CWeaponQuadLaser::ComputeMuzzlePoint_WS( u32 uArm, CFVec3A *pMuzzlePoint_WS ) const {
	FASSERT( uArm < ARM_COUNT );
    
    *pMuzzlePoint_WS = m_apMtxFireBones[m_auFirePos[uArm]]->m_vPos;
}

void CWeaponQuadLaser::ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const {
	// this is going to be kind of tricky, we need a bone
	FASSERT( m_uCurrentFireSeg < ARM_COUNT );
	*pMuzzlePoint_WS = m_apMtxFireBones[m_auFirePos[m_uCurrentFireSeg]]->m_vPos;
}

f32 CWeaponQuadLaser::GetProjectileSpeed(void) const {

	return m_aTracerDefPri[m_nUpgradeLevel].fSpeed_WS;
}


void CWeaponQuadLaser::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );
	u32 i;

	CWeapon::ClassHierarchyAddToWorld();

	if( m_pResourceData->m_pWorldMesh ) {
		m_pResourceData->m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
		m_pResourceData->m_pWorldMesh->UpdateTracker();
	}

	for( i=0; i<ARM_COUNT; i++ ) {
		m_aqArm0[i].Identity();
		m_aqArm1[i].Identity();
		m_aqArm2[i].Identity();
	}

	if( m_pOwnerBot ) {
		for( u32 i=0; i<ARM_COUNT; i++ ) {
			m_avDesiredAimPoints[i]		= m_pOwnerBot->m_TargetedPoint_WS;
			m_avAimPoints[i]			= m_pOwnerBot->m_TargetedPoint_WS;
			if( m_apMuzzleSmokeParticles[i] != NULL ) {
				m_apMuzzleSmokeParticles[i]->AddToWorld();
			}
		}
	}

	m_apMuzzleSmokeParticles[ARM_TL]->StartEmission( m_ahFireParticleDef[m_nUpgradeLevel] );
	m_apMuzzleSmokeParticles[ARM_TR]->StartEmission( m_ahFireParticleDef[m_nUpgradeLevel] );
	m_apMuzzleSmokeParticles[ARM_BL]->StartEmission( m_ahFireParticleDef[m_nUpgradeLevel] );
	m_apMuzzleSmokeParticles[ARM_BR]->StartEmission( m_ahFireParticleDef[m_nUpgradeLevel] );

	m_fFireTimer		= 0.0f;
	m_eSecondaryStatus = SECONDARY_READY;
	m_fSecondaryFireTimer = 0.0f;
	m_uLastSoundTime = 0;

}


void CWeaponQuadLaser::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	CWeapon::ClassHierarchyRemoveFromWorld();

	// Remove the weapon mesh from the world...
	if( m_pResourceData->m_pWorldMesh ) {
		m_pResourceData->m_pWorldMesh->RemoveFromWorld();
	}

	for( u32 i=0; i<ARM_COUNT; i++ ) {
		m_apMuzzleSmokeParticles[i]->StopEmission();
		m_apMuzzleSmokeParticles[i]->RemoveFromWorld();
	}
}

void CWeaponQuadLaser::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	if( m_pResourceData->m_pWorldMesh ) {
		FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
		apTrackerSkipList[nTrackerSkipListCount++] = m_pResourceData->m_pWorldMesh;
	}

	// SER: Ask Elliott why he's doing this:
//	GetOwner()->AppendTrackerSkipList();
}


void CWeaponQuadLaser::CheckpointRestore( void ) {
	CWeapon::CheckpointRestore();

	m_eSecondaryStatus = SECONDARY_READY;
	m_fSecondaryFireTimer = 0.0f;
	m_uDisabledArms = 0;
}



void CWeaponQuadLaser::ClassHierarchyWork( void ) {
	// Call our parent. It might need to do some work...
	CWeapon::ClassHierarchyWork();

	// Exit if we don't have any work to do...
	if( !IsOurWorkBitSet() ) {
		return;
	}
	m_nArmFiredThisFrame = -1;

	_ComputeAimPoints();

	if( m_bTrigger1Down && (m_fFireTimer <= 0.0f) && (GetClipAmmo() > 0) ) {
		// fire a round
		_FirePrimary();
		m_fFireTimer += m_pUserProps->fOORoundsPerSecPrimary;
	}

	switch( m_eSecondaryStatus ) {
		case SECONDARY_READY:
			m_fUnitCharge = 1.0f;
			break;

		case SECONDARY_RECHARGING:
			if( m_fSecondaryFireTimer > 0.0f ) {
				m_fSecondaryFireTimer -= FLoop_fPreviousLoopSecs;
			}

			if( m_fSecondaryFireTimer <= 0.0f ) {
				m_fSecondaryFireTimer = 0.0f;
				m_eSecondaryStatus = SECONDARY_READY;
			}

			m_fUnitCharge = (m_pUserProps->fSecondaryFireRechargeTime - m_fSecondaryFireTimer) * m_pUserProps->fOOSecondaryFireRechargeTime;
			break;

		case SECONDARY_FIRING:
			if( m_fSecondaryFireTimer > 0.0f ) {
				m_fSecondaryFireTimer -= FLoop_fPreviousLoopSecs;
			}

			// fire
			if( (m_fFireTimer <= 0.0f) && (GetClipAmmo() > 0)) {				// enough time has passed since last time and have ammo
				_FireSecondary();												// fire
				m_fFireTimer += m_pUserProps->fOORoundsPerSecSecondary;
			}
					
			if( m_fSecondaryFireTimer <= 0.0f ) {
				m_fSecondaryFireTimer = m_pUserProps->fSecondaryFireRechargeTime;
				m_eSecondaryStatus = SECONDARY_RECHARGING;
			} else {
                m_fUnitCharge = m_fSecondaryFireTimer * m_pUserProps->fOOSecondaryFireBarrageTime;
			}

			break;
	}


	m_fFireTimer -= FLoop_fPreviousLoopSecs;

	// handle any recoiling if necessary...
	for( u32 i=0; i<ARM_COUNT; i++ ) {
		if( m_afUnitRecoilTime[i] > 0.0f ) {
			if( m_abRecoilOn[i] ) {
				m_afUnitRecoilTime[i] += _RECOIL_OO_ON_TIME * FLoop_fPreviousLoopSecs;
				
				if( m_afUnitRecoilTime[i] >= 1.0f ) {
					FMATH_CLAMP_MAX1( m_afUnitRecoilTime[i] );
                    m_abRecoilOn[i] = FALSE;
				}
			} else {
				m_afUnitRecoilTime[i] -= _RECOIL_OO_OFF_TIME * FLoop_fPreviousLoopSecs;
				FMATH_CLAMP_MIN0( m_afUnitRecoilTime[i] );
			}
		}
	}
		
	// Recharge...
	m_fEnergyTimer -= FLoop_fPreviousLoopSecs;
	if( !m_bTrigger1Down && !m_bTrigger2Down ) {	
		if( m_fEnergyTimer <= 0.0f ) {
			AddToClip();
			m_fEnergyTimer += m_pUserProps->fRechargeTime * _OO_MAX_CHARGE;
		}
	}

	
	if( (m_bTrigger1Down || m_bTrigger2Down) && !IsClipEmpty() ) {
		m_fUnitAimSpread			+= m_pUserProps->fOOAimSpreadTime * FLoop_fPreviousLoopSecs;
		m_fUnitMuzzleSmokeIntensity += FLoop_fPreviousLoopSecs;
	} else {
        m_fUnitAimSpread			-= m_pUserProps->fOOAimRecoveryTime * FLoop_fPreviousLoopSecs;
		m_fUnitMuzzleSmokeIntensity -= FLoop_fPreviousLoopSecs;
	}
	
	FMATH_CLAMP_UNIT_FLOAT( m_fUnitAimSpread );
	FMATH_CLAMP_UNIT_FLOAT( m_fUnitMuzzleSmokeIntensity );

    
	// set our smoke intensity...
	for( u32 i=0; i<ARM_COUNT; i++ ) {
		if( m_fUnitMuzzleSmokeIntensity > 0.0f ) {
            m_apMuzzleSmokeParticles[i]->SetUnitIntensity( m_fUnitMuzzleSmokeIntensity );
			m_apMuzzleSmokeParticles[i]->Relocate_RotXlatFromUnitMtx_WS( m_apMtxFireBones[i] );
		} else {
			m_apMuzzleSmokeParticles[i]->EnableEmission( FALSE );
		}
	}

	return;
}


/////////////////////////
//QUAD LASER FNs

CWeaponQuadLaser::CWeaponQuadLaser() {
	_ClearDataMembers();
}


CWeaponQuadLaser::~CWeaponQuadLaser() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CWeaponQuadLaser::Create( cchar *pszEntityName/*=NULL*/, const CFMtx43A *pMtx/*=NULL*/, cchar *pszAIBuilderName/*=NULL*/ ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	if( !ClassHierarchyLoadSharedResources() ) {
		// Failure! (resources have already been cleaned up, so we can bail now)
		return FALSE;
	}

	// Get pointer to the leaf class's builder object...
	CWeaponQuadLaserBuilder *pBuilder = (CWeaponQuadLaserBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_WeaponQuadLaserBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_WEAPONQUADLASER );
	}

	// Create an entity...
	return CWeapon::Create( WEAPON_TYPE_QUADLASER, pszEntityName, pMtx, pszAIBuilderName );
}


void CWeaponQuadLaser::_ClearDataMembers( void ) {
	u32 i;

	for( i=0; i<EUK_COUNT_QUADLASER; ++i )
	{
		m_aResourceData[i].m_pWorldMesh = NULL;
	}

	for( i=0; i<ARM_COUNT; i++ ) {
		m_apMtxFireBones[i]		= NULL;
		m_afUnitRecoilTime[i]	= 0.0f;
		m_abRecoilOn[i]			= FALSE;
		m_avAimPoints[i].Zero();
		m_avAimSpread[i].Zero();
		m_avDesiredAimPoints[i].Zero();

		m_avLastSecFirePoint[i].Zero();
		m_afLastSecFireTimer[i] = 0.0f;
	}

	m_eSecondaryStatus = SECONDARY_READY;
	m_fSecondaryFireTimer = 0.0f;
	m_uCurrentFireSeg	= 0;
	m_fEnergyTimer		= 0.0f;

	m_pResourceData = NULL;
	m_pUserProps	= NULL;

	m_bTrigger1Down	= FALSE;
	m_bTrigger2Down = FALSE;

	m_nArmFiredThisFrame	 = -1;
	
	// set our fire order
	m_auFirePos[0] = ARM_TL;
	m_auFirePos[1] = ARM_BR;
	m_auFirePos[2] = ARM_BL;
	m_auFirePos[3] = ARM_TR;

	m_uDisabledArms	= 0;

	m_bEnableArmLag	= TRUE;

	m_fUnitAimSpread = 0.0f;
	m_fUnitRecoilAdjust = 0.0f;
	m_uSoundCounter = 0;
	m_fUnitMuzzleSmokeIntensity = 0.0f;
	m_uLastSoundTime = 0;

}


void CWeaponQuadLaser::SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo ) {
	CWeapon::SetItemInst(pItemInst, bUpdateItemInstAmmoFromWeaponAmmo);
	if( pItemInst ) {
		pItemInst->SetAmmoDisplayType( CItemInst::AMMODISPLAY_METER, CItemInst::AMMODISPLAY_NONE );
	}
}


void CWeaponQuadLaser::SetFireBones( const CFWorldMesh *pWorldMesh, cchar *pszTLBone, cchar *pszTRBone, cchar *pszBLBone, cchar *pszBRBone ) {
	FASSERT( m_pOwnerBot );

	u32 uFireBone0 = pWorldMesh->FindBone( pszTLBone );
	u32 uFireBone1 = pWorldMesh->FindBone( pszTRBone );
	u32 uFireBone2 = pWorldMesh->FindBone( pszBLBone );
	u32 uFireBone3 = pWorldMesh->FindBone( pszBRBone );
	
	FASSERT( (uFireBone0 >= 0) &&
			 (uFireBone1 >= 0) &&
			 (uFireBone2 >= 0) &&
			 (uFireBone3 >= 0) );

	m_apMtxFireBones[ARM_TL] = pWorldMesh->GetBoneMtxPalette()[uFireBone0];
	m_apMtxFireBones[ARM_TR] = pWorldMesh->GetBoneMtxPalette()[uFireBone1];
	m_apMtxFireBones[ARM_BL] = pWorldMesh->GetBoneMtxPalette()[uFireBone2];
	m_apMtxFireBones[ARM_BR] = pWorldMesh->GetBoneMtxPalette()[uFireBone3];
}


void CWeaponQuadLaser::_TracerKilledCallbackPrimary( TracerDef_t *pTracerDef, TracerKillReason_e nKillReason, const FCollImpact_t *pImpact ) {
	CFVec3A vtmp;

	if( nKillReason == TRACER_KILLREASON_HIT_GEO ) {
		FASSERT( pTracerDef->pUser != NULL );
		
		CWeaponQuadLaser* pQLaser = (CWeaponQuadLaser*)(pTracerDef->pUser);

		const CGCollMaterial *pCollMaterial = CGColl::GetMaterial( pImpact->nUserType );

		pCollMaterial->DrawAllDrawableParticles(
			&pImpact->ImpactPoint,
			&pImpact->UnitFaceNormal,
			TRUE,
			fmath_RandomFloatRange( 0.0f, 0.5f ),
			fmath_RandomFloatRange( 0.0f, 0.75f ),
			fmath_RandomFloatRange( 0.0f, 0.25f )
		);

		vtmp.Mul( pImpact->UnitFaceNormal, 0.25f ).Add( pImpact->ImpactPoint );

		CMuzzleFlash::AddFlash_CardPosterXY( m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_BALL_POSTER_XY_1], vtmp, 5.0f, 1.0f );


		//if( pCollMaterial->IsPockMarkEnabled( pImpact ) ) {
		//	potmark_NewPotmark( &pImpact->ImpactPoint, &pImpact->UnitFaceNormal, 2.0f, POTMARKTYPE_LASER1 );
		//}

		CFDecal::Create( pQLaser->m_pUserProps->hDecalDef, pImpact, &pTracerDef->UnitDir_WS );

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
						pDamageForm->m_pDamageProfile	= pQLaser->m_pUserProps->pDamageProfile1;
						pDamageForm->m_Damager			= pTracerDef->Damager;
						pDamageForm->m_pDamageeEntity	= pHitEntity;

						pDamageForm->InitTriDataFromCollImpact( pHitWorldMesh, pImpact, &pTracerDef->UnitDir_WS );

						CDamage::SubmitDamageForm( pDamageForm );

					}
				}
			}
		}

		// make an AI impact sound in case any AI are listening
		if( pQLaser->m_pOwnerBot->m_nPossessionPlayerIndex > -1 && pQLaser->m_fSecsUntilNextSound <= 0.0f) {
			AIEnviro_AddSound( pImpact->ImpactPoint, AIEnviro_fLaserImpactAudibleRange, 0.3f, AISOUNDTYPE_WEAPON, 0, pQLaser );
			pQLaser->m_fSecsUntilNextSound = 0.5f;
		}
	}
}


void CWeaponQuadLaser::_TracerKilledCallbackSecondary( TracerDef_t *pTracerDef, TracerKillReason_e nKillReason, const FCollImpact_t *pImpact ) {
	CFVec3A vtmp;

	if( nKillReason == TRACER_KILLREASON_HIT_GEO ) {
		FASSERT( pTracerDef->pUser != NULL );

		CWeaponQuadLaser *pQLaser = (CWeaponQuadLaser*)(pTracerDef->pUser);
		const _UserProps_t *pUserProps = pQLaser->m_pUserProps;

		if( pUserProps->hExplosionGroup != FEXPLOSION_INVALID_HANDLE ) {
			FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();

			if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
				FExplosionSpawnParams_t SpawnParams;
				CDamageForm::TriData_t DamageImpactTriData, *pDamageImpactTriData;
				CFWorldMesh *pImpactedWorldMesh;

				SpawnParams.InitToDefaults();
				pImpactedWorldMesh = (CFWorldMesh *)pImpact->pTag;
				pDamageImpactTriData = NULL;

				SpawnParams.pDamageProfile = pUserProps->pDamageProfile2;
				SpawnParams.pDamager = &pTracerDef->Damager;
				SpawnParams.Pos_WS = pImpact->ImpactPoint;
				SpawnParams.UnitDir = pImpact->UnitFaceNormal;
				SpawnParams.uSurfaceType = pImpact->nUserType;

				if( pImpactedWorldMesh ) {
					DamageImpactTriData.ImpactUnitNormal_WS = pImpact->UnitFaceNormal;
					DamageImpactTriData.aTriVtx_WS[0] = pImpact->aTriVtx[0];
					DamageImpactTriData.aTriVtx_WS[1] = pImpact->aTriVtx[1];
					DamageImpactTriData.aTriVtx_WS[2] = pImpact->aTriVtx[2];
					DamageImpactTriData.nDamageeBoneIndex = pImpact->nBoneIndex;
					DamageImpactTriData.pDamageeWorldMesh = pImpactedWorldMesh;
					DamageImpactTriData.eSurfaceType = CGColl::GetSurfaceType( pImpact );

					pDamageImpactTriData = &DamageImpactTriData;
				}

				CExplosion2::SpawnExplosion( hSpawner, pUserProps->hExplosionGroup, &SpawnParams, pDamageImpactTriData );
			}
		}
	}
}


void CWeaponQuadLaser::_TracerMovedSecondary( void* pUserData, const CFVec3A& NewPos_WS, BOOL bDied) {
	if( !bDied ) {
        smoketrail_Puff( pUserData, &NewPos_WS );
	} else {
		smoketrail_ReturnToFreePool( pUserData, TRUE );
	}
}

void CWeaponQuadLaser::_TracerBuildTrackerSkipList( void *pThisWeapon ) {
	if( pThisWeapon ) {
		((CWeaponQuadLaser*)pThisWeapon)->AppendTrackerSkipList();
	}
}

static u64 m_uLastSoundTime = 0;
#define _OO_MAX_SOUNDS_PER_SEC ( 1.0f / 15.0f )

void CWeaponQuadLaser::_FirePrimary( void ) {
	CFVec3A vFirePos;
	CFVec3A vUnitFireDir;
//	f32 fMag2;
	
	u32 uFireArm = m_auFirePos[m_uCurrentFireSeg];

	if( IsArmDisabled( uFireArm ) ) {
		AdvanceFireArm();
		return;
	}

	ComputeMuzzlePoint_WS( &vFirePos );

	FASSERT( m_pOwnerBot );
	
	ComputeFireUnitDir( &(m_aTracerDefPri[m_nUpgradeLevel].UnitDir_WS), &vFirePos, &(m_apMtxFireBones[uFireArm]->m_vFront), &(m_pOwnerBot->m_TargetedPoint_WS), FALSE );
	//if( m_pOwnerBot->IsPlayerBot() ) {
	//	vUnitFireDir.Sub( m_pOwnerBot->m_TargetedPoint_WS, vFirePos );
	//	fMag2 = vUnitFireDir.MagSq();
	//	if( fMag2 > 0.01f ) {
	//		vUnitFireDir.Mul( fmath_InvSqrt( fMag2 ) );
	//	} else {
	//		vUnitFireDir = m_apMtxFireBones[uFireArm]->m_vFront;
	//	}
	//} else {
	//	vUnitFireDir = m_apMtxFireBones[uFireArm]->m_vFront;
	//}

	m_nArmFiredThisFrame = uFireArm;

	_StartRecoil( uFireArm );

	m_avAimSpread[uFireArm].Set( fmath_RandomBipolarUnitFloat() * m_fUnitAimSpread * m_pUserProps->fMaxShotSpread,
								 fmath_RandomBipolarUnitFloat() * m_fUnitAimSpread * m_pUserProps->fMaxShotSpread,
								 fmath_RandomBipolarUnitFloat() * m_fUnitAimSpread * m_pUserProps->fMaxShotSpread );

	m_apMuzzleSmokeParticles[uFireArm]->EnableEmission( TRUE );

	// Now create the tracer
	m_aTracerDefPri[m_nUpgradeLevel].pUser		= this;
	//m_aTracerDefPri[m_nUpgradeLevel].UnitDir_WS = m_apMtxFireBones[uFireArm]->m_vFront;
	m_aTracerDefPri[m_nUpgradeLevel].TailPos_WS = vFirePos;

	ConstructDamagerData();
	if( tracer_NewTracer( m_hTracerGroupPri[m_nUpgradeLevel], &m_aTracerDefPri[m_nUpgradeLevel], 0.2f, FALSE, FALSE, &CDamageForm::m_TempDamager ) ) {
		
		CMuzzleFlash::AddFlash_Mesh3D( m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_3D_ENERGY_WHITE], vFirePos,
									   m_apMtxFireBones[uFireArm]->m_vFront, 1.0f, fmath_RandomFloatRange( 0, FMATH_PI ) );

		if( FLoop_nRealTotalLoopTicks - m_uLastSoundTime > (u64)(FLoop_nTicksPerSec * _OO_MAX_SOUNDS_PER_SEC) ) {
			PlaySound( m_pUserProps->pSoundGroupFire1, 1.0f, 1.0f, -1.0f, &vFirePos );
			m_uLastSoundTime = FLoop_nRealTotalLoopTicks;
		}
		//if( m_uSoundCounter == 0 ) {
		//	PlaySound( m_pUserProps->pSoundGroupFire1, 1.0f, 1.0f, -1.0f, &vFirePos );
		//	m_uSoundCounter = _SHOTS_PER_SOUND_PRI-1;
		//} else {
		//	m_uSoundCounter--;
		//}

		if( m_fEnergyTimer <= 0.0f ) {
			RemoveFromClip();
			m_fEnergyTimer += m_pUserProps->fPrimaryDischargeTime * _OO_MAX_CHARGE;
		}

		//do some force feedback work
		if( IsOwnedByPlayer() ) {
			fforce_Kill( &m_hForce );
			fforce_Play( Player_aPlayer[GetOwner()->m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROUGH_RUMBLE_MED, &m_hForce );
		}
	}



	// advance fire point
	AdvanceFireArm();
}



void CWeaponQuadLaser::_FireSecondary( void ) {
	FASSERT( m_uCurrentFireSeg < 4 );

	
	CFVec3A vFirePos;
	u32		uFireArm = m_auFirePos[m_uCurrentFireSeg];

	FASSERT( uFireArm < ARM_COUNT );
	
	if( IsArmDisabled( uFireArm ) ) {
		AdvanceFireArm();
		return;
	}

	ComputeMuzzlePoint_WS( &vFirePos );

	m_nArmFiredThisFrame = uFireArm;

	// Add recoil effects
	_StartRecoil( uFireArm );

	if( FLoop_nRealTotalLoopTicks - m_uLastSoundTime > (u64)(FLoop_nTicksPerSec * _OO_MAX_SOUNDS_PER_SEC) ) {
		PlaySound( m_pUserProps->pSoundGroupFire2, 1.0f, 1.0f, -1.0f, &vFirePos );
		m_uLastSoundTime = FLoop_nRealTotalLoopTicks;
	}

	//if( m_uSoundCounter == 0 ) {
	//	PlaySound( m_pUserProps->pSoundGroupFire2, 1.0f, 1.0f, -1.0f, &vFirePos );
	//	m_uSoundCounter = _SHOTS_PER_SOUND_SEC-1;
	//} else {
	//	m_uSoundCounter--;
	//}

	// draw some power if enough time has elapsed...
	if( m_fEnergyTimer <= 0.0f ) {
		RemoveFromClip();
		m_fEnergyTimer += m_pUserProps->fSecondaryDischargeTime * _OO_MAX_CHARGE;
	}

	// turn on muzzle smoke for this barrel
	m_apMuzzleSmokeParticles[uFireArm]->EnableEmission( TRUE );
	m_fUnitMuzzleSmokeIntensity = 1.0f;

	// capture the point we just shot at... 
	m_avLastSecFirePoint[uFireArm] = m_avAimPoints[uFireArm];
	m_afLastSecFireTimer[uFireArm] = 0.0f;

	// ok, choose a new desired shoot position;
	CFVec3A vX;		// not really X & Y, just two arbitrary vecs
	CFVec3A vY;

	vX.Cross( CFVec3A::m_UnitAxisY, m_apMtxFireBones[uFireArm]->m_vFront );
	if( vX.MagSq() < 0.001f ) {
		vX = m_apMtxFireBones[uFireArm]->m_vRight;
		vY = m_apMtxFireBones[uFireArm]->m_vUp;
	} else {
		vX.Unitize();
		vY.Cross( vX, m_apMtxFireBones[uFireArm]->m_vFront );
	}

	vX.Mul( fmath_RandomBipolarUnitFloat() * 20.0f );
	vY.Mul( fmath_RandomBipolarUnitFloat() * 20.0f );

	// make sure that our targeted point is not too close
	if( m_MtxToWorld.m_vPos.DistSq( m_pOwnerBot->m_TargetedPoint_WS ) < 100.0f * 100.0f ) {
		CFVec3A vTgtPt;
		vTgtPt.Sub( m_pOwnerBot->m_TargetedPoint_WS, m_MtxToWorld.m_vPos );
		vTgtPt.Unitize();
		vTgtPt.Mul( 100.0f );
		vTgtPt.Add( m_MtxToWorld.m_vPos, vTgtPt );
        m_avDesiredAimPoints[uFireArm].Add( vTgtPt, vX );
	} else {
        m_avDesiredAimPoints[uFireArm].Add( m_pOwnerBot->m_TargetedPoint_WS, vX );
	}

	m_avDesiredAimPoints[uFireArm].Add( vY );

	// Now create the tracer
	m_aTracerDefSec[m_nUpgradeLevel].pUser		= this;
	m_aTracerDefSec[m_nUpgradeLevel].UnitDir_WS = m_apMtxFireBones[uFireArm]->m_vFront;
	m_aTracerDefSec[m_nUpgradeLevel].TailPos_WS = vFirePos;

	ConstructDamagerData();
	if( tracer_NewTracer( m_hTracerGroupSec[m_nUpgradeLevel], &m_aTracerDefSec[m_nUpgradeLevel], 0.0f, FALSE, FALSE, &CDamageForm::m_TempDamager ) ) {

		// init smoke
		SmokeTrailHandle_t hSmokeTrail = smoketrail_GetFromFreePoolAndSetAttributes( &m_SmokeTrailAttrib );
		if( hSmokeTrail != NULL ) {
			tracer_AddExtraParamsToLastTracer( _TracerMovedSecondary, (void*)hSmokeTrail );
		}
		
		// advance fire point
		if( ++m_uCurrentFireSeg > 3 ) {
			m_uCurrentFireSeg = 0;
		}

		//do some force feedback work
		if( IsOwnedByPlayer() ) {
			fforce_Kill( &m_hForce );
			fforce_Play( Player_aPlayer[GetOwner()->m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROCKET_THRUST_LIGHT, &m_hForce );
		}

		CMuzzleFlash::AddFlash_Mesh3D(	m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_3D_ENERGY_ORANGE], vFirePos, 
										m_apMtxFireBones[uFireArm]->m_vFront, 1.0f, fmath_RandomFloatRange( 0, FMATH_PI ) );

	}
}


void CWeaponQuadLaser::ResetAimPoints( const CFVec3A &vTgtPoint ) {
	for( u32 i=0; i<ARM_COUNT; i++ ) {
		m_avDesiredAimPoints[i] = vTgtPoint;
		m_avAimPoints[i]		= vTgtPoint;
	}
}


void CWeaponQuadLaser::_ComputeAimPoints( void ) {
	u32	i;
	CFVec3A vTmp;

	f32 fUnitDistance;

	if( !m_bEnableArmLag ) {
		for( i=0; i<ARM_COUNT; i++ ) {
			m_avDesiredAimPoints[i] = m_pOwnerBot->m_TargetedPoint_WS;
			m_avAimPoints[i]		= m_pOwnerBot->m_TargetedPoint_WS;
		}
	}


	// use a different style of aiming when we're doing secondary fire...
	if( (m_eSecondaryStatus == SECONDARY_FIRING) && (GetClipAmmo() > 0) ) {
		for( i=0; i<ARM_COUNT; i++ ) {
			vTmp.Sub( m_avDesiredAimPoints[i], m_avLastSecFirePoint[i] );
			m_afLastSecFireTimer[i] += FLoop_fPreviousLoopSecs;
			fUnitDistance = fmath_Div( m_afLastSecFireTimer[i], m_pUserProps->fOORoundsPerSecSecondary * 4.0f );
			FMATH_CLAMP_UNIT_FLOAT( fUnitDistance );
			fUnitDistance = fmath_UnitLinearToSCurve( fUnitDistance );
			vTmp.Mul( fUnitDistance );
			m_avAimPoints[i].Add( m_avLastSecFirePoint[i], vTmp );
		}
	} else {
		for( i=0; i<ARM_COUNT; i++ ) {
			m_avAimPoints[i] = m_pOwnerBot->m_TargetedPoint_WS;
		//	FMATH_BIPOLAR_CLAMPMAX( m_avAimSpread[i].x, m_fUnitAimSpread * m_pUserProps->fMaxShotSpread );
		//	FMATH_BIPOLAR_CLAMPMAX( m_avAimSpread[i].y, m_fUnitAimSpread * m_pUserProps->fMaxShotSpread );
		//	FMATH_BIPOLAR_CLAMPMAX( m_avAimSpread[i].z, m_fUnitAimSpread * m_pUserProps->fMaxShotSpread );

		//	f32 fUnitSpeedAvail = fmath_Div( m_avAimPoints[0].DistSq( m_apMtxFireBones[0]->m_vPos ), 250.0f * 250.0f );
		//	FMATH_CLAMP( fUnitSpeedAvail, 0.025f, 1.0f );

		//	vTmp.Add( m_pOwnerBot->m_TargetedPoint_WS, m_avAimSpread[i] );
		//	vTmp.Sub( m_avDesiredAimPoints[i] );
		//	if( vTmp.MagSq() > 0.01f ) {
		//		fAimDist = FMATH_MIN( vTmp.Mag(), _AIM_SPEED * FLoop_fPreviousLoopSecs * fUnitSpeedAvail );
		//		vTmp.Unitize();
		//		vTmp.Mul( fAimDist );
		//		m_avDesiredAimPoints[i].Add( vTmp ); 
		//	}           

		//	vTmp.Sub( m_avDesiredAimPoints[i], m_avAimPoints[i] );
		//	if( vTmp.MagSq() > 0.01f ) {
		//		fAimDist = FMATH_MIN( vTmp.Mag(), _AIM_SPEED * FLoop_fPreviousLoopSecs * fUnitSpeedAvail );
		//		vTmp.Unitize();
		//		vTmp.Mul( fAimDist );
		//		m_avAimPoints[i].Add( vTmp ); 
		//	}           
		//}
		}
	}
}

void CWeaponQuadLaser::_StartSecondaryFire( void ) {
	// capture the point we just shot at... 
	for( u32 i=0; i<ARM_COUNT; i++ ) {
		m_avLastSecFirePoint[i] = m_avAimPoints[i];
		m_afLastSecFireTimer[i] = 0.0f;

		// ok, choose a new desired shoot position;
		CFVec3A vX;		// not really X & Y, just two arbitrary vecs
		CFVec3A vY;

		vX.Cross( CFVec3A::m_UnitAxisY, m_apMtxFireBones[i]->m_vFront );
		if( vX.MagSq() < 0.001f ) {
			vX = m_apMtxFireBones[i]->m_vRight;
			vY = m_apMtxFireBones[i]->m_vUp;
		} else {
			vX.Unitize();
			vY.Cross( vX, m_apMtxFireBones[i]->m_vFront );
		}

		vX.Mul( fmath_RandomBipolarUnitFloat() * 20.0f );
		vY.Mul( fmath_RandomBipolarUnitFloat() * 20.0f );
		m_avDesiredAimPoints[i].Add( m_pOwnerBot->m_TargetedPoint_WS, vX );
		m_avDesiredAimPoints[i].Add( vY );
	}

	m_fFireTimer = m_pUserProps->fOORoundsPerSecSecondary * 4.0f;
	m_fSecondaryFireTimer = m_pUserProps->fSecondaryFireBarrageTime;
	m_eSecondaryStatus = SECONDARY_FIRING;
}


void CWeaponQuadLaser::DebugDraw( void ) {
	for( u32 i=0; i<1; i++ ) {
		fdraw_FacetedWireSphere( &m_avAimPoints[i].v3,		  2.0f, 4, 4, &FColor_MotifBlue );
		fdraw_FacetedWireSphere( &m_avDesiredAimPoints[i].v3, 2.0f, 4, 4, &FColor_MotifRed );
		fdraw_FacetedWireSphere( &m_avLastSecFirePoint[i].v3, 2.0f, 4, 4, &FColor_MotifGreen );
	}

}


/////////////////////////
//AIMING FNs

#define _AIM_RATE_0			( 5.0f )
#define _AIM_RATE_1			( 4.0f )
#define _AIM_RATE_2			( 7.5f )

// bone callback for the shoulders
void CWeaponQuadLaser::AimSeg0( u32 uIdx, f32 fUnitCtl, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	FASSERT( uIdx < ARM_COUNT );

	rNewMtx.Mul( rParentMtx, rBoneMtx );

	// if fUnitCtl == 0, we don't want to aim, just leave it alone and get out
	if( fUnitCtl == 0.0f ) {
		m_aqArm0[uIdx].BuildQuat( rNewMtx );
		return;
	}

	// otherwise...
	FASSERT( uIdx < ARM_COUNT );

	rNewMtx.Mul( rParentMtx, rBoneMtx );

	rNewMtx.m_vRight.Sub( m_avAimPoints[uIdx], rNewMtx.m_vPos );
	rNewMtx.m_vRight.Unitize();

	switch( uIdx ) {
		case ARM_TL:
			rNewMtx.m_vFront.CrossVecWithY( rNewMtx.m_vRight );
			CFMtx43A::m_RotX.SetRotationX( _AIM_SEG0_XROT_TL );
			break;

		case ARM_BL:
			rNewMtx.m_vFront.CrossVecWithY( rNewMtx.m_vRight );
			CFMtx43A::m_RotX.SetRotationX( _AIM_SEG0_XROT_BL );
			break;

		case ARM_TR:
			rNewMtx.m_vFront.CrossYWithVec( rNewMtx.m_vRight );
			CFMtx43A::m_RotX.SetRotationX( _AIM_SEG0_XROT_TR );
			break;

		case ARM_BR:
			rNewMtx.m_vFront.CrossYWithVec( rNewMtx.m_vRight );
			CFMtx43A::m_RotX.SetRotationX( _AIM_SEG0_XROT_BR );
			break;
	}

	//CFMtx43A::m_RotY.SetRotationY( _RECOIL_ANGLE_0 * fmath_UnitLinearToSCurve( m_afUnitRecoilTime[uIdx] ) * m_fUnitRecoilAdjust );
	
	rNewMtx.m_vFront.Unitize();
	rNewMtx.m_vUp.Cross( rNewMtx.m_vFront, rNewMtx.m_vRight );

	//rNewMtx.Mul( CFMtx43A::m_RotY );
	rNewMtx.Mul( CFMtx43A::m_RotX );

	CFQuatA qTmp;
	qTmp.BuildQuat( rNewMtx );
	m_aqArm0[uIdx].ReceiveSlerpOf( _AIM_RATE_0 * FLoop_fPreviousLoopSecs, m_aqArm0[uIdx], qTmp );
	//m_aqArm0[uIdx].BuildMtx33( rNewMtx );

    //
	//

	if( fUnitCtl == 1.0f ) {
		m_aqArm0[uIdx].BuildMtx33( rNewMtx );
		CFMtx43A::m_RotY.SetRotationY( _RECOIL_ANGLE_0 * fmath_UnitLinearToSCurve( m_afUnitRecoilTime[uIdx] ) * m_fUnitRecoilAdjust );
		rNewMtx.Mul( CFMtx43A::m_RotY );

	} else {
		// transitioning in or out
		CFMtx43A::m_Temp.Mul( rParentMtx, rBoneMtx );

		fUnitCtl = (fUnitCtl - _AIM_ON_SEG0) * _OO_AIM_INTERVAL_SEG0;
		FMATH_CLAMP_UNIT_FLOAT( fUnitCtl );
		fUnitCtl = fmath_UnitLinearToSCurve( fUnitCtl );

		qTmp.BuildQuat( CFMtx43A::m_Temp );
		qTmp.ReceiveSlerpOf( fUnitCtl, qTmp, m_aqArm0[uIdx] );
		qTmp.BuildMtx33( rNewMtx );
	}


	//if( fUnitCtl < 1.0f ) {
	//	// here we go...
	//	CFMtx43A mtxTmp;
	//	CFQuatA qtStart, qtEnd, qtInterp;

	//	fUnitCtl = fmath_Div( fUnitCtl - _AIM_ON_SEG0, _AIM_OFF_SEG0 - _AIM_ON_SEG0 );
	//	FMATH_CLAMP_UNIT_FLOAT( fUnitCtl );
	//	fUnitCtl = fmath_UnitLinearToSCurve( fUnitCtl );

	//	mtxTmp.Mul( rParentMtx, rBoneMtx );
	//	qtStart.BuildQuat( mtxTmp );
	//	qtEnd.BuildQuat( rNewMtx );

	//	qtInterp.ReceiveSlerpOf( fUnitCtl, qtStart, qtEnd );
 //       qtInterp.BuildMtx( rNewMtx );
	//	rNewMtx.m_vPos = mtxTmp.m_vPos;
	//}
}


// bone callback for the inner arms
void CWeaponQuadLaser::AimSeg1( u32 uIdx, f32 fUnitCtl, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	// if fUnitCtl == 0, we don't want to aim, just leave it alone and get out
	FASSERT( uIdx < ARM_COUNT );

	rNewMtx.Mul( rParentMtx, rBoneMtx );
	if( fUnitCtl == 0.0f ) {
		m_aqArm1[uIdx].BuildQuat( rNewMtx );
		return;
	}

	// otherwise...
	
	CFVec3A vAimVec;
	f32		fUnitAimDist;		// actually going to use dist squared, because it's not really important, it's just for effect
	f32		fAngle;

	//vAimVec.Sub( m_avAimPoints[uIdx], rParentMtx.m_vPos );
	vAimVec.Sub( m_avAimPoints[uIdx], rParentMtx.m_vPos );
	vAimVec.MagSq();
	fUnitAimDist = fmath_Div( vAimVec.MagSq() - _AIM_MIN_DISTSQ, _AIM_MAX_DISTSQ - _AIM_MIN_DISTSQ );
	FMATH_CLAMP_UNIT_FLOAT( fUnitAimDist );

	fAngle = FMATH_FPOT( fUnitAimDist, _AIM_ELBOW_ANGLE_MAX, _AIM_ELBOW_ANGLE_MIN );
	fAngle += _RECOIL_ANGLE_1 * fmath_UnitLinearToSCurve( m_afUnitRecoilTime[uIdx] ) * m_fUnitRecoilAdjust;

	//if( fUnitCtl < 1.0f ) {
	//	fUnitCtl = fmath_Div( fUnitCtl - _AIM_ON_SEG1, _AIM_OFF_SEG1 - _AIM_ON_SEG1 );
	//	FMATH_CLAMP_UNIT_FLOAT( fUnitCtl );
	//	fUnitCtl = fmath_UnitLinearToSCurve( fUnitCtl );
	//	fAngle *= fUnitCtl;	// easy enough
	//}

	CFMtx43A::m_RotY.SetRotationY( fAngle );
	CFMtx43A::m_Temp.Mul( rBoneMtx, CFMtx43A::m_RotY );

	rNewMtx.Mul( rParentMtx, CFMtx43A::m_Temp );


	CFQuatA qTmp;
	qTmp.BuildQuat( rNewMtx );
	m_aqArm1[uIdx].ReceiveSlerpOf( _AIM_RATE_1 * FLoop_fPreviousLoopSecs, m_aqArm1[uIdx], qTmp );

	if( fUnitCtl == 1.0f ) {
		m_aqArm1[uIdx].BuildMtx33( rNewMtx );
	} else {
		// transitioning in or out
		CFMtx43A::m_Temp.Mul( rParentMtx, rBoneMtx );

		fUnitCtl = (fUnitCtl - _AIM_ON_SEG1) * _OO_AIM_INTERVAL_SEG1;
		FMATH_CLAMP_UNIT_FLOAT( fUnitCtl );
		fUnitCtl = fmath_UnitLinearToSCurve( fUnitCtl );

		qTmp.BuildQuat( CFMtx43A::m_Temp );
		qTmp.ReceiveSlerpOf( fUnitCtl, qTmp, m_aqArm1[uIdx] );
		qTmp.BuildMtx33( rNewMtx );
	}
}


// bone callback for the inner arms
void CWeaponQuadLaser::AimSeg2( u32 uIdx, f32 fUnitCtl, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	// if fUnitCtl == 0, we don't want to aim, just leave it alone and get out
	FASSERT( uIdx < ARM_COUNT );

	rNewMtx.Mul( rParentMtx, rBoneMtx );

	if( fUnitCtl == 0.0f ) {
		m_aqArm2[uIdx].BuildQuat( rNewMtx );
		return;
	}

	// otherwise...
	

	rNewMtx.Mul( rParentMtx, rBoneMtx );
	//rNewMtx.m_vFront.Sub( m_avAimPoints[uIdx], rNewMtx.m_vPos );
	rNewMtx.m_vFront.Sub( m_avAimPoints[uIdx], rNewMtx.m_vPos );
	rNewMtx.m_vFront.Unitize();

	rNewMtx.m_vRight.Cross( rNewMtx.m_vUp, rNewMtx.m_vFront );
	rNewMtx.m_vRight.Unitize();

	rNewMtx.m_vUp.Cross( rNewMtx.m_vFront, rNewMtx.m_vRight );


	CFQuatA qTmp;
	qTmp.BuildQuat( rNewMtx );
	m_aqArm2[uIdx].ReceiveSlerpOf( _AIM_RATE_2 * FLoop_fPreviousLoopSecs, m_aqArm2[uIdx], qTmp );
	
	if( fUnitCtl == 1.0f ) {
		m_aqArm2[uIdx].BuildMtx33( rNewMtx );
	} else {
		// transitioning in or out
		CFMtx43A::m_Temp.Mul( rParentMtx, rBoneMtx );

		fUnitCtl = (fUnitCtl - _AIM_ON_SEG2) * _OO_AIM_INTERVAL_SEG2;
		FMATH_CLAMP_UNIT_FLOAT( fUnitCtl );
		fUnitCtl = fmath_UnitLinearToSCurve( fUnitCtl );

		qTmp.BuildQuat( CFMtx43A::m_Temp );
		qTmp.ReceiveSlerpOf( fUnitCtl, qTmp, m_aqArm2[uIdx] );
		qTmp.BuildMtx33( rNewMtx );
	}


	//if( fUnitCtl < 1.0f ) {
	//	


	//	// here we go...
	//	//CFMtx43A mtxTmp;
	//	CFQuatA qtStart, qtEnd, qtInterp;
	//	
	//	FMATH_CLAMP_UNIT_FLOAT( fUnitCtl );
	//	fUnitCtl = fmath_UnitLinearToSCurve( fUnitCtl );

	//	mtxTmp.Mul( rParentMtx, rBoneMtx );
	//	qtStart.BuildQuat( mtxTmp );
	//	qtEnd.BuildQuat( rNewMtx );

	//	qtInterp.ReceiveSlerpOf( fUnitCtl, qtStart, qtEnd );
 //       qtInterp.BuildMtx( rNewMtx );
	////	rNewMtx.m_vPos = mtxTmp.m_vPos; 
	////}
}


void CWeaponQuadLaser::_StartRecoil( u32 uArm ) {
	FASSERT( uArm < ARM_COUNT );

	m_abRecoilOn[uArm]		  = TRUE;
	m_afUnitRecoilTime[uArm] += 0.000001f;	// needs a little push to activate it
}


BOOL CWeaponQuadLaser::_InitSmokeTrail( void ) {

	m_SmokeTrailAttrib.nFlags = SMOKETRAIL_FLAG_NONE;
	m_SmokeTrailAttrib.pTexDef = NULL;					// will be set in load shared resources

	m_SmokeTrailAttrib.fScaleMin_WS	= 2.0f;
	m_SmokeTrailAttrib.fScaleMax_WS	= 2.5f;
	m_SmokeTrailAttrib.fScaleSpeedMin_WS = -2.0f;
	m_SmokeTrailAttrib.fScaleSpeedMax_WS = -2.5f;
	m_SmokeTrailAttrib.fScaleAccelMin_WS = 0.0f;
	m_SmokeTrailAttrib.fScaleAccelMax_WS = 0.0f;

	m_SmokeTrailAttrib.fXRandSpread_WS = 0.1f;
	m_SmokeTrailAttrib.fYRandSpread_WS = 0.1f;
	m_SmokeTrailAttrib.fDistBetweenPuffs_WS = 1.0f;
	m_SmokeTrailAttrib.fDistBetweenPuffsRandSpread_WS = 0.05f;

	m_SmokeTrailAttrib.fYSpeedMin_WS = 0.5f;
	m_SmokeTrailAttrib.fYSpeedMax_WS = 1.0f;
	m_SmokeTrailAttrib.fYAccelMin_WS = -0.2f;
	m_SmokeTrailAttrib.fYAccelMax_WS = -0.5f;

	m_SmokeTrailAttrib.fUnitOpaqueMin_WS = 0.15f;
	m_SmokeTrailAttrib.fUnitOpaqueMax_WS = 0.25f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMin_WS = -0.25f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMax_WS = -0.25f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMin_WS = 0.0f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMax_WS = 0.0f;

	m_SmokeTrailAttrib.StartColorRGB.Set( 1.0f, 0.75f, 0.2f );
	m_SmokeTrailAttrib.EndColorRGB.Set( 0.1f, 0.0f, 0.0f );

	m_SmokeTrailAttrib.fStartColorUnitIntensityMin = 1.0f;
	m_SmokeTrailAttrib.fStartColorUnitIntensityMax = 0.9f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMin = 0.8f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMax = 1.0f;

	m_SmokeTrailAttrib.fColorUnitSliderSpeedMin = 1.75f;
	m_SmokeTrailAttrib.fColorUnitSliderSpeedMax = 2.25f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMin = 0.0f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMax = 0.0f;

	return TRUE;
}


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponQuadLaserBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************


void CWeaponQuadLaserBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CWeaponBuilder, 0/*ENTITY_BIT_WEAPONQUADLASER*/, pszEntityType );
}


BOOL CWeaponQuadLaserBuilder::PostInterpretFixup( void ) {
	return CWeaponBuilder::PostInterpretFixup();
}


BOOL CWeaponQuadLaserBuilder::InterpretTable( void ) {
	return CWeaponBuilder::InterpretTable();
}

