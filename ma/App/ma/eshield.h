//////////////////////////////////////////////////////////////////////////////////////
// EShield.h	- bot shields
//
// Author: Mike Elliott
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
// 11.19.02 Elliott		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _ESHIELD_H_
#define _ESHIELD_H_ 1

#include "fang.h"
#include "entity.h"

class  CBot;
struct _ShieldSplash_t;		//defined in .cpp

typedef struct {
	u32				uShieldFlags;
	f32				fShieldScale;								// scale for the shield's radius
	f32				fShieldRechargeTime;						// time to go from 0 - 1.0
	f32				fShieldRechargeDelay;						// time to wait after taking damage before recharging
	CArmorProfile*	pArmorProfile;								// the shield's armor profile
	CFColorRGB		rgbTint;									// tint to apply to the shield and effects (must set SHIELD_ENABLE_TINT)
} ShieldInit_t;

typedef enum {
	SHIELD_NOTIFY_IMPACT,
	SHIELD_NOTIFY_UP,
	SHIELD_NOTIFY_DOWN
} ShieldNotifications_e;


FCLASS_ALIGN_PREFIX class CEShield : public CEntity {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	// not enabled.  maybe later
	enum {
		SHIELD_ENABLE_TINT		= 0x01,							// tint the shield
		SHIELD_NOPROTECTION		= 0x02,
		SHIELD_INVULNERABLE		= 0x04,
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
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL m_bSystemInitialized;
	static BOOL m_bEffectsLoaded;

	CFWorldMesh *m_pWorldMesh;
	static _ShieldSplash_t *m_aSplashes;
	static FLinkRoot_t		m_FreeSplashList;

	FLinkRoot_t				m_SplashList;			// each shield has a list of its active splashes

	FMeshTexLayerHandle_t	m_hTexLayer;			// the tex layer for animating the shield up/down effect
	
	
	CBot	*m_pOwnerBot;							// who we're shielding
	f32		 m_fShieldRechargeTime;					// time to recharge
	f32		 m_fOOShieldRechargeTime;				
	f32		 m_fShieldRechargeDelay;				// time to wait after taking damage before recharging
	f32		 m_fShieldRechargeTimer;
	f32		m_fShieldAlpha;							// alpha for the shield mesh

	static CEShield* m_pCollThis;					// to be used by the shield callback
	static const CFVec3A* m_pPosToDrawSplash;		// used to cache splash effect requests, so they don't happen in callbacks

	BOOL m_bEnabled;								// is the shield enabled?  If not, no work gets called
	BOOL m_bShieldActive;							// is shield actively defending the bot?
	BOOL m_bPushOutBots;							// should this shield push away bots that are inside it?
	
	CFColorRGB	m_rgbTintColor;						// color to tint this shield's effects

	f32 m_fFlickerTimer;							// countdown to next flicker
	u32 m_nFlickerCount;							// number of flickers remaining
	BOOL m_bShieldSuspended;						// suspend the shield's effect

	u32 m_uFlags;
	

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );

	CEShield();
	~CEShield();

	BOOL Create( cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );
	void AddSplashEffect( const CFVec3A &rvPos, f32 fMagnitude=1.0f );
	void SetOwner( CBot *pBot )		{ m_pOwnerBot = pBot; };
	
	void Work( void );

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);

	void Init( CBot *pOwnerBot, const ShieldInit_t *pShieldInit=NULL, BOOL bStartOn=TRUE );							// Init the shield

	void NotifyBotDamage( void )  { FASSERT( IsCreated() ); m_fShieldRechargeTimer = m_fShieldRechargeDelay; };		// notify the shield that the bot received damage


	//handle damage
	virtual void InflictDamageResult( const CDamageResult *pDamageResult );
	virtual void InflictDamage( CDamageData *pDamageData );


	void EnablePushOutBots( BOOL bEnable )		{ FASSERT( IsCreated() ); m_bPushOutBots = bEnable; };
	BOOL IsPushOutBotsEnabled( void ) const		{ return m_bPushOutBots; };
	void SetShieldHealth( f32 fUnitHealth );																	// directly sets the health & state without any visible effect
	void RechargeNow( void )					{ FASSERT( IsCreated() ); m_fShieldRechargeTimer = -1.0f; };	// tell the shield to immediately recharge

	FINLINE BOOL IsEnabled(void)		const { return m_bEnabled;	};			// a disabled shield does no work, does not recharge, etc
	FINLINE BOOL IsShieldOn( void )		const { return NormHealth() > 0.0f; };	// returns whe
	FINLINE BOOL IsShieldActive( void ) const { return m_bShieldActive; };		// returns TRUE if the shield is actively protecting the bot from damage

	FINLINE void SetShieldInvulnerable( BOOL bInv ) { if( bInv ) m_uFlags |= SHIELD_INVULNERABLE; else m_uFlags &= ~SHIELD_INVULNERABLE; };
	FINLINE BOOL IsShieldInvulnerable( void ) const { return !!( m_uFlags & SHIELD_INVULNERABLE );	};

	FINLINE f32 GetShieldScale( void ) const { return m_fScaleToWorld; };
	FINLINE void SetShieldScale( f32 fScale ) { FMATH_CLAMPMIN( fScale, 0.00001f ); m_fScaleToWorld = fScale; };

	FINLINE virtual CFWorldMesh *GetMesh( void ) const { FASSERT( IsCreated() ); return m_pWorldMesh; }

	void Suspend( BOOL bSuspend );					// causes the shield to remain visible, but disables protection (collision and pushing out of bots)
	void DropShield( f32 fRechargeTime=-1.0f );		// Turns off the shield.  Shield logic is still active, and the shield will come back up after the power down time.  Will drop
													// immediately (No flickering/turning red)
	void EnableShield( BOOL bEnable );				// Enable/disable all shield logic / work.  Doesn't change the shield's states
	
	void SetTint(CFColorRGB& rTint);


//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:
	virtual CEntityBuilder *GetLeafClassBuilder( void );
	virtual BOOL ClassHierarchyBuild( void );
	virtual void ClassHierarchyDestroy( void );
	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );

	virtual void ClassHierarchyRelocated( void *pIdentifier );

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	static u32 uHighestSplashOverride;
	static u32 uCurrentSplashOverride;
#endif

	static _ShieldSplash_t* _GetNextSplash( void );								// returns the next available splash, off the list of free splashes if possible
																				// if not, will get the oldest one out of the array
	static BOOL _InitEffects( void );
	static void _DestroyEffects( void );

	void _KillSplash( f32 fTime, BOOL bReleaseAll=FALSE );						// Release the (first if !bReleaseAll) splash in the list that is at least as old as fTime
	
	void _UpdateSplashPosition( const CFVec3A &vShieldPos, _ShieldSplash_t *pSplash );

	void _HandleShieldLevelandState( void );

	void _ShieldOn( void );
	void _ShieldOff( BOOL bShowEffects=TRUE );

	void _PushOutBots( void );

	static BOOL _CollisionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume );

	void _OrientMesh( void );

	_ShieldSplash_t* _ReleaseSplash( _ShieldSplash_t *pSplash );				// turns it off, removes from our list and adds back to free list.  Returns next in our work list or NULL

FCLASS_STACKMEM_ALIGN( CEShield );
} FCLASS_ALIGN_SUFFIX;






//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEShieldBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CEShieldBuilder : public CEntityBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:


//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CEShieldBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );


//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CEShieldBuilder );
} FCLASS_ALIGN_SUFFIX;


#endif
