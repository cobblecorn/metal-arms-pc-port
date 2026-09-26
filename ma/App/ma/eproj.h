//////////////////////////////////////////////////////////////////////////////////////
// eproj.h - Projectile entity class.
//
// Author: Steve Ranck     
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
// 06/10/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _EPROJ_H_
#define _EPROJ_H_ 1

#include "fang.h"
#include "entity.h"
#include "flinklist.h"
#include "fmesh.h"
#include "fworld.h"
#include "smoketrail.h"
#include "fxstreamer.h"
#include "fcoll.h"
#include "damage.h"
#include "bot.h"
#include "explosion.h"
#include "fgamedata.h"


class CBot;
class CEProj;
class CEProjPool;
class CEntity;
class CWeapon;
class CFSoundGroup;
class CFAudioEmitter;


#define EPROJ_COLL_THICK_RADIUS			0.5f
#define EPROJ_COLL_OVERLAP_Z_DIST		1.0f


typedef enum {
	EPROJ_ARROW_TYPE_SHRINK,
	EPROJ_ARROW_TYPE_BOOM,
	EPROJ_ARROW_TYPE_REMOTE_BOOM,

	EPROJ_ARROW_TYPE_COUNT
} EProj_Arrow_Type_e;



#define EPROJ_ARROW_STATICPARAMS_GAMEDATA_VOCAB \
	FGAMEDATA_VOCAB_SOUND_GROUP,							/* pSoundGroupSteam */		\
	FGAMEDATA_VOCAB_PARTICLE,								/* hParticleSteam */		\
	FGAMEDATA_VOCAB_EXPLODE_GROUP,							/* ahExplosionGroup[0] */	\
	FGAMEDATA_VOCAB_EXPLODE_GROUP,							/* ahExplosionGroup[1] */	\
	FGAMEDATA_VOCAB_EXPLODE_GROUP,							/* ahExplosionGroup[2] */	\
	FGAMEDATA_VOCAB_F32_MIN0,								/* fArrowLifeSecs */		\
	FGAMEDATA_VOCAB_F32_MIN0,								/* fTerminateAnimDuration */\
	FGAMEDATA_VOCAB_U32_BOUND( F32_DATATABLE_0, F32_DATATABLE_2 )	/* nType */



#define EPROJ_MERV_STATICPARAMS_GAMEDATA_VOCAB \
	FGAMEDATA_VOCAB_EXPLODE_GROUP,	/* hExplosionGroupSpawn */	\
	FGAMEDATA_VOCAB_EXPLODE_GROUP,	/* hExplosionGroupImpact */	\
	FGAMEDATA_VOCAB_SOUND_GROUP,	/* pSoundGroupIgnite */		\
	FGAMEDATA_VOCAB_F32_MINEPSILON,	/* fMervSpawnSecs */		\
	FGAMEDATA_VOCAB_F32_MINEPSILON,	/* fIgniteCoronaScale */	\
	FGAMEDATA_VOCAB_F32_UNIT,		/* fIgniteCoronaRed */		\
	FGAMEDATA_VOCAB_F32_UNIT,		/* fIgniteCoronaGreen */	\
	FGAMEDATA_VOCAB_F32_UNIT,		/* fIgniteCoronaBlue */		\
	FGAMEDATA_VOCAB_MOTIF,			/* nIgniteCoronaMotif */	\
	FGAMEDATA_VOCAB_F32_MIN0,		/* fIgniteAcceleration */	\
	FGAMEDATA_VOCAB_F32_MIN0,		/* fIgniteMaxSpeed */		\
	FGAMEDATA_VOCAB_F32_DEG2RAD,	/* fAngularSpeed */			\
	FGAMEDATA_VOCAB_F32_DEG2RAD,	/* fLaunchPitch */			\
	FGAMEDATA_VOCAB_F32_MIN0,		/* fLaunchSpeedMultMin */	\
	FGAMEDATA_VOCAB_F32_MIN0,		/* fLaunchSpeedMultMax */	\
	FGAMEDATA_VOCAB_F32_MIN0,		/* fSwarmingSpiralSpeed */	\
	FGAMEDATA_VOCAB_F32_MIN0,		/* fSwarmingBlendInDist */	\
	FGAMEDATA_VOCAB_F32_MIN0		/* fSwarmingAmplitude */




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEProjExt
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CEProjExt {
public:

	typedef struct {
		u32 nMinRicochetCount;							// We guarantee at least this many ricochets
		u32 nMaxRicochetCount;							// After this many, the saw will stick or carve

		BOOL bCarvingAllowed;							// TRUE if the saw supports carving
		const CDamageProfile *pCarveDamageProfile;		// Carving damage profile applied every N seconds (N is from the profile itself)
		f32 fCarveDamageSecs;							// The amount of time the carving will last
		f32 fCarveInitialSpeed;							// The initial speed of the carving saw

		f32 fDamageSphereRadius;						// Used to damage nearby entities

		f32 fMuzzleScale_Impact;						// Scale of impact flash
		f32 fMuzzleAlpha_Impact;						// Alpha of impact flash
		f32 fMuzzleOffset_Impact;						// Offset of impact flash

		CFSoundGroup *pSoundGroupRicochet;				// Blade ricochet sound
		CFSoundGroup *pSoundGroupStick;					// Blade stick sound
		CFSoundGroup *pSoundGroupCutStart;				// Blade cut start sound
		CFSoundGroup *pSoundGroupCutStop;				// Blade cut stop sound
		CFSoundGroup *pSoundGroupCutLoop;				// Blade cut loop sound
	} CEProj_Saw_Params_t;


	typedef struct {
		CFSoundGroup *pSoundGroupSteam;					// Steam looping sound (NULL if none)
		FParticle_DefHandle_t hParticleSteam;			// Steam particle (FPARTICLE_INVALID_HANDLE if none)

		FExplosion_GroupHandle_t ahExplosionGroup[3];	// Explosions. Types 0 and 1 use [0] only. Type 2 uses all three depending on how much the arrow is charged up.

		f32 fArrowLifeSecs;								// Total life time of the arrow
		f32 fTerminateAnimDuration;						// Duration of the steam or shrink effect
		EProj_Arrow_Type_e nType;						// Type of arrow
	} CEProj_Arrow_StaticParams_t;


	typedef struct {
		const FCollImpact_t *pCollImpact;					// This must persist through the call to CEProj::Launch()
		CEntity *pArrowIsStuckToEntity;						// The entity the arrow is stuck to (NULL=none)
		CEProj_Arrow_StaticParams_t *pArrowStaticParams;	// Static parameters from game data
	} CEProj_Arrow_Params_t;


	typedef struct {
		FExplosion_GroupHandle_t hExplosionGroupSpawn;	// Explosion to use when Mervs are spawned
		FExplosion_GroupHandle_t hExplosionGroupImpact;	// Explosion each Merv uses when it impacts
		CFSoundGroup *pSoundGroupIgnite;				// Sound used when Merv ignites
		f32 fMervSpawnSecs;								// Seconds until rocket splits into multiple rockets (0=not a Merv)
		f32 fIgniteCoronaScale;							// Corona scale to use on ignited Mervs
		f32 fIgniteCoronaRed;							// Corona color to use on ignited Mervs
		f32 fIgniteCoronaGreen;
		f32 fIgniteCoronaBlue;
		u32 nIgniteCoronaMotif;
		f32 fIgniteAcceleration;						// How quickly ignited Mervs accelerate
		f32 fIgniteMaxSpeed;							// Top speed of an ignited Merv
		f32 fAngularSpeed;								// How quickly Mervs can change their direction
		f32 fLaunchPitch;								// Angle Mervs are launched relative to their owner
		f32 fLaunchSpeedMultMin;						// Minimum launch speed multiplier
		f32 fLaunchSpeedMultMax;						// Maximum launch speed multiplier
		f32 fSwarmingAmplitude;							// Swarming amplitude
		f32 fSwarmingSpiralSpeed;						// Swarming spiral speed
		f32 fSwarmingBlendInInvDist;					// Swarming blend-in inverse distance
	} CEProj_Merv_StaticParams_t;


	typedef struct {
		f32 fSwarmAmplitude;							// Swarming radius (0=linear)
		f32 fSwarmingSpiralSpeed;						// Swarming spiral speed
		f32 fSwarmingBlendInInvDist;					// Swarming blend-in inverse distance
		CEProj_Merv_StaticParams_t *pMervStaticParams;	// NULL=not a Merv spawner
	} CEProj_Swarmer_Params_t;


	typedef struct {
		f32 fRocketMaxDist;								// Max distance the payload rockets can travel
		f32 fRocketMaxSecs;								// Max life of the payload rockets
		f32 fRocketMinSpeed;							// Min speed of the payload rockets
		f32 fRocketMaxSpeed;							// Max speed of the payload rockets
		f32 fRocketScale;								// Scale of the payload rockets
		f32 fRocketSwarmAmp;							// Swarming amplitude of the payload rockets
		FExplosion_GroupHandle_t hRocketExplosionGroup;	// Explosion group for the payload rockets
		CFSoundGroup *pSoundGroupWhir;					// Fly-away sound
	} CEProj_Cleaner_Params_t;


	typedef enum {
		CEPROJ_GRENADE_TYPE_CORING_CHARGE,
		CEPROJ_GRENADE_TYPE_EMP,
		CEPROJ_GRENADE_TYPE_MAGMABOMB,
		CEPROJ_GRENADE_TYPE_RECRUITER,
	} CEProj_Grenade_Type_e;


	FCLASS_ALIGN_PREFIX typedef struct {
		CEProj_Grenade_Type_e nGrenadeType;			// The type of grenade to simulate

		// Dampening vectors should be values between 0.0f and 1.0f for X, Y and Z
		// Dampening used as following: vel = vel - ( vel.Mul( Dampening ) )
		CFVec3A FloorDampening;						// How much to dampen velocity when we hit the floor
		CFVec3A WallDampening;						// How much to dampen velocity when we hit the wal

		BOOL bUseAngularWallDampening;				// Should we use angular wall dampening
		f32 afAngularWallDampeningRange[2];			// The range we should clamp the values to: [0] = min [1] = max

		f32 fRadiusPercentUse;						// How much of the radius to use when pushing the grenade off of the ground
		f32 fFatRadiusDelta;						// Amount to add to the grenade's radius when it's near an entity that will instantly detonate it

		// Only used with a type of CEPROJ_GRENADE_TYPE_EMP
		f32 fDetonationRadius;
		f32 fDetonationLife;
		f32 fShutdownTime;

		CFSoundGroup *pSoundGroupBounce;
		CFSoundGroup *pSoundGroupEMPEffect;
		CFSoundGroup *pSoundGroupRecruiterBolt;
	} FCLASS_ALIGN_SUFFIX CEProj_Grenade_Params_t;


	// Construct/Destruct:
	CEProjExt() {}
	virtual ~CEProjExt() {}


	// Initialization:
	virtual FINLINE BOOL Create( CEProj *pProj ) { return TRUE; }
	virtual FINLINE void Destroy( CEProj *pProj ) {}


	// Movement:
	virtual FINLINE void Init( CEProj *pProj ) {}
	virtual FINLINE void Launched( CEProj *pProj ) {}
	virtual FINLINE void Frozen( CEProj *pProj ) {}
	virtual FINLINE BOOL Detonated( CEProj *pProj, BOOL bMakeEffect, u32 nEvent, FCollImpact_t *pCollImpact ) { return TRUE; }

	virtual FINLINE BOOL Work( CEProj *pProj ) { return FALSE; }


	// State:
	virtual FINLINE void DrawEnable( BOOL bDrawingHasBeenEnabled ) {}
	virtual FINLINE void Removed( CEProj *pProj ) {}


	// Projectile-specific functions:
	virtual FINLINE void SetSwarmerParams( CEProj *pProj, const CEProj_Swarmer_Params_t *pParams ) {}
	virtual FINLINE void SetArrowParams( CEProj *pProj, const CEProj_Arrow_Params_t *pParams ) {}
	virtual FINLINE void GetDefaultSawParams( CEProj *pProj, CEProj_Saw_Params_t *pParams ) {}
	virtual FINLINE void SetSawParams( CEProj *pProj, const CEProj_Saw_Params_t *pParams ) {}
	virtual FINLINE void SetCleanerParams( CEProj *pProj, const CEProj_Cleaner_Params_t *pParams ) {}
	virtual FINLINE void SetGrenadeParams( const CEProj_Grenade_Params_t *pParams ) {}


	FCLASS_STACKMEM_ALIGN( CEProjExt );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEProj
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CEProj : public CEntity {

//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		PROJTYPE_ARROW,						// Arrow impact projectile
		PROJTYPE_SWARMER,					// Swarmer path
		PROJTYPE_GRENADE,					// Grenade projectile
		PROJTYPE_SAW,						// Saw blade
		PROJTYPE_CLEANER,					// Cleaner projectile
		PROJTYPE_SLOWER,					// Slower projectile
		PROJTYPE_MORTAR,					// Mortar projectile

		PROJTYPE_COUNT
	} ProjType_e;


	typedef enum {
		EVENT_REACHED_MAX_DIST,				// Projectile reached its maximum distance
		EVENT_HIT_GEO,						// Projectile hit terrain or object geometry
		EVENT_LIFE_TIME_OVER,				// Projectile has lived its time
		EVENT_DETONATE_API,					// Detonate() was called
		EVENT_INTERCEPTED,					// This projectile was destroyed by unfriendly fire before reaching target

		EVENT_COUNT
	} Event_e;


	// Called when the projectile hits geometry.
	//
	// pProj = Pointer to CEProj object
	// pImpact = collision impact when event has indicated EVENT_HIT_GEO (NULL if not).
	//           (pImpact->pTag will either be NULL for terrain or a pointer to the CFWorldMesh that it hit)
	//
	// Callback returns:
	//   TRUE = Destroy the projectile object
	//   FALSE = Do not destroy the projectile object
	typedef BOOL HitGeoCallback_t( CEProj *pProj, const FCollImpact_t *pImpact );


	// Called for the game to provide its own detonation callback.
	//
	// Callback returns:
	//   TRUE = The projectile system should make the default detonation effect.
	//   FALSE = The projectile system should not make any detonation effect.
	typedef BOOL DetonateCallback_t( CEProj *pProj, BOOL bMakeEffect, Event_e nEvent, const FCollImpact_t *pImpact );

	typedef void DamageCallback_t( CEProj *pProj, CDamageData *pDamageData );
	typedef void DeathCallback_t( CEProj* pProj );

	// Called to have the tracker skip list built in FWorld_apTrackerSkipList and FWorld_nTrackerSkipListCount.
	typedef void BuildTrackerSkipList_t( CEProj *pProj );




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
		PROJFLAG_ACTIVE_POOL		= 0x00000001,		// TRUE=The projectile is in the active pool
		PROJFLAG_LAUNCHED			= 0x00000002,		// TRUE=The projectile has been launched
		PROJFLAG_DESTROYED			= 0x00000004,		// TRUE=The projectile is destroyed
		PROJFLAG_MANUAL_XFM			= 0x00000008,		// TRUE=The extension class is responsible for updating m_pWorldMesh->m_Xfm
		PROJFLAG_THIN_PROJECTILE	= 0x00000010,		// TRUE=This is a thin projectile
		PROJFLAG_DETONATE_ON_IMPACT	= 0x00000020,		// TRUE=Detonate projectile when it first hits something (used by some projectile types)
		PROJFLAG_CREATE_SMOKETRAIL	= 0x00000040,		// TRUE=Create a smoketrail in the next cycle through the work function

		PROJFLAG_NONE				= 0
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static CFVec3A m_CollRayStart_WS;
	static const CFMtx43A *m_pCollUnitMtx;
	static f32 m_fCollRadius;
	static f32 m_fCollDeltaAngle;
	static f32 m_fCollOverlapDistZ;
	static u32 m_nCollPerimeterPointCount;
	static u16 m_nCollMask;


	ProjType_e m_nProjType;				// Projectile type
	CEProjExt *m_pProjExt;				// Projectile extension class
	u32 m_nProjFlags;					// See PROJFLAG_* for info

	CEProjPool *m_pProjPool;			// If this CEProj came from a pool, this is a pointer to the pool. Otherwise, this is NULL
	FLink_t m_Link;						// If this CEProj is in a pool, this links to other CEProj's in the pool

	DetonateCallback_t		*m_pFcnDetonateCallback;		// Detonate effect callback (NULL=use default effect)
	HitGeoCallback_t		*m_pFcnHitGeoCallback;			// Hit-geo callback function pointer (NULL=none)
	BuildTrackerSkipList_t	*m_pFcnBuildSkipListCallback;	// Function that builds the tracker skip list (NULL=none)
	DamageCallback_t		*m_pFcnDamageCallback;
	DeathCallback_t			*m_pFcnDeathCallback;			// 

	CFWorldMesh *m_pWorldMesh;			// The world mesh of this projectile (NULL=none)

	CDamageForm::Damager_t m_Damager;	// Damager info (entity, weapon, bot)
	CDamageProfile *m_pDamageProfile;	// The profile to use when inflicting damage (NULL=none) (not used by all projectile types)

	f32 m_fMaxDistCanTravel_WS;			// Maximum distance the projectile is allowed to travel
	f32 m_fDistTraveled_WS;				// Total distance traveled so far

	f32 m_fMaxLifeSecs;					// Maximum time the projectile is allowed to live (-1=infinite)
	f32 m_fLifeSecs;					// Life time so far

	f32 m_fLinSpeed_WS;					// Linear speed in world space
	f32 m_fRotSpeed_WS;					// Rotational speed around m_RotUnitAxis_WS in world units

	CFVec3A m_LinUnitDir_WS;			// Unit linear direction vector in world space
	CFVec3A m_RotUnitAxis_WS;			// Unit axis of rotation in world space
	CFVec3A m_SmokePos_MS;				// Smoke emitter position in model space

	CFSoundGroup *m_pLoopingSoundGroup;		// The sound group of the looping sound that's attached to the projectile (NULL=none)
	CFAudioEmitter *m_pLoopingAudioEmitter;	// The audio emitter of the looping sound that's attached to the projectile (NULL=none)

	SmokeTrailHandle_t m_hSmokeTrail;	// The smoke trail to use (SMOKETRAIL_NULLHANDLE=no smoke trail)
	SmokeTrailAttrib_t *m_pSmokeAttrib;	// The smoke trail attributes (NULL=none)

	FXStreamerHandle_t m_hStreamer;		// The streamer to use (FXSTREAMER_INVALID_HANDLE=no streamer)
	CFTexInst *m_pStreamerTexInst;		// The streamer texture (NULL=none)
	u32 m_nStreamerVtxCount;			// These are parameters for the streamer:
	f32 m_fStreamerAlpha;
	f32 m_fStreamerWidth;
	f32 m_fStreamerSamplesPerSec;
	CFXStreamerEmitter::UseAxis_e m_nStreamerAxis;
	CEntity *m_pTargetedEntity;

	FLightInit_t *m_pLightInit;
	CFWorldLightItem *m_pWorldLightItem;	// Used for a light attached to this projectile (NULL=none)
	CFWorldLightItem *m_pWorldLightItem2;	// Used for a light attached to this projectile (NULL=none)

	FExplosion_GroupHandle_t m_hExplosion;	// Used to spawn a default explosion when detonated (FEXPLOSION_INVALID_HANDLE if none)




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// Construct/Destruct:
	CEProj();
	virtual ~CEProj();


	// Create/Destroy:
	BOOL Create( ProjType_e nProjType, CFWorldMesh *pWorldMesh, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );
	BOOL Create( ProjType_e nProjType, cchar *pszMeshResName, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );


	// Type:
	FINLINE ProjType_e ProjType( void ) const { FASSERT( IsCreated() ); return m_nProjType; }


	// Info:
	FINLINE void *GetRelocateIdentifier( void ) const { FASSERT( IsCreated() ); return (void *)&m_fDistTraveled_WS; }

	FINLINE void SetActivePoolFlag( void ) { FASSERT( IsCreated() ); FMATH_SETBITMASK( m_nProjFlags, PROJFLAG_ACTIVE_POOL ); }
	FINLINE void ClearActivePoolFlag( void ) { FASSERT( IsCreated() ); FMATH_CLEARBITMASK( m_nProjFlags, PROJFLAG_ACTIVE_POOL ); }
	FINLINE BOOL IsActivePoolFlagSet( void ) const { FASSERT( IsCreated() ); return m_nProjFlags & PROJFLAG_ACTIVE_POOL; }


	// Properties:
	FINLINE void SetDamager( CWeapon *pDamagerWeapon, CBot *pDamagerBot );
	FINLINE void SetDamager( CWeapon *pDamagerWeapon, CBot *pDamagerBot, s32 nPlayerIndex );
	FINLINE const CDamageForm::Damager_t *GetDamager( void ) const { FASSERT( IsCreated() ); return &m_Damager; }
	FINLINE CWeapon *GetDamagerWeapon( void ) const { FASSERT( IsCreated() ); return m_Damager.pWeapon; }
	FINLINE CBot *GetDamagerBot( void ) const { FASSERT( IsCreated() ); return m_Damager.pBot; }

	FINLINE void SetDamageProfile( CDamageProfile *pDamageProfile ) { FASSERT( IsCreated() ); m_pDamageProfile = pDamageProfile; }
	FINLINE CDamageProfile *GetDamageProfile( void ) const { FASSERT( IsCreated() ); return m_pDamageProfile; }

	FINLINE BuildTrackerSkipList_t *GetSkipListCallback( void ) const { FASSERT( IsCreated() ); return m_pFcnBuildSkipListCallback; }
	FINLINE void SetSkipListCallback( BuildTrackerSkipList_t *pFcnSkipListCallback ) { FASSERT( IsCreated() ); m_pFcnBuildSkipListCallback = pFcnSkipListCallback; }

	FINLINE HitGeoCallback_t *GetHitGeoCallback( void ) const { FASSERT( IsCreated() ); return m_pFcnHitGeoCallback; }
	FINLINE void SetHitGeoCallback( HitGeoCallback_t *pFcnHitGeoCallback ) { FASSERT( IsCreated() ); m_pFcnHitGeoCallback = pFcnHitGeoCallback; }

	FINLINE DetonateCallback_t *GetDetonateCallback( void ) const { FASSERT( IsCreated() ); return m_pFcnDetonateCallback; }
	FINLINE void SetDetonateCallback( DetonateCallback_t *pFcnDetonateCallback ) { FASSERT( IsCreated() ); m_pFcnDetonateCallback = pFcnDetonateCallback; }

	FINLINE DamageCallback_t *GetDamageCallback( void ) const { FASSERT( IsCreated() ); return m_pFcnDamageCallback; }
	FINLINE void SetDamageCallback( DamageCallback_t *pFcnDamageCallback ) { FASSERT( IsCreated() ); m_pFcnDamageCallback = pFcnDamageCallback; }

	FINLINE DeathCallback_t *GetDeathCallback( void ) const { FASSERT( IsCreated() ); return m_pFcnDeathCallback; }
	FINLINE void SetDeathCallback( DeathCallback_t *pFcnDeathCallback ) { FASSERT( IsCreated() ); m_pFcnDeathCallback = pFcnDeathCallback; }

	FINLINE BOOL IsCollisionFlagSet( void ) const { FASSERT( IsCreated() ); if( m_pWorldMesh ) { return m_pWorldMesh->IsCollisionFlagSet(); } else { return FALSE; } }
	FINLINE void SetCollisionFlag( BOOL bSetFlag ) { FASSERT( IsCreated() ); if( m_pWorldMesh ) { m_pWorldMesh->SetCollisionFlag( bSetFlag ); } }

	FINLINE const CFSphere *GetBoundSphere_WS( void ) const { FASSERT( IsCreated() ); if( m_pWorldMesh ) { return &m_pWorldMesh->GetBoundingSphere(); } else { return NULL; } }
	FINLINE CFWorldMesh *GetWorldMesh( void ) const { FASSERT( IsCreated() ); return m_pWorldMesh; }

	FINLINE f32 GetMaxDistCanTravel( void ) const { FASSERT( IsCreated() ); return m_fMaxDistCanTravel_WS; }
	FINLINE void SetMaxDistCanTravel( f32 fMaxDistCanTravel ) { FASSERT( IsCreated() ); m_fMaxDistCanTravel_WS = fMaxDistCanTravel; }

	FINLINE f32 GetDistTraveled( void ) const { FASSERT( IsCreated() ); return m_fDistTraveled_WS; }
	FINLINE void SetDistTraveled( f32 fDistTraveled ) { FASSERT( IsCreated() ); m_fDistTraveled_WS = fDistTraveled; }

	FINLINE f32 GetMaxLifeSecs( void ) const { FASSERT( IsCreated() ); return m_fMaxLifeSecs; }
	FINLINE void SetMaxLifeSecs( f32 fMaxLifeSecs ) { FASSERT( IsCreated() ); m_fMaxLifeSecs = fMaxLifeSecs; }

	FINLINE f32 GetLifeSecs( void ) const { FASSERT( IsCreated() ); return m_fMaxLifeSecs; }
	FINLINE void SetLifeSecs( f32 fLifeSecs ) { FASSERT( IsCreated() ); m_fMaxLifeSecs = fLifeSecs; }

	FINLINE f32 GetLinSpeed( void ) const { FASSERT( IsCreated() ); return m_fLinSpeed_WS; }
	FINLINE void SetLinSpeed( f32 fLinSpeed ) { FASSERT( IsCreated() ); m_fLinSpeed_WS = fLinSpeed; }

	FINLINE f32 GetRotSpeed( void ) const { FASSERT( IsCreated() ); return m_fRotSpeed_WS; }
	FINLINE void SetRotSpeed( f32 fRotSpeed ) { FASSERT( IsCreated() ); m_fRotSpeed_WS = fRotSpeed; }

	FINLINE const CFVec3A *GetLinUnitDir_WS( void ) const { FASSERT( IsCreated() ); return &m_LinUnitDir_WS; }
	FINLINE void SetLinUnitDir_WS( const CFVec3A *pLinUnitDir_WS ) { FASSERT( IsCreated() ); m_LinUnitDir_WS = *pLinUnitDir_WS; }

	FINLINE const CFVec3A *GetRotUnitAxis_WS( void ) const { FASSERT( IsCreated() ); return &m_RotUnitAxis_WS; }
	FINLINE void SetRotUnitAxis_WS( const CFVec3A *pRotUnitAxis_WS ) { FASSERT( IsCreated() ); m_RotUnitAxis_WS = *pRotUnitAxis_WS; }

	FINLINE const CFVec3A *GetSmokePos_MS( void ) const { FASSERT( IsCreated() ); return &m_SmokePos_MS; }
	FINLINE void SetSmokePos_MS( const CFVec3A *pSmokePos_MS ) { FASSERT( IsCreated() ); m_SmokePos_MS = *pSmokePos_MS; }

	FINLINE FLightInit_t *GetLightInit( void ) const { FASSERT( IsCreated() ); return m_pLightInit; }
	FINLINE void SetLightInit( FLightInit_t *pLightInit ) { FASSERT( IsCreated() ); m_pLightInit = pLightInit; }

	void LoopingSoundOn( CFSoundGroup *pLoopingSoundGroup );
	FINLINE CFSoundGroup *GetLoopingSoundGroup( void ) const { FASSERT( IsCreated() ); return m_pLoopingSoundGroup; }
	FINLINE void LoopingSoundOff( void ) { FASSERT( IsCreated() ); _KillLoopingSound(); }

	void SmokeTrailOn( SmokeTrailAttrib_t *pSmokeTrailAttributes );
	FINLINE SmokeTrailAttrib_t *GetSmokeTrailAttributes( void ) const { FASSERT( IsCreated() ); return m_pSmokeAttrib; }
	FINLINE void SmokeTrailOff( void ) { FASSERT( IsCreated() ); _KillSmokeTrail(); }

	void StreamerOn( CFTexInst *pTexInst, f32 fAlpha, CFXStreamerEmitter::UseAxis_e nAxis, f32 fWidth, u32 nVtxCount=5, f32 fSamplesPerSec=15.0f );
	FINLINE void StreamerOff( void ) { FASSERT( IsCreated() ); _KillStreamer(); }

	void SetAttachedLight( CFWorldLightItem *pWorldLightItem );
	void SetAttachedLight2( CFWorldLightItem *pWorldLightItem );
	FINLINE CFWorldLightItem *GetAttachedLight( void ) const { FASSERT( IsCreated() ); return m_pWorldLightItem; }
	FINLINE CFWorldLightItem *GetAttachedLight2( void ) const { FASSERT( IsCreated() ); return m_pWorldLightItem2; }

	FINLINE void SetExplosionGroup( FExplosion_GroupHandle_t hExplosion ) { FASSERT( IsCreated() ); m_hExplosion = hExplosion; }
	FINLINE FExplosion_GroupHandle_t GetExplosionGroup( void ) { FASSERT( IsCreated() ); return m_hExplosion; }

	FINLINE void SetThinProjectile( void ) { FASSERT( IsCreated() ); FMATH_SETBITMASK( m_nProjFlags, PROJFLAG_THIN_PROJECTILE ); }
	FINLINE void SetThickProjectile( void ) { FASSERT( IsCreated() ); FMATH_CLEARBITMASK( m_nProjFlags, PROJFLAG_THIN_PROJECTILE ); }
	FINLINE BOOL IsThinProjectile( void ) const { FASSERT( IsCreated() ); return m_nProjFlags & PROJFLAG_THIN_PROJECTILE; }
	FINLINE BOOL IsThickProjectile( void ) const { FASSERT( IsCreated() ); return !(m_nProjFlags & PROJFLAG_THIN_PROJECTILE); }

	FINLINE void SetDetonateOnImpact( BOOL bDetonateOnImpact ) { FASSERT( IsCreated() ); FMATH_WRITEBIT( bDetonateOnImpact, m_nProjFlags, PROJFLAG_DETONATE_ON_IMPACT ); }
	FINLINE BOOL WillDetonateOnImpact( void ) const { FASSERT( IsCreated() ); return m_nProjFlags & PROJFLAG_DETONATE_ON_IMPACT; }


	// Projectile-specific functions:
	FINLINE void SetSwarmerParams( const CEProjExt::CEProj_Swarmer_Params_t *pParams ) { FASSERT( IsCreated() ); m_pProjExt->SetSwarmerParams( this, pParams ); }
	FINLINE void SetArrowParams( const CEProjExt::CEProj_Arrow_Params_t *pParams ) { FASSERT( IsCreated() ); m_pProjExt->SetArrowParams( this, pParams ); }
	FINLINE void GetDefaultSawParams( CEProjExt::CEProj_Saw_Params_t *pParams ) { FASSERT( IsCreated() ); m_pProjExt->GetDefaultSawParams( this, pParams ); }
	FINLINE void SetSawParams( const CEProjExt::CEProj_Saw_Params_t *pParams ) { FASSERT( IsCreated() ); m_pProjExt->SetSawParams( this, pParams ); }
	FINLINE void SetCleanerParams( const CEProjExt::CEProj_Cleaner_Params_t *pParams ) { FASSERT( IsCreated() ); m_pProjExt->SetCleanerParams( this, pParams ); }
	FINLINE void SetGrenadeParams( const CEProjExt::CEProj_Grenade_Params_t *pParams ) { FASSERT( IsCreated() ); m_pProjExt->SetGrenadeParams( pParams ); }

	// Actions:
	void Init( void );
	void Launch( void );
	void Freeze( void );
	void Detonate( BOOL bMakeEffect=TRUE, Event_e nEvent=EVENT_DETONATE_API, FCollImpact_t *pCollImpact=NULL );
	void SpawnExplosionEffect( Event_e nEvent, const FCollImpact_t *pImpact );

	FINLINE CEProjExt *GetExtensionObject( ProjType_e nProjType ){ FASSERT( m_nProjType == nProjType ); return m_pProjExt; }

	FINLINE BOOL IsLaunched( void ) { FASSERT( IsCreated() ); return( m_nProjFlags & PROJFLAG_LAUNCHED ); };

	FINLINE CEntity *GetTargetedEntity( void ) { FASSERT( IsCreated() ); return m_pTargetedEntity; }
	FINLINE void SetTargetedEntity( CEntity *pEntity ) { FASSERT( IsCreated() ); m_pTargetedEntity = pEntity; }

	FINLINE virtual CFWorldMesh *GetMesh( void ) const { FASSERT( IsCreated() ); return m_pWorldMesh; }



//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL ClassHierarchyBuild( void );
	virtual BOOL ClassHierarchyBuilt( void );
	virtual void ClassHierarchyDestroy( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );
	virtual void ClassHierarchyRelocated( void *pIdentifier );
	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );
	virtual void ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled );
	virtual void ClassHierarchyWork();
	virtual void InflictDamage( CDamageData *pDamageData );
	virtual void Die( BOOL bSpawnDeathEffects=TRUE, BOOL bSpawnGoodies=TRUE );

	void EmitStreamerPoint( void );

	void PlaySound( CFSoundGroup *pSoundGroup );
	CFAudioEmitter *AllocAndPlaySound( CFSoundGroup *pSoundGroup );

	static BOOL PerformThinRayCollisionTest( FCollImpact_t *pCollImpact, const CFVec3A *pPrevPos_WS, const CFMtx43A *pUnitMtx, BOOL bIgnoreWires=TRUE, f32 fOverlapDistZ=EPROJ_COLL_OVERLAP_Z_DIST );
	static BOOL PerformThickRayCollisionTest( FCollImpact_t *pCollImpact, const CFVec3A *pPrevPos_WS, const CFMtx43A *pUnitMtx, BOOL bIgnoreWires=TRUE, f32 fOverlapDistZ=EPROJ_COLL_OVERLAP_Z_DIST, f32 fCollRadius=EPROJ_COLL_THICK_RADIUS );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL _TrackersCallbackProjSphere( CFWorldTracker *pTracker, FVisVolume_t *pWorldLeafNode );

	void _ClearDataMembers( void );
	void _UpdateXfm( void );
	BOOL _Event( Event_e nEvent, const FCollImpact_t *pImpact );
	void _DestroyProjectile( void );
	void _StartStreamer( void );
	void _KillStreamer( void );
	void _KillSmokeTrail( void );
	void _KillAttachedLight( void );
	void _KillAttachedLight2( void );
	void _KillLoopingSound( void );


	// Friend classes;
	friend class CEProjPool;
	friend class CEProj_Arrow;
	friend class CEProj_Grenade;
	friend class CEProj_Swarmer;
	friend class CEProj_Saw;
	friend class CEProj_Cleaner;
	friend class CEProj_Slower;
	friend class CEProj_Mortar;
	friend class CEProj_Linear;


	FCLASS_STACKMEM_ALIGN( CEProj );
} FCLASS_ALIGN_SUFFIX;


FINLINE void CEProj::SetDamager( CWeapon *pDamagerWeapon, CBot *pDamagerBot ) {
	FASSERT( IsCreated() );
	
	m_Damager.pWeapon = pDamagerWeapon;
	m_Damager.pBot = pDamagerBot;

	if( pDamagerBot ) {
		m_Damager.nDamagerPlayerIndex = pDamagerBot->m_nPossessionPlayerIndex;
	} else {
		m_Damager.nDamagerPlayerIndex = -1;
	}
}


FINLINE void CEProj::SetDamager( CWeapon *pDamagerWeapon, CBot *pDamagerBot, s32 nPlayerIndex ) {
	FASSERT( IsCreated() );
	
	m_Damager.pWeapon = pDamagerWeapon;
	m_Damager.pBot = pDamagerBot;
	m_Damager.nDamagerPlayerIndex = nPlayerIndex;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEProjBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CEProjBuilder : public CEntityBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	CEProj::ProjType_e m_nProjType;	// The projectile type
	cchar *m_pszMeshResName;		// If building from a mesh name, this is the mesh resource name (NULL otherwise)
	CFWorldMesh *m_pWorldMesh;		// If building from a world mesh, this points to the existing world mesh (NULL otherwise)




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CEProjBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CEProjBuilder );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEProjPool
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_NOALIGN_PREFIX class CEProjPool {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef void *PoolHandle_t;

	#define EPROJPOOL_NULL_HANDLE	((CEProjPool::PoolHandle_t)0)




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;	// TRUE: InitSystem() has been called
	static FLinkRoot_t m_PoolLinkRoot;	// Linklist of all CEProjPool's

	static CFCollInfo m_CollInfoFat;
	static CFTrackerCollideProjSphereInfo m_CollProjSphereInfo;
	static BOOL m_bCollIgnoreWires;


	FMesh_t *m_pMesh;					// The mesh this pool dispenses
	u32 m_uMeshInstFlags;				// The flags for this projectile

	FLink_t m_PoolLink;					// Link to other CEProjPool's
	FLinkRoot_t m_FreeProjLinkRoot;		// Linklist of all free CEProj's
	FLinkRoot_t m_ActiveProjLinkRoot;	// Linklist of all active CEProj's

	BOOL8 m_bAddedToPoolList;			// TRUE: This object has been added to the pool linklist
	BOOL8 m_bBuiltBeforeWorld;			// TRUE: This pool was built before the world was created
	BOOL8 m_bCreated;					// TRUE: This pool has been created

	CEProj::ProjType_e m_nProjType;		// The type of projectiles in this pool
	u32 m_nProjCount;					// Number of projectiles in m_pProjArray
	CEProj *m_pProjArray;				// Array of projectile entities in this pool
	CFWorldMesh *m_pWorldMeshArray;		// Array of world mesh objects




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );
	static BOOL IsSystemInitialized( void ) { return m_bSystemInitialized; }


	// Creation:
	static PoolHandle_t Create( CEProj::ProjType_e nProjType, cchar *pszMeshResName, u32 nProjCount, u32 m_uFlags=FMESHINST_FLAG_NONE);
	static PoolHandle_t Create( CEProj::ProjType_e nProjType, FMesh_t *pMeshRes, u32 nProjCount, u32 m_uFlags=FMESHINST_FLAG_NONE);


	// General:
	static u32 GetFreePoolProjectileCount( PoolHandle_t hPool );
	static CEProj *GetProjectileFromFreePool( PoolHandle_t hPool );
	static void ReturnProjectileToFreePool( CEProj *pProj );

	static void RemoveProjectileFromAllPools( CEProj *pProj );
	static void ReturnAllProjectilesToFreePoolForAllPools( void );
	static void ReturnAllProjectilesToFreePool( PoolHandle_t hPool );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	// Construction/Destruction:
	CEProjPool();
	~CEProjPool();

	// General:
	BOOL _AllocatePoolArrays( void );
	void _Destroy( void );
	static void _ResDestroyedCallback( void *pResMem );
	static BOOL _WorldCallback( FWorldEvent_e nEvent );

	// Pool linklist:
	static void _Pool_Init( void ) { flinklist_InitRoot( &m_PoolLinkRoot, FANG_OFFSETOF( CEProjPool, m_PoolLink ) ); }
	static CEProjPool *_Pool_GetHead( void ) { return (CEProjPool *)flinklist_GetHead( &m_PoolLinkRoot ); }
	static CEProjPool *_Pool_GetTail( void ) { return (CEProjPool *)flinklist_GetTail( &m_PoolLinkRoot ); }
	CEProjPool *_Pool_GetNext( void ) { return (CEProjPool *)flinklist_GetNext( &m_PoolLinkRoot, this ); }
	CEProjPool *_Pool_GetPrev( void ) { return (CEProjPool *)flinklist_GetPrev( &m_PoolLinkRoot, this ); }
	void _Pool_Add( void ) { flinklist_AddTail( &m_PoolLinkRoot, this ); }
	void _Pool_Remove( void ) { flinklist_Remove( &m_PoolLinkRoot, this ); }

	// Free projectile linklist:
	void _FreeProj_Init( void ) { flinklist_InitRoot( &m_FreeProjLinkRoot, FANG_OFFSETOF( CEProj, m_Link ) ); }
	CEProj *_FreeProj_GetHead( void ) { return (CEProj *)flinklist_GetHead( &m_FreeProjLinkRoot ); }
	CEProj *_FreeProj_GetTail( void ) { return (CEProj *)flinklist_GetTail( &m_FreeProjLinkRoot ); }
	CEProj *_FreeProj_GetNext( CEProj *pProj ) { return (CEProj *)flinklist_GetNext( &m_FreeProjLinkRoot, pProj ); }
	CEProj *_FreeProj_GetPrev( CEProj *pProj ) { return (CEProj *)flinklist_GetPrev( &m_FreeProjLinkRoot, pProj ); }
	void _FreeProj_AddTail( CEProj *pProj ) { flinklist_AddTail( &m_FreeProjLinkRoot, pProj ); }
	CEProj *_FreeProj_RemoveTail( void ) { return (CEProj *)flinklist_RemoveTail( &m_FreeProjLinkRoot ); }
	void _FreeProj_Remove( CEProj *pProj ) { flinklist_Remove( &m_FreeProjLinkRoot, pProj ); }

	// Active projectile linklist:
	void _ActiveProj_Init( void ) { flinklist_InitRoot( &m_ActiveProjLinkRoot, FANG_OFFSETOF( CEProj, m_Link ) ); }
	CEProj *_ActiveProj_GetHead( void ) { return (CEProj *)flinklist_GetHead( &m_ActiveProjLinkRoot ); }
	CEProj *_ActiveProj_GetTail( void ) { return (CEProj *)flinklist_GetTail( &m_ActiveProjLinkRoot ); }
	CEProj *_ActiveProj_GetNext( CEProj *pProj ) { return (CEProj *)flinklist_GetNext( &m_ActiveProjLinkRoot, pProj ); }
	CEProj *_ActiveProj_GetPrev( CEProj *pProj ) { return (CEProj *)flinklist_GetPrev( &m_ActiveProjLinkRoot, pProj ); }
	void _ActiveProj_AddTail( CEProj *pProj ) { flinklist_AddTail( &m_ActiveProjLinkRoot, pProj ); }
	CEProj *_ActiveProj_RemoveTail( void ) { return (CEProj *)flinklist_RemoveTail( &m_ActiveProjLinkRoot ); }
	void _ActiveProj_Remove( CEProj *pProj ) { flinklist_Remove( &m_ActiveProjLinkRoot, pProj ); }


	friend class CEProj;


	FCLASS_STACKMEM_NOALIGN( CEProjPool );
} FCLASS_NOALIGN_SUFFIX;







#endif

