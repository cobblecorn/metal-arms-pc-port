#ifndef _SITEFLOORSENTRY_H_
#define _SITEFLOORSENTRY_H_ 1

#include "site_botWeapon.h"
#include "eproj.h"
#include "fforce.h"
#include "fdecal.h"

#include "Weapon_Recruiter.h"

FCLASS_ALIGN_PREFIX class CFloorSentry : public CBotSiteWeapon::CSiteWeaponAnimator
{
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef struct 
	{						
		f32 fClipAmmoMax;					// Maximum number of clip rounds (<0 means infinite)
		f32 fReserveAmmoMax;				// Maximum number of reserve rounds (<0 means infinite)
		f32 fRoundsPerSec;					// Maximum fire rate
		f32 fOORoundsPerSec;				// 1.0f / fRoundsPerSec
		f32 fMaxLiveRange;					// The maximum distance the projectile can travel before self-destructing
		f32 fDistFromWeaponOrigToMuzzle;	// Distance from the weapon's origin to where the projectiles come out
		f32 fBestShotSpreadFactor;			// Tightest shot spread factor
		f32 fWorstShotSpreadFactor;			// Widest shot spread factor
		f32 fShotSpreadSpeed;				// Speed at which shot spreads out
		f32 fShotSpreadRecoverySpeed;		// Speed at which shot spreads in

		f32 fMuzzleWidth_Z;					// Width of the muzzle flash Z
		f32 fMuzzleHeight_Z;				// Height of the muzzle flash Z
		f32 fMuzzleAlpha_Z;					// Alpha of muzzle flash Z

		f32 fMuzzleScale_XY;				// Scale of muzzle flash XY
		f32 fMuzzleAlpha_XY;				// Alpha of muzzle flash XY

		f32 fMuzzleScale_Mesh;				// Scale of muzzle flash mesh
		f32 fMuzzleRot_Mesh;				// Rotation of muzzle flash mesh

		f32 fMuzzleScale_Impact;			// Scale of impact flash
		f32 fMuzzleAlpha_Impact;			// Alpha of impact flash
		f32 fMuzzleOffset_Impact;			// Offset of impact flash

		f32 fBulletDamage;
		f32 fExplosionDamage;
		f32 fFlameDamage;

		cchar *pszDamageProfile;			// Damage profile string (references a damage profile in damage.csv)

		f32 fSoundRadius;					// Distance sound from this weapon travels

		f32 fCameraShakeIntensity;			
		f32 fCameraShakeDuration;
		FDecalDefHandle_t hBulletDecal;

	} _FloorSpewProps_t;

	typedef struct 
	{						
		cchar *apszMeshName;				// Mesh names
		f32 fClipAmmoMax;					// Maximum number of clip rounds (<0 means infinite)
		f32 fReserveAmmoMax;				// Maximum number of reserve rounds (<0 means infinite)
		f32 fRoundsPerSec;					// Maximum fire rate
		f32 fOORoundsPerSec;				// 1.0f / fRoundsPerSec
		f32 fShotSpreadFactor;				// Shot spread factor
		f32 fRocketsInPoolCount;			// The number of rockets in the projectile pool
		f32 fMaxLiveRange;					// The maximum distance the rocket can travel before self-destructing
		f32 fRocketCullDist;				// Don't draw the rocket if it's this far from the camera
		f32 fRocketSpeed;					// Rocket's linear speed
		f32 fDistFromWeaponOrigToMuzzle;	// Distance from the weapon's origin to where the projectiles come out
		f32 fBulletDamage;
		f32 fExplosionDamage;
		f32 fFlameDamage;
		cchar *pszDamageProfile;			// Damage profile string (references a damage profile in damage.csv)
		f32 fSoundRadius;
		FExplosion_GroupHandle_t hExplosionGroup;
	} _FloorRocketProps_t;
	typedef enum
	{
		RED,
		YELLOW,
		OFF
	}Light_e;

//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	enum
	{
		ANIM_IDLE_COUNT = 0,
		TAG_POINT_COUNT = 4,						// Number of tag points on guy
		MAX_SIMULTANEOUS_FIRE_SOUNDS	= 3
	};

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

	// Base animations:
	typedef enum
	{
		ANIM_BASE_COUNT=0,
	} Anim_e;

	typedef enum
	{
		BONE_HORIZONTAL,	// "Horizontal",
		BONE_VERTICAL,  	// "Vertical",
		BONE_BARREL_T_R,    // "Barrel_T_R",
		BONE_BARREL_B_R,	// "Barrel_B_R",
		BONE_BARREL_T_L,	// "Barrel_T_L",
		BONE_BARREL_B_L,	// "Barrel_B_L",
		BONE_ATTACHPOINT_BOT,	// "AttachPoint_Bot",
		BONE_PRIMARY_FIRE1,	// "primary_fire1",
		BONE_PRIMARY_FIRE2,	// "primary_fire2",
		BONE_PRIMARY_FIRE3,	// "primary_fire3",
		BONE_PRIMARY_FIRE4,	// "primary_fire4",
		BONE_ATTACHPOINT_LIGHT,
		BONE_COUNT
	} Bone_e;	

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:
//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static SmokeTrailAttrib_t m_aSmokeTrailAttrib;
	static const FGameData_TableEntry_t m_aSpewPropVocab[]; 
	static const FGameData_TableEntry_t m_aRocketPropVocab[];
    static const FGameDataMap_t m_aGameDataMap[];
	static _FloorSpewProps_t	m_aSpewProps[2];
	static _FloorRocketProps_t  m_aRocketProps[2];
	static CDamageProfile *m_aSpewDamageProfile[2];
	static CDamageProfile *m_aRocketDamageProfile[2];

	// Bot animation stack data:
	static CBotAnimStackDef m_AliveAnimStackDef;						// Animation stack definition data
	static CBotAnimStackDef m_DeadAnimStackDef;							// Animation stack definition data
	
	static cchar *m_apszBoneNameList[BONE_COUNT];						// Array of bone names

	static cchar *m_apszBoneNameList2[2];								// 2nd Array of bone names

	static cchar *m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT];	// Array of base animation control names
	static cchar *m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT];			// Array of base animation tap names
	static const u8 *m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT];
	static const u8 m_anEnableBoneNameIndexTableForSummer_Normal[];
	static const u8 m_anTagPointBoneNameIndexArray[TAG_POINT_COUNT];

	static CFAnimCombinerConfig::ConfigStack_t	m_aAnimCombinerConfigStack[];
	static CFAnimCombinerConfig::ConfigTap_t	m_aAnimCombinerConfigTaps[];
	static CFAnimCombiner::AttachList_t			m_aAnimAttach[];
	
	static CBotPartPool*   m_pPartPool;

	CFAudioEmitter *m_apAudioEmitterFire[MAX_SIMULTANEOUS_FIRE_SOUNDS];	// Audio emitter used when we're playing the bullet firing sound (NULL=not playing forming sound)
	u32 m_nAudioEmitterIndex;						// Index of next audio emitter in m_apAudioEmitterFire[] to use

	CBotAnimStack m_DeadAnim;												// Animation stack

	BOOL m_bPossessed;
	u32 m_uAIVisibleSoundHandle;


	// Mesh handles for geometry
	CFWorldMesh m_AliveMesh;
	CFWorldMesh m_DeadMesh;
	CFWorldMesh m_apCustomBarrels[4];

	CWeaponRecruiter m_DummyRecruiter; // handle to load class resources with...
	// rest anim sources
	CFAnimMeshRest		m_AliveMeshRestAnimSource;
	CFAnimMeshRest		m_DeadMeshRestAnimSource;
	
	CFMtx43A		m_MtxRecoil;

	CFMtx43A* m_apMuzzlePts[5];
//	CFMtx43A* m_pSearchLightMtx;

	CFAnimManMtx	m_AliveAnimManMtxAim;
	CFAnimManMtx	m_DeadAnimManMtxAim;

	BOOL m_bFirePrimaryThisFrame;
	f32 m_fLastFireTime;
	Light_e m_eLightPage;
	u16 m_nSpewClipAmmo;							// Number of rounds in the clip (if infinite clip ammo, this is INFINITE_AMMO)
	u16 m_nSpewReserveAmmo;							// Number of rounds outside the clip (if infinite reserve ammo, this is INFINITE_AMMO)
	f32 m_fSpewFiringDelay;
	f32 m_fSpewUnitShotSpread;
	f32 m_fSpewSecondsUntilNextTracer;
	f32 m_fSpewSecondsCountdownTimer;
	
	f32 m_fRocketSecondsCountdownTimer;
	
	f32 m_fRecoilDistanceCustom;
	f32 m_fRecoilDistanceNormal;
	f32 m_fRecoilReturnRate;

	f32			m_fRecoil0;
	BOOL		m_bFireTopBarrels;
	CBot*		m_pOwnerBot;

	f32			m_fPowerDownVelocity;
	f32			m_fPowerDownVelocityDamp;
	f32			m_fPowerDownAcceleration;

	f32			m_fDeathRandomAngularVelocityRange; // the R.A.V will be between 0 and here (ever want neg??)
	f32			m_fDeathRandomAngularVelocity;		// the velocity to add per frame 
	f32			m_fDeathVelocityDamp;				// how much velocity is left after hitting an extrema
	f32			m_fDeathAcceleration;				// downward pull from gravity
	f32			m_fOOSparksPerSecond;
	f32			m_fSparkCountDown;
	const CFVec3A *m_apTagPoint_WS[TAG_POINT_COUNT];	// Tag points in world space
	CFVec3A		m_RocketAimUnitVec_WS;
	CFColorRGBA m_vRedLightColor;
	CFColorRGBA m_vYellowLightColor;
	CFWorldAttachedLight* m_pWorldLight;
	f32				m_fCurrentLightAlpha;
	CReticle::Type_e m_OldReticleType;
	f32				m_fOldNormX;
	f32				m_fOldNormY;
	BOOL			m_bOldReticleIsDrawn;
	CBotSiteWeapon::State_e m_ePreviousState;
	_FloorSpewProps_t*		m_pSpewProps;
	_FloorRocketProps_t*	m_pRocketProps;
	CDamageProfile *		m_pSpewDamageProfile;
	CDamageProfile *		m_pRocketDamageProfile;
	f32						m_fUnitTimeUntilRocketReady;
	FForceHandle_t			m_hForce;						// Force feedback handle so we can kill it when we need to
	
	FMeshTexLayerHandle_t	hLightLayerHandle;
	FMeshTexLayerHandle_t	hPanelLayerHandle;
	
	
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static BOOL InitSystem( void );
	static void UninitSystem( void );

	virtual ~CFloorSentry();
	virtual BOOL	Create( CBotSiteWeapon::SiteWeaponData_t* pData, void* pUser, const u64 userTypeBits );
	virtual void	RemoveFromWorld(void);
	virtual void	AddToWorld(void);
	virtual void	Destroy(void);

	virtual u32 GetTagPointCount( void ) const;
	virtual const CFVec3A *GetTagPoint( u32 nTagPointIndex ) const;

	virtual CFMtx43A* AttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName );

	virtual u32		TriggerWorkPrimary( f32 fUnitTriggerVal1);
	virtual u32		TriggerWorkSecondary( f32 fUnitTriggerVal2);

	virtual void 	Work();
	virtual void 	CueNewState();

	virtual void	ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS );
	virtual BOOL	CheckpointSave( void );
	virtual void	CheckpointRestore( const CBotSiteWeapon::SiteWeaponData_t& rSiteData );

	virtual void	PreUserRender( void );
	virtual void	PostUserRender( void );

	virtual void	AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);
	virtual BOOL	CanOccupyStation(CBot * pBot);
//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static	void _BuildRocketSkipList( CEProj *pProj );
	static	BOOL _RL12_ProjDetonateCallback( CEProj *pProj, BOOL bMakeEffect, CEProj::Event_e nEvent, const FCollImpact_t *pImpact );
	static	void _SetSmokeTrailAttributes( void );

			void 	_PowerUpWork(void);						// my other tick
			void 	_PowerDownWork(void);					// my other other tick
			void 	_DeathWork(void);						// my other other other tick
			void 	_PossessedPowerUpWork(void);			// my other tick
			void 	_CuePowerUp(void);						// turn me on
			void 	_CuePowerDown(void);// WHIRRRWhiRRwwwhir.
			void	_CueDeath(void);		// time to die
			void	_CuePossessedPowerUp(void);
			void	_CuePossessed(BOOL bGoPosed);		// transform!
			void	_ComputeCameraMatrix_WS(void);
			void	_ComputeRocketDirection(f32 fTargetDist);
			void	_DeadMeshDraw( BOOL bDrawDeadMesh);
			void	_HandleFireThisFrame(void);
			void	_UpdateLight(void);
			void	_KillAudioEmitters(void);
public:
	FCLASS_STACKMEM_ALIGN( CFloorSentry );
} FCLASS_ALIGN_SUFFIX;

#endif