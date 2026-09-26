//////////////////////////////////////////////////////////////////////////////////////
// SloshTank.cpp - Slosh's backpack code
//
// Author: Michael Scholz
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
// 01.03.03 Scholz		Created.
//////////////////////////////////////////////////////////////////////////////////////


#include "sloshtank.h"
#include "fresload.h"
#include "fdraw.h"
#include "frenderer.h"
#include "fcoll.h"
#include "fworld_coll.h"
#include "floop.h"
#include "player.h"
#include "fverlet.h"
#include "fvtxpool.h"
#include "fdraw.h"

static cchar _apszSloshTankInfo_Filename[]	= "sloshtank";

BOOL		CSloshTank::m_bSystemInitialized	= FALSE;				// TRUE if the system has been initialized
BOOL		CSloshTank::m_bSystemActive			= FALSE;
CSloshTank* CSloshTank::m_aTanks				= NULL;
CSloshTank::_SloshTankProps_t CSloshTank::m_aSloshTankDefaultProps[1];

#define _MAX_SLOSHTANKS 2

struct _TriFace_t
{
	FDrawVtx_t aPoints[3];
	FINLINE operator FDrawVtx_t*() {return &aPoints[0];}

	FINLINE void SetAlpha(f32 fAlpha)
	{
		aPoints[0].ColorRGBA.fAlpha = fAlpha;
		aPoints[1].ColorRGBA.fAlpha = fAlpha;
		aPoints[2].ColorRGBA.fAlpha = fAlpha;
	}
	FINLINE void Set(FDrawVtx_t& v0,FDrawVtx_t& v1,FDrawVtx_t& v2)
	{
		aPoints[0] = v0,aPoints[1] = v1,aPoints[2] = v2;
	}
	FINLINE void Set(FDrawVtx_t& v0,FDrawVtx_t& v1,FDrawVtx_t& v2,CFVec2& ST0,CFVec2& ST1,CFVec2& ST2)
	{
		aPoints[0] = v0;aPoints[0].ST=ST0;
		aPoints[1] = v1;aPoints[1].ST=ST1;
		aPoints[2] = v2;aPoints[2].ST=ST2;
	}
};

FDrawVtx_t CSloshTank::m_aTankVertsMS[_SLOSHTANK_NUM_VERTS] = 
{
	// Base verts
	//         x         y         z        r   g   b    a    s           t
	FDrawVtx_t(-0.5312f, -0.0000f, -0.2193f,1.f,1.f,1.0f,1.0f,0.f*1.f/6.f,0.0f), // v7 => v0
	FDrawVtx_t(-0.5312f, -0.0000f, 0.2207f, 1.f,1.f,1.0f,1.0f,1.f*1.f/6.f,0.0f), // v9 => v1
	FDrawVtx_t(-0.2200f, -0.0000f, -0.5305f,1.f,1.f,1.0f,1.0f,2.f*1.f/6.f,0.0f), // v5 => v2
	FDrawVtx_t(0.0000f,  -0.0000f, 0.5319f, 1.f,1.f,1.0f,1.0f,3.f*1.f/6.f,0.0f), // v11 => v3
	FDrawVtx_t(0.2200f,  -0.0000f, -0.5305f,1.f,1.f,1.0f,1.0f,4.f*1.f/6.f,0.0f), // v3 => v4
	FDrawVtx_t(0.5312f,  -0.0000f, -0.2193f,1.f,1.f,1.0f,1.0f,5.f*1.f/6.f,0.0f), // v0 => v5
	FDrawVtx_t(0.5312f,  -0.0000f, 0.2207f, 1.f,1.f,1.0f,1.0f,6.f*1.f/6.f,0.0f),  // v13 => v6
	
	// Cap verts
	FDrawVtx_t(-0.5312f, 1.6458f,  -0.2166f,1.f,1.f,1.0f,1.0f,0.f*1.f/6.f,1.0f), // v6 => v7
	FDrawVtx_t(-0.5312f, 1.6458f,  0.2235f, 1.f,1.f,1.0f,1.0f,1.f*1.f/6.f,1.0f), // v8 => v8
	FDrawVtx_t(-0.2200f, 1.6458f,  -0.5278f,1.f,1.f,1.0f,1.0f,2.f*1.f/6.f,1.0f), // v4 => v9
	FDrawVtx_t(0.0000f,  1.6458f,  0.5347f, 1.f,1.f,1.0f,1.0f,3.f*1.f/6.f,1.0f), // v10 => v10
	FDrawVtx_t(0.2200f,  1.6458f,  -0.5278f,1.f,1.f,1.0f,1.0f,4.f*1.f/6.f,1.0f), // v2 => v11
	FDrawVtx_t(0.5312f,  1.6458f,  -0.2166f,1.f,1.f,1.0f,1.0f,5.f*1.f/6.f,1.0f), // v1 => v12
	FDrawVtx_t(0.5312f,  1.6458f,  0.2235f, 1.f,1.f,1.0f,1.0f,6.f*1.f/6.f,1.0f) // v12 => v13
};

cchar* CSloshTank::m_apszTexNames[_SLOSHTANK_NUM_TEXTURES] = 
{
	"TRDSmagma01",
	"TRDSmagma02"
};
CFTexInst	CSloshTank::m_aTextures[_SLOSHTANK_NUM_TEXTURES];

const FGameData_TableEntry_t CSloshTank::m_aSloshTankPropVocab[] =
{
	FGAMEDATA_VOCAB_F32_UNBOUND, // (f32 fNumTanks;)
	FGAMEDATA_VOCAB_F32_UNBOUND, //(f32 fRefillRate;)
	FGAMEDATA_VOCAB_F32_UNBOUND, //(f32 fAlphaFluid;		)
	FGAMEDATA_VOCAB_F32_UNBOUND, //(f32 fAlphaFluidSurface;)		
	FGAMEDATA_VOCAB_F32_UNBOUND, //(f32 fAlphaFluidCard;	)	
	FGAMEDATA_VOCAB_F32_UNBOUND, //(f32 fVerletPendentLength;	)
	FGAMEDATA_VOCAB_F32_UNBOUND, //(f32 fVerletPendentGravityX;)	
	FGAMEDATA_VOCAB_F32_UNBOUND, //(f32 fVerletPendentGravityY;)	
	FGAMEDATA_VOCAB_F32_UNBOUND, //(f32 fVerletPendentGravityZ;)	
	FGAMEDATA_VOCAB_F32_UNBOUND, //(f32 fVerletDampeningConstant;	)
	FGAMEDATA_VOCAB_F32_UNBOUND, //(f32 fTextureVelocityX;		)
	FGAMEDATA_VOCAB_F32_UNBOUND, //(f32 fTextureVelocityY;		)
	FGAMEDATA_VOCAB_F32_UNBOUND, //(f32 fTextureTravelMaxX;		)
	FGAMEDATA_VOCAB_F32_UNBOUND, //(f32 fTextureTravelMaxY;		)
	FGAMEDATA_VOCAB_F32_UNBOUND, //(f32 fTextureChangeProbabilityX;)		// odds of texture change dir
	FGAMEDATA_VOCAB_F32_UNBOUND, //(f32 fTextureChangeProbabilityY;)		// odds of texture change dir

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

const FGameDataMap_t CSloshTank::m_aGameDataMap[] = 
{
	"SloshTankTable",
	m_aSloshTankPropVocab,
	sizeof(_SloshTankProps_t),
	(void *)&CSloshTank::m_aSloshTankDefaultProps[0],

	NULL
};

BOOL CSloshTank::_InitTextures( void )
{
	FTexDef_t *pTexDef;
	
	for( u32 i=0; i<_SLOSHTANK_NUM_TEXTURES; i++ ) 
	{
		pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, m_apszTexNames[i] );
		if( pTexDef == NULL ) 
		{
			DEVPRINTF( "CSloshTank::_InitTextures():  Error loading texture:  %s\n", m_apszTexNames[i] );
			return FALSE;
		}
		m_aTextures[i].SetTexDef( pTexDef );
	}
	m_aTextures[0].SetFlags( CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T );

	return TRUE;
}

BOOL CSloshTank::InitSystem( void )
{
	FASSERT( !m_bSystemInitialized );

	FResFrame_t Frame;

	Frame = fres_GetFrame();

	if( !fgamedata_ReadFileUsingMap( m_aGameDataMap, _apszSloshTankInfo_Filename) )
	{
		DEVPRINTF( "CRatGunRocket::Create(): Could not create .\n" );
		goto _ExitInitSystemWithError;
	}

	m_aTanks = fnew CSloshTank[_MAX_SLOSHTANKS];
	if( m_aTanks == NULL ) 
	{
		DEVPRINTF( "CSplat::InitSystem(): Could not allocate CSloshTank array\n" );
		goto _ExitInitSystemWithError;
	}
	
	//initialize the textures
	if( !_InitTextures() ) 
	{
		DEVPRINTF( "CSloshTank::InitSystem(): Could not initialize textures\n" );
		goto _ExitInitSystemWithError;
	}

	for (u32 uIndex = 0;uIndex < _SLOSHTANK_NUM_VERTS; uIndex++)
	{
		m_aTankVertsMS[uIndex].ColorRGBA.fAlpha = CSloshTank::m_aSloshTankDefaultProps[0].fAlphaFluid;
	}
	m_bSystemInitialized = TRUE;
	return TRUE;

	// Error:
_ExitInitSystemWithError:
	UninitSystem();
	fres_ReleaseFrame( Frame );
	return TRUE;
}



void CSloshTank::UninitSystem( void )
{
	if( !m_bSystemInitialized )
	{
		return;
	}
	m_bSystemInitialized = FALSE;
}

CSloshTank* CSloshTank::GetTank(void)
{
	for( u32 i=0; i<_MAX_SLOSHTANKS; i++ ) 
	{
		if (!m_aTanks[i].m_bActive)
		{
			CSloshTank* pTank = &m_aTanks[i];
			CSloshTank::m_bSystemActive = TRUE;
			pTank->m_bActive = TRUE;
			pTank->m_SloshTankProps = CSloshTank::m_aSloshTankDefaultProps[0];
			pTank->m_fTankHeight = m_aTankVertsMS[_SLOSHTANK_NUM_VERTS-1].Pos_MS.y;
			pTank->m_fTankFillPercentHeight= 1.0f;
			pTank->m_eTankState = TR_STATE_ACTIVE;
			pTank->m_fSOffset = 0.0f;
			pTank->m_fSDir = 0.0f;
			pTank->m_fTOffset = 0.0f;
			pTank->m_fTDir = 0.0f;
			return pTank;
		}
	}
	FASSERT(!"Maximum number of tanks in world exceeded, Tell Scholz to revise maximum ");
	return 0;
}

void CSloshTank::PutTankBack(CSloshTank* pTank)
{
	pTank->m_bActive = FALSE;
}

void CSloshTank::Work( void )
{
	if( m_bSystemActive ) 
	{
		m_bSystemActive = FALSE;

		for( u32 i=0; i<_MAX_SLOSHTANKS; i++ ) 
		{
			if( m_aTanks[i].m_bActive ) 
			{
				m_bSystemActive = TRUE;
				break;
			}
		}
	}
}



// Assumes the FDraw renderer is currently active.
// Assumes the game's main perspective camera is
// set up. Assumes there are no model Xfms on the
// stack.
//
// Does not preserve the current FDraw state.
// Does (not) preserve the current viewport.
// Will preserve the Xfm stack.
// Will preserve the frenderer fog state.

void CSloshTank::DrawAll( void )
{
	if ( !m_bSystemInitialized)
	{
		return;
	}

	FASSERT( m_bSystemInitialized );
	
	if( !m_bSystemActive )
	{
		return;
	}

	// fdraw_SetCullDir( FDRAW_CULLDIR_NONE );
	// fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	// fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER );
	
	// fdraw_Depth_EnableWriting( FALSE );

	BOOL bRenderStateSet = FALSE;

	for( u32 i=0; i<_MAX_SLOSHTANKS; i++ ) 
	{
		if( m_aTanks[i].m_bActive ) 
		{
			if( !bRenderStateSet )
			{
				bRenderStateSet = TRUE;

				frenderer_Push( FRENDERER_DRAW, NULL );

				fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
				fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
				fdraw_SetCullDir( FDRAW_CULLDIR_CW );
			}

			m_aTanks[i]._Draw();
		}
	}

	if( bRenderStateSet )
	{
		frenderer_Pop();	
	}
}



CSloshTank::CSloshTank()
{
	m_bActive	= FALSE;
}




BOOL CSloshTank::_IsOffscreen( void )
{
	if( !m_bActive )
	{
		return TRUE;
	}

	//check vs. viewing frustrum
//	CFSphere sphTemp;
//	sphTemp.m_Pos		= m_vPos;
//	sphTemp.m_fRadius	= m_fSize * 0.5f;
//	if( fviewport_TestSphere_WS( CPlayer::m_pCurrent->m_pViewportPersp3D, &sphTemp, FVIEWPORT_PLANESMASK_ALL ) != -1 )
//	{
		// in current frustrum
//		return FALSE; // not offscreen
//	}
    return TRUE;
}

void CSloshTank::UpdateTank( f32 fTankPercent )
{
	if (m_eTankState == TR_STATE_ACTIVE)
	{
		m_fTankFillPercentHeight = fTankPercent;
	}
	else if (m_eTankState == TR_STATE_REFILLING)
	{
		m_fTankFillPercentHeight += m_SloshTankProps.fRefillRate * FLoop_fPreviousLoopSecs;
		if (m_fTankFillPercentHeight >= fTankPercent)
		{
			m_fTankFillPercentHeight = fTankPercent;
			m_eTankState = TR_STATE_ACTIVE;
		}
	}
}

void CSloshTank::_Draw( void )
{
	// verlet computation
	CFVec3A vGravity;
	vGravity.Set( m_SloshTankProps.fVerletPendentGravityX, m_SloshTankProps.fVerletPendentGravityY, m_SloshTankProps.fVerletPendentGravityZ);
	// Do the verlet integration
	CFVec3A CurrTemp = m_vPendentVerletCurr;
	CFVec3A OldToNew;
	CFQuatA RotQuat;
	f32 fDelta = FLoop_fPreviousLoopSecs;
	FMATH_CLAMP(fDelta,0.005f,0.033f);

	vGravity.Mul( ( fDelta * fDelta ) );

	// Dampening
	OldToNew.Sub( m_vPendentVerletCurr, m_vPendentVerletPrev ).Mul( m_SloshTankProps.fVerletDampeningConstant * fDelta );
	m_vPendentVerletPrev.Add( OldToNew );

	CurrTemp.Mul( 2.0f );
	CurrTemp.Sub( m_vPendentVerletPrev );
	CurrTemp.Add( vGravity );

	CFVerlet::Constraint_AnchorDistance( &CurrTemp, &m_pmtxFluid_WS->m_vPos, m_SloshTankProps.fVerletPendentLength );
	
	m_vPendentVerletPrev = m_vPendentVerletCurr;
	m_vPendentVerletCurr = CurrTemp;

	CFVec3A vPtOnPlane;
	f32 fPtDistanceToSurface = m_fTankHeight*m_fTankFillPercentHeight;
	vPtOnPlane.Mul(m_pmtxFluid_WS->m_vUp,fPtDistanceToSurface);
	vPtOnPlane.Add( m_pmtxFluid_WS->m_vPos);

	CFVec3A vPlaneNormal; 
	vPlaneNormal.Sub(m_pmtxFluid_WS->m_vPos,m_vPendentVerletCurr);
	vPlaneNormal.Unitize();
	
	FDrawVtx_t* pTankVertsWS = fvtxpool_GetArray(_SLOSHTANK_NUM_VERTS);
	_TriFace_t* pTankFacesWS = (_TriFace_t*)fvtxpool_GetArray(_SLOSHTANK_NUM_FACES*3);
	
	fdraw_TransformVerts(pTankVertsWS, m_aTankVertsMS, *m_pmtxFluid_WS, _SLOSHTANK_NUM_BASEVERTS);

	CFVec3A vWorkTemp;
	f32 fDistanceToCapPlane;
	for (u32 i = 0; i < _SLOSHTANK_NUM_VERTS; i++)
	{
		if (i >=_SLOSHTANK_NUM_BASEVERTS)
		{
			vWorkTemp.Sub(pTankVertsWS[i-_SLOSHTANK_NUM_BASEVERTS].Pos_MS,vPtOnPlane);
			f32 fNumerator = -vWorkTemp.Dot(vPlaneNormal);
			f32 fDenominator = m_pmtxFluid_WS->m_vUp.Dot(vPlaneNormal);

			fDistanceToCapPlane = fmath_Div(fNumerator,fDenominator);

			FMATH_CLAMP(fDistanceToCapPlane,0.0f,m_fTankHeight);
			vWorkTemp.Mul(m_pmtxFluid_WS->m_vUp, fDistanceToCapPlane);

			pTankVertsWS[i].Pos_MS.x = pTankVertsWS[i-_SLOSHTANK_NUM_BASEVERTS].Pos_MS.x + vWorkTemp.x;
			pTankVertsWS[i].Pos_MS.y = pTankVertsWS[i-_SLOSHTANK_NUM_BASEVERTS].Pos_MS.y + vWorkTemp.y;
			pTankVertsWS[i].Pos_MS.z = pTankVertsWS[i-_SLOSHTANK_NUM_BASEVERTS].Pos_MS.z + vWorkTemp.z;
		}
		pTankVertsWS[i].ColorRGBA = m_aTankVertsMS[i].ColorRGBA;
		pTankVertsWS[i].ST.x = m_aTankVertsMS[i].ST.x - m_fSOffset;
		pTankVertsWS[i].ST.y = fPtDistanceToSurface * m_aTankVertsMS[i].ST.y - m_fTOffset;
	}
	
	if (fmath_RandomChance(m_SloshTankProps.fTextureChangeProbabilityX))
	{
		m_fSDir = fmath_RandomFloatRange(-m_SloshTankProps.fTextureTravelMaxX,m_SloshTankProps.fTextureTravelMaxX);
	}
	f32 fSOffset = fmath_RandomFloatRange(0.f,m_SloshTankProps.fTextureVelocityX);
	
	if (m_fSDir > 0.0f)
		m_fSOffset = m_fSOffset+fSOffset;
	else
		m_fSOffset = m_fSOffset-fSOffset;

	if (m_fSOffset > 1.0f)
		m_fSOffset=0.0f;
	if (m_fSOffset < 0.0f)
		m_fSOffset=1.0f;

	if (fmath_RandomChance(m_SloshTankProps.fTextureChangeProbabilityY))
	{
		m_fTDir = fmath_RandomFloatRange(-m_SloshTankProps.fTextureTravelMaxY,m_SloshTankProps.fTextureTravelMaxY);
	}
	f32 fTOffset = fmath_RandomFloatRange(0.f,m_SloshTankProps.fTextureVelocityY);

	if (m_fTDir > 0.0f)
		m_fTOffset = m_fTOffset+fTOffset;
	else
		m_fTOffset = m_fTOffset-fTOffset;

	if (m_fTOffset > 1.0f)
		m_fTOffset=0.0f;
	if (m_fTOffset < 0.0f)
		m_fTOffset=1.0f;
	
	
	pTankFacesWS[ 0].Set(pTankVertsWS[5 ], pTankVertsWS[12], pTankVertsWS[11]);
	pTankFacesWS[ 1].Set(pTankVertsWS[5 ], pTankVertsWS[11], pTankVertsWS[4 ]);
	pTankFacesWS[ 2].Set(pTankVertsWS[4 ], pTankVertsWS[11], pTankVertsWS[9 ]);
	pTankFacesWS[ 3].Set(pTankVertsWS[4 ], pTankVertsWS[9 ], pTankVertsWS[2 ]);
	pTankFacesWS[ 4].Set(pTankVertsWS[2 ], pTankVertsWS[9 ], pTankVertsWS[7 ]);
	pTankFacesWS[ 5].Set(pTankVertsWS[2 ], pTankVertsWS[7 ], pTankVertsWS[0 ]);
	pTankFacesWS[ 6].Set(pTankVertsWS[0 ], pTankVertsWS[7 ], pTankVertsWS[8 ]);
	pTankFacesWS[ 7].Set(pTankVertsWS[0 ], pTankVertsWS[8 ], pTankVertsWS[ 1]);
	pTankFacesWS[ 8].Set(pTankVertsWS[ 1], pTankVertsWS[8 ], pTankVertsWS[10]);
	pTankFacesWS[ 9].Set(pTankVertsWS[ 1], pTankVertsWS[10], pTankVertsWS[3 ]);
	pTankFacesWS[10].Set(pTankVertsWS[3 ], pTankVertsWS[10], pTankVertsWS[13]);
	pTankFacesWS[11].Set(pTankVertsWS[3 ], pTankVertsWS[13], pTankVertsWS[6]);
	pTankFacesWS[12].Set(pTankVertsWS[6 ], pTankVertsWS[13], pTankVertsWS[12]);
	pTankFacesWS[13].Set(pTankVertsWS[6 ], pTankVertsWS[12], pTankVertsWS[5 ]);

	pTankFacesWS[14].Set(pTankVertsWS[13], pTankVertsWS[10], pTankVertsWS[11],m_aTankVertsMS[13].ST,m_aTankVertsMS[10].ST,m_aTankVertsMS[11].ST);
	pTankFacesWS[15].Set(pTankVertsWS[11], pTankVertsWS[12], pTankVertsWS[13],m_aTankVertsMS[11].ST,m_aTankVertsMS[12].ST,m_aTankVertsMS[13].ST);
	pTankFacesWS[16].Set(pTankVertsWS[9 ], pTankVertsWS[11], pTankVertsWS[10],m_aTankVertsMS[9 ].ST,m_aTankVertsMS[11].ST,m_aTankVertsMS[10].ST);
	pTankFacesWS[17].Set(pTankVertsWS[9 ], pTankVertsWS[8 ], pTankVertsWS[7 ],m_aTankVertsMS[9 ].ST,m_aTankVertsMS[8].ST,m_aTankVertsMS[7 ].ST);
	pTankFacesWS[18].Set(pTankVertsWS[8 ], pTankVertsWS[9 ], pTankVertsWS[10],m_aTankVertsMS[8 ].ST,m_aTankVertsMS[9].ST,m_aTankVertsMS[10].ST);
	pTankFacesWS[14].SetAlpha(m_SloshTankProps.fAlphaFluidSurface);
	pTankFacesWS[15].SetAlpha(m_SloshTankProps.fAlphaFluidSurface);
	pTankFacesWS[16].SetAlpha(m_SloshTankProps.fAlphaFluidSurface);
	pTankFacesWS[17].SetAlpha(m_SloshTankProps.fAlphaFluidSurface);
	pTankFacesWS[18].SetAlpha(m_SloshTankProps.fAlphaFluidSurface);

	FDrawVtx_t* pCardVertsWS = fvtxpool_GetArray(_SLOSHTANK_NUM_CARDVERTS);
	_TriFace_t* pTankCardFacesWS = (_TriFace_t*)fvtxpool_GetArray(_SLOSHTANK_NUM_CARDFACES*3);
	
	pCardVertsWS[0].Pos_MS.x = pTankVertsWS[1].Pos_MS.x; // -.53 (ms)
	pCardVertsWS[0].Pos_MS.y = pTankVertsWS[1].Pos_MS.y; // 0 (ms)
	pCardVertsWS[0].Pos_MS.z = pTankVertsWS[1].Pos_MS.z; // (0.22

	pCardVertsWS[1].Pos_MS.x = pTankVertsWS[3].Pos_MS.x; 
	pCardVertsWS[1].Pos_MS.y = pTankVertsWS[3].Pos_MS.y; 
	pCardVertsWS[1].Pos_MS.z = pTankVertsWS[3].Pos_MS.z; 

	pCardVertsWS[2].Pos_MS.x = pTankVertsWS[8].Pos_MS.x;
	pCardVertsWS[2].Pos_MS.y = pTankVertsWS[8].Pos_MS.y;
	pCardVertsWS[2].Pos_MS.z = pTankVertsWS[8].Pos_MS.z;

	pCardVertsWS[3].Pos_MS.x = pTankVertsWS[10].Pos_MS.x; 
	pCardVertsWS[3].Pos_MS.y = pTankVertsWS[10].Pos_MS.y; 
	pCardVertsWS[3].Pos_MS.z = pTankVertsWS[10].Pos_MS.z; 

	pCardVertsWS[4].Pos_MS.x = pTankVertsWS[6].Pos_MS.x; // .53 (ms)
	pCardVertsWS[4].Pos_MS.y = pTankVertsWS[6].Pos_MS.y; // 0 (ms)
	pCardVertsWS[4].Pos_MS.z = pTankVertsWS[6].Pos_MS.z;

	pCardVertsWS[5].Pos_MS.x = pTankVertsWS[13].Pos_MS.x;
	pCardVertsWS[5].Pos_MS.y = pTankVertsWS[13].Pos_MS.y;
	pCardVertsWS[5].Pos_MS.z = pTankVertsWS[13].Pos_MS.z;

	CFColorRGBA ColorRGBA;
	ColorRGBA.Set(1.0f,1.0f,1.0f,m_SloshTankProps.fAlphaFluidCard);

	pCardVertsWS[0].ST.x = 0.0f;
	pCardVertsWS[0].ST.y = 1.0f;
	pCardVertsWS[0].ColorRGBA = ColorRGBA;

	pCardVertsWS[1].ST.x = 0.5f;
	pCardVertsWS[1].ST.y = 1.0f;
	pCardVertsWS[1].ColorRGBA = ColorRGBA;

	pCardVertsWS[2].ST.x = 0.0f;
	pCardVertsWS[2].ST.y = 0.0f;
	pCardVertsWS[2].ColorRGBA = ColorRGBA;

	pCardVertsWS[3].ST.x = 0.5f;
	pCardVertsWS[3].ST.y = 0.0f;
	pCardVertsWS[3].ColorRGBA = ColorRGBA;

	pCardVertsWS[4].ST.x = 1.0f;
	pCardVertsWS[4].ST.y = 1.0f;
	pCardVertsWS[4].ColorRGBA = ColorRGBA;

	pCardVertsWS[5].ST.x = 1.0f;
	pCardVertsWS[5].ST.y = 0.0f;
	pCardVertsWS[5].ColorRGBA = ColorRGBA;

	pTankCardFacesWS[0].Set(pCardVertsWS[0],pCardVertsWS[1],pCardVertsWS[2]);
	pTankCardFacesWS[1].Set(pCardVertsWS[2],pCardVertsWS[1],pCardVertsWS[3]);
	pTankCardFacesWS[2].Set(pCardVertsWS[3],pCardVertsWS[1],pCardVertsWS[4]);
	pTankCardFacesWS[3].Set(pCardVertsWS[4],pCardVertsWS[5],pCardVertsWS[3]);

	fdraw_SetTexture( &m_aTextures[1] ); // the card in the tank
	fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, pTankCardFacesWS[0], (_SLOSHTANK_NUM_CARDFACES * 3) );
	
	fdraw_SetTexture( &m_aTextures[0] ); // the sides of the fluid
	fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, pTankFacesWS[0], 14 * 3 );

	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AI ); // the top of the fluid
	fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, pTankFacesWS[14], 5 * 3 );

	fvtxpool_ReturnArray(*pTankFacesWS);
	fvtxpool_ReturnArray(*pTankCardFacesWS);
	fvtxpool_ReturnArray(pTankVertsWS);
	fvtxpool_ReturnArray(pCardVertsWS);
}