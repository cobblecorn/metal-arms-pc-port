//////////////////////////////////////////////////////////////////////////////////////
// botmortar.h -
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

#ifndef _BOTMORTAR_H_
#define _BOTMORTAR_H_ 1

#include "bot.h"
#include "eproj.h"
#include "explosion.h"

// Forward Declarations

class CEBox;

FCLASS_ALIGN_PREFIX class CBotMortar : public CBot
{
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum
	{
		ANIM_IDLE_COUNT = 0,
	};
	
	typedef enum 
	{
		MORTARSND_FIRE,
		MORTARSND_IDLE,
		MORTARSND_PITCH,
		MORTARSND_COUNT,
	}MortarSound_e;

	typedef struct
	{
		u32 uPoolCannonsBalls;
		f32 fOORoundsPerSec;
		f32 fRoundsPerSec;

		f32 fFiringTime;
		f32 fOOFiringTime;
		
		f32 fUnrecoilTime;
		f32 fOOUnrecoilTime;

		f32 fScanVisionDistance;
		f32 fAlertVisionDistance;
		
		cchar *pszLaunchExplosion;
		cchar *pszDetonateExplosion;

		f32 fMinCannonballVelocityXZ;
		f32 fMaxCannonballVelocityXZ;

		f32	fMinRangeXZ;
		f32	fMaxRangeXZ;

		f32 fRadiansMaxDeviate;

		f32 fRetargetTime;
	
		f32 fMinShockwaveImpulse;
		f32 fMaxShockwaveImpulse;

		f32 fTargetRadius;

		CFSoundGroup* pCannonBallInAir;

	}MortarInfo_t;
//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	// Base animations:
	typedef enum
	{
		ANIM_STAND,						// ARMMidle001
		ANIM_WALK,						// ARMMwalkF01
		ANIM_RECOIL,					// ARMMfire_01
		ANIM_UNRECOIL,					// ARMMfire_02
		ANIM_POWER_DOWN,				// ARMMpwrdn01
		ANIM_POWER_UP,					// ARMMpwrup01

		ANIM_BASE_COUNT
	} Anim_e;

	// Animation controls:
	typedef enum
	{
		ANIMCONTROL_STAND,				// ARMMidle001
		ANIMCONTROL_WALK,				// ARMMwalkF01
		ANIMCONTROL_RECOIL,				// ARMMfire_01
		ANIMCONTROL_UNRECOIL,			// ARMMfire_02
		ANIMCONTROL_POWER_DOWN,			// ARMMpwrdn01
		ANIMCONTROL_POWER_UP,			// ARMMpwrup01

		ANIMCONTROL_BASE_COUNT
	} AnimControl_e;


	// Animation taps:
	typedef enum
	{
		ANIMTAP_STAND,				// ARMMidle001
		ANIMTAP_WALK,				// ARMMwalkF01
		ANIMTAP_RECOIL,				// ARMMfire_01
		ANIMTAP_UNRECOIL,			// ARMMfire_02
		ANIMTAP_POWER_DOWN,			// ARMMpwrdn01
		ANIMTAP_POWER_UP,			// ARMMpwrup01

		ANIMTAP_BASE_COUNT
	} AnimTap_e;


	// Bones:
	typedef enum
	{
		BONE_MORTARDUMMY,
		BONE_BASE,
		BONE_R_FRONT_ANKLE,
		BONE_R_FRONT_FOOT,
		BONE_R_F_FOOT_DUMMY,
		BONE_R_BACK_ANKLE,
		BONE_R_BACK_FOOT,
		BONE_R_B_FOOT_DUMMY,
		BONE_L_FRONT_ANKLE,
		BONE_L_FRONT_FOOT,
		BONE_L_F_FOOT_DUMMY,
		BONE_L_BACK_ANKLE,
		BONE_L_BACK_FOOT,
		BONE_L_B_FOOT_DUMMY,
		BONE_SWIVEL,
		BONE_BARREL_BASE,
		BONE_BARREL_MID,
		BONE_BARREL_TOP,
		BONE_FIRE_PRIMARY,

		BONE_COUNT
	} Bone_e;
	
	typedef enum
	{
		MS_SCANNING,	// run through 360 line of sight test searching for targets, if not at home, go home
		MS_TARGETING,	// pitch, yaw, pause, prepare to fire, do box evasion if taking damage or if target is too close
		MS_FIRING,		// play through fire states
		MS_UNRECOILING,	// play through fire states
		MS_RETARGETING,	// target lost, do box patrol looking for target

		MS_COUNT
	} MortarState_e;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;								// TRUE: InitSystem() has been called
	static u32	m_nBotClassClientCount;								// Number of bots of this class using the class-wide resources
	static const CBotDef m_BotDef;									// Bot definition data

	// BotInfo data:
	static BotInfo_Gen_t m_BotInfo_Gen;					// General bot info
	static BotInfo_MountAim_t m_BotInfo_MountAim;		// Mount aim bot info
	static BotInfo_Walk_t m_BotInfo_Walk;				// Walk bot info
	static MortarInfo_t m_MortarInfo;					// mortar specific info

	static const FGameDataMap_t m_aGameDataMap[];

	// Bot animation stack data:
	static CBotAnimStackDef m_AnimStackDef;				// Animation stack definition data

	static cchar *m_apszBoneNameTable[BONE_COUNT];										// Array of bone names
	static cchar *m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT];					// Array of base animation control names
	static cchar *m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT];							// Array of base animation tap names
	static cchar *m_apszBaseAnimNameTable[ANIM_BASE_COUNT];								// Array of base animation names
//	static cchar *m_apszIdleAnimNameTable[ANIM_IDLE_COUNT];								// Array of idle animation names
	static const u8 *m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT];
//	static const u8 *m_apnEnableBoneNameIndexTableForEachIdleTap[ANIM_IDLE_COUNT];
	static const u8 m_anTagPointBoneNameIndexArray[];
	static const u8 m_nApproxEyePointBoneNameIndex;

	static CFAnimCombinerConfig::ConfigStack_t m_aAnimCombinerConfigStack[];
	static CFAnimCombinerConfig::ConfigTap_t m_aAnimCombinerConfigTaps[];
	static CFAnimCombiner::AttachList_t m_aAnimAttach[];

	static const u8 m_aBoneEnableIndices_FullBody[];
	static const u8 m_aBoneEnableIndices_FullBodyExceptAimSummer[];
	static const u8 m_aBoneEnableIndices_AimSummer[];

	static CFSoundGroup* m_pSounds[MORTARSND_COUNT];
	CFAudioEmitter* m_pSoundEmitters[MORTARSND_COUNT];

	static CEProjPool::PoolHandle_t	m_hMortarProjPool;
	static s32	m_nBoneIndexPrimaryFire;
	static s32	m_nBoneIndexBarrelBase;
	static s32	m_nBoneIndexSwivel;

	static FExplosion_GroupHandle_t m_hExplLaunchGroup;
	static FExplosion_GroupHandle_t m_hExplDetonateGroup;
	
	static CBotPartPool* m_pPartPool;
	// my new privates here
	CFWorldMesh		m_DeadMesh;
	CFWorldLight	m_WorldLight;

	union
	{
		cchar*  m_pszBoxRange;
		CEBox*	m_pEBoxRange;
	};

	CFVec3A m_vEnemyVelocity;
	CFVec3A m_vHomeCoordWS;
	CFVec3A m_vGotoCoordWS;
	
	
	
	MortarState_e m_eState;
	CBot*   m_pEnemyTarget;
	f32     m_fPitchAdjWS;
	f32     m_pEnemyTargetDistance2;
	f32		m_fTimeSinceEnemyLOS;
	f32		m_fRetargetTime;
	s32		m_nLastTagPointInspected;
	f32		m_fMortarSecondsCountdownTimer;
	f32		m_fRotateVolume;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	CBotMortar();
	virtual ~CBotMortar();

	static BOOL InitSystem( void );
	static void UninitSystem( void );

	BOOL Create( s32 nPlayerIndex=-1, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL);

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);
	virtual void Die( BOOL bSpawnDeathEffects=TRUE, BOOL bSpawnGoodies=TRUE );

	virtual void CheckpointSaveSelect( s32 nCheckpoint );
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );

	virtual void ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS );
	virtual void ComputeWeaponMuzzlePoint_WS( CFVec3A *pMuzzlePt ){ return ComputeApproxMuzzlePoint_WS(pMuzzlePt);}
	
	FINLINE	f32	 ComputeCannonXZVel(const CFVec3A& rTargetPos)
			{
				CFVec3A vSelf  = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire]->m_vPos;
				f32 fDistXZ = vSelf.DistXZ(rTargetPos);

				f32 fUnitMult = (fDistXZ-m_MortarInfo.fMinRangeXZ)/(m_MortarInfo.fMaxRangeXZ-m_MortarInfo.fMinRangeXZ);
				FMATH_CLAMPMIN(fUnitMult,0.0f);
				f32 fStraightLineVel =FMATH_FPOT(fUnitMult,m_MortarInfo.fMinCannonballVelocityXZ,m_MortarInfo.fMaxCannonballVelocityXZ);
				return fStraightLineVel;
			}

	FINLINE	void ComputeTargetsFutureLoc(CFVec3A& rTargetPos,const CBot& rTarget, f32 fHowLongFromPresent)
			{		
				CFVec3A vTargetVelXZ;
				vTargetVelXZ.x = rTarget.m_Velocity_WS.x;
				vTargetVelXZ.y = 0.f;
				vTargetVelXZ.z = rTarget.m_Velocity_WS.z;
				rTargetPos.Mul(vTargetVelXZ,fHowLongFromPresent).Add(rTarget.MtxToWorld()->m_vPos);
			}


//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:
	virtual BOOL ClassHierarchyLoadSharedResources( void );
	virtual void ClassHierarchyUnloadSharedResources( void );

	virtual void ClassHierarchyResolveEntityPointerFixups();

	virtual void ClassHierarchyDestroy( void );
	virtual BOOL ClassHierarchyBuild( void );				// Must be implemented by all classes in the derivation chain
	virtual BOOL ClassHierarchyBuilt( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );	// Must be implemented by all classes in the derivation chain
	virtual void ClassHierarchyWork( void );
	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );

	static void _AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL _BuildAnimStackDef( void );
	static BOOL _ProjDetonateCallback( CEProj *pProj, BOOL bMakeEffect, CEProj::Event_e nEvent, const FCollImpact_t *pImpact );

	void _ClearDataMembers( void );
	void _UpdateMatrices( void );

	void _SetMortarState(MortarState_e eNewState);
	void _HandleMortarWork(void);
	void _SearchForTarget(f32 fSearchSphereRadius);
	void _PlaySnd(MortarSound_e eSnd, BOOL bPlay);
	void _FireProjectile(void);
	BOOL _HandleTargeting(void);
	BOOL _SetGotoCoordWS(const CFVec3A& rGoto);
	BOOL _GotoCoordWS(BOOL bGoto=TRUE);
	void _PitchBaseNoise(BOOL bOn, f32 fUnitVolume=1.0f);
	friend class CBotMortarBuilder;

	FCLASS_STACKMEM_ALIGN( CBotMortar );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotMortarBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotMortarBuilder : public CBotBuilder {

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	cchar*  m_pszBoxRange;


//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CBotMortarBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );

	FCLASS_STACKMEM_ALIGN( CBotMortarBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif
