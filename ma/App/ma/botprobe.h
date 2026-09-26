//////////////////////////////////////////////////////////////////////////////////////
// botprobe.h - 
//
// Author: Nathan Miller    
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
// 12/30/02	Miller	    Created.
// 03/13/03	Mackellar	Beefed up the CanGrabBot Func, added func for asking if Grab is in progress
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _BOTPROBE_H_
#define _BOTPROBE_H_ 1

#include "fang.h"
#include "bot.h"

class CBotGlitch;
class CDamageProfile;
class CFDebrisGroup;
struct _MagnetRing_t;

FCLASS_ALIGN_PREFIX class CBotProbe : public CBot {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	typedef struct {
		// Offset from the mount position
		f32 fGrabCollisionSphereBumpX;
		f32 fGrabCollisionSphereBumpY;
		f32 fGrabCollisionSphereBumpZ;
		f32 fGrabCollisionSphereRadius;

		f32 fSpinDeathZoneX_MS;
		f32 fSpinDeathZoneY_MS;
		f32 fSpinDeathZoneZ_MS;
		f32 fSpinCollisionSphereRadius;
		
		f32 fMagnetOuterRadius;				// Outer radius to actually bother the bot
		f32 fOOMagnetOuterRadius;			// 1.0f / fMagnetOuterRadius
		f32 fMagnetYRange;					// Range in Y where we can have the magnet bother the bot

		cchar *pszJetParticleName;
		cchar *pszSparkParticleName;
		cchar *pszPuffParticleName;

		cchar *pszDamageProfilePinch;
		cchar *pszDamageProfileShake;
		cchar *pszDamageProfileSpin;

	} BotInfo_Probe_t;

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF


//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	// Misc definitions:
	enum {
		ANIM_IDLE_COUNT	= 1,
	};

	// Animation controls:
	typedef enum {
		ANIMCONTROL_STAND,
		ANIMCONTROL_ATTACK_LEECH,
		ANIMCONTROL_ATTACK_SPIN,
		ANIMCONTROL_ATTACK_FLING,
		ANIMCONTROL_POWER_DOWN,
		ANIMCONTROL_POWER_UP,

		ANIMCONTROL_BASE_COUNT
	} AnimControl_e;


	// Animation taps:
	typedef enum {
		ANIMTAP_STAND,
		ANIMTAP_ATTACK_LEECH,
		ANIMTAP_ATTACK_SPIN,
		ANIMTAP_ATTACK_FLING,
		ANIMTAP_POWER_DOWN,
		ANIMTAP_POWER_UP,

		ANIMTAP_BASE_COUNT
	} AnimTap_e;


	// Base animations:
	typedef enum {
		ANIM_STAND,
		ANIM_ATTACK_LEECH,
		ANIM_ATTACK_SPIN,
		ANIM_ATTACK_FLING,
		ANIM_POWER_DOWN,
		ANIM_POWER_UP,

		ANIM_BASE_COUNT
	} Anim_e;


	// Bones:
	typedef enum {
		BONE_TORSO,
		BONE_HEAD,
		BONE_LEG1_A,
		BONE_LEG1_B,
		BONE_LEG1_C,
		BONE_LEG1_D,
		BONE_LEG1_E,
		BONE_LEG2_A,
		BONE_LEG2_B,
		BONE_LEG2_C,
		BONE_LEG2_D,
		BONE_LEG2_E,
		BONE_LEG3_A,
		BONE_LEG3_B,
		BONE_LEG3_C,
		BONE_LEG3_D,
		BONE_LEG3_E,
		BONE_ATTACH_POINT_JET,

		BONE_COUNT
	} Bone_e;

	typedef enum { 
		BOTSTATE_IDLE,
		BOTSTATE_LEECH_ATTACK,
		BOTSTATE_SPIN_ATTACK,
		BOTSTATE_LEECH_TURN,
		BOTSTATE_LEECHING,
		BOTSTATE_SPINING
	} ProbeBotState_e;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:


//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	enum { 
		_NUM_COLLISION_BONES = 6,
		PART_INSTANCE_COUNT_PER_TYPE = 2,
	};

	enum {
		_DAMAGE_PINCH = 0,
		_DAMAGE_HIT_GROUND,
		_DAMAGE_SPIN,

		_DAMAGE_COUNT
	};

	enum {
		_SOUND_ATTACK_IMPACT = 0,
		_SOUND_ATTACK_MISS,
		_SOUND_ATTACK_FLING,
		_SOUND_ATTACK_GROUND_HIT,
		_SOUND_HOVER_LOOP,
		_SOUND_ATTACK_SPIN_LOOP,
		_SOUND_ATTACK_MAGNET_LOOP,

		_SOUND_COUNT
	};

	enum {
		_NUM_PULL_PARTICLES = 4,
	};

	typedef enum {
		LIMB_TYPE_HEAD,
		LIMB_TYPE_TORSO,

		LIMB_TYPE_LEG1,
		LIMB_TYPE_LEG2,
		LIMB_TYPE_LEG3,

		LIMB_TYPE_COUNT
	} LimbType_e;

	typedef struct {
		f32 fUnitTime;
		f32 fTimeMult;

		CFVec3A SrcPos;

		FParticle_EmitterHandle_t hMagEmitter;
	} MagnetPullParticle_t;

	static BOOL	m_bSystemInitialized;									// TRUE: InitSystem() has been called
	static u32 m_nBotClassClientCount;
	static BOOL m_bRingsSetup;
	static const CBotDef m_BotDef;										// Bot definition data
	static CBotAnimStackDef m_AnimStackDef;

//	static CFCollInfo m_CollInfo;										// Used for collision detection
//	static CFSphere m_CollSphere;										// Used for collision detection
	static CBotProbe *m_pThisBot;										// Used for collision detection

	static BotInfo_Gen_t		m_BotInfo_Gen;							// General bot info
	static BotInfo_MountAim_t	m_BotInfo_MountAim;						// Mount aim bot info
	static BotInfo_Walk_t		m_BotInfo_Walk;							// Walk bot info
	static BotInfo_Weapon_t		m_BotInfo_Weapon;						// Weapon bot info
	static BotInfo_Probe_t		m_BotInfo_Probe;						// Probe bot info
	
	static const FGameData_TableEntry_t m_aBotInfoVocab_Probe[];
	static const FGameDataMap_t m_aGameDataMap[];

	static cchar *m_apszBoneNameTable[BONE_COUNT];						// Array of bone names
	static cchar *m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT];	// Array of base animation control names
	static cchar *m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT];			// Array of base animation tap names
	static cchar *m_apszIdleAnimNameTable[ANIM_IDLE_COUNT];				// Array of idle animation names
	static cchar *m_apszBaseAnimNameTable[ANIM_BASE_COUNT];				// Array of base animation names
	static const u8 m_anTagPointBoneNameIndexArray[];
	static const u8 m_nApproxEyePointBoneNameIndex;
	static const u8 *m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT];
	static const u8 *m_apnEnableBoneNameIndexTableForEachIdleTap[ANIM_IDLE_COUNT];
	static const u8 m_anEnableBoneNameIndexTableForSummer_Normal[];

	static CFAnimCombinerConfig::ConfigStack_t	m_aAnimCombinerConfigStack[];
	static CFAnimCombinerConfig::ConfigTap_t	m_aAnimCombinerConfigTaps[];
	static CFAnimCombiner::AttachList_t			m_aAnimAttach[];

	static FParticle_DefHandle_t m_hJetParticleDef;
	static FParticle_DefHandle_t m_hSparkParticleDef;
	static FParticle_DefHandle_t m_hPuffParticleDef;

	static CDamageProfile *m_apDamageProfile[_DAMAGE_COUNT];
	static FSndFx_FxHandle_t m_hSounds[_SOUND_COUNT];
	static CFDebrisGroup *m_pDebrisGroup;

	static _MagnetRing_t *m_aRings;
	static FLinkRoot_t m_FreeRingList;

	static CBotPartPool *m_pPartPool;

	const CFVec3A *m_pGazeDir_WS;							// Points to the gaze direction vector (might not be unit in length)

	u8 m_BotState;											// Bot's state
	u8 m_BotAnimControl;									// Current animation control
	u8 m_BotAnimTap;										// Current animation tap

	BOOL8 m_bSkipAnimAdvance;								// Don't advance the animations
	BOOL m_bAttackTested;									// TRUE if we have done the attack test for the grab
	f32 m_fUnitBlendAttack;									// Blend into the attack animation, shared for both spin and leech
	f32 m_fUnitBlendDir;									// Direction to blend, 1.0=up, -1.0=down, shared for both spin and leech

	f32 m_fLeechTurnTime;									// Used to match the Probe's position and yaw to the bot
	CFVec3A m_StartPosition;								// Where we started
	CBotGlitch *m_pBotLeeching;								// The bot that we are currently leeching

	f32 m_fJetIntensity;									// Intensity of the jet particle system
	CFVec3A m_JetEmitterDir;								// Direction of the emitter
	FParticle_EmitterHandle_t m_hJetEmitter;				// Particle handle

	CFVec3A m_SpinDeathZonePos_WS;							// WS position for the AI where the bots will get hit

	f32 m_fEmitterVolume;									// Used to blend in emitter volume
	f32 m_fEmitterVolumeDir;								// 1.0=go up -1.0=go down
	CFAudioEmitter *m_pIdleEmitter;							// Idle sound emitter
	CFAudioEmitter *m_pSpinEmitter;							// Spin attack emitter
	CFAudioEmitter *m_pMagnetEmitter;						// Magnet attack emitter

	CFVec3A m_LagPoint;										// Lag point used to pitch bot

	f32 m_fNextShakeTime;									// Time till the next camera / control shake
	f32 m_fNextRingTime;									// Time till the next magnet ring is spawned
	CBot *m_pMagnetBot;										// Bot we are pulling towards us
	CFVec3A m_MagnetDir_MS;									// Direction we want to pull the player in player's MS
	CFVec3A m_LastBotPosition;								// Last known bot position
	FLinkRoot_t m_MagnetRings;								// Rings we are using

	// Bones of interest
	const CFVec3A *m_pTorsoBonePos;
	const CFVec3A *m_pJetAttachBonePos;
	
	const CFVec3A *m_apLegTestBones[_NUM_COLLISION_BONES];	// leg1 dummy, 2, 3, leg1_e, 2, 3
	CFVec3A m_aLegTestBonesPrev[_NUM_COLLISION_BONES];		// Same as above

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	CBotProbe();
	~CBotProbe();

	static BOOL InitSystem( void );
	static void UninitSystem( void );

	BOOL Create( s32 nPlayerIndex=-1, BOOL bInstallDataPort=FALSE, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

	virtual void Die( BOOL bSpawnDeathEffects=TRUE, BOOL bSpawnGoodies=TRUE );
	virtual void Power( BOOL bPowerUp, f32 fPowerOffTime=0.0f, f32 fPowerOffOnSpeedMult=1.0f );
	virtual f32 ComputeEstimatedControlledStopTimeXZ( void );
	virtual f32 ComputeEstimatedControlledStopTimeY( void );

	// Begin AI Interface
	FINLINE BOOL IsIdle( void )							const { return m_BotState == BOTSTATE_IDLE; }
	FINLINE BOOL IsAttackLeechEnabled( void )			const { return m_BotAnimControl == ANIMCONTROL_ATTACK_LEECH || m_BotAnimControl == ANIMCONTROL_ATTACK_FLING; }
	FINLINE BOOL IsAttackSpinEnabled( void )			const { return m_BotAnimControl == ANIMCONTROL_ATTACK_SPIN; }
	FINLINE BOOL IsUsingMagnet( void )					const { return m_pMagnetBot != NULL; }
	FINLINE BOOL IsLeeching(CBot** ppWho = NULL)			  { if (ppWho) {*ppWho = (CBot*)m_pBotLeeching;} return m_pBotLeeching != NULL;}

	FINLINE f32 GetSpinDeathZoneRadius( void )				{ return m_BotInfo_Probe.fSpinCollisionSphereRadius; }
	FINLINE const CFVec3A *GetSpinDeathZonePos_WS( void )	{ return &m_SpinDeathZonePos_WS; }

	FINLINE f32 GetGrabPointDistanceYFromMount( void )		{ return m_BotInfo_Probe.fGrabCollisionSphereBumpY; }
	FINLINE f32 GetGrabSphereRadius( void )					{ return m_BotInfo_Probe.fGrabCollisionSphereRadius; }

	FINLINE f32 GetMagnetMaxHeight( void )					{ return m_BotInfo_Probe.fMagnetYRange; }
	FINLINE f32 GetMagnetMaxRadius( void )					{ return m_BotInfo_Probe.fMagnetOuterRadius; }

	FINLINE BOOL CanGrabBot( const CBot *pBot )				{ return pBot && (pBot->TypeBits() & ENTITY_BIT_BOTGLITCH ) && !pBot->GetCurMech(); }	  //can only grab botglitches that aren't in vehicles.

	void BeginMagnet( CBot *pBot );
	void EndMagnet( void );

	void AbortAttack( void );
	// End AI

	void GroundHitDamageBot( CBot *pBot );

	virtual void CheckpointSaveSelect( s32 nCheckpoint );
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );

	virtual void AppendTrackerSkipList( void );

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
	
	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );

	virtual void ClassHierarchyWork( void );
	
//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	void _ClearDataMembers( void );
	BOOL _SetupRings( void );
	void _DestroyRings( void );
	void _InitSoundHandles( void );
	void _UpdateMatrices( void );
	void _SpawnBotParts( const CFVec3A *pPos, const u8 &uMin, const u8 &uMax );
	void _HandleHover( void );
	void _HandleTranslation( void );
	void _HandleCollision( void );
	void _HandleCollisionResponse( void );
	void _HandleAttacks( void );
	void _HandleAttackAnimation( void );
	BOOL _HandleLeechTurn( void );
	void _LeechGrabTest( void );
	void _SpinHitTest( void );
	void _ReleaseRings( void );

	void _DisableMagnet( void );
	void _UpdateMagnet( void );
	void _UpdateActiveRings( void );

	static _MagnetRing_t *_GetRing( void );
	static void _FreeRing( _MagnetRing_t *pRing );

	static BOOL _TrackersCallbackProjSphere( CFWorldTracker *pTracker, FVisVolume_t *pWorldLeafNode );
	static BOOL _TrackerCollisionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	static BOOL _GrabCollisionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	static BOOL _BuildAnimStackDef( void );
	static void _AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );

	friend class CBotProbeBuilder;

FCLASS_STACKMEM_ALIGN( CBotProbe );
} FCLASS_ALIGN_SUFFIX;


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotProbeBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotProbeBuilder : public CBotBuilder {
public:


//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CBotProbeBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CBotProbeBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif

