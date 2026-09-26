//////////////////////////////////////////////////////////////////////////////////////
// ErrorLog.cpp - "manages writting data to an error log file (text mode)
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
// 01/23/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "ErrorLog.h"
#include <string.h>
#include <time.h>
#include <stdio.h>

static CErrorLog _ErrorLog;
static s32 _nRefCount = 0;
static char _szErrorLogFilename[256];

CErrorLog::CErrorLog() {
	_nRefCount++;
	FASSERT( _nRefCount == 1 );
	memset( _szErrorLogFilename, 0, 256 );
}

CErrorLog::~CErrorLog() {
	_nRefCount--;
	FASSERT( _nRefCount == 0 );
}

// set the name of error log file, this needs to be done, before any calls to GetCurrent(),
// and should happen only once
void CErrorLog::SetFilename( cchar *pszFullFilename ) {
	strncpy( _szErrorLogFilename, pszFullFilename, 256 );
}

CErrorLog& CErrorLog::GetCurrent() {
			
	return _ErrorLog;
}

cchar *CErrorLog::GetErrorFilename() {
	
	if( !_szErrorLogFilename[0] ) {
		return NULL;
	}

	return _szErrorLogFilename;
}

// writes an error line to the error log that looks like:
// pszTitle 
// Occurred - Sun May 01 20:27:01 1994
void CErrorLog::WriteErrorHeader( cchar *pszTitle ) {

	if( !_szErrorLogFilename[0] ) {
		return;
	}
	// get the time
	struct tm *newtime;
	time_t aclock;
	time( &aclock );                 // Get time in seconds
	newtime = localtime( &aclock );  // Convert time to struct tm form
	// open the file
	FILE *pFileStream = fopen( _szErrorLogFilename, "a+t" );
	if( !pFileStream ) {
		return;
	}
	fprintf( pFileStream, "\n%s\nOccurred - %s", pszTitle, asctime( newtime ) );
	fclose( pFileStream );
}

void CErrorLog::WriteErrorLine( cchar *pszLine ) {

	if( !_szErrorLogFilename[0] ) {
		return;
	}
	// open the file
	FILE *pFileStream = fopen( _szErrorLogFilename, "a+t" );
	if( !pFileStream ) {
		return;
	}
	fprintf( pFileStream, "%s\n", pszLine );
	fclose( pFileStream );
}

void CErrorLog::ClearErrorLog() {

	if( !_szErrorLogFilename[0] ) {
		return;
	}
	// open the file
	FILE *pFileStream = fopen( _szErrorLogFilename, "wt" );
	if( !pFileStream ) {
		return;
	}
	fprintf( pFileStream, " " );
	fclose( pFileStream );
}
