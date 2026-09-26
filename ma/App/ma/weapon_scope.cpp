//////////////////////////////////////////////////////////////////////////////////////
// weapon_scope.cpp - Sniper scope.
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

#include "fang.h"
#include "weapon_scope.h"
#include "weapon.h"
#include "iteminst.h"
#include "gamecam.h"
#include "player.h"
#include "bot.h"
#include "fsound.h"
#include "sas_user.h"

#define _USER_PROP_FILENAME		"w_scope.csv"




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CScopeBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CWeaponScopeBuilder _WeaponScopeBuilder;


void CWeaponScopeBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CWeaponBuilder, ((u64)(ENTITY_BIT_WEAPONSCOPE)), pszEntityType );
}


BOOL CWeaponScopeBuilder::PostInterpretFixup( void ) {
	return CWeaponBuilder::PostInterpretFixup();
}


BOOL CWeaponScopeBuilder::InterpretTable( void ) {
	return CWeaponBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponScope
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CWeaponScope::_UserProps_t CWeaponScope::m_aUserProps[EUK_COUNT_SCOPE];


// This table describes to fgamedata how our user property table is to be interpreted:
const FGameData_TableEntry_t CWeaponScope::m_aUserPropVocab[] = {
	// apszMeshName[_MESH_SCOPE]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// nZoomLevelCount:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CHECK_RANGE_AND_FAIL,
	sizeof( u32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_3,

	// afMag[0]:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_1000,

	// afMag[1]:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000,

	// afMag[2]:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000,

	// fDigitalZoomSpeed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt0001,
	F32_DATATABLE_1000,

	// fScopeCullDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// bDisplayTargetInfo:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( BOOL ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// afBotAimRevsPerSec[0]:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt00001,
	F32_DATATABLE_100,

	// afBotAimRevsPerSec[1]:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt00001,
	F32_DATATABLE_100,

	// afBotAimRevsPerSec[2]:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt00001,
	F32_DATATABLE_100,

	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupReject
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupActivate
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupDeactivate
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupZoomInOneLevel


	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameDataMap_t CWeaponScope::m_aUserPropMapTable[] = {
	"ScopeL1",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[0],

	"ScopeL2",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[1],

	NULL
};




BOOL CWeaponScope::InitSystem( void ) {
	Info_t *pInfo;
	u32 i;

	FResFrame_t ResFrame = fres_GetFrame();

	fang_MemZero( m_aUserProps, sizeof(m_aUserProps) );

	// Read the user properties for all EUK levels of this weapon...
	if( !fgamedata_ReadFileUsingMap( m_aUserPropMapTable, _USER_PROP_FILENAME ) ) {
		DEVPRINTF( "CWeaponScope::InitSystem(): Could not read user properties from file '%s'.\n", _USER_PROP_FILENAME );
		goto _ExitWithError;
	}

	// Do this for each EUK level...
	for( i=0; i<EUK_COUNT_SCOPE; i++ ) {
		// Fill out our global info data...
		pInfo = &m_aaInfo[WEAPON_TYPE_SCOPE][i];

		pInfo->nGrip = GRIP_LEFT_ARM;
		pInfo->nReloadType = RELOAD_TYPE_FULLYAUTOMATIC;
		pInfo->nStanceType = STANCE_TYPE_STANDARD;
		pInfo->fMinTargetAssistDist = 0.0f;
		pInfo->fMaxTargetAssistDist = 0.0f;
		pInfo->fMaxLiveRange = 0.0f;
		pInfo->nClipAmmoMax = INFINITE_AMMO;
		pInfo->nReserveAmmoMax = INFINITE_AMMO;
		pInfo->nInfoFlags = INFOFLAG_NO_AMMO | INFOFLAG_NO_AUTO_TARGETING;
		pInfo->nReticleType = CReticle::TYPE_NONE;
	}

	// Success...

	return TRUE;

	// Error...
_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CWeaponScope::UninitSystem( void ) {
}


CWeaponScope::CWeaponScope() {
	_ClearDataMembers();
}


CWeaponScope::~CWeaponScope() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CWeaponScope::Create( cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	// Get pointer to the leaf class's builder object...
	CWeaponScopeBuilder *pBuilder = (CWeaponScopeBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_WeaponScopeBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_WEAPONSCOPE );
	}

	// Set our builder parameters...

	// Create an entity...
	return CWeapon::Create( WEAPON_TYPE_SCOPE, pszEntityName, pMtx, pszAIBuilderName );
}


BOOL CWeaponScope::ClassHierarchyBuild( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CWeaponScopeBuilder *pBuilder = (CWeaponScopeBuilder *)GetLeafClassBuilder();

	// Set input parameters for CWeapon creation...

	// Build parent class...
	if( !CWeapon::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	// Initialize from builder object...

	m_pUserProps = &m_aUserProps[ m_nUpgradeLevel ];

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


BOOL CWeaponScope::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CWeapon::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	EnableOurWorkBit();

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CWeaponScope::ClassHierarchyDestroy( void ) {
	// Destroy ourselves first...
	_ClearDataMembers();

	// Now destroy our parent...
	CWeapon::ClassHierarchyDestroy();
}


CEntityBuilder *CWeaponScope::GetLeafClassBuilder( void ) {
	return &_WeaponScopeBuilder;
}


void CWeaponScope::_ClearDataMembers( void ) {
	m_pUserProps = NULL;

	m_fUnitJolt = 0.0f;

	m_nZoomMode = ZOOMMODE_DISABLED;
	m_nScopeFlags = _SCOPEFLAG_NONE;
	m_nTargetZoomLevel = -1;
	m_fZoomMag = 1.0f;

	m_fUnitZoomAnim = 0.0f;
	m_fUnitZoomAnimSpeed = 0.0f;
	m_fZoomAnimStartHalfFOV = 0.0f;
	m_fZoomAnimEndHalfFOV = 0.0f;

	m_fOrigHalfFOV = 0.0f;
	m_fOrigTanHalfFOV = 0.0f;
	m_nOrigReticleType = CReticle::TYPE_NONE;
	m_fOrigReticleOffsetX = 0.0f;
	m_fOrigReticleOffsetY = 0.0f;

	m_CamInfo.m_Pos_WS.Zero();
	m_CamInfo.m_Quat_WS.Identity();
}


void CWeaponScope::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	CWeapon::ClassHierarchyRemoveFromWorld();
}


void CWeaponScope::SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo ) {
	CWeapon::SetItemInst( pItemInst, bUpdateItemInstAmmoFromWeaponAmmo );

	if( pItemInst ) {
		pItemInst->SetAmmoDisplayType( CItemInst::AMMODISPLAY_NONE, CItemInst::AMMODISPLAY_NONE );
	}
}


// Called once per frame to update animations, timers, etc.
void CWeaponScope::ClassHierarchyWork( void ) {
	// Call our parent. It might need to do some work...
	CWeapon::ClassHierarchyWork();

	// Exit if we don't have any work to do...
	if( !IsOurWorkBitSet() ) {
		return;
	}

#if !FANG_PRODUCTION_BUILD
	#if SAS_ACTIVE_USER == SAS_USER_STEVE
		//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
		if( m_nUpgradeLevel != 1 ) {
			SetUpgradeLevel( 1 );
		}
	#endif
#endif

	// Handle zoom animations...
	if( m_fUnitZoomAnimSpeed != 0.0f ) {
		f32 fZoomHalfFOV;

		m_fUnitZoomAnim += m_fUnitZoomAnimSpeed * FLoop_fPreviousLoopSecs;

		if( m_fUnitZoomAnimSpeed > 0.0f ) {
			if( m_fUnitZoomAnim >= 1.0f ) {
				m_fUnitZoomAnim = 1.0f;
				m_fUnitZoomAnimSpeed = 0.0f;
			}
		} else {
			if( m_fUnitZoomAnim <= 0.0f ) {
				m_fUnitZoomAnim = 0.0f;
				m_fUnitZoomAnimSpeed = 0.0f;
			}
		}

		fZoomHalfFOV = FMATH_FPOT( m_fUnitZoomAnim, m_fZoomAnimStartHalfFOV, m_fZoomAnimEndHalfFOV );

		if( m_nScopeFlags & (_SCOPEFLAG_TRANSITIONING_INTO_ZOOM | _SCOPEFLAG_TRANSITIONING_OUT_OF_ZOOM) ) {
			// Transitioning between bot-cam and zoom-cam...

			if( m_nScopeFlags & _SCOPEFLAG_TRANSITIONING_INTO_ZOOM ) {
				// Transitioning from bot-cam into zoom-cam...

				if( !(m_nScopeFlags & _SCOPEFLAG_USING_ZOOM_CAM) ) {
					// See if it's time to switch into zoom-cam...

					if( m_fUnitZoomAnim >= 0.0f ) {
						// Switch to zoom-cam...

						GetOwner()->GetScopeViewPos( &m_CamInfo.m_Pos_WS );

						m_CamInfo.m_Quat_WS.BuildQuat( *GetOwner()->MtxToWorld() );
						gamecam_SwitchPlayerToSimpleCamera( PLAYER_CAM(CPlayer::m_nCurrent), &m_CamInfo );
						_GetMyCamera()->SetFOV( fZoomHalfFOV );

						FMATH_SETBITMASK( m_nScopeFlags, _SCOPEFLAG_USING_ZOOM_CAM );

						s32 nPlayerIndex = GetOwner()->m_nPossessionPlayerIndex;
						if( nPlayerIndex >= 0 ) {
							// For single player, do a brute force hide of our
							// geometry because the scene is rendered to a texture
							// outside of the normal render loop.
							if( CPlayer::m_nPlayerCount == 1 ) {
								GetOwner()->DrawEnable( FALSE );
							} else {
								Player_aPlayer[ nPlayerIndex ].SetHiddenFromSelf( TRUE );
							}

							CReticle *pReticle = &Player_aPlayer[ nPlayerIndex ].m_Reticle;

							m_nOrigReticleType = pReticle->GetType();
							m_fOrigReticleOffsetX = pReticle->GetNormOriginX();
							m_fOrigReticleOffsetY = pReticle->GetNormOriginY();

							pReticle->SetType( CReticle::TYPE_DROID_SCOPE );
							pReticle->SetNormOrigin( 0.0f, 0.0f );
						}

						GetOwner()->UpdateAimToTargetSamePoint( TRUE );
					}
				}

				if( m_fUnitZoomAnimSpeed == 0.0f ) {
					// Transition complete...
					m_nZoomMode = ZOOMMODE_ENABLED;
					FMATH_CLEARBITMASK( m_nScopeFlags, _SCOPEFLAG_TRANSITIONING_INTO_ZOOM );
				}
			} else {
				// Transitioning from zoom-cam to bot-cam...

				if( m_nScopeFlags & _SCOPEFLAG_USING_ZOOM_CAM ) {
					// See if it's time to switch into bot-cam...

					if( m_fUnitZoomAnim >= 1.0f ) {
						// Switch to bot-cam...

						_SwitchToBotCam( m_fOrigHalfFOV );
					}
				}

				if( m_fUnitZoomAnimSpeed == 0.0f ) {
					// Transition complete...
					m_nZoomMode = ZOOMMODE_DISABLED;
					FMATH_CLEARBITMASK( m_nScopeFlags, _SCOPEFLAG_TRANSITIONING_OUT_OF_ZOOM );
				}
			}
		}

		_GetMyCamera()->SetFOV( fZoomHalfFOV );

		// Compute magnification level...
		f32 fSin, fCos, fOOTanHalfFOV;

		fmath_SinCos( fZoomHalfFOV, &fSin, &fCos );
		fOOTanHalfFOV = fmath_Div( fCos, fSin );

		m_fZoomMag = m_fOrigTanHalfFOV * fOOTanHalfFOV;
	}

	if( m_nScopeFlags & _SCOPEFLAG_USING_ZOOM_CAM ) {
		GetOwner()->GetScopeViewPos( &m_CamInfo.m_Pos_WS );
		m_CamInfo.m_Quat_WS.AcuBuildQuat( *GetOwner()->MtxToWorld() );

		if( GetOwner()->m_nPossessionPlayerIndex >= 0 ) {
			CReticle *pReticle = &Player_aPlayer[ GetOwner()->m_nPossessionPlayerIndex ].m_Reticle;

			pReticle->Scope_SetParameters(
				GetOwner()->m_fMountYaw_WS, GetOwner()->m_fMountPitch_WS,
				m_fZoomMag,
				GetTargetedEntity(),
				m_pUserProps->bDisplayTargetInfo,
				m_fUnitJolt
			);

			m_fUnitJolt = 0.0f;
		}
	}
}


void CWeaponScope::_SwitchToBotCam( f32 fNewHalfFOV ) {
	// Get our owner's index
	s32 nPlayerIndex = GetOwner()->m_nPossessionPlayerIndex;

	// Should never happen...
	if( nPlayerIndex < 0 ) {
		return;
	}

	// Switch back to bot's camera and update its FOV...
	gamecam_SwitchPlayerTo3rdPersonCamera( PLAYER_CAM(nPlayerIndex), GetOwner() );
	fcamera_GetCameraByIndex(nPlayerIndex)->SetFOV( fNewHalfFOV );

	// Draw the bot again...
	if( CPlayer::m_nPlayerCount == 1 ) {
		GetOwner()->DrawEnable( TRUE );
	} else {
		Player_aPlayer[nPlayerIndex].SetHiddenFromSelf( FALSE );
	}

	// Reset zoom-cam flag...
	FMATH_CLEARBITMASK( m_nScopeFlags, _SCOPEFLAG_USING_ZOOM_CAM );

	// Restore bot's original reticle...
	if( nPlayerIndex >= 0 ) {
		CReticle *pReticle = &Player_aPlayer[ nPlayerIndex ].m_Reticle;

		pReticle->SetType( m_nOrigReticleType );
		pReticle->SetNormOrigin( m_fOrigReticleOffsetX, m_fOrigReticleOffsetY );
	}

	// Make sure we're still targeting the same point...
	GetOwner()->UpdateAimToTargetSamePoint( FALSE );
}


// Get the camera for the bot that owns this weapon. In the unlikely event
// that our owner is not a player, returns the current game camera.
CFCamera* CWeaponScope::_GetMyCamera( void ) {
	if( GetOwner() ) {
		s32 nPlayerIndex = GetOwner()->m_nPossessionPlayerIndex;

		if( nPlayerIndex >= 0 ) {
			return fcamera_GetCameraByIndex( nPlayerIndex );
		}
	}

	return gamecam_GetActiveCamera();
}


void CWeaponScope::PlayRejectSound( void ) {
	FASSERT( IsCreated() );

	PlaySound( m_pUserProps->pSoundGroupReject );
}


void CWeaponScope::EnterZoomMode( u32 uTargetZoomLevel/*=0*/ ) {
	const FViewport_t *pViewport;

	FASSERT( IsCreated() );

	switch( m_nZoomMode ) {
	case ZOOMMODE_DISABLED:
		// We're currently in bot-cam mode.
		// Start up the transition from bot-cam to zoom-cam...

		m_nZoomMode = ZOOMMODE_TRANSITIONING;
		FMATH_SETBITMASK( m_nScopeFlags, _SCOPEFLAG_TRANSITIONING_INTO_ZOOM );

		pViewport = _GetMyCamera()->GetViewport();
		m_fOrigTanHalfFOV = pViewport->fTanHalfFOVX;
		m_fOrigHalfFOV = pViewport->fHalfFOVX;

		m_fUnitZoomAnim = 0.0f;
		m_fUnitZoomAnimSpeed = m_pUserProps->fDigitalZoomSpeed;

		FMATH_CLAMP( uTargetZoomLevel, 0, m_pUserProps->nZoomLevelCount );
		m_nTargetZoomLevel = uTargetZoomLevel;

		m_fZoomAnimEndHalfFOV = fmath_Atan( fmath_Div( m_fOrigTanHalfFOV, m_pUserProps->afMag[m_nTargetZoomLevel] ), 1.0f );
		m_fZoomAnimStartHalfFOV = m_fOrigHalfFOV;

		m_fUnitJolt = 0.0f;

		PlaySound( m_pUserProps->pSoundGroupActivate );

		break;

	case ZOOMMODE_TRANSITIONING:
		if( m_nScopeFlags & _SCOPEFLAG_TRANSITIONING_OUT_OF_ZOOM ) {
			// We're currently transitioning from zoom-cam to bot-cam.
			// Let's reverse this...

			m_fUnitZoomAnim = 1.0f - m_fUnitZoomAnim;
			FMATH_FSWAP( m_fZoomAnimStartHalfFOV, m_fZoomAnimEndHalfFOV );

			FMATH_CLEARBITMASK( m_nScopeFlags, _SCOPEFLAG_TRANSITIONING_OUT_OF_ZOOM );
			FMATH_SETBITMASK( m_nScopeFlags, _SCOPEFLAG_TRANSITIONING_INTO_ZOOM );

			m_nTargetZoomLevel = 0;
			m_fZoomAnimStartHalfFOV = _GetMyCamera()->GetViewport()->fHalfFOVX;
			m_fZoomAnimEndHalfFOV = fmath_Atan( fmath_Div( m_fOrigTanHalfFOV, m_pUserProps->afMag[m_nTargetZoomLevel] ), 1.0f );

			PlaySound( m_pUserProps->pSoundGroupActivate );
		}

		break;

	};
}


void CWeaponScope::LeaveZoomMode( BOOL bImmediately ) {
	FASSERT( IsCreated() );

	m_nTargetZoomLevel = -1;

	if( bImmediately ) {
		// Snap out of zoom-cam...
		if( m_nScopeFlags & _SCOPEFLAG_USING_ZOOM_CAM ) {
			_SwitchToBotCam( m_fOrigHalfFOV );
		}

		m_nZoomMode = ZOOMMODE_DISABLED;
		m_nScopeFlags = _SCOPEFLAG_NONE;
		m_fUnitZoomAnim = 0.0f;
		m_fUnitZoomAnimSpeed = 0.0f;
		m_fZoomMag = 1.0f;

		return;
	}

	switch( m_nZoomMode ) {
	case ZOOMMODE_ENABLED:
		// We're currently in zoom-cam mode.
		// Start up the transition from zoom-cam to bot-cam...

		m_nZoomMode = ZOOMMODE_TRANSITIONING;
		FMATH_SETBITMASK( m_nScopeFlags, _SCOPEFLAG_TRANSITIONING_OUT_OF_ZOOM );

		m_fUnitZoomAnim = 0.0f;
		m_fUnitZoomAnimSpeed = m_pUserProps->fDigitalZoomSpeed;

		m_fZoomAnimStartHalfFOV = fmath_Atan( _GetMyCamera()->GetViewport()->fTanHalfFOVX, 1.0f );
		m_fZoomAnimEndHalfFOV = m_fOrigHalfFOV;

		PlaySound( m_pUserProps->pSoundGroupDeactivate );

		break;

	case ZOOMMODE_TRANSITIONING:
		if( m_nScopeFlags & _SCOPEFLAG_TRANSITIONING_INTO_ZOOM ) {
			// We're currently transitioning from bot-cam to zoom-cam.
			// Let's reverse this...

			m_fUnitZoomAnim = 1.0f - m_fUnitZoomAnim;
			FMATH_FSWAP( m_fZoomAnimStartHalfFOV, m_fZoomAnimEndHalfFOV );

			FMATH_CLEARBITMASK( m_nScopeFlags, _SCOPEFLAG_TRANSITIONING_INTO_ZOOM );
			FMATH_SETBITMASK( m_nScopeFlags, _SCOPEFLAG_TRANSITIONING_OUT_OF_ZOOM );

			m_fZoomAnimStartHalfFOV = fmath_Atan( _GetMyCamera()->GetViewport()->fTanHalfFOVX, 1.0f );
			m_fZoomAnimEndHalfFOV = m_fOrigHalfFOV;

			PlaySound( m_pUserProps->pSoundGroupDeactivate );
		}

		break;
	}
}


// Returns TRUE when the toggle resulted in leaving zoom mode.
BOOL CWeaponScope::DigitalZoom_ToggleZoomLevel( void ) {
	BOOL bLeavingZoomMode = FALSE;

	FASSERT( IsCreated() );

	switch( m_nZoomMode ) {
	case ZOOMMODE_DISABLED:
		EnterZoomMode();

		break;

	case ZOOMMODE_ENABLED:
		if( m_fUnitZoomAnimSpeed == 0.0f ) {
			// Not currently animating the zoom...

			FASSERT( m_nTargetZoomLevel != -1 );

			if( (u32)m_nTargetZoomLevel == (m_pUserProps->nZoomLevelCount - 1) ) {
				// We're at the max zoom level...

				LeaveZoomMode();
				bLeavingZoomMode = TRUE;
			} else {
				// We're not at the max zoom level...

				m_fUnitZoomAnim = 0.0f;
				m_fUnitZoomAnimSpeed = m_pUserProps->fDigitalZoomSpeed;
				++m_nTargetZoomLevel;

				m_fZoomAnimStartHalfFOV = _GetMyCamera()->GetViewport()->fHalfFOVX;
				m_fZoomAnimEndHalfFOV = fmath_Atan( fmath_Div( m_fOrigTanHalfFOV, m_pUserProps->afMag[m_nTargetZoomLevel] ), 1.0f );

				PlaySound( m_pUserProps->pSoundGroupZoomInOneLevel );
			}
		}

		break;
	};

	return bLeavingZoomMode;
}


void CWeaponScope::SetJolt( f32 fUnitJolt ) {
	FASSERT( IsCreated() );

	if( fUnitJolt > m_fUnitJolt ) {
		m_fUnitJolt = fUnitJolt;
	}
}


// Switches immediately to state CWeapon::CurrentState().
void CWeaponScope::ClassHierarchyResetToState( void ) {
	CWeapon::ClassHierarchyResetToState();

	LeaveZoomMode( TRUE );
}


// Note that ClassHierarchyResetToState() is always called prior to this call.
void CWeaponScope::ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel ) {
	FASSERT( !IsTransitionState( CurrentState() ) );

	// Set up new level...
	m_pUserProps = &m_aUserProps[ m_nUpgradeLevel ];
}

