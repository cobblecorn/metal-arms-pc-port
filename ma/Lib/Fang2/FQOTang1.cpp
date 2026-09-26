//////////////////////////////////////////////////////////////////////////////////////
// FQOTang1.cpp - CFVec3Obj defining a path from an ordered set of points.
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
// 04/09/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "FQOTang1.h"
#include "floop.h"
#include "FVec3Obj.h"

// =============================================================================================================

u32 CFQOTang1::m_uBufPos = 0;
u32 CFQOTang1::m_uNumInPool = 0;
//CFQOTang1 CFQOTang1::m_aoPool[CFQOTang1_uPoolSize];
CFQOTang1 FQOTang1_aoPool[CFQOTang1_uPoolSize];

// =============================================================================================================

BOOL CFQOTang1::LevelInit()
{
	s32 nIndex;
	for( nIndex = 0; nIndex < CFQOTang1_uPoolSize; ++nIndex )
	{
		FQOTang1_aoPool[ nIndex ].m_uFlags = FLAG_DONE;
	}

	m_uBufPos = 0; 

	return TRUE;
}

// =============================================================================================================

CFQOTang1::CFQOTang1()
{
	m_pV3OPath = NULL;
}

// =============================================================================================================

CFQOTang1 *CFQOTang1::GetAvailable()
{
	if(m_uBufPos == CFQOTang1_uPoolSize)
	{
		return(NULL);
	}
//	CFQOTang1 *poSO = &(m_aoPool[m_uBufPos]);
	CFQOTang1 *poSO = &(FQOTang1_aoPool[m_uBufPos]);
	++m_uBufPos;
	FASSERT(m_uBufPos <= CFQOTang1_uPoolSize);

	return(poSO);
}

// =============================================================================================================

BOOL CFQOTang1::Init(CFVec3AObj *pV3OPath, BOOL bAutoStart/* = TRUE*/)
{
	FASSERT(pV3OPath != NULL);

	if(m_pV3OPath != NULL)
	{
		// Let the old object know that we no longer need a tangent from it.
		m_pV3OPath->SetTangVel(FALSE);
	}

	// Set up the new one to give us a tangent vector.
	m_pV3OPath = pV3OPath;
	pV3OPath->SetTangVel(TRUE);

	Calculate();

	m_uFlags |= FLAG_INITTED;
	
	if( bAutoStart ) {
		Start();
	}

	return(TRUE);
}

// =============================================================================================================

void CFQOTang1::Work()
{
	Calculate();
}

// =============================================================================================================

void CFQOTang1::Calculate()
{
	CFMtx43A mtxTemp;
	mtxTemp.m_vZ = m_pV3OPath->GetTang();
	mtxTemp.m_vX.x = mtxTemp.m_vZ.z;
	mtxTemp.m_vX.y = 0.0f;
	mtxTemp.m_vX.z = -mtxTemp.m_vZ.x;
	if (mtxTemp.m_vX.SafeUnitAndMag(mtxTemp.m_vX) > 0.0f)
	{
		mtxTemp.m_vY.Cross(mtxTemp.m_vZ, mtxTemp.m_vX);
	}
	else
	{
		if (mtxTemp.m_vZ.y > 0.0f)
		{
			mtxTemp.m_vZ = CFVec3A::m_UnitAxisY;
			mtxTemp.m_vY = CFVec3A::m_UnitAxisX;
			mtxTemp.m_vX = CFVec3A::m_UnitAxisZ;
		}
		else
		{
			mtxTemp.m_vZ = CFVec3A::m_UnitAxisY;
			mtxTemp.m_vZ.y = -1.0f;
			mtxTemp.m_vY = CFVec3A::m_UnitAxisX;
			mtxTemp.m_vX = CFVec3A::m_UnitAxisZ;
		}
	}
	m_qValue.BuildQuat(mtxTemp);

/* pgm commented this out
	/////////////////////////////////////////////////////////////
	// Get the direction in which we want to be looking.
	CFVec3A vecLook;
	vecLook.Set();

	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// Calculate the 'right' vector.
	CFVec3A vecRight;
	vecRight.x = vecLook.z;
	vecRight.y = 0.0f;
	vecRight.z = -vecLook.x;
	FASSERT(vecRight.SafeUnitAndMag(vecRight)> 0.0f);
	vecRight.Unitize();


	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// Calculate the 'up' vector.
	CFVec3A vecUp;
	vecUp.Cross(vecLook, vecRight);
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// Calculate the quaternion.
	mtxTemp.m_vX = vecRight;
	mtxTemp.m_vY = vecUp;
	mtxTemp.m_vZ = vecLook;
	// TODO: This might not be necessary.
	mtxTemp.m_vP.Set(0.0f, 0.0f, 0.0f);
	m_qValue.BuildQuat(mtxTemp);

	//
	/////////////////////////////////////////////////////////////
*/
}

// =============================================================================================================

CFQOTang1 *CFQOTang1::GetPool( void )
{
	return &FQOTang1_aoPool[0];
}

// =============================================================================================================

s32 CFQOTang1::GetPoolNumEntries( void )
{
	return CFQOTang1_uPoolSize;
}
