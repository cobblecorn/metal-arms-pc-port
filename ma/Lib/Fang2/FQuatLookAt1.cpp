//////////////////////////////////////////////////////////////////////////////////////
// FQuatLookAt1.cpp - Quaternion look at object.
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
// 04/30/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "FQuatLookAt1.h"
#include "FVec3Obj.h"

// =============================================================================================================

u32 CFQuatLookAt1::m_uBufPos = 0;
u32 CFQuatLookAt1::m_uNumInPool = 0;
//CFQuatLookAt1 CFQuatLookAt1::m_aoPool[CFQuatLookAt1_uPoolSize];
static CFQuatLookAt1 s_aoPool[CFQuatLookAt1_uPoolSize];

// =============================================================================================================

CFQuatLookAt1::CFQuatLookAt1()
{
}

// =============================================================================================================

CFQuatLookAt1 *CFQuatLookAt1::GetAvailable()
{
	if(m_uBufPos == CFQuatLookAt1_uPoolSize)
	{
		return(NULL);
	}
	CFQuatLookAt1 *poQO = &(s_aoPool[m_uBufPos]);
	++m_uBufPos;
	FASSERT(m_uBufPos <= CFQuatLookAt1_uPoolSize);

	return(poQO);
}

// =============================================================================================================

BOOL CFQuatLookAt1::Init(CFVec3AObj *pV3OLookFrom, CFVec3AObj *pV3OLookTo, BOOL bAutoStart/* = TRUE*/)
{
	FASSERT(pV3OLookFrom != NULL);
	FASSERT(pV3OLookTo != NULL);

	m_pV3OLookFrom = pV3OLookFrom;
	m_pV3OLookTo = pV3OLookTo;

	// Calculate the direction in which we want to be looking.
	CFVec3A vecLook;
	vecLook = pV3OLookTo->GetValue();
	vecLook.Sub(pV3OLookFrom->GetValue());
	vecLook.Unitize();

	// Calculate the 'right' vector.
	CFVec3A vecRight;
	vecRight.x = vecLook.z;
	vecRight.y = 0.0f;
	vecRight.z = -vecLook.x;
	vecRight.Unitize();

	// Calculate the new 'up' vector.
	CFVec3A vecUp;
	vecUp.Cross(vecLook, vecRight);

	CFMtx43A mtxTemp;
	mtxTemp.m_vX = vecRight;
	mtxTemp.m_vY = vecUp;
	mtxTemp.m_vZ = vecLook;
	mtxTemp.m_vP.Set(0.0f, 0.0f, 0.0f);

	m_qValue.BuildQuat(mtxTemp);

//	m_bIsActive = TRUE;
//	AddToWorkList(this);
	m_uFlags |= FLAG_INITTED;
	Start();

	return(TRUE);
}

// =============================================================================================================

void CFQuatLookAt1::Work()
{
	// Calculate the direction in which we want to be looking.
	CFVec3A vecLook;
	vecLook = m_pV3OLookTo->GetValue();
	vecLook.Sub(m_pV3OLookFrom->GetValue());
	vecLook.Unitize();

	// Calculate the 'right' vector.
	CFVec3A vecRight;
	vecRight.x = vecLook.z;
	vecRight.y = 0.0f;
	vecRight.z = -vecLook.x;
	vecRight.Unitize();

	// Calculate the new 'up' vector.
	CFVec3A vecUp;
	vecUp.Cross(vecLook, vecRight);

	// Get a matrix from these bases.
	CFMtx43A mtxTemp;
	mtxTemp.m_vX = vecRight;
	mtxTemp.m_vY = vecUp;
	mtxTemp.m_vZ = vecLook;
	mtxTemp.m_vP.Set(0.0f, 0.0f, 0.0f);

	// Get a quat from the matrix.
	m_qValue.BuildQuat(mtxTemp);
}

// =============================================================================================================