//////////////////////////////////////////////////////////////////////////////////////
// fang.h - Master Fang header file. Must be included by all Fang modules.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 08/29/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FEXCEPTION_H_
#define _FEXCEPTION_H_ 1

#include "fang.h"

BOOL fexception_ModuleStartup( void );
void fexception_ModuleShutdown( void );

// Enables the on-screen exception reporting functionality. Floating point exception
// handling is enabled.
// The function returns TRUE if handler was installed successfully, FALSE otherwise.
BOOL fexception_EnableHandler( void );


// Disables the on-screen exception reporting functionality. Floating point exception
// handling is disabled.
// The function returns TRUE if handler was installed successfully, FALSE otherwise.
BOOL fexception_DisableHandler( void );

// Allows the exception handler to respond to floating point exceptions. (On by default.)
BOOL fexception_EnableFloatingPointExceptions( void );

// Causes the exception handler to ignore floating point exceptions.
BOOL fexception_DisableFloatingPointExceptions( void );

#endif	// _FEXCEPTION_H_

