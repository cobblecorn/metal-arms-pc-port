//////////////////////////////////////////////////////////////////////////////////////
// eproj_saw.h - Saw projectiles.
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
// 06/19/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _EPROJ_SAW_H_
#define _EPROJ_SAW_H_ 1

#include "fang.h"
#include "eproj.h"
#include "smoketrail.h"


class CEProj;
class CFTrackerCollideRayInfo;



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEProj_Saw
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CEProj_Saw : public CEProjExt {
//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	enum {
		MAX_CARVE_PROJ_COUNT	= 5,					// The maximum number of projectiles that can be in carve mode simultaneously
		DAMAGE_FORM_COUNT		= 3						// Up to this many entities can be damaged by the proximity sphere in a single frame
	};


	typedef enum {
		STATE_DEAD,										// Projectile is dead
		STATE_FLYING,									// Projectile is flying through the air
		STATE_CARVING_SURFACE,							// Projectile is carving a surface
		STATE_SPIN_DOWN,								// Projectile is spinning down
		STATE_WAITING_TO_SHRINK,						// Projectile is stuck to surface and waiting to shrink

		STATE_COUNT
	} State_e;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static u32 m_nCarvingCount;							// The number of projectiles currently in carve mode
	static SmokeTrailAttrib_t m_ImpactSmokeTrailAttrib;	// Smoke used when saw hits something
	static SmokeTrailAttrib_t m_CarveSmokeTrailAttrib;	// Smoke used when saw is carving
	static CFCollInfo m_CollInfo;
	static CFCollInfo m_CollInfoNearbySphereTest;
	static CFTrackerCollideRayInfo m_CollRayInfo;
	static FCollImpact_t m_CollImpact;
	static CFVec3A m_ImpactNormal;
	static CFVec3A m_ImpactPoint;
	static CEntity *m_pHitEntity;
	static CEProj *m_pCollProj;
	static const CDamageProfile *m_pCollDamageProfile;
	static CDamageForm *m_apDamageFormArray[DAMAGE_FORM_COUNT];
	static u32 m_nDamageFormCount;


	CEProj_Saw_Params_t m_SetupParms;				// Parameters specified by launcher
	State_e m_nState;								// Our current state
	u32 m_nRicochetCount;							// Number of times this projectile has rocochet

	f32 m_fCountdownTimer;							// Used to time various things
	f32 m_fSawSpinAngle;							// Current saw blade spin angle
	f32 m_fDeltaShrinkRate;							// 0=not shrinking, >0=shrinking

	const CDamageProfile *m_pCarveDamageProfile;	// Carving damage profile applied every N seconds (N is from the profile itself)
	f32 m_fCarveDamageCountdownTimer;				// Used to space out the damage we're doing while in carve mode
	f32 m_fCarveSurfaceJumpAngle;					// Normally 0, but can change when we're jumping from one surface to another
	f32 m_fCarveDeceleration;						// Linear deceleration while in carve mode
	f32 m_fCarveSparkTimer;							// When this reaches zero, spawn some sparks
	CEntity *m_pCarveEntity;						// The entity we're carving
	CFQuatA m_CarveMeshQuat;						// Mesh's current quaternion when we're in carve mode
	CFAudioEmitter *m_pAudioEmitterCarving;			// Carving sound




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );


	// Construct/Destruct:
	CEProj_Saw() {}
	~CEProj_Saw() {}


	// Extension interface:
	virtual BOOL Create( CEProj *pProj );
	virtual void Destroy( CEProj *pProj );

	virtual void Init( CEProj *pProj );
	virtual void GetDefaultSawParams( CEProj *pProj, CEProj_Saw_Params_t *pParms );
	virtual void SetSawParams( CEProj *pProj, const CEProj_Saw_Params_t *pParms );
	virtual void Launched( CEProj *pProj );
	virtual BOOL Detonated( CEProj *pProj, BOOL bMakeEffect, u32 nEvent, FCollImpact_t *pCollImpact );

	virtual BOOL Work( CEProj *pProj );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	BOOL _DealImpactDamageToEntity( CEProj *pProj, BOOL bDealDamageToNearbyEntities );

	BOOL _UpdateCarveModeMtxAndRelocate( CEProj *pProj );
	BOOL _ComputeCarveModeMtxAndRelocate( CEProj *pProj );
	void _UpdateCarveModeXfm( CEProj *pProj );
	BOOL _FindCarveModeSurfaceColl( CEProj *pProj, const CFVec3A *pRightVec, BOOL bCauseDamage, BOOL bSpawnSparks );
	static BOOL _CollRayWithTrackersCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume, const CFVec3 *pIntersectionPoint_WS, f32 fUnitDistToIntersection );
	static void _SetSmokeTrailAttributes( void );

	BOOL _Work_Flying( CEProj *pProj, f32 fDistTraveledThisFrame );

	BOOL _StartStickModeFromImpact( CEProj *pProj );
	void _StartStickMode( CEProj *pProj );
	void _Work_Sticking( CEProj *pProj );

	BOOL _StartCarveMode( CEProj *pProj );
	BOOL _Work_Carving( CEProj *pProj );

	void _DrawImpactParticles( const CFVec3A *pPos_WS, const CFVec3A *pSparksUnitDir_WS, const CFVec3A *pDustUnitDir_WS, f32 fSparksUnitIntensity, f32 fDustUnitIntensity );
	void _DrawFlashAtImpact( const CFVec3A *pPos_WS );

	void _KillCarvingSound( CEProj *pProj );
	void _WindDownCarvingSound( CEProj *pProj );
	void _StartCarvingSound( CEProj *pProj );
	void _UpdateCarvingSound( CEProj *pProj );

	BOOL _DamageNearbyEntities( CEProj *pProj, const CDamageProfile *pDamageProfile );
	static BOOL _DamageNearbyTrackerCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume );


	FCLASS_STACKMEM_ALIGN( CEProj_Saw );
} FCLASS_ALIGN_SUFFIX;




#endif

