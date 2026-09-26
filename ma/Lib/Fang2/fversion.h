//////////////////////////////////////////////////////////////////////////////////////
// fversion.h - stores version numbers for all aspects of the fang engine & tools
//
// Author: Michael Starich   
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
// 03/15/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FVERSION_H_
#define _FVERSION_H_ 1 

#include "fang.h"

// The platform represents who can read the file
enum {
	FVERSION_PLATFORM_FLAG_XBOX		= 0x01,
	FVERSION_PLATFORM_FLAG_PC		= 0x04,
	FVERSION_PLATFORM_FLAG_TOOLS	= 0x08,
	FVERSION_PLATFORM_FLAG_GC		= 0x10,
	FVERSION_PLATFORM_FLAG_PS2		= 0x20,	//ARG
};

#define FVERSION_FILE_SIGNATURE		( 'F' | 'A'<<8 | 'N'<<16 | 'G'<<24 )

typedef struct {
	u32 nSignature;// should always == FVERSION_FILE_SIGNATURE
	u32 nVersion; // use macros to get platform, major, minor, & sub version number

	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa
	void ChangeEndian() { 
		nSignature = fang_ConvertEndian( nSignature );
		nVersion = fang_ConvertEndian( nVersion );
	}
} FVersionHeader_t;

// macros to extract elements from a version u32
#define FVERSION_GET_PLATFORM_MASK( nVersion )	( ((u32)nVersion >> 24) & 0xFF )
#define FVERSION_GET_MAJOR_NUMBER( nVersion )	( ((u32)nVersion >> 16) & 0xFF )
#define FVERSION_GET_MINOR_NUMBER( nVersion )	( ((u32)nVersion >> 8) & 0xFF )
#define FVERSION_GET_SUB_NUMBER( nVersion )		( (u32)nVersion & 0xFF )

// macros to create a u32 version from bytes
#define FVERSION_CREATE_PLATFORM( nPlatform )	( (u32)( ((u32)nPlatform & 0xFF) << 24 ) )
#define FVERSION_CREATE_MAJOR( nMajor )			( (u32)( ((u32)nMajor & 0xFF) << 16 ) )
#define FVERSION_CREATE_MINOR( nMinor )			( (u32)( ((u32)nMinor & 0xFF) << 8 ) )
#define FVERSION_CREATE_SUB( nSub )				( (u32)( ((u32)nSub & 0xFF) ) )
#define FVERSION_CREATE_VERSION( nPlatform, nMajor, nMinor, nSub )	( (u32)( ((u32)FVERSION_CREATE_PLATFORM( nPlatform )) | \
																			 ((u32)FVERSION_CREATE_MAJOR( nMajor )) | \
																			 ((u32)FVERSION_CREATE_MINOR( nMinor )) | \
																			 ((u32)FVERSION_CREATE_SUB( nSub )) ) )

// used by the exporters, and the apefile
#define FVERSION_APE_PLATFORM	( FVERSION_PLATFORM_FLAG_TOOLS )
#define FVERSION_APE_MAJOR		0x01 // 1 Byte
#define FVERSION_APE_MINOR		0x05 // 1 Byte
#define FVERSION_APE_SUB		0x00 // 1 Byte
#define FVERSION_APE_VERSION	( FVERSION_CREATE_VERSION( FVERSION_APE_PLATFORM, FVERSION_APE_MAJOR, FVERSION_APE_MINOR, FVERSION_APE_SUB ) )

// switched to functions to avoid massive recompiles, look in fversion.cpp to update the version number
extern u8 fversion_GetToolPlatform( void );
extern u8 fversion_GetToolMajorVer( void );
extern u8 fversion_GetToolMinorVer( void );
extern u8 fversion_GetToolSubVer( void );
extern u32 fversion_GetToolVersion( void );

#endif

