//////////////////////////////////////////////////////////////////////////////////////
// botscientist.h - Jump Trooper
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
// 04/19/03 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _BOTSCIENTIST_H_
#define _BOTSCIENTIST_H_ 1


#include "fang.h"
#include "bot.h"
#include "fparticle.h"
#include "fforce.h"
#include "FXStreamer.h"
#include "tracer.h"
#include "explosion.h"
#include "fbolt.h"

// Forward Declarations
struct FAnim_t;
class  CBotAIBrain;
class  CMeshEntity;
class  CBotPartPool;



FCLASS_ALIGN_PREFIX class CBotScientist : public CBot {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum
	{
		SCIENTISTFLAG_NONE = 0x00000000
	};



//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	enum {
		PART_INSTANCE_COUNT_PER_TYPE	= 3,
		ANIM_IDLE_COUNT					= 1,	// SER: Eventually this will be retrieved from a data file
		MAX_DAMAGE_BOTS					= 4,	// max bots we can damage w/ a single attack
	};


	typedef enum {
		LIMB_TYPE_HEAD,
		LIMB_TYPE_LEFT_ARM,
		LIMB_TYPE_RIGHT_ARM,

		LIMB_TYPE_COUNT
	} LimbType_e;


	// Animation controls:
	typedef enum {
		ANIMCONTROL_STAND,
		ANIMCONTROL_CONSOLE,
		ANIMCONTROL_ATTACK,
		ANIMCONTROL_RC_TETHERED,
		ANIMCONTROL_RC_POWER_DOWN,
		ANIMCONTROL_RC_POWER_UP,
		
		ANIMCONTROL_BASE_COUNT
	} AnimControl_e;


	// Animation taps:
	typedef enum {
		ANIMTAP_STAND,
		ANIMTAP_CONSOLE,
		ANIMTAP_ATTACK,

		ANIMTAP_RC_TETHERED,
		ANIMTAP_RC_POWER_DOWN,
		ANIMTAP_RC_POWER_UP,

		ANIMTAP_BASE_COUNT
	} AnimTap_e;


	// Base animations:
	typedef enum {
		ANIM_STAND,
		ANIM_CONSOLE,
		ANIM_ATTACK,

		ANIM_RC_TETHERED,
		ANIM_RC_POWER_DOWN,
		ANIM_RC_POWER_UP,

		ANIM_BASE_COUNT
	} Anim_e;


	// Bones:
	typedef enum {
		BONE_BASE,			// "Base",
		BONE_WAIST,			// "Waist",
		BONE_TORSO,			// "Torso",

		BONE_ARM_UPPER_L,	// "L_Arm_Upper",
		BONE_ARM_ELBOW_L,	// "L_Elbow",
		BONE_ARM_LOWER_L,	// "L_Arm_Lower",
		BONE_HAND_L,		// "L_Hand",
		BONE_FINGER_LA1,	// "L_Finger_A1",
		BONE_FINGER_LA2,	// "L_Finger_A2",
		BONE_FINGER_LA3,	// "L_Finger_A3",
		BONE_FINGER_LB1,	// "L_Finger_B1",
		BONE_FINGER_LB2,	// "L_Finger_B2",
		BONE_FINGER_LB3,	// "L_Finger_B3",

		BONE_ARM_UPPER_R,	// "R_Arm_Upper",
		BONE_ARM_ELBOW_R,	// "R_Elbow",
		BONE_ARM_LOWER_R,	// "R_Arm_Lower",
		BONE_HAND_R,		// "R_Hand",
		BONE_FINGER_RA1,	// "R_Finger_B1",
		BONE_FINGER_RA2,	// "R_Finger_B2",
		BONE_FINGER_RA3,	// "R_Finger_B3",
		BONE_FINGER_RB1,	// "R_Finger_A1",
		BONE_FINGER_RB2,	// "R_Finger_A2",
		BONE_FINGER_RB3,	// "R_Finger_A3",

		// non anim driven
		BONE_TARGET,		// "Target"

		BONE_COUNT
	} Bone_e;

	typedef struct {
		f32 					fAttackTime;
		f32 					fAttackRechargeTime;
		f32 					fAttackMaxRange;
		f32						fHealMaxRange;
		FTexDef_t*				pBoltTex;
		CDamageProfile* 		pJoltDamageProfile;
		CDamageProfile*			pHealDamageProfile;
		FParticle_DefHandle_t	hJoltParticleDef;
		FParticle_DefHandle_t	hHealParticleDef;
		CFSoundGroup*			pSoundShock;
		CFSoundGroup*			pSoundHeal;
		CFSoundGroup*			pSoundHoverLoop;

	} BotInfo_Scientist_t;

	typedef enum {
		ATTACKSTATE_NONE,
		ATTACKSTATE_STARTING,
		ATTACKSTATE_ATTACKING,
		ATTACKSTATE_STOPPING,
		ATTACKSTATE_RECHARGING

	} AttackState_e;


//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:


//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL	 m_bSystemInitialized;									// TRUE: InitSystem() has been called
	static u32 m_nBotClassClientCount;									// Number of bots of this class using the class-wide resources
	static const CBotDef m_BotDef;										// Bot definition data
	static CFVec3A m_GroinVecY_WS;

	// Bot parts:
	static CBotPartPool *m_pPartPool;									// One part pool per bot class


	// BotInfo data:
	static BotInfo_Gen_t			m_BotInfo_Gen;						// General bot info
	static BotInfo_MountAim_t		m_BotInfo_MountAim;					// Mount aim bot info
	static BotInfo_Walk_t			m_BotInfo_Walk;						// Walk bot info
	static BotInfo_Jump_t			m_BotInfo_Jump;						// Jump bot info
	static BotInfo_Weapon_t			m_BotInfo_Weapon;					// Weapon bot info
	static BotInfo_Scientist_t		m_BotInfo_Scientist;				// Scientist bot info

	static CBotScientist*			m_pCBScientist;						// ptr to this used for callbacks

	// Bot animation stack data:
	static CBotAnimStackDef m_AnimStackDef;								// Animation stack definition data

	static const FGameDataMap_t m_aGameDataMap[];
	static const FGameDataMap_t m_aSoundDataMap[];
	static const FGameData_TableEntry_t m_aBotInfoVocab_Scientist[];

	static cchar *m_apszBoneNameTable[BONE_COUNT];						// Array of bone names
	static cchar *m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT];	// Array of base animation control names
	static cchar *m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT];			// Array of base animation tap names
	static cchar *m_apszIdleAnimNameTable[ANIM_IDLE_COUNT];				// Array of idle animation names
	static cchar *m_apszBaseAnimNameTable[ANIM_BASE_COUNT];				// Array of base animation names

	static const u8 *m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT];
	static const u8 *m_apnEnableBoneNameIndexTableForEachIdleTap[ANIM_IDLE_COUNT];
	static const u8 m_anEnableBoneNameIndexTableForSummer_Normal[];
	static const u8 m_anEnableBoneNameIndexTableForSummer_TetherShock[];
	static const u8 m_aBoneEnableIndices_FullBody[];

	static const u8 m_anTagPointBoneNameIndexArray[];
	static const u8 m_nApproxEyePointBoneNameIndex;

	//user anim bone masks
	static const u8 m_aBoneEnableIndices_UserAnim_UpperBody[];
	static const u8 m_aBoneEnableIndices_UserAnim_LowerBody[];
	static const u8 m_aBoneEnableIndices_UserAnim_UpperTorso[];
	static const u8 m_aBoneEnableIndices_UserAnim_LowerTorso[];
	static const u8 m_aBoneEnableIndices_UserAnim_LeftArm[];
	static const u8 m_aBoneEnableIndices_UserAnim_RightArm[];
	static const u8 m_aBoneEnableIndices_UserAnim_Head[];

	//bone groups
	static CFAnimCombinerConfig::ConfigStack_t	m_aAnimCombinerConfigStack[];
	static CFAnimCombinerConfig::ConfigTap_t	m_aAnimCombinerConfigTaps[];
	static CFAnimCombiner::AttachList_t			m_aAnimAttach[];

	const	CFVec3A *m_pGazeDir_WS;									// Points to the gaze direction vector (might not be unit in length)
	
	BOOL m_bUsingConsole;
	s32 m_nBoneIdxBase;
	s32 m_nBoneIdxGroin;
	s32 m_nBoneIdxTorso;
	s32 m_nBoneIdxLHand;
	s32 m_nBoneIdxRHand;


	f32 m_fUnitFloatTimer;			// timer for float effect
	f32 m_fUnitFloatTimer2;

	// attack
	CBot *m_pTargetBot;
	AttackState_e m_AttackState;
	f32 m_fAttackTimer;
	BOOL m_bHealing;

	u32 m_uNumDamagedBots;
	CBot* m_apDamagedBots[MAX_DAMAGE_BOTS];

	static CFBolt::CFBoltData m_BoltData;
	static FLinkRoot_t m_ScientistList;
	FLink_t m_Link;

	CFAudioEmitter *m_pHoverAudioEmitter;
	
	

	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
public:
	CBotScientist();
	~CBotScientist();

	static BOOL InitSystem( void );
	static void UninitSystem( void );
	static void DrawAllFX( void );

	BOOL	Create( s32 nPlayerIndex=-1, BOOL bInstallDataPort=FALSE, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

	virtual const CFVec3A *GetApproxEyePoint( void ) const;

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);
	virtual void SpawnDeathEffects(void);

	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );
	virtual void CheckpointSaveSelect( s32 nCheckpoint );

	//virtual void UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup );

	virtual void ComputeMtxPalette( BOOL bApplyOffscreenOptimizations )	{ FASSERT( IsCreated() ); FASSERT( m_Anim.m_pAnimCombiner ); m_pCBScientist = this; m_Anim.m_pAnimCombiner->ComputeMtxPalette(bApplyOffscreenOptimizations); m_pCBScientist = NULL; };

	FINLINE BOOL IsUsingConsole( void ) const	{ FASSERT( IsCreated() ); return (m_bUsingConsole || (GetControlValue( ANIMCONTROL_CONSOLE ) > 0.0f )); }
	FINLINE void UseConsole( BOOL bUse=TRUE )	{ FASSERT( IsCreated() ); m_bUsingConsole = TRUE; }
	FINLINE BOOL IsAttacking(void) const { return (m_AttackState	 == ATTACKSTATE_STARTING || m_AttackState == ATTACKSTATE_ATTACKING || m_AttackState == ATTACKSTATE_STOPPING);}
	FINLINE const CBot* GetTargetBot(void) const	{ return m_pTargetBot;}

	FINLINE f32 GetMaxAttackRange( void ) const { FASSERT( IsCreated() ); return m_BotInfo_Scientist.fAttackMaxRange; }
	FINLINE f32 GetMaxHealRange( void ) const { FASSERT( IsCreated() ); return m_BotInfo_Scientist.fHealMaxRange; }
	

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL ClassHierarchyLoadSharedResources( void );
	virtual void ClassHierarchyUnloadSharedResources( void );

	virtual BOOL ClassHierarchyBuild( void );				// Must be implemented by all classes in the derivation chain
	virtual void ClassHierarchyDestroy( void );

	virtual BOOL ClassHierarchyBuilt( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );	// Must be implemented by all classes in the derivation chain

	virtual void ClassHierarchyWork( void );
	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );


//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL _BuildAnimStackDef( void );

	void _ClearDataMembers( void );
	void _UpdateMatrices( void );

	void _HandleConsoleAnimations( void );

	static void _AnimBoneCallback( u32 uBoneidx, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	static u32 _TrackerCollCB( CFWorldTracker *pTracker );
	void _CalcBasePosition( CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );

	void _AttackWork( void );
	BOOL _BeginAttack( void );
	void _EndAttack( void );
	BOOL _FindAttackTarget( void );
	void _DamageBot( CBot *pBot );


	friend class CBotScientistBuilder;

FCLASS_STACKMEM_ALIGN( CBotScientist );
} FCLASS_ALIGN_SUFFIX;


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotScientistBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotScientistBuilder : public CBotBuilder {
public:

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CBotScientistBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CBotScientistBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif

