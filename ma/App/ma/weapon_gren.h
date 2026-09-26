//////////////////////////////////////////////////////////////////////////////////////
// weapon_gren.h - Grenade weapons.
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
// 06/05/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _WEAPON_GREN_H_
#define _WEAPON_GREN_H_ 1

#include "fang.h"
#include "weapon.h"
#include "eproj.h"



class CBot;
class CDamageProfile;



FCLASS_ALIGN_PREFIX class CWeaponGren : public CWeapon {
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
		f32 fSecsPerRound;					// Seconds between throws

		f32 fGrenadeCullDist;				// Don't draw the weapon if it's this far from the camera

		f32 fGrenadesInPoolCount;			// The number of rockets in the projectile pool
		f32 fMaxLiveRange;					// The maximum distance the grenade can travel before self-destructing
		f32 fGrenadeSpeed;					// Grenade's linear speed
		f32 fGrenadeLife;					// Idle seconds before grenade detonates

		f32 fGrenadeMeshScale;				// Scale of the grenade mesh
		f32 fFatRadiusDelta;				// Amount to add to the grenade's radius when it's near an entity that will instantly detonate it

		CDamageProfile *pDamageProfile;				// If not NULL, this overrides the damage profile in the explosion profile
		FExplosion_GroupHandle_t hExplosionGroup;	// Explosion profile to use

		CFSoundGroup *pSoundGroupUse;		// Sound made when grenade is used
		CFSoundGroup *pSoundGroupBounce;	// Sound made when grenade bounces
	} _UserProps_t;


	typedef struct {
		CEProjPool::PoolHandle_t m_hProjPool;			// Projectile pool
	} _ResourceData_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static _UserProps_t m_aUserProps[EUK_COUNT_GRENADE];

	static const FGameData_TableEntry_t m_aUserPropVocab[];
	static const FGameDataMap_t m_aUserPropMapTable[];

	static CEProjPool::PoolHandle_t m_ahProjPool[EUK_COUNT_GRENADE];
	static SmokeTrailAttrib_t m_SmokeTrailAttrib;

	static CFTexInst m_StreamerTexInst;				// The streamer texture to use

	_ResourceData_t m_aResourceData[EUK_COUNT_GRENADE];
	_ResourceData_t *m_pResourceData;

	f32 m_fSecondsCountdownTimer;					// Used to measure time between rounds, reloads, etc.

	CEProj *m_pProjToThrow;							// The projectile we're ready to throw (NULL=none)




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );
	virtual void SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo=FALSE );

	CWeaponGren();
	virtual ~CWeaponGren();


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


	FCLASS_STACKMEM_ALIGN( CWeaponGren );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponGrenBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CWeaponGrenBuilder : public CWeaponBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CWeaponGrenBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CWeaponGrenBuilder );
} FCLASS_ALIGN_SUFFIX;






#endif

