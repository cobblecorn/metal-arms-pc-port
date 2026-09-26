//////////////////////////////////////////////////////////////////////////////////////
// fgcexception.cpp - exception handler
//
// Author: Chris MacDonald
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 01/22/03 MacDonald   Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fexception.h"

BOOL fexception_ModuleStartup( void )
{
	return TRUE;
}

void fexception_ModuleShutdown( void )
{
}


BOOL fexception_EnableHandler( void )
{
	return TRUE;
}

BOOL fexception_DisableHandler( void )
{
	return TRUE;
}

BOOL fexception_EnableFloatingPointExceptions( void )
{
	return TRUE;
}

BOOL fexception_DisableFloatingPointExceptions( void )
{
	return TRUE;
}

