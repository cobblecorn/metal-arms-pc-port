//////////////////////////////////////////////////////////////////////////////////////
// botzombieboss.h -
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
// 01/23/02 Scholz       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _BOTZOMBIEBOSS_H_
#define _BOTZOMBIEBOSS_H_ 1

#include "bot.h"
#include "iteminst.h"
#include "Fxstreamer.h"
#include "EParticle.h"

// Forward Declarations
struct FAnim_t;
class CFDebrisGroup;
class CBotPartPool;

#include "miscDraw.h"
FCLASS_ALIGN_PREFIX class CFHermiteInterpolator
{
public:
	enum Flags
	{
		CREATED				= 0x00000001,	// TRUE after initialized
		OVERRIDE_XZ_LINEAR	= 0x00000002,	// don't Hermite in X, use linear instead
	};

	CFHermiteInterpolator(): m_uFlags(0){}

	BOOL IsCreated(void) {return !!(m_uFlags & CREATED);}
	BOOL Create(const CFVec3A& rBeginPt,const CFVec3A& rBeginTang,const CFVec3A& rEndPt,const CFVec3A& rEndTang)
	{
		m_uFlags |= CREATED;

		m_vBeginPt=rBeginPt;
		m_vBeginTang=rBeginTang;
		m_vEndPt=rEndPt;
		m_vEndTang=rEndTang;
		return TRUE;
	}
	void SetXZLinear(void)
	{
		m_uFlags |= OVERRIDE_XZ_LINEAR;
	}
	void ClearXZLinear(void)
	{
		m_uFlags &= (~OVERRIDE_XZ_LINEAR);
	}


	BOOL Sample(CFVec3A& rSampled, f32 fUnitValue)
	{
		FASSERT(m_uFlags & CREATED);

		f32 ftSquared = fUnitValue*fUnitValue;
		f32 ftCubed = ftSquared * fUnitValue;
		// h0(t) =  2t^3 - 3t^2 + 1
		f32 h0 = 2 * ftCubed - 3 * ftSquared + 1;
		// h1(t) = -2t^3 + 3t^2
		f32 h1 = -2 * ftCubed + 3 * ftSquared;
		// h2(t) =   t^3 - 2t^2 + t
		f32 h2 = ftCubed - 2 * ftSquared + fUnitValue;
		// h3(t) =   t^3 -  t^2
		f32 h3 = ftCubed - ftSquared;
				
		if (m_uFlags & OVERRIDE_XZ_LINEAR)
		{
			rSampled.y = h0 * m_vBeginPt.y + h1 * m_vEndPt.y + h2 * m_vBeginTang.y + h3 * m_vEndTang.y;
			rSampled.x = FMATH_FPOT(fUnitValue, m_vBeginPt.x, m_vEndPt.x);
			rSampled.z = FMATH_FPOT(fUnitValue, m_vBeginPt.z, m_vEndPt.z);
		}
		else
		{
			CFVec3A H0,H1,H2,H3;
			H0.Mul(m_vBeginPt,h0);
			H1.Mul(m_vEndPt,h1);
			H2.Mul(m_vBeginTang,h2);
			H3.Mul(m_vEndTang, h3);
			rSampled.Add(H0,H1);
			rSampled.Add(H2);
			rSampled.Add(H3);
		}
		return TRUE;
	}
#if !FANG_PRODUCTION_BUILD
	BOOL DebugDraw(s32 nLineSegs)
	{
		FASSERT(m_uFlags & CREATED);

		CFVec3A vEndPt,vOtherEndPt;
		f32 fOOLineSegs = 1.0f / (f32)nLineSegs; // distance between pts

		Sample(vEndPt, 0.0f);

		for (s32 nSegment = 1; nSegment <= nLineSegs; nSegment++ )
		{
			f32 fUnitVal = (f32)(nSegment)* fOOLineSegs;
					
			Sample(vOtherEndPt, fUnitVal);
			
			CDebugDraw::Line(vEndPt,vOtherEndPt);

			vEndPt = vOtherEndPt;
		}

		CFColorRGBA yellow;
		yellow.OpaqueYellow();
		
		CDebugDraw::Sphere(m_vBeginPt,0.4f,&FColor_MotifGreen);
		CDebugDraw::Sphere(m_vBeginTang,0.4f,&yellow);
		CDebugDraw::Sphere(m_vEndTang,0.4f,&yellow);
		CDebugDraw::Sphere(m_vEndPt,0.4f,&FColor_MotifRed);
		
		return TRUE;
	}
#endif

private:
	CFVec3A m_vBeginPt;
	CFVec3A m_vBeginTang;
	CFVec3A m_vEndPt;
	CFVec3A m_vEndTang;
	u32 m_uFlags;

}FCLASS_ALIGN_SUFFIX;

FCLASS_ALIGN_PREFIX class CBotZombieBoss : public CBot
{
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum
	{
		HIT_ENTITY_MAX_COUNT = 5,				// Number of entities we can cause damage to during a slash
	};

	typedef enum
	{
		BOSS_CRITICAL_HIT,
	}BossEvent_e;


	typedef enum 
	{
		ZBOSS_SND_BALL_REEL,
		ZBOSS_SND_BALL_DRAG,
		ZBOSS_SND_BALL_SWOOSH,
		ZBOSS_SND_GRAB_CONTACT,
		ZBOSS_SND_EFFORT,
		ZBOSS_SND_DAMAGED,
		ZBOSS_SND_ROAR,
		ZBOSS_SND_DIE,
		ZBOSS_SND_COUNT,
	}ZombieBossSnd_e;

	typedef enum 
	{
		STOP=0,
		START=1,
		START_IF_NOT_PLAYING,
	}SndCommand_e;

	typedef enum
	{
		ZOMBIEBOSS_STANDING_IDLE,
		ZOMBIEBOSS_STANDING_INTO_MOVING,
		ZOMBIEBOSS_STANDING_INTO_GRAB,
		ZOMBIEBOSS_MOVING_IDLE,
		ZOMBIEBOSS_MOVING_INTO_STANDING,
		ZOMBIEBOSS_MOVING_INTO_SWING,
		ZOMBIEBOSS_SWING_PREPARE,
		ZOMBIEBOSS_SWING,
		ZOMBIEBOSS_SWING_ROLL_OUT,
		ZOMBIEBOSS_SWING_TUG_PAUSE,
		ZOMBIEBOSS_SWING_TUG_PULL,
		ZOMBIEBOSS_SWING_TUG_RETURN,
		ZOMBIEBOSS_SWING_ROLL_IN,
		ZOMBIEBOSS_SWING_RETURN,
		ZOMBIEBOSS_SWING_INTO_IDLE,
		ZOMBIEBOSS_GRAB_START,
		ZOMBIEBOSS_GRAB_FAILURE,
		ZOMBIEBOSS_GRAB_SUCCESS,
		ZOMBIEBOSS_GRAB_PICKUP,
		ZOMBIEBOSS_GRAB_ROAR,
		ZOMBIEBOSS_GRAB_THROWN,
		ZOMBIEBOSS_GRAB_INTO_STANDING,
		ZOMBIEBOSS_INTO_DEATH,
		ZOMBIEBOSS_STATE_COUNT,
	}ZombieBossState_e;
//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF

	typedef struct ZombieBossFightLevel_t
	{
		u32 uMoveSpeed;
		f32 fTurnSpeed;
		f32 fBallSpeedXZ;
		f32 fBallAcceleration;
		f32 fBallReturnSpeed;
		CArmorProfile * pArmorProfile;
		f32 fPowerDownTime;
	};


	



//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	// System flags:
	enum SYSTEM_FLAG
	{
		SYS_INITIALIZED				= 0x00000001,	// TRUE after system initialized
	};

	enum
	{
		PART_INSTANCE_COUNT_PER_TYPE = 3,
	};

	// Base animations:
	typedef enum
	{
		ANIM_STAND_LOOK,				// ARMEidle001
		ANIM_STAND_SWING,				// ARZZidle002
		ANIM_WALK,						// ARZZwalkF01

		ANIM_CHAIN_SWING,				// ARZZactn101
		ANIM_CHAIN_ROLL,				// ARZZactn102
		ANIM_CHAIN_RETURN,				// ARZZactn103

		ANIM_ROAR,						// ARZZroar_01
		ANIM_TUG,						// ARZZtug_001

		ANIM_GRAB_ATTACK,				// ARZZactn2dm

		ANIM_FREEZE,					// ARZZfreeze1
		ANIM_DEATH,						// ARZZdeath01

		ANIM_BASE_COUNT
	} Anim_e;

	// Animation controls:
	typedef enum
	{
		ANIMCONTROL_STAND_LOOK,				// ARMEidle001
		ANIMCONTROL_STAND_SWING,			// ARZZidle002
		ANIMCONTROL_WALK,					// ARZZwalkF01

		ANIMCONTROL_CHAIN_SWING_UPPER,		// ARZZactn101
		ANIMCONTROL_CHAIN_ROLL_UPPER,		// ARZZactn102
		ANIMCONTROL_CHAIN_RETURN_UPPER,		// ARZZactn103
		
		ANIMCONTROL_ROAR_UPPER,				// ARZZroar_01
		ANIMCONTROL_TUG_UPPER,				// ARZZtug_001
		
		ANIMCONTROL_CHAIN_SWING_LOWER,		// ARZZactn101
		ANIMCONTROL_CHAIN_ROLL_LOWER,		// ARZZactn102
		ANIMCONTROL_CHAIN_RETURN_LOWER,		// ARZZactn103

		ANIMCONTROL_ROAR_LOWER,				// ARZZroar_01
		ANIMCONTROL_TUG_LOWER,				// ARZZtug_001

		ANIMCONTROL_GRAB_ATTACK,			// ARZZactn2dm

		ANIMCONTROL_FREEZE,					// ARZZfreeze1
		ANIMCONTROL_DEATH,					// ARZZdeath01

		ANIMCONTROL_BASE_COUNT

	} AnimControl_e;


	// Animation taps:
	typedef enum
	{
		ANIMTAP_STAND_LOOK,				// ARMEidle001
		ANIMTAP_STAND_SWING,			// ARZZidle002
		ANIMTAP_WALK,					// ARZZwalkF01

		ANIMTAP_CHAIN_SWING_UPPER,		// ARZZactn101
		ANIMTAP_CHAIN_ROLL_UPPER,		// ARZZactn102
		ANIMTAP_CHAIN_RETURN_UPPER,		// ARZZactn103

		ANIMTAP_ROAR_UPPER,				// ARZZroar_01
		ANIMTAP_TUG_UPPER,				// ARZZtug_001
		
		ANIMTAP_CHAIN_SWING_LOWER,		// ARZZactn101
		ANIMTAP_CHAIN_ROLL_LOWER,		// ARZZactn102
		ANIMTAP_CHAIN_RETURN_LOWER,		// ARZZactn103

		ANIMTAP_ROAR_LOWER,				// ARZZroar_01
		ANIMTAP_TUG_LOWER,				// ARZZtug_001

		ANIMTAP_GRAB_ATTACK,			// ARZZactn2dm

		ANIMTAP_FREEZE,					// ARZZfreeze1
		ANIMTAP_DEATH,					// ARZZdeath01

		ANIMTAP_BASE_COUNT
	} AnimTap_e;


	// Bones:
	typedef enum
	{
		BONE_ZOMBIEBOSS_DUMMY,
		BONE_L_FINGER_3B,
		BONE_GROIN,
		BONE_L_LEG_UPPER,
		BONE_L_LEG_MIDDLE,
		BONE_L_LEG_LOWER,
		BONE_L_FOOT,
		BONE_R_LEG_UPPER,
		BONE_R_LEG_LOWER,
		BONE_R_FOOT,
		BONE_TORSO,
		BONE_R_ARM_UPPER,
		BONE_R_ARM_LOWER,
		BONE_R_HAND,
		BONE_CHAIN_REEL,
		BONE_L_ARM_UPPER,
		BONE_L_ARM_LOWER,
		BONE_L_HAND,
		BONE_L_FINGER_2A,
		BONE_L_FINGER_2B,
		BONE_L_FINGER_3A,
		BONE_L_FINGER_1A,
		BONE_L_FINGER_1B,
		BONE_HEAD,
		BONE_JAW,
		BONE_JUNK_GROIN_B,
		BONE_HOSE_TORSO,
		BONE_PIPE_BACK_C,
		BONE_PIPE_BACK_B,
		BONE_RIBLET,
		BONE_PIPE_BACK_A,
		BONE_JUNK_TORSO_A,
		BONE_JUNK_TORSO_B,
		BONE_HOSE_GROIN,
		BONE_JUNK_GROIN_C,
		BONE_JUNK_GROIN_A,
		BONE_JUNK_GROIN_D,
		BONE_WRECKING_BALL,
		BONE_ATTACHPOINT_CHAIN,
		BONE_ATTACHPOINT_GLITCH,
		BONE_CHAIN_LINK01,
		BONE_CHAIN_LINK02,
		BONE_CHAIN_LINK03,
		BONE_CHAIN_LINK04,
		BONE_CHAIN_LINK05,
		BONE_CHAIN_LINK06,
		BONE_CHAIN_LINK07,
		BONE_CHAIN_LINK08,
		BONE_CHAIN_LINK09,
		BONE_CHAIN_LINK10,
		BONE_CHAIN_LINK11,
		BONE_CHAIN_LINK12,
		BONE_CHAIN_LINK13,
		BONE_CHAIN_LINK14,
		BONE_CHAIN_LINK15,
		BONE_CHAIN_LINK16,
		BONE_CHAIN_LINK17,
		BONE_CHAIN_LINK18,
		BONE_CHAIN_LINK19,
		BONE_CHAIN_LINK20,
		BONE_CHAIN_LINK21,
		BONE_CHAIN_LINK22,
		BONE_CHAIN_LINK23,
		BONE_CHAIN_LINK24,
		BONE_CHAIN_LINK25,
		BONE_CHAIN_LINK26,
		BONE_CHAIN_LINK27,
		BONE_CHAIN_LINK28,
		BONE_CHAIN_LINK29,
		BONE_CHAIN_LINK30,
		BONE_PTACK_HEAD,
		BONE_PTACK_SMOKE01,
		BONE_PTACK_SMOKE02,
		BONE_PTACK_SMOKE03,
		BONE_COUNT
	} Bone_e;


//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static u32  m_uSystemFlags;										// See System flags: for info
	static u32	m_nBotClassClientCount;								// Number of bots of this class using the class-wide resources
	static const CBotDef m_BotDef;									// Bot definition data
	static CFSphere m_GrabSphere;

	// Bot parts:
	static CBotPartPool *m_pPartPool;								// One part pool per bot class
	
	static const FGameData_TableEntry_t m_ZombieBossInfoVocab_Fight[];
	// BotInfo data:
	static BotInfo_Gen_t m_BotInfo_Gen;					// General bot info
	static BotInfo_MountAim_t m_BotInfo_MountAim;		// Mount aim bot info
	static BotInfo_Walk_t m_BotInfo_Walk;				// Walk bot info
	
	static ZombieBossFightLevel_t* m_paFightLevels; // Fight bot info
	static u32 m_uNumFightLevels;

	static const FGameDataMap_t m_aGameDataMap[];

	static FExplosion_GroupHandle_t m_hExplosionBallHitGroup;
	static FExplosion_GroupHandle_t m_hExplosionDie;

	static CDamageForm::Damager_t m_Damager;
	static const CDamageProfile *m_pDamageProfileDrag;	// Damage profile for dragging the ball
	static const CDamageProfile *m_pDamageProfileDeath;	// Damage profile for dying
	
	static CFDebrisGroup* m_pDebrisGroup;

	// Bot animation stack data:
	static CBotAnimStackDef m_AnimStackDef;				// Animation stack definition data

	static cchar *m_apszBoneNameTable[BONE_COUNT];										// Array of bone names
	static cchar *m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT];					// Array of base animation control names
	static cchar *m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT];							// Array of base animation tap names
	static cchar *m_apszBaseAnimNameTable[ANIM_BASE_COUNT];								// Array of base animation names
	static const u8 *m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT];
	static const u8 m_anTagPointBoneNameIndexArray[];
	static const u8 m_nApproxEyePointBoneNameIndex;

	static CFAnimCombinerConfig::ConfigStack_t m_aAnimCombinerConfigStack[];
	static CFAnimCombinerConfig::ConfigTap_t m_aAnimCombinerConfigTaps[];
	static CFAnimCombiner::AttachList_t m_aAnimAttach[];

	static const u8 m_aBoneEnableIndices_FullBody[];
	static const u8 m_aBoneEnableIndices_FullBodyNoArm[];
	static const u8 m_aBoneEnableIndices_UpperBody[];
	static const u8 m_aBoneEnableIndices_LowerBody[];

	static const u8 m_anEnableBoneNameIndexTableForSummer_Normal[];

	static s32 m_nBoneIndexHead;			// Mesh's head bone
	static s32 m_nBoneIndexTorso;			// Mesh's torso bone
	static s32 m_nBoneIndexGroin;			// Mesh's groin bone
	static s32 m_nBoneIndexAttachBall;		// Ball attach point index
	static s32 m_nBoneIndexAttachGlitch;	// Glitch Attach's his head here
	static s32 m_nBoneIndexBall;			// Ball index
	static s32 m_nBoneIndexCollidePlane;	
	static s32 m_nBoneIndexLHand;	
	static s32 m_nBoneIndexL_Foot;	
	static s32 m_nBoneIndexR_Foot;	
	static s32 m_nBoneIndexChainReel;
	
	static s32 m_nBoneIndexPTack_Head;
	static s32 m_nBoneIndexPTack_Smoke01;
	static s32 m_nBoneIndexPTack_Smoke02;
	static s32 m_nBoneIndexPTack_Smoke03;

	
	static CFMtx43A m_mtxBall;
	static CFVec3A m_vChainUnitDirection;
	static CFVec3A m_vChainLengthAndDirection;
	static CFVec3A m_vChainNextLinkPositionWS;
	static f32 m_fChainLinkLength;			// 
	static f32 m_fOOChainLinkLength;		// 
	static s32 m_nCurrentLinkIndex;			// 

	static CFSoundGroup* m_pSounds[ZBOSS_SND_COUNT];

	static CFQuatA m_TorsoQuat;

	static FParticle_DefHandle_t	m_hBreathParticleDef;
	static FParticle_DefHandle_t	m_hSmokeParticleDef;

	static CDamageProfile*		m_pFootstepDamageProfile;

	CFMtx43A	m_mChainLinks;
	
	CEParticle m_FlameEmitter1;
	CEParticle m_SmokeEmitter1;
	CEParticle m_SmokeEmitter2;
	CEParticle m_SmokeEmitter3;

	CFVec3A		m_vBallPosOneAgoWS;
	CFVec3A		m_vBallPosTwoAgoWS;
		
	CFVec3A		m_vBallTargetWS;
	
	CFVec3A		m_vGrabPtMS;
	CFVec3A		m_vTorsoPtMS;

	CFSphere	m_NormalSphereMS;
	CFSphere	m_BlastDamageReceiverSphere;

	CFHermiteInterpolator m_Interpolator;

	static CBot*	 m_pGrabbedBot;
	static s32		 m_nBoneIndexGrabbedBotsHead;

	CFWorldMesh*	m_pBallMesh;
	u32				m_uBallCollCountLastFrame;
	
	ZombieBossState_e m_eState;
	f32				m_fStateTime;
	CFAudioEmitter* m_pSoundEmitters[ZBOSS_SND_COUNT];
	f32				m_fUnitAimPitch;				// compute the unit aim pitch used within anim bone callbacks
	f32				m_fChainLength;	
	f32				m_fBallTimeInAir;
	f32				m_fOOBallTimeInAir;
	f32				m_fUnitBallFlight;
	f32				m_fTimeUntilNextOuch;
	f32				m_fTimeUntilNextGrunt;
	f32				m_fIsOver;
	f32				m_fReelAngle;
	f32				m_fMaximumChainLength;
	u32				m_uFightLevelIndex;
	BOOL8			m_bRoaring;
	BOOL8			m_bFreeBalling;
	BOOL8			m_bSmoking;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	CBotZombieBoss();
	virtual ~CBotZombieBoss();
	static BOOL IsInitialized() { return(m_uSystemFlags & SYS_INITIALIZED); }

	BOOL Create( s32 nPlayerIndex=-1, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL);
	BOOL ComputeGrabPointWS(CFVec3A& rPtWs);
	BOOL ShouldNotPursue(void); // there's a variety of reasons to disallow pursuit
	void Power_Work( void );

FINLINE	BOOL DeathIsBegun(void) { return (m_uFightLevelIndex==m_uNumFightLevels);}
FINLINE	BOOL DeathIsOver(void) { return (m_fIsOver==0.0f);}
FINLINE void SetBallTargetWS(CFVec3A& rPtWs){m_vBallTargetWS = rPtWs;}
FINLINE u8	 GetSpeedForAI(void){ return ((u8)m_paFightLevels[m_uFightLevelIndex].uMoveSpeed);}
FINLINE f32	 GetMaxChainLength(void){ return m_fMaximumChainLength;}


//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:
	virtual BOOL ClassHierarchyLoadSharedResources( void );
	virtual void ClassHierarchyUnloadSharedResources( void );

	virtual void ClassHierarchyDestroy( void );

	virtual BOOL ClassHierarchyBuild( void );				// Must be implemented by all classes in the derivation chain
	virtual BOOL ClassHierarchyBuilt( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );	// Must be implemented by all classes in the derivation chain

	virtual void ClassHierarchyWork( void );
	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );

	virtual void NotifyFootDown( BOOL bLeftFoot, BOOL bRightFoot, f32 fUnitMag );

	virtual void Power( BOOL bPowerUp, f32 fPowerOffTime, f32 fPowerOffOnSpeedMult );

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);
	virtual void InflictDamage( CDamageData *pDamageData );
	virtual void InflictDamageResult( const CDamageResult *pDamageResult );
	virtual void Die( BOOL bSpawnDeathEffects=TRUE, BOOL bSpawnGoodies=TRUE );
	virtual void DeathWork(void);
	
	virtual void ComputeWeaponMuzzlePoint_WS( CFVec3A *pMuzzlePt ){ FASSERT( IsCreated() ); *pMuzzlePt = m_MtxToWorld.m_vPos; };
	virtual void ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS );
	
	// Checkpoint:
	virtual void CheckpointSaveSelect( s32 nCheckpoint );
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );
//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL _BuildAnimStackDef( void );
	
	static BOOL _ProposedBallVictim( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	static BOOL _ZombieGrabCheck( CFWorldTracker *pTracker, FVisVolume_t *pVolume );

	void _ClearDataMembers( void );
	void _ComputeGrabPointMS(CFVec3A& rPtMS);
	void _ComputeGrabPointWS(CFVec3A& rPtMS);
	
	BOOL _BuildChainLinkPool(void);

	void _SetUpBallPath(void);
	void _FlyBallFly(CFVec3A& vBall);

	void _HandleAnimations ( void );
	
	BOOL _HandleBallCollision( FCollImpact_t& rCollImpact );
	void _BallAttackCollides(const FCollImpact_t *pCollImpact );
	void _BallReturnCollides(const FCollImpact_t *pCollImpact );

	void _ComputeBallMatrix( void );
	void _ComputeVerletPosition(void);
	void _SpawnSlashImpactEffects( const FCollImpact_t *pCollImpact );
	void _CollideWithPlane(void);
	
	FINLINE f32  _PlaneCollides(const CFVec3A& rPlaneN,const  CFVec3A& rPlanePt,const CFSphere& rSphere)
	{
		f32 fPenetrationDist = rSphere.m_fRadius - ( rPlaneN.v3.Dot(rSphere.m_Pos) - rPlaneN.Dot(rPlanePt) );
		if (fPenetrationDist > 0.0f) 
		{	// return delta as penetration distance
			return  fPenetrationDist;
		}
		return 0.0f; // return 0 for all cases where push back is irrelevant...
	}
	FINLINE BOOL _BallIsOutThere(void)
	{
		return (
			(m_eState == ZOMBIEBOSS_SWING_ROLL_OUT) ||
			(m_eState == ZOMBIEBOSS_SWING_TUG_PAUSE) ||
			(m_eState == ZOMBIEBOSS_SWING_TUG_PULL) ||
			(m_eState == ZOMBIEBOSS_SWING_TUG_RETURN) ||
			(m_eState == ZOMBIEBOSS_SWING_ROLL_IN));
	}
	void _UpdateMatrices( void );
	static void _AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );

	void _PlaySnd(ZombieBossSnd_e eSnd, SndCommand_e eSndCommand,CFVec3A* pPos_WS=NULL);
	
	void _HandleReelAudio(void);
	void _HandleBallAudio(void);
	void _HandleGruntAudio(void);
	void _Smoke(BOOL bEnable, f32 fIntensity, f32 fDuration=-1.0f);
	BOOL _ReadFightLevel(cchar* pszLevel);
	friend class CBotZombieBossBuilder;

	FCLASS_STACKMEM_ALIGN( CBotZombieBoss );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotZombieBossBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotZombieBossBuilder : public CBotBuilder {
public:

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CBotZombieBossBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CBotZombieBossBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif
