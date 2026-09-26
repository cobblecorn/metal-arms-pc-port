//////////////////////////////////////////////////////////////////////////////////////
// vehiclerat.h -
//
// Author: Chris MacDonald
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
// 11/07/02 MacDonald 	Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _VEHICLERAT_H_
#define _VEHICLERAT_H_ 1

#include "vehicle.h"
#include "fphysics.h"
#include "fparticle.h"
#include "eparticle.h"
#include "gcoll.h"
#include "site_BotWeapon.h"
#include "explosion.h"
#include "eparticle.h"

#define ENABLE_BOOST_FEATURE	0

FCLASS_ALIGN_PREFIX class CVehicleRat : public CVehicle
{
	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Definitions:
	//----------------------------------------------------------------------------------------------------------------------------------
public:
	typedef struct
	{
		CFMtx43A mFriction;					// friction matrix
		CFVec3A vCurrentPos;				// current WS position of sphere center
		CFVec3A vProposedPos;				// proposed WS position of sphere center (i.e. forms the projection of the sphere)
		const CGCollMaterial *pMaterial;	// type of surface at most recent collision
		f32 fRadius;						// radius of collision sphere
		u32 uFlags;
	} RatColPoint_t;

	enum
	{
		RATCOLPOINT_FLAG_ENABLE_COLLISION = 0x00000001,
		RATCOLPOINT_FLAG_TOUCHING_WATER   = 0x00000002,
	};

	enum
	{
		NUM_WHEELS = 6
	};

	enum
	{
		MAX_DIRT_PARTICLES = 6 
	};

	enum
	{
		MAX_CLOUD_PARTICLES = 2 
	};

	enum
	{
		MAX_COL_POINTS = 13
	};

	// also add entry to m_anTagPointBoneNameIndexArray in vehiclerat_data.cpp
	enum
	{
		TAG_POINT_INDEX_DUMMY = 0,
		TAG_POINT_INDEX_CHASSIS,
		TAG_POINT_INDEX_R_HATCH,
		TAG_POINT_INDEX_L_WHEEL_FRONT,
		TAG_POINT_INDEX_R_WHEEL_FRONT,
		TAG_POINT_INDEX_L_WHEEL_MIDDLE,
		TAG_POINT_INDEX_R_WHEEL_MIDDLE,
		TAG_POINT_INDEX_L_WHEEL_REAR,
		TAG_POINT_INDEX_R_WHEEL_REAR,
		TAG_POINT_COUNT,			// Number of tag points on VehicleRat
	};

	typedef enum
	{
		VEHICLERAT_STATE_DRIVER_UNOCCUPIED,
		VEHICLERAT_STATE_ENGINE_STARTING,
		VEHICLERAT_STATE_DRIVING,
		VEHICLERAT_STATE_ENGINE_STOPPING,
		VEHICLERAT_STATE_START_MOVE_DRIVER_TO_ENTRY_POINT,
		VEHICLERAT_STATE_MOVE_DRIVER_TO_ENTRY_POINT,
		VEHICLERAT_STATE_START_DRIVER_ENTER,
		VEHICLERAT_STATE_DRIVER_ENTERING,
		VEHICLERAT_STATE_START_DRIVER_EXIT,
		VEHICLERAT_STATE_DRIVER_EXITING,
		VEHICLERAT_STATE_DRIVER_EXIT_CAMERA,
		VEHICLERAT_STATE_START_UPSIDE_DOWN,
		VEHICLERAT_STATE_UPSIDE_DOWN,
		VEHICLERAT_STATE_START_FLIPPING_UPRIGHT,
		VEHICLERAT_STATE_FLIPPING_UPRIGHT,
	} VehicleRatState_e;

	typedef enum
	{
		VEHICLERAT_GUN_STATE_UNOCCUPIED,
		VEHICLERAT_GUN_STATE_START_GUNNER_TO_ENTRY_POINT,
		VEHICLERAT_GUN_STATE_MOVE_GUNNER_TO_ENTRY_POINT,
		VEHICLERAT_GUN_STATE_ENTERING,
		VEHICLERAT_GUN_STATE_POST_ENTER_WAIT,
		VEHICLERAT_GUN_STATE_OCCUPIED,
		VEHICLERAT_GUN_STATE_EXITING,
	} VehicleRatGunState_e;

	// indices into m_anBoneIndexWheel
	enum
	{
		WHEEL_BONE_ARRAY_L_FRONT_INDEX = 0,
		WHEEL_BONE_ARRAY_R_FRONT_INDEX,
		WHEEL_BONE_ARRAY_L_MIDDLE_INDEX,
		WHEEL_BONE_ARRAY_R_MIDDLE_INDEX,
		WHEEL_BONE_ARRAY_L_REAR_INDEX,
		WHEEL_BONE_ARRAY_R_REAR_INDEX,
	};

	// indices into m_anBoneIndexAxle
	enum
	{
		AXLE_BONE_ARRAY_L_FRONT_INDEX = 0,
		AXLE_BONE_ARRAY_R_FRONT_INDEX,
		AXLE_BONE_ARRAY_L_MIDDLE_INDEX,
		AXLE_BONE_ARRAY_R_MIDDLE_INDEX,
		AXLE_BONE_ARRAY_L_REAR_INDEX,
		AXLE_BONE_ARRAY_R_REAR_INDEX,
	};

	// indices into m_anBoneIndexHubcap
	enum
	{
		HUBCAP_BONE_ARRAY_L_FRONT_INDEX = 0,
		HUBCAP_BONE_ARRAY_R_FRONT_INDEX,
		HUBCAP_BONE_ARRAY_L_MIDDLE_INDEX,
		HUBCAP_BONE_ARRAY_R_MIDDLE_INDEX,
		HUBCAP_BONE_ARRAY_L_REAR_INDEX,
		HUBCAP_BONE_ARRAY_R_REAR_INDEX,
	};

	// indices into m_aColPoints and m_aRatColPoints array
	enum
	{
		COLPOINT_L_FRONT_WHEEL_INDEX = 0,
		COLPOINT_R_FRONT_WHEEL_INDEX,
		COLPOINT_L_MIDDLE_WHEEL_INDEX,
		COLPOINT_R_MIDDLE_WHEEL_INDEX,
		COLPOINT_L_REAR_WHEEL_INDEX,
		COLPOINT_R_REAR_WHEEL_INDEX,
		COLPOINT_L_FRONT_BUMPER_INDEX,
		COLPOINT_R_FRONT_BUMPER_INDEX,
		COLPOINT_TURRET_INDEX,
		COLPOINT_L_REAR_BUMPER_INDEX,
		COLPOINT_R_REAR_BUMPER_INDEX,
		COLPOINT_FRONT_BUMPER_INDEX,
		COLPOINT_REAR_BUMPER_INDEX,
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
	enum
	{
		ANIM_IDLE_COUNT = 0
	};

	enum
	{
		PART_INSTANCE_COUNT_PER_TYPE = 4,
	};
    
	typedef enum
	{
		LIMB_TYPE_CRATE_RIGHT_REAR,				// right rear crate
		LIMB_TYPE_COUNT
	} LimbType_e;


	// Animation controls:
	typedef enum
	{
		ANIMCONTROL_REST,
		ANIMCONTROL_BASE_COUNT
	} AnimControl_e;


	// Animation taps:
	typedef enum
	{
		ANIMTAP_REST,
		ANIMTAP_BASE_COUNT
	} AnimTap_e;


	// Base animations:
	typedef enum
	{
		ANIM_BASE_COUNT=0,
	} Anim_e;

	// Bones:
	typedef enum
	{
		BONE_RATDUMMY,
		BONE_CHASSIS,
		BONE_CRATE02,
		BONE_CRATE05,
		BONE_CRATE03,
		BONE_CRATE01,
		BONE_R_HATCH,
		BONE_L_HATCH,
		BONE_CRATE04,
		BONE_L_AXLE_REAR,
		BONE_L_HUBCAP_REAR,
		BONE_L_WHEEL_REAR,
		BONE_L_AXLE_MIDDLE,
		BONE_L_HUBCAP_MIDDLE,
		BONE_L_WHEEL_MIDDLE,
		BONE_L_AXLE_FRONT,
		BONE_L_HUBCAP_FRONT,
		BONE_L_WHEEL_FRONT,
		BONE_R_AXLE_REAR,
		BONE_R_HUBCAP_REAR,
		BONE_R_WHEEL_REAR,
		BONE_R_AXLE_MIDDLE,
		BONE_R_HUBCAP_MIDDLE,
		BONE_R_WHEEL_MIDDLE,
		BONE_R_AXLE_FRONT,
		BONE_R_HUBCAP_FRONT,
		BONE_R_WHEEL_FRONT,
		BONE_ATTACHPOINT_TURRET,
		BONE_ATTACHPOINT_DRIVER,
		BONE_ATTACHPOINT_GUNNER,
		BONE_COUNT,
	} Bone_e;

	enum
	{
		RAT_FLAG_FINAL_BONE_UPDATE				= 0x00000001,	// TRUE if this is the last anim bone callback this frame
		RAT_FLAG_DISABLE_COLLSION_ON_DEAD_RATS	= 0x00000002,	// TRUE if collision should be disabled when the rat dies
		RAT_FLAG_FLIP_RAT_ON_DEATH				= 0x00000004,	// TRUE if rat should flip over when it dies
		RAT_FLAG_FLIP_RAT_PROJECTILE_ONLY		= 0x00000008,	// TRUE if only projectile damage should cause rat to flip on death
		RAT_FLAG_DONT_KILL_DRIVER_ON_DIE		= 0x00000010,	// TRUE if driver should not be killed when rat dies
		RAT_FLAG_AI_STAY_BEHIND					= 0x00000020,	// TRUE if AI rat should try to stay behind the player rat
		RAT_FLAG_AI_STAY_BESIDE					= 0x00000040,	// TRUE if AI rat should try to stay beside the player rat
		RAT_FLAG_FLIP							= 0x00000080,	// TRUE if rat should be flipped in air this frame
		RAT_FLAG_ZOBBY_DRIVING					= 0x00000100,	// TRUE if Zobby mesh should appear in driver seat
		RAT_FLAG_ZOBBY_GUNNING					= 0x00000200,	// TRUE if Zobby mesh should appear in gunner seat

		RAT_FLAG_NONE							= 0x00000000
	};

	typedef enum
	{
		HATCH_STATE_CLOSED,
		HATCH_STATE_OPENING,
		HATCH_STATE_OPEN,
		HATCH_STATE_CLOSING,
	} HatchState_e;

	typedef enum
	{
		RAT_TYPE_MIL = 0,
		RAT_TYPE_DROID,
		RAT_TYPE_COUNT,
	};

	typedef struct
	{
		CFSoundGroup *pSoundGroupHatchClank;	// driver hatch open/close sound
		CFSoundGroup *pSoundGroupBoost;			// boosting sound
		CFSoundGroup *pSoundGroupSkid;			// tire skid sound
	} BotInfo_Rat_t;

	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Data:
	//----------------------------------------------------------------------------------------------------------------------------------
public:
	static BotInfo_VehiclePhysics_t m_BotInfo_VehiclePhysics;// Vehicle physics data
	static CBot *m_pCurrentCollider;						// current vehicle colliding, used by callback passed to physics object
	static u32 m_anBoneIndexWheel[RAT_TYPE_COUNT][ NUM_WHEELS ];	// bone indices of wheels
	static u32 m_anBoneIndexAxle[RAT_TYPE_COUNT][ NUM_WHEELS ];		// bone indices of axles
	static u32 m_anBoneIndexHubcap[RAT_TYPE_COUNT][ NUM_WHEELS ];	// bone indices of "hubcaps" (wheel vertical support arm)
	static u32 m_nBoneIdxDriverAttach[RAT_TYPE_COUNT];		// bone index of bot driving position
	static u32 m_nBoneIdxGunnerAttach[RAT_TYPE_COUNT];		// bone index of gunning position (or -1 if vehicle doesn't have it)
	static u32 m_nBoneIndexRightHatch[RAT_TYPE_COUNT];		// bone index of driver's hatch door
	static u32 m_nBoneIndexTurretAttach[RAT_TYPE_COUNT];	// bone index for attaching Rat Gun to Rat.
	static CFCollData m_RatCollData;						// new collision info struct used for rat collision detection
	static CFSphere m_RatColSphere;							// sphere used for rat collision detection
	f32 m_afWheelAngle[ NUM_WHEELS ];						// animation rotation angle of wheels
	RatColPoint_t m_aRatColPoints[MAX_COL_POINTS];			// collision point array used by rat collision detection
	CFPhysicsObject::CollisionPoint_t m_aColPoints[MAX_COL_POINTS];	// collision point array passed to rat's physics object
	static FParticle_DefHandle_t m_hDirtParticleDef;		// handle to tire dirt spray particle effect
	static FParticle_DefHandle_t m_hCloudParticleDef;		// handle to dust cloud particle effect
	static FParticle_DefHandle_t m_hWaterParticleDef;		// handle to water particle effect
	FParticle_EmitterHandle_t m_hDirtParticle[MAX_DIRT_PARTICLES];	// handles to particle emitters for tire dirt spray effect
	FParticle_EmitterHandle_t m_hCloudParticle[MAX_CLOUD_PARTICLES];// handle to particle emitter for dust cloud effect
	CFVec3A m_vDirtParticlePosition[MAX_DIRT_PARTICLES];	// position vector for dirt particle emitters
	CFVec3A m_vDirtParticleDirection[MAX_DIRT_PARTICLES];	// direction vector for dirt particle emitters
	CFVec3A m_vDirtParticleVelocity[MAX_DIRT_PARTICLES];	// velocity vector for dirt particle emitters
	f32 m_fDirtParticleIntensity[MAX_DIRT_PARTICLES];		// unit intensity for dirt particle emitters
	CFVec3A m_vCloudParticlePosition[MAX_CLOUD_PARTICLES];	// position vector for dust cloud emitter
	CFVec3A m_vCloudParticleDirection[MAX_CLOUD_PARTICLES];	// direction vector for dust cloud emitter
	CFVec3A m_vCloudParticleVelocity[MAX_CLOUD_PARTICLES];	// velocity vector for dust cloud emitter
	f32 m_fCloudParticleIntensity[MAX_CLOUD_PARTICLES];		// unit intensity for dust cloud particle emitter
	CBot *m_pGunnerBot;										// pointer to bot currently occupying gun
	static BOOL m_bLoadingDroidRat;							// flag to tell rat gun to load droid mesh
	f32 m_fInterVehicleDamageTimer;							// time between submitting damage when vehicles are colliding
	f32 m_fBoomerDamageTimer;								// time between submitting damage to boomer objects (and slowing down vehicle)
	f32 m_fMomentumReduction;								// used to reduce vehicle momentum after hitting boomer object
	f32 m_fWaterY;											// height of most recent water impact

	// moved here so that Lose Your Ground could gain access to these to overwrite them
	static FExplosion_GroupHandle_t m_hBigFinalExplosion;	// Huge Final Rat Explosion
	static FExplosion_GroupHandle_t m_hExplosionGroup;		// all other RAT destruction explosion group handle	

	//----------------------------------------------------------------------------------------------------------------------------------
	// Private Data:
	//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL	 m_bSystemInitialized;						// TRUE: InitSystem() has been called
	static const CBotDef m_BotDef;							// Bot definition data
	static BotInfo_Gen_t m_BotInfo_Gen;						// General bot info
	static BotInfo_Vehicle_t m_BotInfo_Vehicle;				// Vehicle bot info
	static BotInfo_Rat_t m_BotInfo_Rat;						// Rat specific data
	static BotInfo_Engine_t m_BotInfo_Engine;				// engine specific data
	static CVehicleCamera::BotInfo_VehicleCamera_t m_BotInfo_DriverCamera;		// single player driver camera parameters
	static CVehicleCamera::BotInfo_VehicleCamera_t m_BotInfo_MPDriverCamera;	// Multiplayer driver camera parameters
	static CVehicleCamera::BotInfo_VehicleCamera_t m_BotInfo_GunnerCamera;		// Gunner camera parameters for non-driveable rat guns
	static CVehicleCamera::BotInfo_VehicleCamera_t m_BotInfo_MGGunnerCamera;	// Gunner camera parameters
	static const FGameData_TableEntry_t m_aBotInfoVocab_Rat[];
	static const FGameDataMap_t m_aGameDataMap[];			// Vocabulary map used to parse through user props
	static u32 m_nBotClassClientCount;						// Number of bots of this class using the class-wide resources
	static FMesh_t *m_pRatMesh;								// pointer to rat mesh
	static FMesh_t *m_pDroidRatMesh;						// pointer to droid mesh  (only loads on rat race level)
	static FMesh_t *m_pDeadRatMesh;							// pointer to destroyed rat mesh
	static FMesh_t *m_pDeadDroidRatMesh;					// pointer to destroyed droid rat mesh (only loads on rat race level)
	static SmokeTrailAttrib_t m_SmokeTrailAttrib;			// Smoke trail attributes for wheels

	// Bot animation stack data:
	static CBotAnimStackDef m_AnimStackDef;					// Animation stack definition data

	static cchar *m_apszBoneNameTable[BONE_COUNT];						// Array of bone names
	static cchar *m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT];	// Array of base animation control names
	static cchar *m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT];			// Array of base animation tap names
	static const u8 *m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT];
	static const u8 m_anEnableBoneNameIndexTableForSummer_Normal[];
	static const u8 m_anTagPointBoneNameIndexArray[TAG_POINT_COUNT];
	static const u8 m_nApproxEyePointBoneNameIndex;

	static CFAnimCombinerConfig::ConfigStack_t	m_aAnimCombinerConfigStack[];
	static CFAnimCombinerConfig::ConfigTap_t	m_aAnimCombinerConfigTaps[];
	static CFAnimCombiner::AttachList_t			m_aAnimAttach[];

	static f32 m_fAxleLength;								// length of suspension arm

	// Bot parts:
	static CBotPartPool *m_pPartPool;						// One part pool per bot class

	static FSndFx_FxHandle_t m_hEngineStartSFX;				// sfx handle
	static FSndFx_FxHandle_t m_hEngineStopSFX;				// sfx handle
	static FSndFx_FxHandle_t m_hSkidSFX;					// sfx handle

	CFVec3A m_vFlipDir;										// direction to flip rat in air
	CFVec3A m_vLeftTurretWaterPos;							// position of turret water particle emitter
	CFVec3A m_vRightTurretWaterPos;

	CCameraTrans m_GunnerCameraTrans;						// camera transition object for gunner
	CVehicleCamera m_GunnerCamera;							// vehicle camera object for gunner

	CFAnimManMtx *m_pManMtx;
	CFAnimMeshRest *m_pRestMtx;

	u32 m_uRatFlags;										// flags pertaining to Rat events & states
	u32 m_nTagPointCount;									// Number of tag points
	const CFVec3A *m_apTagPoint_WS[TAG_POINT_COUNT];		// Tag points in world space

	VehicleRatState_e m_eRatState;							// state of vehicle
	VehicleRatGunState_e m_eGunState;						// state of vehicle gun
	f32 m_fGunStateTimer;									// timer for gun entry/exit states

	f32 m_fRawSteeringPosition;								// current unmodified steering angle of tires, values from -1.0 to 1.0
	f32 m_fSteeringPosition;								// final steering angle of tires (scaled by vehicle velocity, etc)
	f32 m_fFlipOverTime;									// remaining time to apply force when flipping over to return to right-side-up.

	FMeshTexLayerHandle_t m_hBrakeLightLayer;				// handle to brake light texture
	FMeshTexLayerHandle_t m_hReverseLightLayer;				// handle to reverse light texture
	FMeshTexLayerHandle_t m_hTireLayer;						// handle to tire texture
	CBotSiteWeapon m_RatGun;

	f32 m_fGunEntryJumpTimer;								// time remaining for gunner entry jump
	f32 m_fDeathWorkTimer;									// timer for blowing up Rat part-by-part
	s32 m_nDeathEvents;										// number of explosions, wheel destructions, etc. spawned so far
	CEParticle *m_pEParticleSmoke;							// damaged smoke emitter
	CFWorldMesh *m_pRatWorldMesh;							// pointer to allocated RAT CFWorldMesh
	CFWorldMesh *m_pDeadRatWorldMesh;						// pointer to allocated dead RAT CFWorldMesh 
	f32 m_fRightHatchUnitPos;								// unit position of driver's hatch 0.0 = open, 1.0 = close
	HatchState_e m_eRightHatchState;						// state of driver's hatch
	u32 m_uRatType;											// type of rat, see RAT_TYPE_*
	CMeshEntity *m_pZobbyME;								// pointer to zobby mesh entity for when he's driving/gunning
#if ENABLE_BOOST_FEATURE
	f32 m_fBoostTimer;										// speed boost timer.  > 0.0f = time remaining to boost. < 0.0f time remaining to recharge boost
	f32 m_fUnitBoost;										// unitized boost/recovery time for HUD meter
#endif
	CArmorProfile *m_pPreviousGunnerArmorProfile;			// Armor profile of gunner before entering vehicle

	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
public:
	virtual BOOL ClassHierarchyLoadSharedResources( void );
	virtual void ClassHierarchyUnloadSharedResources( void );

	CVehicleRat();
	~CVehicleRat();

	static BOOL InitSystem( void );
	static void UninitSystem( void );

	virtual u32 GetTagPointCount( void ) const { return m_nTagPointCount; }
	virtual const CFVec3A *GetTagPoint( u32 nTagPointIndex ) const;
	virtual const CFVec3A *GetApproxEyePoint( void ) const;

	BOOL Create( s32 nPlayerIndex=-1, BOOL bInstallDataPort=FALSE, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);
	BOOL ActionNearby( CEntity *pEntity );

	// checkpoint load/save functions
	virtual void CheckpointSaveSelect( s32 nCheckpoint );
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );
	virtual void CheckpointRestorePostamble( void );

	// anim bone callback functions
	void AnimateAxles( u32 uAxleIndex, u32 uBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	void AnimateWheels( u32 uWheelIndex, u32 uBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	void AnimateHubcaps( u32 uWheelIndex, u32 uBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	void AnimateHatch( u32 uBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );

	virtual void ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS );
	virtual f32 ComputeEstimatedControlledStopTimeXZ( void );
	virtual f32 ComputeEstimatedMinimumTurnRadius( f32 fVelocity );

	FINLINE CBotSiteWeapon *RatGun( void ) { return &m_RatGun; }

	CBot *IsStationObstructed( Station_e eStation );
	BOOL IsInStationEntryArea( CBot *pBot, Station_e eStation );
	StationStatus_e CanOccupyStation( CBot *pBot, Station_e eStation );
	StationStatus_e EnterStation( CBot *pBot, Station_e eStation, BOOL bProximityCheck, BOOL bImmediately );
	StationStatus_e ExitStation( CBot *pBot, BOOL bImmediately );
	BOOL GetStationEntryPoint( Station_e eStation, CFVec3A *pPoint );
	virtual FINLINE CBot *GetGunnerBot( void ) const { return m_pGunnerBot; };

	FINLINE BOOL IsUpsideDown( void ) const { return m_eRatState == VEHICLERAT_STATE_UPSIDE_DOWN; };
	FINLINE BOOL IsFlippingUpright( void ) const { return m_eRatState == VEHICLERAT_STATE_FLIPPING_UPRIGHT; };

	void FlipRightsideUp( void );
	virtual f32 ComputeMinimumManeuveringSpeed( void );
	FINLINE void SetFinalBoneUpdate( BOOL bFinal ) { if( bFinal ) m_uRatFlags |= RAT_FLAG_FINAL_BONE_UPDATE; else m_uRatFlags &= ~RAT_FLAG_FINAL_BONE_UPDATE; };
	FINLINE BOOL IsFinalBoneUpdate( void ) const { return( !!(m_uRatFlags & RAT_FLAG_FINAL_BONE_UPDATE) ); };

	virtual void Die( BOOL bSpawnDeathEffects=TRUE, BOOL bSpawnGoodies=TRUE );
	virtual void UnDie(void);

	void InflictDamage( CDamageData *pDamageData );
	void InflictDamageResult( const CDamageResult *pDamageResult );
	
	// assumes that the rat is currently out the of the world
	// relocates the vehicle to the new location
	// puts the vehicle back into the world
	// inits everything so that this vehicle will not remember anything from his past life.
	// it is assumed that a driver will be placed nearby and that he has been configured to hop into the vehicle
	void RecycleVehicle( CFMtx43A &rNewLocation, CESpline *pSpline, u32 nEnemyGUID, f32 fUnitMaxSpeed );

	// Returns TRUE if the rat is being run through the real physics system every frame,
	// FALSE if the physics simulator isn't running (the bot is probably dead and just sitting on the ground)
	BOOL IsRealPhysicsRunning( void );

	// used by hold your ground to disable the collision on both the actual and damaged meshes
	void DisableAllMeshCollisionsIfDeadOrDying( void );

	// used to destroy all dust and cloud particles
	void DestroyDustAndCloudParticles( void );

	// allows a different death smoke to be used by all loaded rats on the current level
	static void OverWriteDeathSmoke( FParticle_DefHandle_t hNewSmokeHandle );

	// rat flips in air on death if enabled.
	void SetRatFlipOnDeath( BOOL bEnable, BOOL bProjectileOnly = FALSE );

	void SetRatTargetable( BOOL bEnable );

	void DisableDriverKillingOnDeath( void );
	
	FINLINE BOOL AIShouldStayBehind( void ) const { FASSERT( IsCreated() ); return( !!(m_uRatFlags & RAT_FLAG_AI_STAY_BEHIND) ); };
	FINLINE BOOL AIShouldStayBeside( void ) const { FASSERT( IsCreated() ); return( !!(m_uRatFlags & RAT_FLAG_AI_STAY_BESIDE) ); };

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
	virtual void ClassHierarchyBecameActive( void );
	virtual void ClassHierarchyBecameInactive( void );

	void VelocityHasChanged( void );

	void DriverEnter( CBot *pDriverBot, cchar *pszAttachPointBoneName = NULL, BOOL bImmediately = FALSE );
	void DriverExit( CBot* pDriverBot );
	void UpdateDriverCamera( void );
	void UpdateGunnerCamera( void );
	void StartTransitionToGunnerVehicleCamera( s32 nPlayerIndex );
	void StartTransitionToGunnerBotCamera( void );
	void MoveVehicle( void );

	void CrankEngine( void );
	void StartEngine( void );
	void StopEngine( BOOL bImmediately = FALSE );
	void RunEngine( void );

	void DeathWork( void );

	void SetHatchOpen( BOOL bOpen, BOOL bImmediately );
	void ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled );

	//----------------------------------------------------------------------------------------------------------------------------------
	// Private Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
private:
	static	BOOL _BuildAnimStackDef( void );

	void _ClearDataMembers( void );

	static void _AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );

	void _InitPhysicsObject( void );
	void _ResetSprings( void );
	void _UpdateTireTextures( void );
	void _UpdateEngineSounds( void );
	void _UpdateCollisionFlags( void );
	void _BlowOffRandomWheel( void );
	void _RandomExplosionOnBody( void );
	void _BigExplosion( void );
	void _StartDriverEnterWork( BOOL bImmediately );
	void _StartGunnerEnterWork( BOOL bImmediately );
	void _AbortDriverEntry( void );
	void _SquishBots( void );
	void _UpdateRatState( void );
	void _UpdateRatGunState( void );
	void _UpdateRatGun( void );
	void _UpdateDamageSmoke( void );
	void _UpdateDustCloud( void );
	void _UpdateAISound( void );
	void _UpdateDriverControls( void );
	void _UpdateSkidSound( void );
	void _AllocateEngineEmitters( void );
#if ENABLE_BOOST_FEATURE
	void _UpdateBoost( void );
	void _SetBoostMeter( BOOL bRecovering );
#endif
	void _UpdateWaterEffects( void );

	friend class CVehicleRatBuilder;

	FCLASS_STACKMEM_ALIGN( CVehicleRat );
} FCLASS_ALIGN_SUFFIX;



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CVehicleRatBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CVehicleRatBuilder : public CVehicleBuilder 
{
public:

	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CVehicleRatBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );
	u32 m_uFlags;

#define RAT_BUILDER_FLAG_DROID_MESH			0x00000001
#define RAT_BUILDER_FLAG_DEATH_FLIP			0x00000002
#define RAT_BUILDER_FLAG_AI_STAY_BEHIND		0x00000004
#define RAT_BUILDER_FLAG_AI_STAY_BESIDE		0x00000008
#define RAT_BUILDER_FLAG_ZOBBY_DRIVER		0x00000010
#define RAT_BUILDER_FLAG_ZOBBY_GUNNER		0x00000020

	//----------------------------------------------------------------------------------------------------------------------------------
	// Protected Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CVehicleRatBuilder );
} FCLASS_ALIGN_SUFFIX;
#endif

