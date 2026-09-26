//////////////////////////////////////////////////////////////////////////////////////
// vehicleloader.h -
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
// 01/14/02 Elliott 	Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _VEHICLELOADER_H_
#define _VEHICLELOADER_H_ 1

#include "fang.h"
#include "vehicle.h"
#include "fphysics.h"
#include "fparticle.h"
#include "eparticle.h"
#include "tracer.h"
#include "gcoll.h"
#include "fforce.h"
#include "reticle.h"
#include "fdecal.h"

FCLASS_ALIGN_PREFIX class CVehicleLoader : public CVehicle
{
	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Definitions:
	//----------------------------------------------------------------------------------------------------------------------------------
public:
	// Tag Point indices
	enum {
		TAG_POINT_COUNT			= 4,			// Number of tag points on the loader
	};


	//----------------------------------------------------------------------------------------------------------------------------------
	// Protected Definitions:
	//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF

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
	// Private Definitions:
	//----------------------------------------------------------------------------------------------------------------------------------
private:
	// Bones:
	typedef enum
	{
		BONE_JET_R_ATTACH,	//"AttachPoint_Jet_R",
		BONE_FUSELAGE,		//"Fuselage",
		BONE_ENGINE_L,		//"L_Engine",
		BONE_JET_L_ATTACH,	//"AttachPoint_Jet_L",
		BONE_ENGINE_R,		//"R_Engine",
		BONE_CLAW_ARM,		//"Claw_Arm",
		BONE_CLAW_L,		//"L_Claw",
		BONE_CLAW_R,		//"R_Claw",
		BONE_DRIVER_ATTACH,	//"AttachPoint_Driver",
		BONE_PRIMARY_FIRE,	//"Primary_Fire",
		BONE_VICTIM_ATTACH,	//"AttachPoint_Victim"
		BONE_GUN,			//"Gun"

		BONE_COUNT,
	} Bone_e;

	typedef enum {
		ANIMBONE_ENGINE_LEFT,
		ANIMBONE_ENGINE_RIGHT,
		ANIMBONE_CLAW_LEFT,
		ANIMBONE_CLAW_RIGHT,
		ANIMBONE_CLAW_ARM,
		ANIMBONE_GUN,

		ANIMBONE_COUNT
	} AnimBone_e;

	typedef struct {
		CFMtx43A	mFriction;					// friction matrix
		CFVec3A 	vCurrentPos;				// current WS position of sphere center
		CFVec3A 	vProposedPos;				// proposed WS position of sphere center (i.e. forms the projection of the sphere)
		const		CGCollMaterial *pMaterial;	// type of surface at most recent collision
		f32			fRadius;						// radius of collision sphere
	} ColPoint_t;

	typedef enum {
		COLPOINT_ENGINE_LEFT,
		COLPOINT_ENGINE_RIGHT,
		COLPOINT_BODY,
		COLPOINT_FRONT,

		COLPOINT_CLAW_LEFT,
		COLPOINT_CLAW_RIGHT,
		COLPOINT_CLAW_CENTER,

		COLPOINT_OBJECT,					// this will equal the object's bounding sphere when the loader has something grabbed

		COLPOINT_COUNT,
	} ColPoints_e;

	typedef enum {
		CLAWSTATE_OPEN,
		CLAWSTATE_OPENING,
		CLAWSTATE_CLOSING,
		CLAWSTATE_CLOSED,
		CLAWSTATE_CLOSED_ON_OBJECT,
		CLAWSTATE_CLOSED_ON_BOT,
	} ClawState_e;


	typedef struct {
		cchar*				pszExhaustParticle;
		cchar*				pszEngineStopParticle;
		f32					fDecelBonusZ;
		f32					fDecelBonusX;
		f32					fOOGunRoundsPerSec;
		f32					fShotSpreadRate;
		f32					fShotSpreadRecRate;
		f32					fMinShotSpread;
		f32					fMaxShotSpread;
		f32					fMaxRange;
		f32					fMuzzleFlashZWidth;
		f32					fMuzzleFlashZHeight;
		f32					fMuzzleFlashZAlpha;
		f32					fMuzzleFlashXYScale;
		f32					fMuzzleFlashXYAlpha;
		f32					fMuzzleFlashMeshScale;
		f32					fMuzzleLightRadius;
		f32					fMuzzleImpactScale;
		f32					fMuzzleImpactAlpha;
		f32					fMuzzleImpactOffset;
		f32					fPotmarkScale;
		f32					fTracerTime;
		f32					fTracerWidth;
		f32					fTracerAlpha;
		f32					fSoundVolume3D;
		f32					fSoundRadius3D;
		f32					fSoundVolume2D;
		FDecalDefHandle_t	hBulletDecal;
	} BotInfo_Loader_t;

	typedef struct {
		// sound handles
		FSndFx_FxHandle_t	hPowerUp;
		FSndFx_FxHandle_t	hPowerDown;
		FSndFx_FxHandle_t	hHoverLow;
		FSndFx_FxHandle_t	hHoverHigh;
		FSndFx_FxHandle_t	hClawMove;
		FSndFx_FxHandle_t	hClawClose;
		FSndFx_FxHandle_t	hClawCloseObject;
		FSndFx_FxHandle_t	hGun;
	} BotInfo_LoaderSounds_t;


	enum {
		_VERLET_COUNT			= 3,
//		_COLL_PER_POINT			= 2,
	};

	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Data:
	//----------------------------------------------------------------------------------------------------------------------------------
public:
	CBot *m_pCameraBot;											// pointer to bot involved in camera transitions


	//----------------------------------------------------------------------------------------------------------------------------------
	// Private Data:
	//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL m_bSystemInitialized;											// TRUE: InitSystem() has been called
	static u32 m_nBotClassClientCount;											// Number of bots of this class using the class-wide resources

	static const CBotDef				m_BotDef;								// Bot definition data

	static cchar						*m_apszBoneNameTable[BONE_COUNT];		// Array of bone names
	static cchar						*m_apszAnimBoneNames[ANIMBONE_COUNT];	// Array of bone names driven by man anim mtx
	static const u8						m_nApproxEyePointBoneNameIndex;
	static const u8						m_anTagPointBoneNameIndexArray[TAG_POINT_COUNT];

	static BotInfo_Gen_t				m_BotInfo_Gen;							// General bot info
	static BotInfo_Vehicle_t			m_BotInfo_Vehicle;						// Vehicle bot info
	static BotInfo_MountAim_t			m_BotInfo_MountAim;						// Mount aim bot info
	static BotInfo_Loader_t				m_BotInfo_Loader;						// Loader specific data
	static BotInfo_VehiclePhysics_t		m_BotInfo_VehiclePhysics;				// Vehicle physics data
	static BotInfo_LoaderSounds_t		m_BotInfo_LoaderSounds;					// Loader sounds
	static CVehicleCamera::BotInfo_VehicleCamera_t m_BotInfo_DriverCamera;		// single player driver camera parameters
	static CVehicleCamera::BotInfo_VehicleCamera_t m_BotInfo_MPDriverCamera;	// multiplayer driver camera parameters

	static const FGameDataMap_t			m_aGameDataMap[];						// Vocabulary map used to parse through user props
	static const FGameDataMap_t			m_aSoundDataMap[];
	static const FGameData_TableEntry_t m_aBotInfoVocab_Loader[];
	static const FGameData_TableEntry_t m_aBotInfoVocab_LoaderSounds[];

	static CVehicleLoader*				m_pCBLoader;

	static CFVec3A						m_avCollisionPts[COLPOINT_COUNT];
	static f32							m_afCollisionPtsRadius[COLPOINT_COUNT];

	static f32							m_fOOMaxForce;
	static f32							m_fOOMaxTorque;
	static f32							m_fRightingForceMag;

	u32				m_nTagPointCount;											// Number of tag points
	const CFVec3A	*m_apTagPoint_WS[TAG_POINT_COUNT];							// Tag points in world space
	FForceHandle_t	m_hForce;													// Force feedback handle so we can kill it when we need to


	static s32 m_nBoneIdxEngineLeft;
	static s32 m_nBoneIdxEngineRight;
	static s32 m_nBoneIdxClawLeft;
	static s32 m_nBoneIdxClawRight;
	static s32 m_nBoneIdxClawArm;
	static s32 m_nBoneIdxDriverAttach;
	static s32 m_nBoneIdxVictimAttach;
	static s32 m_nBoneIdxGun;
	static s32 m_nBoneIdxPriFire;


	// animation...
	CFAnimCombiner* m_pAnimCombiner;
	CFAnimManFrame* m_pAnimManFrame;

	//physics stuff...
	CFVec3A				m_vForceApplied;							// Total force applied by the controls each frame
	f32					m_fTorqueApplied;							// total (yaw) torque applied by the controls each frame
	CFPhysicsObject::CollisionPoint_t m_aColPoints[COLPOINT_COUNT];	// collision point array passed to vehicle's physics object

	static CFVec3A	m_vForceAppPos;	// point where xlat forces are applied
	f32 m_fAltitudeLeft;
	f32 m_fAltitudeRight;
	f32 m_fAltitudeFront;
	f32 m_fAltitude;

	// the claw...
	ClawState_e	m_ClawState;
	f32			m_fClawPressure;				// this is the pressure exerted by the claw (basically delta between where the trigger is & where the claw is, smoothed)
	f32			m_fClawAngle;					// the angle (in rads) of the claw
	f32			m_fUnitClawAngle;				// the angle of the claw 0-1
	f32			m_fLastUnitClawAngle;				
	f32			m_fClawSpan;					// distance between the pincers
	f32			m_fMaxUnitClawAngle;			// set each frame if claw is colliding
	f32			m_fClawLockUnitTimer;			// used to time raising and lowering the claw
	CFMtx43A	m_ClawEntityOrientMtx;

	CEntity*	m_pClawedEntity;				// the entity we're picking up

	f32		m_fClawedEntitySpan;				// how wide our clawed entity is
	CFVec3A m_vClawedEntityDesiredPos;			// where we want the entity to go


	// engine particle effects
	static FParticle_DefHandle_t	m_hExhaustParticleDef;
	static FParticle_DefHandle_t	m_hEngineStopParticleDef;
	static f32						m_fExhaustParticleHeight;
	FParticle_EmitterHandle_t		m_hExhaustPartEmitterLeft;
	FParticle_EmitterHandle_t		m_hExhaustPartEmitterRight;
	FParticleShape_t				m_ExhaustPartShapeLeft;
	FParticleShape_t				m_ExhaustPartShapeRight;

	FParticleEmitCfg_t m_ExhaustPartCfgLeft;
	FParticleEmitCfg_t m_ExhaustPartCfgRight;

	CFVec3A	m_vExhaustPartPosLeft;
	CFVec3A	m_vExhaustPartDirLeft;
	f32		m_fExhaustPartIntensityLeft;
	f32		m_fExhaustPartRadiusLeft;

	f32		m_fExhaustPartIntensityRight;
	f32		m_fExhaustPartRadiusRight;
	CFVec3A	m_vExhaustPartPosRight;
	CFVec3A	m_vExhaustPartDirRight;

	// animation
	CFAnimFrame m_AnimFrameEngineLeft;
	CFAnimFrame m_AnimFrameEngineRight;

	// verlet
	static BOOL				m_abVerletPoolActive[_VERLET_COUNT];
	static CFVerlet*		m_apVerletPool[_VERLET_COUNT];
	static CMeshEntity*		m_apVerletPoolME[_VERLET_COUNT];
	static CFVec3A			m_avVerletObjectAveSpeed[_VERLET_COUNT];
	static BOOL				m_abVerletMarkedForDeactivate[_VERLET_COUNT];

	// weapon data
	BOOL		m_bGunEnabled;
	f32 		m_fGunFireTimer;
	f32 		m_fGunUnitShotSpread;
	f32 		m_fGunAISoundTimer;
	f32 		m_fGunTracerTimer;
	BOOL		m_bGunReady;
	CFQuatA		m_qLastGun;

	// camera
	f32				m_fUnitCameraTransition;
	CFMtx43A		m_ManCamMtx;
	CFMtx43A		m_TransCamMtx;
	CameraState_e	m_eCameraState;

	// engine data
	static CFVec2 m_avEngineVib[4];
	f32	m_fEngineVibTimerLeft;
	f32	m_fEngineVibTimerRight;
	f32 m_fEngineVibMagLeft;
	f32 m_fEngineVibMagRight;
	u32 m_uEngineVibCtrLeft;
	u32 m_uEngineVibCtrRight;
	f32 m_fEngineDisruptTimer;				// used to disrupt the engines/stabilization for a short time
	
	f32 m_fFlipVehicleTimer;
	CFVec3A m_vFlipPosition;

	// sound
	CFAudioEmitter *m_pAudioEmitterEngHigh;
	CFAudioEmitter *m_pAudioEmitterEngLow;
	BOOL			m_b3DSounds;
	f32				m_fEngineLoad;			// how much load the engines are under so we can play appropriate sound
	f32				m_fLastEngineLoad;		// what it was last frame
	f32				m_fEngineLoadAccum;		// used to accumulate load from things other than engines directly (lifting objects for example)
	f32				m_fUnitEngineVolume;

	static CFSphere m_CBSphere;
	static CEntity *m_pCBEntity;
	

	StationStatus_e m_eStationStatus;
	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
public:
	CVehicleLoader();
	~CVehicleLoader();

	static BOOL InitSystem( void );
	static void UninitSystem( void );

	virtual u32 GetTagPointCount( void ) const { return m_nTagPointCount; }
	virtual const CFVec3A *GetTagPoint( u32 nTagPointIndex ) const;
	virtual const CFVec3A *GetApproxEyePoint( void ) const;

	BOOL Create( s32 nPlayerIndex=-1, BOOL bInstallDataPort=FALSE, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);

	// checkpoint load/save functions
	virtual void CheckpointSaveSelect( s32 nCheckpoint );
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );
	virtual void CheckpointRestorePostamble( void );

	BOOL ActionNearby( CEntity *pEntity );

	static void DebugDraw( CVehicleLoader *pLoader );

	// Stations
	virtual StationStatus_e CanOccupyStation( CBot *pBot, Station_e eStation );
	virtual StationStatus_e EnterStation( CBot *pBot, Station_e eStation, BOOL bProximityCheck, BOOL bImmediately );
	virtual StationStatus_e ExitStation( CBot *pBot, BOOL bImmediately );
	virtual BOOL GetStationEntryPoint( Station_e eStation, CFVec3A *pPoint );

	FINLINE BOOL ClawIsClosed( void ) { return (m_ClawState == CLAWSTATE_CLOSED) || (m_ClawState == CLAWSTATE_CLOSED_ON_OBJECT) || (m_ClawState == CLAWSTATE_CLOSED_ON_BOT); };
	FINLINE BOOL IsClawInUse( void ) { return m_ClawState != CLAWSTATE_OPEN;}

	virtual f32 ComputeEstimatedControlledStopTimeXZ( void );
	virtual BOOL IsUpsideDown( void )  const;
	void FlipRightsideUp( CEntity *pEntity );
	virtual void FlipRightsideUp( void )		{ FlipRightsideUp( NULL ); }

	virtual FINLINE void ComputeMtxPalette( BOOL bApplyOffscreenOptimizations ) { FASSERT( IsCreated() ); FASSERT( m_Anim.m_pAnimCombiner ); m_pCBLoader = this; m_Anim.m_pAnimCombiner->ComputeMtxPalette(bApplyOffscreenOptimizations); m_pCBLoader = NULL; }

	CBot *IsStationObstructed( Station_e eStation );



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
	virtual void ClassHierarchyRelocated( void *pIdentifier );

	void VelocityHasChanged( void );

	virtual void DriverEnter( CBot* pDriverBot, cchar *pszAttachPointBoneName = NULL, BOOL bImmediately = FALSE );
	virtual void DriverExit( CBot* pDriverBot );
	
	virtual void MoveVehicle( void );

	virtual void StartEngine( void );
	virtual void StopEngine( BOOL bImmediately = FALSE );

	virtual void ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS );
	virtual void HandleTargeting( void );

	virtual void ApplyVelocityImpulse_WS( const CFVec3A &rVelocityImpulseVec_WS );

	virtual void InflictDamageResult( const CDamageResult *pDamageResult );

	//----------------------------------------------------------------------------------------------------------------------------------
	// Private Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
private:
	void _ClearDataMembers( void );
	f32	 _CheckAltitude( const CFVec3A &vPt, const CFVec3A &vDir, f32 fMax, CFVec3A *pvGroundPt, CFVec3A *pvGroundNormal );

	void _InitPhysicsObject( void );
	void _GetColSphere( u32 uID, CFSphere *pSphere, const CFMtx43A &mtx  );				// given ColPoints_e, will return a sphere in WS
	
	void _ApplyControlForces( void );
	void _ApplyHoverForce( void );
	void _ApplyStabilizingTorque( void );

	void _ClawWork( void );
	BOOL _ValidateClawableObject( CMeshEntity *pME );				// test whether it's ok to keep trying to pick up this object
	static BOOL _StackedObjectTrackerCB( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	
	FINLINE const CFVec3A& _GetClawCenter( void )		{ return ((m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxClawLeft])->m_vPos); };

	static void _LoaderCollisionDetect( const CFPhysicsObject::PhysicsState_t *pState );

	void _ClawTriggerWork( void );									// handle the opening and closing of the claw
	void _CheckForClawEntity( void );								// check whether a potential victim is in our claws
	void _ClawOrientObject( void );									// attach the object to us, move it and orient it if necessary
	void _ClawAnimWork( void );										// handle claw animation work
	void _ClawLock( BOOL bLocked );									// claw is locked or unlocked, pickup or drop entities
	BOOL _CheckForStackedObject( void );							// will return true if there is an object above the crate that will interfere with lifting it
	void _CauseCrushDamage( CBot* pBot );							// crush this bot

	static BOOL _AltitudeTrackerCB( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	static BOOL _TrackerCollisionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	static BOOL _VerletTrackerCB( CFVerlet *pVerlet, CFWorldTracker *pTracker );
	static u32  _IntersectingTrackerCallback( CFWorldTracker *pTracker );

	void _EngineWork( void );										// handle the engines

	static BOOL _VerletStoppedTrackerCB( CFWorldTracker *pTracker, FVisVolume_t *pVolume, u32 nSphereIndex );
	static void _VerletMovedCB( CFVerlet *pVerlet, f32 fDeltaSecs );
	static void _VerletDeactivatedCB( CFVerlet *pVerlet );
	static void _VerletDeactivate( u32 uIdx, BOOL bCheckForAttach = TRUE );		// deactivate this verlet object
	static void _ClearEntityVerletObject( CEntity *pME );			// check to see whether this object has an active verlet object driving it.  If so, kill it

	u32	 _GetFreeVerletFromPool( void );							// get index of a free verlet from pool.  If none are free, kill one and return its index (should always return one)
	void _DropHeldObject( void );									// drop whatever object we're holding and make it a verlet object
	void _InitAndStartVerlet( CMeshEntity *pME );
	
	void _HandleDriverState( void );

	FINLINE void _ApplyControlForce( const CFVec3A &vForce )		{ m_vForceApplied.Add( vForce ); m_PhysObj.ApplyForce( &vForce, &m_vForceAppPos ); };
	FINLINE void _ApplyControlTorque( const CFVec3A &vTorque )		{ m_fTorqueApplied += vTorque.y; m_PhysObj.ApplyTorque( &vTorque ); };

	void _CameraTransitionWork( void );								// handles camera transitions when player enters/exits vehicle

	void _SoundWork( void );

	void _GetNormForcesAppliedByEngines( CFVec3A *pvLeft, CFVec3A *pvRight );

	void _WeaponWork( void );
	void _UpdateReticle( void );
	void _OrientWeapon( void );

	static void _AnimBoneCallback( u32 uBoneIdx, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );






	friend class CVehicleLoaderBuilder;

	FCLASS_STACKMEM_ALIGN( CVehicleLoader );
} FCLASS_ALIGN_SUFFIX;



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CVehicleSentinelBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CVehicleLoaderBuilder : public CVehicleBuilder 
{
public:
	BOOL m_bEnableGun;

	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CVehicleLoaderBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );

	//----------------------------------------------------------------------------------------------------------------------------------
	// Protected Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CVehicleLoaderBuilder );
} FCLASS_ALIGN_SUFFIX;
#endif
