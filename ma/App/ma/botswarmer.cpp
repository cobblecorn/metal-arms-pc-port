//////////////////////////////////////////////////////////////////////////////////////
// botswarmer.cpp - 
//
// Author: Nathan Miller
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
// 01/22/03 Miller		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fworld_coll.h"
#include "fresload.h"
#include "ProTrack.h"
#include "MeshTypes.h"
#include "botswarmer.h"
#include "Player.h" //findfix: pat's testing stuff
#include "ai\AIGameUtils.h"
#include "AI\AIEnviro.h"
#include "AI\AIBrain.h"
#include "AI\AIMover.h"
#include "AI\AiApi.h"
#include "AI\AiBrainMems.h"
#include "espline.h"
#include "spawnsys.h"
#include "explosion.h"
#include "fsound.h"



#define _BOTINFO_FILENAME				"b_swarmer"
#define _BOTINFO_SWARMER_MESHNAME		"GRMWswarm00"
#define _BOTINFO_VERMIN_MESHNAME		"grdvvermin1"
#define _ATTACK_SPARK_NAME				"spark01"
#define _DEATH_DEBRIS_GROUP				"DeadSwarmer1"

#define _MAX_VELOCITY_XZ				( 50.0f )
#define _MAX_VELOCITY_XZ_OO				( ( 1.0f / _MAX_VELOCITY_XZ ) )
#define _MAX_MISS_VELOCITY				( _MAX_VELOCITY_XZ - 5.0f )		
#define _MAX_ACCEL_XZ					( 30.0f )
#define _DECEL_MULT_XZ					( 30.0f )
#define _BITE_TIME						( 0.4f )
#define _MAX_ATTACHED_SWARMERS			( 3 )
#define _GRAVITY						( -64.0f )
#define _ANGLE_TEST						( 60.0f )
#define _ROTATE_TO_PLANE_RAMP			( 8.0f )
#define	_BOT_ATTACH_IGNORE_TIME			( 1.0f )
#define _JUMP_ANGLE						( FMATH_DEG2RAD( -35.0f ) )
#define _YAW_WOBBLE_MAX					( FMATH_DEG2RAD( 25.0f ) )
#define _WOBBLE_RATE					( 4.0f )

#define _SWARMER_SHOOT_MIN				( 0.0f )
#define _SWARMER_SHOOT_MAX				( 50.0f )

#define _CRAWL_SOUND					( "SwarmerCrawl" )
#define _ATTACK_SOUND					( "SwarmerAttack" )
#define _SOUND_BANK						( "Swarmer" )

static f32 _kfFreqRandTimeOutMin = 0.5f;
static f32 _kfFreqRandTimeOutMax = 1.5f;
static f32 _kfNearbyEnemyNotifyDist = 100.0f;
static f32 _kfMaxEnemyAttackDist = 300.0f;

static f32 _kfSwarmerInitialHealth = 0.02f;
//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// Swarmer flocking
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************
FCLASS_ALIGN_PREFIX class CSwarmerFlock
{
public:
	static BOOL InitLevel(void);
	static void UninitLevel(void);
	static void CheckpointRestore(void);
	static void Work(void);
	static void Add(CBotSwarmer* pSwarmer);
	static void Remove(CBotSwarmer* pSwarmer);
	static void _HandleSounds( CBotSwarmer *pSwarmer0, CBotSwarmer *pSwarmer1 );
	static BOOL CheckSoundTimer( void );
	static u32 AllowDeathEffects( BOOL bOneJustDied );		// this fn will return 0 for no, 1 for light effects, 2 for go ahead, impress me

	static const u32 m_kuMaxSepForces;
	static const f32 m_kfSepRadius;
	static CFVec3A *m_paSepForceDir;
	static f32 *m_pafSepForceMag;

	static FLinkRoot_t m_SwarmerFlockList;	 //list of all swarmers to be flocked	   //findfix: could separate into separate lists for multiple flocks 
	static CFSphereA m_SwarmBounds;
	static CESpline* m_pHiddenSpawnPts;
	static CESpline* m_pVisibleSpawnPts;
	static u32 m_uObstacleCount;
	static CAIBrain* m_pBrain;
	static CAIMover* m_pMover;
	static CFAudioEmitter *m_pSwarmerEmitter0;
	static CFAudioEmitter *m_pSwarmerEmitter1;
	static CFSoundGroup *m_pCrawlSoundGroup;
	static CFSoundGroup *m_pAttackSoundGroup;
	static f32 m_fAttackSoundTimer;
	static u32 m_uDeadSwarmersThisFrame;

	FCLASS_STACKMEM_ALIGN( CSwarmerFlock );
} FCLASS_ALIGN_SUFFIX;

const u32 CSwarmerFlock::m_kuMaxSepForces = CBotSwarmer::MAX_SEP_NEIGHBORS;
const f32 CSwarmerFlock::m_kfSepRadius = 2.0f;
FLinkRoot_t CSwarmerFlock::m_SwarmerFlockList;	 //list of all swarmers to be flocked	   //findfix: could separate into separate lists for multiple flocks 
CFVec3A *CSwarmerFlock::m_paSepForceDir = NULL;
f32 *CSwarmerFlock::m_pafSepForceMag = NULL;
u32 CSwarmerFlock::m_uObstacleCount = 0;
CAIBrain* CSwarmerFlock::m_pBrain = NULL;
CAIMover* CSwarmerFlock::m_pMover = NULL;
CFAudioEmitter* CSwarmerFlock::m_pSwarmerEmitter0 = NULL;
CFAudioEmitter* CSwarmerFlock::m_pSwarmerEmitter1 = NULL;
CFSoundGroup* CSwarmerFlock::m_pCrawlSoundGroup = NULL;
CFSoundGroup* CSwarmerFlock::m_pAttackSoundGroup = NULL;
f32 CSwarmerFlock::m_fAttackSoundTimer = 0.0f;
u32 CSwarmerFlock::m_uDeadSwarmersThisFrame;




BOOL CSwarmerFlock::InitLevel(void)
{
	FResFrame_t ResFrame;
	ResFrame = fres_GetFrame();

	m_paSepForceDir = (CFVec3A*) fres_AlignedAllocAndZero(sizeof(CFVec3A)*m_kuMaxSepForces, 16);
	m_pafSepForceMag = (f32*) fres_AlignedAllocAndZero(sizeof(f32)*m_kuMaxSepForces, 16);
	if (!m_paSepForceDir || !m_pafSepForceMag)
	{
		goto ExitSwarmerFlockInitSysWithError;
	}

	flinklist_InitRoot(&m_SwarmerFlockList, FANG_OFFSETOF(CBotSwarmer, m_FlockLink));

	m_pBrain = fnew CAIBrain();
	m_pMover = fnew CAIMover();
	if (!m_pBrain || !m_pMover)
	{
		goto ExitSwarmerFlockInitSysWithError;
	}

	m_pBrain->Create(m_pMover);

	m_pBrain->SetHearingMagUnalert(1.0f);
	m_pBrain->SetHearingMagAlert(1.0f);
	m_pBrain->SetEyeScanDistUnalert(75.0f);
	m_pBrain->SetEyeScanDistAlert(-1.0f);
	m_pBrain->SetBonusEyeScanDistForPlayersUnalert(100.0f);
	m_pBrain->SetBonusEyeScanDistForPlayersAlert(200.0f);
	m_pBrain->SetEyeCosHalfFOVUnalert(-1.0f);
	m_pBrain->SetEyeCosHalfFOVAlert(-1.0f);

	ai_IgnorePerceptor(m_pBrain, AI_PERCEPTOR_EYES); 	// Ignore means DON"T do things base on the input from this perceptor
	ai_IgnorePerceptor(m_pBrain, AI_PERCEPTOR_EARS); 	// Ignore means DON"T do things base on the input from this perceptor
	ai_IgnorePerceptor(m_pBrain, AI_PERCEPTOR_TOUCH); 	// Ignore means DON"T do things base on the input from this perceptor
	ai_IgnorePerceptor(m_pBrain, AI_PERCEPTOR_RADIO); 	// Ignore means DON"T do things base on the input from this perceptor

	m_fAttackSoundTimer = 0.0f;

	return TRUE;

ExitSwarmerFlockInitSysWithError:
	fres_ReleaseFrame(ResFrame);
	return FALSE;
}


void CSwarmerFlock::UninitLevel(void)
{
	FASSERT(m_SwarmerFlockList.nCount == 0);
	flinklist_InitRoot(&m_SwarmerFlockList, FANG_OFFSETOF(CBotSwarmer, m_FlockLink));

	m_paSepForceDir = NULL;	 //don't delete since fres_ handles it
	m_pafSepForceMag = NULL;//don't delete since fres_ handles it

	if (m_pBrain)
	{
		m_pBrain->ClearBrain();
	}
	fdelete (m_pBrain); m_pBrain = NULL;
	fdelete (m_pMover); m_pMover = NULL;
}

void CSwarmerFlock::CheckpointRestore(void)
{
	if (m_pBrain)
	{
		m_pBrain->ClearBrain();
	}
}


#define _MAX_SOUND_RANGE_SQ ( 50.0f * 50.0f )

//static AISoundHandle uTestBarrier = 0;
void CSwarmerFlock::Work(void)
{
	CBotSwarmer *pSoundSwarmer0 = NULL;
	CBotSwarmer *pSoundSwarmer1 = NULL;

	f32 fDist0 = _MAX_SOUND_RANGE_SQ;
	f32 fDist1 = _MAX_SOUND_RANGE_SQ;
	f32 fDist;

	m_uDeadSwarmersThisFrame = 0;

	FASSERT( Player_aPlayer[0].m_pEntityCurrent && Player_aPlayer[0].m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT );
	CBot *pPlayerBot = (CBot*)(Player_aPlayer[0].m_pEntityCurrent);
	

	if (CSwarmerFlock::m_SwarmerFlockList.nCount ==0)
	{
		return;
	}

	//
	// find swarmer with the longest time since last update.
	//	  there is only one swarmer brain work per frame.  
	//    it moves around all active swarmers, frame to frame
	//
	f32 fNowTime = aiutils_GetCurTimeSecs();
	f32 fOldestTime = -1.0f;
	CBotSwarmer* pBrainSwarmer = NULL;
	CBotSwarmer* pIt;
	pIt = (CBotSwarmer* ) flinklist_GetHead(&CSwarmerFlock::m_SwarmerFlockList);
	while (pIt)
	{
		if (!pBrainSwarmer || pIt->m_fLastBrainUpdateTime < pBrainSwarmer->m_fLastBrainUpdateTime)
		{
			pBrainSwarmer = pIt;
		}

		// check to see which two swarmers are closest to the player
		fDist = pIt->m_MtxToWorld.m_vPos.DistSq( pPlayerBot->MtxToWorld()->m_vPos );
		if( !pIt->IsDeadOrDying() ) {
			if( fDist < fDist0 )
			{
				pSoundSwarmer0 = pIt;
				fDist0 = fDist;
			} 
			else if( fDist < fDist1 )
			{
				pSoundSwarmer1 = pIt;
				fDist1 = fDist;
			}
		}

		pIt = (CBotSwarmer* ) flinklist_GetNext(&CSwarmerFlock::m_SwarmerFlockList, pIt);

	}
	
	_HandleSounds( pSoundSwarmer0, pSoundSwarmer1 );

	if (pBrainSwarmer)
	{
		pBrainSwarmer->m_fLastBrainUpdateTime = fNowTime;
		m_pMover->m_pEntity = pBrainSwarmer;
		m_pMover->BeginFrame();
		m_pBrain->Work();
		m_pMover->EndFrame();
	}

	if (CAIBrain::m_bGlobalBlindDefAndDumb || CAIBrain::m_bGlobalIgnoreMode)
	{
		//
		//  No enemies during global ignore (cutscenes) or blindefanddumb mode
		//

		pIt = (CBotSwarmer* ) flinklist_GetHead(&CSwarmerFlock::m_SwarmerFlockList);
		while (pIt)
		{
			pIt->m_pEnemy = NULL;
			pIt = (CBotSwarmer* ) flinklist_GetNext(&CSwarmerFlock::m_SwarmerFlockList, pIt);
		}
	}
	else
	{

		//
		//	Does the brain, remember seeing any other ai based bots?
		//
		CNiIterator<CAIMemoryLarge*> it;
		it.Reset();
		CAIMemoryLarge* pMem = NULL;
		CEntity* pOtherEntity = NULL;
	#define MAX_GLOBAL_SWARMER_ENEMIES 5
		CBot* apPotentialEnemy[MAX_GLOBAL_SWARMER_ENEMIES];
		u32 uNumGlobalSwarmerEnemies = 0;
		while ( uNumGlobalSwarmerEnemies < MAX_GLOBAL_SWARMER_ENEMIES &&
				(pMem = m_pBrain->GetKnowledge().NextUnAcknowledgeMemory(MEMORY_LARGE_SEEN, &it)))
		{
			pOtherEntity = pMem->m_pEntity;
			if (pOtherEntity &&
				(pOtherEntity->TypeBits() & ENTITY_BIT_BOT) &&
				pOtherEntity->IsInWorld() &&
				!(pOtherEntity->TypeBits() & ENTITY_BIT_BOTSWARMER))//aiutils_IsPlayer(pOtherEntity))
			{
				apPotentialEnemy[uNumGlobalSwarmerEnemies] = (CBot*) pOtherEntity;
				uNumGlobalSwarmerEnemies++;
			}
		}


		//
		// Assign enemies to swarmer, so that they start attacking!!!!
		//
		pIt = (CBotSwarmer* ) flinklist_GetHead(&CSwarmerFlock::m_SwarmerFlockList);
		while (pIt)
		{
			if (pIt->_IsSwarmer() && !pIt->m_pEnemy)
			{
				CBot* pClosestEnemy = NULL;
				for (u32 i = 0; i < uNumGlobalSwarmerEnemies; i++)
				{
					f32 fDistSq = apPotentialEnemy[i]->MtxToWorld()->m_vPos.DistSq(pIt->MtxToWorld()->m_vPos);
					if (fDistSq < _kfMaxEnemyAttackDist*_kfMaxEnemyAttackDist &&
						(!pClosestEnemy || fDistSq < pClosestEnemy->MtxToWorld()->m_vPos.DistSq(pIt->MtxToWorld()->m_vPos)))
					{
						pClosestEnemy = apPotentialEnemy[i];
					}
				}

				if (pClosestEnemy)
				{
					pIt->m_pEnemy = pClosestEnemy;
					pIt->m_uEnemyGUID = pClosestEnemy->Guid();
				}
			}
			pIt = (CBotSwarmer* ) flinklist_GetNext(&CSwarmerFlock::m_SwarmerFlockList, pIt);
		}

	/*
		CFVec3A BarrierTestWhere(-404.0f, 0.f, 94.0f);  //JumpTest location

		if (!uTestBarrier || !AIEnviro_ModifySound(uTestBarrier, BarrierTestWhere, 20.0f, 0.0f, AISOUNDTYPE_SWARMERBARRIER, NULL))
		{
			uTestBarrier = AIEnviro_AddSound(BarrierTestWhere, 20.0f, 0.0f, AISOUNDTYPE_SWARMERBARRIER, AISOUNDCTRL_PERSISTENT, NULL);
		}
		*/
	}
}


void CSwarmerFlock::Add(CBotSwarmer* pSwarmer)
{
	flinklist_AddHead(&m_SwarmerFlockList, pSwarmer);
}


void CSwarmerFlock::Remove(CBotSwarmer* pSwarmer)
{
	flinklist_Remove(&m_SwarmerFlockList, pSwarmer);
}

#define _SOUND_VELOCITY		( 100.0f )


void CSwarmerFlock::_HandleSounds( CBotSwarmer *pSoundSwarmer0, CBotSwarmer *pSoundSwarmer1 ) {
	f32 fMag2;
	CFVec3A vToSound;

	if( pSoundSwarmer0 ) {
		if( m_pSwarmerEmitter0 ) {

			vToSound.v3.Sub( pSoundSwarmer0->MtxToWorld()->m_vPos.v3, m_pSwarmerEmitter0->GetBoundingSphere().m_Pos );
			fMag2 = vToSound.MagSq();       
			if( (fMag2 > _SOUND_VELOCITY * FLoop_fPreviousLoopSecs) && (fMag2 < _MAX_SOUND_RANGE_SQ) ) {
				vToSound.Mul( fmath_InvSqrt( fMag2 ) * _SOUND_VELOCITY * FLoop_fPreviousLoopSecs );
			}
			vToSound.v3.Add( m_pSwarmerEmitter0->GetBoundingSphere().m_Pos );

			m_pSwarmerEmitter0->SetPosition( &vToSound );
		} else {
			m_pSwarmerEmitter0 = CFSoundGroup::AllocAndPlaySound( m_pCrawlSoundGroup, FALSE,  &(pSoundSwarmer0->MtxToWorld()->m_vPos) );
		}
	} else if( m_pSwarmerEmitter0 ) {
		m_pSwarmerEmitter0->Destroy();
		m_pSwarmerEmitter0 = NULL;
	}

	if( pSoundSwarmer1 ) {
		if( m_pSwarmerEmitter1 ) {

			vToSound.v3.Sub( pSoundSwarmer1->MtxToWorld()->m_vPos.v3, m_pSwarmerEmitter1->GetBoundingSphere().m_Pos );
			fMag2 = vToSound.MagSq();       
			if( (fMag2 > _SOUND_VELOCITY * FLoop_fPreviousLoopSecs) && (fMag2 < _MAX_SOUND_RANGE_SQ) ) {
				vToSound.Mul( fmath_InvSqrt( fMag2 ) * _SOUND_VELOCITY * FLoop_fPreviousLoopSecs );
			}
			vToSound.v3.Add( m_pSwarmerEmitter1->GetBoundingSphere().m_Pos );

			m_pSwarmerEmitter1->SetPosition(  &vToSound );
		} else {
			m_pSwarmerEmitter1 = CFSoundGroup::AllocAndPlaySound( m_pCrawlSoundGroup, FALSE,  &(pSoundSwarmer1->MtxToWorld()->m_vPos) );
		}
	} else if( m_pSwarmerEmitter1 ) {
		m_pSwarmerEmitter1->Destroy();
		m_pSwarmerEmitter1 = NULL;
	}

	m_fAttackSoundTimer -= FLoop_fPreviousLoopSecs;
}


BOOL CSwarmerFlock::CheckSoundTimer( void ) {
	if( m_fAttackSoundTimer <= 0.0f ) {
		m_fAttackSoundTimer = fmath_RandomFloatRange( 0.25f, 0.33f );
		return TRUE;
	} else {
		return FALSE;
	}
}


u32 CSwarmerFlock::AllowDeathEffects( BOOL bOneJustDied ) {
	FASSERT( m_uDeadSwarmersThisFrame >= 0 );

	if( bOneJustDied ) {
		m_uDeadSwarmersThisFrame++;
	}

	if( m_uDeadSwarmersThisFrame == 1 ) {
		return 2;
	}

	if( m_uDeadSwarmersThisFrame % 4 ) {
		return 0;
	} else {
		return 1;
	}
}


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotSwarmerBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CBotSwarmerBuilder _BotSwarmerBuilder;

void CBotSwarmerBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_BOTSWARMER, pszEntityType );

	m_pszEC_ArmorProfile = CBotSwarmer::m_BotInfo_Gen.pszArmorProfile;
}


BOOL CBotSwarmerBuilder::PostInterpretFixup( void ) {
	if( !CBotBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	return TRUE;

	// Error:
_ExitWithError:
	return FALSE;
}


BOOL CBotSwarmerBuilder::InterpretTable( void ) {
	return CBotBuilder::InterpretTable();
}



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotSwarmer
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CBotSwarmer::m_bSystemInitialized = FALSE;
BOOL CBotSwarmer::m_bDataLoaded = FALSE;

CBotAnimStackDef CBotSwarmer::m_AnimStackDef;

CFCollInfo CBotSwarmer::m_CollInfo;										// Used for collision detection
CFTrackerCollideRayInfo CBotSwarmer::m_TrackerCollInfo;
CBotSwarmer *CBotSwarmer::m_pThisBot = NULL;							// Used for collision detection
CDamageProfile *CBotSwarmer::m_pDamageProfile = NULL;
FParticle_DefHandle_t CBotSwarmer::m_hSparkParticleDef = FPARTICLE_INVALID_HANDLE;

//CBotSwarmer::BotInfo_Gen_t		CBotSwarmer::m_BotInfo_Gen;
//CBotSwarmer::BotInfo_MountAim_t	CBotSwarmer::m_BotInfo_MountAim;
CBotSwarmer::BotInfo_Walk_t			CBotSwarmer::m_BotInfo_Walk;
CBotSwarmer::BotInfo_Weapon_t		CBotSwarmer::m_BotInfo_Weapon;
CBotSwarmer::BotInfo_Swarmer_t		CBotSwarmer::m_BotInfo_Swarmer;
CFDebrisGroup*						CBotSwarmer::m_pDeathDebrisGroup = NULL;

CBotSwarmer::BotInfo_Gen_t CBotSwarmer::m_BotInfo_Gen = {
	0.0f,							//	f32 fGravity;

	0.0f, 							//	f32 fCollSphere1X_MS;
	2.9f, 							//	f32 fCollSphere1Y_MS;
	0.0f, 							//	f32 fCollSphere1Z_MS;
	3.0f, 							//	f32 fCollSphere1Radius_MS;

	NULL,							// cchar *pszDataPortBoneName;					// NULL=Bot cannot have a data port
	NULL, NULL, NULL,				// cchar *apszDataPortMeshName[3];				// [0]=closed, [1]=opened, [2]=effect   (NULL=none)

	1.0f,							// f32 fPowerDownSpeedMult;						// Anim speed multiplier applied to the power-down animation
	1.0f,							// f32 fPowerUpSpeedMult;						// Anim speed multiplier applied to the power-up animation
	30.0f,							// f32 fDisguiseTime;							// Amount of time a possessed bot will be disguised
	150.0f,							// f32 fPossessionDist;							// Maximum distance a possessed bot can travel
	"none",						// cchar *pszWeightClass;						// Weight class of the bot: 'light', 'medium' and 'heavy'

	1,								// u32 nBatteryCount;							// Number of batteries (health containers) for this bot
	"Swarmer",						// cchar *pszArmorProfile;						// The default armor profile to use with this bot
};


CBotSwarmer::BotInfo_MountAim_t CBotSwarmer::m_BotInfo_MountAim = {
	0.191f,						//# fMountPitchUpDownRevsPerSec
	FMATH_DEG2RAD( 50.0f ),		//# fMountPitchDownLimit
	FMATH_DEG2RAD( -30.0f ),	//# fMountPitchUpLimit
	2.5f,						//# fMountYawBaseRevsPerSec
	2.5f,						//# fMountYawOverdriveRevsPerSec
	2.0f,						//# fYawDeltaUnitIntoOverdrivePerSec
	10.0f,						//# fYawDeltaUnitOutofOverdrivePerSec
	0.93f,						//# fYawUnitIntoControlOverdriveThresh
	0.89f,						//# fYawUnitOutofControlOverdriveThresh
	2.5f						//# fMountYawAIQuickTurnRevsPerSec
};

BOOL CBotSwarmer::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );

	m_bSystemInitialized = TRUE;

	fang_MemZero( &m_BotInfo_Walk, sizeof( m_BotInfo_Walk ) );
	fang_MemZero( &m_BotInfo_Weapon, sizeof( m_BotInfo_Weapon ) );

	if( !_BuildAnimStackDef() ) {
		goto _ExitInitSystemWithError;
	}

	return TRUE;

	// Error:
_ExitInitSystemWithError:
	FASSERT_NOW;
	UninitSystem();
	return FALSE;
}


void CBotSwarmer::WorkSystem( void ) {
	CSwarmerFlock::Work();
}

void CBotSwarmer::UninitSystem( void ) {
	if( m_bSystemInitialized ) {
		m_AnimStackDef.Destroy();
		m_bSystemInitialized = FALSE;
	}
}


BOOL CBotSwarmer::InitLevel( void )
{
	return CSwarmerFlock::InitLevel();
}


void CBotSwarmer::UninitLevel( void )
{
	CSwarmerFlock::UninitLevel();

	//hi nate, had to stick these here, cause it was xing on the next level with swarmers
	m_pDamageProfile = NULL;
	m_hSparkParticleDef = FPARTICLE_INVALID_HANDLE;
	m_bDataLoaded = FALSE;
}




CBotSwarmer::CBotSwarmer() : CBot() {
	// NKM - Set this here since it is used in Create() to set the type of the bot
	//		 Should never be set back to 0 again.
	m_uFlags = 0; 
}

CBotSwarmer::~CBotSwarmer() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld(TRUE);
		ClassHierarchyDestroy();

	}
}

BOOL CBotSwarmer::Create( s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName, BotType_e eBotType /*= BotTypeSwarmer*/ ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	if( eBotType == BotTypeSwarmer ) {
		_SetSwarmer();
	} else {
		_ClearSwarmer();
	}

	CBotSwarmerBuilder *pBuilder = (CBotSwarmerBuilder *) GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_BotSwarmerBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOTSWARMER );
	}

	m_fPauseChance = 0.60f;
	m_fPauseSecsRemaining = 0.f;

	// set up any builder params...
	return CBot::Create( &m_BotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}

void CBotSwarmer::CheckpointSaveSelect( s32 nCheckpoint ) {
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}

BOOL CBotSwarmer::CheckpointSave( void ) { 
	CBot::CheckpointSave();

	//CFCheckPoint::SaveData( m_uFlags );

	return TRUE;
}

void CBotSwarmer::CheckpointRestore( void ) { 
	CBot::CheckpointRestore();

	_ClearJumping();
	_ClearGravity();
	_ClearStopped();
	_ClearChangingSurface();

	PositionForSpawn( m_MtxToWorld.m_vPos );

	_ResetAI();

	m_uNumHitsTaken = 0;
}

void CBotSwarmer::ApplyVelocityImpulse_WS( const CFVec3A &rVelocityImpulseVec_WS ) {
	CBot::ApplyVelocityImpulse_WS( rVelocityImpulseVec_WS );

	_SetGravity();
	_ClearOnSurface();

	m_fUnitJump = 0.0f;

	if( _IsSwarmer() ) {
		ZeroTime( ANIMTAP_JUMP );
	}

	StickToSurface( CFVec3A::m_UnitAxisY, CFVec3A::m_UnitAxisX, CFVec3A::m_UnitAxisZ );

	// Must reset these since they are set in StickToSurface().
	_SetGravity();
	_ClearOnSurface();
	_SetChangingSurface();

	_SetSpin();

	m_fYawWobble = fmath_RandomFloat();
	m_fYawWobbleDir = FMATH_FSIGN( m_ImpulseVelocity_WS.Dot( m_MoveDirectionForward ) );
	m_fYawWobbleDir *= fmath_RandomFloatRange( FMATH_2PI, FMATH_4PI );
}

void CBotSwarmer::InflictDamageResult( const CDamageResult *pDamageResult ) {
	FASSERT( pDamageResult );

	//CBot::InflictDamageResult( pDamageResult );

	if( (m_uNumHitsTaken > 1) || (fmath_RandomFloat() < 0.5f) ) {
		Die();
	} else {
		m_uNumHitsTaken++;
	}
	
	//f32 fHealth = NormHealth();
	//FMATH_CLAMP_MAX1( fHealth );
	//fHealth -= fmath_RandomFloatRange( 0.33f, 1.5f );

	//if( fHealth <= 0.0f ) {
	//	Die();
	//} else {
	//	SetNormHealth( fHealth );
	//}
	
	if( pDamageResult->m_pDamageData->m_pEpicenter_WS == NULL ) {
		return;
	}

	CFVec3A Impulse = pDamageResult->m_Impulse_WS;
	
	// If we got something > 0
	if( Impulse.MagSq() > FMATH_POS_EPSILON ) {
		CFVec3A ImpactToMe;
		const CFVec3A *pEpicenter_WS;
		f32 fBlastRadius2;
		f32 fDist2;

		pEpicenter_WS = pDamageResult->m_pDamageData->m_pEpicenter_WS;
		fBlastRadius2 = pDamageResult->m_pDamageData->m_pDamageProfile->m_ImpulseRange.m_fOuterRadius;
		fBlastRadius2 *= fBlastRadius2;

		if( fBlastRadius2 < FMATH_POS_EPSILON ) {
			return;
		}

		Impulse.Unitize();
		Impulse.Add( m_SurfaceNormal ); // So we can fly off the surface we are on
		Impulse.Unitize();

		ImpactToMe.Sub( m_MtxToWorld.m_vPos, *pEpicenter_WS );
		fDist2 = ImpactToMe.MagSq();

		if( fDist2 > FMATH_POS_EPSILON ) {
			f32 fUnitDis = fDist2 * fmath_Inv( fBlastRadius2 );

			Impulse.Mul( FMATH_FPOT( 1.0f - fUnitDis, _SWARMER_SHOOT_MIN, _SWARMER_SHOOT_MAX ) );
			ApplyVelocityImpulse_WS( Impulse );
		}
	}
}

/* 	static */
void CBotSwarmer::CheckpointRestoreSystem(void)
{
	CSwarmerFlock::CheckpointRestore();
}

void CBotSwarmer::DetachFromParent( void ) {
	if( m_pBot ) {
		--m_pBot->m_uNumAttachedSwarmers;
		m_pBot = 0;
		_SetGravity();
		_ClearOnSurface();

		CFVec3A Vel;
		
		Vel.Sub( m_MountPos_WS, m_MountPrevPos_WS );

		m_Velocity_WS.Add( Vel );
		WS2MS( m_Velocity_MS, m_Velocity_WS );
		VelocityHasChanged();
	}

	CBot::DetachFromParent();
}

f32 CBotSwarmer::ComputeEstimatedGroundStopTime( void ) {
	FASSERT( IsCreated() );
	return ComputeEstimatedControlledStopTimeXZ();
}

f32 CBotSwarmer::ComputeEstimatedControlledStopTimeXZ( void ) {
	FASSERT( IsCreated() );
	return 0.0f;
}


f32 CBotSwarmer::ComputeEstimatedControlledStopTimeY( void ) {
	FASSERT( IsCreated() );
	return 0.0f;
}

void CBotSwarmer::ClassHierarchyDestroy( void ) {
	m_Anim.Destroy();

	fdelete( m_pWorldMesh );
	m_pWorldMesh = NULL;

	CBot::ClassHierarchyDestroy();
}

BOOL CBotSwarmer::ClassHierarchyBuild( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	FMesh_t		*pMesh;
	FMeshInit_t  MeshInit;
	FResFrame_t  frame = fres_GetFrame();
	s32			 uBoneIdx;
	CBotSwarmerBuilder *pBuilder = NULL;		//CPS 4.7.03
	cchar *pszMeshName = NULL;					//CPS 4.7.03
	
	if( !_LoadData() ) {
		goto _ExitWithError;
	}

	// Get pointer to the leaf class's builder object...
//CPS 4.7.03	CBotSwarmerBuilder *pBuilder = (CBotSwarmerBuilder *) GetLeafClassBuilder();
	pBuilder = (CBotSwarmerBuilder *) GetLeafClassBuilder();

	// Set input parameters for CBot creation...
	pBuilder->m_pBotDef = &m_BotDef;

	// Build parent class...
	if( !CBot::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	// Create the mesh...
//CPS 4.7.03	cchar *pszMeshName = _IsSwarmer() ?  _BOTINFO_SWARMER_MESHNAME : _BOTINFO_VERMIN_MESHNAME;
	pszMeshName = _IsSwarmer() ?  _BOTINFO_SWARMER_MESHNAME : _BOTINFO_VERMIN_MESHNAME;

	pMesh = (FMesh_t*) fresload_Load( FMESH_RESTYPE, pszMeshName );
	if( pMesh == NULL ) {
		DEVPRINTF( "CBotSwarmer::ClassHierarchyBuild(): Could not find mesh '%s'\n", pszMeshName );
		goto _ExitWithError;
	}

	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL ) {
		DEVPRINTF( "CBotSwarmer::ClassHierarchyBuild(): Not enough memory to create CFWorldMesh.\n" );
		goto _ExitWithError;
	}

	MeshInit.pMesh		= pMesh;
	MeshInit.nFlags		= 0;
	MeshInit.fCullDist	= FMATH_MAX_FLOAT;
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

	// We have lots of bones for the Swarmer
	if( _IsSwarmer() ) {
		// Build our animation stack and load animations...
		if( !m_Anim.Create( &m_AnimStackDef, m_pWorldMesh ) ) {
			DEVPRINTF( "CBotSwarmer::ClassHierarchyBuild(): Trouble creating m_Anim.\n" );
			goto _ExitWithError;
		}

		SetControlValue( ANIMCONTROL_STAND, 1.0f );
		UpdateUnitTime( ANIMTAP_STAND, fmath_RandomFloat() );

		// Initialize matrix palette (this is important for attached entities)...
		AtRestMatrixPalette();

		// Find tag point...
		uBoneIdx = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
		if( uBoneIdx < 0 ) {
			DEVPRINTF( "CBotSwarmer::ClassHierarchyBuild(): Could not locate tag point bone '%s'.\n", m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
		} else {
			TagPoint_CreateFromBuiltInVector( &m_pWorldMesh->GetBoneMtxPalette()[uBoneIdx]->m_vPos );
		}
	}

	// Set approx eye point...
	m_pApproxEyePoint_WS = m_pTagPointArray_WS;

	// Set gaze direction...
	m_pGazeDir_WS = &m_MtxToWorld.m_vFront;

	fresload_Load( FSNDFX_RESTYPE, _SOUND_BANK );
	if( !CSwarmerFlock::m_pCrawlSoundGroup ) {
		CSwarmerFlock::m_pCrawlSoundGroup = CFSoundGroup::RegisterGroup( _CRAWL_SOUND );
	}

	if( !CSwarmerFlock::m_pAttackSoundGroup ) {
		CSwarmerFlock::m_pAttackSoundGroup = CFSoundGroup::RegisterGroup( _ATTACK_SOUND );
	}

	// Misc stuff...
	SetMaxHealth();
	SetUnitHealth(_kfSwarmerInitialHealth);
	SetBotFlag_Enemy();

//	m_pWorldMesh->SetMeshTint(0.0f, 1.0f, 0.0f);

	return TRUE;

// Error:
_ExitWithError:
	FASSERT_NOW;
	Destroy();
	fres_ReleaseFrame( frame );
	return FALSE;
}

BOOL CBotSwarmer::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CBot::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	// All bots, by default, cast shadows.  THis one doesn't
	m_pWorldMesh->m_nFlags &= ~FMESHINST_FLAG_CAST_SHADOWS;

	EnableOurWorkBit();

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

CEntityBuilder *CBotSwarmer::GetLeafClassBuilder( void ) {
	return &_BotSwarmerBuilder;
}



void CBotSwarmer::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	//m_uBotDeathFlags |= BOTDEATHFLAG_PLAYDEATHANIM;
	//m_uBotDeathFlags |= BOTDEATHFLAG_AUTOPERSISTAFTERDEATH;

	CBot::ClassHierarchyAddToWorld();
	m_pWorldMesh->UpdateTracker();

	_SetGravity();
	_ClearOnSurface();

	m_fYawCurr = m_fYawPrev = m_fMountYaw_WS;
	
	//ai states
	_ResetAI();

	EnableOurWorkBit();

	m_uNumHitsTaken = 0;

	SetUnitHealth(_kfSwarmerInitialHealth);
	CSwarmerFlock::Add(this);
}


void CBotSwarmer::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	m_pWorldMesh->RemoveFromWorld();

	CBot::ClassHierarchyRemoveFromWorld();

	CSwarmerFlock::Remove(this);
}


static f32 _CalcJumpVelFor45DegreeTakeoffTrajectory(f32 dy, f32 dxz, f32 a)
{
	if (dxz < dy)
	{
		dxz = dy + 2.0f;  //somebody is trying to take off at a little more than a 45deg angle, adjust it
	}
	f32 y0 = -0.5f* FMATH_SQRT2 * fmath_Sqrt((dy - dxz)*a)*dxz/(dy-dxz);
	return  y0;
}

static BOOL SwarmerAIObstacleCollisionCallback(CAISound* pSound, void* pData)		  //CB should return TRUE to stop iteration
{
	if (pSound->m_uSoundType == AISOUNDTYPE_SWARMERBARRIER || (pSound->m_uSoundCtrlFlags & AISOUNDCTRL_SWARMERBARRIER))
	{
		CSwarmerFlock::m_paSepForceDir[CSwarmerFlock::m_uObstacleCount] = pSound->m_Origin;
		CSwarmerFlock::m_pafSepForceMag[CSwarmerFlock::m_uObstacleCount] = pSound->m_fAudibleRadius;
		CSwarmerFlock::m_uObstacleCount++;
	}
	return FALSE;
}

BOOL _WSPtToSurf(const CFVec3A& SurfNorm, const CFVec3A& SurfPt, const CFVec3A& Pt_WS, CFVec3A* pRes)
{
	CFVec3A DeltaToGoal;
	DeltaToGoal.Sub(Pt_WS, SurfPt);
	*pRes = SurfNorm;
	(*pRes).Mul(-SurfNorm.Dot(DeltaToGoal));
	(*pRes).Add(Pt_WS);
	return TRUE;
}
		   

void CBotSwarmer::_ResetAI(void)
{
	SpawnLoc = MtxToWorld()->m_vPos;
	m_fJumpTimeOut = 0.0f;;
	m_uSwarmerFlags = SWARMERFLAG_NONE;
	m_pEnemy = NULL;
	m_fFreqRand = fmath_RandomFloat();
	m_fAddToWorldTime = aiutils_GetCurTimeSecs();
	m_fFreqRandTimeOut = m_fAddToWorldTime + _kfFreqRandTimeOutMin+ (_kfFreqRandTimeOutMax - _kfFreqRandTimeOutMin) * fmath_RandomFloat();
	m_fLastBrainUpdateTime = 0.0f;
	m_fNeighborEvalTimeOut = 0.0f;
	m_fPauseChance = 0.60f;
	m_fPauseSecsRemaining = 0.f;
	m_uNumNeighbors = 0;

	m_pSpawnNet = CSpawnSys::FindSwarmersSpawnNet(this);
}

void CBotSwarmer::_DoAIWork(BOOL bIsPaused, BOOL* pbAIWouldLikeToUnPausePlease)
{
	if (m_fFreqRandTimeOut < aiutils_GetCurTimeSecs())
	{
		m_fFreqRand = fmath_RandomFloat();
		m_fFreqRandTimeOut = aiutils_GetCurTimeSecs()+ _kfFreqRandTimeOutMin+ (_kfFreqRandTimeOutMax - _kfFreqRandTimeOutMin) * fmath_RandomFloat();
	}

	//swarmer AI
	m_fControls_Fire1 = 0.0f;
	m_bControlsBot_JumpVec = FALSE;
	if (pbAIWouldLikeToUnPausePlease)
	{
		*pbAIWouldLikeToUnPausePlease = FALSE;
	}

	//
	// When To Bite
	//
	if (!bIsPaused)
	{
		if (GetParent() && GetParent() == m_pEnemy)
		{
			m_fControls_Fire1 = 1.0f;
		}
		else
		{
			m_fControls_Fire1 = 0.0f;
		}
	}
	
	//
	// When To Move or Jump
	//
	if (_IsOnSurface() && !(GetParent() && GetParent() == m_pEnemy))
	{
		//steer toward goal.
		m_fControls_RotateCW = 0.0f;
		m_fControlsHuman_Forward = 0.0f;

		CFVec3A SurfaceFront = m_MoveDirectionForward;//MtxToWorld()->m_vFront;//m_SurfaceUp ;
		CFVec3A SurfaceNormal = m_SlerpedUp;//m_pWorldMesh->GetBoneMtxPalette()[0]->m_vUp;//m_SurfaceNormal;	  //MtxToWorld()->m_vUp;//
		CFVec3A SurfaceRight = m_MoveDirectionRight;//MtxToWorld()->m_vRight;//m_SurfaceRight;
		CFVec3A SwarmerLoc = MtxToWorld()->m_vPos;
		CFVec3A GoalForceDir = SurfaceFront;
		f32 fGoalForceDamp = 1.0f;
		u32 uCount = 0;
		f32 fBaseSpeed = 0.15f;
		f32 fBaseThrustSpeed = 0.5f;

		if (SurfaceNormal.SafeUnitAndMag(SurfaceNormal) > 0.0f &&
			SurfaceRight.SafeUnitAndMag(SurfaceRight) > 0.0f)
		{

			//
			// Movement Logic
			//

			
			//
			// Goal!
			//
			BOOL bHasEnemyGoal = FALSE;
			CFVec3A GoalLoc;
			if (m_pEnemy)
			{
				if (m_pEnemy->TypeBits() & ENTITY_BIT_BOT)
				{
					CBot* pBot = (CBot*) m_pEnemy;
					GoalLoc = *(pBot->GetTagPoint(0));				  //findfix: is there any better way to find out where this swarmer should attack on the bot?
				}
				else
				{
					GoalLoc = *(m_pEnemy->GetTagPoint(0));
				}
				bHasEnemyGoal = TRUE;
			}
			else
			{
				GoalLoc = SpawnLoc;
				GoalLoc.x+= m_fFreqRand*10.0f- 5.0f;		 //-5.0f to 5.0f
				GoalLoc.y+= m_fFreqRand*10.0f- 5.0f;		 //-5.0f to 5.0f
				GoalLoc.z+= m_fFreqRand*10.0f- 5.0f;		 //-5.0f to 5.0f
				fBaseSpeed *=m_fFreqRand;
			}
			CFVec3A DeltaToGoal;
			DeltaToGoal.Sub(GoalLoc, SwarmerLoc);
			CFVec3A UnitDeltaToGoal = DeltaToGoal;
			f32 fDistToGoal= 0.0f;
			f32 fNormalToGoalUnitDot = 0.5f;
			f32 fAbsNormalToGoalUnitDot = 0.5f;
			if ((fDistToGoal = UnitDeltaToGoal.SafeUnitAndMag(UnitDeltaToGoal)) > 0.0f)
			{
				f32 fNormalDot = SurfaceNormal.Dot(DeltaToGoal);
				CFVec3A SurfaceGoal = SurfaceNormal;
				SurfaceGoal.Mul(-fNormalDot);
				SurfaceGoal.Add(GoalLoc);
				SurfaceGoal.Sub( MtxToWorld()->m_vPos);

				fGoalForceDamp = 1.0f;
				fNormalToGoalUnitDot = FMATH_FABS(fNormalDot/fDistToGoal);
				fAbsNormalToGoalUnitDot = FMATH_FABS(fNormalToGoalUnitDot);
				f32 fMinPot = 0.9f;
				f32 fMaxPot = 1.0f;
				if (fAbsNormalToGoalUnitDot > fMinPot)
				{
					fGoalForceDamp = (fMaxPot - fAbsNormalToGoalUnitDot) / (fMaxPot - fMinPot);
				}
				if (SurfaceGoal.SafeUnitAndMag(SurfaceGoal) > 0.0f)
				{	
					GoalForceDir = SurfaceGoal;
				}
			}

				
			if (!bIsPaused)
			{
				//
				// Separation
				//

				//Find Neighbors
				f32 fDistSq = 0.0f;

				if (m_fNeighborEvalTimeOut < aiutils_GetCurTimeSecs())
				{
					if ((Guid() & 7) == (FVid_nFrameCounter & 7))
					{
						m_uNumNeighbors = 0;
						CBotSwarmer* pSep = (CBotSwarmer* ) flinklist_GetHead(&CSwarmerFlock::m_SwarmerFlockList);
						while (pSep && m_uNumNeighbors < MAX_SEP_NEIGHBORS)
						{
							if (pSep != this)	 //could check to see if in same volume here?
							{
								CSwarmerFlock::m_paSepForceDir[uCount].Sub(pSep->MtxToWorld()->m_vPos, MtxToWorld()->m_vPos);
								fDistSq = pSep->MtxToWorld()->m_vPos.DistSq(MtxToWorld()->m_vPos);
								if (fDistSq > 0.0f && fDistSq < CSwarmerFlock::m_kfSepRadius*CSwarmerFlock::m_kfSepRadius)
								{
									m_apNeighbor[m_uNumNeighbors] = pSep;
									m_uNumNeighbors++;
								}
							}
							pSep = (CBotSwarmer* ) flinklist_GetNext(&CSwarmerFlock::m_SwarmerFlockList, pSep);
						}
						m_fNeighborEvalTimeOut = aiutils_GetCurTimeSecs()+0.5f; //half second
					}
				}

				CFVec3A SepSumForceSurface = CFVec3A::m_Null;
				CFVec3A SepSumForce = CFVec3A::m_Null;
				f32 fSepSumForceMag = 0.0f;
				uCount = 0;
				if (m_uNumNeighbors)
				{
					for (u32 i = 0;i < m_uNumNeighbors; i++)
					{
						if (m_apNeighbor[i] && m_apNeighbor[i]->IsInWorld())
						{
							CSwarmerFlock::m_paSepForceDir[uCount].Sub(m_apNeighbor[i]->MtxToWorld()->m_vPos, MtxToWorld()->m_vPos);
	/*

							f32 fDist = CSwarmerFlock::m_paSepForceDir[uCount].SafeUnitAndMag(CSwarmerFlock::m_paSepForceDir[uCount]);
							if (fDist > 0.0f && fDist < CSwarmerFlock::m_kfSepRadius)
							{
								CSwarmerFlock::m_paSepForceDir[uCount].Mul(fDist*(1.0f/CSwarmerFlock::m_kfSepRadius));
								SepSumForce.Add(CSwarmerFlock::m_paSepForceDir[uCount]);
								uCount++;
							}
	*/

							f32 fDistSq = CSwarmerFlock::m_paSepForceDir[uCount].MagSq();
							if (fDistSq > 0.0f && fDistSq < (CSwarmerFlock::m_kfSepRadius*CSwarmerFlock::m_kfSepRadius))
							{
						//		CSwarmerFlock::m_paSepForceDir[uCount].Mul(fDistSq*(1.0f/(CSwarmerFlock::m_kfSepRadius*CSwarmerFlock::m_kfSepRadius)));
								SepSumForce.Add(CSwarmerFlock::m_paSepForceDir[uCount]);
								uCount++;
							}


						}
					}
					if (uCount)
					{
						SepSumForce.Mul(-1.0f*fmath_Inv((f32) uCount));
						fSepSumForceMag = SepSumForce.SafeUnitAndMag(SepSumForce);


						//SepSupForceSurface
						f32 fSepNormalDot = SurfaceNormal.Dot(SepSumForce);
						SepSumForceSurface = SurfaceNormal;
						SepSumForceSurface.Mul(-fSepNormalDot);
						SepSumForceSurface.Add(SepSumForce);
					}
				}

				//
				// Obstacles
				//
				CSwarmerFlock::m_uObstacleCount = 0;
				AIEnviro_DetectSoundCollisions(MtxToWorld()->m_vPos,
												this,	  //listener
												1.0f,	  //the radius of all sounds will be magnified by this (represents ear quality)
												SwarmerAIObstacleCollisionCallback,
												this);	  //cb data
				//find most offensive obstacle
				CFVec3A ObstacleOrigin;
				f32 fObstacleRad;
				s32 uCurObstacle = -1;
				f32 fMaxPenetration = 0.0f;
				for (u32 i = 0; i < CSwarmerFlock::m_uObstacleCount; i++)
				{
					f32 fPenetration = CSwarmerFlock::m_pafSepForceMag[i] - MtxToWorld()->m_vPos.Dist(CSwarmerFlock::m_paSepForceDir[i]);
					if (uCurObstacle == -1 || fPenetration > fMaxPenetration)
					{
						uCurObstacle = i;
						fMaxPenetration = fPenetration;
					}
				}
				FASSERT(uCurObstacle == -1 || fMaxPenetration >= 0.0f);
				if (uCurObstacle>-1)
				{
					ObstacleOrigin = CSwarmerFlock::m_paSepForceDir[uCurObstacle];
					fObstacleRad = CSwarmerFlock::m_pafSepForceMag[uCurObstacle];
				}


    			//
				// Total Steering
				//
				f32 fSepWeight = 0.3f;
				f32 fGoalWeight = 1.0f - fSepWeight;
				CFVec3A Tmp;
				Tmp = SepSumForceSurface;
				Tmp.Mul(fSepWeight);
				CFVec3A WeightedSumSurfaceGoal = Tmp;
				Tmp = GoalForceDir;
				Tmp.Mul(fGoalWeight);
				WeightedSumSurfaceGoal.Add(Tmp);

				CFVec3A AvoidedWeightedSumSurfaceGoal = WeightedSumSurfaceGoal;
				CFVec3A TempNorm;
				if (uCurObstacle>-1)
				{
					CFVec3A ObstacleOriginSurface;
					_WSPtToSurf(SurfaceNormal, MtxToWorld()->m_vPos, ObstacleOrigin, &ObstacleOriginSurface);
					f32 fDistSq = MtxToWorld()->m_vPos.DistSq(ObstacleOrigin);
					f32 fObstacleSurfaceRad = fmath_Sqrt(fObstacleRad*fObstacleRad - fDistSq);

					f32 fAvoidDist = 3.0f;
					if (MtxToWorld()->m_vPos.DistSq(ObstacleOriginSurface) > (fObstacleSurfaceRad-fAvoidDist)*(fObstacleSurfaceRad-fAvoidDist)   )
					{	//Steer around
						//rotate steering right or left 90degs to avoid obstacle
						Tmp.Sub(ObstacleOriginSurface, MtxToWorld()->m_vPos);
						if (Tmp.SafeUnitAndMag(Tmp) > 0.0f && Tmp.Dot(WeightedSumSurfaceGoal) > 0.1f)
						{
							TempNorm.Cross(AvoidedWeightedSumSurfaceGoal, Tmp);
							AvoidedWeightedSumSurfaceGoal.Cross(TempNorm, Tmp);
							if (AvoidedWeightedSumSurfaceGoal.MagSq() < 0.08f)
							{
								AvoidedWeightedSumSurfaceGoal.Cross(SurfaceNormal, Tmp);
							}
							if (AvoidedWeightedSumSurfaceGoal.SafeUnitAndMag(WeightedSumSurfaceGoal) < 0.0f)
							{
								AvoidedWeightedSumSurfaceGoal = WeightedSumSurfaceGoal;
							}
						}
					}
					else
					{	//Repel
						AvoidedWeightedSumSurfaceGoal.Sub(MtxToWorld()->m_vPos, ObstacleOriginSurface);
						if (AvoidedWeightedSumSurfaceGoal.SafeUnitAndMag(AvoidedWeightedSumSurfaceGoal) <= 0.0f)
						{
							AvoidedWeightedSumSurfaceGoal = SurfaceFront;
						}
					}
				}

				//
				// convert total steerforce to vehicle model
				//
				f32 fSteerGoal = SurfaceRight.Dot(AvoidedWeightedSumSurfaceGoal);
				if (SurfaceFront.Dot(AvoidedWeightedSumSurfaceGoal) < 0.0f)
				{
					fSteerGoal = FMATH_FSIGN(fSteerGoal);
				}
				m_fControls_RotateCW = fGoalForceDamp*fSteerGoal;
				f32 fThrust = AvoidedWeightedSumSurfaceGoal.Dot(SurfaceFront);  //speed based on alignment with goal
				if (fThrust < 0.0f)
				{
					fThrust = 0.0f;
				}
	//					m_fControlsHuman_Forward = 0.15f;	 test code for slowing them down
				m_fControlsHuman_Forward = fBaseSpeed+fBaseThrustSpeed*fThrust;
			}

			//
			// Jump logic
			//
			if (bHasEnemyGoal)
			{
				if (m_fJumpTimeOut < aiutils_GetCurTimeSecs())
				{
					CFVec3A IdealVec;;

					if (fDistToGoal < 15.0f)
					{
						//
						// jump at goal to attack it!
						//
						if (SurfaceNormal.Dot(CFVec3A::m_UnitAxisY) > -0.707f)
						{
							//jump off at a positive 45 degree angle
							f32 fIdealJumpDist = DeltaToGoal.MagXZ();
							if (fIdealJumpDist > 0.0f)
							{
								f32 fIdealJumpSpeed = _CalcJumpVelFor45DegreeTakeoffTrajectory(GoalLoc.y - SwarmerLoc.y, fIdealJumpDist, _GRAVITY);
								IdealVec = DeltaToGoal;
								IdealVec.y = 0.0f;
								IdealVec.Unitize();
								IdealVec.y = 1.0f;
								IdealVec.Mul(fIdealJumpSpeed);  
								m_bControlsBot_JumpVec = TRUE;
							}
						}
						else
						{	//
							// drop off at current velocity
							//
							IdealVec = UnitDeltaToGoal;
							f32 fSpeed = m_fSpeed_WS;
							IdealVec.Mul(fSpeed);  
							m_bControlsBot_JumpVec = TRUE;
						}
					}
					else if (_MissedAllRays())
					{
						if (SurfaceNormal.Dot(CFVec3A::m_UnitAxisY) > -0.707f)
						{
							IdealVec = UnitDeltaToGoal;
							IdealVec.Add(SurfaceNormal);
							IdealVec.Mul(0.5f);
							f32 fSpeed = m_fSpeed_WS*1.3f;
							if (fSpeed < 10.0f)
							{
								fSpeed = 10.0f;
							}
							IdealVec.Mul(fSpeed);  
							m_bControlsBot_JumpVec = TRUE;
						}
						else
						{	//drop off at current velocity
 							IdealVec = UnitDeltaToGoal;
							IdealVec.Add(SurfaceNormal);
							IdealVec.Mul(0.5f);
							IdealVec.Mul(m_fSpeed_WS);  
							m_bControlsBot_JumpVec = TRUE;
						}
					}
					else if (fNormalToGoalUnitDot > 0.0f && fAbsNormalToGoalUnitDot > 0.95f)		   //we've done the best we can do on this sruface, why not try another?
					{	//Jump off surface hoping for something good to land on
						if (SurfaceNormal.Dot(CFVec3A::m_UnitAxisY) > -0.707f)
						{
							IdealVec = UnitDeltaToGoal;
							f32 fSpeed = m_fSpeed_WS*1.3f;
							if (fSpeed < 10.0f)
							{
								fSpeed = 10.0f;
							}
							IdealVec.Mul(fSpeed);  
							m_bControlsBot_JumpVec = TRUE;
						}
						else
						{	//drop off at current velocity
 							IdealVec = UnitDeltaToGoal;
							IdealVec.Mul(m_fSpeed_WS);  
							m_bControlsBot_JumpVec = TRUE;
						}
					}
					
					if (m_bControlsBot_JumpVec)
					{
						m_ControlsBot_JumpVelocity_WS = IdealVec;
						m_fJumpTimeOut = aiutils_GetCurTimeSecs()+ 1.5f+2.5f*fmath_RandomFloat();
						if (pbAIWouldLikeToUnPausePlease)
						{
							*pbAIWouldLikeToUnPausePlease = TRUE;
						}
					}
				}
			}
			else if (!bIsPaused)
			{
				//no goal, must be swarming, so jump every now and then.
				if (SurfaceNormal.Dot(CFVec3A::m_UnitAxisY) > 0.707f &&
					m_fFreqRand < 0.4f &&
					m_fJumpTimeOut < aiutils_GetCurTimeSecs())
				{
					m_fJumpTimeOut =  aiutils_GetCurTimeSecs()+ 1.5f+2.5f*fmath_RandomFloat();
					f32 fIdealJumpDist = 5.0f;

					if (fIdealJumpDist > 0.0f)
					{
						f32 fIdealJumpSpeed = _CalcJumpVelFor45DegreeTakeoffTrajectory(0.0f, fIdealJumpDist, _GRAVITY);
						CFVec3A IdealVec = SurfaceFront;
						IdealVec.y = 0.0f;
						IdealVec.Unitize();
						IdealVec.y = 1.0f;
						IdealVec.Mul(fIdealJumpSpeed);  
						m_ControlsBot_JumpVelocity_WS = IdealVec;
						m_bControlsBot_JumpVec = TRUE;
					}

				}
			}
		}
#if !FANG_PRODUCTION_BUILD
		CFVec3A End;
		End = SurfaceNormal;
		End.Mul(10.0f);
		End.Add(MtxToWorld()->m_vPos);
		_aiutils_DebugTrackRay(MtxToWorld()->m_vPos, End, 0);

		End = SurfaceFront;
		End.Mul(10.0f);
		End.Add(MtxToWorld()->m_vPos);
		_aiutils_DebugTrackRay(MtxToWorld()->m_vPos, End, 1);

		End = SurfaceRight;
		End.Mul(10.0f);
		End.Add(MtxToWorld()->m_vPos);
		_aiutils_DebugTrackRay(MtxToWorld()->m_vPos, End, 2);
#endif // !FANG_PRODUCTION_BUILD
	}
}

void CBotSwarmer::ClassHierarchyWork( void ) {
	FASSERT( m_bSystemInitialized );

	CBot::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}

	//swarmers die if the go outside of the slop bucket
	if (!aiutils_IsInBoundsOfWorld(this))
	{
		RemoveFromWorld();
		return;
	}

	// if
	//		Not in active volume &&
	//		Don't have an enemy &&
	//		it isn't an odd/even frame of the first 10 seconds of my life
	//
	// then
	//		skip work
	if( !(m_pWorldMesh->GetVolumeFlags() & FVIS_VOLUME_IN_ACTIVE_LIST) &&
		!m_pEnemy &&
		!( aiutils_GetCurTimeSecs() - m_fAddToWorldTime < 10.0f && ((Guid() & 1) == (FVid_nFrameCounter & 1)) )) {
		return;	 
	}

	m_fControls_Fire1 = 0.0f;
	BOOL bPaused = TRUE;
	if ( m_fPauseSecsRemaining <= 0.f )
	{
		if ( m_fPauseSecsRemaining < 0.f )
		{
			m_fPauseSecsRemaining += FLoop_fPreviousLoopSecs;
			if ( m_fPauseSecsRemaining > 0.f )
			{
				m_fPauseSecsRemaining = 0.f;
			}
		}

		if ( m_fPauseSecsRemaining == 0.f && _IsOnSurface() && fmath_RandomFloat() < m_fPauseChance )
		{
			m_fPauseSecsRemaining = (fmath_RandomFloat() * 1.f) + 0.4f;
			m_fControls_RotateCW = 0.f;
			m_fControlsHuman_Forward = 0.f;
			m_bControlsBot_JumpVec = FALSE;
//			SetControlValue( ANIMCONTROL_WALK, 0.f );
			_SetStopped();
			m_fJumpTimeOut = aiutils_GetCurTimeSecs()+ 1.5f+2.5f*fmath_RandomFloat();
		}
		else
		{
			bPaused = FALSE;
		}
	}
	else
	{
		m_fPauseSecsRemaining -= FLoop_fPreviousLoopSecs;
		if ( m_fPauseSecsRemaining < 0.f || !_IsOnSurface() )
		{
			m_fPauseSecsRemaining = (fmath_RandomFloat() * -4.f) - 1.f;
		}
	}
	BOOL bAINeedsUnPaused = FALSE;
	_DoAIWork(bPaused, &bAINeedsUnPaused);


//	ComputeXlatStickInfo();	   //pgm notices that ComputeXlatStickInfo convert m_fControls into m_XlatStickUnitVecXZ_WS. Swarmer doesn't use m_XlatStickUnitVecXZ_WS, 

	// Save a copy of the previous frame's info...
	m_MountPrevPos_WS	= m_MountPos_WS;
	m_nPrevState		= m_nState;

	// Apply velocity impulses that were accumulated last frame...
	BOOL bImpulseApplied = HandleVelocityImpulses();

	// let CBot handle the mount yaw and pitch so it's consistent w/ other bots
	HandleYawMovement();
	//HandlePitchMovement();

	// Rotate model space velocity to account for yaw change...
	WS2MS( m_Velocity_MS, m_Velocity_WS );

	// If an external velocity impulse was applied above, recompute our velocities
	// and speeds...
	if( bImpulseApplied ) {
		VelocityHasChanged();
	}

	PROTRACK_BEGINBLOCK("Move");
	if ( m_fPauseSecsRemaining <= 0.f )
	{
		_HandleTranslation();
	}
	PROTRACK_ENDBLOCK();//"Move"

	PROTRACK_BEGINBLOCK("Coll");
	
	// Need to do this so we can slerp correctly
	m_fUnitRot += ( _ROTATE_TO_PLANE_RAMP * FLoop_fPreviousLoopSecs );
	FMATH_CLAMPMAX( m_fUnitRot, 1.0f );

	if ( m_fPauseSecsRemaining <= 0.f )
	{
		_HandleCollision();
	}
	PROTRACK_ENDBLOCK();//"Coll"

	_HandleAttack();
	_HandleAnimations();
	_UpdateMatrices();

	if( IsDeadOrDying() ) {
		DeathWork();
	}
}

void CBotSwarmer::ClassHierarchyRelocated( void *pIdentifier ) {
	CBot::ClassHierarchyRelocated( pIdentifier );

	if( IsInWorld() ) {
		m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
		m_pWorldMesh->UpdateTracker();
	}
}

// Up means forward on the surface we are going to stick to
void CBotSwarmer::StickToSurface( const CFVec3A &SurfaceNormal, const CFVec3A &SurfaceRight, const CFVec3A &SurfaceUp ) {
	CFVec3A TempRight = SurfaceRight;

	m_SurfaceNormalPrev = SurfaceNormal;
	m_SurfaceNormal = SurfaceNormal;

	m_SurfaceRight.Cross( m_SurfaceNormal, SurfaceUp );

	if( m_SurfaceRight.MagSq() < 0.001f ) {
		//m_SurfaceRight.Sub( Impact.aTriVtx[1], Impact.aTriVtx[0] ).Unitize();
		m_SurfaceRight = TempRight;
	} else {
		m_SurfaceRight.Unitize();
	}

	m_SurfaceUp.Cross( m_SurfaceRight, m_SurfaceNormal ).Unitize();

	CFMtx43A::m_Temp.m_vUp = m_SurfaceNormal;
	CFMtx43A::m_Temp.m_vRight = m_SurfaceRight;
	CFMtx43A::m_Temp.m_vFront = m_SurfaceUp;

	m_PrevQuat = m_CurrQuat;
	m_CurrQuat.BuildQuat( CFMtx43A::m_Temp );

	m_fUnitRot = 0.0f;

	_ClearGravity();
	_SetOnSurface();
	_SetChangingSurface();
}

void CBotSwarmer::_ClearDataMembers( void ) {
	m_pBotInfo_Gen		= &m_BotInfo_Gen;
	m_pBotInfo_MountAim	= &m_BotInfo_MountAim;
	m_pBotInfo_Walk		= &m_BotInfo_Walk;
	m_pBotInfo_Jump		= NULL;
	m_pBotInfo_Weapon	= &m_BotInfo_Weapon;
	m_pMoveIdentifier	= &m_uFlags;

	m_pGazeDir_WS = NULL;

	// Swarmer specific stuff
	_ClearJumping();
	_SetGravity();
	_ClearOnSurface();
	_ClearStopped();
	_ClearChangingSurface();

	m_SurfaceNormal = CFVec3A::m_UnitAxisY;
	m_SurfaceNormalPrev = m_SurfaceNormal;
	m_SurfaceUp.Set( CFVec3A::m_UnitAxisZ ).Mul( 1.0f );
	m_SurfaceRight = CFVec3A::m_UnitAxisX;

	m_MoveDirectionForward.Zero();
	m_MoveDirectionRight.Zero();

	m_fTestAngle = 0.0f;

	m_PrevQuat.Identity();
	m_CurrQuat.Identity();

	m_fUnitWalkBlend = 0.0f;
	m_fUnitAirBlend = 0.0f;
	m_fUnitAttackBlend = 0.0f;
	m_fUnitJump = 0.0f;

	m_pBot = 0;

	m_fYawCurr = 0.0f;
	m_fYawPrev = 0.0f;
	m_fUnitYaw = 0.0f;
	
	m_fTimeTillNextBite = 0.0f;
	
	m_fMaxFlatSurfaceSpeed_WS = _MAX_VELOCITY_XZ;
	m_fMaxVerticalSpeed_WS = _MAX_VELOCITY_XZ;
	m_fJumpTimeOut = 0.0f;

	m_fYawWobble = -1.0f;
	m_fYawWobbleDir = _WOBBLE_RATE;

	m_SlerpedUp.Zero();
}

BOOL CBotSwarmer::_LoadData( void ) {
	if( m_bDataLoaded ) {
		return TRUE;
	}

	m_CollInfo.pTag = NULL;
	m_CollInfo.nCollTestType = FMESH_COLLTESTTYPE_RAY;
	m_CollInfo.bCalculateImpactData = TRUE;
	m_CollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_NONE;
	m_CollInfo.bFindClosestImpactOnly = FALSE;
	m_CollInfo.nCollMask = FCOLL_MASK_CHECK_ALL;
	m_CollInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
//	m_CollInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
	m_CollInfo.nTrackerUserTypeBitsMask = ~(ENTITY_BIT_BOTSWARMER|ENTITY_BIT_BOT);
	m_CollInfo.bCullBacksideCollisions = TRUE;

	m_TrackerCollInfo.pCallback = _CollRayWithTrackersCallback;
//	m_TrackerCollInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
	m_TrackerCollInfo.nTrackerUserTypeBitsMask =  ~(ENTITY_BIT_BOTSWARMER|ENTITY_BIT_BOT);
	m_TrackerCollInfo.nTrackerTypeBits = FWORLD_TRACKERTYPEBIT_MESH;
	m_TrackerCollInfo.bIgnoreCollisionFlag = FALSE;
	m_TrackerCollInfo.bComputeIntersection = FALSE;
	m_TrackerCollInfo.bSort = FALSE;
	m_TrackerCollInfo.nTrackerSkipCount = 0;
	m_TrackerCollInfo.ppTrackerSkipList = NULL;

	m_pDamageProfile  = CDamage::FindDamageProfile( "SwarmerBite" );

	m_hSparkParticleDef = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, _ATTACK_SPARK_NAME );
		
	if( m_hSparkParticleDef == FPARTICLE_INVALID_HANDLE ) {
		DEVPRINTF( "CBotSwarmer::_LoadData(): Could not find particle definition '%s'.\n", _ATTACK_SPARK_NAME );
	}

	m_pDeathDebrisGroup = CFDebrisGroup::Find( _DEATH_DEBRIS_GROUP );
	if( !m_pDeathDebrisGroup ) {
		DEVPRINTF( "CBotSwarmer::InitSystem():  Unable to load debris group %s\n", _DEATH_DEBRIS_GROUP );
		return FALSE;
	}

	m_bDataLoaded = TRUE;

	return TRUE;
}

void CBotSwarmer::_UpdateMatrices( void ) {
	f32 fYaw;

	fYaw = FMATH_FPOT( m_fUnitYaw, m_fYawPrev, m_fYawCurr );

	// We are on the ground
	if( !_IsGravity() ) {
		CFQuatA WallQuat;
		
		WallQuat.ReceiveSlerpOf( m_fUnitRot, m_PrevQuat, m_CurrQuat );
		WallQuat.BuildMtx( CFMtx43A::m_Temp );
		m_SlerpedUp = CFMtx43A::m_Temp.m_vUp;

		CFMtx43A::m_XlatRotY.SetRotationY( fYaw );
		CFMtx43A::m_XlatRotY.m_vPos.Zero();
		CFMtx43A::m_Temp.Mul( CFMtx43A::m_XlatRotY );
		CFMtx43A::m_Temp.m_vPos = m_MountPos_WS;
	} else {
		// Jumping through the air
		if( !_IsSpin() ) {
			CFQuatA WallQuat;
			CFQuatA QuatRight, QuatUp, QuatForward;

			m_fYawWobble += ( m_fYawWobbleDir * FLoop_fPreviousLoopSecs );

			if( m_fYawWobble >= 1.0f ) {
				m_fYawWobbleDir = -_WOBBLE_RATE;
			} else if( m_fYawWobble <= 0.0f ) {
				m_fYawWobbleDir = _WOBBLE_RATE;
			}

			FMATH_CLAMP( m_fYawWobble, 0.0f, 1.0f );

			if( m_fYawWobble != -1.0f ) {
				f32 fAmnt = FMATH_FPOT( m_fYawWobble, -_YAW_WOBBLE_MAX, _YAW_WOBBLE_MAX );
				fYaw += fAmnt;
			}

			WallQuat.ReceiveSlerpOf( m_fUnitRot, m_PrevQuat, m_CurrQuat );
			WallQuat.BuildMtx( CFMtx43A::m_Temp );
			m_SlerpedUp = CFMtx43A::m_Temp.m_vUp;

			QuatUp.BuildQuat( CFMtx43A::m_Temp.m_vUp, fYaw ); 
			QuatRight.BuildQuat( m_MoveDirectionRight, FMATH_FPOT( m_fYawWobble, -FMATH_QUARTER_PI, FMATH_QUARTER_PI ) );
			QuatForward.BuildQuat( m_MoveDirectionForward, FMATH_FPOT( m_fYawWobble, FMATH_QUARTER_PI, -FMATH_QUARTER_PI ) );

			WallQuat.Mul( QuatUp );
			WallQuat.Mul( QuatRight );
			WallQuat.Mul( QuatForward );

			WallQuat.BuildMtx( CFMtx43A::m_Temp );
			CFMtx43A::m_Temp.m_vPos = m_MountPos_WS;
		} else {
			// Spin all over after a blast
			CFQuatA Quat;

			m_fYawWobble += ( m_fYawWobbleDir * FLoop_fPreviousLoopSecs );
			Quat.BuildQuat( m_MoveDirectionRight, m_fYawWobble );
			Quat.BuildMtx( CFMtx43A::m_Temp );

			m_SlerpedUp = CFMtx43A::m_Temp.m_vUp;

			CFMtx43A::m_XlatRotY.SetRotationY( fYaw );
			CFMtx43A::m_XlatRotY.m_vPos.Zero();
			CFMtx43A::m_Temp.Mul( CFMtx43A::m_XlatRotY );
			CFMtx43A::m_Temp.m_vPos = m_MountPos_WS;
		}
	}

	if( IsDeadOrDying() ) {
		CFQuatA qRot;
		qRot.BuildQuat( m_vDeathRotAxis, m_fDeathRotRate * FLoop_fPreviousLoopSecs );
		qRot.MulPoint( CFMtx43A::m_Temp.m_vFront );
		qRot.MulPoint( CFMtx43A::m_Temp.m_vUp );
		qRot.MulPoint( CFMtx43A::m_Temp.m_vRight );
	}

	m_pWorldMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_Temp );
	m_pWorldMesh->UpdateTracker();

	if( _IsSwarmer() ) {
		PROTRACK_BEGINBLOCK("ComputeMtxPal");
			ComputeMtxPalette( TRUE );
		PROTRACK_ENDBLOCK();//"ComputeMtxPal");
	}

	m_GazeUnitVec_WS.ReceiveUnit( *m_pGazeDir_WS );
	Relocate_RotXlatFromUnitMtx_WS( &CFMtx43A::m_Temp, TRUE, m_pMoveIdentifier );
}

void CBotSwarmer::_HandleAnimations( void ) {
	if( !_IsSwarmer() ) {
		return;
	}

	// Always play the stand animation
	DeltaTime( ANIMTAP_STAND );

	// Walking animation
	if( !_IsStopped() ) {
		DeltaTime( ANIM_WALK, FLoop_fPreviousLoopSecs * 1.5f * m_fSpeed_WS * _MAX_VELOCITY_XZ_OO * 4.0f );
	}

	if( FMATH_FABS( m_fSpeed_WS ) <= 0.0001f || _IsStopped() ) {
		m_fUnitWalkBlend -= FLoop_fPreviousLoopSecs * 2.0f;
	} else {
		m_fUnitWalkBlend += FLoop_fPreviousLoopSecs;
	}

	FMATH_CLAMP( m_fUnitWalkBlend, 0.0f, 1.0f );
	SetControlValue( ANIMCONTROL_WALK, m_fUnitWalkBlend );

	// Jump animation
	if( _IsGravity() ) {
		m_fUnitJump += ( 4.0f * FLoop_fPreviousLoopSecs );

		if( m_fUnitJump >= 1.0f ) {
			m_fUnitJump = -1.0f;
		}

		UpdateTime( ANIMTAP_JUMP, FMATH_FABS( m_fUnitJump ) );
		m_fUnitAirBlend += ( FLoop_fPreviousLoopSecs * 3.0f );
	} else {
		m_fUnitAirBlend -= ( FLoop_fPreviousLoopSecs * 4.0f );
	}

	FMATH_CLAMP( m_fUnitAirBlend, 0.0f, 1.0f );
	SetControlValue( ANIMCONTROL_JUMP, m_fUnitAirBlend );

	if( m_fUnitAttackBlend != 0.0f ) {
		DeltaTime( ANIMTAP_ATTACK, FLoop_fPreviousLoopSecs * 2.0f );
		SetControlValue( ANIMCONTROL_ATTACK, m_fUnitAttackBlend );
	} else {
		SetControlValue( ANIMCONTROL_ATTACK, 0.0f );
	}
}

void CBotSwarmer::_HandleAttack( void ) {
	
	if (m_bControlsBot_JumpVec)	{
		m_Velocity_WS = m_ControlsBot_JumpVelocity_WS;
		WS2MS( m_Velocity_MS, m_Velocity_WS );

		_SetGravity();
		_ClearOnSurface();

		VelocityHasChanged();

		//findfix: Nate, there must be a better way to get these guys of off the surface.
		//CFVec3A BumpUp = m_SurfaceNormal;
		//m_MountPos_WS.Add(BumpUp);

		m_fUnitJump = 0.0f;

		if( _IsSwarmer() ) {
			ZeroTime( ANIMTAP_JUMP );
		}

		StickToSurface( CFVec3A::m_UnitAxisY, CFVec3A::m_UnitAxisX, CFVec3A::m_UnitAxisZ );

		// Must reset these since they are set in StickToSurface().
		_SetGravity();
		_ClearOnSurface();
		_SetChangingSurface();

		m_fYawWobble = fmath_RandomFloat();
		m_fYawWobbleDir = _WOBBLE_RATE;
	}

	if( m_fControls_Fire1 != 0.0f ) {
		m_fUnitAttackBlend += ( 3.0f * m_fControls_Fire1 * FLoop_fPreviousLoopSecs );
		_TestBiteHit();
	} else {
		m_fUnitAttackBlend -= FLoop_fPreviousLoopSecs;
	}

	FMATH_CLAMP( m_fUnitAttackBlend, 0.0f, 1.0f );
}

void CBotSwarmer::_HandleTranslation( void ) {
	CFVec3A vDesVel_WS;
	CFVec3A vDeltaVel_WS;
	CFVec3A MoveDirForward, MoveDirRight, MoveDir;
	f32 fMaxAccelThisFrame = _MAX_ACCEL_XZ * FLoop_fPreviousLoopSecs;
	CFQuatA RotQuat;

	m_fUnitYaw += ( 4.0f * FLoop_fPreviousLoopSecs );
	FMATH_CLAMPMAX( m_fUnitYaw, 1.0f );

	m_MoveDirectionForward = m_SurfaceUp;
	m_MoveDirectionRight = m_SurfaceRight;

	RotQuat.BuildQuat( m_SurfaceNormal, m_fMountYaw_WS );
	RotQuat.MulPoint( m_MoveDirectionForward );
	RotQuat.MulPoint( m_MoveDirectionRight );

	if( !_IsGravity() ) {
		MoveDir.Set( m_MoveDirectionForward ).Mul( m_fControlsHuman_Forward );

		if( MoveDir.MagSq() > 0.0001f ) {
			MoveDir.Unitize();

			if( FMATH_FABS( m_fMountYaw_WS - m_fYawCurr ) > 0.001f ) {
				if( m_fUnitYaw == 1.0f ) {
					m_fUnitYaw = 0.0f;
					m_fYawPrev = m_fYawCurr;
				}
				
				m_fYawCurr = m_fMountYaw_WS;

				// Make sure we take the shortest distance to our destination yaw
				if( m_fYawCurr > 0.0f && m_fYawPrev < 0.0f ) {
					if( m_fYawCurr - m_fYawPrev > ( FMATH_PI - m_fYawCurr ) + ( FMATH_FABS( -FMATH_PI - m_fYawPrev ) ) ) {
						m_fYawPrev += FMATH_2PI;
					}
				} else if( m_fYawCurr < 0.0f && m_fYawPrev > 0.0f ) {
					if( m_fYawPrev - m_fYawCurr > ( FMATH_PI - m_fYawPrev ) + ( FMATH_FABS( -FMATH_PI - m_fYawCurr ) ) ) {
						m_fYawCurr += FMATH_2PI;
					}
				}
			}
		} else {
			MoveDir.Zero();
		}

		vDesVel_WS.Mul( MoveDir, m_fControlsHuman_Forward*_MAX_VELOCITY_XZ );
		vDeltaVel_WS.Sub( vDesVel_WS, m_Velocity_WS );

		if( vDeltaVel_WS.Dot( m_Velocity_WS ) < 0.0f ) {
			fMaxAccelThisFrame *= _DECEL_MULT_XZ;
		}

		if( vDeltaVel_WS.MagSq() < ( fMaxAccelThisFrame * fMaxAccelThisFrame ) ) {
			m_Velocity_WS = vDesVel_WS;
		} else  {
			vDeltaVel_WS.Unitize().Mul( fMaxAccelThisFrame );
			m_Velocity_WS.Add( vDeltaVel_WS );     
		}
	} else {
		m_Velocity_WS.y += ( _GRAVITY * FLoop_fPreviousLoopSecs );
		_ClearStopped();
	}

	WS2MS( m_Velocity_MS, m_Velocity_WS );
	VelocityHasChanged();

	if( m_fSpeed_WS <= 0.0001f ) {
		_SetStopped();
	} else {
		_ClearStopped();
	}
}

void CBotSwarmer::_SetupRay( CFVec3A *pStart, CFVec3A *pEnd, CFVec3A *pVelMove ) {
	FASSERT( pStart );
	FASSERT( pEnd );
	FASSERT( pVelMove );

	CFVec3A Pos = m_MountPos_WS;
	CFVec3A Dir;

	pVelMove->Mul( m_Velocity_WS, FLoop_fPreviousLoopSecs );

	if( _IsGravity() ) {
		pStart->Sub( Pos, *pVelMove );
		pEnd->Add( Pos, *pVelMove );
	} else if( m_fTestAngle == 0.0f ) {
		Dir = m_UnitVelocity_WS;
		Dir.Mul( -1.0f ); // Don't scale, so we don't miss when going fast
		Dir.Add( m_SurfaceNormal );
		Dir.Unitize();

		Pos.Add( *pVelMove );

		pStart->Add( Pos, Dir );
		pEnd->Sub( Pos, Dir );
	} else {
		// Rotate the ray
		CFQuatA Rot;
		CFVec3A Dir = m_SurfaceNormal;
		CFVec3A Vel = *pVelMove;

		Rot.BuildQuat( m_MoveDirectionRight, FMATH_DEG2RAD( m_fTestAngle ) );
		Rot.MulPoint( Dir );
		Rot.MulPoint( Vel );

		Vel.Mul( 0.5f ); // So when we go fast we don't move our ray too much
		Pos.Add( Vel );

		pStart->Mul( Dir, CFVec3A::m_Ones ).Add( Pos );
		pEnd->Mul( Dir, CFVec3A::m_NegOnes ).Add( Pos );
	}
}

void CBotSwarmer::_HandleCollision( void ) {
	if( _IsStopped() ) {
		return;
	}

	FCollImpact_t Impact;
	CFVec3A StartPoint_WS, EndPoint_WS, VelMove;

	m_pThisBot = this;

	_ClearChangingSurface();

	_SetupRay( &StartPoint_WS, &EndPoint_WS, &VelMove );

	m_CollInfo.pTag = NULL;
	m_CollInfo.nCollTestType = FMESH_COLLTESTTYPE_RAY;
	m_CollInfo.bCalculateImpactData = TRUE;
	m_CollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_NONE;
	m_CollInfo.bFindClosestImpactOnly = FALSE;

	m_CollInfo.Ray.Init( &StartPoint_WS, &EndPoint_WS );
	m_TrackerCollInfo.StartPoint_WS = StartPoint_WS;
	m_TrackerCollInfo.EndPoint_WS = EndPoint_WS;

	FWorld_nTrackerSkipListCount = 0;
	FWorld_apTrackerSkipList[FWorld_nTrackerSkipListCount++] = m_pWorldMesh;
	m_TrackerCollInfo.nTrackerSkipCount = FWorld_nTrackerSkipListCount;
	m_TrackerCollInfo.ppTrackerSkipList = FWorld_apTrackerSkipList;	

	fcoll_Clear();
	fworld_CollideWithWorldTris( &m_CollInfo, m_pWorldMesh );
	fworld_CollideWithTrackers( &m_TrackerCollInfo, m_pWorldMesh );
	fcoll_Sort();

	BOOL bHaveCollision = FALSE;

	if( FColl_nImpactCount ) {
		fang_MemCopy( &Impact, FColl_apSortedImpactBuf[0], sizeof( FCollImpact_t ) );

		// NKM - Do this so that when we jump we can not stick to the ground surface
		if( _IsGravity() && !_IsOnSurface() && !Impact.pTag && m_Velocity_WS.Dot( Impact.UnitFaceNormal ) > 0.0f ) {
			bHaveCollision = FALSE;
		} else {
			bHaveCollision = TRUE;
		}
		
		DetachFromParent();

		if( Impact.pTag && bHaveCollision ) {
			// Hit an object...
			CFWorldMesh *pWorldMesh = (CFWorldMesh *) Impact.pTag;
			
			bHaveCollision = FALSE;

			// No need to check for Swarmer vs. Swarmer here since they will be filtered out in collision system
			// See if the world mesh that we found is an entity...
			if( pWorldMesh->m_nUser == MESHTYPES_ENTITY ) {
				if( ( (CEntity *) pWorldMesh->m_pUser )->TypeBits() & ENTITY_BIT_BOT ) {
					BOOL bCanAttach = TRUE;

					m_pBot = (CBot * )pWorldMesh->m_pUser;
					bCanAttach = _AttachToBot();

					if( bCanAttach ) {
						bHaveCollision = TRUE;

						if( Impact.nBoneIndex != -1 ) {
							Attach_ToParent_WithGlue_WS( m_pBot, pWorldMesh->m_pMesh->pBoneArray[Impact.nBoneIndex].szName );
						} else {
							Attach_ToParent_WS( m_pBot );
						}
					} else {
						m_pBot = 0;
					}
				} else {
					bHaveCollision = TRUE;
				}
			} else {
				// Not an entity, just walk on like normal
				bHaveCollision = TRUE;
			}
		}

		if( bHaveCollision ) { 
			CFVec3A Push = Impact.UnitFaceNormal;

			Push.Mul( 0.01f );

			m_MountPos_WS.Set( Impact.ImpactPoint );
			m_MountPos_WS.Add( Push );

			// Only re-stick when surface normal changes
			if( _IsSpin() || _IsGravity() || FMATH_FABS( m_SurfaceNormal.Dot( Impact.UnitFaceNormal ) ) <= 0.9999f ) {
				if( !m_pBot ) {
					// If not bot, stick
					StickToSurface( Impact.UnitFaceNormal, m_SurfaceRight, m_SurfaceUp );
				} else {
					// If we have a bot, only stick once we are finished with the interpolation
					if( m_fUnitRot == 1.0f ) {
						StickToSurface( Impact.UnitFaceNormal, m_SurfaceRight, m_SurfaceUp );
					}
				}
			}

			if( _IsGravity() ) {
				m_Velocity_WS.y = 0.0f;
				WS2MS( m_Velocity_MS, m_Velocity_WS );
				VelocityHasChanged();
			}

			_ClearMissedAllRays();
			_ClearGravity();
			_SetOnSurface();
			_ClearSpin();

			m_fTestAngle = 0.0f;
			m_fYawWobble = -1.0f;
		}
	} 

	if( !bHaveCollision ) {
		if( m_fTestAngle == 0.0f ) {
			m_fTestAngle = -_ANGLE_TEST;
		} else if( m_fTestAngle == -_ANGLE_TEST ) {
			m_fTestAngle = _ANGLE_TEST;
		} else {
			m_fTestAngle = 0.0f;//-_ANGLE_TEST;
			_SetMissedAllRays();
		}

		// NKM - Only move when we have gravity after missing a collision test
		//		 If we don't have gravity applied, we want to stick to our position
		//		 with the hope that we will be able to collide in some other way.
		if( _IsGravity() ) {
			m_MountPos_WS.Add( VelMove );
		}
	}

	if( m_nBotFlags & BOTFLAG_IMMOBILIZE_PENDING ) {
		if( m_fSpeed_WS == 0.0f ) {
			FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_IMMOBILIZE_PENDING );
			FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_IS_IMMOBILE );
		}
	}
}

void CBotSwarmer::_TestBiteHit( void ) {
	m_fTimeTillNextBite -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN( m_fTimeTillNextBite, 0.0f );

	if( m_fTimeTillNextBite == 0.0f ) {
		CFSphere Sphere;
		CFVec3A Pos;

		Pos.Set( CFVec3A::m_UnitAxisY ).Mul( 0.5f ).Add( m_MountPos_WS );

		Sphere.m_Pos = Pos.v3;
		Sphere.m_fRadius = 1.0f;

		m_CollInfo.pSphere_WS = &Sphere;
		m_CollInfo.nCollTestType = FMESH_COLLTESTTYPE_SPHERE;
		m_CollInfo.bCalculateImpactData = TRUE;
		m_CollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_CHECK_ALL;
		m_CollInfo.bFindClosestImpactOnly = FALSE;

		m_pThisBot = this;
		fworld_FindTrackersIntersectingSphere( &Sphere, FWORLD_TRACKERTYPE_MESH, _TrackerCallback, 0, NULL, NULL, MESHTYPES_ENTITY, ENTITY_BIT_BOT );
		m_pThisBot = NULL;

		m_fTimeTillNextBite = _BITE_TIME;
	}
}

BOOL CBotSwarmer::_AttachToBot( void ) {
	if( !m_pBot ) {
		return FALSE;
	}

	if( m_pBot->m_uNumAttachedSwarmers < _MAX_ATTACHED_SWARMERS ) {
		++m_pBot->m_uNumAttachedSwarmers;

		return TRUE;
	}

	return FALSE;
}

void CBotSwarmer::_DetachFromBot( void ) {
	if( !m_pBot ) {
		return;
	}

	--m_pBot->m_uNumAttachedSwarmers;
}

BOOL CBotSwarmer::_CollRayWithTrackersCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume, const CFVec3 *pIntersectionPoint_WS, f32 fUnitDistToIntersection ) {
	if( m_pThisBot->m_pWorldMesh == pTracker ) {
		return TRUE;
	}

	CFWorldMesh *pWorldMesh = (CFWorldMesh *)pTracker;
	FASSERT(pWorldMesh);
	u32 i;

	for( i=0; i<FWorld_nTrackerSkipListCount; i++ ) {
		if( pTracker == FWorld_apTrackerSkipList[i] ) {
			return TRUE;
		}
	}

	if( pWorldMesh->m_nUser == MESHTYPES_ENTITY  && ( (CEntity *) pWorldMesh->m_pUser )->TypeBits() & ENTITY_BIT_BOTSWARMER ) {
		return TRUE;
	}

	m_CollInfo.pTag = pTracker;

	if( pWorldMesh->CollideWithMeshTris( &m_CollInfo ) ) {
		return FALSE;
	} else {
		return TRUE;
	}
}

BOOL CBotSwarmer::_TrackerCallback( CFWorldTracker *pTracker, FVisVolume_t *pWorldLeafNode ) {
	CBot *pBot = (CBot *) pTracker->m_pUser;
	
	if( pBot == m_pThisBot ) {
		return TRUE;
	}

	if( pBot->TypeBits() & ENTITY_BIT_VEHICLE ) {
		return TRUE;
	}

	if( pBot->TypeBits() & ENTITY_BIT_WEAPON ) {
		return TRUE;
	}

	if( pBot->TypeBits() & ENTITY_BIT_BOTSWARMER ) {
		return TRUE;
	}

	if( ((CFWorldMesh *) pTracker)->CollideWithMeshTris( &m_CollInfo ) ) {
		CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

		if( pDamageForm ) {
			// Fill out the form...
			pDamageForm->m_nDamageLocale	= CDamageForm::DAMAGE_LOCALE_AMBIENT;
			pDamageForm->m_nDamageDelivery	= CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
			pDamageForm->m_pDamageProfile	= m_pDamageProfile;
			pDamageForm->m_Damager.pWeapon	= NULL;
			pDamageForm->m_Damager.pBot		= m_pThisBot;
			pDamageForm->m_Damager.nDamagerPlayerIndex = m_pThisBot ? m_pThisBot->m_nPossessionPlayerIndex : -1;
			pDamageForm->m_pDamageeEntity	= pBot;
			
			CDamage::SubmitDamageForm( pDamageForm );

			FASSERT( m_pThisBot && m_pThisBot->TypeBits() & ENTITY_BIT_BOTSWARMER );
			if( CSwarmerFlock::CheckSoundTimer() ) {
				if( pBot->IsPlayerBot() ) {
					CFSoundGroup::PlaySound( CSwarmerFlock::m_pAttackSoundGroup );
				} else {
					CFSoundGroup::PlaySound( CSwarmerFlock::m_pAttackSoundGroup, FALSE, &(m_pThisBot->MtxToWorld()->m_vPos) );
				}
				
			}
		}

		if( FColl_nImpactCount && m_hSparkParticleDef != FPARTICLE_INVALID_HANDLE) {
			fparticle_SpawnEmitter( m_hSparkParticleDef, FColl_aImpactBuf[0].ImpactPoint.v3 );
		}

		return FALSE;
	}

	return TRUE;
}


#define _MAX_RAYS	( 6 )
void CBotSwarmer::TestDirToRay(s32 nDir, CFVec3A* pRay)
{
	switch (nDir%3)
	{
	case 0:
			*pRay = CFVec3A::m_UnitAxisX;
			break;
	case 1:
			*pRay = CFVec3A::m_UnitAxisY;
			break;
	case 2:
			*pRay = CFVec3A::m_UnitAxisZ;
			break;
	};
	if (nDir&1)
	{
		(*pRay).Mul(-1.0f);
	}
}


BOOL CBotSwarmer::PositionForSpawn( const CFVec3A &StartPos, s32 nTestRayDirHint/* = -1*/ ) {
	CFMtx43A Mtx;
	f32 fSpawnRad = 15.0f;

	CFVec3A EndPos;
	CFVec3A RandomDir;
	BOOL bHit = FALSE;
	s32 nMaxRays = _MAX_RAYS;
	s32 nTestRayDir = nTestRayDirHint;
	if (nTestRayDir==-1)
	{
		nTestRayDir = fmath_RandomChoice(_MAX_RAYS);
	}
	CFVec3A Spawn_WS = StartPos;

	FASSERT(m_bDataLoaded);
	
	while( !bHit && nMaxRays>0 ) {
		TestDirToRay(nTestRayDir, &RandomDir);
		nTestRayDir++;

		RandomDir.Mul( fSpawnRad );
		EndPos.Add( StartPos, RandomDir );


		m_CollInfo.pTag = NULL;
		m_CollInfo.nCollTestType = FMESH_COLLTESTTYPE_RAY;
		m_CollInfo.bCalculateImpactData = TRUE;
		m_CollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_NONE;
		m_CollInfo.bFindClosestImpactOnly = TRUE;
		m_CollInfo.Ray.Init( &StartPos, &EndPos );
		m_TrackerCollInfo.StartPoint_WS = StartPos;
		m_TrackerCollInfo.EndPoint_WS = EndPos;

		FWorld_nTrackerSkipListCount = 0;
		FWorld_apTrackerSkipList[FWorld_nTrackerSkipListCount++] = m_pWorldMesh;
		m_TrackerCollInfo.nTrackerSkipCount = FWorld_nTrackerSkipListCount;
		m_TrackerCollInfo.ppTrackerSkipList = FWorld_apTrackerSkipList;	

		fcoll_Clear();
		fworld_CollideWithWorldTris( &m_CollInfo );
//		fworld_CollideWithTrackers( &m_TrackerCollInfo, m_pWorldMesh );
		fcoll_Sort();

		if( FColl_nImpactCount ) {
			if( FColl_aImpactBuf[0].pTag ) {
				// Hit an object...
				CFWorldMesh *pWorldMesh = (CFWorldMesh *) FColl_aImpactBuf[0].pTag;
				
				// No need to check for Swarmer vs. Swarmer here since they will be filtered out in callback
				// See if the world mesh that we found is an entity...
				if( pWorldMesh->m_nUser == MESHTYPES_ENTITY ) {
					if( ( (CEntity *) pWorldMesh->m_pUser )->TypeBits() & ENTITY_BIT_BOT ) {
					} else {
					}
				} else {
					// Not an entity, just walk on like normal
				}
			}
			bHit = TRUE;
		} 
		--nMaxRays;
	}

	if (bHit)
	{
		Mtx.Identity();
		Mtx.m_vPos = FColl_aImpactBuf[0].UnitFaceNormal;
		Mtx.m_vPos.Mul(0.5f);
		Mtx.m_vPos.Add(FColl_aImpactBuf[0].ImpactPoint);
		Mtx.m_vUp = FColl_aImpactBuf[0].UnitFaceNormal;
		Mtx.m_vRight.Cross(Mtx.m_vUp, CFVec3A::m_UnitAxisX);
		if (Mtx.m_vRight.MagSq() < 0.01f)
		{
			Mtx.m_vRight.Cross(Mtx.m_vUp, CFVec3A::m_UnitAxisY);
			if (Mtx.m_vRight.MagSq() < 0.01f)
			{
				Mtx.m_vRight.Cross(Mtx.m_vUp, CFVec3A::m_UnitAxisZ);
			}
		}
		if (Mtx.m_vRight.SafeUnitAndMag(Mtx.m_vRight))
		{
			Mtx.m_vFront.Cross(Mtx.m_vRight, Mtx.m_vUp);
		}
		else
		{
			Mtx.Identity();
		}
	}
	else
	{
		Mtx.Identity();
		Mtx.m_vPos = StartPos;
		Mtx.m_vPos.Add(FColl_aImpactBuf[0].ImpactPoint);
	}
	Relocate_RotXlatFromUnitMtx_WS( &Mtx );
	if (bHit)
	{
		StickToSurface( FColl_aImpactBuf[0].UnitFaceNormal, CFVec3A::m_UnitAxisX, CFVec3A::m_UnitAxisZ );
	}
	return bHit;
}






void CBotSwarmer::DebugRender( f32 fScreenTextX, f32 fScreenTextY ) {

}


void CBotSwarmer::DeathWork( void ) {
	FASSERT( IsDeadOrDying() );

	m_fDeathJumpTimer -= FLoop_fPreviousLoopSecs;

	if( (m_uLifeCycleState == BOTLIFECYCLE_DYING) && (m_fDeathJumpTimer <= 0.0f) ) {
		FExplosionSpawnParams_t SpawnParams;
		FExplosion_SpawnerHandle_t hSpawner;
		FExplosion_GroupHandle_t hExp = CExplosion2::GetExplosionGroup( "DeadSwarmer" );

		SpawnParams.InitToDefaults();
		SpawnParams.uFlags = FEXPLOSION_SPAWN_NONE;
		SpawnParams.Pos_WS = m_MountPos_WS;
		SpawnParams.UnitDir = m_MtxToWorld.m_vUp;
		SpawnParams.uSurfaceType = 0;
		SpawnParams.pDamageProfile = NULL;
		SpawnParams.pDamager = NULL;

		hSpawner = CExplosion2::GetExplosionSpawner();
		if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
			CExplosion2::SpawnExplosion( hSpawner, hExp, &SpawnParams );
		}

		m_uLifeCycleState = BOTLIFECYCLE_DEAD;
		RemoveFromWorld();
	}
}


void CBotSwarmer::Die( BOOL bSpawnDeathEffects/*=TRUE*/, BOOL bSpawnGoodies/*=TRUE*/ ) {
	u32 uEffectLevel = CSwarmerFlock::AllowDeathEffects( TRUE );

	if( bSpawnDeathEffects && uEffectLevel ) {
		// spawn some debris
	
		CFDebrisSpawner DebrisSpawner;
		DebrisSpawner.InitToDefaults();

		DebrisSpawner.m_Mtx.m_vPos			= m_MountPos_WS;
		DebrisSpawner.m_Mtx.m_vZ			= m_MtxToWorld.m_vUp;
		DebrisSpawner.m_nEmitterType		= CFDebrisSpawner::EMITTER_TYPE_POINT;
		DebrisSpawner.m_pDebrisGroup		= m_pDeathDebrisGroup;
		DebrisSpawner.m_fSpawnerAliveSecs	= 0.0f;
		DebrisSpawner.m_fMinSpeed			= 10.0f;
		DebrisSpawner.m_fMaxSpeed			= 20.0f;
		DebrisSpawner.m_fUnitDirSpread		= 0.75f;
		DebrisSpawner.m_fScaleMul			= 1.0f;
		DebrisSpawner.m_fGravityMul			= 1.0f;
		DebrisSpawner.m_fRotSpeedMul		= 1.0f;
		DebrisSpawner.m_nMinDebrisCount		= 1;

		if( uEffectLevel == 2 ) {
			DebrisSpawner.m_nMaxDebrisCount		= 3;
		} else {
			DebrisSpawner.m_nMaxDebrisCount		= 1;
		}
		

		CGColl::SpawnDebris( &DebrisSpawner );

		m_vDeathRotAxis.SetUnitRandom();
		m_fDeathRotRate = fmath_RandomFloatRange( FMATH_DEG2RAD( 60.0f ), FMATH_DEG2RAD( 180.0f ) );

		fmath_ScatterUnitVec( &m_vDeathJumpVel, &(m_MtxToWorld.m_vUp), 0.15f );
		m_vDeathJumpVel.Mul( 15.0f );
		ApplyVelocityImpulse_WS( m_vDeathJumpVel );

		m_fDeathJumpTimer = fmath_RandomFloatRange( 0.01f, 0.5f );
	}

	CBot::Die( FALSE, bSpawnGoodies );
}

