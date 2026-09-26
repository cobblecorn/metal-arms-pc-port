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
	WriteCustomString( "CONFIG", "LAST FILE", (cchar *)m_sLastFileWorkedOn );
	WriteCustomString( "CONFIG", "MASTER FILE", (cchar *)m_sMasterFileLocation );
	WriteCustomFloat( "CONFIG", "CAMERA X", m_fCameraX );
	WriteCustomFloat( "CONFIG", "CAMERA Y", m_fCameraY );
	WriteCustomFloat( "CONFIG", "CAMERA Z", m_fCameraZ );
	WriteCustomFloat( "CONFIG", "CAMERA PITCH", m_fCameraPitch );
	WriteCustomFloat( "CONFIG", "CAMERA HEADING", m_fCameraHeading );
	WriteCustomInt( "CONFIG", "EMIT Y", m_nEmitY );
	WriteCustomInt( "CONFIG", "EMIT ROT", m_nEmitRot );
	WriteCustomInt( "CONFIG", "AMBIENT ON", m_bAmbientOn );
	WriteCustomFloat( "CONFIG", "AMBIENT RED", m_fAmbientRed );
	WriteCustomFloat( "CONFIG", "AMBIENT GREEN", m_fAmbientGreen );
	WriteCustomFloat( "CONFIG", "AMBIENT BLUE", m_fAmbientBlue );

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
	char szTempString[_MAX_PATH];
	
	if( m_bReadCommonDataFromFile ) {
		// read our CONFIG settings
		ReadCustomString( "CONFIG", "LAST FILE", "", szTempString, _MAX_PATH );
		m_sLastFileWorkedOn = szTempString;
		ReadCustomString( "CONFIG", "MASTER FILE", "", szTempString, _MAX_PATH );
		m_sMasterFileLocation = szTempString;
		m_fCameraX = ReadCustomFloat( "CONFIG", "CAMERA X", 0.0f );
		m_fCameraY = ReadCustomFloat( "CONFIG", "CAMERA Y", 0.0f );
		m_fCameraZ = ReadCustomFloat( "CONFIG", "CAMERA Z", 0.0f );
		m_fCameraPitch = ReadCustomFloat( "CONFIG", "CAMERA PITCH", 0.0f );
		m_fCameraHeading = ReadCustomFloat( "CONFIG", "CAMERA HEADING", 0.0f );
		m_nEmitY = ReadCustomInt( "CONFIG", "EMIT Y", 0 );
		m_nEmitRot = ReadCustomInt( "CONFIG", "EMIT ROT", 0 );
		m_bAmbientOn = ReadCustomInt( "CONFIG", "AMBIENT ON", 0 );
		m_fAmbientRed = ReadCustomFloat( "CONFIG", "AMBIENT RED", 1.0f );
		m_fAmbientGreen = ReadCustomFloat( "CONFIG", "AMBIENT GREEN", 1.0f );
		m_fAmbientBlue = ReadCustomFloat( "CONFIG", "AMBIENT BLUE", 1.0f );
		
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
		
		// mark that we don't need to read the data from the file anymore
		m_bReadCommonDataFromFile = FALSE;
	}
}