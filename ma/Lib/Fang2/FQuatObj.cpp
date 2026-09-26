//////////////////////////////////////////////////////////////////////////////////////
// FQuatObj.cpp - System for CFQuatA motion objects in Fang.
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
// 03/14/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "floop.h"
#include "FQuatObj.h"
#include "FQuatComp.h"
#include "FQuatConst.h"
#include "FQuatLookAt1.h"
#include "FQuatTang2.h"
#include "FQuatTang3.h"
#include "FQuatTwirl.h"
#include "FQOTang1.h"
#include "FQuatSLERP.h"

// =============================================================================================================

u32 CFQOTwirlY::m_uBufPos = 0;
u32 CFQOTwirlY::m_uNumInPool = 0;
//CFQOTwirlY CFQOTwirlY::m_aoPool[10];
CFQOTwirlY CFQOTwirlY_aoPool[10];

// =============================================================================================================

BOOL CFQuatObj::ModuleStartup()
{
	if(!CFQuatComp::ModuleStartup())
	{
		return(FALSE);
	}
	if(!CFQuatConst::ModuleStartup())
	{
		return(FALSE);
	}
	if(!CFQuatLookAt1::ModuleStartup())
	{
		return(FALSE);
	}
	if(!CFQuatTang2::ModuleStartup())
	{
		return(FALSE);
	}
	if(!CFQuatTang3::ModuleStartup())
	{
		return(FALSE);
	}
	if(!CFQuatTwirl::ModuleStartup())
	{
		return(FALSE);
	}
	if(!CFQOTang1::ModuleStartup())
	{
		return(FALSE);
	}
	if(!CFQuatSLERP::ModuleStartup())
	{
		return(FALSE);
	}

	return(TRUE);
}

// =============================================================================================================

void CFQuatObj::ModuleShutdown()
{
	CFQuatSLERP::ModuleShutdown();
	CFQOTang1::ModuleShutdown();
	CFQuatTwirl::ModuleShutdown();
	CFQuatTang3::ModuleShutdown();
	CFQuatTang2::ModuleShutdown();
	CFQuatLookAt1::ModuleShutdown();
	CFQuatConst::ModuleShutdown();
	CFQuatComp::ModuleShutdown();
}

// =============================================================================================================

BOOL CFQuatObj::LevelInit()
{
	if(!CFQuatComp::LevelInit())
	{
		return(FALSE);
	}
	if(!CFQuatConst::LevelInit())
	{
		return(FALSE);
	}
	if(!CFQuatLookAt1::LevelInit())
	{
		return(FALSE);
	}
	if(!CFQuatTang2::LevelInit())
	{
		return(FALSE);
	}
	if(!CFQuatTang3::LevelInit())
	{
		return(FALSE);
	}
	if(!CFQuatTwirl::LevelInit())
	{
		return(FALSE);
	}
	if(!CFQOTang1::LevelInit())
	{
		return(FALSE);
	}
	if(!CFQuatSLERP::LevelInit())
	{
		return(FALSE);
	}

	return(TRUE);
}

// =============================================================================================================

void CFQuatObj::LevelUninit()
{
	CFQuatSLERP::LevelUninit();
	CFQOTang1::LevelUninit();
	CFQuatTwirl::LevelUninit();
	CFQuatTang3::LevelUninit();
	CFQuatTang2::LevelUninit();
	CFQuatLookAt1::LevelUninit();
	CFQuatConst::LevelUninit();
	CFQuatComp::LevelUninit();
}

// =============================================================================================================

CFQOTwirlY::CFQOTwirlY()
{
}

// =============================================================================================================

CFQOTwirlY::~CFQOTwirlY()
{
}

// =============================================================================================================

CFQOTwirlY *CFQOTwirlY::GetAvailable()
{
//	CFQOTwirlY *poSO = &(m_aoPool[m_uBufPos]);
	CFQOTwirlY *poSO = &(CFQOTwirlY_aoPool[m_uBufPos]);
	++m_uBufPos;
	if(m_uBufPos == 10)
		m_uBufPos = 0;
	FASSERT(m_uBufPos < 10);

	return(poSO);
}

// =============================================================================================================

BOOL CFQOTwirlY::Init(CFQuatA &qInitValue, f32 fOmegaY, BOOL bAutoStart/* = TRUE*/)
{
	m_qValue = qInitValue;
	m_qInitValue = qInitValue;
	m_fOmegaY = fOmegaY;
	m_fTimer = 0.0f;

	m_uFlags |= FLAG_INITTED;
	Start();

	return(TRUE);
}

// =============================================================================================================

void CFQOTwirlY::Work()
{
	m_fTimer += FLoop_fPreviousLoopSecs;
	f32 fThetaY = m_fOmegaY * m_fTimer;

	CFQuatA qTemp;
	f32 fTempSin, fTempCos;
	fmath_SinCos(fThetaY, &fTempSin, &fTempCos);

	qTemp.x = 0.0f;
	qTemp.y = fTempSin;
	qTemp.z = 0.0f;
	qTemp.w = fTempCos;

	m_qValue = m_qInitValue;
	m_qValue.Mul(qTemp);
}

// =============================================================================================================
// =============================================================================================================
// =============================================================================================================
