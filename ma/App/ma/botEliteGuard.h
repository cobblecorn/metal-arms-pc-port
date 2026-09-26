//////////////////////////////////////////////////////////////////////////////////////
// botEliteGuard.h -
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

#ifndef _BOTLEETGUARD_H_
#define _BOTLEETGUARD_H_ 1

#include "bot.h"
#include "iteminst.h"
#include "Fxstreamer.h"
#include "fdecal.h"
#include "tracer.h"

// Forward Declarations
struct FAnim_t;
class CBotAIBrain;
class CBotPartPool;
class CEProj;


FCLASS_ALIGN_PREFIX class CBotEliteGuard : public CBot
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
		STAFF_FIRE_NONE,			// no anim
		STAFF_FIRE_CHARGING,		// ARMEactn101
		STAFF_FIRE_CHARGED,			// no anim
		STAFF_FIRE_FIRING,			// ARMEactn102
		STAFF_FIRE_DONE,			// ARMEactn103
		STAFF_FIRE_STATE_COUNT,
	}StaffFireState_e;

	typedef enum  
	{
		STAFF_SWING_NONE,			// no anim
		STAFF_SWING_CHARGING,		// ARMEactn201
		STAFF_SWING_FORWARD,		// ARMEactn202
		STAFF_SWING_BACK,			// ARMEactn203
		STAFF_SWING_DONE,			// ARMEactn204
		STAFF_SWING_DONE_UNDO,		// ARMEactn204
		STAFF_SWING_STATE_COUNT,
	}StaffSwingState_e;

	typedef enum 
	{
		ELITESND_SWIPEATTACK,
		ELITESND_SWIPEIMPACT,
		ELITESND_PRIMARY_FIRE,
		ELITESND_COUNT,
	}EliteGuardSound_e;

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	enum
	{
		PART_INSTANCE_COUNT_PER_TYPE = 3,
		ANIM_IDLE_COUNT = 1						// SER: Eventually this will be retrieved from a data file
	};


	typedef enum
	{
		LIMB_TYPE_HEAD,							// The bot's head
#if FANG_WINGC
		// The retail elite guard can lose more limbs; bp_elitegrd.csv limbinfo order.
		LIMB_TYPE_LEFT_LEG,
		LIMB_TYPE_RIGHT_LEG,
		LIMB_TYPE_LEFT_ARM,
		LIMB_TYPE_RIGHT_ARM,
		LIMB_TYPE_RIBS_LEFT,
		LIMB_TYPE_RIBS_RIGHT,
#endif
		LIMB_TYPE_COUNT
	} LimbType_e;


	// Base animations:
	typedef enum
	{
		// Ground:
		ANIM_STAND,			
		ANIM_WALK,			
		ANIM_RUN,			

		ANIM_STAND_ALERT,	
		ANIM_WALK_ALERT,	
		
		ANIM_JUMP_LAUNCH,	
		ANIM_JUMP_FLY,		
		ANIM_JUMP_LAND,		

		ANIM_CABLE_REACH,	
		ANIM_CABLE_GRASP,	
		ANIM_CABLE_RELEASE,	

		ANIM_JUMP_TUCK_STAFF,
		ANIM_JUMP_TUCK,		
		ANIM_JUMP_UNTUCK,

		ANIM_DIVEROLL,

		ANIM_FIRE_CHARGE,	
		ANIM_FIRE,			
		ANIM_FIRE_DONE,		
		
		ANIM_SWING_CHARGE,	
		ANIM_SWING_FORWARD,	
		ANIM_SWING_BACK,	
		ANIM_SWING_DONE,	
		
		ANIM_RC_TETHERED,	
		ANIM_RC_POWER_DOWN,	
		ANIM_RC_POWER_UP,	

		ANIM_FIRE_STAFF,
		
		
		ANIM_BASE_COUNT

	} Anim_e;

	// Animation controls:
	typedef enum
	{
		ANIMCONTROL_STAND,					
		ANIMCONTROL_WALK,					
		ANIMCONTROL_RUN,					

		ANIMCONTROL_STAND_ALERT,					
		ANIMCONTROL_WALK_ALERT,			
		
		ANIMCONTROL_JUMP_LAUNCH,
		ANIMCONTROL_JUMP_FLY,					
		ANIMCONTROL_JUMP_LAND_UPPER,			
		ANIMCONTROL_JUMP_LAND_LOWER,			

		ANIMCONTROL_JUMP_TUCK_STAFF,			
		ANIMCONTROL_JUMP_TUCK,					
		ANIMCONTROL_JUMP_UNTUCK,				

		ANIMCONTROL_DIVEROLL,

		ANIMCONTROL_FIRE_CHARGE_UPPER,			
		ANIMCONTROL_FIRE_UPPER,					
		ANIMCONTROL_FIRE_DONE_UPPER,			
		
		ANIMCONTROL_SWING_CHARGE_UPPER,			
		ANIMCONTROL_SWING_FORWARD_UPPER,		
		ANIMCONTROL_SWING_BACK_UPPER,			
		ANIMCONTROL_SWING_DONE_UPPER,			
		
		ANIMCONTROL_FIRE_CHARGE_LOWER,			
		ANIMCONTROL_FIRE_LOWER,					
		ANIMCONTROL_FIRE_DONE_LOWER,			
		
		ANIMCONTROL_SWING_CHARGE_LOWER,			
		ANIMCONTROL_SWING_FORWARD_LOWER,		
		ANIMCONTROL_SWING_BACK_LOWER,			
		ANIMCONTROL_SWING_DONE_LOWER,			

		ANIMCONTROL_RC_TETHERED,				
		ANIMCONTROL_RC_POWER_DOWN,				
		ANIMCONTROL_RC_POWER_UP,				
		
		ANIMCONTROL_CABLE_REACH,
		ANIMCONTROL_CABLE_GRASP,
		ANIMCONTROL_CABLE_RELEASE,

		ANIMCONTROL_FIRE_STAFF,

		// without anim
		ANIMCONTROL_AIM_SUMMER,
		ANIMCONTROL_BASE_COUNT

	} AnimControl_e;


	// Animation taps:
	typedef enum
	{
		ANIMTAP_STAND,
		ANIMTAP_WALK,					
		ANIMTAP_RUN,					

		ANIMTAP_STAND_ALERT,
		ANIMTAP_WALK_ALERT,			

		ANIMTAP_JUMP_LAUNCH,
		ANIMTAP_JUMP_FLY,					
		ANIMTAP_JUMP_LAND_UPPER,			
		ANIMTAP_JUMP_LAND_LOWER,			

		ANIMTAP_JUMP_TUCK_STAFF,			
		ANIMTAP_JUMP_TUCK,					
		ANIMTAP_JUMP_UNTUCK,				

		ANIMTAP_DIVEROLL,

		ANIMTAP_FIRE_CHARGE_UPPER,			
		ANIMTAP_FIRE_UPPER,					
		ANIMTAP_FIRE_DONE_UPPER,			
		
		ANIMTAP_SWING_CHARGE_UPPER,			
		ANIMTAP_SWING_FORWARD_UPPER,		
		ANIMTAP_SWING_BACK_UPPER,			
		ANIMTAP_SWING_DONE_UPPER,			
		
		ANIMTAP_FIRE_CHARGE_LOWER,			
		ANIMTAP_FIRE_LOWER,					
		ANIMTAP_FIRE_DONE_LOWER,			
		
		ANIMTAP_SWING_CHARGE_LOWER,			
		ANIMTAP_SWING_FORWARD_LOWER,		
		ANIMTAP_SWING_BACK_LOWER,			
		ANIMTAP_SWING_DONE_LOWER,			

		ANIMTAP_RC_TETHERED,				
		ANIMTAP_RC_POWER_DOWN,				
		ANIMTAP_RC_POWER_UP,				
		
		ANIMTAP_CABLE_REACH,				
		ANIMTAP_CABLE_GRASP,				
		ANIMTAP_CABLE_RELEASE,				

		ANIMTAP_FIRE_STAFF,
		
		// without anim
		ANIMTAP_AIM_SUMMER,
		ANIMTAP_BASE_COUNT
	} AnimTap_e;


	// Bones:
	typedef enum
	{
		// Staff
		BONE_R_ARM_UPPER,
		BONE_R_ELBOW,
		BONE_R_ARM_LOWER,
		BONE_R_HAND,
		BONE_R_THUMBA,
		BONE_R_THUMBB,
		BONE_R_INDEXFINGERA,
		BONE_R_INDEXFINGERB,
		BONE_R_PINKYFINGERA,
		BONE_R_PINKYFINGERB,
		BONE_R_MIDDLEFINGERA,
		BONE_R_MIDDLEFINGERB,
		BONE_R_HAND_DUMMY,
		BONE_STAFF,
		BONE_PRIMARY_FIRE,

		// Left arm
		BONE_L_ARM_UPPER,
		BONE_L_ELBOW,
		BONE_L_ARM_LOWER,
		BONE_L_HAND,
		BONE_L_HAND_DUMMY,
		BONE_L_PINKYFINGERA,
		BONE_L_PINKYFINGERB,
		BONE_L_MIDDLEFINGERA,
		BONE_L_MIDDLEFINGERB,
		BONE_L_INDEXFINGERA,
		BONE_L_INDEXFINGERB,
		BONE_L_THUMBA,
		BONE_L_THUMBB,

		// Upper Body
		BONE_TETHER_PORT,
		BONE_WAIST1,
		BONE_WAIST2,
		BONE_WAIST3,
		BONE_TORSO,
		BONE_NECK,
		BONE_HEAD,

		// Lower Body
		BONE_GROIN,
		BONE_R_LEG_UPPER,
		BONE_R_LEG_PISTON1_TOP,
		BONE_R_LEG_PISTON2_TOP,
		BONE_R_KNEE_DUMMY,
		BONE_R_LEG_LOWER,
		BONE_R_FOOT,
		BONE_R_FOOT_BACK,
		BONE_R_HEEL,
		BONE_R_FOOT_FRONT,
		BONE_R_TOE,
		BONE_R_LEG_PISTON1_BOTTOM,
		BONE_R_LEG_PISTON3_BOTTOM,
		BONE_R_LEG_PISTON2_BOTTOM,
		BONE_R_LEG_PISTON3_TOP,
		BONE_L_LEG_UPPER,
		BONE_L_LEG_PISTON2_TOP,
		BONE_L_LEG_PISTON3_TOP,
		BONE_L_KNEE_DUMMY,
		BONE_L_LEG_LOWER,
		BONE_L_FOOT,
		BONE_L_FOOT_BACK,
		BONE_L_HEEL,
		BONE_L_FOOT_FRONT,
		BONE_L_TOE,
		BONE_L_LEG_PISTON3_BOTTOM,
		BONE_L_LEG_PISTON1_BOTTOM,
		BONE_L_LEG_PISTON2_BOTTOM,
		BONE_L_LEG_PISTON1_TOP,
		BONE_COUNT
	} Bone_e;

	typedef struct 
	{
		f32 fMinVerticalVel;
		f32 fTuckDistance;
		f32 fBoostVertical;
		f32 fBoostForward;
		
		f32 fLandingSpeedMult;
		f32 fDiveMinCosHalfAngle;

		f32 fCamEnterLookAtX;
		f32 fCamEnterLookAtY;
		f32 fCamEnterLookAtZ;
		f32 fCamEnterFollowPosX;
		f32 fCamEnterFollowPosY;
		f32 fCamEnterFollowPosZ;

		f32 fCameraOverrideEnterTime;
		f32 fCameraOverrideExitTime;

		f32 fAngleExitEnter;
		f32 fAngleExitFly;
		f32 fAngleExitTuck;
		f32 fAngleExitLand;
		f32 fAngleExitExit;

		f32 fEnterPitchVel;
		f32 fEnterBlendVel;
		f32 fEnterAnimVel;

		f32 fFlyPitchVelMin;
		f32 fFlyPitchVelMax;
		
		f32 fTuckPitchVel;
		f32 fTuckBlendVel;
		f32 fTuckAnimVel;

		f32 fLandPitchVel;

		f32 fExitPitchVel;
		f32 fExitBlendVel;
		f32 fExitAnimVel;
		f32 fTimeScale;
	} EliteInfoDive_t;

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
		FDecalDefHandle_t hDecalDef;
		FParticle_DefHandle_t hLaserParticle;
	} EliteInfoStaff_t;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:


	f32 m_fSecsUntilNextSound;


//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;								// TRUE: InitSystem() has been called
	static u32	m_nBotClassClientCount;								// Number of bots of this class using the class-wide resources
	static const CBotDef m_BotDef;									// Bot definition data
	

	// Bot parts:
	static CBotPartPool *m_pPartPool;								// One part pool per bot class


	// BotInfo data:
	static BotInfo_Gen_t m_BotInfo_Gen;					// General bot info
	static BotInfo_MountAim_t m_BotInfo_MountAim;		// Mount aim bot info
	static BotInfo_Walk_t m_BotInfo_Walk;				// Walk bot info
	static BotInfo_Jump_t m_BotInfo_Jump;				// Jump bot info

	static EliteInfoDive_t m_DefaultEliteInfoDive;
	static EliteInfoStaff_t m_DefaultEliteInfoStaff;

	static const FGameDataMap_t m_aGameDataMap[];
	static const CDamageProfile *m_pDamageProfileSlash;	// Damage profile for the slash attack

	// Bot animation stack data:
	static CBotAnimStackDef m_AnimStackDef;				// Animation stack definition data

	static cchar *m_apszBoneNameTable[BONE_COUNT];										// Array of bone names
	static cchar *m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT];					// Array of base animation control names
	static cchar *m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT];							// Array of base animation tap names
	static cchar *m_apszBaseAnimNameTable[ANIM_BASE_COUNT];								// Array of base animation names
	static cchar *m_apszIdleAnimNameTable[ANIM_IDLE_COUNT];								// Array of idle animation names
	static const u8 *m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT];
	static const u8 *m_apnEnableBoneNameIndexTableForEachIdleTap[ANIM_IDLE_COUNT];
	static const u8 m_anTagPointBoneNameIndexArray[];
	static const u8 m_nApproxEyePointBoneNameIndex;

	static CFAnimCombinerConfig::ConfigStack_t m_aAnimCombinerConfigStack[];
	static CFAnimCombinerConfig::ConfigTap_t m_aAnimCombinerConfigTaps[];
	static CFAnimCombiner::AttachList_t m_aAnimAttach[];

	static const u8 m_aBoneEnableIndices_FullBody[];
	static const u8 m_aBoneEnableIndices_UpperBody[];
	static const u8 m_aBoneEnableIndices_LowerBody[];
	static const u8 m_aBoneEnableIndices_LeftArm[];
	static const u8 m_aBoneEnableIndices_AimSummer[];
	static const u8 m_aBoneEnableIndices_FireStaff[];
	
	static const u8 m_anEnableBoneNameIndexTableForSummer_Normal[];
	static const u8 m_anEnableBoneNameIndexTableForSummer_TetherShock[];

/*
	static const u8 m_aBoneEnableIndices_UserAnim_UpperBody[];
	static const u8 m_aBoneEnableIndices_UserAnim_LowerBody[];
	static const u8 m_aBoneEnableIndices_UserAnim_UpperTorso[];
	static const u8 m_aBoneEnableIndices_UserAnim_LowerTorso[];
	static const u8 m_aBoneEnableIndices_UserAnim_LeftArm[];
	static const u8 m_aBoneEnableIndices_UserAnim_RightArm[];
	static const u8 m_aBoneEnableIndices_UserAnim_Head[];
*/
	static s32 m_nBoneIndexPrimaryFire;		// Mesh's primary fire bone
	static s32 m_nBoneIndexStaff;			// Mesh's staff bone
	static s32 m_nBoneIndexHead;			// Mesh's head bone
	static s32 m_nBoneIndexTorso;			// Mesh's torso bone
	static s32 m_nBoneIndexGroin;			// Mesh's groin bone

	EliteInfoDive_t* m_pEliteInfo_Dive;
	EliteInfoStaff_t* m_pEliteInfo_Staff;

	static CFSoundGroup* m_pSounds[ELITESND_COUNT];
	CFAudioEmitter* m_pSoundEmitters[ELITESND_COUNT];
	
	const CFVec3A *m_pGazeDir_WS;						// Points to the gaze direction vector (might not be unit in length)

	// Animation:
	static CFVec3A m_GroinVecY_WS;						// Recorded groin matrice's Y vector (used in the animation callback)
	
	// Death:
	static FMesh_t **m_papDeathChunkMeshs;				// A special set of meshes that are grunt chunks used for when the grunt dies
	static u32 m_uNumDeathChunkMeshs;					// How many are specified by name in the .cpp file
//	friend void _EliteGruntDeathDebris_StateChangeCBFunc( CDebris* pDebris, u32 uStateChange );

	static CFTexInst m_StreamerTexInst;

	CFMtx43A	m_mtxStreamerWS;
	CFVec3A		m_vFirePosPrev;
	CFVec3A		m_vFireUnitDir;
	CFVec3A		m_vPreviousPos;

	CFVec3A		m_vecDiveCameraLookat;
	CFVec3A		m_vecDiveCameraPos;

	const CFVec3A *m_pLeftHandSecondaryFireBonePos;		// Used to align the bot to the attached zip line
	const CFVec3A *m_pStaffUnitDir;
	CFSphere			m_StaffCollSphereWS;
	StaffFireState_e	m_eStaffFireState;
	StaffSwingState_e	m_eStaffSwingState;
	FXStreamerHandle_t	m_hStreamerEmitter;

	f32			m_fStaffRocketLoadTimer;
	f32			m_fDiveTargetYaw_WS;
	f32			m_fDiveYaw_WS;

	f32			m_fBeamTransTexFrame;
	f32			m_fBeamTransTexScroll;
	f32			m_fMaxLandUnitBlendLower;
	f32			m_fAttackAimTimer;
	BOOL		m_bFireLaserThisFrame;

//	CEntity*	m_apHitEntity[HIT_ENTITY_MAX_COUNT];		// Array of entities that the bot has hit during a slash move
//	u8			m_nHitEntityCount;							// Number of valid entries in m_apHitEntity
	

	MeleeData_t m_MeleeData;
	MeleeData_t m_MeleeData2;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	CBotEliteGuard();
	virtual ~CBotEliteGuard();

	static BOOL InitSystem( void );
	static void UninitSystem( void );

	BOOL Create( s32 nPlayerIndex=-1, BOOL bInstallDataPort=FALSE, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);
	virtual const CFVec3A *GetApproxEyePoint( void ) const;
	virtual void ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS );
	virtual void Possess( s32 nPlayerIndex, f32 fPossessionArmorModifier );
	//virtual BOOL NotifyBotCollision( CBot *pBot );
	virtual void Die( BOOL bSpawnDeathEffects=TRUE, BOOL bSpawnGoodies=TRUE );
	virtual BOOL CanAttachToZipLine( void );

	BOOL GrabCable( CEZipLine *pCable );
	void ReleaseCable( void );
	BOOL IsDiveJumping(void);
	BOOL IsSwinging(void);
	const CFVec3A& GetDiveUnitDir(void);
	virtual void UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup );

	virtual void CheckpointSaveSelect( s32 nCheckpoint );

	f32		EstimatedDiveDistance( void );
	f32		EstimatedSwingRangeXZ( void )
	{
		// measured a XZ swing range of up to 12.81f, therefore choosing to return 12.5
		return 12.5f;
	}

	f32 GetApproxPrimaryWeaponSpeed(void);

	FINLINE void StopAiming( void ) { FASSERT( IsCreated() ); FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM ); m_fAttackAimTimer = 0.0f; }
	

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

	BOOL _FirePrimaryLaser( const CFVec3A& TargetedPoint_WS  );
	BOOL _FireRocket( void );
	void _FireBeamThisFrame( const CFVec3A & TargetPt_WS );
	static void _BuildRocketSkipList( CEProj *pProj );
	void _InitProjectile( CEProj *pProj, const CFVec3A *pPos_WS, const CFVec3A *pUnitDir_WS, BOOL bNoDynamicLight );

	virtual void MeleeCollision( CEntity *pEntity, FCollImpact_t *pImpact, BOOL bFirstImpact );


//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL _BuildAnimStackDef( void );
	static BOOL _SwingStrikeCallback(CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	static BOOL _SwingStruckBot(CFWorldTracker *pTracker);
	static BOOL _SwingStruckMeshEntity(CFWorldTracker *pTracker);
	void _HandleStaffCollision();


	void _ClearDataMembers( void );
	void _HandleCableAndDoubleJump( void );
	void _HandleCableTranslation( void );
	void _HandleCableAnimations( void );

	void _HandleJumping( void );
	void _HandleHopRollStartle(void);
	void _HandleJumpAnimations( void );
	void _HandleDiveAnimations( void );
	void _EnterFlyMode( void );
	void _StartVelocityJump( const CFVec3A *pJumpVelocity_WS );
	void _StartSingleJump( const CFVec3A *pJumpVelocity_WS=NULL );
	void _StartDoubleJump( BOOL bAddToVelocityY=TRUE );
	void _StartDive( f32 fDiveYaw_WS );

	void _JumpLanded( void );
	void _HandleTerminalState(void);
	
	void _HandleStaffWork( void );
	void _HandleStaffAnimations( void );

	void _HandleFiringAnimations( void );
	void _HandleAimAnimations( void );
	void _HandleWeaponWork( void );

	void _UpdateMatrices( void );
	static BOOL _InventoryCallback( CInventoryCallbackReason_e eReason, CInventory *pInventory, u32 nHandIndex, u32 uWpnIdx );
	static void _AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );

	void _StreamerOn( f32 fFadeOut);
	void _InflictSlashDamage( CEntity *pHitEntity, CFWorldMesh *pHitWorldMesh, const FCollImpact_t *pCollImpact );
	void _SpawnSlashImpactEffects( const FCollImpact_t *pCollImpact );
	void _PlaySnd(EliteGuardSound_e eSnd, BOOL bPlay);
	

	static void _EGTracerKilledCallbackPrimary( TracerDef_t *pTracerDef, TracerKillReason_e nKillReason, const FCollImpact_t *pImpact );
	void _CalcMeleeAttackPos( u32 i, CFVec3A *pvMeleeAttackPos );
	BOOL _IsFingerOnTrigger( void );


	friend class CBotEliteGuardBuilder;

	FCLASS_STACKMEM_ALIGN( CBotEliteGuard );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotEliteGuardBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotEliteGuardBuilder : public CBotBuilder {
public:

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CBotEliteGuardBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );


//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CBotEliteGuardBuilder );
} FCLASS_ALIGN_SUFFIX;


void BotEliteGuard_DrawBeams(void);
void BotEliteGuard_InitLevel(void);

#endif
