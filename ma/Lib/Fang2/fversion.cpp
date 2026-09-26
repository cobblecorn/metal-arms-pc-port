//////////////////////////////////////////////////////////////////////////////////////
// fversion.cpp - 
//
// Author: Michael Starich   
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
// 09/26/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "fversion.h"


// used by the pasm and the various tools,
#define FVERSION_PASM_PLATFORM	( FVERSION_PLATFORM_FLAG_TOOLS )
#define FVERSION_PASM_MAJOR		1	// 1 Byte
#define FVERSION_PASM_MINOR		19	// 1 Byte -> this now corresponds to the milestone number being worked on
#define FVERSION_PASM_SUB		1	// 1 Byte
#define FVERSION_PASM_VERSION	( FVERSION_CREATE_VERSION( FVERSION_PASM_PLATFORM, FVERSION_PASM_MAJOR, FVERSION_PASM_MINOR, FVERSION_PASM_SUB ) )

u8 fversion_GetToolPlatform( void ) {
	return FVERSION_PASM_PLATFORM;
}

u8 fversion_GetToolMajorVer( void ) {
	return FVERSION_PASM_MAJOR;
}

u8 fversion_GetToolMinorVer( void ) {
	return FVERSION_PASM_MINOR;
}

u8 fversion_GetToolSubVer( void ) {
	return FVERSION_PASM_SUB;
}

u32 fversion_GetToolVersion( void ) {
	return FVERSION_PASM_VERSION;
}

