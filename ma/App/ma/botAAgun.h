//////////////////////////////////////////////////////////////////////////////////////
// botAAgun.h -
//
// Author: Michael Starich
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
// 3/14/03 Starich       Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _BOTAAGUN_H_
#define _BOTAAGUN_H_ 1

#include "bot.h"
#include "vehicle.h"
#include "reticle.h"
#include "eproj.h"
#include "fforce.h"
#include "fdecal.h"
#include "fsound.h"

FCLASS_ALIGN_PREFIX class CBotAAGun : public CBot {

//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// Misc definitions:
	enum {
		ANIM_IDLE_COUNT					= 0,
		NUM_BARRELS						= 4,
		ANIM_BASE_COUNT					= 0,
		BARREL_TO_SHEATH_BONE_OFFSET	= 4,
		BARREL_TO_TIP_BONE_OFFSET		= 8,
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

	// Animation controls:
	typedef enum
	{
		ANIMCONTROL_REST,
		ANIMCONTROL_AIM_SUMMER,
		ANIMCONTROL_BASE_COUNT
	} AnimControl_e;

	// Animation taps:
	typedef enum
	{
		ANIMTAP_REST,
		ANIMTAP_AIM_SUMMER,
		ANIMTAP_BASE_COUNT
	} AnimTap_e;

	// Bones:
	typedef enum {
		// DON'T CHANGE THE ORDER OF THE BARRELS IN THIS LIST
		BONE_BARREL_T_R,
		BONE_BARREL_B_L,
		BONE_BARREL_T_L,
		BONE_BARREL_B_R,
		BONE_SHEATH_T_R,
		BONE_SHEATH_B_L,
		BONE_SHEATH_T_L,
		BONE_SHEATH_B_R,
		// DON'T CHANGE THE ORDER OF THE BARRELS IN THIS LIST
		BONE_PRIMARY_FIRE1,
		BONE_PRIMARY_FIRE2,
		BONE_PRIMARY_FIRE3,
		BONE_PRIMARY_FIRE4,
		BONE_HORIZONTAL,		
		BONE_VERTICAL,  		
		BONE_ATTACHPOINT_BOT,	
		BONE_SECONDARY,			
		BONE_BASE,

		BONE_COUNT
	} Bone_e;

	typedef struct {
        f32 fShotSpreadRate;
		f32	fShotSpreadRecRate;
		f32	fMinShotSpread;
		f32	fMaxShotSpread;
		f32	fMaxRange;
		f32	fMuzzleLightRadius;
		f32	fMuzzleImpactAlpha;
		f32	fMuzzleImpactOffset;
		f32	fPotmarkScale;
		f32	fTracerTime;
		f32	fTracerWidth;
		f32	fTracerAlpha;
		f32	fSoundVolume3D;
		f32	fSoundRadius3D;
		f32	fSoundVolume2D;
		CFSoundGroup *pGunFire;
		f32 fMaxHeading;		// *Over rideable through user props
		f32 fMinHeading;		// *Over rideable through user props
		f32 fHeadingPerSec;		// *Over rideable through user props
		f32 fPitchPerSec;		// *Over rideable through user props
		f32 fSecsBetweenBarrelFires;
		f32 fRecoilBack;
		f32 fPercentRecoilBackwards;
		CDamageProfile *pPrimaryWeaponDamageProfile;	// Damage profile for vehicle's primary weapon
		FParticle_DefHandle_t hFlashPartDef;
		FParticle_DefHandle_t hSmokePartDef;
		FParticle_DefHandle_t hFlakPartDef;
		f32 fReticleX;
		f32 fReticleY;
		f32 fEnterRadius;
		f32 fEnterHalfHeight;
		CFSoundGroup *pRotateTuret;
		FDecalDefHandle_t hBulletDecalDef;
			} BotInfo_AAGun_t;

	typedef struct {
		u32 uPoolCannonsBalls;
		f32 fRoundsPerSec;
		f32 fOORoundsPerSec;
#if FANG_WINGC
		f32 fFiringTime;					// Retail field 2 (X and 1/X, as CBotMortar's); not used yet
		f32 fOOFiringTime;
#endif
		FExplosion_GroupHandle_t hExplLaunchGroup;
		FExplosion_GroupHandle_t hExplDetonateGroup;
		f32 fMinCannonballVelocityXZ;
		f32 fMaxCannonballVelocityXZ;
		f32	fMinRangeXZ;
		f32	fMaxRangeXZ;
		f32 fMinShockwaveImpulse;
		f32 fMaxShockwaveImpulse;
		CFSoundGroup *pCannonBallInAir;
		CFSoundGroup *pMortarFire;
		CFSoundGroup *pMortarExplode;
		CFSoundGroup *pMortarNoFire;
#if FANG_WINGC
		CDamageProfile *pDetonateDamageProfile;	// Retail field 15 (AAGunMortarDetonate); not used yet
		f32 fRetailField16;						// Retail field 16, [0, 10000], 0.333 in the retail data; meaning unknown
#endif
	} MortarInfo_t;

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
	static BotInfo_Weapon_t m_BotInfo_Weapon;			// Weapon bot info
	static BotInfo_AAGun_t m_BotInfo_AAGun;				// AAGun bot info
	static MortarInfo_t m_MortarInfo;					// mortar specific info
		
	static const FGameDataMap_t m_aGameDataMap[];
	static const FGameData_TableEntry_t m_aBotInfoVocab_AAGun[];
	static const FGameData_TableEntry_t m_aBotInfoVocab_Mortar[];
		
	// Bot animation stack data:
	static CBotAnimStackDef m_AnimStackDef;				// Animation stack definition data

	static cchar *m_apszBoneNameTable[BONE_COUNT];										// Array of bone names
	static cchar *m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT];					// Array of base animation control names
	static cchar *m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT];							// Array of base animation tap names

	static const u8 *m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT];
	static const u8 m_anEnableBoneNameIndexTableForSummer_Normal[];
	static const u8 m_anTagPointBoneNameIndexArray[];
	
	static CFAnimCombinerConfig::ConfigStack_t m_aAnimCombinerConfigStack[];
	static CFAnimCombinerConfig::ConfigTap_t m_aAnimCombinerConfigTaps[];
	static CFAnimCombiner::AttachList_t m_aAnimAttach[];

	// shot vars:
	static f32 m_fSecsBackward;
	static f32 m_fOOSecsBackward;
	static f32 m_fSecsForward;
	static f32 m_fOOSecsForward;
	static f32 m_fSecsBetweenShots;

	CFVec3A m_vUnFocusedTargetPoint_WS;

	// mortar vars:
	static CEProjPool::PoolHandle_t	m_hMortarProjPool;
	f32 m_fSecsTillNextMortarFire;
	f32 m_fSecsTillNextMortarClick;
			
	// Misc:
	BOOL m_bLimitHeading;		// from the user props
	f32 m_fMaxHeading;			// *Over rideable through user props
	f32 m_fMinHeading;			// *Over rideable through user props
	f32 m_fMinPitch;			// *Over rideable through user props	
	f32 m_fMaxPitch;			// *Over rideable through user props
	f32 m_fHeadingPerSec;		// *Over rideable through user props
	f32 m_fPitchPerSec;			// *Over rideable through user props
	BOOL m_bMorterAttachment;	// *Over rideable through user props

	FForceHandle_t	m_hForce;	// Force feedback handle so we can kill it when we need to

	// Barrel and firing related vars:
	f32 m_fGunHeading;
	f32 m_fGunPitch;
	f32 m_fSecsTillNextFire;
	u32 m_nNextFireBarrelIndex;// less than NUM_BARRELS
	f32 m_afBarrelRecoilZ[NUM_BARRELS];
	f32 m_afBarrelTimer[NUM_BARRELS];
	f32 m_afOuterBarrelRecoilZ[NUM_BARRELS];
	FParticle_EmitterHandle_t m_ahEmitters[NUM_BARRELS];
	f32 m_fGunUnitShotSpread;
	f32 m_fGunAISoundTimer;
	f32 m_fGunTracerTimer;
	f32 m_fSecsTillNextFlakFx;
	CFAudioEmitter *m_pRotSoundEmitter;
	f32 m_fRotVolume;

	// Animation:
	s32 m_anBoneIndices[BONE_COUNT];
	CFAnimMeshRest m_MeshRestAnimSource;
	CFAnimManMtx m_AnimManMtxAim;

	// operator related:
	CBot *m_pDriverBot;											// pointer to bot currently operating the gun
	CVehicle::StationStatus_e m_eStationStatus;	
	f32 m_fEntryJumpTimer;
	BOOL8 m_bAllowDriverMovement;
	BOOL8 m_bAllowDriverExit;
	BOOL8 m_bDisplayEnterMsg;
	BOOL8 m_____bPad;

	// camera
	f32 m_fUnitCameraTransition;
	CCamManualInfo m_CamInfo;
	CFMtx43A m_ManCamMtx;
	CFMtx43A m_TransCamMtx;
	CameraState_e m_eCameraState;								// state of aa gun camera
	CBot *m_pCameraBot;											// pointer to bot involved in camera transitions

	// bot settings that need to be restored upon leaving the gun
	BOOL8 m_bPrevCastingShadows;
	BOOL8 m_bPrevInvincible;
	BOOL8 m_bPrevSpotlightOn;

	// stored reticle data
	BOOL8 m_bReticleDrawPrev;
	f32	m_fReticlePrevX;
	f32	m_fReticlePrevY;	
	CReticle::Type_e m_ReticlePrevType;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	CBotAAGun();
	virtual ~CBotAAGun();

	static BOOL InitSystem( void );
	static void UninitSystem( void );
	
	BOOL Create( s32 nPlayerIndex=-1, 
				BOOL bInstallDataPort=FALSE, 
				cchar *pszEntityName=NULL,
				const CFMtx43A *pMtx=NULL,
				cchar *pszAIBuilderName=NULL );

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);
	
	virtual BOOL ActionNearby( CEntity *pEntity );

	// checkpoint load/save functions
	virtual void CheckpointSaveSelect( s32 nCheckpoint );
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );
	virtual void CheckpointRestorePostamble( void );

	FINLINE BOOL IsGunInUse( void )				{ return (m_eStationStatus == CVehicle::STATION_STATUS_OCCUPIED); }

	void SwiftKickOutDriver( void );
	FINLINE CBot *GetDriverBot( void )			{ return m_pDriverBot; }

	// getting bots into the gun interface (used the vehicle interface for consistency)
		// the following functions are borrowed from botloader
	void DriverEnter( CBot *pDriverBot, cchar *pszAttachPointBoneName=NULL );
	void DriverExit( CBot *pDriverBot );
	CVehicle::StationStatus_e CanOccupyStation( CBot *pBot, CVehicle::Station_e eStation );
	
	CVehicle::StationStatus_e EnterStation( CBot *pBot, CVehicle::Station_e eStation, BOOL bProximityCheck=TRUE );
	CVehicle::StationStatus_e ExitStation( CBot *pBot, CVehicle::Station_e eStation );
	BOOL GetStationEntryPoint( CVehicle::Station_e eStation, CFVec3A *pPoint );

	void EnableDriverMovement( BOOL bEnable );
	void EnableDriverExit( BOOL bEnable );
	void EnableEnterMsgDisplay( BOOL bEnable );
	BOOL ShouldEnterTextBeDisplayed( void );

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	// Entity functions:
	virtual BOOL ClassHierarchyLoadSharedResources( void );
	virtual void ClassHierarchyUnloadSharedResources( void );
	virtual void ClassHierarchyDestroy( void );
	virtual BOOL ClassHierarchyBuild( void );				// Must be implemented by all classes in the derivation chain
	virtual BOOL ClassHierarchyBuilt( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );	// Must be implemented by all classes in the derivation chain
	virtual void ClassHierarchyWork( void );
	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );

	// Bot functions:
	virtual void InflictDamage( CDamageData *pDamageData);
	virtual void ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS );
	virtual void HandleTargeting( void );
	

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	
	static BOOL _BuildAnimStackDef( void );

	void _ClearDataMembers( void );
	void _UpdateMatrices( void );

	void _HandleDriverState( void );
	void _CameraTransitionWork( void );

	// these are modified from vehicle
	void _DriverEnter( CBot *pDriverBot, cchar *pszAttachPointBoneName=NULL );
	void _DriverExit( CBot *pDriverBot );

	void _YawPitchWork( f32 fControllerX, f32 fControllerY, f32 fMouseYaw = 0.0f, f32 fMousePitch = 0.0f );
	void _BarrelWork( void );
	void _PrimaryWeaponWork( BOOL bPrimaryFireDown );
	void _SecondaryWeaponWork( BOOL bSecondaryFireDown );
	void _UpdateReticle( void );

	static void _ParticleEmitterCallback( FParticle_EmitterHandle_t hHandle, void *pUserData );

	void _ResetToNeutral( void );
	static BOOL _ProjDetonateCallback( CEProj *pProj, BOOL bMakeEffect, CEProj::Event_e nEvent, const FCollImpact_t *pImpact );
	void _UpdateMountVars( void );
	
	friend class CBotAAGunBuilder;

	FCLASS_STACKMEM_ALIGN( CBotAAGun );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotAAGunBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotAAGunBuilder : public CBotBuilder {

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	BOOL m_bFull360;
	f32 m_fMaxRightHeading;
	f32 m_fMaxDownPitch;
	BOOL m_bMorterAttachment;
	f32 m_fHeadingPerSec;
	f32 m_fPitchPerSec;
	

	FINLINE CBotAAGunBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );

	FCLASS_STACKMEM_ALIGN( CBotAAGunBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif