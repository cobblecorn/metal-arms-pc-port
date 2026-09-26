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
	WriteCustomString(	"CONFIG", "ART DIR",			(cchar *)m_sArtPath );
	WriteCustomString(	"CONFIG", "LAST FILE",			(cchar *)m_sLastFile );
	WriteCustomFloat(	"CONFIG", "3D BG RED",			m_f3dBGRed );
	WriteCustomFloat(	"CONFIG", "3D BG GREEN",		m_f3dBGGreen );
	WriteCustomFloat(	"CONFIG", "3D BG BLUE",			m_f3dBGBlue );
	WriteCustomString(	"CONFIG", "SCREEN SHOT DIR",	(cchar *)m_sScreenshotPath );
	WriteCustomInt(		"CONFIG", "SKIP LEVEL SELECT",	m_bSkipLevelSelect );
	WriteCustomInt(		"CONFIG", "XBOX EMULATION",		m_bXbox );
	WriteCustomInt(		"CONFIG", "GAMECUBE EMULATION",	m_bGameCube );
	WriteCustomInt(		"CONFIG", "MEMCARD SLOT 1",		m_bMemSlot1 );
	WriteCustomInt(		"CONFIG", "MEMCARD SLOT 2",		m_bMemSlot2 );
	WriteCustomInt(		"CONFIG", "SHOW FPS",			m_bShowFPS );
	WriteCustomString(	"CONFIG", "MEMCARD DIR",		(cchar *)m_sMemCardPath );
	WriteCustomInt(		"CONFIG", "INSTALL AUDIO",		m_bInstallAudio );
	WriteCustomInt(		"CONFIG", "DEBUG AI",		m_bDebugAI );
	WriteCustomInt(		"CONFIG", "PLAYER DEATH",		m_bPlayerDeath);
	WriteCustomInt(		"CONFIG", "SERVO LEG LEVEL",	m_nServoLegLevel);
	WriteCustomInt(		"CONFIG", "SERVO ARM LEVEL",	m_nServoArmLevel);

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

	// save our INPUT EMULATION settings
	WriteCustomString(	"INPUT EMULATION", "DEV NAME",	(cchar *)m_sInputEmulationDevName );

	CString oTemp;
	for( u32 uIndex = 0; uIndex < 36; ++uIndex ) {
		oTemp.Format( "EMUL %u MAP", uIndex );
		WriteCustomInt(	"INPUT EMULATION", oTemp,	m_auInputEmulationMap[ uIndex ] );
	}
}

void CSettings::GetCommonDataFromFile() {
	char szTempString[_MAX_PATH];
	
	if( m_bReadCommonDataFromFile ) {
		// read our CONFIG settings
		ReadCustomString( "CONFIG", "ART DIR", "", szTempString, _MAX_PATH );
		m_sArtPath = szTempString;
		ReadCustomString( "CONFIG", "LAST FILE", "", szTempString, _MAX_PATH );
		m_sLastFile = szTempString;
		m_f3dBGRed = ReadCustomFloat( "CONFIG", "3D BG RED", 0.0f );
		m_f3dBGGreen = ReadCustomFloat( "CONFIG", "3D BG GREEN", 0.0f );
		m_f3dBGBlue = ReadCustomFloat( "CONFIG", "3D BG BLUE", 0.0f );
		ReadCustomString( "CONFIG", "SCREEN SHOT DIR", "", szTempString, _MAX_PATH );
		m_sScreenshotPath = szTempString;
		m_bSkipLevelSelect = ReadCustomInt( "CONFIG", "SKIP LEVEL SELECT", 0 );
		m_bXbox = ReadCustomInt( "CONFIG", "XBOX EMULATION", TRUE );
		m_bGameCube = ReadCustomInt( "CONFIG", "GAMECUBE EMULATION", FALSE);
		m_bMemSlot1 = ReadCustomInt( "CONFIG", "MEMCARD SLOT 1", TRUE );
		m_bMemSlot2 =  ReadCustomInt( "CONFIG", "MEMCARD SLOT 2", FALSE );
		m_bShowFPS = ReadCustomInt(	"CONFIG", "SHOW FPS", FALSE );
		ReadCustomString(	"CONFIG", "MEMCARD DIR", "", szTempString, _MAX_PATH );
		m_sMemCardPath = szTempString;
		m_bInstallAudio = ReadCustomInt( "CONFIG", "INSTALL AUDIO", TRUE );
		m_bDebugAI = ReadCustomInt( "CONFIG", "DEBUG AI", FALSE );
		m_bPlayerDeath = ReadCustomInt( "CONFIG", "PLAYER DEATH", TRUE );
		m_nServoLegLevel = ReadCustomInt( "CONFIG", "SERVO LEG LEVEL", TRUE );
		m_nServoArmLevel = ReadCustomInt( "CONFIG", "SERVO ARM LEVEL", TRUE );

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

		// read our VIDEO settings
		ReadCustomString( "INPUT EMULATION", "DEV NAME", "", szTempString, _MAX_PATH );
		m_sInputEmulationDevName = szTempString;

		CString oTemp;
		for( u32 uIndex = 0; uIndex < 36; ++uIndex ) {
			oTemp.Format( "EMUL %u MAP", uIndex );
			m_auInputEmulationMap[ uIndex ] = ReadCustomInt( "INPUT EMULATION", oTemp, 0 );
		}

		// mark that we don't need to read the data from the file anymore
		m_bReadCommonDataFromFile = FALSE;
	} else {
		//defaults?
		m_bDebugAI = FALSE;
		m_bPlayerDeath  = TRUE;
	}
}