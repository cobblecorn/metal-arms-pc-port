//////////////////////////////////////////////////////////////////////////////////////
// fdx8sysinfo.cpp - 
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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Module.
static BOOL _bModuleStarted = FALSE;

static u32 _uCapabilities;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL fsysinfo_ModuleStartup( void )
{
	FASSERT_MSG( ! _bModuleStarted, "[ FSYSINFO ] Error: System already started !!!" );

	_uCapabilities = FSYSINFO_BOOTMODE_COLD;

	////
	//
	u16 uLanguage = (u16)GetKeyboardLayout( NULL );

	if( 0x040c == uLanguage )
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_FRENCH;
	}
	else if( 0x0407 == uLanguage )
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_GERMAN;
	}
	else if( 0x0c0a == uLanguage )
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_SPANISH;
	}
	else if( 0x0410 == uLanguage )
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_ITALIAN;
	}
	else if( 0x0413 == uLanguage )
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_DUTCH;
	}
	else if( 0x0411 == uLanguage )
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_JAPANESE;
	}
	else if( 0x0412 == uLanguage )
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_KOREAN;
	}
	else if( 0x0804 == uLanguage )
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_CHINESE;
	}
	else if( 0x0816 == uLanguage )
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_PORTUGUESE;
	}
	else // 0x0409
	{
		_uCapabilities |= FSYSINFO_LANGUAGE_ENGLISH;
	}
	//
	////

	////
	//
	_uCapabilities |= FSYSINFO_AUDIO_STEREO;
	//
	////

	////
	//
	_uCapabilities |= ( FSYSINFO_VIDEO_NTSC | FSYSINFO_VIDEO_480I | FSYSINFO_VIDEO_43RATIO );
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
