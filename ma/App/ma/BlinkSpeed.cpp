//////////////////////////////////////////////////////////////////////////////////////
// BlinkSpeed.cpp - 
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
// 07/22/02 Link        Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "BlinkSpeed.h"
#include "FkDOP.h"
#include "ftex.h"
#include "fvtxpool.h"
#include "fresload.h"
#include "gamecam.h"
#include "bot.h"

//====================
// private definitions

//=================
// public variables

//==================
// private variables
#define _NUM_SPEED		( 4 )	  //max one at a time per player
#define _NUM_LIGHTNING	( 4 )	  //max one at a time per player

static const f32 _fBranchWidth = 0.03f;

CBlinkSpeed *CBlinkSpeed::m_apSpeedPool = NULL;
BOOL CBlinkSpeed::m_bSystemInitted = FALSE;

cchar *CBlinkSpeed::s_apszBoneNames[CBlinkSpeed::DOPBONE_COUNT] = { "R_Leg_Lower", "L_Leg_Lower", "R_Heel", "L_Heel" };

//===================
// private prototypes

//=================
// public static functions

CLightningBoltGroup *CLightningBoltGroup::s_paLBPool = NULL;
const u32 CLightningBoltGroup::s_uPoolSize = _NUM_LIGHTNING;
CFTexInst CLightningBoltGroup::s_oTexInst;

// Static function.
BOOL CLightningBoltGroup::InitSystem()
{
	s_paLBPool = fnew CLightningBoltGroup[s_uPoolSize];
	if(s_paLBPool == NULL)
	{
		DEVPRINTF("CLightningBoltGroup::InitSystem() : Out of memory.\n");
		goto _ExitWithError;
	}

	s_oTexInst.SetTexDef((FTexDef_t *)(fresload_Load(FTEX_RESNAME, "tf_bolt02")));
//	s_oTexInst.SetFlags(CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T);

	return(TRUE);

_ExitWithError:
	return(FALSE);
}



// Static function.
void CLightningBoltGroup::UninitSystem()
{
	fdelete_array(s_paLBPool);
	s_paLBPool = NULL;
}



// Static function.
BOOL CLightningBoltGroup::InitLevel()
{
	return(TRUE);
}



// Static function.
void CLightningBoltGroup::UninitLevel()
{
}



CLightningBoltGroup::CLightningBoltGroup()
{
	_Clear();
}


CLightningBoltGroup::~CLightningBoltGroup()
{
	_Clear();
}


// Static function.
void CLightningBoltGroup::Work()
{
	u32 uPoolIdx;
	CLightningBoltGroup *pCurGroup;
	for(uPoolIdx = 0; uPoolIdx < s_uPoolSize; ++uPoolIdx)
	{
		pCurGroup = &s_paLBPool[uPoolIdx];
		if(pCurGroup->m_uFlags & FLAG_ACTIVE)
		{
			pCurGroup->_Work();
		}
	}
}



// Static function.
void CLightningBoltGroup::FDraw()
{
	// Get camera, pass it to _FDraw functions.
	CFCamera *pCam = gamecam_GetActiveCamera();

	fdraw_Color_SetFunc(FDRAW_COLORFUNC_DIFFUSETEX_AIAT);
	fdraw_Alpha_SetBlendOp(FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST);

	fdraw_SetTexture(&s_oTexInst);

	u32 uPoolIdx;
	CLightningBoltGroup *pCurGroup;
	for(uPoolIdx = 0; uPoolIdx < s_uPoolSize; ++uPoolIdx)
	{
		pCurGroup = &s_paLBPool[uPoolIdx];
		if(pCurGroup->m_uFlags & FLAG_ACTIVE)
		{
			pCurGroup->_FDraw(pCam->GetFinalXfm());
		}
	}
}



CLightningBoltGroup *CLightningBoltGroup::GetLightning()
{
	CLightningBoltGroup *pCurLBGroup;
	u32 uCurIdx;
	for(uCurIdx = 0; uCurIdx < s_uPoolSize; ++uCurIdx)
	{
		pCurLBGroup = &s_paLBPool[uCurIdx];
		if((pCurLBGroup->m_uFlags & FLAG_ACQUIRED) == 0)
		{
			break;
		}
	}

	if(uCurIdx == s_uPoolSize)
	{
		DEVPRINTF("CLightningBoltGroup::Acquire() : Could not acquire, pool exhausted.\n");
		return(NULL);
	}

	pCurLBGroup->m_uFlags |= FLAG_ACQUIRED;
	return(pCurLBGroup);
}

BOOL CLightningBoltGroup::ReturnLightning()
{
	m_uFlags &= ~(FLAG_ACQUIRED | FLAG_ACTIVE);

	return TRUE;
}


BOOL CLightningBoltGroup::Init(u32 uNumBolts, CBot *pBot, u8 uNumBoneNames, cchar **papszBoneNames)
{
	FASSERT(uNumBoneNames <= _LBBoneListSize);
	u32 uCurBoneIdx;
	for(uCurBoneIdx = 0; uCurBoneIdx < uNumBoneNames; ++uCurBoneIdx)
	{
		m_anBoneIdxList[uCurBoneIdx] = pBot->m_pWorldMesh->FindBone(papszBoneNames[uCurBoneIdx]);
	}
	m_uBoneListSize = uNumBoneNames;

	FASSERT(uNumBolts <= _LBG_MaxBolts);
	u32 uCurBoltIdx;
	for(uCurBoltIdx = 0; uCurBoltIdx < uNumBolts; ++uCurBoltIdx)
	{
		_InitBolt(&m_aoLB[uCurBoltIdx]);
	}
	m_uNumBolts = uNumBolts;

	m_pBot = pBot;

	return(TRUE);
}



void CLightningBoltGroup::Activate()
{
	m_uFlags |= FLAG_ACTIVE;
	m_fUnitEffectEnd = 0.0f;
}



void CLightningBoltGroup::SetUnitEffectEnd(f32 fUnitEffectEnd)
{
	m_fUnitEffectEnd = fUnitEffectEnd;
}



void CLightningBoltGroup::Deactivate()
{
	m_uFlags &= ~(FLAG_ACTIVE);
}



void CLightningBoltGroup::_Clear()
{
	m_uFlags = FLAG_NONE;

	u32 uCurBoneIdx;
	for(uCurBoneIdx = 0; uCurBoneIdx < _LBBoneListSize; ++uCurBoneIdx)
	{
		m_anBoneIdxList[uCurBoneIdx] = -1;
	}
}



void CLightningBoltGroup::_Work()
{
	CLightningBolt *pCurBolt;
	u32 uCurBoltIdx;
	//////////////////////////////////////////////////////////////////////
	// Go through each bolt and tell it to do its work.
	for(uCurBoltIdx = 0; uCurBoltIdx < m_uNumBolts; ++uCurBoltIdx)
	{
		pCurBolt = &m_aoLB[uCurBoltIdx];
		pCurBolt->Work();
		if(pCurBolt->m_fCurAlpha == 0.0f)
		{
			_InitBolt(pCurBolt);
		}
	}
	//
	//////////////////////////////////////////////////////////////////////
}



void CLightningBoltGroup::_FDraw(const CFXfm *pCamXfm)
{
	u32 uCurBranchIdx;
	CFMtx43A *pBS2WS;
	CFMtx43A **paBoneMtx = m_pBot->m_pWorldMesh->GetBoneMtxPalette();

	//////////////////////////////////////////////////////////////////////
	// Go through each bolt and tell it to draw itself.
	for(uCurBranchIdx = 0; uCurBranchIdx < m_uNumBolts; ++uCurBranchIdx)
	{
		if((m_fUnitEffectEnd > 0.0f) && (fmath_RandomChance(m_fUnitEffectEnd)))
		{
			continue;
		}
		pBS2WS = paBoneMtx[m_aoLB[uCurBranchIdx].m_nBoneIdx];
		m_aoLB[uCurBranchIdx].FDraw(pCamXfm, pBS2WS);
	}
	//
	//////////////////////////////////////////////////////////////////////
}




void CLightningBoltGroup::_InitBolt(CLightningBolt *pBolt)
{
	FASSERT(pBolt != NULL);

	u32 uRandomIdx = fmath_RandomChoice(m_uBoneListSize);
	s32 nBoneIdx = m_anBoneIdxList[uRandomIdx];
	f32 fLifetime = fmath_RandomFloatRange(0.2f, 0.6f);
	pBolt->Init(nBoneIdx, fLifetime);
}









CLightningBoltGroup::CLightningBolt::CLightningBolt()
{
}



CLightningBoltGroup::CLightningBolt::~CLightningBolt()
{
}




void CLightningBoltGroup::CLightningBolt::Init(s32 nBoneIdx, f32 fLifeTime)
{
	FASSERT(nBoneIdx > -1);
	FASSERT(nBoneIdx < 345);	// DEBUG line, Some fairly arbitrary number.

	m_nBoneIdx = nBoneIdx;
	// Have this second number come from a table.
	m_fDistFromBoneOrigin = fmath_RandomFloat() * 0.5f;
	m_fCurAlpha = fmath_RandomFloat();

	f32 fTheta;
	u32 uRandVal = fmath_RandomChoice(8);
	if((uRandVal & 1) == 0)
	{
		fTheta = 0.0f;
	}
	else
	{
		fTheta = FMATH_PI;
	}
	f32 fLength = fmath_RandomFloatRange(0.25f, 0.35f);

	u32 uBranchCount = 0;
	_BuildBolt(NULL, uBranchCount, 0, fTheta, fLength);

	m_fFadeRate = 1.0f / fLifeTime;
	m_fOOFadeRate = fLifeTime;
}



void CLightningBoltGroup::CLightningBolt::Work()
{
	//////////////////////////////////////////////////////////////////////
	// Do our fading work.
	m_fCurAlpha -= m_fFadeRate * FLoop_fPreviousLoopSecs;
	FMATH_CLAMP_MIN0(m_fCurAlpha);
	//
	//////////////////////////////////////////////////////////////////////
}



void CLightningBoltGroup::CLightningBolt::FDraw(const CFXfm *pCamXfm, CFMtx43A *pmtxBS2WS)
{
	FDrawVtx_t *pavtxArray = fvtxpool_GetArray(_LB_MaxBranches * 6);

	u32 uCurVtx = 0;
	CFVec3A vecStartPos = pmtxBS2WS->m_vY;
	vecStartPos.Mul(m_fDistFromBoneOrigin);
	vecStartPos.Add(pmtxBS2WS->m_vPos);

	m_aBranches[0].AddToVtxList(pavtxArray, uCurVtx, vecStartPos, pCamXfm->m_MtxR.m_vX, pCamXfm->m_MtxR.m_vY, m_fCurAlpha);

	fdraw_PrimList(FDRAW_PRIMTYPE_TRILIST, pavtxArray, uCurVtx);

	fvtxpool_ReturnArray(pavtxArray);
}



void CLightningBoltGroup::CLightningBolt::_BuildBolt(CLBBranch **pBuiltBoltHead, u32 &uStartBranchIdx, u32 uDepth, f32 fThetaIn, f32 fLengthIn)
{
//	FASSERT(uStartBranchIdx < _LB_MaxBranches);
	if(uStartBranchIdx == _LB_MaxBranches)
	{
		// We're out of branches, there's nothing we can do.
		if(pBuiltBoltHead != NULL)
		{
			*pBuiltBoltHead = NULL;
		}
		return;
	}

	CLBBranch *pCurBranch = &m_aBranches[uStartBranchIdx];

	// Later on: Perhaps use uDepth in making this calculation.
	fThetaIn += fmath_RandomBipolarUnitFloat() * 0.70f;
	fLengthIn *= fmath_RandomFloatRange(0.9f, 1.0f);

	// Initialize our first branch for this bolt.
	pCurBranch->Init(fThetaIn, fLengthIn);
	if(pBuiltBoltHead != NULL)
	{
		*pBuiltBoltHead = pCurBranch;
	}
	++uStartBranchIdx;

	// Decide if we want to build child bolts.
	f32 fProb = 1.0f - (f32)(uDepth) * 0.20f;
	FMATH_CLAMP_MIN0(fProb);
	if(fmath_RandomChance(fProb))
	{
		_BuildBolt(&pCurBranch->m_apChildren[0], uStartBranchIdx, uDepth + 1, fThetaIn, fLengthIn);
		fProb = 0.9f - (f32)(uDepth) * 0.20f;
		FMATH_CLAMP_MIN0(fProb);

		if(fmath_RandomChance(fProb))
		{
			_BuildBolt(&pCurBranch->m_apChildren[1], uStartBranchIdx, uDepth + 1, fThetaIn, fLengthIn);
		}
		else
		{
			pCurBranch->m_apChildren[1] = NULL;
		}
	}
	else
	{
		pCurBranch->m_apChildren[0] = NULL;
		pCurBranch->m_apChildren[1] = NULL;
	}
}





CLightningBoltGroup::CLightningBolt::CLBBranch::CLBBranch()
{
	_Clear();
}


CLightningBoltGroup::CLightningBolt::CLBBranch::~CLBBranch()
{
	_Clear();
}



void CLightningBoltGroup::CLightningBolt::CLBBranch::AddToVtxList(FDrawVtx_t *pavtxArray, u32 &uCurVtx, const CFVec3A &vecParentPos, const CFVec3A &vecRight_WS, const CFVec3A &vecUp_WS, f32 fAlpha)
{
	static CFVec3A vecDelY, vecDelX;		// These are in the space of the branch.
	static CFVec3A aavecPts[2][2];

	static CFVec3A vecTemp;

	static CFVec3A vecNewUp_WS, vecNewRight_WS;

	//////////////////////////////////////////////////////////////////////
	// Calculate some interesting and useful vectors.
	// Don't you just love hand-crafted vector math?
	vecNewRight_WS = vecUp_WS;
	vecNewRight_WS.Mul(m_vecUnitOfsFromParent.y);
	vecTemp = vecRight_WS;
	vecTemp.Mul(m_vecUnitOfsFromParent.x);
	vecNewRight_WS.Add(vecTemp);

	vecDelX = vecNewRight_WS;
	vecDelX.Mul(m_fDistFromParent);

	vecNewUp_WS = vecUp_WS;
	vecNewUp_WS.Mul(m_vecUnitOfsFromParent.x);
	vecTemp = vecRight_WS;
	vecTemp.Mul(m_vecUnitOfsFromParent.y);
	vecNewUp_WS.Sub(vecTemp);

	vecDelY = vecNewUp_WS;
	vecDelY.Mul(_fBranchWidth);

	CFVec3A vecEndPos = vecParentPos;
	vecEndPos.Add(vecDelX);
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	//
	aavecPts[0][0] = vecParentPos;
	aavecPts[0][0].Add(vecDelY);

	aavecPts[0][1] = vecParentPos;
	aavecPts[0][1].Sub(vecDelY);

	if((m_apChildren[0] != NULL) || (m_apChildren[1] != NULL))
	{
		// We have at least one children, draw the branch as usual.
		aavecPts[1][0] = aavecPts[0][0];
		aavecPts[1][0].Add(vecDelX);

		aavecPts[1][1] = aavecPts[0][1];
		aavecPts[1][1].Add(vecDelX);
	}
	else
	{
		// We have no children, let's taper this last branch.
		aavecPts[1][0] = vecEndPos;

		aavecPts[1][1] = vecEndPos;
	}
	//
	//////////////////////////////////////////////////////////////////////

	pavtxArray[uCurVtx].ColorRGBA.White();
	pavtxArray[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	pavtxArray[uCurVtx].Pos_MS = aavecPts[0][0].v3;
	pavtxArray[uCurVtx++].ST.Set(0.0f, 0.0f);

	pavtxArray[uCurVtx].ColorRGBA.White();
	pavtxArray[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	pavtxArray[uCurVtx].Pos_MS = aavecPts[1][0].v3;
	pavtxArray[uCurVtx++].ST.Set(0.0f, 1.0f);

	pavtxArray[uCurVtx].ColorRGBA.White();
	pavtxArray[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	pavtxArray[uCurVtx].Pos_MS = aavecPts[0][1].v3;
	pavtxArray[uCurVtx++].ST.Set(1.0f, 0.0f);

	pavtxArray[uCurVtx].ColorRGBA.White();
	pavtxArray[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	pavtxArray[uCurVtx].Pos_MS = aavecPts[1][0].v3;
	pavtxArray[uCurVtx++].ST.Set(0.0f, 1.0f);

	pavtxArray[uCurVtx].ColorRGBA.White();
	pavtxArray[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	pavtxArray[uCurVtx].Pos_MS = aavecPts[0][1].v3;
	pavtxArray[uCurVtx++].ST.Set(1.0f, 0.0f);

	pavtxArray[uCurVtx].ColorRGBA.White();
	pavtxArray[uCurVtx].ColorRGBA.fAlpha = fAlpha;
	pavtxArray[uCurVtx].Pos_MS = aavecPts[1][1].v3;
	pavtxArray[uCurVtx++].ST.Set(1.0f, 1.0f);

	if(m_apChildren[0] != NULL)
	{
		m_apChildren[0]->AddToVtxList(pavtxArray, uCurVtx, vecEndPos, vecRight_WS, vecUp_WS, fAlpha);
		if(m_apChildren[1] != NULL)
		{
			m_apChildren[1]->AddToVtxList(pavtxArray, uCurVtx, vecEndPos, vecRight_WS, vecUp_WS, fAlpha);
		}
	}

	// If this ASSERTs, you have several choices (in order of recommendation):
	//   1) Decrease the number of lightning bolts.
	//   2) Increase the size of the vertex array (and change the number below).
	//   3) Decrease the probability of spawning children branches (probably not the best choice).
	FASSERT(uCurVtx < 2000);
}




void CLightningBoltGroup::CLightningBolt::CLBBranch::Init(f32 fTheta, f32 fLength)
{
	// These are reversed from what you might normally think because we want to measure
	//   the angle from off of the x+ axis.
	fmath_SinCos(fTheta, &m_vecUnitOfsFromParent.y, &m_vecUnitOfsFromParent.x);
	m_fDistFromParent = fLength;
}




void CLightningBoltGroup::CLightningBolt::CLBBranch::_Clear()
{
	m_apChildren[0] = NULL;
	m_apChildren[1] = NULL;
}





BOOL CBlinkSpeed::InitSystem()
{
	s32 nIndex;
	FASSERT(!m_bSystemInitted);
	m_bSystemInitted = TRUE;

	m_apSpeedPool = fnew CBlinkSpeed[_NUM_SPEED];
	if(m_apSpeedPool == NULL)
	{
		DEVPRINTF("CBlinkShell::InitSystem() : Out of memory.\n");
		return(FALSE);
	}

	for( nIndex = 0; nIndex < _NUM_SPEED; nIndex++ )
	{
		m_apSpeedPool[nIndex].m_bAllocated = FALSE;
	}

	return(TRUE);
}



void CBlinkSpeed::UninitSystem()
{
	fdelete_array(m_apSpeedPool);
	m_apSpeedPool = NULL;

	m_bSystemInitted = FALSE;
}



BOOL CBlinkSpeed::InitLevel()
{
	s32 nIndex;
	FASSERT(m_bSystemInitted);

	for( nIndex = 0; nIndex < _NUM_SPEED; nIndex++ )
	{
		m_apSpeedPool[nIndex].m_bAllocated = FALSE;
	}

	return(TRUE);
}



void CBlinkSpeed::UninitLevel()
{
	FASSERT(m_bSystemInitted);
}


CBlinkSpeed *CBlinkSpeed::GetSpeed()
{
	s32 nIndex;
	FASSERT(m_bSystemInitted);

	for( nIndex = 0; nIndex < _NUM_SPEED; nIndex++ )
	{
		if( !m_apSpeedPool[nIndex].m_bAllocated )
		{
			m_apSpeedPool[nIndex].m_bAllocated = TRUE;
			return(&m_apSpeedPool[nIndex]);
		}
	}

	return(NULL);
}

BOOL CBlinkSpeed::ReturnSpeed()
{
	if( m_pBoltGroup )
	{
		m_pBoltGroup->Deactivate();
		m_pBoltGroup->ReturnLightning();
		m_pBoltGroup = NULL;
	}

	m_bAllocated = FALSE;

	return TRUE;
}

void CBlinkSpeed::Work()
{
	FASSERT(m_bSystemInitted);

	u32 uSpeedIdx;
	for(uSpeedIdx = 0; uSpeedIdx < _NUM_SPEED; ++uSpeedIdx)
	{
		if( m_apSpeedPool[uSpeedIdx].m_bAllocated )
		{
			m_apSpeedPool[uSpeedIdx]._Work();
		}
	}
}



void CBlinkSpeed::FDraw()
{
	FASSERT(m_bSystemInitted);
}



//=================
// public functions

void CBlinkSpeed::Activate()
{
	m_uFlags |= FLAG_ACTIVATED;

	if(m_pBoltGroup == NULL)
	{
		m_pBoltGroup = CLightningBoltGroup::GetLightning();
	}
	m_pBoltGroup->Init(15, m_pBot, DOPBONE_COUNT, s_apszBoneNames);
	m_pBoltGroup->Activate();
}



void CBlinkSpeed::SetUnitEffectEnd(f32 fUnitEffectEnd)
{
	if(m_pBoltGroup != NULL)
	{
		m_pBoltGroup->SetUnitEffectEnd(fUnitEffectEnd);
	}
}



void CBlinkSpeed::Deactivate()
{
	m_uFlags &= ~(FLAG_ACTIVATED);
	if(m_pBoltGroup != NULL)
	{
		m_pBoltGroup->Deactivate();
	}
}

//==================
// private functions

CBlinkSpeed::CBlinkSpeed()
{
	m_uFlags = FLAG_NONE;
	m_pBoltGroup = NULL;
}



CBlinkSpeed::~CBlinkSpeed()
{
}



void CBlinkSpeed::_Work()
{
}



void CBlinkSpeed::_FDraw()
{
}



void CBlinkSpeed::_AddToVtxList(FDrawVtx_t *pVtxArray, u32 &uCurVtx, u32 uDOPIdx, u32 uFaceIdx, u32 uVrtIdx, CFMtx43A *pBS2WS, CFVec3A &vecBlinkFront, f32 fGlobalAlpha, CFColorRGB &rgbBase)
{
}