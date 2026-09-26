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
void CSettings::SetSettingsFilename( LPCTSTR pszSettingsFilenameOnly ) {
	_sSettingsFilename = pszSettingsFilenameOnly;
}

void CSettings::SetApplicationName( LPCTSTR pszAppName ) {
	_sAppName = pszAppName;
}

LPCTSTR CSettings::GetApplicationName() {
	return _sAppName;
}

CSettings& CSettings::GetCurrent() {
	// make sure that a settings file has been specified
	ASSERT( _sSettingsFilename != "" );

	_Settings.GetCommonDataFromFile();
	
	return _Settings;
}

void CSettings::WriteCustomInt( LPCTSTR pszSectionName, LPCTSTR pszItemName, u32 nValue ) {
	CString s;

	s.Format( _T( "%d" ), nValue );
	WriteCustomString( pszSectionName, pszItemName, s );
}

void CSettings::WriteCustomString( LPCTSTR pszSectionName, LPCTSTR pszItemName, LPCTSTR pszString ) {
	WritePrivateProfileString( pszSectionName, pszItemName, pszString, _sSettingsFilename );
}

void CSettings::WriteCustomFloat( LPCTSTR pszSectionName, LPCTSTR pszItemName, f32 fValue ) {
	CString s;

	s.Format( _T( "%f" ), fValue );
	WriteCustomString( pszSectionName, pszItemName, s );
}

u32 CSettings::ReadCustomInt( LPCTSTR pszSectionName, LPCTSTR pszItemName, u32 nDefault ) {
	u32 nReturnVal = GetPrivateProfileInt( pszSectionName, pszItemName, nDefault, _sSettingsFilename );
	return nReturnVal;
}

void CSettings::ReadCustomString( LPCTSTR pszSectionName, 
								  LPCTSTR pszItemName, 
								  LPCTSTR pszDefault,
								  TCHAR* pszDest, 
								  u32 nSizeOfDest ) {
	GetPrivateProfileString( pszSectionName, pszItemName, pszDefault, pszDest, nSizeOfDest, _sSettingsFilename );
}

f32 CSettings::ReadCustomFloat( LPCTSTR pszSectionName, LPCTSTR pszItemName, f32 fDefault ) {
	TCHAR szTempString[32];
	CString s;
	f32 fReturnVal;

	s.Format( _T( "%f" ), fDefault );
	GetPrivateProfileString( pszSectionName, pszItemName, s, szTempString, 32, _sSettingsFilename );
	_stscanf( szTempString, _T( "%f" ), &fReturnVal );
	return fReturnVal;
}

void CSettings::SaveCommonDataOutToFile() {
	// save our CONFIG settings
	WriteCustomString(	_T( "CONFIG" ), _T( "INPUT FILE" ),			m_sInputFile );
	WriteCustomString(	_T( "CONFIG" ), _T( "OUTPUT DIR" ),			m_sOutputDir );
	WriteCustomString(	_T( "CONFIG" ), _T( "CHAR SET" ),			m_sCharSet );
	WriteCustomString(	_T( "CONFIG" ), _T( "MASTER FILE" ),		m_sMasterFile );
	WriteCustomInt(		_T( "CONFIG" ), _T( "ONE TO ONE" ),			m_bOneToOne );
	WriteCustomInt(		_T( "CONFIG" ), _T( "DRAW GUIDES" ),		m_bDrawGuides );
	WriteCustomInt(		_T( "CONFIG" ), _T( "FIXED WIDTH FONT" ),	m_bFixedWidthFont );
	WriteCustomInt(		_T( "CONFIG" ), _T( "FIRST RUN" ),			m_bFirstRun );
	WriteCustomInt(		_T( "CONFIG" ), _T( "ITALIC FONT" ),		m_bItalicFont );
	WriteCustomFloat(	_T( "CONFIG" ), _T( "LETTER SPACING" ),		m_fLetterSpacing );
	WriteCustomFloat(	_T( "CONFIG" ), _T( "WORD SPACING" ),		m_fWordSpacing );
	WriteCustomFloat(	_T( "CONFIG" ), _T( "LINES SPACING" ),		m_fLineSpacing );
	WriteCustomFloat(	_T( "CONFIG" ), _T( "NUMBER OF LINES" ),	m_fNumberOfLines );
	WriteCustomFloat(	_T( "CONFIG" ), _T( "ITALIC SLANT" ),		m_fItalicSlant );
	// save our VIDEO settings
	WriteCustomString(	_T( "VIDEO" ), _T( "DEV NAME" ),			m_sDevName );
	WriteCustomInt(		_T( "VIDEO" ), _T( "DEV FLAGS" ),			m_nDevFlags );
	WriteCustomInt(		_T( "VIDEO" ), _T( "DEV ORDINAL" ),			m_nDevOrdinal );
	WriteCustomInt(		_T( "VIDEO" ), _T( "DEV RENDERER" ),		m_nDevRenderer );
	WriteCustomInt(		_T( "VIDEO" ), _T( "MODE FLAGS" ),			m_nModeFlags );
	WriteCustomInt(		_T( "VIDEO" ), _T( "MODE COLOR BITS" ),		m_nModeColorBits );
	WriteCustomInt(		_T( "VIDEO" ), _T( "MODE DEPTH BITS" ),		m_nModeDepthBits );
	WriteCustomInt(		_T( "VIDEO" ), _T( "MODE STENCIL BITS" ),	m_nModeStencilBits );
	WriteCustomInt(		_T( "VIDEO" ), _T( "MODE PIXELS ACROSS" ),	m_nModePixelsAcross );
	WriteCustomInt(		_T( "VIDEO" ), _T( "MODE PIXELS DOWN" ),	m_nModePixelsDown );
	WriteCustomInt(		_T( "VIDEO" ), _T( "MODE SWAP INTERVAL" ),	m_nSwapInterval );
	WriteCustomFloat(	_T( "VIDEO" ), _T( "UNIT FSAA" ),			m_fUnitFSAA );
}

void CSettings::GetCommonDataFromFile() {
	TCHAR szTempString[MAX_PATH];
	
	if( m_bReadCommonDataFromFile ) {
		// read our CONFIG settings
		m_fLetterSpacing = ReadCustomFloat( _T( "CONFIG" ), _T( "LETTER SPACING" ), 2.0f );
		m_fWordSpacing = ReadCustomFloat( _T( "CONFIG" ), _T( "WORD SPACING" ), 10.0f );
		m_fLineSpacing = ReadCustomFloat( _T( "CONFIG" ), _T( "LINES SPACING" ), 2.0f );
		m_fNumberOfLines = ReadCustomFloat( _T( "CONFIG" ), _T( "NUMBER OF LINES" ), 25.0f );
		m_fItalicSlant = ReadCustomFloat( _T( "CONFIG" ), _T( "ITALIC SLANT" ), 2.0f );
		m_bOneToOne = ReadCustomInt( _T( "CONFIG" ), _T( "ONE TO ONE" ), 0 );
		m_bDrawGuides = ReadCustomInt( _T( "CONFIG" ), _T( "DRAW GUIDES" ), 0 );
		m_bFixedWidthFont = ReadCustomInt( _T( "CONFIG" ), _T( "FIXED WIDTH FONT" ), 0 );
		m_bFirstRun = ReadCustomInt( _T( "CONFIG" ), _T( "FIXED WIDTH FONT" ), 1 );
		m_bItalicFont = ReadCustomInt( _T( "CONFIG" ), _T( "ITALIC FONT" ), 0 );
		ReadCustomString( _T( "CONFIG" ), _T( "MASTER FILE" ), _T( "" ), szTempString, MAX_PATH );
		m_sMasterFile = szTempString;
		ReadCustomString( _T( "CONFIG" ), _T( "INPUT FILE" ), _T( "" ), szTempString, MAX_PATH );
		m_sInputFile = szTempString;
		ReadCustomString( _T( "CONFIG" ), _T( "OUTPUT DIR" ), _T( "" ), szTempString, MAX_PATH );
		m_sOutputDir = szTempString;
		ReadCustomString( _T( "CONFIG" ), _T( "CHAR SET" ), _T( "" ), szTempString, MAX_PATH );
		m_sCharSet = szTempString;
		// read our VIDEO settings
		ReadCustomString( _T( "VIDEO" ), _T( "DEV NAME" ), _T( "" ), szTempString, MAX_PATH );
		m_sDevName = szTempString;
		m_nDevFlags = ReadCustomInt( _T( "VIDEO" ), _T( "DEV FLAGS" ), 0 );
		m_nDevOrdinal = ReadCustomInt( _T( "VIDEO" ), _T( "DEV ORDINAL" ), 0 );
		m_nDevRenderer = ReadCustomInt( _T( "VIDEO" ), _T( "DEV RENDERER" ), 0 );
		m_nModeFlags = ReadCustomInt( _T( "VIDEO" ), _T( "MODE FLAGS" ), 0 );
		m_nModeColorBits = ReadCustomInt( _T( "VIDEO" ), _T( "MODE COLOR BITS" ), 0 );
		m_nModeDepthBits = ReadCustomInt( _T( "VIDEO" ), _T( "MODE DEPTH BITS" ), 0 );
		m_nModeStencilBits = ReadCustomInt( _T( "VIDEO" ), _T( "MODE STENCIL BITS" ), 0 );
		m_nModePixelsAcross = ReadCustomInt( _T( "VIDEO" ), _T( "MODE PIXELS ACROSS" ), 0 );
		m_nModePixelsDown = ReadCustomInt( _T( "VIDEO" ), _T( "MODE PIXELS DOWN" ), 0 );
		m_nSwapInterval = ReadCustomInt( _T( "VIDEO" ), _T( "MODE SWAP INTERVAL" ), 0 );
		m_fUnitFSAA = ReadCustomFloat( _T( "VIDEO" ), _T( "UNIT FSAA" ), 0 );

		// mark that we don't need to read the data from the file anymore
		m_bReadCommonDataFromFile = FALSE;
	}
}
