//////////////////////////////////////////////////////////////////////////////////////
// weapon_scope.h - Sniper scope.
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

#ifndef _WEAPON_SCOPE_H_
#define _WEAPON_SCOPE_H_ 1

#include "fang.h"
#include "weapon.h"
#include "eproj.h"
#include "camsimple.h"



class CBot;
class CFSoundGroup;



	
FCLASS_ALIGN_PREFIX class CWeaponScope : public CWeapon {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		ZOOMMODE_DISABLED,					// Not zoomed
		ZOOMMODE_TRANSITIONING,				// Transitioning between bot-cam and zoom-cam
		ZOOMMODE_ENABLED,					// In zoom mode (may be animating, but not for a transition)

		ZOOMMODE_COUNT
	} _ZoomMode_e;


	typedef enum {
		_MESH_SCOPE,

		_MESH_COUNT
	} _Mesh_e;


	enum {
		ZOOM_LEVEL_COUNT = 3
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

	typedef struct {						// NOTE: If you change this structure, you must also change _Anim_e and m_aUserPropVocab
		cchar *apszMeshName[_MESH_COUNT];	// Mesh names

		u32 nZoomLevelCount;				// Number of zoom levels
		f32 afMag[ZOOM_LEVEL_COUNT];		// Magnification levels (from lowest to highest, 0=not used)
		f32 fDigitalZoomSpeed;				// Zoom speed when adjusting with the digital control (toggling through the zoom levels)
		f32 fScopeCullDist;					// Don't draw the weapon if it's this far from the camera
		BOOL bDisplayTargetInfo;			// TRUE=display information about the target
		f32 afBotAimRevsPerSec[ZOOM_LEVEL_COUNT];	// Bot aim revolutions per second

		CFSoundGroup *pSoundGroupReject;			// Scope cannot be used at this time
		CFSoundGroup *pSoundGroupActivate;			// Activate the scope (normal cam -> zoom cam)
		CFSoundGroup *pSoundGroupDeactivate;		// Deactivate the scope (zoom cam -> normal cam)
		CFSoundGroup *pSoundGroupZoomInOneLevel;	// Zoom in one level (zoom cam -> zoom cam)
	} _UserProps_t;


	enum {
		_SCOPEFLAG_USING_ZOOM_CAM				= 0x00000001,	// The zoom camera is active
		_SCOPEFLAG_TRANSITIONING_INTO_ZOOM		= 0x00000002,	// We're transitioning from normal-bot-cam to zoom-cam
		_SCOPEFLAG_TRANSITIONING_OUT_OF_ZOOM	= 0x00000004,	// We're transitioning from zoom-cam to normal-bot-cam

		_SCOPEFLAG_NONE							= 0
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static _UserProps_t m_aUserProps[EUK_COUNT_SCOPE];

	static const FGameData_TableEntry_t m_aUserPropVocab[];
	static const FGameDataMap_t m_aUserPropMapTable[];

	_UserProps_t *m_pUserProps;

	f32 m_fUnitJolt;						// Jolt from the bot

	_ZoomMode_e m_nZoomMode;				// Current zoom mode
	u32 m_nScopeFlags;						// Scope flags
	s32 m_nTargetZoomLevel;					// Current zoom level we're shooting for, or are currently at (-1=none)
	f32 m_fZoomMag;							// Current zoom magnification (e.g. 2 = x2)

	f32 m_fUnitZoomAnim;					// Slides between 0 and 1 when animating the zoom FOV from m_fZoomAnimStartHalfFOV to m_fZoomAnimEndHalfFOV
	f32 m_fUnitZoomAnimSpeed;				// Speed at which m_fUnitZoomAnim is animated (0=none)
	f32 m_fZoomAnimStartHalfFOV;			// When animating the zoom, this is the start half-FOV
	f32 m_fZoomAnimEndHalfFOV;				// When animating the zoom, this is the end half-FOV

	f32 m_fOrigHalfFOV;						// This is the bot camera's original half-FOV
	f32 m_fOrigTanHalfFOV;					// This is tan(m_fOrigHalfFOV);
	CReticle::Type_e m_nOrigReticleType;	// This is the original reticle type
	f32 m_fOrigReticleOffsetX;				// This is the original reticle offset X
	f32 m_fOrigReticleOffsetY;				// This is the original reticle offset Y

	CCamSimpleInfo m_CamInfo;				// Camera info




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );
	virtual void SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo=FALSE );

	CWeaponScope();
	virtual ~CWeaponScope();


	// Creation:
	BOOL Create( cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );


	// Scope Operation:
	f32 GetZoomMag( void ) const { FASSERT( IsCreated() ); return m_fZoomMag; }

	void PlayRejectSound( void );
	void EnterZoomMode( u32 uTargetZoomLevel=0  );
	void LeaveZoomMode( BOOL bImmediately=FALSE );

	BOOL DigitalZoom_ToggleZoomLevel( void );

	void SetJolt( f32 fUnitJolt );


	// Scope Info:
	FINLINE BOOL IsZoomEnabled( void ) const { FASSERT( IsCreated() ); return (m_nZoomMode == ZOOMMODE_ENABLED); }
	FINLINE f32 GetBotAimRevsPerSec( void ) const;




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL ClassHierarchyBuild( void );
	virtual BOOL ClassHierarchyBuilt( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );
	virtual void ClassHierarchyDestroy( void );
	virtual void ClassHierarchyRemoveFromWorld( void );

	virtual void ClassHierarchyWork( void );
	virtual void ClassHierarchyResetToState( void );
	virtual void ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _ClearDataMembers( void );
	void _SwitchToBotCam( f32 fNewHalfFOV );
	CFCamera* _GetMyCamera( void );	// Get the camera for my owner

	FCLASS_STACKMEM_ALIGN( CWeaponScope );
} FCLASS_ALIGN_SUFFIX;



FINLINE f32 CWeaponScope::GetBotAimRevsPerSec( void ) const {
	FASSERT( IsCreated() );
	
	if( m_nTargetZoomLevel >= 0 ) {
		return m_pUserProps->afBotAimRevsPerSec[m_nTargetZoomLevel];
	} else {
		return 0.0f;
	}
}



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponScopeBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CWeaponScopeBuilder : public CWeaponBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CWeaponScopeBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CWeaponScopeBuilder );
} FCLASS_ALIGN_SUFFIX;






#endif

