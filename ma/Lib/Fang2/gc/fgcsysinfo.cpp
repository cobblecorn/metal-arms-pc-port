//////////////////////////////////////////////////////////////////////////////////////
// fgcsysinfo.cpp - 
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
#include "fgc.h"
#include "fsysinfo.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Module.
static BOOL _bModuleStarted = FALSE;

static u32 _uCapabilities;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL fsysinfo_ModuleStartup( void )
{
	FASSERT_MSG( ! _bModuleStarted, "[ FSYSINFO ] Error: System already started !!!" );

	u32 uReturn;

	////
	//
	if( OSGetResetCode() )
	{
		_uCapabilities = FSYSINFO_BOOTMODE_HOT;
	}
	else
	{
		_uCapabilities = FSYSINFO_BOOTMODE_COLD;
	}
	//
	////

	////
	//
	uReturn = OSGetLanguage();

	if( OS_LANG_FRENCH == uReturn )
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_FRENCH;
	}
	else if( OS_LANG_GERMAN == uReturn )
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_GERMAN;
	}
	else if( OS_LANG_SPANISH == uReturn )
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_SPANISH;
	}
	else if( OS_LANG_ITALIAN == uReturn )
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_ITALIAN;
	}
	else if( OS_LANG_DUTCH == uReturn )
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_DUTCH;
	}
	else // OS_LANG_ENGLISH
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_ENGLISH;
	}
	//
	////

	////
	//
	if( OS_SOUND_MODE_MONO == OSGetSoundMode() )
	{
		_uCapabilities |= FSYSINFO_AUDIO_MONO;
	}
	else // OS_SOUND_MODE_STEREO
	{
		_uCapabilities |= FSYSINFO_AUDIO_STEREO;
	}
	//
	////

	////
	//
	uReturn = VIGetTvFormat();

	if( VI_NTSC == uReturn )
	{
		if( VIGetDTVStatus() )
		{
			_uCapabilities |= ( FSYSINFO_VIDEO_NTSC | FSYSINFO_VIDEO_480I | FSYSINFO_VIDEO_480P | FSYSINFO_VIDEO_43RATIO | FSYSINFO_VIDEO_ANAMORPHIC_SUPPORTED | FSYSINFO_VIDEO_LETTERBOX_SUPPORTED );
		}
		else
		{
			_uCapabilities |= ( FSYSINFO_VIDEO_NTSC | FSYSINFO_VIDEO_480I | FSYSINFO_VIDEO_43RATIO | FSYSINFO_VIDEO_ANAMORPHIC_SUPPORTED | FSYSINFO_VIDEO_LETTERBOX_SUPPORTED );
		}
	}
	else if( ( VI_EURGB60 == uReturn ) || ( VI_MPAL == uReturn ) )
	{
		if( VIGetDTVStatus() )
		{
			_uCapabilities |= ( FSYSINFO_VIDEO_PAL | FSYSINFO_VIDEO_PAL_60HZ | FSYSINFO_VIDEO_480I | FSYSINFO_VIDEO_480P | FSYSINFO_VIDEO_43RATIO | FSYSINFO_VIDEO_ANAMORPHIC_SUPPORTED | FSYSINFO_VIDEO_LETTERBOX_SUPPORTED );
		}
		else
		{
			_uCapabilities |= ( FSYSINFO_VIDEO_PAL | FSYSINFO_VIDEO_PAL_60HZ | FSYSINFO_VIDEO_480I | FSYSINFO_VIDEO_43RATIO | FSYSINFO_VIDEO_ANAMORPHIC_SUPPORTED | FSYSINFO_VIDEO_LETTERBOX_SUPPORTED );
		}
	}
	else // VI_PAL
	{
		if( VIGetDTVStatus() )
		{
			_uCapabilities |= ( FSYSINFO_VIDEO_PAL | FSYSINFO_VIDEO_PAL_50HZ | FSYSINFO_VIDEO_480I | FSYSINFO_VIDEO_480P | FSYSINFO_VIDEO_43RATIO | FSYSINFO_VIDEO_ANAMORPHIC_SUPPORTED | FSYSINFO_VIDEO_LETTERBOX_SUPPORTED );
		}
		else
		{
			_uCapabilities |= ( FSYSINFO_VIDEO_PAL | FSYSINFO_VIDEO_PAL_50HZ | FSYSINFO_VIDEO_480I | FSYSINFO_VIDEO_43RATIO | FSYSINFO_VIDEO_ANAMORPHIC_SUPPORTED | FSYSINFO_VIDEO_LETTERBOX_SUPPORTED );
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

	OSCalendarTime oTime;

	OSTicksToCalendarTime( OSGetTime(), &oTime );

	return (FSysinfo_DateTime_t)( FSYSINFO_YEARFROM2000_TO_DATETIME( oTime.year ) | FSYSINFO_MONTH_TO_DATETIME( oTime.mon + 1 ) | FSYSINFO_DAYOFMONTH_TO_DATETIME( oTime.mday ) | FSYSINFO_HOUR_TO_DATETIME( oTime.hour ) | FSYSINFO_MIN_TO_DATETIME( oTime.min ) | FSYSINFO_SEC_TO_DATETIME( oTime.sec ) );

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
