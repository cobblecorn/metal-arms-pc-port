//////////////////////////////////////////////////////////////////////////////////////
// vehicle.h -
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

#ifndef _VEHICLE_H_
#define _VEHICLE_H_ 1

#include "fphysics.h"
#include "cammanual.h"
#include "bot.h"
#include "espline.h"
#include "fforce.h"
#include "reticle.h"


//----------------------------------------------------------------------------------------------------------------------------------
// CVehicleCamera
// Vehicle camera algorithm
//----------------------------------------------------------------------------------------------------------------------------------
FCLASS_ALIGN_PREFIX class CVehicleCamera
{
	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Definitions:
	//----------------------------------------------------------------------------------------------------------------------------------
public:
	typedef struct
	{
		f32 fCamDistance;				// camera distance behind object
		f32 fCamDistanceVel;			// max velocity of camera distance when object is on the ground.
		f32 fCamDistanceAirVel;			// max velocity of camera distance when object is airborne
		f32 fCamMinDistance;			// minimum camera distance from object
		f32 fCamMaxDistance;			// maximum camera distance from object
		f32 fCamDeadDistance;			// camera distance when object is dead
		f32 fCamGroundRotRate;			// rate at which camera interpolates to new orientation while object is on the ground
		f32 fCamAirRotRate;				// rate at which camera interpolates to new orientation while object is airborne
		f32 fCamDefaultHeight;			// starting height of camera
		f32 fCamMinHeight;				// minimum camera height above object
		f32 fCamMaxHeight;				// maximum camera height above object
		f32 fCamHeightVel;				// max velocity of height change
		f32 fCamColSphereRadius;		// radius of collision sphere projected from object to camera
		f32 fLookForward;				// distance look point should be projected in front of object
		f32 fLookHeight;				// height above object at which look point is set
		f32 fFOV;						// camera field of view
	} BotInfo_VehicleCamera_t;

	//----------------------------------------------------------------------------------------------------------------------------------
	// Private Definitions:
	//----------------------------------------------------------------------------------------------------------------------------------
private:
#define _FLAG_SNAP						0x00000001	// camera should snap to current position rather than smoothly interpolate
#define _FLAG_DEAD						0x00000002	// object is dead or dying
#define _FLAG_DIRECT_ELEVATION_ANGLE	0x00000004	// directly set elevation of camera using SetElevationAngle() instead of using SetHeight()
#define _FLAG_HEIGHT_FROM_XZ_PLANE		0x00000008	// camera height is always relative to XZ plane, not ground normal

	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Data:
	//----------------------------------------------------------------------------------------------------------------------------------
public:
	static const FGameData_TableEntry_t m_aBotInfoVocab_VehicleCamera[];

	//----------------------------------------------------------------------------------------------------------------------------------
	// Private Data:
	//----------------------------------------------------------------------------------------------------------------------------------
private:
	CFQuatA m_qCamLookBack;				// current orientation of camera lookback vector
	CCamManualInfo *m_pCamInfo;			// pointer to manual camera to be animated
	BotInfo_VehicleCamera_t *m_pParams;	// camera parameters
	f32 m_fCamDistance;					// current camera distance
	f32 m_fCamHeight;					// current camera height
	f32 m_fElevationAngle;				// elevation angle of camera in radians, used in lieu of m_fCamHeight
	f32 m_fCamBipolarUnitHeight;		// unit camera height requested by client
	f32 m_fCamSlerp;					// current camera slerp rate
	f32 m_fAirTime;						// time that object has been airborne.  Must be 0.0 if on ground.
	f32 m_fFOV;							// camera field of view
	u32 m_uFlags;						// misc. flags, see _FLAG_*

	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
public:
	void Init( CCamManualInfo *pCamInfo, BotInfo_VehicleCamera_t *pParams );
	void Update( CFMtx43A *pmTargetObject, const CFVec3A &vObjectVel, CFWorldTracker *pWorldMesh, const CFVec3A &vGroundNormal );
	void SetHeight( f32 fBipolarUnitHeight );
	void SetHeightFromXZPlane( f32 fBipolarUnitHeight );
	void SetElevationAngle( f32 fAngle );
	FINLINE void SetAirTime( f32 fTime ) { FMATH_CLAMPMIN( fTime, 0.0f ); m_fAirTime = fTime; };
	FINLINE void Snap( void ) { m_uFlags |= _FLAG_SNAP; };
	FINLINE void SetDead( BOOL bDead ) { if( bDead ) m_uFlags |= _FLAG_DEAD; else m_uFlags &= ~_FLAG_DEAD; };
	FINLINE void SetFOV( f32 fFOV ) { m_fFOV = fFOV; };

	//----------------------------------------------------------------------------------------------------------------------------------
	// Private Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL _TrackerCallback( CFWorldTracker *pTracker );

	FCLASS_STACKMEM_ALIGN( CVehicleCamera );
} FCLASS_ALIGN_SUFFIX;


//----------------------------------------------------------------------------------------------------------------------------------
// CCameraTrans
// Performs smooth transitions from player bot camera to a manual camera (and back).
//----------------------------------------------------------------------------------------------------------------------------------
FCLASS_ALIGN_PREFIX class CCameraTrans
{
	//----------------------------------------------------------------------------------------------------------------------------------
	// Private Definitions:
	//----------------------------------------------------------------------------------------------------------------------------------
private:

#define FLAG_SNAP_CAM_TRANS		0x00000001						// TRUE if camera transition should snap to end position this frame

	typedef enum
	{
		CAMERA_STATE_INACTIVE,
		CAMERA_STATE_START_ENTER_VEHICLE,
		CAMERA_STATE_ENTERING_VEHICLE,
		CAMERA_STATE_IN_VEHICLE,
		CAMERA_STATE_START_EXIT_VEHICLE,
		CAMERA_STATE_EXITING_VEHICLE,
		CAMERA_STATE_ABORT_TO_INACTIVE,
	} CameraState_e;

	//----------------------------------------------------------------------------------------------------------------------------------
	// Private Data:
	//----------------------------------------------------------------------------------------------------------------------------------
private:
	CFQuatA m_qPreviousCam;										// entering bot's cam orientation before entering vehicle
	CFVec3A m_vPreviousCam;										// entering bot's cam position before entering vehicle
	CCamManualInfo *m_pCamInfo;									// pointer to client's manual camera which is to be transitioned
	CameraState_e m_eCameraState;								// state of vehicle camera
	f32 m_fTransitionLength;									// total length of transition in seconds
	f32 m_fOOTransitionLength;									// one over transition length
	f32 m_fRemainingTime;										// transition time remaining from one camera state to another
	f32 m_fPreviousCamFOV;										// entering bot's cam FOV
	s32 m_nCameraPlayerIndex;									// index of player whose bot camera will be transitioned from/to.
	u32 m_uFlags;												// camera transition flags


	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
public:
	void Init( CCamManualInfo *pCamInfo, f32 fTransitionLength );
	void InitTransitionToVehicleCamera( s32 nPlayerIndex );
	void InitTransitionToBotCamera( void );
	void InitTransitionToCamera( CCamManualInfo *pCamInfo, f32 fTransitionLength );
	void UpdateTransition( void );																	// work function called every frame to perform transition
	FINLINE BOOL GetPlayerIndex( void ) const { return m_nCameraPlayerIndex; };						// returns player index for current transition
	FINLINE BOOL IsCameraInactive( void ) const { return m_eCameraState == CAMERA_STATE_INACTIVE; };// TRUE if in inactive state
	FINLINE void SetCameraStateInactive( void ) { m_eCameraState = CAMERA_STATE_INACTIVE; };		// go directly to inactive state
	FINLINE void SetCameraStateAbort( void ) { m_eCameraState = CAMERA_STATE_ABORT_TO_INACTIVE; };	// abort transition and go to inactive state
	FINLINE void SetTransitionSnap( void ) { m_uFlags |= FLAG_SNAP_CAM_TRANS; };					// snap transition to end position
	FINLINE void SetTransitionLength( f32 fLength ) { FASSERT( fLength > 0.0f ); m_fTransitionLength = fLength; m_fOOTransitionLength = 1.0f / fLength; };	// set length of transition in seconds

	FCLASS_STACKMEM_ALIGN( CCameraTrans );
} FCLASS_ALIGN_SUFFIX;


//----------------------------------------------------------------------------------------------------------------------------------
// CVehicle
//----------------------------------------------------------------------------------------------------------------------------------

FCLASS_ALIGN_PREFIX class CVehicle : public CBot 
{
	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Definitions:
	//----------------------------------------------------------------------------------------------------------------------------------
public:

#define VEHICLE_FLAG_SMOOTH_CONTACT		0x00000001		// true if vehicle is in contact with ground
#define VEHICLE_FLAG_HIT_GROUND			0x00000002		// true if vehicle touched ground this frame after being airborne
#define VEHICLE_FLAG_COLLIDED			0x00000004		// true if vehicle received a sharp jolt this frame
#define VEHICLE_FLAG_JOSTLED			0x00000008		// true if vehicle received a moderate jolt this frame
#define VEHICLE_FLAG_DRIVER_LIGHT_ON	0x00000010		// true if driver's spotlight was on when entering vehicle
#define VEHICLE_FLAG_DRIVER_HAS_SHADOW	0x00000020		// TRUE if driver had shadow upon entry into vehicle
#define VEHICLE_FLAG_NO_PLAYER_DRIVE	0x00000040		// TRUE if players are unable to drive this vehicle (AI drive only)
#define VEHICLE_FLAG_BECAME_AIRBORNE	0x00000080		// TRUE if vehicle departed from the ground this frame
#define VEHICLE_FLAG_NO_ENTITY_DRIVE	0x00000100		// TRUE means vehicle isn't meant to be driven, just a placed gun for instance
#define VEHICLE_FLAG_DET_PACK_DESTROY	0x00000200		// TRUE if vehicle can be destroyed with a detpack
#define VEHICLE_FLAG_DRIVER_EXIT		0x00000400		// TRUE if bot can exit driving station of this vehicle once entered
#define VEHICLE_FLAG_GUNNER_EXIT		0x00000800		// TRUE if bot can exit gunning station of this vehicle once entered
#define VEHICLE_FLAG_NO_DMG_IMPULSE		0x00001000		// TRUE if damage impulses should not be applied (the derived class will be responsible instead)

	typedef enum
	{
		ENGINE_STATE_STOPPED = 0,
		ENGINE_STATE_CRANKING,
		ENGINE_STATE_RUNNING,
		ENGINE_STATE_STOPPING,
	} EngineState_e;

	// vehicle station positions. Update all m_afStationEntryPoints[][] if adding to enum or changing order.
	typedef enum
	{
		STATION_DRIVER,
		STATION_GUNNER,
		NUM_STATIONS,
		STATION_NONE,
	} Station_e;

	// vehicle station status
	typedef enum
	{
		STATION_STATUS_EMPTY,			// station is availble for occupation by inquiring bot
		STATION_STATUS_OCCUPIED,		// station is occupied by a bot
		STATION_STATUS_PRE_ENTERING,	// bot is jumping towards entry point
		STATION_STATUS_ENTERING,		// requested bot is entering the station
		STATION_STATUS_EXITING,			// requested bot is exiting the station
		STATION_STATUS_OUT_OF_RANGE,	// inquiring bot is not in position to occupy the station
		STATION_STATUS_NO_STATION,		// no station of the type specified exists on the vehicle
		STATION_STATUS_UNAVAILABLE,		// it is not currently possible to enter station
		STATION_STATUS_WRONG_BOT,		// specified bot is not the one occupying the station
		STATION_STATUS_TOO_FAST,		// vehicle is traveling too fast to perform requested function
		STATION_STATUS_WEAPON_SWITCH,	// bot is switching weapons
	} StationStatus_e;

	// must change vehicle csv table if number of types changes
	enum
	{
		COLLISION_SOUND_TYPE_DIRT = 0,
		COLLISION_SOUND_TYPE_ROCK,
		COLLISION_SOUND_TYPE_JUNK,
		COLLISION_SOUND_TYPE_METAL,
		COLLISION_SOUND_NUM_TYPES
	};

	// must change vehicle csv table if number of lengths changes
	enum
	{
		COLLISION_SOUND_LENGTH_SHORT = 0,
		COLLISION_SOUND_LENGTH_LONG,
		COLLISION_SOUND_NUM_LENGTH
	};

	typedef struct
	{
		CFSoundGroup *pSoundGroupSoftHitGround;
		CFSoundGroup *pSoundGroupHardHitGround;
		CFSoundGroup *apSoundGroupCollision[COLLISION_SOUND_NUM_TYPES][COLLISION_SOUND_NUM_LENGTH];
		f32 fSoftCollMom;					
		f32 fHardCollMom;				
		f32 fSoftHitGroundMom;
		f32 fHardHitGroundMom;
		FParticle_DefHandle_t hSmokeParticle;			// damaged smoke particle
		f32 fImpulseFraction;							// fraction of damage impulse to pass to vehicle physics object
		f32 fDriverStationRadius;						// collision radius of vehicle's driving position
		f32 fGunnerStationRadius;						// collision radius of vehicle's gunning position
		CArmorProfile *pOccupantArmorProfile;			// Armor profile assigned to vehicle occupants
		CArmorProfile *pOccupantRacingArmorProfile;		// Armor profile assigned to vehicle occupants when racing
		cchar *pszDamageProfile;						// name of Damage profile used when vehicle hits with non bumpers
		CDamageProfile *pRammingDamageProfile;			// Damage profile used when vehicle rams with bumpers
		CDamageProfile *pPrimaryWeaponDamageProfile;	// Damage profile for vehicle's primary weapon
		CDamageProfile *pSecondaryWeaponDamageProfile;	// Damage profile for vehicle's secondary weapon
	} BotInfo_Vehicle_t;

	typedef struct
	{
		CFSoundGroup *pSoundGroupEngineStart;			// engine cranking sound
		CFSoundGroup *pSoundGroupEngineStop;			// engine dying sound
		CFSoundGroup *pSoundGroupEngineIdle;			// engine idling sound
		CFSoundGroup *pSoundGroupEngineLow;				// low speed engine sound
		CFSoundGroup *pSoundGroupEngineHigh;			// high speed engine sound
		f32 fIdleUnitWidth;								// fraction of throttle value at which idle sound is completely faded out
		f32 fCrossfadeUnitCenter;						// unit center of crossfade point between low and high engine sounds
		f32 fCrossfadeUnitWidth;						// unit width of crossfade between low and high engine sounds
		f32 fAirSpeedMul;								// multiplier for engine speed when airborne
		f32 fOOAirSpeedMul;								// inverse multiplier for engine speed when airborne
		f32 fRevRate;									// rate at which unit engine pitch changes in units per second
		f32 fAirRevRate;								// rate at which unit engine pitch changes in units per second while airborne
		f32 fVarianceChance;							// random chance that engine pitch will vary slightly (die cast once per update)
		f32 fVarianceMinMul;							// minimum multiplier for engine pitch variance (1.0 = no change)
		f32 fVarianceMaxMul;							// maximum multiplier for engine pitch variance
		f32 fLowMinVolume;								// minimum volume for low engine sound at idle (0.0 = silent)
	} BotInfo_Engine_t;

	//----------------------------------------------------------------------------------------------------------------------------------
	// Protected Definitions:
	//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF

	typedef struct
	{
		f32 fMass;					
		f32 fMaxSpeed;				
		f32 fMaxAccel;				
		f32 fLinearDampingHigh;
		f32 fLinearDampingLow;
		f32 fAngularDampingHigh;
		f32 fAngularDampingLow;
		f32 fBodyFriction;
		f32 fBodyStickiness;
		f32 fRightingForceMag;			// used to keep vehicle righted
	} BotInfo_VehiclePhysics_t;

	//----------------------------------------------------------------------------------------------------------------------------------
	// Private Definitions:
	//----------------------------------------------------------------------------------------------------------------------------------
private:

	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Data:
	//----------------------------------------------------------------------------------------------------------------------------------
public:
	CFPhysicsObject m_PhysObj;									// physics object
	CFVec3A m_vCMOffset;										// vector from dummy bone to vehicle center-of-mass
	const CGCollMaterial *m_pRecentCollisionMaterial;			// pointer to most recent collision material
	f32 m_fRecentCollisionMagnitude;							// magnitude of most recent collision
	BotInfo_Vehicle_t *m_pBotInfo_Vehicle;						// pointer to vehicle's leaf class collision botinfo
	BotInfo_VehiclePhysics_t *m_pBotInfo_VehiclePhysics;		// pointer to vehicle's leaf class collision botinfo
	BotInfo_Engine_t *m_pBotInfo_Engine;						// pointer to vehicle's leaf class engine botinfo
	CVehicleCamera::BotInfo_VehicleCamera_t *m_pBotInfo_DriverCamera;	// pointer to vehicle's leaf class driver camera info

	//----------------------------------------------------------------------------------------------------------------------------------
	// Protected Data:
	//----------------------------------------------------------------------------------------------------------------------------------
protected:
	static const FGameData_TableEntry_t m_aBotInfoVocab_VehiclePhysics[];
	static const FGameData_TableEntry_t m_aBotInfoVocab_Vehicle[];
	static const FGameData_TableEntry_t m_aBotInfoVocab_Engine[];

	CFVec3A m_vGroundNormal;									// smoothed normal vector of ground under center of vehicle
	CFQuatA m_qGroundNormal;									// quaternion representation of ground normal

	// camera
	CFMtx43A m_mCamera;											// matrix for vehicle camera
	CCamManualInfo m_CamInfo;									// camera info for vehicle camera
	CCameraTrans m_DriverCameraTrans;							// camera transition object for driver
	CVehicleCamera m_DriverCamera;								// vehicle camera object for driver

	f32 m_fHeightAboveGround;									// height of vehicle center-of-mass above ground
	f32 m_fMaxHeightAboveGround;								// maximum height attained on last jump
	const CFVec3A *m_pGazeDir_WS;								// Points to the gaze direction vector (might not be unit in length)
	f32 m_fUnitSpeed_WS;										// unitized 3D speed of vehicle
	u32 m_uVehicleFlags;										// vehicle flags
	f32 m_fMaxSpeed;											// current max speed of vehicle (can be different than m_fMaxFlatSurfaceSpeed_WS)
	f32 m_fOOMaxSpeed;											// one over max speed.

	CBot *m_pDriverBot;											// pointer to bot currently driving vehicle

	// audio
	CFAudioEmitter *m_pEngineIdleEmitter;
	CFAudioEmitter *m_pEngineLowEmitter;
	CFAudioEmitter *m_pEngineHighEmitter;
	CFAudioEmitter *m_pSkidEmitter;
	f32 m_fSoundFxRadiusMultiplier;								// a factor to multiply all sound fx radii by

	// engine
	EngineState_e m_eEngineState;								// state of engine
	f32 m_fEngineTime;											// timer for starting/stopping engine
	f32 m_fThrottle;											// amount of throttle being applied, 0.0 to 1.0
	f32 m_fBrake;												// amount of brake being applied, 0.0 to 1.0
	f32 m_fEngineSpeed;											// speed of engine revolutions, 0.0 to 1.0
	f32 m_fEngineVariance;										// multiplier to simulate random engine speed changes

	f32 m_fTimeSinceLastContact;								// time in seconds since vehicle last contacted 
	f32 m_fTimeSinceLastCollide;								// time in seconds since vehicle last collided
	f32 m_fTimeSinceLastHitGround;								// time in seconds since vehicle last hit the ground

	CESpline* m_pSplinePath;									// pointer to spline path that vehicle is following
	f32 m_fEntryJumpTimer;										// time remaining for driver entry jump
	CArmorProfile *m_pPreviousDriverArmorProfile;				// Armor profile of driver before entering vehicle

	FForceHandle_t m_hForce;									// collision rumble fforce handle

	// stored reticle data
	f32	m_fReticlePrevX;										// x and y positions of driver's original reticle before entering vehicle
	f32	m_fReticlePrevY;	
	BOOL m_bReticleDrawPrev;									// draw status of driver's original reticle
	CReticle::Type_e m_ReticlePrevType;							// driver's original reticle type
	f32 m_fEngineMasterVolume;									// overall volume of engine sounds

	//----------------------------------------------------------------------------------------------------------------------------------
	// Private Data:
	//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL	 m_bSystemInitialized;							// TRUE: InitSystem() has been called
	static u32 m_nBotClassClientCount;							// Number of bots of this class using the class-wide resources
	u32	 m_uPreviousHudFlags;									// used to store the status of the HUD when someone jumps in, so we can restore it when they get out

	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
public:
	CVehicle();
	~CVehicle();

	static BOOL InitSystem( void );
	static void UninitSystem( void );

	BOOL Create( const CBotDef *pBotDef, s32 nPlayerIndex=-1, BOOL bInstallDataPort=FALSE, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

	virtual BOOL ClassHierarchyLoadSharedResources( void );
	virtual void ClassHierarchyUnloadSharedResources( void );

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);
	virtual BOOL SquishBot( CBot *pBot );		// TRUE if bot was squished, FALSE if not

	FINLINE BOOL SmoothContact( void ) const { FASSERT( IsCreated() ); return !!(m_uVehicleFlags & VEHICLE_FLAG_SMOOTH_CONTACT); }
	FINLINE BOOL HitGround( void ) const { FASSERT( IsCreated() ); return !!(m_uVehicleFlags & VEHICLE_FLAG_HIT_GROUND); }
	FINLINE BOOL BecameAirborne( void ) const { FASSERT( IsCreated() ); return !!(m_uVehicleFlags & VEHICLE_FLAG_BECAME_AIRBORNE); }
	FINLINE BOOL Collided( void ) const { FASSERT( IsCreated() ); return !!(m_uVehicleFlags & VEHICLE_FLAG_COLLIDED); }
	FINLINE BOOL Jostled( void ) const { FASSERT( IsCreated() ); return !!(m_uVehicleFlags & VEHICLE_FLAG_JOSTLED); }
				   
	EngineState_e EngineState( void ) { FASSERT( IsCreated() ); return m_eEngineState; }

	virtual StationStatus_e CanOccupyStation( CBot *pBot, Station_e eStation ) { return STATION_STATUS_NO_STATION; };
	virtual StationStatus_e EnterStation( CBot *pBot, Station_e eStation, BOOL bProximityCheck, BOOL bImmediately ) { return STATION_STATUS_NO_STATION; };
	virtual StationStatus_e ExitStation( CBot *pBot, BOOL bImmediately ) { return STATION_STATUS_NO_STATION; };
	virtual BOOL GetStationEntryPoint( Station_e eStation, CFVec3A *pPoint ) { return FALSE; };
	virtual Station_e WhichStationOccupied( CBot *pBot );
	FINLINE CBot *GetDriverBot( void ) const { return m_pDriverBot; };
	virtual FINLINE CBot *GetGunnerBot( void ) const { return NULL; };

	FINLINE void SetDriveable( BOOL bDriveable ) { FASSERT( IsCreated() ); if( !bDriveable ) m_uVehicleFlags |= VEHICLE_FLAG_NO_PLAYER_DRIVE; else m_uVehicleFlags &= ~VEHICLE_FLAG_NO_ENTITY_DRIVE; };
	FINLINE BOOL IsDriveable( void ) const { FASSERT( IsCreated() ); return !(m_uVehicleFlags & VEHICLE_FLAG_NO_ENTITY_DRIVE); };

	FINLINE void SetPlayerDriveable( BOOL bDriveable ) { FASSERT( IsCreated() ); if( !bDriveable ) m_uVehicleFlags |= VEHICLE_FLAG_NO_PLAYER_DRIVE; else m_uVehicleFlags &= ~VEHICLE_FLAG_NO_PLAYER_DRIVE; };
	FINLINE BOOL IsPlayerDriveable( void ) const { FASSERT( IsCreated() ); return !(m_uVehicleFlags & VEHICLE_FLAG_NO_PLAYER_DRIVE); };
	
	FINLINE void SetDriverExitable( BOOL bExitable ) { FASSERT( IsCreated() ); if( bExitable ) m_uVehicleFlags |= VEHICLE_FLAG_DRIVER_EXIT; else m_uVehicleFlags &= ~VEHICLE_FLAG_DRIVER_EXIT; };
	FINLINE BOOL IsDriverExitable( void ) const { FASSERT( IsCreated() ); return !!(m_uVehicleFlags & VEHICLE_FLAG_DRIVER_EXIT); };

	FINLINE void SetGunnerExitable( BOOL bExitable ) { FASSERT( IsCreated() ); if( bExitable ) m_uVehicleFlags |= VEHICLE_FLAG_GUNNER_EXIT; else m_uVehicleFlags &= ~VEHICLE_FLAG_GUNNER_EXIT; };
	FINLINE BOOL IsGunnerExitable( void ) const { FASSERT( IsCreated() ); return !!(m_uVehicleFlags & VEHICLE_FLAG_GUNNER_EXIT); };

	virtual void InflictDamage( CDamageData *pDamageData );
	virtual void InflictDamageResult( const CDamageResult *pDamageResult );

	virtual BOOL IsUpsideDown( void ) const { return FALSE; };
	virtual BOOL IsFlippingUpright( void ) const { return FALSE; };
	virtual void FlipRightsideUp( void ) {};
	virtual void UpdateMatrices( void );
	FINLINE f32 GetUnitSpeed( void ) const { FASSERT( IsCreated() ); return m_fUnitSpeed_WS; };
	FINLINE CESpline *GetSplinePath( void ) const { FASSERT( IsCreated() ); return m_pSplinePath; };
	FINLINE void SetSplinePath( CESpline *pSpline ) { FASSERT( IsCreated() ); m_pSplinePath = pSpline; };
	virtual FINLINE f32 ComputeMinimumManeuveringSpeed( void ) { FASSERT_NOW; return 0.0f; };	// minimum speed at which vehicle can maneuver

	FINLINE void SnapCameraTransition( void ) { FASSERT( IsCreated() ); m_DriverCameraTrans.SetTransitionSnap(); };
	FINLINE void SnapCameraPosition( void ) { FASSERT( IsCreated() ); m_DriverCamera.Snap(); };

	static f32 ComputeEntryTrajectory( const CFVec3A &rvStartPos, const CFVec3A &rvEndPos, f32 fMinJumpHeight, f32 fGravity, CFVec3A &rvOutputVel );

	virtual CBot *IsStationObstructed( Station_e eStation ) {FASSERT_NOW; return NULL;};

	static CBot *StationObstructed( CBot* pThis, u32 uBoneIndex, f32 fRadius );
	static BOOL IsDriverUseless(CBot* pDriverBot);

	void SetMaxUnitSpeed( f32 fUnitSpeed, BOOL bInternalOnly = FALSE );
	f32 GetMaxUnitSpeed( void ) const;
	FINLINE f32 GetMaxSpeed( void ) const { FASSERT( IsCreated() ); return m_fMaxSpeed; };
	FINLINE f32 GetOOMaxSpeed( void ) const { FASSERT( IsCreated() ); return m_fOOMaxSpeed; };

	FINLINE CArmorProfile* GetPrevArmorProfile( void ) const { return m_pPreviousDriverArmorProfile; }

	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );

	void SetSoundFxRadiusMultiplier( f32 fNewRadiusMultipler );

	BOOL SaveDriverReticle( CBot *pDrivingBot );
	BOOL SetVehicleReticle( CBot *pDrivingBot, CReticle::Type_e eType, f32 fXPos, f32 fYPos );	// sets reticle to specified type when appropriate
	void UnSetVehicleReticle( CBot *pDrivingBot );	// returns reticle to driver's original reticle if appropriate
	FINLINE void SetEngineMasterVolume( f32 fVolume ) { FASSERT( IsCreated() ); FMATH_CLAMP( fVolume, 0.0f, 1.0f ); m_fEngineMasterVolume = fVolume; };
	FINLINE f32 GetEngineMasterVolume( void ) const { FASSERT( IsCreated() ); return m_fEngineMasterVolume; };


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

	virtual void DriverEnter( CBot *pDriverBot, cchar *pszAttachPointBoneName = NULL, BOOL bImmediately = FALSE );	// puts driver bot into vehicle
	virtual void DriverExit( CBot* pDriverBot );

	virtual void MoveVehicle( void ) {};
	virtual void UpdateCamera( void ) {};
	void StartTransitionToDriverVehicleCamera( s32 nPlayerIndex );
	void StartTransitionToDriverBotCamera( void );
	void HandleCameraTransitions( void );
	void ComputeGroundNormal( void );
	void LocateDriverAtBone( s32 nBoneIndex );
	void UpdateEngineSounds( void );
	void PlayCollisionSounds( void );
	void UpdateFForceRumble( void );

	//----------------------------------------------------------------------------------------------------------------------------------
	// Private Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
private:
	void _ClearDataMembers( void );

	FCLASS_STACKMEM_ALIGN( CVehicle );
} FCLASS_ALIGN_SUFFIX;


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CVehicleBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CVehicleBuilder : public CBotBuilder 
{
public:
#define VEHICLE_BUILDER_FLAG_NO_PLAYER_DRIVE	( 0x00000001 )
#define VEHICLE_BUILDER_FLAG_NO_ENTITY_DRIVE	( 0x00000002 )
#define VEHICLE_BUILDER_FLAG_DET_PACK_DESTROY	( 0x00000004 )
#define VEHICLE_BUILDER_FLAG_DRIVER_EXIT		( 0x00000008 )
#define VEHICLE_BUILDER_FLAG_GUNNER_EXIT		( 0x00000010 )
#define VEHICLE_BUILDER_FLAG_DRIVER_EXIT_SELECT	( 0x00000020 )
#define VEHICLE_BUILDER_FLAG_GUNNER_EXIT_SELECT	( 0x00000040 )

	u32 m_uFlags;

	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CVehicleBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );

	//----------------------------------------------------------------------------------------------------------------------------------
	// Protected Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CVehicleBuilder );
} FCLASS_ALIGN_SUFFIX;
#endif

