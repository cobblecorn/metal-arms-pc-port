//////////////////////////////////////////////////////////////////////////////////////
// botswarmer.h - 
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
// 01/22/03	Miller	    Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _BOTSWARMER_H_
#define _BOTSWARMER_H_ 1

#include "fang.h"
#include "flinklist.h"
#include "bot.h"
#include "fdebris.h"

class CSwarmerFlock;
class CSpawnNet;
class CFWorldTracker;
struct FVisVolume_t;

FCLASS_ALIGN_PREFIX class CBotSwarmer : public CBot {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	typedef struct {
		FExplosion_GroupHandle_t hDeathExplosion;
		CFSoundGroup *pMoveSound;
		CFSoundGroup *pAttackSound;
        
	} BotInfo_Swarmer_t;

	// Enum for Pat
	typedef enum {
		BotTypeSwarmer,
		BotTypeVermin
	} BotType_e;

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
		ANIMCONTROL_WALK,
		ANIMCONTROL_STAND,
		ANIMCONTROL_JUMP,
		ANIMCONTROL_ATTACK,

		ANIMCONTROL_BASE_COUNT
	} AnimControl_e;


	// Animation taps:
	typedef enum {
		ANIMTAP_WALK,
		ANIMTAP_STAND,
		ANIMTAP_JUMP,
		ANIMTAP_ATTACK,

		ANIMTAP_BASE_COUNT
	} AnimTap_e;


	// Base animations:
	typedef enum {
		ANIM_WALK,
		ANIM_STAND,
		ANIM_JUMP,
		ANIM_ATTACK,

		ANIM_BASE_COUNT
	} Anim_e;


	// Bones:
	typedef enum {
		BONE_TORSO,
		BONE_LF_LEG,
		BONE_LB_LEG,
		BONE_RF_LEG,
		BONE_RB_LEG,

		BONE_COUNT,

		BONE_VERMIN_DUMMY = 0,
		BONE_COUNT_VERMIN,
	} Bone_e;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:


//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	enum {
		_SWARMER_STATE_NONE				= 0x000,
		_SWARMER_STATE_JUMPING			= 0x001,
		_SWARMER_STATE_GRAVITY			= 0x002,
		_SWARMER_STATE_SURFACE			= 0x004,
		_SWARMER_STATE_STOPPED			= 0x010,
		_SWARMER_STATE_CHANGE_SURFACE	= 0x020,
		_SWARMER_IS_SWARMER				= 0x040,
		_SWARMER_MISSED_ALL_RAYS		= 0x080,
		_SWARMER_STATE_SPIN				= 0x100,
	};

	static BOOL	m_bSystemInitialized;									// TRUE: InitSystem() has been called
	static BOOL m_bDataLoaded;

	static const CBotDef m_BotDef;										// Bot definition data
	static CBotAnimStackDef m_AnimStackDef;

	static CFCollInfo m_CollInfo;										// Used for collision detection
	static CFTrackerCollideRayInfo m_TrackerCollInfo;
	static CBotSwarmer *m_pThisBot;										// Used for collision detection

	static BotInfo_Gen_t		m_BotInfo_Gen;							// General bot info
	static BotInfo_MountAim_t	m_BotInfo_MountAim;						// Mount aim bot info
	static BotInfo_Walk_t		m_BotInfo_Walk;							// Walk bot info
	static BotInfo_Weapon_t		m_BotInfo_Weapon;						// Weapon bot info
	static BotInfo_Swarmer_t	m_BotInfo_Swarmer;						// Swarmer bot info

	static CFDebrisGroup*		m_pDeathDebrisGroup;
	
//	static const FGameData_TableEntry_t m_aBotInfoVocab_Swarmer[];
//	static const FGameDataMap_t m_aGameDataMap[];

	static cchar *m_apszBoneNameTable[BONE_COUNT];						// Array of bone names
	static cchar *m_apszBoneNameTableVermin[BONE_COUNT_VERMIN];			// Array of bone names
	static cchar *m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT];	// Array of base animation control names
	static cchar *m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT];			// Array of base animation tap names
	static cchar *m_apszIdleAnimNameTable[ANIM_IDLE_COUNT];				// Array of idle animation names
	static cchar *m_apszBaseAnimNameTable[ANIM_BASE_COUNT];				// Array of base animation names
	static const u8 m_nApproxEyePointBoneNameIndex;
	static const u8 *m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT];
	static const u8 *m_apnEnableBoneNameIndexTableForEachIdleTap[ANIM_IDLE_COUNT];
	static const u8 m_anEnableBoneNameIndexTableForSummer_Normal[];

	static CFAnimCombinerConfig::ConfigStack_t	m_aAnimCombinerConfigStack[];
	static CFAnimCombinerConfig::ConfigTap_t	m_aAnimCombinerConfigTaps[];
	static CFAnimCombiner::AttachList_t			m_aAnimAttach[];

	static CDamageProfile *m_pDamageProfile;
	static FParticle_DefHandle_t m_hSparkParticleDef;

	const CFVec3A *m_pGazeDir_WS;							// Points to the gaze direction vector (might not be unit in length)

	u32 m_uFlags;											// Flags with info about the bot

	f32 m_fTestAngle;										// Rotation for ray test angles
	f32 m_fUnitRot;											// Used to slerp between surfaces

	f32 m_fYawCurr;											// Current yaw
	f32 m_fYawPrev;											// Previous yaw
	f32 m_fUnitYaw;											// Used to animate between current and previous

	CFQuatA m_PrevQuat;										// Previous rotation quat
	CFQuatA m_CurrQuat;										// Current rotation quat

	CFVec3A m_SurfaceNormal;								// Surface normal we are on now
	CFVec3A m_SurfaceNormalPrev;							// Previous surface normal
	CFVec3A m_SurfaceUp;									// The vector to move forward along a surface
	CFVec3A m_SurfaceRight;									// The vector to move right along a surface

	CFVec3A m_MoveDirectionForward;							// Forward direction
	CFVec3A m_MoveDirectionRight;							// Right direction

	CFVec3A m_SlerpedUp;

	f32 m_fUnitWalkBlend;									// Blend into walk animation
	f32 m_fUnitAirBlend;									// Blend into jump animation
	f32 m_fUnitAttackBlend;									// Blend into attack animation
	f32 m_fUnitJump;										// Can be + and -

	f32 m_fTimeTillNextBite;								// Time until we bite again

	f32 m_fPauseChance;										// Percentage chance this swarmer will pause
	f32 m_fPauseSecsRemaining;								// When paused, the time remaining

	f32 m_fYawWobble;										// When jumping, wobbles the yaw around
	f32 m_fYawWobbleDir;									// -1.0 or 1.0f

	CBot *m_pBot;

	CFVec3A	m_vDeathRotAxis;
	f32		m_fDeathRotRate;
	CFVec3A m_vDeathJumpVel;
	f32		m_fDeathJumpTimer;

	//
	// AI data
	//
	enum
	{
		SWARMERFLAG_NONE		 = 0x0000,
		SWARMERFLAG_SWARMSTATE   = 0x0002
	};
	enum
	{
		MAX_SEP_NEIGHBORS = 5
	};
	CFVec3A SpawnLoc;		  //where the swarmer was spawned
	f32 m_fJumpTimeOut;		  //next time that swarmer will jump under normal circumstances
	FLink_t m_FlockLink;	  //links all swarmers in the world(aka flock)
	CSpawnNet* m_pSpawnNet;	  //ptr to the SpawnNet that this swarmer came from (could be null)
	CEntity* m_pEnemy;				// Current enemy
	u32 m_uEnemyGUID;				// Guid of my enemy (watch for it to change meaning that my enemy has died)
	f32 m_fFreqRand;				// Unit value changed frequently
	f32 m_fFreqRandTimeOut;			// Next time that freqRand will change
	f32 m_fLastBrainUpdateTime;		// the last time that this swarmer acted as the flocks brain.
	f32 m_fAddToWorldTime;			// time it was when the swarmer was added to the world
	f32 m_fNeighborEvalTimeOut;		// next time at which swarmer will re-evaluate its closest neightbors
	CBotSwarmer* m_apNeighbor[MAX_SEP_NEIGHBORS];	   //swarmers that are closest to me
	u16 m_uSwarmerFlags;
	u8 m_uNumNeighbors;
	u8 m_uNumHitsTaken;				// counter for how many times this swarmer has been damaged

	friend class CSwarmerFlock;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	CBotSwarmer();
	~CBotSwarmer();

	static BOOL InitSystem( void );
	static void UninitSystem( void );
	static BOOL InitLevel( void );
	static void UninitLevel( void );
	static void WorkSystem( void );
	static void CheckpointRestoreSystem(void);
	static void TestDirToRay(s32 nDir, CFVec3A* pRay);

	BOOL Create( s32 nPlayerIndex=-1, BOOL bInstallDataPort=FALSE, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL, BotType_e eBotType = BotTypeSwarmer );

	virtual f32 ComputeEstimatedGroundStopTime( void );
	virtual f32 ComputeEstimatedControlledStopTimeXZ( void );
	virtual f32 ComputeEstimatedControlledStopTimeY( void );

	virtual void CheckpointSaveSelect( s32 nCheckpoint );
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );

	virtual void ApplyVelocityImpulse_WS( const CFVec3A &rVelocityImpulseVec_WS );
	virtual void InflictDamageResult( const CDamageResult *pDamageResult );

	virtual void DetachFromParent( void );
	virtual void DebugRender( f32 fScreenTextX, f32 fScreenTextY );

	BOOL PositionForSpawn(const CFVec3A &StartPos, s32 nTestRayDirHint = -1);  //-1 means that this func will try 6 compass directions looking for a surface
	void StickToSurface( const CFVec3A &SurfaceNormal, const CFVec3A &SurfaceRight, const CFVec3A &SurfaceUp );
	virtual void Die( BOOL bSpawnDeathEffects = TRUE, BOOL bSpawnGoodies = TRUE );

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
	
	virtual void ClassHierarchyRelocated( void *pIdentifier );
	virtual void DeathWork( void );

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	FINLINE BOOL _IsJumping( void )				{ return ( m_uFlags & _SWARMER_STATE_JUMPING ); }
	FINLINE BOOL _IsGravity( void )				{ return ( m_uFlags & _SWARMER_STATE_GRAVITY ); }
	FINLINE BOOL _IsOnSurface( void )			{ return ( m_uFlags & _SWARMER_STATE_SURFACE ); }
	FINLINE BOOL _IsStopped( void )				{ return ( m_uFlags & _SWARMER_STATE_STOPPED ); }
	FINLINE BOOL _IsChangingSurface( void )		{ return ( m_uFlags & _SWARMER_STATE_CHANGE_SURFACE ); }
	FINLINE BOOL _IsSwarmer( void )				{ return ( m_uFlags & _SWARMER_IS_SWARMER ); }
	FINLINE BOOL _MissedAllRays( void )			{ return ( m_uFlags & _SWARMER_MISSED_ALL_RAYS ); }
	FINLINE BOOL _IsSpin( void )				{ return ( m_uFlags & _SWARMER_STATE_SPIN ); }

	FINLINE void _SetJumping( void )			{ FMATH_SETBITMASK( m_uFlags, _SWARMER_STATE_JUMPING ); }
	FINLINE void _SetGravity( void )			{ FMATH_SETBITMASK( m_uFlags, _SWARMER_STATE_GRAVITY ); }
	FINLINE void _SetOnSurface( void )			{ FMATH_SETBITMASK( m_uFlags, _SWARMER_STATE_SURFACE ); }
	FINLINE void _SetStopped( void )			{ FMATH_SETBITMASK( m_uFlags, _SWARMER_STATE_STOPPED ); }
	FINLINE void _SetChangingSurface( void )	{ FMATH_SETBITMASK( m_uFlags, _SWARMER_STATE_CHANGE_SURFACE ); }
	FINLINE void _SetSwarmer( void )			{ FMATH_SETBITMASK( m_uFlags, _SWARMER_IS_SWARMER ); }
	FINLINE void _SetMissedAllRays( void )		{ FMATH_SETBITMASK( m_uFlags, _SWARMER_MISSED_ALL_RAYS ); }
	FINLINE void _SetSpin( void )				{ FMATH_SETBITMASK( m_uFlags, _SWARMER_STATE_SPIN ); }

	FINLINE void _ClearJumping( void )			{ FMATH_CLEARBITMASK( m_uFlags, _SWARMER_STATE_JUMPING ); }
	FINLINE void _ClearGravity( void )			{ FMATH_CLEARBITMASK( m_uFlags, _SWARMER_STATE_GRAVITY ); }
	FINLINE void _ClearOnSurface( void )		{ FMATH_CLEARBITMASK( m_uFlags, _SWARMER_STATE_SURFACE ); }
	FINLINE void _ClearStopped( void )			{ FMATH_CLEARBITMASK( m_uFlags, _SWARMER_STATE_STOPPED ); }
	FINLINE void _ClearChangingSurface( void )	{ FMATH_CLEARBITMASK( m_uFlags, _SWARMER_STATE_CHANGE_SURFACE ); }
	FINLINE void _ClearSwarmer( void )			{ FMATH_CLEARBITMASK( m_uFlags, _SWARMER_IS_SWARMER ); }
	FINLINE void _ClearMissedAllRays( void )    { FMATH_CLEARBITMASK( m_uFlags, _SWARMER_MISSED_ALL_RAYS ); }
	FINLINE void _ClearSpin( void )				{ FMATH_CLEARBITMASK( m_uFlags, _SWARMER_STATE_SPIN ); }

	void _ClearDataMembers( void );
	BOOL _LoadData( void );

	void _UpdateMatrices( void );
	void _HandleAnimations( void );
	void _HandleAttack( void );
	void _HandleTranslation( void );
	void _SetupRay( CFVec3A *pStart, CFVec3A *pEnd, CFVec3A *pVelMove );
	void _HandleCollision( void );
	void _TestBiteHit( void );

	BOOL _AttachToBot( void );
	void _DetachFromBot( void );

	void _DoAIWork(BOOL bIsPaused, BOOL* pbAIWouldLikeToUnPausePlease);
	void _ResetAI(void);
	void _BuildWorldMatrix( CFMtx43A *pMtx, const CFVec3A& Pos_WS,  BOOL bRandDirection);


	static BOOL _CollRayWithTrackersCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume, const CFVec3 *pIntersectionPoint_WS, f32 fUnitDistToIntersection );
	static BOOL _TrackerCallback( CFWorldTracker *pTracker, FVisVolume_t *pWorldLeafNode );
	static BOOL _BuildAnimStackDef( void );

	friend class CBotSwarmerBuilder;
	friend class CESwarmerSpawner;

FCLASS_STACKMEM_ALIGN( CBotSwarmer );
} FCLASS_ALIGN_SUFFIX;


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotSwarmerBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotSwarmerBuilder : public CBotBuilder {
public:


//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CBotSwarmerBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CBotSwarmerBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif

