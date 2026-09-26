//////////////////////////////////////////////////////////////////////////////////////
// weapon_staff.cpp - Elite guard's weapon implementation.
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
// 02.07.03 Scholz       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "weapon_staff.h"
#include "weapon.h"
#include "fworld.h"
#include "fresload.h"
#include "ftex.h"
#include "fviewport.h"
#include "tracer.h"
#include "gamecam.h"
#include "floop.h"
#include "fforce.h"
#include "reticle.h"
#include "fcolor.h"
#include "player.h"
#include "reticle.h"
#include "floop.h"
#include "explosion.h"
#include "meshtypes.h"
#include "potmark.h"
#include "iteminst.h"
#include "fsound.h"
#include "Ai/AIEnviro.h"
#include "gamesave.h"
#include "damage.h"
#include "eproj_slower.h"
#include "eparticlepool.h"
#include "bot.h"

#define _USER_PROP_FILENAME		"w_staff.csv"
#define _PARTICLE_NAME			"stf_prt01" 
static cchar* _BotEliteGuard_pszSoundEffectBank = "elite";
//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponStaffBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CWeaponStaffBuilder _WeaponStaffBuilder;


//
//
//
void CWeaponStaffBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType )
{
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CWeaponBuilder, ENTITY_BIT_WEAPON, pszEntityType );
}


//
//
//
BOOL CWeaponStaffBuilder::PostInterpretFixup( void )
{
	return CWeaponBuilder::PostInterpretFixup();
}


//
//
//
BOOL CWeaponStaffBuilder::InterpretTable( void )
{
	return CWeaponBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponStaff
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************
BOOL						CWeaponStaff::m_bSystemInitialized	= FALSE;
u32							CWeaponStaff::m_uWpnClassClientCount = 0;
CFXSlower::_UserProps_t		CWeaponStaff::m_aFXProps[EUK_COUNT_STAFF];
CWeaponStaff::_UserProps_t	CWeaponStaff::m_aUserProps[EUK_COUNT_STAFF];
CEProjPool::PoolHandle_t	CWeaponStaff::m_hSlowerProjPool;
FParticle_DefHandle_t		CWeaponStaff::m_hParticleDef = FPARTICLE_INVALID_HANDLE;
CFSoundGroup*				CWeaponStaff::m_pSounds[STAFFSND_COUNT];
FLightInit_t				CWeaponStaff::m_LightInit;

static cchar* _pszSoundGroupTable[CWeaponStaff::STAFFSND_COUNT] = 
{
	"SRMEcharup",
	"SRMEchardn",
	"SRMEcharge",
	"SRMEstafire",
};

// This table describes to fgamedata how our user property table is to be interpreted:
const FGameData_TableEntry_t CWeaponStaff::m_aUserPropVocab[] =
{
	// pszMeshName:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszProjectilMeshName:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fProjInPoolCount:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_1000,

	// fMaxLiveRange:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_100000,

	// fProjCullDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fProjSpeed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_100000,

	// fMinTargetAssistDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fMaxTargetAssistDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fReloadTimeForOneRound:
	// fTicksPerRoundReload:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO | FGAMEDATA_FLAGS_FLOAT_OO_X,
	sizeof( f32 ) * 2,
	F32_DATATABLE_Pt01,
	F32_DATATABLE_60,

	// fStartScale:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10,
	
	// fEndScale:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10,

	// fShotFiredScale:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10,
	
	// fShotMaxScale:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10,

	// f32 fCosMaxProjAngle
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fMaxDevLengthSquared:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_100000,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

const FGameData_TableEntry_t CWeaponStaff::m_aFXPropsVocab[] =
{
	// pszBlastSphere:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszPartTrailName:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,
	
	// pszPartBlastName:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszDetonateBlastName:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//		f32		fGrowRateMin;		//	,20.0,#		_SLOWER_BLAST_GROW_RATE_MIN
	FGAMEDATA_VOCAB_F32_UNBOUND,
	//		f32		fGrowRateMax;		//	,60.0,#		_SLOWER_BLAST_GROW_RATE_MAX
	FGAMEDATA_VOCAB_F32_UNBOUND,
	//		f32		fGrowRateWobble;		//,1.5,#		_SLOWER_BLAST_GROW_RATE_WOBBLE
	FGAMEDATA_VOCAB_F32_UNBOUND,
	//		f32		fFadeOutTime;		// ,F32_DATATABLE_1,#		_SLOWER_SPHERE_FADE_OUT_TIME
	FGAMEDATA_VOCAB_F32_UNBOUND,
	//		f32		fSphereScale;		//	 ,F32_DATATABLE_1,#		_SLOWER_BLAST_SPHERE_SCALE
	FGAMEDATA_VOCAB_F32_UNBOUND,
	//		f32		fPartRadiusGrow;		//	,50.0,#		_SLOWER_PARTICLE_RADIUS_GROW_RATE
	FGAMEDATA_VOCAB_F32_UNBOUND,
	//		f32		fPartBurstRateMin;	//	,0.3,#		_SLOWER_PARTICLE_BURST_RATE_MIN
	FGAMEDATA_VOCAB_F32_UNBOUND,
	//		f32		fPartBurstRateMax;		// ,1.5,#		_SLOWER_PARTICLE_BURST_RATE_MAX
	FGAMEDATA_VOCAB_F32_UNBOUND,
	//		f32		fPartBaseVel;		//,2.7,#		_SLOWER_PARTICLE_BASE_VEL
	FGAMEDATA_VOCAB_F32_UNBOUND,
	//		f32		fPartBaseVelMin;		// ,0.5,#		_SLOWER_PARTICLE_BASE_VEL_MIN
	FGAMEDATA_VOCAB_F32_UNBOUND,
	//		f32		fPartBaseVelMax;//	,F32_DATATABLE_1,#		_SLOWER_PARTICLE_BASE_VEL_MAX
	FGAMEDATA_VOCAB_F32_UNBOUND,

	//u32		uMaxBlasts;			// ,1
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,
	//u32		uMaxSpheres;			// ,3,		_MAX_BLAST_SPHERES
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,
	//u32		uMaxSparklers;		//	,13,# 		_MAX_SPARKLERS
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//		f32		fMaxRadius;			// ,3,		_MAX_BLAST_SPHERES
	FGAMEDATA_VOCAB_F32_UNBOUND,

	//		f32		fLife;			// ,3,		_MAX_BLAST_SPHERES
	FGAMEDATA_VOCAB_F32_UNBOUND,

	// f32		fUnitSlowedDown;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,
	// f32		fFullEffectTime;
	FGAMEDATA_VOCAB_F32_UNBOUND,
	// f32		fRecoveryTime;
	FGAMEDATA_VOCAB_F32_UNBOUND,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

const FGameDataMap_t CWeaponStaff::m_aUserPropMapTable[] =
{
	"StaffL1",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[0],

	"ImpactL1",
	m_aFXPropsVocab,
	sizeof(m_aFXPropsVocab),
	(void *)&m_aFXProps[0],

	NULL
};


//
//
//
CWeaponStaff::CWeaponStaff( void )
{
//	m_paWorldMesh = NULL;
//	m_hTracerGroup = TRACER_NULLGROUPHANDLE;
}


//
//
//
CWeaponStaff::~CWeaponStaff( void )
{
	if ( IsSystemInitialized() && IsCreated() )
	{
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CWeaponStaff::InitSystem( void )
{
	FASSERT( !m_bSystemInitialized );
	m_bSystemInitialized = TRUE;
	m_uWpnClassClientCount = 0;

	return TRUE;
}

BOOL CWeaponStaff::ClassHierarchyLoadSharedResources ( void )
{
	FASSERT( m_bSystemInitialized );
	FASSERT( m_uWpnClassClientCount != 0xffffffff );

	m_uWpnClassClientCount++;

	if( !CWeapon::ClassHierarchyLoadSharedResources() ) 
	{
		// Bail now since parent class has already called
		// ClassHierarchyUnloadSharedResources() and decremented
		// our client counter...
		return FALSE;
	}

	if( m_uWpnClassClientCount > 1 ) 
	{
		// Resources already loaded...
		return TRUE;
	}

	FResFrame_t ResFrame = fres_GetFrame();

	fang_MemZero( m_aUserProps, sizeof(m_aUserProps) );
	fang_MemZero( &m_LightInit, sizeof(m_LightInit) );

	// Read the user properties for all EUK levels of this weapon...
	if ( !fgamedata_ReadFileUsingMap( m_aUserPropMapTable, _USER_PROP_FILENAME ) )
	{
		DEVPRINTF( "CWeaponStaff::ClassHierarchyLoadSharedResources: Could not read user properties from file '%s'.\n", _USER_PROP_FILENAME );
		return FALSE;
	}

	CFXSlower::InitSystem(&m_aFXProps[0]);

	if( !CEProj_Slower::InitSystem(m_aUserProps[0].fMaxDevLengthSquared) )
	{
		DEVPRINTF( "CWeaponStaff::ClassHierarchyLoadSharedResources(): CEProj_Slower::InitSystem() failed.\n" );
		goto _ExitWithError;
	}

	// Create a pool of projectiles 
	m_hSlowerProjPool = CEProjPool::Create( CEProj::PROJTYPE_SLOWER, m_aUserProps[0].pszProjectileMeshName, (u32)m_aUserProps[0].fProjInPoolCount, FMESHINST_FLAG_POSTER_X|FMESHINST_FLAG_POSTER_Y|FMESHINST_FLAG_POSTER_Z);
	if ( m_hSlowerProjPool == EPROJPOOL_NULL_HANDLE )
	{
		DEVPRINTF( "CWeaponStaff::ClassHierarchyLoadSharedResources(): Could not create projectile pool.\n" );
		goto _ExitWithError;
	}

	m_hParticleDef = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, _PARTICLE_NAME );

	// Fill out our global info data for each EUK level...
	for (s32 i = 0; i < EUK_COUNT_STAFF; i++ )
	{
		FASSERT(i<1); // since I haven't implement more yet 

		CWeapon::Info_t* pInfo = &m_aaInfo[WEAPON_TYPE_STAFF][i];
		pInfo->nGrip = GRIP_RIGHT_ARM;
		pInfo->nReloadType = RELOAD_TYPE_FULLYAUTOMATIC;
		pInfo->nStanceType = STANCE_TYPE_STANDARD;
		pInfo->fMinTargetAssistDist = m_aUserProps[i].fMinTargetAssistDist;
		pInfo->fMaxTargetAssistDist = m_aUserProps[i].fMaxTargetAssistDist;
		pInfo->fMaxLiveRange = m_aUserProps[i].fMaxLiveRange;
		pInfo->nClipAmmoMax = -1;//(m_aUserProps[i].fClipAmmoMax) >= 0.0f ? (u16)m_aUserProps[i].fClipAmmoMax : -1;
		pInfo->nReserveAmmoMax = -1;//(m_aUserProps[i].fReserveAmmoMax) >= 0.0f ? (u16)m_aUserProps[i].fReserveAmmoMax : -1;
		pInfo->nInfoFlags = INFOFLAG_NONE;
		pInfo->nReticleType = CReticle::TYPE_DROID_STANDARD;
	}
	
	// Load the sound effects bank for this bot...
	if( !fresload_Load( FSNDFX_RESTYPE, _BotEliteGuard_pszSoundEffectBank) )
	{
		DEVPRINTF( "CWeaponStaff::ClassHierarchyLoadSharedResources(): Could not load sound effect bank '%s'\n", _BotEliteGuard_pszSoundEffectBank);
	}
	for (StaffSound_e iSndIdx = (StaffSound_e)0; iSndIdx < STAFFSND_COUNT; iSndIdx = (StaffSound_e)(int(iSndIdx)+1))
	{
		m_pSounds[iSndIdx] = CFSoundGroup::RegisterGroup( _pszSoundGroupTable[iSndIdx] );
		if (!m_pSounds[iSndIdx])
			DEVPRINTF( "CWeaponStaff::ClassHierarchyLoadSharedResources(): Could not load sound effect '%s'\n", _pszSoundGroupTable[iSndIdx]);
	}


	return TRUE;

	// Error...
_ExitWithError:
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


//
//
//
void CWeaponStaff::UninitSystem( void )
{
	m_bSystemInitialized = FALSE;
}

void CWeaponStaff::ClassHierarchyUnloadSharedResources( void )
{
	FASSERT( m_uWpnClassClientCount > 0 );

	--m_uWpnClassClientCount;

	if( m_uWpnClassClientCount > 0 ) 
	{
		return;
	}

	CFXSlower::UninitSystem();
	CEProj_Slower::UninitSystem();

	m_hSlowerProjPool = EPROJPOOL_NULL_HANDLE; 
	m_hParticleDef	  = FPARTICLE_INVALID_HANDLE;

	CWeapon::ClassHierarchyUnloadSharedResources();
}

//
//
//
BOOL CWeaponStaff::Create( cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName )
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
	CWeaponStaffBuilder *pBuilder = (CWeaponStaffBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if ( pBuilder == &_WeaponStaffBuilder )
	{
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_WEAPON );
	}

	// Set our builder parameters...

	// Create an entity...
	return CWeapon::Create( WEAPON_TYPE_STAFF, pszEntityName, pMtx, pszAIBuilderName );
}


//
//
//
void CWeaponStaff::ClassHierarchyDestroy( void )
{
	CEProj_Slower::DestroyEffects();
	fforce_Kill( &m_hForce );
/*
	fforce_Kill( &m_hForce );

	tracer_DestroyGroup( m_hTracerGroup );
	m_hTracerGroup = TRACER_NULLGROUPHANDLE;

	fdelete_array( m_paWorldMesh );
	m_paWorldMesh = NULL;
*/
	CWeapon::ClassHierarchyDestroy();
}


//
//
//
BOOL CWeaponStaff::ClassHierarchyBuild( void )
{
//	FMesh_t *pMesh;
	FMeshInit_t MeshInit;
//	FTexDef_t *pTexDef;
//	u32 i;

	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CWeaponStaffBuilder *pBuilder = (CWeaponStaffBuilder *)GetLeafClassBuilder();

	// Build parent class...
	if ( !CWeapon::ClassHierarchyBuild() )
	{
		// Parent class could not be built...
		goto _ExitWithError;
	}

    if( !CEProj_Slower::InitEffects() )
	{
		DEVPRINTF( "CWeaponStaff::Init(): CEProj_Slower::InitSystem() failed.\n" );
		goto _ExitWithError;
	}


	// Set defaults...
	_SetDefaults();
	// Fill out our global info data for each EUK level...
	_ResourceData_t *pResourceData;
	s32 i;
	for (i = 0,pResourceData=m_aResourceData; i < EUK_COUNT_STAFF; i++,++pResourceData )
	{
		FASSERT(i<1); // since I haven't implement more yet 

		// Create the world mesh...
		pResourceData->m_pWorldMesh = fnew CFWorldMesh;
		if( pResourceData->m_pWorldMesh == NULL ) {
			DEVPRINTF( "CWeaponStaff::Create(): Not enough memory to create m_pWorldMesh.\n" );
			goto _ExitWithError;
		}

		// Load the mesh resource...
		FMesh_t *pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, m_aUserProps[i].pszMeshName );
		if( pMesh == NULL ) {
			DEVPRINTF( "CWeaponStaff::Create(): Could not find mesh '%s'.\n", m_aUserProps[i].pszMeshName );
			pMesh = m_pErrorMesh;
			if( pMesh == NULL ) {
				goto _ExitWithError;
			}
		}

		// Init the world mesh...
		MeshInit.pMesh = pMesh;
		MeshInit.nFlags = FMESHINST_FLAG_NOBONES;
		MeshInit.fCullDist = m_aUserProps[i].fProjCullDist;
		MeshInit.Mtx.Set(*MtxToWorld());
		pResourceData->m_pWorldMesh->Init( &MeshInit, TRUE );
		CFWorldAttachedLight* pLight = pResourceData->m_pWorldMesh->GetFirstAttachedLight();
		if (pLight)
		{
			pLight->GetInit(&m_LightInit);
		}

		pResourceData->m_pWorldMesh->RemoveFromWorld();
		pResourceData->m_pWorldMesh->SetCollisionFlag( TRUE );
		pResourceData->m_pWorldMesh->m_nUser = MESHTYPES_ENTITY;
		pResourceData->m_pWorldMesh->m_pUser = this;
		pResourceData->m_pWorldMesh->SetUserTypeBits( TypeBits() );
		pResourceData->m_pWorldMesh->SetLineOfSightFlag(FALSE); //pgm added this so that weapons never block los tests
		FMATH_SETBITMASK( pResourceData->m_pWorldMesh->m_nFlags, FMESHINST_FLAG_POSTER_X|FMESHINST_FLAG_POSTER_Y|FMESHINST_FLAG_POSTER_Z); 


	}
	fforce_NullHandle( &m_hForce );


	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


//
//
//
BOOL CWeaponStaff::ClassHierarchyBuilt( void )
{
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if ( !CWeapon::ClassHierarchyBuilt() )
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


//
//
//
CEntityBuilder *CWeaponStaff::GetLeafClassBuilder( void )
{
	return &_WeaponStaffBuilder;
}


//
//
//
void CWeaponStaff::_SetDefaults( void )
{
	FASSERT( IsCreated() );

	m_pOwnerBot = NULL;
	m_nClipAmmo = 0;
	m_nReserveAmmo = 0;

	m_fUnitCharge = 0.f;

	m_pvStaffUnitDir = NULL;
	m_bFireTriggerDown=FALSE; 
	m_bFireThisFrame=FALSE;
	m_pParticleEmitter = NULL;
	m_fParticleIntensityMult = 0.75f;
	m_pWorldLight = NULL;
	fang_MemSet(m_pSoundEmitters,0,sizeof(m_pSoundEmitters[0])*STAFFSND_COUNT);
//	m_pLockedWorldMesh = NULL;
}


//
//
//
void CWeaponStaff::SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo )
{
	CWeapon::SetItemInst( pItemInst, bUpdateItemInstAmmoFromWeaponAmmo );

	if( pItemInst ) 
	{
		pItemInst->SetAmmoDisplayType( CItemInst::AMMODISPLAY_NUMERIC, CItemInst::AMMODISPLAY_NUMERIC );
	}
}


//
//
//
void CWeaponStaff::ClassHierarchyAddToWorld( void )
{
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CWeapon::ClassHierarchyAddToWorld();

	fang_MemSet(m_pSoundEmitters,0,sizeof(m_pSoundEmitters[0])*STAFFSND_COUNT);

	m_pWorldLight = m_aResourceData[m_nUpgradeLevel].m_pWorldMesh->GetFirstAttachedLight();
	m_pvStaffUnitDir = &m_MtxToWorld.m_vUp;
}


//
//
//
void CWeaponStaff::ClassHierarchyRemoveFromWorld( void )
{
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	for (StaffSound_e iSndIdx = (StaffSound_e)0; iSndIdx < STAFFSND_COUNT; iSndIdx = (StaffSound_e)(int(iSndIdx)+1))
	{
		_PlaySnd(iSndIdx, FALSE);
	}

	CWeapon::ClassHierarchyRemoveFromWorld();
	FASSERT(m_pWorldLight);
	m_pWorldLight->m_Light.Enable(FALSE);
}


void CWeaponStaff::ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled ) {
	CWeapon::ClassHierarchyDrawEnable( bDrawingHasBeenEnabled );
}


//
//
//
void CWeaponStaff::ClassHierarchyRelocated( void *pIdentifier )
{
	CWeapon::ClassHierarchyRelocated( pIdentifier );
}


//
//
//
void CWeaponStaff::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList)
{
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
 	apTrackerSkipList[nTrackerSkipListCount++] = m_aResourceData[m_nUpgradeLevel].m_pWorldMesh;
}


//
// Note that ClassHierarchyResetToState() is always called prior to this call.
//
void CWeaponStaff::ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel )
{
	FASSERT( !IsTransitionState( CurrentState() ) );
/*
	// Remove old mesh from world...
	m_paWorldMesh[nPreviousUpgradeLevel].RemoveFromWorld();

	if ( IsInWorld() )
	{
		m_paWorldMesh[m_nUpgradeLevel].m_Xfm = m_paWorldMesh[nPreviousUpgradeLevel].m_Xfm;
		m_paWorldMesh[m_nUpgradeLevel].UpdateTracker();
	}
*/
}


//
//
//
void CWeaponStaff::ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const
{
	FASSERT( IsCreated() );
	
	return CWeapon::ComputeMuzzlePoint_WS(pMuzzlePoint_WS);
}


u32 CWeaponStaff::TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pvTargetPoint_WS, const CFVec3A *pBuddyFirePos_WS/* = NULL*/ )
{
	/*
	CFColorRGBA red; red.OpaqueRed();
	CFVec3A begin,end;
	begin.Set(m_MtxToWorld.m_vPos);
	end.Mul(*m_pvStaffUnitDir,6.0f);
	end.Add(begin);
	fdraw_DevLine(&begin, &end, &red);
	*/
	m_bFireTriggerDown = (fUnitTriggerVal1 == 1.0f);
	m_bFireThisFrame = FALSE;

	if (m_bFireTriggerDown)
	{
		m_fUnitCharge += FLoop_fPreviousLoopSecs * m_aUserProps[0].fOOChargeTime;
		FMATH_CLAMP_UNIT_FLOAT(m_fUnitCharge);
	}
	else
	{
		if (m_fUnitCharge <  1.0f)
		{
			m_fUnitCharge = 0.0f;
		}
		else 
		{
			m_TargetPos_WS.Set( *pvTargetPoint_WS );
			if ( m_pOwnerBot && m_pOwnerBot->m_nPossessionPlayerIndex >= 0 )
			{
				m_pOwnerBot->FocusHumanTargetPoint_WS( &m_TargetPos_WS, this );
			}

			m_fUnitCharge = 0.0f;
			m_bFireThisFrame = TRUE;
		}
	}
	return m_bFireThisFrame;
}



//
//
//
BOOL CWeaponStaff::_DoesRayIntersectMesh( const CFVec3A *pRayStart, const CFVec3A *pRayEnd, CFWorldMesh *pWorldMesh )
{
	CFCollInfo CollInfo;

	CollInfo.nCollTestType = FMESH_COLLTESTTYPE_RAY;
	CollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_CHECK_ALL;
	CollInfo.bFindClosestImpactOnly = FALSE;
	CollInfo.nCollMask = FCOLL_MASK_CHECK_ALL;
	CollInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
	CollInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
	CollInfo.bCullBacksideCollisions = TRUE;
	CollInfo.Ray.Init( pRayStart, pRayEnd );

	return pWorldMesh->CollideWithMeshTris( &CollInfo );
}


//
//
//
void CWeaponStaff::ClassHierarchyWork( void )
{
	CWeapon::ClassHierarchyWork();

	if ( !IsOurWorkBitSet() )
	{
		return;
	}
	if (!m_aResourceData[m_nUpgradeLevel].m_pWorldMesh->IsAddedToWorld()) // if not in world
	{
		if (m_bFireTriggerDown) // and trigger's down
		{						// add to world, start charging up
			m_aResourceData[m_nUpgradeLevel].m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_aUserProps[m_nUpgradeLevel].fChargeBeginScale );
			m_aResourceData[m_nUpgradeLevel].m_pWorldMesh->UpdateTracker();
			FASSERT(m_pWorldLight);
			m_pWorldLight->m_Light.Enable(TRUE);
			_PlaySnd(STAFFSND_CHARGE_UP, TRUE);
		}
	}
	else // else if in world
	{
		if (m_bFireTriggerDown) // and triger down
		{						// scale and audioize
			f32 fScaleSlider = fmath_UnitLinearToSCurve(m_fUnitCharge);
			f32 fSetScale = FMATH_FPOT(fScaleSlider,m_aUserProps[m_nUpgradeLevel].fChargeBeginScale,m_aUserProps[m_nUpgradeLevel].fChargeEndScale);
			m_aResourceData[m_nUpgradeLevel].m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, fSetScale );
			m_aResourceData[m_nUpgradeLevel].m_pWorldMesh->UpdateTracker();
			if ( (m_fUnitCharge==1.0f) && !m_pSoundEmitters[STAFFSND_CHARGE_LOOP])
			{
				_PlaySnd(STAFFSND_CHARGE_LOOP, TRUE); 
			}
		}
		else // else fire or scale down
		{
			m_aResourceData[m_nUpgradeLevel].m_pWorldMesh->RemoveFromWorld();
			FASSERT(m_pWorldLight);
			m_pWorldLight->m_Light.Enable(FALSE);

			if (m_bFireThisFrame)
			{
				_PlaySnd(STAFFSND_CHARGE_LOOP, FALSE);
			}
			else
			{
				if (m_pSoundEmitters[STAFFSND_CHARGE_UP]) // we're charging
				{
					_PlaySnd(STAFFSND_CHARGE_UP, FALSE); // not anymore
					_PlaySnd(STAFFSND_CHARGE_LOOP, FALSE); // not anymore
					_PlaySnd(STAFFSND_CHARGE_DOWN, TRUE);
				}	
			}
		}
	}

	if (m_aResourceData[m_nUpgradeLevel].m_pWorldMesh->IsAddedToWorld())
	{
		if (!m_pParticleEmitter)
		{
			m_pParticleEmitter = eparticlepool_GetEmitter();
			if (m_pParticleEmitter)
			{
				m_pParticleEmitter->Relocate_RotXlatFromUnitMtx_WS(&m_MtxToWorld);
				m_pParticleEmitter->Attach_ToParent_WS(this);
				m_pParticleEmitter->StartEmission(m_hParticleDef,m_fUnitCharge*m_fParticleIntensityMult);
			}
			m_pWorldLight->m_Light.SetIntensity(0.0f); // starting from out.
		}
		else
		{
			m_pParticleEmitter->SetUnitIntensity(m_fUnitCharge*m_fParticleIntensityMult);
			FASSERT(m_pWorldLight);
			m_pWorldLight->m_Light.SetIntensity(m_fUnitCharge); 
		}
	}
	else
	{
		if (m_pParticleEmitter && m_pParticleEmitter->IsEmitting())
		{
			m_pParticleEmitter->DetachFromParent();
			m_pParticleEmitter->StopEmission();
			m_pParticleEmitter = NULL;
		}
	}

	if (!m_bFireThisFrame)
	{
		return;
	}

	// moved from triggerwork... calculate fire dir
	CFVec3A vUnitFireDir;
	CFVec3A vMuzzlePt;
	f32 fMag2;
	ComputeMuzzlePoint_WS( &vMuzzlePt );
	vUnitFireDir.Sub( m_TargetPos_WS, vMuzzlePt );
	fMag2 = vUnitFireDir.MagSq();
	if( fMag2 < 0.0f ) 
	{
		vUnitFireDir = *m_pvStaffUnitDir;
	}
	else
	{
		vUnitFireDir.Mul( fmath_InvSqrt( fMag2 ) );

		if (m_pvStaffUnitDir->Dot(vUnitFireDir) < m_aUserProps[0].fCosMaxProjAngle)
		{
			vUnitFireDir = *m_pvStaffUnitDir;
		}
	}


	// Get this far means we're firing the impulse....
	if ( m_nUpgradeLevel == 0 )
	{
		CEProj *pProj = CEProjPool::GetProjectileFromFreePool( m_hSlowerProjPool );
		if( pProj == NULL ) 
		{
			// No more projectiles...
			return;
		}

		// Let's fire a round...
		CFMtx43A ProjMtx;
		ProjMtx.m_vFront.Set(vUnitFireDir);
		ProjMtx.m_vRight.UnitCrossYWithVec( ProjMtx.m_vFront );
		ProjMtx.m_vUp.Cross( ProjMtx.m_vFront, ProjMtx.m_vRight );
		ComputeMuzzlePoint_WS( &ProjMtx.m_vPos );

		pProj->Init();
		pProj->SetAmbientPropertiesFromSoundGroup(m_pSounds[STAFFSND_CHARGE_LOOP]);
		pProj->SetDamager( this, m_pOwnerBot );

		//		pProj->SetDamageProfile( m_apDamageProfile[ m_nUpgradeLevel ] );
		pProj->SetLightInit(&m_LightInit);
		pProj->SetSkipListCallback( _BuildProjSkipList );
		pProj->SetDamageCallback( CEProj_Slower::DamageCallback );
		pProj->SetDetonateCallback( _ProjDetonateCallback );
		pProj->SetMaxDistCanTravel( m_aUserProps[m_nUpgradeLevel].fMaxLiveRange );
		pProj->SetLinSpeed( m_aUserProps[m_nUpgradeLevel].fProjSpeed );
		pProj->SetLinUnitDir_WS( &vUnitFireDir );
		pProj->Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &ProjMtx, m_aUserProps[m_nUpgradeLevel].fShotFiredScale );

		// assign targets while trigger is pressed
		pProj->SetTargetedEntity(GetTargetedEntity());
		pProj->Launch();

		// Spawn a force feedback effect
		s32 nPlayerIndex = GetSafeOwnerPlayerIndex();
		if ( nPlayerIndex >= 0 )
		{
			fforce_Kill( &m_hForce );
			fforce_Play( Player_aPlayer[nPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_LIGHT_PULSE, &m_hForce );
		}
		_PlaySnd(STAFFSND_FIRE, TRUE);

		if (GetOwner() && GetOwner()->m_nPossessionPlayerIndex > -1)
		{
			AIEnviro_BoostPlayerSoundTo(GetOwner()->m_nPossessionPlayerIndex, 30.0f);
		}
	}
}

//
//
//

//
//
//
void CWeaponStaff::KillRumble( void )
{
	fforce_Kill( &m_hForce );
}


BOOL CWeaponStaff::CheckpointSave( void )
{
	// save base class data
	CWeapon::CheckpointSave();
	return TRUE;
}

void CWeaponStaff::CheckpointRestore( void )
{
	// restore base class data
	CWeapon::CheckpointRestore();
}

void CWeaponStaff::_BuildProjSkipList( CEProj *pProj )
{
	CBot* pBot = (CBot*)pProj->GetDamagerBot();
	pBot->AppendTrackerSkipList();
}

BOOL CWeaponStaff::_ProjDetonateCallback( CEProj *pProj, BOOL bMakeEffect, CEProj::Event_e nEvent, const FCollImpact_t *pImpact ) 
{
	if (bMakeEffect)
	{
		if (nEvent==CEProj::EVENT_INTERCEPTED)
		{
			CFXSlower::SpawnSlowerEffect(NULL, *pProj->MtxToWorld(), NULL, TRUE);
			return FALSE;
		}
		else if (nEvent==CEProj::EVENT_HIT_GEO)
		{
			FExplosionSpawnParams_t SpawnParams;
			SpawnParams.InitToDefaults();
			SpawnParams.pDamager= pProj->GetDamager();
			SpawnParams.Pos_WS  = pImpact->ImpactPoint;
			SpawnParams.UnitDir = pImpact->UnitFaceNormal;

			CEntity* pEntity = CGColl::ExtractEntity( pImpact );

			if( pEntity ) 
			{
				// It's an Entity!
				CEntity *pHitEntity = (CEntity *)pEntity;
				if ( (pHitEntity->TypeBits() & ENTITY_BIT_BOT) &&
					(!(pHitEntity->TypeBits() & ENTITY_BIT_VEHICLE)) )
				{
					pProj->MtxToWorld()->m_vUp.Set(pHitEntity->MtxToWorld()->m_vRight);
					pProj->MtxToWorld()->m_vFront.Set(pHitEntity->MtxToWorld()->m_vUp);
					pProj->MtxToWorld()->m_vRight.Cross(pProj->MtxToWorld()->m_vUp,pProj->MtxToWorld()->m_vFront);
					pProj->MtxToWorld()->m_vPos.Set(pHitEntity->MtxToWorld()->m_vPos);
					CFXSlower::SpawnSlowerEffect((CBot*)pProj->GetDamagerBot(),*pProj->MtxToWorld(),&pHitEntity->MtxToWorld()->m_vPos);
					CFXSlower::SpawnSlowerBurst( SpawnParams );
					return FALSE;
				}
			}
			CFVec3A vUp;
			vUp.Sub(pImpact->aTriVtx[1],pImpact->aTriVtx[0]);
			vUp.Unitize();
			
			CFMtx43A mtxEffect;
			mtxEffect.m_vUp.Set(vUp);
			mtxEffect.m_vFront.Set(pImpact->UnitFaceNormal);
			mtxEffect.m_vRight.Cross(mtxEffect.m_vUp,mtxEffect.m_vFront);
			mtxEffect.m_vPos.Set(pImpact->ImpactPoint);
			CFXSlower::SpawnSlowerEffect((CBot*)pProj->GetDamagerBot(),mtxEffect);
			CFXSlower::SpawnSlowerBurst( SpawnParams );
			return FALSE;
		}
	}
	return FALSE;
}

void CWeaponStaff::_PlaySnd(StaffSound_e eSnd, BOOL bPlay)
{
	if (eSnd >= STAFFSND_COUNT)
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