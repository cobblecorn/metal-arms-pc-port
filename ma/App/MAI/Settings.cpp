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


extern f32 fCamXLatSpeed;
CSettings::CSettings() {
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

CSettings& CSettings::GetCurrent(void) {
	// make sure that a settings file has been specified
	ASSERT( _sSettingsFilename != "" );

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
	char szTempString[_MAX_PATH];
	
	// save our CONFIG settings
	WriteCustomString(	"CONFIG", "DISPLAY LIST DIR",	(cchar *)m_sDisplayListPath );
	WriteCustomString(	"CONFIG", "ART DIR",			(cchar *)m_sArtPath );
	WriteCustomString(	"CONFIG", "LAST FILE",			(cchar *)m_sLastFile );
	WriteCustomInt(		"CONFIG", "RADIUS",				m_nRadius );
	WriteCustomFloat(	"CONFIG", "3D BG RED",			m_f3dBGRed );
	WriteCustomFloat(	"CONFIG", "3D BG GREEN",		m_f3dBGGreen );
	WriteCustomFloat(	"CONFIG", "3D BG BLUE",			m_f3dBGBlue );
	WriteCustomInt(		"CONFIG", "DRAW PERF STATS",	m_bDrawPerfStats );
	WriteCustomInt(		"CONFIG", "DRAW FPS",			m_bDrawFPS );
	WriteCustomInt(		"CONFIG", "DRAW CAM INFO",		m_bDrawCamInfo );
	WriteCustomInt(		"CONFIG", "DRAW LEAF INFO",		m_bDrawLeafInfo );
	WriteCustomString(	"CONFIG", "SCREEN SHOT DIR",	(cchar *)m_sScreenshotPath );
	wsprintf(szTempString, "%s Cam X", m_sLastFile);
	WriteCustomFloat(	"CONFIG", szTempString,		  m_fBookmarkCamX );
	wsprintf(szTempString, "%s Cam Y", m_sLastFile);
	WriteCustomFloat(	"CONFIG", szTempString,		  m_fBookmarkCamY );
	wsprintf(szTempString, "%s Cam Z", m_sLastFile);
	WriteCustomFloat(	"CONFIG", szTempString,		  m_fBookmarkCamZ );
	wsprintf(szTempString, "%s Cam Zoom", m_sLastFile);
	WriteCustomFloat(	"CONFIG", szTempString,	  m_fBookmarkCamZoom );
	wsprintf(szTempString, "%s Cam Heading", m_sLastFile);
	WriteCustomFloat(	"CONFIG", szTempString, m_fBookmarkCamHeading );
	wsprintf(szTempString, "%s Cam Pitch", m_sLastFile);
	WriteCustomFloat(	"CONFIG", szTempString,	  m_fBookmarkCamPitch );
	wsprintf(szTempString, "%s Cam Speed", m_sLastFile);
	WriteCustomFloat(	"CONFIG", szTempString,	  fCamXLatSpeed );
	WriteCustomInt(		"CONFIG", "Bookmark Valid",		m_bBookmarkValid );
	WriteCustomInt(		"CONFIG", "DRAW CURSOR",		m_bDrawCursor );
		
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

void CSettings::GetCommonDataFromFile(cchar* pszLevelName) {
	char szTempString[_MAX_PATH];
	
	// read our CONFIG settings
	ReadCustomString( "CONFIG", "DISPLAY LIST DIR", "", szTempString, _MAX_PATH );
	m_sDisplayListPath = szTempString;
	ReadCustomString( "CONFIG", "ART DIR", "", szTempString, _MAX_PATH );
	m_sArtPath = szTempString;
	ReadCustomString( "CONFIG", "LAST FILE", "", szTempString, _MAX_PATH );
	m_sLastFile = szTempString;
	CString lookForLevel;
	if (pszLevelName == NULL)
	{
		lookForLevel = szTempString;
	}
	else
	{
		lookForLevel = pszLevelName; 
		m_sLastFile = lookForLevel;
	}
	m_nRadius = ReadCustomInt( "CONFIG", "RADIUS", 50 );
	m_f3dBGRed = ReadCustomFloat( "CONFIG", "3D BG RED", 0.0f );
	m_f3dBGGreen = ReadCustomFloat( "CONFIG", "3D BG GREEN", 0.0f );
	m_f3dBGBlue = ReadCustomFloat( "CONFIG", "3D BG BLUE", 0.0f );
	m_bDrawPerfStats = ReadCustomInt( "CONFIG", "DRAW PERF STATS", FALSE );
	m_bDrawFPS = ReadCustomInt( "CONFIG", "DRAW FPS", TRUE );
	m_bDrawCamInfo = ReadCustomInt( "CONFIG", "DRAW CAM INFO", TRUE );
	m_bDrawLeafInfo = ReadCustomInt( "CONFIG", "DRAW LEAF INFO", TRUE );
	ReadCustomString( "CONFIG", "SCREEN SHOT DIR", "", szTempString, _MAX_PATH );
	m_sScreenshotPath = szTempString;
	wsprintf(szTempString, "%s Cam X", lookForLevel);
	m_fBookmarkCamX = ReadCustomFloat( "CONFIG", szTempString, 0.0f );
	wsprintf(szTempString, "%s Cam Y", lookForLevel);
	m_fBookmarkCamY = ReadCustomFloat( "CONFIG", szTempString, 0.0f );
	wsprintf(szTempString, "%s Cam Z", lookForLevel);
	m_fBookmarkCamZ = ReadCustomFloat( "CONFIG", szTempString, 0.0f );
	wsprintf(szTempString, "%s Cam Zoom", lookForLevel);
	m_fBookmarkCamZoom = ReadCustomFloat( "CONFIG", szTempString, 0.0f );
	wsprintf(szTempString, "%s Cam Heading", lookForLevel);
	m_fBookmarkCamHeading = ReadCustomFloat( "CONFIG", szTempString, 0.0f );
	wsprintf(szTempString, "%s Cam Pitch", lookForLevel);
	m_fBookmarkCamPitch = ReadCustomFloat( "CONFIG", szTempString, 0.0f );
	wsprintf(szTempString, "%s Cam Speed", lookForLevel);
	fCamXLatSpeed = ReadCustomFloat( "CONFIG", szTempString, 0.0f );

	m_bBookmarkValid = ReadCustomInt( "CONFIG", "Bookmark Valid", FALSE );
	m_bDrawCursor = ReadCustomInt( "CONFIG", "DRAW CURSOR", TRUE );

	// read our VIDEO settings
	ReadCustomString( "VIDEO", "DEV NAME", "", szTempString, _MAX_PATH );
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
	
}