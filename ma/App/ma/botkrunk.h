//////////////////////////////////////////////////////////////////////////////////////
// botkrunk.h -
//
// Author: Michael Starich
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
// 1/27/03 Starich       Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _BOTKRUNK_H_
#define _BOTKRUNK_H_ 1

#include "bot.h"
#include "iteminst.h"


// Forward Declarations
struct FAnim_t;
class CBotAIBrain;


FCLASS_ALIGN_PREFIX class CBotKrunk : public CBot {

//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// Misc definitions:
	enum {
		PART_INSTANCE_COUNT_PER_TYPE = 1,
		ANIM_SOUNDS_PER_SURFACE = 3,	// the number of peg leg sounds per surface	
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
	typedef enum {
        ANIMCONTROL_STAND,
		ANIMCONTROL_WALK,
		ANIMCONTROL_RUN,

		ANIMCONTROL_JUMP_LAUNCH,
		ANIMCONTROL_JUMP_FLY,
		ANIMCONTROL_JUMP_LAND_LOWER,
		ANIMCONTROL_JUMP_LAND_UPPER,

		ANIMCONTROL_FIRE_1,
		ANIMCONTROL_FIRE_2,

		ANIMCONTROL_AIM_SUMMER,

		ANIMCONTROL_RELOAD_CLIP_GRAB_NEW,
		ANIMCONTROL_RELOAD_CLIP_INSERT_NEW,
		ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW,

		ANIMCONTROL_DISASSEMBLE,
		ANIMCONTROL_ASSEMBLE,
		
		ANIMCONTROL_BASE_COUNT
	} AnimControl_e;

	// Animation taps:
	typedef enum {
		ANIMTAP_STAND,
		ANIMTAP_WALK,
		ANIMTAP_RUN,

		ANIMTAP_JUMP_LAUNCH,
		ANIMTAP_JUMP_FLY,
		ANIMTAP_JUMP_LAND_LOWER,
		ANIMTAP_JUMP_LAND_UPPER,
		
		ANIMTAP_FIRE_1,
		ANIMTAP_FIRE_2,

		ANIMTAP_AIM_SUMMER,

		ANIMTAP_RELOAD_CLIP_GRAB_NEW,
		ANIMTAP_RELOAD_CLIP_INSERT_NEW,
		ANIMTAP_RELOAD_CLIP_SLAPIN_NEW,

		ANIMTAP_DISASSEMBLE,
		ANIMTAP_ASSEMBLE,

		ANIMTAP_BASE_COUNT
	} AnimTap_e;

	// Base animations:
	typedef enum {
		ANIM_STAND,
		ANIM_WALK,
		ANIM_RUN,

		ANIM_JUMP_LAUNCH,
		ANIM_JUMP_FLY,
		ANIM_JUMP_LAND,
		
		ANIM_FIRE_1,
		ANIM_FIRE_2,

		ANIM_RELOAD_CLIP_GRAB_NEW,
		ANIM_RELOAD_CLIP_INSERT_NEW,
		ANIM_RELOAD_CLIP_SLAPIN_NEW,

		ANIM_DISASSEMBLE,
		ANIM_ASSEMBLE,

		ANIM_BASE_COUNT
	} Anim_e;

	// Bones:
	typedef enum {
		BONE_GROIN = 0,
		BONE_R_LEG_UPPER,
		BONE_R_KNEE,
		BONE_R_LEG_LOWER,
		BONE_R_FOOT_DUMMY,
		BONE_L_LEG_UPPER,
		BONE_L_KNEE,
		BONE_L_LEG_LOWER,
		BONE_L_FOOT,
		BONE_BELTPACK_FRONT_A,
		BONE_BELTPACK_FRONT_B,
		BONE_BELTPACK_FRONT_C,
		BONE_BELTPACK_FRONT_D,
		BONE_BELTPACK_FRONT_E,
		BONE_BELTPACK_BACK_A,
		BONE_BELTPACK_BACK_B,
		BONE_BELTPACK_BACK_C,
		BONE_BELTPACK_BACK_D,
		BONE_TORSO,
		BONE_HEAD,
		BONE_L_EYEBROW,
		BONE_R_EYEBROW,
		BONE_JAW,
		BONE_L_HAND,
		BONE_L_HAND_DUMMY,
		BONE_L_ARMB_UPPER,
		BONE_L_ARMB_LOWER,
		BONE_L_INDEXFINGERB,
		BONE_L_PINKYFINGERB,
		BONE_L_MIDDLEFINGERB,
		BONE_L_THUMBB,
		BONE_L_ARMA_UPPER,
		BONE_L_ARMA_LOWER,
		BONE_L_INDEXFINGERA,
		BONE_L_PINKYFINGERA,
		BONE_L_MIDDLEFINGERA,
		BONE_L_THUMBA,
		BONE_TORCHDUMMY,
		BONE_R_ARMA_UPPER,
		BONE_R_ARMA_LOWER,
		BONE_ATTACHPOINT_PRIMARY,
		BONE_BARREL,
		BONE_PRIMARY_FIRE,
		BONE_HANDLE,
		BONE_R_ARMB_UPPER,
		BONE_R_ARMB_LOWER,
		BONE_R_SCREWDRIVER_FLAT,
		BONE_R_SCREWDRIVER_PHILIPS,
		BONE_SCREWDRIVER_DUMMY,
		BONE_DUMMY,

		BONE_COUNT
	} Bone_e;

	typedef enum {
		LIMB_TYPE_HEAD,
		LIMB_TYPE_LEFTARM1,
		LIMB_TYPE_LEFTARM2,
		LIMB_TYPE_RIGHTARM1,
		LIMB_TYPE_RIGHTARM2,
		LIMB_TYPE_TORSO,

		LIMB_TYPE_COUNT,

		LIMB_TYPE_FIRST_DEATH_LIMB	= LIMB_TYPE_HEAD,
		LIMB_TYPE_NUM_DEATH_LIMBS	= LIMB_TYPE_TORSO - LIMB_TYPE_FIRST_DEATH_LIMB + 1
	} LimbType_e;


//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;					// TRUE: InitSystem() has been called
	static u32 m_nBotClassClientCount;					// Number of bots of this class using the class-wide resources
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
	static const u8 m_anTagPointBoneNameIndexArray[];
	static const u8 m_nApproxEyePointBoneNameIndex;

	static CFAnimCombinerConfig::ConfigStack_t m_aAnimCombinerConfigStack[];
	static CFAnimCombinerConfig::ConfigTap_t m_aAnimCombinerConfigTaps[];
	static CFAnimCombiner::AttachList_t m_aAnimAttach[];

	// Areas of bone indices for various masks:
	static const u8 m_aBoneEnableIndices_FullBody[];
	static const u8 m_aBoneEnableIndices_AimSummer[];
	static const u8 m_aBoneEnableIndices_UpperBody[];
	static const u8 m_aBoneEnableIndices_LowerBody[];
	static const u8 m_aBoneEnableIndices_UpperRightArm[];

	static CBotPartPool *m_pPartPool;


	// Misc:
	const CFVec3A *m_pGazeDir_WS;						// Points to the gaze direction vector (might not be unit in length)
	
	// Animation:
	s32 m_nBoneIndexGroin;								// Mesh's groin bone index
	static CFVec3A m_GroinVecY_WS;						// Recorded groin matrice's Y vector (used in the animation callback)
	s32 m_nBoneIndexTorso;								// Mesh's torso bone index
	f32 m_fUnitTetherShockVibration;					// Slides from 0 to 1 when the tether is shocking another bot
		
	// Peg Leg footstep sounds:
	typedef struct {
		FSndFx_FxHandle_t ahStepSound[SURFACE_TYPE_COUNT][ANIM_SOUNDS_PER_SURFACE];
	} _KrunkFootsteps_t;
	static _KrunkFootsteps_t m_KrunkFootStepSounds;		// Used to store the Krunk's peg leg footsteps

	// Way to restore Krunk's inventory when he possesses other bots:
	CInventory m_InventoryCache;						// Used while Krunk possesses another unit

	// Flame vars:
	static FParticle_DefHandle_t m_hFlameParticleDef;
	const CFMtx43A* m_pTorchMtx;						// Where is the torch bone
	f32 m_fSecsUntilNextBurner;

	PlayerDeathData_t m_PlayerDamageData;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	CBotKrunk();
	virtual ~CBotKrunk();

	static BOOL InitSystem( void );
	static void UninitSystem( void );
	
	BOOL Create( s32 nPlayerIndex=-1, 
				BOOL bInstallDataPort=FALSE, 
				cchar *pszEntityName=NULL,
				const CFMtx43A *pMtx=NULL,
				cchar *pszAIBuilderName=NULL );

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);
	virtual const CFVec3A *GetApproxEyePoint( void ) const;

	virtual void SpawnDeathEffects(void);  //this gets called just before bot removed from the world

	virtual void CheckpointSaveSelect( s32 nCheckpoint );

	
//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	// Entity functions:
	virtual BOOL ClassHierarchyLoadSharedResources( void );
	virtual void ClassHierarchyUnloadSharedResources( void );
	virtual void ClassHierarchyDestroy( void );
	virtual BOOL ClassHierarchyBuild( void );				// Must be implemented by all classes in the derivation chain
	virtual BOOL ClassHierarchyBuilt( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );	// Must be implemented by all classes in the derivation chain
	virtual void ClassHierarchyWork( void );
	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );

	// Bot functions:
	virtual void InflictDamage( CDamageData *pDamageData);
	virtual void PlayFootSound( BOOL bLeft, BOOL bRight, SurfaceType_e nSurfaceType );
	virtual void AboutToPossessBot( CBot *pBotToBePossessed );
	virtual void HaveUnPossessedBot( CBot *pBotBeingLost );


//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	
	static BOOL _BuildAnimStackDef( void );

	void _ClearDataMembers( void );

	void _HandleDoubleJump( void );
	void _HandleJumping( void );
	void _HandleJumpAnimations( void );
	void _StartVelocityJump( const CFVec3A *pJumpVelocity_WS );
	void _StartSingleJump( const CFVec3A *pJumpVelocity_WS=NULL );
	void _StartDoubleJump( BOOL bAddToVelocityY=TRUE );
	void _JumpLanded( void );	
	void _EnterFlyMode( void );
	
	void _HandleTerminalState(void);

	void _HandleFiringAnimations( void );
	void _HandleAimAnimations( void );
	void _HandleWeaponAnimations( void );
	void _HandleWeaponWork( void );
	void _SwitchPrimaryWeaponsInBackpack( void );
	BOOL _IsFingerOnTrigger1( void ) const;
	BOOL _IsFingerOnTrigger2( void ) const;
	void _HandleWeaponFiring( void );
	
	void _UpdateMatrices( void );
	
	static BOOL _InventoryCallback( CInventoryCallbackReason_e eReason, CInventory *pInventory, u32 nHandIndex, u32 uWpnIdx );
	static void _AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );

	void _SpawnBurners( void );

	friend class CBotKrunkBuilder;

	FCLASS_STACKMEM_ALIGN( CBotKrunk );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotKrunkBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotKrunkBuilder : public CBotBuilder {

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CBotKrunkBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );

	FCLASS_STACKMEM_ALIGN( CBotKrunkBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif


