//////////////////////////////////////////////////////////////////////////////////////
// botpred.h - 
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
// 12/10/02 Elliott		Took ownership
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _BOTPRED_H_
#define _BOTPRED_H_ 1

#include "bot.h"
#include "fmotion.h"
#include "FXStreamer.h"
#include "fexplosion.h"



class CBotPartPool;
class CFSoundGroup;
class CEParticle;



FCLASS_ALIGN_PREFIX class CBotPred : public CBot {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// Predator flags:
	enum {
//		PREDFLAG_NEXT_DELTA_VELOCITY_VALID		= 0x00000001,	// TRUE when m_NextDeltaVelocity_WS is non-zero

		PREDFLAG_NONE							= 0x00000000
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
		PART_INSTANCE_COUNT_PER_TYPE = 2,
//		ANIM_USER_COUNT = 4,
		ANIM_IDLE_COUNT = 1	
	};


	typedef enum {
		LIMB_TYPE_TOP_RIGHT_ARM,
		LIMB_TYPE_BOTTOM_RIGHT_ARM,
		LIMB_TYPE_TOP_LEFT_ARM,
		LIMB_TYPE_BOTTOM_LEFT_ARM,

		LIMB_TYPE_COUNT
	} LimbType_e;

	// Animation controls:
	typedef enum {
		ANIMCONTROL_STAND,
		ANIMCONTROL_RC_TETHERED,
		ANIMCONTROL_RC_POWER_DOWN,
		ANIMCONTROL_RC_POWER_UP,

		ANIMCONTROL_BASE_COUNT
	} AnimControl_e;


	// Animation taps:
	typedef enum {
		ANIMTAP_STAND,
		ANIMTAP_RC_TETHERED,
		ANIMTAP_RC_POWER_DOWN,
		ANIMTAP_RC_POWER_UP,

		ANIMTAP_BASE_COUNT
	} AnimTap_e;


	// Base animations:
	typedef enum {
		ANIM_STAND,
		ANIM_RC_TETHERED,
		ANIM_RC_POWER_DOWN,
		ANIM_RC_POWER_UP,

		ANIM_BASE_COUNT
	} Anim_e;


	// Bones:
	typedef enum {
		BONE_DUMMY,
		BONE_TORSO,					// Torso
		BONE_ARM_UPPER_R2,			// ArmUpperR2
		BONE_ARM_LOWER_R2,			// ArmLowerR2
		BONE_GUN_R2,				// GunR2
		BONE_GUN_BARREL_R2,			// GunBarrelR2
									   
		BONE_PRIMARY_FIRE_3,		// Primary_Fire03
									   
		BONE_ARM_UPPER_R1,			// ArmUpperR1
		BONE_ARM_LOWER_R1,			// ArmLowerR1
		BONE_GUN_R1,				// GunR1
		BONE_GUN_BARREL_R1,			// GunBarrelR1
									   
		BONE_PRIMARY_FIRE_4,		// Primary_Fire04

		BONE_ARM_UPPER_L2,			// ArmUpperL2
		BONE_ARM_LOWER_L2,			// ArmLowerL2
		BONE_GUN_L2,				// GunL2
		BONE_GUN_BARREL_L2,			// GunBarrelL2
									
		BONE_PRIMARY_FIRE_1,		// Primary_Fire01

		BONE_ARM_UPPER_L1,			// ArmUpperL1
		BONE_ARM_LOWER_L1,			// ArmLowerL1
		BONE_GUN_L1,				// GunL1
		BONE_GUN_BARREL_L1,			// GunBarrelL1

		BONE_PRIMARY_FIRE_2,		// Primary_Fire02

		BONE_JET_PORT,				// JetPort
		BONE_SPOTLITE,				// Spotlite
		BONE_SPOTLITE_ATTACH,		// AttachPoint_Light
		BONE_TETHER,				// Tether_Port

		BONE_STREAMER_R_ATTACH,		// AttachPoint_R_Streamer
		BONE_STREAMER_L_ATTACH,		// AttachPoint_L_Streamer
		BONE_STREAMER_C_ATTACH,		// AttachPoint_C_Streamer

		BONE_JETBACK1_ATTACH,		// AttachPoint_JetBack1
		BONE_JETBACK2_ATTACH,		// AttachPoint_JetBack2
		BONE_JETDOWN_ATTACH,		// AttachPoint_JetDown
		
		BONE_COUNT
	} Bone_e;


	typedef struct {
		cchar*	pszSpotlightMesh;
		f32		fMinTargetDist;
		cchar*	pszStreamerTex;
		cchar*	pszParticleJetBack;
		cchar*	pszParticleJetDown;

		FExplosion_GroupHandle_t hExplosionBig;
		FExplosion_GroupHandle_t hExplosionSecondary;
		FExplosion_GroupHandle_t hExplosionTiny;
		FExplosion_GroupHandle_t hExplosionFinal;

		CFSoundGroup *pSoundGroupHover;
		CFSoundGroup *pSoundGroupWind;

		FParticle_DefHandle_t hParticleBurnSmoke;
	} BotInfo_Pred_t;


//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:


//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL				m_bSystemInitialized;		// TRUE: InitSystem() has been called
	static u32				m_nBotClassClientCount;		// ref counter for this bot type
	static CBotAnimStackDef m_AnimStackDef;

	static const CBotDef m_BotDef;		// Bot definition data


	// Bot parts:
	static CBotPartPool *m_pPartPool;								// One part pool per bot class


	// BotInfo data:
	static BotInfo_Gen_t				m_BotInfo_Gen;			// General bot info
	static BotInfo_MountAim_t			m_BotInfo_MountAim;		// Mount aim bot info

	static BotInfo_Pred_t				m_BotInfo_Pred;			// Predator bot info
	static const FGameDataMap_t			m_aGameDataMap[];
	static const FGameData_TableEntry_t m_aBotInfoVocab_Pred[];
	
	static CBotPred *m_pCBPred;
	static FMesh_t *m_pMesh;
	static FMesh_t *m_pSpotlightMesh;
	
	static const u32	m_auArmLimbTable[LIMB_TYPE_COUNT];								// used to map firing order to limbs

	// Bot is built from this data:
	static cchar *m_apszBoneNameTable[BONE_COUNT];										// Array of bone names
	static cchar *m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT];					// Array of base animation control names
	static cchar *m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT];							// Array of base animation tap names
	static cchar *m_apszBaseAnimNameTable[ANIM_BASE_COUNT];								// Array of base animation names
	static cchar *m_apszIdleAnimNameTable[ANIM_IDLE_COUNT];								// Array of idle animation names
	
	static const u8 m_aBoneEnableIndices_FullBody[];									// All animatable bones

	static const u8 *m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT];
	static const u8 *m_apnEnableBoneNameIndexTableForEachIdleTap[ANIM_IDLE_COUNT];
	static const u8 m_anEnableBoneNameIndexTableForSummer[];
	static const u8 m_anTagPointBoneNameIndexArray[];
	static const u8 m_nApproxEyePointBoneNameIndex;

	static CFAnimCombinerConfig::ConfigStack_t	m_aAnimCombinerConfigStack[];
	static CFAnimCombinerConfig::ConfigTap_t	m_aAnimCombinerConfigTaps[];
	static CFAnimCombiner::AttachList_t			m_aAnimAttach[];

	//user anim bone masks
	static const u8 m_aBoneEnableIndices_UserAnim_UpperBody[];
	static const u8 m_aBoneEnableIndices_UserAnim_LowerBody[];
	static const u8 m_aBoneEnableIndices_UserAnim_UpperTorso[];
	static const u8 m_aBoneEnableIndices_UserAnim_LowerTorso[];
	static const u8 m_aBoneEnableIndices_UserAnim_LeftArm[];
	static const u8 m_aBoneEnableIndices_UserAnim_RightArm[];
	static const u8 m_aBoneEnableIndices_UserAnim_Head[];


	const CFVec3A *m_pGazeDir_WS;							// Points to the gaze direction vector (might not be unit in length)

	f32			m_fAltitude;								// our altitude
	CFVec3A		m_ModelLagPos;								// the model's position( lags behind the mount

	CFMotion	m_Motion;									// our motion object
	CFVec3A		m_PrevVelocity_WS;							// previous velocity
	CFVec3A		m_vLastValidTargetPoint_WS;					// our last good target so we can blend out of aiming state properly

	u32 m_uBoneIndexSpotlight;
	u32 m_uBoneIndexExhaust;

	s32	m_uBoneIndexArmSeg0TL;
	s32	m_uBoneIndexArmSeg0TR;
	s32	m_uBoneIndexArmSeg0BL;
	s32	m_uBoneIndexArmSeg0BR;

	s32	m_uBoneIndexArmSeg1TL;
	s32	m_uBoneIndexArmSeg1TR;
	s32	m_uBoneIndexArmSeg1BL;
	s32	m_uBoneIndexArmSeg1BR;

	s32 m_uBoneIndexArmSeg2TL;
	s32 m_uBoneIndexArmSeg2TR;
	s32 m_uBoneIndexArmSeg2BL;
	s32 m_uBoneIndexArmSeg2BR;

	
	//BOOL				m_bLightEnabled;					// should our light be on or off?

	// streamer data
	static cchar*		m_apszStreamerBones[];
	FXStreamerHandle_t	m_hStreamer;
	static CFTexInst	m_StreamerTexInst;

	// exhaust smoke particles
	static FParticle_DefHandle_t	m_hExhaustParticleDefBack;
	static FParticle_DefHandle_t	m_hExhaustParticleDefDown;
	FParticle_EmitterHandle_t		m_hExhaustParticleEmitterBack1;
	FParticle_EmitterHandle_t		m_hExhaustParticleEmitterBack2;
	FParticle_EmitterHandle_t		m_hExhaustParticleEmitterDown;
	f32								m_fUnitExhaustBackMag;
	f32								m_fUnitExhaustDownMag;
	f32								m_fUnitExhaustPitch;				// controls the exhaust vent on the bottom of the pred
	f32								m_fUnitExhaustYaw;					// controls the exhaust vent on the bottom of the pred

	f32		m_fMaxAltitude_WS;								// this is the max altitude for this predator in world space.  Only used if human is in control
	f32		m_fUnitAim;										// 0 = not aiming, 1 = fully aimed

	CFAudioEmitter *m_pWindAudioEmitter;
	CFAudioEmitter *m_pHoverAudioEmitter;
	f32 m_fUnitWindLevel;

	CFVec3A m_DeathRotUnitAxis_WS;
	f32 m_fDeathMaxDestructionCountdownSecs;
	f32 m_fDeathUnitRotVel;
	f32 m_fDeathBigExplosionCountdownSecs;
	f32 m_fDeathTinyExplosionCountdownSecs;
	f32 m_fDeathLimbBlowCountdownSecs;
	u32 m_nDeathLimbBlowIndex;
	CEParticle *m_pDeathBurnSmokeParticle;



//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	CBotPred();
	virtual ~CBotPred();

	static BOOL InitSystem( void );
	static void UninitSystem( void );

	BOOL Create( s32 nPlayerIndex=-1, BOOL bInstallDataPort=FALSE, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);	
	virtual void ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS );


	virtual void CheckpointSaveSelect( s32 nCheckpoint );
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );

	virtual f32 ComputeEstimatedControlledStopTimeXZ( void );
	virtual f32 ComputeEstimatedControlledStopTimeY( void );

	//virtual void NotifyShieldStatus( u32 uStatus, const CFVec3A &vPos );

	//DEBUG FN
	static void DebugDraw( CBotPred *pPred );
	FINLINE f32 GetAltitude( void ) { return m_fAltitude;}
	
	virtual FINLINE void ComputeMtxPalette( BOOL bApplyOffscreenOptimizations ) { FASSERT( IsCreated() ); FASSERT( m_Anim.m_pAnimCombiner != NULL ); m_pCBPred = this; m_Anim.m_pAnimCombiner->ComputeMtxPalette(bApplyOffscreenOptimizations); m_pCBPred = NULL; };

	virtual void Die( BOOL bSpawnDeathEffects=TRUE, BOOL bSpawnGoodies=TRUE );
	virtual void DeathWork( void );
	virtual void SpawnDeathEffects( void );

	BOOL GetLimbsStatus(u32* puNumIntact = NULL, u32* puNumDangling = NULL);  //returns true if any limbs are existing, no matter how limp they may be


//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual void ClassHierarchyDestroy( void );

	virtual BOOL ClassHierarchyBuild( void );				// Must be implemented by all classes in the derivation chain
	virtual BOOL ClassHierarchyBuilt( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );	// Must be implemented by all classes in the derivation chain

	virtual void ClassHierarchyWork( void );
	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );
	virtual void ClassHierarchyRelocated( void *pIdentifier );

	virtual void InflictDamageResult( const CDamageResult *pDamageResult );

	virtual void Possess( s32 nPlayerIndex, f32 fPossessionArmorModifier );

	virtual BOOL ClassHierarchyLoadSharedResources( void );
	virtual void ClassHierarchyUnloadSharedResources( void );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL _BuildAnimStackDef( void );

	void _ClearDataMembers( void );
	void _UpdateMatrices( void );

	void _HandleHover( void );
	void _HandleXZTranslation( void );

	void _HandleCollision( void );
	void _CheckAltitude( void );		// does a projected sphere test straight down to determine how high we are

	void _HandleTargeting( void );		// pred is a little different when it comes to targetting
	void _HandleWeaponWork( void );

	void _MoveModel( void );

	static void _AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	void UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup );

	void _ExhaustWork( void );				// handle the exhaust smoke particle emitters
	void _SoundWork( void );				// handle looping type sounds

	BOOL _AnimationsActive( void );			// checks to make sure all animations are inactive (control set to 0) except idle

	void _RemoveAllAliveEffects( void );

	void _DeathWork_UpdatePos( void );
	void _DeathWork_UpdateRot( void );
	void _DeathWork_UpdateExplosion( void );
	void _DeathWork_SpawnExplosion( FExplosion_GroupHandle_t hExplosion, f32 fRadius );
	void _StartBurnSmoke( void );
	void _StopBurnSmoke( void );


	friend class CBotPredBuilder;

	FCLASS_STACKMEM_ALIGN( CBotPred );
} FCLASS_ALIGN_SUFFIX;






//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotPredBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotPredBuilder : public CBotBuilder {

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	f32 m_fMaxAltitude;
	f32 m_fMaxSpeed;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CBotPredBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CBotPredBuilder );
} FCLASS_ALIGN_SUFFIX;


#endif