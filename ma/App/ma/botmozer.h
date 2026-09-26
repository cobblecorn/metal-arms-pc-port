//////////////////////////////////////////////////////////////////////////////////////
// botmozer.h -
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

#ifndef _BOTMOZER_H_
#define _BOTMOZER_H_ 1

#include "bot.h"
#include "fxstreamer.h"
#include "iteminst.h"
#include "fforce.h"
// Forward Declarations

class CBotPartPool;


FCLASS_ALIGN_PREFIX class CBotMozer : public CBot
{
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum
	{
		PART_INSTANCE_COUNT_PER_TYPE = 1,
		HIT_ENTITY_MAX_COUNT = 5,				// Number of entities we can cause damage to during a slash
	};

	typedef enum 
	{
		MOZERSND_SWIPEFORWARD,
		MOZERSND_SWIPEBACK,
		MOZERSND_SWIPEIMPACT,
		MOZERSND_COUNT,
	}MozerSound_e;

	typedef enum
	{
		GIRDER_SWING_NONE,			// no anim
		GIRDER_SWING_WINDUP,		
		GIRDER_SWING_FORWARD,		
		GIRDER_SWING_FORWARD_DONE,	
		GIRDER_SWING_FORWARD_DONE_UNDO,
		GIRDER_SWING_BACK,			
		GIRDER_SWING_BACK_DONE,	
		GIRDER_SWING_BACK_DONE_UNDO,
		GIRDER_SWING_STATE_COUNT,
	}GirderSwingState_e;


	typedef enum {
		LIMB_TYPE_HEAD,
		LIMB_TYPE_LEFTARM,
		LIMB_TYPE_RIGHTARM,
		LIMB_TYPE_BACK,

		LIMB_TYPE_COUNT,

		LIMB_TYPE_FIRST_DEATH_LIMB	= LIMB_TYPE_HEAD,
		LIMB_TYPE_NUM_DEATH_LIMBS	= LIMB_TYPE_BACK - LIMB_TYPE_FIRST_DEATH_LIMB + 1
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

	// Base animations:
	typedef enum
	{
		ANIM_STAND,					// ARDCidle001
		ANIM_WALK,					// ARDCwalkF01
		ANIM_RUN,					// ARDCrun_F01

		ANIM_JUMP_LAUNCH,			// ARDCjump_01
		ANIM_JUMP_FLY,				// ARDCjump_02
		ANIM_JUMP_LAND,				// ARDCjump_03

		ANIM_JUMP2_FLY,				// ARDCjump201
		ANIM_JUMP2_LAND,			// ARDCjump202
									
		ANIM_FIRE_PRIMARY,			// ARDCready01

		ANIM_SWING_WINDUP,			// ARDCactn2a0
		ANIM_SWING_FORWARD,			// ARDCactn2a1
		ANIM_SWING_BACK,			// ARDCactn2b1
		ANIM_SWING_FORWARD_DONE,	// ARDCactn2a2
		ANIM_SWING_BACK_DONE,		// ARDCactn2b2

		ANIM_RELOAD_CLIP_EJECT_OLD,
		ANIM_RELOAD_CLIP_GRAB_NEW,
		ANIM_RELOAD_CLIP_INSERT_NEW,
		ANIM_RELOAD_CLIP_SLAPIN_NEW,

		ANIM_BASE_COUNT

	} Anim_e;

	// Animation controls:
	typedef enum
	{
		ANIMCONTROL_STAND,					// ARDCidle001
		ANIMCONTROL_WALK,					// ARDCwalkF01
		ANIMCONTROL_RUN,					// ARDCrun_F01

		ANIMCONTROL_JUMP_LAUNCH,			// ARDCjump_01
		ANIMCONTROL_JUMP_FLY,				// ARDCjump_02
		ANIMCONTROL_JUMP_LAND_UPPER,		// ARDCjump_03
		ANIMCONTROL_JUMP_LAND_LOWER,		// ARDCjump_03

		ANIMCONTROL_JUMP2_FLY,				// ARDCjump201
		ANIMCONTROL_JUMP2_LAND,				// ARDCjump202

		ANIMCONTROL_FIRE_1_UPPER,			// ARDCready01
		ANIMCONTROL_FIRE_1_LOWER,			// ARDCready01
		
		ANIMCONTROL_SWING_WINDUP_UPPER,			// ARDCactn2a0
		ANIMCONTROL_SWING_FORWARD_UPPER,		// ARDCactn2a1
		ANIMCONTROL_SWING_BACK_UPPER,			// ARDCactn2b1
		ANIMCONTROL_SWING_FORWARD_DONE_UPPER,	// ARDCactn2a2
		ANIMCONTROL_SWING_BACK_DONE_UPPER,		// ARDCactn2b2

		ANIMCONTROL_SWING_WINDUP_LOWER,			// ARDCactn2a0
		ANIMCONTROL_SWING_FORWARD_LOWER,		// ARDCactn2a1
		ANIMCONTROL_SWING_BACK_LOWER,			// ARDCactn2b1
		ANIMCONTROL_SWING_FORWARD_DONE_LOWER,	// ARDCactn2a2
		ANIMCONTROL_SWING_BACK_DONE_LOWER,		// ARDCactn2b2

		ANIMCONTROL_RELOAD_CLIP_EJECT_OLD,
		ANIMCONTROL_RELOAD_CLIP_GRAB_NEW,
		ANIMCONTROL_RELOAD_CLIP_INSERT_NEW,
		ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW,

		// without anim
		ANIMCONTROL_RECOIL_SUMMER,
		ANIMCONTROL_BASE_COUNT

	} AnimControl_e;


	// Animation taps:
	typedef enum
	{
		ANIMTAP_STAND,					// ARDCidle001
		ANIMTAP_WALK,					// ARDCwalkF01
		ANIMTAP_RUN,					// ARDCrun_F01

		ANIMTAP_JUMP_LAUNCH,			// ARDCjump_01
		ANIMTAP_JUMP_FLY,				// ARDCjump_02
		ANIMTAP_JUMP_LAND_UPPER,		// ARDCjump_03
		ANIMTAP_JUMP_LAND_LOWER,		// ARDCjump_03

		ANIMTAP_JUMP2_FLY,				// ARDCjump201
		ANIMTAP_JUMP2_LAND,				// ARDCjump202

		ANIMTAP_FIRE_1_UPPER,			// ARDCready01
		ANIMTAP_FIRE_1_LOWER,			// ARDCready01
		
		ANIMTAP_SWING_WINDUP_UPPER,			// ARDCactn2a0
		ANIMTAP_SWING_FORWARD_UPPER,		// ARDCactn2a1
		ANIMTAP_SWING_BACK_UPPER,			// ARDCactn2b1
		ANIMTAP_SWING_FORWARD_DONE_UPPER,	// ARDCactn2a2
		ANIMTAP_SWING_BACK_DONE_UPPER,		// ARDCactn2b2

		ANIMTAP_SWING_WINDUP_LOWER,			// ARDCactn2a0
		ANIMTAP_SWING_FORWARD_LOWER,		// ARDCactn2a1
		ANIMTAP_SWING_BACK_LOWER,			// ARDCactn2b1
		ANIMTAP_SWING_FORWARD_DONE_LOWER,	// ARDCactn2a2
		ANIMTAP_SWING_BACK_DONE_LOWER,		// ARDCactn2b2

		ANIMTAP_RELOAD_CLIP_EJECT_OLD,
		ANIMTAP_RELOAD_CLIP_GRAB_NEW,
		ANIMTAP_RELOAD_CLIP_INSERT_NEW,
		ANIMTAP_RELOAD_CLIP_SLAPIN_NEW,

		// without anim
		ANIMTAP_RECOIL_SUMMER,
		ANIMTAP_BASE_COUNT
	} AnimTap_e;


	// Bones:
	typedef enum
	{
		// lower body				
		BONE_GROIN,				
		BONE_R_LEG_UPPER,			
		BONE_R_LEG_LOWER,			
		BONE_R_FOOT_FRONT,			
		BONE_R_TOE,				
		BONE_R_FOOT_BACK,			
		BONE_R_HEEL,				
		BONE_L_LEG_UPPER,			
		BONE_L_LEG_LOWER,			
		BONE_L_FOOT_FRONT,			
		BONE_L_TOE,				
		BONE_L_FOOT_BACK,			
		BONE_L_HEEL,				
		BONE_COLOSSUSDUMMY,			
		BONE_OFF_R_FOOT,			
		BONE_OFF_L_FOOT,			
		BONE_OFF_R_HAND,			
		BONE_OFF_L_LEG_IK,			
		BONE_OFF_R_LEG_IK,			

		// upper body				
		BONE_L_ARM_UPPER,			
		BONE_TORSO,				
		BONE_R_ARM_UPPER,			
		BONE_R_ARM_ELBOW,			
		BONE_GUN,				
		BONE_L_ARM_ELBOW,			
		BONE_L_ARM_LOWER,			
		BONE_L_HAND,				
		BONE_GIRDER,				
		BONE_SECONDARY_FIRE,			
		BONE_BACKB,				
		BONE_BACKA,				
		BONE_R_JOWL,				
		BONE_L_JOWL,				
		BONE_BACKC,				
		BONE_BACKD,				
		BONE_CYLINDER01,			
		BONE_HEAD,				
		BONE_JAW,				
		BONE_R_FLAP,				
		BONE_L_FLAP,				
		BONE_ATTACHPOINT_PRIMARY,		
		BONE_OFF_L_HAND,			
		BONE_OFF_L_ARM_IK,			
		BONE_OFF_R_ARM_IK,
		BONE_COUNT
	} Bone_e;

	// bones used by recoil animation
	typedef enum {
		ANIMBONE_RECOIL_R_ARM_UPPER,
		ANIMBONE_RECOIL_TORSO,
		ANIMBONE_RECOIL_L_ARM_UPPER,
		ANIMBONE_RECOIL_DUMMY,
		ANIMBONE_RECOIL_COUNT
	};

	typedef struct 
	{
		f32 fMaxShockwaveImpulse;				// maximum velocity impulse to give affected bots
		f32 fMinShockwaveImpulse;				// minimum velocity impulse to give affected bots
	} BotInfo_Mozer_t;

	typedef struct
	{
		cchar *pszDamageProfileSlash;					// Name of the damage profile to use when slashing
		f32 fSwingPush;
		f32 fSwingSpeedForward;
		f32 fSwingSpeedBackward;
		f32 fStreamerLength;
		f32 fSwingSwingForwardStreamerOn;
		f32 fSwingSwingForwardStreamerFadeOff;
		f32 fSwingSwingForwardStreamerOff;
		f32 fSwingSwingBackwardStreamerOn;
		f32 fSwingSwingBackwardStreamerFadeOff;
		f32 fSwingSwingBackwardStreamerOff;
		f32 fStreamerAlpha;
		f32 fTimeScale;
	} BotInfo_Girder_t;

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
	static BotInfo_Jump_t m_BotInfo_Jump;				// Jump bot info
	static BotInfo_Weapon_t m_BotInfo_Weapon;			// Weapon bot info
	static BotInfo_Idle_t*	m_BotInfo_Idle;				// Idle bot info
	static u32				m_nBotInfo_Idles;			// Idle bot info count
	static BotInfo_Mozer_t	m_BotInfo_Mozer;			// Mozer information
	static BotInfo_Girder_t	m_BotInfo_Girder;			// Girder information

	static const FGameData_TableEntry_t m_aBotInfoVocab_Mozer[];
	static const FGameData_TableEntry_t m_aBotInfoVocab_Girder[];

	static const FGameDataMap_t m_aGameDataMap[];
	static const CDamageProfile *m_pDamageProfileSlash;	// Damage profile for the slash attack

	// Bot animation stack data:
	static CBotAnimStackDef m_AnimStackDef;				// Animation stack definition data

	static CBotPartPool *m_pPartPool;

	static cchar *m_apszBoneNameTable[BONE_COUNT];										// Array of bone names
	static cchar *m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT];					// Array of base animation control names
	static cchar *m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT];							// Array of base animation tap names
	static cchar *m_apszBaseAnimNameTable[ANIM_BASE_COUNT];								// Array of base animation names
	static cchar **m_apszIdleAnimNameTable;												// Array of idle animation names
	static cchar *m_apszRecoilBoneNameTable[ANIMBONE_RECOIL_COUNT];						// Array of bone names in recoil animation
	static const u8 *m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT];
	static const u8 m_anTagPointBoneNameIndexArray[];
	static const u8 m_nApproxEyePointBoneNameIndex;
	static const u8 **m_apnEnableBoneNameIndexTableForEachIdleTap;

	static CFAnimCombinerConfig::ConfigStack_t m_aAnimCombinerConfigStack[];
	static CFAnimCombinerConfig::ConfigTap_t m_aAnimCombinerConfigTaps[];
	static CFAnimCombiner::AttachList_t m_aAnimAttach[];

	static const u8 m_aBoneEnableIndices_FullBody[];
	static const u8 m_aBoneEnableIndices_UpperBody[];
	static const u8 m_aBoneEnableIndices_LowerBody[];
	static const u8 m_aBoneEnableIndices_RecoilSummer[];
	static const u8 m_aBoneEnableIndices_PriAttack[];

	static const u8 m_anEnableBoneNameIndexTableForSummer_Normal[];

/*
	static const u8 m_aBoneEnableIndices_UserAnim_UpperBody[];
	static const u8 m_aBoneEnableIndices_UserAnim_LowerBody[];
	static const u8 m_aBoneEnableIndices_UserAnim_UpperTorso[];
	static const u8 m_aBoneEnableIndices_UserAnim_LowerTorso[];
	static const u8 m_aBoneEnableIndices_UserAnim_LeftArm[];
	static const u8 m_aBoneEnableIndices_UserAnim_RightArm[];
	static const u8 m_aBoneEnableIndices_UserAnim_Head[];
*/
	static s32 m_nBoneIndexSecondaryFire;		// Mesh's primary fire bone
	static s32 m_nBoneIndexGirder;			// Mesh's Girder bone
	static s32 m_nBoneIndexHead;			// Mesh's head bone
	static s32 m_nBoneIndexTorso;			// Mesh's torso bone
	static s32 m_nBoneIndexGroin;			// Mesh's groin bone
	static s32 m_nBoneIndexArmLower;		// used to twist glitch's right arm
	static s32 m_nBoneIndexArmElbow;		// used to bend glitch's right arm
	static s32 m_nBoneIndexL_Leg_Upper;		
	static s32 m_nBoneIndexR_Leg_Upper;		
	
	// static because it never varies per instance
	static CFVec3A	m_vGroinToGroinPivot;

	static CFSoundGroup* m_pSounds[MOZERSND_COUNT];
	CFAudioEmitter*		 m_pSoundEmitters[MOZERSND_COUNT];

	const CFVec3A *m_pGazeDir_WS;						// Points to the gaze direction vector (might not be unit in length)

	static CFTexInst m_StreamerTexInst;

	CFMtx43A	m_mtxStreamerWS;
	
	CFVec3A		m_vFirePosPrev;
	CFVec3A		m_vFireUnitDir;

	const CFVec3A		*m_pGirderUnitDir;
	CFSphere			m_GirderCollSphereWS;
	GirderSwingState_e	m_eGirderSwingState;
	FXStreamerHandle_t	m_hStreamerEmitter;
	f32					m_fUnitAimPitch;				// compute the unit aim pitch used within anim bone callbacks
	FForceHandle_t		m_hForce;						// Force feedback handle so we can kill it when we need to

	// Some Titan variables
	CFVec3A m_vJumpVelocity;								// velocity to jump with when launch anim done
	BOOL	m_bDoubleJump;									// are we doing the stomp jump?
	u32		m_uSquishedBotDuringJump;						// if we squished a bot, set this to the bot type

	static CFVec3A m_vecStompCameraLookat;
	static CFVec3A m_vecStompCameraPos;

	// Some Glitch variables
	CFAnimManFrame m_AnimManFrameRecoil;				// Anim frame used for recoil animation
	f32 m_fUnitRecoil;									// Amount glitch is currently recoiling
	f32 m_fUnitRecoilTarget;							// The recoil point glitch needs to get to
	f32 m_fFireAnimSpeedMult;							// Fire animation speed multiplier
	f32 m_fUnitFireCtlUpper;							// controls our upper body fire animation for one handed weapons
	f32 m_fUnitFireCtlLower;							// controls our lower body fire animation blend for one handed weapons
	f32 m_fFireTimer;									// timer to keep track of when we last fired.
	CFQuatA m_qRightLowerArm;							// used to aim glitch's gun
	CFQuatA m_qRightElbow;								// used to aim glitch's gun

	MeleeData_t m_MeleeData;							// holds all data for mozer's melee attack
	MeleeData_t m_MeleeData2;							// second collision sphere
	CBot *m_pBotToSquishThisFrame;

	PlayerDeathData_t m_PlayerDamageData;


//	CEntity*	m_apHitEntity[HIT_ENTITY_MAX_COUNT];		// Array of entities that the bot has hit during a slash move
//	CBot*		m_pMeleeBotToDamageThisFrame;				// 
//	u8			m_nHitEntityCount;							// Number of valid entries in m_apHitEntity
	
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	CBotMozer();
	virtual ~CBotMozer();

	static BOOL InitSystem( void );
	static void UninitSystem( void );

	BOOL Create( s32 nPlayerIndex=-1, BOOL bInstallDataPort=FALSE, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);
	virtual const CFVec3A *GetApproxEyePoint( void ) const;
	virtual void ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS );
	virtual void Die( BOOL bSpawnDeathEffects=TRUE, BOOL bSpawnGoodies=TRUE );

	FINLINE BOOL IsSwinging(void)
	{
		return ( m_eGirderSwingState != GIRDER_SWING_NONE );
	}

	// ME:  difference with this fn is that it only works on the attack parts of the swing
	FINLINE BOOL IsAttackingWithMelee( void ) { FASSERT( IsCreated() );	return (m_eGirderSwingState == GIRDER_SWING_FORWARD) || (m_eGirderSwingState == GIRDER_SWING_BACK); }
	virtual void UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup );

	virtual void CheckpointSaveSelect( s32 nCheckpoint );
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );

	f32		EstimatedSwingRangeXZ( void )
	{
		// measured a XZ swing range of up to 12.81f, therefore choosing to return 12.5
		return 12.5f;
	}



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
	virtual BOOL NotifyBotCollision( CBot *pBot );
	virtual void MeleeCollision( CEntity *pEntity, FCollImpact_t *pImpact, BOOL bFirstHit );

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL _BuildAnimStackDef( void );
	static BOOL _SwingStrikeCallback(CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	static BOOL _SwingStruckBot(CFWorldTracker *pTracker);
	static BOOL _SwingStruckMeshEntity(CFWorldTracker *pTracker);

	void _ClearDataMembers( void );

	void _HandleGirderCollision();
	//void _StartGirderSwing( void );
	void _CalcGirderMeleePos( CFVec3A *pvTmp );

	void _HandleJumping( void );
	void _HandleJumpAnimations( void );
	void _HandleDiveAnimations( void );
	void _HandleLanding( void );
	void _EnterFlyMode( void );
	void _CheckForDoubleJump( void );
	void _StartSingleJump( const CFVec3A *pJumpVelocity_WS=NULL );
	
	void _HandleWeaponFiring(void);
	void _HandleWeaponAnimations(void);
	void _HandleFiringAnimations(void);

	//void _HandleGirderAttacking( void );
	void _HandleGirderWork( void );
	void _HandleGirderAnimations( void );

	void _HandleWeaponWork( void );

	void _UpdateMatrices( void );
	static BOOL _InventoryCallback( CInventoryCallbackReason_e eReason, CInventory *pInventory, u32 nHandIndex, u32 uWpnIdx );
	static void _AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );

	void _StreamerOn( f32 fFadeOut);
	void _InflictSlashDamage( CEntity *pHitEntity, CFWorldMesh *pHitWorldMesh, const FCollImpact_t *pCollImpact );
	void _SpawnSlashImpactEffects( const FCollImpact_t *pCollImpact );
	void _PlaySnd(MozerSound_e eSnd, BOOL bPlay);

	void _AimLowerArm( CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	void _AimElbow( CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	void _InitRecoilAnim( void );

	void _Recoil( f32 fRecoilAmount );
	void _HandleRecoil( void );
	void _ResetRecoilSummer( void );

	

	friend class CBotMozerBuilder;

	FCLASS_STACKMEM_ALIGN( CBotMozer );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotMozerBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotMozerBuilder : public CBotBuilder {
public:

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CBotMozerBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CBotMozerBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif
