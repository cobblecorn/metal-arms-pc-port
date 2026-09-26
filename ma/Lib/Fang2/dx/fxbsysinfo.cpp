//////////////////////////////////////////////////////////////////////////////////////
// fxbsysinfo.cpp - 
//
// Author: Albert Yale
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
// 03/06/02 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "fang.h"
#include "fdx8.h"
#include "fsysinfo.h"

#include <xtl.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Module.
static BOOL _bModuleStarted = FALSE;

static u32 _uCapabilities;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL fsysinfo_ModuleStartup( void )
{
	FASSERT_MSG( ! _bModuleStarted, "[ FSYSINFO ] Error: System already started !!!" );

	////
	//
	DWORD dwType;
	u32 uReturn;
	LAUNCH_DATA oLaunchData;
	fang_MemZero( &oLaunchData, sizeof( oLaunchData ) );
	_uCapabilities = FSYSINFO_NONE;

	uReturn = XGetLaunchInfo( &dwType, &oLaunchData );

	if( ERROR_SUCCESS == uReturn )
	{
		if( LDT_FROM_DASHBOARD == dwType )
		{
			if( ((LD_FROM_DASHBOARD *)( &oLaunchData ))->dwContext )
			{
				_uCapabilities = FSYSINFO_BOOTMODE_HOT;
			}
			else
			{
				_uCapabilities = FSYSINFO_BOOTMODE_COLD;
			}
		}
		else if( LDT_TITLE == dwType )
		{
			_uCapabilities = FSYSINFO_BOOTMODE_HOT;
		}
		else if( LDT_FROM_DEBUGGER_CMDLINE == dwType )
		{
			_uCapabilities = FSYSINFO_BOOTMODE_COLD;
		}
	}
	else if( ERROR_NOT_FOUND == uReturn )
	{
		_uCapabilities = FSYSINFO_BOOTMODE_COLD;
	}
	//
	////

	////
	//
	uReturn = XGetLanguage();

	if( XC_LANGUAGE_FRENCH == uReturn )
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_FRENCH;
	}
	else if( XC_LANGUAGE_GERMAN == uReturn )
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_GERMAN;
	}
	else if( XC_LANGUAGE_SPANISH == uReturn )
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_SPANISH;
	}
	else if( XC_LANGUAGE_ITALIAN == uReturn )
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_ITALIAN;
	}
	else if( XC_LANGUAGE_JAPANESE == uReturn )
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_JAPANESE;
	}
	else if( XC_LANGUAGE_KOREAN == uReturn )
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_KOREAN;
	}
	else if( XC_LANGUAGE_TCHINESE == uReturn )
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_CHINESE;
	}
	else if( XC_LANGUAGE_PORTUGUESE == uReturn )
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_PORTUGUESE;
	}
	else // XC_LANGUAGE_ENGLISH
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_ENGLISH;
	}
	//
	////

	////
	//
	uReturn = XGetAudioFlags();

	if( XC_AUDIO_FLAGS_SURROUND == XC_AUDIO_FLAGS_BASIC( uReturn ) )
	{
		_uCapabilities |= FSYSINFO_AUDIO_SURROUND;
	}
	else if( XC_AUDIO_FLAGS_STEREO == XC_AUDIO_FLAGS_BASIC( uReturn ) )
	{
		_uCapabilities |= FSYSINFO_AUDIO_STEREO;
	}
	else if( XC_AUDIO_FLAGS_MONO == XC_AUDIO_FLAGS_BASIC( uReturn ) )
	{
		_uCapabilities |= FSYSINFO_AUDIO_MONO;
	}
	//
	////

	////
	//
	if( XC_VIDEO_STANDARD_PAL_I == XGetVideoStandard() )
	{
		_uCapabilities |= ( FSYSINFO_VIDEO_PAL | FSYSINFO_VIDEO_480I | FSYSINFO_VIDEO_43RATIO | FSYSINFO_VIDEO_ANAMORPHIC_SUPPORTED | FSYSINFO_VIDEO_LETTERBOX_SUPPORTED );
	}
	else
	{
		_uCapabilities |= ( FSYSINFO_VIDEO_NTSC | FSYSINFO_VIDEO_480I | FSYSINFO_VIDEO_43RATIO | FSYSINFO_VIDEO_ANAMORPHIC_SUPPORTED | FSYSINFO_VIDEO_LETTERBOX_SUPPORTED );
	}

	uReturn = XGetVideoFlags();

	if( XC_VIDEO_FLAGS_WIDESCREEN & uReturn )
	{
		_uCapabilities |= FSYSINFO_VIDEO_ANAMORPHIC_SELECTED;
	}
	if( XC_VIDEO_FLAGS_LETTERBOX & uReturn )
	{
		_uCapabilities |= FSYSINFO_VIDEO_LETTERBOX_SELECTED;
	}
	if( XC_VIDEO_FLAGS_HDTV_480p & uReturn )
	{
		_uCapabilities |= FSYSINFO_VIDEO_480P;
	}
	if( XC_VIDEO_FLAGS_HDTV_720p & uReturn )
	{
		_uCapabilities |= FSYSINFO_VIDEO_720P;
	}
	if( XC_VIDEO_FLAGS_HDTV_1080i & uReturn )
	{
		_uCapabilities |= FSYSINFO_VIDEO_1080I;
	}
	if( _uCapabilities & FSYSINFO_VIDEO_PAL ) 
	{
		if ( XC_VIDEO_FLAGS_PAL_60Hz & uReturn )
		{
			_uCapabilities |= FSYSINFO_VIDEO_PAL_60HZ;
		}
		else
		{
			_uCapabilities |= FSYSINFO_VIDEO_PAL_50HZ;
		}
	}
	//
	////

	_bModuleStarted = TRUE;

	return TRUE;

} // fsysinfo_ModuleStartup

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void fsysinfo_ModuleShutdown( void )
{
	_bModuleStarted = FALSE;

} // fsysinfo_ModuleShutdown

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

u32 fsysinfo_GetCapabilities( void )
{
	FASSERT_MSG( _bModuleStarted, "[ FSYSINFO ] Error: System not started !!!" );

	return _uCapabilities;

} // fsysinfo_GetCapabilities

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FSysinfo_DateTime_t fsysinfo_CurrentDatetime( void )
{
	FASSERT_MSG( _bModuleStarted, "[ FSYSINFO ] Error: System not started !!!" );

	SYSTEMTIME oSystemTime;

	GetLocalTime( &oSystemTime );

	return (FSysinfo_DateTime_t)( FSYSINFO_YEARFROM2000_TO_DATETIME( oSystemTime.wYear - 2000 ) | FSYSINFO_MONTH_TO_DATETIME( oSystemTime.wMonth ) | FSYSINFO_DAYOFMONTH_TO_DATETIME( oSystemTime.wDay ) | FSYSINFO_HOUR_TO_DATETIME( oSystemTime.wHour ) | FSYSINFO_MIN_TO_DATETIME( oSystemTime.wMinute ) | FSYSINFO_SEC_TO_DATETIME( oSystemTime.wSecond ) );

} // fsysinfo_CurrentDatetime

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Specilized implementation of Zeller's Congruence. Not reusable in any other context.
FSysinfo_DayOfWeek_e fsysinfo_DatetimeToDayOfWeek( FSysinfo_DateTime_t oDatetime )
{
	FASSERT_MSG( _bModuleStarted, "[ FSYSINFO ] Error: System not started !!!" );

	u32 uCentury, uYear, uMonth, uDayOfMonth, uDayOfWeek;

	uYear		= FSYSINFO_DATETIME_TO_YEARFROM2000( oDatetime );
	uMonth		= FSYSINFO_DATETIME_TO_MONTH( oDatetime );
	uDayOfMonth	= FSYSINFO_DATETIME_TO_DAYOFMONTH( oDatetime );

	FASSERT_MSG( ( 64 > uYear ),                        "[ FSYSINFO ] Error: Invalid year !!!" );
	FASSERT_MSG( ( ( 13 > uMonth ) && ( 0 < uMonth ) ), "[ FSYSINFO ] Error: Invalid month !!!" );
	FASSERT_MSG( ( 0 < uDayOfMonth ),                   "[ FSYSINFO ] Error: Invalid day of month !!!" );

	uCentury = 20;

	if( uMonth < 3 )
	{
		uMonth += 12;
		if( uYear )
		{
			--uYear;
		}
		else
		{
			uYear = 99;
			uCentury = 19;
		}
	}

	uDayOfWeek = ( ( uDayOfMonth + ( ( ( uMonth + 1 ) * 26 ) / 10 ) + uYear + ( uYear >> 2 ) + ( uCentury >> 2 ) + ( uCentury * 5 ) ) % 7 );
	if( ! uDayOfWeek ) uDayOfWeek = 7;
	--uDayOfWeek; // Sunday == 0.

	return (FSysinfo_DayOfWeek_e)uDayOfWeek;

} // fsysinfo_DatetimeToDayOfWeek

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
