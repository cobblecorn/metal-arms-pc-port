//////////////////////////////////////////////////////////////////////////////////////
// weapon_staff.cpp - Elite guard's weapon implementation.
//
// Author: Michael Scholz
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
// 02.07.03 Scholz       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _WEAPON_STAFF_H_
#define _WEAPON_STAFF_H_ 1

#include "fang.h"
#include "weapon.h"
#include "fworld.h"
#include "fxfm.h"
#include "fviewport.h"
#include "fmath.h"
#include "fmesh.h"
#include "eproj.h"
#include "fforce.h"
#include "fparticle.h"
#include "fxslower.h"

class CBot;
class CDamageProfile;
class CEParticle;


FCLASS_ALIGN_PREFIX class CWeaponStaff : public CWeapon
{
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	typedef enum 
	{
		STAFFSND_CHARGE_UP,
		STAFFSND_CHARGE_DOWN,
		STAFFSND_CHARGE_LOOP,
		STAFFSND_FIRE,
		STAFFSND_COUNT,
	}StaffSound_e;


//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF

	typedef struct
	{						
		cchar *pszMeshName;					// My mesh
		cchar *pszProjectileMeshName;		// The projectile mesh
		f32 fProjInPoolCount;				// The number of projectiles in the projectile pool
		f32 fMaxLiveRange;					// The maximum distance the projectile can travel before self-destructing
		f32 fProjCullDist;					// Don't draw the projectile if it's this far from the camera
		f32 fProjSpeed;						// Projectile's linear speed

		f32 fMinTargetAssistDist;			// The minimum targeting assistance distance (closer than this gets clamped)
		f32 fMaxTargetAssistDist;			// The maximum targeting assistance distance (farther will not get assisted) (0=no targeting assistance)

		f32 fChargeTime;			// The reload time for one round
		f32 fOOChargeTime;			//

//		cchar *pszDamageProfile;			// Damage profile string (references a damage profile in damage.csv)
//		cchar *pszBlastDamageProfile;		// Damage profile string (references a damage profile in damage.csv) used for the L3 blast

		f32 fChargeBeginScale;
		f32 fChargeEndScale;
		f32 fShotFiredScale;
		f32 fShotMaxScale;

		f32 fCosMaxProjAngle;
		
		f32 fMaxDevLengthSquared;

	} _UserProps_t;

	typedef struct 
	{
		CFWorldMesh *m_pWorldMesh;			// This EUK's world mesh
	} _ResourceData_t;

//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL			m_bSystemInitialized;
	static u32			m_uWpnClassClientCount;					// Number of this class using the class-wide resources

	static _UserProps_t m_aUserProps[EUK_COUNT_STAFF];
	static CFXSlower::_UserProps_t m_aFXProps[EUK_COUNT_STAFF];

	static const FGameData_TableEntry_t m_aUserPropVocab[];
	static const FGameData_TableEntry_t m_aFXPropsVocab[];

	static const FGameDataMap_t m_aUserPropMapTable[];
	
	static CEProjPool::PoolHandle_t m_hSlowerProjPool;
	static FParticle_DefHandle_t m_hParticleDef;
	static FLightInit_t			 m_LightInit;

	static CFSoundGroup* m_pSounds[STAFFSND_COUNT];
	CFAudioEmitter* m_pSoundEmitters[STAFFSND_COUNT];

	_ResourceData_t m_aResourceData[EUK_COUNT_STAFF];
	CEParticle* m_pParticleEmitter;
	FForceHandle_t m_hForce;				// Force feedback handle so we can kill it when we need to
	
	CFVec3A m_TargetPos_WS;					// target point
	CFVec3A* m_pvStaffUnitDir;
	CFWorldAttachedLight* m_pWorldLight;
	BOOL m_bFireTriggerDown; 
	BOOL m_bFireThisFrame;
	f32 m_fUnitCharge;
	f32 m_fParticleIntensityMult;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	CWeaponStaff();
	virtual ~CWeaponStaff();

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );

	BOOL Create( cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

	// Collision:
	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);

	// Firing:
	virtual void ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const;
	virtual u32 TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pvTargetPoint_WS, const CFVec3A *pBuddyFirePos_WS = NULL );

	void KillRumble( void );

	// checkpoint save/restore
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );

	FINLINE f32  GetChargeRadius(void) { return ((m_fUnitCharge == 0.0f) ? 0.0f : m_aResourceData[m_nUpgradeLevel].m_pWorldMesh->GetBoundingSphere().m_fRadius);}
	FINLINE f32* GetUnitChargePtr(void){return &m_fUnitCharge;}
	FINLINE void ResetUnitCharge(void)
	{
		m_fUnitCharge=0.0f;
		m_aResourceData[m_nUpgradeLevel].m_pWorldMesh->RemoveFromWorld();
		m_bFireTriggerDown = FALSE;
	}

	FINLINE virtual CFWorldMesh *GetMesh( void ) const { FASSERT( IsCreated() ); return m_aResourceData[m_nUpgradeLevel].m_pWorldMesh; }

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:
	virtual BOOL ClassHierarchyLoadSharedResources( void );
	virtual void ClassHierarchyUnloadSharedResources( void );

	virtual void ClassHierarchyDestroy( void );

	virtual BOOL ClassHierarchyBuild( void );
	virtual BOOL ClassHierarchyBuilt( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );

	virtual void ClassHierarchyWork( void );
	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );
	virtual void ClassHierarchyRelocated( void *pIdentifier );

	virtual void ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled );

	virtual void ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel );

	virtual void SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo=FALSE );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _SetDefaults( void );
	static BOOL _DoesRayIntersectMesh( const CFVec3A *pRayStart, const CFVec3A *pRayEnd, CFWorldMesh *pWorldMesh );
	static void _BuildProjSkipList( CEProj *pProj );
	static BOOL _ProjDetonateCallback( CEProj *pProj, BOOL bMakeEffect, CEProj::Event_e nEvent, const FCollImpact_t *pImpact );
	void _PlaySnd(StaffSound_e eSnd, BOOL bPlay);
//----------------------------------------------------------------------------------------------------------------------------------
// Friendship
//----------------------------------------------------------------------------------------------------------------------------------
	friend class CBotEliteGuard;

	FCLASS_STACKMEM_ALIGN( CWeaponStaff );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponStaffBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

//
//
//
FCLASS_ALIGN_PREFIX class CWeaponStaffBuilder : public CWeaponBuilder
{
	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
	public:

		FINLINE CWeaponStaffBuilder() {}
		virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




	//----------------------------------------------------------------------------------------------------------------------------------
	// Protected Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
	protected:

		virtual BOOL InterpretTable( void );
		virtual BOOL PostInterpretFixup( void );


		FCLASS_STACKMEM_ALIGN( CWeaponStaffBuilder );
} FCLASS_ALIGN_SUFFIX;


#endif

