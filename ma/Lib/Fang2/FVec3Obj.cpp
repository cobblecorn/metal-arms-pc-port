//////////////////////////////////////////////////////////////////////////////////////
// FVec3Obj.cpp - System for CFVec3 motion objects in Fang.
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

#include "FVec3Obj.h"
#include "FMulPath1.h"
#include "FPointPath1.h"
#include "FPointPath2.h"
#include "FSumPath1.h"
#include "FV3OCircleXZ.h"
#include "FV3OConst.h"
#include "FV3OLine1.h"

// =============================================================================================================

BOOL CFVec3AObj::ModuleStartup()
{
	if(!CFV3OMulPath1::ModuleStartup())
	{
		return(FALSE);
	}
	if(!CFV3OPointPath1::ModuleStartup())
	{
		return(FALSE);
	}
	if(!CFV3OPointPath2::ModuleStartup())
	{
		return(FALSE);
	}
	if(!CFV3OSumPath1::ModuleStartup())
	{
		return(FALSE);
	}
	if(!CFV3OCircleXZ::ModuleStartup())
	{
		return(FALSE);
	}
	if(!CFV3OConst::ModuleStartup())
	{
		return(FALSE);
	}
	if(!CFV3OLine1::ModuleStartup())
	{
		return(FALSE);
	}

	return(TRUE);
}

// =============================================================================================================

void CFVec3AObj::ModuleShutdown()
{
	CFV3OLine1::ModuleShutdown();
	CFV3OConst::ModuleShutdown();
	CFV3OCircleXZ::ModuleShutdown();
	CFV3OSumPath1::ModuleShutdown();
	CFV3OPointPath2::ModuleShutdown();
	CFV3OPointPath1::ModuleShutdown();
	CFV3OMulPath1::ModuleShutdown();
}

// =============================================================================================================

BOOL CFVec3AObj::LevelInit()
{
	if(!CFV3OMulPath1::LevelInit())
	{
		return(FALSE);
	}
	if(!CFV3OPointPath1::LevelInit())
	{
		return(FALSE);
	}
	if(!CFV3OPointPath2::LevelInit())
	{
		return(FALSE);
	}
	if(!CFV3OSumPath1::LevelInit())
	{
		return(FALSE);
	}
	if(!CFV3OCircleXZ::LevelInit())
	{
		return(FALSE);
	}
	if(!CFV3OConst::LevelInit())
	{
		return(FALSE);
	}
	if(!CFV3OLine1::LevelInit())
	{
		return(FALSE);
	}

	return(TRUE);
}

// =============================================================================================================

void CFVec3AObj::LevelUninit()
{
	CFV3OLine1::LevelUninit();
	CFV3OConst::LevelUninit();
	CFV3OCircleXZ::LevelUninit();
	CFV3OSumPath1::LevelUninit();
	CFV3OPointPath2::LevelUninit();
	CFV3OPointPath1::LevelUninit();
	CFV3OMulPath1::LevelUninit();
}

// =============================================================================================================
