//////////////////////////////////////////////////////////////////////////////////////
// FPointPath2.cpp - CFVec3Obj defining a path from an ordered set of points.
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
// 04/04/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "FPointPath2.h"
#include "floop.h"

// =============================================================================================================
#define _NUM_POOL_ENTRIES		( 30 )
u32 CFV3OPointPath2::m_uBufPos = 0;
u32 CFV3OPointPath2::m_uNumInPool = 0;
CFV3OPointPath2 FPointPath2_aoPool[_NUM_POOL_ENTRIES];

// =============================================================================================================

BOOL CFV3OPointPath2::LevelInit()
{
	s32 nIndex;
	for( nIndex = 0; nIndex < _NUM_POOL_ENTRIES; ++nIndex )
	{
		FPointPath2_aoPool[ nIndex ].m_uFlags = FLAG_DONE;
	}

	m_uBufPos = 0; 

	return TRUE;
}

// =============================================================================================================

CFV3OPointPath2::CFV3OPointPath2()
{
	m_pavecCoeff = NULL;
//	m_bInitted = FALSE;
}

// =============================================================================================================

CFV3OPointPath2::~CFV3OPointPath2()
{
	fdelete_array(m_pavecCoeff);
	m_pavecCoeff = NULL;
}

// =============================================================================================================

void CFV3OPointPath2::SetTimePos(f32 fNewTime)
{
//	if(!m_bInitted)
	if(!IsInitted())
	{
		return;
	}


	FMATH_CLAMPMIN(fNewTime, 0.0f);
	FMATH_CLAMPMAX(fNewTime, m_fTotalTime);
	m_fTimer = fNewTime;
	m_fTimePos = fNewTime;
	m_uCurSeg = 0;
	while((m_fTimer >= m_afSegTime[m_uCurSeg]) && (m_uCurSeg < m_uNumSegs))
	{
		m_fTimer -= m_afSegTime[m_uCurSeg];
		++m_uCurSeg;
	}

	if(m_uCurSeg == m_uNumSegs)
	{
		this->End();
	}
	else
	{
		Calculate();
	}
}

// =============================================================================================================

CFV3OPointPath2 *CFV3OPointPath2::GetAvailable()
{
	if(m_uBufPos == _NUM_POOL_ENTRIES)
	{
		DEVPRINTF("CFV3OPointPath2::GetAvailable : No more available.\n");
		return(NULL);
	}
//	CFV3OPointPath2 *poSO = &(m_aoPool[m_uBufPos]);
	CFV3OPointPath2 *poSO = &(FPointPath2_aoPool[m_uBufPos]);

	++m_uBufPos;
	FASSERT(m_uBufPos <= _NUM_POOL_ENTRIES);

	return(poSO);
}

// =============================================================================================================

BOOL CFV3OPointPath2::Init(const char *pszPathName, f32 fTotalTime, BOOL bAutoStart/* = TRUE*/)
{
	FASSERT(pszPathName != NULL);

	// TODO: Retrieve the data from the world file.
//	CFVec3A avecTempPtList[4] = { CFVec3A(0.0f, 40.0f, 0.0f), CFVec3A(0.0f, 40.0f, -40.0f), CFVec3A(40.0f, 40.0f, -80.0f), CFVec3A(60.0f, 40.0f, -120.0f) };
	CFVec3A avecTempPtList[7] = {	CFVec3A(327.0f, -780.0f, -66.0f), CFVec3A(559.0f, -786.0f, -53.0f), CFVec3A(625.0f, -786.0f, 51.0f),
									CFVec3A(754.0f, -793.0f, -74.0f), CFVec3A(1028.0f, -831.0f, -92.0f), CFVec3A(948.0f, -831.0f, -92.0f),
									CFVec3A(950.0f, -202.0f, -831.0f) };

	GetCoeffFromPtList(avecTempPtList, 7, fTotalTime);

	m_fTimer = 0.0f;
	m_fTimePos = 0.0f;
	m_uCurSeg = 0;

	m_vecValue = avecTempPtList[0];//m_avecCoeff[0][0];
	m_vecVel.Zero();// = m_avecCoeff[0][1];
	m_vecTang = m_pavecCoeff[0].m_avecCoeff[2];
	m_vecTang.Unitize();

	m_uFlags |= FLAG_INITTED;
	Start();

	return(TRUE);
}

// =============================================================================================================

BOOL CFV3OPointPath2::Init(const CFVec3A *aPtList, u32 uNumPts, f32 fTotalTime, BOOL bAutoStart)
{
	FASSERT(aPtList != NULL);
	FASSERT(uNumPts > 1);
	FASSERT(uNumPts < FPointPath2_uMaxPts);

	m_pavecCoeff = fnew CPathCoeff[uNumPts - 1];
	if(m_pavecCoeff == NULL)
	{
		DEVPRINTF("CFV3OPointPath2::Init() : Out of memory.\n");
		return(FALSE);
	}

	GetCoeffFromPtList(aPtList, uNumPts, fTotalTime);
//	m_bInitted = TRUE;

	m_fTimer = 0.0f;
	m_fTimePos = 0.0f;
	m_uCurSeg = 0;

	m_vecValue = aPtList[0];//m_avecCoeff[0][0];
	m_vecVel.Zero();// = m_avecCoeff[0][1];
	m_vecTang = m_pavecCoeff[0].m_avecCoeff[2];
	m_vecTang.Unitize();

	m_uFlags |= FLAG_INITTED;
	m_uFlags &= ~(FLAG_WORKING);

	if(bAutoStart)
	{
		Start();
	}

	return(TRUE);
}

// =============================================================================================================

void CFV3OPointPath2::Work()
{
//	if(!m_bInitted)
	if(!IsInitted())
	{
		return;
	}

	FASSERT((m_uFlags & FLAG_DONE) == 0);
	m_fTimer += FLoop_fPreviousLoopSecs;
	m_fTimePos += FLoop_fPreviousLoopSecs;
//	DEVPRINTF("Object %d, timer = %f.\n", this - m_aoPool, m_fTimer);
	FASSERT(m_uCurSeg < m_uNumSegs);
	while(m_fTimer >= m_afSegTime[m_uCurSeg])
	{
		m_fTimer -= m_afSegTime[m_uCurSeg];
		++m_uCurSeg;
		if(m_uCurSeg == m_uNumSegs)
		{
//			m_vecValue = m_avecCoeff[m_uNumSegs - 1][0].v3;
			m_vecVel.Zero();
			m_vecAccel.Zero();
			End();
			return;
		}
		FASSERT(m_uCurSeg < m_uNumSegs);
	}
	FASSERT(m_uCurSeg < m_uNumSegs);

	if(m_fTimePos >= m_fTotalTime)
	{
		// We're already at the end, don't do any more calculations.
		End();
		return;
	}

	Calculate();
}

// =============================================================================================================

void CFV3OPointPath2::GetCoeffFromPtList(const CFVec3A *aPtList, u32 uNumPts, f32 fTotalTime)
{
	m_uNumPts = uNumPts;
	m_uNumSegs = uNumPts - 1;
	m_fTotalTime = fTotalTime;

	f32 afSegLen[FPointPath2_uMaxSegs];
	/////////////////////////////////////////////////////////////
	// Find the length of each segment and of the entire path.
	f32 fTempLen, fTotalLen = 0.0f;
	CFVec3A vecTemp;
	u32 uCurSeg;
	for(uCurSeg = 0; uCurSeg < m_uNumSegs; ++uCurSeg)
	{
		vecTemp = aPtList[uCurSeg + 1];
		vecTemp.Sub(aPtList[uCurSeg]);
		fTempLen = vecTemp.Mag();
		afSegLen[uCurSeg] = fTempLen;
		fTotalLen += fTempLen;
	}
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// Determine the time for each segment of the path
	//   individually.
	f32 fOOTotalLen = 1.0f / fTotalLen;
	for(uCurSeg = 0; uCurSeg < m_uNumSegs; ++uCurSeg)
	{
		m_afSegTime[uCurSeg] = afSegLen[uCurSeg] * fOOTotalLen * fTotalTime;
	}
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// Determine the coefficients for each segment of the path.
	CFVec3A avecVel[2];
	f32 fMag;
	for(uCurSeg = 0; uCurSeg < m_uNumSegs; ++uCurSeg)
	{
		/////////////////////////////////////////////////////////////
		// Set up the appropriate initial and final velocities.
		if(uCurSeg == 0)
		{
			avecVel[0].Set(0.0f, 0.0f, 0.0f);
		}
		else
		{
			// Get the direction in which the velocity should point.
			avecVel[0] = aPtList[uCurSeg + 1];
			avecVel[0].Sub(aPtList[uCurSeg - 1]);
			avecVel[0].Unitize();

			// Get the magnitude of the velocity.
			fMag = (afSegLen[uCurSeg - 1] + afSegLen[uCurSeg + 0]) / (m_afSegTime[uCurSeg - 1] + m_afSegTime[uCurSeg + 0]);
			avecVel[0].Mul(fMag);
		}

		if(uCurSeg == (m_uNumSegs - 1))
		{
			avecVel[1].Set(0.0f, 0.0f, 0.0f);
		}
		else
		{
			// Get the direction in which the velocity should point.
			avecVel[1] = aPtList[uCurSeg + 2];
			avecVel[1].Sub(aPtList[uCurSeg - 0]);
			avecVel[1].Unitize();

			// Get the magnitude of the velocity.
			fMag = (afSegLen[uCurSeg + 0] + afSegLen[uCurSeg + 1]) / (m_afSegTime[uCurSeg + 0] + m_afSegTime[uCurSeg + 1]);
			avecVel[1].Mul(fMag);
		}
		//
		/////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////
		// Calculate some temporary values that we will use.
		CFVec3A vecTemp1, vecTemp2;
		f32 fOOTime;

		vecTemp1 = aPtList[uCurSeg + 1];
		vecTemp1.Sub(aPtList[uCurSeg]);
		vecTemp = avecVel[0];
		vecTemp.Mul(m_afSegTime[uCurSeg]);
		vecTemp1.Sub(vecTemp);

		vecTemp2 = avecVel[1];
		vecTemp2.Sub(avecVel[0]);
		FASSERT(m_afSegTime[uCurSeg] != 0.0f);
		fOOTime = 1.0f / m_afSegTime[uCurSeg];
		//
		/////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////
		// Now let's actually calculate the coefficients.
		m_pavecCoeff[uCurSeg].m_avecCoeff[0] = aPtList[uCurSeg];

		m_pavecCoeff[uCurSeg].m_avecCoeff[1] = avecVel[0];

		m_pavecCoeff[uCurSeg].m_avecCoeff[2] = vecTemp1;
		m_pavecCoeff[uCurSeg].m_avecCoeff[2].Mul(3.0f * m_afSegTime[uCurSeg] * m_afSegTime[uCurSeg]);
		vecTemp = vecTemp2;
		vecTemp.Mul(m_afSegTime[uCurSeg] * m_afSegTime[uCurSeg] * m_afSegTime[uCurSeg]);
		m_pavecCoeff[uCurSeg].m_avecCoeff[2].Sub(vecTemp);
		m_pavecCoeff[uCurSeg].m_avecCoeff[2].Mul(fOOTime * fOOTime * fOOTime * fOOTime);

		m_pavecCoeff[uCurSeg].m_avecCoeff[3] = vecTemp2;
		m_pavecCoeff[uCurSeg].m_avecCoeff[3].Mul(m_afSegTime[uCurSeg] * m_afSegTime[uCurSeg]);
		vecTemp = vecTemp1;
		vecTemp.Mul(2.0f * m_afSegTime[uCurSeg]);
		m_pavecCoeff[uCurSeg].m_avecCoeff[3].Sub(vecTemp);
		m_pavecCoeff[uCurSeg].m_avecCoeff[3].Mul(fOOTime * fOOTime * fOOTime * fOOTime);
		//
		/////////////////////////////////////////////////////////////
	}
	//
	/////////////////////////////////////////////////////////////
}

// =============================================================================================================

void CFV3OPointPath2::Calculate()
{
	FASSERT(m_fTimer >= 0.0f);
	FASSERT(m_fTimer <= m_afSegTime[m_uCurSeg]);
	FASSERT(m_fTimePos >= 0.0f);
	FASSERT(m_fTimePos <= m_fTotalTime);
	CalcPos();
	CalcVel();
	CalcTang();
	CalcAccel();
}

// =============================================================================================================

void CFV3OPointPath2::CalcPos()
{
	/////////////////////////////////////////////////////////////
	// Evaluate the current position.
	CFVec3A vecTemp;

	m_vecValue.Zero();
	m_vecValue = m_pavecCoeff[m_uCurSeg].m_avecCoeff[0];
	vecTemp = m_pavecCoeff[m_uCurSeg].m_avecCoeff[1];
	vecTemp.Mul(m_fTimer);
	m_vecValue.Add(vecTemp);
	vecTemp = m_pavecCoeff[m_uCurSeg].m_avecCoeff[2];
	vecTemp.Mul(m_fTimer * m_fTimer);
	m_vecValue.Add(vecTemp);
	vecTemp = m_pavecCoeff[m_uCurSeg].m_avecCoeff[3];
	vecTemp.Mul(m_fTimer * m_fTimer * m_fTimer);
	m_vecValue.Add(vecTemp);
	//
	/////////////////////////////////////////////////////////////
}

// =============================================================================================================

void CFV3OPointPath2::CalcVel()
{
	CFVec3A vecTemp;

	/////////////////////////////////////////////////////////////
	// Calculate the velocity.
	m_vecVel.Zero();
	m_vecVel = m_pavecCoeff[m_uCurSeg].m_avecCoeff[1];
	vecTemp = m_pavecCoeff[m_uCurSeg].m_avecCoeff[2];
	vecTemp.Mul(2.0f * m_fTimer);
	m_vecVel.Add(vecTemp);
	vecTemp = m_pavecCoeff[m_uCurSeg].m_avecCoeff[3];
	vecTemp.Mul(3.0f * m_fTimer * m_fTimer);
	m_vecVel.Add(vecTemp);
	//
	/////////////////////////////////////////////////////////////
}

// =============================================================================================================

void CFV3OPointPath2::CalcTang()
{
	// TODO: Add in some facility for turning on/off the velocity and tangent calculations.

	CFVec3A vecTemp;

	/////////////////////////////////////////////////////////////
	// Calculate the geometric tangent.
	if(m_vecVel.MagSq() < 0.0001f)
	{
		// The velocity is zero, so we can't just unitize it to get the geometric tangent.
		// We will need to consider the limiting velocity so that we can get a direction.
		m_vecVel.Zero();
		m_vecVel = m_pavecCoeff[m_uCurSeg].m_avecCoeff[2];
		m_vecVel.Mul(2.0f);
		vecTemp = m_pavecCoeff[m_uCurSeg].m_avecCoeff[3];
		vecTemp.Mul(6.0f * m_fTimer);
		m_vecVel.Add(vecTemp);

		FASSERT(m_vecVel.MagSq() >= 0.0001f);
	}
	else
	{
		m_vecTang = m_vecVel;
	}
	m_vecTang.Unitize();
	//
	/////////////////////////////////////////////////////////////
}

// =============================================================================================================

void CFV3OPointPath2::CalcAccel()
{
	CFVec3A vecTemp;

	m_vecAccel.Zero();
	m_vecAccel = m_pavecCoeff[m_uCurSeg].m_avecCoeff[2];
	m_vecAccel.Mul(2.0f);
	vecTemp = m_pavecCoeff[m_uCurSeg].m_avecCoeff[3];
	vecTemp.Mul(6.0f * m_fTimer);
	m_vecAccel.Add(vecTemp);
}

// =============================================================================================================

CFV3OPointPath2 *CFV3OPointPath2::GetPool( void )
{
	return &FPointPath2_aoPool[0];
}

s32 CFV3OPointPath2::GetPoolNumEntries( void )
{
	return _NUM_POOL_ENTRIES;
}

