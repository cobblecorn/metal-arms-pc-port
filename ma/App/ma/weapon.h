//////////////////////////////////////////////////////////////////////////////////////
// weapon.h - Main weapon module.
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
// 03/28/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _WEAPON_H_
#define _WEAPON_H_ 1

#include "fang.h"
#include "fxfm.h"
#include "fworld.h"
#include "entity.h"
#include "reticle.h"
#include "muzzleflash.h"
#include "fgamedata.h"
#include "eproj.h"
#include "fdebris.h"



class CBot;
class CItemInst;
class CEProj;
class CWeaponScope;
class CFSoundGroup;



FCLASS_ALIGN_PREFIX class CWeapon : public CEntity {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		WEAPON_TYPE_BLINK_RIGHT_HAND,			// Blink's right hand
		WEAPON_TYPE_LASER,						// Laser
		WEAPON_TYPE_RIVET_GUN,					// Rivet gun
		WEAPON_TYPE_GRENADE,					// Grenade
		WEAPON_TYPE_BLASTER,					// Blaster
		WEAPON_TYPE_ROCKET_LAUNCHER,			// Rocket launcher
		WEAPON_TYPE_TETHER,						// Remote control tether
		WEAPON_TYPE_SPEW,						// Small Projectile Emitter Weapon
		WEAPON_TYPE_MORTAR,						// Mortar
		WEAPON_TYPE_FLAMER,						// Molten rain thrower
		WEAPON_TYPE_RIPPER,						// Ripper
		WEAPON_TYPE_CLEANER,					// Cleaner grenade
		WEAPON_TYPE_SCOPE,						// Sniper scope
		WEAPON_TYPE_CHAINGUN,					// Titan's chaingun
		WEAPON_TYPE_EMP,						// EMP grenade
		WEAPON_TYPE_QUADLASER,					// the predator's quad laser
		WEAPON_TYPE_MAGMABOMB,					// slosh's grenade
		WEAPON_TYPE_STAFF,						// The guard staff
		WEAPON_TYPE_WRENCH,						// The wrench
		WEAPON_TYPE_RECRUITER,					// Recruiter grenade

		WEAPON_TYPE_COUNT
	} WeaponType_e;


	typedef enum {
		GRIP_RIGHT_ARM,							// Snaps onto the bot's right arm
		GRIP_LEFT_ARM,							// Thrown or used by the bot's left arm
		GRIP_BOTH_ARMS,							// Requires the bot to use both arms
		GRIP_EMBEDDED,							// The weapon is embedded into the bot (not held or thrown by arms)
		GRIP_DROPPED_FOR_BLINK,					// A weapon that has been dropped by an enemy bot and that Blink can pick up (always needs 2 arms)
		GRIP_MOUNTED_FOR_BLINK,					// A mounted weapon like a sentry gun or the gun aboard the RAT that Blink can use (usually 1st person POV)

		GRIP_COUNT
	} Grip_e;


	typedef enum {
		RELOAD_TYPE_FULLYAUTOMATIC,				// The weapon will automatically reload (doesn't require any special handling from its owner)
		RELOAD_TYPE_SHELL,						// The weapon requires a shell-based reload (weapon up, it reloads over time, cock, then weapon down)
		RELOAD_TYPE_CLIP,						// The weapon requires a clip-based reload (pop out old clip, insert new, slap it in)
		RELOAD_TYPE_ROCKET1,					// The weapon requires a rocket L1 reload (grab a rocket, insert it, grab another, etc.)
		RELOAD_TYPE_TETHER,						// The weapon requires a tether reload (pump, insert new round)
		RELOAD_TYPE_MORTAR,						// The weapon requires a mortar reload (grab a round and place it in slingshot)

		RELOAD_TYPE_COUNT
	} ReloadType_e;


	typedef enum {
		STANCE_TYPE_STANDARD,					// The owner will use the standard weapon stance
		STANCE_TYPE_TWO_HANDED_TORSO_TWIST,		// The owner will use the two-handed torso-twist weapon stance
		STANCE_TYPE_SHOULDER_MOUNT,				// The owner will use the shoulder-mounted weapon stance
		STANCE_TYPE_MORTAR,						// The owner will use the mortar stance

		STANCE_TYPE_COUNT
	} StanceType_e;


	enum {
		INFOFLAG_THROWABLE					= 0x00000001,	// This is a throwable weapon (use the Throwable interface instead of the Trigger interface)
		INFOFLAG_NO_AMMO					= 0x00000002,	// This weapon has no notion of ammo
		INFOFLAG_LEFT_HAND_RELOADS			= 0x00000004,	// This weapon requires Blink to load it with his left hand (he can't reload while on a zipline)
		INFOFLAG_NO_AUTO_TARGETING			= 0x00000008,	// TRUE: HumanTargeting_FindDesiredTargetPoint_WS() will just return the point far in front of the muzzle
		INFOFLAG_SCOPE_ENABLED				= 0x00000010,	// TRUE: This weapon can be used with a scope
		INFOFLAG_THICK_TARGETING			= 0x00000020,	// TRUE: Use FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES in HumanTargeting_FindDesiredTargetPoint_WS()
		INFOFLAG_DISPLAY_AMMO_TEXT			= 0x00000040,	// TRUE: Display low/no/reload ammo text

		INFOFLAG_NONE						= 0x00000000
	};


	enum {
		// NKM - If you happen to change the lower EUKs for Glitch, make sure the ClassifyPlayerWeapon() in CCollectable.
		INFINITE_AMMO = 0xffff,				// Don't change this from 0xffff or it'll break a lot of things!

		EUK_COUNT_BLINK_RIGHT_HAND	= 3,	// (Glitch=0) (Miner=1) (Grunt=2)
		EUK_COUNT_LASER				= 5,	// (Glitch=0,1,2) (Mil Grunt=3) (Possessed Mil Grunt=4)
		EUK_COUNT_RIVET				= 6,	// (Glitch=0,1,2) (Mozer=3) (Mil Grunt=4) (Possessed Mil Grunt=5)
		EUK_COUNT_GRENADE			= 3,	// (Glitch=0) (Mil Grunt=1) (Possessed Mil Grunt=2)
		EUK_COUNT_BLASTER			= 5,	// (Glitch=0,1,2) (Mil Grunt=3) (Possessed Mil Grunt=4)
		EUK_COUNT_ROCKET_LAUNCHER	= 6,	// (Glitch=0,1,2) (Mil Grunt=3) (Possessed Mil Grunt=4) (Buddy=5)
		EUK_COUNT_TETHER			= 4,	// (Glitch=0,1,2) (Krunk=3)					
		EUK_COUNT_SPEW				= 5,	// (Glitch=0,1,2) (Mil Grunt=3) (Possessed Mil Grunt=4)
		EUK_COUNT_MORTAR			= 1,
		EUK_COUNT_FLAMER			= 4,	// (Glitch=0,1,2) (Slosh=3)
		EUK_COUNT_RIPPER			= 3,
		EUK_COUNT_CLEANER			= 1,
		EUK_COUNT_SCOPE				= 2,
		EUK_COUNT_CHAINGUN			= 3,	// (Glitch:0) (Titan:1) (Possessed Titan:2)
		EUK_COUNT_EMP				= 1,
		EUK_COUNT_QUADLASER			= 2,	// (Predator:0) (Possessed Predator:1)
		EUK_COUNT_MAGMABOMB			= 1,
		EUK_COUNT_WATERBOMB			= 1,
		EUK_COUNT_STAFF				= 1,
		EUK_COUNT_WRENCH			= 1,
		EUK_COUNT_MAX				= 6,	// The maximum number of EUKs for any weapon
		EUK_COUNT_RECRUITER			= 1,	// Only a single EUK level for the Recruiter grenade
	};


	typedef struct {
		Grip_e nGrip;							// The type of grip
		ReloadType_e nReloadType;				// The reload type required by this weapon
		StanceType_e nStanceType;				// The stance type required by this weapon
		f32 fMinTargetAssistDist;				// Minimum range that this weapon will offer targeting assistance (closer than this and assistance amount will be clamped)
		f32 fMaxTargetAssistDist;				// Maximum range that this weapon will offer targeting assistance (0=no assistance)
		f32 fMaxLiveRange;						// Maximum range that this weapon's projectiles can travel before auto-destructing (in feet)
		u16 nClipAmmoMax;						// Maximum number of rounds the clip can hold (ignore for throwable)
		u16 nReserveAmmoMax;					// Maximum number of rounds the player can carry in his reserve (ignore for throwable)
		u32 nInfoFlags;							// See INFOFLAG_* for info
		CReticle::Type_e nReticleType;			// Reticle type (CReticle::TYPE_NONE if no reticle for this weapon)
		f32	fUnitRecoil;						// amount of recoil this weapon generates
	} Info_t;


	typedef enum {
		// Final states:
		STATE_PRESENT,										// Presentation
		STATE_STOWED,										// Fully stowed
		STATE_DEPLOYED,										// Fully deployed
		STATE_RELOAD_OPENED,								// Fully opened for reload

		// Transition states:
		STATE_FIRST_TRANSITION,
		STATE_STOWING = STATE_FIRST_TRANSITION | 0x80000000,// Transitioning from deployed to stowed
		STATE_DEPLOYING,									// Transitioning from stowed to deployed
		STATE_OPENING_RELOAD,								// Transitioning from deployed to reload-opened
		STATE_CLOSING_RELOAD,								// Transitioning from reload-opened to deployed
		STATE_RELOADING,									// Reloading

		// Don't use these:
		STATE_LAST,
		STATE_COUNT = STATE_LAST & 0x7fffffff
	} State_e;


	enum {
		WEAPONFLAG_PREVENT_RELOAD					= 0x00000001,	// When set, the bot should refrain from beginning a reload animation
		WEAPONFLAG_PREVENT_SWITCH					= 0x00000002,	// When set, the bot should refrain from beginning a weapon switch animation
		WEAPONFLAG_DISABLE_RETICLE					= 0x00000004,	// Don't draw the reticle
		WEAPONFLAG_CLIP_INSTALLED					= 0x00000008,	// For clip-based weapons, this indicates that the clip is installed (whether or not it has ammo)
		WEAPONFLAG_MADE_EMPTY_CLICK_SOUND			= 0x00000010,	// TRUE when we've made the empty click sound already (reset when the trigger is released)
		WEAPONFLAG_WAIT_FOR_TRIGGER_UP				= 0x00000020,	// Optionally used by derived classes to help determine when the player has released the trigger
		WEAPONFLAG_WEAPON_DISABLED					= 0x00000040,	// The weapon has been disabled (stop animations, disable firing, etc)
		WEAPONFLAG_NEEDS_TARGETED_ENTITY_FOR_USE	= 0x00000080,	// The weapon needs to have the targeting logic run to even USE the weapon

		WEAPONFLAG_NONE						= 0x00000000
	};


	typedef enum {
		MUZZLEFLASH_TYPE_BLASTER,
		MUZZLEFLASH_TYPE_LASER_YELLOW,
		MUZZLEFLASH_TYPE_LASER_RED,
		MUZZLEFLASH_TYPE_FLAME_POSTER_Z_1,
		MUZZLEFLASH_TYPE_FLAME_POSTER_Z_2,
		MUZZLEFLASH_TYPE_BALL_POSTER_XY_1,
		MUZZLEFLASH_TYPE_3D_STAR_1,
		MUZZLEFLASH_TYPE_3D_STAR_2,
		MUZZLEFLASH_TYPE_3D_ENERGY_WHITE,
		MUZZLEFLASH_TYPE_3D_ENERGY_ORANGE,

		MUZZLEFLASH_TYPE_COUNT
	};


	typedef enum {
		AMMO_ALERT_OK,							// Ammo level is ok
		AMMO_ALERT_LOW_AMMO,					// Ammo level is low
		AMMO_ALERT_NO_AMMO,						// No ammo

		AMMO_ALERT_COUNT
	} AmmoAlert_e;


	typedef enum {
		PROJ_POOL_TYPE_SWARMER_ROCKET,

		PROJ_POOL_TYPE_COUNT
	} ProjPoolType_e;


	typedef struct {
		cchar *pszRocketMeshName;				// Mesh name of the rocket projectile
		u32 nRocketPoolCount;					// Number of rockets in the rocket pool

		CFDebrisGroup *pDebrisGroupClipEject;	// Debris group used to eject clips
	} _UserProps_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF


	typedef struct {
		MuzzleFlashType_e nType;		// Muzzle flash type
		cchar *pszResName;				// Resource name
		u32 nMaxCount;					// Maximum number of simultaneous flashes in this group
		FDrawBlendOp_e nBlendOp;		// The blend operation for this group
	} MuzzleFlashDef_t;



	typedef struct {
		u32				uEUK;
		CFWorldMesh*	pWorldMesh;
		CFAnimCombiner*	pAnimCombiner;
		CFAnimMeshRest*	pAnimMeshRest;
	} SharedResourceData_t;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static _UserProps_t m_UserProps;												// General weapon user props
	static const FGameData_TableEntry_t m_aUserPropVocab[];
	static const FGameDataMap_t m_aUserPropMapTable[];

	static const u32 m_anMaxUpgradeCount[WEAPON_TYPE_COUNT];						// The maximum EUK upgrade level for each weapon type
	static Info_t m_aaInfo[WEAPON_TYPE_COUNT][EUK_COUNT_MAX];						// Global data for each weapon & EUK level
	static MuzzleFlash_GroupHandle_t m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_COUNT];	// Array of muzzle flash groups
	static CEProjPool::PoolHandle_t m_ahProjPool[PROJ_POOL_TYPE_COUNT];				// Pools of projectiles

	const Info_t *m_pInfo;															// Points to the Info_t structure for this weapon (never NULL)

	f32 m_fSecsUntilNextSound;						// How frequently this weapon can make an AI sound	
	u32 m_uAISoundHandle;							// Handle to the AI Sound that this weapon makes




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Data:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	WeaponType_e m_nWeaponType;					// The type of weapon
	u32 m_nWeaponFlags;							// See WEAPONFLAG_* for info
	u32 m_nUpgradeLevel;						// EUK upgrade level
	s32 m_nBotDamageBoneIndex;					// Which bot bone index damage should be passed to if this weapon takes damage (-1=don't pass on damage to bot)
	CBot *m_pOwnerBot;							// Points to the bot who owns this weapon (NULL=no owner)
	CEntity *m_pTargetedEntity;					// Currently targeted entity (NULL=none)
	f32 m_fTargetedEntityHoldTimer;				// Time remaining to hold onto the targeted entity if it's obstructed by an obstacle
	CWeaponScope *m_pAttachedScope;				// Points to the scope that's attached to this weapon (NULL=none)

	SharedResourceData_t m_aSharedResourceData[EUK_COUNT_MAX];
	s32 m_nSingleMeshForEUKs;					// default = -1.  Set to an EUK if all EUKs will share this mesh

	// Note: For throwable weapons m_nClipAmmo
	//       indicates the count in the bot's
	//       inventory, and m_nReserveAmmo is
	//       always 0.
	u16 m_nClipAmmo;							// Number of rounds in the clip (if infinite clip ammo, this is INFINITE_AMMO)
	u16 m_nReserveAmmo;							// Number of rounds outside the clip (if infinite reserve ammo, this is INFINITE_AMMO)

	static CWeapon *m_pCallback_Weapon;			// This weapon
	static CReticle *m_pCallback_Reticle;		// The reticle
	static CEntity *m_pCallback_ClosestEntity;	// The entity that's closest to our ray start point (NULL=none)
	static const FViewport_t *m_pCallback_Viewport;	// Viewport
	static CFSphere m_Callback_TrackerSphere;	// The tracker's sphere
	static CFVec3A *m_pCallback_RayStartPoint;	// Our ray start point
	static CFVec3A *m_pCallback_RayEndPoint;	// Our ray end point
	static f32 m_fCallback_ShortestDistance2;	// Shortest distance squared so far




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static const MuzzleFlashDef_t m_aMuzzleFlashDef[MUZZLEFLASH_TYPE_COUNT];		// Array of muzzle flash definitions

	State_e m_nCurrentState;					// Current weapon state
	State_e m_nDesiredState;					// Desired state we're heading toward

	CItemInst *m_pItemInst;						// Pointer to CItemInst (NULL=none)




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );


	// Destructor:
	virtual ~CWeapon();


	// Item Inst:
	virtual void SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo=FALSE );
	FINLINE CItemInst *GetItemInst( void ) const { FASSERT( IsCreated() ); return m_pItemInst; }


	// Type and upgrade:
	FINLINE WeaponType_e Type( void ) const { FASSERT( IsCreated() ); return m_nWeaponType; }

	u32 GetUpgradeLevelCount( void ) const;
	FINLINE u32 GetMaxUpgradeLevel( void ) const { FASSERT( IsCreated() ); u32 nCount = GetUpgradeLevelCount(); FASSERT( nCount > 0 ); return nCount ? nCount - 1 : 0; }
	FINLINE BOOL IsUpgradeAtMax( void ) const { FASSERT( IsCreated() ); return GetUpgradeLevel() == GetMaxUpgradeLevel(); }

	FINLINE u32 GetUpgradeLevel( void ) const { FASSERT( IsCreated() ); return m_nUpgradeLevel; }
	void SetUpgradeLevel( u32 nUpgradeLevel );
	BOOL UpgradeWeapon( void );

	FINLINE CWeaponScope *GetAttachedScope( void ) const { FASSERT( IsCreated() ); return m_pAttachedScope; }
	FINLINE void SetAttachedScope( CWeaponScope *pScope ) { FASSERT( IsCreated() ); m_pAttachedScope = pScope; }


	// Owner:
	FINLINE CBot *GetOwner( void ) const { FASSERT( IsCreated() ); return m_pOwnerBot; }
	FINLINE void SetOwner( CBot *pOwnerBot ) { FASSERT( IsCreated() ); m_pOwnerBot = pOwnerBot; }
	BOOL IsOwnedByPlayer( void ) const;	  //not inline, so that weapon.h doesn't have to include bot.h
	s32 GetSafeOwnerPlayerIndex( void ) const;
	CReticle *GetSafeReticle( void ) const;


	// Sound:
	void PlaySound( CFSoundGroup *pSoundGroup, f32 fVolMult=1.0f, f32 fPitchMult=1.0f, f32 fRadiusOverride=-1.0f, const CFVec3A *pSoundPos_WS=NULL, CFAudioEmitter **ppUserAudioEmitter=NULL ) const;
	CFAudioEmitter *AllocAndPlaySound( CFSoundGroup *pSoundGroup, f32 fVolMult=1.0f, f32 fPitchMult=1.0f, f32 fRadiusOverride=-1.0f, const CFVec3A *pSoundPos_WS=NULL ) const;


	// Damage:
	FINLINE s32 GetBotDamageBoneIndex( void ) const { FASSERT( IsCreated() ); return m_nBotDamageBoneIndex; }
	void SetBotDamageBoneIndex( s32 nBotDamageBoneIndex );
	void ConstructDamagerData( CDamageForm::Damager_t *pDestDamager=NULL );


	// State:
	static FINLINE BOOL IsValidState( State_e nState ) { return (nState>=0 && nState<STATE_FIRST_TRANSITION) || (nState>=STATE_STOWING && nState<STATE_LAST); }
	static FINLINE BOOL IsTransitionState( State_e nState ) { FASSERT( IsValidState(nState) ); return nState & 0x80000000; }
	static State_e ComputeDestinationState( State_e nState );

	FINLINE State_e CurrentState( void ) const { FASSERT( IsCreated() ); return m_nCurrentState; }
	FINLINE State_e DesiredState( void ) const { FASSERT( IsCreated() ); return m_nDesiredState; }
	void SetDesiredState( State_e nDesiredState );
	void ResetToState( State_e nState, BOOL bForce=FALSE );


	// Ammo:
	AmmoAlert_e ComputeAmmoAlert( void ) const;

	FINLINE BOOL HasAmmo( void ) const { FASSERT( IsCreated() ); return m_nClipAmmo | m_nReserveAmmo; }
	u16 ComputeTotalAmmo( void ) const;

	FINLINE u16 GetClipAmmo( void ) const { FASSERT( IsCreated() ); return m_nClipAmmo; }
	FINLINE u16 GetReserveAmmo( void ) const { FASSERT( IsCreated() ); return m_nReserveAmmo; }
	u16 SetClipAmmo( u16 nRounds, BOOL bNotify=TRUE );
	u16 SetReserveAmmo( u16 nRounds, BOOL bNotify=TRUE );

	FINLINE BOOL IsClipFull( void ) const { FASSERT( IsCreated() ); return m_nClipAmmo == m_pInfo->nClipAmmoMax; }
	FINLINE BOOL IsClipEmpty( void ) const { FASSERT( IsCreated() ); return m_nClipAmmo == 0; }
	FINLINE BOOL IsReserveFull( void ) const { FASSERT( IsCreated() ); return m_nReserveAmmo == m_pInfo->nReserveAmmoMax; }
	FINLINE BOOL IsReserveEmpty( void ) const { FASSERT( IsCreated() ); return m_nReserveAmmo == 0; }

	u16 AddToClip( u16 nRounds=1, BOOL bNotify=TRUE );
	u16 RemoveFromClip( u16 nRounds=1, BOOL bNotify=TRUE );
	u16 AddToReserve( u16 nRounds=1, BOOL bNotify=TRUE );
	u16 RemoveFromReserve( u16 nRounds=1, BOOL bNotify=TRUE );
	u16 TransferFromReserveToClip( u16 nRounds=1, BOOL bNotify=TRUE );
	u16 TransferFromClipToReserve( u16 nRounds=1, BOOL bNotify=TRUE );
	void AdjustAmmoToFitMax( BOOL bNotify=TRUE );
	u16 AddToClipThenReserve( u16 nRounds=1, BOOL bNotify=TRUE );

	FINLINE u16 GetMaxClipAmmo( void ) const { FASSERT( IsCreated() ); return m_pInfo->nClipAmmoMax; }
	FINLINE u16 GetMaxReserveAmmo( void ) const { FASSERT( IsCreated() ); return m_pInfo->nReserveAmmoMax; }
	void OverrideMaxClipAmmo( u16 nMaxRounds, BOOL bNotify=TRUE );
	void OverrideMaxReserveAmmo( u16 nMaxRounds, BOOL bNotify=TRUE );

	FINLINE BOOL CanClipBeReloaded( void ) const { FASSERT( IsCreated() ); return !IsClipFull() && GetReserveAmmo(); }
	FINLINE BOOL IsClipEmptyAndCanItBeReloaded( void ) const { FASSERT( IsCreated() ); return IsClipEmpty() && CanClipBeReloaded(); }
	void Reload( void );


	// Targeting:
	virtual void HumanTargeting_FindDesiredTargetPoint_WS( CFVec3A *pTargetPoint_WS );
	FINLINE BOOL NeedsTargetedEntityForUse( void ) const { FASSERT( IsCreated() ); return m_nWeaponFlags & WEAPONFLAG_NEEDS_TARGETED_ENTITY_FOR_USE; }
	virtual f32 GetProjectileSpeed( void ) const { return 100000000.0f; }
	void UpdateReticle( void );
	void ComputeFireUnitDir( CFVec3A *pUnitFireDir_WS, const CFVec3A *pMuzzlePos_WS, const CFVec3A *pMuzzleDir_WS, const CFVec3A *pTargetPos_WS, BOOL bNegateMuzzleDir );


	// Fire (Trigger Interface):
	FINLINE BOOL IsPreventReloadFlagSet( void ) const { FASSERT( IsCreated() ); return m_nWeaponFlags & WEAPONFLAG_PREVENT_RELOAD; }
	FINLINE BOOL IsPreventSwitchFlagSet( void ) const { FASSERT( IsCreated() ); return m_nWeaponFlags & WEAPONFLAG_PREVENT_SWITCH; }

	virtual FINLINE void ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const { *pMuzzlePoint_WS = m_MtxToWorld.m_vPos; }
	virtual FINLINE void ComputeMuzzleUnitDir_WS( CFVec3A *pMuzzleUnitDir_WS ) const { *pMuzzleUnitDir_WS = m_MtxToWorld.m_vFront; }

	virtual u32 TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pvTargetPos_WS, const CFVec3A *pBuddyFirePos_WS = NULL ) { FASSERT( IsCreated() ); return 0; }

	virtual FINLINE void Trigger_AttachRoundToOwnerBotBone( cchar *pszBoneName ) { FASSERT( IsCreated() ); }
	virtual FINLINE void Trigger_AttachRoundToWeapon( void ) { FASSERT( IsCreated() ); }
	virtual FINLINE void Trigger_RemoveRoundFromWorld( void ) { FASSERT( IsCreated() ); }


	// For clip-based weapons:
	virtual FINLINE void Clip_Eject( void ) { FASSERT( IsCreated() ); }
	virtual FINLINE void Clip_AttachToOwnerBotBone( cchar *pszBoneName ) { FASSERT( IsCreated() ); }
	virtual FINLINE void Clip_AttachToWeapon( void ) { FASSERT( IsCreated() ); }
	virtual FINLINE void Clip_DiscardAttachedToOwnerBotBone( void ) { FASSERT( IsCreated() ); }
	virtual FINLINE void Clip_SlapIn( void ) { FASSERT( IsCreated() ); }
	FINLINE BOOL Clip_IsInstalled( void ) { FASSERT( IsCreated() ); return m_nWeaponFlags & WEAPONFLAG_CLIP_INSTALLED; }


	// Fire (Throwable Interface):
	virtual FINLINE BOOL Throwable_TriggerWork( f32 fUnitTriggerVal ) { FASSERT( IsCreated() ); return FALSE; }
	virtual FINLINE BOOL Throwable_Prime( void ) { FASSERT( IsCreated() ); return FALSE; }
	virtual FINLINE void Throwable_AttachGrenadeToOwnerBotBone( cchar *pszBoneName ) { FASSERT( IsCreated() ); }
	virtual FINLINE void Throwable_ThrowGrenade_TargetPoint( const CFVec3A *pTargetPos_WS, f32 fSpeed ) { FASSERT( IsCreated() ); }
	virtual FINLINE void Throwable_ThrowGrenade_MountAimDirection( f32 fSlingSpeed=-1.0f, BOOL bLimbDangling=FALSE ) { FASSERT( IsCreated() ); }
	virtual FINLINE void Throwable_ReturnGrenade( void ) { FASSERT( IsCreated() ); }
	void Throwable_ThrowGrenade( CEProj *pProjToThrow, const CFVec3A &vVelocity_WS, CFSoundGroup *pSoundToPlay=NULL );


	// Rumble:
	virtual void KillRumble( void ) { FASSERT( IsCreated() ); }


	// Status:
	virtual BOOL DidFireLastFrame( void ) const { FASSERT( IsCreated() ); return FALSE; }
	virtual f32	GetAIFireSoundAudibleRange( void ) const { FASSERT( IsCreated() ); return 0.0f; };

	FINLINE CEntity *GetTargetedEntity( void ) const { FASSERT( IsCreated() ); return( m_pTargetedEntity ); }
	FINLINE void SetTargetedEntity( CEntity *pEntity ) { FASSERT( IsCreated() ); m_pTargetedEntity = pEntity; }

	virtual FINLINE BOOL IsThrowableProjectileReady( void ) const { return FALSE; }
	virtual FINLINE void ClearAllWeaponTargets( void ) {}

	virtual FINLINE void SetDisabledFlag( BOOL bDisabled ) { bDisabled ? FMATH_SETBITMASK( m_nWeaponFlags, WEAPONFLAG_WEAPON_DISABLED ) : FMATH_CLEARBITMASK( m_nWeaponFlags, WEAPONFLAG_WEAPON_DISABLED ); };


	virtual void CheckpointSaveSelect( s32 nCheckpoint );
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );


	FINLINE void SetSingleMeshForEUKs( s32 nEUK ) { m_nSingleMeshForEUKs = nEUK; }	// if this is set, all EUKs will share this mesh

	static u32 GetMaxUpgradeLevelForGlitch( WeaponType_e eType );



//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	CWeapon();
	BOOL Create( WeaponType_e nWeaponType, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

	virtual BOOL ClassHierarchyBuild( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );

	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );
	virtual void ClassHierarchyWork( void );
	virtual void InflictDamage( CDamageData *pDamageData );

	virtual FINLINE void ClassHierarchyResetToState( void ) {}
	virtual FINLINE void BeginStateChange( void ) { FASSERT( IsCreated() ); StateChangeComplete(); }
	void StateChangeComplete( void );

	virtual FINLINE void ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel ) { FASSERT( IsCreated() ); }

	virtual void BeginReload( void ) { FASSERT( IsCreated() ); ReloadComplete(); }
	void ReloadComplete( void );

	virtual FINLINE void NotifyAmmoMightHaveChanged( void ) { FASSERT( IsCreated() ); }

	virtual FINLINE void ReticleTypeSet( void ) { FASSERT( IsCreated() ); }

	FINLINE void SetPreventReloadFlag( void ) { FASSERT( IsCreated() ); FMATH_SETBITMASK( m_nWeaponFlags, WEAPONFLAG_PREVENT_RELOAD ); }
	FINLINE void ClearPreventReloadFlag( void ) { FASSERT( IsCreated() ); FMATH_CLEARBITMASK( m_nWeaponFlags, WEAPONFLAG_PREVENT_RELOAD ); }
	FINLINE void SetPreventSwitchFlag( void ) { FASSERT( IsCreated() ); FMATH_SETBITMASK( m_nWeaponFlags, WEAPONFLAG_PREVENT_SWITCH ); }
	FINLINE void ClearPreventSwitchFlag( void ) { FASSERT( IsCreated() ); FMATH_CLEARBITMASK( m_nWeaponFlags, WEAPONFLAG_PREVENT_SWITCH ); }

	// Low-level reticle enable functions.
	// Note that CBot::ReticleEnable(FALSE) disables drawing regardless of Reticle_EnableDrawing() setting.
	void Reticle_EnableDraw( BOOL bDraw );
	FINLINE BOOL Reticle_IsDrawEnabled( void ) { FASSERT( IsCreated() ); return (!(m_nWeaponFlags & WEAPONFLAG_DISABLE_RETICLE)); }

	void EjectClip( const CFVec3A *pPos_WS, FMesh_t *pEjectMesh, f32 fDebrisMeshScale=1.0f );

	BOOL CreateSharedEUKData( FMeshInit_t *pMeshInit, u32 *puEUK, CFWorldMesh **ppWorldMesh, CFAnimCombiner **ppAnimCombiner=NULL, CFAnimMeshRest **ppAnimMeshRest=NULL );
	BOOL DestroySharedEUKData( u32 uEUK );
	//BOOL CreateSharedEUKData( FMeshInit_t *pMeshInit, BOOL *pbNewData, CFWorldMesh **ppWorldMesh, CFAnimCombiner **ppAnimCombiner=NULL, CFAnimMeshRest **ppAnimMeshRest=NULL );
	//void DestroySharedEUKData( void );


//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _ClearDataMembers( void );
	void _AmmoMayHaveChanged( BOOL bNotify );
	static BOOL _FindTrackersIntersectingRayCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume, const CFVec3 *pIntersectionPoint_WS, f32 fUnitDistToIntersection );
	static void _EjectClipDebrisCallback( CFDebris *pDebris, CFDebrisDef::CallbackReason_e nReason, const FCollImpact_t *pCollImpact );
	void _UpdateReticle( s32 nPlayerIndex );
	static void _BuildProjSkipList( CEProj *pProj );


	FCLASS_STACKMEM_ALIGN( CWeapon );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CWeaponBuilder : public CEntityBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	CWeapon::WeaponType_e m_nWeaponType;		// The type of weapon
	CBot *m_pOwnerBot;							// Points to the bot who owns this weapon (NULL=no owner)
	u32 m_nUpgradeLevel;						// EUK upgrade level
	u16 m_nClipAmmo;							// Number of rounds in the clip (if infinite clip ammo, this is INFINITE_AMMO)
	u16 m_nReserveAmmo;							// Number of rounds outside the clip (if infinite reserve ammo, this is INFINITE_AMMO)




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CWeaponBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CWeaponBuilder );
} FCLASS_ALIGN_SUFFIX;


#endif

