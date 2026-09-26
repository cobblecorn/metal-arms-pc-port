//////////////////////////////////////////////////////////////////////////////////////
// botSiteWeapon.h -
//
// Author: michael scholz
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
// 11/08/02 MScholz 	Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _BOTSITEWEAPON_H_
#define _BOTSITEWEAPON_H_ 1

#include "bot.h"
#include "gamecam.h"
#include "CamManual.h"
#include "reticle.h"

#define CSTEP_EPSILON					(0.000797f)
#define CSTEP_FLOATS_EQUAL(x,y)			( FMATH_FABS((x) - (y)) < CSTEP_EPSILON )
#define CSTEP_FLOAT_ISZERO(x)			( FMATH_FABS(x) < CSTEP_EPSILON )

#define _stringize( L )		#L
#define _makestring( M, L )	M(L)
#define _line				_makestring( _stringize, __LINE__ )
#define msg(str)			message( __FILE__ "(" _line ") : " str)

//#define VEHICLE_FLAG_DRIVER_LIGHT_ON	0x00000020		// true if driver's spotlight was on when entering vehicle
//#define VEHICLE_FLAG_DRIVER_HAS_SHADOW	0x00000100		// TRUE if driver had shadow upon entry into vehicle


class CAISound;
class CFSoundGroup;

FCLASS_ALIGN_PREFIX class CStepMotor
{
public:
	void Reset()
	{
		m_fCurrentVal = 0.0f;
		m_fCurrentVelocity = 0.0f;
		m_fTargetVal =0.0f;
	}

	void Create(f32 fInitialPos, f32 fMaxAccel, f32 fMaxVelocity, f32 fTarget)
	{
		m_fCurrentVal = fInitialPos;
		m_fMaxAcceleration = fMaxAccel;
		m_fMinAcceleration = -fMaxAccel;
		m_fMaxVelocity = fMaxVelocity;
		m_fMinVelocity = -fMaxVelocity;
		m_fTargetVal = fTarget;
		m_fCurrentVelocity = (0.0f);
	}
 
	void SetValue(f32 fVal){m_fCurrentVal=fVal;}
	f32  GetValue(void){return m_fCurrentVal;}

	void SetTarget(f32 fVal){m_fTargetVal=fVal;}
	f32  GetTarget(void){return m_fTargetVal;}

	f32 GetVelocity(void){return m_fCurrentVelocity;}
	void SetVelocity(f32 fNewVel){m_fCurrentVelocity=fNewVel;}

	void SetMaxVelocity(f32 fVal){m_fMaxVelocity = fVal;m_fMinVelocity = -fVal;}
	void SetMaxAcceleration(f32 fVal){m_fMaxAcceleration = fVal;m_fMinAcceleration = -fVal;}

	f32 Work(void)
	{
		f32 fStoppingDistance=0.0f;
		f32 fApplyAcceleration=0.0f;

		if (!CSTEP_FLOATS_EQUAL(m_fCurrentVal,m_fTargetVal))
		{
			// COMPUTE DESIRED ACCELERATION
			if (m_fCurrentVelocity > 0)
				fStoppingDistance = -fmath_Div((m_fCurrentVelocity*m_fCurrentVelocity),(2*m_fMinAcceleration));
			else
				fStoppingDistance = -fmath_Div((m_fCurrentVelocity*m_fCurrentVelocity),(2*m_fMaxAcceleration));

			// aim for myTargetPos minus stoppingDistance
			f32 fDesiredMoveAmount = m_fTargetVal - m_fCurrentVal;
			fDesiredMoveAmount -= fStoppingDistance;

			f32 fDesiredVelocity = fDesiredMoveAmount * FLoop_fPreviousLoopOOSecs;
			f32 fDesiredAcceleration = fDesiredVelocity * FLoop_fPreviousLoopOOSecs;
			fApplyAcceleration = FMATH_MIN(fDesiredAcceleration, m_fMaxAcceleration);
			fApplyAcceleration = FMATH_MAX(fApplyAcceleration, m_fMinAcceleration);

			// COMPUTE NEW VELOCITY
			m_fCurrentVelocity = m_fCurrentVelocity + fApplyAcceleration * FLoop_fPreviousLoopSecs;

			m_fCurrentVelocity = FMATH_MIN(m_fCurrentVelocity ,m_fMaxVelocity);
			m_fCurrentVelocity = FMATH_MAX(m_fCurrentVelocity ,m_fMinVelocity);

			// COMPUTE NEW POSITION
			m_fCurrentVal = m_fCurrentVal + m_fCurrentVelocity * FLoop_fPreviousLoopSecs;
		}// end !equal
		else
		{
			m_fCurrentVelocity=0.0f;
		}
		return m_fCurrentVal;
	}// end work
private:
	f32 m_fCurrentVelocity;
	f32 m_fCurrentVal;
	f32 m_fTargetVal;
	f32 m_fMaxAcceleration;
	f32 m_fMaxVelocity;
	f32 m_fMinAcceleration;
	f32 m_fMinVelocity;
	FCLASS_STACKMEM_ALIGN( CStepMotor );
} FCLASS_ALIGN_SUFFIX;

class CSiteWeaponAnimator;

FCLASS_ALIGN_PREFIX class CBotSiteWeapon : public CBot
{
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	typedef enum 
	{
		SITE_NULL, // not a state;
		// states where AI drives the bot
		SITE_IDLING,
		SITE_PATROLLING,	// smooth scans, no firing
		SITE_TURNING,		// faster than patrol, faces a point
        SITE_TARGETING,		// keep target in range long enough to fire
		SITE_ATTACKING,		// on or off, barrels blaze
		SITE_STRAFING,		// can't see you, but shooting anyway
        SITE_SEARCHING,		// there's a point of interest that's got my attention
		SITE_DEFENDING,		// respond to damage
		
		SITE_CONFUSION,	 // I'm confused, no shooting, target swivel (rare)
		
		// states where control is passed off to constants or player
		SITE_ENTERING_POSSESSED,
		SITE_POWERING_UP_POSSESSED,
		SITE_POSSESSED,

		SITE_POWER_UP,	 
		SITE_POWER_DOWN,
		SITE_DEAD,
		SITE_STATE_COUNT
	}State_e;
	State_e GetSiteWeaponState(void){return m_pData->m_eSiteWeaponState;}
	
	typedef struct
	{
		CFMtx43A m_MtxPosition_WS; // the matrix for the site weapon's origin
		CFMtx43A m_MtxAimX_MS;     // matrix used to pitch
		CFMtx43A m_MtxAimY_MS;	   // matrix used to yaw
		
		CFMtx43A  m_MtxCam_WS;				// matrix of the camera, use aim orientation and camera position... 
		CFVec3A   m_vAimingOrigin_WS;		// what's the point of basic aiming from 
		CFVec3A	  m_vEnemyVelocity;

		CCamManualInfo m_CamManInfo;
		
		CFMtx43A* m_pMtxAimOrientation_WS;	// world space aiming matrix;
		CBotSiteWeapon::State_e m_eSiteWeaponState;

		f32 m_fDistanceToTarget;
		f32 m_fYawAdjustWS2MS;
		f32 m_fPitchAdjustWS2MS;
		f32 m_fYawWS;
		f32 m_fPitchWS;
		f32 m_fYawVelocity;
	
		f32 m_fMaxYawVelocity;
		f32 m_fMaxYawVelocityPossess;
		f32 m_fMaxYawVelocityPatrol;
		f32 m_fMaxYawAcceleration;
		f32 m_fMaxPitchAcceleration;
		f32 m_fMaxPitchVelocity;
		f32 m_fMaxPitchVelocityPossess;

		f32 m_fMinimumPitchWS;
		f32 m_fMaximumPitchWS;
		f32 m_fMinimumYawWS;
		f32 m_fMaximumYawWS;

		f32 m_fTimeToPowerUp;
		f32 m_fTimeToPowerDown;
		f32 m_fTimeToEnter;
		f32 m_fTimeSinceStateChanged;
		CBot* m_pDriverBot;

		CBot*   m_pEnemyTarget;
		f32     m_pEnemyTargetDistance2;

		BOOL m_bEnableLight;
		f32	 m_fLightRadiusOverride;


		FCLASS_STACKMEM_ALIGN( SiteWeaponData_t );
	} SiteWeaponData_t;

	typedef struct
	{
		cchar*	pszArmorProfile;
		f32		fMaxYawVelocity;
		f32		fMaxYawVelocityPossess;
		f32		fMaxYawVelocityPatrol;
		f32		fMaxYawAcceleration;
		f32		fMaxPitchAcceleration;
		f32		fMaxPitchVelocity;
		f32		fMaxPitchVelocityPossess;
		f32		fMinimumPitch;
		f32		fMaximumPitch;
		f32		fMinimumYaw;
		f32		fMaximumYaw;
	} BotInfo_SiteWeapon_t;

	typedef struct
	{
		f32		fPatrolPitch;
		f32		fPatrolMinYaw;
		f32		fPatrolMaxYaw;	
		f32		fVisionCosHalfFOV;
		f32		fVisionDistance;
		f32		fRadarCosHalfFOV;
		f32		fRadarDistance;
		f32		fRadarVelocity;
		f32		fTargetingYawSin;
		f32		fSpewFireYawSin;
		f32		fRocketFireYawSin;
		f32		fAttackDelayTime;
		f32		fStrafingTime;
		f32		fSearchingTime;
		f32		fDefendingTime;
	} BotInfo_SiteWeaponAI_t;

	typedef struct
	{
		f32 fMinCapBlendAngle;
		f32 fMaxCapBlendAngle;
		f32 fMaxPitchCapped;
	} BotInfo_RatGun_t;

	typedef struct
	{
		FExplosion_GroupHandle_t hExplosion;
		FExplosion_GroupHandle_t hExplosion2;
		FParticle_DefHandle_t hSmoke;
		s32	nExplosions;
		f32 fMinTimeBetweenExpl;
		f32 fMaxTimeBetweenExpl;
		f32 fSecSmoke;
	} BotInfo_Death_t;

	typedef enum 
	{
		SITESND_POWERUP,
		SITESND_POWERDOWN,
		SITESND_DEATH,
		SITESND_FIRE,
		SITESND_ROCKET,
		SITESND_LOCK,
		SITESND_ROTATE,
		
		// added for pillbox
		SITESND_POSSESS,
		SITESND_DISPOSSESS,
		
		SITESND_LIGHTSWITCH,

		SITESND_COUNT
	}SiteSound_e;

	class CSiteWeaponAnimator // made non-pure for dev purposes
	{
	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Virtual Interface:
	//----------------------------------------------------------------------------------------------------------------------------------
	public:
		virtual ~CSiteWeaponAnimator(){};
		virtual BOOL	Create(CBotSiteWeapon::SiteWeaponData_t* pData, void* pUser, const u64 userTypeBits )=0;
		virtual void	RemoveFromWorld(void)=0;
		virtual void	AddToWorld(void)=0;
		virtual void	Destroy(void)=0;

		virtual u32 GetTagPointCount( void ) const =0;
		virtual const CFVec3A *GetTagPoint( u32 nTagPointIndex ) const=0;

		virtual CFMtx43A* AttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName )=0;
		
		virtual u32		TriggerWorkPrimary( f32 fUnitTriggerVal1)=0;
		virtual u32		TriggerWorkSecondary( f32 fUnitTriggerVal2)=0;

		virtual void 	Work()=0;
		virtual void 	CueNewState()=0;

		virtual void	ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS )=0;
		virtual BOOL	CheckpointSave( void )=0;
		virtual void	CheckpointRestore( const CBotSiteWeapon::SiteWeaponData_t& rSiteData )=0;

		virtual void PreUserRender( void ) {}
		virtual void PostUserRender( void ) {}

		virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0])=0;
		virtual BOOL CanOccupyStation(CBot * pBot)=0;

		
	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Data
	//----------------------------------------------------------------------------------------------------------------------------------
	public:
	CFWorldMesh* m_pAliveMesh;
	CFWorldMesh* m_pDeadMesh;
	f32 m_fSecsUntilNextSound;	// How frequently this weapon can make an AI sound	

	//----------------------------------------------------------------------------------------------------------------------------------
	// Protected Data
	//----------------------------------------------------------------------------------------------------------------------------------
	protected:
		CBotSiteWeapon::SiteWeaponData_t* m_pData;

	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions
	//----------------------------------------------------------------------------------------------------------------------------------
	public:
		FINLINE	s32		GetPlayerIndex(void) {	return ( (m_pData->m_pDriverBot == NULL) ? -1 : m_pData->m_pDriverBot->m_nPossessionPlayerIndex);}
		FINLINE BOOL	IsOwnedByPlayer(void){ return (GetPlayerIndex() >= 0);}
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

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static s32 m_nBoneIndexPillboxAttachPt;
	static s32 m_nBoneIndexFloorSentryAttachPt;

//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	// Class Scope variables
	static BOOL		m_bSystemInitialized;		// TRUE: InitSystem() has been called
	static u32		m_nBotClassClientCount;		// Number of bots of this class using the class-wide resources

	// BotInfo data:
	
	static const FGameDataMap_t m_aGameDataMap[];

	static BotInfo_Gen_t		m_BotInfo_Gen;			// General bot info
	static BotInfo_Gen_t		m_BotInfo_PillBoxGen;	// General bot info
	static BotInfo_Gen_t		m_BotInfo_FloorSentryGen;// General bot info

	static BotInfo_MountAim_t	m_BotInfo_MountAim;		// Mount aim bot info
	static BotInfo_Weapon_t		m_BotInfo_Weapon;		// Weapon bot info
	static BotInfo_Walk_t		m_BotInfo_Walk;			// Walk bot info
	
	static BotInfo_Death_t		m_Death_Floor;
	static BotInfo_Death_t		m_Death_Wall;
	static BotInfo_Death_t		m_Death_PillBox;

	static const FGameData_TableEntry_t m_BotInfo_SiteWeaponVocab[];
	static const FGameData_TableEntry_t m_BotInfo_SiteAIWeaponVocab[];
	static const FGameData_TableEntry_t m_BotInfo_RatGunVocab[];
	static const FGameData_TableEntry_t m_BotInfo_DeathVocab[];
	static BotInfo_SiteWeapon_t m_BotInfo_FloorSentry;
	static BotInfo_SiteWeapon_t m_BotInfo_CeilingSentry;
	static BotInfo_SiteWeapon_t m_BotInfo_PillBox;
	static BotInfo_SiteWeapon_t m_BotInfo_RatGun;
	static BotInfo_RatGun_t		m_BotInfo_RatGunXtra;

	static BotInfo_SiteWeaponAI_t m_BotInfo_FloorSentryAI;
	static BotInfo_SiteWeaponAI_t m_BotInfo_CeilingSentryAI;
	
	static CFSoundGroup* m_pSounds[SITESND_COUNT];

	CFAudioEmitter* m_pSoundEmitters[SITESND_COUNT];
    
	BotInfo_SiteWeapon_t	m_SiteWeaponInfo;
	BotInfo_SiteWeaponAI_t	m_SiteWeaponAI;
	SiteWeaponData_t		m_SiteWeaponData;

	BotInfo_Death_t*		m_pDeathInfo;

	CFVec3A m_vRadarDirectionWS;
	CFVec3A m_vImpactPoint_WS;
	CFVec3A m_vUnitDirToDamageSource;

	f32			m_fFixedEyeOffset;
	f32			m_fTimeToNextExplosion;
	s32			m_nExplosionsLeftInDeath;
	
	CStepMotor m_RotateXAxis;
	CStepMotor m_RotateYAxis;
	SiteWeaponData_t* m_pData;
	
	CSiteWeaponAnimator* m_pGun;
	s32 m_nLastTagPointInspected;
	f32 m_fLastBeepTime; 
	f32 m_fStrafingTime;
	f32 m_fSearchingTime;
	f32 m_fSearchingInitialYaw;
	f32 m_fSearchingInitialPitch;
	f32 m_fSearchTheta;
	f32 m_fRotateVolume;
	f32 m_fDefendingTime;
	BOOL m_bAimRockets;
	BOOL m_bDamageTaken;
	BOOL m_bSearchPointReached; // 0= turning to targPt, 1 is circling
	
	u32 m_uVehicleFlags;		// vehicle flags a la chris

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	virtual ~CBotSiteWeapon();

	static BOOL InitSystem( void );
	static void UninitSystem( void );
	
	static CFSoundGroup* GetSiteWeaponSndHandle(SiteSound_e eSound){return m_pSounds[eSound];}
#if FANG_WINGC
	// Reads a site weapon's game data file (paMap) whose gun tables (those using pGunVocab) have the
	// retail 31-field layout: this source's 22 leading fields, four inserted smoke fields, then the
	// damage profile, sound radius, two camera-shake values and the decal. Other tables load as-is.
	static BOOL ReadRetailGunTables( const FGameDataMap_t *paMap, const FGameData_TableEntry_t *pGunVocab, cchar *pszFileName );
#endif
	static BOOL GetRandomPointOnMeshSurface(CFVec3A& rPoint, CFVec3A& rNormal, const CFWorldMesh* pMesh);

	virtual BOOL ActionNearby( CEntity *pEntity );
			BOOL Create( s32 eBotSubClass=0, s32 nPlayerIndex=-1, BOOL bInstallDataPort=FALSE, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );
	virtual void Power( BOOL bPowerUp, f32 fPowerOffTime=0.0f, f32 fPowerOffOnSpeedMult=1.0f );
	virtual void Possess( s32 nPlayerIndex, f32 fPossessionArmorModifier );

	virtual BotRace_e SetRace( BotRace_e eNewRace);

	virtual u32 GetTagPointCount( void ) const;
	virtual const CFVec3A *GetTagPoint( u32 nTagPointIndex ) const;

	virtual void CheckpointSaveSelect( s32 nCheckpoint );
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );
	virtual void CheckpointRestorePostamble( void );

	virtual void PreUserRender( void ) { if (m_pGun) m_pGun->PreUserRender(); }
	virtual void PostUserRender( void ) { if (m_pGun) m_pGun->PostUserRender(); }

	BOOL		 SetSiteWeaponDriver(CBot* pWhom, BOOL bDriversChoice=TRUE); // NULL to exit;
	BOOL		 SetOpenOrClosed(BOOL bOpen, f32 fHowLongItShouldTakeSecs );
	BOOL		 GetOpenOrClosed(void); // 1 mean open, 0 means closed
	
	// Functions provide to pat for a.i.
			BOOL  BotIsEligibleToOperatePillbox(CBot* pBot);
			CBot *IsStationObstructed( void );
			BOOL GetStationEntryPoint( CFVec3A *pPoint );
			BOOL CanOccupyStation( CBot * pBot){return m_pGun->CanOccupyStation(pBot);}

FINLINE CFMtx43A* BaseMtxToWorld( void ) const { return (CFMtx43A *)&m_pData->m_MtxPosition_WS; }
FINLINE		void  GetTurretRanges(f32* pfYawMin_WS,
								  f32* pfYawMax_WS,
								  f32* pfPitchMin_WS,
								  f32* pfPitchMax_WS) const 
			{
				*pfYawMin_WS = m_pData->m_fMinimumYawWS;
				*pfYawMax_WS = m_pData->m_fMaximumYawWS;
				*pfPitchMin_WS = m_pData->m_fMinimumPitchWS;
				*pfPitchMax_WS = m_pData->m_fMinimumPitchWS;
			}
				
	FINLINE CBot* GetDriverBot(void) {return m_SiteWeaponData.m_pDriverBot;}
	virtual void Die( BOOL bSpawnDeathEffects=TRUE, BOOL bSpawnGoodies=TRUE );
	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]) ;
	static void AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	FINLINE CSiteWeaponAnimator *GetMyGun( void ) const { return m_pGun; };
	FINLINE CCamManualInfo *GetCameraInfo( void ) const { return &m_pData->m_CamManInfo; };
	FINLINE f32 GetGunPitch( void ) const { return m_pData->m_fPitchWS; };
	FINLINE f32 GetGunYaw( void ) const { return m_pData->m_fYawWS; };

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
	
	virtual void ClassHierarchyWork( void ); // the update function!

	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );
	virtual	void ClassHierarchyRelocated( void *pIdentifier );

	virtual void SetControls( CEntityControl *pControl );
	
	virtual void InflictDamageResult( const CDamageResult *pDamageResult );
	virtual void ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS );

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL _ProposedVisionTarget( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	static BOOL _ProposedRadarTarget( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	static BOOL _SoundCollisionCB(CAISound* pSound, void* pData);

	void _SetDataMembersToDefaults( void );
	void _SetDataMembersToBuilder ( void );
	void _RotateBaseNoise(BOOL bOn,f32 fUnitVolume=1.0f);

    static BOOL _InitSoundHandles(void);
	
	void _SetSiteWeaponState(State_e);
	void _CallSiteStateFunction(void);
	f32  _aihelper_aim_at_m_TargetedPoint(); // used by multiple to maintain a target, returns positive when good lock
	BOOL _aihelper_can_see_you(CBot* pYOU);
	void _aihelper_SearchForTarget(void);
	void _ai_patrolling(void);
	void _ai_targeting(void);
	void _ai_turning(void);
	void _ai_attacking(void);
	void _ai_strafing(void);
	void _aihelper_SearchPoint(f32 fAngularVelocity = 2.0f,	f32 fYawRadius = 0.15f,	f32 fPitchRadius = 0.10f);
	void _ai_searching(void);
	void _ai_defending(void);	
	void _ai_confusion(void);
	// not really ai, renaming:
	void _Do_Idling(void);
	void _Do_Entering_Possessed(void);
	void _Do_Powering_Up_Possessed(void);
	void _Do_Possessed(void);
	void _Do_Power_Up(void);
	void _Do_Power_Down(void);		
	void _Do_Dead(void);
	void _MoveXYAxes(void);
	f32 _TurnToTargetAndFocusRadar2d(const CFVec3A& Location,f32* pDistToTarget);
	
//----------------------------------------------------------------------------------------------------------------------------------
// FriendShip
//----------------------------------------------------------------------------------------------------------------------------------
	friend class CBotSiteWeaponBuilder;
	
	FCLASS_STACKMEM_ALIGN( CBotSiteWeapon );
} FCLASS_ALIGN_SUFFIX;

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotSiteWeaponBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotSiteWeaponBuilder : public CBotBuilder
{
public:
	CBotSiteWeaponBuilder();

	// Set default initializes builder data to class defaults, 
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );
	
	// then interpret table changes the overrideable members to match
	virtual BOOL InterpretTable( void );

	virtual BOOL PostInterpretFixup( void );
	
	CBotDef	m_WallSentryBotDef;
    CBotDef	m_MilFloorSentryBotDef;
	CBotDef	m_DroidFloorSentryBotDef;
	CBotDef	m_AmbientPillBoxBotDef;
	CBotDef	m_DroidPillBoxBotDef;
	CBotDef	m_AmbientRatGunBotDef;
	CBotDef	m_DroidRatGunBotDef;

	CBotSiteWeapon::BotInfo_SiteWeapon_t m_SiteWeaponInfo;
	CBotSiteWeapon::BotInfo_SiteWeaponAI_t m_SiteWeaponAI;

	BOOL m_bEnableLight;
	f32	 m_fLightRadiusOverride;

	FCLASS_STACKMEM_ALIGN( CBotSiteWeaponBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif
