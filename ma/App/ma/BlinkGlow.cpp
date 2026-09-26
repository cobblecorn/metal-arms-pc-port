//////////////////////////////////////////////////////////////////////////////////////
// BlinkGlow.cpp - 
//
// Author: Justin Link      
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
// 07/31/02 Link        Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "BlinkGlow.h"

#include "fvtxpool.h"
#include "FkDOP.h"
#include "botglitch.h"

//====================
// private definitions

//=================
// public variables

//==================
// private variables

#define _NUM_GLOW	( 4 )

CBlinkGlow *CBlinkGlow::m_apGlowPool = NULL;
u32 CBlinkGlow::m_nNextInPool = -1;
BOOL CBlinkGlow::m_bSystemInitted = FALSE;

const FkDOP_Geo_t *CBlinkGlow::s_apGeo[CBlinkGlow::DOPBONE_COUNT];
cchar *CBlinkGlow::s_apszBoneNames[CBlinkGlow::DOPBONE_COUNT] = { "R_Leg_Lower", "L_Leg_Lower", "R_Heel", "L_Heel" };
const f32 CBlinkGlow::s_afScale[CBlinkGlow::DOPBONE_COUNT] = { 1.01f, 1.01f, 1.01f, 1.01f };

static const f32 _fActivateTime = 0.5f;

static const f32 _fOOActivateTime = 1.0f / _fActivateTime;

//===================
// private prototypes

//=================
// public functions
BOOL CBlinkGlow::InitSystem()
{
	s32 nIndex;
	FASSERT(!m_bSystemInitted);
	m_bSystemInitted = TRUE;

	m_apGlowPool = fnew CBlinkGlow[_NUM_GLOW];
	if(m_apGlowPool == NULL)
	{
		DEVPRINTF("CBlinkGlow::InitSystem() : Out of memory.\n");
		return(FALSE);
	}

	for( nIndex = 0; nIndex < _NUM_GLOW; nIndex++ )
	{
		m_apGlowPool[nIndex].m_bAllocated = FALSE;
	}

	return(TRUE);
}


void CBlinkGlow::UninitSystem()
{
	fdelete_array(m_apGlowPool);
	m_apGlowPool = NULL;

	m_bSystemInitted = FALSE;
}



BOOL CBlinkGlow::InitLevel()
{
	s32 nIndex;
	FASSERT(m_bSystemInitted);

	for( nIndex = 0; nIndex < _NUM_GLOW; nIndex++ )
	{
		m_apGlowPool[nIndex].m_bAllocated = FALSE;
	}

	return(TRUE);
}



void CBlinkGlow::UninitLevel()
{
	FASSERT(m_bSystemInitted);
	m_nNextInPool = 0;
}


CBlinkGlow *CBlinkGlow::GetGlow()
{
	s32 nIndex;
	FASSERT(m_bSystemInitted);

	for( nIndex = 0; nIndex < _NUM_GLOW; nIndex++ )
	{
		if( !m_apGlowPool[nIndex].m_bAllocated )
		{
			m_apGlowPool[nIndex].m_bAllocated = TRUE;
			return(&m_apGlowPool[nIndex]);
		}
	}

	return(NULL);
}

// return a CBlinkShell object (acquired with GetShell()) to the pool.
// returns TRUE if successful, FALSE otherwise.
BOOL CBlinkGlow::ReturnGlow( void )
{
	s32 nIndex;

	for( nIndex = 0; nIndex < _NUM_GLOW; nIndex++ )
	{
		if( this == &m_apGlowPool[nIndex] )
		{
			m_bAllocated = FALSE;
			return TRUE;
		}
	}

	return FALSE;
}


void CBlinkGlow::Work()
{
	FASSERT(m_bSystemInitted);

	u32 uGlowIdx;
	for(uGlowIdx = 0; uGlowIdx < _NUM_GLOW; ++uGlowIdx)
	{
		if( m_apGlowPool[uGlowIdx].m_bAllocated )
		{
			m_apGlowPool[uGlowIdx]._Work();
		}
	}
}



void CBlinkGlow::FDraw()
{
	FASSERT(m_bSystemInitted);

	fdraw_Color_SetFunc(FDRAW_COLORFUNC_DECAL_AI);
	fdraw_Alpha_SetBlendOp(FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST);

	u32 uGlowIdx;
	for(uGlowIdx = 0; uGlowIdx < _NUM_GLOW; ++uGlowIdx)
	{
		if( m_apGlowPool[uGlowIdx].m_bAllocated )
		{
			m_apGlowPool[uGlowIdx]._FDraw();
		}
	}
}


void CBlinkGlow::Activate()
{
	if( !( m_uFlags & FLAG_ACTIVATED ) ) {
		// Only get the kDOP Geo when we first start up
		m_uFlags |= FLAG_ACTIVATED;

		u32 uDOPIdx;
		for (uDOPIdx = 0; uDOPIdx < DOPBONE_COUNT; uDOPIdx++ )
		{
			s_apGeo[uDOPIdx] = m_pBot->m_pWorldMesh->GetkDOPGeo(m_pBot->m_pWorldMesh->FindBone(s_apszBoneNames[uDOPIdx]));
		}
	}

	m_eState = STATE_ACTIVATING;
	m_fStateTimer = 0.0f;

	m_fUnitEffectEnd = 0.0f;
}



void CBlinkGlow::SetColor(f32 fRed, f32 fGreen, f32 fBlue, f32 fAlpha1, f32 fAlpha2)
{
	m_rgbColor.Set(fRed, fGreen, fBlue);
	m_afAlpha[0] = fAlpha1;
	m_afAlpha[1] = fAlpha2;
}



void CBlinkGlow::EnableDOPBone(DopBone_e eDOPBone)
{
	m_uDOPFlags |= (1 << eDOPBone);
}



void CBlinkGlow::DisableDOPBone(DopBone_e eDOPBone)
{
	m_uDOPFlags &= ~(1 << eDOPBone);
}



void CBlinkGlow::SetUnitEffectEnd(f32 fUnitEffectEnd)
{
	m_fUnitEffectEnd = fUnitEffectEnd;
}



void CBlinkGlow::Deactivate()
{
	m_uFlags &= ~(FLAG_ACTIVATED);
}


//==================
// private functions


CBlinkGlow::CBlinkGlow()
{
}



CBlinkGlow::~CBlinkGlow()
{
}



void CBlinkGlow::_Work()
{
	switch(m_eState)
	{
		case STATE_ACTIVATING:
		{
			m_fStateTimer += FLoop_fPreviousLoopSecs;
			if(m_fStateTimer > _fActivateTime)
			{
				m_eState = STATE_ACTIVE;
			}
			break;
		}
		case STATE_ACTIVE:
		{
			break;
		}
	}
}



void CBlinkGlow::_FDraw()
{
	if(!(m_uFlags & FLAG_ACTIVATED))
	{
		return;
	}

	if((m_fUnitEffectEnd > 0.0f) && (fmath_RandomChance(m_fUnitEffectEnd)))
	{
		return;
	}

	FDrawVtx_t *pavtxPool = fvtxpool_GetArray( 1250 );
	u32 uCurVtx = 0;

	u32 uDOPIdx, uFaceIdx, uVrtIdx;

	FASSERT(m_pBot != NULL);
	CFMtx43A **paBoneMtx = m_pBot->m_pWorldMesh->GetBoneMtxPalette();
	CFMtx43A mtxCurBone;
	CFVec3A vecBlinkFront = m_pBot->m_MountUnitFrontXZ_WS;

	//	static const f32 _afAlphas[5] = { 0.15f, 0.10f, 0.07f, 0.04f, 0.01f };

	// JUSTIN: Optimization: Only set the colors and STs once outside the loop since I reuse the vertex list.
	for (uDOPIdx = 0; uDOPIdx < DOPBONE_COUNT; ++uDOPIdx)
	{
		if(s_apGeo[uDOPIdx] != NULL)
		{
			u32 uCloneIdx;
			for(uCloneIdx = 0; uCloneIdx < 5; ++uCloneIdx)
			{
				// This needs to correspond to the number of clones.
				f32 fExtraScale = (f32)(uCloneIdx) * (10.0f / 5.0f) * 0.10f * fmath_RandomFloatRange(0.8f, 1.0f);

				mtxCurBone = *paBoneMtx[s_apGeo[uDOPIdx]->nBoneIdx];
				mtxCurBone.m_vX.Mul(0.8f + fExtraScale);
				mtxCurBone.m_vY.Mul(0.8f + fExtraScale);
				mtxCurBone.m_vZ.Mul(0.8f + fExtraScale);

				// This needs to correspond to the number of clones.
				f32 fGlobalAlpha = 0.15f - (0.15f / 5.0f) * (f32)(uCloneIdx);

				if(m_eState == STATE_ACTIVATING)
				{
					f32 fWeight1 = m_fStateTimer * _fOOActivateTime;
					f32 fWeight2 = 1.0f - fWeight1;
					fGlobalAlpha = fWeight1 * fGlobalAlpha + fWeight2 * 0.30f;
				}
				// This can get moved outside the loop.
//				f32 fGlobalAlpha = _afAlphas[uCloneIdx];

				for (uFaceIdx = 0; uFaceIdx < s_apGeo[uDOPIdx]->nFaceCount; ++uFaceIdx)
				{
					for (uVrtIdx = 2; uVrtIdx < s_apGeo[uDOPIdx]->aFaces[uFaceIdx].nVertCount; ++uVrtIdx)
					{
						if(1250 - uCurVtx < 3)
						{
							// Draw the triangle list
							fdraw_PrimList(FDRAW_PRIMTYPE_TRILIST, pavtxPool, uCurVtx);
							uCurVtx = 0;
						}

						//////////////////////////////////////////////////////////////////////
						//
						_AddToVtxList(pavtxPool, uCurVtx, uDOPIdx, uFaceIdx, 0, &mtxCurBone, vecBlinkFront, fGlobalAlpha, m_rgbColor);
						_AddToVtxList(pavtxPool, uCurVtx, uDOPIdx, uFaceIdx, uVrtIdx - 1, &mtxCurBone, vecBlinkFront, fGlobalAlpha, m_rgbColor);
						_AddToVtxList(pavtxPool, uCurVtx, uDOPIdx, uFaceIdx, uVrtIdx, &mtxCurBone, vecBlinkFront, fGlobalAlpha, m_rgbColor);
						//
						//////////////////////////////////////////////////////////////////////
					}
				}
			}
		}
	}

	// Draw the triangle list
	fdraw_PrimList(FDRAW_PRIMTYPE_TRILIST, pavtxPool, uCurVtx);

	fvtxpool_ReturnArray(pavtxPool);
}




void CBlinkGlow::_AddToVtxList(FDrawVtx_t *pVtxArray, u32 &uCurVtx, u32 uDOPIdx, u32 uFaceIdx, u32 uVrtIdx, CFMtx43A *pBS2WS, CFVec3A &vecBlinkFront, f32 fGlobalAlpha, CFColorRGB &rgbBase)
{
	static CFVec3A vecNewVert_WS;
	static CFVec3A vecVertMinusBoneOrigin;
	static CFVec3A vecTemp;

	//////////////////////////////////////////////////////////////////////
	//
	vecNewVert_WS.v3 = s_apGeo[uDOPIdx]->aVerts[s_apGeo[uDOPIdx]->aFaces[uFaceIdx].anVertIdx[uVrtIdx]];
	pBS2WS->MulPoint(vecNewVert_WS);
	vecVertMinusBoneOrigin = vecNewVert_WS;
	vecVertMinusBoneOrigin.Sub(pBS2WS->m_vPos);
	vecVertMinusBoneOrigin.Unitize();
	// JUSTIN: I changed these constants.
	f32 fDot = vecBlinkFront.Dot(vecVertMinusBoneOrigin);
//	f32 fAlphaMult = 0.50f + 0.50f * FMATH_FABS(vecBlinkFront.Dot(vecVertMinusBoneOrigin));
	f32 fAlphaMult = 0.50f + 0.50f * fDot * fDot;
	f32 fAlphaFinal = fAlphaMult * fGlobalAlpha;

	static const f32 _fMaxAlpha = 0.75f;

	pVtxArray[uCurVtx].ColorRGBA.ColorRGB = rgbBase;
	pVtxArray[uCurVtx].ColorRGBA.fAlpha = fAlphaFinal;
	pVtxArray[uCurVtx].Pos_MS = vecNewVert_WS.v3;
	pVtxArray[uCurVtx++].ST.Set(0.0f, 0.0f);
	//
	//////////////////////////////////////////////////////////////////////
}