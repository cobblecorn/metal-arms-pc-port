//////////////////////////////////////////////////////////////////////////////////////
// BlinkShell.cpp - 
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
// 07/12/02 Link        Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "BlinkShell.h"

#include "FkDOP.h"
#include "fresload.h"
#include "botglitch.h"
#include "fvtxpool.h"
#include "botglitch.h"

//====================
// private definitions

enum _DopBones_e
{
	DOPBONE_RLEGLOWER	=	0,
	DOPBONE_LLEGLOWER,
	DOPBONE_GROIN,
	DOPBONE_BEDROLL,
	DOPBONE_BACKPACK,
	DOPBONE_TORSO,
	DOPBONE_RARMUPPER,
	DOPBONE_LARMUPPER,
	DOPBONE_RARMLOWER,
	DOPBONE_LARMLOWER,
	DOPBONE_LARMELBOW,
	DOPBONE_LHAND,
	DOPBONE_RSHOULDER,
	DOPBONE_LSHOULDER,
	DOPBONE_HEAD,

	DOPBONE_COUNT
};

//=================
// public variables

//==================
// private variables

#define _NUM_SHELLS		( 4 )

static const u32 _KDop_BufferSize = DOPBONE_COUNT;

CBlinkShell *CBlinkShell::m_apShellPool = NULL;
BOOL CBlinkShell::m_bSystemInitted = FALSE;
CFTexInst CBlinkShell::s_oTexInst;

static const FkDOP_Geo_t *_apGeo[_KDop_BufferSize];

//static cchar *_apszBoneNames[DOPBONE_COUNT] = { "Torso", "Backpack", "Head", "R_Shoulder", "L_Shoulder", "BedRoll", "R_Leg_Lower", "L_Leg_Lower", "Groin", "R_Arm_Upper", "L_Arm_Upper", "R_Arm_Lower", "L_Arm_Lower", "L_Arm_Elbow", "L_Hand"/*, "L_ThumbB", "L_IndexFingerB", "L_MiddleFingerB", "L_PinkyFingerB"*/ };
//static cchar *_apszBoneNames[DOPBONE_COUNT] = { "R_Leg_Lower", "L_Leg_Lower", "Groin", "BedRoll", "Backpack", "Torso", "R_Arm_Upper", "L_Arm_Upper", "R_Arm_Lower", "L_Arm_Lower", "L_Arm_Elbow", "L_Hand", "R_Shoulder", "L_Shoulder", "Head"/*, "L_ThumbB", "L_IndexFingerB", "L_MiddleFingerB", "L_PinkyFingerB"*/ };
//static cchar *_apszBoneNames[DOPBONE_COUNT] = { "R_Leg_Lower", "Groin", "L_Leg_Lower", "BedRoll", "Torso", "Backpack", "R_Shoulder", "R_Arm_Upper", "R_Arm_Lower", "L_Shoulder", "L_Arm_Upper", "L_Arm_Lower", "L_Arm_Elbow", "L_Hand", "Head"/*, "L_ThumbB", "L_IndexFingerB", "L_MiddleFingerB", "L_PinkyFingerB"*/ };

static cchar *_apszBoneNames[DOPBONE_COUNT] = { 
	"R_Leg_Lower", "Groin", "L_Leg_Lower", "R_Leg_Upper", "Torso", "Backpack", 
	"R_Shoulder", "R_Arm_Elbow", "R_Arm_Lower", 
	"L_Leg_Upper", "L_Arm_Upper", "L_Arm_Lower", "L_Arm_Elbow", "L_Hand", "Head"
	/*, "L_ThumbB", "L_IndexFingerB", "L_MiddleFingerB", "L_PinkyFingerB"*/ 
};
static const f32 _afScale[DOPBONE_COUNT] = { 1.01f, 1.01f, 1.01f, 1.01f, 1.01f, 1.01f, 1.01f, 1.01f, 1.01f, 1.01f, 1.01f, 1.49f, 1.01f, 1.01f, 1.01f/*, 1.01f, 1.01f, 1.01f, 1.01f*/ };

static const f32 _fHitRadInner = 0.6f;//1.0f;//0.5f;//1.0f;
static const f32 _fHitRadOuter = 1.6f;//1.5f;//1.5f;//3.0f;
static const f32 _fHitFadeOutTime = 0.5f;

static const f32 _fActivateTime = 0.8f;//2.0f;//0.5f;//3.0f;
static const f32 _fGlowMoveTime = 0.8f;//2.0f;//0.5f;			// This needs to be less than the above.

static const f32 _fHitRadInnerSq = _fHitRadInner * _fHitRadInner;
static const f32 _fHitRadOuterSq = _fHitRadOuter * _fHitRadOuter;
static const f32 _fOOHitFadeOutTime = 1.0f / _fHitFadeOutTime;

static const f32 _fOOActivateTime = 1.0f / _fActivateTime;
static const f32 _fGlowMoveRate = (f32)(DOPBONE_COUNT) / _fGlowMoveTime;

//===================
// private prototypes

//=================
// public static functions

BOOL CBlinkShell::InitSystem()
{
	s32 nIndex;
	FASSERT(!m_bSystemInitted);
	m_bSystemInitted = TRUE;

	m_apShellPool = fnew CBlinkShell[_NUM_SHELLS];
	if(m_apShellPool == NULL)
	{
		DEVPRINTF("CBlinkShell::InitSystem() : Out of memory.\n");
		return(FALSE);
	}

	s_oTexInst.SetTexDef((FTexDef_t *)(fresload_Load(FTEX_RESNAME, "TF_1shield1")));
	s_oTexInst.SetFlags(CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T);

	for( nIndex = 0; nIndex < _NUM_SHELLS; nIndex++ )
	{
		m_apShellPool[nIndex].m_bAllocated = FALSE;
	}

	return(TRUE);
}



void CBlinkShell::UninitSystem()
{
	fdelete_array(m_apShellPool);
	m_apShellPool = NULL;

	m_bSystemInitted = FALSE;
}



BOOL CBlinkShell::InitLevel()
{
	s32 nIndex;
	FASSERT(m_bSystemInitted);

	for( nIndex = 0; nIndex < _NUM_SHELLS; nIndex++ )
	{
		m_apShellPool[nIndex].m_bAllocated = FALSE;
	}

	return(TRUE);
}



void CBlinkShell::UninitLevel()
{
	FASSERT(m_bSystemInitted);
}


// static function to request a pointer to a CBlinkShell object from the pool.
// returns NULL if no object is available.
CBlinkShell *CBlinkShell::GetShell()
{
	s32 nIndex;

	FASSERT(m_bSystemInitted);

	for( nIndex = 0; nIndex < _NUM_SHELLS; nIndex++ )
	{
		if( !m_apShellPool[nIndex].m_bAllocated )
		{
			m_apShellPool[nIndex].m_bAllocated = TRUE;
			return(&m_apShellPool[nIndex]);
		}
	}

	return(NULL);
}

// return a CBlinkShell object (acquired with GetShell()) to the pool.
// returns TRUE if successful, FALSE otherwise.
BOOL CBlinkShell::ReturnShell( void )
{
	s32 nIndex;

	for( nIndex = 0; nIndex < _NUM_SHELLS; nIndex++ )
	{
		if( this == &m_apShellPool[nIndex] )
		{
			m_bAllocated = FALSE;
			return TRUE;
		}
	}

	return FALSE;
}

void CBlinkShell::Work()
{
	FASSERT(m_bSystemInitted);

	u32 uShellIdx;
	for(uShellIdx = 0; uShellIdx < _NUM_SHELLS; ++uShellIdx)
	{
		if( m_apShellPool[uShellIdx].m_bAllocated )
		{
			m_apShellPool[uShellIdx]._Work();
		}
	}
}



void CBlinkShell::FDraw()
{
	FASSERT(m_bSystemInitted);

//#define METHOD1

#ifdef METHOD1
//	fdraw_Color_SetFunc(FDRAW_COLORFUNC_DIFFUSETEX_AIAT);
//	fdraw_Alpha_SetBlendOp(FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST);
#else
	fdraw_Color_SetFunc(FDRAW_COLORFUNC_BLEND_AIPLUSAT);
	fdraw_Alpha_SetBlendOp(FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE);
#endif

	FDrawTexCoordXfmMode_e eOldTexCoordXfmMode = fdraw_GetTexCoordXfmMode();
	fdraw_SetTexCoordXfmMode(FDRAW_TEXCOORDXFMMODE_CAMSPACE_POS);

	fdraw_SetTexture(&s_oTexInst);

	u32 uShellIdx;
	for(uShellIdx = 0; uShellIdx < _NUM_SHELLS; ++uShellIdx)
	{
		if( m_apShellPool[uShellIdx].m_bAllocated )
		{
			fdraw_SetTexCoordXfmMtx(&FXfm_Identity.m_MtxF);
			m_apShellPool[uShellIdx]._FDraw();
		}
	}

	fdraw_SetTexCoordXfmMtx(&FXfm_Identity.m_MtxF);
	fdraw_SetTexCoordXfmMode(eOldTexCoordXfmMode);
}



//=================
// public functions

void CBlinkShell::Activate()
{
	if((m_uFlags & FLAG_ACTIVATED) == 0)
	{
		m_uFlags |= FLAG_ACTIVATED;

		u32 uDOPIdx;
		for (uDOPIdx = 0; uDOPIdx < DOPBONE_COUNT; uDOPIdx++ )
		{
			_apGeo[uDOPIdx] = m_pBot->m_pWorldMesh->GetkDOPGeo(m_pBot->m_pWorldMesh->FindBone(_apszBoneNames[uDOPIdx]));
			FASSERT(_apGeo[uDOPIdx] != NULL);
		}

		m_eState = STATE_ACTIVATING;
		m_fStateTimer = 0.0f;

		m_fHotSpotFactor = 0.0f;

		m_bAlreadyGlowed = FALSE;
		m_fDOPGlowPos = 0.0f;

		m_fUnitEffectEnd = 0.0f;
	}
}


#if 0
void CBlinkShell::BotGotHit(CDamageInfo *pDamageInfo)
{
	// SER: Fix this
	CFVec3A pkDOPCollision; // Filled in with the collision info
	s32 nBoneIdx = pDamageInfo->m_nBoneIdx;

	CFWorldMesh *pMesh = m_pBot->m_pWorldMesh;
	CFVec3A vecNewDir = pDamageInfo->m_vecDir_WS;
	vecNewDir.Mul(-1.0f);

	if(pMesh->ConvertMeshCollTokDOP(&pDamageInfo->m_vecPos_WS, &vecNewDir, &nBoneIdx, &m_vecHitPos_WS, &m_nHitFaceIdx))
	{
		FASSERT(nBoneIdx != -1);
		m_nHitBoneIdx = nBoneIdx;
		CFMtx43A **paBoneMtx = m_pBot->m_pWorldMesh->GetBoneMtxPalette();
		CFMtx43A mtxWorldToBone = *paBoneMtx[nBoneIdx];
		mtxWorldToBone.Invert();
		m_vecHitPos_BS.w = 0.0f;
		mtxWorldToBone.MulPoint(m_vecHitPos_BS, m_vecHitPos_WS);
		m_vecHitPos_BS.w = 0.0f;
		FASSERT(m_vecHitPos_BS.w == 0.0f);

		m_fHotSpotFactor = 1.0f;
	}
}
#endif



void CBlinkShell::SetUnitEffectEnd(f32 fUnitEffectEnd)
{
	m_fUnitEffectEnd = fUnitEffectEnd;
}



void CBlinkShell::Deactivate()
{
	// This should get moved to inside Work().
	m_uFlags &= ~(FLAG_ACTIVATED);

	u32 uDOPIdx;
	for (uDOPIdx = 0; uDOPIdx < DOPBONE_COUNT; uDOPIdx++ )
	{
		// NULL shouldn't happen but does...
		if (_apGeo[uDOPIdx] != NULL)
			m_pBot->m_pWorldMesh->ReleasekDOPGeo(_apGeo[uDOPIdx]);
	}
}

//==================
// private functions

CBlinkShell::CBlinkShell()
{
	m_uFlags = FLAG_NONE;
}



CBlinkShell::~CBlinkShell()
{
}



void CBlinkShell::_Work()
{
	if(m_fHotSpotFactor > 0.0f)
	{
		// Last frame we still were doing a hit effect.  Let's see if we should continue.
		m_fHotSpotFactor -= _fOOHitFadeOutTime * FLoop_fPreviousLoopSecs;
		m_fHotSpotFactor = m_fHotSpotFactor < 0.0f ? 0.0f : m_fHotSpotFactor;

		if(m_fHotSpotFactor > 0.0f)
		{
			// Okay, we're still doing a hit effect.  Let's calculate the new hit position in world space.
			CFMtx43A **paBoneMtx = m_pBot->m_pWorldMesh->GetBoneMtxPalette();
			CFMtx43A mtxBoneToWorld = *paBoneMtx[m_nHitBoneIdx];
			mtxBoneToWorld.MulPoint(m_vecHitPos_WS, m_vecHitPos_BS);
		}
	}

	switch(m_eState)
	{
		case STATE_ACTIVATING:
		{
			m_fStateTimer += FLoop_fPreviousLoopSecs;
			if(m_fStateTimer >= _fActivateTime)
			{
				m_fStateTimer = 0.0f;
				m_eState = STATE_ACTIVE;
			}
			break;
		}
	}

	if(!m_bAlreadyGlowed)
	{
		m_fDOPGlowPos += FLoop_fPreviousLoopSecs * _fGlowMoveRate;
		// + 1 should change to factor in the decay.
		if(m_fDOPGlowPos > (f32)(DOPBONE_COUNT + (1 / 0.2f)))
		{
//			m_fDOPGlowPos -= (f32)(DOPBONE_COUNT + 1);
			m_fDOPGlowPos = 0.0f;
			m_bAlreadyGlowed = TRUE;
		}
	}
}



void CBlinkShell::_FDraw()
{
	if(!(m_uFlags & FLAG_ACTIVATED))
	{
		return;
	}

	FDrawVtx_t *pavtxPool = fvtxpool_GetArray(2000);
	if( !pavtxPool ) {
		return;
	}
	u32 uCurVtx = 0;

	u32 uDOPIdx, uFaceIdx, uVrtIdx;

	FASSERT(m_pBot != NULL);
	CFMtx43A **paBoneMtx = m_pBot->m_pWorldMesh->GetBoneMtxPalette();
	CFMtx43A mtxCurBone;
	CFVec3A vecNewVert;
	// Method #1
	CFVec3A vecBlinkFront = m_pBot->m_MountUnitFrontXZ_WS;
	CFColorRGB rgbColor(1.0f, 1.0f, 1.0f);
	CFVec3A vecVertMinusBoneOrigin;
	CFVec3A vecTemp;

	//////////////////////////////////////////////////////////////////////
	// Create a base texture matrix to be used by all of the DOPs.
	CFMtx43A mtxTexBase;
	mtxTexBase.Identity();
	mtxTexBase.m_vX.Mul(0.9f + 0.3f * fmath_Cos(1.8f * 0.4f * (f32)(FLoop_nTotalLoopTicks) * (FLoop_fSecsPerTick)));
	mtxTexBase.m_vY.Mul(0.9f + 0.3f * fmath_Cos(1.8f * 0.5f * (f32)(FLoop_nTotalLoopTicks) * (FLoop_fSecsPerTick)));

	if(m_fHotSpotFactor > 0.0f)
	{
		mtxTexBase.m_vP.x = 0.81f * (f32)(FLoop_nTotalLoopTicks) * (FLoop_fSecsPerTick) + fmath_RandomFloat() * m_fHotSpotFactor;
		mtxTexBase.m_vP.y = fmath_RandomFloat() * m_fHotSpotFactor;
	}
	else
	{
		mtxTexBase.m_vP.x = 0.81f * (f32)(FLoop_nTotalLoopTicks) * (FLoop_fSecsPerTick);
	}
	//
	//////////////////////////////////////////////////////////////////////

	u32 uLastDOPToDraw = DOPBONE_COUNT;
	f32 fLastDOPToDraw = (f32)(DOPBONE_COUNT);
	if(m_eState == STATE_ACTIVATING)
	{
		fLastDOPToDraw = (f32)(DOPBONE_COUNT) * m_fStateTimer * _fOOActivateTime;
		uLastDOPToDraw = (u32)(fLastDOPToDraw);
	}

	for (uDOPIdx = 0; uDOPIdx < uLastDOPToDraw; ++uDOPIdx)
	{
		if(_apGeo[uDOPIdx] != NULL)
		{
			mtxCurBone = *paBoneMtx[_apGeo[uDOPIdx]->nBoneIdx];
			mtxCurBone.m_vX.Mul(_afScale[uDOPIdx]);
			mtxCurBone.m_vY.Mul(_afScale[uDOPIdx]);
			mtxCurBone.m_vZ.Mul(_afScale[uDOPIdx]);

			// This stuff doesn't make any difference per kDOP.  Doh!
			CFMtx43A mtxTexBone;
			mtxTexBone.Identity();
			CFMtx43A mtxTexFinal;
			mtxTexFinal.Mul(mtxTexBase, mtxTexBone);
			fdraw_SetTexCoordXfmMtx(&mtxTexFinal);

			f32 fDOPPhase = (f32)(uDOPIdx) * 1.23f;
			f32 fGlobalAlpha = 0.09f + 0.06f * fmath_Sin(3.5f * (f32)FLoop_nTotalLoopTicks * FLoop_fSecsPerTick + fDOPPhase);

			if(!m_bAlreadyGlowed)
			{
				f32 fCurDOP = (f32)(uDOPIdx);

				f32 fWeight1 = 1.0f - 0.2f * (m_fDOPGlowPos - 1.0f - fCurDOP);
				FMATH_CLAMP_MIN0(fWeight1);
				f32 fWeight2 = 1.0f - fWeight1;

				fGlobalAlpha = fWeight2 * fGlobalAlpha + fWeight1 * 1.0f;
			}

			for (uFaceIdx = 0; uFaceIdx < _apGeo[uDOPIdx]->nFaceCount; ++uFaceIdx)
			{
				if((m_fUnitEffectEnd >= 0.0f) && (fmath_RandomChance(m_fUnitEffectEnd)))
				{
					continue;
				}

				for (uVrtIdx = 2; uVrtIdx < _apGeo[uDOPIdx]->aFaces[uFaceIdx].nVertCount; ++uVrtIdx)
				{

					if(2000 - uCurVtx < 3)
					{
						// Draw the triangle list
						fdraw_PrimList(FDRAW_PRIMTYPE_TRILIST, pavtxPool, uCurVtx);
						uCurVtx = 0;
					}

					//////////////////////////////////////////////////////////////////////
					//
					_AddToVtxList(pavtxPool, uCurVtx, uDOPIdx, uFaceIdx, 0, &mtxCurBone, vecBlinkFront, fGlobalAlpha, rgbColor);
					_AddToVtxList(pavtxPool, uCurVtx, uDOPIdx, uFaceIdx, uVrtIdx - 1, &mtxCurBone, vecBlinkFront, fGlobalAlpha, rgbColor);
					_AddToVtxList(pavtxPool, uCurVtx, uDOPIdx, uFaceIdx, uVrtIdx, &mtxCurBone, vecBlinkFront, fGlobalAlpha, rgbColor);
					//
					//////////////////////////////////////////////////////////////////////
				}
			}
		}
	}

	// Draw the triangle list
	fdraw_PrimList(FDRAW_PRIMTYPE_TRILIST, pavtxPool, uCurVtx);

	fvtxpool_ReturnArray(pavtxPool);
}



void CBlinkShell::_AddToVtxList(FDrawVtx_t *pVtxArray, u32 &uCurVtx, u32 uDOPIdx, u32 uFaceIdx, u32 uVrtIdx, CFMtx43A *pBS2WS, CFVec3A &vecBlinkFront, f32 fGlobalAlpha, CFColorRGB &rgbBase)
{
	static CFVec3A vecNewVert_WS;
	static CFVec3A vecVertMinusBoneOrigin;
	static CFVec3A vecTemp;

	//////////////////////////////////////////////////////////////////////
	//
	vecNewVert_WS.v3 = _apGeo[uDOPIdx]->aVerts[_apGeo[uDOPIdx]->aFaces[uFaceIdx].anVertIdx[uVrtIdx]];
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

		//////////////////////////////////////////////////////////////////////
		// Fix up our alpha if we need to.
		if(m_fHotSpotFactor > 0.0f)
		{
			vecTemp = m_vecHitPos_WS;
			vecTemp.Sub(vecNewVert_WS);
			if(vecTemp.MagSq() < _fHitRadOuterSq)
			{
				// JUSTIN: This could also be optimized.
				f32 fDistToHitPos = vecTemp.Mag();
				f32 fWeight1, fWeight2;
				if(fDistToHitPos <= _fHitRadInner)
				{
					fWeight1 = m_fHotSpotFactor;
					fWeight2 = 1.0f - fWeight1;
				}
				else
				{
					f32 fRadiusFactor = (fDistToHitPos - _fHitRadInner) * (1.0f / (_fHitRadOuter - _fHitRadInner));
					fWeight1 = (m_fHotSpotFactor) * (1.0f - fRadiusFactor);
					fWeight2 = 1.0f - fWeight1;
				}
				fAlphaFinal = (fWeight2 * fAlphaFinal) + (fWeight1 * _fMaxAlpha);
				FASSERT_UNIT_FLOAT(fAlphaFinal);
			}
		}
		//
		//////////////////////////////////////////////////////////////////////

	pVtxArray[uCurVtx].ColorRGBA.ColorRGB = rgbBase;
	pVtxArray[uCurVtx].ColorRGBA.fAlpha = fAlphaFinal;
	pVtxArray[uCurVtx].Pos_MS = vecNewVert_WS.v3;
	pVtxArray[uCurVtx++].ST.Set(0.0f, 0.0f);
	//
	//////////////////////////////////////////////////////////////////////
}