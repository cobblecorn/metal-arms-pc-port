//////////////////////////////////////////////////////////////////////////////////////
// weapon_emp.h - EMP grenade weapons.
//
// Author: Nathan Miller   
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
// 11/18/02 Miller      Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _WEAPON_EMP_H_
#define _WEAPON_EMP_H_ 1

#include "fang.h"
#include "weapon.h"
#include "eproj.h"
#include "explosion.h"
#include "fparticle.h"

class CBot;

FCLASS_ALIGN_PREFIX class CWeaponEMP : public CWeapon {

//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		_MESH_GRENADE,

		_MESH_COUNT
	} _Mesh_e;


//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	typedef struct {						// NOTE: If you change this structure, you must also change _Anim_e and m_aUserPropVocab
		cchar *apszMeshName[_MESH_COUNT];	// Mesh names

		f32 fClipAmmoMax;					// Maximum number of clip rounds (<0 means infinite)
		f32 fRoundsPerSec;					// Maximum fire rate

		f32 fGrenadeCullDist;				// Don't draw the weapon if it's this far from the camera

		f32 fGrenadesInPoolCount;			// The number of EMPs in the projectile pool
		f32 fMaxLiveRange;					// The maximum distance the EMP can travel before self-destructing
		f32 fGrenadeSpeed;					// EMP's linear speed
		f32 fGrenadeLife;					// Idle seconds before grenade detonates

		f32 fGrenadeMeshScale;				// Scale of the grenade mesh
		f32 fFatRadiusDelta;				// Amount to add to the grenade's radius when it's near an entity that will instantly detonate it

		f32 fDetonationRadius;
		f32 fDetonationLife;
		f32 fShutdownTime;

		FExplosion_GroupHandle_t hExplosionGroup;	// Explosion profile to use

		CFSoundGroup *pSoundGroupUse;		// Sound made when grenade is used
		CFSoundGroup *pSoundGroupBounce;	// Sound made when grenade bounces
		CFSoundGroup *pSoundGroupEffect;	// Sound made when EMP is activated (looping)
	} _UserProps_t;


	typedef struct {
		CEProjPool::PoolHandle_t m_hProjPool;			// Projectile pool
	} _ResourceData_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static _UserProps_t m_aUserProps[EUK_COUNT_EMP];

	static const FGameData_TableEntry_t m_aUserPropVocab[];
	static const FGameDataMap_t m_aUserPropMapTable[];

	static CEProjPool::PoolHandle_t m_ahProjPool[EUK_COUNT_EMP];
	static SmokeTrailAttrib_t m_SmokeTrailAttrib;

	static CFTexInst m_StreamerTexInst;				// The streamer texture to use

	_ResourceData_t m_aResourceData[EUK_COUNT_EMP];
	_ResourceData_t *m_pResourceData;

	f32 m_fSecondsCountdownTimer;					// Used to measure time between rounds, reloads, etc.

	CEProj *m_pProjToThrow;							// The projectile we're ready to throw (NULL=none)

#if FANG_WINGC
	// PC port: the cut Water Grenade is this EMP grenade with the unused gp_swater model: its burst
	// of coolant still shorts out Mils (the EMP effect), with a water blast on top.
	BOOL m_bPortWater;
	static CEProjPool::PoolHandle_t m_hPortWaterPool;
	static FParticle_DefHandle_t m_hPortWaterSplash;
	static BOOL _PortWaterDetonated( CEProj *pProj, BOOL bMakeEffect, CEProj::Event_e nEvent, const FCollImpact_t *pImpact );
#endif




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );
	virtual void SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo=FALSE );

	CWeaponEMP();
	virtual ~CWeaponEMP();


	// Creation:
	BOOL Create( cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

#if FANG_WINGC
	FINLINE void SetPortWater( BOOL bWater ) { m_bPortWater = bWater; }
	FINLINE BOOL IsPortWater( void ) const { return m_bPortWater; }
#endif


	// Firing:
	virtual BOOL Throwable_TriggerWork( f32 fUnitTriggerVal );
	virtual BOOL Throwable_Prime( void );
	virtual void Throwable_AttachGrenadeToOwnerBotBone( cchar *pszBoneName );
	virtual void Throwable_ThrowGrenade_TargetPoint( const CFVec3A *pTargetPos_WS, f32 fSpeed=50.0f );
	virtual void Throwable_ThrowGrenade_MountAimDirection( f32 fSlingSpeed, BOOL bLimbDangling );
	virtual void Throwable_ReturnGrenade( void );



//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL ClassHierarchyBuild( void );
	virtual BOOL ClassHierarchyBuilt( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );
	virtual void ClassHierarchyDestroy( void );
	virtual void ClassHierarchyRemoveFromWorld( void );
	virtual void ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled );

	virtual void ClassHierarchyWork( void );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _ClearDataMembers( void );
	BOOL _GetProjectileFromPoolAndInit( void );
	static void _BuildSkipList( CEProj *pProj );


	FCLASS_STACKMEM_ALIGN( CWeaponEMP );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponEMPBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CWeaponEMPBuilder : public CWeaponBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CWeaponEMPBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CWeaponEMPBuilder );
} FCLASS_ALIGN_SUFFIX;






#endif
