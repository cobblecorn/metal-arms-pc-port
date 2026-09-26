//////////////////////////////////////////////////////////////////////////////////////
// botmortar.cpp -
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
#include "fclib.h"
#include "botmortar.h"
#include "fresload.h"
#include "meshtypes.h"
#include "protrack.h"
#include "AI/AIGameUtils.h"
#include "AI/AIAPI.h"
#include "AI/AIBrain.h"
#include "AI/AIMover.h"
#include "eproj_mortar.h"
#include "fxshockwave.h"
#include "ebox.h"
#include "fsound.h"
#include "fstringtable.h"
#include "botpart.h"
//#include "miscdraw.h"

#define _BOTINFO_FILENAME		"b_mortar"
#define _BOTPART_FILENAME		"bp_mortar"

static s32   _PART_INSTANCE_COUNT_PER_TYPE	= 1;
static s32	 _LIMB_TYPE_COUNT				= 1;

static cchar*			_apszMortarMeshFilenames[] = {"GRMMMortr00"};
static cchar*			_Mortar_pszSoundEffectBank = "mortar";
static cchar*			_pszProjectileMeshName = "GRMMshell01";
static CBotMortar*		_pThisBot = NULL;
//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotMortarBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CBotMortarBuilder _BotMortarBuilder;

void CBotMortarBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType )
{	
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_BOTMORTAR, pszEntityType );

	// Override defaults set by our parent classes...
	m_nEC_HealthContainerCount = CBotMortar::m_BotInfo_Gen.nBatteryCount;
	m_pszEC_ArmorProfile = CBotMortar::m_BotInfo_Gen.pszArmorProfile;
	
	m_pszBoxRange = NULL;

	// This class of bots shows up on Glitch's radar...
	FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_SHOWS_UP_ON_RADAR );
}

BOOL CBotMortarBuilder::PostInterpretFixup( void )
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
	
BOOL CBotMortarBuilder::InterpretTable( void )
{
	if (!fclib_stricmp( CEntityParser::m_pszTableName, "entityrange" ))
	{
		cchar* szEntityRange=NULL;
		CEntityParser::Interpret_String(&szEntityRange);
		if (!szEntityRange)
		{
			CEntityParser::Error_InvalidParameterValue();
		}
		m_pszBoxRange = szEntityRange;
		return TRUE;
	}
	
	return CBotBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotMortar
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL	CBotMortar::m_bSystemInitialized	= FALSE;
u32		CBotMortar::m_nBotClassClientCount	= 0;
s32		CBotMortar::m_nBoneIndexPrimaryFire;
s32		CBotMortar::m_nBoneIndexBarrelBase;
s32		CBotMortar::m_nBoneIndexSwivel;

CBotMortar::BotInfo_Gen_t 		CBotMortar::m_BotInfo_Gen;
CBotMortar::BotInfo_MountAim_t 	CBotMortar::m_BotInfo_MountAim;
CBotMortar::BotInfo_Walk_t 		CBotMortar::m_BotInfo_Walk;
CBotMortar::MortarInfo_t		CBotMortar::m_MortarInfo;
CEProjPool::PoolHandle_t		CBotMortar::m_hMortarProjPool;
CBotPartPool*					CBotMortar::m_pPartPool;

CBotAnimStackDef CBotMortar::m_AnimStackDef;
FExplosion_GroupHandle_t CBotMortar::m_hExplLaunchGroup = FEXPLOSION_INVALID_HANDLE;
FExplosion_GroupHandle_t CBotMortar::m_hExplDetonateGroup = FEXPLOSION_INVALID_HANDLE;

CFSoundGroup* CBotMortar::m_pSounds[MORTARSND_COUNT];

static cchar* _pszSoundGroupTable[CBotMortar::MORTARSND_COUNT] = 
{
	"SRMMfire",//	MORTARSND_FIRE,
	"SRMMidle",//	MORTARSND_IDLE,
	"SRMMpitch",//	MORTARSND_PITCH,
};

BOOL CBotMortar::InitSystem( void )
{
	FASSERT( !m_bSystemInitialized );
	m_bSystemInitialized = TRUE;
	m_nBotClassClientCount = 0;
	return TRUE;
}

BOOL CBotMortar::ClassHierarchyLoadSharedResources( void ) 
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

	// Load the sound effects bank for this bot... (gotta load before reading the data file when there's soundgroups the data file)
	if( !fresload_Load( FSNDFX_RESTYPE, _Mortar_pszSoundEffectBank) )
	{
		DEVPRINTF( "CBotMozer::ClassHierarchyLoadSharedResources(): Could not load sound effect bank '%s'\n", _Mortar_pszSoundEffectBank);
	}

	if( !ReadBotInfoFile( m_aGameDataMap, _BOTINFO_FILENAME ) )
	{
		goto _ExitInitSystemWithError;
	}

	if( !_BuildAnimStackDef() )
	{
		goto _ExitInitSystemWithError;
	}
	
	if( !CEProj_Mortar::LoadSharedResources() )
	{
		DEVPRINTF( "CBotMortar::Init(): CEProj_Mortar::LoadSharedResources() failed.\n" );
		goto _ExitInitSystemWithError;
	}

	// Create a pool of projectiles 
	m_hMortarProjPool = CEProjPool::Create(CEProj::PROJTYPE_MORTAR, _pszProjectileMeshName, m_MortarInfo.uPoolCannonsBalls,FMESHINST_FLAG_NOSHADOWLOD|FMESHINST_FLAG_CAST_SHADOWS);
	if ( m_hMortarProjPool == EPROJPOOL_NULL_HANDLE )
	{
		DEVPRINTF( "CBotMortar::Init(): Could not create projectile pool.\n" );
		goto _ExitInitSystemWithError;
	}

	m_hExplLaunchGroup = CExplosion2::GetExplosionGroup( m_MortarInfo.pszLaunchExplosion );
	if( m_hExplLaunchGroup == FEXPLOSION_INVALID_HANDLE ) 
	{
		DEVPRINTF( "CBotMortar::Init(): Could not find explosion definition '%s'.\n", m_MortarInfo.pszLaunchExplosion );
		goto _ExitInitSystemWithError;
	}
	
	m_hExplDetonateGroup = CExplosion2::GetExplosionGroup( m_MortarInfo.pszDetonateExplosion );
	if( m_hExplDetonateGroup == FEXPLOSION_INVALID_HANDLE ) 
	{
		DEVPRINTF( "CBotMortar::Init(): Could not find explosion definition '%s'.\n", m_MortarInfo.pszDetonateExplosion );
		goto _ExitInitSystemWithError;
	}

	if (1)
	{
		FLightInit_t LightInit;
		LightInit.nType = FLIGHT_TYPE_SPOT;
		fclib_strncpy(LightInit.szPerPixelTexName,"TRMMtarget1",FLIGHT_TEXTURE_NAME_LEN);
		LightInit.szPerPixelTexName[FLIGHT_TEXTURE_NAME_LEN]=0;
		LightInit.szCoronaTexName[0] = 0;
		LightInit.nLightID = 0xffff;
		LightInit.fIntensity = 1.0f;
		LightInit.Motif.Set( 1.0f, 1.0f, 1.0f, 1.0f );
		LightInit.nFlags = FLIGHT_FLAG_GAMEPLAY_LIGHT|FLIGHT_FLAG_HASPOS|FLIGHT_FLAG_HASDIR|FLIGHT_FLAG_PER_PIXEL;

		fclib_strncpy(LightInit.szName,"TRMMtarget1",FLIGHT_NAME_LEN);
		LightInit.szName[FLIGHT_NAME_LEN] = 0;
		LightInit.fSpotInnerRadians = 0.1f;
		LightInit.fSpotOuterRadians = 0.2f;
		
		LightInit.mtxOrientation.Identity();
		LightInit.spInfluence.m_Pos.Set(0.0f,0.0f,0.0f);
		LightInit.spInfluence.m_fRadius = 99.0f;
		m_WorldLight.Init(&LightInit);
		m_WorldLight.RemoveFromWorld();
	}

	for (MortarSound_e iSndIdx = (MortarSound_e)0; iSndIdx < MORTARSND_COUNT; iSndIdx = (MortarSound_e)(int(iSndIdx)+1))
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

void CBotMortar::UninitSystem( void )
{
	m_bSystemInitialized = FALSE;
}

void CBotMortar::ClassHierarchyUnloadSharedResources( void ) 
{
	FASSERT( m_nBotClassClientCount > 0 );

	--m_nBotClassClientCount;

	if( m_nBotClassClientCount > 0 ) 
	{
		return;
	}
	m_AnimStackDef.Destroy();

	CEProj_Mortar::UnloadSharedResources();
	m_hMortarProjPool = EPROJPOOL_NULL_HANDLE; 
	m_hExplLaunchGroup = FEXPLOSION_INVALID_HANDLE;
	m_hExplDetonateGroup = FEXPLOSION_INVALID_HANDLE;

	CBot::ClassHierarchyUnloadSharedResources();
}

void CBotMortar::ClassHierarchyResolveEntityPointerFixups()
{
	FASSERT(IsCreated());

	CBot::ClassHierarchyResolveEntityPointerFixups();
	
//ARG - >>>>>
//ARG - initialization crosses 'goto' scope
	CEntity* pEntity;
//ARG - <<<<<

	if (!m_pszBoxRange)
	{
		return; // we're just not going to move around, ok?
	}
	pEntity = CEntity::Find((cchar *)(m_pszBoxRange));
	if (pEntity && pEntity->TypeBits() & ENTITY_BIT_BOX)
	{
		m_pEBoxRange = (CEBox*) pEntity;
	}
	else
	{
		goto _EXIT_WITH_ERROR;
	}
	return;

_EXIT_WITH_ERROR:
	DEVPRINTF("FAILED TO RESOLVE BOX RANGE NAME");
	return;
}

CBotMortar::CBotMortar() : CBot()
{

}


CBotMortar::~CBotMortar()
{
	if( IsSystemInitialized() && IsCreated() )
	{
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld(TRUE);
		ClassHierarchyDestroy();
	}
}

BOOL CBotMortar::Create( s32 nPlayerIndex, cchar *pszEntityName, const CFMtx43A *pMtx)
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
	CBotMortarBuilder *pBuilder = (CBotMortarBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_BotMortarBuilder )
	{
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOTMORTAR );
	}

	// Set our builder parameters...

	// Create an entity...
	return CBot::Create( &m_BotDef, nPlayerIndex, FALSE, pszEntityName, pMtx, "Default" );
}


void CBotMortar::ClassHierarchyDestroy( void )
{	// Delete the items that we had instantiated for us...
	m_Anim.Destroy();

	fdelete( m_pWorldMesh );
	m_pWorldMesh = NULL;

	for (MortarSound_e iSndIdx = (MortarSound_e)0; iSndIdx < MORTARSND_COUNT; iSndIdx = (MortarSound_e)(int(iSndIdx)+1))
	{
		if (m_pSoundEmitters[iSndIdx])
		{
			m_pSoundEmitters[iSndIdx]->Destroy();
			m_pSoundEmitters[iSndIdx] = NULL;
		}
	}

	CBot::ClassHierarchyDestroy();
}


BOOL CBotMortar::ClassHierarchyBuild( void )
{
	FMesh_t *pMesh;
	FMeshInit_t MeshInit;
	s32 nBoneIndex;
	cchar * pszMeshFilename = NULL;
	CFMtx43A* pBarrelMtx = NULL;		//CPS 4.7.03
	
	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CBotMortarBuilder *pBuilder = (CBotMortarBuilder *)GetLeafClassBuilder();

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
	m_vHomeCoordWS.Set(pBuilder->m_EC_Mtx_WS.m_vPos);
	if (pBuilder->m_pszBoxRange)
	{
		// copy the string into the "Main" string table and store the pointer
		m_pszBoxRange = CFStringTable::AddString( "Main", pBuilder->m_pszBoxRange );
	}
	else
	{
		m_pszBoxRange = NULL;
	}

	pszMeshFilename = "GRMMmortrd0";
	if( pBuilder->m_pszMeshReplacement ) {
	   pszMeshFilename = pBuilder->m_pszMeshReplacement;
	}
	pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pszMeshFilename);
	if( pMesh == NULL )
	{
		DEVPRINTF( "CBotMortar::ClassHierarchyBuild(): Could not find mesh '%s'\n", pszMeshFilename );
		goto _ExitWithError;
	}
	// Init the world mesh...
	MeshInit.pMesh = pMesh;
	MeshInit.nFlags = 0;
	MeshInit.fCullDist = FMATH_MAX_FLOAT;
	MeshInit.Mtx.Set( pBuilder->m_EC_Mtx_WS );
	m_DeadMesh.Init( &MeshInit );
	m_DeadMesh.m_nUser = MESHTYPES_ENTITY;
	m_DeadMesh.m_pUser = this;
	m_DeadMesh.SetUserTypeBits( TypeBits() );
	m_DeadMesh.m_nFlags &= ~(FMESHINST_FLAG_DONT_DRAW | FMESHINST_FLAG_NOCOLLIDE);
	m_DeadMesh.SetCollisionFlag( TRUE );
	m_DeadMesh.SetLineOfSightFlag( FALSE );
	m_DeadMesh.UpdateTracker();
	m_DeadMesh.RemoveFromWorld();

	// Load mesh resource...
	pszMeshFilename = _apszMortarMeshFilenames[0];
	if ( pBuilder->m_uMeshVersionOverride >0 && pBuilder->m_uMeshVersionOverride < sizeof(_apszMortarMeshFilenames)/sizeof(char*) )
	{
		pszMeshFilename = _apszMortarMeshFilenames[pBuilder->m_uMeshVersionOverride];
	}
	pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pszMeshFilename);
	if( pMesh == NULL )
	{
		DEVPRINTF( "CBotMortar::ClassHierarchyBuild(): Could not find mesh '%s'\n", pszMeshFilename );
		goto _ExitWithError;
	}

	// Create world mesh...
	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL )
	{
		DEVPRINTF( "CBotMortar::ClassHierarchyBuild(): Not enough memory to create CFWorldMesh.\n" );
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

	m_nBoneIndexPrimaryFire = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_FIRE_PRIMARY] );
	m_nBoneIndexBarrelBase = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_BARREL_BASE] );
	m_nBoneIndexSwivel = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_SWIVEL] );

	if ( (m_nBoneIndexPrimaryFire < 0) || (m_nBoneIndexBarrelBase  < 0) || (m_nBoneIndexSwivel < 0))
	{
		DEVPRINTF( "CBotMortar::ClassHierarchyBuild(): Trouble retrieving bone indices.\n" );
		goto _ExitWithError;
	}

	// Build our animation stack and load animations...
	if( !m_Anim.Create( &m_AnimStackDef, m_pWorldMesh ) )
	{
		DEVPRINTF( "CBotMortar::ClassHierarchyBuild(): Trouble creating m_Anim.\n" );
		goto _ExitWithError;
	}

	SetControlValue( ANIMCONTROL_STAND, 1.0f );
	UpdateUnitTime( ANIMTAP_STAND, fmath_RandomFloat() );

	m_Anim.m_pAnimCombiner->ComputeMtxPalette( FALSE );
//CPS 4.7.03	CFMtx43A* pBarrelMtx = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexBarrelBase];
	pBarrelMtx = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexBarrelBase];
	m_fPitchAdjWS = -fmath_Atan( pBarrelMtx->m_vFront.y, pBarrelMtx->m_vFront.MagXZ() );


	m_Anim.m_pAnimCombiner->SetBoneCallback( &_AnimBoneCallback );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_SWIVEL] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_BARREL_BASE] );

	// TODO: ADD DUST ON FOOTSTEPS
/*
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
*/
	
	/*
	if( !m_AnimManFrameAim.Create( m_AnimStackDef.m_nBoneCount, m_AnimStackDef.m_apszBoneNameTable ) )
	{
		DEVPRINTF( "CBotMortar::ClassHierarchyCreate(): Could not create aim animation summer.\n" );
		goto _ExitWithError;
	}

	m_AnimManFrameAim.Reset();
	*/
	SetMaxHealth();

	// SER: For some reason, botmortar didn't have this in it. I put it in!
	// Initialize matrix palette (this is important for attached entities)...
	AtRestMatrixPalette();

	// Create tag points...
	if( !TagPoint_CreateFromBoneArray( m_anTagPointBoneNameIndexArray, m_apszBoneNameTable ) )
	{
		goto _ExitWithError;
	}

	// Find approx eye point...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_BARREL_TOP ] );
	if( nBoneIndex < 0 )
	{
		DEVPRINTF( "CBotMozer::ClassHierarchyBuild(): Could not locate approx eye point bone '%s'.\n", m_apszBoneNameTable[ BONE_BARREL_TOP ] );
		m_pApproxEyePoint_WS = &m_MtxToWorld.m_vPos;
	}
	else
	{
		m_pApproxEyePoint_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
	}

	SetBotFlag_Enemy();

	// set up part mgr...
	if( !m_pPartMgr->Create( this, &m_pPartPool, _BOTPART_FILENAME, _PART_INSTANCE_COUNT_PER_TYPE, _LIMB_TYPE_COUNT ) ) 
	{
		DEVPRINTF("problem parsing file %s", _BOTPART_FILENAME);
	}

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


BOOL CBotMortar::ClassHierarchyBuilt( void )
{
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CBot::ClassHierarchyBuilt() )
	{
		goto _ExitWithError;
	}

	EnableOurWorkBit();
	SetBotFlag_MeshCollideOnly();

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


CEntityBuilder *CBotMortar::GetLeafClassBuilder( void )
{
	return &_BotMortarBuilder;
}


void CBotMortar::_ClearDataMembers( void )
{
	// Init data members...

	m_pMoveIdentifier = &m_fPitchAdjWS;

	m_pBotInfo_Gen = &m_BotInfo_Gen;
	m_pBotInfo_MountAim = &m_BotInfo_MountAim;
	m_pBotInfo_Walk = &m_BotInfo_Walk;


	m_fGravity = m_pBotInfo_Gen->fGravity;
	m_fMaxFlatSurfaceSpeed_WS = m_pBotInfo_Walk->fMaxXlatVelocity * m_fRunMultiplier;

	m_fMountPitchMax_WS = m_pBotInfo_MountAim->fMountPitchDownLimit;
	m_fMountPitchMin_WS = m_pBotInfo_MountAim->fMountPitchUpLimit;

	m_anAnimStackIndex[ASI_STAND] = -1;
	m_anAnimStackIndex[ASI_STAND_ALERT] = -1;

	m_anAnimStackIndex[ASI_WALK] = ANIMTAP_WALK;
	m_anAnimStackIndex[ASI_WALK_ALERT] = -1;
	m_anAnimStackIndex[ASI_RUN] = ANIMTAP_WALK;
	m_anAnimStackIndex[ASI_RUN_PANIC] = -1;

	m_anAnimStackIndex[ASI_FALL] = ANIMTAP_STAND;

	m_anAnimStackIndex[ASI_HOP_LEFT] = -1;
	m_anAnimStackIndex[ASI_HOP_RIGHT] = -1;
	m_anAnimStackIndex[ASI_STARTLE] = -1;
	m_anAnimStackIndex[ASI_ROLL_LEFT] = -1;
	m_anAnimStackIndex[ASI_ROLL_RIGHT] = -1;

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

	m_anAnimStackIndex[ASI_RC_TETHERED]		= -1;
	m_anAnimStackIndex[ASI_RC_POWER_DOWN]	= ANIMTAP_POWER_DOWN;
	m_anAnimStackIndex[ASI_RC_POWER_UP]		= ANIMTAP_POWER_UP;
	m_anAnimStackIndex[ASI_STOOP]			= -1;

	m_anAnimStackIndex[ASI_AIM_PILLBOX]		= -1;

	m_pnEnableBoneNameIndexTableForSummer_Normal	  =	 NULL;//m_anEnableBoneNameIndexTableForSummer_Normal;
	m_pnEnableBoneNameIndexTableForSummer_TetherShock =  NULL;//m_anEnableBoneNameIndexTableForSummer_TetherShock;
	
	m_fCollCylinderRadius_WS = 2.0f;
	m_fCollCylinderHeight_WS = 4.0f;

	// new variables inited here
	m_eState = MS_SCANNING;
	m_vEnemyVelocity.Set(0.f,0.f,0.f);
	m_vGotoCoordWS.Set(0.f,0.f,0.f);
	m_pEnemyTarget = NULL;
	m_pEnemyTargetDistance2 = 0.f;
	m_fRetargetTime = 0.0f;
	m_nLastTagPointInspected = 0;
	m_fMortarSecondsCountdownTimer = m_MortarInfo.fOORoundsPerSec;
	m_fTimeSinceEnemyLOS = 0.0f;
	m_fRotateVolume = 0.35f;
	fang_MemSet(m_pSoundEmitters,0,sizeof(m_pSoundEmitters[0])*MORTARSND_COUNT);
}


void CBotMortar::ClassHierarchyAddToWorld( void )
{
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CBot::ClassHierarchyAddToWorld();
	m_pWorldMesh->UpdateTracker();
	SetAmbientPropertiesFromSoundGroup( m_pSounds[MORTARSND_IDLE], TRUE );
}


void CBotMortar::ClassHierarchyRemoveFromWorld( void )
{
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	for (MortarSound_e iSndIdx = (MortarSound_e)0; iSndIdx < MORTARSND_COUNT; iSndIdx = (MortarSound_e)(int(iSndIdx)+1))
	{
		_PlaySnd(iSndIdx, FALSE);
	}

	m_pWorldMesh->RemoveFromWorld();
	CBot::ClassHierarchyRemoveFromWorld();
}


void CBotMortar::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList)
{
	FASSERT( IsCreated() );
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );

	apTrackerSkipList[nTrackerSkipListCount++] = m_pWorldMesh;
}


void CBotMortar::ClassHierarchyWork()
{
	CFVec3A TempVec3A;

	FASSERT( m_bSystemInitialized );

	CBot::ClassHierarchyWork();

	if( !(m_pWorldMesh->GetVolumeFlags() & FVIS_VOLUME_IN_ACTIVE_LIST))
	{
		// We're not part of any active volume, therefore no workie;
		return;
	}

	if( !IsOurWorkBitSet() )
	{
		return;
	}

	Power_Work();

	switch( m_nPowerState ) 
	{
	case POWERSTATE_POWERED_UP:
		break;

	case POWERSTATE_POWERED_DOWN:
		_UpdateMatrices();
		return;

	case POWERSTATE_WAITING_FOR_IMMOBILIZED:
		break;

	case POWERSTATE_POWERING_DOWN:
		_PitchBaseNoise(FALSE);
		_UpdateMatrices();
		return;

	case POWERSTATE_POWERING_UP:
		_UpdateMatrices();
		return;

	case POWERSTATE_FINISHING_UP:
		_UpdateMatrices();
		return;

	default:
		FASSERT_NOW;
	}


	if( m_fMortarSecondsCountdownTimer > 0.0f ) 
	{
		m_fMortarSecondsCountdownTimer -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN(m_fMortarSecondsCountdownTimer, 0.0f);
	}

	if (m_nPossessionPlayerIndex >= 0)
	{
		ParseControls();
	}
	else
	{
		ParseControls();
		if ( (IsImmobileOrPending() == FALSE) && !IsDeadOrDying())
			_HandleMortarWork();
	}


	// Update position...
	TempVec3A.Mul( m_Velocity_WS, FLoop_fPreviousLoopSecs );
	m_MountPos_WS.Add( TempVec3A );
	
	// Handle pitch and yaw...
	HandlePitchMovement();
	HandleYawMovement();

	// Rotate model space velocity to account for yaw change...
	WS2MS( m_Velocity_MS, m_Velocity_WS );

	
	// Update translation analog stick vectors from raw controller data...
	ComputeXlatStickInfo();

	// Collide with the world below our feet...
	PROTRACK_BEGINBLOCK("Coll");
	HandleCollision();
	/*
	CFVec3A pos1 = m_CollSphere.m_Pos;
	fdraw_DevSphere(&pos1,m_CollSphere.m_fRadius);

	static CFVec3A pos2(0.0f,0.0f,0.0f);
	pos1.Add(pos2);
	static f32 frad = m_CollSphere.m_fRadius;
	CFColorRGBA red; red.OpaqueRed();
	fdraw_DevSphere(&pos1,frad,&red);
	*/
	PROTRACK_ENDBLOCK();//"Coll"

	// Move and animate our bot...
	switch( m_nState ) 
	{
	case STATE_GROUND:
		PROTRACK_BEGINBLOCK("GroundXlat");
			HandleGroundTranslation();
		PROTRACK_ENDBLOCK();//"GroundXlat");

		PROTRACK_BEGINBLOCK("GroundAnim");
			HandleGroundAnimations();
		PROTRACK_ENDBLOCK();//"GroundAnim");
		break;

	case STATE_AIR:
		PROTRACK_BEGINBLOCK("AirXlat");
			HandleAirTranslation();
		PROTRACK_ENDBLOCK();//"AirXlat");
		break;
	}

	PROTRACK_BEGINBLOCK("CommonAnim");
	HandleHipsAnimation();
	PROTRACK_ENDBLOCK();//"CommonAnim");

	_UpdateMatrices();

	if (IsDeadOrDying())
	{
		DeathWork();
	}
}


void CBotMortar::_UpdateMatrices( void )
{
	CFMtx43A FlipMtx, MeshMtx, NewMtx, EntityMtxToWorld;
	CFMtx43A::m_XlatRotY.SetRotationY( m_fMountYaw_WS + m_fLegsYaw_MS );
	CFMtx43A::m_XlatRotY.m_vPos = m_MountPos_WS;

	// Update xfm...
	m_pWorldMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_XlatRotY );

	m_pWorldMesh->UpdateTracker();

	PROTRACK_BEGINBLOCK("ComputeMtxPal");
		// Update bone matrix palette...
		m_pCollBot = this;
		m_Anim.m_pAnimCombiner->ComputeMtxPalette( TRUE );
		m_pCollBot = NULL;
	PROTRACK_ENDBLOCK();//"ComputeMtxPal");

	EntityMtxToWorld.Identity();
	EntityMtxToWorld.SetRotationYXZ( m_fMountYaw_WS, m_fMountPitch_WS, 0.0f );
	EntityMtxToWorld.m_vPos = m_MountPos_WS;
	Relocate_RotXlatFromUnitMtx_WS( &EntityMtxToWorld, TRUE, m_pMoveIdentifier );
}


void CBotMortar::_SetMortarState(MortarState_e eNewState)
{
	MortarState_e eOldState = m_eState;
	switch(eOldState)
	{
	case MS_SCANNING:
		break;
	case MS_TARGETING:
		break;
	case MS_FIRING:
		SetControlValue(ANIMCONTROL_RECOIL,0.0f);
		break;
	case MS_UNRECOILING:
		SetControlValue(ANIMCONTROL_UNRECOIL,0.0f);
		break;
	case MS_RETARGETING:
		break;
	default:
		FASSERT(!"Invalid state in Botmortar");
	}

	m_eState = eNewState;

	switch(eNewState)
	{
	case MS_SCANNING:
		break;
	case MS_TARGETING:
		break;
	case MS_FIRING:
		_FireProjectile();
		break;
	case MS_UNRECOILING:
		SetControlValue(ANIMCONTROL_UNRECOIL,1.0f);
		ZeroTime(ANIMTAP_UNRECOIL);
		break;
	case MS_RETARGETING:
		m_fRetargetTime = m_MortarInfo.fRetargetTime;
		_SetGotoCoordWS(m_TargetedPoint_WS);

		break;
	default:
		FASSERT(!"Invalid state in Botmortar");
	}
}

void CBotMortar::_HandleMortarWork(void)
{
	if (AIBrain())
	{
		AIBrain()->GetAIMover()->BeginFrame();
	}

	if (m_eState == MS_SCANNING)	// run through 360 line of sight test searching for targets
	{
//ftext_DebugPrintf(0.5f,0.4f,"SCANNING");
		// if not at home, take steps toward home
		_GotoCoordWS(FALSE);

		// create a search sphere at xyz radius from us to do tracker callbacks on 
		_SearchForTarget(m_MortarInfo.fScanVisionDistance);

		// if target was found, and we've crossed the center, go to next state, else not
		f32 fPreTickUnitTime = GetUnitTime(ANIMTAP_STAND);
		DeltaTime( ANIMTAP_STAND );
		f32 fPostTickUnitTime = GetUnitTime(ANIMTAP_STAND);
		BOOL bCrossedCenter = FALSE;
		if ((fPreTickUnitTime  < 0.5f) && (fPostTickUnitTime >= 0.5f))
			bCrossedCenter = TRUE;
		else if ((fPostTickUnitTime < 0.5f) && (fPreTickUnitTime >= 0.5f))
			bCrossedCenter = TRUE;

		if (m_pEnemyTarget != NULL && bCrossedCenter)
		{
			_SetMortarState(MS_TARGETING);
		}
	}
	else if (m_eState == MS_TARGETING)	// pitch, yaw, pause, prepare to fire, do box evasion if taking damage or if target is too close
	{
//ftext_DebugPrintf(0.5f,0.4f,"TARGETING");

		// tests line-of-sight to target
		_SearchForTarget(m_MortarInfo.fAlertVisionDistance);

		if (m_pEnemyTarget != NULL)
		{
			BOOL bTargetInSight = _HandleTargeting();

			// if we're locked on, and the counters done, go ahead and fire
            if (bTargetInSight && m_fMortarSecondsCountdownTimer==0.0f)
			{
				_SetMortarState(MS_FIRING);
			}
		}
		else
		{
			_SetMortarState(MS_RETARGETING);
		}
	}
	else if (m_eState == MS_FIRING)		// play through fire states
	{	
//ftext_DebugPrintf(0.5f,0.4f,"FIRING");

		if( !DeltaUnitTime(ANIMTAP_RECOIL, FLoop_fPreviousLoopSecs*m_MortarInfo.fOOFiringTime, TRUE) )
		{
		}
		else
		{
			_SetMortarState(MS_UNRECOILING);
		}
	}
	else if (m_eState == MS_UNRECOILING)	// play through fire states
	{
//ftext_DebugPrintf(0.5f,0.4f,"UNRECOILING");

		if( !DeltaUnitTime(ANIMTAP_UNRECOIL, FLoop_fPreviousLoopSecs*m_MortarInfo.fOOUnrecoilTime, TRUE) )
		{
		}
		else
		{
			_SetMortarState(MS_TARGETING);
		}
	}
	else if (m_eState == MS_RETARGETING)	// target lost, do box patrol looking for target
	{
//ftext_DebugPrintf(0.5f,0.4f,"RETARGETING");

		// Take steps some direction
		_GotoCoordWS(TRUE);

		// create a search sphere at xyz radius from us to do tracker callbacks on 
		_SearchForTarget(m_MortarInfo.fAlertVisionDistance);

		if (m_pEnemyTarget != NULL)
		{
			// aim the mortar toward the enemy
			_SetMortarState(MS_TARGETING);
		}
		else if (m_fRetargetTime < 0.0f) // retarget timed out, go back home
		{
			_SetMortarState(MS_SCANNING);
		}
		m_fRetargetTime -= FLoop_fPreviousLoopSecs;

	}
	else
	{
		FASSERT(!"Invalid state in Botmortar");
	}

	if (AIBrain())
	{
		AIBrain()->GetAIMover()->EndFrame();
	}
}

BOOL CBotMortar::_HandleTargeting(void)
{
	BOOL bOkayToFire=FALSE;
	FASSERT(m_pEnemyTarget);
	// aim the mortar toward the enemy
	f32 fStraightLineVel = ComputeCannonXZVel(m_pEnemyTarget->MtxToWorld()->m_vPos);
	f32 fDistXZ = m_MtxToWorld.m_vPos.DistXZ(m_pEnemyTarget->MtxToWorld()->m_vPos);
	f32 fTimeToTarget = fmath_Div(fDistXZ, fStraightLineVel);

	ComputeTargetsFutureLoc(m_TargetedPoint_WS, *m_pEnemyTarget,fTimeToTarget);

	CFVec3A vToTargetXZ;
	vToTargetXZ.x = m_TargetedPoint_WS.x - m_MtxToWorld.m_vPos.x; 
	vToTargetXZ.y = 0.f;
	vToTargetXZ.z = m_TargetedPoint_WS.z - m_MtxToWorld.m_vPos.z; 

	f32 fMagSq =  vToTargetXZ.MagSqXZ();
	if (fMagSq < 0.001f)
	{
		DEVPRINTF("CBotMortar::ComputeYawWSToPoint() bad imput values.\n");
		return bOkayToFire;
	}

	f32 fDesiredYaw_WS = fmath_Atan(vToTargetXZ.x,vToTargetXZ.z );
	f32 fOOMag = fmath_InvSqrt(fMagSq);

	m_TargetLockUnitVec_WS.x = vToTargetXZ.x * fOOMag;
	m_TargetLockUnitVec_WS.y = 0.f;
	m_TargetLockUnitVec_WS.z = vToTargetXZ.z * fOOMag;

	f32 fUnitMovement = m_fMountPitchClampedNormDelta + m_fMountYawClampedNormDelta + m_fClampedNormSpeedXZ_WS;
	FMATH_CLAMPMAX( fUnitMovement, 1.0f );
	f32 fTimeStep = FMATH_FPOT( fUnitMovement, 0.25f*FLoop_fPreviousLoopSecs, FLoop_fPreviousLoopSecs );

	// Compute aim delta yaw...
	f32 fCurrentYaw = m_fMountYaw_WS + m_fAimDeltaYaw_MS;
	if( fCurrentYaw > FMATH_PI ) 
	{
		do 
		{
			fCurrentYaw -= FMATH_2PI;
		} 
		while( fCurrentYaw > FMATH_PI );
	} 
	else if( fCurrentYaw < -FMATH_PI ) 
	{
		do 
		{
			fCurrentYaw += FMATH_2PI;
		} 
		while( fCurrentYaw < -FMATH_PI );
	}
	BOOL bMovingOurBarrels = FALSE;
	f32 fDesiredMinusCurrentYaw = fDesiredYaw_WS - fCurrentYaw;
	if (fmath_Abs(fDesiredMinusCurrentYaw) < m_MortarInfo.fRadiansMaxDeviate)
	{
		bOkayToFire = TRUE;
	}
	else
	{
		bMovingOurBarrels = TRUE;
	}

	f32 fAdjustedYaw_WS;
	if( fDesiredMinusCurrentYaw > 0.0f ) 
	{
		if( fDesiredMinusCurrentYaw >= FMATH_PI ) 
		{
			// Step is counter-clockwise...

			fDesiredYaw_WS -= FMATH_2PI;
			fAdjustedYaw_WS = fCurrentYaw;
			fAdjustedYaw_WS -= 2.0f * fTimeStep;
			FMATH_CLAMPMIN( fAdjustedYaw_WS, fDesiredYaw_WS );
			m_fAimDeltaYaw_MS += (fAdjustedYaw_WS - fCurrentYaw);
		}
		else 
		{
			// Step is clockwise...

			fAdjustedYaw_WS = fCurrentYaw;
			fAdjustedYaw_WS += 2.0f * fTimeStep;
			FMATH_CLAMPMAX( fAdjustedYaw_WS, fDesiredYaw_WS );
			m_fAimDeltaYaw_MS += (fAdjustedYaw_WS - fCurrentYaw);
		}
	} 
	else if( fDesiredMinusCurrentYaw < 0.0f )
	{
		if( fDesiredMinusCurrentYaw <= -FMATH_PI ) 
		{
			// Step is clockwise...

			fDesiredYaw_WS += FMATH_2PI;
			fAdjustedYaw_WS = fCurrentYaw;
			fAdjustedYaw_WS += 2.0f * fTimeStep;
			FMATH_CLAMPMAX( fAdjustedYaw_WS, fDesiredYaw_WS );
			m_fAimDeltaYaw_MS += (fAdjustedYaw_WS - fCurrentYaw);
		}
		else
		{
			// Step is counter-clockwise...

			fAdjustedYaw_WS = fCurrentYaw;
			fAdjustedYaw_WS -= 2.0f * fTimeStep;
			FMATH_CLAMPMIN( fAdjustedYaw_WS, fDesiredYaw_WS );
			m_fAimDeltaYaw_MS += (fAdjustedYaw_WS - fCurrentYaw);
		}
	}
	
	CFVec3A vPitchPt = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]->m_vPos;
	CFVec3A vVelocity;
	aiutils_ComputeTrajectoryFromXZVel(vVelocity,vPitchPt,m_TargetedPoint_WS,m_MortarInfo.fMinCannonballVelocityXZ);
	f32 fDesiredPitch_WS = -fmath_Atan( vVelocity.y, vVelocity.MagXZ() );

	// Compute aim pitch...
	if( fmath_Abs( fDesiredPitch_WS - m_fAimPitch_WS ) > FMATH_DEG2RAD(0.1f) ) 
	{
		bMovingOurBarrels = TRUE;

		if( fDesiredPitch_WS > m_fAimPitch_WS ) 
		{
			m_fAimPitch_WS += 2.0f * fTimeStep;
			FMATH_CLAMPMAX( m_fAimPitch_WS, fDesiredPitch_WS );
		}
		else 
		{
			m_fAimPitch_WS -= 2.0f * fTimeStep;
			FMATH_CLAMPMIN( m_fAimPitch_WS, fDesiredPitch_WS );
		}
	}

	if (bMovingOurBarrels == TRUE)
	{
		_PitchBaseNoise(TRUE,1.0f);
	}
	else
	{
		_PitchBaseNoise(TRUE, 0.0f);
	}
	// draw spotlight upon the targeted point
	CFVec3A vSpotLight;
	vSpotLight.x = m_TargetedPoint_WS.x;
	vSpotLight.y = m_TargetedPoint_WS.y + 10.f;
	vSpotLight.z = m_TargetedPoint_WS.z;

	f32 fLightRadius = 200.f;
	f32 fInnerCone = fmath_Atan(m_MortarInfo.fTargetRadius,10.f);
	f32 fOuterCone = fmath_Atan(fLightRadius,10.f);
	//f32 fLightRadius = fmath_Sqrt(25.0f*25.0f +m_MortarInfo.fTargetRadius*m_MortarInfo.fTargetRadius);
	
	m_WorldLight.m_Light.SetPositionAndRadius(&vSpotLight, fLightRadius );
	m_WorldLight.m_Light.SetSpotCone(fInnerCone,fOuterCone);
	m_WorldLight.m_Light.SetDirection(0.f,-1.0f,0.0f);
	m_WorldLight.m_Light.SetMotif(&FColor_MotifWhite);
	m_WorldLight.m_Light.SetColor(1.0f,0.0f,0.0f);
	m_WorldLight.m_Light.SetIntensity(2.0f);
	m_WorldLight.UpdateTracker();

//	fdraw_DevSphere(&vSpotLight,fLightRadius);
//	fdraw_DevSphere(&vSpotLight,.25f);

/*		
	CFSphere spLight;
	spLight.m_Pos.Set(vSpotLight);
	spLight.m_fRadius = 99.0f;

	m_WorldLight.UpdateTracker( 

*/
	return bOkayToFire;
/*
	
	CFVec3A startPt = m_MtxToWorld.m_vPos;
	CFVec3A endPt;
	endPt.Mul(m_TargetLockUnitVec_WS,100.f).Add(startPt);
	CFColorRGBA blue,cyan;blue.OpaqueBlue(),cyan.OpaqueCyan();
	fdraw_DevLine(&startPt,&endPt,&blue,&cyan);

	CFVec3A startPt2 = vPitchPt;
	CFVec3A endPt2;
	endPt2.Mul(vVelocity,100.f).Add(startPt);
	CFColorRGBA red,yellow;red.OpaqueRed(),yellow.OpaqueYellow();
	fdraw_DevLine(&startPt2,&endPt2,&yellow,&red);

	CFVec3A startPt = m_MtxToWorld.m_vPos;
	CFVec3A endPt;
	endPt.Mul(m_TargetLockUnitVec_WS,100.f).Add(startPt);
	CFColorRGBA red,yellow;red.OpaqueRed(),yellow.OpaqueYellow();
	fdraw_DevLine(&startPt,&endPt,&yellow,&red);

	CFColorRGBA blue,cyan;blue.OpaqueBlue(),cyan.OpaqueCyan();
	CFVec3A endPt2;
	endPt2.Mul(m_MountUnitFrontXZ_WS,100.f).Add(startPt);
	fdraw_DevLine(&startPt,&endPt2,&blue,&cyan);
*/
}

void CBotMortar::_FireProjectile(void)
{
	FExplosionSpawnParams_t SpawnParams; // moved to top, wasn't aligning?

	SetControlValue(ANIMCONTROL_RECOIL,1.0f);
	ZeroTime(ANIMTAP_RECOIL);

	m_fMortarSecondsCountdownTimer = m_MortarInfo.fOORoundsPerSec;

	CEProj *pProj = CEProjPool::GetProjectileFromFreePool( m_hMortarProjPool );
	if( pProj == NULL ) 
	{
		// No more projectiles...
		return;
	}
	CFVec3A vCannonDir;
	vCannonDir.Set(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]->m_vFront);
	// Let's fire a round...
	FASSERT(m_nBoneIndexPrimaryFire>=0);
    CFMtx43A ProjMtx;
//	ProjMtx.UnitMtxFromUnitVec(&vFireUnitDir);
	ProjMtx.Set(*m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]);
	
	pProj->Init();
	pProj->SetAmbientPropertiesFromSoundGroup(m_MortarInfo.pCannonBallInAir);
	pProj->SetDamager( NULL, this);
	
	FASSERT(m_pEnemyTarget);
	CFVec3A vEnemy = m_pEnemyTarget->MtxToWorld()->m_vPos;
	CFVec3A vSelf  = ProjMtx.m_vPos;
	f32 fStraightLineVel = ComputeCannonXZVel(vEnemy);
	f32 fDistXZ = vSelf.DistXZ(vEnemy);
	f32 fTimeToTarget = fmath_Div(fDistXZ, fStraightLineVel);

	ComputeTargetsFutureLoc(vEnemy, *m_pEnemyTarget,fTimeToTarget);

	CFVec3A vVelocity;
	f32 fTimeInAir = aiutils_ComputeTrajectoryFromXZVel(vVelocity,vSelf,vEnemy,fStraightLineVel,32.0f);

	CFVec3A vLeftTargetDirCapXZ,vRightTargetDirCapXZ;
	CFVec3A vLeftTargetDirCapXZPlus90,vRightTargetDirCapXZPlus90;
	vLeftTargetDirCapXZ.y = 0.0f;
	vRightTargetDirCapXZ = 0.0f;
	vLeftTargetDirCapXZPlus90.y=0.0f;
	vRightTargetDirCapXZPlus90.y=0.0f;

	f32 fSin, fCos;
	fmath_SinCos( -m_MortarInfo.fRadiansMaxDeviate, &fSin, &fCos );
	vLeftTargetDirCapXZ.x = vCannonDir.z*fSin + vCannonDir.x*fCos;
	vLeftTargetDirCapXZ.z = vCannonDir.z*fCos - vCannonDir.x*fSin;

	vRightTargetDirCapXZ.x = -vCannonDir.z*fSin + vCannonDir.x*fCos;
	vRightTargetDirCapXZ.z =  vCannonDir.z*fCos + vCannonDir.x*fSin;
/*
#if !FANG_PRODUCTION_BUILD
	CFVec3A vStart, vEnd;
	vStart = ProjMtx.m_vPos;
	vEnd.Mul(vLeftTargetDirCapXZ, 120.f).Add(vStart);
	CDebugDraw::Line(vStart, vEnd, &FColor_MotifRed,&FColor_MotifRed);
	vEnd.Mul(vRightTargetDirCapXZ, 120.f).Add(vStart);
	CDebugDraw::Line(vStart, vEnd, &FColor_MotifBlue,&FColor_MotifBlue);
#endif
*/
	vLeftTargetDirCapXZPlus90.x  = vLeftTargetDirCapXZ.z;
	vLeftTargetDirCapXZPlus90.z  = -vLeftTargetDirCapXZ.x;
	vRightTargetDirCapXZPlus90.x = vRightTargetDirCapXZ.z;
	vRightTargetDirCapXZPlus90.z = -vRightTargetDirCapXZ.x;

	f32 fCosTestLeft = vVelocity.Dot(vLeftTargetDirCapXZPlus90);
	if (fCosTestLeft < 0.0f) // out of cap left, set to left dir
	{
		vLeftTargetDirCapXZ.UnitizeXZ();
		vLeftTargetDirCapXZ.Mul(fStraightLineVel);
		vVelocity.x = vLeftTargetDirCapXZ.x;
		vVelocity.z = vLeftTargetDirCapXZ.z;
	}
	else
	{
		f32 fCosTestRight = vVelocity.Dot(vRightTargetDirCapXZPlus90);
		if (fCosTestRight > 0.0f) // out of cap right, set to right dir
		{
			vRightTargetDirCapXZ.UnitizeXZ();
			vRightTargetDirCapXZ.Mul(fStraightLineVel);
			vVelocity.x = vRightTargetDirCapXZ.x;
			vVelocity.z = vRightTargetDirCapXZ.z;
		}
	}
	
	CFVec3A vUnitVelocity;
	f32 fLinearVelocity = vUnitVelocity.SafeUnitAndMag(vVelocity);

	pProj->SetLinSpeed( fLinearVelocity );
	pProj->SetLinUnitDir_WS( &vUnitVelocity );

	pProj->Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &ProjMtx, 1.0f );
	pProj->SetMaxLifeSecs( 20.0f );
	pProj->SetMaxDistCanTravel( 1000.f );
	pProj->SetDetonateCallback( _ProjDetonateCallback );
	pProj->Launch();
	_PlaySnd(MORTARSND_FIRE,TRUE);

/*
	CFVec3A testpt;
	CFColorRGBA red; red.OpaqueRed();	
	testpt.Mul(vUnitVelocity, fLinearVelocity * fTimeInAir).Add(vSelf);
	testpt.y = vSelf.y + vUnitVelocity.y * fLinearVelocity * fTimeInAir - .5f * 32.f * fTimeInAir * fTimeInAir;
	fdraw_DevSphere(&testpt,1.0f,&red);
*/

//	pProj->SetSkipListCallback( _BuildProjSkipList );
//	pProj->SetDamageCallback( CEProj_Slower::DamageCallback );
//	pProj->SetAmbientSFXHandle(m_hSounds[STAFFSND_CHARGE_LOOP]);
//	pProj->SetAmbientRadius(m_aUserProps[m_nUpgradeLevel].fProjAudioRange);
//	pProj->SetDamageProfile( m_apDamageProfile[ m_nUpgradeLevel ] );
//	pProj->SetLightInit(&m_LightInit);
	// assign targets while trigger is pressed
//	pProj->SetTargetedEntity(GetTargetedEntity());

	SpawnParams.InitToDefaults();
	SpawnParams.pDamager= pProj->GetDamager();
	SpawnParams.Pos_WS.Set(ProjMtx.m_vPos);
	SpawnParams.UnitDir.Set(vUnitVelocity);
	SpawnParams.uFlags |= FEXPLOSION_SPAWN_EXPLOSION_FOLLOW_UNIT_DIR;

	// we'll probably need this handle later, save it off?	
	FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();
	if( hSpawner != FEXPLOSION_INVALID_HANDLE ) 
	{
		CExplosion2::SpawnExplosion( hSpawner, m_hExplLaunchGroup, &SpawnParams );
	}
}

BOOL CBotMortar::_ProjDetonateCallback( CEProj *pProj, BOOL bMakeEffect, CEProj::Event_e nEvent, const FCollImpact_t *pImpact ) 
{
	if (bMakeEffect)
	{
		if (nEvent==CEProj::EVENT_INTERCEPTED)
		{
			DEVPRINTF("HUH?  CBotMortar::_ProjDetonateCallback( void ).\n");
		}
		else if (nEvent==CEProj::EVENT_HIT_GEO)
		{
			FExplosionSpawnParams_t SpawnParams;
			SpawnParams.InitToDefaults();
			SpawnParams.pDamager= pProj->GetDamager();
			SpawnParams.Pos_WS  = pImpact->ImpactPoint;
			SpawnParams.UnitDir = pImpact->UnitFaceNormal;
			
			// we'll probably need this handle later, save it off?	
			FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();
			if( hSpawner != FEXPLOSION_INVALID_HANDLE ) 
			{
				CExplosion2::SpawnExplosion( hSpawner, m_hExplDetonateGroup, &SpawnParams );
			}
			CFXShockwave::AddShockwave( pImpact->ImpactPoint, 2.0f, 5.0f, ENTITY_BIT_BOT, m_MortarInfo.fMaxShockwaveImpulse, m_MortarInfo.fMinShockwaveImpulse );
		}
	}
	return FALSE;
}

void CBotMortar::CheckpointSaveSelect( s32 nCheckpoint )
{
	// then save self
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}

BOOL CBotMortar::CheckpointSave( void )
{
	// save parent class data
	CBot::CheckpointSave();

	BOOL bDeadMeshInWorld = m_DeadMesh.IsAddedToWorld();

	CFCheckPoint::SaveData( (u32&) bDeadMeshInWorld );

	return TRUE;
}

void CBotMortar::CheckpointRestore( void )
{
	// load parent class data
	CBot::CheckpointRestore();

	BOOL bDeadMeshInWorld;

	CFCheckPoint::LoadData( (u32&) bDeadMeshInWorld );

	if (bDeadMeshInWorld)
	{
		m_DeadMesh.UpdateTracker();
	}
	else
	{
		m_DeadMesh.RemoveFromWorld();
	}
}

void CBotMortar::ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS )
{
	FASSERT(pApproxMuzzlePoint_WS);
	*pApproxMuzzlePoint_WS = MtxToWorld()->m_vPos;
}


void CBotMortar::_PlaySnd(MortarSound_e eSnd, BOOL bPlay)
{
	if (eSnd >= MORTARSND_COUNT)
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

void CBotMortar::Die( BOOL bSpawnDeathEffects/*=TRUE*/, BOOL bSpawnGoodies )
{
	SetArmorModifier(1.0f); // max out the armour, so that no further damage occurs.
	m_WorldLight.RemoveFromWorld();
	CBot::Die(bSpawnDeathEffects, bSpawnGoodies);
	m_DeadMesh.m_Xfm.BuildFromMtx(m_pWorldMesh->m_Xfm.m_MtxF);
	m_DeadMesh.UpdateTracker();
}

void CBotMortar::_SearchForTarget(f32 fSearchSphereRadius)
{
	CFVec3A vAIEnemyLoc;
	BOOL bLOS=FALSE;
	u16 uSeenTimeStamp;
	CEntity* pAIReportedEnemy = ai_GetLastEnemySeen(AIBrain(),&vAIEnemyLoc,&bLOS,&uSeenTimeStamp);
	if (!bLOS)
	{
		m_fTimeSinceEnemyLOS += FLoop_fPreviousLoopSecs;
	}

	if ( (m_pEnemyTarget) &&								// if I've had an enemy
		 (m_pEnemyTarget->m_nPossessionPlayerIndex >= 0) &&	// who is a player
		 (m_fTimeSinceEnemyLOS < 0.25f) &&					// that I've seen within quarter second
		  !m_pEnemyTarget->IsDeadOrDying())					// who's still functional.
	{
		// pick this enemy again!
		return;
	}
	//	else
	m_pEnemyTarget = NULL;
	m_pEnemyTargetDistance2 = FMATH_MAX_FLOAT;

	if (!pAIReportedEnemy)
		return;

	m_pEnemyTargetDistance2 = m_MtxToWorld.m_vPos.DistSq(vAIEnemyLoc);
	if (bLOS)
	{
		m_fTimeSinceEnemyLOS = 0.0f;
		FASSERT( pAIReportedEnemy->TypeBits() & ENTITY_BIT_BOT);
		m_pEnemyTarget = (CBot*)pAIReportedEnemy;
	}
}
/*
void CBotMortar::_HandleAimAnimations(void)
{
	CFAnimFrame YawQuat, PitchQuat;

	f32 fYawNumber = m_fAimDeltaYaw_MS - m_fLegsYaw_MS;
	f32 fPitchNumber = m_fAimPitch_WS - m_fPitchAdjWS;

	FMATH_CLAMP(fPitchNumber, m_BotInfo_MountAim.fMountPitchDownLimit,m_BotInfo_MountAim.fMountPitchUpLimit);

	YawQuat.BuildQuat ( CFVec3A::m_UnitAxisY, fYawNumber, CFVec3A::m_Null );
	PitchQuat.BuildQuat( CFVec3A::m_UnitAxisX, fPitchNumber, CFVec3A::m_Null );

	// Apply pitch and yaw quaternions...
	m_AnimManFrameAim.UpdateFrame( BONE_SWIVEL, YawQuat );
	m_AnimManFrameAim.UpdateFrame( BONE_BARREL_BASE, PitchQuat );
}
*/
BOOL CBotMortar::_SetGotoCoordWS(const CFVec3A& rGoto)
{
	if (!m_pEBoxRange) // you can't move me
	{
		m_vGotoCoordWS.Set(m_vHomeCoordWS);
		return TRUE;
	}

	// Clip the goto pt to the extents we can travel within
	CFVec3A vMinWS,vMaxWS;
	vMinWS.Set(*m_pEBoxRange->CornerMin_WS());
	vMaxWS.Set(*m_pEBoxRange->CornerMax_WS());

	f32 fRadius = m_pWorldMesh->GetBoundingSphere().m_fRadius;
    f32 fRadiusOverRoot2 = fmath_Div(fRadius,FMATH_SQRT2);
	vMinWS.x += fRadiusOverRoot2;
	vMinWS.z += fRadiusOverRoot2;
	vMaxWS.x -= fRadiusOverRoot2;
	vMaxWS.z -= fRadiusOverRoot2;
	
	CFVec3A vAdjGotoPt;
	vAdjGotoPt.x = FMATH_MAX(vMinWS.x,rGoto.x);
	vAdjGotoPt.x = FMATH_MIN(vMaxWS.x,vAdjGotoPt.x);
	vAdjGotoPt.y = FMATH_MAX(vMinWS.y,rGoto.y);
	vAdjGotoPt.y = FMATH_MIN(vMaxWS.y,vAdjGotoPt.y);
	vAdjGotoPt.z = FMATH_MAX(vMinWS.z,rGoto.z);
	vAdjGotoPt.z = FMATH_MIN(vMaxWS.z,vAdjGotoPt.z);
	
	m_vGotoCoordWS.Set(vAdjGotoPt);

	return TRUE;
}

BOOL CBotMortar::_GotoCoordWS(BOOL bGoto)
{
	CFVec3A* pGotoCoord;
	if (bGoto)
		pGotoCoord = &m_vGotoCoordWS;
	else
		pGotoCoord = &m_vHomeCoordWS;

	CFVec3A vMeToGotoPt;
	vMeToGotoPt.Sub(*pGotoCoord,m_MtxToWorld.m_vPos);

	// Are we there? Return False if so.
	f32 fDistXZ = vMeToGotoPt.MagXZ();
	if ( (fDistXZ < 0.25f) || (m_pEBoxRange==NULL) )// non-walkers are always "there"
	{
		return FALSE;
	}
	CFVec3A vUnitMeToGotoPt;
	vUnitMeToGotoPt.Mul(vMeToGotoPt,fmath_Inv(fDistXZ));
	
	FASSERT(m_pEBoxRange->IsPointWithinBoxXZ(*pGotoCoord));
	

	// turn toward the vect, 

	// move toward the vect
	if (AIBrain())
	{
		AIBrain()->GetAIMover()->MoveToward(*pGotoCoord);

/*		if (this->m_fSpeedXZ_WS < 1.01f)
		{
			CFVec3A tmp;
			tmp = AIBrain()->GetAIMover()->GetTorsoLookAtXZ();
			tmp.Mul(15.0f);
			tmp.Add(MtxToWorld()->m_vPos);

			AIBrain()->GetAIMover()->FaceToward(tmp);
		}
*/
	}
	return TRUE;

}

void CBotMortar::_PitchBaseNoise(BOOL bOn, f32 fUnitVolume)
{
	if (bOn && !m_pSoundEmitters[MORTARSND_PITCH])
	{
		m_pSoundEmitters[MORTARSND_PITCH]=AllocAndPlaySound(m_pSounds[MORTARSND_PITCH]);
	}
	if (!bOn && m_pSoundEmitters[MORTARSND_PITCH])
	{
		m_pSoundEmitters[MORTARSND_PITCH]->Destroy();
		m_pSoundEmitters[MORTARSND_PITCH] = FALSE;
	}

	if (bOn)
	{
 		f32 fVolDelta = fUnitVolume - m_fRotateVolume;
		if (fVolDelta > 0.005f) // we want to increase our volume
			m_fRotateVolume += FLoop_fPreviousLoopSecs;
		if (fVolDelta < 0.005f) // we want to decrease our volume
			m_fRotateVolume -= FLoop_fPreviousLoopSecs;
		
		FMATH_CLAMP(m_fRotateVolume, .35f, 1.0f); 
		if( m_pSoundEmitters[MORTARSND_PITCH] )
		{
			m_pSoundEmitters[MORTARSND_PITCH]->SetVolume(m_fRotateVolume);
		}
	}
}

void CBotMortar::_AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx )
{
	CBotMortar* pBotMortar = ((CBotMortar*)m_pCollBot);
	
	if (pBotMortar->IsDeadOrDying())
		return;

	if ( nBoneIndex == CBotMortar::m_nBoneIndexSwivel ) 
	{
		f32 fYaw = pBotMortar->m_fAimDeltaYaw_MS - pBotMortar->m_fLegsYaw_MS;

		// Yaw
		CFQuatA SwivelQuat;
		SwivelQuat.BuildQuat( rBoneMtx.m_vUp, fYaw);
		
		CFMtx43A NewBoneMtx;
		NewBoneMtx.m_vPos = rBoneMtx.m_vPos;
		SwivelQuat.MulPoint( NewBoneMtx.m_vRight, rBoneMtx.m_vRight );
		SwivelQuat.MulPoint( NewBoneMtx.m_vUp, rBoneMtx.m_vUp );
		SwivelQuat.MulPoint( NewBoneMtx.m_vFront, rBoneMtx.m_vFront );
		
		rNewMtx.Mul( rParentMtx, NewBoneMtx );
	}
	else if ( nBoneIndex == CBotMortar::m_nBoneIndexBarrelBase ) 
	{
		FMATH_CLAMP(pBotMortar->m_fAimPitch_WS, m_BotInfo_MountAim.fMountPitchUpLimit, m_BotInfo_MountAim.fMountPitchDownLimit);
		f32 fBonesPitchWS = -fmath_Atan( rBoneMtx.m_vFront.y, rBoneMtx.m_vFront.MagXZ() );

		CFQuatA PitchQuat;
		PitchQuat.BuildQuat( rBoneMtx.m_vRight, pBotMortar->m_fAimPitch_WS - fBonesPitchWS);

		CFMtx43A NewBoneMtx;
		NewBoneMtx.m_vPos = rBoneMtx.m_vPos;
		PitchQuat.MulPoint( NewBoneMtx.m_vRight, rBoneMtx.m_vRight );
		PitchQuat.MulPoint( NewBoneMtx.m_vUp,	 rBoneMtx.m_vUp );
		PitchQuat.MulPoint( NewBoneMtx.m_vFront, rBoneMtx.m_vFront );

		rNewMtx.Mul( rParentMtx, NewBoneMtx );	
	}
	else
	{
		rNewMtx.Mul( rParentMtx, rBoneMtx );
	}
}