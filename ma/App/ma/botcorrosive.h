//////////////////////////////////////////////////////////////////////////////////////
// botcorrosive.h - General Corrosive
//
// Author: Russell Foushee     
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
// 03/13/03 Foushee     Created.
//////////////////////////////////////////////////////////////////////////////////////


#ifndef _BOTCORROSIVE_H_
#define _BOTCORROSIVE_H_ 1


#include "fang.h"
#include "bot.h"
#include "fforce.h"
#include "eproj.h"

// Forward Declarations
struct FAnim_t;
class  CBotAIBrain;
class  CBotPartPool;
class  CAlarmNet;
class  CESphere;

typedef void (*DamageInflictedCallback_t)(void);

FCLASS_ALIGN_PREFIX class CBotCorrosive : public CBot {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum {
		PART_INSTANCE_COUNT_PER_TYPE	= 3,
		HIT_ENTITY_MAX_COUNT			= 5,	// Number of entities we can cause damage to during a slash
		ANIM_IDLE_COUNT					= 1,	// SER: Eventually this will be retrieved from a data file
		NUM_DEATH_RANDOM_ROCKETS		= 5,
	};

	typedef enum {
		BEHAVIOR_STATE_NONE,
		BEHAVIOR_STATE_STOMP,
		BEHAVIOR_STATE_FORWARD_STOMP,
		BEHAVIOR_STATE_FIST_SMASH,
		BEHAVIOR_STATE_CHEST_ROCKETLAUNCH, 
		BEHAVIOR_STATE_FINGER_FLICK,
		BEHAVIOR_STATE_BEAT_CHEST_AND_ROAR,
		BEHAVIOR_STATE_BEAT_CHEST_AND_LAUGH,
		BEHAVIOR_STATE_PEER_UNDER,
		BEHAVIOR_STATE_BOT_TOSS,
		BEHAVIOR_STATE_SWAT,

		BEHAVIOR_STATE_COUNT
	} BehaviorState_e;

	typedef enum {
		LIMB_TYPE_HEAD_CHUNKS,					// The bot's head chunks
		LIMB_TYPE_LEFT_ARM_UPPER,				// The bot's upper left arm
		LIMB_TYPE_LEFT_ARM_LOWER,				// The bot's lower left arm
		LIMB_TYPE_RIGHT_ARM_UPPER,				// The bot's upper right arm
		LIMB_TYPE_RIGHT_LEG_UPPER,				// The bot's upper right leg
		LIMB_TYPE_RIGHT_LEG_LOWER,				// The bot's lower right leg

		LIMB_TYPE_COUNT
	} LimbType_e;

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
		CORROSIVEFLAG_DAMAGEDVERSION			= 0x00000001, // TRUE when we will be using the DAMAGED VERSION of corrosive.
		CORROSIVEFLAG_ZEROROTATION				= 0x00000002, // TRUE when we will zero out the m_fControls_RotateCW value
		CORROSIVEFLAG_RELEASEROCKET				= 0x00000004, // TRUE when a rocket should be fired from corrosives chest.
		CORROSIVEFLAG_ROCKETSRELEASED			= 0x00000008, // TRUE when rockets have fired from his chest attack
		CORROSIVEFLAG_INFLICTHANDATTACKDAMAGE	= 0x00000010, // TRUE when damage and collision checks should be performed for corrosives hand attacks
		CORROSIVEFLAG_SMASHHITTERRAIN			= 0x00000020, // TRUE when a smash move has impacted with the world or non entity mesh
		CORROSIVEFLAG_USEFOOTSTOMPDAMAGEPROFILE	= 0x00000040, // TRUE when left foot bot collisions should use m_BotInfo_Corrosive.pDamageProfileFootStomp 
		CORROSIVEFLAG_CRASHEFFECTSSPAWNED		= 0x00000080, // TRUE when the crashing effects of the foot stomping or fist smashing has been generated...
		CORROSIVEFLAG_NOAIXLAT					= 0x00000100, // TRUE when we don't want the AI to apply any XZ xlation effects
		CORROSIVEFLAG_CANTOSSBOTS				= 0x00000200, // TRUE when an Bot Toss Alarm net and a spawn spline are located and initialized
		CORROSIVEFLAG_ALARMBOTSPAWNED			= 0x00000400, // TRUE when a Bot Toss is active, and a bot has been spawned from the alarm net
		CORROSIVEFLAG_ALARMBOTRELEASED			= 0x00000800, // TRUE when a Bot Toss is active, and the bot has been thrown from Corrosives hand
		CORROSIVEFLAG_DOBOTTOSSWORK				= 0x00001000, // TRUE when we should be calling the _BotTossWork routine to handle the particle system 
		CORROSIVEFLAG_CRITICALDAMAGEREACHED		= 0x00002000, // TRUE when Corrosives head has been exposed and only head shots will kill him off.
		CORROSIVEFLAG_LOSTLIMBTHISFRAME			= 0x00004000, // TRUE when Corrosive lost a limb this frame

		CORROSIVEFLAG_NONE						= 0x00000000
	};

	// Animation controls:
	typedef enum {
		ANIMCONTROL_STAND,
		ANIMCONTROL_STAND_LOOK,
		ANIMCONTROL_WALK,
		ANIMCONTROL_DAMAGED_WALK,

		ANIMCONTROL_JUMP_LAUNCH,
		ANIMCONTROL_JUMP_FLY,
		ANIMCONTROL_JUMP_LAND_UPPER,
		ANIMCONTROL_JUMP_LAND_LOWER,

		ANIMCONTROL_STOMP,
		ANIMCONTROL_FORWARD_STOMP,
		ANIMCONTROL_FIST_SMASH_UPPER,
		ANIMCONTROL_FIST_SMASH_LOWER,
		ANIMCONTROL_SWAT,
		ANIMCONTROL_BOT_TOSS,
		ANIMCONTROL_BEAT_CHEST,
		ANIMCONTROL_FINGER_FLICK,

		ANIMCONTROL_PEER_UNDER,
		ANIMCONTROL_OPEN_CHEST_DOORS,

		ANIMCONTROL_STAND_LIMP_LEFT,
		ANIMCONTROL_LIMP_LEFT,

		ANIMCONTROL_DEATH,
		ANIMCONTROL_RC_POWER_DOWN,
		ANIMCONTROL_RC_POWER_UP,

		ANIMCONTROL_AIM_SUMMER,

		ANIMCONTROL_BASE_COUNT
	} AnimControl_e;


	// Animation taps:
	typedef enum {
		ANIMTAP_STAND,
		ANIMTAP_STAND_LOOK,
		ANIMTAP_WALK,
		ANIMTAP_DAMAGED_WALK,

		ANIMTAP_JUMP_LAUNCH,
		ANIMTAP_JUMP_FLY,
		ANIMTAP_JUMP_LAND_UPPER,
		ANIMTAP_JUMP_LAND_LOWER,

		ANIMTAP_STOMP,
		ANIMTAP_FORWARD_STOMP,
		ANIMTAP_FIST_SMASH_UPPER,
		ANIMTAP_FIST_SMASH_LOWER,
		ANIMTAP_SWAT,
		ANIMTAP_BOT_TOSS,
		ANIMTAP_BEAT_CHEST,
		ANIMTAP_FINGER_FLICK,

		ANIMTAP_PEER_UNDER,
		ANIMTAP_OPEN_CHEST_DOORS,

		ANIMTAP_STAND_LIMP_LEFT,
		ANIMTAP_LIMP_LEFT,

		ANIMTAP_DEATH,
		ANIMTAP_RC_POWER_DOWN,
		ANIMTAP_RC_POWER_UP,

		ANIMTAP_AIM_SUMMER,

		ANIMTAP_BASE_COUNT
	} AnimTap_e;


	// Base animations:
	typedef enum {
		ANIM_STAND,
		ANIM_STAND_LOOK,
		ANIM_WALK,
		ANIM_DAMAGED_WALK,

		ANIM_JUMP_LAUNCH,
		ANIM_JUMP_FLY,
		ANIM_JUMP_LAND,

		ANIM_STOMP,
		ANIM_FORWARD_STOMP,
		ANIM_FIST_SMASH,
		ANIM_SWAT,
		ANIM_BOT_TOSS,
		ANIM_BEAT_CHEST,
		ANIM_FINGER_FLICK,

		ANIM_PEER_UNDER,
		ANIM_OPEN_CHEST_DOORS,

		ANIM_STAND_LIMP_LEFT,
		ANIM_LIMP_LEFT,

		ANIM_DEATH,
		ANIM_RC_POWER_DOWN,
		ANIM_RC_POWER_UP,

		ANIM_BASE_COUNT
	} Anim_e;


	// Bones:
	typedef enum {
		BONE_DUMMY,					// "CorrosiveDummy",
		BONE_GROIN,					// "Groin"

		BONE_R_LEG_UPPER,			// "R_Leg_Upper"
		BONE_R_LEG_MIDDLE,			// "R_Leg_Middle"
		BONE_R_LEG_LOWER,			// "R_Leg_Lower"
		BONE_R_FOOT,				// "R_Foot"
		BONE_R_FOOT_BACK,			// "R_Foot_Back"
		BONE_R_HEEL,				// "R_Heel"
		BONE_R_FOOT_FRONT,			// "R_Foot_Front"
		BONE_R_TOE,					// "R_Toe"

		BONE_L_LEG_UPPER,			// "L_Leg_Upper"
		BONE_L_LEG_MIDDLE,			// "L_Leg_Middle"
		BONE_L_LEG_LOWER,			// "L_Leg_Lower"
		BONE_L_FOOT,				// "L_Foot"
		BONE_L_FOOT_BACK,			// "L_Foot_Back"
		BONE_L_HEEL,				// "L_Heel"
		BONE_L_FOOT_FRONT,			// "L_Foot_Front"
		BONE_L_TOE,					// "L_Toe"

		BONE_WAISTA,				// "WaistA"
		BONE_WAISTB,				// "WaistB"
		BONE_WAISTC,				// "WaistC"
		BONE_TORSO,					// "Torso"

		BONE_L_ARM_UPPER,			// "L_Arm_Upper"
		BONE_L_ELBOW,				// "L_Elbow"
		BONE_L_ARM_LOWER,			// "L_Arm_Lower"
		BONE_L_HAND,				// "L_Hand"
		BONE_L_INDEXFINGERA,		// "L_IndexFingerA"
		BONE_L_INDEXFINGERB,		// "L_IndexFingerB"
		BONE_L_MIDDLEFINGERA,		// "L_MiddleFingerA"
		BONE_L_MIDDLEFINGERB,		// "L_MiddleFingerB"
		BONE_L_PINKYFINGERA,		// "L_PinkyFingerA"
		BONE_L_PINKYFINGERB,		// "L_PinkyFingerB"
		BONE_L_THUMBA,				// "L_ThumbA"
		BONE_L_THUMBB,				// "L_ThumbB"
		BONE_L_HAND_DUMMY,			// "L_Hand_Dummy"

		BONE_R_ARM_UPPER,			// "R_Arm_Upper"
		BONE_R_ELBOW,				// "R_Elbow"
		BONE_R_ARM_LOWER,			// "R_Arm_Lower"
		BONE_R_HAND,				// "R_Hand"
		BONE_R_THUMBA,				// "R_ThumbA"
		BONE_R_THUMBB,				// "R_ThumbB"
		BONE_R_MIDDLEFINGERA,		// "R_MiddleFingerA"
		BONE_R_MIDDLEFINGERB,		// "R_MiddleFingerB"
		BONE_R_PINKYFINGERA,		// "R_PinkyFingerA"
		BONE_R_PINKYFINGERB,		// "R_PinkyFingerB"
		BONE_R_INDEXFINGERA,		// "R_IndexFingerA"
		BONE_R_INDEXFINGERB,		// "R_IndexFingerB"
		BONE_R_HAND_DUMMY,			// "R_Hand_Dummy"

		BONE_R_SHOULDERB,			// "R_ShoulderB"
		BONE_R_SHOULDERA,			// "R_ShoulderA"
		BONE_L_SHOULDERA,			// "L_ShoulderA"
		BONE_L_SHOULDERB,			// "L_ShoulderB"

		BONE_NECK_UPPER,			// "Neck_Upper"
		BONE_HEAD,					// "Head"
		BONE_L_JAW_PISTON_A,		// "L_Jaw_Piston_A"
		BONE_R_JAW_PISTON_A,		// "R_Jaw_Piston_A"
		BONE_NECK_PISTONB,			// "Neck_PistonB"
		BONE_JAW,					// "Jaw"
		BONE_L_JAW_PISTON_B,		// "L_Jaw_Piston_B"
		BONE_R_JAW_PISTON_B,		// "R_Jaw_Piston_B"
		BONE_NECK_PISTONA,			// "Neck_PistonA"

		BONE_CHEST_DOORA,			// "Chest_DoorA"
		BONE_CHEST_DOORB,			// "Chest_DoorB"
		BONE_CHEST_DOORC,			// "Chest_DoorC"
		BONE_CHEST_DOORD,			// "Chest_DoorD"
		BONE_CHEST_DOORE,			// "Chest_DoorE"
		BONE_PRIMARY_FIRE,			// "Primary_Fire"
		BONE_WAISTD,				// "WaistD"
		
		BONE_HEAD_WOUND01,			// "Head_Wound01"
		BONE_HEAD_WOUND02,			// "Head_Wound02"
		BONE_R_ARM_UPPER_WOUND01,	// "R_Arm_Upper_Wound01"
		BONE_R_FOOT_FRONT_WOUND01,	// "R_Foot_Front_Wound01"
		BONE_R_LEG_UPPER_WOUND01,	// "R_Leg_Upper_Wound01"
		BONE_R_LEG_LOWER_WOUND01,	// "R_Leg_Lower_Wound01"
		BONE_L_ARM_UPPER_WOUND01,	// "L_Arm_Upper_Wound01"
		BONE_L_ARM_LOWER_WOUND01,	// "L_Arm_Lower_Wound01"
		BONE_TORSO_WOUND01,			// "Torso_Wound01"
		BONE_TORSO_WOUND02,			// "Torso_Wound02"
		BONE_TORSO_WOUND03,			// "Torso_Wound03"
		BONE_TORSO_WOUND04,			// "Torso_Wound04"
		BONE_TORSO_WOUND05,			// "Torso_Wound05"
		BONE_TORSO_WOUND06,			// "Torso_Wound06"
		BONE_TORSO_WOUND07,			// "Torso_Wound07"
		BONE_TORSO_WOUND08,			// "Torso_Wound08"
		BONE_TORSO_WOUND09,			// "Torso_Wound09"

		BONE_COUNT
	} Bone_e;


	typedef struct {
		CDamageProfile *pDamageProfileRocketAI;	// Rocket damage profile for rockets when AI is in control
		CDamageProfile *pDamageProfileRocketPlayer; // Rocket damage profile for rockets when Player is in control.
		CDamageProfile *pDamageProfileFistSmash;// Damage profile to use when fist smashing
		CDamageProfile *pDamageProfileFootStep; // Damage profile to use when corrosive steps on someone
		CDamageProfile *pDamageProfileFootStomp;// Damage profile to use when corrosive stomps on someone
		CDamageProfile *pDamageProfileFootStepRumble; // The rumble associated with a corrosive footstep
		CDamageProfile *pDamageProfileFootStompRumble;	// The rumble associated with a corrosive foot stomp
		CDamageProfile *pDamageProfileDeathCrashRumble;	// The rumble associated with corrosive falling to the ground dead

		// Explosions
		FExplosion_GroupHandle_t	hRocketExplosion;
		FExplosion_GroupHandle_t	hLimbExplosion;
		FExplosion_GroupHandle_t	hDeathExplosion; // Smaller explosions to play as corrosive dies

		//ROCKET PARAMS
		cchar *pszRocketSmokeTex;
		f32 fRocketReleaseAnimTime;				// time into the chest door opening animation to actually fire the rockets
		f32 fRocketRange;
		f32	fRocketSpeed;

		cchar *pszCoronaTexName;			// Corona texture
		f32 fCoronaScale;					// Corona scale
		f32 fCoronaUnitScreenspaceScale;	// 0=worldspace scale is constant, 1=screenspace scale is constant
		f32 fCoronaColorRed;				// Corona color red (also used for dynamic light)
		f32 fCoronaColorGreen;				// Corona color green (also used for dynamic light)
		f32 fCoronaColorBlue;				// Corona color blue (also used for dynamic light)

		f32 fLightRadius;					// Dynamic light radius

		f32 fProjectileMeshScale;			// Scale to use for the projectile itself

		CEProjExt::CEProj_Merv_StaticParams_t MervStaticParams;

		//hand attack variables
		f32 fFistSmashDamageSphereRadius;	// Radius of sphere which represents the smashing closed fist
		f32 fSwatDamageSphereRadius;		// Radius of sphere which represents the swating hand

		//sounds
		CFSoundGroup *pSoundGroupAngryRoar;			// Roraring sound group
		CFSoundGroup *pSoundGroupLaugh;				// Laughing sound group
		CFSoundGroup *pSoundGroupFistSmashGrunt;	// Grunting sounds to use with fist smash animation
		CFSoundGroup *pSoundGroupSmashCrash;		// Crash sound group used for foot stomps and fist smashes
		CFSoundGroup *pSoundGroupDeath1;			// Death Howl1
		CFSoundGroup *pSoundGroupDeath2;			// Death Howl2
		CFSoundGroup *pSoundGroupDeathCrash;		// The sound of corrosive crashing to the ground dead
		CFSoundGroup *pSoundGroupRocketLauncher;	// Rocket launcher sound
	
		cchar* pszBotTossParticleFXResName;			// The name of the particle effect that gets attached to the spawned bot for a bot toss
		f32 fBotTossFXIntensity;					// The intensity of the particle animation to play for the spawned tossed bot

		cchar* pszVSpotHitParticleFXResName;		// The name of the particle effect that spawns when a vspot gets hit

	} BotInfo_Corrosive_t;

	typedef struct {
		CFVec3A vPos;				// The position of the bounding sphere around this bone
		u8		bAlreadyImpacted;	// This bone has already impacted the world terrain
		u8		nBoneIndex;			// The index of this bone
	} LimbElement_t;

	typedef struct {
		LimbElement_t *paElement;	// pointer to an Array of LimbElement_t ( count == nPartElements )
		u8 nLimbElements;			// The number of limb elements for this part tracker
		u8 nPreviousLimbState;		// The previous state of the limb
	} LimbTracker_t;

	typedef struct {
		f32 fRadius;					// The radius of the sphere
		f32 fMaxEmitTime;				// The length of time this vspot particle system should emit when being hit
		f32 fEmitIntensity;				// The intensity of the vspot particle emission
		u8 nBoneIndex;					// The bone index representing the origin of this VSpot
		s8 nLimbTypeID;					// The limb this VSpot bone belongs to
		u8 bCriticalDamageOnlySpot;		// This Vspot represents a critical damage area
		u8 bAcceptSplashDamage;			// This Vspot accepts splash damage as opposed to requiring an epicenter be within the vspot radius.

		FParticle_EmitterHandle_t hVSpotHitFXEmitter;		// The handle to the individual particle emitter spawned for a specific bot toss
		f32 fEmitTime;					// The amount of time this vspot particle system has been emitting 
	} VSpot_t;

	typedef struct {
		f32 fTriggerTime;				// The amount of time into the death animation to trigger the explosion
		u8 nBoneIndex;					// The bone where this explosion should be located
	} DeathExplosion_t;

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


	// Bot parts:
	static CBotPartPool *m_pPartPool;									// One part pool per bot class

	// BotInfo data:
	static BotInfo_Gen_t			m_BotInfo_Gen;						// General bot info
	static BotInfo_MountAim_t		m_BotInfo_MountAim;					// Mount aim bot info
	static BotInfo_Walk_t			m_BotInfo_Walk;						// Walk bot info
	static BotInfo_Jump_t			m_BotInfo_Jump;						// Jump bot info
	static BotInfo_Weapon_t			m_BotInfo_Weapon;					// Weapon bot info
	static BotInfo_Corrosive_t		m_BotInfo_Corrosive;				// Corrosive bot info

	static CBotCorrosive*			m_pCBCorrosive;						// ptr to this used for callbacks

	// Bot animation stack data:
	static CBotAnimStackDef m_AnimStackDef;								// Animation stack definition data

	static const FGameDataMap_t m_aGameDataMap[];
	static const FGameData_TableEntry_t m_aBotInfoVocab_Corrosive[];

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
	static const u8 m_anEnableBonesUpperBody[];
	static const u8 m_aBoneEnableIndices_Attack[];
	static const u8 m_aBoneEnableIndices_AimSummer[];
	static const u8 m_anEnableBonesLand_Upper[];
	static const u8 m_anEnableBonesLand_Lower[];
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

	static	CFVec3A m_GroinVecY_WS;								// Recorded groin matrice's Y vector (used in the animation callback)
    
	// bones
	s32 m_nBoneIdxGroin;
	s32 m_nBoneIdxTorso;
	s32 m_nBoneIdxHead;
	s32 m_nBoneIdxPrimaryFire;										// Where the hurt comes from
	s32 m_nBoneIdxFistDamageSphere;									// The bone used to track the fist smash damage sphere
	s32 m_nBoneIdxSwatDamageSphere;									// The bone used to track the swat damage sphere
	s32 m_nBoneIdxLFoot;
	s32 m_nBoneIdxLToe;												// The left Toe bone
	s32 m_nBoneIdxLHeel;
	s32 m_nBoneIdxRFoot;											
	s32 m_nBoneIdxRToe;
	s32 m_nBoneIdxRHeel;

	const	CFVec3A *m_pGazeDir_WS;									// Points to the gaze direction vector (might not be unit in length)

	// ffeedback
	FForceHandle_t m_hForce;

	static SmokeTrailAttrib_t	m_SmokeTrailAttrib;							// attributes for our rocket's smoke trail

	//particle stuff
//	static FParticle_DefHandle_t	m_hSparksPartDef;
	static CFTexInst m_CoronaTexInst;


	u32 m_nCorrosiveFlags;											// See CORROSIVEFLAG_* for info

	static CFCollInfo m_CollInfo;
	u8 m_nHitEntityCount;											// Number of valid entries in m_apHitEntity
	u8 m_nLeftFootDamagedEntityCount;								// Number of valid entries in m_apFootDamagedEntity
	u8 m_nRightFootDamagedEntityCount;								// Number of valid entries in m_apFootDamagedEntity
	CEntity *m_apHitEntity[HIT_ENTITY_MAX_COUNT];					// Array of entities that the bot has hit during a slash move
	CEntity *m_apLeftFootDamagedEntity[HIT_ENTITY_MAX_COUNT];		// Array of entities that are currently inside the left foot damage capsules
	CEntity *m_apRightFootDamagedEntity[HIT_ENTITY_MAX_COUNT];		// Array of entities that are currently inside the right foot damage capsules

	//AI / Control interface variables
	BehaviorState_e m_eRequestedBehaviorState;						// What type of behavior is being requested...?
	BehaviorState_e m_eBehaviorState;								// What type of behavior is currently in progress
	BehaviorState_e m_eLastBehaviorState;							// What type of behavior was last in progress
	CEntity* m_pTargetedEntity;										// What entity is currently being targeted? (Used for Rocket Attacks AND Tells Tossed Bots who to attack)

	//
	f32 m_fSwatDamageRadiusMul;										// Radius multiplier for swat attack.

	// Limb tracker variables
	LimbTracker_t *m_paLimbTrackers;								// Array of LimbTracker structures ( Count == LIMB_TYPE_COUNT )

	// VSpot / Damage system variables
	VSpot_t *m_paVSpots;											// Array of VSpot structures (Count == m_nVSpotCount);
	f32 m_fSavedNormHealth;											// Used to store the previous normalized value of the bots health.  NOT FOR ANY OTHER USE
	f32 *m_pafDamageMilestones;										// Pointer to an array of Normalized damage milestones
	u16 m_nVSpotCount;												// Number of VSpot structures
	u16 m_nDamageMilestones;										// Count of floats in m_pafDamageMilestones
	static FParticle_DefHandle_t m_hVSpotHitFXParticle;				// The handle to the particle system used for recently hit vspots


	// Bot Toss variables
	CAlarmNet *m_pBotTossAlarmNet;									// The alarm that will spawn the bots
	CEntity	*m_pBotTossAlarmSpawnSpline;							// The spline entity that will be attached to corrosive for his bot toss
	CBot* m_pAlarmNetSpawnedBot;									// The pointer to the alarm system spawned bot
	u32	  m_nAlarmNetSpawnedBotGUID;								// The guid of the spawned bot.  Use this for sanity checks as the pointer above is volatile.
	static FParticle_DefHandle_t m_hBotTossFXParticle;				// The handle to the particle system used for bot tossess.  This is a shared resource
	FParticle_EmitterHandle_t m_hBotTossFXEmitter;					// The handle to the individual particle emitter spawned for a specific bot toss
	CFVec3A m_vBotTossParticleEpicenter_WS;							// The location the bot toss particle system emits at.
	f32 m_fBotTossParticleTime;										// The length of time the bot toss particle system has been active...

	static DamageInflictedCallback_t m_pDamageCallback;

	// Death variables
	f32 m_fDeathMaxDestructionCountdownSecs;						// How much longer until we transition to BOTLIFECYCLE_DEAD
	f32 m_afDeathRandomRocketFireTimes[ NUM_DEATH_RANDOM_ROCKETS ];	// An array of times into the death sequence to fire off random rockets...
	DeathExplosion_t *m_paDeathExplosions;							// An array of explosions to play during the death sequence
	u8 m_nDeathExplosions;
	
	static BOOL m_bForce2D_RoarRocket;

	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
public:
	CBotCorrosive();
	~CBotCorrosive();

	static BOOL InitSystem( void );
	static void UninitSystem( void );

	BOOL	Create( s32 nPlayerIndex=-1, BOOL bInstallDataPort=FALSE, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

	virtual const CFVec3A *GetApproxEyePoint( void ) const;

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);			

	// This function is custom for corrosive.  I couldn't use ComputeApproxMuzzlePoint because
	// the AI uses that, and when I mess with that routine, corrosive loses LOS with the enemy
	// when under his feet...  So I'll make this routine instead...
	void ComputeExactMuzzlePoint_WS( CFVec3A *pExactMuzzlePoint_WS );

	// Checkpoint routines
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );
	virtual void CheckpointSaveSelect( s32 nCheckpoint );

	virtual void UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup );

	virtual FINLINE void ComputeMtxPalette( BOOL bApplyOffscreenOptimizations ) { FASSERT( IsCreated() ); FASSERT( m_Anim.m_pAnimCombiner != NULL ); m_pCBCorrosive = this; m_Anim.m_pAnimCombiner->ComputeMtxPalette(bApplyOffscreenOptimizations); m_pCBCorrosive = NULL; };

	//AI BEHAVIOR interface
	BOOL RequestBehavior( BehaviorState_e eRequestedBehavior );
	BehaviorState_e GetBehaviorState( void ) { FASSERT( IsCreated() ); return m_eBehaviorState; }
	FINLINE void SetTargetedEntity( CEntity* pEntity ) { FASSERT( IsCreated() ); m_pTargetedEntity = pEntity; } // Use this to define what Entity Rockets should home on, and What entity Tossed bots should attack

	BOOL IsFingerFlicked();
	BOOL IsInMiddleOfSwat();

	BOOL IsEntityHit( CEntity *pEntity );
	void ClearEntityHitList();

	FINLINE void SetSwatDamageRadiusMul(f32 fMul=1.0f) { m_fSwatDamageRadiusMul = fMul; }

	// Bot Toss interfaces
	BOOL CanTossABot( void );								//Checks to see if a bot is available to be tossed.

	// Death functions
	virtual void Die( BOOL bSpawnDeathEffects=TRUE, BOOL bSpawnGoodies=TRUE );
	virtual void DeathWork( void );

	// Misc API's
	FINLINE BOOL IsDamagedVersion( void ) { FASSERT( IsCreated() ); return ( m_nCorrosiveFlags & CORROSIVEFLAG_DAMAGEDVERSION ); }
	FINLINE BOOL LostLimbThisFrame( void ) { FASSERT( IsCreated() ); return ( m_nCorrosiveFlags & CORROSIVEFLAG_LOSTLIMBTHISFRAME ); }
	BOOL ImpactPointInVSpot( CFVec3A *pImpactPt_WS );
	static void DebugDraw(  CBotCorrosive *pCorrosive  );

	FINLINE static void SetInflictDamageCallback(DamageInflictedCallback_t pCallback) { m_pDamageCallback = pCallback; }
	
	FINLINE static void SetForce2DRoarRocket(BOOL bForce) { m_bForce2D_RoarRocket = bForce; }

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
	virtual void ClassHierarchyResolveEntityPointerFixups( void );

	virtual void ClassHierarchyWork( void );
	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );

	virtual void FINLINE ComputeWeaponMuzzlePoint_WS( CFVec3A *pMuzzlePt )		{ FASSERT( IsCreated() ); *pMuzzlePt = m_MtxToWorld.m_vPos; };
	virtual void FINLINE ComputeWeaponMuzzleVec_WS( CFVec3A *pMuzzleVec )		{ FASSERT( IsCreated() ); *pMuzzleVec = m_MtxToWorld.m_vFront; };


	virtual void SetNormHealth( f32 fNormHealth );
	virtual void InflictDamage( CDamageData *pDamageData);

	virtual void NotifyFootDown( BOOL bLeftFoot, BOOL bRightFoot, f32 fUnitMag );

	virtual void Possess( s32 nPlayerIndex, f32 fPossessionArmorModifier );
	//virtual BOOL NotifyBotCollision( CBot *pBot );


//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL _BuildAnimStackDef( void );
	static void _AnimBoneCallback( u32 uBoneidx, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );

	void _ClearDataMembers( void );
	void _UpdateMatrices( void );

	// Foot collision support code
	void _PrepareFootCollisionCapsules( CFCapsule *pCollCapsuleL, CFCapsule *pCollCapsuleR );
	void _CheckForFootCollisions( CFVec3A *pPrevLFootPos_WS, CFVec3A *pPrevRFootPos_WS );
	void _CheckFootCollision( CFCapsule *pCollCapsulePrev, CFVec3A *pvDeltaDist, CFVec3A *pvFootDownVec, BOOL bFootStomping, 
							  u8 *pnEntitiesInCapsuleArray, CEntity** papEntityInCapsuleArray ); 
//	static BOOL _FootCollisionCallback( CFWorldTracker *pTracker );

	// Jumping
	void _EnterFlyMode( void );
	void _HandleJumping( void );
	void _StartVelocityJump( const CFVec3A *pJumpVelocity_WS );
	void _StartSingleJump( const CFVec3A *pJumpVelocity_WS=NULL );
	void _StartDoubleJump( BOOL bAddToVelocityY=TRUE );
	void _JumpLanded( void );
	void _HandleJumpAnimations( void );

	// Limb Tracker support functions
	BOOL _BuildLimbTrackerArrays( void );
	void _LimbTrackerWork_PreUpdateMatricies( void );
	void _LimbTrackerWork_PostUpdateMatricies( void );
	void _LimbTrackerWork_SpawnExplosion( FExplosion_GroupHandle_t hExplosion, CFVec3A &vPos, f32 fRadius );

	// Damage System Support Routines
	BOOL _LoadVSpots( void );
	BOOL _LoadDamageMilestones( void ) ;
	s32 _ComputeImpactDamage( CDamageData *pDamageData );
	s32 _ComputeVSpotBlastDamage( CDamageData *pDamageData );
	void _DamageWork( void );
	LimbType_e _DamageWork_FindLimbToBlowOff( u32 nMilestoneID );

	// Death support routines
	BOOL _LoadDeathExplosions( void );
	void _DeathWork_PlaySounds( f32 fPrevTime, f32 fCurrentTime );
	void _DeathWork_HandleEffects( f32 fPrevTime, f32 fCurrentTime );

	///////////////////////
	// Weapons / Attacks //
	///////////////////////

	// COMMON (SHARED) ROUTINES
	void _HandleCrashEffects( f32 fTime, f32 fTimeToSpawnEffects, const CFVec3A *pCrashEpicenter );

	// ROCKET
	void _FireRocket( void );				// fire a rocket at current aimpoint
	void _InitProjectile( CEProj *pProj, const CFVec3A *pPos_WS, const CFVec3A *pUnitDir_WS, BOOL bNoDynamicLight );
	static void _BuildRocketSkipList( CEProj *pProj );

	// HAND ATTACK SUPPORT ROUTINES ( FIST SMASH / SWAT )
	void _HandleFistSmashAnimation( void );
	void _HandleSwatAnimation( void );
	void _InitStartHandAttackData( void );
	void _StartHandAttackInflictDamageMode( void );
	void _StopHandAttackInflictDamageMode( void );
	void _HandleHandAttackInflictDamageSweetRange( f32 fPrevTime, f32 fCurrentTime, f32 fTimeSweetStart, f32 fTimeSweetEnd );
	void _PerformHandAttackDamageCheck( const CFVec3A *pPrevSpherePos_WS, const CFVec3A *pCurrentSpherePos_WS );
	void _InflictHandAttackDamage( CEntity *pHitEntity, CFWorldMesh *pHitWorldMesh, const FCollImpact_t *pCollImpact );
	static BOOL _HandAttackHitTrackerCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	void _SpawnHandAttackImpactEffects( const FCollImpact_t *pCollImpact );

	// FOOT STOMP -- Also uses foot collision support functions above
	void _HandleFootStompAnimation( AnimTap_e eStompTap );
	void _InitStartFootStompData( void );
	void _StartFootStompInflictDamageMode( void );
	void _StopFootStompInflictDamageMode( void );
	void _HandleFootStompInflictDamageSweetRange( f32 fPrevTime, f32 fCurrentTime, f32 fTimeSweetStart, f32 fTimeSweetEnd );

	// BOT TOSS Routines
	void _HandleBotTossAnimation( void );
	void _BotTossWork( void );

	void _HandleBehaviorAnimations();
	void _HandleAimAnimations( void );

	friend class CBotCorrosiveBuilder;

FCLASS_STACKMEM_ALIGN( CBotCorrosive );
} FCLASS_ALIGN_SUFFIX;


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotCorrosiveBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotCorrosiveBuilder : public CBotBuilder {

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------

public:

	cchar *m_pszBotTossSpawnPtName;			// Name of the Bot Toss spawn point spline entity name.
	cchar *m_pszBotTossAlarmName;			// Name of the Bot Toss Alarm Net entity name.

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CBotCorrosiveBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CBotCorrosiveBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif