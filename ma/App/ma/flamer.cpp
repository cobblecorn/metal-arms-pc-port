//////////////////////////////////////////////////////////////////////////////////////
// flamer.cpp - Flaming stream.
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
// 09/11/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "flamer.h"
#include "flinklist.h"
#include "fresload.h"
#include "fres.h"
#include "frenderer.h"
#include "fdraw.h"
#include "floop.h"
#include "fvtxpool.h"
#include "pspool.h"
#include "fcamera.h"
#include "gamecam.h"
#include "fxfm.h"
#include "smoketrail.h"
#include "fparticle.h"
#include "eparticle.h"
#include "fworld_coll.h"
#include "meshtypes.h"
#include "weapon.h"
#include "fsndfx.h"
#include "eparticlepool.h"
#include "AI\AIEnviro.h"
#include "bot.h"



#define _TEXNAME_MAIN				"TWDMflame05"
#define _TEXNAME_PARTICLE_1			"TWDMflame02"
#define _TEXNAME_PARTICLE_2			"TWDMflame03"
#define _TEXNAME_PARTICLE_3			"TWDMflame04"

#define _PARTICLE_DEF_NAME			"flame1"
#define _PARTICLE_SMOKE_DEF_NAME	"smoke1"

#define _SECS_BETWEEN_NODE_SPAWNS	0.059f
#define _INITIAL_STREAM_SPEED		150.0f
#define _INITIAL_STREAM_WIDTH		0.1f
#define _VTX_POOL_COUNT				500
#define _HORIZ_TC_WRAP_COUNT		2.0f

#define _SOUND_FREQ_MULTIPLIER		0.6f



BOOL CFlamer::m_bSystemInitialized;
FLinkRoot_t CFlamer::m_LinkRoot;
CFTexInst CFlamer::m_MainTexDef;
CFTexInst CFlamer::m_ParticleTexInst;
CFTexInst CFlamer::m_aParticleTexDef[_FLAME_PARTICLE_TYPE_COUNT];
BOOL CFlamer::m_bCanDraw;
FDrawVtx_t *CFlamer::m_pVtxArray;
FDrawVtx_t *CFlamer::m_pNextVtx;
const CFXfm *CFlamer::m_pCamXfm;
CFVec3A CFlamer::m_aCornerUnitVec[_CROSS_SECTION_CORNER_COUNT+1];
f32 CFlamer::m_afCornerTC[_CROSS_SECTION_CORNER_COUNT+1];
CFVec3A *CFlamer::m_pNextCornerUnitVec;
SmokeTrailAttrib_t CFlamer::m_SmokeTrailAttrib;
FPSprite_t *CFlamer::m_pPSpriteArray;
FParticle_DefHandle_t CFlamer::m_hParticleDef;
FParticle_DefHandle_t CFlamer::m_hParticleSmokeDef;


cchar *CFlamer::m_apszParticleTexName[_FLAME_PARTICLE_TYPE_COUNT] = {
	_TEXNAME_PARTICLE_1,
	_TEXNAME_PARTICLE_2,
	_TEXNAME_PARTICLE_3,
};



BOOL CFlamer::InitSystem( void ) {
	FTexDef_t *pTexDef;
	u32 i;
	f32 fAngle, fSin, fCos;

	FASSERT( !m_bSystemInitialized );

	m_bCanDraw = TRUE;

	// Load main texture...
	pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, _TEXNAME_MAIN );
	if( pTexDef == NULL ) {
		DEVPRINTF( "CFlamer::InitSystem(): Could not find texture '%s'. Flamer won't be drawn.\n", _TEXNAME_MAIN );
		m_bCanDraw = FALSE;
	}
	m_MainTexDef.SetTexDef( pTexDef );
	m_MainTexDef.SetFlags( CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T );

	// Load particle textures...
	for( i=0; i<_FLAME_PARTICLE_TYPE_COUNT; ++i ) {
		pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, m_apszParticleTexName[i] );
		if( pTexDef == NULL ) {
			DEVPRINTF( "CFlamer::InitSystem(): Could not find texture '%s'. Flamer won't be drawn.\n", m_apszParticleTexName[i] );
			m_bCanDraw = FALSE;
		}
		m_aParticleTexDef[i].SetTexDef( pTexDef );
		m_aParticleTexDef[i].SetFlags( CFTexInst::FLAG_NONE );
	}

	// Load particle texture...
	pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, _TEXNAME_PARTICLE_1 );
	if( pTexDef == NULL ) {
		DEVPRINTF( "CFlamer::InitSystem(): Could not find texture '%s'.\n", _TEXNAME_PARTICLE_1 );
	}
	m_ParticleTexInst.SetTexDef( pTexDef );

	flinklist_InitRoot( &m_LinkRoot, (s32)FANG_OFFSETOF( CFlamer, m_Link ) );

	for( i=0, fAngle=(FMATH_PI / (f32)_CROSS_SECTION_CORNER_COUNT); i<_CROSS_SECTION_CORNER_COUNT; ++i, fAngle+=(FMATH_2PI / (f32)_CROSS_SECTION_CORNER_COUNT) ) {
		fmath_SinCos( fAngle, &fSin, &fCos );
		m_aCornerUnitVec[i].Set( fSin, fCos, 0.0f );
		m_afCornerTC[i] = _HORIZ_TC_WRAP_COUNT * (f32)i * (1.0f / (f32)_CROSS_SECTION_CORNER_COUNT);
	}
	m_aCornerUnitVec[i] = m_aCornerUnitVec[0];
	m_afCornerTC[i] = _HORIZ_TC_WRAP_COUNT;

	// Load particle def file...
	m_hParticleDef = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, _PARTICLE_DEF_NAME );
	if( m_hParticleDef == FPARTICLE_INVALID_HANDLE ) {
		DEVPRINTF( "CFlamer::InitSystem(): Could not find particle definition '%s'.\n", _PARTICLE_DEF_NAME );
	}

	m_hParticleSmokeDef = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, _PARTICLE_SMOKE_DEF_NAME );
	if( m_hParticleSmokeDef == FPARTICLE_INVALID_HANDLE ) {
		DEVPRINTF( "CFlamer::InitSystem(): Could not find particle smoke definition '%s'.\n", _PARTICLE_SMOKE_DEF_NAME );
	}

	_SetSmokeTrailAttributes();

	m_pVtxArray = NULL;
	m_pPSpriteArray = NULL;

	m_bSystemInitialized = TRUE;

	return TRUE;
}


void CFlamer::UninitSystem( void ) {
	if( m_bSystemInitialized ) {
		m_bSystemInitialized = FALSE;
	}
}


void CFlamer::_SetSmokeTrailAttributes( void ) {
	FTexDef_t *pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, "tfp1smoke01" );
	if( pTexDef == NULL ) {
		DEVPRINTF( "CWeaponRocket::_SetSmokeTrailAttributes(): Could not load smoke trail texture.\n" );
	}

	m_SmokeTrailAttrib.nFlags = SMOKETRAIL_FLAG_NONE;
	m_SmokeTrailAttrib.pTexDef = pTexDef;

	m_SmokeTrailAttrib.fScaleMin_WS = 5.0f;
	m_SmokeTrailAttrib.fScaleMax_WS = 7.0f;
	m_SmokeTrailAttrib.fScaleSpeedMin_WS = 1.2f;
	m_SmokeTrailAttrib.fScaleSpeedMax_WS = 1.5f;
	m_SmokeTrailAttrib.fScaleAccelMin_WS = -1.0f;
	m_SmokeTrailAttrib.fScaleAccelMax_WS = -1.5f;

	m_SmokeTrailAttrib.fXRandSpread_WS = 2.5f;
	m_SmokeTrailAttrib.fYRandSpread_WS = 2.5f;
	m_SmokeTrailAttrib.fDistBetweenPuffs_WS = 0.6f;
	m_SmokeTrailAttrib.fDistBetweenPuffsRandSpread_WS = 0.2f;

	m_SmokeTrailAttrib.fYSpeedMin_WS = 25.0f;
	m_SmokeTrailAttrib.fYSpeedMax_WS = 27.0f;
	m_SmokeTrailAttrib.fYAccelMin_WS = -0.002f;
	m_SmokeTrailAttrib.fYAccelMax_WS = -0.005f;

	m_SmokeTrailAttrib.fUnitOpaqueMin_WS = 0.3f;
	m_SmokeTrailAttrib.fUnitOpaqueMax_WS = 0.5f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMin_WS = -1.2f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMax_WS = -1.6f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMin_WS = 0.0f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMax_WS = 0.0f;

	m_SmokeTrailAttrib.StartColorRGB.Set( 1.0f, 0.5f, 0.25f );
	m_SmokeTrailAttrib.EndColorRGB.White();
	m_SmokeTrailAttrib.fStartColorUnitIntensityMin = 1.0f;
	m_SmokeTrailAttrib.fStartColorUnitIntensityMax = 0.9f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMin = 0.0f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMax = 0.2f;

	m_SmokeTrailAttrib.fColorUnitSliderSpeedMin = 4.5f;
	m_SmokeTrailAttrib.fColorUnitSliderSpeedMax = 5.5f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMin = 0.0f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMax = 0.0f;
}


CFlamer::CFlamer() {
	_ClearDataMembers();
}


CFlamer::~CFlamer() {
	Destroy();
}


BOOL CFlamer::Create( CWeapon *pOwnerWeapon, const CDamageProfile *pDamageProfileHit, const CDamageProfile *pDamageProfileBurn, u32 nStreamNodeCount, FSndFx_FxHandle_t hSoundFire, FSndFx_FxHandle_t hSoundOff ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );

	_ClearDataMembers();

	FMATH_SETBITMASK( m_nFlags, FLAG_CREATED );
	flinklist_AddTail( &m_LinkRoot, this );

	FResFrame_t ResFrame = fres_GetFrame();

	m_pOwnerWeapon = pOwnerWeapon;
	m_pDamageProfileHit = pDamageProfileHit;
	m_pDamageProfileBurn = pDamageProfileBurn;

	if( nStreamNodeCount < 10 ) {
		DEVPRINTF( "CFlamer::Create(): At least 10 nodes are required.\n" );
		nStreamNodeCount = 10;
	}

	m_nMaxFlamerNodeCount = nStreamNodeCount;
	m_pFlamerNodeArray = fnew CFlamerNode[nStreamNodeCount];
	if( m_pFlamerNodeArray == NULL ) {
		DEVPRINTF( "CFlamer::Create(): Not enough memory to create m_pFlamerNodeArray.\n" );
		goto _ExitWithError;
	}

	m_pPSGroup = fnew CFPSpriteGroup;
	if( m_pPSGroup == NULL ) {
		DEVPRINTF( "CFlamer::Create(): Not enough memory to create m_pPSGroup.\n" );
		goto _ExitWithError;
	}

	m_pPSGroup->m_Xfm.Identity();
	m_pPSGroup->m_BoundSphere_MS.Set( CFVec3A::m_Null.v3, 1.0f );
	m_pPSGroup->m_fCullDist = FMATH_MAX_FLOAT;//0.0f;//(300.0f * 300.0f);
	m_pPSGroup->m_nPSFlags = FPSPRITE_FLAG_NONE;
	m_pPSGroup->m_nBlend = FPSPRITE_BLEND_ADD;
	m_pPSGroup->m_TexInst = m_ParticleTexInst;
	m_pPSGroup->m_TexInst.SetFlags( CFTexInst::FLAG_NONE );
	m_pPSGroup->m_nMaxCount = 0;
	m_pPSGroup->m_pBase = NULL;
	m_pPSGroup->m_nRenderCount = 0;
	m_pPSGroup->m_nRenderStartIndex = 0;

	m_nActivePartCount = 0;

	m_hSoundFire = hSoundFire;
	m_hSoundOff = hSoundOff;

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CFlamer::Destroy( void ) {

	if( IsCreated() ) {
		if( m_pSoundFireEmitter ) {
			m_pSoundFireEmitter->Destroy();
			m_pSoundFireEmitter = NULL;
		}

		fdelete( m_pPSGroup );
		flinklist_Remove( &m_LinkRoot, this );
		_ClearDataMembers();
	}
}


void CFlamer::_ClearDataMembers( void ) {
	u32 i;

	m_pOwnerWeapon = NULL;
	m_nFlags = FLAG_NONE;
	m_pFlamerNodeArray = NULL;
	m_nAliveNodeCount = 0;
	m_nNextAvailNodeIndex = 0;
	m_nOldestAliveNodeIndex = 0;
	m_nEmitterNodeIndex = 0;
	m_fSecsUntilNextSmokeSpawn = 0.0f;
	m_fSecsUntilNextNodeSpawn = 0.0f;
	m_pPSGroup = NULL;
	m_hSoundFire = NULL;
	m_hSoundOff = NULL;
	m_pSoundFireEmitter = NULL;

	m_fSecsUntilNextBurner = 0.0f;

	for( i=0; i<FLAMER_LAYER_COUNT; ++i ) {
		m_afCurrentTC[i] = 0.0f;
	}

	m_uAISoundHandle = 0;
}


// Assumes the FDraw renderer is currently active.
// Assumes the game's main perspective camera is
// set up. Assumes there are no model Xfms on the
// stack.
//
// Does not preserve the current FDraw state.
// Does not preserve the current viewport.
// Will preserve the Xfm stack.
// Will preserve the frenderer fog state.
void CFlamer::DrawAll( void ) {
	CFlamer *pFlamer;
	BOOL bSetState;
	u32 nMaxParticleCount;
	CFCamera *pCamera;

	FASSERT( m_bSystemInitialized );

	m_pVtxArray = NULL;

	if( !m_bCanDraw ) {
		// No textures. Cannot draw...
		goto _Exit;
	}

	m_pVtxArray = fvtxpool_GetArray( _VTX_POOL_COUNT );		// SER: Fix this!!!!!! xxxxxxxxxxxxxxxxxxxxxx
	if( m_pVtxArray == NULL ) {
		// Couldn't get a pool of FDrawVtx_t's.
		// We'll try again next frame...
		goto _Exit;
	}

	// Grab camera position...
	pCamera = gamecam_GetActiveCamera();
	m_pCamXfm = pCamera->GetFinalXfm();

	// Draw flamer...
	bSetState = FALSE;
	for( pFlamer=(CFlamer *)flinklist_GetHead( &m_LinkRoot ); pFlamer; pFlamer=(CFlamer *)flinklist_GetNext( &m_LinkRoot, pFlamer ) ) {
		if( !bSetState ) {
			frenderer_SetDefaultState();

			fdraw_SetTexture( &m_MainTexDef );
			fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
			fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST );
			fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER );
			fdraw_Depth_EnableWriting( FALSE );

			bSetState = TRUE;
		}

		pFlamer->_Draw();
	}

	// Draw particles...
	nMaxParticleCount = 0;
	for( pFlamer=(CFlamer *)flinklist_GetHead( &m_LinkRoot ); pFlamer; pFlamer=(CFlamer *)flinklist_GetNext( &m_LinkRoot, pFlamer ) ) {
		if( pFlamer->m_nActivePartCount > nMaxParticleCount ) {
			nMaxParticleCount = pFlamer->m_nActivePartCount;
		}
	}

	if( nMaxParticleCount ) {
		// We need to draw some particles...
		m_pPSpriteArray = pspool_GetArray( nMaxParticleCount );

		if( m_pPSpriteArray ) {
			frenderer_Push( FRENDERER_PSPRITE, NULL );

			for( pFlamer=(CFlamer *)flinklist_GetHead( &m_LinkRoot ); pFlamer; pFlamer=(CFlamer *)flinklist_GetNext( &m_LinkRoot, pFlamer ) ) {
				if( pFlamer->m_nActivePartCount ) {
					pFlamer->_DrawParticles();
				}
			}

			frenderer_Pop();

			pspool_ReturnArray( m_pPSpriteArray );
			m_pPSpriteArray = NULL;
		}
	}

_Exit:
	fvtxpool_ReturnArray( m_pVtxArray );
}


void CFlamer::_Draw( void ) {
	FASSERT( IsCreated() );

	if( m_nAliveNodeCount < 2 ) {
		// No nodes...
		return;
	}

	CFVec3A VecFromPrevToCurrentNode, UnitVecFromPrevToCurrentNode, LastUnitVecFromPrevToCurrentNode, Corner_WS;
	CFVec3A *pCornerUnitDirArray, *pSrcCornerUnitDir, *pPrevSrcCornerUnitDir;
	CFQuatA Quat;
	CFlamerNode *pPrevNode, *pNode, **ppCornerNodeArray;
	s32 i, j, nNodeIndex, nCornerNodeCount, nLayer, nTriStripVtxCount;
	f32 fMag2, fMag, fDot;
	BOOL bInitialCornerVecsComputed;

	// Grab a temporary memory frame...
	FMemFrame_t MemFrame = fmem_GetFrame();

	// Allocate a temporary memory buffer to store our corner direction vectors...
	pCornerUnitDirArray = (CFVec3A *)fmem_AllocAndZero( sizeof(CFVec3A) * (_CROSS_SECTION_CORNER_COUNT+1) * m_nAliveNodeCount, FCLASS_BYTE_ALIGN );
	if( pCornerUnitDirArray == NULL ) {
		// Not enough memory for our buffer. Try again next frame...
		goto _Exit;
	}

	ppCornerNodeArray = (CFlamerNode **)fmem_AllocAndZero( sizeof(CFlamerNode *) * m_nAliveNodeCount );
	if( ppCornerNodeArray == NULL ) {
		// Not enough memory for our buffer. Try again next frame...
		goto _Exit;
	}

	pPrevNode = m_pFlamerNodeArray + m_nEmitterNodeIndex;

	nNodeIndex = m_nEmitterNodeIndex - 1;
	if( nNodeIndex < 0 ) {
		nNodeIndex = m_nMaxFlamerNodeCount - 1;
	}

	nCornerNodeCount = 0;
	m_pNextCornerUnitVec = pCornerUnitDirArray;
	bInitialCornerVecsComputed = FALSE;

	for( i=1; i<(s32)m_nAliveNodeCount; ++i ) {
		pNode = m_pFlamerNodeArray + nNodeIndex;

		// Form vector from previous point to current point...
		VecFromPrevToCurrentNode.Sub( pNode->m_Pos_WS, pPrevNode->m_Pos_WS );

		fMag2 = VecFromPrevToCurrentNode.MagSq();

		if( fMag2 > 0.1f ) {
			// Valid length...

			fMag = fmath_Sqrt( fMag2 );

			UnitVecFromPrevToCurrentNode.Mul( VecFromPrevToCurrentNode, fmath_InvSqrt(fMag2) );

			if( bInitialCornerVecsComputed ) {
				// We've already formed our initial corner vectors...

				fDot = UnitVecFromPrevToCurrentNode.Dot( LastUnitVecFromPrevToCurrentNode );

				if( fDot < 0.5f ) {
					UnitVecFromPrevToCurrentNode = LastUnitVecFromPrevToCurrentNode;
				}

				Quat.BuildQuat( LastUnitVecFromPrevToCurrentNode, UnitVecFromPrevToCurrentNode );

				pSrcCornerUnitDir = m_pNextCornerUnitVec - (_CROSS_SECTION_CORNER_COUNT + 1);
				for( j=0; j<_CROSS_SECTION_CORNER_COUNT; ++j, ++m_pNextCornerUnitVec, ++pSrcCornerUnitDir ) {
					Quat.MulPoint( *m_pNextCornerUnitVec, *pSrcCornerUnitDir );
				}

				*m_pNextCornerUnitVec = m_pNextCornerUnitVec[-_CROSS_SECTION_CORNER_COUNT];
				++m_pNextCornerUnitVec;

				ppCornerNodeArray[nCornerNodeCount++] = pNode;

				u32 nPartCount = fmath_RandomRange( 2, 2 );
				u32 nEmptySlotCount = _PARTICLE_COUNT - m_nActivePartCount;
				u32 k;
				f32 fScale;
				FMATH_CLAMPMAX( nPartCount, nEmptySlotCount );
				CFVec3A TempVec, TempVec2;

				CFlamerPart *pPart = &m_aPartArray[m_nActivePartCount];

				for( k=0; k<nPartCount; ++k, ++pPart ) {
					fScale = pNode->m_afHalfWidth_WS[FLAMER_LAYER_COUNT-1];

					TempVec.Set( fmath_RandomFloatRange( -1.0f, 1.0f ), fmath_RandomFloatRange( 0.05f, 1.0f ), fmath_RandomFloatRange( -1.0f, 1.0f ) );
					TempVec.Unitize().Mul( fmath_RandomFloatRange( 0.0f, fScale * 2.0f ) );

					TempVec2.Mul( UnitVecFromPrevToCurrentNode, fmath_RandomFloatRange( 0.0f, fMag ) ).Add( pNode->m_Pos_WS );

					pPart->m_Pos_WS.Add( TempVec, TempVec2 );
					pPart->m_Vel_WS.Set( 0.0f, fmath_RandomFloatRange( 13.0f, 16.0f ), 0.0f ).Add( pNode->m_CurrentVel_WS );
					pPart->m_fAlpha_WS = 0.5f;
					pPart->m_fScale_WS = fScale + fmath_RandomFloatRange( 1.0f, 1.0f );
				}

				m_nActivePartCount += nPartCount;
			} else {
				// We haven't formed our initial corner vectors yet, so let's do it...

				bInitialCornerVecsComputed = TRUE;

				_CreateInitialCornerUnitDirs( &UnitVecFromPrevToCurrentNode );

				// In this case, the current corner vectors are the same as the initial ones...
				fang_MemCopy( m_pNextCornerUnitVec, &m_pNextCornerUnitVec[-(_CROSS_SECTION_CORNER_COUNT+1)], sizeof(CFVec3A) * (_CROSS_SECTION_CORNER_COUNT+1) );
				m_pNextCornerUnitVec += (_CROSS_SECTION_CORNER_COUNT + 1);

				ppCornerNodeArray[nCornerNodeCount++] = pPrevNode;
				ppCornerNodeArray[nCornerNodeCount++] = pNode;
			}

			LastUnitVecFromPrevToCurrentNode = UnitVecFromPrevToCurrentNode;
		}

		pPrevNode = pNode;

		--nNodeIndex;
		if( nNodeIndex < 0 ) {
			nNodeIndex = m_nMaxFlamerNodeCount - 1;
		}
	}

	if( nCornerNodeCount < 2 ) {
		goto _Exit;
	}

	f32 fTCOffset;

	for( nLayer=0; nLayer<FLAMER_LAYER_COUNT; ++nLayer ) {
		m_pNextVtx = m_pVtxArray;
		nTriStripVtxCount = 0;

		fTCOffset = (f32)nLayer * 0.3f;

		pPrevSrcCornerUnitDir = &pCornerUnitDirArray[0];
		pSrcCornerUnitDir = &pCornerUnitDirArray[(_CROSS_SECTION_CORNER_COUNT+1)];

		for( i=1; i<nCornerNodeCount; ++i ) {
			pPrevNode = ppCornerNodeArray[i-1];
			pNode = ppCornerNodeArray[i];

			if( pNode->m_bTerminalNode ) {
				if( nTriStripVtxCount ) {
					fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, m_pVtxArray, nTriStripVtxCount );

					m_pNextVtx = m_pVtxArray;
					nTriStripVtxCount = 0;
				}
			} else {
				for( j=0; j<(_CROSS_SECTION_CORNER_COUNT+1); ++j, ++pPrevSrcCornerUnitDir, ++pSrcCornerUnitDir ) {
					Corner_WS.Mul( *pPrevSrcCornerUnitDir, pPrevNode->m_afHalfWidth_WS[nLayer] ).Add( pPrevNode->m_Pos_WS );

					m_pNextVtx->Pos_MS.Set( Corner_WS.x, Corner_WS.y, Corner_WS.z );
					m_pNextVtx->ColorRGBA.Set( 1.0f, 1.0f, 1.0f, pPrevNode->m_afAlpha[nLayer] );
					m_pNextVtx->ST.Set( m_afCornerTC[j] + fTCOffset, pPrevNode->m_afTC[nLayer] );
					++m_pNextVtx;

					Corner_WS.Mul( *pSrcCornerUnitDir, pNode->m_afHalfWidth_WS[nLayer] ).Add( pNode->m_Pos_WS );

					m_pNextVtx->Pos_MS.Set( Corner_WS.x, Corner_WS.y, Corner_WS.z );
					m_pNextVtx->ColorRGBA.Set( 1.0f, 1.0f, 1.0f, pNode->m_afAlpha[nLayer] );
					m_pNextVtx->ST.Set( m_afCornerTC[j] + fTCOffset, pNode->m_afTC[nLayer] );
					++m_pNextVtx;
				}

				nTriStripVtxCount += ((_CROSS_SECTION_CORNER_COUNT + 1) << 1);
			}
		}

		if( nTriStripVtxCount ) {
			fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, m_pVtxArray, nTriStripVtxCount );
		}
	}

	_SpawnSmokeTrail();

_Exit:
	fmem_ReleaseFrame( MemFrame );
}


void CFlamer::_CreateInitialCornerUnitDirs( const CFVec3A *pUnitVecFromPrevToCurrentNode ) {
	CFMtx43A Mtx;
	f32 fMag2;
	u32 i;

	// Front axis points along node axis...
	Mtx.m_vFront = *pUnitVecFromPrevToCurrentNode;

	// Compute right axis...
	Mtx.m_vRight.CrossYWithVec( Mtx.m_vFront );
	fMag2 = Mtx.m_vRight.MagSq();
	if( fMag2 > 0.0001f ) {
		Mtx.m_vRight.Mul( fmath_InvSqrt(fMag2) );
	} else {
		Mtx.m_vRight.UnitCross( CFVec3A::m_UnitAxisX, Mtx.m_vFront );
	}

	// Compute up axis...
	Mtx.m_vUp.UnitCross( Mtx.m_vFront, Mtx.m_vRight );

	// Compute the transformed unit direction vectors...
	for( i=0; i<_CROSS_SECTION_CORNER_COUNT; ++i, ++m_pNextCornerUnitVec ) {
		Mtx.MulDir( *m_pNextCornerUnitVec, m_aCornerUnitVec[i] );
	}

	// Last vector is the same as the first...
	*m_pNextCornerUnitVec = m_pNextCornerUnitVec[-_CROSS_SECTION_CORNER_COUNT];
	++m_pNextCornerUnitVec;
}


void CFlamer::_DrawParticles( void ) {
	CFlamerPart *pPart;
	FPSprite_t *pPSprite;
	u32 i;

	for( i=0, pPart=m_aPartArray, pPSprite=m_pPSpriteArray; i<m_nActivePartCount; ++i, ++pPart, ++pPSprite ) {
		pPSprite->Point_MS.Set( pPart->m_Pos_WS.x, pPart->m_Pos_WS.y, pPart->m_Pos_WS.z );
		pPSprite->fDim_MS = pPart->m_fScale_WS;
		pPSprite->ColorRGBA.Set( pPart->m_fAlpha_WS, pPart->m_fAlpha_WS, pPart->m_fAlpha_WS, pPart->m_fAlpha_WS );
	}

	m_pPSGroup->m_pBase = m_pPSpriteArray;
	m_pPSGroup->m_nRenderCount = m_pPSGroup->m_nMaxCount = m_nActivePartCount;
	m_pPSGroup->Render();
}


BOOL CFlamer::TriggerWork( f32 fTriggerValue, const CFVec3A *pUnitEmitterDir_WS, const CFVec3A *pEmitterPos_WS, const CFVec3A *pUserVel_WS ) {
	u32 i;

	FASSERT( IsCreated() );

	if( fTriggerValue == 0.0f ) {
		// Trigger is up...

		if( m_nFlags & FLAG_STREAM_ON ) {
			// Turn stream off...

			FMATH_CLEARBITMASK( m_nFlags, FLAG_STREAM_ON );
			m_fSecsUntilNextNodeSpawn = 0.0f;

			for( i=0; i<FLAMER_LAYER_COUNT; ++i ) {
				m_afCurrentTC[i] = 0.0f;
			}

			// Turn our emitter node into a standard terminal node...
			_ReplaceNode( &m_pFlamerNodeArray[m_nEmitterNodeIndex], pUnitEmitterDir_WS, pEmitterPos_WS, pUserVel_WS, TRUE, FALSE );

			// Stop sound...
			if( m_pSoundFireEmitter ) {
				m_pSoundFireEmitter->Destroy();
				m_pSoundFireEmitter = NULL;
			}

			if( m_pOwnerWeapon->IsOwnedByPlayer() ) {
				fsndfx_Play2D( m_hSoundOff, 1.0f, _SOUND_FREQ_MULTIPLIER );
			} else {
				fsndfx_Play3D( m_hSoundOff, &m_pOwnerWeapon->MtxToWorld()->m_vPos, 100.0f, 1.0f, _SOUND_FREQ_MULTIPLIER );
			}
		}

		return FALSE;
	}

	// Trigger is down...

	for( i=0; i<FLAMER_LAYER_COUNT; ++i ) {
		m_afCurrentTC[i] += (1.0f + (f32)(FLAMER_LAYER_COUNT-i-1) * 4.0f) * FLoop_fPreviousLoopSecs;
	}

	if( m_nFlags & FLAG_STREAM_ON ) {
		// The stream is already on...

		// Can we spawn a new node?
		if( m_fSecsUntilNextNodeSpawn > 0.0f ) {
			// Not yet. Just update the position of our emitter...

			m_pFlamerNodeArray[m_nEmitterNodeIndex].m_Pos_WS = *pEmitterPos_WS;

			return TRUE;
		}

		// Turn our current emitter into a standard node...
		_ReplaceNode( &m_pFlamerNodeArray[m_nEmitterNodeIndex], pUnitEmitterDir_WS, pEmitterPos_WS, pUserVel_WS, FALSE, FALSE );

		// Create a new emitter...
		m_nEmitterNodeIndex = m_nNextAvailNodeIndex;
		_CreateNewNode( pUnitEmitterDir_WS, pEmitterPos_WS, pUserVel_WS, TRUE, TRUE );

		return TRUE;
	}

	// We're starting a new stream...

	FMATH_SETBITMASK( m_nFlags, FLAG_STREAM_ON );
	m_fSecsUntilNextNodeSpawn = _SECS_BETWEEN_NODE_SPAWNS;

	// Create a new node...
	_CreateNewNode( pUnitEmitterDir_WS, pEmitterPos_WS, pUserVel_WS, FALSE, FALSE );

	// Create a new emitter node...
	m_nEmitterNodeIndex = m_nNextAvailNodeIndex;
	_CreateNewNode( pUnitEmitterDir_WS, pEmitterPos_WS, pUserVel_WS, TRUE, TRUE );

	if( m_pOwnerWeapon->IsOwnedByPlayer() ) {
		m_pSoundFireEmitter = FSNDFX_ALLOCNPLAY2D( m_hSoundFire, 1.0f, _SOUND_FREQ_MULTIPLIER, FAudio_EmitterDefaultPriorityLevel, 0.f, TRUE );
	} else {
		m_pSoundFireEmitter = FSNDFX_ALLOCNPLAY3D( m_hSoundFire, &m_pOwnerWeapon->MtxToWorld()->m_vPos, 100.0f, 1.0f, _SOUND_FREQ_MULTIPLIER, FAudio_EmitterDefaultPriorityLevel, TRUE );
	}

	return TRUE;
}

void CFlamer::StopFlamer( void ) {
	m_nAliveNodeCount = 0;
	m_nNextAvailNodeIndex = 0;
	m_nOldestAliveNodeIndex = 0;
	m_nEmitterNodeIndex = 0;
	m_fSecsUntilNextSmokeSpawn = 0.0f;
	m_fSecsUntilNextNodeSpawn = 0.0f;
	m_fSecsUntilNextBurner = 0.0f;

	FMATH_CLEARBITMASK( m_nFlags, FLAG_STREAM_ON );

	u32 i;

	for( i=0; i<FLAMER_LAYER_COUNT; ++i ) {
		m_afCurrentTC[i] = 0.0f;
	}

	// Stop sound...
	if( m_pSoundFireEmitter ) {
		m_pSoundFireEmitter->Destroy();
		m_pSoundFireEmitter = NULL;
	}

	m_uAISoundHandle = 0;
}

void CFlamer::_ReplaceNode( CFlamerNode *pNode, const CFVec3A *pUnitEmitterDir_WS, const CFVec3A *pEmitterPos_WS, const CFVec3A *pUserVel_WS, BOOL bTerminalNode, BOOL bEmitterNode, BOOL bCreateNewColor ) {
	u32 i;
	f32 fUnitLayer;

	pNode->m_Pos_WS = *pEmitterPos_WS;
	pNode->m_InitialVel_WS.Mul( *pUnitEmitterDir_WS, _INITIAL_STREAM_SPEED ).Add( *pUserVel_WS );
	pNode->m_CurrentVel_WS = pNode->m_InitialVel_WS;
	pNode->m_fUnitLife_WS = 0.0f;
	pNode->m_bTerminalNode = bTerminalNode;
	pNode->m_bEmitterNode = bEmitterNode;

	for( i=0; i<FLAMER_LAYER_COUNT; ++i ) {
		fUnitLayer = (f32)i * (1.0f / (f32)(FLAMER_LAYER_COUNT-1));

		pNode->m_afHalfWidth_WS[i] = _INITIAL_STREAM_WIDTH;
		pNode->m_afWidthSpeed_WS[i] = 4.0f + 4.0f * fUnitLayer + fmath_RandomFloatRange( -1.0f, 1.0f );
		pNode->m_afTC[i] = m_afCurrentTC[i];

		if( bCreateNewColor ) {
			pNode->m_afAlpha[i] = 1.0f - fUnitLayer*0.8f;
		}
	}
}


void CFlamer::_CreateNewNode( const CFVec3A *pUnitEmitterDir_WS, const CFVec3A *pEmitterPos_WS, const CFVec3A *pUserVel_WS, BOOL bTerminalNode, BOOL bEmitterNode ) {
	_ReplaceNode( &m_pFlamerNodeArray[m_nNextAvailNodeIndex], pUnitEmitterDir_WS, pEmitterPos_WS, pUserVel_WS, bTerminalNode, bEmitterNode, TRUE );

	// Compute next available node index...
	++m_nAliveNodeCount;
	++m_nNextAvailNodeIndex;
	if( m_nNextAvailNodeIndex >= m_nMaxFlamerNodeCount ) {
		m_nNextAvailNodeIndex = 0;
	}

	if( m_nNextAvailNodeIndex == m_nOldestAliveNodeIndex ) {
		--m_nAliveNodeCount;
		++m_nOldestAliveNodeIndex;
		if( m_nOldestAliveNodeIndex >= m_nMaxFlamerNodeCount ) {
			m_nOldestAliveNodeIndex = 0;
		}
	}
}


void CFlamer::WorkAll( void ) {
	CFlamer *pFlamer;

	FASSERT( m_bSystemInitialized );

	if( FLoop_bGamePaused ) {
		return;
	}

	for( pFlamer=(CFlamer *)flinklist_GetHead( &m_LinkRoot ); pFlamer; pFlamer=(CFlamer *)flinklist_GetNext( &m_LinkRoot, pFlamer ) ) {
		pFlamer->_Work();
	}
}


void CFlamer::_Work( void ) {
	FASSERT( IsCreated() );

	// Update node spawn timer...
	m_fSecsUntilNextNodeSpawn -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN( m_fSecsUntilNextNodeSpawn, 0.0f );

	_ParticleWork();

	if( m_nAliveNodeCount == 0 ) {
		// No nodes...
		return;
	}

	// There's at least a node...

	u32 i, nNodeIndex, nKillCount, nLayer;
	f32 fUnit, fWidthSpeed, fUnitLayer, fInitialAlpha;
	CFlamerNode *pNode, *pPrevNode;
	CFVec3A VelStep_WS;
	FCollImpact_t CollImpact;

	nKillCount = 0;
	nNodeIndex = m_nOldestAliveNodeIndex;

	for( i=0; i<m_nAliveNodeCount; ++i ) {
		pNode = m_pFlamerNodeArray + nNodeIndex;

#if 0
		if( i ) {
			if( fworld_FindClosestImpactPointToRayStart( &CollImpact, &pNode->m_Pos_WS, &pPrevNode->m_Pos_WS ) ) {
				pNode->m_bTerminalNode = TRUE;
				pNode->m_Pos_WS = CollImpact.ImpactPoint;
			}
		}
#endif

		if( !pNode->m_bEmitterNode ) {
			// Update node life...
			pNode->m_fUnitLife_WS += 2.0f * FLoop_fPreviousLoopSecs;
			if( pNode->m_fUnitLife_WS >= 1.0f ) {
				// Kill this node...
				pNode->m_fUnitLife_WS = 1.0f;

				++nKillCount;
				++m_nOldestAliveNodeIndex;
				if( m_nOldestAliveNodeIndex == m_nMaxFlamerNodeCount ) {
					m_nOldestAliveNodeIndex = 0;
				}
			}

			// Compute current velocity...
			fUnit = pNode->m_fUnitLife_WS * pNode->m_fUnitLife_WS;
			fUnit = 1.0f - fUnit;
			VelStep_WS.Mul( pNode->m_InitialVel_WS, fUnit );
			pNode->m_CurrentVel_WS = VelStep_WS;

//			pNode->m_InitialVel_WS.y -= 64.0f * FLoop_fPreviousLoopSecs;

			// Compute velocity step...
			VelStep_WS.Mul( FLoop_fPreviousLoopSecs );

			// Update position...
			pNode->m_Pos_WS.Add( VelStep_WS );

			// Update width...
			for( nLayer=0; nLayer<FLAMER_LAYER_COUNT; ++nLayer ) {
				fUnitLayer = (f32)nLayer * (1.0f / (f32)(FLAMER_LAYER_COUNT-1));

				fWidthSpeed = pNode->m_afWidthSpeed_WS[nLayer] + FMATH_FPOT( fUnit, 0.0f, 5.0f );

				pNode->m_afHalfWidth_WS[nLayer] += pNode->m_afWidthSpeed_WS[nLayer] * FLoop_fPreviousLoopSecs;

				pNode->m_afWidthSpeed_WS[nLayer] -= (pNode->m_afWidthSpeed_WS[nLayer] * FLoop_fPreviousLoopSecs);

				fInitialAlpha = 1.0f - fUnitLayer*0.8f;
				pNode->m_afAlpha[nLayer] = FMATH_FPOT( pNode->m_fUnitLife_WS, fInitialAlpha, 0.0f );
			}
		}

		pPrevNode = pNode;

		++nNodeIndex;
		if( nNodeIndex >= m_nMaxFlamerNodeCount ) {
			nNodeIndex = 0;
		}
	}

	m_nAliveNodeCount -= nKillCount;

	_SpawnBurners();
}


void CFlamer::_SpawnSmokeTrail( void ) {
	SmokeTrailHandle_t hSmokeTrail;
	u32 i, nNodeIndex;
	CFlamerNode *pNode;

	m_fSecsUntilNextSmokeSpawn -= FLoop_fPreviousLoopSecs;
	if( m_fSecsUntilNextSmokeSpawn <= 0.0f ) {
		m_fSecsUntilNextSmokeSpawn = 0.05f;

		if( m_nAliveNodeCount >= 2 ) {
			hSmokeTrail = smoketrail_GetFromFreePoolAndSetAttributes( &m_SmokeTrailAttrib, FALSE );

			if( hSmokeTrail != SMOKETRAIL_NULLHANDLE ) {
				nNodeIndex = m_nOldestAliveNodeIndex + 1;
				if( nNodeIndex >= m_nMaxFlamerNodeCount ) {
					nNodeIndex = 0;
				}

				for( i=1; i<m_nAliveNodeCount; ++i ) {
					pNode = &m_pFlamerNodeArray[nNodeIndex];

					if( pNode->m_afHalfWidth_WS[FLAMER_LAYER_COUNT-1] > 2.0f ) {
						smoketrail_Puff( hSmokeTrail, &pNode->m_Pos_WS );

						++nNodeIndex;
						if( nNodeIndex >= m_nMaxFlamerNodeCount ) {
							nNodeIndex = 0;
						}
					}
				}

				smoketrail_ReturnToFreePool( hSmokeTrail, TRUE );
			}
		}
	}
}


void CFlamer::_ParticleWork( void ) {
	if( m_nActivePartCount == 0 ) {
		return;
	}

	CFlamerPart *pPart;
	s32 i;
	CFVec3A VelStep_WS;

	for( i=0, pPart=m_aPartArray; i<(s32)m_nActivePartCount; ++i, ++pPart ) {
		VelStep_WS.Mul( pPart->m_Vel_WS, FLoop_fPreviousLoopSecs );
		pPart->m_Pos_WS.Add( VelStep_WS );

//		pPart->m_Vel_WS.y -= 64.0f * FLoop_fPreviousLoopSecs;

		pPart->m_fScale_WS += 0.25f * FLoop_fPreviousLoopSecs;

		pPart->m_fAlpha_WS -= 2.0f * FLoop_fPreviousLoopSecs;
		if( pPart->m_fAlpha_WS <= 0.0f ) {
			// Kill particle by swapping with last...

			if( --m_nActivePartCount == (u32)i ) {
				break;
			}

			*pPart = m_aPartArray[m_nActivePartCount];

			--i;
			--pPart;
		}
	}

	m_pPSGroup->ComputeBoundingSphere();
}


void CFlamer::_SpawnBurners( void ) {
	if( m_nActivePartCount == 0 ) {
		return;
	}

	m_fSecsUntilNextBurner -= FLoop_fPreviousLoopSecs;
	if( m_fSecsUntilNextBurner > 0.0f ) {
		return;
	}

	m_fSecsUntilNextBurner = fmath_RandomFloatRange( 0.05f, 0.15f );

	FCollImpact_t CollImpact;
	CFVec3A RayStart, RayEnd, UnitFireDir;
	f32 fMag2;

	RayStart = m_pFlamerNodeArray[m_nEmitterNodeIndex].m_Pos_WS;
	RayEnd = m_pFlamerNodeArray[m_nOldestAliveNodeIndex].m_Pos_WS;

	UnitFireDir.Sub( RayEnd, RayStart );
	fMag2 = UnitFireDir.MagSq();
	if( fMag2 < 0.0001f ) {
		return;
	}
	UnitFireDir.Mul( fmath_InvSqrt( fMag2 ) );

	if( !fworld_FindClosestImpactPointToRayStart( &CollImpact, &RayStart, &RayEnd, 0, NULL, TRUE, NULL, -1, FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES ) ) {
		// No collision...
		return;
	}

	// We hit something...
	if( !m_uAISoundHandle || !AIEnviro_ModifySound( m_uAISoundHandle, CollImpact.ImpactPoint, AIEnviro_fFlamerImpactSoundRadius, 1.0f, AISOUNDTYPE_SWARMERBARRIER, NULL ) ) {
		m_uAISoundHandle = AIEnviro_AddSound(CollImpact.ImpactPoint, AIEnviro_fFlamerImpactSoundRadius, 1.0, AISOUNDTYPE_SWARMERBARRIER, AISOUNDCTRL_NONE, NULL);
	}

	CFMtx43A Mtx;
	f32 fRandSecs;
	CEParticle *pEPart = NULL;
//	CEParticle *pEPartSmoke = NULL;

	Mtx.m_vPos = CollImpact.ImpactPoint;
	Mtx.m_vFront = CollImpact.UnitFaceNormal;
	Mtx.m_vRight.CrossYWithVec( Mtx.m_vFront );
	fMag2 = Mtx.m_vRight.MagSq();
	if( fMag2 > 0.0001f ) {
		Mtx.m_vRight.Mul( fmath_InvSqrt(fMag2) );
	} else {
		Mtx.m_vRight.UnitCross( CFVec3A::m_UnitAxisX, Mtx.m_vFront );
	}
	Mtx.m_vUp.UnitCross( Mtx.m_vFront, Mtx.m_vRight );

	// grab a particle emitter
	pEPart = eparticlepool_GetEmitter();
	if( !pEPart ) {
		return;
	}

	fRandSecs = fmath_RandomFloatRange( 2.0f, 4.0f );

	pEPart->AddToWorld();
	pEPart->Relocate_RotXlatFromUnitMtx_WS( &Mtx );
	pEPart->StartEmission( m_hParticleDef, fmath_RandomFloatRange( 0.5f, 1.0f ), fRandSecs, TRUE );

//	pEPartSmoke->AddToWorld();
//	pEPartSmoke->Relocate_RotXlatFromUnitMtx_WS( &Mtx );
//	pEPartSmoke->StartEmission( m_hParticleSmokeDef, 1.0f, fRandSecs, TRUE );

	if( CollImpact.pTag ) {
		// Hit an object...

		CFWorldMesh *pWorldMesh = (CFWorldMesh *)CollImpact.pTag;

		// See if the world mesh that we found is an entity...
		if( pWorldMesh->m_nUser == MESHTYPES_ENTITY ) {
			// It's an Entity!

			CEntity *pHitEntity = (CEntity *)pWorldMesh->m_pUser;

			pEPart->Attach_ToParent_WS( pHitEntity );
//			pEPartSmoke->Attach_ToParent_WS( pHitEntity );

//			if( pHitEntity->AIBrain() ) {
//				aibrainman_DamagedNotify( pHitEntity->AIBrain(), NULL, 
//			}

			// Get an empty damage form...
			CDamageForm *pDamageFormHit = CDamage::GetEmptyDamageFormFromPool();

			if( pDamageFormHit ) {
				// Fill out the form...

				pDamageFormHit->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
				pDamageFormHit->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
				pDamageFormHit->m_pDamageProfile = m_pDamageProfileHit;
				pDamageFormHit->m_Damager.pWeapon = m_pOwnerWeapon;
				pDamageFormHit->m_Damager.pBot = m_pOwnerWeapon->GetOwner();
				pDamageFormHit->m_Damager.nDamagerPlayerIndex = m_pOwnerWeapon->GetOwner() ? m_pOwnerWeapon->GetOwner()->m_nPossessionPlayerIndex : -1;
				pDamageFormHit->m_pDamageeEntity = pHitEntity;
				pDamageFormHit->InitTriDataFromCollImpact( pWorldMesh, &CollImpact, &UnitFireDir );

				CDamage::SubmitDamageForm( pDamageFormHit );
			}

			// Get an empty damage form...
			CDamageForm *pDamageFormBurn = CDamage::GetEmptyDamageFormFromPool();

			if( pDamageFormBurn ) {
				// Fill out the form...

				pDamageFormBurn->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_AMBIENT;
				pDamageFormBurn->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
				pDamageFormBurn->m_pDamageProfile = m_pDamageProfileBurn;
				pDamageFormBurn->m_Damager.pWeapon = m_pOwnerWeapon;
				pDamageFormBurn->m_Damager.pBot = m_pOwnerWeapon->GetOwner();
				pDamageFormBurn->m_Damager.nDamagerPlayerIndex = m_pOwnerWeapon->GetOwner() ? m_pOwnerWeapon->GetOwner()->m_nPossessionPlayerIndex : -1;
				pDamageFormBurn->m_pDamageeEntity = pHitEntity;

				CDamage::SubmitDamageForm( pDamageFormBurn, fRandSecs );
			}
		}
	}
}

