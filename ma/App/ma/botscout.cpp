//////////////////////////////////////////////////////////////////////////////////////
// botscout.cpp  Scout Bot
//
// Author: Chris MacDonald
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
// 02/25/03 MacDonald   Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fanim.h"
#include "fmesh.h"
#include "fresload.h"
#include "frenderer.h"
#include "fvtxpool.h"
#include "fsound.h"
#include "botscout.h"
#include "botanim.h"
#include "meshtypes.h"
#include "gamecam.h"
#include "eparticlepool.h"
#include "meshentity.h"

#define _BOTINFO_FILENAME					( "b_scout" )
#define _SCOUT_SOUND_BANK					( "Scout" )
#define _MESH_NAME							( "grmuscout00" )
#define _HEADLIGHT_TEXTURE_ID				( 1 )
#define _SCOUT_DAMAGE_PROFILE_NAME			( "Scout" )
#define _RIGHT_TREAD_TEXTURE_ID				( 1 )
#define _LEFT_TREAD_TEXTURE_ID				( 2 )

#define _TRANS_TEX_0_NAME					( "TRMUtrans01" )
#define _TRANS_TEX_1_NAME					( "TRMUtrans02" )
#define _TRANS_TEX_2_NAME					( "TRMUtrans03" )
#define _TRANS_TEX_FRAME_RATE				( 30.0f )			// rate at which transmission texture frames animate (frames/sec)
#define _TRANS_TEX_SCROLL_RATE				( 2.0f )			// rate at which transmission texture coords scroll (units/sec)
#define _MAX_SPEED							( 10.0f )
#define _TREAD_SPEED						( 3.0f )

#define _SURPRISE_FLASH_TIME				( 1.0f )
#define _SCAN_ANIM_UNIT_TIME				( 0.6f )
#define _SURPRISE_FLASH_CYCLE_RATE			( FMATH_DEG2RAD( 360.0f * 7.0f ) )

#define _ANTENNA_SEGMENTS					( 4 )
#define _ANTENNA_FREQUENCY					( 170.0f )
#define _ANTENNA_DAMPING					( 1.0f )
#define _ANTENNA_AMPLITUDE					( 25.0f )

#define  _TICK_TINT_R					( 0.0f )
#define  _TICK_TINT_G					( 0.5f )
#define  _TICK_TINT_B					( 0.5f )
#define _TICK_TINT_CYCLE_RATE			( FMATH_DEG2RAD( 360.0f * 1.0f ) )

static CBotScoutBuilder _BotScoutBuilder;
static CDamageProfile* pScoutDetDamageProfile = NULL;

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotScout
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL 								CBotScout::m_bSystemInitialized	= FALSE;
u32									CBotScout::m_nBotClassClientCount = 0;
u32									CBotScout::m_nBoneIndexHead;
static f32							_fSupriseHopImpulseMag = 25.0f;
CBotScout::BotInfo_Gen_t			CBotScout::m_BotInfo_Gen;
CBotScout::BotInfo_Walk_t			CBotScout::m_BotInfo_Walk;
CBotScout::BotInfo_Weapon_t			CBotScout::m_BotInfo_Weapon;	// required by AI 
CBotScout::BotInfo_Scout_t			CBotScout::m_BotInfo_Scout;

CBotScout*							CBotScout::m_pCBScout = NULL;

CBotAnimStackDef					CBotScout::m_AnimStackDef;

CFTexInst							CBotScout::m_TransmissionTexInst[CBotScout::NUM_TRANS_TEX];	// texture for transmission effect

FExplosion_GroupHandle_t			CBotScout::m_hExplosionGroup = FEXPLOSION_INVALID_HANDLE;

#define _NUM_TRANS_BEAMS	( 8 )
static CBotScout::TransBeam_t _aTransBeam[ _NUM_TRANS_BEAMS ];
static s32 _nTransBeam = 0;

// Static FNs

//-----------------------------------------------------------------------------
BOOL CBotScout::InitSystem( void )
{
	FASSERT( !m_bSystemInitialized );

	m_bSystemInitialized = TRUE;

	m_nBotClassClientCount = 0;

		
	return TRUE;
}


//-----------------------------------------------------------------------------
void CBotScout::UninitSystem( void )
{
	if( !m_bSystemInitialized )
	{
		return;
	}

	m_bSystemInitialized = FALSE;
}

//-----------------------------------------------------------------------------
BOOL CBotScout::InitLevel( void )
{
	_nTransBeam = 0;
	return TRUE;
}

//-----------------------------------------------------------------------------
void CBotScout::UninitLevel( void )
{

}


//-----------------------------------------------------------------------------
CBotScout::CBotScout() : CBot()
{
	m_pInventory	= NULL;
	m_pWorldMesh	= NULL;
}


//-----------------------------------------------------------------------------
CBotScout::~CBotScout()
{
	if( IsSystemInitialized() && IsCreated() )
	{
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}

//-----------------------------------------------------------------------------
BOOL CBotScout::Create( s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName )
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
	CBotScoutBuilder *pBuilder = (CBotScoutBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_BotScoutBuilder )
	{
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOTSCOUT );
	}

	// Create an entity...
	return CBot::Create( &m_BotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}


//-----------------------------------------------------------------------------
void CBotScout::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList)
{
	FASSERT( IsCreated() );
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );

	apTrackerSkipList[nTrackerSkipListCount++] = m_pWorldMesh;
}


//-----------------------------------------------------------------------------
const CFVec3A* CBotScout::GetTagPoint( u32 nTagPointIndex ) const
{
	FASSERT( IsCreated() );
	FASSERT( nTagPointIndex < m_nTagPointCount );

	return m_apTagPoint_WS[nTagPointIndex];
}

//-----------------------------------------------------------------------------
const CFVec3A* CBotScout::GetApproxEyePoint( void ) const
{
	FASSERT( IsCreated() );

	return m_pApproxEyePoint_WS;
}


//-----------------------------------------------------------------------------
f32 CBotScout::ComputeEstimatedControlledStopTimeXZ( void )
{
	return 0.1f;
}

//-----------------------------------------------------------------------------
f32 CBotScout::ComputeEstimatedControlledStopTimeY( void )
{
	return 0.1f;
}

//-----------------------------------------------------------------------------
void CBotScout::CheckpointSaveSelect( s32 nCheckpoint )
{
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}


//-----------------------------------------------------------------------------
void CBotScout::CheckpointRestore( void )
{
	m_uScoutState = SCOUT_STATE_IDLE;
	CBot::CheckpointRestore();
	u32 uTmp;
	CFCheckPoint::LoadData(uTmp);	//only certain flags were saved
	m_uScoutFlags &=~(SCOUT_FLAG_SHOULD_SCAN | SCOUT_FLAG_DAMAGE_TAKEN);
	m_uScoutFlags |= uTmp;

	CFCheckPoint::LoadData(m_fTickRate);
}


//-----------------------------------------------------------------------------
BOOL CBotScout::CheckpointSave( void )
{
	if (CBot::CheckpointSave())
	{
		CFCheckPoint::SaveData(m_uScoutFlags & (SCOUT_FLAG_DAMAGE_TAKEN | SCOUT_FLAG_SHOULD_SCAN));
		CFCheckPoint::SaveData(m_fTickRate);
		return TRUE;
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
void CBotScout::DrawBeams( void )
{
	s32 nIndex;

	if( _nTransBeam >= _NUM_TRANS_BEAMS ) _nTransBeam = _NUM_TRANS_BEAMS - 1;
	for( nIndex = 0; nIndex < _nTransBeam; nIndex++ )
	{
		DrawTransmissionBeam(	_aTransBeam[nIndex].vStart, 
								_aTransBeam[nIndex].vEnd, 
								_aTransBeam[nIndex].vColor, 
								_aTransBeam[nIndex].fTexFrame, 
								_aTransBeam[nIndex].fTexScroll,
								&(m_TransmissionTexInst[(s32) _aTransBeam[nIndex].fTexFrame]));

	}
	_nTransBeam = 0;
}


//-----------------------------------------------------------------------------
void CBotScout::InflictDamageResult( const CDamageResult *pDamageResult )
{
	//return;
	CBot::InflictDamageResult( pDamageResult );

	if( pDamageResult->m_fImpulseMag == 0.0f )
	{
		return;
	}

	CFVec3A vMomentum, vPoint, vPerp;

	vPoint.Set( pDamageResult->m_pDamageData->m_ImpactPoint_WS );
	vPoint.Sub( m_MtxToWorld.m_vPos );

	// calculate a y rotation amount for this damage infliction
	vPerp.x = vPoint.z;
	vPerp.y = 0.0f;
	vPerp.z = -vPoint.x;
	m_fDamageRot = vPerp.Dot( pDamageResult->m_Impulse_WS ) * 0.0002f;

	vMomentum.Set( pDamageResult->m_Impulse_WS );
	vMomentum.Mul( 0.005f );
	ApplyVelocityImpulse_WS( vMomentum );

	if( vMomentum.y > 5.0f )
	{
		m_uScoutFlags |= SCOUT_FLAG_TUMBLE;
	}
	if (pDamageResult->m_fDeltaHitpoints > 0.0f)
	{
		m_uScoutFlags |= SCOUT_FLAG_DAMAGE_TAKEN;
	}

//	m_uScoutState = SCOUT_STATE_START_LOWERING_SCANNER;
}


//-----------------------------------------------------------------------------
void CBotScout::ActivateBeam( CFVec3A &rvBeamEnd, BOOL bScoutAtSwitch, f32 fPctSwitchPressComplete)		// must be called every frame that transmission beam should be drawn
{
	m_uScoutFlags |= SCOUT_FLAG_DRAW_BEAM | (bScoutAtSwitch*SCOUT_FLAG_AT_SWITCH);
	m_fPctSwitchPressComplete = fPctSwitchPressComplete;

								
	m_vTransBeamEnd.Set( rvBeamEnd );
}


//-----------------------------------------------------------------------------
void CBotScout::TargetSighted( f32 fDistToTarget )
{
//	m_uScoutState = SCOUT_STATE_SURPRISE;
	if( m_uScoutState == SCOUT_STATE_SCANNING )
	{
		m_uScoutFlags |= SCOUT_FLAG_TARGET_SIGHTED;
	}
	SetScanning( FALSE );
	m_fDistToTarget = fDistToTarget;
}


//-----------------------------------------------------------------------------
BOOL CBotScout::IsSurprised( void )
{
	switch( m_uScoutState )
	{
		case SCOUT_STATE_SURPRISE:
		case SCOUT_STATE_SURPRISE_JUMP:
		case SCOUT_STATE_SURPRISE_LAND:
		case SCOUT_STATE_SURPRISE_FLASH:
			return TRUE;
	}

	return FALSE;
}

void CBotScout::SetTickRate(f32 fTicksPerSec)
{
	this->m_fTickRate = fTicksPerSec;
}

void CBotScout::Die( BOOL bSpawnDeathEffects/*=TRUE*/, BOOL bSpawnGoodies )
{
	CBot::Die( bSpawnDeathEffects, bSpawnGoodies );

	if( !bSpawnDeathEffects ) {
		return;
	}

	if( m_hExplosionGroup == FEXPLOSION_INVALID_HANDLE )
	{
		return;
	}

	FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();
	if( hSpawner != FEXPLOSION_INVALID_HANDLE )
	{
		FExplosionSpawnParams_t SpawnParams;

		SpawnParams.InitToDefaults();

		SpawnParams.uFlags = FEXPLOSION_SPAWN_NONE;
		SpawnParams.Pos_WS = m_MtxToWorld.m_vPos;
		SpawnParams.UnitDir = m_MtxToWorld.m_vUp;
		SpawnParams.uSurfaceType = 0;
		SpawnParams.pDamageProfile = GetDamageProfile();
		SpawnParams.pDamager = NULL;

		CExplosion2::SpawnExplosion( hSpawner, m_hExplosionGroup, &SpawnParams );
	}
}

//-----------------------------------------------------------------------------
// Protected Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
BOOL CBotScout::ClassHierarchyLoadSharedResources( void )
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

	// Resources not yet loaded...
	FResFrame_t frame = fres_GetFrame();

	if( !_BuildAnimStackDef() )
	{
		DEVPRINTF( "CBotScout Load Shared Resources:  Error creating bot anim stack def\n" );
		goto _ExitWithError;
	}

	m_TransmissionTexInst[0].SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, _TRANS_TEX_0_NAME ) );
	if( m_TransmissionTexInst[0].GetTexDef() == NULL )
	{
		DEVPRINTF( "CBotScout Load Shared Resources: Could not load transmission texture %s.\n", _TRANS_TEX_0_NAME );
		goto _ExitWithError;
	}

	m_TransmissionTexInst[1].SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, _TRANS_TEX_1_NAME ) );
	if( m_TransmissionTexInst[1].GetTexDef() == NULL )
	{
		DEVPRINTF( "CBotScout Load Shared Resources: Could not load transmission texture %s.\n", _TRANS_TEX_1_NAME );
		goto _ExitWithError;
	}

	m_TransmissionTexInst[2].SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, _TRANS_TEX_2_NAME ) );
	if( m_TransmissionTexInst[2].GetTexDef() == NULL )
	{
		DEVPRINTF( "CBotScout Load Shared Resources: Could not load transmission texture %s.\n", _TRANS_TEX_2_NAME );
		goto _ExitWithError;
	}

	if( !fresload_Load( FSNDFX_RESTYPE, _SCOUT_SOUND_BANK ) ) 
	{
		DEVPRINTF( "CBotScout Load Shared Resources: Could not load sound effect bank '%s'\n", _SCOUT_SOUND_BANK );
	}

	if( !ReadBotInfoFile( m_aGameDataMap, _BOTINFO_FILENAME ) )
	{
		DEVPRINTF( "CBotScout Load Shared Resources:  Error parsing %s\n", _BOTINFO_FILENAME );
		goto _ExitWithError;
	}

	m_hExplosionGroup = CExplosion2::GetExplosionGroup( "Scout" );
	if ( m_hExplosionGroup == FEXPLOSION_INVALID_HANDLE )
	{
		DEVPRINTF( "CBotScout Load Shared Resources:  Could not find Explosion Group 'Scout'\n" );
	}

	return TRUE;

	// Error:
_ExitWithError:
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( frame );

	return FALSE;
}


//-----------------------------------------------------------------------------
void CBotScout::ClassHierarchyUnloadSharedResources( void )
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


//-----------------------------------------------------------------------------
BOOL CBotScout::ClassHierarchyBuild( void )
{
	FMesh_t		*pMesh;
	FMeshInit_t	meshInit;
	FResFrame_t	frame;
	u32 i;
	s32 nBoneIndex;

	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	frame = fres_GetFrame();

	// get pointer to leaf class's builder object...
	CBotScoutBuilder *pBuilder = (CBotScoutBuilder*)GetLeafClassBuilder();

	// set input params for CBot creation...
	pBuilder->m_pBotDef = &m_BotDef;

	// build parent...
	if( !CBot::ClassHierarchyBuild() )
	{
		goto _ExitWithError;
	}

	// set defaults...
	_ClearDataMembers();

	// init from builder...
	// nothing yet

	// load mesh...
	pMesh = (FMesh_t*)fresload_Load( FMESH_RESTYPE, _MESH_NAME );
	if( pMesh == NULL )
	{
		DEVPRINTF( "CBotScout::ClassHierarchyBuild(): Could not find mesh '%s'\n", _MESH_NAME );
		goto _ExitWithError;
	}

	// create worldmesh...
	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL )
	{
		DEVPRINTF( "CBotScout::ClassHierarchyBuild(): Unable to allocate CFWorldMesh\n" );
		goto _ExitWithError;
	}

	// init worldmesh...
	meshInit.pMesh		= pMesh;
	meshInit.fCullDist	= FMATH_MAX_FLOAT;
	meshInit.Mtx.Set( pBuilder->m_EC_Mtx_WS );
	meshInit.nFlags		= 0;
	m_pWorldMesh->Init( &meshInit );

	m_pWorldMesh->m_nUser	= MESHTYPES_ENTITY;
	m_pWorldMesh->m_pUser	= this;
	m_pWorldMesh->SetUserTypeBits( TypeBits() );
	m_pWorldMesh->UpdateTracker();
	m_pWorldMesh->RemoveFromWorld();


	// build animation stack...
	if( !m_Anim.Create( &m_AnimStackDef, m_pWorldMesh ) )
	{
		DEVPRINTF( "CBotScout::ClassHierarchyBuild(): Unable to create animation stack\n" );
		goto _ExitWithError;
	}

	m_Anim.m_pAnimCombiner->DisableAllBoneCallbacks();

	// Find tag points...
	for( i=0, m_nTagPointCount=0; i<TAG_POINT_COUNT; i++ )
	{
		nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_anTagPointBoneNameIndexArray[i] ] );
 
		if( nBoneIndex < 0 )
		{
			DEVPRINTF( "CBotScout::ClassHierarchyBuild(): Could not locate TagPoint bone '%s'.\n", m_apszBoneNameTable[ m_anTagPointBoneNameIndexArray[i] ] );
		}
		else
		{
			m_apTagPoint_WS[m_nTagPointCount++] = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
		}
	}

	if( m_nTagPointCount == 0 )
	{
		// No tag points. Use entity origin...
		m_nTagPointCount = 1;
		m_apTagPoint_WS[0] = &m_MtxToWorld.m_vPos;
	}

	// Find approx eye point...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
	if( nBoneIndex < 0 )
	{
		DEVPRINTF( "CBotScout::ClassHierarchyBuild(): Could not locate approx eye point bone '%s'.\n", m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
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
		DEVPRINTF( "CBotScout::ClassHierarchyBuild(): Could not locate gaze bone '%s'.\n", m_apszBoneNameTable[ BONE_HEAD ] );
		m_pGazeDir_WS = &m_MtxToWorld.m_vFront;
	}
	else
	{
		m_pGazeDir_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vFront;
	}

	// Find torso mtx.  Head is torso on scout.
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_HEAD ] );
	if( nBoneIndex < 0 )
	{
		DEVPRINTF( "CBotScout::ClassHierarchyBuild(): Could not locate torso bone '%s'.\n", m_apszBoneNameTable[ nBoneIndex ] );
		m_pAISteerMtx = &m_MtxToWorld;
	}
	else
	{
		m_pAISteerMtx = m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex];
	}

	// initialize mtx palette...
	AtRestMatrixPalette();

	m_pSpotLight = m_pWorldMesh->GetAttachedLightByID( _HEADLIGHT_TEXTURE_ID );
	SetSpotLightOff();

	SetArmorProfile( CDamage::FindArmorProfile( _SCOUT_DAMAGE_PROFILE_NAME ) );

	m_hTreadTexLayerR = m_pWorldMesh->GetTexLayerHandle( _RIGHT_TREAD_TEXTURE_ID );
	if( m_hTreadTexLayerR == FMESH_TEXLAYERHANDLE_INVALID )
	{
		DEVPRINTF( "CBotScout::ClassHierarchyBuild():  Unable to get tex layer handle for right Tread\n" );
	}
	else
	{
		m_pWorldMesh->AnimTC_AnimateScroll( m_hTreadTexLayerR, FALSE );
		m_pWorldMesh->LayerAlpha_Set( m_hTreadTexLayerR, 1.0f );
	}

	m_hTreadTexLayerL = m_pWorldMesh->GetTexLayerHandle( _LEFT_TREAD_TEXTURE_ID );
	if( m_hTreadTexLayerL == FMESH_TEXLAYERHANDLE_INVALID )
	{
		DEVPRINTF( "CBotScout::ClassHierarchyBuild():  Unable to get tex layer handle for left Tread\n" );
	}
	else
	{
		m_pWorldMesh->AnimTC_AnimateScroll( m_hTreadTexLayerL, FALSE );
		m_pWorldMesh->LayerAlpha_Set( m_hTreadTexLayerL, 1.0f );
	}

	// set up anim bone callback
	m_nBoneIndexHead = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_HEAD ] );
	m_Anim.m_pAnimCombiner->SetBoneCallback( &_AnimBoneCallback );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_HEAD ] );

	// set up antenna
	{
		s32 anBones[_ANTENNA_SEGMENTS];
		u32 uIndex;
		m_Antenna.Create( m_pWorldMesh, _ANTENNA_SEGMENTS, _ANTENNA_FREQUENCY, _ANTENNA_DAMPING, _ANTENNA_AMPLITUDE );
		anBones[0] = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ANTENNA1] );
		anBones[1] = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ANTENNA2] );
		anBones[2] = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ANTENNA3] );
		anBones[3] = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ANTENNA4] );
		for( uIndex = 0; uIndex < _ANTENNA_SEGMENTS; uIndex++ )
		{
			if( anBones[uIndex] < 0 )
			{
				DEVPRINTF( "CBotScout::ClassHierarchyBuild: Antenna Bone %d doesn't exist in object '%s'.\n", uIndex, Name() );
			}
		}

		m_Antenna.RegisterBones( anBones );
	}
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ANTENNA1] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ANTENNA2] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ANTENNA3] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ANTENNA4] );

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( frame );
	return FALSE;

}


//-----------------------------------------------------------------------------
void CBotScout::ClassHierarchyDestroy( void )
{
	m_Anim.Destroy();

	if( m_pWorldMesh )
	{
		fdelete( m_pWorldMesh );
		m_pWorldMesh = NULL;
	}

	if( m_pScanConeMeshEntity )
	{
		fdelete( m_pScanConeMeshEntity );
		m_pScanConeMeshEntity = NULL;
	}

	CBot::ClassHierarchyDestroy();
}


//-----------------------------------------------------------------------------
BOOL CBotScout::ClassHierarchyBuilt( void )
{
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t frame = fres_GetFrame();

	if( !CBot::ClassHierarchyBuilt() )
	{
		goto _ExitWithError;
	}

	EnableOurWorkBit();

	if( LoadAuxMeshEntity( &m_pScanConeMeshEntity, m_pBotInfo_Scout->apszScanConeMeshName, 1, "ScanCone" ) )
	{
		m_pScanConeMeshEntity->Attach_UnitMtxToParent_PS( this, m_apszBoneNameTable[ BONE_ATTACHPOINT_BEAM ] );
		m_pScanConeMeshEntity->AddToWorld();
		m_pScanConeMeshEntity->SelectMesh( 0 );
		m_pScanConeMeshEntity->GetMesh()->m_nFlags |= FMESHINST_FLAG_POSTER_Y;
		m_pScanConeMeshEntity->GetMesh()->SetMeshAlpha( 0.0f );
	}

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( frame );
	return FALSE;
}


//-----------------------------------------------------------------------------
CEntityBuilder* CBotScout::GetLeafClassBuilder( void )
{
	return &_BotScoutBuilder;
}


//-----------------------------------------------------------------------------
void CBotScout::ClassHierarchyAddToWorld( void )
{
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CBot::ClassHierarchyAddToWorld();

	m_uScoutFlags = 0;   //pgm: just incase bot is re-used
	SetScanning( TRUE );

	if( m_pScanConeMeshEntity )
	{
		m_pScanConeMeshEntity->Attach_UnitMtxToParent_PS( this, m_apszBoneNameTable[ BONE_ATTACHPOINT_BEAM ] );
		m_pScanConeMeshEntity->AddToWorld();
		m_pScanConeMeshEntity->GetMesh()->m_nFlags |= FMESHINST_FLAG_POSTER_Y;
		m_pScanConeMeshEntity->GetMesh()->SetMeshAlpha( 0.0f );
	}

	FASSERT( m_pAudioEmitterTreads == NULL );
	if( !m_pAudioEmitterTreads )
	{
		m_pAudioEmitterTreads = AllocAndPlaySound( m_BotInfo_Scout.pSoundGroupTreads );
		if( m_pAudioEmitterTreads )
		{
			m_pAudioEmitterTreads->SetVolume( 0.0f );
		}
	}

	// snap antenna to new position
	m_Antenna.SetPos();
	AtRestMatrixPalette();
	m_fPctSwitchPressComplete  = 0.0f;
	m_pWorldMesh->SetMeshTint(1.0f, 1.0f, 1.0f);
}


//-----------------------------------------------------------------------------
void CBotScout::ClassHierarchyRemoveFromWorld( void )
{
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	m_pWorldMesh->RemoveFromWorld();

	// added IsCreated() because particles are destroyed on level exit, before this function is called
	if( m_pEParticleSmoke && m_pEParticleSmoke->IsCreated() )	
	{
		m_pEParticleSmoke->StopEmission();
		m_pEParticleSmoke = NULL;
	}

	if( m_pScanConeMeshEntity )
	{
		m_pScanConeMeshEntity->RemoveFromWorld();
	}

	if( m_pAudioEmitterScanning )
	{
		m_pAudioEmitterScanning->Destroy();
		m_pAudioEmitterScanning = NULL;
	}

	if( m_pAudioEmitterTreads )
	{
		m_pAudioEmitterTreads->Destroy();
		m_pAudioEmitterTreads = NULL;
	}

	if( m_pAudioEmitterBeam )
	{
		m_pAudioEmitterBeam->Destroy();
		m_pAudioEmitterBeam = NULL;
	}

	CBot::ClassHierarchyRemoveFromWorld();
}


//-----------------------------------------------------------------------------
void CBotScout::ClassHierarchyWork( void )
{
	CFVec3A vTemp;
	f32 fUnitTime;
	FASSERT( IsSystemInitialized() );

	CBot::ClassHierarchyWork();

	if( !IsOurWorkBitSet() )
	{
		return;
	}

	UpdateSpotLight();

	ParseControls();

	if( !IsSurprised() )
	{
		vTemp.Set( m_ControlsBot_XlatNormSpeedXZ_WS );
		vTemp.Mul( m_fMaxFlatSurfaceSpeed_WS );
		vTemp.Mul( 1.0f - (1.0f - ComputeUnitHealth()) * m_pBotInfo_Scout->fDamageSlowdownAmount );
		m_Velocity_WS.x = vTemp.x;
		m_Velocity_WS.z = vTemp.z;
	}

	// save a copy of previous frame info...
	m_MountPrevPos_WS	= m_MountPos_WS;
	m_nPrevState		= m_nState;


	HandleVelocityImpulses();

	WS2MS( m_Velocity_MS, m_Velocity_WS );

	// update position...
	vTemp.Mul( m_Velocity_WS, FLoop_fPreviousLoopSecs );
	m_MountPos_WS.Add( vTemp );

	// Collide with the world below our feet...
	HandleCollision();

	// Move and animate our bot...
	switch( m_nState )
	{
		case STATE_GROUND:
			HandleGroundTranslation();
			if( m_pAudioEmitterTreads )
			{
				f32 fVolume = m_fClampedNormSpeedAlongSurface_WS;
				if( fVolume > 0.0f )
				{
					FMATH_CLAMPMIN( fVolume, 0.3f );
				}
				m_pAudioEmitterTreads->SetVolume( fVolume );
				CFSoundInfo *pSoundInfo;
				pSoundInfo = CFSoundGroup::GetSoundInfo( m_pBotInfo_Scout->pSoundGroupTreads, 0 );

				if( pSoundInfo )
				{
					f32 fMinFrequencyFactor = ((f32)pSoundInfo->m_nMinPitchPercent * 0.01f);
					f32 fFrequencyFactor = ((f32)pSoundInfo->m_nMaxPitchPercent * 0.01f) - fMinFrequencyFactor;
					fFrequencyFactor *= m_fClampedNormSpeedAlongSurface_WS;
					fFrequencyFactor += fMinFrequencyFactor;
					m_pAudioEmitterTreads->SetFrequencyFactor( fFrequencyFactor );
				}

				m_pAudioEmitterTreads->SetPosition( &m_MtxToWorld.m_vPos );
			}

			break;

		case STATE_AIR:
			HandleAirTranslation();
			if( m_pAudioEmitterTreads )
			{
				m_pAudioEmitterTreads->SetVolume( 0.0f );
			}

			break;
	}

	m_pCBScout = this;
	_UpdateMatrices();
	m_pCBScout = NULL;

	if( IsDeadOrDying() )
	{
		DeathWork();
	}

	switch( m_uScoutState )
	{
		//---------------------------------------------------------------------
		case SCOUT_STATE_IDLE:
			if( m_uScoutFlags & SCOUT_FLAG_SHOULD_SCAN )
			{
				SetControlValue( ANIMCONTROL_HEADUP, 1.0f );
				SetSpotLightOn();
				PlaySound( m_BotInfo_Scout.pSoundGroupServo1 );
				m_uScoutFlags &= ~SCOUT_FLAG_TARGET_SIGHTED;
				m_uScoutState = SCOUT_STATE_RAISING_SCANNER;
			}
			break;

		//---------------------------------------------------------------------
		case SCOUT_STATE_RAISING_SCANNER:
			DeltaTime( ANIMTAP_HEADUP );
			fUnitTime = GetUnitTime( ANIMTAP_HEADUP );
			if( fUnitTime > _SCAN_ANIM_UNIT_TIME )
			{
				UpdateUnitTime( ANIMTAP_HEADUP, _SCAN_ANIM_UNIT_TIME );
				m_uScoutState = SCOUT_STATE_SCANNING;

				FASSERT( m_pAudioEmitterScanning == NULL );
				if( !m_pAudioEmitterScanning )
				{
					m_pAudioEmitterScanning = AllocAndPlaySound( m_BotInfo_Scout.pSoundGroupScanning );
				}

				if( m_pScanConeMeshEntity )
				{
					// scan cone on 100%
					m_pScanConeMeshEntity->GetMesh()->SetMeshAlpha( 1.0f );
				}
			}
			else
			{
				if( m_pScanConeMeshEntity )
				{
					// fade in scan cone
					FMATH_CLAMP_UNIT_FLOAT( fUnitTime );
					m_pScanConeMeshEntity->GetMesh()->SetMeshAlpha( fUnitTime );
				}
			}
			break;

		//---------------------------------------------------------------------
		case SCOUT_STATE_SCANNING:
			if( m_pScanConeMeshEntity )
			{
				// animate scan cone entity scale
				m_fScanConeScaleAngle += FLoop_fPreviousLoopSecs * m_pBotInfo_Scout->fScanConeScaleFrequency * FMATH_DEG2RAD( 360.0f );
				if( m_fScanConeScaleAngle > FMATH_DEG2RAD( 360.0f ) )
				{
					m_fScanConeScaleAngle -= FMATH_DEG2RAD( 360.0f );
				}
				m_pScanConeMeshEntity->Relocate_Scale_WS( m_pBotInfo_Scout->fScanConeMinScale + fmath_PositiveCos( m_fScanConeScaleAngle ) * m_pBotInfo_Scout->fScanConeScaleRange );

				// animate spotlight intensity
				if( m_pSpotLight )
				{
					f32 fIntensity = 1.0f - ( fmath_Div( m_fScanConeScaleAngle, FMATH_DEG2RAD( 360.0f ) ) * m_pBotInfo_Scout->fScanConeLightFlicker );
					FMATH_CLAMP_UNIT_FLOAT( fIntensity );
						
					m_pSpotLight->m_Light.SetIntensity( fIntensity );
				}
			}

			if( m_pAudioEmitterScanning )
			{
				m_pAudioEmitterScanning->SetPosition( &m_MtxToWorld.m_vPos );
			}

			if(!( m_uScoutFlags & SCOUT_FLAG_SHOULD_SCAN ) ||
				(m_nState != STATE_GROUND && m_nPrevState == STATE_GROUND ) )
			{
				m_uScoutState = SCOUT_STATE_START_LOWERING_SCANNER;
			}

			if( m_uScoutFlags & SCOUT_FLAG_TARGET_SIGHTED )
			{
				m_uScoutFlags &= ~SCOUT_FLAG_TARGET_SIGHTED;
				m_uScoutState = SCOUT_STATE_START_SURPRISE;
			}
			break;

		//---------------------------------------------------------------------
		case SCOUT_STATE_START_LOWERING_SCANNER:
			SetSpotLightOff();
			PlaySound( m_BotInfo_Scout.pSoundGroupServo2 );
			m_uScoutState = SCOUT_STATE_LOWERING_SCANNER;
			break;

		//---------------------------------------------------------------------
		case SCOUT_STATE_LOWERING_SCANNER:
			if( DeltaTime( ANIMTAP_HEADUP, -FLoop_fPreviousLoopSecs, TRUE ) )
			{
				m_uScoutState = SCOUT_STATE_IDLE;

				if( m_pAudioEmitterScanning )
				{
					m_pAudioEmitterScanning->Destroy();
					m_pAudioEmitterScanning = NULL;
				}

				if( m_pScanConeMeshEntity )
				{
					// turn off scan cone
					m_pScanConeMeshEntity->GetMesh()->SetMeshAlpha( 0.0f );
				}
			}
			else
			{
				if( m_pScanConeMeshEntity )
				{
					// fade out scan cone
					fUnitTime = GetUnitTime( ANIMTAP_HEADUP );
					FMATH_CLAMP_UNIT_FLOAT( fUnitTime );
					m_pScanConeMeshEntity->GetMesh()->SetMeshAlpha( fUnitTime );
				}
			}
			break;

		//---------------------------------------------------------------------
		case SCOUT_STATE_START_SURPRISE:
			m_uScoutState = SCOUT_STATE_SURPRISE;
		case SCOUT_STATE_SURPRISE:
			if( DeltaTime( ANIMTAP_HEADUP, 2.0f * FLoop_fPreviousLoopSecs, TRUE ) )
			{
				m_uScoutState = SCOUT_STATE_SURPRISE_JUMP;
			}
			break;

		//---------------------------------------------------------------------
		case SCOUT_STATE_SURPRISE_JUMP:
			vTemp.Set( 0.0f, _fSupriseHopImpulseMag, 0.0f );
			ApplyVelocityImpulse_WS( vTemp );
			PlaySound( m_BotInfo_Scout.pSoundGroupAlertChirp, 1.0f,1.0f, m_fDistToTarget + 30.0f );
			m_uScoutState = SCOUT_STATE_SURPRISE_LAND;
			break;

		//---------------------------------------------------------------------
		case SCOUT_STATE_SURPRISE_LAND:
			if( m_nState == STATE_GROUND )
			{
				m_uScoutState = SCOUT_STATE_SURPRISE_FLASH;
				m_fSurpriseTimer = _SURPRISE_FLASH_TIME;
			}
			break;

		//---------------------------------------------------------------------
		case SCOUT_STATE_SURPRISE_FLASH:
			m_fSurpriseTimer -= FLoop_fPreviousLoopSecs;

			if( m_fSurpriseTimer > 0.0f )
			{
				f32 fCycle = fmath_PositiveCos( m_fSurpriseTimer * _SURPRISE_FLASH_CYCLE_RATE );
				if( fCycle > 0.5f )
				{
					m_pScanConeMeshEntity->GetMesh()->SetMeshAlpha( 1.0f );
					m_pSpotLight->m_Light.SetIntensity( 1.0f );
				}
				else
				{
					m_pScanConeMeshEntity->GetMesh()->SetMeshAlpha( 0.0f );
					m_pSpotLight->m_Light.SetIntensity( 0.0f );
				}
			}
			else
			{
				m_fSurpriseTimer = 0.0f;
				m_uScoutState = SCOUT_STATE_START_LOWERING_SCANNER;

				m_pScanConeMeshEntity->GetMesh()->SetMeshAlpha( 0.0f );
				m_pSpotLight->m_Light.SetIntensity( 0.0f );
			}
			break;

		default:
			FASSERT_NOW;
			break;

	}

	// update treads
	if( m_hTreadTexLayerR != FMESH_TEXLAYERHANDLE_INVALID && m_hTreadTexLayerL != FMESH_TEXLAYERHANDLE_INVALID )
	{
		CFVec2 vVec;
		f32 fVel = m_Velocity_WS.Dot( m_MtxToWorld.m_vFront );
		m_fTreadUnitAnimPos += FLoop_fPreviousLoopSecs * fVel * _TREAD_SPEED;
		if( m_fTreadUnitAnimPos > 1.0f )
		{
			m_fTreadUnitAnimPos -=1.0f;
		}
		vVec.x = 0.0f;
		vVec.y = -m_fTreadUnitAnimPos;
		m_pWorldMesh->AnimTC_SetScrollST( m_hTreadTexLayerR, vVec );
		m_pWorldMesh->AnimTC_SetScrollST( m_hTreadTexLayerL, vVec );
	}

	// damage smoke 
	if( m_BotInfo_Scout.hSmokeParticle != FPARTICLE_INVALID_HANDLE )
	{
		f32 fUnitHealth = ComputeUnitHealth();
		if( (m_uScoutFlags & SCOUT_FLAG_DAMAGE_TAKEN) &&  //must have taken damage atleast once
			fUnitHealth < 0.8f )
		{
			if( !m_pEParticleSmoke )
			{
				m_pEParticleSmoke = eparticlepool_GetEmitter();
				if( m_pEParticleSmoke )
				{
					CFMtx43A mOr;

					mOr.m_vFront.Set( m_MtxToWorld.m_vUp );
					mOr.m_vUp.Set( m_MtxToWorld.m_vFront );
					mOr.m_vUp.Negate();
					mOr.m_vRight.Set( m_MtxToWorld.m_vRight );
					mOr.m_vPos.Set( *m_apTagPoint_WS[ TAG_POINT_TORSO ] );

					m_pEParticleSmoke->StartEmission( m_BotInfo_Scout.hSmokeParticle, 0.2f + (0.8f - fUnitHealth) );

					m_pEParticleSmoke->Relocate_RotXlatFromUnitMtx_WS( &mOr );
					m_pEParticleSmoke->Attach_ToParent_WS( this, m_apszBoneNameTable[BONE_TORSO], FALSE );
				}
			}
			else
			{
				m_pEParticleSmoke->SetUnitIntensity( 0.2f + (0.8f - fUnitHealth) );
			}
		}
	}

	if( m_uScoutFlags & SCOUT_FLAG_DRAW_BEAM )
	{
		if( m_pAudioEmitterBeam )
		{
			m_pAudioEmitterBeam->SetPosition( &m_MtxToWorld.m_vPos );
		}
		else
		{
			m_pAudioEmitterBeam = AllocAndPlaySound( m_BotInfo_Scout.pSoundGroupAlarmZap );
		}
	}
	else if( m_pAudioEmitterBeam )
	{
		m_pAudioEmitterBeam->Destroy();
		m_pAudioEmitterBeam = NULL;
	}

	// update transmission beam
	_UpdateBeam();

	m_Antenna.Work();


	//
	// the scout is ticking and blinking
	//
	if (m_fTickRate > 0.0f && 
		(m_pWorldMesh->GetVolumeFlags() & FVIS_VOLUME_IN_ACTIVE_LIST))
	{

		f32 fCycle = fmath_PositiveCos( m_fTickTimer * _TICK_TINT_CYCLE_RATE );
		fCycle*=fCycle;
		m_pWorldMesh->SetMeshTint(1.0f-(1.0f-_TICK_TINT_R)*fCycle,
									1.0f-(1.0f-_TICK_TINT_G)*fCycle,
									1.0f-(1.0f-_TICK_TINT_B)*fCycle);

		m_fTickTimer-=FLoop_fPreviousLoopSecs*m_fTickRate;
		if (m_fTickTimer < 0.0f)
		{
			PlaySound( m_BotInfo_Scout.pSoundGroupTick);
			m_fTickTimer = 1.0f;
		}
	}

}


//-----------------------------------------------------------------------------
// Private Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void CBotScout::_ClearDataMembers( void )
{
	m_fCollCylinderHeight_WS	= 4.0f;	// Height w/o antenna 4.3 (with scanner raised)
	m_fCollCylinderRadius_WS	= 2.3f;	// length 4.6, width 3.2

	m_pBotInfo_Gen		= &m_BotInfo_Gen;
	m_pBotInfo_Walk		= &m_BotInfo_Walk;
	m_pBotInfo_Weapon	= &m_BotInfo_Weapon;
	m_pBotInfo_Scout	= &m_BotInfo_Scout;

	m_fGravity = m_pBotInfo_Gen->fGravity;
	ClearJumping();
	m_fMaxFlatSurfaceSpeed_WS	= m_pBotInfo_Walk->fMaxXlatVelocity * m_fRunMultiplier;
	m_fMaxVerticalSpeed_WS		= 0.0f;
	m_fMountPitchMax_WS			= 0.0f;
	m_fMountPitchMin_WS			= 0.0f;
	m_pMoveIdentifier			= &m_apTagPoint_WS[0];
	m_ImpulseVelocity_WS.Zero();
	m_qOrientation.Identity();
	m_uScoutState = SCOUT_STATE_IDLE;
	m_uScoutFlags = 0;
	m_fTransTexFrame = 0.0f;
	m_fTransTexScroll = 0.0f;
	m_mAirRot.Identity();
	m_fTreadUnitAnimPos = 0.0f;
	m_fSurpriseTimer = 0.0f;
	m_fHeadScanSpeed = 0.0f;
	m_fHeadScanAngle = 0.0f;
	m_fDamageRot = 0.0f;
	m_pEParticleSmoke = NULL;
	m_pScanConeMeshEntity = NULL;
	m_fScanConeScaleAngle = 0.0f;
	m_pAudioEmitterScanning = NULL;
	m_pAudioEmitterTreads = NULL;
	m_pAudioEmitterBeam = NULL;
	m_fTickTimer = 0.0f;
	m_fTickRate = 0.0f;
	m_fPctSwitchPressComplete = 0.0f;
}


//-----------------------------------------------------------------------------
void CBotScout::_UpdateMatrices( void )
{
	CFQuatA qNewOrientation;
	CFQuatA qTargetOrientation;
	CFVec3A vFront;
	CFMtx43A mOr;

	if( vFront.SafeUnitAndMag( m_ControlsBot_XlatNormSpeedXZ_WS ) <= 0.0f )
	{
		vFront.Set( m_MtxToWorld.m_vFront );
	}

	m_fMountYaw_WS = fmath_Atan( vFront.x, vFront.z );

	if( m_fDamageRot != 0.0f )
	{
		// incorporate rotation caused by damage infliction into mount yaw
		m_fMountYaw_WS += m_fDamageRot;
		m_fDamageRot = 0.0f;

		// reconstruct vFront from new mount yaw
		fmath_SinCos( m_fMountYaw_WS, &vFront.x, &vFront.z );
	}

	if( m_nState == STATE_GROUND )
	{
		mOr.m_vUp.Set( m_SurfaceUnitNorm_WS );
		mOr.m_vRight.Cross( m_SurfaceUnitNorm_WS, vFront );
		if( mOr.m_vRight.SafeUnitAndMag( mOr.m_vRight ) > 0.0f )
		{
			mOr.m_vFront.Cross( mOr.m_vRight, mOr.m_vUp );
			if( mOr.m_vFront.SafeUnitAndMag( mOr.m_vFront ) <= 0.0f )
			{
				mOr.Identity();
			}
		}
		else
		{
			mOr.Identity();
		}

		mOr.m_vPos = m_MountPos_WS;
	}
	else if( m_uScoutFlags & SCOUT_FLAG_TUMBLE )
	{
		if( m_nPrevState != STATE_AIR )
		{
			m_fAirRot = 0.0f;
		}

		m_fAirRot += FLoop_fPreviousLoopSecs * FMATH_DEG2RAD( 180.0f );
		if( m_fAirRot > FMATH_DEG2RAD( 360.0f ) ) m_fAirRot -= FMATH_DEG2RAD( 360.0f );
		m_mAirRot.SetRotationYXZ( m_fAirRot, m_fAirRot, 0.0f );

		mOr.Set( m_MtxToWorld );
		mOr.Mul( m_mAirRot );
		mOr.m_vPos = m_MountPos_WS;
	}
	else	// in air, but not tumbling
	{
		mOr.Set( m_MtxToWorld );		
		mOr.m_vPos = m_MountPos_WS;
	}

	qTargetOrientation.BuildQuat( mOr );
	if( m_nState == STATE_GROUND && m_nPrevState != STATE_GROUND && (m_uScoutFlags & SCOUT_FLAG_TUMBLE) )
	{
		// snap when hit ground after tumble
		qNewOrientation.Set( qTargetOrientation );
		m_qOrientation.Set( qTargetOrientation );
	}
	else
	{
		qNewOrientation.ReceiveSlerpOf( 0.2f, m_qOrientation, qTargetOrientation );
		m_qOrientation.Set( qNewOrientation );
	}

	if( m_nState == STATE_GROUND )
	{
		m_uScoutFlags &= ~SCOUT_FLAG_TUMBLE;
	}

	qNewOrientation.BuildMtx( mOr );
	mOr.m_vPos = m_MountPos_WS;

	m_pWorldMesh->m_Xfm.BuildFromMtx( mOr );

	m_pWorldMesh->UpdateTracker();

	ComputeMtxPalette( TRUE );

	m_GazeUnitVec_WS.ReceiveUnit( *m_pGazeDir_WS );

//	CFMtx43A::m_Temp.Identity();
//	CFMtx43A::m_Temp.SetRotationYXZ( m_fMountYaw_WS, m_fMountPitch_WS, 0.0f );
//	CFMtx43A::m_Temp.m_vPos = m_MountPos_WS;
//	Relocate_RotXlatFromUnitMtx_WS( &(CFMtx43A::m_Temp), TRUE, m_pMoveIdentifier );

	Relocate_RotXlatFromUnitMtx_WS( &mOr, TRUE, m_pMoveIdentifier );
}

#define _HEAD_SCAN_SPEED		( FMATH_DEG2RAD( 20.0f ) )
#define _HEAD_CENTERING_SPEED	( FMATH_DEG2RAD( 540.0f ) )
#define _HEAD_SCAN_MAX_ANGLE	( FMATH_DEG2RAD( 45.0f ) )
#define _ROTATECW_TURN_SPEED	( FMATH_DEG2RAD( 270.0f ) )
//-----------------------------------------------------------------------------
void CBotScout::AnimateHead( u32 uBoneIdx, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx )
{
	if( m_uScoutState == SCOUT_STATE_SCANNING )
	{
		if( m_fControls_RotateCW != 0.0f )
		{
			// AI is requesting head move, scout must've heard or felt something
			m_fHeadScanAngle += m_fControls_RotateCW * _ROTATECW_TURN_SPEED * FLoop_fPreviousLoopSecs;
			if( m_fHeadScanAngle > FMATH_DEG2RAD( 180.0f ) )
			{
				m_fHeadScanAngle = FMATH_DEG2RAD( -180.0f ) + ( m_fHeadScanAngle - FMATH_DEG2RAD( 180.0f ) );
			}
			else if( m_fHeadScanAngle < FMATH_DEG2RAD( -180.0f ) )
			{
				m_fHeadScanAngle = FMATH_DEG2RAD( 180.0f ) + ( m_fHeadScanAngle + FMATH_DEG2RAD( 180.0f ) );
			}
		}
		else
		{
			// move head back and forth while scanning

			if( m_fHeadScanSpeed == 0.0f )
			{
				m_fHeadScanSpeed = _HEAD_SCAN_SPEED;
			}

			m_fHeadScanAngle += FLoop_fPreviousLoopSecs * m_fHeadScanSpeed;

			if( m_fHeadScanSpeed > 0.0f && m_fHeadScanAngle > _HEAD_SCAN_MAX_ANGLE )
			{
//				m_fHeadScanAngle = _HEAD_SCAN_MAX_ANGLE;
				m_fHeadScanSpeed = -_HEAD_SCAN_SPEED;
			}
			else if( m_fHeadScanSpeed < 0.0f && m_fHeadScanAngle < -_HEAD_SCAN_MAX_ANGLE )
			{
//				m_fHeadScanAngle = -_HEAD_SCAN_MAX_ANGLE;
				m_fHeadScanSpeed = _HEAD_SCAN_SPEED;
			}

		}
	}
	else if( IsSurprised() )
	{
		// don't rotate head when surprised
		m_fHeadScanSpeed = 0.0f;
	}
	else
	{
		// not scanning, return head to center position

		if( m_fHeadScanAngle > 0.0f )
		{
			m_fHeadScanSpeed = -_HEAD_CENTERING_SPEED;
		}
		else if( m_fHeadScanAngle < 0.0f )
		{
			m_fHeadScanSpeed = _HEAD_CENTERING_SPEED;
		}
		else
		{
			m_fHeadScanSpeed = 0.0f;
		}

		m_fHeadScanAngle += FLoop_fPreviousLoopSecs * m_fHeadScanSpeed;

		if( m_fHeadScanSpeed > 0.0f && m_fHeadScanAngle > 0.0f )
		{
			m_fHeadScanAngle = 0.0f;
		}
		else if( m_fHeadScanSpeed < 0.0f && m_fHeadScanAngle < 0.0f )
		{
			m_fHeadScanAngle = 0.0f;
		}
	}

	CFMtx43A mOr;
	mOr.Identity();
	mOr.SetRotationY( m_fHeadScanAngle );

	rNewMtx.Mul( rParentMtx, rBoneMtx );
	rNewMtx.Mul( mOr );
}


//-----------------------------------------------------------------------------
void CBotScout::_AnimBoneCallback( u32 uBoneIdx, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx )
{
	FASSERT( m_pCBScout );
	FASSERT( m_pCBScout->TypeBits() & ENTITY_BIT_BOTSCOUT );

	if( m_pCBScout )
	{
		if( m_pCBScout->m_Antenna.AnimateBone( uBoneIdx, rNewMtx, rParentMtx, rBoneMtx ) )
		{
			if (m_pCBScout->m_fTickRate > 0.0f)
			{
				rNewMtx = rParentMtx;//antenna is retracted if m_fTickRate > 0.0f
			}
			// antenna returns TRUE if it handled bone...
		}
		else if( uBoneIdx == m_nBoneIndexHead )
		{
			m_pCBScout->AnimateHead( uBoneIdx, rNewMtx, rParentMtx, rBoneMtx );
		}
	}
	else
	{
		rNewMtx.Mul( rParentMtx, rBoneMtx );
		return;
	}
}


//-----------------------------------------------------------------------------
void CBotScout::_UpdateBeam( void )
{
	if( m_uScoutFlags & SCOUT_FLAG_DRAW_BEAM )
	{
		m_uScoutFlags &= ~SCOUT_FLAG_DRAW_BEAM;

		// animate transmission texture frame
		m_fTransTexFrame += FLoop_fPreviousLoopSecs * _TRANS_TEX_FRAME_RATE;
		if( m_fTransTexFrame >= (f32) NUM_TRANS_TEX )
		{
			m_fTransTexFrame -= (f32) NUM_TRANS_TEX;
		}
		if ((u32) m_fTransTexFrame >= NUM_TRANS_TEX)
		{
			m_fTransTexFrame = 0.0f;
		}

		// animate transmission texture scroll rate
		m_fTransTexScroll += FLoop_fPreviousLoopSecs * _TRANS_TEX_SCROLL_RATE;
		if( m_fTransTexScroll >= 1.0f )
		{
			m_fTransTexScroll -= 1.0f;
		}

		// submit transmission beams for drawing
		if( _nTransBeam < _NUM_TRANS_BEAMS )
		{
			f32 fYRange = m_apTagPoint_WS[ TAG_POINT_ANTENNA3 ]->y - m_MtxToWorld.m_vPos.y;
			f32 fYOff = fYRange* (1.0f-2.0f*fmath_Abs(0.5f - m_fPctSwitchPressComplete));
			f32 fAlpha = 0.52539f;
			f32 fScrollRate = 1.0f;

			if (!(this->m_uScoutFlags & SCOUT_FLAG_AT_SWITCH))
			{
				fAlpha *=0.35f;
				fScrollRate = 0.5f;
			}
			//draw beam 1
			_aTransBeam[ _nTransBeam ].vStart.Set( *m_apTagPoint_WS[ TAG_POINT_ANTENNA3 ] );
			_aTransBeam[ _nTransBeam ].vEnd.Set( m_vTransBeamEnd );
			_aTransBeam[ _nTransBeam ].vStart.y -= fYOff;
			_aTransBeam[ _nTransBeam ].vEnd.y -= fYOff;
			if (this->m_uScoutFlags & SCOUT_FLAG_AT_SWITCH)
			{
				_aTransBeam[ _nTransBeam ].vColor.Set( 0.8f, 0.8f, 1.0f, fAlpha );
			}
			else
			{
				_aTransBeam[ _nTransBeam ].vColor.Set( 0.4f, 0.8f, 4.0f, fAlpha );
			}
			_aTransBeam[ _nTransBeam ].fTexFrame = m_fTransTexFrame;
			_aTransBeam[ _nTransBeam ].fTexScroll = m_fTransTexScroll*fScrollRate;
			++_nTransBeam;

			//draw beam 2 if at switch
			if (this->m_uScoutFlags & SCOUT_FLAG_AT_SWITCH)
			{
				if( _nTransBeam < _NUM_TRANS_BEAMS )
				{
					_aTransBeam[ _nTransBeam ].vStart.Set( *m_apTagPoint_WS[ TAG_POINT_ANTENNA3 ] );
					_aTransBeam[ _nTransBeam ].vEnd.Set( m_vTransBeamEnd );
					_aTransBeam[ _nTransBeam ].vStart.y -= fYOff;
					_aTransBeam[ _nTransBeam ].vEnd.y -= fYOff;
					_aTransBeam[ _nTransBeam ].vColor.Set( 1.0f, 1.0f, 0.8f, fAlpha );
					_aTransBeam[ _nTransBeam ].fTexFrame = m_fTransTexFrame;
					_aTransBeam[ _nTransBeam ].fTexScroll = m_fTransTexScroll*fScrollRate;
					++_nTransBeam;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
#define _NUM_TRANSMISSION_VERTICES		( 32 )
#define _TRANSMISSION_BEAM_HEIGHT		( 2.0f )
#define _TRANSMISSION_BEAM_MOD_AMOUNT	( 0.15f )
#define _TRANS_BEAM_BOW_MOD_AMOUNT		( 0.65f )
void CBotScout::DrawTransmissionBeam( CFVec3A &rvStart, CFVec3A &rvEnd, CFColorRGBA &rvColor, f32 fTexFrame, f32 fTexScroll, CFTexInst* pTexInst )
{
	CFXfm Xfm;
	CFMtx43A mOr;
	CFVec3A vDelta, vRight, vTemp;
	FDrawVtx_t *pVtxArray;
	CFCamera *pCurCam;
	s32 nVertex;
	s32 nPair;
	f32 fRightMag;
	f32 fCoord;
	f32 fYMod;
	f32 fBow;

	FASSERT( _NUM_TRANSMISSION_VERTICES % 2 == 0 );

	pVtxArray = fvtxpool_GetArray( _NUM_TRANSMISSION_VERTICES );

	if( pVtxArray == NULL )
	{
		return;
	}

	pCurCam = gamecam_GetActiveCamera();

	if( pCurCam == NULL )
	{
		return;
	}

	for( nVertex = 0; nVertex < _NUM_TRANSMISSION_VERTICES; nVertex++ )
	{
		pVtxArray[ nVertex ].ColorRGBA.Set( 0.5f, 0.5f, 1.0f, 1.0f );
		pVtxArray[ nVertex ].ColorRGBA.Set( rvColor );
	}

	// compute texture coordinates for beam polygons
	for( nPair = 0; nPair < _NUM_TRANSMISSION_VERTICES / 2; nPair++ )
	{
		nVertex = nPair * 2;
		// find fractional tex coord along strip for this pair of vertically-aligned vertices
		fCoord = 1.0f - ( (f32)nPair / (f32)((_NUM_TRANSMISSION_VERTICES / 2) - 1) );

		pVtxArray[nVertex].ST.Set( fTexScroll + fCoord, 1.0f );
		pVtxArray[nVertex+1].ST.Set( fTexScroll + fCoord, 0.0f );
	}

	// find vector from start of beam to end
	vDelta.Set( rvEnd );
	vDelta.Sub( rvStart );

	// find unit "Right" vector to beam direction
	vTemp.x = vDelta.z;
	vTemp.y = 0.0f;
	vTemp.z = -vDelta.x;
	fRightMag = vRight.SafeUnitAndMag( vTemp );

//	fBow = FMATH_2PI * fmath_RandomBipolarUnitFloat();
	fBow = fmath_RandomBipolarUnitFloat();

	// compute model space coords of vertical beam vertex pairs
	for( nPair = 0; nPair < _NUM_TRANSMISSION_VERTICES / 2; nPair++ )
	{
		nVertex = nPair * 2;
		fCoord = (f32)nPair / (f32)((_NUM_TRANSMISSION_VERTICES / 2) - 1);
		fYMod = fmath_Sin( 2.0f * -fTexScroll * FMATH_4PI + 2.0f * fCoord * FMATH_2PI ) * _TRANSMISSION_BEAM_MOD_AMOUNT;
//		fYMod += fmath_Sin( fBow + fCoord * FMATH_PI ) * _TRANS_BEAM_BOW_MOD_AMOUNT;
		fYMod += fmath_Sin( fCoord * FMATH_PI ) * fBow * _TRANS_BEAM_BOW_MOD_AMOUNT;

		vTemp.Mul( vDelta, fCoord );
		pVtxArray[nVertex].Pos_MS.Set( vTemp.x, vTemp.y + fYMod, vTemp.z );
		pVtxArray[nVertex+1].Pos_MS.Set( vTemp.x, vTemp.y+_TRANSMISSION_BEAM_HEIGHT+fYMod, vTemp.z );
	}

	mOr.Identity();
	mOr.m_vPos.Set( rvStart );
	Xfm.BuildFromMtx( mOr );

	// set up drawing modes
	frenderer_Push( FRENDERER_DRAW, NULL );
	Xfm.PushModel();
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST );
		
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_SetCullDir( FDRAW_CULLDIR_NONE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER_OR_EQUAL );
	fdraw_SetTexture( pTexInst );
	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, pVtxArray, _NUM_TRANSMISSION_VERTICES );				

	if( fRightMag >= 0.0f )
	{
		vRight.Mul( _TRANSMISSION_BEAM_HEIGHT * 0.5f );

		// compute model space coords of horizontal beam vertex pairs
		for( nPair = 0; nPair < _NUM_TRANSMISSION_VERTICES / 2; nPair++ )
		{
			nVertex = nPair * 2;
			fCoord = (f32)nPair / (f32)((_NUM_TRANSMISSION_VERTICES / 2) - 1);
			fYMod = fmath_Sin( 2.0f * -fTexScroll * FMATH_4PI + 2.0f * fCoord * FMATH_2PI ) * _TRANSMISSION_BEAM_MOD_AMOUNT;
//			fYMod += fmath_Sin( fBow + fCoord * FMATH_PI ) * _TRANS_BEAM_BOW_MOD_AMOUNT;
			fYMod += fmath_Sin( fCoord * FMATH_PI ) * fBow * _TRANS_BEAM_BOW_MOD_AMOUNT;

			vTemp.Mul( vDelta, fCoord );
			pVtxArray[nVertex].Pos_MS.Set( vRight.x + vTemp.x, vTemp.y + fYMod + _TRANSMISSION_BEAM_HEIGHT * 0.5f, vRight.z + vTemp.z );
			pVtxArray[nVertex+1].Pos_MS.Set( -vRight.x + vTemp.x, vTemp.y + fYMod + _TRANSMISSION_BEAM_HEIGHT * 0.5f, -vRight.z + vTemp.z );
		}

		fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, pVtxArray, _NUM_TRANSMISSION_VERTICES );
	}
	CFXfm::PopModel();
	frenderer_Pop();

	fvtxpool_ReturnArray( pVtxArray );
}






// notifies the bot it has collided with another bot.  if FALSE is returned, normal collision response is skipped.
BOOL CBotScout::NotifyBotCollision( CBot *pBot )
{
	if (pBot->IsPlayerBot() && !IsDeadOrDying())
	{
		if (m_fTickRate > 0.0f)
		{
			//blow up if player touches us
			Die();

			CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();
			if( pDamageForm )
			{
				pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_AMBIENT;
				pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
				pDamageForm->m_Damager.pWeapon = NULL;
				pDamageForm->m_Damager.pBot = this;
				pDamageForm->m_Damager.nDamagerPlayerIndex = m_nPossessionPlayerIndex;
				pDamageForm->m_pDamageeEntity = pBot;
				pDamageForm->m_pDamageProfile = m_BotInfo_Scout.pScoutDetDamageProfile;
				CDamage::SubmitDamageForm( pDamageForm );
			}

		}
	}

	return TRUE;
}

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotScoutBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

//-----------------------------------------------------------------------------
void CBotScoutBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType )
{
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_BOTSCOUT, pszEntityType );

	// Override defaults set by our parent classes...
	m_nEC_HealthContainerCount = CBotScout::m_BotInfo_Gen.nBatteryCount;
	m_pszEC_ArmorProfile = CBotScout::m_BotInfo_Gen.pszArmorProfile;

	// This class of bots shows up on Glitch's radar...
	FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_SHOWS_UP_ON_RADAR );
}


//-----------------------------------------------------------------------------
BOOL CBotScoutBuilder::PostInterpretFixup( void )
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


//-----------------------------------------------------------------------------
BOOL CBotScoutBuilder::InterpretTable( void )
{
	return CBotBuilder::InterpretTable();
}

void CBotScout::SetScanning(BOOL bScan)
{
	FASSERT( IsCreated() );
	if( bScan )
	{
		m_uScoutFlags |= SCOUT_FLAG_SHOULD_SCAN;
	}
	else 
	{
		m_uScoutFlags &= ~SCOUT_FLAG_SHOULD_SCAN;
	}
}