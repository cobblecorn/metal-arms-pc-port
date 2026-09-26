//////////////////////////////////////////////////////////////////////////////////////
// botzombieboss.cpp -
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
// 03/24/02 Scholz       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "BotZombieBoss.h"
#include "fanim.h"
#include "fresload.h"
#include "floop.h"
#include "gamepad.h"
#include "meshtypes.h"
#include "reticle.h"
#include "player.h"
#include "level.h"
#include "ProTrack.h"
#include "fsound.h"
#include "meshentity.h"
#include "botpart.h"
#include "Ai\AIEnviro.h"
#include "AI/AIGameUtils.h"
#include "fworld_coll.h"
#include "miscDraw.h"
#include "botGlitch.h"
#include "fxshockwave.h"
#include "potmark.h"
#include "FScriptSystem.h"
#include "Fdebris.h"
#include "EBotfire.h"

#define _BOTINFO_FILENAME		"b_zombiebos"
#define _BOTPART_FILENAME		"bpzombiebos"


static cchar*		   _BotZombieBoss_pszSoundEffectBank = "ZombieBoss";

static cchar* _pszSoundGroupTable[CBotZombieBoss::ZBOSS_SND_COUNT] =
{
	"SRZBReel",		// ZBOSS_SND_BALL_REEL,
	"SRZBDrag",		// ZBOSS_SND_BALL_DRAG
	"SRZBSwoosh",	// ZBOSS_SND_BALL_SWOOSH,
	"SRZBGrab",		// ZBOSS_SND_GRAB_CONTACT,
	"SRZBEffort",	// ZBOSS_SND_EFFORT,
	"SRZBDamage",	// ZBOSS_SND_DAMAGED,
	"SRZBRoar",		// ZBOSS_SND_ROAR,
	"SRZBDie",		// ZBOSS_SND_DIE,
};

struct BossBallParams_t
{
	f32 fVerletPendentLength;			// ,2.5,#	Verlet pendent length
	f32 fVerletPendentGravityX;			// ,0,#		Verlet Pendent gravity X
	f32 fVerletPendentGravityY;			// ,-200,#	Verlet Pendent gravity Y
	f32 fVerletPendentGravityZ;			// ,0,#		Verlet Pendent gravity Z
	f32 fVerletDampeningConstant;		// ,0.3,#	Verlet Dampening constant
	
	BossBallParams_t():
		fVerletPendentLength(12.f),
		fVerletPendentGravityX(0.f),
		fVerletPendentGravityY(-625.f),
		fVerletPendentGravityZ(0.f),
		fVerletDampeningConstant(0.4f)
		{}
};
static const BossBallParams_t _BossBallParams;
static const f32 _fMovingToSwingTime = .30f;
static const f32 _fOOMovingToSwingTime = 1.0f/_fMovingToSwingTime;
static const f32 _fSwingToMovingTime = .5f;
static const f32 _fOOSwingToMovingTime = 1.0f/_fSwingToMovingTime;

static const f32 _fMovingToLookTime = .5f;
static const f32 _fOOMovingToLookTime = 1.0f/_fMovingToLookTime;
static const f32 _fLookToMovingTime = .5f;
static const f32 _fOOLookToMovingTime = 1.0f/_fLookToMovingTime;

static const f32 _fLookToGrabbingTime = .5f;
static const f32 _fOOLookToGrabbingTime = 1.0f/_fLookToGrabbingTime;

static const f32 _fGrabbingToLookTime = .1f;
static const f32 _fOOGrabbingToLookTime = 1.0f/_fGrabbingToLookTime;

static const f32 _fGrabFailureToLookTime = 1.0f;
static const f32 _fOOGrabFailureToLookTime = 1.0f/_fGrabFailureToLookTime;

static const f32 _fTugPauseTime = .2f;
static const f32 _fOOTugPauseTime = 1.0f/_fTugPauseTime;

static const f32 _fTugPullSpeedMult = .7f;
static const f32 _fTimeBetweenTugs = 1.2f;

static const f32 _fGrabSpeedMult = 1.0f;
static const f32 _fGrabSuccessSphereRadius = 6.0f;

static const f32 _fDamageSphereRadius=3.50f;

static const f32 _fMinTimeBetweenOuches = .5f;
static const f32 _fMaxTimeBetweenOuches = 2.0f;

static const f32 _fMinTimeBetweenGrunts = 2.0f;
static const f32 _fMaxTimeBetweenGrunts = 6.0f;

static const f32 _fDeathSpeedMult = 1.0f;
static const f32 _fToDeathTime = .5f;
static const f32 _fOOToDeathTime = 1.0f / _fToDeathTime;

static const f32 _fReelVelocity = 14.0f;
static const f32 _fCollideWithBossChainLength = _BossBallParams.fVerletPendentLength+ 2.0f;

static const f32 _fTimeOfGrabTest =   19.0f / 30.0f;
static const f32 _fTimeOfGrabPickup = 20.0f / 30.0f;
static const f32 _fMaxChainLength = 109.f;
static CFVec3A _vInTangent(0.0f,80.f,0.0f);
static CFVec3A _vOutTangent(0.0f,-100.f,0.0f);

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotZombieBossBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CBotZombieBossBuilder _BotZombieBossBuilder;


void CBotZombieBossBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType )
{	
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_BOTZOMBIEBOSS, pszEntityType );

	// Override defaults set by our parent classes...
	m_nEC_HealthContainerCount = CBotZombieBoss::m_BotInfo_Gen.nBatteryCount;
	m_pszEC_ArmorProfile = CBotZombieBoss::m_BotInfo_Gen.pszArmorProfile;
}


BOOL CBotZombieBossBuilder::PostInterpretFixup( void )
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


BOOL CBotZombieBossBuilder::InterpretTable( void )
{
	return CBotBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotZombieBoss
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************


u32		CBotZombieBoss::m_uSystemFlags			= 0;
u32		CBotZombieBoss::m_nBotClassClientCount	= 0;

CBotZombieBoss::BotInfo_Gen_t CBotZombieBoss::m_BotInfo_Gen;
CBotZombieBoss::BotInfo_MountAim_t CBotZombieBoss::m_BotInfo_MountAim;
CBotZombieBoss::BotInfo_Walk_t CBotZombieBoss::m_BotInfo_Walk;
CBotZombieBoss::ZombieBossFightLevel_t* CBotZombieBoss::m_paFightLevels; 
u32 CBotZombieBoss::m_uNumFightLevels;

CBotAnimStackDef CBotZombieBoss::m_AnimStackDef;

CBotPartPool*	CBotZombieBoss::m_pPartPool;
CFSphere		CBotZombieBoss::m_GrabSphere;

FParticle_DefHandle_t	CBotZombieBoss::m_hBreathParticleDef	= FPARTICLE_INVALID_HANDLE;
FParticle_DefHandle_t	CBotZombieBoss::m_hSmokeParticleDef	    = FPARTICLE_INVALID_HANDLE;

FExplosion_GroupHandle_t CBotZombieBoss::m_hExplosionBallHitGroup= FEXPLOSION_INVALID_HANDLE;
FExplosion_GroupHandle_t CBotZombieBoss::m_hExplosionDie = FEXPLOSION_INVALID_HANDLE;
CDamageForm::Damager_t	 CBotZombieBoss::m_Damager;
const CDamageProfile*	 CBotZombieBoss::m_pDamageProfileDrag;
const CDamageProfile*	 CBotZombieBoss::m_pDamageProfileDeath;

s32			CBotZombieBoss::m_nBoneIndexHead		= -1;
s32			CBotZombieBoss::m_nBoneIndexTorso		= -1;
s32			CBotZombieBoss::m_nBoneIndexGroin		= -1;
s32			CBotZombieBoss::m_nBoneIndexAttachBall	= -1;
s32			CBotZombieBoss::m_nBoneIndexAttachGlitch= -1;
s32			CBotZombieBoss::m_nBoneIndexBall		= -1;
s32			CBotZombieBoss::m_nBoneIndexCollidePlane= -1;
s32			CBotZombieBoss::m_nBoneIndexLHand		= -1;
s32			CBotZombieBoss::m_nBoneIndexL_Foot		= -1;
s32			CBotZombieBoss::m_nBoneIndexR_Foot		= -1;
s32			CBotZombieBoss::m_nBoneIndexChainReel	= -1;

s32			CBotZombieBoss::m_nBoneIndexPTack_Head		= -1;
s32			CBotZombieBoss::m_nBoneIndexPTack_Smoke01	= -1;
s32			CBotZombieBoss::m_nBoneIndexPTack_Smoke02	= -1;
s32			CBotZombieBoss::m_nBoneIndexPTack_Smoke03	= -1;

s32			CBotZombieBoss::m_nBoneIndexGrabbedBotsHead=-1;
CBot*		CBotZombieBoss::m_pGrabbedBot=NULL;
CFQuatA		CBotZombieBoss::m_TorsoQuat;

CDamageProfile* CBotZombieBoss::m_pFootstepDamageProfile;
CFDebrisGroup*  CBotZombieBoss::m_pDebrisGroup;

CFMtx43A CBotZombieBoss::m_mtxBall;
CFVec3A CBotZombieBoss::m_vChainUnitDirection;
CFVec3A CBotZombieBoss::m_vChainLengthAndDirection;
CFVec3A CBotZombieBoss::m_vChainNextLinkPositionWS;
f32 CBotZombieBoss::m_fChainLinkLength;			 
f32 CBotZombieBoss::m_fOOChainLinkLength;		
s32 CBotZombieBoss::m_nCurrentLinkIndex;			 

CFSoundGroup* CBotZombieBoss::m_pSounds[ZBOSS_SND_COUNT];

// Compute's the union of Sphere A and Sphere B, and stores into A
BOOL SphereUnion(CFSphere& SphereA,CFVec3& vSphereC0,f32 fSphereR0)
{
	CFVec3 vOriginDiff;
	vOriginDiff.Sub(SphereA.m_Pos,vSphereC0);
	f32 fRadiusDiff = SphereA.m_fRadius - fSphereR0;
	f32 fRadiusDiffSqr = fRadiusDiff * fRadiusDiff;
	f32 fLengthSqr = vOriginDiff.Mag2();
	if (fRadiusDiffSqr >= fLengthSqr)
	{
		if (fRadiusDiff >= 0.0f)
		{
			return FALSE;
		}
		else
		{
			SphereA.m_Pos = vSphereC0;
			SphereA.m_fRadius = fSphereR0;
			return TRUE;
		}
	}
	else
	{
		f32 fOneOverL = fmath_InvSqrt(fLengthSqr);
		f32 fL = fmath_Inv(fOneOverL);
        f32 t  = (fL + fRadiusDiff) * (0.5f * fOneOverL);
		SphereA.m_Pos.Add(vSphereC0,(vOriginDiff.Mul(t)));
		SphereA.m_fRadius = (fL + SphereA.m_fRadius + fSphereR0) * 0.5f;
		return TRUE;
	}
}

BOOL CBotZombieBoss::ClassHierarchyLoadSharedResources( void )
{
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
	FASSERT(!IsInitialized());
   	FMATH_SETBITMASK(m_uSystemFlags, SYS_INITIALIZED);

	FResFrame_t ResFrame;
	ResFrame = fres_GetFrame();

	if( !ReadBotInfoFile( m_aGameDataMap, _BOTINFO_FILENAME ) )
	{
		goto _ExitInitSystemWithError;
	}

	if  (!_ReadFightLevel(_BOTINFO_FILENAME))
	{
		goto _ExitInitSystemWithError;
	}

	if( !_BuildAnimStackDef() )
	{
		goto _ExitInitSystemWithError;
	}

	// Load the sound effects bank for this bot...
	if( !fresload_Load( FSNDFX_RESTYPE, _BotZombieBoss_pszSoundEffectBank) )
	{
		DEVPRINTF( "CBotZombieBoss::ClassHierarchyLoadSharedResources(): Could not load sound effect bank '%s'\n", _BotZombieBoss_pszSoundEffectBank);
	}


	for (ZombieBossSnd_e iSndIdx = (ZombieBossSnd_e)0; iSndIdx < ZBOSS_SND_COUNT; iSndIdx = (ZombieBossSnd_e)(int(iSndIdx)+1))
	{
		m_pSounds[iSndIdx] = CFSoundGroup::RegisterGroup( _pszSoundGroupTable[iSndIdx] );
		if (!m_pSounds[iSndIdx])
			DEVPRINTF( "CBotZombieBoss::ClassHierarchyLoadSharedResources(): Could not load sound effect '%s'\n", _pszSoundGroupTable[iSndIdx]);
	}

	m_hExplosionBallHitGroup = CExplosion2::GetExplosionGroup( "ZombBossBall" );
	if( m_hExplosionBallHitGroup == FEXPLOSION_INVALID_HANDLE ) 
	{
		DEVPRINTF( "CBotMortar::Init(): Could not find explosion definition '%s'.\n", "ZombBossBall" );
		goto _ExitInitSystemWithError;
	}

	m_hExplosionDie = CExplosion2::GetExplosionGroup( "ZBossDie" );
	if( m_hExplosionDie == FEXPLOSION_INVALID_HANDLE ) 
	{
		DEVPRINTF( "CBotMortar::Init(): Could not find explosion definition '%s'.\n", "ZBossDie" );
		goto _ExitInitSystemWithError;
	}

	m_pDamageProfileDrag = CDamage::FindDamageProfile("ZBossBallDrag",FALSE,TRUE);
	if( m_pDamageProfileDrag == NULL) 
	{
		DEVPRINTF( "CBotZBoss::Init(): Could not find damage profile '%s'.\n", "ZBossBallDrag" );
		goto _ExitInitSystemWithError;
	}

	m_pDamageProfileDeath = CDamage::FindDamageProfile("ZBossDeath",FALSE,TRUE);
	if( m_pDamageProfileDeath == NULL) 
	{
		DEVPRINTF( "CBotZBoss::Init(): Could not find damage profile '%s'.\n", "ZBossDeath" );
		goto _ExitInitSystemWithError;
	}

	// Particle effects
	m_hBreathParticleDef = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, "zombiefire" );
	m_hSmokeParticleDef = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, "zombiesmk" );
	
	if ( ( m_hBreathParticleDef == FPARTICLE_INVALID_HANDLE ) || ( m_hSmokeParticleDef== FPARTICLE_INVALID_HANDLE ) )
	{
		DEVPRINTF( "CBotZombieBoss::ClassHierarchyBuild(): Could not find particle definitions");
	}
	
	m_pFootstepDamageProfile = CDamage::FindDamageProfile( "ZBossFootstep" );

	m_pDebrisGroup = CFDebrisGroup::Find("ZomChunks");

	return TRUE;
	// Error:
_ExitInitSystemWithError:
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

void CBotZombieBoss::ClassHierarchyUnloadSharedResources( void )
{
	FASSERT( m_nBotClassClientCount > 0 );

	--m_nBotClassClientCount;

	if( m_nBotClassClientCount > 0 )
	{
		return;
	}

	m_AnimStackDef.Destroy();
	CBot::ClassHierarchyUnloadSharedResources();
	FMATH_CLEARBITMASK(m_uSystemFlags, SYS_INITIALIZED);

}

CBotZombieBoss::CBotZombieBoss() : CBot()
{
}


CBotZombieBoss::~CBotZombieBoss()
{
	if( IsSystemInitialized() && IsCreated() )
	{
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld(TRUE);
		ClassHierarchyDestroy();
	}
}


BOOL CBotZombieBoss::Create( s32 nPlayerIndex, cchar *pszEntityName, const CFMtx43A *pMtx )
{
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	if( !ClassHierarchyLoadSharedResources() )
	{
		// Failure! (resources have already been cleaned up, so we can bail now)
		return FALSE;
	}
	// Get pointer to the leaf class's builder object...
	CBotZombieBossBuilder *pBuilder = (CBotZombieBossBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_BotZombieBossBuilder )
	{
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOTZOMBIEBOSS );
	}

	// Create an entity...
	if (nPlayerIndex > -1)
		return CBot::Create( &m_BotDef, nPlayerIndex, FALSE, pszEntityName, pMtx, NULL );
	else
		return CBot::Create( &m_BotDef, nPlayerIndex, FALSE, pszEntityName, pMtx, "Default" );
}


void CBotZombieBoss::ClassHierarchyDestroy( void )
{	// Delete the items that we had instantiated for us...
	m_Anim.Destroy();

	fdelete( m_pWorldMesh );
	m_pWorldMesh = NULL;

	fdelete( m_pBallMesh );
	m_pBallMesh = NULL;

	for (ZombieBossSnd_e iSndIdx = (ZombieBossSnd_e)0; iSndIdx < ZBOSS_SND_COUNT; iSndIdx = (ZombieBossSnd_e)(int(iSndIdx)+1))
	{
		if (m_pSoundEmitters[iSndIdx])
		{
			m_pSoundEmitters[iSndIdx]->Destroy();
			m_pSoundEmitters[iSndIdx] = NULL;
		}
	}

	CBot::ClassHierarchyDestroy();
}


BOOL CBotZombieBoss::ClassHierarchyBuild( void )
{	FMesh_t *pMesh;
	FMeshInit_t MeshInit;
	cchar * pszMeshFilename = NULL;

	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CBotZombieBossBuilder *pBuilder = (CBotZombieBossBuilder *)GetLeafClassBuilder();

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
	pszMeshFilename = "grzzboss_00";
	if( pBuilder->m_pszMeshReplacement ) {
	   pszMeshFilename = pBuilder->m_pszMeshReplacement;
	}
	pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pszMeshFilename);
	if( pMesh == NULL )
	{
		DEVPRINTF( "CBotZombieBoss::ClassHierarchyBuild(): Could not find mesh '%s'\n", "grzzboss_00" );
		goto _ExitWithError;
	}

	// Create world mesh...
	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL )
	{
		DEVPRINTF( "CBotZombieBoss::ClassHierarchyBuild(): Not enough memory to create CFWorldMesh.\n" );
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
	m_pWorldMesh->m_nFlags |=  (FMESHINST_FLAG_NOSHADOWLOD|FMESHINST_FLAG_CAST_SHADOWS);
	m_pWorldMesh->SetCollisionFlag( TRUE );
	m_pWorldMesh->SetLineOfSightFlag( FALSE );

	m_pWorldMesh->UpdateTracker();
	m_pWorldMesh->RemoveFromWorld();
	m_NormalSphereMS = m_pWorldMesh->m_BoundSphere_MS;

	// Build our animation stack and load animations...
	if( !m_Anim.Create( &m_AnimStackDef, m_pWorldMesh ) )
	{
		DEVPRINTF( "CBotZombieBoss::ClassHierarchyBuild(): Trouble creating m_Anim.\n" );
		goto _ExitWithError;
	}

	SetControlValue( ANIMCONTROL_STAND_LOOK, 1.0f );
	UpdateUnitTime( ANIMCONTROL_STAND_LOOK, fmath_RandomFloat() );

	m_nBoneIndexHead = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_HEAD] );
	m_nBoneIndexTorso = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_TORSO] );
	m_nBoneIndexGroin = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_GROIN] );
	m_nBoneIndexAttachBall = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ATTACHPOINT_CHAIN] );
	m_nBoneIndexBall = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_WRECKING_BALL] );
	m_nBoneIndexCollidePlane = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_R_ARM_UPPER] );
	m_nBoneIndexLHand = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_L_HAND] );
	m_nBoneIndexAttachGlitch = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ATTACHPOINT_GLITCH] );

	m_nBoneIndexL_Foot = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_L_FOOT] );
	m_nBoneIndexR_Foot = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_R_FOOT] );

	m_nBoneIndexChainReel = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_CHAIN_REEL] );

	if( (m_nBoneIndexTorso < 0)  || (m_nBoneIndexHead < 0) || (m_nBoneIndexGroin <0) || (m_nBoneIndexBall < 0)|| (m_nBoneIndexAttachBall<0) || (m_nBoneIndexCollidePlane<0)|| (m_nBoneIndexLHand<0) || (m_nBoneIndexL_Foot<0)||(m_nBoneIndexR_Foot<0) || (m_nBoneIndexChainReel < 0))
	{
		DEVPRINTF( "CBotZombieBoss::ClassHierarchyBuild(): Trouble retrieving bone indices.\n" );
		goto _ExitWithError;
	}

	m_nBoneIndexPTack_Head	  = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_PTACK_HEAD] );
	m_nBoneIndexPTack_Smoke01 = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_PTACK_SMOKE01] );
	m_nBoneIndexPTack_Smoke02 = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_PTACK_SMOKE02] );
	m_nBoneIndexPTack_Smoke03 = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_PTACK_SMOKE03] );

	if ( (m_nBoneIndexPTack_Head < 0) || (m_nBoneIndexPTack_Smoke01 < 0)|| (m_nBoneIndexPTack_Smoke02 < 0) ||(m_nBoneIndexPTack_Smoke03 < 0))
	{
		DEVPRINTF( "CBotZombieBoss::ClassHierarchyBuild(): Trouble retrieving PTACK bone indices.\n" );
		goto _ExitWithError;
	}
	
	m_Anim.m_pAnimCombiner->SetBoneCallback( &_AnimBoneCallback );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_TORSO] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_HEAD] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ATTACHPOINT_CHAIN] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_WRECKING_BALL] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_L_HAND] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_CHAIN_REEL] );

	// Find approx eye point...
	m_pApproxEyePoint_WS = &m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexHead]->m_vPos;
	
	// Initialize matrix palette (this is important for attached entities)...
	AtRestMatrixPalette();

	m_FlameEmitter1.Create();
	m_SmokeEmitter1.Create();
	m_SmokeEmitter2.Create();
	m_SmokeEmitter3.Create();

	// Create tag points...
	if( !TagPoint_CreateFromBoneArray( m_anTagPointBoneNameIndexArray, m_apszBoneNameTable ) ) {
		goto _ExitWithError;
	}

	if (!_BuildChainLinkPool())
	{
		DEVPRINTF("Failed to build chain\n");
		goto _ExitWithError;
	}
	
	_ComputeGrabPointMS(m_vGrabPtMS);
	
	// Load mesh resource...
	pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, "GRZZball_01");
	if( pMesh == NULL )
	{
		DEVPRINTF( "CBotZombieBoss::ClassHierarchyBuild(): Could not find mesh '%s'\n", "GRZZball_01" );
		goto _ExitWithError;
	}

	// Create world mesh...
	m_pBallMesh = fnew CFWorldMesh;
	if( m_pBallMesh == NULL )
	{
		DEVPRINTF( "CBotZombieBoss::ClassHierarchyBuild(): Not enough memory to create CFWorldMesh.\n" );
		goto _ExitWithError;
	}

	// Init the world mesh...
	MeshInit.pMesh = pMesh;
	MeshInit.nFlags = 0;
	MeshInit.fCullDist = FMATH_MAX_FLOAT;
	MeshInit.Mtx.Set( pBuilder->m_EC_Mtx_WS );
	m_pBallMesh->Init( &MeshInit );

	m_pBallMesh->m_nUser = MESHTYPES_UNKNOWN;
	m_pBallMesh->m_pUser = NULL;
	m_pBallMesh->SetUserTypeBits( NULL );
	m_pBallMesh->m_nFlags &= ~(FMESHINST_FLAG_DONT_DRAW | FMESHINST_FLAG_NOCOLLIDE);
	m_pBallMesh->m_nFlags |= (FMESHINST_FLAG_NOSHADOWLOD|FMESHINST_FLAG_CAST_SHADOWS);
	m_pBallMesh->SetCollisionFlag( TRUE );
	m_pBallMesh->SetLineOfSightFlag( FALSE );

	m_pBallMesh->UpdateTracker();
	m_pBallMesh->RemoveFromWorld();
	m_pBallMesh->m_BoundSphere_MS.m_Pos = m_pBallMesh->m_BoundSphere_MS.m_Pos;
	m_pBallMesh->m_BoundSphere_MS.m_fRadius *= .60f;

	m_fMaximumChainLength= _fMaxChainLength;
	
	SetMaxHealth();
	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

BOOL CBotZombieBoss::_BuildChainLinkPool(void)
{
	char szLinkName[] = "Chain_Link01";

	s32 nBoneIndexLink =  m_pWorldMesh->FindBone(szLinkName);
	if ( (nBoneIndexLink != -1) )
	{
		m_fChainLinkLength = 3.64f;
		m_fOOChainLinkLength = 1.0f/m_fChainLinkLength;
	}

	for (s32 nBoneIndex = 0; nBoneIndex < m_pWorldMesh->m_pMesh->nBoneCount;nBoneIndex++)
	{
		if (fclib_strncmp(m_pWorldMesh->m_pMesh->pBoneArray[nBoneIndex].szName,"Chain_Link",10)==0)
		{
			m_Anim.m_pAnimCombiner->EnableBoneCallback(nBoneIndex);
		}
	}
	return TRUE;
}

BOOL CBotZombieBoss::ClassHierarchyBuilt( void )
{
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CBot::ClassHierarchyBuilt() )
	{
		goto _ExitWithError;
	}

	SetBotFlag_MeshCollideOnly();
	SetBotFlag_Enemy();
	SetWalkable(FALSE);
	EnableOurWorkBit();

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


CEntityBuilder *CBotZombieBoss::GetLeafClassBuilder( void )
{
	return &_BotZombieBossBuilder;
}


void CBotZombieBoss::_ClearDataMembers( void )
{
	// Init data members...

	m_pMoveIdentifier = m_pSoundEmitters;

	m_fCollCylinderRadius_WS = 20.0f;
	m_fCollCylinderHeight_WS = 30.0f;

	m_pBotInfo_Gen = &m_BotInfo_Gen;
	m_pBotInfo_MountAim = &m_BotInfo_MountAim;
	m_pBotInfo_Walk = &m_BotInfo_Walk;

	m_fGravity = m_pBotInfo_Gen->fGravity;
	m_fMaxFlatSurfaceSpeed_WS = m_pBotInfo_Walk->fMaxXlatVelocity * m_fRunMultiplier;

	m_fMountPitchMax_WS = m_pBotInfo_MountAim->fMountPitchDownLimit;
	m_fMountPitchMin_WS = m_pBotInfo_MountAim->fMountPitchUpLimit;

	m_anAnimStackIndex[ASI_STAND] = ANIMTAP_STAND_LOOK;
	m_anAnimStackIndex[ASI_STAND_ALERT] = -1;

	m_anAnimStackIndex[ASI_WALK] = ANIMTAP_WALK;
	m_anAnimStackIndex[ASI_WALK_ALERT] = -1;
	m_anAnimStackIndex[ASI_RUN] = ANIMTAP_WALK;
	m_anAnimStackIndex[ASI_RUN_PANIC] = -1;

	m_anAnimStackIndex[ASI_FALL] = -1;

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
	m_anAnimStackIndex[ASI_RC_POWER_DOWN]	= -1;
	m_anAnimStackIndex[ASI_RC_POWER_UP]		= -1;
	m_anAnimStackIndex[ASI_STOOP]			= -1;

	m_anAnimStackIndex[ASI_AIM_PILLBOX]		= -1;

	m_pnEnableBoneNameIndexTableForSummer_Normal	  =	 m_anEnableBoneNameIndexTableForSummer_Normal;

	// mike start
	m_Damager.pBot = this;
	m_Damager.pWeapon = NULL;
	m_Damager.pEntity = this;
	m_Damager.nDamagerPlayerIndex = -1;
	
	m_BlastDamageReceiverSphere.m_fRadius = _fDamageSphereRadius;
	m_GrabSphere.m_fRadius = _fGrabSuccessSphereRadius;

	m_nSleepState = BOTSLEEPSTATE_NONE;
	m_eState = ZOMBIEBOSS_STANDING_IDLE;
	fang_MemSet(m_pSoundEmitters,0,sizeof(m_pSoundEmitters[0])*ZBOSS_SND_COUNT);
	
	m_mtxBall.Identity();
	m_vBallPosOneAgoWS.Zero();
	m_vBallPosTwoAgoWS.Zero();
	
	m_fReelAngle = 0.0f;
	m_fStateTime = 0.0f;
	m_fUnitAimPitch = 0.0f;
	m_fBallTimeInAir = 0.0f;
	m_fOOBallTimeInAir = 0.0f;
	m_fUnitBallFlight = 0.0f;
	m_fChainLength = _BossBallParams.fVerletPendentLength;
	m_bRoaring = FALSE;
	m_bFreeBalling = TRUE;
	m_bSmoking = FALSE;
	m_uBallCollCountLastFrame = 0;
	m_fTimeUntilNextOuch = 0.0f;
	m_fTimeUntilNextGrunt = 0.0f;
	m_fIsOver = 10.f;
	m_uFightLevelIndex = 0; 
}


void CBotZombieBoss::ClassHierarchyAddToWorld( void )
{
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CBot::ClassHierarchyAddToWorld();
	m_pWorldMesh->UpdateTracker();
	m_pBallMesh->UpdateTracker();

	// ensure that no previous sound handles remain resident
	fang_MemSet(m_pSoundEmitters,0,sizeof(m_pSoundEmitters[0])*ZBOSS_SND_COUNT);
	m_uBotDeathFlags |= BOTDEATHFLAG_PERSISTAFTERDEATH|BOTDEATHFLAG_COLLIDES_AFTER_DEATH;	
	EnableCameraCollision( FALSE );
	SetProjectileReaction(CEntity::PROJECTILE_REACTION_DEFAULT);
	m_bFreeBalling = TRUE;
	m_pBotInfo_MountAim->fMountYawBaseRevsPerSec = m_paFightLevels[m_uFightLevelIndex].fTurnSpeed;
	SetInvincible(FALSE); 

	m_SmokeEmitter1.AddToWorld();
	m_SmokeEmitter1.Relocate_RotXlatFromUnitMtx_WS( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPTack_Smoke01]);
	m_SmokeEmitter1.Attach_ToParent_WS(this,m_apszBoneNameTable[BONE_PTACK_SMOKE01]);
	
	m_SmokeEmitter2.AddToWorld();
	m_SmokeEmitter2.Relocate_RotXlatFromUnitMtx_WS( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPTack_Smoke02]);
	m_SmokeEmitter2.Attach_ToParent_WS(this,m_apszBoneNameTable[BONE_PTACK_SMOKE02]);

	m_SmokeEmitter3.AddToWorld();
	m_SmokeEmitter3.Relocate_RotXlatFromUnitMtx_WS( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPTack_Smoke03]);
	m_SmokeEmitter3.Attach_ToParent_WS(this,m_apszBoneNameTable[BONE_PTACK_SMOKE03]);

	_Smoke(TRUE,0.0f);

	m_vTorsoPtMS = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexTorso]->m_vPos;
	m_vTorsoPtMS.Sub(m_MtxToWorld.m_vPos);
}

void CBotZombieBoss::_Smoke(BOOL bEnable, f32 fIntensity, f32 fDuration)
{
	if (bEnable )
	{
		if (m_bSmoking==FALSE) // turn on
		{
			m_bSmoking = TRUE;
			m_SmokeEmitter1.StartEmission(m_hSmokeParticleDef, fIntensity, fDuration);
			m_SmokeEmitter2.StartEmission(m_hSmokeParticleDef, fIntensity, fDuration);
			m_SmokeEmitter3.StartEmission(m_hSmokeParticleDef, fIntensity, fDuration);
		}
		else// update intensity
		{
			m_SmokeEmitter1.SetUnitIntensity( fIntensity );
			m_SmokeEmitter2.SetUnitIntensity( fIntensity );
			m_SmokeEmitter3.SetUnitIntensity( fIntensity );
		}
	}
	else if (m_bSmoking==TRUE) // turn off
	{
		m_bSmoking = FALSE;
		m_SmokeEmitter1.StopEmission();
		m_SmokeEmitter2.StopEmission();
		m_SmokeEmitter3.StopEmission();
	}
}

void CBotZombieBoss::ClassHierarchyRemoveFromWorld( void )
{
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	for (ZombieBossSnd_e iSndIdx = (ZombieBossSnd_e)0; iSndIdx < ZBOSS_SND_COUNT; iSndIdx = (ZombieBossSnd_e)(int(iSndIdx)+1))
	{
		_PlaySnd(iSndIdx, CBotZombieBoss::STOP);
	}

	m_pWorldMesh->RemoveFromWorld();
	m_pBallMesh->RemoveFromWorld();
	CBot::ClassHierarchyRemoveFromWorld();
}


void CBotZombieBoss::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList)
{
	FASSERT( IsCreated() );
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );

	apTrackerSkipList[nTrackerSkipListCount++] = m_pWorldMesh;
	apTrackerSkipList[nTrackerSkipListCount++] = m_pBallMesh;
}


void CBotZombieBoss::ClassHierarchyWork()
{
 	FASSERT( IsInitialized() );
/*
#if !FANG_PRODUCTION_BUILD
		{
		CFVec3A vGrabPtWS;
		ComputeGrabPointWS(vGrabPtWS);
		CDebugDraw::Sphere(vGrabPtWS, _fGrabSuccessSphereRadius, &FColor_MotifRed);

		CFVec3A vGrabPt2WS=CFVec3A::m_Null;
		_ComputeGrabPointWS(vGrabPt2WS); // debug-only function
		CDebugDraw::Sphere(vGrabPt2WS, _fGrabSuccessSphereRadius, &FColor_MotifBlue);
		}
#endif
*/
		
	CBot::ClassHierarchyWork();

	if( !IsOurWorkBitSet() )
	{
		return;
	}

/*
#if (!FANG_PRODUCTION_BUILD)
	if (m_Interpolator.IsCreated())
		m_Interpolator.DebugDraw(20);
#endif
*/
	m_fTimeUntilNextOuch -= FLoop_fPreviousLoopSecs;
	m_fTimeUntilNextGrunt -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMP_MIN0(m_fTimeUntilNextOuch);
	FMATH_CLAMP_MIN0(m_fTimeUntilNextGrunt);

	Power_Work();

	ParseControls();

	// Save a copy of the previous frame's info...
	m_MountPrevPos_WS = m_MountPos_WS;
	m_nPrevState = m_nState;

	// Update position...
	CFVec3A TempVec3A;
	TempVec3A.Mul( m_Velocity_WS, FLoop_fPreviousLoopSecs );
	m_MountPos_WS.Add( TempVec3A );

	// Handle pitch and yaw...
//	HandlePitchMovement();
	HandleYawMovement();
	
	if (_BallIsOutThere())// Need to cap our yaws to the range that works for ballnchain
	{
		CFVec3A vBossToBallXZ;
		vBossToBallXZ.Sub(m_mtxBall.m_vPos, m_MtxToWorld.m_vPos);
		vBossToBallXZ.y = 0.0f;
		
		f32 fOOMagXZ = vBossToBallXZ.InvMagXZ();
		f32 fMagXZ  = fmath_Inv(fOOMagXZ);
		if ( (fMagXZ > 0.001f) )// no XZ means don't do the following
		{
			m_fMountYaw_WS; // the current facing direction coords -pi to pi

			f32 fMaxRadiansDelta = FMATH_DEG2RAD(80);
			f32 fBossToChainYawWS = fmath_Atan(vBossToBallXZ.x,vBossToBallXZ.z ); // the current chain direction from -pi to pi

			f32 fDeltaYaw = m_fMountYaw_WS - fBossToChainYawWS;
			// normalize delta range 
			if( fDeltaYaw > FMATH_PI ) 
			{
				fDeltaYaw -= FMATH_2PI;
			}
			else if( fDeltaYaw < -FMATH_PI ) 
			{
				fDeltaYaw += FMATH_2PI;
			} 
			if (fDeltaYaw > fMaxRadiansDelta)
				ChangeMountYaw(fBossToChainYawWS + fMaxRadiansDelta);
			else if (fDeltaYaw < -fMaxRadiansDelta)
				ChangeMountYaw(fBossToChainYawWS - fMaxRadiansDelta);
		}
	}
	// receive and process head look
	// commands from entity control
	HandleHeadLook();

	// Rotate model space velocity to account for yaw change...
	WS2MS( m_Velocity_MS, m_Velocity_WS );

	// Update translation analog stick vectors from raw controller data...
	ComputeXlatStickInfo();

	// Collide with the world below our feet...
	PROTRACK_BEGINBLOCK("Coll");
	HandleCollision();
    PROTRACK_ENDBLOCK();

// Move and animate our bot...
	switch( m_nState ) {
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

	PROTRACK_BEGINBLOCK("Weapon");
    HandleTargeting();
	PROTRACK_ENDBLOCK();//"WeaponAnim");

	_HandleAnimations();
	
	m_vBallPosTwoAgoWS = m_vBallPosOneAgoWS;
	m_vBallPosOneAgoWS = m_mtxBall.m_vPos;
	_UpdateMatrices();

	PROTRACK_BEGINBLOCK("XtraColl");

	PROTRACK_ENDBLOCK();
	if ( (m_eState != ZOMBIEBOSS_SWING_ROLL_IN)  &&
		 (m_eState != ZOMBIEBOSS_SWING_TUG_PAUSE) &&
		 (m_eState != ZOMBIEBOSS_SWING_TUG_PULL) &&
		 (m_eState != ZOMBIEBOSS_SWING_TUG_RETURN) &&
		 (m_eState != ZOMBIEBOSS_SWING_ROLL_OUT) &&
		 (m_eState != ZOMBIEBOSS_SWING_RETURN) )
	{
		if (m_fChainLength < _BossBallParams.fVerletPendentLength)
		{
			m_fChainLength += 8.0f * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMAX(m_fChainLength, _BossBallParams.fVerletPendentLength);
		}
		
	}

	_HandleReelAudio();
	_HandleBallAudio();
	_HandleGruntAudio();

	if( IsDeadOrDying() ) 
	{
		DeathWork();
	}
/*
	ftext_Printf(0.60f,0.30f,"DEATH:%1.2f", GetControlValue(ANIMCONTROL_DEATH));
	ftext_Printf(0.60f,0.33f,"FREEZE:%1.2f", GetControlValue(ANIMCONTROL_FREEZE));
	ftext_Printf(0.60f,0.36f,"GRAB_ATTACK:%1.2f", GetControlValue(ANIMCONTROL_GRAB_ATTACK));
	ftext_Printf(0.60f,0.39f,"ROAR_UPPER:%1.2f", GetControlValue(ANIMCONTROL_ROAR_UPPER));
	ftext_Printf(0.60f,0.42f,"TUG_UPPER:%1.2f", GetControlValue(ANIMCONTROL_TUG_UPPER));
	ftext_Printf(0.60f,0.45f,"CHAIN_SWING_UPPER:%1.2f", GetControlValue(ANIMCONTROL_CHAIN_SWING_UPPER));
	ftext_Printf(0.60f,0.48f,"CHAIN_ROLL_UPPER:%1.2f", GetControlValue(ANIMCONTROL_CHAIN_ROLL_UPPER));
	ftext_Printf(0.60f,0.51f,"CHAIN_RETURN_UPPER:%1.2f", GetControlValue(ANIMCONTROL_CHAIN_RETURN_UPPER));
	ftext_Printf(0.60f,0.54f,"WALK:%1.2f", GetControlValue(ANIMCONTROL_WALK));
	ftext_Printf(0.60f,0.57f,"ROAR_LOWER:%1.2f", GetControlValue(ANIMCONTROL_ROAR_LOWER));
	ftext_Printf(0.60f,0.60f,"TUG_LOWER:%1.2f", GetControlValue(ANIMCONTROL_TUG_LOWER));
	ftext_Printf(0.60f,0.63f,"CHAIN_SWING_LOWER:%1.2f", GetControlValue(ANIMCONTROL_CHAIN_SWING_LOWER));
	ftext_Printf(0.60f,0.66f,"CHAIN_ROLL_LOWER:%1.2f", GetControlValue(ANIMCONTROL_CHAIN_ROLL_LOWER));
	ftext_Printf(0.60f,0.69f,"CHAIN_RETURN_LOWER:%1.2f", GetControlValue(ANIMCONTROL_CHAIN_RETURN_LOWER));
*/	

//	CDebugDraw::Line(*m_pApproxEyePoint_WS, m_vHeadLookPoint_WS);
}

void CBotZombieBoss::_UpdateMatrices( void ) 
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

	EntityMtxToWorld.Identity();
	EntityMtxToWorld.SetRotationYXZ( m_fMountYaw_WS, m_fMountPitch_WS, 0.0f );
	EntityMtxToWorld.m_vPos = m_MountPos_WS;
	Relocate_RotXlatFromUnitMtx_WS( &EntityMtxToWorld, TRUE, m_pMoveIdentifier );

	m_pBallMesh->m_Xfm.BuildFromMtx(*m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexBall]);
	m_pBallMesh->UpdateTracker();

	m_GazeUnitVec_WS = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexHead]->m_vFront;

	m_BlastDamageReceiverSphere.m_Pos.Mul(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexHead]->m_vFront.v3,2.0f).Add(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexHead]->m_vPos.v3);
	m_BlastDamageReceiverSphere.m_Pos.y -= 3.0f;

//	CDebugDraw::Sphere(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexAttachGlitch]->m_vPos, 1.25f, &FColor_MotifWhite);
}

void CBotZombieBoss::_AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx )
{
	FASSERT( m_pCollBot );
	CBotZombieBoss *pThisBot = (CBotZombieBoss *)m_pCollBot;

	if ( nBoneIndex == CBotZombieBoss::m_nBoneIndexHead ) 
	{
		pThisBot->HeadLookUpdate( rNewMtx, rParentMtx, rBoneMtx, FALSE );
	}
	if ( nBoneIndex == CBotZombieBoss::m_nBoneIndexChainReel ) 
	{
		CFQuatA ReelQuat;

		while (pThisBot->m_fReelAngle > FMATH_2PI)
			pThisBot->m_fReelAngle -= FMATH_2PI;
		while (pThisBot->m_fReelAngle < 0.0f)
			pThisBot->m_fReelAngle += FMATH_2PI;

		ReelQuat.BuildQuat(rBoneMtx.m_vRight, pThisBot->m_fReelAngle);
		
		CFMtx43A NewBoneMtx;
		NewBoneMtx.m_vPos = rBoneMtx.m_vPos;
		ReelQuat.MulPoint( NewBoneMtx.m_vRight, rBoneMtx.m_vRight );
		ReelQuat.MulPoint( NewBoneMtx.m_vUp,	rBoneMtx.m_vUp );
		ReelQuat.MulPoint( NewBoneMtx.m_vFront, rBoneMtx.m_vFront );
		
		rNewMtx.Mul( rParentMtx, NewBoneMtx );
	}
	else if ( nBoneIndex == CBotZombieBoss::m_nBoneIndexTorso )
	{
		CFQuatA TorsoYawQuat,TorsoPitchQuat;

		// TorsoYaw
		f32 fLegsYaw = pThisBot->m_fLegsYaw_MS;
		TorsoYawQuat.BuildQuat(rBoneMtx.m_vUp, -fLegsYaw);
		if( !pThisBot->IsReverseFacingForward() ) 
		{
			// Rotate torso when in reverse facing mode...
			CFQuatA RevQuat;
			RevQuat.BuildQuat( rBoneMtx.m_vUp, fmath_UnitLinearToSCurve( pThisBot->m_fReverseFacingYaw_MS * ( 1.0f / FMATH_DEG2RAD( 180.0f ) ) ) * FMATH_DEG2RAD( 180.0f ) );
			TorsoYawQuat.Mul( RevQuat );
		}
/*
#if !FANG_PRODUCTION_BUILD
		CFVec3A start,end;
		start = pThisBot->m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexTorso]->m_vPos;
		end = pThisBot->m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexTorso]->m_vFront;
		end.Mul(100.f);
		end.Add(start);
		CDebugDraw::Line(start,end);
#endif 
*/		
		FMATH_CLAMP(pThisBot->m_fAimPitch_WS, m_BotInfo_MountAim.fMountPitchUpLimit, m_BotInfo_MountAim.fMountPitchDownLimit);
		f32 fBonesPitchWS = -fmath_Atan( rBoneMtx.m_vFront.y, rBoneMtx.m_vFront.MagXZ() );
		TorsoPitchQuat.BuildQuat( rBoneMtx.m_vRight, pThisBot->m_fAimPitch_WS - fBonesPitchWS);

		m_TorsoQuat.Mul(TorsoYawQuat,TorsoPitchQuat);

		CFMtx43A NewBoneMtx;
		NewBoneMtx.m_vPos = rBoneMtx.m_vPos;
		m_TorsoQuat.MulPoint( NewBoneMtx.m_vRight,	rBoneMtx.m_vRight );
		m_TorsoQuat.MulPoint( NewBoneMtx.m_vUp,		rBoneMtx.m_vUp );
		m_TorsoQuat.MulPoint( NewBoneMtx.m_vFront,	rBoneMtx.m_vFront );
		
		rNewMtx.Mul( rParentMtx, NewBoneMtx );
	}
	else if ( nBoneIndex == CBotZombieBoss::m_nBoneIndexBall )
	{
		if ( (pThisBot->m_eState == ZOMBIEBOSS_MOVING_IDLE) || (pThisBot->m_bFreeBalling==FALSE) )
		{
			CFMtx43A NewBoneMtx;
			m_TorsoQuat.MulPoint( NewBoneMtx.m_vRight, rBoneMtx.m_vRight );
			m_TorsoQuat.MulPoint( NewBoneMtx.m_vUp, rBoneMtx.m_vUp );
			m_TorsoQuat.MulPoint( NewBoneMtx.m_vFront, rBoneMtx.m_vFront );
			m_TorsoQuat.MulPoint( NewBoneMtx.m_vPos, rBoneMtx.m_vPos );

			rNewMtx.Mul( rParentMtx, NewBoneMtx);
			
//			CDebugDraw::Sphere(rNewMtx.m_vPos, .5f);

			if (pThisBot->m_bFreeBalling) // implies moving idle
			{
				if (pThisBot->m_pBallMesh->GetBoundingSphere().IsIntersecting(rNewMtx.m_vPos.v3) && pThisBot->Power_IsPoweredUp())
				{
					pThisBot->m_bFreeBalling = FALSE;
				}
				else // if you ARE freeballing, the put the ball on m_mtxBall
				{
					rNewMtx = m_mtxBall;
				}
			}
			if (pThisBot->m_bFreeBalling == FALSE) // if not freeballing, the ball matches anim
			{
				m_mtxBall = rNewMtx;
			}
		}
		else // if you ARE freeballing, the put the ball on m_mtxBall
		{
			rNewMtx = m_mtxBall;
		}
		
		m_vChainUnitDirection.Sub(pThisBot->m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexAttachBall]->m_vPos,m_mtxBall.m_vPos);
		m_vChainUnitDirection.Unitize();
		m_vChainLengthAndDirection.Mul(m_vChainUnitDirection,m_fChainLinkLength);
		m_vChainNextLinkPositionWS.Set(m_mtxBall.m_vPos);
		m_nCurrentLinkIndex=0;
		
		pThisBot->m_fChainLength = pThisBot->m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexAttachBall]->m_vPos.Dist(rNewMtx.m_vPos);
		pThisBot->m_mChainLinks.UnitMtxFromUnitVec(&m_vChainUnitDirection);
		FMATH_CLAMPMAX(pThisBot->m_fChainLength, _fMaxChainLength);
	}
	else if ( nBoneIndex == CBotZombieBoss::m_nBoneIndexHead ) 
	{
		rNewMtx.Mul( rParentMtx, rBoneMtx );
	}
	else if ( nBoneIndex == CBotZombieBoss::m_nBoneIndexAttachBall ) 
	{
		rNewMtx.Mul( rParentMtx, rBoneMtx );
		if (pThisBot->m_bFreeBalling) 
			pThisBot->_ComputeBallMatrix();
	}
	else if ( nBoneIndex == CBotZombieBoss::m_nBoneIndexLHand )
	{
		rNewMtx.Mul( rParentMtx, rBoneMtx );
		if (pThisBot->m_eState == ZOMBIEBOSS_GRAB_SUCCESS)
		{
			FASSERT(m_pGrabbedBot);
			CFVec3A vGlitchHead;
			CFVec3A vUnitToHead;
            vGlitchHead = m_pGrabbedBot->m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexGrabbedBotsHead]->m_vPos;
			vUnitToHead.Sub(vGlitchHead, rNewMtx.m_vPos);
			if (vUnitToHead.MagSq() > 0.0001f)
			{
				vUnitToHead.Unitize();
				rNewMtx.UnitMtxFromUnitVecPreserveAxes(&vUnitToHead);
				
//				CFVec3A Endpt = vUnitToHead;
//				Endpt.Mul(10.0f);
//				Endpt.Add(rNewMtx.m_vPos);
//				CDebugDraw::Line(rNewMtx.m_vPos, Endpt ); 
			}
		}
	}
	else  // all bones not specifically handled MUST be link bones
	{
		FASSERT(fclib_strncmp(pThisBot->m_pWorldMesh->m_pMesh->pBoneArray[nBoneIndex].szName,"Chain_Link",10)==0);
		
		s32 nNumWholeLinks = s32(pThisBot->m_fChainLength*m_fOOChainLinkLength);
		if (m_nCurrentLinkIndex < nNumWholeLinks)
		{
			rNewMtx = pThisBot->m_mChainLinks;
			rNewMtx.m_vPos = m_vChainNextLinkPositionWS;
			m_vChainNextLinkPositionWS.Add(m_vChainLengthAndDirection);

//			CDebugDraw::Sphere(rNewMtx.m_vPos, .44f, &FColor_MotifWhite);
		}
		else if (m_nCurrentLinkIndex == nNumWholeLinks) // the partial
		{
			
			f32 fLeftOver = ((f32)(nNumWholeLinks+1))*m_fChainLinkLength - pThisBot->m_fChainLength;
			CFVec3A vChainNextLinkAdj;
			vChainNextLinkAdj.Mul(m_vChainUnitDirection,-fLeftOver);
			m_vChainNextLinkPositionWS.Add(vChainNextLinkAdj);
			rNewMtx = pThisBot->m_mChainLinks;
			rNewMtx.m_vPos = m_vChainNextLinkPositionWS;

//			CDebugDraw::Sphere(rNewMtx.m_vPos, .44f,&FColor_MotifRed);
//			CDebugDraw::Sphere(m_vChainNextLinkPositionWS, .50f,&FColor_MotifGreen);
//			m_vChainNextLinkPositionWS.Add(m_vChainLengthAndDirection);
			//			CDebugDraw::Sphere(m_vChainNextLinkPositionWS, .50f,&FColor_MotifRed);
		}
		else
		{
			rNewMtx.m_vPos.Set(1000000.0f,1000000.0f,1000000.0f);
		}
		if (m_nCurrentLinkIndex==0)
		{
			CFVec3A chainPtMS;
			CFSphere newBounds;
			chainPtMS.Sub(rNewMtx.m_vPos,pThisBot->m_MountPos_WS);
			newBounds.Set(pThisBot->m_NormalSphereMS.m_Pos,pThisBot->m_NormalSphereMS.m_fRadius);
			SphereUnion(newBounds,chainPtMS.v3,m_fChainLinkLength);
			pThisBot->m_pWorldMesh->m_BoundSphere_MS.Set(newBounds.m_Pos,newBounds.m_fRadius);
		}
		m_nCurrentLinkIndex++;
	}
}

void CBotZombieBoss::_ComputeBallMatrix( void )
{
	if (Power_IsPoweredUp())
	{
		switch(m_eState)
		{
		case ZOMBIEBOSS_STANDING_IDLE:
		case ZOMBIEBOSS_STANDING_INTO_MOVING:
		case ZOMBIEBOSS_STANDING_INTO_GRAB:
		case ZOMBIEBOSS_MOVING_IDLE:
		case ZOMBIEBOSS_MOVING_INTO_STANDING:
		case ZOMBIEBOSS_MOVING_INTO_SWING:
		case ZOMBIEBOSS_SWING_PREPARE:
		case ZOMBIEBOSS_SWING_INTO_IDLE:
		case ZOMBIEBOSS_SWING_RETURN:
		case ZOMBIEBOSS_GRAB_START:
		case ZOMBIEBOSS_GRAB_FAILURE:
		case ZOMBIEBOSS_GRAB_SUCCESS:
		case ZOMBIEBOSS_GRAB_PICKUP:
		case ZOMBIEBOSS_GRAB_ROAR:
		case ZOMBIEBOSS_GRAB_THROWN:
		case ZOMBIEBOSS_GRAB_INTO_STANDING:
			// Ball is free fall
			_ComputeVerletPosition();
			break;

		case ZOMBIEBOSS_SWING:
		case ZOMBIEBOSS_SWING_ROLL_OUT:
			// Ball is splined
			_FlyBallFly(m_mtxBall.m_vPos);
			m_fReelAngle += _fReelVelocity * FLoop_fPreviousLoopSecs;
			break;

		case ZOMBIEBOSS_SWING_TUG_PAUSE:
		case ZOMBIEBOSS_SWING_TUG_RETURN:
		case ZOMBIEBOSS_INTO_DEATH:
			if (m_uBallCollCountLastFrame == 0)
				_ComputeVerletPosition();
			break;

		case ZOMBIEBOSS_SWING_TUG_PULL:
			{	// ball is tugged
				CFVec3A vAttachPtToBall;
				vAttachPtToBall.ReceiveNegative(m_vChainUnitDirection); //  Sub(m_mtxBall.m_vPos,m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexAttachBall]->m_vPos);
				// vAttachPtToBall.Unitize();
				f32 fNewChainLength = m_fChainLength- m_paFightLevels[m_uFightLevelIndex].fBallReturnSpeed * FLoop_fPreviousLoopSecs;
				fNewChainLength = FMATH_MAX(fNewChainLength, _BossBallParams.fVerletPendentLength);
				vAttachPtToBall.Mul(fNewChainLength);
				vAttachPtToBall.Add(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexAttachBall]->m_vPos);

				m_mtxBall.m_vPos = vAttachPtToBall;

				if (m_uBallCollCountLastFrame == 0)
					_ComputeVerletPosition();
				break;
			}

		case ZOMBIEBOSS_SWING_ROLL_IN:
			{	// ball is returning
				f32 fDeltaChain  = m_paFightLevels[m_uFightLevelIndex].fBallReturnSpeed * FLoop_fPreviousLoopSecs;
				m_fChainLength -= fDeltaChain;

				CFVec3A vToAttachPt;
				vToAttachPt.Sub(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexAttachBall]->m_vPos,m_mtxBall.m_vPos);
				vToAttachPt.Unitize();
				vToAttachPt.Mul(fDeltaChain);

				m_mtxBall.m_vPos.Add(vToAttachPt);

				if (m_uBallCollCountLastFrame == 0)
					_ComputeVerletPosition();
				m_fReelAngle -= _fReelVelocity * FLoop_fPreviousLoopSecs;
				break;
			}
		} // end switch (m_eState)
	}
	else
	{
		if (m_uBallCollCountLastFrame == 0)
			_ComputeVerletPosition();
	}
	m_mtxBall.UnitMtxFromUnitVecPreserveAxes(&m_vChainUnitDirection);
	
	// Now, we've chosen a new, good ball matrix, let's collidathon it...
//	CFSphere CollSphere;
//	CollSphere.m_fRadius = m_pBallMesh->GetBoundingSphere().m_fRadius;
//	CollSphere.m_Pos = m_mtxBall.m_vPos.v3;

	FCollImpact_t CollImpact;
	if (_HandleBallCollision(CollImpact)) // we're in contact with something
	{
		switch(m_eState)
		{
		case ZOMBIEBOSS_STANDING_IDLE:
		case ZOMBIEBOSS_STANDING_INTO_MOVING:
		case ZOMBIEBOSS_STANDING_INTO_GRAB:
		case ZOMBIEBOSS_MOVING_INTO_STANDING:
		case ZOMBIEBOSS_MOVING_INTO_SWING:
		case ZOMBIEBOSS_SWING_PREPARE:
		case ZOMBIEBOSS_SWING_INTO_IDLE:
		case ZOMBIEBOSS_SWING_RETURN:
		case ZOMBIEBOSS_SWING_TUG_PAUSE:
		case ZOMBIEBOSS_SWING_TUG_RETURN:
		case ZOMBIEBOSS_GRAB_START:
		case ZOMBIEBOSS_GRAB_FAILURE:
		case ZOMBIEBOSS_GRAB_SUCCESS:
		case ZOMBIEBOSS_GRAB_PICKUP:
		case ZOMBIEBOSS_GRAB_ROAR:
		case ZOMBIEBOSS_GRAB_THROWN:
		case ZOMBIEBOSS_GRAB_INTO_STANDING:
			m_mtxBall.m_vPos.y = FMATH_MAX(m_mtxBall.m_vPos.y,CollImpact.ImpactPoint.y + m_pBallMesh->GetBoundingSphere().m_fRadius);
			break;

		case ZOMBIEBOSS_SWING:
		case ZOMBIEBOSS_SWING_ROLL_OUT:
			m_mtxBall.m_vPos.y = FMATH_MAX(m_mtxBall.m_vPos.y,CollImpact.ImpactPoint.y + m_pBallMesh->GetBoundingSphere().m_fRadius);
			if ( ( (m_eState==ZOMBIEBOSS_SWING) && (m_fUnitBallFlight > 0.1f) ) ||		// we must be swinging
				(m_eState==ZOMBIEBOSS_SWING_ROLL_OUT) )// we must be swinging
			{
				_BallAttackCollides(&CollImpact);
			}

			break;

		case ZOMBIEBOSS_SWING_TUG_PULL:
		case ZOMBIEBOSS_SWING_ROLL_IN:
			m_mtxBall.m_vPos.y = FMATH_MAX(m_mtxBall.m_vPos.y,CollImpact.ImpactPoint.y + m_pBallMesh->GetBoundingSphere().m_fRadius);
			_BallReturnCollides(&CollImpact);
			break;

		} // end switch (m_eState)
	}
}

/*
#if !FANG_PRODUCTION_BUILD
switch(m_eState)
	{
	case ZOMBIEBOSS_STANDING_IDLE:
		ftext_Printf(.33f,.24f,"%s", "ZOMBIEBOSS_STANDING_IDLE");
		break;
	case ZOMBIEBOSS_STANDING_INTO_MOVING:
		ftext_Printf(.33f,.24f,"%s", "ZOMBIEBOSS_STANDING_INTO_MOVING");
		break;
	case ZOMBIEBOSS_STANDING_INTO_GRAB:
		ftext_Printf(.33f,.24f,"%s", "ZOMBIEBOSS_STANDING_INTO_GRAB");
		break;
	case ZOMBIEBOSS_MOVING_INTO_STANDING:
		ftext_Printf(.33f,.24f,"%s", "ZOMBIEBOSS_MOVING_INTO_STANDING");
		break;
	case ZOMBIEBOSS_MOVING_INTO_SWING:
		ftext_Printf(.33f,.24f,"%s", "ZOMBIEBOSS_MOVING_INTO_SWING");
		break;
	case ZOMBIEBOSS_SWING_PREPARE:
		ftext_Printf(.33f,.24f,"%s", "ZOMBIEBOSS_SWING_PREPARE");
		break;
	case ZOMBIEBOSS_SWING_INTO_IDLE:
		ftext_Printf(.33f,.24f,"%s", "ZOMBIEBOSS_SWING_INTO_IDLE");
		break;
	case ZOMBIEBOSS_SWING_RETURN:
		ftext_Printf(.33f,.24f,"%s", "ZOMBIEBOSS_SWING_RETURN");
		break;
	case ZOMBIEBOSS_GRAB_START:
		ftext_Printf(.33f,.24f,"%s", "ZOMBIEBOSS_GRAB_START");
		break;
	case ZOMBIEBOSS_GRAB_FAILURE:
		ftext_Printf(.33f,.24f,"%s", "ZOMBIEBOSS_GRAB_FAILURE");
		break;
	case ZOMBIEBOSS_GRAB_SUCCESS:
		ftext_Printf(.33f,.24f,"%s", "ZOMBIEBOSS_GRAB_SUCCESS");
		break;
	case ZOMBIEBOSS_GRAB_GRABBED:
		ftext_Printf(.33f,.24f,"%s", "ZOMBIEBOSS_GRAB_GRABBED");
		break;
	case ZOMBIEBOSS_GRAB_THROWN:
		ftext_Printf(.33f,.24f,"%s", "ZOMBIEBOSS_GRAB_THROWN");
		break;
	case ZOMBIEBOSS_GRAB_INTO_STANDING:
		ftext_Printf(.33f,.24f,"%s", "ZOMBIEBOSS_GRAB_INTO_STANDING");
		break;
	case ZOMBIEBOSS_SWING:
		ftext_Printf(.33f,.24f,"%s", "ZOMBIEBOSS_SWING");
		break;
	case ZOMBIEBOSS_SWING_ROLL_OUT:
		ftext_Printf(.33f,.24f,"%s", "ZOMBIEBOSS_SWING_ROLL_OUT");
		break;
	case ZOMBIEBOSS_SWING_TUG_PAUSE:
		ftext_Printf(.33f,.24f,"%s", "ZOMBIEBOSS_SWING_TUG_PAUSE");
		break;
	case ZOMBIEBOSS_SWING_TUG_PULL:
		ftext_Printf(.33f,.24f,"%s", "ZOMBIEBOSS_SWING_TUG_PULL");
		break;
	case ZOMBIEBOSS_SWING_TUG_RETURN:
		ftext_Printf(.33f,.24f,"%s", "ZOMBIEBOSS_SWING_TUG_RETURN");
		break;
	case ZOMBIEBOSS_SWING_ROLL_IN:
		ftext_Printf(.33f,.24f,"%s", "ZOMBIEBOSS_SWING_ROLL_IN");
		break;	
	default:
	ftext_Printf(.33f,.24f,"%s", "XXXXXXXXXXXXXXXX");
		break;	
	}
#endif
//
}
*/
void CBotZombieBoss::_HandleAnimations( void )
{
	if (Power_IsPoweredDown())
	{
		if (m_pGrabbedBot)
		{
			((CBotGlitch*)m_pGrabbedBot)->DisableGrabbedByHead();
			m_pGrabbedBot = NULL;
			m_eState = ZOMBIEBOSS_GRAB_FAILURE;
		}
		return;
	}
		
	BOOL bTryToSwing = FALSE;
	BOOL bTryToGrab = FALSE;
	
	// button is pressed, and we're not cabling
	if (m_fControls_Fire1 > 0.1f)
	{
		bTryToSwing = TRUE;
	}
	else if (m_fControls_Fire2 > 0.1f)
	{
		bTryToGrab = TRUE;
	}
	
	m_fStateTime += FLoop_fPreviousLoopSecs;
	f32 fUnitControl = 0.f;
	switch (m_eState)
 	{
	case ZOMBIEBOSS_STANDING_IDLE:
		if (bTryToSwing)
		{
			UpdateUnitTime(ANIMTAP_CHAIN_SWING_UPPER, 0.0f);
			m_eState = ZOMBIEBOSS_STANDING_INTO_MOVING;
			m_fStateTime = 0.0f;
		}
		if (bTryToGrab)
		{
			UpdateUnitTime(ANIMTAP_GRAB_ATTACK, 0.0f);
			m_eState = ZOMBIEBOSS_STANDING_INTO_GRAB;
			m_fStateTime = 0.0f;
		}
	
		// Automatic Idle code takes care of this for us 
		// DeltaTime(ANIMTAP_STAND_LOOK,FLoop_fPreviousLoopSecs);
		break;

	case ZOMBIEBOSS_STANDING_INTO_MOVING:
		DeltaTime(ANIMTAP_STAND_SWING,FLoop_fPreviousLoopSecs);
		fUnitControl = m_fStateTime * _fOOLookToMovingTime;
		if (fUnitControl <= 1.0f)
            SetControlValue(ANIMCONTROL_STAND_SWING, fUnitControl);
		else
		{
			SetControlValue(ANIMCONTROL_STAND_SWING, 1.0f);
			m_eState = ZOMBIEBOSS_MOVING_IDLE;
			m_fStateTime = 0.0f;
		}
		break;

	case ZOMBIEBOSS_MOVING_INTO_STANDING:
		DeltaTime(ANIMTAP_STAND_SWING,FLoop_fPreviousLoopSecs);
		fUnitControl = (1.0f - m_fStateTime * _fOOMovingToLookTime);
		if (fUnitControl >= 0.0f)
            SetControlValue(ANIMCONTROL_STAND_SWING, fUnitControl);
		else
		{
			SetControlValue(ANIMCONTROL_STAND_SWING, 0.0f);
			m_eState = ZOMBIEBOSS_STANDING_IDLE;
			m_fStateTime = 0.0f;
		}
		break;

	case ZOMBIEBOSS_MOVING_IDLE:
		if (bTryToSwing)
		{
			m_bFreeBalling = TRUE;
			UpdateUnitTime(ANIMTAP_CHAIN_SWING_UPPER, 0.0f);
			m_eState = ZOMBIEBOSS_MOVING_INTO_SWING;
			m_fStateTime = 0.0f;
		}
		else if (bTryToGrab)
		{
			m_bFreeBalling = TRUE;
			UpdateUnitTime(ANIMTAP_STAND_LOOK, 0.5f); // 0.5, looking ahead I estimate.
			m_eState = ZOMBIEBOSS_MOVING_INTO_STANDING;
			m_fStateTime = 0.0f;
		}
		DeltaTime(ANIMTAP_STAND_SWING,FLoop_fPreviousLoopSecs);
		break;

	case ZOMBIEBOSS_MOVING_INTO_SWING:
		fUnitControl = 3.0f * m_fStateTime * _fOOMovingToSwingTime;
		FMATH_CLAMP_UNIT_FLOAT(fUnitControl);
		SetControlValue( ANIMCONTROL_CHAIN_SWING_UPPER, fUnitControl );
		SetControlValue( ANIMCONTROL_CHAIN_SWING_LOWER, fUnitControl );
		if (m_fStateTime >= _fMovingToSwingTime)
		{
			SetControlValue( ANIMCONTROL_CHAIN_SWING_UPPER, 1.0f ); // should already be here
			SetControlValue( ANIMCONTROL_CHAIN_SWING_LOWER, 1.0f ); // should already be here
			m_eState = ZOMBIEBOSS_SWING_PREPARE;
			m_fStateTime = 0.0f;
		}
		break;

	case ZOMBIEBOSS_SWING_PREPARE:
		if (GetUnitTime(ANIMTAP_CHAIN_SWING_UPPER) < .44f)
		{
			DeltaTime( ANIMTAP_CHAIN_SWING_UPPER, FLoop_fPreviousLoopSecs, TRUE );
		}
		else
		{
			CFVec3A vToBall;
			vToBall.ReceiveNegative(m_vChainUnitDirection); //  Sub(m_mtxBall.m_vPos,m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexAttachBall]->m_vPos);
			// vToBall.UnitizeXZ(); // not necessary when check sign, put in during a debug session
			f32 fCosFrontAndToBallXZ = vToBall.x * m_MountUnitFrontXZ_WS.x + vToBall.z * m_MountUnitFrontXZ_WS.z;
			if ( (fCosFrontAndToBallXZ >= 0) ||// the ball's in front of us
				 (m_vBallPosOneAgoWS.y-m_mtxBall.m_vPos.y > 0.4f) )
			{}
			else // the balls behind the chain pt, and swing will look a bit better 
			{
				_SetUpBallPath();
				m_eState = ZOMBIEBOSS_SWING;
				m_fStateTime = 0.0f;
				PlaySound(m_pSounds[ZBOSS_SND_BALL_SWOOSH]);
			}
		}
		break;

	case ZOMBIEBOSS_SWING:
		if(!DeltaTime( ANIMTAP_CHAIN_SWING_UPPER, FLoop_fPreviousLoopSecs, TRUE )) 
		{
		}
		else
		{
			UpdateUnitTime(ANIMTAP_CHAIN_RETURN_UPPER, 0.0f);
			SetControlValue( ANIMCONTROL_CHAIN_SWING_UPPER, 0.0f ); 
			SetControlValue( ANIMCONTROL_CHAIN_SWING_LOWER, 0.0f ); 
			SetControlValue( ANIMCONTROL_CHAIN_ROLL_UPPER,  1.0f ); 
			SetControlValue( ANIMCONTROL_CHAIN_ROLL_LOWER,  1.0f ); 
			m_eState = ZOMBIEBOSS_SWING_ROLL_OUT;
		}
		break;

	case ZOMBIEBOSS_SWING_ROLL_OUT:
		if(!DeltaTime( ANIMTAP_CHAIN_ROLL_UPPER, FLoop_fPreviousLoopSecs, FALSE )) 
		{
		}
		else
		{
		}
		break;

	case ZOMBIEBOSS_SWING_TUG_PAUSE:
		if (m_fStateTime < _fTugPauseTime)
		{
			fUnitControl = m_fStateTime * _fOOTugPauseTime;
			f32 OneMinusfUnitControl = 1.0f - fUnitControl;
			SetControlValue(ANIMCONTROL_TUG_UPPER, fUnitControl);
			SetControlValue(ANIMCONTROL_TUG_LOWER, fUnitControl);
			SetControlValue( ANIMCONTROL_CHAIN_SWING_UPPER, OneMinusfUnitControl); 
			SetControlValue( ANIMCONTROL_CHAIN_SWING_LOWER, OneMinusfUnitControl); 

		}
		else
		{
			SetControlValue( ANIMCONTROL_CHAIN_SWING_UPPER, 0.0f ); 
			SetControlValue( ANIMCONTROL_CHAIN_SWING_LOWER, 0.0f ); 
			SetControlValue(ANIMCONTROL_TUG_UPPER, 1.f);
			SetControlValue(ANIMCONTROL_TUG_LOWER, 1.f);
			ZeroTime(ANIMTAP_TUG_UPPER);
			m_fStateTime = 0.0f;
			m_eState = ZOMBIEBOSS_SWING_TUG_PULL;
		}

		break;

	case ZOMBIEBOSS_SWING_TUG_PULL:
		DeltaTime( ANIMTAP_TUG_UPPER, FLoop_fPreviousLoopSecs * _fTugPullSpeedMult, FALSE );
		if (GetUnitTime(ANIMTAP_TUG_UPPER) > .50f)
		{
			m_fStateTime = 0.0f;
			m_eState = ZOMBIEBOSS_SWING_TUG_RETURN;
		}
		break;

	case ZOMBIEBOSS_SWING_TUG_RETURN:
		if(!DeltaTime( ANIMTAP_TUG_UPPER, FLoop_fPreviousLoopSecs, FALSE )) 
		{
		}
		else
		{
			SetControlValue(ANIMCONTROL_TUG_UPPER, 0.f);
			SetControlValue(ANIMCONTROL_TUG_LOWER, 0.f);
			SetControlValue( ANIMCONTROL_CHAIN_ROLL_UPPER,  1.0f ); 
			SetControlValue( ANIMCONTROL_CHAIN_ROLL_LOWER,  1.0f ); 
			m_fStateTime = 0.0f;
			m_eState = ZOMBIEBOSS_SWING_ROLL_IN;
		}
		break;


	case ZOMBIEBOSS_SWING_ROLL_IN:
		if ( (m_fStateTime > _fTimeBetweenTugs) && (m_fChainLength >= 39.0f ) ) //pendent(14) + maxdraglength(25)
		{
			SetControlValue(ANIMCONTROL_TUG_UPPER, 1.f);
			SetControlValue(ANIMCONTROL_TUG_LOWER, 1.f);
			ZeroTime(ANIMTAP_TUG_UPPER);
			m_fStateTime = 0.0f;
			m_eState = ZOMBIEBOSS_SWING_TUG_PULL;
		}
		else if (m_fChainLength > _BossBallParams.fVerletPendentLength)
		{
			DeltaTime( ANIMTAP_CHAIN_ROLL_UPPER, -FLoop_fPreviousLoopSecs, FALSE );
		}
		else
		{
			SetControlValue( ANIMCONTROL_CHAIN_ROLL_UPPER,  0.0f ); 
			SetControlValue( ANIMCONTROL_CHAIN_ROLL_LOWER,  0.0f ); 
			SetControlValue( ANIMCONTROL_CHAIN_RETURN_UPPER,  1.0f ); 
			SetControlValue( ANIMCONTROL_CHAIN_RETURN_LOWER,  1.0f ); 
			ZeroTime(ANIMTAP_CHAIN_RETURN_UPPER);
			m_eState = ZOMBIEBOSS_SWING_RETURN;
			m_fStateTime = 0.0f;
		}
		break;

	case ZOMBIEBOSS_SWING_RETURN:
		if (!DeltaTime( ANIMTAP_CHAIN_RETURN_UPPER, FLoop_fPreviousLoopSecs, TRUE )) 
		{
		}
		else
		{
			UpdateUnitTime( ANIMTAP_STAND_SWING, 0.0f);
			m_eState = ZOMBIEBOSS_SWING_INTO_IDLE;
			m_fStateTime = 0.0f;
		}
		break;
	case ZOMBIEBOSS_SWING_INTO_IDLE:
		if (m_fStateTime < _fSwingToMovingTime)
		{
			fUnitControl = m_fStateTime * _fOOSwingToMovingTime;
			FMATH_CLAMP_UNIT_FLOAT(fUnitControl);
			SetControlValue( ANIMCONTROL_CHAIN_RETURN_UPPER,  1.0f - fUnitControl ); 
			SetControlValue( ANIMCONTROL_CHAIN_RETURN_LOWER,  1.0f - fUnitControl ); 
			SetControlValue( ANIMCONTROL_STAND_SWING, fUnitControl );
			DeltaTime(ANIMTAP_STAND_SWING, FLoop_fPreviousLoopSecs*fUnitControl);
		}
		else
		{
			SetControlValue( ANIMCONTROL_CHAIN_RETURN_UPPER,  0.0f ); 
			SetControlValue( ANIMCONTROL_CHAIN_RETURN_LOWER,  0.0f ); 
			SetControlValue( ANIMCONTROL_STAND_SWING, 1.0f ); // should already be here
			m_eState = ZOMBIEBOSS_MOVING_IDLE;
			m_fStateTime = 0.0f;
		}
		break;
	
	case ZOMBIEBOSS_STANDING_INTO_GRAB:
		fUnitControl = (m_fStateTime * _fOOLookToGrabbingTime);
		if (fUnitControl < 1.0f)
            SetControlValue(ANIMCONTROL_GRAB_ATTACK, fUnitControl);
		else
		{
			SetControlValue(ANIMCONTROL_GRAB_ATTACK, 1.0f);
			m_eState = ZOMBIEBOSS_GRAB_START;
			m_fStateTime = 0.0f;
		}
		break;

	case ZOMBIEBOSS_GRAB_START:
		DeltaTime(ANIMTAP_GRAB_ATTACK,FLoop_fPreviousLoopSecs);
		m_fStateTime = GetTime(ANIMTAP_GRAB_ATTACK);

		if (m_fStateTime > _fTimeOfGrabTest) // decide the next state
		{
			DeltaTime(ANIMTAP_GRAB_ATTACK,-FLoop_fPreviousLoopSecs);

			CFVec3A vGrabPtWS;
			ComputeGrabPointWS(vGrabPtWS);
			m_GrabSphere.m_Pos = vGrabPtWS.v3;
			
			fcoll_Clear();
			FWorld_nTrackerSkipListCount = 0;
			m_pGrabbedBot = NULL;
			this->AppendTrackerSkipList();
			m_pCollBot = this;
			fworld_FindTrackersIntersectingSphere(	&m_GrabSphere,
				FWORLD_TRACKERTYPE_MESH,
				_ZombieGrabCheck,
				FWorld_nTrackerSkipListCount,
				FWorld_apTrackerSkipList,
				m_pWorldMesh,
				MESHTYPES_ENTITY,
				ENTITY_BIT_BOTGLITCH);
			
			if (m_pGrabbedBot)
			{
				m_nBoneIndexGrabbedBotsHead = m_pGrabbedBot->m_pWorldMesh->FindBone("Head");
				m_eState = ZOMBIEBOSS_GRAB_SUCCESS;
			}
			else
			{
				m_eState = ZOMBIEBOSS_GRAB_FAILURE;
			}
		}
		break;

	case ZOMBIEBOSS_GRAB_FAILURE:
		{
		DeltaTime(ANIMTAP_GRAB_ATTACK,FLoop_fPreviousLoopSecs);
		f32 m_fStateTime = GetTime(ANIMTAP_GRAB_ATTACK);
		if (m_fStateTime > _fTimeOfGrabPickup) // decide the next state
		{
			fUnitControl = GetControlValue(ANIMCONTROL_GRAB_ATTACK);
			fUnitControl -= (FLoop_fPreviousLoopSecs * _fOOGrabFailureToLookTime);
			if (fUnitControl >= 0.0f)
				SetControlValue(ANIMCONTROL_GRAB_ATTACK, fUnitControl);
			else
			{	
				SetControlValue(ANIMCONTROL_GRAB_ATTACK, 0.0f);
				m_eState = ZOMBIEBOSS_STANDING_IDLE;
				m_fStateTime = 0.0f;
			}
		}
		}
		break;

	case ZOMBIEBOSS_GRAB_SUCCESS:
		DeltaTime(ANIMTAP_GRAB_ATTACK,FLoop_fPreviousLoopSecs*_fGrabSpeedMult);
		m_fStateTime = GetTime(ANIMTAP_GRAB_ATTACK);
		if (m_fStateTime > _fTimeOfGrabPickup) // decide the next state
		{
			m_eState = ZOMBIEBOSS_GRAB_PICKUP;
			FASSERT(m_pGrabbedBot);
			if (m_pGrabbedBot->TypeBits() & ENTITY_BIT_BOTGLITCH)
			{
				((CBotGlitch*)m_pGrabbedBot)->SetGrabbedByHead(this,m_apszBoneNameTable[BONE_ATTACHPOINT_GLITCH]);
				CFSoundGroup::PlaySound(m_pSounds[ZBOSS_SND_GRAB_CONTACT]);
			}
		}
		
		break;
	case ZOMBIEBOSS_GRAB_PICKUP:
		m_fStateTime = GetTime(ANIMTAP_GRAB_ATTACK);
		if (m_fStateTime >= 2.8f) // decide the next state
		{
			m_eState = ZOMBIEBOSS_GRAB_ROAR;
			m_pSoundEmitters[ZBOSS_SND_ROAR] = CFSoundGroup::AllocAndPlaySound(m_pSounds[ZBOSS_SND_ROAR],TRUE);
			m_FlameEmitter1.AddToWorld();
			m_FlameEmitter1.Relocate_RotXlatFromUnitMtx_WS( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPTack_Head]);
			m_FlameEmitter1.Attach_ToParent_WS(this,m_apszBoneNameTable[BONE_PTACK_HEAD]);
			m_FlameEmitter1.StartEmission(m_hBreathParticleDef,1.0f,3.0f);
		}
		else
		{
			DeltaTime(ANIMTAP_GRAB_ATTACK,FLoop_fPreviousLoopSecs*_fGrabSpeedMult);
		}
		break;

	case ZOMBIEBOSS_GRAB_ROAR:
		m_fStateTime = GetTime(ANIMTAP_GRAB_ATTACK);
		if (m_fStateTime >= 4.475f) // decide the next state
		{
			FASSERT(m_pGrabbedBot);
			if (m_pGrabbedBot->TypeBits() & ENTITY_BIT_BOTGLITCH)
			{
				((CBotGlitch*)m_pGrabbedBot)->ClearGrabbedByHead();
				
				CFVec3A vDir;
				vDir = m_MountUnitFrontXZ_WS;
				((CBotGlitch*)m_pGrabbedBot)->ThrownAway(vDir, 50.0f);
				m_pGrabbedBot = NULL;

			}
			m_eState = ZOMBIEBOSS_GRAB_THROWN;
		}
		else
		{
			DeltaTime(ANIMTAP_GRAB_ATTACK,FLoop_fPreviousLoopSecs*_fGrabSpeedMult);
			if ( (GetTime(ANIMTAP_GRAB_ATTACK) > 3.90f) && m_fStateTime < 3.90f)
			{
				CEBotFire* pBotFire = CEBotFire::GetFreeEBotFireFromPool();
				if (!pBotFire)
				{
					DEVPRINTF("EBotFire Pool ran out of flames");
				}
				else
				{
					CFMtx43A FireMtx = CFMtx43A::m_IdentityMtx;
					pBotFire->AddToWorld();
					pBotFire->Attach_ToParent_WS(m_pGrabbedBot,"Head");
					
					FireMtx.m_vPos = pBotFire->m_pParentMtxToWorld->m_vPos;
					pBotFire->Relocate_RotXlatFromUnitMtx_WS(&FireMtx);
					
					pBotFire->Start(NULL,NULL,this,fmath_RandomFloatRange(3.0f,4.0f));
				}
			}
		}
		break;
	case ZOMBIEBOSS_GRAB_THROWN:
		if (!DeltaTime(ANIMTAP_GRAB_ATTACK,FLoop_fPreviousLoopSecs*_fGrabSpeedMult))
		{

		}
		else
		{
			m_fStateTime = 0.0f;
			m_eState = ZOMBIEBOSS_GRAB_INTO_STANDING;
			UpdateUnitTime(ANIMTAP_STAND_LOOK, 0.0f);
		}
		break;

	case ZOMBIEBOSS_GRAB_INTO_STANDING:
		fUnitControl = (1.0f - m_fStateTime * _fOOGrabbingToLookTime);
		if (fUnitControl >= 0.0f)
            SetControlValue(ANIMCONTROL_GRAB_ATTACK, fUnitControl);
		else
		{	
			SetControlValue(ANIMCONTROL_GRAB_ATTACK, 0.0f);
			m_eState = ZOMBIEBOSS_STANDING_IDLE;
			m_fStateTime = 0.0f;
		}
		break;
	
	case ZOMBIEBOSS_INTO_DEATH:
		fUnitControl = m_fStateTime * _fOOToDeathTime;
		FMATH_CLAMP_UNIT_FLOAT(fUnitControl);
		SetControlValue(ANIMCONTROL_DEATH, fUnitControl);
		break;

	default:
		break;
	}

	if (m_bRoaring)
	{
		if (!DeltaTime(ANIMTAP_ROAR_UPPER,FLoop_fPreviousLoopSecs, TRUE))
		{
			static float _fVel = 25.f;
			fUnitControl = GetControlValue(ANIMCONTROL_ROAR_UPPER) + _fVel * FLoop_fPreviousLoopSecs;
			FMATH_CLAMP_UNIT_FLOAT(fUnitControl);
            SetControlValue(ANIMCONTROL_ROAR_UPPER, fUnitControl);
			SetControlValue(ANIMCONTROL_ROAR_LOWER, fUnitControl);
		}
		else
		{
			static float _fVel = 50.f;
			fUnitControl = GetControlValue(ANIMCONTROL_ROAR_UPPER) - _fVel * FLoop_fPreviousLoopSecs;
			FMATH_CLAMP_UNIT_FLOAT(fUnitControl);
            SetControlValue(ANIMCONTROL_ROAR_UPPER, fUnitControl);
			SetControlValue(ANIMCONTROL_ROAR_LOWER, fUnitControl);
			if (fUnitControl == 0.0f)
			{
				m_bRoaring = FALSE;
				if (IsImmobile())
					MobilizeBot();
				if (IsInvincible())
					SetInvincible(FALSE);
			}
		}
		_Smoke(TRUE,fUnitControl);
	}
	else if (m_bControls_Jump)
	{
		m_bRoaring = TRUE;
		UpdateUnitTime(ANIMTAP_ROAR_UPPER, 0.0f);
		_PlaySnd(ZBOSS_SND_ROAR,CBotZombieBoss::START);
	}
}

void CBotZombieBoss::_PlaySnd(ZombieBossSnd_e eSnd, SndCommand_e ePlay, CFVec3A* pPos_WS)
{
	if (eSnd >= ZBOSS_SND_COUNT)
	{
		DEVPRINTF("_PlaySnd() attempted sound past limit\n");
		return;
	}
	if (!m_pSounds[eSnd])
	{
		DEVPRINTF("_PlaySnd() attempted sound not loaded\n");
		return;
	}
	if (pPos_WS==NULL)
	{
		pPos_WS = &m_MtxToWorld.m_vPos;
	}

	BOOL bIsPlaying = !!m_pSoundEmitters[eSnd]; // test good for looping sounds, else not
	
	if (bIsPlaying && (ePlay==START_IF_NOT_PLAYING))
	{
		m_pSoundEmitters[eSnd]->SetPosition(pPos_WS);
		return;
	}

	if (m_pSoundEmitters[eSnd])
	{
		m_pSoundEmitters[eSnd]->Destroy();
		m_pSoundEmitters[eSnd]=NULL;
	}

	if ( (ePlay==CBotZombieBoss::START) || (ePlay==CBotZombieBoss::START_IF_NOT_PLAYING) )
	{
		m_pSoundEmitters[eSnd] = CFSoundGroup::AllocAndPlaySound(m_pSounds[eSnd],FALSE,pPos_WS);
	}
}
	
void CBotZombieBoss::Die( BOOL bSpawnDeathEffects, BOOL bSpawnGoodies)
{
	if (m_uFightLevelIndex==m_uNumFightLevels)
	{
		CBot::Die(bSpawnDeathEffects,bSpawnGoodies);
		_PlaySnd(ZBOSS_SND_DIE,CBotZombieBoss::START);
		UpdateUnitTime(ANIMTAP_DEATH, 0.0f);
		m_fStateTime = 0.0f;
		m_eState = ZOMBIEBOSS_INTO_DEATH;
		EnableCameraCollision( TRUE );
		SetProjectileReaction(CEntity::PROJECTILE_REACTION_HURT_ME);
		_Smoke(TRUE,1.0f,5.0f);
		
		FExplosionSpawnParams_t SpawnParams;
		SpawnParams.InitToDefaults();
		SpawnParams.pDamager= &CBotZombieBoss::m_Damager;
		
		SpawnParams.Pos_WS  = m_MountPos_WS;
		SpawnParams.UnitDir = CFVec3A::m_UnitAxisY;

		// we'll probably need this handle later, save it off?	
		FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();
		if( hSpawner != FEXPLOSION_INVALID_HANDLE ) 
		{
			CExplosion2::SpawnExplosion( hSpawner, m_hExplosionDie, &SpawnParams );
		}
	}
	else
	{
		SetMaxHealth();
		Power(FALSE,0,1.0f);
	}
}

void CBotZombieBoss::DeathWork(void)
{
	CBot::DeathWork();
	f32 fPreTime = GetUnitTime(ANIMTAP_DEATH);
	DeltaTime(ANIMTAP_DEATH,FLoop_fPreviousLoopSecs * _fDeathSpeedMult, TRUE);
	f32 fPostTime = GetUnitTime(ANIMTAP_DEATH);
    if ( (fPreTime <= 0.77) && (fPostTime > 0.77) )
	{
		// Get an empty damage form...
		CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();
		if( pDamageForm )
		{
			// Fill out the form...
			pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_BLAST;
			pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS;
			pDamageForm->m_pDamageProfile = m_pDamageProfileDeath;
			pDamageForm->m_Damager = CBotZombieBoss::m_Damager;
			pDamageForm->m_Epicenter_WS = m_MountPos_WS;
			CDamage::SubmitDamageForm( pDamageForm );
		}
		CFXShockwave::AddShockwave( m_MountPos_WS, 1.0f, 10.0f, ENTITY_BIT_BOT, 0.f, 0.f); // i got pushed out of world, no push for now
	}
	m_fIsOver -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMP_MIN0(m_fIsOver);
}

void CBotZombieBoss::ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS ) 
{
	pApproxMuzzlePoint_WS->Add(m_MtxToWorld.m_vPos,m_vTorsoPtMS);
}

void CBotZombieBoss::_ComputeVerletPosition(void)
{
		// verlet computation
	CFVec3A vGravity;
	vGravity.Set( _BossBallParams.fVerletPendentGravityX, _BossBallParams.fVerletPendentGravityY, _BossBallParams.fVerletPendentGravityZ);
	// Do the verlet integration
	CFVec3A CurrTemp = m_vBallPosOneAgoWS;
	CFVec3A PrevTemp = m_vBallPosTwoAgoWS;

	CFVec3A OldToNew;
	CFQuatA RotQuat;
	f32 fDelta = FLoop_fPreviousLoopSecs;
	FMATH_CLAMP(fDelta,0.005f,0.033f);

	vGravity.Mul( ( fDelta * fDelta ) );

	// Dampening
	OldToNew.Sub( CurrTemp, PrevTemp).Mul( _BossBallParams.fVerletDampeningConstant * fDelta );
	
	PrevTemp.Add( OldToNew );

	CurrTemp.Mul( 2.0f );
	CurrTemp.Sub( PrevTemp );
	CurrTemp.Add( vGravity );

	CFVerlet::Constraint_AnchorDistance( &CurrTemp, &m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexAttachBall]->m_vPos, m_fChainLength );
	if (!_BallIsOutThere())
		m_mtxBall.m_vPos = CurrTemp;
	else
	{
		m_mtxBall.m_vPos.y = CurrTemp.y;

		CFVec3A vDeltaVecXZ,vDeltaVecSnapToChainXZ;
		vDeltaVecXZ.x = CurrTemp.x - m_mtxBall.m_vPos.x;
		vDeltaVecXZ.z = CurrTemp.z - m_mtxBall.m_vPos.z;
		f32 fDeltaXZMag = vDeltaVecXZ.MagXZ();
		
		vDeltaVecSnapToChainXZ.x = m_vChainUnitDirection.x;
		vDeltaVecSnapToChainXZ.y = 0.0f;
		vDeltaVecSnapToChainXZ.z = m_vChainUnitDirection.z;
		f32 fInvMagChainXZ =  vDeltaVecSnapToChainXZ.InvMag();
		vDeltaVecSnapToChainXZ.Mul(fInvMagChainXZ*fDeltaXZMag); // sets this XZ vector to length from physics, but direction from chain
		m_mtxBall.m_vPos.x += vDeltaVecSnapToChainXZ.x;
		m_mtxBall.m_vPos.z += vDeltaVecSnapToChainXZ.z;
	}
}

BOOL CBotZombieBoss::_ProposedBallVictim( CFWorldTracker *pTracker, FVisVolume_t *pVolume )
{
	CFWorldMesh* pWorldMesh = (CFWorldMesh*)pTracker;
	if( !pWorldMesh->IsCollisionFlagSet() ) 
	{
		// Not collidable...
		return TRUE;
	}

	CBotZombieBoss* pThisBot = (CBotZombieBoss*)m_pCollBot;
	CFSphere m_Bounds = pThisBot->m_pBallMesh->GetBoundingSphere();

	if (pWorldMesh==pThisBot->m_pWorldMesh && (pThisBot->m_fChainLength < _fCollideWithBossChainLength))
	{
		pThisBot->_CollideWithPlane();
	}
	else
	{
//		if (pWorldMesh->CollideWithMeshTris( &m_CollInfo ))
//		{
//			// we are in collision with the pWorldMesh
//		}
	}

	return TRUE;
}

BOOL CBotZombieBoss::_ZombieGrabCheck( CFWorldTracker *pTracker, FVisVolume_t *pVolume )
{
	CFWorldMesh* pWorldMesh = (CFWorldMesh*)pTracker;
	CBotZombieBoss* pThisBot = (CBotZombieBoss*)m_pCollBot;
	
	CFCollInfo m_GrabTestCollInfo;
	m_GrabTestCollInfo.nCollTestType = FMESH_COLLTESTTYPE_SPHERE;
	m_GrabTestCollInfo.nCollMask = FCOLL_MASK_COLLIDE_WITH_NPCS;
	m_GrabTestCollInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
	
	m_GrabTestCollInfo.nTrackerUserTypeBitsMask = ENTITY_BIT_BOTGLITCH;

	// what values apply to sphere tests:
	m_GrabTestCollInfo.pSphere_WS = &m_GrabSphere;

	m_GrabTestCollInfo.pTag = pTracker;
	
	if (pWorldMesh->CollideWithMeshTris( &m_GrabTestCollInfo ))
	{
		CEntity* pEntity = CGColl::ExtractEntity( &FColl_aImpactBuf[0] );
		if (pEntity && (pEntity->TypeBits() & ENTITY_BIT_BOT))
		{
			pThisBot->m_pGrabbedBot = (CBot*)pEntity;
		}
	}
	return TRUE;
}

void CBotZombieBoss::_SpawnSlashImpactEffects( const FCollImpact_t *pCollImpact ) 
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
		if( pMaterial->CanDrawParticle( CGCollMaterial::PARTICLE_TYPE_DUST ) ) {
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

void CBotZombieBoss::_CollideWithPlane(void)
{
	FASSERT(m_nBoneIndexCollidePlane > -1);
	CFMtx43A* pMtx = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexCollidePlane];

	CFSphere m_Bounds = m_pBallMesh->GetBoundingSphere();
	f32 fColDist = _PlaneCollides(pMtx->m_vFront,pMtx->m_vPos, m_Bounds);
	if (fColDist > 0.0f)
	{
		CFVec3A vPlaneAdj;
		vPlaneAdj.Mul(pMtx->m_vFront, fColDist);
		m_mtxBall.m_vPos.Add(vPlaneAdj);
	}
}


void CBotZombieBoss::_SetUpBallPath(void)
{
	// DEBUG : REMOVE!
	CEntityControl* pControl = Controls();
	if (pControl->Type() == CEntityControl::TYPE_HUMAN)
	{
		f32 fAttackDist = fmath_RandomFloatRange(75.0f,110.f);
		m_vBallTargetWS.Mul(m_MountUnitFrontXZ_WS,fAttackDist);
		m_vBallTargetWS.x += m_mtxBall.m_vPos.x; 
		m_vBallTargetWS.y += m_MtxToWorld.m_vPos.y;
		m_vBallTargetWS.z += m_mtxBall.m_vPos.z; 
	}


	CFVec3A vTargetPoint;
	f32 fOORange;
	vTargetPoint.Sub(m_vBallTargetWS, m_mtxBall.m_vPos);
	fOORange = vTargetPoint.InvMagXZ();
	if(fOORange > 1.0f/10.f) // enemy within 10 ft;
	{
		m_vBallTargetWS.Mul(m_MountUnitFrontXZ_WS,20.0f);
		vTargetPoint.Sub(m_vBallTargetWS, m_mtxBall.m_vPos);
		fOORange = vTargetPoint.InvMagXZ();
	}

	vTargetPoint.Mul(fOORange);// now is unitized
	f32 fRange = fmath_Inv(fOORange);
	vTargetPoint.Mul(fRange + 2.0f * m_pBallMesh->GetBoundingSphere().m_fRadius);
	vTargetPoint.Add(m_mtxBall.m_vPos);
	vTargetPoint.y = m_vBallTargetWS.y;
	
	m_fBallTimeInAir = fmath_Div(fRange,m_paFightLevels[m_uFightLevelIndex].fBallSpeedXZ);
	m_fOOBallTimeInAir = fmath_Inv(m_fBallTimeInAir);
	m_fUnitBallFlight = 0.0f;

	m_Interpolator.Create(m_mtxBall.m_vPos,_vInTangent, vTargetPoint, _vOutTangent);
	m_Interpolator.SetXZLinear();
}

void CBotZombieBoss::_FlyBallFly(CFVec3A& vBall)
{
	f32 m_fUnitTimeSwinging = m_fStateTime * m_fBallTimeInAir;
	m_fUnitBallFlight += (m_paFightLevels[m_uFightLevelIndex].fBallAcceleration * (m_fUnitTimeSwinging - 0.5f) + m_fOOBallTimeInAir) * FLoop_fPreviousLoopSecs;

	m_Interpolator.Sample(vBall,m_fUnitBallFlight);
	if (m_fUnitBallFlight > 1.05f) // safety check on ball trajectory
	{
		_BallAttackCollides(NULL);
	}
}
#define TEST_CENTER_TO_GROUND_Y   (5.0f)
#define TEST0_X_SHIFT (0.0f)
#define TEST0_Y_EXTRA (0.0f)
#define TEST0_Z_SHIFT (0.0f)

#define TEST1_X_SHIFT (0.0f)
#define TEST1_Y_EXTRA (5.0f)
#define TEST1_Z_SHIFT (0.33f)

BOOL CBotZombieBoss::_HandleBallCollision( FCollImpact_t& rCollImpact )
{
	const u32 nTrackerSkipCount = 2;
	const CFWorldTracker* pSkipList[nTrackerSkipCount] = {m_pBallMesh,m_pWorldMesh};
	CFVec3A vGnd,vBall;

	vGnd.x  = m_mtxBall.m_vPos.x + TEST0_X_SHIFT;
	vGnd.y  = m_mtxBall.m_vPos.y - TEST_CENTER_TO_GROUND_Y;
	vGnd.z  = m_mtxBall.m_vPos.z + TEST0_Z_SHIFT;
	vBall.x = m_vBallPosOneAgoWS.x + TEST0_X_SHIFT;
	vBall.y = m_vBallPosOneAgoWS.y + TEST_CENTER_TO_GROUND_Y + TEST0_Y_EXTRA;
	vBall.z = m_vBallPosOneAgoWS.z + TEST0_Z_SHIFT;

//	CDebugDraw::Line(vGnd, vBall,&FColor_MotifGreen,&FColor_MotifGreen);

	BOOL bCollideDetected = FALSE;
	if( fworld_FindClosestImpactPointToRayStart( &rCollImpact, &vBall, &vGnd,nTrackerSkipCount,(const CFWorldTracker * const *)&pSkipList) )
	{
		bCollideDetected = TRUE;
	}
	else // didn't collide? Perturb the check, and try again
	{
		vGnd.x  = m_mtxBall.m_vPos.x + TEST1_X_SHIFT;
//		vGnd.y  = m_mtxBall.m_vPos.y - TEST_CENTER_TO_GROUND_Y;
		vGnd.z  = m_mtxBall.m_vPos.z + TEST1_Z_SHIFT;
		vBall.x = m_vBallPosOneAgoWS.x + TEST1_X_SHIFT;
		vBall.y = m_vBallPosOneAgoWS.y + TEST_CENTER_TO_GROUND_Y + TEST1_Y_EXTRA;
		vBall.z = m_vBallPosOneAgoWS.z + TEST1_Z_SHIFT;
//		CDebugDraw::Line(vGnd, vBall, &FColor_MotifBlack,&FColor_MotifBlack);
		if ( fworld_FindClosestImpactPointToRayStart( &rCollImpact, &vBall, &vGnd,nTrackerSkipCount,(const CFWorldTracker * const *)&pSkipList) )
		{
			bCollideDetected = TRUE;
		}
		else // didn't collide? Perturb the check, one last time, and try again
		{
			vGnd.x  = m_mtxBall.m_vPos.x;
			vGnd.y  = m_mtxBall.m_vPos.y;
			vGnd.z  = m_mtxBall.m_vPos.z;

			vBall = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexAttachBall]->m_vPos;
//			CDebugDraw::Line(vGnd, vBall,&FColor_MotifRed,&FColor_MotifRed);
			if ( fworld_FindClosestImpactPointToRayStart( &rCollImpact, &vBall, &vGnd,nTrackerSkipCount,(const CFWorldTracker * const *)&pSkipList) )
			{
				bCollideDetected = TRUE;
			}
		}
	}
	if (bCollideDetected)
	{
		m_mtxBall.m_vPos.y = FMATH_MAX(m_mtxBall.m_vPos.y,rCollImpact.ImpactPoint.y + m_pBallMesh->GetBoundingSphere().m_fRadius);
		m_uBallCollCountLastFrame = 1;
		return TRUE;
	}

	CBotZombieBoss* pThisBot = (CBotZombieBoss*)m_pCollBot;
	CFSphere m_Bounds = pThisBot->m_pBallMesh->GetBoundingSphere();
	// tracker check
	FWorld_nTrackerSkipListCount = 0;
	FWorld_apTrackerSkipList[FWorld_nTrackerSkipListCount++] = m_pBallMesh;
	fworld_FindTrackersIntersectingSphere(&m_Bounds,
		FWORLD_TRACKERTYPE_MESH,
		_ProposedBallVictim,
		FWorld_nTrackerSkipListCount,
		FWorld_apTrackerSkipList,
		m_pBallMesh,
		MESHTYPES_ENTITY,
		(u64)-1);

	m_uBallCollCountLastFrame = 0;
	return FALSE;
/*
	CFVec3A vMovement, vPushVec_WS, vAdjust;
	CFCollData CollData;

	// Determine the movement that is to be applied
	vMovement.Sub( m_mtxBall.m_vPos, m_vBallPosOneAgoWS);

	CDebugDraw::Sphere(m_mtxBall.m_vPos, 5.0f, &FColor_MotifRed,5,5);
	CDebugDraw::Sphere(m_vBallPosOneAgoWS, 1.0f, &FColor_MotifBlue,5,5);

	CollData.pMovement = &vMovement;
	// Fill out the CollData
	CollData.nCollMask = FCOLL_MASK_COLLIDE_WITH_NPCS | FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES | FCOLL_MASK_COLLIDE_WITH_DEBRIS;
	CollData.nFlags = FCOLL_DATA_IGNORE_BACKSIDE;
	CollData.nStopOnFirstOfCollMask = FCOLL_MASK_NONE;
	CollData.pCallback = _ProposedBallVictim;
	CollData.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
	CollData.pLocationHint = m_pBallMesh;

	// Clear the collision buffer and check the collision
	fcoll_Clear();
	fcoll_Check( &CollData, &CollSphere );

	// If there is no collision, we can just exit the loop
	if( FColl_nImpactCount == 0 ) 
	{
		return FALSE;
	}
	else
	{
		// choose the collision of greatest UnitY
		rCollImpact = FColl_aImpactBuf[0];

		for (u32 i = 1; (i < FColl_nImpactCount); i++)
		{
			if (FColl_aImpactBuf[i].UnitFaceNormal.y > rCollImpact.UnitFaceNormal.y)
			{
				rCollImpact = FColl_aImpactBuf[i];
			}
		}
		CDebugDraw::Sphere(rCollImpact.ImpactPoint, .33f, &FColor_MotifRed);
		return TRUE;
	}
*/
}

void CBotZombieBoss::_BallAttackCollides(const FCollImpact_t *pCollImpact )
{
	//	FASSERT(m_eState==ZOMBIEBOSS_SWING_ROLL_OUT);

	m_fStateTime = 0.0f;
	m_eState = ZOMBIEBOSS_SWING_TUG_PAUSE;
	
	if (pCollImpact)
	{
		m_mtxBall.m_vPos = pCollImpact->ImpactPoint;
		m_mtxBall.m_vPos.y = FMATH_MAX(m_mtxBall.m_vPos.y,pCollImpact->ImpactPoint.y + m_pBallMesh->GetBoundingSphere().m_fRadius);

		FExplosionSpawnParams_t SpawnParams;
		SpawnParams.InitToDefaults();
		SpawnParams.pDamager= &CBotZombieBoss::m_Damager;
		SpawnParams.Pos_WS  = pCollImpact->ImpactPoint;
		SpawnParams.UnitDir = pCollImpact->UnitFaceNormal;

		// we'll probably need this handle later, save it off?	
		FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();
		if( hSpawner != FEXPLOSION_INVALID_HANDLE ) 
		{
			CExplosion2::SpawnExplosion( hSpawner, m_hExplosionBallHitGroup, &SpawnParams );
		}
		CFXShockwave::AddShockwave( pCollImpact->ImpactPoint, 2.0f, 5.0f, ENTITY_BIT_BOT, 25.f, 10.f);
	}
}
void CBotZombieBoss::_BallReturnCollides( const FCollImpact_t *pCollImpact )
{
	if (Power_IsPoweredDown())
		return;

	FASSERT(pCollImpact);
	CFWorldMesh *pImpactWorldMesh = NULL;
	pImpactWorldMesh = (CFWorldMesh *)(pCollImpact->pTag);
	// See if the world mesh that we found is an entity...
	CEntity* pEntity = CGColl::ExtractEntity( pCollImpact );
	if ( (pEntity==NULL) || ((pEntity->TypeBits() & ENTITY_BIT_BOT) == 0))
	{
//		potmark_NewPotmark(&pCollImpact->ImpactPoint, &pCollImpact->UnitFaceNormal, 5.0f, POTMARKTYPE_LASER2);
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


	// Get an empty damage form...
	CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

	if( pDamageForm )
	{
		// Fill out the form...
		pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_BLAST;
		pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS;
		pDamageForm->m_pDamageProfile = m_pDamageProfileDrag;
		pDamageForm->m_Damager = CBotZombieBoss::m_Damager;
		pDamageForm->m_Epicenter_WS = pCollImpact->ImpactPoint;
		CDamage::SubmitDamageForm( pDamageForm );
	}
}

void CBotZombieBoss::CheckpointSaveSelect( s32 nCheckpoint ) 
{
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}

BOOL CBotZombieBoss::CheckpointSave( void ) 
{
	// Save parent class data...
	CBot::CheckpointSave();

	// Save bot class data. Order must match load order below!
	CFCheckPoint::SaveData( m_vBallPosOneAgoWS );
	CFCheckPoint::SaveData( m_vBallPosTwoAgoWS );
	
	CFCheckPoint::SaveData( m_eState );
	CFCheckPoint::SaveData( m_fStateTime );
	CFCheckPoint::SaveData( m_fUnitAimPitch );
	CFCheckPoint::SaveData( m_fChainLength );
	CFCheckPoint::SaveData( m_fUnitBallFlight );

	CFCheckPoint::SaveData( m_fIsOver );
	CFCheckPoint::SaveData( m_uFightLevelIndex );

	CFCheckPoint::SaveData( GetControlValue(ANIMCONTROL_STAND_LOOK));
	CFCheckPoint::SaveData( GetControlValue(ANIMCONTROL_STAND_SWING));
	CFCheckPoint::SaveData( GetControlValue(ANIMCONTROL_WALK));
	CFCheckPoint::SaveData( GetControlValue(ANIMCONTROL_CHAIN_SWING_UPPER));
	CFCheckPoint::SaveData( GetControlValue(ANIMCONTROL_CHAIN_ROLL_UPPER));
	CFCheckPoint::SaveData( GetControlValue(ANIMCONTROL_CHAIN_RETURN_UPPER));
	CFCheckPoint::SaveData( GetControlValue(ANIMCONTROL_ROAR_UPPER));
	CFCheckPoint::SaveData( GetControlValue(ANIMCONTROL_TUG_UPPER));
	CFCheckPoint::SaveData( GetControlValue(ANIMCONTROL_CHAIN_SWING_LOWER));
	CFCheckPoint::SaveData( GetControlValue(ANIMCONTROL_CHAIN_ROLL_LOWER));
	CFCheckPoint::SaveData( GetControlValue(ANIMCONTROL_CHAIN_RETURN_LOWER));
	CFCheckPoint::SaveData( GetControlValue(ANIMCONTROL_ROAR_LOWER));
	CFCheckPoint::SaveData( GetControlValue(ANIMCONTROL_TUG_LOWER));
	CFCheckPoint::SaveData( GetControlValue(ANIMCONTROL_GRAB_ATTACK));		
	CFCheckPoint::SaveData( GetControlValue(ANIMCONTROL_FREEZE));		
	CFCheckPoint::SaveData( GetControlValue(ANIMCONTROL_DEATH));		

	return TRUE;
}


void CBotZombieBoss::CheckpointRestore( void ) 
{
	// Load parent class data...
	FASSERT( m_hSaveData[CFCheckPoint::GetCheckPoint()] != CFCheckPoint::NULL_HANDLE );
	CBot::CheckpointRestore();

	CFCheckPoint::LoadData( m_vBallPosOneAgoWS );
	CFCheckPoint::LoadData( m_vBallPosTwoAgoWS );


	CFCheckPoint::LoadData( (u32&) m_eState );
	CFCheckPoint::LoadData( m_fStateTime );
	CFCheckPoint::LoadData( m_fUnitAimPitch );
	CFCheckPoint::LoadData( m_fChainLength );
	CFCheckPoint::LoadData( m_fUnitBallFlight );

	CFCheckPoint::LoadData( m_fIsOver );
	CFCheckPoint::LoadData( m_uFightLevelIndex );

	f32 fControlValue;
	CFCheckPoint::LoadData( fControlValue ); SetControlValue(ANIMCONTROL_STAND_LOOK,fControlValue );
	CFCheckPoint::LoadData( fControlValue ); SetControlValue(ANIMCONTROL_STAND_SWING,fControlValue );
	CFCheckPoint::LoadData( fControlValue ); SetControlValue(ANIMCONTROL_WALK,fControlValue );
	CFCheckPoint::LoadData( fControlValue ); SetControlValue(ANIMCONTROL_CHAIN_SWING_UPPER,fControlValue );
	CFCheckPoint::LoadData( fControlValue ); SetControlValue(ANIMCONTROL_CHAIN_ROLL_UPPER,fControlValue );
	CFCheckPoint::LoadData( fControlValue ); SetControlValue(ANIMCONTROL_CHAIN_RETURN_UPPER,fControlValue );
	CFCheckPoint::LoadData( fControlValue ); SetControlValue(ANIMCONTROL_ROAR_UPPER,fControlValue );
	CFCheckPoint::LoadData( fControlValue ); SetControlValue(ANIMCONTROL_TUG_UPPER,fControlValue );
	CFCheckPoint::LoadData( fControlValue ); SetControlValue(ANIMCONTROL_CHAIN_SWING_LOWER,fControlValue );
	CFCheckPoint::LoadData( fControlValue ); SetControlValue(ANIMCONTROL_CHAIN_ROLL_LOWER,fControlValue );
	CFCheckPoint::LoadData( fControlValue ); SetControlValue(ANIMCONTROL_CHAIN_RETURN_LOWER,fControlValue );
	CFCheckPoint::LoadData( fControlValue ); SetControlValue(ANIMCONTROL_ROAR_LOWER,fControlValue );
	CFCheckPoint::LoadData( fControlValue ); SetControlValue(ANIMCONTROL_TUG_LOWER,fControlValue );
	CFCheckPoint::LoadData( fControlValue ); SetControlValue(ANIMCONTROL_GRAB_ATTACK,fControlValue );
	CFCheckPoint::LoadData( fControlValue ); SetControlValue(ANIMCONTROL_FREEZE,fControlValue );
	CFCheckPoint::LoadData( fControlValue ); SetControlValue(ANIMCONTROL_DEATH,fControlValue );


	m_pBotInfo_MountAim->fMountYawBaseRevsPerSec = m_paFightLevels[m_uFightLevelIndex].fTurnSpeed;
}

BOOL CBotZombieBoss::ComputeGrabPointWS(CFVec3A& rPtWs)
{
	CFVec3A vGrabPtWS;
	MS2WS(vGrabPtWS, m_vGrabPtMS);
	rPtWs.Add(m_MtxToWorld.m_vPos, vGrabPtWS);
	if ( (m_eState==ZOMBIEBOSS_GRAB_PICKUP) ||
		 (m_eState==ZOMBIEBOSS_GRAB_ROAR) )
	{
		return FALSE;
	}
	else
		return TRUE;
}

void CBotZombieBoss::_ComputeGrabPointMS(CFVec3A& rPtMS)
{
	// reset mesh to Identity to compute MS points;
	m_pWorldMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_IdentityMtx );

	// Set the controls to grab;
	f32 fOldGrabCtl = GetControlValue(ANIMCONTROL_GRAB_ATTACK);
	SetControlValue(ANIMCONTROL_GRAB_ATTACK, 1.0f);
	// Set the time of the grab
	f32 fOldGrabTime = GetTime(ANIMTAP_GRAB_ATTACK);
	
	UpdateTime(ANIMTAP_GRAB_ATTACK, _fTimeOfGrabPickup);

	// update the matrices
	m_Anim.m_pAnimCombiner->SetBoneCallback( NULL ); // don't want the cb functionality
	ComputeMtxPalette( FALSE );
	m_Anim.m_pAnimCombiner->SetBoneCallback( &_AnimBoneCallback ); // re-enable callbacks

	// copy Attchk matrix
	CFMtx43A mtxAttachG = *m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexAttachGlitch];
	
	// Unset the time of the grab
	UpdateTime(ANIMTAP_GRAB_ATTACK, fOldGrabTime);
	// Unset the controls of the grab
	SetControlValue(ANIMCONTROL_GRAB_ATTACK, fOldGrabCtl	);

	rPtMS.Set(mtxAttachG.m_vPos); // compute's the MS grabpoint
}


void CBotZombieBoss::_ComputeGrabPointWS(CFVec3A& rPtMS)
{
#ifdef FANG_DEBUG_BUILD
	// Set the controls to grab;
	f32 fOldGrabCtl = GetControlValue(ANIMCONTROL_GRAB_ATTACK);
	SetControlValue(ANIMCONTROL_GRAB_ATTACK, 1.0f);
	// Set the time of the grab
	f32 fOldGrabTime = GetTime(ANIMTAP_GRAB_ATTACK);
	
	UpdateTime(ANIMTAP_GRAB_ATTACK, _fTimeOfGrabPickup);

	// update the matrices
	m_Anim.m_pAnimCombiner->SetBoneCallback( NULL ); // don't want the cb functionality
	ComputeMtxPalette( TRUE );
	m_Anim.m_pAnimCombiner->SetBoneCallback( &_AnimBoneCallback ); // re-enable callbacks

	// copy Attchk matrix
	CFMtx43A mtxAttachG = *m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexAttachGlitch];
	
	// Unset the time of the grab
	UpdateTime(ANIMTAP_GRAB_ATTACK, fOldGrabTime);
	// Unset the controls of the grab
	SetControlValue(ANIMCONTROL_GRAB_ATTACK, fOldGrabCtl	);

	rPtMS.Set(mtxAttachG.m_vPos); // compute's the MS grabpoint
#endif
}

#define CLOSE_ENOUGH_TO_BALL_SQ (20.0f * 20.0f)
BOOL CBotZombieBoss::ShouldNotPursue(void)
{
	if ((m_eState==ZOMBIEBOSS_SWING_ROLL_IN)||
		(m_eState==ZOMBIEBOSS_SWING_RETURN) ||
		(m_eState==ZOMBIEBOSS_SWING_ROLL_OUT)
		)
	{
		f32 fDistBallSq = m_mtxBall.m_vPos.DistSqXZ(m_MtxToWorld.m_vPos);
		if (fDistBallSq <= CLOSE_ENOUGH_TO_BALL_SQ)
		{
			return TRUE;
		}
	}
	if ((m_eState==ZOMBIEBOSS_GRAB_START) ||
		(m_eState==ZOMBIEBOSS_GRAB_SUCCESS) ||
		(m_eState==ZOMBIEBOSS_GRAB_PICKUP) ||
		(m_eState==ZOMBIEBOSS_GRAB_ROAR) ||
		(m_eState==ZOMBIEBOSS_GRAB_THROWN) )
	{
		return TRUE;
	}
	

	return FALSE;
}
void CBotZombieBoss::InflictDamage( CDamageData *pDamageData )
{
	if (IsDeadOrDying())
		return;

	if (Power_IsPoweredUp())
	{
		if (!pDamageData->m_Damager.pWeapon && (pDamageData->m_nDamageLocale == CDamageForm::DAMAGE_LOCALE_AMBIENT)) // lava case
			return;
		CDamageResult DamageResult;
		pDamageData->ComputeDamageResult( m_paFightLevels[m_uFightLevelIndex].pArmorProfile, &DamageResult, GetArmorModifier() );
		// need to filter out 
		InflictDamageResult( &DamageResult );
	}
	else if (Power_IsPoweredDown() && m_fPowerOffOnTime > 0.f)
	{
		if (pDamageData->m_Damager.pWeapon)
		{
			if ( (pDamageData->m_Damager.pWeapon->TypeBits() & ENTITY_BIT_WEAPONGREN) ||
						((pDamageData->m_Damager.pWeapon->TypeBits() & ENTITY_BIT_MAGMABOMB) && 
						(fclib_stricmp(pDamageData->m_pDamageProfile->m_pszName,"MagmaDetonate")==0)) )
			{
				if ( m_BlastDamageReceiverSphere.IsIntersecting(pDamageData->m_pEpicenter_WS->v3))
				{
					m_uFightLevelIndex++;
					m_fPowerOffOnTime = 0.0f; // wake up from power up

					if (m_uFightLevelIndex==m_uNumFightLevels)
					{
						Die();
						// Give the player who killed us credit for the kill...
						FASSERT( pDamageData );
						s32 nPlayer = pDamageData->m_Damager.nDamagerPlayerIndex;
						if( nPlayer >= 0 ) 
						{
							Player_aPlayer[nPlayer].CreditKill(m_nPossessionPlayerIndex, this);
						}
					}
					else if (m_uFightLevelIndex < m_uNumFightLevels )
					{
						SetInvincible(TRUE); // don't take damage for a sec
						m_bRoaring = TRUE;
						UpdateUnitTime(ANIMTAP_ROAR_UPPER, 0.0f);
						_PlaySnd(ZBOSS_SND_ROAR,CBotZombieBoss::START);
						m_pBotInfo_MountAim->fMountYawBaseRevsPerSec = m_paFightLevels[m_uFightLevelIndex].fTurnSpeed;
					}
					CFScriptSystem::TriggerEvent(CFScriptSystem::GetEventNumFromName("boss"), BOSS_CRITICAL_HIT, (u32) this,  (u32) m_uFightLevelIndex);
				}
			}
		}
	}
}

void CBotZombieBoss::InflictDamageResult( const CDamageResult *pDamageResult ) 
{
	f32 fNormHealthBefore = NormHealth();
	CFSphere sphDamage;
	
	if (pDamageResult->m_pDamageData->m_Damager.pBot==this)
		return;
	
	if (pDamageResult->m_pDamageData->m_Damager.pBot && !(pDamageResult->m_pDamageData->m_Damager.pBot->TypeBits() & ENTITY_BIT_BOTGLITCH))
		return;

	CBot::InflictDamageResult(pDamageResult);

	f32 fNormHealthAfter = NormHealth();
	
	if (fNormHealthAfter < fNormHealthBefore)
	{
		if (m_fTimeUntilNextOuch == 0.0f)
		{
			PlaySound(m_pSounds[ZBOSS_SND_DAMAGED]);
			m_fTimeUntilNextOuch = fmath_RandomFloatRange(_fMinTimeBetweenOuches,_fMaxTimeBetweenOuches);
		}
		if (pDamageResult->m_pDamageData->m_nDamageLocale == CDamageForm::DAMAGE_LOCALE_IMPACT)
		{
			CFDebrisSpawner DebrisSpawner;
			DebrisSpawner.InitToDefaults();

			DebrisSpawner.m_Mtx.m_vPos = pDamageResult->m_pDamageData->m_ImpactPoint_WS; 
			DebrisSpawner.m_Mtx.m_vZ =  pDamageResult->m_pDamageData->m_pTriData->ImpactUnitNormal_WS;
			DebrisSpawner.m_nEmitterType = CFDebrisSpawner::EMITTER_TYPE_POINT;
			DebrisSpawner.m_pDebrisGroup = m_pDebrisGroup;
			DebrisSpawner.m_fMinSpeed = 10.0f;
			DebrisSpawner.m_fMaxSpeed = 20.0f;
			DebrisSpawner.m_fUnitDirSpread = 0.2f;
			DebrisSpawner.m_nMinDebrisCount = 1;
			DebrisSpawner.m_nMaxDebrisCount = 3;

			CGColl::SpawnDebris( &DebrisSpawner );
		}
	}
}

void CBotZombieBoss::_HandleReelAudio(void)
{
	if (Power_IsPoweredDown())
	{
		_PlaySnd(ZBOSS_SND_BALL_REEL, STOP);
	}
	else if ( (m_eState==ZOMBIEBOSS_SWING_ROLL_IN) ||
		 (m_eState==ZOMBIEBOSS_SWING_ROLL_OUT) ||
		 (m_eState==ZOMBIEBOSS_SWING) )
	{
		_PlaySnd(ZBOSS_SND_BALL_REEL, START_IF_NOT_PLAYING);
	}
	else
	{
		_PlaySnd(ZBOSS_SND_BALL_REEL, STOP);
	}
}

void CBotZombieBoss::_HandleBallAudio(void)
{
	if (Power_IsPoweredDown())
	{
		_PlaySnd(ZBOSS_SND_BALL_DRAG, STOP);
	}
	else if  (m_eState==ZOMBIEBOSS_SWING_TUG_PULL)
	{
		_PlaySnd(ZBOSS_SND_BALL_DRAG, START_IF_NOT_PLAYING, &m_mtxBall.m_vPos);
	}
	else
	{
		_PlaySnd(ZBOSS_SND_BALL_DRAG, STOP);
	}
}

void CBotZombieBoss::_HandleGruntAudio(void)
{
	if ( (m_eState == ZOMBIEBOSS_GRAB_ROAR ) ||
		 (Power_IsPoweredDown()) ||
		 (IsDead()) )
		return;

	if (m_fTimeUntilNextGrunt==0.0f)
	{
		PlaySound(m_pSounds[ZBOSS_SND_EFFORT]);
		m_fTimeUntilNextGrunt = fmath_RandomFloatRange(_fMinTimeBetweenGrunts,_fMaxTimeBetweenGrunts);
	}
}

void CBotZombieBoss::Power( BOOL bPowerUp, f32 fPowerOffTime, f32 fPowerOffOnSpeedMult )
{
	CBot::Power( bPowerUp, m_paFightLevels[m_uFightLevelIndex].fPowerDownTime, fPowerOffOnSpeedMult );
	if (bPowerUp==FALSE)
	{
		m_bFreeBalling = TRUE;
		_Smoke(FALSE,0.0f);
	}
}

void CBotZombieBoss::Power_Work( void )
{
	FASSERT( IsCreated() );

	switch( m_nPowerState ) {
	case POWERSTATE_POWERED_UP:
		// Nothing to do, so bail!
		if (!IsDeadOrDying())
			_Smoke(TRUE,0.0f);
		break;

	case POWERSTATE_POWERED_DOWN:
		// If we were turned off on a timer...
		DeltaTime( ANIMTAP_FREEZE, FLoop_fPreviousLoopSecs );
		if( m_fPowerOffOnTime > 0.0f ) 
		{
			m_fPowerOffOnTime -= FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fPowerOffOnTime, 0.0f );
		}
		// Time is done, turn the bot back on
		if( m_fPowerOffOnTime == 0.0f && !IsDead() ) 
		{
			CBot::Power( TRUE );
		}
		break;

	case POWERSTATE_WAITING_FOR_IMMOBILIZED:
		if( IsImmobile() ) {
			// Bot is finally immobilized...
			m_nPowerState = POWERSTATE_POWERING_DOWN;
			ZeroTime( ANIMTAP_FREEZE );
			m_fPowerAnimUnitBlend = 0.0f;
		} else {
			// keep trying to immobilize him.  fixes bug where bot is 
			// re-mobilized in same frame after being EMPed.
			ImmobilizeBot();
		}


		break;

	case POWERSTATE_POWERING_DOWN:
		if( m_fPowerAnimUnitBlend < 1.0f ) 
		{
			m_fPowerAnimUnitBlend += FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMAX( m_fPowerAnimUnitBlend, 1.0f );
			SetControlValue( ANIMCONTROL_FREEZE, m_fPowerAnimUnitBlend );
		}
		else 
		{
			// Done powering down...
			m_nPowerState = POWERSTATE_POWERED_DOWN;
			SetControlValue( ANIMCONTROL_FREEZE, 1.0f );

			if( m_bOppositePowerStatePending ) 
			{
				// Start power-up...
				Power( TRUE, 0.0f, m_fPendingPowerOffOnAnimSpeedMult );
			}
		}
		break;

	case POWERSTATE_POWERING_UP:
		// We're powering up...
		if( m_fPowerAnimUnitBlend < 1.0f ) 
		{
			m_fPowerAnimUnitBlend += FLoop_fPreviousLoopSecs * 1.5f;
			FMATH_CLAMPMAX( m_fPowerAnimUnitBlend, 1.0f );
			SetControlValue( ANIMCONTROL_FREEZE, (1.0f - m_fPowerAnimUnitBlend) );
		}
		else 
		{
			// Done powering up...
			m_nPowerState = POWERSTATE_FINISHING_UP;
			SetControlValue( ANIMCONTROL_FREEZE, 0.0f );
			m_fPowerAnimUnitBlend = 1.0f;
		}
		break;

	case POWERSTATE_FINISHING_UP:
		// Finished up...

		m_nPowerState = POWERSTATE_POWERED_UP;

		if( m_bOppositePowerStatePending ) {
			// Start powering down...
			Power( FALSE, m_fPendingOffOnTime, m_fPendingPowerOffOnAnimSpeedMult );
		} else {
			if (!m_bRoaring)
				MobilizeBot();
		}
		break;

	default:
		FASSERT_NOW;
	}
}

void CBotZombieBoss::NotifyFootDown( BOOL bLeftFoot, BOOL bRightFoot, f32 fUnitMag ) 
{
	CBot::NotifyFootDown( bLeftFoot, bRightFoot, fUnitMag );

	// We want to get a damage profile and apply rumble here...
	if( fUnitMag > 0.0f ) 
	{
		// Get an empty damage form...
		CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

		if( pDamageForm ) 
		{
			// Fill out the form...
			pDamageForm->m_nDamageLocale	= CDamageForm::DAMAGE_LOCALE_BLAST;
			pDamageForm->m_nDamageDelivery	= CDamageForm::DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS;
			pDamageForm->m_pDamageProfile	= m_pFootstepDamageProfile;
			if( bLeftFoot ) 
			{
				pDamageForm->m_Epicenter_WS = m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIndexL_Foot ]->m_vPos;
			} else if ( bRightFoot ) 
			{
				pDamageForm->m_Epicenter_WS = m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIndexR_Foot ]->m_vPos;
			} else 
			{
				// We should never get here, but I'll put this here to just be safe!
				pDamageForm->m_Epicenter_WS		= m_MountPos_WS;
			}
			pDamageForm->m_Damager.pBot		= this;
			pDamageForm->m_Damager.nDamagerPlayerIndex = m_nPossessionPlayerIndex;
			pDamageForm->m_fNormIntensity	= 1.0f;//fmath_Sqrt( fUnitMag );

			// submitting footdown damage form for titan mag = 
			CDamage::SubmitDamageForm( pDamageForm );
		}
	}
}

const FGameData_TableEntry_t CBotZombieBoss::m_ZombieBossInfoVocab_Fight[] = 
{
//	u32 fMoveSpeed;
	FGAMEDATA_VOCAB_U32_BOUND(0,F32_DATATABLE_100),
//	f32 fTurnSpeed;
	FGAMEDATA_VOCAB_F32_BOUNDMIN(F32_DATATABLE_Pt1),
//	f32 fBallSpeedXZ;
	FGAMEDATA_VOCAB_F32_BOUNDMIN(F32_DATATABLE_1),
//	f32 fBallAcceleration;
	FGAMEDATA_VOCAB_F32_BOUNDMIN(F32_DATATABLE_Pt1),
//	f32 fBallReturnSpeed;
	FGAMEDATA_VOCAB_F32_BOUNDMIN(F32_DATATABLE_Pt1),
//	CArmorProfile * pArmorProfile;
	FGAMEDATA_VOCAB_ARMOR,
//	f32 fPowerDownTime;
	FGAMEDATA_VOCAB_F32_BOUNDMIN(F32_DATATABLE_0),
	
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

BOOL CBotZombieBoss::_ReadFightLevel(cchar* pszFileName)
{
	FResFrame_t ResFrame = fres_GetFrame();

	FGameDataFileHandle_t hGameDataFile = fgamedata_LoadFileToFMem( pszFileName );
	if( hGameDataFile == FGAMEDATA_INVALID_FILE_HANDLE ) 
	{
		DEVPRINTF( "CBotZombieBoss::_ReadFightLevel(): Could not open '%s'.\n", pszFileName );
		goto _ExitWithError;
	}
	cchar* pszTableName = "Fight";
	FGameDataTableHandle_t hFightTable = fgamedata_GetFirstTableHandle( hGameDataFile, pszTableName);
	if( hFightTable == FGAMEDATA_INVALID_TABLE_HANDLE ) 
	{
		DEVPRINTF( "CBotZombieBoss::_ReadFightLevel(): Failed to find %s handle\n", pszTableName );
		goto _ExitWithError;
	}
	
	s32 nStructElementCount = sizeof( m_ZombieBossInfoVocab_Fight ) / sizeof( FGameData_TableEntry_t );
	FASSERT( nStructElementCount > 0 );
	--nStructElementCount;

	s32 nTableFieldCount = fgamedata_GetNumFields( hFightTable );
	if( nTableFieldCount % nStructElementCount ) 
	{
			DEVPRINTF( "CBotZombieBoss::_ReadFightLevel():  Invalid number of fields table '%s' of file '%s.csv'.\n", pszTableName, pszFileName );
			goto _ExitWithError;
	}

	m_uNumFightLevels = nTableFieldCount / nStructElementCount;

	u32 nBytesToAlloc = m_uNumFightLevels * sizeof(ZombieBossFightLevel_t);
	m_paFightLevels = (CBotZombieBoss::ZombieBossFightLevel_t*)fres_Alloc(nBytesToAlloc);
	if( m_paFightLevels == NULL ) 
	{
		DEVPRINTF( "CBotZombieBoss::_ReadFightLevel(): Not enough memory for fight levels .\n" );
		goto _ExitWithError;
	}

	if(fgamedata_GetTableData_ArrayOfStructures(hFightTable, m_ZombieBossInfoVocab_Fight, m_paFightLevels,sizeof(ZombieBossFightLevel_t),m_uNumFightLevels)==FALSE)
	{
		DEVPRINTF( "CBotZombieBoss::_ReadFightLevel(): FAILED.\n" );
		goto _ExitWithError;
	}

	return TRUE;

_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}