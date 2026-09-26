//////////////////////////////////////////////////////////////////////////////////////
// edebris.cpp - Debris spawner class.
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
// 05/10/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "edebris.h"
#include "entity.h"
#include "fdebris.h"
#include "floop.h"




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEDebrisBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CEDebrisBuilder _EDebrisBuilder;


void CEDebrisBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CEBoxBuilder, ENTITY_BIT_DEBRIS, pszEntityType );

	u32 i;

	for( i=0; i<EDEBRIS_SPAWNER_COUNT; ++i ) {
		m_apDebrisGroup[i] = NULL;
	}

	m_nMinDebrisCount = 1;
	m_nMaxDebrisCount = 5;
	m_fMinDuration = 2.0f;
	m_fMaxDuration = 2.0f;
	m_fFadeOutSecs = 1.0f;
	m_fSpawnsPerSec = 5.0f;
	m_fMinSpeed = 0.0f;
	m_fMaxSpeed = 0.0f;
	m_fUnitDirSpread = 0.0f;
	m_fScaleMult = 1.0f;
	m_fGravityMult = 1.0f;
	m_fRotSpeedMult = 1.0f;
	m_fAutoShakeSecsMin = 0.0f;
	m_fAutoShakeSecsMin = 0.0f;
	m_bRumbleShake = TRUE;

	m_pszSound = NULL;
	m_fSoundUnitVolume = 1.0f;
	m_fSoundRadius = 100.0f;
	m_fSoundPitchMult = 1.0f;
	m_bSoundAttenuate = FALSE;
	m_bPlayImpactSound = FALSE;
	m_bPlayFlamingSound = FALSE;
}


BOOL CEDebrisBuilder::InterpretTable( void ) {
	cchar *pszString;
	f32 fVal, afMinMax[2];

	if( !fclib_stricmp( CEntityParser::m_pszTableName, "DebrisGroupSmall" ) ) {
		if( CEntityParser::Interpret_String( &pszString ) ) {
			// Valid string...

			m_apDebrisGroup[0] = CFDebrisGroup::Find( pszString );
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DebrisGroupMedium" ) ) {
		if( CEntityParser::Interpret_String( &pszString ) ) {
			// Valid string...

			m_apDebrisGroup[1] = CFDebrisGroup::Find( pszString );
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DebrisGroupLarge" ) ) {
		if( CEntityParser::Interpret_String( &pszString ) ) {
			// Valid string...

			m_apDebrisGroup[2] = CFDebrisGroup::Find( pszString );
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DebrisCountPerShake" ) ) {
		if( CEntityParser::m_nFieldCount == 1 ) {
			// Only one value specified. Use it for both min and max...

			CEntityParser::Interpret_F32( &fVal, 0.0f, 255.0f, TRUE );
			m_nMinDebrisCount = m_nMaxDebrisCount = (u32)(fVal + 0.1f);
		} else {
			// More than one value provided. Use it for a range...

			if( CEntityParser::Interpret_F32Array( afMinMax, 2, 0.0f, 255.0f, TRUE ) ) {
				m_nMinDebrisCount = (u32)(afMinMax[0] + 0.1f);
				m_nMaxDebrisCount = (u32)(afMinMax[1] + 0.1f);
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DebrisDuration" ) ) {
		if( CEntityParser::m_nFieldCount == 1 ) {
			// Only one value specified. Use it for both min and max...

			CEntityParser::Interpret_F32( &fVal, -10000.0f, 10000.0f, TRUE );
			m_fMinDuration = m_fMaxDuration = fVal;
		} else {
			// More than one value provided. Use it for a range...

			if( CEntityParser::Interpret_F32Array( afMinMax, 2, -10000.0f, 10000.0f, TRUE ) ) {
				m_fMinDuration = afMinMax[0];
				m_fMaxDuration = afMinMax[1];
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DebrisShakesPerSec" ) ) {
		CEntityParser::Interpret_F32( &m_fSpawnsPerSec, 0.01f, 1000.0f, TRUE );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DebrisFadeSecs" ) ) {
		CEntityParser::Interpret_F32( &m_fFadeOutSecs, 0.0f, 10000.0f, TRUE );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DebrisSpeed" ) ) {
		if( CEntityParser::m_nFieldCount == 1 ) {
			// Only one value specified. Use it for both min and max...

			CEntityParser::Interpret_F32( &fVal, -10000.0f, 10000.0f, TRUE );
			m_fMinSpeed = m_fMaxSpeed = fVal;
		} else {
			// More than one value provided. Use it for a range...

			if( CEntityParser::Interpret_F32Array( afMinMax, 2, -10000.0f, 10000.0f, TRUE ) ) {
				m_fMinSpeed = afMinMax[0];
				m_fMaxSpeed = afMinMax[1];
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DebrisDirSpread" ) ) {
		CEntityParser::Interpret_F32( &m_fUnitDirSpread, 0.0f, 1.0f, TRUE );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DebrisScaleMult" ) ) {
		CEntityParser::Interpret_F32( &m_fScaleMult, 0.01f, 1000.0f, TRUE );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DebrisGravityMult" ) ) {
		CEntityParser::Interpret_F32( &m_fGravityMult, 0.01f, 1000.0f, TRUE );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DebrisRotSpeedMult" ) ) {
		CEntityParser::Interpret_F32( &m_fRotSpeedMult, 0.01f, 1000.0f, TRUE );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DebrisAutoShakeSecs" ) ) {
		if( CEntityParser::m_nFieldCount == 1 ) {
			// Only one value specified. Use it for both min and max...

			CEntityParser::Interpret_F32( &fVal, 0.0f, 1000000000.0f, TRUE );
			m_fAutoShakeSecsMin = m_fAutoShakeSecsMax = fVal;
		} else {
			// More than one value provided. Use it for a range...

			if( CEntityParser::Interpret_F32Array( afMinMax, 2, 0.0f, 1000000000.0f, TRUE ) ) {
				m_fAutoShakeSecsMin = afMinMax[0];
				m_fAutoShakeSecsMax = afMinMax[1];
			}
		}

		if( m_fAutoShakeSecsMin==0.0f || m_fAutoShakeSecsMax==0.0f ) {
			m_fAutoShakeSecsMin = 0.0f;
			m_fAutoShakeSecsMax = 0.0f;
		} else if( m_fAutoShakeSecsMin > m_fAutoShakeSecsMax ) {
			FMATH_FSWAP( m_fAutoShakeSecsMin, m_fAutoShakeSecsMax );
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DebrisRumbleShake" ) ) {
		CEntityParser::Interpret_BOOL( &m_bRumbleShake );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DebrisSound" ) ) {
		CEntityParser::Interpret_String( &m_pszSound, TRUE, 0, TRUE );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DebrisSoundVolume" ) ) {
		CEntityParser::Interpret_F32( &m_fSoundUnitVolume, 0.0f, 1.0f, TRUE );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DebrisSoundRadius" ) ) {
		CEntityParser::Interpret_F32( &m_fSoundRadius, 0.01f, 10000.0f, TRUE );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DebrisSoundPitch" ) ) {
		CEntityParser::Interpret_F32( &m_fSoundPitchMult, 0.01f, 100.0f, TRUE );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DebrisSoundAtten" ) ) {
		CEntityParser::Interpret_BOOL( &m_bSoundAttenuate );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DebrisPlayImpactSound" ) ) {
		CEntityParser::Interpret_BOOL( &m_bPlayImpactSound );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DebrisPlayFlamingSound" ) ) {
		CEntityParser::Interpret_BOOL( &m_bPlayFlamingSound );
		return TRUE;

	}

	return CEBoxBuilder::InterpretTable();
}


BOOL CEDebrisBuilder::PostInterpretFixup( void ) {
	if( !CEBoxBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	return TRUE;

_ExitWithError:
	return FALSE;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEDebris
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CEDebris::m_bDebrisSystemInitialized;
CArmorProfile CEDebris::m_ArmorProfileRumble;


BOOL CEDebris::InitSystem( void ) {
	FASSERT( !IsDebrisSystemInitialized() );

	m_ArmorProfileRumble.m_pszName = "RumbleOnly";
	m_ArmorProfileRumble.m_afUnitProtection[DAMAGE_HITPOINT_TYPE_PROJECTILE] = 1.0f;
	m_ArmorProfileRumble.m_afUnitProtection[DAMAGE_HITPOINT_TYPE_BLAST] = 1.0f;
	m_ArmorProfileRumble.m_afUnitProtection[DAMAGE_HITPOINT_TYPE_FLAME] = 1.0f;
	m_ArmorProfileRumble.m_fUnitProtectionFromImpulse = 1.0f;
	m_ArmorProfileRumble.m_fUnitProtectionFromRumble = 0.0f;
	m_ArmorProfileRumble.m_fMinBlastHitpointFilter = 0.0f;

	m_bDebrisSystemInitialized = TRUE;

	return TRUE;
}


void CEDebris::UninitSystem( void ) {
	m_bDebrisSystemInitialized = FALSE;
}


CEDebris::CEDebris() {
	FASSERT( IsDebrisSystemInitialized() );

	_SetDefaults();
}


CEDebris::~CEDebris() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


CEntityBuilder *CEDebris::GetLeafClassBuilder( void ) {
	return &_EDebrisBuilder;
}


BOOL CEDebris::ClassHierarchyBuild( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	BOOL bPointEmitter = FALSE;
	u32 i;

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CEDebrisBuilder *pBuilder = (CEDebrisBuilder *)GetLeafClassBuilder();

	pBuilder->m_HalfDim_MS.z = 0.0f;
	if( pBuilder->m_HalfDim_MS.x==0.0f || pBuilder->m_HalfDim_MS.y==0.0f ) {
		pBuilder->m_HalfDim_MS.Zero();
		bPointEmitter = TRUE;
	}

	// Build parent class...
	if( !CEBox::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Initialize from builder object...

	_SetDefaults();

	for( i=0; i<EDEBRIS_SPAWNER_COUNT; ++i ) {
		m_aDebrisSpawner[i].InitToDefaults();

		m_aDebrisSpawner[i].m_Mtx = m_MtxToWorld;
		m_aDebrisSpawner[i].m_fPlaneDimX = 2.0f * m_HalfDim_MS.x;
		m_aDebrisSpawner[i].m_fPlaneDimY = 2.0f * m_HalfDim_MS.y;

		m_aDebrisSpawner[i].m_pDebrisGroup = pBuilder->m_apDebrisGroup[i];
		m_aDebrisSpawner[i].m_nMinDebrisCount = pBuilder->m_nMinDebrisCount;
		m_aDebrisSpawner[i].m_nMaxDebrisCount = pBuilder->m_nMaxDebrisCount;
		m_aDebrisSpawner[i].m_fSpawnerFadeSecs = pBuilder->m_fFadeOutSecs;
		m_aDebrisSpawner[i].m_fSecsBetweenSpawns = fmath_Inv( pBuilder->m_fSpawnsPerSec );
		m_aDebrisSpawner[i].m_fMinSpeed = pBuilder->m_fMinSpeed;
		m_aDebrisSpawner[i].m_fMaxSpeed = pBuilder->m_fMaxSpeed;
		m_aDebrisSpawner[i].m_fUnitDirSpread = pBuilder->m_fUnitDirSpread;
		m_aDebrisSpawner[i].m_fScaleMul = pBuilder->m_fScaleMult;
		m_aDebrisSpawner[i].m_fGravityMul = pBuilder->m_fGravityMult;
		m_aDebrisSpawner[i].m_fRotSpeedMul = pBuilder->m_fRotSpeedMult;
		m_aDebrisSpawner[i].m_fSpawnerAliveSecs = 0.0f;

		if( bPointEmitter ) {
			m_aDebrisSpawner[i].m_nEmitterType = CFDebrisSpawner::EMITTER_TYPE_POINT;
		} else {
			m_aDebrisSpawner[i].m_nEmitterType = CFDebrisSpawner::EMITTER_TYPE_BOUNDED_PLANE;
		}

		if( pBuilder->m_bPlayImpactSound ) {
			FMATH_SETBITMASK( m_aDebrisSpawner[i].m_nFlags, CFDebrisSpawner::FLAG_PLAY_IMPACT_SOUND );
		}

		if( pBuilder->m_bPlayFlamingSound ) {
			FMATH_SETBITMASK( m_aDebrisSpawner[i].m_nFlags, CFDebrisSpawner::FLAG_PLAY_FLAMING_SOUND );
		}
	}

	m_fGravityMult = pBuilder->m_fGravityMult;
	m_fAutoShakeSecsMin = pBuilder->m_fAutoShakeSecsMin;
	m_fAutoShakeSecsMax = pBuilder->m_fAutoShakeSecsMax;

	m_fMinDebrisCount = (f32)pBuilder->m_nMinDebrisCount;
	m_fMaxDebrisCount = (f32)pBuilder->m_nMaxDebrisCount;

	m_fMinDuration = pBuilder->m_fMinDuration;
	m_fMaxDuration = pBuilder->m_fMaxDuration;

	FMATH_WRITEBIT( bPointEmitter, m_nFlags, FLAG_POINT_EMITTER );

	SetArmorProfile( pBuilder->m_bRumbleShake ? &m_ArmorProfileRumble : &CDamage::m_ArmorProfileInfinite );

	m_hSound = fsndfx_GetFxHandle( pBuilder->m_pszSound );
	m_fSoundUnitVolume = pBuilder->m_fSoundUnitVolume;
	m_fSoundRadius = pBuilder->m_fSoundRadius;
	m_fSoundPitchMult = pBuilder->m_fSoundPitchMult;

	if( pBuilder->m_bSoundAttenuate ) {
		FMATH_SETBITMASK( m_nFlags, FLAG_ATTENUATE_SOUND );
	}

	// Success...

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


BOOL CEDebris::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CEBox::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	_UpdateOurWorkBit();

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CEDebris::ClassHierarchyRelocated( void *pIdentifier ) {
	FASSERT( IsCreated() );

	CEBox::ClassHierarchyRelocated( pIdentifier );

	if( !IsInWorld() ) {
		// We're not in the world...
		return;
	}
}


void CEDebris::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );

	CEBox::ClassHierarchyAddToWorld();
}


void CEDebris::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );

	CEBox::ClassHierarchyRemoveFromWorld();

	Kill( TRUE );
}


void CEDebris::Kill( BOOL bKillSound ) {
	u32 i;

	if( CFDebrisSpawner::IsSpawnerSystemInitialized() ) {
		for( i=0; i<EDEBRIS_SPAWNER_COUNT; ++i ) {
			if( m_aDebrisSpawner[i].m_pDebrisGroup ) {
				if( m_aDebrisSpawner[i].IsSpawning() ) {
					m_aDebrisSpawner[i].Kill();
				}
			}
		}
	}

	if( bKillSound ) {
		if( m_pAudioEmitter ) {
			m_pAudioEmitter->Destroy();
			m_pAudioEmitter = NULL;
		}
	}
}


void CEDebris::InflictDamageResult( const CDamageResult *pDamageResult ) {
	CEBox::InflictDamageResult( pDamageResult );

	if( pDamageResult->m_pDamageData->m_fRumbleUnitIntensity <= 0.0f ) {
		return;
	}

	if( GetArmorProfile()->m_fUnitProtectionFromRumble >= 1.0f ) {
		return;
	}

	if( pDamageResult->m_pDamageData->m_nDamageLocale != CDamageForm::DAMAGE_LOCALE_BLAST ) {
		return;
	}

	CFVec3A EpicenterToPlaneCenter_WS;
	f32 fDistFromEpicenterToPlane_WS, fUnitRumbleIntensity;

	EpicenterToPlaneCenter_WS.Sub( m_MtxToWorld.m_vPos, *pDamageResult->m_pDamageData->m_pEpicenter_WS );
	fDistFromEpicenterToPlane_WS = fmath_Abs( EpicenterToPlaneCenter_WS.Dot( m_MtxToWorld.m_vZ ) );

	fUnitRumbleIntensity = CDamage::ComputeRadialIntensity_Rumble( fDistFromEpicenterToPlane_WS, pDamageResult->m_pDamageData->m_pDamageProfile, pDamageResult->m_pDamageData->m_fDamageFormOrigNormIntensity );

	fUnitRumbleIntensity *= (1.0f - GetArmorProfile()->m_fUnitProtectionFromRumble);

	if( fUnitRumbleIntensity <= 0.0f ) {
		return;
	}

	fUnitRumbleIntensity *= 4.0f;
	FMATH_CLAMPMAX( fUnitRumbleIntensity, 1.0f );

	Shake( fUnitRumbleIntensity );
}


void CEDebris::Shake( f32 fUnitRumbleIntensity ) {
	static const f32 __afGroupDebrisCountMult[EDEBRIS_SPAWNER_COUNT] = { 4.0f, 1.0f, 0.15f };
	static const f32 __afGroupGravityMult[EDEBRIS_SPAWNER_COUNT] = { 0.5f, 1.0f, 1.5f };

	FASSERT( IsCreated() );

	u32 i;
	f32 fDebrisCountMult;

	Kill( TRUE );

	for( i=0; i<EDEBRIS_SPAWNER_COUNT; ++i ) {
		if( m_aDebrisSpawner[i].m_pDebrisGroup ) {
			fDebrisCountMult = fUnitRumbleIntensity * __afGroupDebrisCountMult[i];

			m_aDebrisSpawner[i].m_fGravityMul = m_fGravityMult * __afGroupGravityMult[i];

			m_aDebrisSpawner[i].m_nMinDebrisCount = (u8)fmath_FloatToU32( m_fMinDebrisCount * fDebrisCountMult );
			m_aDebrisSpawner[i].m_nMaxDebrisCount = (u8)fmath_FloatToU32( m_fMaxDebrisCount * fDebrisCountMult );

			m_aDebrisSpawner[i].m_fSpawnerAliveSecs = fmath_RandomFloatRange( m_fMinDuration, m_fMaxDuration );

			CGColl::SpawnDebris( &m_aDebrisSpawner[i], FALSE, FALSE );
		}
	}

	if( m_hSound != FSNDFX_INVALID_FX_HANDLE ) {
		m_pAudioEmitter = FSNDFX_ALLOCNPLAY3D( m_hSound, &MtxToWorld()->m_vPos, m_fSoundRadius, m_fSoundUnitVolume, m_fSoundPitchMult, FAudio_EmitterDefaultPriorityLevel, TRUE );
	}
}


void CEDebris::_SetDefaults( void ) {
	m_nFlags = FLAG_NONE;
	m_fGravityMult = 1.0f;
	m_fMinDebrisCount = 1.0f;
	m_fMaxDebrisCount = 1.0f;
	m_fMinDuration = 1.0f;
	m_fMaxDuration = 1.0f;

	m_fAutoShakeSecsMin = 0.0f;
	m_fAutoShakeSecsMax = 0.0f;
	m_fAutoShakeTimer = 0.0f;

	m_hSound = FSNDFX_INVALID_FX_HANDLE;
	m_fSoundUnitVolume = 1.0f;
	m_fSoundRadius = 100.0f;
	m_fSoundPitchMult = 1.0f;

	m_pAudioEmitter = NULL;
}


void CEDebris::EnableRumbleShake( BOOL bEnable ) {
	FASSERT( IsCreated() );

	SetArmorProfile( bEnable ? &m_ArmorProfileRumble : &CDamage::m_ArmorProfileInfinite );
}


void CEDebris::SetAutoShake( f32 fMinTime, f32 fMaxTime ) {
	FASSERT( IsCreated() );

	if( fMinTime==0.0f || fMaxTime==0.0f ) {
		fMinTime = 0.0f;
		fMaxTime = 0.0f;
	} else if( fMinTime > fMaxTime ) {
		FMATH_FSWAP( fMinTime, fMaxTime );
	}

	m_fAutoShakeSecsMin = fMinTime;
	m_fAutoShakeSecsMax = fMaxTime;
	FMATH_CLAMPMAX( m_fAutoShakeTimer, fMaxTime );

	_UpdateOurWorkBit();
}


void CEDebris::ClassHierarchyWork( void ) {
	_UpdateOurWorkBit();

	if( m_fAutoShakeSecsMin > 0.0f ) {
		m_fAutoShakeTimer -= FLoop_fPreviousLoopSecs;

		if( m_fAutoShakeTimer <= 0.0f ) {
			m_fAutoShakeTimer = fmath_RandomFloatRange( m_fAutoShakeSecsMin, m_fAutoShakeSecsMax );

			Shake( 1.0f );
		}
	}

	if( (m_pAudioEmitter == NULL) || !(m_nFlags & FLAG_ATTENUATE_SOUND) ) {
		return;
	}

	u32 i;

	for( i=0; i<EDEBRIS_SPAWNER_COUNT; ++i ) {
		if( m_aDebrisSpawner[i].m_pDebrisGroup ) {
			break;
		}
	}

	if( i == EDEBRIS_SPAWNER_COUNT ) {
		DisableOurWorkBit();
		return;
	}

	f32 fDebrisIntensity = m_aDebrisSpawner[i].GetUnitIntensity();

	m_pAudioEmitter->SetVolume( m_fSoundUnitVolume * fDebrisIntensity );

	if( fDebrisIntensity == 0.0f ) {
		m_pAudioEmitter->Destroy();
		m_pAudioEmitter = NULL;
	}
}

void CEDebris::ClassHierarchyBecameActive( void ) {
	_UpdateOurWorkBit();
}


void CEDebris::_UpdateOurWorkBit( void ) {
	if( (m_pAudioEmitter && (m_nFlags & FLAG_ATTENUATE_SOUND)) || (m_fAutoShakeSecsMin > 0.0f) ) {
		EnableOurWorkBit();
	} else {
		DisableOurWorkBit();
	}
}


void CEDebris::ClassHierarchyBecameInactive( void ) {
	DisableOurWorkBit();
}


void CEDebris::CheckpointSaveSelect( s32 nCheckpoint ) {
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}


BOOL CEDebris::CheckpointSave( void ) {
	CEBox::CheckpointSave();

	CFCheckPoint::SaveData( m_fAutoShakeSecsMin );
	CFCheckPoint::SaveData( m_fAutoShakeSecsMax );

	return TRUE;
}


void CEDebris::CheckpointRestore( void ) {
	CEBox::CheckpointRestore();

	CFCheckPoint::LoadData( m_fAutoShakeSecsMin );
	CFCheckPoint::LoadData( m_fAutoShakeSecsMax );

	m_fAutoShakeTimer = 0.0f;

	if( m_fAutoShakeSecsMin > 0.0f ) {
		EnableOurWorkBit();
	} else {
		DisableOurWorkBit();
	}
}


