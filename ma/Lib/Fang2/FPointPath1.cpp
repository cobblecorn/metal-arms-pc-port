//////////////////////////////////////////////////////////////////////////////////////
// FPointPath1.cpp - CFVec3Obj defining a path from an ordered set of points.
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
// 04/03/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "FPointPath1.h"
#include "floop.h"

// =============================================================================================================
#define _NUM_POOL_ENTRIES	( 10 )
u32 CFV3OPointPath1::m_uBufPos = 0;
u32 CFV3OPointPath1::m_uNumInPool = 0;
//CFV3OPointPath1 CFV3OPointPath1::m_aoPool[10];
static CFV3OPointPath1 s_aoPool[_NUM_POOL_ENTRIES];

// =============================================================================================================

BOOL CFV3OPointPath1::LevelInit()
{
	s32 nIndex;
	for( nIndex = 0; nIndex < _NUM_POOL_ENTRIES; ++nIndex )
	{
		s_aoPool[ nIndex ].m_uFlags = FLAG_DONE;
	}

	m_uBufPos = 0;
	return(TRUE);
}



CFV3OPointPath1::CFV3OPointPath1()
{
}

// =============================================================================================================

CFV3OPointPath1 *CFV3OPointPath1::GetAvailable()
{
	if(m_uBufPos == 10)
	{
		return(NULL);
	}
	FASSERT(m_uBufPos < 10);
	CFV3OPointPath1 *poSO = &(s_aoPool[m_uBufPos]);
	++m_uBufPos;

//	FASSERT(!poSO->IsActive());
//	poSO->m_uFlags |= FLAG_ACTIVE;

	return(poSO);
}

// =============================================================================================================

BOOL CFV3OPointPath1::Init(const CFVec3A *aPtList, u32 uNumPts, f32 fTotalTime, BOOL bRestAtEnds, BOOL bAutoStart/* = TRUE*/)
{
	FASSERT(aPtList != NULL);
	FASSERT(uNumPts <= FPointPath1_uMaxPts);

	m_uNumPts = uNumPts;
	m_uNumSegs = uNumPts - 1;
	m_fTotalTime = fTotalTime;

	f32 afSegLen[FPointPath1_uMaxPts - 1];
	f32 afTimeStamp[FPointPath1_uMaxPts];
	f32 afSegLenNorm[FPointPath1_uMaxPts - 1];
	const u32 uMatrixSize = (FPointPath1_uMaxPts + 3) * (FPointPath1_uMaxPts + 2);
	f32 afMatrix[uMatrixSize];

	/////////////////////////////////////////////////////////////
	// Find the length of each segment and of the entire path.
	f32 fTempLen, fTotalLen = 0.0f;
	CFVec3A vecTemp;
	u32 uCurSeg;
	for(uCurSeg = 0; uCurSeg < m_uNumSegs; ++ uCurSeg)
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
	// Calculate the relative length of each segment, along with
	//   the timestamp at each point.
	afTimeStamp[0] = 0.0f;
	f32 fOOTotalLen = 1.0f / fTotalLen;
	for(uCurSeg = 0; uCurSeg < m_uNumSegs; ++uCurSeg)
	{
		afSegLenNorm[uCurSeg] = afSegLen[uCurSeg] * fOOTotalLen;
		afTimeStamp[uCurSeg + 1] = afTimeStamp[uCurSeg] + afSegLenNorm[uCurSeg] * fTotalTime;
	}
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// Get the coefficients for each dimension.
	u32 uDim;
	for(uDim = 0; uDim < 3; ++uDim)
	{
		/////////////////////////////////////////////////////////////
		// Set up the augmented matrix that we will solve to get the
		//   coefficients.
		// TODO: This is assuming that they want to start from/stop at rest.
		u32 uNumCols = uNumPts + 3, uNumRows = uNumPts + 2;
		u32 uCurRow, uCurCol, uCurIdx = 0;
		f32 fCurT;
			/////////////////////////////////////////////////////////////
			// Set up constraints that the curve fit through the given
			//   points and at the proper times.
			for(uCurRow = 0; uCurRow < uNumPts; ++uCurRow)
			{
				// Fill in the body of the matrix.
				fCurT = 1.0f;
				for(uCurCol = 0; uCurCol < uNumRows; ++uCurCol)
				{
					FASSERT(uCurIdx < uMatrixSize);
					afMatrix[uCurIdx] = fCurT;
					++uCurIdx;
					fCurT *= afTimeStamp[uCurRow];
				}
				
				// Fill in the augmented column.
				afMatrix[uCurIdx] = aPtList[uCurRow].a[uDim];
				++uCurIdx;
			}
			//
			/////////////////////////////////////////////////////////////

			/////////////////////////////////////////////////////////////
			// Set up the constraints that the path start and stop with
			//   zero velocity.
			f32 fMultiplier;

				/////////////////////////////////////////////////////////////
				// Set up the constraint to start at rest.
				afMatrix[uCurIdx] = 0.0f;
				++uCurIdx;
				fMultiplier = 1.0f;
				fCurT = 1.0f;
				for(uCurCol = 1; uCurCol < uNumRows; ++uCurCol)
				{
					afMatrix[uCurIdx] = fMultiplier * fCurT;
					++uCurIdx;
					fMultiplier += 1.0f;
					fCurT *= afTimeStamp[0];
				}
				afMatrix[uCurIdx] = 0.0f;
				++uCurIdx;
				//
				/////////////////////////////////////////////////////////////

				/////////////////////////////////////////////////////////////
				// Set up the constraint to stop at rest.
				afMatrix[uCurIdx] = 0.0f;
				++uCurIdx;
				fMultiplier = 1.0f;
				fCurT = 1.0f;
				for(uCurCol = 1; uCurCol < uNumRows; ++uCurCol)
				{
					afMatrix[uCurIdx] = fMultiplier * fCurT;
					++uCurIdx;
					fMultiplier += 1.0f;
					fCurT *= afTimeStamp[uNumPts - 1];
				}
				afMatrix[uCurIdx] = 0.0f;
				++uCurIdx;
				//
				/////////////////////////////////////////////////////////////
			//
			/////////////////////////////////////////////////////////////
		//
		/////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////
		// Get the matrix into upper eschelon form.
		u32 uSrcRow, uDestRow;
		u32 uSrcIdx, uDestIdx;
		f32 fDiagEntry, fOODiagEntry, fLeadEntry;
		for(uSrcRow = 0; uSrcRow < uNumRows; ++uSrcRow)
		{
			/////////////////////////////////////////////////////////////
			// Get a 1.0 in diagonal entry by dividing the row by the 
			//   diagonal entry.
			uCurIdx = uSrcRow * uNumCols + uSrcRow;
			fDiagEntry = afMatrix[uCurIdx];
			FASSERT(fDiagEntry != 0.0f);
			fOODiagEntry = 1.0f / fDiagEntry;
			afMatrix[uCurIdx] = 1.0f;
			for(uCurCol = uSrcRow + 1; uCurCol < uNumCols; ++uCurCol)
			{
				++uCurIdx;
				afMatrix[uCurIdx] *= fOODiagEntry;
			}
			//
			/////////////////////////////////////////////////////////////

			/////////////////////////////////////////////////////////////
			// Use this row to clear out the rows below it.
			for(uDestRow = uSrcRow + 1; uDestRow < uNumRows; ++uDestRow)
			{
				uSrcIdx = uSrcRow * uNumCols + uSrcRow;
				uDestIdx = uDestRow * uNumCols + uSrcRow;
				fLeadEntry = afMatrix[uDestIdx];
				afMatrix[uDestIdx] = 0.0f;
				for(uCurCol = uSrcRow + 1; uCurCol < uNumCols; ++uCurCol)
				{
					++uDestIdx;
					++uSrcIdx;
					afMatrix[uDestIdx] -= fLeadEntry * afMatrix[uSrcIdx];
				}
			}
			//
			/////////////////////////////////////////////////////////////
		}
		//
		/////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////
		// Do the 'backward' phase of row reduction.
		f32 fLeadingCoeff;
		u32 uK;
		for(uK = 1; uK < uNumRows; ++uK)
		{
			for(uCurRow = 0; uCurRow < uK; ++uCurRow)
			{
				fLeadingCoeff = afMatrix[uCurRow * (uNumCols) + uK];
				for(uCurCol = 0; uCurCol < uNumCols; ++uCurCol)
				{
					afMatrix[uCurRow * uNumCols + uCurCol] -= (afMatrix[uK * uNumCols + uCurCol] * fLeadingCoeff);
				}
			}
		}
		//
		/////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////
		// Copy the solution of the system into our coefficient
		//   array.
		uCurIdx = uNumRows;
		for(uCurRow = 0; uCurRow < uNumRows; ++uCurRow)
		{
			m_avecCoeff[uCurRow].a[uDim] = afMatrix[uCurIdx];
			uCurIdx += uNumCols;
		}
		//
		/////////////////////////////////////////////////////////////
	}
	//
	/////////////////////////////////////////////////////////////

	m_vecValue = aPtList[0];
	m_vecVel.Zero();
	m_vecTang = m_avecCoeff[2];
	m_vecTang.Unitize();

//	AddToWorkList(this);
//	m_uFlags |= FLAG_ACTIVE;

	m_fTimer = 0.0f;

	m_uFlags |= FLAG_INITTED;
	m_uFlags &= ~(FLAG_WORKING);

	if(bAutoStart)
	{
		Start();
	}

	return(TRUE);
}

// =============================================================================================================

void CFV3OPointPath1::Work()
{
	m_fTimer += FLoop_fPreviousLoopSecs;

	if(m_fTimer >= m_fTotalTime)
	{
		// TODO: This should set a final value.
		End();
		return;
	}

	/////////////////////////////////////////////////////////////
	// Use the precalculated coefficients to evaluate our
	//   function at the current point in time.
	u32 uCurTerm;
	f32 fCurT, fFactor;
	CFVec3A vecTemp;

	m_vecValue.Zero();
	vecTemp.Zero();

	fCurT = 1.0f;
	for(uCurTerm = 0; uCurTerm < (m_uNumPts + 2); ++uCurTerm)
	{
		vecTemp = m_avecCoeff[uCurTerm];
		vecTemp.Mul(fCurT);
		m_vecValue.Add(vecTemp);
		fCurT *= m_fTimer;
	}
	//
	/////////////////////////////////////////////////////////////

	//// If requested, calculate the velocity and the geometric tangent.
	//
	// TODO: Add in some facility for turning on/off the velocity and tangent calculations.
	m_vecVel.Zero();

	fCurT = 1.0f;
	fFactor = 1.0f;
	for(uCurTerm = 1; uCurTerm < (m_uNumPts + 2); ++uCurTerm)
	{
		vecTemp = m_avecCoeff[uCurTerm];
		vecTemp.Mul(fCurT * fFactor);
		m_vecVel.Add(vecTemp);
		fCurT *= m_fTimer;
		fFactor += 1.0f;
	}

	m_vecTang = m_vecVel;
	m_vecTang.Unitize();
	//
	////
}

// =============================================================================================================

void CFV3OPointPath1::PrintMtx(f32 *afMtx, u32 uNumCols, u32 uNumRows)
{
	u32 uCurCol, uCurRow;
	for(uCurRow = 0; uCurRow < uNumRows; ++uCurRow)
	{
		DEVPRINTF("[ ");
		for(uCurCol = 0; uCurCol < uNumCols; ++uCurCol)
		{
			DEVPRINTF("%f ", afMtx[uCurRow * uNumCols + uCurCol]);
		}
		DEVPRINTF("]\n");
	}
}

// =============================================================================================================

CFV3OPointPath1 *CFV3OPointPath1::GetPool( void )
{
	return &s_aoPool[0];
}

s32 CFV3OPointPath1::GetPoolNumEntries( void )
{
	return _NUM_POOL_ENTRIES;
}

