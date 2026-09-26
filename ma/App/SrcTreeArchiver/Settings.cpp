//////////////////////////////////////////////////////////////////////////////////////
// Settings.cpp - "manages reading/writting/store data from the .ini file"
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
// 12/14/00 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Settings.h"

static CSettings _Settings;
static int _nRefCount = 0;
static CString _sSettingsFilename;
static CString _sAppName;

CSettings::CSettings() {
	m_bReadCommonDataFromFile = TRUE;
	_nRefCount++;
	ASSERT( _nRefCount == 1 );
	//_sSettingsFilename = "";
}

CSettings::~CSettings() {
	_nRefCount--;
	ASSERT( _nRefCount == 0 );
}

// set the name of settings file, this needs to be done, before any calls to GetCurrent(),
// and should happen only once
void CSettings::SetSettingsFilename( cchar *pszSettingsFilenameOnly ) {
	_sSettingsFilename = pszSettingsFilenameOnly;
}

void CSettings::SetApplicationName( cchar *pszAppName ) {
	_sAppName = pszAppName;
}

cchar *CSettings::GetApplicationName() {
	return (cchar *)_sAppName;
}

CSettings& CSettings::GetCurrent() {
	// make sure that a settings file has been specified
	ASSERT( _sSettingsFilename != "" );

	_Settings.GetCommonDataFromFile();
	
	return _Settings;
}

void CSettings::WriteCustomInt( cchar *pszSectionName, cchar *pszItemName, u32 nValue ) {
	CString s;

	s.Format( "%d", nValue );
	WriteCustomString( pszSectionName, pszItemName, (cchar *)s );
}

void CSettings::WriteCustomString( cchar *pszSectionName, cchar *pszItemName, cchar *pszString ) {
	WritePrivateProfileString( pszSectionName, pszItemName, pszString, (cchar *)_sSettingsFilename );
}

void CSettings::WriteCustomFloat( cchar *pszSectionName, cchar *pszItemName, f32 fValue ) {
	CString s;

	s.Format( "%f", fValue );
	WriteCustomString( pszSectionName, pszItemName, (cchar *)s );
}

u32 CSettings::ReadCustomInt( cchar *pszSectionName, cchar *pszItemName, u32 nDefault ) {
	u32 nReturnVal = GetPrivateProfileInt( pszSectionName, pszItemName, nDefault, (cchar *)_sSettingsFilename );
	return nReturnVal;
}

void CSettings::ReadCustomString( cchar *pszSectionName, 
								  cchar *pszItemName, 
								  cchar *pszDefault,
								  char *pszDest, 
								  u32 nSizeOfDest ) {
	GetPrivateProfileString( pszSectionName, pszItemName, pszDefault, pszDest, nSizeOfDest, (cchar *)_sSettingsFilename );
}

f32 CSettings::ReadCustomFloat( cchar *pszSectionName, cchar *pszItemName, f32 fDefault ) {
	char szTempString[32];
	CString s;
	f32 fReturnVal;

	s.Format( "%f", fDefault );
	GetPrivateProfileString( pszSectionName, pszItemName, (cchar *)s, szTempString, 32, (cchar *)_sSettingsFilename );
	sscanf( szTempString, "%f", &fReturnVal );
	return fReturnVal;
}

void CSettings::SaveCommonDataOutToFile() {
	
	// save our CONFIG settings
	WriteCustomInt( "CONFIG", "PROFILE", m_nProfileIndex );
	
	u32 i;
	CString sItemName;
	for( i=0; i < SETTINGS_MAX_PROFILES; i++ ) {
		sItemName.Format( "SRC DIR %d", i );
		WriteCustomString( "CONFIG", sItemName, (cchar *)m_asSrcPath[i] );

		sItemName.Format( "DEST FILENAME %d", i );
		WriteCustomString( "CONFIG", sItemName, (cchar *)m_asDestFilename[i] );
		
		sItemName.Format( "IGNORE EXT LIST %d", i );
		WriteCustomString( "CONFIG", sItemName, (cchar *)m_asIgnoreExtList[i] );
		
		sItemName.Format( "RECURSE SUB DIRS %d", i );
		WriteCustomInt( "CONFIG", sItemName, m_abRecurseSubDirs[i] );

		sItemName.Format( "DATE %d", i );
		WriteCustomInt( "CONFIG", sItemName, (u32)m_aDate[i].GetTime() );

		sItemName.Format( "DISK SPAN INDEX %d",	i );
		WriteCustomInt( "CONFIG", sItemName, m_anDiskSpanIndex[i] );
	}
	WriteCustomInt( "CONFIG", "CLICK_N_GO", m_bClickNGo );	
}

void CSettings::GetCommonDataFromFile() {
	char szTempString[_MAX_PATH];
	
	if( m_bReadCommonDataFromFile ) {
		// read our CONFIG settings
		m_nProfileIndex = ReadCustomInt( "CONFIG", "PROFILE", 0 );
		
		u32 i;
		CString sItemName;
		for( i=0; i < SETTINGS_MAX_PROFILES; i++ ) {
			sItemName.Format( "SRC DIR %d", i );
			ReadCustomString( "CONFIG", sItemName, "", szTempString, _MAX_PATH );
			m_asSrcPath[i] = szTempString;

			sItemName.Format( "DEST FILENAME %d", i );
			ReadCustomString( "CONFIG", sItemName, "", szTempString, _MAX_PATH );
			m_asDestFilename[i] = szTempString;

			sItemName.Format( "IGNORE EXT LIST %d", i );
			ReadCustomString( "CONFIG", sItemName, "", szTempString, _MAX_PATH );
			m_asIgnoreExtList[i] = szTempString;

			sItemName.Format( "RECURSE SUB DIRS %d", i );
			m_abRecurseSubDirs[i] = ReadCustomInt( "CONFIG", sItemName, TRUE );

			sItemName.Format( "DATE %d", i );
			m_aDate[i] = ReadCustomInt( "CONFIG", sItemName, 0 );

			sItemName.Format( "DISK SPAN INDEX %d",	i );
			m_anDiskSpanIndex[i] = ReadCustomInt( "CONFIG", sItemName, 0 );
		}
		m_bClickNGo = ReadCustomInt( "CONFIG", "CLICK_N_GO", FALSE );	
		
		// mark that we don't need to read the data from the file anymore
		m_bReadCommonDataFromFile = FALSE;
	}
}