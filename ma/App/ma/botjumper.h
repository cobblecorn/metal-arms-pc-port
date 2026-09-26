//////////////////////////////////////////////////////////////////////////////////////
// botjumper.h - Jump Trooper
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
// 02/09/03 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _BOTJUMPER_H_
#define _BOTJUMPER_H_ 1


#include "fang.h"
#include "bot.h"
#include "fparticle.h"
#include "fforce.h"
#include "FXStreamer.h"
#include "tracer.h"
#include "explosion.h"

// Forward Declarations
struct FAnim_t;
class  CBotAIBrain;
class  CMeshEntity;
class  CBotPartPool;



FCLASS_ALIGN_PREFIX class CBotJumper : public CBot {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum
	{
		JUMPERFLAG_NONE = 0x00000000
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
		_MAX_HIT_ENTITIES				= 4,
		_FIREPOINT_COUNT				= 4
	};


	typedef enum {
		LIMB_TYPE_HEAD,							// The bot's head
		LIMB_TYPE_LEFT_ARM,						// The bot's left arm
		LIMB_TYPE_RIGHT_ARM,					// The bot's right arm
		LIMB_TYPE_LEFT_WING,					// The jumper's left wing
		LIMB_TYPE_RIGHT_WING,					// The jumper's right wing
		LIMB_TYPE_CHEST_TOP, 
		LIMB_TYPE_CHEST_MIDDLE,
		LIMB_TYPE_CHEST_BOTTOM,

		LIMB_TYPE_COUNT
	} LimbType_e;


	// Animation controls:
	typedef enum {
		ANIMCONTROL_STAND,
		ANIMCONTROL_WALK,
		ANIMCONTROL_RUN,

		ANIMCONTROL_JUMP_LAUNCH,
		ANIMCONTROL_JUMP_FLY_JETS,
		ANIMCONTROL_JUMP_FLY,
		ANIMCONTROL_JUMP_LAND_UPPER,
		ANIMCONTROL_JUMP_LAND_LOWER,

		ANIMCONTROL_AIM_SUMMER,

		ANIMCONTROL_FIRE_1,
		ANIMCONTROL_ATTACK_SUMMER,
		ANIMCONTROL_ATTACK_START,
		ANIMCONTROL_ATTACK,
		ANIMCONTROL_ATTACK_END,

		ANIMCONTROL_HOP_LEFT,
		ANIMCONTROL_HOP_RIGHT,

		ANIMCONTROL_RC_TETHERED_UPPER,
		ANIMCONTROL_RC_TETHERED,
		ANIMCONTROL_RC_POWER_DOWN,
		ANIMCONTROL_RC_POWER_UP,
		ANIMCONTROL_RC_POWER_DOWN_INV,
		ANIMCONTROL_RC_POWER_UP_INV,

		ANIMCONTROL_DROP,
		ANIMCONTROL_AWARE,
		ANIMCONTROL_WAKE,
		ANIMCONTROL_SLEEP,

		ANIMCONTROL_DIVE,
		ANIMCONTROL_DIVE_RECOVER_UPPER,
		ANIMCONTROL_DIVE_RECOVER_LOWER,

		ANIMCONTROL_BASE_COUNT
	} AnimControl_e;


	// Animation taps:
	typedef enum {
		ANIMTAP_STAND,
		ANIMTAP_WALK,
		ANIMTAP_RUN,

		ANIMTAP_JUMP_LAUNCH,
		ANIMTAP_JUMP_FLY_JETS,
		ANIMTAP_JUMP_FLY,
		ANIMTAP_JUMP_LAND_UPPER,
		ANIMTAP_JUMP_LAND_LOWER,

		ANIMTAP_AIM_SUMMER,

		ANIMTAP_FIRE_1,
		ANIMTAP_ATTACK_SUMMER,
		ANIMTAP_ATTACK_START,
		ANIMTAP_ATTACK,
		ANIMTAP_ATTACK_END,

		ANIMTAP_HOP_LEFT,
		ANIMTAP_HOP_RIGHT,

		ANIMTAP_RC_TETHERED_UPPER,
		ANIMTAP_RC_TETHERED,
		ANIMTAP_RC_POWER_DOWN,
		ANIMTAP_RC_POWER_UP,
		ANIMTAP_RC_POWER_DOWN_INV,
		ANIMTAP_RC_POWER_UP_INV,

		ANIMTAP_DROP,
		ANIMTAP_AWARE,
		ANIMTAP_WAKE,
		ANIMTAP_SLEEP,

		ANIMTAP_DIVE,
		ANIMTAP_DIVE_RECOVER_UPPER,
		ANIMTAP_DIVE_RECOVER_LOWER,


		ANIMTAP_BASE_COUNT
	} AnimTap_e;


	// Base animations:
	typedef enum {
		ANIM_STAND,
		ANIM_WALK,
		ANIM_RUN,

		ANIM_JUMP_LAUNCH,
		ANIM_JUMP_FLY_JETS,
		ANIM_JUMP_FLY,
		ANIM_JUMP_LAND,

		ANIM_FIRE_1,
		ANIM_ATTACK_START,
		ANIM_ATTACK,
		ANIM_ATTACK_END,

		ANIM_HOP_LEFT,
		ANIM_HOP_RIGHT,

		ANIM_RC_TETHERED,
		ANIM_RC_POWER_DOWN,
		ANIM_RC_POWER_UP,
		ANIM_RC_POWER_DOWN_INV,
		ANIM_RC_POWER_UP_INV,

		ANIM_DROP,
		ANIM_WAKE,
		ANIM_SLEEP,

		ANIM_DIVE,
		ANIM_DIVE_RECOVER,

		ANIM_BASE_COUNT
	} Anim_e;


	// Bones:
	typedef enum {
		BONE_GROIN,				// "Groin",						
		BONE_TORSO,				// "Torso",
		BONE_HEAD,				// "Head",
		BONE_CHEST_TOP,			// "Chest_Top",
		BONE_CHEST_BOTTOM,		// "Chest_Bottom",
		BONE_CHEST_MIDDLE,		// "Chest_Middle",
		BONE_ARM_UPPER_L,		// "L_Arm_Upper",
		BONE_ARM_LOWER_L,		// "L_Arm_Lower",
		BONE_GUN_2_L,			// "L_Gun_Barrel2",
		BONE_GUN_1_L,			// "L_Gun_Barrel1",
		BONE_ARM_ARMOR_L,		// "L_Arm_Armor",
		BONE_PRIFIRE_1_L,		// "Primary_Fire_L1",
		BONE_PRIFIRE_2_L,		// "Primary_Fire_L2",

		BONE_ARM_UPPER_R,		// "R_Arm_Upper",
		BONE_ARM_LOWER_R,		// "R_Arm_Lower",
		BONE_GUN_2_R,			// "R_Gun_Barrel2",
		BONE_GUN_1_R,			// "R_Gun_Barrel1",
		BONE_ARM_ARMOR_R,		// "R_Arm_Armor",
		BONE_PRIFIRE_1_R,		// "Primary_Fire_R1",
		BONE_PRIFIRE_2_R,		// "Primary_Fire_R2",

		BONE_WING_L,			// "L_Wing",
		BONE_WING_R,			// "R_Wing",

		BONE_LEG_UPPER_L,		// "L_Leg_Upper",
		BONE_LEG_KNEE_L,		// "L_Kneepad",
		BONE_LEG_LOWER_L,		// "L_Leg_Lower",
		BONE_LEG_ANGKLE_L,		// "L_Ankle",
		BONE_FOOT_BACK_L,		// "L_Foot_Back",
		BONE_FOOT_BLADE_L,		// "Left_Foot_Blade",
		BONE_FOOT_FRONT_L,		// "L_Foot_Front",
		BONE_TOE1_L,			// "L_Toe1",
		BONE_TOE2_L,			// "L_Toe2",
								

		BONE_LEG_UPPER_R,		// "R_Leg_Upper",
		BONE_LEG_KNEE_R,		// "R_Kneepad",
		BONE_LEG_LOWER_R,		// "R_Leg_Lower",
		BONE_LEG_ANGKLE_R,		// "R_Ankle",
		BONE_FOOT_BACK_R,		// "R_Foot_Front",
		BONE_FOOT_BLADE_R,		// "R_Foot_Back",
		BONE_FOOT_FRONT_R,		// "R_Foot_Blade",
		BONE_TOE1_R,			// "R_Toe1",	
		BONE_TOE2_R,			// "R_Toe2",

		BONE_DUMMY,				// "JumpTrooperDummy"

		BONE_COUNT
	} Bone_e;

	typedef enum {
		SUMMERBONE_ARM_UPPER_L,
		SUMMERBONE_ARM_UPPER_R,
		
		SUMMERBONE_COUNT
	} SummerBone_e;


	typedef enum {
		MELEE_STATE_NONE,

		MELEE_STATE_STARTING,
		MELEE_STATE_SLASHING,
		MELEE_STATE_FINISHING

	} MeleeState_e;

	typedef enum {
		JUMPJETS_OFF,

		JUMPJETS_AIFLY,
		JUMPJETS_LAUNCH,
		JUMPJETS_HOVER,
		JUMPJETS_DESCEND,
		JUMPJETS_STOPPING,

		JUMPJETS_DIVE_STARTING,
		JUMPJETS_DIVING,
		JUMPJETS_DIVE_ENDING

	} JumpJetState_e;


	typedef struct {
		CDamageProfile *apDamageProfileGun[2];		// [0]=NPC, [1]=Possessed
		CDamageProfile *apDamageProfileMelee[2];	// [0]=NPC, [1]=Possessed
		CDamageProfile *apDamageProfileDive[2];		// [0]=NPC, [1]=Possessed

		FExplosion_GroupHandle_t hGunExplosion;		// Secondary gun explosion group

		FParticle_DefHandle_t hParticleDefJumpJetSmoke;
		FParticle_DefHandle_t hParticleDefJumpJetFire;
		FParticle_DefHandle_t hParticleDefJumpLaunch;

		f32		fMaxAirAccel;		// max air acceleration
		f32		fDecelMult;			// bonus for decelerating in air
		f32		fTerminalVelocity;	// fastest falling speed
		f32		fJumpLaunchVel;
		f32		fJumpLaunchAccel;
		f32		fJumpLaunchTime;
		f32		fJumpDescendVel;
		f32		fJumpMinInterval;	// min time between stopping jets & relaunching
		f32		fJumpDiveInterval;	// min time after diving before relaunching

		f32		fMeleeAttackSpeed;

		u32		uMaxTracers;		// how many tracers & streamers are in the pool?

		// gun data
		f32		fTracerSpeed;
		f32		fFireRate;			// rate the fire animation plays

		f32		fWingStreamerAlphaAI;
		f32		fWingStreamerAlphaPlayer;
		f32		fDiveMaxDistance;

		f32		fSoundVolume3D;
		f32		fSoundVolume2D;
		f32		fSoundRadius3D;

		f32		fDiveTurnDegPerSec;
		f32		fDiveTurnDegPerSecSinSq;

		//f32		fHorizVelocityJump1;
	} BotInfo_Jumper_t;

	typedef struct {
		// sound handles
		FSndFx_FxHandle_t hMeleeAttack;
		FSndFx_FxHandle_t hMeleeImpact;
		FSndFx_FxHandle_t hJetsLaunch;
		FSndFx_FxHandle_t hJetsLoop;
		FSndFx_FxHandle_t hGunsFire;
		FSndFx_FxHandle_t hDive;
	} BotInfo_JumperSounds_t;


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
	static BotInfo_Jumper_t			m_BotInfo_Jumper;					// Jumper bot info
	static BotInfo_JumperSounds_t	m_BotInfo_JumperSounds;				// Jumper sfx

	static CBotJumper*				m_pCBJumper;						// ptr to this used for callbacks

	// Bot animation stack data:
	static CBotAnimStackDef m_AnimStackDef;								// Animation stack definition data

	static const FGameDataMap_t m_aGameDataMap[];
	static const FGameDataMap_t m_aSoundDataMap[];
	static const FGameData_TableEntry_t m_aBotInfoVocab_Jumper[];
	static const FGameData_TableEntry_t m_aBotInfoVocab_JumperSounds[];


	static cchar *m_apszBoneNameTable[BONE_COUNT];						// Array of bone names
	static cchar *m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT];	// Array of base animation control names
	static cchar *m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT];			// Array of base animation tap names
	static cchar *m_apszIdleAnimNameTable[ANIM_IDLE_COUNT];				// Array of idle animation names
	static cchar *m_apszBaseAnimNameTable[ANIM_BASE_COUNT];				// Array of base animation names
	static cchar *m_apszSummerBoneNames[SUMMERBONE_COUNT];				// Array of names for summer bones

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

	static TracerGroupHandle_t		m_hTracerGroup;				// tracer group
	static TracerDef_t				m_TracerDef;				// Tracer definition for guns
	static CFTexInst				m_TracerTexInst;			// tracer texture

	// gun streamer data
	static CFTexInst				m_GunStreamerTexInst;
	static u32						m_uNoActiveStreamers;
	static FXStreamerHandle_t		*m_aGunStreamerPool;
	static CFMtx43A					*m_aGunStreamerMtxPool;
	static f32						*m_afGunStreamerTimer;

	static CFTexInst				m_BladeStreamerTex;
	static CFTexInst				m_StreamerTexGreen;
	static CFTexInst				m_StreamerTexRed;
	
	// jump jet particles
	FParticle_EmitterHandle_t m_hJetSmokeParticleLeft;
	FParticle_EmitterHandle_t m_hJetSmokeParticleRight;
	FParticle_EmitterHandle_t m_hJetFireParticleLeft;
	FParticle_EmitterHandle_t m_hJetFireParticleRight;
	FParticle_EmitterHandle_t m_hJetLaunchParticleLeft;
	FParticle_EmitterHandle_t m_hJetLaunchParticleRight;
	
	f32 m_fJetsParticleIntensity;

	CFAnimManFrame *m_pAnimManFrameAttack;

	// bones
	s32 m_nBoneIdxGroin;
	s32 m_nBoneIdxTorso;
	s32 m_nBoneIdxHead;
	s32 m_nBoneIdxMeleeLeft;
	s32 m_nBoneIdxMeleeRight;
	s32 m_nBoneIdxArmLower_L;										// lower arm (gun aiming)
	s32 m_nBoneIdxArmLower_R;										// lower arm (gun aiming)

	s32 m_nBoneIdxJetFireLeft;
	s32 m_nBoneIdxJetFireRight;
	s32 m_nBoneIdxJetSmokeLeft;
	s32 m_nBoneIdxJetSmokeRight;


	s32 m_anBoneIdxPriFire[_FIREPOINT_COUNT];

	const	CFVec3A *m_pGazeDir_WS;									// Points to the gaze direction vector (might not be unit in length)

	// melee attack:
	//BOOL			m_bControls_Melee;
	MeleeState_e	m_MeleeState;
	f32				m_fAttackAngleCurrentL;
	f32				m_fAttackAngleCurrentR;
	f32				m_fAttackAngleDesiredL;			
	f32				m_fAttackAngleDesiredR;
	//CFVec3A			m_vLastMeleePositionLeft;						// where the melee attack bone was last frame
	//CFVec3A			m_vLastMeleePositionRight;						// where the melee attack bone was last frame
	//u8				m_nHitEntityCountLeft;
	//u8				m_nHitEntityCountRight;
	//CEntity*		m_apHitEntityBufferLeft[_MAX_HIT_ENTITIES];
	//CEntity*		m_apHitEntityBufferRight[_MAX_HIT_ENTITIES];
	//BOOL			m_bAttackingWithLeft;
	//BOOL			m_bHitWithLeftThisAttack;
	//BOOL			m_bHitWithRightThisAttack;

	// gun:
	f32		m_fFireTimer;
	CFQuatA m_qArmAim_L;
	CFQuatA m_qArmAim_R;

	CFAnimFrame	m_TorsoAnimFrame;

	// jump jets
	JumpJetState_e	m_JumpJetState;
	f32				m_fJumpJetTimer;
	f32				m_fJumpJetDesiredVelocity;
	CEntity*		m_pDiveTgtEntity;
	CFVec3A			m_vDiveTgt;										// the target point for the dive attack
	f32				m_fDiveTimer;
	f32				m_fDiveStartAccel;								// calculated when a dive is requested to get headed in the right direction
	f32				m_fDiveDecelSpeed;								// amount needed to decelerate from current speed
	f32				m_fDiveDistance;								// distance covered by this dive
	BOOL			m_bDiveUseControls;								// starts as FALSE, if player moves the stick, becomes true, then player is in control
	BOOL			m_b3DSounds;									// are the sounds 3d
	CFAudioEmitter	*m_pJumpJetAudioEmitter;
	CFAudioEmitter	*m_pJumpDiveAudioEmitter;
	f32				m_fJumpJetSoundVolume;
	f32				m_fUnitJumpJetRecharge;
	f32				m_fJumpJetSputterTime;							// timer used in making the jump jets sputter when out of gas
	BOOL			m_bAllowJumpTranslation;						// bot flies on original velocity until controls are touched

	// streamers
	static cchar*		m_apszStreamerBonesRed[];					// only one
	static cchar*		m_apszStreamerBonesGreen[];					// only one
	static cchar*		m_apszStreamerBonesBladeLeft[];				// only one
	static cchar*		m_apszStreamerBonesBladeRight[];			// only one

	FXStreamerHandle_t	m_hStreamerGreen;
	FXStreamerHandle_t	m_hStreamerRed;
	FXStreamerHandle_t	m_hStreamerBladeLeft;
	FXStreamerHandle_t	m_hStreamerBladeRight;

	// ffeedback
	FForceHandle_t m_hForce;

	MeleeData_t m_MeleeDataLeft;
	MeleeData_t m_MeleeDataRight;
	

	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
public:
	CBotJumper();
	~CBotJumper();

	static BOOL InitSystem( void );
	static void UninitSystem( void );

	BOOL	Create( s32 nPlayerIndex=-1, BOOL bInstallDataPort=FALSE, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

	virtual const CFVec3A *GetApproxEyePoint( void ) const;

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);			

	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );
	virtual void CheckpointSaveSelect( s32 nCheckpoint );

	virtual void UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup );

	void KillJumpJets( void );

	FINLINE BOOL JumpJetsAreOn( void ) const			{ return (m_JumpJetState > JUMPJETS_OFF); };
	FINLINE BOOL IsDiving( void ) const					{ return (m_JumpJetState >= JUMPJETS_DIVE_STARTING) && (m_JumpJetState <= JUMPJETS_DIVE_ENDING); };
	FINLINE BOOL IsSlashing( void ) const				{ return (m_MeleeState != MELEE_STATE_NONE); }
	virtual BOOL WakeUp( void );
	virtual FINLINE BOOL CanSleep( void ) const 		{ return TRUE;}
	virtual BOOL NapJerk( BOOL bAllowAutoWakeup = TRUE) { return FALSE; };
	virtual void ComputeMtxPalette( BOOL bApplyOffscreenOptimizations )	{ FASSERT( IsCreated() ); FASSERT( m_Anim.m_pAnimCombiner ); m_pCBJumper = this; m_Anim.m_pAnimCombiner->ComputeMtxPalette(bApplyOffscreenOptimizations); m_pCBJumper = NULL; };

	virtual f32 ComputeEstimatedControlledStopTimeXZ( void );
	virtual FINLINE f32 ComputeEstimatedControlledStopTimeY( void );


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

	virtual void FINLINE ComputeWeaponMuzzlePoint_WS( CFVec3A *pMuzzlePt )		{ FASSERT( IsCreated() ); *pMuzzlePt = m_pWorldMesh->GetBoneMtxPalette()[m_anBoneIdxPriFire[0]]->m_vPos; };
	virtual void FINLINE ComputeWeaponMuzzleVec_WS( CFVec3A *pMuzzleVec )		{ FASSERT( IsCreated() ); *pMuzzleVec = m_pWorldMesh->GetBoneMtxPalette()[m_anBoneIdxPriFire[0]]->m_vFront; };

	virtual void Possess( s32 nPlayerIndex, f32 fPossessionArmorModifier );
	virtual BOOL NotifyBotCollision( CBot *pBot );
	virtual void MeleeCollision( CEntity *pEntity, FCollImpact_t *pImpact, BOOL bFirstCollision );

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL _BuildAnimStackDef( void );
	static BOOL _InitTracerPool( void );

	void _ClearDataMembers( void );
	void _UpdateMatrices( void );

	static void _AnimBoneCallback( u32 uBoneidx, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	static void _TracerKilledCallback( TracerDef_t *pTracerDef, TracerKillReason_e nKillReason, const FCollImpact_t *pImpact );
	static void _TracerBuildTrackerSkipList( void *pUser );
	static void _TracerMovedCB( void* pUserData, const CFVec3A& NewPos_WS, BOOL bDied );


	void _EnterFlyMode( void );
	void _HandleJumping( void );
	void _JumpLanded( void );
	void _HandleJumpAnimations( void );
	void _StartSingleJump( const CFVec3A *pJumpVelocity_WS=NULL );
	void _StartDoubleJump( void );
	void _HandleAirTranslation( void );

	void _HandleSleeping( void );

	void _MeleeWork( void );
	void _NewRandomAttack( void );
	void _AttackSummerBlendWork( void );
	void _SpawnMeleeEffects( const FCollImpact_t *pCollImpact );

	void _GunWork( void );
	void _SoundWork( void );

	void _HandleAimAnimations( void );
	void _AimArm( u32 uBoneIdx, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );

	void _JumpJetWork( void );
	void _JumpJetsSputter( void );


	void _BeginDive( void );
	void _HandleDiveTranslation( void );
	void _EndDive( BOOL bHitSomething );
	void _CheckForDiveCollision( void );

	void _KillAllEmitters( void );										// will stop all particle emitters
	void _CheckAndStartEmitter( FParticle_EmitterHandle_t *pEmitter, FParticle_DefHandle_t *pDefHandle, s32 nBoneIdx);	// check to see if particle emitter is 
	void _StopEmitter( FParticle_EmitterHandle_t *pEmitter );
	void _SetInverted( BOOL bInverted );

	BOOL _CheckForCeilingCollision( void );
	void _MeleeSwing( void );
	
	
	friend class CBotJumperBuilder;

FCLASS_STACKMEM_ALIGN( CBotJumper );
} FCLASS_ALIGN_SUFFIX;


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotJumperBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotJumperBuilder : public CBotBuilder {
public:

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CBotJumperBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CBotJumperBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif

