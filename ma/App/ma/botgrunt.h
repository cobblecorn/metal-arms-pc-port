//////////////////////////////////////////////////////////////////////////////////////
// botgrunt.h - 
//
// Author: Steve Ranck     
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
// 02/18/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _BOTGRUNT_H_
#define _BOTGRUNT_H_ 1

#include "bot.h"
#include "iteminst.h"


// Forward Declarations
struct FAnim_t;
class CBotAIBrain;
class CBotPartPool;
class CFSoundGroup;



FCLASS_ALIGN_PREFIX class CBotGrunt : public CBot {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum {
		GAS_SOUND_COUNT = 3
	};


	enum {
		GRUNTFLAG_TERMINAL_STATE_ON		= 0x00000001,

		GRUNTFLAG_NONE					= 0x00000000
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
		PART_INSTANCE_COUNT_PER_TYPE = 3,

		ANIM_IDLE_COUNT = 1						// SER: Eventually this will be retrieved from a data file
	};


	typedef enum {
		LIMB_TYPE_HEAD,							// The bot's head
		LIMB_TYPE_LEFT_ARM,						// The bot's left arm
		LIMB_TYPE_RIGHT_ARM,					// The bot's right arm
		LIMB_TYPE_TORSO,						// The bot's torso

		LIMB_TYPE_COUNT
	} LimbType_e;


	// Animation controls:
	typedef enum {
		ANIMCONTROL_STAND,
		ANIMCONTROL_STAND_ALERT,
		ANIMCONTROL_TURN_IN_PLACE,
		ANIMCONTROL_WALK,
		ANIMCONTROL_WALK_ALERT,
		ANIMCONTROL_RUN,
		ANIMCONTROL_RUN_PANIC,

		ANIMCONTROL_AIM_SUMMER,

		ANIMCONTROL_JUMP_LAUNCH,
		ANIMCONTROL_JUMP_LAND_LOWER,
		ANIMCONTROL_JUMP_LAND_UPPER,
		ANIMCONTROL_JUMP_FLY,
		ANIMCONTROL_JUMP_TUCK,
		ANIMCONTROL_JUMP_UNTUCK,
		ANIMCONTROL_CABLE_REACH,
		ANIMCONTROL_CABLE_GRASP,
		ANIMCONTROL_CABLE_RELEASE,

		ANIMCONTROL_HOP_LEFT,
		ANIMCONTROL_HOP_RIGHT,
		ANIMCONTROL_STARTLE,
		ANIMCONTROL_ROLL_LEFT,
		ANIMCONTROL_ROLL_RIGHT,

		ANIMCONTROL_FIRE_1,
		ANIMCONTROL_FIRE_1_ALERT,
		ANIMCONTROL_FIRE_2_LOWER,
		ANIMCONTROL_FIRE_2_UPPER,

		ANIMCONTROL_RC_TETHERED,
		ANIMCONTROL_RC_POWER_DOWN,
		ANIMCONTROL_RC_POWER_UP,

		ANIMCONTROL_DOZE_LOOP,
		ANIMCONTROL_NAPJERK,
		ANIMCONTROL_WAKE,

		ANIMCONTROL_RELOAD_CLIP_EJECT_OLD,
		ANIMCONTROL_RELOAD_CLIP_GRAB_NEW,
		ANIMCONTROL_RELOAD_CLIP_INSERT_NEW,
		ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW,

		// rocket
		ANIMCONTROL_RELOAD_ROCKET1_GRAB_AMMO,
		ANIMCONTROL_RELOAD_ROCKET1_DROP_IN,
		ANIMCONTROL_RELOAD_ROCKET1_FINISH,
		ANIMCONTROL_FIRE_ROCKET1,
		ANIMCONTROL_AIM_ROCKET1,


		ANIMCONTROL_AIM_PILLBOX,

		ANIMCONTROL_BASE_COUNT
	} AnimControl_e;


	// Animation taps:
	typedef enum {
		ANIMTAP_STAND,
		ANIMTAP_STAND_ALERT,
		ANIMTAP_TURN_IN_PLACE,
		ANIMTAP_WALK,
		ANIMTAP_WALK_ALERT,
		ANIMTAP_RUN,
		ANIMTAP_RUN_PANIC,

		ANIMTAP_AIM_SUMMER,

		ANIMTAP_JUMP_LAUNCH,
		ANIMTAP_JUMP_LAND_LOWER,
		ANIMTAP_JUMP_LAND_UPPER,
		ANIMTAP_JUMP_FLY,
		ANIMTAP_JUMP_TUCK,
		ANIMTAP_JUMP_UNTUCK,
		ANIMTAP_CABLE_REACH,
		ANIMTAP_CABLE_GRASP,
		ANIMTAP_CABLE_RELEASE,

		ANIMTAP_HOP_LEFT,
		ANIMTAP_HOP_RIGHT,
		ANIMTAP_STARTLE,
		ANIMTAP_ROLL_LEFT,
		ANIMTAP_ROLL_RIGHT,


		ANIMTAP_FIRE_1,
		ANIMTAP_FIRE_1_ALERT,
		ANIMTAP_FIRE_2_LOWER,
		ANIMTAP_FIRE_2_UPPER,

		ANIMTAP_RC_TETHERED,
		ANIMTAP_RC_POWER_DOWN,
		ANIMTAP_RC_POWER_UP,

		ANIMTAP_DOZE_LOOP,
		ANIMTAP_NAPJERK,
		ANIMTAP_WAKE,

		ANIMTAP_RELOAD_CLIP_EJECT_OLD,
		ANIMTAP_RELOAD_CLIP_GRAB_NEW,
		ANIMTAP_RELOAD_CLIP_INSERT_NEW,
		ANIMTAP_RELOAD_CLIP_SLAPIN_NEW,

		// rocket
		ANIMTAP_RELOAD_ROCKET1_GRAB_AMMO,
		ANIMTAP_RELOAD_ROCKET1_DROP_IN,
		ANIMTAP_RELOAD_ROCKET1_FINISH,
		ANIMTAP_FIRE_ROCKET1,
		ANIMTAP_AIM_ROCKET1,



		ANIMTAP_AIM_PILLBOX,

		ANIMTAP_BASE_COUNT
	} AnimTap_e;


	// Base animations:
	typedef enum {
		// Ground:
		ANIM_STAND,
		ANIM_WALK_FORWARD,
		ANIM_RUN_FORWARD,
		ANIM_RUN_PANIC,
		ANIM_STAND_ALERT,
		ANIM_WALK_ALERT,
		ANIM_TURN_IN_PLACE,

		// Air:
		ANIM_JUMP_LAUNCH,
		ANIM_JUMP_LAND,
		ANIM_JUMP_FLY,
		ANIM_JUMP_TUCK,
		ANIM_JUMP_UNTUCK,
		ANIM_CABLE_REACH,
		ANIM_CABLE_GRASP,
		ANIM_CABLE_RELEASE,

		ANIM_HOP_LEFT,
		ANIM_HOP_RIGHT,
		ANIM_STARTLE,
		ANIM_ROLL_LEFT,
		ANIM_ROLL_RIGHT,

		// Weapon firing:
		ANIM_FIRE_PRIMARY,
		ANIM_FIRE_SECONDARY,
		ANIM_FIRE_PRIMARY_ALERT,

		// Remote control:
		ANIM_RC_TETHERED,
		ANIM_RC_POWER_DOWN,
		ANIM_RC_POWER_UP,
		
		ANIM_DOZE_LOOP,
		ANIM_NAPJERK,
		ANIM_WAKE,

		ANIM_RELOAD_CLIP_EJECT_OLD,
		ANIM_RELOAD_CLIP_GRAB_NEW,
		ANIM_RELOAD_CLIP_INSERT_NEW,
		ANIM_RELOAD_CLIP_SLAPIN_NEW,

		// rocket
		ANIM_RELOAD_ROCKET1_GRAB_AMMO,
		ANIM_RELOAD_ROCKET1_DROP_IN,
		ANIM_RELOAD_ROCKET1_FINISH,
		ANIM_FIRE_ROCKET1,
		ANIM_AIM_ROCKET1,


		ANIM_AIM_PILLBOX, 
		ANIM_BASE_COUNT

	} Anim_e;


	// Bones:
	typedef enum {

		BONE_RIGHT_FOOT,
		BONE_RIGHT_LEG_LOWER,
		BONE_RIGHT_LEG_MIDDL,
		BONE_RIGHT_LEG_PISTONA,
		BONE_RIGHT_LEG_PISTONB,
		BONE_RIGHT_LEG_UPPER,
		BONE_RIGHT_HIP,

		BONE_LEFT_FOOT,
		BONE_LEFT_LEG_LOWER ,
		BONE_LEFT_LEG_MIDDLE,
		BONE_LEFT_LEG_PISTONA,
		BONE_LEFT_LEG_PISTONB,
		BONE_LEFT_LEG_UPPER,
		BONE_LEFT_HIP,

		BONE_RIGHT_ARM_UPPER,
		BONE_RIGHT_ELBOW,
		BONE_RIGHT_ARM_LOWER,
		BONE_ATTACHPOINT_PRIMARY,

		BONE_LEFT_ARM_UPPER,
		BONE_LEFT_ELBOW,
		BONE_LEFT_ARM_LOWER,
		BONE_LEFT_HAND,
		BONE_SECONDARY_FIRE,
		BONE_LEFT_THUMB,
		BONE_LEFT_FINGER_A,
		BONE_LEFT_FINGER_B,
		BONE_LEFT_FINGER_C,

		BONE_GROIN,
		BONE_WAIST,
		BONE_TORSO,
		BONE_NECK,
		BONE_NECK_DUMMY,
		BONE_HEAD,

		BONE_COUNT
	} Bone_e;


	typedef struct {
		FParticle_DefHandle_t hGasParticle;
		CFSoundGroup *pSoundGroupGas;
		f32 fGasTimeMin;
		f32 fGasTimeMax;
		cchar *pszShieldArmorProfile;
		f32 fShieldRechargeTime;
		f32 fShieldRechargeDelay;
	} BotInfo_Grunt_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;					// TRUE: InitSystem() has been called
	static u32 m_nBotClassClientCount;					// Number of bots of this class using the class-wide resources
	static const CBotDef m_BotDef;						// Bot definition data


	// Bot parts:
	static CBotPartPool *m_pPartPool;					// One part pool per bot class

	static CBotGrunt *m_pCBGrunt;

	// BotInfo data:
	static BotInfo_Gen_t m_BotInfo_Gen;					// General bot info
	static BotInfo_MountAim_t m_BotInfo_MountAim;		// Mount aim bot info
	static BotInfo_Walk_t m_BotInfo_Walk;				// Walk bot info
	static BotInfo_Jump_t m_BotInfo_Jump;				// Jump bot info
	static BotInfo_Weapon_t m_BotInfo_Weapon;			// Weapon bot info
	static BotInfo_Grunt_t m_BotInfo_Grunt;				// Grunt specific info

	static const FGameData_TableEntry_t m_aBotInfoVocab_Grunt[];
	static const FGameDataMap_t m_aGameDataMap[];

	// Bot animation stack data:
	static CBotAnimStackDef m_AnimStackDef;				// Animation stack definition data

	static cchar *m_apszBoneNameTable[BONE_COUNT];										// Array of bone names
	static cchar *m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT];					// Array of base animation control names
	static cchar *m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT];							// Array of base animation tap names
	static cchar *m_apszBaseAnimNameTable[ANIM_BASE_COUNT];								// Array of base animation names
	static cchar *m_apszIdleAnimNameTable[ANIM_IDLE_COUNT];								// Array of idle animation names
	static const u8 *m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT];
	static const u8 *m_apnEnableBoneNameIndexTableForEachIdleTap[ANIM_IDLE_COUNT];
	static const u8 m_anEnableBoneNameIndexTableForSummer_Normal[];
	static const u8 m_anEnableBoneNameIndexTableForSummer_TetherShock[];
	static const u8 m_anTagPointBoneNameIndexArray[];
	static const u8 m_nApproxEyePointBoneNameIndex;

	static CFAnimCombinerConfig::ConfigStack_t m_aAnimCombinerConfigStack[];
	static CFAnimCombinerConfig::ConfigTap_t m_aAnimCombinerConfigTaps[];
	static CFAnimCombiner::AttachList_t m_aAnimAttach[];

	static const u8 m_aBoneEnableIndices_FullBody[];
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
	static const u8 m_aBoneEnableIndices_AimSummer[];
	static const u8 m_aBoneEnableIndices_ReloadClip[];

	static const u8 m_aBoneEnableIndices_UserAnim_UpperBody[];
	static const u8 m_aBoneEnableIndices_UserAnim_LowerBody[];
	static const u8 m_aBoneEnableIndices_UserAnim_UpperTorso[];
	static const u8 m_aBoneEnableIndices_UserAnim_LowerTorso[];
	static const u8 m_aBoneEnableIndices_UserAnim_LeftArm[];
	static const u8 m_aBoneEnableIndices_UserAnim_RightArm[];
	static const u8 m_aBoneEnableIndices_UserAnim_Head[];

	static const s32 m_anWeaponUpgradeLevel[CBotBuilder::_NPC_WEAPON_COUNT];


	// Misc:
	static CFAnimFrame m_Rocket1TorsoTwistQuat;			// Quaternion used to twist Blink's torso when he's holding the L1 rocket launcher
	static CFAnimFrame m_Rocket1TorsoUntwistQuat;		// Quaternion used to untwist Blink's torso when he's holding the L1 rocket launcher
	u32 m_nGruntFlags;									// the place where any specif grunt flags go

	const CFVec3A *m_pGazeDir_WS;						// Points to the gaze direction vector (might not be unit in length)

	// Animation:
	s32 m_nBoneIndexGroin;								// Mesh's groin bone index
	static CFVec3A m_GroinVecY_WS;						// Recorded groin matrice's Y vector (used in the animation callback)
	s32 m_nBoneIndexTorso;								// Mesh's torso bone index
	s32 m_nBoneIndexPriFire;							// Mesh's primary fire bone

	f32 m_fGasParticleTimer;							// Countdown seconds until next gas emission

#if 0
	// Death:
	static FMesh_t **m_papDeathChunkMeshs;				// A special set of meshes that are grunt chunks used for when the grunt dies
	static u32 m_uNumDeathChunkMeshs;					// How many are specified by name in the .cpp file
//	friend void _GruntDeathDebris_StateChangeCBFunc( CDebris* pDebris, u32 uStateChange );

	enum {
		NUM_PARTICLE_EMITTER_LOCKS = 5,
	};
	static void* m_ahParticleEmitter[NUM_PARTICLE_EMITTER_LOCKS];		// Shared classwide is the emitter for sparks coming from the grunt when he is in terminal panic
	static CBot* m_apParticleEmitterLock[NUM_PARTICLE_EMITTER_LOCKS];	// The grunt who has the spark emitter locked
	static void* m_ahParticleEmitterDef[NUM_PARTICLE_EMITTER_LOCKS];	// Particle def to be used for particleEmitters
	static cchar* m_apszParticleDefNames[];	 // Set of all possible particle defs, will be randomized and matched up with emitter locks
#endif

	const CFVec3A *m_pLeftHandSecondaryFireBonePos;		// Used to align the bot to the attached zip line




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	CBotGrunt();
	virtual ~CBotGrunt();

	static BOOL InitSystem( void );
	static void UninitSystem( void );

	BOOL Create( s32 nPlayerIndex=-1, BOOL bInstallDataPort=FALSE, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL, u32 nMeshVersionOverride=0, CBotBuilder::NPCWeapon_e nWeapon=CBotBuilder::_NPC_WEAPON_LASER );

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);
	virtual void InflictDamage( CDamageData *pDamageData );
	virtual const CFVec3A *GetApproxEyePoint( void ) const;

	BOOL GrabCable( CEZipLine *pCable );
	void ReleaseCable( void );

	virtual void NotifyWeaponUpgraded( CWeapon *pUpgradedWeapon );

	virtual void UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup );

//	virtual void SpawnDeathEffects(void);  // This gets called just before bot removed from the world
	
	virtual void CheckpointSaveSelect( s32 nCheckpoint );

	virtual FINLINE void ComputeMtxPalette( BOOL bApplyOffscreenOptimizations ) { FASSERT( IsCreated() ); FASSERT( m_Anim.m_pAnimCombiner != NULL ); m_pCBGrunt = this; m_Anim.m_pAnimCombiner->ComputeMtxPalette(bApplyOffscreenOptimizations); m_pCBGrunt = NULL; };

	virtual void Die( BOOL bSpawnDeathEffects=TRUE, BOOL bSpawnGoodies=TRUE );

	void DrawText( void );


	BOOL IsFingerOnTrigger2( void ) const;

	void ResetToNeutral( void );
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



//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL _BuildAnimStackDef( void );

	void _ClearDataMembers( void );

	void _HandleCableAndDoubleJump( void );
	void _HandleCableTranslation( void );
	void _HandleCableAnimations( void );

	void _HandleParticles( void );
	void _ComputeGasMatrix( CFMtx43A *pMtx );

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
	void _WeaponOrUpgradeLevelMayHaveChanged( void );
	void _UpdateMatrices( void );
	BOOL _IsFingerOnTrigger1( void ) const;
	void _HandleWeaponFiring( void );
	static BOOL _InventoryCallback( CInventoryCallbackReason_e eReason, CInventory *pInventory, u32 nHandIndex, u32 uWpnIdx );
	static void _AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );

#if 0
	// These functions govern access to a limited number of particle emitters that are shared by all grunts.
	// This is done so that there only need to be one set of particle emitter handles class wide
	// and not on a per grunt basis
	static BOOL InitParticleEmitterLock(void);
	static void UninitParticleEmitterLock(void);
	static BOOL RequestParticleEmitterLock(CBot* pWho, u8* pnLockId);	 // Did pWho get the lock?  If so, which slot?
	static void FreeParticleEmitterLock(void*, CBot* pWho);
	static void FreeAllParticleEmitterLocks(CBot* pWho);
	static BOOL HasParticleEmitterLock(CBot* pWho);
#endif


	friend class CBotGruntBuilder;

	FCLASS_STACKMEM_ALIGN( CBotGrunt );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotGruntBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotGruntBuilder : public CBotBuilder {
public:
	CFColorRGB m_MeshTintColor;
	BOOL m_bAllowWeaponDrop;
	BOOL m_bEnableShield;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CBotGruntBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	static const CFColorRGB *_GetGruntColor( NPCWeapon_e nNPCWeaponType );

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );

	cchar* m_apszAnimFileOverrides[CBotGrunt::ANIM_BASE_COUNT];	  //specifically override the anims for each tap

	friend class CBotGrunt;

	FCLASS_STACKMEM_ALIGN( CBotGruntBuilder );
} FCLASS_ALIGN_SUFFIX;





#endif


