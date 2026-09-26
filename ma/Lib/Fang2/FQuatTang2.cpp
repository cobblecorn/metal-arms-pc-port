//////////////////////////////////////////////////////////////////////////////////////
// FQuatTang2.cpp - Hanging tangential orientation object.
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
// 04/19/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "FQuatTang2.h"
#include "floop.h"
#include "FVec3Obj.h"

// =============================================================================================================

u32 CFQuatTang2::m_uBufPos = 0;
u32 CFQuatTang2::m_uNumInPool = 0;
//CFQuatTang2 CFQuatTang2::m_aoPool[CFQuatTang2_uPoolSize];
static CFQuatTang2 s_aoPool[CFQuatTang2_uPoolSize];

// =============================================================================================================

BOOL CFQuatTang2::LevelInit()
{
	s32 nIndex;
	for( nIndex = 0; nIndex < CFQuatTang2_uPoolSize; ++nIndex )
	{
		s_aoPool[ nIndex ].m_uFlags = FLAG_DONE;
	}

	m_uBufPos = 0; 

	return TRUE;
}

// =============================================================================================================

CFQuatTang2::CFQuatTang2()
{
	m_pV3OPath = NULL;
}

// =============================================================================================================

CFQuatTang2 *CFQuatTang2::GetAvailable()
{
	if(m_uBufPos == CFQuatTang2_uPoolSize)
	{
		return(NULL);
	}
	CFQuatTang2 *poQO = &(s_aoPool[m_uBufPos]);
	++m_uBufPos;
	FASSERT(m_uBufPos <= CFQuatTang2_uPoolSize);

	return(poQO);
}

// =============================================================================================================

BOOL CFQuatTang2::Init(CFVec3AObj *pV3OPath, BOOL bAutoStart/* = TRUE*/)
{
//	FASSERT(pV3OPath != NULL);
	if(pV3OPath == NULL)
	{
		return(FALSE);
	}
	if(!pV3OPath->IsInitted())
	{
		return(FALSE);
	}

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

void CFQuatTang2::Work()
{
	FASSERT(IsWorking());

	if(!IsInitted())
	{
		return;
	}

	Calculate();
}

// =============================================================================================================

void CFQuatTang2::Calculate()
{
	/////////////////////////////////////////////////////////////
	// Get the direction in which we want to be looking.
	CFVec3A vecLook;
	vecLook.Set(m_pV3OPath->GetTang());
	vecLook.UnitizeXZ();
	vecLook.y = 0.0f;
//	FASSERT_UNIT_FLOAT(vecLook.MagSq());
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// Calculate the 'right' vector.
	CFVec3A vecRight;
	vecRight.x = vecLook.z;
	vecRight.y = 0.0f;
	vecRight.z = -vecLook.x;
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
	CFMtx43A mtxTemp;
	mtxTemp.m_vX = vecRight;
	mtxTemp.m_vY = vecUp;
	mtxTemp.m_vZ = vecLook;
	// TODO: This might not be necessary.
	mtxTemp.m_vP.Set(0.0f, 0.0f, 0.0f);

	m_qValue.BuildQuat(mtxTemp);
	//
	/////////////////////////////////////////////////////////////
}

// =============================================================================================================

CFQuatTang2 *CFQuatTang2::GetPool( void )
{
	return &s_aoPool[0];
}

// =============================================================================================================

s32 CFQuatTang2::GetPoolNumEntries( void )
{
	return CFQuatTang2_uPoolSize;
}
