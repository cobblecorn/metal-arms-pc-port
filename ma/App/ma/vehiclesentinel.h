//////////////////////////////////////////////////////////////////////////////////////
// vehiclesentinel.h -
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

#ifndef _VEHICLESENTINEL_H_
#define _VEHICLESENTINEL_H_ 1

#include "vehicle.h"
#include "fphysics.h"
#include "fparticle.h"
#include "fAntenna.h"
#include "eparticle.h"
#include "tracer.h"
#include "gcoll.h"
#include "fforce.h"
#include "reticle.h"
#include "explosion.h"
#include "fdecal.h"


FCLASS_ALIGN_PREFIX class CVehicleSentinel : public CVehicle
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
	} SentinelColPoint_t;

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

	// Tag Point indices
	// also add entry to m_anTagPointBoneNameIndexArray in vehiclesentinel_data.cpp
	enum
	{
		TAG_POINT_INDEX_DUMMY = 0,
		TAG_POINT_INDEX_TORSO,
		TAG_POINT_INDEX_L_WHEEL_A,
		TAG_POINT_INDEX_R_WHEEL_A,
		TAG_POINT_INDEX_L_WHEEL_C,
		TAG_POINT_INDEX_R_WHEEL_C,
		TAG_POINT_INDEX_L_WHEEL_E,
		TAG_POINT_INDEX_R_WHEEL_E,
		TAG_POINT_INDEX_PRIMARY_FIRE,
		TAG_POINT_INDEX_SECONDARY_FIRE,
		TAG_POINT_INDEX_TURRET,
		TAG_POINT_INDEX_ATTACHPOINT_DRIVER,
		TAG_POINT_INDEX_COLLBONE_R1,
		TAG_POINT_INDEX_COLLBONE_L1,
		TAG_POINT_COUNT,					// Number of tag points on VehicleSentinel
	};

	typedef enum
	{
		VEHICLESENTINEL_STATE_UNOCCUPIED,
		VEHICLESENTINEL_STATE_ENGINE_STARTING,
		VEHICLESENTINEL_STATE_DRIVING,
		VEHICLESENTINEL_STATE_ENGINE_STOPPING,
		VEHICLESENTINEL_STATE_START_MOVE_DRIVER_TO_ENTRY_POINT,
		VEHICLESENTINEL_STATE_MOVE_DRIVER_TO_ENTRY_POINT,
		VEHICLESENTINEL_STATE_START_DRIVER_ENTER,
		VEHICLESENTINEL_STATE_DRIVER_ENTERING,
		VEHICLESENTINEL_STATE_START_DRIVER_EXIT,
		VEHICLESENTINEL_STATE_DRIVER_EXITING,
		VEHICLESENTINEL_STATE_START_UPSIDE_DOWN,
		VEHICLESENTINEL_STATE_UPSIDE_DOWN,
		VEHICLESENTINEL_STATE_START_FLIPPING_UPRIGHT,
		VEHICLESENTINEL_STATE_FLIPPING_UPRIGHT,
	} VehicleSentinelState_e;

	// indices into m_anBoneIndexWheel
	enum
	{
		WHEEL_BONE_ARRAY_L_A_INDEX = 0,
		WHEEL_BONE_ARRAY_R_A_INDEX,
		WHEEL_BONE_ARRAY_L_C_INDEX,
		WHEEL_BONE_ARRAY_R_C_INDEX,
		WHEEL_BONE_ARRAY_L_E_INDEX,
		WHEEL_BONE_ARRAY_R_E_INDEX,
		NUM_WHEELS,
		// non colliding wheels below
		WHEEL_BONE_ARRAY_L_B_INDEX = NUM_WHEELS,
		WHEEL_BONE_ARRAY_R_B_INDEX,
		WHEEL_BONE_ARRAY_L_D_INDEX,
		WHEEL_BONE_ARRAY_R_D_INDEX,
		NUM_ALL_WHEELS,
	};

	// indices into m_aColPoints and m_aSentinelColPoints array
	enum
	{
		COLPOINT_L_A_WHEEL_INDEX = 0,
		COLPOINT_R_A_WHEEL_INDEX,
		COLPOINT_L_C_WHEEL_INDEX,
		COLPOINT_R_C_WHEEL_INDEX,
		COLPOINT_L_E_WHEEL_INDEX,
		COLPOINT_R_E_WHEEL_INDEX,
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
		LIMB_TYPE_BOX_A_LEFT_FRONT,				// left front box, "A"
		LIMB_TYPE_BOX_B_LEFT_REAR,				// left rear box, "B"
		LIMB_TYPE_BOX_C_RIGHT_FRONT,			// right front box, "C"
		LIMB_TYPE_BOX_D_RIGHT_REAR,				// right rear box, "D"
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
		BONE_TANKDUMMY,
		BONE_TORSO,
		BONE_R_WHEEL_E,
		BONE_L_WHEEL_E,
		BONE_L_WHEEL_D,
		BONE_L_WHEEL_C,
		BONE_L_WHEEL_B,
		BONE_L_WHEEL_A,
		BONE_R_WHEEL_D,
		BONE_R_WHEEL_A,
		BONE_R_WHEEL_B,
		BONE_R_WHEEL_C,
		BONE_TURRET,
		BONE_CANNONBASE,
		BONE_CANNON,
		BONE_PRIMARY_FIRE,
		BONE_MACHINEGUN,
		BONE_SECONDARY_FIRE,
		BONE_SPOTLIGHT,
		BONE_ATTACHPOINT_LITE,
		BONE_L_JOYSTICK,
		BONE_R_JOYSTICK,
		BONE_ATTACHPOINT_DRIVER,
		BONE_STUFF_D,
		BONE_STUFF_C,
		BONE_STUFF_A,
		BONE_STUFF_B,
		BONE_COLLBONE_R1,
		BONE_COLLBONE_L1,
		BONE_ANTENNA1,
		BONE_ANTENNA2,
		BONE_ANTENNA3,
		BONE_ANTENNA4,
		BONE_ANTENNA5,

		BONE_COUNT,
	} Bone_e;

#define SENTINEL_FLAG_DONT_CALL_BOTPARTMANAGER		( 0x00000001 )

	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Data:
	//----------------------------------------------------------------------------------------------------------------------------------
public:
	static BotInfo_VehiclePhysics_t m_BotInfo_VehiclePhysics;// Vehicle physics data
	static CBot *m_pCurrentCollider;						// current vehicle colliding, used by callback passed to physics object
	static u32 m_anBoneIndexWheel[ NUM_ALL_WHEELS ];		// bone indices of wheels
	static u32 m_nBoneIdxDriverAttach;						// bone index of bot driving position
	static u32 m_nBoneIndexTurret;							// bone index of turret
	static u32 m_nBoneIndexCannonBase;						// bone index of cannon base
	static u32 m_nBoneIndexCannon;							// bone index of cannon barrel
	static u32 m_nBoneIndexSpotLight;						// bone index of spotlight
	static u32 m_nBoneIndexPrimaryFire;						// bone index of cannon fire point
	static u32 m_nBoneIndexSecondaryFire;					// bone index of machine gun fire point

	f32 m_afWheelAngle[ NUM_ALL_WHEELS ];					// animation rotation angle of wheels
	static CFCollData m_SentinelColData;					// new collision info struct used for vehicle collision detection
	static CFSphere m_SentinelColSphere;					// sphere used for vehicle collision detection
	SentinelColPoint_t m_aSentinelColPoints[MAX_COL_POINTS];		// collision point array used by vehicle collision detection
	CFPhysicsObject::CollisionPoint_t m_aColPoints[MAX_COL_POINTS];	// collision point array passed to vehicle's physics object
	static FParticle_DefHandle_t m_hDirtParticleDef;		// handle to tire dirt spray particle effect
	static FParticle_DefHandle_t m_hCloudParticleDef;		// handle to dust cloud particle effect
	static FParticle_DefHandle_t m_hSmokeParticleDef;		// handle to muzzle smoke effect
	FParticle_EmitterHandle_t m_hDirtParticle[MAX_DIRT_PARTICLES];	// handles to particle emitters for tire dirt spray effect
	FParticle_EmitterHandle_t m_hCloudParticle[MAX_CLOUD_PARTICLES];// handle to particle emitter for dust cloud effect
	FParticle_EmitterHandle_t m_hSmokeParticle;				// handle to particle emitter for muzzle smoke effect
	CFVec3A m_vDirtParticlePosition[MAX_DIRT_PARTICLES];	// position vector for dirt particle emitters
	CFVec3A m_vDirtParticleDirection[MAX_DIRT_PARTICLES];	// direction vector for dirt particle emitters
	CFVec3A m_vDirtParticleVelocity[MAX_DIRT_PARTICLES];	// velocity vector for dirt particle emitters
	f32 m_fDirtParticleIntensity[MAX_DIRT_PARTICLES];		// unit intensity for dirt particle emitters
	CFVec3A m_vCloudParticlePosition[MAX_CLOUD_PARTICLES];	// position vector for dust cloud emitter
	CFVec3A m_vCloudParticleDirection[MAX_CLOUD_PARTICLES];	// direction vector for dust cloud emitter
	CFVec3A m_vCloudParticleVelocity[MAX_CLOUD_PARTICLES];	// velocity vector for dust cloud emitter
	f32 m_fCloudParticleIntensity[MAX_CLOUD_PARTICLES];		// unit intensity for dust cloud particle emitter

	//----------------------------------------------------------------------------------------------------------------------------------
	// Private Data:
	//----------------------------------------------------------------------------------------------------------------------------------
private:
	CFAnimManMtx *m_pManMtx;
	CFAnimMeshRest *m_pRestMtx;

	static BOOL	 m_bSystemInitialized;						// TRUE: InitSystem() has been called
	static const CBotDef m_BotDef;							// Bot definition data
	static BotInfo_Gen_t m_BotInfo_Gen;						// General bot info
	static BotInfo_Vehicle_t m_BotInfo_Vehicle;				// Vehicle bot info
	static BotInfo_Engine_t m_BotInfo_Engine;				// engine specific data
	static CVehicleCamera::BotInfo_VehicleCamera_t m_BotInfo_DriverCamera;		// single player driver camera parameters
	static CVehicleCamera::BotInfo_VehicleCamera_t m_BotInfo_MPDriverCamera;	// multiplayer driver camera parameters
	static const FGameDataMap_t m_aGameDataMap[];			// Vocabulary map used to parse through user props
	static u32 m_nBotClassClientCount;						// Number of bots of this class using the class-wide resources
	static FExplosion_GroupHandle_t m_hShellExplosion;		// Shell explosion handle
	static FMesh_t *m_pSentinelMesh;						// pointer to Sentinel mesh

	// Bot animation stack data:
	static CBotAnimStackDef m_AnimStackDef;					// Animation stack definition data

	// bones
	static cchar *m_apszBoneNameTable[BONE_COUNT];						// Array of bone names
	static cchar *m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT];	// Array of base animation control names
	static cchar *m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT];			// Array of base animation tap names
	static const u8 *m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT];
	static const u8 m_anEnableBoneNameIndexTableForSummer_Normal[];
	static const u8 m_anTagPointBoneNameIndexArray[TAG_POINT_COUNT];
	static const u8 m_nApproxEyePointBoneNameIndex;

	static f32 m_afStationEntryPoints[NUM_STATIONS][3];		// array of local space vectors containing entry points to vehicle stations

	// anim
	static CFAnimCombinerConfig::ConfigStack_t	m_aAnimCombinerConfigStack[];
	static CFAnimCombinerConfig::ConfigTap_t	m_aAnimCombinerConfigTaps[];
	static CFAnimCombiner::AttachList_t			m_aAnimAttach[];

	// flags & states
	VehicleSentinelState_e m_eSentinelState;				// state of vehicle

	// tag points
	u32 m_nTagPointCount;									// Number of tag points
	const CFVec3A *m_apTagPoint_WS[TAG_POINT_COUNT];		// Tag points in world space

	// movement
	f32 m_fSteeringPosition;								// current steering angle of tires, values from -1.0 to 1.0
	f32 m_fFlipOverTime;									// remaining time to apply force when flipping over to return to right-side-up.
	CFVec3A m_fFlipArm;										// direction (and magnitude) of arm to flip vehicle rightside up


	FMeshTexLayerHandle_t m_hTreadTexLayerR;				// handle to right tread texture
	FMeshTexLayerHandle_t m_hTreadTexLayerL;				// handle to left tread texture
	f32 m_fTreadUnitAnimPosR;								// right tread texture animation position
	f32 m_fTreadUnitAnimPosL;								// left tread texture animation position

	// weapons
	f32 m_fCannonReloadTime;								// time remaining to reload cannon
	f32 m_fMachineGunReloadTime;							// time remaining to reload machine gun
	FForceHandle_t m_hForce;								// Force feedback handle so we can kill it when we need to
	CFVec3A m_vTurretRecoil;								// vector along which turret is recoiling
	CFVec3A m_vTurretXZ;									// unit facing direction of turret in XZ plane
	f32 m_fTurretAngleWS;									// world space yaw angle of turret in radians, set by driving controls.
	f32 m_fTurretUnitRot;									// local space bipolar unit rotation of turret 0.0=forward, + = right, - = left
	f32 m_fCannonElevation;									// pitch angle of main gun in radians. 0.0f = parallel to ground, + = aim down
	static _TracerDef_s m_CannonTracerDef;					// cannon tracer def
	static CFTexInst m_CannonTracerTexInst;					// cannon tracer texture
	TracerGroupHandle_t m_hCannonTracerGroup;				// cannon tracer group
	f32 m_fNextAIMachineGunSound;							// time until next ai machine gun sound
	static FSndFx_FxHandle_t m_hCannonSFX;					// Cannon sfx handle
	static FSndFx_FxHandle_t m_hMachineGunSFX;				// machine gun sfx handle
	f32 m_fMachineGunDebrisTimer;							// timer for spawning debris chunks
	f32 m_fNextMachineGunTracer;							// timer for machine gun tracers
	static FDecalDefHandle_t m_hBulletDecal;				// decal for bullet impact

	CEntity *m_pTargetedEntity;								// entity under cursor, or NULL if none
	CFVec3A m_vTargetPoint;									// WS point targeted by reticle

	// Bot parts:
	static CBotPartPool *m_pPartPool;						// One part pool per bot class

	static FSndFx_FxHandle_t m_hEngineStartSFX;				// sfx handle
	static FSndFx_FxHandle_t m_hEngineStopSFX;				// sfx handle
	static FSndFx_FxHandle_t m_hSkidSFX;					// sfx handle

	CFAntenna m_Antenna;									// antenna animator

	u32 m_uSentinelFlags;									// flags specific to sentinel

	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
public:
	CVehicleSentinel();
	~CVehicleSentinel();

	static BOOL InitSystem( void );
	static void UninitSystem( void );

	virtual BOOL ClassHierarchyLoadSharedResources( void );
	virtual void ClassHierarchyUnloadSharedResources( void );

	virtual u32 GetTagPointCount( void ) const { return m_nTagPointCount; }
	virtual const CFVec3A *GetTagPoint( u32 nTagPointIndex ) const;
	virtual const CFVec3A *GetApproxEyePoint( void ) const;
	virtual void HandleTargeting( void );

	BOOL Create( s32 nPlayerIndex=-1, BOOL bInstallDataPort=FALSE, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);
	BOOL ActionNearby( CEntity *pEntity );

	// checkpoint load/save functions
	virtual void CheckpointSaveSelect( s32 nCheckpoint );
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );
	virtual void CheckpointRestorePostamble( void );

	// anim bone callback functions
	void AnimateWheels( u32 uWheelIndex, u32 uBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	void AnimateTurret( u32 uBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	void AnimateCannonBase( u32 uBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	void AnimateCannon( u32 uBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	void AnimateSpotLight( u32 uBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );

	virtual void ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS );
	virtual f32 ComputeEstimatedControlledStopTimeXZ( void );

	FINLINE BOOL IsUpsideDown( void ) const { return m_eSentinelState == VEHICLESENTINEL_STATE_UPSIDE_DOWN; };
	FINLINE BOOL IsFlippingUpright( void ) const { return m_eSentinelState == VEHICLESENTINEL_STATE_FLIPPING_UPRIGHT; };

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

	virtual CFMtx43A *ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName=NULL );
	void VelocityHasChanged( void );

	void DriverEnter( CBot* pDriverBot, cchar *pszAttachPointBoneName = NULL, BOOL bImmediately = FALSE );
	void DriverExit( CBot* pDriverBot );
	void UpdateCamera( void );
	void MoveVehicle( void );

	CBot *IsStationObstructed( Station_e eStation );
	StationStatus_e CanOccupyStation( CBot *pBot, Station_e eStation );
	StationStatus_e EnterStation( CBot *pBot, Station_e eStation, BOOL bProximityCheck, BOOL bImmediately );
	StationStatus_e ExitStation( CBot *pBot, BOOL bImmediately );
	BOOL GetStationEntryPoint( Station_e eStation, CFVec3A *pPoint );
	void FlipRightsideUp( void );

	void CrankEngine( void );
	void StartEngine( void );
	void StopEngine( BOOL bImmediately = FALSE );
	void RunEngine( void );

	//----------------------------------------------------------------------------------------------------------------------------------
	// Private Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
private:
	static	BOOL _BuildAnimStackDef( void );

	void _ClearDataMembers( void );

	void _HandleWeaponFiring( void );
	void _HandleWeaponWork( void );

	static void _AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	static void _TracerKilledCallback( TracerDef_t *pTracerDef, TracerKillReason_e nKillReason, const FCollImpact_t *pImpact );

	void _InitPhysicsObject( void );
	void _ResetSprings( void );
	void _UpdateCannon( void );
	void _UpdateMachineGun( void );
	void _UpdateReticle( void );
	void _UpdateTreads( void );
	void _UpdateCollisionFlags( void );
	void _UpdateDustCloud( void );
	void _UpdateEngineSounds( void );
	void _StartDriverEnterWork( void );
	void _AbortEntry( void );

	friend class CVehicleSentinelBuilder;

	FCLASS_STACKMEM_ALIGN( CVehicleSentinel );
} FCLASS_ALIGN_SUFFIX;



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CVehicleSentinelBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CVehicleSentinelBuilder : public CVehicleBuilder 
{
public:

	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CVehicleSentinelBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );

	//----------------------------------------------------------------------------------------------------------------------------------
	// Protected Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CVehicleSentinelBuilder );
} FCLASS_ALIGN_SUFFIX;
#endif

