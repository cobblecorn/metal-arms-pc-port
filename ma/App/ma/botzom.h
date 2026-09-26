//////////////////////////////////////////////////////////////////////////////////////
// botzom.h - Zombie bot.
//
// Author: Steve Ranck     
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
// 01/27/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _BOTZOM_H_
#define _BOTZOM_H_ 1

#include "fang.h"
#include "bot.h"
#include "fexplosion.h"
#include "FXStreamer.h"
#include "fgamedata.h"




class CBotZomPartGroup;
class CBotZomPart;
class CFAudioEmitter;
class CEParticle;




FCLASS_ALIGN_PREFIX class CBotZom : public CBot {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		ZOMMODE_IN_PIECES,
		ZOMMODE_FORMING,
		ZOMMODE_ALIVE,
		ZOMMODE_CRUMBLING,

		ZOMMODE_COUNT
	} ZomMode_e;


	// Misc definitions:
	enum {
		ANIM_IDLE_COUNT = 1,					// SER: Eventually this will be retrieved from a data file
		HIT_ENTITY_MAX_COUNT = 5,				// Number of entities we can cause damage to during a slash
		SLASH_STREAMER_COUNT = 3,				// Number of slashing streamers
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
		ZOMFLAG_SLASHING				= 0x00000001,		// TRUE when the slashing animation is in the damage range
		ZOMFLAG_SLASH_HIT_TERRAIN		= 0x00000002,		// TRUE when the slashing attack has hit terrain, static object, or any other non-entity
		ZOMFLAG_START_FORMED			= 0x00000004,		// TRUE if this Zombie Bot is to start in a formed state, as opposed to scattered pieces
		ZOMFLAG_PLAY_FORM_ANIM			= 0x00000008,		// TRUE when we're playing the formation animation
		ZOMFLAG_DISABLE_AI_WAKE_UP		= 0x00000010,		// TRUE if the AI isn't allowed to wake up the bot
		ZOMFLAG_LOWER_FORMING_VOLUME	= 0x00000020,		// TRUE when we're lowering the forming volume
		ZOMFLAG_WAIT_FOR_FIRE2_RELEASE	= 0x00000040,		// TRUE when we're waiting for FIRE2 to be released

		ZOMFLAG_NONE					= 0x00000000,
	};


	// Animation controls:
	typedef enum {
		ANIMCONTROL_FORM,

		ANIMCONTROL_STAND,
		ANIMCONTROL_WALK,
		ANIMCONTROL_RUN,

		ANIMCONTROL_JUMP_LAUNCH,
		ANIMCONTROL_JUMP_FLY,
		ANIMCONTROL_JUMP_LAND_LOWER,
		ANIMCONTROL_JUMP_LAND_UPPER,

		ANIMCONTROL_SLASH_D,
		ANIMCONTROL_SLASH_FB,
		ANIMCONTROL_SLASH_D_LOWER,
		ANIMCONTROL_SLASH_FB_LOWER,

		ANIMCONTROL_AIM_SUMMER,

		ANIMCONTROL_BASE_COUNT
	} AnimControl_e;


	// Animation taps:
	typedef enum {
		ANIMTAP_FORM,

		ANIMTAP_STAND,
		ANIMTAP_WALK,
		ANIMTAP_RUN,

		ANIMTAP_JUMP_LAUNCH,
		ANIMTAP_JUMP_FLY,
		ANIMTAP_JUMP_LAND_LOWER,
		ANIMTAP_JUMP_LAND_UPPER,

		ANIMTAP_SLASH_D,
		ANIMTAP_SLASH_FB,
		ANIMTAP_SLASH_D_LOWER,
		ANIMTAP_SLASH_FB_LOWER,

		ANIMTAP_AIM_SUMMER,

		ANIMTAP_BASE_COUNT
	} AnimTap_e;


	// Base animations:
	typedef enum {
		ANIM_FORM,

		ANIM_STAND,
		ANIM_WALK,
		ANIM_RUN,

		ANIM_JUMP_LAUNCH,
		ANIM_JUMP_FLY,
		ANIM_JUMP_LAND,

		// Forward slashing:
		ANIM_SLASH_F_FROM_REST,
		ANIM_SLASH_F,
		ANIM_SLASH_F_TO_REST,

		// Backward slashing:
		ANIM_SLASH_B_FROM_REST,
		ANIM_SLASH_B,
		ANIM_SLASH_B_TO_REST,

		// Downward slashing:
		ANIM_SLASH_D_FROM_REST,
		ANIM_SLASH_D,
		ANIM_SLASH_D_TO_REST,

		ANIM_BASE_COUNT
	} Anim_e;


	// Bones:
	typedef enum {
		BONE_EYE,
		BONE_GROIN,
		BONE_R_LEG_UPPER,
		BONE_R_KNEE,
		BONE_R_LEG_MIDDLE,
		BONE_R_LEG_LOWER,
		BONE_R_FOOT,
		BONE_R_TOE,
		BONE_L_LEG_UPPER,
		BONE_L_LEG_LOWER,
		BONE_L_TOE,
		BONE_WAIST,
		BONE_WAIST_SCRAPB,
		BONE_WAIST_SCRAPA,
		BONE_WAIST_SCRAPC,
		BONE_TORSO,
		BONE_NECK_A,
		BONE_NECK_B,
		BONE_NECK_C,
		BONE_NECK_D,
		BONE_NECK_E,
		BONE_NECK_F,
		BONE_EYESTALK,
		BONE_BACK_JUNK_D,
		BONE_BACK_JUNK_C,
		BONE_BACK_JUNK_A,
		BONE_BACK_JUNK_B,
		BONE_R_ARM_UPPER,
		BONE_L_ARM_UPPER,
		BONE_L_ARM_MIDDLE,
		BONE_L_ARM_LOWER,
		BONE_L_HAND,
		BONE_THUMB_A,
		BONE_THUMB_B,
		BONE_L_FINGER_1A,
		BONE_L_FINGER_1B,
		BONE_L_FINGER_1_TIP,
		BONE_L_FINGER_2A,
		BONE_L_FINGER_2B,
		BONE_L_FINGER_2_TIP,
		BONE_HOSE_A,

		BONE_COUNT
	} Bone_e;


	typedef enum {
		SLASH_STATE_NONE,

		// Forward slashing:
		SLASH_STATE_F_FROM_REST,
		SLASH_STATE_F,
		SLASH_STATE_F_TO_REST,

		// Backward slashing:
		SLASH_STATE_B_FROM_REST,
		SLASH_STATE_B,
		SLASH_STATE_B_TO_REST,

		// Downward slashing:
		SLASH_STATE_D_FROM_REST,
		SLASH_STATE_D_FROM_FB,
		SLASH_STATE_D,
		SLASH_STATE_D_TO_REST,
		SLASH_STATE_D_TO_F,
		SLASH_STATE_D_TO_B,

		SLASH_STATE_COUNT
	} SlashState_e;


	typedef struct {
		FTexDef_t *pSlashStreamerTexDef;				// Texture to use for the slashing streamer
		f32 fSlashStreamerAlpha;						// Alpha of the slashing streamer
		f32 fSlashStreamerWidth;						// Width of the slashing streamer

		CDamageProfile *pDamageProfileSlash;			// Damage profile to use when slashing

		cchar *pszClawDamageSphereBoneName;				// Name of the bone to attach the claw damage sphere to
		f32 fClawDamageSphereRadius;					// Radius of the claw damage sphere

		u32 nPartGroupCount;							// Number of part groups to create

		u32 nDeathCount;								// Number of times the bot can die before being dead for good
		f32 fDeathStayAsleepSecs;						// Stay asleep for this long after dying
		f32 fReformInterruptionStayAsleepSecs;			// Stay asleep for this long after collapsing while reforming

		FParticle_DefHandle_t hBreathParticle;			// Breath particle
		f32 fBreathTimeMin;								// Minimum time between breaths
		f32 fBreathTimeMax;								// Maximum time between breaths

		FExplosion_GroupHandle_t hPartExplosion;		// Handle to explosion group to use for this part

		CFSoundGroup *pSoundGroupSlash;
		CFSoundGroup *pSoundGroupAssemble;
		CFSoundGroup *pSoundGroupCrumbleApart;
		CFSoundGroup *pSoundGroupBlownApart;
		CFSoundGroup *pSoundGroupBreathPuff;
		CFSoundGroup *pSoundGroupHitDamageable;
		CFSoundGroup *pSoundGroupHitNonDamageable;
		CFSoundGroup *pSoundGroupClatter;
	} BotInfo_Zom_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;					// TRUE: InitSystem() has been called
	static u32 m_nBotClassClientCount;					// Number of bots of this class using the class-wide resources
	static const CBotDef m_BotDef;						// Bot definition data


	// BotInfo data:
	static BotInfo_Gen_t m_BotInfo_Gen;					// General bot info
	static BotInfo_MountAim_t m_BotInfo_MountAim;		// Mount aim bot info
	static BotInfo_Walk_t m_BotInfo_Walk;				// Walk bot info
	static BotInfo_Jump_t m_BotInfo_Jump;				// Jump bot info
	static BotInfo_Weapon_t m_BotInfo_Weapon;			// Weapon bot info
	static BotInfo_Zom_t m_BotInfo_Zom;					// ZombieBot bot info

	static const FGameData_TableEntry_t m_aBotInfoVocab_Zom[];
	static const FGameDataMap_t m_aGameDataMap[];

	static FMesh_t *m_pZomBotMesh;
	static CFTexInst m_StreamerTexInst;
	static cchar *m_apszStreamerBones[];
	static CFVec3A m_GroinVecY_WS;						// Recorded groin matrice's Y vector (used in the animation callback)
	static CFCollInfo m_CollInfo;
	static CFMtx43A m_TempAnimMtx;



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
	static const u8 m_anTagPointBoneNameIndexArray[];
	static const u8 m_nApproxEyePointBoneNameIndex;

	static CFAnimCombinerConfig::ConfigStack_t m_aAnimCombinerConfigStack[];
	static CFAnimCombinerConfig::ConfigTap_t m_aAnimCombinerConfigTaps[];
	static CFAnimCombiner::AttachList_t m_aAnimAttach[];

	static const u8 m_aBoneEnableIndices_FullBody[];
	static const u8 m_aBoneEnableIndices_AimSummer[];
	static const u8 m_aBoneEnableIndices_UpperBody[];
	static const u8 m_aBoneEnableIndices_LowerBody[];
	static const u8 m_aBoneEnableIndices_JumpLand_Lower[];
	static const u8 m_aBoneEnableIndices_JumpLand_Upper[];


	CBotZomPartGroup *m_pPartGroup;						// Our current part group (NULL=none)

	u32 m_nZomFlags;									// See ZOMFLAG_* for info

	FMeshTexLayerHandle_t m_hTexLayerLightning;			// Handle to the emissive lightning on the bot
	FMeshTexLayerHandle_t m_hTexLayerEye;				// Handle to the emissive eye on the bot
	f32 m_fEmissiveIntensity;							// The intensity of the emissive materials on the bot
	f32 m_fEmissiveAnimAngle;							// Used to animate the emissive eye

	f32 m_fReformNormHealth;							// Health to use when re-forming
	f32 m_fSecsUntilAutoWakeup;							// Number of seconds until we'll automatically wake up our bot (-1=disable auto-wakeup)

	s32 m_nBreathParticleBoneIndex;						// Bone index for breath particle emission (-1=none)
	f32 m_fBreathParticleTimer;							// Countdown seconds until next emission

	const CFVec3A *m_pGazeDir_WS;						// Points to the gaze direction vector (might not be unit in length)

	u8 m_nSlashState;									// See SlashState_e for info
	u8 m_nZomMode;										// See ZomMode_e for info
	u8 m_nRemainingDeathCount;							// Number of remaining times bot can die before being dead for good

	s8 m_nBoneIndexGroin;								// Mesh's groin bone index
	s8 m_nBoneIndexTorso;								// Mesh's torso bone index
	s8 m_nBoneIndexDamageSphere;						// Mesh's damage sphere bone index

	u8 m_nHitEntityCount;								// Number of valid entries in m_apHitEntity
	CEntity *m_apHitEntity[HIT_ENTITY_MAX_COUNT];		// Array of entities that the bot has hit during a slash move

	FXStreamerHandle_t m_hStreamer;

	f32 m_fFormingVolume;								// Current volume we're playing the forming sound with
	CFAudioEmitter *m_pAudioEmitterForming;				// Audio emitter used when the bot is forming (NULL=not playing forming sound)




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// Creation/Destruction:
	CBotZom();
	virtual ~CBotZom();

	static BOOL InitSystem( void );
	static void UninitSystem( void );

	BOOL Create( s32 nPlayerIndex=-1, BOOL bInstallDataPort=FALSE, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL, BOOL bStartInPieces=FALSE );


	// Misc:
	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);
	virtual const CFVec3A *GetApproxEyePoint( void ) const;


	// Checkpoint:
	virtual void CheckpointSaveSelect( s32 nCheckpoint );
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );


	// Actions:
	BOOL Hop( BOOL bHopLeft );
	BOOL StartForming( void );
	BOOL SpreadIntoHoveringPieces( void );
	BOOL CollapseIntoDebris( void );
	BOOL CrumbleIntoDebris( void );
	BOOL ReactToBlast( const CDamageResult *pDamageResult, BOOL bIfAliveUseRecoil );
	BOOL ReactToImpact( const CDamageResult *pDamageResult );
	void ResetBotToScatteredPieces( void );
	virtual void Squish( const CBot* pSquisher );

	FINLINE ZomMode_e GetZomMode( void ) const { FASSERT( IsCreated() ); return (ZomMode_e)m_nZomMode; }
	FINLINE BOOL IsSlashing(void) const { return m_nSlashState != SLASH_STATE_NONE;}


	// Sleep/Wake:
	FINLINE void AllowAIToWakeBot( void ) { FASSERT( IsCreated() ); FMATH_CLEARBITMASK( m_nZomFlags, ZOMFLAG_DISABLE_AI_WAKE_UP ); }
	FINLINE void DontAllowAIToWakeBot( void ) { FASSERT( IsCreated() ); FMATH_SETBITMASK( m_nZomFlags, ZOMFLAG_DISABLE_AI_WAKE_UP ); }
	FINLINE BOOL CanAIWakeBot( void ) const { FASSERT( IsCreated() ); return m_nZomFlags & ZOMFLAG_DISABLE_AI_WAKE_UP; }

	virtual FINLINE BOOL IsCapableOfHopping( void ) const { FASSERT( IsCreated() ); return TRUE; }
	virtual FINLINE BOOL CanSleep( void ) const { FASSERT( IsCreated() ); return TRUE; }
	virtual BOOL WakeUp( void );
	virtual BOOL NapJerk( BOOL bAllowAutoWakeup = TRUE );
	virtual BOOL Sleep( void );


	// Power:
	virtual void Power( BOOL bPowerUp, f32 fPowerOffTime=0.0f, f32 fPowerOffOnSpeedMult=1.0f );
	virtual void Power_SetState( BOOL bPowerUp, f32 fPowerOffTime=0.0f );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL ClassHierarchyLoadSharedResources( void );
	virtual void ClassHierarchyUnloadSharedResources( void );

	virtual void ClassHierarchyDestroy( void );

	virtual BOOL ClassHierarchyBuild( void );
	virtual BOOL ClassHierarchyBuilt( void );
	virtual void ClassHierarchyResolveEntityPointerFixups( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );

	virtual void ClassHierarchyWork( void );
	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );

	virtual void ApplyVelocityImpulse_MS( const CFVec3A &rVelocityImpulseVec_MS );
	virtual void ApplyVelocityImpulse_WS( const CFVec3A &rVelocityImpulseVec_WS );

	virtual void InflictDamageResult( const CDamageResult *pDamageResult );

	void LosingPartGroup( void );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL _LoadBotClassResources( void );
	static void _UnloadBotClassResources( void );

	static void _AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	static BOOL _BuildAnimStackDef( void );
	static BOOL _SlashHitTrackerCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	static BOOL _PickRandomPlaceTrackerCallback1( CFWorldTracker *pTracker, FVisVolume_t *pVolume, const CFVec3 *pIntersectionPoint_WS, f32 fUnitDistToIntersection );
	static BOOL _PickRandomPlaceTrackerCallback2( CFWorldTracker *pTracker, FVisVolume_t *pVolume, const CFVec3 *pIntersectionPoint_WS, f32 fUnitDistToIntersection );

	BOOL _ShadowWork( void );
	void _StandardBotWork( void );
	void _ResetAnimSystem( void );

	void _InitInPieces( void );
	void _InitAlive( void );
	void _InitCommon( void );

	void _ConfigureBotToAlive( BOOL bMobilizeBot );
	void _ConfigureBotToForming( void );
	void _ConfigureBotToPieces( void );

	void _SetAutoWakeupTimer( f32 fAutoWakeupSecs );

	void _ClearDataMembers( void );
	void _UpdateEmissiveIntensity( void );

	void _PerformSlashDamage( const CFVec3A *pPrevSpherePos_WS, const CFVec3A *pCurrentSpherePos_WS );

	void _HandleParticles( void );
	void _ComputeBreathMatrix( CFMtx43A *pMtx );

	void _HandleFormingAnim( void );
	void _HandleDoubleJump( void );

	void _HandleHopping( void );

	void _HandleJumping( void );
	void _HandleJumpAnimations( void );
	void _EnterFlyMode( void );
	void _StartVelocityJump( const CFVec3A *pJumpVelocity_WS );
	void _StartSingleJump( const CFVec3A *pJumpVelocity_WS=NULL );
	void _StartDoubleJump( BOOL bAddToVelocityY=TRUE );
	void _JumpLanded( void );
	void _HandleTerminalState(void);
	void _SwitchPrimaryWeaponsInBackpack( void );
	void _HandleFiringAnimations( void );
	void _HandleAimAnimations( void );
	void _HandleWeaponAnimations( void );
	void _UpdateMatrices( void );

	void _InitStartSwingData( void );
	void _StartSlashMode( void );
	void _StopSlashMode( void );
	void _HandleSlashingSweetRange( f32 fPrevUnitTime, f32 fCurrentUnitTime, f32 fUnitTimeSweetStart, f32 fUnitTimeSweetEnd );
	void _AttachAnimAndSetBoneMaskToUpperBody( AnimTap_e nTapIndex, Anim_e nAnimIndex );
	void _HandleWeaponFiring( void );

	void _StartStreamer( void );
	void _KillStreamer( void );

	void _KillFormingSound( void );
	void _StartFormingSound( void );
	void _FadeOutFormingSound( void );
	void _FormingSoundWork( void );

	void _PlayClatterSound( const CFVec3A *pPos_WS, f32 fVolMultiplier=1.0f );
	void _PlayCrumbleApartSound( const CFVec3A *pPos_WS, f32 fVolMultiplier=1.0f );
	void _PlayBlowApartSound( const CFVec3A *pPos_WS, f32 fVolMultiplier=1.0f );
	void _PlayBreathPuffSound( const CFVec3A *pPos_WS, f32 fVolMultiplier=1.0f );

	void _InflictSlashDamage( CEntity *pHitEntity, CFWorldMesh *pHitWorldMesh, const FCollImpact_t *pCollImpact );
	void _SpawnSlashImpactEffects( const FCollImpact_t *pCollImpact );

	void _ScatterPieces( void );
	void _PickRandomStartingPlaceForPiece( const CFVec3A *pStartRayPos_WS, CFVec3A *pPos_WS, CFVec3A *pUnitFaceNormal_WS );
	BOOL _FindPieceStartingPlaceAlongRayXZ( const CFVec3A *pStartRayPos_WS, f32 fAngleOnXZPlane, CFVec3A *pPos_WS, CFVec3A *pUnitFaceNormal_WS );
	BOOL _TryRayAtGivenDistXZ( const CFVec3A *pStartRayPos_WS, const CFVec3A *pRayUnitDirXZ, f32 fRayDistXZ, CFVec3A *pPos_WS, CFVec3A *pUnitFaceNormal_WS );

	BOOL _DoesNormalBoundSphereIntersectAnotherBot( void );
	static BOOL _TrackerCollisionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume );


	friend class CBotZomBuilder;
	friend class CBotZomPartGroup;
	friend class CBotZomPart;


	FCLASS_STACKMEM_ALIGN( CBotZom );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotZomBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotZomBuilder : public CBotBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	BOOL m_bStartFormed;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CBotZomBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CBotZomBuilder );
} FCLASS_ALIGN_SUFFIX;





#endif


