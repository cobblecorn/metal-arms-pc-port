//////////////////////////////////////////////////////////////////////////////////////
// ZipLine.cpp - Zip line entity
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
// 09/30/02 Miller      Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "bot.h"
#include "fsound.h"
#include "ZipLine.h"
#include "flinklist.h"
#include "fdraw.h"
#include "frenderer.h"
#include "floop.h"
#include "meshtypes.h"
#include "fclib.h"
#include "fresload.h"

#define _INTENSITY_FALLOFF_MAX_TIME	   ( 2.0f )
#define _OO_INTENSITY_FALLOFF_MAX_TIME ( 1.0f / _INTENSITY_FALLOFF_MAX_TIME )

static CEZipLineBuilder _EZipLineBuilder;

void CEZipLineBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CEntityBuilder, ENTITY_BIT_ZIPLINE, pszEntityType );

	// Set our builder defaults.
	// Every data member of our builder MUST be initialized!
	m_End1_WS.Zero();
	m_End2_WS.Zero();

	m_fUnitGive = 0.0f;
	m_fUnitFriction = 1.0f;
	m_fAccel = 1.0f;

	m_bVisible = TRUE;
	m_bOpaque = TRUE;

	m_ColorBiasRGBA.Set( 1.0f, 1.0f, 1.0f, 1.0f );

	m_szTextureName = NULL;
	m_fTextureRepeatPerFoot = 0.5f;//0.01f;

	m_szParticleName = "zipspark2";
}

BOOL CEZipLineBuilder::InterpretTable( void )
{
	if( !fclib_stricmp( CEntityParser::m_pszTableName, "Give" ) ) {
		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			CEntityParser::Interpret_F32( &m_fUnitGive, 0.0f, 1.0f );	
		}

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Friction" ) ) {
		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			CEntityParser::Interpret_F32( &m_fUnitFriction, 0.0f, 1.0f );	
			m_fUnitFriction = 1.0f - m_fUnitFriction;
		}

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Accel" ) ) {
		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			CEntityParser::Interpret_F32( &m_fAccel, 1.0f, 2.0f );	
		}

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Visible" ) ) {
		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			u32 uValue;

			if (CEntityParser::Interpret_U32( &uValue, 0, 1 )) {
				if (uValue != 1 && uValue != 0) {
					CEntityParser::Error_InvalidParameterValue();
				} else {
					m_bVisible = uValue == 1 ? TRUE : FALSE;
				}
			}
		}

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Opaque" ) ) {
		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			u32 uValue;

			if (CEntityParser::Interpret_U32( &uValue, 0, 1 )) {
				if (uValue != 1 && uValue != 0) {
					CEntityParser::Error_InvalidParameterValue();
				} else {
					m_bOpaque = uValue == 1 ? TRUE : FALSE;
				}
			}
		}

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Texture" ) ) {
		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			CEntityParser::Interpret_String( &m_szTextureName );
		}
		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "TextureRepeat" ) ) {
		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			CEntityParser::Interpret_F32( &m_fTextureRepeatPerFoot, 0.0f, 1.0f );	
		}

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "ColorBias" ) ) {
		if( CEntityParser::m_nFieldCount != 4 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			u32 i;
			FGameData_VarType_e nVarType;
			f32 fColors[4];

			for( i = 0; i < 4; ++i ) {
				fColors[i] = *(const f32 *)fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, i, nVarType );

				if( nVarType != FGAMEDATA_VAR_TYPE_FLOAT ) {
					CEntityParser::Error_InvalidParameterType();
					break;
				}

				if( fColors[i] < 0.0f || fColors[i] > 255.0f ) {
					CEntityParser::Error_InvalidParameterType();
					break;
				}

				fColors[i] /= 255.0f;
			}

			if (i == 4) {
				m_ColorBiasRGBA.Set(fColors[0], fColors[1], fColors[2], fColors[3]);
			}
		}
		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Particle" ) ) {
		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			CEntityParser::Interpret_String( &m_szParticleName );
		}
		return TRUE;
	} 

	return CEntityBuilder::InterpretTable();
}

BOOL CEZipLineBuilder::PostInterpretFixup( void ) {
	if( !CEntityBuilder::PostInterpretFixup() ) {
		return FALSE;
	}
 
	const CFWorldShapeInit *pWorldShapeInit = CEntityParser::m_pWorldShapeInit;

	switch( pWorldShapeInit->m_nShapeType ) {

		case FWORLD_SHAPETYPE_SPLINE:
			if( pWorldShapeInit->m_pSpline->m_nPointCount != 2 ) {
				DEVPRINTF("CEZipLineBuilder::PostInterpretFixup: Too many spline points in zipline!\n");	
				return FALSE;
			}

			m_End1_WS.Set( pWorldShapeInit->m_pSpline->m_pPtArray[0] );
			m_End2_WS.Set( pWorldShapeInit->m_pSpline->m_pPtArray[1] );
		break;
		default:
			DEVPRINTF("CEZipLineBuilder::PostInterpretFixup: Did not get a spline world shape!\n");
			return FALSE;
		break;
	}

	return TRUE;
}

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEZipLine
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************
BOOL CEZipLine::m_bSystemInitialized = FALSE;
FLinkRoot_t CEZipLine::m_LinkRoot;
FDrawVtx_t CEZipLine::m_aDrawVtx[MAX_VTX_COUNT];
u32 CEZipLine::m_nClassClientCount = 0;

CFSoundGroup *CEZipLine::m_pAttachSound = NULL;
CFSoundGroup *CEZipLine::m_pDetachSound = NULL;
CFSoundGroup *CEZipLine::m_pZippingSound = NULL;
CFSoundGroup *CEZipLine::m_pBrakeSound = NULL;

static const CFSphere *_CollisionSphere = NULL;
static CEZipLine *_pZipLineHit = NULL;

#define _ZIPLINE_SOUNDS		"Zipline"
#define _ZIPLINE_GRAB		"zipgrab"
#define _ZIPLINE_RELEASE	"zipoff"
#define _ZIPLINE_ZIPPING	"zipping"
#define _ZIPLINE_BRAKING	"ZipBrake"

#define _MIN_BRAKE_VOL		0.25f
#define _MAX_BRAKE_VOL		1.00f


CEZipLine::CEZipLine() {
	_Init();
}

// !!Don't touch
CEZipLine::~CEZipLine() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}

BOOL CEZipLine::InitSystem( void ) {
	u32 i;

	FASSERT( !IsSystemInitialized() );

	m_bSystemInitialized = TRUE;

	flinklist_InitRoot( &m_LinkRoot, FANG_OFFSETOF( CEZipLine, m_Link ) );

	for( i=0; i<MAX_VTX_COUNT; i++ ) {
		m_aDrawVtx[i].ST.y = 0.5f;
	}

	return TRUE;
}


void CEZipLine::UninitSystem( void ) {
	if( IsSystemInitialized() ) {
		m_bSystemInitialized = FALSE;
	}
}

void CEZipLine::Destroy( void ) {

}

CEZipLine *CEZipLine::GetZipCollision( const CFSphere &Sphere )
{
	// Don't trace through the world if we don't have any zip lines
	if( _GetHead() == NULL) {
		return NULL;
	}

	CFWorldUser UserTracker;

	_CollisionSphere = &Sphere;

	UserTracker.MoveTracker( Sphere );
	BOOL bFound = !UserTracker.FindIntersectingTrackers( CEZipLine::_CollisionCallback, FWORLD_TRACKERTYPE_USER );
	UserTracker.RemoveFromWorld();

	return bFound ? _pZipLineHit : NULL;
}

// Assumes the FDraw renderer is currently active.
// Assumes the the 3D game camera and viewport
// are set up.
// Assumes there are no model Xfms on the stack.
//
// Does not preserve the current FDraw state.
// Will preserve the viewport.
// Will preserve the Xfm stack.
// Will preserve the frenderer fog state.
void CEZipLine::DrawAll( void ) {
	BOOL bDrewOne;
	CEZipLine *pCable;
	f32 fLineWidth, fPrevLineWidth=0.0f;

	FASSERT( IsSystemInitialized() );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	bDrewOne = FALSE;
	
	for( pCable=_GetHead(); pCable; pCable = _GetNext( pCable ) ) {
		FASSERT( pCable->IsCreated() );

		if( pCable->IsEnabled() && pCable->IsVisible() ) {
			if( !bDrewOne ) {
				bDrewOne = TRUE;

				frenderer_SetDefaultState();

				fdraw_Depth_EnableWriting( TRUE );
				fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER );
			}

			fLineWidth = pCable->_GetLineWidth();
			
			if (fLineWidth != fPrevLineWidth)
			{
				#if FANG_PLATFORM_GC
					fdraw_SetLineWidth( fLineWidth+0.5f );
				#else
					fdraw_SetLineWidth( fLineWidth+0.01f );
				#endif
				
				fPrevLineWidth = fLineWidth;
			}

			pCable->_Draw();
		}
	}
	
	fdraw_SetLineWidth( 1.0f );
}


void CEZipLine::WorkAll( void ) {
	CEZipLine *pCable;

	FASSERT( IsSystemInitialized() );

	for( pCable=_GetHead(); pCable; pCable = _GetNext( pCable ) ) {
		if( pCable->IsEnabled() && pCable->m_nAttachedHookCount ) {
			pCable->_Work();
		}
	}
}

BOOL CEZipLine::ClassHierarchyLoadSharedResources( void ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( m_nClassClientCount != 0xffffffff );

	++m_nClassClientCount;

	if( !CEntity::ClassHierarchyLoadSharedResources() ) {
		// Bail now since parent class has already called
		// ClassHierarchyUnloadSharedResources() and decremented
		// our client counter...

		return FALSE;
	}

	if( m_nClassClientCount > 1 ) {
		return TRUE;
	}

	// Resources not yet loaded...
	FResFrame_t ResFrame = fres_GetFrame();

	if( !fresload_Load( FSNDFX_RESTYPE, _ZIPLINE_SOUNDS ) ) {
		DEVPRINTF( "CEZipLine::ClassHierarchyLoadSharedResources(): Could not load sound effect bank '%s'\n", _ZIPLINE_SOUNDS );
	}

	m_pAttachSound = CFSoundGroup::RegisterGroup( _ZIPLINE_GRAB );
	m_pDetachSound = CFSoundGroup::RegisterGroup( _ZIPLINE_RELEASE );
	m_pZippingSound = CFSoundGroup::RegisterGroup( _ZIPLINE_ZIPPING );
	m_pBrakeSound = CFSoundGroup::RegisterGroup( _ZIPLINE_BRAKING );

	return TRUE;
}

void CEZipLine::ClassHierarchyUnloadSharedResources( void ) {
	FASSERT( m_nClassClientCount > 0 );

	--m_nClassClientCount;

	if( m_nClassClientCount == 0 ) {
		m_pAttachSound = NULL;
		m_pDetachSound = NULL;
		m_pZippingSound = NULL;
		m_pBrakeSound = NULL;
	}

	CEntity::ClassHierarchyUnloadSharedResources();
}

void CEZipLine::ClassHierarchyDestroy( void ) {
	_Destroy();
	_Init();

	CEntity::ClassHierarchyDestroy();
}

BOOL CEZipLine::ClassHierarchyBuild( void ) {
	CEZipLineBuilder *pBuilder = (CEZipLineBuilder *) GetLeafClassBuilder();

	// Build our parent entity class...
	if( !CEntity::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	m_fUnitGive = pBuilder->m_fUnitGive;
	m_fTextureRepeatPerFoot = pBuilder->m_fTextureRepeatPerFoot;
	m_fUnitFriction = pBuilder->m_fUnitFriction;
	m_fAccel = pBuilder->m_fAccel;

	m_ColorRGBA.Set(pBuilder->m_ColorBiasRGBA);

	_SetEndPoints( &pBuilder->m_End1_WS, &pBuilder->m_End2_WS );

	m_UnitVec12XZ.x = m_Mtx.m_vFront.x;
	m_UnitVec12XZ.y = m_Mtx.m_vFront.z;
	m_UnitVec12XZ.Unitize();

	if( pBuilder->m_szTextureName != NULL) {
		FTexDef_t *pTexDef = (FTexDef_t *) fresload_Load( FTEX_RESNAME, pBuilder->m_szTextureName );

		if (pTexDef == NULL) {
			DEVPRINTF( "CEZipLine::Create(): Could not find texture '%s'.\n", pBuilder->m_szTextureName );
			//goto _ExitWithError;
		}

		m_TexInst.SetTexDef(pTexDef);
	}

	if( pBuilder->m_szParticleName != NULL) {
		m_hSparkParticleDef = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, pBuilder->m_szParticleName );
		
		if( m_hSparkParticleDef == FPARTICLE_INVALID_HANDLE ) {
			DEVPRINTF( "CEZipLine::ClassHierarchyBuild(): Could not find particle definition '%s'.\n", pBuilder->m_szParticleName );
		}
	}

	if( pBuilder->m_bVisible ) {
		FMATH_SETBITMASK( m_nFlags, FLAG_VISIBLE );
	}

	if( pBuilder->m_bOpaque ) {
		FMATH_SETBITMASK( m_nFlags, FLAG_OPAQUE );
	}

	m_nAttachedHookCount = 0;
	
	_AddZipLine();

	m_pWorldTracker = fnew CFWorldTracker(FWORLD_TRACKERTYPE_USER);

	if( m_pWorldTracker == NULL ) {
		DEVPRINTF( "CEZipLine::ClassHierarchyBuild(): Not enough memory to allocate m_pWorldTracker.\n" );
		goto _ExitWithError;
	}

	m_nFlags |= (FLAG_ENABLE);

	return TRUE;

_ExitWithError:	
	// Call CEntity::Destroy(). This will, in turn, call the ClassHierarchyDestroy()
	// chain so that our entire object has been destroyed...
	Destroy();

	return FALSE;
}

CEntityBuilder *CEZipLine::GetLeafClassBuilder( void ) {
	// Do this and nothing more...
	return &_EZipLineBuilder;
}

void CEZipLine::ClassHierarchyAddToWorld( void ) {
	CFSphere BoundSphere;

	BoundSphere.BuildLooseTriangleBound(m_Mtx.m_vPos.v3, m_End2_WS.v3, m_End2_WS.v3);

	m_pWorldTracker->SetUserTypeBits( TypeBits() );
	m_pWorldTracker->SetCollisionFlag( TRUE );
	m_pWorldTracker->SetLineOfSightFlag( TRUE );
	m_pWorldTracker->MoveTracker( BoundSphere );
	m_pWorldTracker->m_nUser = MESHTYPES_ENTITY;
	m_pWorldTracker->m_pUser = this;
	m_pWorldTracker->AddToWorld();
}

void CEZipLine::ClassHierarchyRemoveFromWorld( void ) {
	if( m_pWorldTracker ) {
		m_pWorldTracker->RemoveFromWorld();
	}

	CEntity::ClassHierarchyRemoveFromWorld();
}

// Returns NULL if too many hooks on the cable. The caller
// should not error out or FASSERT, but should handle this
// gracefully!
CZipLineHook *CEZipLine::AttachHook( const CFVec3A *pInitialPos_WS, const CFVec3A* pVelocity_WS, CBot *pBot ) {
	CFVec2 VecXZ;
	CZipLineHook *pHook;
	u32 i;
	f32 fDeltaY, fDistToCableXZ, fPrevHookUnitDist, fNextHookUnitDist, fPrevHookY, fNextHookY, fUnitInterp;

	FASSERT( IsCreated() );

	if( m_nAttachedHookCount >= MAX_HOOK_COUNT ) {
		return NULL;
	}

	// Find an empty hook...
	for( i=0, pHook=m_aAttachedHook; i<MAX_HOOK_COUNT; i++, pHook++ ) {
		if( !pHook->m_bUsedSlot ) {
			// Found an empty slot...
			break;
		}
	}
	FASSERT( i < MAX_HOOK_COUNT );
	FASSERT( pHook->m_hSparkEmitter == FPARTICLE_INVALID_HANDLE );

	pHook->m_bUsedSlot = TRUE;
	pHook->m_bAttached = TRUE;
	pHook->m_bTakingSlack = TRUE;
	pHook->m_bBrakeStartThisFrame = FALSE;
	pHook->m_pCable = this;
	pHook->m_fInputBrakeAmount = 0.0f;
	pHook->m_fGripSlowDownTotal = 0.0f;
	pHook->m_fSparkIntensity = 0.0f;
	pHook->m_hSparkEmitter = FPARTICLE_INVALID_HANDLE;
	pHook->m_fYawSnapOffset = 0.0f;

	m_apAttachedHook[ m_nAttachedHookCount++ ] = pHook;

	pHook->m_Vel_WS.Zero();
	pHook->m_fSpeed_WS = -1.0f;	// Flag our hook!
	//pHook->m_fUnitLoadMass = fUnitLoadMass;
	//pHook->m_fUnitFriction = fUnitFriction;

	// Center inital pos on cable in XZ space...
	VecXZ.x = pInitialPos_WS->x - m_Mtx.m_vPos.x;
	VecXZ.y = pInitialPos_WS->z - m_Mtx.m_vPos.z;
	fDistToCableXZ = m_Mtx.m_vRight.x*VecXZ.x + m_Mtx.m_vRight.z*VecXZ.y;
	pHook->m_Pos_WS.Mul( m_Mtx.m_vRight, -fDistToCableXZ ).Add( *pInitialPos_WS );

	// Compute unit distance along cable...
	VecXZ.x = pHook->m_Pos_WS.x - m_Mtx.m_vPos.x;
	VecXZ.y = pHook->m_Pos_WS.z - m_Mtx.m_vPos.z;
	pHook->m_fUnitDist12 = VecXZ.Dot( m_UnitVec12XZ ) * m_fOOLengthXZ;
	if( pHook->m_fUnitDist12 < 0.0f ) {
		pHook->m_fUnitDist12 = 0.0f;
		pHook->m_Pos_WS = m_Mtx.m_vPos;
		_SortHooks();
	} else if( pHook->m_fUnitDist12 > 1.0f ) {
		pHook->m_fUnitDist12 = 1.0f;
		pHook->m_Pos_WS = m_End2_WS;
		_SortHooks();
	} else {
		if( m_nAttachedHookCount == 1 ) {
			pHook->m_Pos_WS.y = FMATH_FPOT( pHook->m_fUnitDist12, m_Mtx.m_vPos.y, m_End2_WS.y );
		} else {
			_SortHooks();

			for( i=0; i<m_nAttachedHookCount; i++ ) {
				pHook = m_apAttachedHook[i];

				if( pHook->m_fSpeed_WS < 0.0f ) {
					// We found our hook...
					pHook->m_fSpeed_WS = 0.0f;
					break;
				}
			}
			FASSERT( i < m_nAttachedHookCount );

			if( i == 0 ) {
				// We're the first hook...
				fPrevHookUnitDist = 0.0f;
				fNextHookUnitDist = m_apAttachedHook[1]->m_fUnitDist12;

				fPrevHookY = m_Mtx.m_vPos.y;
				fNextHookY = m_apAttachedHook[1]->m_Pos_WS.y;
			} else if( i == (m_nAttachedHookCount - 1) ) {
				// We're the last hook...
				fPrevHookUnitDist = m_apAttachedHook[i-1]->m_fUnitDist12;
				fNextHookUnitDist = 1.0f;

				fPrevHookY = m_apAttachedHook[i-1]->m_Pos_WS.y;
				fNextHookY = m_End2_WS.y;
			} else {
				// We're somewhere in between...
				fPrevHookUnitDist = m_apAttachedHook[i-1]->m_fUnitDist12;
				fNextHookUnitDist = m_apAttachedHook[i+1]->m_fUnitDist12;

				fPrevHookY = m_apAttachedHook[i-1]->m_Pos_WS.y;
				fNextHookY = m_apAttachedHook[i+1]->m_Pos_WS.y;
			}

			// Compute interpolator...
			fDeltaY = fNextHookUnitDist - fPrevHookUnitDist;
			if( FMATH_FABS( fDeltaY ) > 0.0001 ) {
				fUnitInterp = fmath_Div( pHook->m_fUnitDist12 - fPrevHookUnitDist,fNextHookUnitDist - fPrevHookUnitDist );
				FMATH_CLAMP( fUnitInterp, 0.0f, 1.0f );

				// Compute our Y position by interpolating our neighboring hook Y positions...
				pHook->m_Pos_WS.y = FMATH_FPOT( fUnitInterp, fPrevHookY, fNextHookY );
			} else {
				pHook->m_Pos_WS.y = fPrevHookY;
			}
		}
	}

	_RecomputeHookVecs();

	// NKM - Keep some of our initial velocity
	pHook->m_fSpeed_WS = pVelocity_WS->Dot( pHook->m_UnitVecToNext ) * 0.5f;
	pHook->m_Vel_WS.Set( pHook->m_UnitVecToNext );
	pHook->m_Vel_WS.Mul( pHook->m_fSpeed_WS );

	// Play the attach sound
	pBot->PlaySound( GetAttachSound() );

	_KillHookEmitter( pHook );
	_KillBrakeAudioEmitter( pHook );

	pHook->m_pZipEmitter = pBot->AllocAndPlaySound( GetZippingSound() );
	pHook->m_pAudioEmitterBrake = pBot->AllocAndPlaySound( GetBrakeSound() );

	if( pHook->m_pZipEmitter ) {
		pHook->m_pZipEmitter->SetVolume( 0.0f );
	}

	if( pHook->m_pAudioEmitterBrake ) {
		pHook->m_pAudioEmitterBrake->SetVolume( 0.0f );
	}

	return pHook;
}


void CEZipLine::DetachHook( CZipLineHook *pCableHook, CBot *pBot, BOOL bPlayDetachSound ) {
	FASSERT( IsCreated() );
	FASSERT( pCableHook );
	FASSERT( pBot );

	u32 i;
	CZipLineHook *pHook;

	if( !pCableHook->m_bUsedSlot ) {
		return;
	}

	if( m_nAttachedHookCount == 0 ) {
		return;
	}

	pCableHook->m_bUsedSlot = FALSE;

	if( m_hSparkParticleDef != FPARTICLE_INVALID_HANDLE && pCableHook->m_hSparkEmitter != FPARTICLE_INVALID_HANDLE ) {
		fparticle_StopEmitter( pCableHook->m_hSparkEmitter );
		pCableHook->m_hSparkEmitter = FPARTICLE_INVALID_HANDLE;
	}

	if( bPlayDetachSound ) {
		// Play the detach sound
		pBot->PlaySound( GetDetachSound() );
	}

	_KillHookEmitter( pCableHook );
	_KillBrakeAudioEmitter( pCableHook );

	if( m_nAttachedHookCount == 1 ) {
		m_nAttachedHookCount = 0;
	}

	for( i=0; i<m_nAttachedHookCount; i++ ) {
		pHook = m_apAttachedHook[i];
		if( pHook == pCableHook ) {
			break;
		}
	}
	if( i == m_nAttachedHookCount ) {
		return;
	}

	if( i == (m_nAttachedHookCount-1) ) {
		m_nAttachedHookCount--;
	} else {
		m_apAttachedHook[i] = m_apAttachedHook[ --m_nAttachedHookCount ];
	}

	_SortHooks();
	_RecomputeHookVecs();
}

// Use draw verts?  Will take away if statements
BOOL CEZipLine::SphereIntersect( const CFSphere &Sphere) {
	CFVec3A *pEnd1, *pEnd2, *pFront;

	if( m_nAttachedHookCount ) {
		u32 nCnt = m_nAttachedHookCount + 2, i;
		u32 nNdxStart;

		for( i = 1; i < nCnt; ++i ) {
			
			nNdxStart = i - 1;
			
			if( !nNdxStart ) {
				// Start point and the first hook
				pEnd1 = &m_Mtx.m_vPos;
				pEnd2 = &m_apAttachedHook[i - 1]->m_Pos_WS;
				pFront = &m_UnitVecToFirstHook;
			} else if( i == nCnt - 1 ) {
				// Last hook and the end point
				pEnd1 = &m_apAttachedHook[m_nAttachedHookCount - 1]->m_Pos_WS;
				pEnd2 = &m_End2_WS;
				pFront = &m_apAttachedHook[m_nAttachedHookCount - 1]->m_UnitVecToNext;
			} else {
				// Between two hooks
				pEnd1 = &m_apAttachedHook[i - 2]->m_Pos_WS;
				pEnd2 = &m_apAttachedHook[i - 1]->m_Pos_WS;
				pFront = &m_apAttachedHook[i - 2]->m_UnitVecToNext;
			}

			if( Sphere.IsIntersecting( (*pEnd1).v3, (*pEnd2).v3 ) ) {
				return _SphereBelowLine( Sphere, *pEnd1, *pFront );
			}
		}

		return FALSE;
	} 

	pEnd1 = &m_Mtx.m_vPos;
	pEnd2 = &m_End2_WS;
	pFront = &m_Mtx.m_vFront;

	if( Sphere.IsIntersecting( (*pEnd1).v3, (*pEnd2).v3 ) ) {
		return _SphereBelowLine( Sphere, *pEnd1, *pFront );
	}

	return FALSE;
}

void CEZipLine::_Init( void ) {
	m_nFlags = 0;
	m_pWorldTracker = 0;

/*	u32 i;

	for( i = 0; i < MAX_HOOK_COUNT; ++i ) {
		m_aAttachedHook[i].m_bAttached = 
			m_aAttachedHook[i].m_bUsedSlot = 
			m_aAttachedHook[i].m_bTakingSlack = 
			m_aAttachedHook[i].m_bBrakeStartThisFrame = FALSE;
		m_aAttachedHook[i].m_hSparkEmitter = FPARTICLE_INVALID_HANDLE;

		m_apAttachedHook[i] = NULL;
	}*/
	CZipLineHook *pHook = NULL;

	for( u32 i = 0; i < MAX_HOOK_COUNT; ++i ) {
		pHook = &m_aAttachedHook[i];
	
		pHook->m_Vel_WS.Zero();
		pHook->m_Pos_WS.Zero();
		pHook->m_Pos_Draw_WS.Zero();
		pHook->m_UnitVecToNext.Zero();
		
		pHook->m_fSpeed_WS = 0.0f;
		pHook->m_fUnitDist12 = 0.0f;
		pHook->m_fUnitLoadMass = 0.0f;
		pHook->m_fUnitFriction = 0.0f;
		pHook->m_fInputBrakeAmount = 0.0f;
		pHook->m_fGripSlowDownTotal = 0.0f;
		pHook->m_fSparkIntensity = 0.0f;
		pHook->m_fYawSnapOffset = 0.0f;

		pHook->m_bAttached = FALSE;
		pHook->m_bUsedSlot = FALSE;
		pHook->m_bTakingSlack = FALSE;
		pHook->m_bBrakeStartThisFrame = FALSE;

		pHook->m_pCable = NULL;
		pHook->m_hSparkEmitter = NULL;
		pHook->m_pZipEmitter = NULL;
		pHook->m_pAudioEmitterBrake = NULL;
	}

	m_hSparkParticleDef = FPARTICLE_INVALID_HANDLE;
}

void CEZipLine::_Destroy( void ) {
	fdelete( m_pWorldTracker );

	_RemoveZipLine();
}

void CEZipLine::_SetEndPoints( const CFVec3A *pEnd1_WS, const CFVec3A *pEnd2_WS ) {
	CFVec2 VecXZ;

	m_Mtx.m_vPos = *pEnd1_WS;
	m_End2_WS = *pEnd2_WS;

	m_Mtx.m_vFront.Sub( m_End2_WS, m_Mtx.m_vPos );
	m_fLength = m_Mtx.m_vFront.Mag();
	m_Mtx.m_vFront.Unitize();

	m_Mtx.m_vUp = CFVec3A::m_UnitAxisY;
	m_Mtx.m_vRight.Cross( m_Mtx.m_vUp, m_Mtx.m_vFront );

	VecXZ.x = m_End2_WS.x - m_Mtx.m_vPos.x;
	VecXZ.y = m_End2_WS.z - m_Mtx.m_vPos.z;

	m_fOOLengthXZ = VecXZ.InvMag();

	m_aDrawVtx[0].Pos_MS = m_Mtx.m_vPos.v3;

	m_UnitVecToFirstHook = m_Mtx.m_vFront;
	
	m_BoundingSphere.m_Pos = m_Mtx.m_vPos.v3 + m_End2_WS.v3;
	m_BoundingSphere.m_Pos = m_BoundingSphere.m_Pos*0.5f;
	
	m_BoundingSphere.m_fRadius = ( m_End2_WS.v3 - m_BoundingSphere.m_Pos ).Mag();
}

void CEZipLine::_Draw( void ) {
	u32 i, nVtx, nVtxCount;

	if( m_TexInst.GetTexDef() ) {
		// Texture provided...
		#if FANG_PLATFORM_GC
			fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AT );
		#else
			fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AI );
		#endif
		fdraw_SetTexture( &m_TexInst );
	} else {
		// No texture provided...
		fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	}

	if( m_nFlags & FLAG_OPAQUE && 0 ) {
		fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_SRC );
	} else {
		fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	}
	
	f32 fY = 0.5f;
	#if FANG_PLATFORM_GC
	fdraw_EnableClipping(TRUE);
	#endif
	
	nVtxCount = m_nAttachedHookCount + 2;

	for( i=0, nVtx=1; i<m_nAttachedHookCount; i++, nVtx++ ) {
		m_aDrawVtx[nVtx].ColorRGBA = m_ColorRGBA;
		m_aDrawVtx[nVtx].ST.x = m_apAttachedHook[i]->m_fUnitDist12 * m_fLength * m_fTextureRepeatPerFoot;

		m_aDrawVtx[nVtx].ST.y = fY;
		m_aDrawVtx[nVtx].Pos_MS = m_apAttachedHook[i]->m_Pos_Draw_WS.v3;
	}

	m_aDrawVtx[0].ColorRGBA = m_ColorRGBA;
	m_aDrawVtx[0].ST.x = 0.0f;
	m_aDrawVtx[0].ST.y = fY;
	m_aDrawVtx[0].Pos_MS = m_Mtx.m_vPos.v3;

	m_aDrawVtx[nVtxCount-1].ColorRGBA = m_ColorRGBA;
	m_aDrawVtx[nVtxCount-1].ST.x = m_fLength * m_fTextureRepeatPerFoot;
	m_aDrawVtx[nVtxCount-1].ST.y = fY;
	m_aDrawVtx[nVtxCount-1].Pos_MS = m_End2_WS.v3;

	fdraw_PrimList( FDRAW_PRIMTYPE_LINESTRIP, m_aDrawVtx, nVtxCount );
}

f32 CEZipLine::_GetLineWidth( void ) {
	f32 fRet = 3.0f, fScale, fScaleFactor=0.01f;
	
	CFVec3 vOffs = m_BoundingSphere.m_Pos - FXFM_CAM_ORIG_WS.v3;
	f32 fDist = vOffs.Mag2();
	
	if (fDist > FMATH_POS_EPSILON) {
		fDist = fmath_Sqrt(fDist);

		if (fDist > m_BoundingSphere.m_fRadius+0.1f) {
			fDist -= (m_BoundingSphere.m_fRadius);
			fScale = fDist * fScaleFactor;
			fScale = (fScale < 1.0f)?(fScale):(1.0f);
			
			fRet *= (1.0f - fScale);
			
			if (fRet < 1.0f) { 
				fRet = 1.0f; 
			}
		}
	}
	
	return (fRet);
}

//#define _HAND_BRAKE_SLOWDOWN_RATE 3.0f
#define _ZIP_LINE_OO_MAX_SPEED ( 1.0f / 50.0f )

void CEZipLine::_Work( void ) {
	u32 i;
	CZipLineHook *pHook;
	CFVec2 VecXZ;
	CFVec3A Delta_WS, Delta_Brake;
	f32 fTargetY, fPrevGuysHookY, fAvgHookY;

	// This is wrong, we want the attached hooks, not the hook array
	// This was the cause of the stop on a zip line when another bot got off problem
	//for( i=0, pHook=m_aAttachedHook; i<m_nAttachedHookCount; i++, pHook++ ) { 
	for( i=0; i < m_nAttachedHookCount; ++i) {
		pHook = m_apAttachedHook[i];

		// Move position based on velocity computed during last frame...
		Delta_WS.Mul( pHook->m_Vel_WS, FLoop_fPreviousLoopSecs );
		pHook->m_Pos_WS.Add( Delta_WS );
		
		pHook->m_bBrakeStartThisFrame = FALSE;

		if( pHook->m_fInputBrakeAmount != 0.0f ) {
			// This gives a gradual accumulation of brake
			//pHook->m_fGripSlowDownTotal += (FLoop_fPreviousLoopSecs * pHook->m_fInputBrakeAmount * _HAND_BRAKE_SLOWDOWN_RATE);

			if( pHook->m_pAudioEmitterBrake ) {

				if( pHook->m_pAudioEmitterBrake->Is3D() ) {
					pHook->m_pAudioEmitterBrake->SetPosition( &pHook->m_Pos_WS );
				}

				pHook->m_pAudioEmitterBrake->SetVolume( FMATH_FPOT( pHook->m_fInputBrakeAmount, _MIN_BRAKE_VOL, _MAX_BRAKE_VOL ) );
			}

			pHook->m_fGripSlowDownTotal = pHook->m_fInputBrakeAmount;
			FMATH_CLAMPMAX( pHook->m_fGripSlowDownTotal, 1.0f );

			// We just started to brake, so spawn the particles
			if( m_hSparkParticleDef != FPARTICLE_INVALID_HANDLE ) {
				// There is a spark particle def...

				if( pHook->m_hSparkEmitter == FPARTICLE_INVALID_HANDLE ) {
					CFVec3A NegVelUnit;
					CFVec3A Right;
					CFVec3A Up;

					NegVelUnit = pHook->m_UnitVecToNext;
					NegVelUnit.Negate();
					Right.Cross( NegVelUnit, CFVec3A::m_UnitAxisY );
					Up.Cross( Right, NegVelUnit );
					Up.Mul( 0.5f );

					NegVelUnit.Add( Up );
					if( NegVelUnit.MagSq() > FMATH_POS_EPSILON ) {
						NegVelUnit.Unitize();
					} else {
						NegVelUnit = CFVec3A::m_UnitAxisY;
					}

					pHook->m_hSparkEmitter = fparticle_SpawnEmitter( 
						m_hSparkParticleDef, 
						&pHook->m_Pos_Draw_WS.v3, 
						&NegVelUnit.v3,
						&pHook->m_Vel_WS.v3, 
						pHook->m_fInputBrakeAmount);

					fparticle_SetIntensity( pHook->m_hSparkEmitter, &pHook->m_fSparkIntensity );

					// This is done so that we will know when we started to brake when we re-compute vels
					pHook->m_fInputBrakeAmount = -pHook->m_fInputBrakeAmount;	
					pHook->m_bBrakeStartThisFrame = TRUE;
				} else {
					// Particle effect already spawned. Just update it...
					fparticle_SetIntensity( pHook->m_hSparkEmitter, pHook->m_fInputBrakeAmount );
				}
			}
		} else {
			if( pHook->m_pAudioEmitterBrake ) {
				pHook->m_pAudioEmitterBrake->SetVolume( 0.0f );
			}

			// No more brake, no more particles
			pHook->m_fGripSlowDownTotal = 0.0f;
			if( m_hSparkParticleDef != FPARTICLE_INVALID_HANDLE && pHook->m_hSparkEmitter != FPARTICLE_INVALID_HANDLE ) {
				fparticle_StopEmitter( pHook->m_hSparkEmitter );
				pHook->m_hSparkEmitter = FPARTICLE_INVALID_HANDLE;
			}
		}

		// Compute the unit distance along the cable...
		VecXZ.x = pHook->m_Pos_WS.x - m_Mtx.m_vPos.x;
		VecXZ.y = pHook->m_Pos_WS.z - m_Mtx.m_vPos.z;

		if( VecXZ.Mag2() > FMATH_POS_EPSILON ) {
			pHook->m_fUnitDist12 = VecXZ.Mag() * m_fOOLengthXZ;
		} else {
			pHook->m_fUnitDist12 = 0.0f;
		}

		if( pHook->m_fUnitDist12 < 0.0f ) {
			pHook->m_bAttached = FALSE;
		} else if( pHook->m_fUnitDist12 > 1.0f ) {
			pHook->m_bAttached = FALSE;
		}

		// Don't let the hook position get out of the start and end point area
		if( pHook->m_bAttached ) {
			f32 fD = -m_Mtx.m_vFront.Dot( m_Mtx.m_vPos );

			if( pHook->m_Pos_WS.Dot( m_Mtx.m_vFront ) + fD < 0.0f ) {
				pHook->m_bAttached = FALSE;
			} else {
				CFVec3A Front = m_Mtx.m_vFront;

				Front.Mul( -1.0f );

				fD = -Front.Dot( m_End2_WS );

				if( pHook->m_Pos_WS.Dot( Front ) + fD < 0.0f ) {
					pHook->m_bAttached = FALSE;
				}
			}
		}

		if( !pHook->m_bAttached ) {
			_KillHookEmitter( pHook );
			_KillBrakeAudioEmitter( pHook );
		}

		if( pHook->m_pZipEmitter ) {
			f32 fVol;

			fVol =  FMATH_FABS( pHook->m_fSpeed_WS ) * _ZIP_LINE_OO_MAX_SPEED;
			fVol += pHook->m_fSparkIntensity;
			FMATH_CLAMPMAX( fVol, 1.0f );

			pHook->m_pZipEmitter->SetVolume( fVol );
			
			// Emitter can be 2D or 3D
			if( pHook->m_pZipEmitter->Is3D() ) {
				pHook->m_pZipEmitter->SetPosition( &pHook->m_Pos_WS );
			}
		}

		// Compute the ideal Y position...
		fTargetY = FMATH_FPOT( pHook->m_fUnitDist12, m_Mtx.m_vPos.y, m_End2_WS.y );
		fTargetY -= 50.0f * m_fUnitGive * fmath_Sin( FMATH_PI * pHook->m_fUnitDist12 );

		if( !pHook->m_bTakingSlack ) {
			pHook->m_Pos_WS.y = fTargetY;
		} else {
			if( pHook->m_Pos_WS.y > fTargetY ) {
				// We're too high...
				pHook->m_Pos_WS.y -= 25.0f * FLoop_fPreviousLoopSecs;
			} else {
				pHook->m_bTakingSlack = FALSE;
				pHook->m_Pos_WS.y = fTargetY;
			}
		}
	}

	_SortHooks();
	_RecomputeHookVecs();

	f32 fVelFrac;

	// Compute new velocities...
	fPrevGuysHookY = m_UnitVecToFirstHook.y;
	for( i=0; i<m_nAttachedHookCount; i++ ) {
		pHook = m_apAttachedHook[i];
		fAvgHookY = (pHook->m_UnitVecToNext.y + fPrevGuysHookY) * 0.5f;

		Delta_WS.Mul( pHook->m_UnitVecToNext, fAvgHookY * FLoop_fPreviousLoopSecs * -64.0f * m_fUnitFriction * m_fAccel );
		
		// Apply the handbrake
		Delta_Brake.Set( pHook->m_Vel_WS );
		Delta_Brake.Mul( -FLoop_fPreviousLoopSecs * pHook->m_fGripSlowDownTotal );

		// Just started to brake
		if( pHook->m_fInputBrakeAmount < 0.0f ) {
			if( pHook->m_Vel_WS.MagSq() > FMATH_POS_EPSILON ) {
				fVelFrac = ( pHook->m_Vel_WS.Mag() * _ZIP_LINE_OO_MAX_SPEED );
			} else {
				fVelFrac = 0.0f;
			}

			if( pHook->m_fGripSlowDownTotal > fVelFrac && fVelFrac <= 0.3f ) {
				pHook->m_fSparkIntensity = fVelFrac;
			} else {
				pHook->m_fSparkIntensity = fVelFrac + pHook->m_fGripSlowDownTotal;
			}

			FMATH_CLAMP( pHook->m_fSparkIntensity, 0.0f, 1.0f );

			// Put things back
			pHook->m_fInputBrakeAmount = -pHook->m_fInputBrakeAmount;
		} else {
			pHook->m_fSparkIntensity -= ( FLoop_fPreviousLoopSecs * _OO_INTENSITY_FALLOFF_MAX_TIME );
			FMATH_CLAMP( pHook->m_fSparkIntensity, 0.0f, 1.0f );
		}

		pHook->m_Vel_WS.Add( Delta_WS );
		pHook->m_Vel_WS.Add( Delta_Brake );

		if( pHook->m_Vel_WS.MagSq() > FMATH_POS_EPSILON ) {
			pHook->m_fSpeed_WS = pHook->m_Vel_WS.Mag();
		} else {
			pHook->m_fSpeed_WS = 0.0f;
		}

		fPrevGuysHookY = pHook->m_UnitVecToNext.y;
	}
}

void CEZipLine::_SortHooks( void ) {
	// Sort the hooks from closest-to-end1 to closest-to-end2...
	if( m_nAttachedHookCount > 1 ) {
		if( m_nAttachedHookCount == 2 ) {
			// Simple swap...

			if( m_apAttachedHook[0]->m_fUnitDist12 > m_apAttachedHook[1]->m_fUnitDist12 ) {
				CZipLineHook *pSwap = m_apAttachedHook[0];
				m_apAttachedHook[0] = m_apAttachedHook[1];
				m_apAttachedHook[1] = pSwap;
			}
		} else {
			fclib_QSort( m_apAttachedHook, m_nAttachedHookCount, sizeof(CZipLineHook *), _HookSortCallback );
		}
	}
}


void CEZipLine::_RecomputeHookVecs( void ) {
	CZipLineHook *pHook;
	f32 fMag2;
	u32 i, j, nValidCount;
	BOOL bFirstVecComputedOk, abVecToNextComputedOk[MAX_HOOK_COUNT];
	CFVec3A *pLastValidUnitVec;

	if( m_nAttachedHookCount == 0 ) {
		m_UnitVecToFirstHook = m_Mtx.m_vFront;
		return;
	}

	// Recompute unit vectors to next hooks toward end2...
	nValidCount = 0;
	for( i=0; i<(m_nAttachedHookCount-1); i++ ) {
		pHook = m_apAttachedHook[i];

		pHook->m_UnitVecToNext.Sub( m_apAttachedHook[i+1]->m_Pos_WS, pHook->m_Pos_WS );

		fMag2 = pHook->m_UnitVecToNext.MagSq();
		if( fMag2 > 0.0001f ) {
			pHook->m_UnitVecToNext.Div( fmath_Sqrt(fMag2) );
			abVecToNextComputedOk[i] = TRUE;
			nValidCount++;
		} else {
			abVecToNextComputedOk[i] = FALSE;
		}
	}

	// Recompute unit vector for last hook...
	pHook = m_apAttachedHook[ m_nAttachedHookCount - 1 ];
	pHook->m_UnitVecToNext.Sub( m_End2_WS, pHook->m_Pos_WS );
	fMag2 = pHook->m_UnitVecToNext.MagSq();
	if( fMag2 > 0.0001f ) {
		pHook->m_UnitVecToNext.Div( fmath_Sqrt(fMag2) );
		abVecToNextComputedOk[ m_nAttachedHookCount - 1 ] = TRUE;
		nValidCount++;
	} else {
		abVecToNextComputedOk[ m_nAttachedHookCount - 1 ] = FALSE;
	}

	if( nValidCount == 0 ) {
		for( i=0, pHook=m_aAttachedHook; i<m_nAttachedHookCount; i++, pHook++ ) {
			pHook->m_UnitVecToNext = m_Mtx.m_vFront;
		}

		m_UnitVecToFirstHook = m_Mtx.m_vFront;

		return;
	}

	// Recompute initial vector...
	m_UnitVecToFirstHook.Sub( m_apAttachedHook[0]->m_Pos_WS, m_Mtx.m_vPos );
	fMag2 = m_UnitVecToFirstHook.MagSq();
	if( fMag2 > 0.0001f ) {
		m_UnitVecToFirstHook.Div( fmath_Sqrt(fMag2) );
		bFirstVecComputedOk = TRUE;
	} else {
		bFirstVecComputedOk = FALSE;
	}

	pLastValidUnitVec = &m_UnitVecToFirstHook;
	for( i=0; i<m_nAttachedHookCount; i++ ) {
		pHook = m_apAttachedHook[i];

		if( abVecToNextComputedOk[i] ) {
			pLastValidUnitVec = &pHook->m_UnitVecToNext;
		} else {
			for( j=i; j<m_nAttachedHookCount; j++ ) {
				if( abVecToNextComputedOk[j] ) {
					pHook->m_UnitVecToNext = m_apAttachedHook[j]->m_UnitVecToNext;
					pLastValidUnitVec = &pHook->m_UnitVecToNext;
					break;
				}
			}
			if( j == m_nAttachedHookCount ) {
				pHook->m_UnitVecToNext = *pLastValidUnitVec;
			}
		}
	}

	if( !bFirstVecComputedOk ) {
		m_UnitVecToFirstHook = m_apAttachedHook[0]->m_UnitVecToNext;
	}
}


void CEZipLine::_KillHookEmitter( CZipLineHook *pHook ) {
	FASSERT( pHook );

	if( pHook->m_pZipEmitter ) {
		pHook->m_pZipEmitter->Destroy();
		pHook->m_pZipEmitter = NULL;
	}
}


void CEZipLine::_KillBrakeAudioEmitter( CZipLineHook *pHook ) {
	FASSERT( pHook );

	if( pHook->m_pAudioEmitterBrake ) {
		pHook->m_pAudioEmitterBrake->Destroy();
		pHook->m_pAudioEmitterBrake = NULL;
	}
}


int CEZipLine::_HookSortCallback( const void *pElement1, const void *pElement2 ) {
	CZipLineHook *pHook1, *pHook2;

	pHook1 = (CZipLineHook *)pElement1;
	pHook2 = (CZipLineHook *)pElement2;

	if( pHook1->m_fUnitDist12 < pHook2->m_fUnitDist12 ) {
		return -1;
	}

	if( pHook1->m_fUnitDist12 > pHook2->m_fUnitDist12 ) {
		return 1;
	}

	return 0;
}

BOOL CEZipLine::_CollisionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume )
{
	CEZipLine *pHitZip;

	if( pTracker->m_nUser != MESHTYPES_ENTITY ) {
		// Not an entity...
		return TRUE;
	}

	if( !(((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_ZIPLINE) ) {
		// Not a zipline...
		return TRUE;
	}

	pHitZip = (CEZipLine *) pTracker->m_pUser;

	if( !pHitZip->IsCreated() ) {
		// this zip is not created yet
		return TRUE;
	}

	// not visible
	if( !pHitZip->IsVisible() ) {
		return TRUE;
	}

	if( !pHitZip->IsEnabled() ) {
		return TRUE;
	}

	if( pHitZip->SphereIntersect(*_CollisionSphere)) {
		_pZipLineHit = pHitZip;
		return FALSE;
	}

	return TRUE;
}