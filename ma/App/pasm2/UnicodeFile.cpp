//////////////////////////////////////////////////////////////////////////////////////
// UnicodeFile.cpp - VERY SIMPLE class that reads and writes Unicode Files 
//
// Author: Russell A. Foushee   
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
// 02/07/03 Foushee     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UnicodeFile.h"

CUnicodeFile::CUnicodeFile() {
	m_pFile = NULL;
}

CUnicodeFile::~CUnicodeFile() {
	Close();
}

BOOL CUnicodeFile::Open( LPCTSTR pszFilename, UFOpenMode dwMode ) {

	CString sOpenMode;
	if( dwMode == UNICODEFILE_OPENMODE_READ ) {
		sOpenMode = _T( "rb" );
	} else if (dwMode == UNICODEFILE_OPENMODE_WRITE ) {
		sOpenMode = _T( "wb" );
	} else {
		m_sError = _T( "Invalid Open Mode" );
		return FALSE;
	}

	m_pFile = _tfopen( pszFilename, sOpenMode );
    if( !m_pFile ) {
		m_sError.Format( _T( "Could not open Unicode File %s" ), pszFilename );
		return FALSE;
	}

	//now, check the mode, and if it's write mode, write the cookie
	//if it's a read mode, then make sure this is a unicode file.
	WORD wCookie = 0xFEFF;
	if( dwMode == UNICODEFILE_OPENMODE_WRITE ) {
		//write the cookie!
		if( !fwrite( &wCookie, sizeof( WORD ), 1, m_pFile ) ) {
			m_sError = _T( "Could not write the unicode header to the text file!" );
			Close();
			return FALSE;
		}
	}
	else { // we are in read mode... make sure we are opening a unicode file...
		if( !fread( &wCookie, sizeof( WORD ), 1, m_pFile ) ) {
			m_sError = _T( "Could not read the unicode header from the text file!" );
			Close();
			return FALSE;
		}

		//now, check to see if this is a unicode file...
		if( wCookie != 0xFEFF ) {
			m_sError = _T( "Not a unicode file!" );
			Close();
			return FALSE;
		}
	}
	return TRUE;
}



void CUnicodeFile::Close( void ) {
	if( m_pFile ) {
		fclose( m_pFile );
		m_pFile = NULL;
	}
}


//this version removes the carriage return/linefeed combo and
//just places a linefeed in instead;
BOOL CUnicodeFile::ReadString( LPWSTR pString, DWORD dwMaxChars ) {

	if( !m_pFile ){
		m_sError = _T( "No open file for reading!" );
		return FALSE;
	}

	//the trick for this routine is to parse out the carriage return and linefeed combination when it gets read in
	//and replace it with a single newline character...
	if( !fgetws( pString, dwMaxChars, m_pFile ) ) {
		m_sError = _T( "EOF" );
		return FALSE;
	}
	
	wchar_t *pPointer = wcsstr( pString, L"\r\n" );
	if( pPointer ) {
		wcscat( pPointer, L"\n" );
	}
	
	return TRUE;
}


//this version removes the carriage return/linefeed combo
//entirely
BOOL CUnicodeFile::ReadString( CStringW &rString ) {

	wchar_t aWChar[ 1024 ];

	if( !m_pFile ){
		m_sError = _T( "No open file for reading!" );
		return FALSE;
	}

	//the trick for this routine is to parse out the carriage return and linefeed combination when it gets read in
	//and replace it with a nullchar...
	if( !fgetws( aWChar, 1024, m_pFile ) ) {
		m_sError = _T( "EOF" );
		return FALSE;
	}
	
	wchar_t *pPointer = wcsstr( aWChar, L"\r\n" );
	if( pPointer ) {
		*pPointer = NULL;
	}
	rString = aWChar;

	return TRUE;
}



BOOL CUnicodeFile::WriteString( LPCWSTR pString ) {

	//this routine needs to write the unicode string back to the file, finding all
	//newlines and appending a carriage return...

	if( !m_pFile ){
		m_sError = _T( "No open file for writing" );
		return FALSE;
	}

	wchar_t *pSource = ( wchar_t *) pString;

	while( *pSource ) {
		if( *pSource == L'\n' ) {
			if( !fwrite( L"\r\n", sizeof ( wchar_t ), 2, m_pFile ) ) {
				m_sError = _T( "Error writing to file" );
				return FALSE;
			}
		} else {
			if( !fwrite( pSource, sizeof ( wchar_t ), 1, m_pFile ) ) {
				m_sError = _T( "Error writing to file" );
				return FALSE;
			}
		}
		pSource++;
	}
	return TRUE;
}



LPCTSTR CUnicodeFile::GetErrorString( void ) {
	return m_sError;
}