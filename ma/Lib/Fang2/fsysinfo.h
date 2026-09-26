//////////////////////////////////////////////////////////////////////////////////////
// fsysinfo.h - Fang system information.
//
// Author: Albert Yale
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
// 03/06/02 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef _FSYSINFO_H_
#define _FSYSINFO_H_ 1

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "fang.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef u32 FSysinfo_DateTime_t;

typedef enum
{
	FSYSINFO_NONE						=	0x00000000, // ( 0 << 00 )

	////
	//
	FSYSINFO_BOOTMODE_COLD				=	0x00000001, // ( 1 << 00 )
	FSYSINFO_BOOTMODE_HOT				=	0x00000002, // ( 1 << 01 )
	//
	////

	////
	//
	FSYSINFO_AUDIO_MONO					=	0x00000004, // ( 1 << 02 )
	FSYSINFO_AUDIO_STEREO				=	0x00000008, // ( 1 << 03 )
	FSYSINFO_AUDIO_SURROUND				=	0x00000010, // ( 1 << 04 )
	//
	////

	////
	//
	FSYSINFO_VIDEO_NTSC					=	0x00000020, // ( 1 << 05 )
	FSYSINFO_VIDEO_PAL					=	0x00000040, // ( 1 << 06 )

	FSYSINFO_VIDEO_PAL_50HZ				=	0x00000080, // ( 1 << 07 )
	FSYSINFO_VIDEO_PAL_60HZ				=	0x00000100, // ( 1 << 08 )

	FSYSINFO_VIDEO_480I					=	0x00000200, // ( 1 << 09 )
	FSYSINFO_VIDEO_480P					=	0x00000400, // ( 1 << 10 )
	FSYSINFO_VIDEO_720P					=	0x00000800, // ( 1 << 11 )
	FSYSINFO_VIDEO_1080I				=	0x00001000, // ( 1 << 12 )

	FSYSINFO_VIDEO_43RATIO				=	0x00002000, // ( 1 << 13 )

	FSYSINFO_VIDEO_ANAMORPHIC_SUPPORTED	=	0x00004000, // ( 1 << 14 ) // Physicaly supported by the hardware.
	FSYSINFO_VIDEO_ANAMORPHIC_SELECTED	=	0x00008000, // ( 1 << 15 ) // Selected by the user.
	FSYSINFO_VIDEO_LETTERBOX_SUPPORTED	=	0x00010000, // ( 1 << 16 ) // Physicaly supported by the hardware.
	FSYSINFO_VIDEO_LETTERBOX_SELECTED	=	0x00020000, // ( 1 << 17 ) // Selected by the user.
	//
	////

	////
	//
	FSYSINFO_LANGUAGE_ENGLISH			=	0x00040000, // ( 1 << 18 )
	FSYSINFO_LANGUAGE_FRENCH			=	0x00080000, // ( 1 << 19 )
	FSYSINFO_LANGUAGE_GERMAN			=	0x00100000, // ( 1 << 20 )
	FSYSINFO_LANGUAGE_SPANISH			=	0x00200000, // ( 1 << 21 )
	FSYSINFO_LANGUAGE_ITALIAN			=	0x00400000, // ( 1 << 22 )
	FSYSINFO_LANGUAGE_DUTCH				=	0x00800000, // ( 1 << 23 )
	FSYSINFO_LANGUAGE_JAPANESE			=	0x01000000, // ( 1 << 24 )
	FSYSINFO_LANGUAGE_KOREAN			=	0x02000000, // ( 1 << 25 )
	FSYSINFO_LANGUAGE_CHINESE			=	0x04000000, // ( 1 << 26 )
	FSYSINFO_LANGUAGE_PORTUGUESE		=	0x08000000  // ( 1 << 27 )
	//
	////

} FSysinfo_Descriptor_e;

typedef enum
{
	////
	//
	FSYSINFO_DAYOFWEEK_SUNDAY			=	0x00000000,
	FSYSINFO_DAYOFWEEK_MONDAY			=	0x00000001,
	FSYSINFO_DAYOFWEEK_TUESDAY			=	0x00000002,
	FSYSINFO_DAYOFWEEK_WEDNESDAY		=	0x00000003,
	FSYSINFO_DAYOFWEEK_THURSDAY			=	0x00000004,
	FSYSINFO_DAYOFWEEK_FRIDAY			=	0x00000005,
	FSYSINFO_DAYOFWEEK_SATURDAY			=	0x00000006
	//
	////

} FSysinfo_DayOfWeek_e;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//// Module.
//
extern BOOL fsysinfo_ModuleStartup( void );
extern void fsysinfo_ModuleShutdown( void );
//
////

////
//
extern u32 fsysinfo_GetCapabilities( void ); // See FSysinfo_Descriptor_e.
//
////

//// Date/Time.
//
extern FSysinfo_DateTime_t fsysinfo_CurrentDatetime( void );
extern FSysinfo_DayOfWeek_e fsysinfo_DatetimeToDayOfWeek( FSysinfo_DateTime_t oDatetime ); // See FSysinfo_DayOfWeek_e.
//
////

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Valid ranges:	Year:	2000-2063.
//					Month:	1-12.
//					Day:	1-31.
//					Hour:	0-23.
//					Min:	0-59.
//					Sec:	0-59.

#define FSYSINFO_DATETIME_TO_YEARFROM2000( dt )		( ( ( dt ) >> 26 ) & 0x3f )
#define FSYSINFO_DATETIME_TO_MONTH( dt )			( ( ( dt ) >> 22 ) & 0x0f )
#define FSYSINFO_DATETIME_TO_DAYOFMONTH( dt )		( ( ( dt ) >> 17 ) & 0x1f )
#define FSYSINFO_DATETIME_TO_HOUR( dt )				( ( ( dt ) >> 12 ) & 0x1f )
#define FSYSINFO_DATETIME_TO_MIN( dt )				( ( ( dt ) >> 06 ) & 0x3f )
#define FSYSINFO_DATETIME_TO_SEC( dt )				( ( ( dt ) >> 00 ) & 0x3f )

#define FSYSINFO_YEARFROM2000_TO_DATETIME( y )		( ( ( y ) & 0x3f ) << 26 )
#define FSYSINFO_MONTH_TO_DATETIME( m )				( ( ( m ) & 0x0f ) << 22 )
#define FSYSINFO_DAYOFMONTH_TO_DATETIME( d )		( ( ( d ) & 0x1f ) << 17 )
#define FSYSINFO_HOUR_TO_DATETIME( h )				( ( ( h ) & 0x1f ) << 12 )
#define FSYSINFO_MIN_TO_DATETIME( m )				( ( ( m ) & 0x3f ) << 06 )
#define FSYSINFO_SEC_TO_DATETIME( s )				( ( ( s ) & 0x3f ) << 00 )

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif // _FSYSINFO_H_

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
