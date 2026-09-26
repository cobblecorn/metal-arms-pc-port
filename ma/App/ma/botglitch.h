//////////////////////////////////////////////////////////////////////////////////////
// botglitch.h - 
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
// 02/26/03 Elliott     Cut & pasted from botblink.h
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _BOTGLITCH_H_
#define _BOTGLITCH_H_ 1

#include "fang.h"
#include "bot.h"
#include "camsimple.h"
#include "weapon.h"
#include "iteminst.h"
#include "fforce.h"
#include "fAntenna.h"
#include "fworld.h"
#include "cammanual.h"
#include "vehicle.h"

#define HEAD_LOOK_IDLE_WAIT_TIME	( 5.0f )
#define HEAD_LOOK_IDLE_LOOK_TIME	( 2.0f )
#define MAX_CP_PARTICLES			( 2 )

// Forward Declarations
class CBotAIBrain;
class CRoboBuddy;
class CEParticle;
class CBotProbe;
struct FAnim_t;

FCLASS_ALIGN_PREFIX class CBotGlitch : public CBot {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// Servo types:
	typedef enum {
		SERVO_TYPE_LEGS,						// Leg servo
		SERVO_TYPE_ARMS,						// Arm servo

		SERVO_TYPE_COUNT
	} ServoType_e;


	// Misc definitions:
	enum {
		SERVO_COUNT = 3,						// Number of possible leg or arm servos
		NUM_DAMAGE_PART_EFFECTS = 6,
		MAX_ACTIVE_DAMAGE_PARTICLES = 4,		// max damage particles glitchy can have going at the same time
		NUM_DAMAGEABLE_LIMBS = 8,
		_MAX_HIT_ENTITIES = 4
	};

	typedef enum {
		LIMB_TYPE_HEAD,
		LIMB_TYPE_BACKPACK,
		LIMB_TYPE_TORSO,
		LIMB_TYPE_LEFTARM,
		LIMB_TYPE_RIGHTARM,

		LIMB_TYPE_SHOULDERPAD,
		LIMB_TYPE_ARMBAR,
		LIMB_TYPE_PICKAXE,
		LIMB_TYPE_FINGER,
		LIMB_TYPE_BELTPACK1,
		LIMB_TYPE_BELTPACK4,
		LIMB_TYPE_BELTPACK5,
		LIMB_TYPE_ANTENNA,

		LIMB_TYPE_LEFTLEG,
		LIMB_TYPE_RIGHTLEG,

		LIMB_TYPE_COUNT
	} LimbType_e;


	typedef enum {
		DAMAGE_PARTICLE_SMOKE,
		DAMAGE_PARTICLE_SPARKS,
		DAMAGE_PARTICLE_OIL,

		DAMAGE_PARTICLE_COUNT,
		
		DAMAGE_PARTICLE_ON_BONE_MAX = DAMAGE_PARTICLE_SPARKS,
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
		ANIMCONTROL_SNEAK,
		ANIMCONTROL_WALK,
		ANIMCONTROL_RUN,

		ANIMCONTROL_RECOIL_SUMMER,
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

		ANIMCONTROL_FIRE_1_LOWER,
		ANIMCONTROL_FIRE_1_UPPER,
		ANIMCONTROL_FIRE_2_LOWER,
		ANIMCONTROL_FIRE_2_UPPER,

		ANIMCONTROL_AIM_DUAL_WITH_TORSO_TWISTED,
		ANIMCONTROL_AIM_SHOULDER_MOUNTED,
		ANIMCONTROL_AIM_MORTAR,

		ANIMCONTROL_RELOAD_ROCKET1_GRAB_AMMO,
		ANIMCONTROL_RELOAD_ROCKET1_DROP_IN,
		ANIMCONTROL_RELOAD_ROCKET1_FINISH,

		ANIMCONTROL_RELOAD_TETHER_PREPARE_TO_PUMP,
		ANIMCONTROL_RELOAD_TETHER_PUMP,
		ANIMCONTROL_RELOAD_TETHER_GRAB_AMMO,
		ANIMCONTROL_RELOAD_TETHER_DROP_IN,
		ANIMCONTROL_RELOAD_TETHER_FINISH,

		ANIMCONTROL_RELOAD_MORTAR_GRAB_AMMO,
		ANIMCONTROL_RELOAD_MORTAR_PLACE_IN_SLING,
		ANIMCONTROL_RELOAD_MORTAR_TAKE_UP_SLACK,

		ANIMCONTROL_RELOAD_SINGLE_ARM,
		ANIMCONTROL_RELOAD_SINGLE_BODY,

		ANIMCONTROL_RELOAD_CLIP_EJECT_OLD,
		ANIMCONTROL_RELOAD_CLIP_GRAB_NEW,
		ANIMCONTROL_RELOAD_CLIP_INSERT_NEW,
		ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW,

		ANIMCONTROL_SCOPE_ATTACHING,

		ANIMCONTROL_WEAPON_SWITCH_RELEASE_SINGLE,
		ANIMCONTROL_WEAPON_SWITCH_RELEASE_DUAL,
		ANIMCONTROL_WEAPON_SWITCH_RELEASE_SHOULDER_MOUNTED,
		ANIMCONTROL_WEAPON_SWITCH,

		ANIMCONTROL_LEECH,
		
		ANIMCONTROL_AIM_PILLBOX,

		ANIMCONTROL_DISASSEMBLE,
		ANIMCONTROL_ASSEMBLE,
		ANIMCONTROL_USE_CONSOLE,

		ANIMCONTROL_GSLAP_UPPER,
		ANIMCONTROL_GSLAP_LOWER,

		ANIMCONTROL_SHOVE_UPPER,
		ANIMCONTROL_SHOVE_LOWER,

		ANIMCONTROL_RESPAWN,

		ANIMCONTROL_GETUP,

		ANIMCONTROL_RC_TETHER,

		ANIMCONTROL_BASE_COUNT
	} AnimControl_e;


	// Animation taps:
	typedef enum {
		ANIMTAP_STAND,
		ANIMTAP_SNEAK,
		ANIMTAP_WALK,
		ANIMTAP_RUN,

		ANIMTAP_RECOIL_SUMMER,
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

		ANIMTAP_FIRE_1_LOWER,
		ANIMTAP_FIRE_1_UPPER,
		ANIMTAP_FIRE_2_LOWER,
		ANIMTAP_FIRE_2_UPPER,

		ANIMTAP_AIM_DUAL_WITH_TORSO_TWISTED,
		ANIMTAP_AIM_SHOULDER_MOUNTED,
		ANIMTAP_AIM_MORTAR,

		ANIMTAP_RELOAD_ROCKET1_GRAB_AMMO,
		ANIMTAP_RELOAD_ROCKET1_DROP_IN,
		ANIMTAP_RELOAD_ROCKET1_FINISH,

		ANIMTAP_RELOAD_TETHER_PREPARE_TO_PUMP,
		ANIMTAP_RELOAD_TETHER_PUMP,
		ANIMTAP_RELOAD_TETHER_GRAB_AMMO,
		ANIMTAP_RELOAD_TETHER_DROP_IN,
		ANIMTAP_RELOAD_TETHER_FINISH,

		ANIMTAP_RELOAD_MORTAR_GRAB_AMMO,
		ANIMTAP_RELOAD_MORTAR_PLACE_IN_SLING,
		ANIMTAP_RELOAD_MORTAR_TAKE_UP_SLACK,

		ANIMTAP_RELOAD_SINGLE_ARM,
		ANIMTAP_RELOAD_SINGLE_BODY,

		ANIMTAP_RELOAD_CLIP_EJECT_OLD,
		ANIMTAP_RELOAD_CLIP_GRAB_NEW,
		ANIMTAP_RELOAD_CLIP_INSERT_NEW,
		ANIMTAP_RELOAD_CLIP_SLAPIN_NEW,

		ANIMTAP_SCOPE_ATTACHING,

		ANIMTAP_WEAPON_SWITCH_RELEASE_SINGLE,
		ANIMTAP_WEAPON_SWITCH_RELEASE_DUAL,
		ANIMTAP_WEAPON_SWITCH_RELEASE_SHOULDER_MOUNTED,
		ANIMTAP_WEAPON_SWITCH,

		ANIMTAP_LEECH,
		ANIMTAP_AIM_PILLBOX,
		
		ANIMTAP_DISASSEMBLE,
		ANIMTAP_ASSEMBLE,
		ANIMTAP_USE_CONSOLE,

		ANIMTAP_GSLAP_UPPER,
		ANIMTAP_GSLAP_LOWER,
		ANIMTAP_SHOVE_UPPER,
		ANIMTAP_SHOVE_LOWER,

		ANIMTAP_RESPAWN,

		ANIMTAP_GETUP,

		ANIMTAP_RC_TETHER,

		ANIMTAP_BASE_COUNT
	} AnimTap_e;


	// Base animations:
	typedef enum {
		// Ground:
		ANIM_STAND,
		ANIM_SNEAK,
		ANIM_WALK_FORWARD,
		ANIM_RUN_FORWARD,

		// Air:
		ANIM_JUMP_LAUNCH,
		ANIM_JUMP_LAND,
		ANIM_JUMP_FLY,
		ANIM_JUMP_TUCK,
		ANIM_JUMP_UNTUCK,
		ANIM_CABLE_REACH,
		ANIM_CABLE_GRASP,
		ANIM_CABLE_RELEASE,

		// Weapon:
		ANIM_FIRE_PRIMARY,
		ANIM_FIRE_ROCKET1,
		ANIM_FIRE_ROCKET23,
		ANIM_FIRE_MORTAR,

		ANIM_AIM_DUAL_WITH_TORSO_TWISTED,
		ANIM_AIM_SHOULDER_MOUNTED,
		ANIM_AIM_MORTAR,

		ANIM_RELOAD_ROCKET1_GRAB_AMMO,
		ANIM_RELOAD_ROCKET1_DROP_IN,
		ANIM_RELOAD_ROCKET1_FINISH,

		ANIM_RELOAD_TETHER_PREPARE_TO_PUMP,
		ANIM_RELOAD_TETHER_PUMP,
		ANIM_RELOAD_TETHER_GRAB_AMMO,
		ANIM_RELOAD_TETHER_DROP_IN,
		ANIM_RELOAD_TETHER_FINISH,

		ANIM_RELOAD_MORTAR_GRAB_AMMO,
		ANIM_RELOAD_MORTAR_PLACE_IN_SLING,
		ANIM_RELOAD_MORTAR_TAKE_UP_SLACK,

		ANIM_FIRE_SECONDARY,
		ANIM_RELOAD_SINGLE,

		ANIM_RELOAD_CLIP_EJECT_OLD,
		ANIM_RELOAD_CLIP_GRAB_NEW,
		ANIM_RELOAD_CLIP_INSERT_NEW,
		ANIM_RELOAD_CLIP_SLAPIN_NEW,

		ANIM_SCOPE_ATTACHING,

		// Weapon switching:
		ANIM_WEAPON_SWITCH_RELEASE_SINGLE,
		ANIM_WEAPON_SWITCH_RELEASE_DUAL,
		ANIM_WEAPON_SWITCH_RELEASE_SHOULDER_MOUNTED,
		ANIM_WEAPON_SWITCH,

		ANIM_LEECH,

		ANIM_AIM_PILLBOX, 

		ANIM_DISASSEMBLE,
		ANIM_ASSEMBLE,
		ANIM_USE_CONSOLE,

		ANIM_GSLAP,
		ANIM_SHOVE,

		ANIM_RESPAWN,

		ANIM_GETUP,

		ANIM_RC_TETHER,

		ANIM_BASE_COUNT
	} Anim_e;


	// Bones:
	// Note: If you add anything to this table, you will probably need to add it to one of
	//   the following tables in botglitch_data.cpp :
	// m_aBoneEnableIndices_UpperBody[]
	//   or
	// m_aBoneEnableIndices_LowerBody[].

	typedef enum {
		BONE_LEFT_EYELID_TOP,
		BONE_LEFT_EYELID_BOTTOM,
		BONE_LEFT_EYE,

		BONE_GROIN,
		BONE_WAIST,
		BONE_TORSO,
		BONE_LEFT_ARM_UPPER,
		BONE_LEFT_ELBOW,
		BONE_LEFT_ARM_LOWER,
		BONE_LEFT_HAND,

		BONE_LEFT_THUMB_A,
		BONE_LEFT_THUMB_B,
		BONE_LEFT_INDEX_FINGER_A,
		BONE_LEFT_INDEX_FINGER_B,
		BONE_LEFT_MIDDLE_FINGER_A,
		BONE_LEFT_MIDDLE_FINGER_B,

		BONE_SECONDARY_FIRE,

		BONE_ATTACHPOINT_PRIMARY,
		BONE_RIGHT_SHOULDER,
		BONE_RIGHT_ARM_LOWER,
		BONE_RIGHT_ARM_ELBOW,
		BONE_RIGHT_SHOULDERPAD,

		BONE_BACKPACK,
		BONE_ANTENNA1,
		BONE_ANTENNA2,
		BONE_ANTENNA3,
		BONE_ANTENNA4,
		BONE_ANTENNA5,

		BONE_BACKPACK_DOOR,
		BONE_NECK,
		BONE_HEAD,
		BONE_JAW,
		BONE_MINER_DUMMY,

		BONE_RIGHT_EYELID_TOP,
		BONE_RIGHT_EYELID_BOTTOM,
		BONE_RIGHT_EYE,

		BONE_BELTPACK_1,
		BONE_BELTPACK_2,
		BONE_BELTPACK_3,
		BONE_BELTPACK_4,
		BONE_BELTPACK_5,

		BONE_RIGHT_LEG_UPPER,
		BONE_RIGHT_LEG_JOINT_DUMMY,
		BONE_RIGHT_KNEE,
		BONE_RIGHT_LEG_LOWER,
		BONE_RIGHT_FOOT_FRONT,
		BONE_RIGHT_TOE,
		BONE_RIGHT_FOOT_BACK,
		BONE_RIGHT_HEEL,

		BONE_LEFT_LEG_UPPER,
		BONE_LEFT_LEG_JOINT_DUMMY,
		BONE_LEFT_KNEE,
		BONE_LEFT_LEG_LOWER,
		BONE_LEFT_FOOT_FRONT,
		BONE_LEFT_TOE,
		BONE_LEFT_FOOT_BACK,
		BONE_LEFT_HEEL,

		BONE_COUNT
	} Bone_e;

	// bones used by recoil animation
	typedef enum {
		ANIMBONE_RECOIL_RIGHT_SHOULDER,
		ANIMBONE_RECOIL_TORSO,
		ANIMBONE_RECOIL_LEFT_SHOULDER,
		ANIMBONE_RECOIL_LEFT_LOWER_ARM,
		ANIMBONE_RECOIL_DUMMY,

		ANIMBONE_RECOIL_COUNT
	};

	// idle head movement states
	typedef enum {
		HEAD_IDLE_STATE_SET_IDLE,
		HEAD_IDLE_STATE_IDLE,
		HEAD_IDLE_STATE_SET_LOOK,
		HEAD_IDLE_STATE_LOOK,
	} HeadIdleStates_e;

	typedef enum {
		MORTAR_STATE_IDLE,										// Mortar idle state
		MORTAR_STATE_GRABBING_ROUND,							// We're reaching for a new round
		MORTAR_STATE_PLACING_ROUND,								// We're in route to place new round on the mortar
		MORTAR_STATE_TAKING_UP_SLACK,							// We're taking up the slack
		MORTAR_STATE_PLACE_TO_AIM_TRANSITION,					// We're transitioning from place to aim
		MORTAR_STATE_AIMING,									// We're aiming (loose <--> tight)
		MORTAR_STATE_FIRING,									// We're firing the mortar
		MORTAR_STATE_ABORTING,									// We're aborting the mortar usage
		MORTAR_STATE_JUMPING,									// Temporarily aborting due to bot jump
		MORTAR_STATE_REPLACE_ROUND_AFTER_JUMP,					// Replacing the round after the jump

		MORTAR_STATE_COUNT
	} MortarState_e;

	typedef enum {
		MELEESTATE_NONE,
		MELEESTATE_STARTING,
		MELEESTATE_ATTACKING,
		MELEESTATE_STOPPING,

		MELEESTATE_COUNT
	} MeleeState_e;


	// Weapon inventory:
	typedef struct {
		CWeapon *m_apWeapon[ItemInst_uMaxInventoryWeapons];		// Array of all weapons Glitch has in his inventory (slot 0 is empty hand)
		u8 m_nWeaponInvIndex;									// Index into m_apWeapon[] for currently-selected weapon (0=empty hand)
		u8 m_nWeaponInvCount;									// Number of weapons currently in m_apWeapon
		u8 m_nWeaponSwitchToIndex;								// The index of the weapon we're switching to (255 = none)
	} WeaponInv_t;


	typedef struct {
		// Parameters effected by Glitch's leg servos:
		f32 afMaxXlatSpeed[3];
		f32 afNormVelocityStepSize[3];
		f32 afVertVelocityJump1[3];
		f32 afVertVelocityJump2[3];

		// Parameters effected by Glitch's arm servos:
		f32 afWeaponSwitchTimeScale[3];
		f32 afTetherReloadAnimSpeedMult[3];
		f32 afTetherPumpAnimSpeedMult[3];
		f32 afClipReloadAnimSpeedMult[3];

		CDamageProfile *pDamageProfileGSlap;
		CFSoundGroup *pSoundGroupGSlap;
		CFSoundGroup *pSoundGroupRespawn;
	} BotInfo_Glitch_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;					// TRUE: InitSystem() has been called
	static const CBotDef m_BotDef;						// Bot definition data

	// BotInfo data:
	static BotInfo_Gen_t m_BotInfo_Gen;					// General bot info
	static BotInfo_MountAim_t m_BotInfo_MountAim;		// Mount aim bot info
	static BotInfo_Walk_t m_BotInfo_Walk;				// Walk bot info
	static BotInfo_Jump_t m_BotInfo_Jump;				// Jump bot info
	static BotInfo_Weapon_t m_BotInfo_Weapon;			// Weapon bot info
	static BotInfo_Glitch_t m_BotInfo_Glitch;			// Glitch bot info
	static BotInfo_Idle_t*	m_BotInfo_Idle;				// Idle bot info
	static u32				m_nBotInfo_Idles;			// Idle bot info count

	// Bot parts:
	static CBotPartPool *m_pPartPool;					// One part pool per bot class


	static const FGameData_TableEntry_t m_aBotInfoVocab_Glitch[];
	static const FGameDataMap_t m_aGameDataMap[];

	// Bot animation stack data:
	static CBotAnimStackDef m_AnimStackDef;				// Animation stack definition data

	static cchar *m_apszBoneNameTable[BONE_COUNT];										// Array of bone names
	static cchar *m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT];					// Array of base animation control names
	static cchar *m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT];							// Array of base animation tap names
	static cchar *m_apszBaseAnimNameTable[ANIM_BASE_COUNT];								// Array of base animation names
	static cchar **m_apszIdleAnimNameTable;												// Array of idle animation names
	static cchar *m_apszRecoilBoneNameTable[ANIMBONE_RECOIL_COUNT];						// Array of bone names in recoil animation
	static const u8 *m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT];
	static const u8 **m_apnEnableBoneNameIndexTableForEachIdleTap;
	static const u8 m_anEnableBoneNameIndexTableForSummer[];
	static const u8 m_anTagPointBoneNameIndexArray[];
	static const u8 m_nApproxEyePointBoneNameIndex;

	static CFAnimCombinerConfig::ConfigStack_t m_aAnimCombinerConfigStack[];
	static CFAnimCombinerConfig::ConfigTap_t m_aAnimCombinerConfigTaps[];
	static CFAnimCombiner::AttachList_t m_aAnimAttach[];

	static const u8 m_aBoneEnableIndices_FullBody[];
	static const u8 m_aBoneEnableIndices_UpperBody[];
	static const u8 m_aBoneEnableIndices_Cable[];
	static const u8 m_aBoneEnableIndices_AimMortar[];
	static const u8 m_aBoneEnableIndices_AimRocket1[];
	static const u8 m_aBoneEnableIndices_AimRocket23[];
	static const u8 m_aBoneEnableIndices_TapFire1[];
	static const u8 m_aBoneEnableIndices_TapFire1_Lower[];
	static const u8 m_aBoneEnableIndices_TapFire2_Lower[];
	static const u8 m_aBoneEnableIndices_ReloadSingle_Arm[];
	static const u8 m_aBoneEnableIndices_ReloadSingle_Body[];
	static const u8 m_aBoneEnableIndices_TapFire2_Upper[];
	static const u8 m_aBoneEnableIndices_JumpLand_Lower[];
	static const u8 m_aBoneEnableIndices_JumpLand_Upper[];
	static const u8 m_aBoneEnableIndices_FireMortar[];
	static const u8 m_aBoneEnableIndices_FireRocket1[];
	static const u8 m_aBoneEnableIndices_FireRocket23[];
	static const u8 m_aBoneEnableIndices_WeaponSwitch[];
	static const u8 m_aBoneEnableIndices_ReloadRocket1[];
	static const u8 m_aBoneEnableIndices_ReloadTether[];
	static const u8 m_aBoneEnableIndices_ReloadMortar[];
	static const u8 m_aBoneEnableIndices_ReloadClip[];
	static const u8 m_aBoneEnableIndices_Scope[];
	static const u8 m_aBoneEnableIndices_AimSummer[];
	static const u8 m_aBoneEnableIndices_RecoilSummer[];

	static const u8 m_aBoneEnableIndices_UserAnim_UpperBody[];
	static const u8 m_aBoneEnableIndices_UserAnim_LowerBody[];
	static const u8 m_aBoneEnableIndices_UserAnim_UpperTorso[];
	static const u8 m_aBoneEnableIndices_UserAnim_LowerTorso[];
	static const u8 m_aBoneEnableIndices_UserAnim_LeftArm[];
	static const u8 m_aBoneEnableIndices_UserAnim_RightArm[];
	static const u8 m_aBoneEnableIndices_UserAnim_Head[];

	// Misc:
	static CFAnimFrame m_Rocket1TorsoTwistQuat;			// Quaternion used to twist Glitch's torso when he's holding the L1 rocket launcher
	static CFAnimFrame m_Rocket1TorsoUntwistQuat;		// Quaternion used to untwist Glitch's torso when he's holding the L1 rocket launcher
	static CFAnimFrame m_MortarTorsoTwistQuat;			// Quaternion used to twist Glitch's torso when he's holding the Mortar
	static CFAnimFrame m_MortarTorsoUntwistQuat;		// Quaternion used to untwist Glitch's torso when he's holding the Mortar

	// particles that are spawned when glitch takes damage
	static cchar*	 m_apszDamageParticles[DAMAGE_PARTICLE_COUNT];
	static const u32 m_auDamageParticleTable[NUM_DAMAGE_PART_EFFECTS*2];
	static const LimbType_e m_aDamageLimbs[NUM_DAMAGEABLE_LIMBS];

	static FParticle_DefHandle_t	m_ahParticleDamage[DAMAGE_PARTICLE_COUNT];

	CFAnimManFrame m_AnimManFrameRecoil;				// Anim frame used for recoil animation

	u32 m_anServoLevel[SERVO_TYPE_COUNT];				// Servo upgrade levels
	const CFVec3A *m_pGazeDir_WS;						// Points to the gaze direction vector (might not be unit in length)

	// Powerups:
	CRoboBuddy *m_pRoboBuddy;							// Utilized when Glitch has picked up the weapon powerup and has his fire buddy
														//   hovering around.  This is only instantiated for the bot if the bot is a player
														//   bot.

	f32 m_fHeadIdleTime;								// timer for idle head movement
	f32 m_fHeadIdleLookChance;							// current chance of idle head turning
	HeadIdleStates_e m_nHeadIdleState;					// states for idle head movement state machine

	// Weapon:
public:
	// JUSTIN: I made this public because, when the player picks up ammo, I need to be able
	//   to find a non-selected weapon and tell it that its ammo has increased.
	WeaponInv_t m_WeaponInv[2];							// Primary [0] and secondary [1] weapon info

private:
	BOOL8 m_bFingerOnTrigger1;							// TRUE when all the conditions are GO for firing of the primary weapon
	BOOL8 m_bFingerOnTrigger2;							// TRUE when all the conditions are GO for firing of the secondary weapon

	f32 m_fFireAnimSpeedMult;							// Fire animation speed multiplier
	BOOL m_bPlayLowerFireAnim;							// Should the lower body animation be played at the same time as the upper body anim
	f32 m_fUnitTetherShockVibration;					// Slides from 0 to 1 when the tether is shocking another bot
	MortarState_e m_nMortarState;						// State of the mortar's aiming & firing
	f32 m_fMortarUnitBlend;								// Used for animation blending of the mortar
	f32 m_fMortarAbortUnitControl1;
	f32 m_fMortarAbortUnitControl2;
	AnimControl_e m_nMortarAbortControlIndex1;
	AnimControl_e m_nMortarAbortControlIndex2;

	// Animation:
	s32 m_nBoneIndexGroin;								// Mesh's groin bone index
	static CFVec3A m_GroinVecY_WS;						// Recorded groin matrice's Y vector (used in the animation callback)

	s32 m_nBoneIndexTorso;								// Mesh's torso bone index
	static CFVec3A m_TorsoVecY_WS;						// Recorded torso matrice's Y vector (used in the animation callback)

	s32 m_nBoneIndexArmLower;							// used to twist glitch's right arm
	s32 m_nBoneIndexArmElbow;							// used to bend glitch's right arm
	s32 m_nBoneIndexPriFire;							// primary fire bone

//	CEParticle *pECheckpointParticle[MAX_CP_PARTICLES];	// pointer to particle entity for checkpoint respawn effect
//	static FParticle_DefHandle_t m_hCheckpointParticleDef;		// handle to checkpoint particle def
//	f32 m_fCheckpointEffectTime;						// > 0.0f while checkpoint restore effect is running

	const CFVec3A *m_pLeftHandSecondaryFireBonePos;		// Used to align the bot to the attached zip line

	f32 m_fUnitRecoil;									// Amount glitch is currently recoiling
	f32 m_fUnitRecoilTarget;							// The recoil point glitch needs to get to

	// For the probe to swing Glitch around
	BOOL8 m_bLeeched;									// TRUE when we are being leeched (swung around)
	BOOL8 m_bNoAdvanceAnim;								// TRUE when we shouldn't advance the probe swing animation
	BOOL8 m_bAnimateCamera;								// TRUE when we can animate the camera
	BOOL8 m_bGrabbedInAir;								// TRUE when we were grabbed in the air
	f32 m_fLookUnit;									// Used to blend our camera views
	f32 m_fLeechUnitBlend;								// To blend in and out of probe animation
	f32 m_fProbeHitSoundTime;							// Time until we can play a Probe hit sound
	f32 m_fProbeCamStartFOV;							// Start FOV of the Probe cam
	f32 m_fProbeCamEndFOV;								// End FOV of the Probe cam
	f32 m_fProbeCamFOVTime;								// Used to blend the FOVs
	f32 m_fProbeCamFOVDir;								// 1.0-go up, -1.0f-go down
	f32 m_fWasherLoseTime;								// Time between losing a washer
	f32 m_fWasherLoseNextTime;							// Time till we lose another washer.
	CFQuatA m_LookStartQuat;							// Quat for our start look direction
	CFVec3A m_LookStartDirection;						// Where we were looking pre-leech attack
	CFVec3A m_LookEndAnimDirection;						// End look position
	const CFVec3A *m_pLookTarget;						// Target point to track while being swung
	const CFVec3A *m_pMinerDummy;						// Used to build a collision sphere for when Glitch is being swung
	CBotProbe *m_pProbe;								// The Probe that grabbed us, so we can let it inflict damage
	CCamSimpleInfo m_ProbeCamSimple;					// Probe camera

	// For the grabbed from above						
	// camera											//
	static CVehicleCamera::BotInfo_VehicleCamera_t m_BotInfo_GrabCamera;		// camera parameters
	static CVehicleCamera::BotInfo_VehicleCamera_t m_BotInfo_ThrowCamera;		// camera parameters
	CFMtx43A		m_mGrabCamera;						// matrix for grab camera
	CFMtx43A		m_mThrowCamera;						// matrix for throw camera
	CCamManualInfo	m_GrabCameraInfo;					// camera info for grab camera
	CCamManualInfo	m_ThrowCameraInfo;					// camera info for throw camera
	CCameraTrans	m_GrabCameraTrans;					// camera transition object to/from grab
	CVehicleCamera	m_GrabCamera;						// vehicle camera object for glitch
	BOOL			m_bGrabbedByHead;					// we are grabbed
	CBot*			m_pGrabberBot;						// Who grabbed me;
	s32				m_nGrabberBoneIndex;				// By which bone;
	s32				m_nBoneIndexHead;					// index of my own head
	f32				m_fThrownYawVelocity;				// The spinny yaw when thrown
	f32				m_fGrabbedHeadYaw;					// Rotate around head
	f32				m_fFallDownTime;					// thrown stun time;


	f32 m_fUnitAim;										// is glitch actually aiming his gun?  How much?
	f32 m_fUnitFireCtlUpper;							// controls our upper body fire animation for one handed weapons
	f32 m_fUnitFireCtlLower;							// controls our lower body fire animation blend for one handed weapons
	f32 m_fFireTimer;									// timer to keep track of when we last fired.
	CFQuatA m_qRightLowerArm;							// used to aim glitch's gun
	CFQuatA m_qRightElbow;								// used to aim glitch's gun

	// used for death/damage effects
	f32 m_fDeathTimer;
	f32 m_fNextDeathEvent;
	f32 m_fDamageAccumulator;							// accumulated damage that happends in a short period of time
	u64 m_uLastTimeDamaged;								// the last time glitch received damage
	FExplosion_GroupHandle_t m_hDeathExplosion;			// the explosion to be spawned as pieces go flying off a dead glitch
	u32 m_uDeathExplosionCounter;						// limit the number of explosions to be spawned while coming apart
	
	FParticle_EmitterHandle_t m_ahActiveDamageParticles[MAX_ACTIVE_DAMAGE_PARTICLES];
	f32 m_afActiveDamageParticleTimers[MAX_ACTIVE_DAMAGE_PARTICLES];
	u32 m_uNumActiveDamageParticles;
	
	u32 m_nStowedWeaponIndex[2];						// remember which Glitch weapons were deployed when entering vehicle

	//BOOL8 m_bInPieces;									// TRUE if we are in pieces or are in the process of building ourself
	//BOOL8 m_bFallDown;									// TRUE when we are falling to pieces
	//BOOL8 m_bFallAnimDone;								// TRUE when the fall animation is done.
	//BOOL8 m_bOnGround;									// TRUE when we are on the ground
	//f32 m_fTimeInPieces;								// How long we should remain in pieces for
	//f32 m_fUnitPiecesBlend;								// Blend in and out of our animation

	//melee attacks
	MeleeState_e m_eMeleeState;
	CFVec3A m_vMeleeLastPos;
	BOOL m_bMeleeHitWithThisAttack;
	u32 m_uMeleeHitEntityCount;
	CEntity* m_apMeleeHitEntityBuffer[_MAX_HIT_ENTITIES];
	FForceHandle_t m_hForce;
	CFAntenna m_Antenna;

//	static BOOL m_bColWithPieces;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	CBotGlitch();
	virtual ~CBotGlitch();

	static BOOL InitSystem( void );
	static void UninitSystem( void );

	BOOL Create( s32 nPlayerIndex=-1, BOOL bInstallDataPort=FALSE, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);
	virtual const CFVec3A *GetApproxEyePoint( void ) const;

	FINLINE u32 GetMaxServoLevel( ServoType_e nServoType ) const { FASSERT( IsCreated() ); return SERVO_COUNT; }
	FINLINE u32 GetServoLevel( ServoType_e nServoType ) const { FASSERT( IsCreated() ); FASSERT( nServoType>=0 && nServoType<SERVO_TYPE_COUNT ); return m_anServoLevel[nServoType]; }
	void SetServoLevel( ServoType_e nServoType, u32 nServoLevel, BOOL bForceUpdate=FALSE );
	BOOL UpgradeServoLevel( ServoType_e nServoType );

	CWeapon *FindWeaponByType( CWeapon::WeaponType_e nWeaponType );

	BOOL GrabCable( CEZipLine *pCable );
	void ReleaseCable( void );

	virtual BOOL SwitchingWeapons( void ) const;
	virtual void NotifyWeaponUpgraded( CWeapon *pUpgradedWeapon );

	virtual void UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup );

	virtual void SetNormHealth( f32 fNormHealth );

	// RoboBuddy (tm) functions.
	void DeployBuddy();
	void SetBuddyUnitEffectEnd( f32 fUnitEffectEnd) ;
	void StowBuddy();

	// Just like CheckpointRestore, but used when restoring only this bot
	// for multiplayer.
	void		 CheckpointRestoreThisBot( void );

	virtual void Die( BOOL bSpawnDeathEffects=TRUE, BOOL bSpawnGoodies=TRUE );

	virtual void CheckpointSaveSelect( s32 nCheckpoint );
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );
	virtual void GetScopeViewPos( CFVec3A *pScopeViewPos_WS );
	virtual void AbortScopeMode( BOOL bImmediate=FALSE );

	virtual void EnteringMechWork(CBot* pMech);
	virtual void ExitingMechWork(void);				// must be called when this bot gets leaves a vehicle or site-weapon

	FINLINE BOOL IsLeeched( void ) { return m_bLeeched; }
	FINLINE f32 GetProbeHitSoundTime( void ) { return m_fProbeHitSoundTime; }
	FINLINE void SetProbeHitSoundTime( const f32 &fTime ) { m_fProbeHitSoundTime = fTime; }
	FINLINE void SetGrabbedInAir( BOOL bGrab ) { m_bGrabbedInAir = bGrab; FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_NO_GRAVITY ); }

	void PreEnableProbeSwing( void );
	void EnableProbeSwing( CBotProbe *pProbe );

	void FallDown(f32 fHowLongSecs);
	void SetGrabbedByHead(CBot* pWhosGotMe, cchar* szByWhatBone);
	void ClearGrabbedByHead(void);
	void DisableGrabbedByHead( void );
	void ThrownAway(CFVec3A& rTossUnitDir, f32 fTossVelocity);

	void SetupWeaponDamageToArm( CWeapon *pWeap );

	//// Pass a time to keep Glitch in pieces.  -1.0f puts him into pieces until BreakIntoPieces( -1.0f ) is called again.
	//void BreakIntoPieces( const f32 &fTimeInPieces, BOOL bImmediately = FALSE );
	//FINLINE BOOL IsFallAnimationDone( void ) { return m_bFallAnimDone; }
	//FINLINE BOOL IsOnGroundInPieces( void )  { return m_bOnGround; } 
	////Is Glitch being taken apart, put together or on the ground?
	//FINLINE BOOL IsInPieces( void ) { return m_bInPieces && (m_bFallDown || m_bOnGround); }
	////For the Coliseum Minigame.
	//FINLINE void OverrideTimeInPieces(f32 fNewTime) { m_fTimeInPieces = fNewTime; }
	////Does glitch still recieve collisions while in pieces? Default = FALSE.
	//static void EnableCollisionWithPieces(BOOL bEnable=FALSE) { m_bColWithPieces = bEnable; }

	FINLINE BOOL IsAttackingWithMelee( void ) { FASSERT( IsCreated() ); return m_eMeleeState != MELEESTATE_NONE; }

	void DrawText( void );

	FINLINE void SnapAntenna( void ) { m_Antenna.SetPos(); }

	FINLINE void SetUseConsoleControl( f32 fVal ) { FASSERT( IsCreated() ); SetControlValue( ANIMCONTROL_USE_CONSOLE, fVal ); };
	FINLINE void UpdateUseConsoleTap( f32 fVal ) { FASSERT( IsCreated() ); UpdateTime( ANIMTAP_USE_CONSOLE, fVal ); };
	FINLINE void DeltaUseConsoleTap( f32 fTime = FLoop_fPreviousLoopSecs ) { FASSERT( IsCreated() ); DeltaTime( ANIMTAP_USE_CONSOLE, fTime ); };
//#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
//	static void DebugDraw( CBotGlitch *pGlitchy );
//#endif

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual void ClassHierarchyDestroy( void );

	virtual BOOL ClassHierarchyBuild( void );				// Must be implemented by all classes in the derivation chain
	virtual BOOL ClassHierarchyBuilt( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );	// Must be implemented by all classes in the derivation chain

	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );
	virtual void ClassHierarchyWork( void );

	virtual CFMtx43A *ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName=NULL );

	void AbortReloadImmediately( void );
	void HandleWeaponReloadingAnimations( void );
	virtual void StartedUsingLeftArmToReload( void );
	virtual void DeathWork( void );

	virtual void InflictDamageResult( const CDamageResult *pDamageResult );


//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL _BuildAnimStackDef( void );

	void _ClearDataMembers( void );

	void _HandleCableAndDoubleJump( void );
	void _HandleCableTranslation( void );
	void _HandleCableAnimations( void );

	void _HandleJumping( void );
	void _HandleJumpAnimations( void );
	void _EnterFlyMode( void );
	void _StartVelocityJump( const CFVec3A *pJumpVelocity_WS );
	void _StartSingleJump( const CFVec3A *pJumpVelocity_WS=NULL );
	void _StartDoubleJump( BOOL bAddToVelocityY=TRUE );
	void _JumpLanded( void );

	void _AbortMortarImmediately( BOOL bKeepRoundInHand = FALSE );
	BOOL _IsPrimaryWeaponSwitchQueued( void );
	BOOL _IsSecondaryWeaponSwitchQueued( void );
	void _StartPrimaryWeaponSwitch( void );
	void _AddJolt( f32 fUnitJolt );
	void _HandleWeaponWork( void );
	void _ChangeWeaponIndex( u32 nHandIndex, u32 nNewIndex );
	void _SwitchPrimaryWeaponsInBackpack( void );
	void _HandleWeaponSwitchingAnimations( void );
	void _HandleMortarReloadingAnimations( void );
	void _HandleFiringAnimations( void );
	void _HandleAimAnimations( void );
	void _HandleWeaponAnimations( void );
	void _WeaponOrUpgradeLevelMayHaveChanged( void );
	void _UpdateMatrices( void );
	BOOL _IsFingerOnTrigger1( void ) const;
	BOOL _IsFingerOnTrigger2( void ) const;
	void _HandleWeaponFiring( void );
	BOOL _InitWeaponInventory( CBotBuilder* pBuilder );
	static BOOL _InventoryCallback( CInventoryCallbackReason_e eReason, CInventory *pInventory, u32 nHandIndex, u32 uWpnIdx );
	static void _AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	static void _AnimBoneCallbackSpyVsSpy( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	void _HandleIdleHeadMovement( void );

	BOOL _CanAttachToZipLine( void );

	void _DisableProbeSwing( void );
	void _ProbeSwingWork( void );
	void _AnimateProbeCamera( void );
	void _TossWasher( const CFVec3A *pPos );

	void _GrabbedByHeadWork( void );
	void _HandleGrabbedPitchYaw( void );

	void _AimLowerArm( CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	void _AimElbow( CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	void _InitRecoilAnim( void );

	void _Recoil( f32 fRecoilAmount );
	void _HandleRecoil( void );
	void _ResetRecoilSummer( void );


	void _StartDamageEffect( s32 nBone );		// Bone that took damage last.  If -1, then it was either ambient or blast
//	void _StartDamageEffect( void );			// spawns a new damage effect
	void _ClearAllDamageEffects( void );		// immediately clears/resets to no damage
	void _RepairAllDamageEffects( void );		// visibly repairs damage effects
	void _DamageEffectWork( void );				// keeps track of any damage effects that are active

	//void _BotInPiecesWork( void );
	void _DebugAnimData( void );
	void _HandleMeleeAnimations( void );
	void _HandleMeleeAttack( void );
	void _StartGSlap( void );
	void _MeleeInflictDamage( CEntity *pEntity, CFWorldMesh *pWMesh, FCollImpact_t *pImpact );
	void _MeleeSpawnEffects( const FCollImpact_t *pImpact );
//	static BOOL _MeleeHitTrackerCB( CFWorldTracker *pTracker/*, FVisVolume_t *pVolume*/ );

	void _ChooseWeaponToDrop( s32* peWeaponType, s32* pnAmmoCount );
	void _PushAwayVehicles( void );
	static BOOL _PushAwayVehiclesCB( CFWorldTracker *pTracker, FVisVolume_t *pVolume );

	friend class CBotGlitchBuilder;

	FCLASS_STACKMEM_ALIGN( CBotGlitch );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotGlitchBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotGlitchBuilder : public CBotBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CBotGlitchBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CBotGlitchBuilder );
} FCLASS_ALIGN_SUFFIX;


#endif