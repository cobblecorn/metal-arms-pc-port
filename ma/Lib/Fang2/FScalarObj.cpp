//////////////////////////////////////////////////////////////////////////////////////
// FScalarObj.cpp - System for scalar motion objects in Fang.
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
// 03/11/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fmath.h"
#include "floop.h"
#include "FScalarObj.h"
#include "FScalarConst.h"
#include "FScalarSinus.h"
#include "FScalarBlend.h"

// =============================================================================================================

BOOL CFScalarObj::ModuleStartup()
{
	CFScalarConst::ModuleStartup();
	CFScalarSinus::ModuleStartup();
	CFScalarBlend::ModuleStartup();

	return(TRUE);
}

// =============================================================================================================

void CFScalarObj::ModuleShutdown()
{
	CFScalarBlend::ModuleShutdown();
	CFScalarSinus::ModuleShutdown();
	CFScalarConst::ModuleShutdown();
}

// =============================================================================================================

BOOL CFScalarObj::LevelInit()
{
	CFScalarConst::LevelInit();
	CFScalarSinus::LevelInit();
	CFScalarBlend::LevelInit();

	return(TRUE);
}

// =============================================================================================================

void CFScalarObj::LevelUninit()
{
	CFScalarBlend::LevelUninit();
	CFScalarSinus::LevelUninit();
	CFScalarConst::LevelUninit();
}

// =============================================================================================================
// =============================================================================================================
// =============================================================================================================
// =============================================================================================================
// =============================================================================================================
