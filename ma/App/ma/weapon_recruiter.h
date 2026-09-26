//////////////////////////////////////////////////////////////////////////////////////
// weapon_recruiter.cpp - Recruiter grenade weapon.
//
// Author: Steve Ranck     
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
// 04/28/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _WEAPON_RECRUITER_H_
#define _WEAPON_RECRUITER_H_ 1

#include "fang.h"
#include "weapon.h"
#include "eproj.h"
#include "explosion.h"
#include "fbolt.h"


class CFMeshPool;
class CRecruitData;




FCLASS_ALIGN_PREFIX class CWeaponRecruiter : public CWeapon {

//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		_MESH_GRENADE,						// The actual grenade mesh

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

		u32 nMaxRecruites;					// Maximum recruited bots at any one time
		FMesh_t *pMeshIcon;					// 3D mesh icon place above recruited bots' heads
		FTexDef_t *pTexDefBolt;				// Bolt texture

		f32 fGrenadeCullDist;				// Don't draw the weapon if it's this far from the camera

		f32 fGrenadesInPoolCount;			// The number of Recruiter grenades in the projectile pool
		f32 fMaxLiveRange;					// The maximum distance the Recruiter grenade can travel before self-destructing
		f32 fGrenadeSpeed;					// Recruiter grenade's linear speed
		f32 fGrenadeLife;					// Idle seconds before grenade detonates

		f32 fGrenadeMeshScale;				// Scale of the grenade mesh
		f32 fFatRadiusDelta;				// Amount to add to the grenade's radius when it's near an entity that will instantly detonate it

		f32 fDetonationRadius;
		f32 fDetonationLife;
		f32 fShutdownTime;

		FExplosion_GroupHandle_t hExplosionGroup;	// Explosion profile to use
		FParticle_DefHandle_t hParticleIconOff;		// Icon off burst effect

		CFSoundGroup *pSoundGroupUse;		// Sound made when grenade is used
		CFSoundGroup *pSoundGroupBounce;	// Sound made when grenade bounces
		CFSoundGroup *pSoundGroupEffect;	// Sound made when Recruiter grenade is activated (looping)
		CFSoundGroup *pSoundGroupBolt;		// Sound made when Recruiter bolt is fired
	} _UserProps_t;


	typedef struct {
		CEProjPool::PoolHandle_t m_hProjPool;			// Projectile pool
	} _ResourceData_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static u32 m_nWeaponClassClientCount;			// Number of recruiter weapons using the class-wide resources

	static CFBolt::CFBoltData m_BoltData;
	static _UserProps_t m_aUserProps[EUK_COUNT_RECRUITER];

	static const FGameData_TableEntry_t m_aUserPropVocab[];
	static const FGameDataMap_t m_aUserPropMapTable[];

	static CEProjPool::PoolHandle_t m_ahProjPool[EUK_COUNT_RECRUITER];
	static SmokeTrailAttrib_t m_SmokeTrailAttrib;

	static CFTexInst m_StreamerTexInst;				// The streamer texture to use
	static CFMeshPool *m_pIconMeshPool;				// Pool of meshes used to place 3D icons above recruited bots' heads

	_ResourceData_t m_aResourceData[EUK_COUNT_RECRUITER];
	_ResourceData_t *m_pResourceData;

	f32 m_fSecondsCountdownTimer;					// Used to measure time between throws
	CEProj *m_pProjToThrow;							// The projectile we're ready to throw (NULL=none)




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );
	virtual void SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo=FALSE );
	static void DrawAll( void );
	static void WorkAll( void );

	CWeaponRecruiter();
	virtual ~CWeaponRecruiter();


	// Creation:
	BOOL Create( cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );


	// Firing:
	virtual BOOL Throwable_TriggerWork( f32 fUnitTriggerVal );
	virtual BOOL Throwable_Prime( void );
	virtual void Throwable_AttachGrenadeToOwnerBotBone( cchar *pszBoneName );
	virtual void Throwable_ThrowGrenade_TargetPoint( const CFVec3A *pTargetPos_WS, f32 fSpeed=50.0f );
	virtual void Throwable_ThrowGrenade_MountAimDirection( f32 fSlingSpeed, BOOL bLimbDangling );
	virtual void Throwable_ReturnGrenade( void );


	// Recruitment:
	static void UnrecruitAllBots( BOOL bMakeEffects=FALSE );
	static BOOL RecruitBot( CBot *pBot, CBot *pRecruitingBot, const CFVec3A *pEpicenter_WS=NULL );
	static void UnrecruitBot( CBot *pBot, BOOL bMakeEffect );
	static void UpdateRecruiterIconPos( CBot *pBot );
	static void SetUserViewMode( CBot* pBot, BOOL bUserView );
	static void RecruitWork( CBot *pBot );
	static void BotDrawEnable( CBot *pBot, BOOL bDrawingHasBeenEnabled );
	static void BotCheckpointSaved( CBot *pBot );
	static void BotCheckpointRestored( CBot *pBot );
	static BOOL WasRecruitedBy( CBot *pBot, CBot *pRecruitingBot );




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

	virtual BOOL ClassHierarchyLoadSharedResources( void );
	virtual void ClassHierarchyUnloadSharedResources( void );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _ClearDataMembers( void );
	BOOL _GetProjectileFromPoolAndInit( void );
	static void _BuildSkipList( CEProj *pProj );
	static void _ComputeBotCylinderCenter( CFVec3A *pPos_WS, CBot *pBot );
	static void _SetAIToRecruitedState( CRecruitData *pRecruitData );


	FCLASS_STACKMEM_ALIGN( CWeaponRecruiter );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponRecruiterBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CWeaponRecruiterBuilder : public CWeaponBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CWeaponRecruiterBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CWeaponRecruiterBuilder );
} FCLASS_ALIGN_SUFFIX;






#endif
