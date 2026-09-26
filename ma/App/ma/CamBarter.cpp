//////////////////////////////////////////////////////////////////////////////////////
// CamBarter.cpp - Barter Cameraman class for Mettle Arms.
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
// 06/03/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "CamBarter.h"
#include "fcamera.h"
#include "gamecam.h"
#include "floop.h"

// =============================================================================================================

static const f32 _fCamMoveInTime = 0.5f;
static const f32 _fCamMoveOutTime = 0.5f;

static const f32 _fOOCamMoveInTime = 1.0f / _fCamMoveInTime;
static const f32 _fOOCamMoveOutTime = 1.0f / _fCamMoveOutTime;

// =============================================================================================================

CCamBarter::CCamBarter()
{
	Reset();
}

// =============================================================================================================

// This is a static function.
BOOL CCamBarter::InitSystem()
{
	return(TRUE);
}

// =============================================================================================================

// This is a static function.
void CCamBarter::UninitSystem()
{
}

// =============================================================================================================

BOOL CCamBarter::Init(CCamBarterInfo *pCamInfo)
{
	if(m_pCamera == NULL)
	{
		DEVPRINTF("CCamBarter::Init() : No camera assigned, call AssignCamera() before Init().\n" );
		return(FALSE);
	}

	if(pCamInfo == NULL)
	{
		DEVPRINTF("CCamBarter::Init() : CCamBarter requires a CCamBarterInfo to init.\n");
		return(FALSE);
	}

	Reset();
	m_pCamInfo = pCamInfo;

	//////////////////////////////////////////////////////////////////////
	// Record the starting camera matrix (we will restore it later and need
	//   for our positioning).
	CFCamera *pCurCamera = gamecam_GetActiveCamera();
 	FASSERT(pCurCamera != NULL);
	m_xfmCamStart = *pCurCamera->GetXfmWithoutShake();
	m_xfmCamStart.Invert();

	m_qStart.BuildQuat(m_xfmCamStart.m_MtxF);

	m_vecStart = m_xfmCamStart.m_MtxF.m_vPos;
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Set up the final position information.
		//////////////////////////////////////////////////////////////////////
		// Set up our final orientation (taking into account the orientation
		//   of the barter droids).
		CFQuatA qPitch, qTurnaround;

		// Create a rotation so that we are *facing* the barter bots.
		qPitch.BuildQuat(CFVec3A::m_UnitAxisY, FMATH_DEG2RAD(180.0f));

		// Create the downward tilt.
		qTurnaround.BuildQuat(CFVec3A::m_UnitAxisX, FMATH_DEG2RAD(15.0f));

		// Concatenate everything to form our final orientation.
		m_qFinal.BuildQuat(pCamInfo->m_xfmBarterPos.m_MtxF);
		m_qFinal.Mul(qPitch);
		m_qFinal.Mul(qTurnaround);
		//
		//////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////
		// Set up our final position (taking into account the position of the
		//   barter droids).
		CFVec3A vecRight, vecUp, vecForward;

		vecRight.Set(pCamInfo->m_xfmBarterPos.m_MtxF.m_vX);
		vecRight.Mul(2.25f);

		vecUp.Set(pCamInfo->m_xfmBarterPos.m_MtxF.m_vY);
		vecUp.Mul(5.5f);

		vecForward.Set(pCamInfo->m_xfmBarterPos.m_MtxF.m_vZ);
		vecForward.Mul(10.0f);

		m_vecFinal.Set(pCamInfo->m_xfmBarterPos.m_MtxF.m_vPos);
		m_vecFinal.Add(vecRight);
		m_vecFinal.Add(vecUp);
		m_vecFinal.Add(vecForward);
		//
		//////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////

	_GotoStartPos();
	m_pCamera->SetFOV(pCamInfo->m_fHalfFOV);

	return(TRUE);
}

// =============================================================================================================

void CCamBarter::Reset()
{
	m_eState = CAMSTATE_OUT;
	m_pCamInfo = NULL;
}

// =============================================================================================================

void CCamBarter::Work()
{
	if(m_pCamInfo == NULL)
	{
		return;
	}

	if(m_bFreezeShot)
	{
		return;
	}

	_Decisions();
	_Work();
}

// =============================================================================================================

void CCamBarter::_Decisions()
{
	switch(m_eState)
	{
		case CAMSTATE_OUT:
		{
			break;
		}
		case CAMSTATE_MOVINGIN:
		{
			m_fCurTimePos += FLoop_fRealPreviousLoopSecs;

			if(m_fCurTimePos >= _fCamMoveInTime)
			{
				_GotoFinalPos();
				m_eState = CAMSTATE_IN;
			}

			break;
		}
		case CAMSTATE_IN:
		{

			break;
		}
		case CAMSTATE_MOVINGOUT:
		{
			m_fCurTimePos -= FLoop_fRealPreviousLoopSecs;

			if(m_fCurTimePos <= 0.0f)
			{
				_GotoStartPos();
				m_eState = CAMSTATE_OUT;
			}

			break;
		}
	}
}

// =============================================================================================================

void CCamBarter::_Work()
{
	switch(m_eState)
	{
		case CAMSTATE_OUT:
		{
			break;
		}
		case CAMSTATE_MOVINGIN:
		{
			//////////////////////////////////////////////////////////////////////
			// Set up the unit weights.
			f32 fUnitTimePos = m_fCurTimePos * _fOOCamMoveInTime;
			fUnitTimePos = fmath_UnitLinearToSCurve(fUnitTimePos);

			FASSERT_UNIT_FLOAT(fUnitTimePos);
			f32 fWeight1, fWeight2;
			fWeight1 = 1.0f - fUnitTimePos;
			fWeight2 = fUnitTimePos;
			//
			//////////////////////////////////////////////////////////////////////

			_CalcCamPosOri(fWeight1, fWeight2);

			break;
		}
		case CAMSTATE_IN:
		{
			break;
		}
		case CAMSTATE_MOVINGOUT:
		{
			//////////////////////////////////////////////////////////////////////
			// Set up the unit weights.
			f32 fUnitTimePos = m_fCurTimePos * _fOOCamMoveInTime;
			fUnitTimePos = fmath_UnitLinearToSCurve(fUnitTimePos);

			FASSERT_UNIT_FLOAT(fUnitTimePos);
			f32 fWeight1, fWeight2;
			fWeight1 = 1.0f - fUnitTimePos;
			fWeight2 = fUnitTimePos;
			//
			//////////////////////////////////////////////////////////////////////

			_CalcCamPosOri(fWeight1, fWeight2);

			break;
		}
	}
}

// =============================================================================================================

void CCamBarter::MoveToFinalPos()
{
	_MoveToFinalPos();
}

// =============================================================================================================

void CCamBarter::MoveToStartPos()
{
	_MoveToStartPos();
}

// =============================================================================================================

void CCamBarter::_MoveToFinalPos()
{
	m_fCurTimePos = 0.0f;

	m_eState = CAMSTATE_MOVINGIN;
}

// =============================================================================================================

void CCamBarter::_GotoFinalPos()
{
	_CalcCamPosOri(0.0f, 1.0f);

	m_eState = CAMSTATE_IN;
}

// =============================================================================================================

void CCamBarter::_MoveToStartPos()
{
	m_fCurTimePos = _fCamMoveOutTime;

	m_eState = CAMSTATE_MOVINGOUT;
}

// =============================================================================================================

void CCamBarter::_GotoStartPos()
{
	_CalcCamPosOri(1.0f, 0.0f);

	m_eState = CAMSTATE_OUT;
}

// =============================================================================================================

void CCamBarter::_CalcCamPosOri(f32 fWeight1, f32 fWeight2)
{
	//////////////////////////////////////////////////////////////////////
	// Calculate the orientation.
	CFQuatA qCurrent;
	qCurrent.ReceiveSlerpOf(fWeight2, m_qStart, m_qFinal);
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Calculate the position.
	CFVec3A vecCurrent, vecTemp;

	vecCurrent.Set(m_vecStart);
	vecCurrent.Mul(fWeight1);
	vecTemp.Set(m_vecFinal);
	vecTemp.Mul(fWeight2);
	vecCurrent.Add(vecTemp);
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Set the camera up.
	m_pCameraData->m_Xfm.BuildFromInvQuat(qCurrent, vecCurrent);
	//
	//////////////////////////////////////////////////////////////////////
}

// =============================================================================================================
