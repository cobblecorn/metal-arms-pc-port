//////////////////////////////////////////////////////////////////////////////////////
// reticle.cpp - Targeting redicle module.
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
// 03/29/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _RETICLE_H_
#define _RETICLE_H_ 1

#include "fang.h"
#include "fcolor.h"
#include "flinklist.h"
#include "fviewport.h"
#include "ftex.h"
#include "fdraw.h"
#include "fsndfx.h"


#define RETICLE_SCOPE_INFO_SLOT_COUNT	6


class CEntity;
class CBot;
class CFSoundGroup;



FCLASS_ALIGN_PREFIX class CReticle {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		TYPE_NONE,					// No reticle will be displayed
		TYPE_DROID_STANDARD,		// Standard Droid reticle
		TYPE_BLASTER,				// Droid Scatter Blaster
		TYPE_TETHER,				// Control tether reticle
		TYPE_FLAMER,				// Flamer reticle
		TYPE_DROID_SCOPE,			// The Droid scope reticle
		TYPE_FLOORSENTRY,
		TYPE_QUADLASER,				// Predator quad laser
		TYPE_TANK_CANNON,			// Tank cannon reticle

		TYPE_COUNT
	} Type_e;


	typedef enum {
		AMMO_TEXT_NONE,			// Displays no ammo text

		AMMO_TEXT_LOW,			// Displays "Low Ammo"
		AMMO_TEXT_NO,			// Displays "No Ammo"
		AMMO_TEXT_RELOAD,		// Displays "Reload"

		_AMMO_TEXT_SET_TO_NONE,	// Used internally to auto-set the ammo text to "none" next frame unless the weapon calls SetAmmoText()

		AMMO_TEXT_COUNT
	} AmmoText_e;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	enum {
		HALO_RING_COUNT = 10,
	};


	enum {
		FLAG_CREATED					= 0x00000001,		// Reticle object has been created
		FLAG_ENABLE_DRAWING				= 0x00000002,		// Enable the drawing of this reticle
		FLAG_ACTIVE_COLOR				= 0x00000004,		// Reticle should be drawn with "active" color scheme
		FLAG_DISPLAY_SCOPE_TARGET_INFO	= 0x00000008,		// Display the scope target info
		FLAG_OVERRIDE_WEAPON_REQUESTS	= 0x00000010,		// TRUE when a site weapon or something wants total reticle power

		FLAG_NONE						= 0x00000000
	};


	typedef enum {
		TEXTURE_NONE,									// No texture
		TEXTURE_STD_CROSSHAIRS,							// Standard crosshairs
		TEXTURE_BLASTER,								// Droid Scatter Blaster
		TEXTURE_FLAMER,									// Flamer reticle
		TEXTURE_DROID_SCOPE1,							// Droid scope texture #1
		TEXTURE_DROID_SCOPE2,							// Droid scope texture #2
		TEXTURE_STATIC,									// Static texture
		TEXTURE_FLOORSENTRY,
		TEXTURE_QUADLASER,								// quad laser tex (must be drawn 4 times)
		TEXTURE_TANK_CANNON,							// tank cannon reticle

		TEXTURE_COUNT
	} Texture_e;


	typedef struct {
		cchar *pszTexName;
		f32 fScale;
		f32 fNormScreenRadius;
	} TexInfo_t;


	typedef struct {
		CFColorRGB ColorRGB;
		f32 fUnitIntensity;
		f32 fRotAngle;
		f32 fIntensityFadeSpeed;
		BOOL bFadeEnds;
	} HaloInfo_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	FLink_t m_Link;								// Link to other reticle objects

	Type_e m_nType;								// Reticle type
	u32 m_nFlags;								// See FLAG_* for info

	f32 m_fUnitAnimScale;						// Amount of stretch animation
	f32 m_fNormScale;							// Normalized scale: 1.0=height of entire viewport, 0.5=half height of entire viewport, etc.
	f32 m_fNormOriginX;							// Normalized X origin: 1.0=center is positioned as far right proportional to vertical dimension of viewport
	f32 m_fNormOriginY;							// Normalized Y origin: 1.0=center is at top of viewport
	CFColorRGBA m_InactiveColor;				// Color and opacity bias used when reticle is inactive
	CFColorRGBA m_ActiveColor;					// Color and opacity bias used when reticle is active
	u32 m_nPlayerIndex;							// The index of the player this reticle belongs to (never <0)

	// Targeting:
	CFVec3A m_TargetedPoint_SS;					// Current point being targeted
	f32 m_fTargetLockOnSpeed;					// 1/seconds to lock onto a target
	f32 m_fTargetedUnitIntensity;				// Used to describe how well the point is being targeted
	f32 m_fTargetedUnitLock;					// Unit lock amount (1=we're completely locked)
	f32 m_fTargetedUnitZoom;					// Used while zooming reticle into target
	f32 m_fTargetedUnitLockSweep;				// While locking, this sweeps to animate lock signal
	HaloInfo_t m_aHaloInfo[HALO_RING_COUNT];	// Info for halo rings
	f32 m_fTargetedLockRotAngle;				// Lock rotation angle
	BOOL8 m_bTargetedIDBeepPlayed;				// ID sound already played
	BOOL8 m_bTargetedLockPlayBeep;				// Play lock sound
	u8 m_nAmmoText;								// Ammo text to display next frame (see AmmoText_e for info)
	u8 m_nAmmoTextFlashCount;					// The remaining number of times to flash the ammo text count (even=off, odd=on)
	f32 m_fAmmoTextFlashSecs;					// Used to time flashing ammo text

	// Scope:
	f32 m_fScopeHeading;						// Scope heading angle
	f32 m_fScopePitch;							// Scope pitch angle
	f32 m_fScopeZoomMultiplier;					// Scope zoom multiplier
	CBot *m_pScopeTargetedBot;					// Currently targeted bot (NULL=none)
	cwchar *m_pwszScopeTargetName;				// Target name (NULL=unknown)
	f32 m_fScopeInfoDisplaySecs;				// Seconds remaining until we go on to display the next info item
	u32 m_nScopeInfoDisplayCount;				// The number of items that are displayed (0=none)
	f32 m_afScopeInfoIntensity[RETICLE_SCOPE_INFO_SLOT_COUNT];	// Intensity of text in the scope info slots
	s32 m_nScopeInfoBorderIndex;				// The info index of where to display the border (-1=none)
	f32 m_fScopeInfoBorderSecs;					// Seconds remaining until we display the next border
	f32 m_fScopeFlashMagSecs;					// Used to time flashing the mag level
	u32 m_nScopeFlashMagCount;					// The remaining number of times to flash the mag level (even=on, odd=off)
	f32 m_fScopeUnitJolt;						// Amount of jolt to the image

	static BOOL m_bSystemInitialized;			// TRUE when InitSystem() has been called successfully
	static BOOL m_bDrawAllEnabled;				// TRUE to allow drawing of all reticles, FALSE to prevent drawing of all reticles
	static FLinkRoot_t m_LinkRoot;				// Linklist of reticle objects
	static FDrawVtx_t m_aFDrawVtx[8];			// FDraw vertices to be drawn
	static f32 m_afNormScreenRadius[TYPE_COUNT];// The normalized screen radius of the reticle
	static CFTexInst m_aTexInst[TEXTURE_COUNT];	// The CFTexInst for each reticle type
	static TexInfo_t m_aTexInfo[TEXTURE_COUNT];	// Texture information
	static s32 m_anTexIndex[TYPE_COUNT];		// Maps the reticle type to a texture (not used by all reticle types) (-1=none)
	static f32 m_fHaloRingStep;					// 1.0f / (HALO_RING_COUNT - 1)
	static f32 m_fPentVtxNormOffset;			// _PENT_NORM_THICKNESS * tan(_PENT_CORNER_ANGLE)
	static f32 m_fPentSideSin;					// Sin of the pentagon's angle
	static f32 m_fPentSideCos;					// Sin of the pentagon's angle

	static CFSoundGroup *m_pSoundGroupTetherIdentified;	// Sound played when tether targeting system first recognizes a data port
	static CFSoundGroup *m_pSoundGroupTetherLockBeep;	// Sound played when tether targeting system has a lock




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	CReticle();
	~CReticle();

	static BOOL InitSystem( void );
	static void UninitSystem( void );
	static FINLINE BOOL IsSystemInitialized( void ) { return m_bSystemInitialized; }

	static void EnableDrawAll( BOOL bEnable );
	static BOOL IsDrawAllEnabled( void ) { return m_bDrawAllEnabled; }

	BOOL Create( Type_e nType, u32 nPlayerIndex );
	void Destroy();

	FINLINE BOOL IsCreated( void ) const { return (BOOL)(m_nFlags & FLAG_CREATED); }

	void SetType( Type_e nType );
	FINLINE Type_e GetType( void ) const { FASSERT( IsCreated() ); return m_nType; }

	FINLINE void EnableDraw( BOOL bEnable, BOOL bThisIsAnOverrideRequest = FALSE );
	FINLINE BOOL IsDrawEnabled( void ) const { FASSERT( IsCreated() ); return ((BOOL)(m_nFlags & FLAG_ENABLE_DRAWING)); }

	FINLINE void ColorScheme_SetInactiveColor( const CFColorRGBA *pColorRGBA ) { FASSERT( IsCreated() ); m_InactiveColor = *pColorRGBA; }
	FINLINE void ColorScheme_SetActiveColor( const CFColorRGBA *pColorRGBA ) { FASSERT( IsCreated() ); m_ActiveColor = *pColorRGBA; }
	FINLINE void ColorScheme_SelectInactive( void ) { FASSERT( IsCreated() ); FMATH_CLEARBITMASK( m_nFlags, FLAG_ACTIVE_COLOR ); }
	FINLINE void ColorScheme_SelectActive( void ) { FASSERT( IsCreated() ); FMATH_SETBITMASK( m_nFlags, FLAG_ACTIVE_COLOR ); }

	FINLINE void SetReticleOverride( void )   { FASSERT( IsCreated() ); FMATH_SETBITMASK( m_nFlags, FLAG_OVERRIDE_WEAPON_REQUESTS); }
	FINLINE void ClearReticleOverride( void ) { FASSERT( IsCreated() ); FMATH_CLEARBITMASK( m_nFlags, FLAG_OVERRIDE_WEAPON_REQUESTS); }
	FINLINE BOOL IsReticleOverriden( void )   { FASSERT( IsCreated() ); return (!!(m_nFlags & FLAG_OVERRIDE_WEAPON_REQUESTS)); }

	FINLINE void SetUnitScale( f32 fNewUnitScale ) { FASSERT( IsCreated() ); m_fUnitAnimScale = fNewUnitScale; }
	FINLINE f32 GetUnitScale( void ) const { FASSERT( IsCreated() ); return m_fUnitAnimScale; }

	FINLINE void SetNormScale( f32 fNewNormScale ) { FASSERT( IsCreated() ); m_fNormScale = fNewNormScale; }
	FINLINE f32 GetNormScale( void ) const { FASSERT( IsCreated() ); return m_fNormScale; }

	FINLINE void SetNormOrigin( f32 fNewX, f32 fNewY ) { FASSERT( IsCreated() ); m_fNormOriginX = fNewX; m_fNormOriginY = fNewY; }
	FINLINE f32 GetNormOriginX( void ) const { FASSERT( IsCreated() ); return m_fNormOriginX; }
	FINLINE f32 GetNormOriginY( void ) const { FASSERT( IsCreated() ); return m_fNormOriginY; }

	f32 GetScreenSpaceRadius( void ) const { FASSERT( IsCreated() ); return m_afNormScreenRadius[m_nType]; }
	BOOL IsPointWithinReticle( const FViewport_t *pViewport, const CFVec3A *pPoint_WS ) const;
	f32 ComputeUnitWithinReticle( const FViewport_t *pViewport, const CFVec3A *pPoint_WS ) const;


	// Ammo text:
	FINLINE void SetAmmoText( AmmoText_e nAmmoText );


	// Targeting:
	void TargetSys_Reset( void );
	void TargetSys_SetLockOnSpeed( f32 fLockOnSpeed );
	FINLINE f32 TargetSys_GetUnitLock( void ) const;
	void TargetSys_UpdateTargetedPoint( const FViewport_t *pViewport, const CFVec3A *pTargetedPoint_WS );
	void TargetSys_UpdateTargetedIntensity( f32 fUnitIntensity );
	f32 TargetSys_ComputeUnitInaccuracy( void ) const;


	// Scope:
	void Scope_SetParameters( f32 fHeading, f32 fPitch, f32 fZoomMultiplier, CEntity *pTargetedEntity, BOOL bDisplayTargetInfo, f32 fUnitJolt );


	static void WorkAll( void );
	static void DrawAll( void );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static CReticle *GetFirst( void ) { return (CReticle *)flinklist_GetHead( &m_LinkRoot ); }
	static CReticle *GetLast( void ) { return (CReticle *)flinklist_GetTail( &m_LinkRoot ); }
	static CReticle *GetNext( CReticle *pReticle ) { return (CReticle *)flinklist_GetNext( &m_LinkRoot, pReticle ); }
	static CReticle *GetPrev( CReticle *pReticle ) { return (CReticle *)flinklist_GetPrev( &m_LinkRoot, pReticle ); }
	static CReticle *AddToEnd( CReticle *pReticle ) { return (CReticle *)flinklist_AddTail( &m_LinkRoot, pReticle ); }
	static CReticle *RemoveLast( void ) { return (CReticle *)flinklist_RemoveTail( &m_LinkRoot ); }
	static CReticle *Remove( CReticle *pReticle ) { return (CReticle *)flinklist_Remove( &m_LinkRoot, pReticle ); }

	void _Draw_DroidStandard( Type_e nType );
	void _Draw_MirrorXY( Type_e nType );
	void _Draw_Tether( void );
	void _Draw_TetherLineScale( FViewport_t *pViewport, const CFColorRGBA *pColorRGBA, f32 fNormInnerRadius, f32 fNormOuterRadius, f32 fNormTickSize, f32 fScale );
	void _Draw_TetherPentagon( FViewport_t *pViewport, const CFVec3A *pNormOrig, const CFColorRGBA *pColorRGBA, f32 fNormThickness, f32 fNormRadius, f32 fRotAngle, BOOL bFadeEnds );
	void _Draw_Scope( void );
	void _Draw_DroidScopeTexture( CFTexInst *pTexInst, const CFColorRGBA *pColorBias );
	void _Draw_ScopeStatic( const CFColorRGBA *pColorBias );
	void _Draw_ScopeGadgets( void );
	
	void _InitScopeInfoTextBoxes( void );

	static void _ScopeFullscreenRenderTargetReadyCallback( void );

	void _DisplayAmmoText( void );


	FCLASS_STACKMEM_ALIGN( CReticle );
} FCLASS_ALIGN_SUFFIX;




FINLINE void CReticle::EnableDraw( BOOL bEnable, BOOL bThisIsAnOverrideRequest ) {
	FASSERT( IsCreated() );

	if  ( (bThisIsAnOverrideRequest) ||		// means this request must be obeyed
		(!(m_nFlags & FLAG_OVERRIDE_WEAPON_REQUESTS)) ){// only allow changes when not in overriden mode
		if( bEnable ) {
			if( !IsDrawEnabled() ) {
				TargetSys_Reset();
				FMATH_SETBITMASK( m_nFlags, FLAG_ENABLE_DRAWING );
			}
		} else {
			if( IsDrawEnabled() ) {
				TargetSys_Reset();
				FMATH_CLEARBITMASK( m_nFlags, FLAG_ENABLE_DRAWING );
			}
		}
	}
}


FINLINE f32 CReticle::TargetSys_GetUnitLock( void ) const {
	FASSERT( IsCreated() );

	if( m_fTargetedUnitIntensity==1.0f && m_fTargetedUnitZoom==1.0f ) {
		return m_fTargetedUnitLock;
	} else {
		return 0.0f;
	}
}


FINLINE void CReticle::SetAmmoText( AmmoText_e nAmmoText ) {
	FASSERT( IsCreated() );
	FASSERT( nAmmoText>=0 && nAmmoText<AMMO_TEXT_COUNT );

	if( nAmmoText != m_nAmmoText ) {
		if( m_nAmmoText != _AMMO_TEXT_SET_TO_NONE ) {
			m_nAmmoTextFlashCount = 0;
			m_fAmmoTextFlashSecs = 0.0f;
		}

		m_nAmmoText = nAmmoText;
	}
}



#endif

