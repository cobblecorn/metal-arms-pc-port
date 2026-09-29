//////////////////////////////////////////////////////////////////////////////////////
// botsniper.h - Sniper
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
// 03/11/03 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////


#ifndef _BOTSNIPER_H_
#define _BOTSNIPER_H_ 1


#include "fang.h"
#include "bot.h"
#include "fforce.h"
#include "tracer.h"

// Forward Declarations
struct FAnim_t;

class CBotAIBrain;
class CBotPartPool;
class CWeaponScope;
class CGrapple;

#define _BULLETTRAIL_NODES			( 10 )
//todo:  move into class
	typedef struct {
		TracerDef_t *pTracer;
		u64 uTimeCreated;
		f32 fUnitLifespan;
		CFVec3A vStartPoint;
		CFVec3A vEndPoint;
		f32 afTopVelocities[_BULLETTRAIL_NODES];
		f32 afBottomVelocities[_BULLETTRAIL_NODES];
	} BulletTrail_t;




FCLASS_ALIGN_PREFIX class CBotSniper : public CBot {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:


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
		MAX_LASER_SIGHTS				= 5
	};


	//typedef enum {
	//	LIMB_TYPE_HEAD,							// The bot's head
	//	LIMB_TYPE_COUNT
	//} LimbType_e;


	// Animation controls:
	typedef enum {
		ANIMCONTROL_STAND,
		ANIMCONTROL_WALK,
		ANIMCONTROL_RUN,

		ANIMCONTROL_JUMP_LAUNCH,
		ANIMCONTROL_JUMP_FLY,
		ANIMCONTROL_JUMP_LAND_UPPER,
		ANIMCONTROL_JUMP_LAND_LOWER,
		ANIMCONTROL_JUMP_LAND_RECOVER,

		ANIMCONTROL_GRAPPLE_FLY,
		ANIMCONTROL_GRAPPLE,
		ANIMCONTROL_GRAPPLE_HANG,
//		ANIMCONTROL_HANG_CEILING,
//		ANIMCONTROL_HANG_WALL,
		ANIMCONTROL_GRAPPLE_LOWER,

		ANIMCONTROL_SUMMER,

		ANIMCONTROL_FIRE_FAST,
		ANIMCONTROL_FIRE_START,
		ANIMCONTROL_FIRE,
		ANIMCONTROL_FIRE_END,

		ANIMCONTROL_FIRE_FAST_LOWER,
		ANIMCONTROL_FIRE_START_LOWER,
		ANIMCONTROL_FIRE_LOWER,
		ANIMCONTROL_FIRE_END_LOWER,

		ANIMCONTROL_RC_TETHERED,
		ANIMCONTROL_RC_POWER_DOWN,
		ANIMCONTROL_RC_POWER_UP,

		ANIMCONTROL_BASE_COUNT
	} AnimControl_e;


	// Animation taps:
	typedef enum {
		ANIMTAP_STAND,
		ANIMTAP_WALK,
		ANIMTAP_RUN,

		ANIMTAP_JUMP_LAUNCH,
		ANIMTAP_JUMP_FLY,
		ANIMTAP_JUMP_LAND_UPPER,
		ANIMTAP_JUMP_LAND_LOWER,
		ANIMTAP_JUMP_LAND_RECOVER,

		ANIMTAP_GRAPPLE_FLY,
		ANIMTAP_GRAPPLE,
		ANIMTAP_GRAPPLE_HANG,
//		ANIMTAP_HANG_CEILING,
//		ANIMTAP_HANG_WALL,
		ANIMTAP_GRAPPLE_LOWER,

		ANIMTAP_SUMMER,

		ANIMTAP_FIRE_FAST,
		ANIMTAP_FIRE_START,
		ANIMTAP_FIRE,
		ANIMTAP_FIRE_END,

		ANIMTAP_FIRE_FAST_LOWER,
		ANIMTAP_FIRE_START_LOWER,
		ANIMTAP_FIRE_LOWER,
		ANIMTAP_FIRE_END_LOWER,

		ANIMTAP_RC_TETHERED,
		ANIMTAP_RC_POWER_DOWN,
		ANIMTAP_RC_POWER_UP,

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
		ANIM_JUMP_RECOVER,

		ANIM_GRAPPLE_FLY,
		ANIM_GRAPPLE,
		ANIM_HANG_CEILING,
		ANIM_HANG_WALL,

		ANIM_FIRE_FAST,
		ANIM_FIRE_START,
		ANIM_FIRE,
		ANIM_FIRE_END,

		ANIM_RC_TETHERED,
		ANIM_RC_POWER_DOWN,
		ANIM_RC_POWER_UP,

		ANIM_BASE_COUNT
	} Anim_e;


	// Bones:
	typedef enum {
		BONE_DUMMY,				// "SniperDummy",
		BONE_GROIN,				// "Groin",
		BONE_SPINE,				// "Spine",
		BONE_TORSO,				// "Torso",
		BONE_NECK,				// "Neck",
		BONE_HEAD,				// "Head",
		BONE_RIB1,				// "Rib1",
		BONE_RIB2,				// "Rib2",
		BONE_RIB3,				// "Rib3",
		BONE_RIB4,				// "Rib4",

		BONE_LEG_UPPER_R,		// "R_Leg_Upper",
		BONE_LEG_LOWER_R,		// "R_Leg_Lower",
		BONE_ANKLE_R,			// "R_Ankle",
		BONE_FOOT_R,			// "R_Foot",
		BONE_LEG_PISTONB_R,		// "R_Leg_PistonB",
		BONE_LEG_PISTONA_R,		// "R_Leg_PistonA",

		BONE_LEG_UPPER_L,		// "L_Leg_Upper",
		BONE_LEG_LOWER_L,		// "L_Leg_Lower",
		BONE_ANKLE_L,			// "L_Ankle",
		BONE_FOOT_L,			// "L_Foot",
		BONE_LEG_PISTONB_L,		// "L_Leg_PistonB",
		BONE_LEG_PISTONA_L,		// "L_Leg_PistonA",

		BONE_ARM_UPPER_L,		// "L_Arm_Upper",
		BONE_ARM_LOWER_L,		// "L_Arm_Lower",
		BONE_ARM_PISTONB_L,		// "L_Arm_PistonB",
		BONE_ARM_PISTONA_L,		// "L_Arm_PistonA",

		BONE_ARM_UPPER_R,		// "R_Arm_Upper",
		BONE_ARM_LOWER_R,		// "R_Arm_Lower",
		BONE_GUN_BARREL,		// "Gun_Barrel",
		BONE_ARM_PISTONB_R,		// "R_Arm_PistonB",
		BONE_ARM_PISTONA_R,		// "R_Arm_PistonA",

		//Non animated bones:
		BONE_PRIMARY_FIRE,		// "Primary_Fire"
		BONE_GRAPPLE_ATTACH,	// "AttachPoint_Grapple"

		BONE_COUNT
	} Bone_e;


	typedef enum {
		ANIMBONE_ARM_UPPER_L,
		ANIMBONE_ARM_LOWER_L,
		ANIMBONE_ARM_UPPER_R,
		ANIMBONE_HEAD,
		ANIMBONE_TORSO,
		ANIMBONE_RIB1,
		ANIMBONE_RIB2,
		ANIMBONE_RIB3,
		ANIMBONE_RIB4,

		ANIMBONE_COUNT
	};


	typedef struct {
		f32 fTracerWidth;
		f32 fTracerLength;
		f32 fTracerSpeed;
		u32 uMaxTracers;
		f32 fReloadTime;

		f32 fWallClingPosX;
		f32 fWallClingPosY;
		f32 fWallClingPosZ;

	} BotInfo_Sniper_t;

	typedef enum {
		SNIPESTATE_NONE,
		SNIPESTATE_BEGINNING,
		SNIPESTATE_SNIPING,
		SNIPESTATE_FIRING,
		SNIPESTATE_RELOADING,
		SNIPESTATE_ENDING,
	} SnipeState_e;

	typedef enum {
		GRAPPLESTATE_NONE,
		GRAPPLESTATE_AIMING,
		GRAPPLESTATE_READYTOFIRE,
		GRAPPLESTATE_FIRING,
		GRAPPLESTATE_LAUNCHING,
		GRAPPLESTATE_FLYING,
		GRAPPLESTATE_LANDING,
		GRAPPLESTATE_CLINGING,
		GRAPPLESTATE_RELEASING,
		GRAPPLESTATE_RELOADING,

	} GrappleState_e;

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
	static BotInfo_Sniper_t			m_BotInfo_Sniper;					// Sniper bot info

	static CBotSniper*				m_pCBSniper;						// ptr to this used for callbacks

	// Bot animation stack data:
	static CBotAnimStackDef m_AnimStackDef;								// Animation stack definition data

	static const FGameDataMap_t m_aGameDataMap[];
	//static const FGameData_TableEntry_t m_aBotInfoVocab_Sniper[];
	
	static cchar *m_apszBoneNameTable[BONE_COUNT];						// Array of bone names
	static cchar *m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT];	// Array of base animation control names
	static cchar *m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT];			// Array of base animation tap names
	static cchar *m_apszIdleAnimNameTable[ANIM_IDLE_COUNT];				// Array of idle animation names
	static cchar *m_apszBaseAnimNameTable[ANIM_BASE_COUNT];				// Array of base animation names
	static cchar *m_apszSummerBoneNameTable[ANIMBONE_COUNT];			// Array of bone names in summer


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
	static const u8 m_aBoneEnableIndices_Summer[];

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
	static	CFVec3A m_vGrappleClingPosWall_MS;					// where the bot clings to the wall

	static TracerGroupHandle_t		m_hTracerGroup;				// tracer group
	static TracerDef_t				m_TracerDef;				// Tracer definition for guns
#if FANG_WINGC
	static CDamageProfile			*m_pPortShotDamageNPC;		// PC port: retail has no Sniper shot profile (RivetL2)
	static CDamageProfile			*m_pPortShotDamagePlayer;	// ... and when a player controls the Sniper (RivetL3)
#endif
	static CFTexInst				m_TracerTexInst;			// tracer texture
	static BulletTrail_t			*m_paBulletTrails;			// bullet trails
	static u32						m_uNumActiveBulletTrails;
	static CFTexInst				m_BulletTrailTex;

	// laser sight
	// this list will get built up in the work phase, and cleared in the draw phase
	static CFVec3A m_avLaserPt[MAX_LASER_SIGHTS*4];
	static u32 m_uNumSightsActive;									
	static CFTexInst m_LaserSightTex;


	// bones
	s32 m_nBoneIdxGroin;
	s32 m_nBoneIdxTorso;
	s32 m_nBoneIdxRib1;
	s32 m_nBoneIdxRib2;
	s32 m_nBoneIdxRib3;
	s32 m_nBoneIdxRib4;
	s32 m_nBoneIdxHead;
	s32 m_nBoneIdxPriFire;
	s32 m_nBoneIdxGrapple;
	s32 m_nBoneIdxRightArm;

	CFAnimManFrame m_AnimManFrameSummer;							// Anim frame used for summer


	const	CFVec3A *m_pGazeDir_WS;									// Points to the gaze direction vector (might not be unit in length)

	SnipeState_e m_eSnipeState;

	// weapons
	f32 m_fFireTimer;
#if FANG_WINGC
	f32 m_fPortDiagTimer;			// -port-diag status log interval
	BOOL m_bPortDiagDeathLogged;
#endif
	f32 m_fUnitAim;
	CFQuatA m_qArmAim;

	// ffeedback
	FForceHandle_t m_hForce;

	// jumping
	f32 m_fVerticalVelocity;										// stored off so we know how fast we were falling when we hit the ground
	BOOL m_bPlayLandRecoverAnimation;								// is the big landing animation playing?


	// grappling
	CFVec3A m_vGrappleTgtPos;
	CFVec3A m_vGrappleTgtNormal;
	CFMtx43A m_mtxGrappleClingPosition;
	CFMtx43A m_mtxGrappleFly;

	CFQuatA m_qGrappleLastOrientation;
	f32 m_fFlightOrientTimer;
	CFQuatA m_qGrappleFlyOrientation;
	CFQuatA m_qGrappleLandOrientation;

	BOOL m_bValidGrapplePos;										// currently aimed at a valid grapple tgt?
    GrappleState_e m_eGrappleState;
	f32 m_fTimeToFlyOnCable;
	f32 m_fOOTimeToFlyOnCable;
	f32 m_fUnitCableFly;
	f32 m_fGrappleRecoil;
	BOOL m_bGrappleRecoilOn;
	f32 m_fGrappleFlyYaw;
	f32 m_fGrappleFlyPitch;
	f32 m_fGrappleLandRate;											// how fast the land animation blends in

	f32 m_fUnitWallYaw;												// unit yaw while on wall

	f32 m_fModelPitch;

	CGrapple *m_pGrapple;

	

	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
public:
	CBotSniper();
	~CBotSniper();

	static BOOL InitSystem( void );
	static void UninitSystem( void );

	BOOL	Create( s32 nPlayerIndex=-1, BOOL bInstallDataPort=FALSE, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

	virtual const CFVec3A *GetApproxEyePoint( void ) const;

	// PC port: this was the old no-argument form, which no longer overrode CEntity's (which adds
	// nothing), so the Sniper's collision hit its own mesh: it never landed and was flung away.
	virtual void AppendTrackerSkipList( u32& nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker **apTrackerSkipList=&FWorld_apTrackerSkipList[0] );

	//virtual BOOL CheckpointSave( void );
	//virtual void CheckpointRestore( void );
	//virtual void CheckpointSaveSelect( s32 nCheckpoint );

	virtual void UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup );

	virtual FINLINE void ComputeMtxPalette( BOOL bApplyOffscreenOptimizations ) { FASSERT( IsCreated() ); FASSERT( m_Anim.m_pAnimCombiner != NULL ); m_pCBSniper = this; m_Anim.m_pAnimCombiner->ComputeMtxPalette( bApplyOffscreenOptimizations ); m_pCBSniper = NULL; };

	static void DebugDraw(  CBotSniper *pSniper  );


	virtual FINLINE void GetScopeViewPos( CFVec3A *pScopeViewPos_WS ) { FASSERT( IsCreated() ); *pScopeViewPos_WS = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxPriFire]->m_vPos; }

	FINLINE BOOL IsClinging( void ) { FASSERT( IsCreated() ); return m_eGrappleState == GRAPPLESTATE_CLINGING; }
	FINLINE BOOL IsFlyingOnGrapple( void ) { FASSERT( IsCreated() ); return (m_eGrappleState >= GRAPPLESTATE_LAUNCHING) && (m_eGrappleState <= GRAPPLESTATE_LANDING); }

	static void DrawEffects( void );

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

	virtual void FINLINE ComputeWeaponMuzzlePoint_WS( CFVec3A *pMuzzlePt )		{ FASSERT( IsCreated() ); *pMuzzlePt = m_MtxToWorld.m_vPos; };
	virtual void FINLINE ComputeWeaponMuzzleVec_WS( CFVec3A *pMuzzleVec )		{ FASSERT( IsCreated() ); *pMuzzleVec = m_MtxToWorld.m_vFront; };

	//virtual void Possess( s32 nPlayerIndex );
	//virtual BOOL NotifyBotCollision( CBot *pBot );


//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL _BuildAnimStackDef( void );
	static void _AnimBoneCallback( u32 uBoneidx, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	static void _TracerKilledCallback( TracerDef_t *pTracerDef, TracerKillReason_e nKillReason, const FCollImpact_t *pImpact );
	static void _TracerBuildTrackerSkipList( void *pUser );
	static void _TracerMovedCB( void* pUserData, const CFVec3A& NewPos_WS, BOOL bDied );

	void _ClearDataMembers( void );
	void _UpdateMatrices( void );
	void _HandleWeaponAnimations( void );
	void _EnterSnipingMode( void );
	void _ExitSnipingMode( void );

	//void _HandleAimAnimations( void );
	//void _HandleRecoilAnimations( void );
	void _HandleSummerAnimations( void );
	void _AimRightArm( CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );


	// Jumping
	void _EnterFlyMode( void );
	void _HandleJumping( void );
	void _StartSingleJump( const CFVec3A *pJumpVelocity_WS=NULL );
	void _JumpLanded( void );
	void _HandleJumpAnimations( void );

	//void _HandleTriggerWork( void );	// necessary?
	void _WeaponWork( void );
	void _HandleFireAnimation( void );

	// Grappling
	BOOL _CheckGrapplePos( void );											// will check to see if the point under the reticle is a valid grapple spot
	void _GrappleStopAiming( void );
	void _FireGrapple( void );												// fires the grapple at the grapple point
	void _GrappleWork( void );
	void _BreakGrappleLine( void );
	void _GrappleTakeoff( void );
	void _FlyAlongCable( void );
	void _ClingToWall( void );
	void _ExitGrappleMode( void );
	void _BuildGrappleFlightMtx( f32 fUnitTime );

	void _DebugAnimData( void );

	void _HandleClingingYaw( void );
	void _HandleClingingPitch( void );
	void _HandleFlightOrientation( void );					// handles orientation tasks for the entire flight
	void _HandleLaunchOrientation( void );					// handles getting the bot oriented into the fly position correctly
	void _HandleLandOrientation( void );					// handles getting the bot oriented into the landing/clinging position correctly
	void _HandleCableAnimations( void );
	void _CalcLaunchOrientation( void );
	void _CalcLandingOrientation( void );

	static void _DrawBulletTrail( const BulletTrail_t *pBTrail, FDrawVtx_t *pVtxArray );
	static void _DrawLaserSights( void );

	void _CalcLaserPt( void );
	

	
	friend class CBotSniperBuilder;

FCLASS_STACKMEM_ALIGN( CBotSniper );
} FCLASS_ALIGN_SUFFIX;


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotSniperBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotSniperBuilder : public CBotBuilder {
public:

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CBotSniperBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CBotSniperBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif
