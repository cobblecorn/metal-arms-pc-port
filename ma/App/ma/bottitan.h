//////////////////////////////////////////////////////////////////////////////////////
// bottitan.h - 
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
// 10/14/02 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _BOTTITAN_H_
#define _BOTTITAN_H_ 1



#include "bot.h"
#include "iteminst.h"
#include "EProj.h"
#include "fparticle.h"
#include "fforce.h"
#include "fexplosion.h"
#include "eproj.h"


// Forward Declarations
struct FAnim_t;
class  CBotAIBrain;
class  CMeshEntity;
class  CBotPartPool;



FCLASS_ALIGN_PREFIX class CBotTitan : public CBot {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum
	{
		TITANFLAG_NONE = 0x00000000
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
		ANIM_IDLE_COUNT		= 1,						// SER: Eventually this will be retrieved from a data file
		ROCKET_NUM_CHAMBERS	= 3
	};


	typedef enum {
		LIMB_TYPE_HEAD,							// The bot's head
		LIMB_TYPE_LEFT_ARM,						// The bot's left arm
		LIMB_TYPE_RIGHT_ARM,					// The bot's right arm
		LIMB_TYPE_ROCKET_LAUNCHER,				// The bot's rocket launcher
		LIMB_TYPE_SPOTLIGHT,					// The bot's spotlight
		LIMB_TYPE_RIBS1,						// A few of the bot's ribs
		LIMB_TYPE_RIBS2,						// A few of the bot's ribs
		LIMB_TYPE_LEFT_LEG,						// The bot's left leg
		LIMB_TYPE_RIGHT_LEG,					// The bot's right leg

		LIMB_TYPE_COUNT
	} LimbType_e;


	// Animation controls:
	typedef enum {
		ANIMCONTROL_STAND,
		ANIMCONTROL_STAND_STAMP,
		ANIMCONTROL_STAND_LOOK,
		ANIMCONTROL_WALK,
		ANIMCONTROL_RUN,

		ANIMCONTROL_AIM_SUMMER,

		ANIMCONTROL_JUMP_LAUNCH,
		ANIMCONTROL_JUMP_FLY,
		ANIMCONTROL_JUMP_LAND_UPPER,
		ANIMCONTROL_JUMP_LAND_LOWER,

		ANIMCONTROL_JUMP2_FLY,
		ANIMCONTROL_JUMP2_LAND,

		ANIMCONTROL_FIRE_1,
		ANIMCONTROL_FIRE_2,

		ANIMCONTROL_RC_TETHERED,
		ANIMCONTROL_RC_POWER_DOWN,
		ANIMCONTROL_RC_POWER_UP,

		ANIMCONTROL_STOOP_SUMMER,
		
		ANIMCONTROL_STAND_LIMP_LEFT,
		ANIMCONTROL_STAND_LIMP_RIGHT,
		ANIMCONTROL_LIMP_LEFT,
		ANIMCONTROL_LIMP_RIGHT,

		ANIMCONTROL_BASE_COUNT
	} AnimControl_e;


	// Animation taps:
	typedef enum {
		ANIMTAP_STAND,
		ANIMTAP_STAND_STAMP,
		ANIMTAP_STAND_LOOK,
		ANIMTAP_WALK,
		ANIMTAP_RUN,

		ANIMTAP_AIM_SUMMER,

		ANIMTAP_JUMP_LAUNCH,
		ANIMTAP_JUMP_FLY,
		ANIMTAP_JUMP_LAND_LOWER,
		ANIMTAP_JUMP_LAND_UPPER,
		
		ANIMTAP_JUMP2_FLY,
		ANIMTAP_JUMP2_LAND,

		ANIMTAP_FIRE_1,
		ANIMTAP_FIRE_2,

		ANIMTAP_RC_TETHERED,
		ANIMTAP_RC_POWER_DOWN,
		ANIMTAP_RC_POWER_UP,

		ANIMTAP_STOOP_SUMMER,

		ANIMTAP_STAND_LIMP_LEFT,
		ANIMTAP_STAND_LIMP_RIGHT,
		ANIMTAP_LIMP_LEFT,
		ANIMTAP_LIMP_RIGHT,

		ANIMTAP_BASE_COUNT
	} AnimTap_e;


	// Base animations:
	typedef enum {
		ANIM_STAND,
		ANIM_STAND_STAMP,
		ANIM_STAND_LOOK,
		ANIM_WALK,
		ANIM_RUN,

		ANIM_JUMP_LAUNCH,
		ANIM_JUMP_FLY,
		ANIM_JUMP_LAND,
		
		ANIM_JUMP2_FLY,
		ANIM_JUMP2_LAND,

		ANIM_FIRE_1,
		ANIM_FIRE_2,

		ANIM_RC_TETHERED,
		ANIM_RC_POWER_DOWN,
		ANIM_RC_POWER_UP,

		ANIM_STAND_LIMP_LEFT,
		ANIM_STAND_LIMP_RIGHT,
		ANIM_LIMP_LEFT,
		ANIM_LIMP_RIGHT,

		ANIM_BASE_COUNT

	} Anim_e;


	// Bones:
	typedef enum {
		BONE_GROIN,

		BONE_L_LEG_UPPER,
		BONE_L_LEG_LOWER,
		BONE_L_FOOT_BACK,
		BONE_L_HEEL,
		BONE_L_FOOT_FRONT,
		BONE_L_TOE,

		BONE_R_LEG_UPPER,
		BONE_R_LEG_LOWER,
		BONE_R_FOOT_BACK,
		BONE_R_HEEL,
		BONE_R_FOOT_FRONT,
		BONE_R_TOE,

		BONE_TORSO,

		BONE_R_RIB_MIDDLE,
		BONE_R_RIB_BOTTOM,
		BONE_R_RIB_TOP,

		BONE_HEAD,
		BONE_REACTOR,

		BONE_ROCKETLAUNCHER,
		BONE_ROCKETCARTRIDGE,
		BONE_SECONDARY_FIRE,
		
		BONE_R_ARM_UPPER,
		BONE_R_ELBOW,
		BONE_R_ARM_LOWER,

		BONE_L_ARM_UPPER,
		BONE_L_ELBOW,
		BONE_L_ARM_LOWER,

		BONE_L_RIB_MIDDLE,
		BONE_L_RIB_TOP,
		BONE_L_RIB_BOTTOM,

		BONE_TETHER_PORT,
		BONE_SPOTLIGHT,
		BONE_ATTACHPOINT_LIGHT,

		BONE_COUNT
	} Bone_e;


	typedef struct {
		CDamageProfile *pPCRocketDamageProfile;	// Rocket damage profile string (references a damage profile in damage.csv)
		CDamageProfile *pNPCRocketDamageProfile;// Rocket damage profile string (references a damage profile in damage.csv)
		CDamageProfile *pFootstepDamageProfile;	// Footstep damage profile string (references a damage profile in damage.csv)
		CDamageProfile *pStompDamageProfile;	// Stomp damage profile
		cchar *pszParticleSparks;				// particle effect to give some sparks to a stomped bot
		cchar *pszParticleFootDust;				// particle effect for footstep dust
		cchar *pszSpotlightMesh;
		
		f32	fUnitTopSpeedWhileFiring;			// Top speed while the bot is firing guns or stooping
		f32 fStompGravityMultiplier;			// Gravity multiplier to pull bot down while stomping
		
		
		//ROCKET PARAMS
		cchar *pszRocketMesh;
		cchar *pszRocketSmokeTex;
		cchar *pszRocketExplosionImpact;
		cchar *pszRocketExplosionFire;
		u32	nRocketsInPool;
		f32	fOORocketReloadTime;
		f32 fRocketRippleDelay;					// time to wait between firing multiple rockets
		f32 fRocketFullReloadTime;				// time it takes to reload after you fire multiple rockets
		f32 fRocketRange;
		f32	fRocketSpeed;
		f32	fRocketAimTime;						// time it takes to orient the rl at the target
		f32 fOORocketAimTime;
		f32 fRocketAimOffTime;					// time it takes to revert after firing
		f32 fOORocketAimOffTime;

		f32 fShieldImpactSoundVolume;
		f32 fShieldImpactSoundInterval;

		f32 fLightInactivityTime;				// max time the titan can be inactive before the light shuts down

		// shield params
		f32 fShieldRechargeTime;				// time it takes to charge the shield
		f32 fShieldRechargeDelay;				// time to wait after taking damage before recharging
		f32	fShieldScale;						
		cchar *pszShieldArmorProfile;

		f32	fMinPowerCycle;						// slowest speed the reactor cycles (cycles per second)
		f32 fMaxPowerCycle;						// fastest speed
		f32 fReactorSoundLevel;
		f32	fSoundRadius;						// radius for titan sounds
		f32 fUnitInPossessionCamShake;			// amount cam shakes w/ each footstep when player is in possession of the titan
		f32 fMaxShockwaveImpulse;				// maximum velocity impulse to give affected bots
		f32 fMinShockwaveImpulse;				// minimum velocity impulse to give affected bots

		cchar *pszDebrisPileMeshExplosion;		// the explosion to use when the debris mesh is destroyed

		cchar *pszCoronaTexName;				// Corona texture
		f32 fCoronaScale;						// Corona scale
		f32 fCoronaUnitScreenspaceScale;		// 0=worldspace scale is constant, 1=screenspace scale is constant
		f32 fCoronaColorRed;					// Corona color red (also used for dynamic light)
		f32 fCoronaColorGreen;					// Corona color green (also used for dynamic light)
		f32 fCoronaColorBlue;					// Corona color blue (also used for dynamic light)

		f32 fLightRadius;						// Dynamic light radius

		f32 fProjectileMeshScale;				// Scale to use for the projectile itself

		CEProjExt::CEProj_Merv_StaticParams_t MervStaticParams;
		CFSoundGroup *pSndGrpRocketLauncher;	// Rocket launcher sound
		CFSoundGroup *pSndGrpRocketLoop;		// Rocket in-air looping sound


	} BotInfo_Titan_t;

	typedef struct {
		// sound handles
		FSndFx_FxHandle_t	hSndRLReady;
		FSndFx_FxHandle_t	hSndRLFire;
		FSndFx_FxHandle_t	hSndStomp;
		FSndFx_FxHandle_t	hSndReactor;
		FSndFx_FxHandle_t	hSndShieldHit;
		FSndFx_FxHandle_t	hSndShieldUp;
		FSndFx_FxHandle_t	hSndShieldDown;
		FSndFx_FxHandle_t	hSndPowerUp;
		FSndFx_FxHandle_t	hSndPowerDown;
		FSndFx_FxHandle_t	hSndServo;
	} BotInfo_TitanSounds_t;


//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:


//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL	 m_bSystemInitialized;					// TRUE: InitSystem() has been called
	static u32 m_nBotClassClientCount;					// Number of bots of this class using the class-wide resources
	static const CBotDef m_BotDef;						// Bot definition data


	// Bot parts:
	static CBotPartPool *m_pPartPool;								// One part pool per bot class
	static FMesh_t		*m_pTitanMesh;


	// BotInfo data:
	static BotInfo_Gen_t			m_BotInfo_Gen;			// General bot info
	static BotInfo_MountAim_t		m_BotInfo_MountAim;		// Mount aim bot info
	static BotInfo_Walk_t			m_BotInfo_Walk;			// Walk bot info
	static BotInfo_Jump_t			m_BotInfo_Jump;			// Jump bot info
	static BotInfo_Weapon_t			m_BotInfo_Weapon;		// Weapon bot info
	static BotInfo_Titan_t			m_BotInfo_Titan;		// Titan bot info
	static BotInfo_TitanSounds_t	m_BotInfo_TitanSounds;	// Titan sfx

	static CBotTitan*				m_pCBTitan;			// ptr to this titan used for callbacks

	// Bot animation stack data:
	static CBotAnimStackDef m_AnimStackDef;				// Animation stack definition data

	static const FGameDataMap_t m_aGameDataMap[];
	static const FGameDataMap_t m_aSoundDataMap[];
	static const FGameData_TableEntry_t m_aBotInfoVocab_Titan[];
	static const FGameData_TableEntry_t m_aBotInfoVocab_TitanSounds[];

	//static CDamageProfile *m_pRocketDamageProfile;
	//static CDamageProfile *m_pFootstepDamageProfile;
	
	static FExplosion_GroupHandle_t	m_hDebrisExplosion;
	static FExplosion_GroupHandle_t	m_hRocketExplosionImpact;
	static FExplosion_GroupHandle_t	m_hRocketExplosionFire;

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
	static const u8 m_anEnableBonesFire[];
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
	static const u8 m_aBoneEnableIndices_AimSummer[];


	static CFAnimCombinerConfig::ConfigStack_t	m_aAnimCombinerConfigStack[];
	static CFAnimCombinerConfig::ConfigTap_t	m_aAnimCombinerConfigTaps[];
	static CFAnimCombiner::AttachList_t			m_aAnimAttach[];

	static BOOL m_bSoundsInitialized;


	// Animation:
	s32		m_nBoneIndexGroin;								// Mesh's groin bone index
	s32		m_nBoneIndexTorso;								// Mesh's torso bone index
	u32     m_nBoneIndexRArm;								// Mesh's right arm bone index
	u32		m_nBoneIndexRElbow;								// Mesh's right elbow bone index
	u32		m_nBoneIndexLArm;	
	u32		m_nBoneIndexLElbow;							
	u32		m_nBoneIndexRocketLauncher;
	u32		m_nBoneIndexRocketFire;							// where the rockets come from
	u32		m_nBoneIndexLight;
	u32		m_nBoneIndexRightToe;							// these are used to spawn dust on footsteps
	u32		m_nBoneIndexRightHeel;
	u32		m_nBoneIndexLeftToe;
	u32		m_nBoneIndexLeftHeel;
	u32		m_nBoneIndexReactor;							// The reactor's bone, used to place the reactor sound

	CFAnimManMtx  m_AnimManMtxStoop;						// Man anim for stoop
	static cchar *m_papszStoopSummerBones[];

	const	CFVec3A *m_pGazeDir_WS;							// Points to the gaze direction vector (might not be unit in length)

	static CFVec3A m_GroinVecY_WS;							// Recorded groin matrice's Y vector (used in the animation callback)


	BOOL	m_bFiringGuns;									// firing chain guns?
	f32		m_fLastAimAngle_LElbow;							// used to smooth out the arm animations for aiming
	f32		m_fLastAimAngle_RElbow;							// used to smooth out the arm animations for aiming
	f32		m_fLastAimAngle_LArm;							// used to smooth out the arm animations for aiming
	f32		m_fLastAimAngle_RArm;							// used to smooth out the arm animations for aiming
    
	CFVec3A m_vJumpVelocity;								// velocity to jump with when launch anim done
	BOOL	m_bDoubleJump;									// are we doing the stomp jump?
	
	f32			   m_fStompCameraTransition;
	static CFVec3A m_vecStompCameraLookat;
	static CFVec3A m_vecStompCameraPos;
	
	f32			  m_fShieldSoundTimer;						// only play so many shield effect sounds a second

	f32			  m_fLightTimer;							// when this hits 0, shut off the spotlight, reset it when the titan moves, ignore if human controlled.
	CMeshEntity	 *m_pLight;
	BOOL		  m_bLightEnabled;							// should our light be on or off?

	CFVec3A		  m_vChaingunMidpoint_WS;					// midpoint of our chainguns, used so we only have to play one sound & draw one muzzle light

	//data for rocket launcher
	static const u32 m_auRocketTexLayerIDs[];
	
	//FMeshTexLayerHandle_t m_ahRocketTexLayers[ROCKET_NUM_CHAMBERS];			// array of rocket lights
	FMeshTexLayerHandle_t m_hRocketTexLayer;

	static SmokeTrailAttrib_t	m_SmokeTrailAttrib;							// attributes for our rocket's smoke trail

	f32		m_fRocketLoadTimer;
	BOOL	m_bRocketTriggerDown;
	u32		m_uRocketsToFire;												// these are rockets that are currently firing
	f32		m_fRocketFireTimer;												// used to handle firing off multiple rockets quickly
	f32		m_fRocketAimTimer;												// timer to control aiming of the rocket launcher 1.0 = not aiming 0.0 is full aiming, can go negative
	BOOL	m_bFireRocketThisFrame;											// used by the AI to override rocket logic
	f32		m_fRocketReaction;												// used to make the titan recoil a little bit from firing
	f32		m_fRocketReactionTarget;										// how much do we want to react?
	BOOL	m_bRocketReactionOn;											// are we reacting?  or moving back to normal?
	CFVec3A m_vFiredRocketAtPoint;											// where we were aiming when we requested rocket firing
	
	CFVec3A	m_vLastValidTargetPt_WS;										// last valid tgt point
	f32		m_fUnitAim;														// 0.0 = not aiming, 1.0 = arms 
	

	//particle stuff
	//static FParticle_DefHandle_t	m_hSparksPartDef;
	static CFTexInst m_CoronaTexInst;

	// power plant data
	f32 m_fPowerDraw;										// how much power the titan's systems are drawing
	f32 m_fPowerPlantRecRate;								// how fast the plant is recovering
	f32 m_fPowerPlantUnitCycle;								// where we are in the cycle
    
	CFAudioEmitter			*m_pReactorEmitter;				// audio emitter for power plant sound (loooping)	
	FMeshTexLayerHandle_t	 m_hPlantTexLayer;				// handle to our power plant tex layer
	FMeshTexLayerHandle_t	 m_hLightTexLayer;				// handle to our light tex layer

	f32 m_fShieldHealth;	// tracked for display on the HUD
	f32 m_fChaingunHeat;	// tracked for display on the HUD
	BOOL m_bShieldSuspended;

	CBot *m_pBotToSquishThisFrame;

	


	// ffeedback
	FForceHandle_t m_hForce;

	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
public:
	CBotTitan();
	~CBotTitan();

	static BOOL InitSystem( void );
	static void UninitSystem( void );

	virtual const CFVec3A *GetApproxEyePoint( void ) const;

	BOOL	Create( s32 nPlayerIndex=-1, BOOL bInstallDataPort=FALSE, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL, BOOL bInstallShield=FALSE );

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);			

	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );

	virtual void CheckpointSaveSelect( s32 nCheckpoint );

	//AI Weapon controls
	f32	 GetRocketTravelSpeed( void );
	FINLINE void FireRocketNow( void )		{ m_bFireRocketThisFrame = TRUE; };

	void SetGunsHeatupPerSecond( f32 fVal );
	void SetGunsCooldownPerSecond( f32 fVal );
	void SetGunsCooldownOverheatPenalty( f32 fVal );

	FINLINE BOOL IsStomping( void ) const { return m_bDoubleJump;}
	virtual void InflictDamage( CDamageData *pDamageData );

	virtual void NotifyShieldStatus( u32 uStatus, const CFVec3A &vPos );

	static void DebugDraw(  CBotTitan *pTitan  );

	virtual FINLINE void ComputeMtxPalette( BOOL bApplyOffscreenOptimizations ) { FASSERT( IsCreated() ); FASSERT( m_Anim.m_pAnimCombiner != NULL ); m_pCBTitan = this; m_Anim.m_pAnimCombiner->ComputeMtxPalette(bApplyOffscreenOptimizations); m_pCBTitan = NULL; };

	void SuspendShield( BOOL bSuspend );		// will cause the shield to stop having any effect as long as it's suspended.  Visually the shield doesn't change

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
	virtual void DeathWork( void );
	virtual void Power( BOOL bPowerUp, f32 fPowerOffTime=0.0f, f32 fPowerOffOnSpeedMult=1.0f );
	virtual void Possess( s32 nPlayerIndex, f32 fPossessionArmorModifier );
	virtual void NotifyFootDown( BOOL bLeftFoot, BOOL bRightFoot, f32 fUnitMag );
	virtual void HandleTargeting( void );

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static	BOOL _BuildAnimStackDef( void );

	void _ClearDataMembers( void );
	void _UpdateMatrices( void );

	void _CheckForDoubleJump( void );
	void _StartSingleJump( const CFVec3A *pJumpVelocity_WS=NULL );

	void _PositionDebrisPile( void );
	void _DestroyDebrisPile( void );

	//handle animations
	void _HandleJumpAnimations( void );
	void _HandleJumping( void );
	void _HandleLanding( void );
	void _EnterFlyMode( void );

	void _HandleWeaponFiring( void );
	void _HandleWeaponWork( void );

	void _HandleFiringAnimations( void );
	void _HandleAimingAnimations( void );

	static void _HandleArmAim( CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx, const CFVec3A &rAimVec, f32 fSlerp, f32 *pfLastAngle );
	static void _HandleElbowAim( CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx, const CFVec3A &rAimVec, f32 fSlerp, f32 *pfLastAngle );

	void _HandleRocketLauncherAim( CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );

	void _HandleRocketWork( void );			// handle reloading, lights, etc
	void _FireRocket( void );				// fire a rocket at current aimpoint
	void _InitProjectile( CEProj *pProj, const CFVec3A *pPos_WS, const CFVec3A *pUnitDir_WS, BOOL bNoDynamicLight );

	static void _BuildRocketSkipList( CEProj *pProj );
//	static BOOL _RocketDetonateCallback( CEProj *pProj, CEProj::Event_e nEvent, const FCollImpact_t *pImpact );

	void UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup );

	void _EnableLight( BOOL bEnable );

	void _HandlePowerPlant( void );

	void _CheckServoSounds( void );

	static void _AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );

	BOOL _InitStoopAnimation( void );

	friend class CBotTitanBuilder;

FCLASS_STACKMEM_ALIGN( CBotTitan );
} FCLASS_ALIGN_SUFFIX;


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotTitanBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotTitanBuilder : public CBotBuilder {
public:
	BOOL m_bEnableLight;
	BOOL m_bEnableShield;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CBotTitanBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CBotTitanBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif

