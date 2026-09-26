//////////////////////////////////////////////////////////////////////////////////////
// FQuatTang3.cpp - 
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

#include "FQuatTang3.h"
#include "floop.h"
#include "FVec3Obj.h"

// =============================================================================================================

u32 CFQuatTang3::m_uBufPos = 0;
u32 CFQuatTang3::m_uNumInPool = 0;
//CFQuatTang3 CFQuatTang3::m_aoPool[CFQuatTang3_uPoolSize];
CFQuatTang3 FQuatTang3_aoPool[CFQuatTang3_uPoolSize];

// =============================================================================================================

BOOL CFQuatTang3::LevelInit()
{
	s32 nIndex;
	for( nIndex = 0; nIndex < CFQuatTang3_uPoolSize; ++nIndex )
    {
		FQuatTang3_aoPool[ nIndex ].m_uFlags = FLAG_DONE;
	}

	m_uBufPos = 0; 

	return TRUE;
}

// =============================================================================================================

CFQuatTang3::CFQuatTang3()
{
	m_pV3OPath = NULL;
}

// =============================================================================================================

void CFQuatTang3::Reset(BOOL bAutoStart/* = TRUE*/)
{
	m_vecVel.Zero();
	m_vecPos.Zero();
	m_qValue.Identity();
	Calculate();
}

// =============================================================================================================

CFQuatTang3 *CFQuatTang3::GetAvailable()
{
	if(m_uBufPos == CFQuatTang3_uPoolSize)
	{
		return(NULL);
	}
	CFQuatTang3 *poQO = &(FQuatTang3_aoPool[m_uBufPos]);
	++m_uBufPos;
	FASSERT(m_uBufPos <= CFQuatTang3_uPoolSize);

	return(poQO);
}

// =============================================================================================================

BOOL CFQuatTang3::Init(CFVec3AObj *pV3OPath, BOOL bAutoStart/* = TRUE*/)
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
/*
	m_vecVel.Zero();
	m_vecPos.Zero();
	m_qValue.Identity();
	Calculate();*/
	Reset();

	m_uFlags |= FLAG_INITTED;

	if( bAutoStart ) {
		Start();
	}

	return(TRUE);
}

// =============================================================================================================

void CFQuatTang3::Work()
{
	if(!IsInitted())
	{
		return;
	}

	Calculate();
}

// =============================================================================================================

void CFQuatTang3::Calculate()
{
	const f32 fAppliedConst = 0.015f * 2.0f * 3.5f;
	const f32 fRestoreConst = 0.350f * 5.0f * 2.5f * 5.0f;
	const f32 fDampConst = 0.050f * 4.0f * 2.5f * 4.0f * 0.50f;

	f32 fSinTorqueArm, fCosTorqueArm;
	f32 fTorqueArm = FMATH_MIN(m_vecPos.Mag(), FMATH_PI * (1.0f / 2.0f));
	fmath_SinCos(fTorqueArm, &fSinTorqueArm, &fCosTorqueArm);

	/////////////////////////////////////////////////////////////
	// Get an xz acceleration vector.
	CFVec3A vecTemp, vecAccel, vecTang;
	vecTemp = m_pV3OPath->GetTang();
	vecTemp.y = 0.0f;
	// A path that is momentarily stationary or vertical has no XZ tangent. Unitizing it made NaNs
	// that spread through this object's orientation and into collision (wessstatn02); treat it as
	// facing +Z, which leaves the acceleration unrotated.
	if( !( vecTemp.MagSq() >= 1.0e-12f ) ) {	// also catches a NaN tangent
		vecTemp.Set( 0.0f, 0.0f, 1.0f );
	} else {
		vecTemp.Unitize();
	}
	vecTemp.Mul( 0.9999f);

	CFQuatA qOrient;
	qOrient.BuildQuat(vecTemp, CFVec3A(0.0f, 0.0f, 1.0f));
	vecAccel = qOrient.MulPoint(m_pV3OPath->GetAccel());
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// Calculate the applied torque based on the acceleration.
	CFVec3A vecApplied;		// This vector is representing a torque.
							// Ask Justin if you don't understand what
							//   this means.
	vecApplied.Set(vecAccel.z, 0.0f, -vecAccel.x);
	vecApplied.Mul(fAppliedConst * fCosTorqueArm);
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// Calculate the restoring torque.
	CFVec3A vecRestore;
	vecRestore = m_vecPos;
	vecRestore.Mul(-fRestoreConst * fSinTorqueArm);
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// Calculate the dampening torque.
	CFVec3A vecDamp;		// This vector is representing a torque.
	vecDamp = m_vecVel;
	vecDamp.Mul(-fDampConst);
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// Get the net torque acting on the object for this frame.
	CFVec3A vecNet;			// This vector is representing a torque.
	vecNet = vecApplied;
	vecNet.Add(vecDamp);
	vecNet.Add(vecRestore);
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// Integrate the torque with respect to time to get the 
	//   rotational impulse (like velocity), then add it to the
	//   velocity.
	CFVec3A vecImpulse;
	vecImpulse = vecNet;
	vecImpulse.Mul(FLoop_fPreviousLoopSecs);
	m_vecVel.Add(vecImpulse);
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	//
	CFVec3A vecDelPos;
	vecDelPos = m_vecVel;
	vecDelPos.Mul(FLoop_fPreviousLoopSecs);
	m_vecPos.Add(vecDelPos);

	CFQuatA qNewMotion;

	CFVec3A vecAxis;
	f32 fTheta;

//	fTheta = m_vecVel.UnitAndMag(vecAxis);
//	fTheta = m_vecVel.Mag();
//	vecAxis = m_vecVel;
//	vecAxis.Unitize();
//	m_vecVel.U

//	qNewMotion.BuildQuat(vecAxis, fTheta);
	fTheta = FMATH_MIN(m_vecPos.Mag(), FMATH_PI * (1.0f / 2.0f));
	vecAxis = m_vecPos;
	if(fTheta > 0.0f)
	{
		vecAxis.Unitize();
	}
	m_qValue.BuildQuat(vecAxis, fTheta);
//	m_qValue.Mul(qNewMotion);
	//
	/////////////////////////////////////////////////////////////
}

// =============================================================================================================

CFQuatTang3 *CFQuatTang3::GetPool( void )
{
	return &FQuatTang3_aoPool[0];
}

// =============================================================================================================

s32 CFQuatTang3::GetPoolNumEntries( void )
{
	return CFQuatTang3_uPoolSize;
}
