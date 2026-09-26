//////////////////////////////////////////////////////////////////////////////////////
// fdev.cpp - Fang development services.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 03/08/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fdev.h"


static BOOL _bModuleInitialized;



BOOL fdev_ModuleStartup( void ) {
	FASSERT( !_bModuleInitialized );
	_bModuleInitialized = TRUE;
	return TRUE;
}

void fdev_ModuleShutdown( void ) {
	FASSERT( _bModuleInitialized );
	_bModuleInitialized = FALSE;
}

