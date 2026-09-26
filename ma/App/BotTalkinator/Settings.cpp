//////////////////////////////////////////////////////////////////////////////////////
// Settings.cpp - "manages reading/writting/store data from the .ini file"
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS.
// Copyright (c) 1999
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 12/14/99 Starich     Created.
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
	WriteCustomString(	"CONFIG", "OUTPUT DIR",			(cchar *)m_sOutputDir );
	WriteCustomString(	"CONFIG", "MASTER FILE",		(cchar *)m_sMasterFile );
	WriteCustomString(	"CONFIG", "INPUT DIR",			(cchar *)m_sInputDir );
	WriteCustomString(	"CONFIG", "LAST BOT",			(cchar *)m_sLastSelectedBot );
	WriteCustomString(	"CONFIG", "LAST TITLE",			(cchar *)m_sLastSelectedTitle );

	// save our VIDEO settings
	WriteCustomString(	"VIDEO", "DEV NAME",			(cchar *)m_sDevName );
	WriteCustomInt(		"VIDEO", "DEV FLAGS",			m_nDevFlags );
	WriteCustomInt(		"VIDEO", "DEV ORDINAL",			m_nDevOrdinal );
	WriteCustomInt(		"VIDEO", "DEV RENDERER",		m_nDevRenderer );
	WriteCustomInt(		"VIDEO", "MODE FLAGS",			m_nModeFlags );
	WriteCustomInt(		"VIDEO", "MODE COLOR BITS",		m_nModeColorBits );
	WriteCustomInt(		"VIDEO", "MODE DEPTH BITS",		m_nModeDepthBits );
	WriteCustomInt(		"VIDEO", "MODE STENCIL BITS",	m_nModeStencilBits );
	WriteCustomInt(		"VIDEO", "MODE PIXELS ACROSS",	m_nModePixelsAcross );
	WriteCustomInt(		"VIDEO", "MODE PIXELS DOWN",	m_nModePixelsDown );
	WriteCustomInt(		"VIDEO", "MODE SWAP INTERVAL",	m_nSwapInterval );
	WriteCustomFloat(	"VIDEO", "UNIT FSAA",			m_fUnitFSAA );
}

void CSettings::GetCommonDataFromFile() {
	char szTempString[MAX_PATH];
	
	if( m_bReadCommonDataFromFile ) {
		// read our CONFIG settings
		ReadCustomString( "CONFIG", "MASTER FILE", "", szTempString, MAX_PATH );
		m_sMasterFile = szTempString;
		ReadCustomString( "CONFIG", "OUTPUT DIR", "", szTempString, MAX_PATH );
		m_sOutputDir = szTempString;
		ReadCustomString( "CONFIG", "INPUT DIR", "", szTempString, MAX_PATH );
		m_sInputDir = szTempString;
		ReadCustomString( "CONFIG", "LAST BOT", "", szTempString, MAX_PATH );
		m_sLastSelectedBot = szTempString;
		ReadCustomString( "CONFIG", "LAST TITLE", "", szTempString, MAX_PATH );
		m_sLastSelectedTitle = szTempString;

		// read our VIDEO settings
		ReadCustomString( "VIDEO", "DEV NAME", "", szTempString, MAX_PATH );
		m_sDevName = szTempString;
		m_nDevFlags = ReadCustomInt( "VIDEO", "DEV FLAGS", 0 );
		m_nDevOrdinal = ReadCustomInt( "VIDEO", "DEV ORDINAL", 0 );
		m_nDevRenderer = ReadCustomInt( "VIDEO", "DEV RENDERER", 0 );
		m_nModeFlags = ReadCustomInt( "VIDEO", "MODE FLAGS", 0 );
		m_nModeColorBits = ReadCustomInt( "VIDEO", "MODE COLOR BITS", 0 );
		m_nModeDepthBits = ReadCustomInt( "VIDEO", "MODE DEPTH BITS", 0 );
		m_nModeStencilBits = ReadCustomInt( "VIDEO", "MODE STENCIL BITS", 0 );
		m_nModePixelsAcross = ReadCustomInt( "VIDEO", "MODE PIXELS ACROSS", 0 );
		m_nModePixelsDown = ReadCustomInt( "VIDEO", "MODE PIXELS DOWN", 0 );
		m_nSwapInterval = ReadCustomInt( "VIDEO", "MODE SWAP INTERVAL", 0 );
		m_fUnitFSAA = ReadCustomFloat( "VIDEO", "UNIT FSAA", 0 );

		// mark that we don't need to read the data from the file anymore
		m_bReadCommonDataFromFile = FALSE;
	}
}
