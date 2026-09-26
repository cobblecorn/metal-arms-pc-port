//////////////////////////////////////////////////////////////////////////////////////
// weapon_magmabomb.h - magma grenade weapons.
//
// Author: Michael Scholz
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
// 12/30/02 Scholz      Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _WEAPON_MAGMABOMB_H_
#define _WEAPON_MAGMABOMB_H_ 1

#include "fang.h"
#include "weapon.h"
#include "eproj.h"
#include "explosion.h"
#include "FXMagmaBomb.h"

class CBot;
class CFSoundGroup;

FCLASS_ALIGN_PREFIX class CWeaponMagmaBomb : public CWeapon
{
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	// NKM - I had to put this here because of a Metroworks weirdness
	typedef enum
	{
		_MESH_BOMB,
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


	typedef struct
	{										// NOTE: If you change this structure, you must also change m_aUserPropVocab
		cchar *apszMeshName[_MESH_COUNT];	// Mesh names

		f32 fClipAmmoMax;					// Maximum number of clip rounds (<0 means infinite)
		f32 fRoundsPerSec;					// Maximum fire rate

		f32 fGrenadeCullDist;				// Don't draw the weapon if it's this far from the camera

		f32 fGrenadesInPoolCount;			// The number of s in the projectile pool
		f32 fMaxLiveRange;					// The maximum distance the can travel before self-destructing
		f32 fGrenadeSpeed;					// 's linear speed

		f32 fGrenadeMeshScale;				// Scale of the grenade mesh
		f32 fFatRadiusDelta;				// Amount to add to the grenade's radius when it's near an entity that will instantly detonate it

		FExplosion_GroupHandle_t hExplosionGroup;	// Explosion profile to use

		CFSoundGroup *pSoundGroupUse;		// Sound made when grenade is used
		CFSoundGroup *pSoundGroupBounce;	// Sound made when grenade bounces

		MagmaBombProps_t MagmaProps;
	} _UserProps_t;


	typedef struct
	{
		CEProjPool::PoolHandle_t m_hProjPool;			// Projectile pool
	} _ResourceData_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL			m_bSystemInitialized;
	static u32			m_uWpnClassClientCount;					// Number of this class using the class-wide resources

	static _UserProps_t m_aUserProps[EUK_COUNT_MAGMABOMB];

	static const FGameData_TableEntry_t m_aUserPropVocab[];
	static const FGameDataMap_t m_aUserPropMapTable[];

	static CEProjPool::PoolHandle_t m_ahProjPool[EUK_COUNT_MAGMABOMB];
	static SmokeTrailAttrib_t m_SmokeTrailAttrib;

	static CFTexInst m_StreamerTexInst;				// The streamer texture to use

	_ResourceData_t m_aResourceData[EUK_COUNT_MAGMABOMB];
	_ResourceData_t *m_pResourceData;

	f32 m_fSecondsCountdownTimer;					// Used to measure time between rounds, reloads, etc.
	u64 m_nTriggerDownTicks;						// Tick count when trigger last made the transition from 0.0f to >0.0f
	BOOL m_bTriggerDownLastFrame;					// TRUE if the trigger was down last frame

	f32 m_fUnitThrowSpeed;							// The unit throw speed
	CEProj *m_pProjToThrow;							// The projectile we're ready to throw (NULL=none)
	
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );
	virtual void SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo=FALSE );
	virtual void CheckpointRestore( void );

	CWeaponMagmaBomb();
	virtual ~CWeaponMagmaBomb();


	// Creation:
	BOOL Create( cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );


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
	virtual BOOL ClassHierarchyLoadSharedResources( void );
	virtual void ClassHierarchyUnloadSharedResources( void );

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

	static BOOL _DetonateCallback( CEProj *pProj, BOOL bMakeEffect, CEProj::Event_e nEvent, const FCollImpact_t *pImpact );

	FCLASS_STACKMEM_ALIGN( CWeaponMagmaBomb );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponMagmaBombBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CWeaponMagmaBombBuilder : public CWeaponBuilder
{
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CWeaponMagmaBombBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CWeaponMagmaBombBuilder );
} FCLASS_ALIGN_SUFFIX;



#endif
