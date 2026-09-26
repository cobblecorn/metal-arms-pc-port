//////////////////////////////////////////////////////////////////////////////////////
// fGCcpu.cpp - Fang CPU module (Gamecube version).
//
// Author: John Lafleur 
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
// 02/18/02	Lafleur		Created from stubbed DX version.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fcpu.h"


static BOOL _bModuleInitialized;


//
//
//
BOOL fcpu_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

	_bModuleInitialized = TRUE;

	return TRUE;
}


//
// Uninstalls the CPU driver.
//
void fcpu_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

	_bModuleInitialized = FALSE;
}


