//////////////////////////////////////////////////////////////////////////////////////
// botslosh.h -
//
// Author: Michael Scholz
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
// 12/18/02 Scholz       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _BOTSLOSH_H_
#define _BOTSLOSH_H_ 1

#include "bot.h"
#include "iteminst.h"
#include "SloshTank.h"

// Forward Declarations
struct FAnim_t;
class CBotAIBrain;
class CFSoundGroup;
class CBotPartPool;

FCLASS_ALIGN_PREFIX class CBotSlosh : public CBot
{
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	
	typedef enum 
	{
		SLOSHSND_BOOST,
		SLOSHSND_COUNT,
	}BotSloshSound_e;


	typedef enum {
		LIMB_TYPE_HEAD,
		LIMB_TYPE_LEFTARM,
		LIMB_TYPE_RIGHTARM,
		LIMB_TYPE_TORSO,
		LIMB_TYPE_TANK,

		LIMB_TYPE_COUNT,

		LIMB_TYPE_FIRST_DEATH_LIMB	= LIMB_TYPE_HEAD,
		LIMB_TYPE_NUM_DEATH_LIMBS	= LIMB_TYPE_TANK - LIMB_TYPE_FIRST_DEATH_LIMB + 1
	} LimbType_e;

	typedef enum {
		PART_INSTANCE_COUNT_PER_TYPE = 1
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

	// Animation controls:
	typedef enum
	{
		ANIMCONTROL_STAND,
		ANIMCONTROL_SNEAK,
		ANIMCONTROL_WALK,
		ANIMCONTROL_RUN,

		ANIMCONTROL_JUMP_LAUNCH,
		ANIMCONTROL_JUMP_FLY,
		ANIMCONTROL_JUMP_BOOST,
		ANIMCONTROL_JUMP_LAND_UPPER,
		ANIMCONTROL_JUMP_LAND_LOWER,

		ANIMCONTROL_FIRE_2,

		ANIMCONTROL_AIM_SUMMER,
		
		ANIMCONTROL_RELOAD_CLIP_EJECT_OLD,
		ANIMCONTROL_RELOAD_CLIP_GRAB_NEW,
		ANIMCONTROL_RELOAD_CLIP_INSERT_NEW,
		ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW,

		ANIMCONTROL_BASE_COUNT
	} AnimControl_e;


	// Animation taps:
	typedef enum
	{
		ANIMTAP_STAND,
		ANIMTAP_SNEAK,
		ANIMTAP_WALK,
		ANIMTAP_RUN,

		ANIMTAP_JUMP_LAUNCH,
		ANIMTAP_JUMP_FLY,
		ANIMTAP_JUMP_BOOST,
		ANIMTAP_JUMP_LAND_UPPER,
		ANIMTAP_JUMP_LAND_LOWER,
		
		ANIMTAP_FIRE_2,

		ANIMTAP_AIM_SUMMER,

		ANIMTAP_RELOAD_CLIP_EJECT_OLD,
		ANIMTAP_RELOAD_CLIP_GRAB_NEW,
		ANIMTAP_RELOAD_CLIP_INSERT_NEW,
		ANIMTAP_RELOAD_CLIP_SLAPIN_NEW,

		ANIMTAP_BASE_COUNT
	} AnimTap_e;


	// Base animations:
	typedef enum
	{
		ANIM_STAND,
		ANIM_SNEAK,
		ANIM_WALK,
		ANIM_RUN,

		ANIM_JUMP_LAUNCH,
		ANIM_JUMP_FLY,
		ANIM_JUMP_BOOST,
		ANIM_JUMP_LAND,
		ANIM_FIRE_2,

		ANIM_RELOAD_CLIP_EJECT_OLD,
		ANIM_RELOAD_CLIP_GRAB_NEW,
		ANIM_RELOAD_CLIP_INSERT_NEW,
		ANIM_RELOAD_CLIP_SLAPIN_NEW,

		ANIM_BASE_COUNT
	} Anim_e;


	// Bones:
	typedef enum
	{
		BONE_R_LEG_UPPER,	
		BONE_R_LEG_LOWER,	
		BONE_R_FOOT_BACK,	
		BONE_R_HEEL		,
		BONE_R_FOOT_FRONT,	
		BONE_R_TOE		,
		BONE_L_LEG_UPPER,	
		BONE_L_LEG_LOWER,	
		BONE_L_FOOT_BACK,	
		BONE_L_HEEL		,
		BONE_L_FOOT_FRONT,	
		BONE_L_TOE		,
		BONE_R_ARM_UPPER,	
		BONE_R_ELBOW	,	
		BONE_R_ARM_LOWER,	
		BONE_L_ARM_UPPER,	
		BONE_L_ELBOW	,	
		BONE_L_ARM_LOWER,	
		BONE_L_HAND		,
		BONE_L_MIDDLEFINGERA,
		BONE_L_MIDDLEFINGERB,
		BONE_L_PINKYFINGERA,	
		BONE_L_PINKYFINGERB,	
		BONE_L_INDEXFINGERA,	
		BONE_L_INDEXFINGERB,	
		BONE_L_THUMBA,	
		BONE_L_THUMBB,
		BONE_SECONDARY_FIRE,
		BONE_GROIN,
		BONE_TORSO,
		BONE_CARAPACE_REAR,
		BONE_TANK,
		BONE_CARAPACE_MIDDLE,
		BONE_CARAPACE_FRONT,
		BONE_NECK,
		BONE_HEAD,
		BONE_MASK,
		BONE_ATTACHPOINT_JET_R,
		BONE_ATTACHPOINT_JET_L,
		BONE_ATTACHPOINT_FLUID,
		BONE_COUNT
	} Bone_e;
	
//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;					// TRUE: InitSystem() has been called
	static u32	m_nBotClassClientCount;					// Number of bots of this class using the class-wide resources
	static const CBotDef m_BotDef;						// Bot definition data

	// BotInfo data:
	static BotInfo_Gen_t m_BotInfo_Gen;					// General bot info
	static BotInfo_MountAim_t m_BotInfo_MountAim;		// Mount aim bot info
	static BotInfo_Walk_t m_BotInfo_Walk;				// Walk bot info
	static BotInfo_Jump_t m_BotInfo_Jump;				// Jump bot info
	static BotInfo_Weapon_t m_BotInfo_Weapon;			// Weapon bot info
	static BotInfo_Idle_t*	m_BotInfo_Idle;				// Idle bot info
	static u32				m_nBotInfo_Idles;			// Idle bot info count

	static const FGameDataMap_t m_aGameDataMap[];
	// particle definition
	static FParticle_DefHandle_t m_hBoostParticleDef;

	// Bot animation stack data:
	static CBotAnimStackDef m_AnimStackDef;				// Animation stack definition data

	static cchar *m_apszBoneNameTable[BONE_COUNT];										// Array of bone names
	static cchar *m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT];					// Array of base animation control names
	static cchar *m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT];							// Array of base animation tap names
	static cchar *m_apszBaseAnimNameTable[ANIM_BASE_COUNT];								// Array of base animation names
	static cchar **m_apszIdleAnimNameTable;												// Array of idle animation names
	static const u8 *m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT];
	static const u8 **m_apnEnableBoneNameIndexTableForEachIdleTap;
	static const u8 m_anEnableBoneNameIndexTableForSummer_Normal[];
	//static const u8 m_anEnableBoneNameIndexTableForSummer_TetherShock[];
	static const u8 m_anTagPointBoneNameIndexArray[];
	static const u8 m_nApproxEyePointBoneNameIndex;


	static CFAnimCombinerConfig::ConfigStack_t m_aAnimCombinerConfigStack[];
	static CFAnimCombinerConfig::ConfigTap_t m_aAnimCombinerConfigTaps[];
	static CFAnimCombiner::AttachList_t m_aAnimAttach[];
/*
	static const u8 m_aBoneEnableIndices_Cable[];
	static const u8 m_aBoneEnableIndices_AimRocket1[];
	static const u8 m_aBoneEnableIndices_AimRocket23[];
	static const u8 m_aBoneEnableIndices_TapFire1[];
	static const u8 m_aBoneEnableIndices_TapFire1_Alert[];
	static const u8 m_aBoneEnableIndices_TapFire2_Lower[];
	static const u8 m_aBoneEnableIndices_ReloadSingle_Arm[];
	static const u8 m_aBoneEnableIndices_ReloadSingle_Body[];
	static const u8 m_aBoneEnableIndices_TapFire2_Upper[];
	static const u8 m_aBoneEnableIndices_JumpLand_Lower[];
	static const u8 m_aBoneEnableIndices_JumpLand_Upper[];
	static const u8 m_aBoneEnableIndices_FireRocket1[];
	static const u8 m_aBoneEnableIndices_FireRocket23[];
	static const u8 m_aBoneEnableIndices_WeaponSwitch[];
	static const u8 m_aBoneEnableIndices_ReloadRocket1[];
*/
	static const u8 m_aBoneEnableIndices_FullBody[];
	static const u8 m_aBoneEnableIndices_AimSummer[];
	static const u8 m_aBoneEnableIndices_UpperBody[];
	static const u8 m_aBoneEnableIndices_LowerBody[];
/*
	static const u8 m_aBoneEnableIndices_UserAnim_UpperBody[];
	static const u8 m_aBoneEnableIndices_UserAnim_LowerBody[];
	static const u8 m_aBoneEnableIndices_UserAnim_UpperTorso[];
	static const u8 m_aBoneEnableIndices_UserAnim_LowerTorso[];
	static const u8 m_aBoneEnableIndices_UserAnim_LeftArm[];
	static const u8 m_aBoneEnableIndices_UserAnim_RightArm[];
	static const u8 m_aBoneEnableIndices_UserAnim_Head[];
*/
	static CBotPartPool *m_pPartPool;

	const CFVec3A *m_pGazeDir_WS;						// Points to the gaze direction vector (might not be unit in length)
	const CFMtx43A *m_pmtxLeftBooster_WS;					// Points to the left booster
	const CFMtx43A *m_pmtxRightBooster_WS;					// Points to the right booster
	
	// Animation:
	s32 m_nBoneIndexGroin;								// Mesh's groin bone index
	static CFVec3A m_GroinVecY_WS;						// Recorded groin matrice's Y vector (used in the animation callback)
	s32 m_nBoneIndexTorso;								// Mesh's torso bone index
	
	CSloshTank* m_pTank;								// Our tank on the back
	CFWorldAttachedLight* m_pTankLight;
	const f32* m_pfTankFillPercent;
	
	static CFSoundGroup*	m_pSounds[SLOSHSND_COUNT];

	PlayerDeathData_t m_PlayerDamageData;
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	CBotSlosh();
	virtual ~CBotSlosh();

	static BOOL InitSystem( void );
	static void UninitSystem( void );

	BOOL Create( s32 nPlayerIndex=-1, BOOL bInstallDataPort=FALSE, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);
	virtual const CFVec3A *GetApproxEyePoint( void ) const;

//	virtual void UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup );
	
	virtual void SpawnDeathEffects(void);  //this gets called just before bot removed from the world

	virtual void CheckpointSaveSelect( s32 nCheckpoint );

	virtual void Die( BOOL bSpawnDeathEffects=TRUE, BOOL bSpawnGoodies=TRUE );
	virtual void DeathWork( void );
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

	virtual void InflictDamage( CDamageData *pDamageData);
//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL _BuildAnimStackDef( void );

	void _ClearDataMembers( void );

	void _HandleDoubleJump( void );

	void _HandleJumping( void );
	void _HandleJumpAnimations( void );
	void _EnterFlyMode( void );
	void _StartVelocityJump( const CFVec3A *pJumpVelocity_WS );
	void _StartSingleJump( const CFVec3A *pJumpVelocity_WS=NULL );
	void _StartDoubleJump( BOOL bAddToVelocityY=TRUE );
	void _JumpLanded( void );
	void _HandleTerminalState(void);
	void _HandleWeaponWork( void );
	void _SwitchPrimaryWeaponsInBackpack( void );
	void _HandleFiringAnimations( void );
	void _HandleAimAnimations( void );
	void _HandleWeaponAnimations( void );
	void _UpdateMatrices( void );
	BOOL _IsFingerOnTrigger1( void ) const;
	BOOL _IsFingerOnTrigger2( void ) const;
	void _HandleWeaponFiring( void );
	static BOOL _InventoryCallback( CInventoryCallbackReason_e eReason, CInventory *pInventory, u32 nHandIndex, u32 uWpnIdx );
	static void _AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );

	friend class CBotSloshBuilder;

	FCLASS_STACKMEM_ALIGN( CBotSlosh );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotSloshBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotSloshBuilder : public CBotBuilder
{
public:

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CBotSloshBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CBotSloshBuilder );
} FCLASS_ALIGN_SUFFIX;





#endif


