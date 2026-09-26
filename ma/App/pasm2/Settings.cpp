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
	WriteCustomString(	"CONFIG", "XBOX OUTPUT DIR",	(cchar *)m_sXBoxPath );
	WriteCustomString(	"CONFIG", "PS2 OUTPUT DIR",		(cchar *)m_sPS2Path );
	WriteCustomString(	"CONFIG", "PC OUTPUT DIR",		(cchar *)m_sPCPath );
	WriteCustomInt(		"CONFIG", "XBOX FORMAT ON",		m_bXBoxFormat );
	WriteCustomInt(		"CONFIG", "PC FORMAT ON",		m_bPCFormat );
	WriteCustomInt(		"CONFIG", "PS2 FORMAT ON",		m_bPS2Format );
	WriteCustomInt(		"CONFIG", "GC FORMAT ON",		m_bGCFormat );
	WriteCustomString(	"CONFIG", "LAST INPUT DIR",		m_sLastInputPath );
	WriteCustomString(	"CONFIG", "LAST OUTPUT NAME",	m_sLastOutputName );
	WriteCustomFloat(	"CONFIG", "DITHER",				m_fDither );
	WriteCustomInt(		"CONFIG", "ALT COLORS",			m_nAltColors );
	WriteCustomInt(		"CONFIG", "LEAF NODE SIZE",		m_nLeafNodeSize );
	WriteCustomString(	"CONFIG", "LOG FILENAME",		m_sLogFilename );
	WriteCustomInt(		"CONFIG", "AUTO LEAF SIZE",		m_bAutoLeafSize );

	// save our PASM2 settings
	WriteCustomInt(		"PASM2", "SHOW DIFFERENCES",	m_bShowDifferences );
	WriteCustomInt(		"PASM2", "RUN CONFIG",			m_bMustRunConfig );
	WriteCustomString(	"PASM2", "MASTER FILE DIR",		m_sMasterFileDir );
	WriteCustomString(	"PASM2", "LOG FILE DIR",		m_sLogDir );
	WriteCustomString(	"PASM2", "EXTRACT FILE DIR",	m_sExtractDir );
	WriteCustomString(	"PASM2", "CONFIG FILE DIR",		m_sConfigDir );
	WriteCustomString(	"PASM2", "IGNORE FILE EXT",		m_sIgnoreExtensions );
	WriteCustomString(	"PASM2", "CONFIG FILE NAME",	m_sConfigFilename );
	WriteCustomInt(		"PASM2", "SORT COLUMN",			m_nSortColumn );
	WriteCustomInt(		"PASM2", "ASCENDING SORT",		m_bAscendingSort );
	WriteCustomInt(		"PASM2", "HIDE NON-COMPLIABLE FILES", m_bHideNonCompliableFiles );
	WriteCustomInt(		"PASM2", "HOLD COMPILE STATS", m_bHoldCompileStats );	
	WriteCustomInt(		"PASM2", "DISABLE LIBRARY SUPPORT", m_bDisableLibrarySupport );
	WriteCustomString(	"PASM2", "GC MASTER FILE DIR",	m_sGCMasterFileDir );
	WriteCustomString(	"PASM2", "XB MASTER FILE DIR",	m_sXBMasterFileDir );
#ifdef _MMI_TARGET_PS2
    WriteCustomString(  "PASM2", "PS2 MASTER FILE DIR",  m_sPS2MasterFileDir );
#endif
	// !!Nate
	WriteCustomString(  "PASM2", "XCLIENT FILE DIR",    m_sXClientFileDir );
	WriteCustomInt(     "PASM2", "XCLIENT AUTO RUN",    m_bAutoRunXClient );
}

void CSettings::GetCommonDataFromFile() {
	char szTempString[_MAX_PATH];
	
	if( m_bReadCommonDataFromFile ) {
		// read our CONFIG settings
		ReadCustomString( "CONFIG", "XBOX OUTPUT DIR", "", szTempString, _MAX_PATH );
		m_sXBoxPath = szTempString;
		ReadCustomString( "CONFIG", "PS2 OUTPUT DIR", "", szTempString, _MAX_PATH );
		m_sPS2Path = szTempString;
		ReadCustomString( "CONFIG", "PC OUTPUT DIR", "", szTempString, _MAX_PATH );
		m_sPCPath = szTempString;
		m_bXBoxFormat = ReadCustomInt(	"CONFIG", "XBOX FORMAT ON", TRUE );
		m_bPCFormat = ReadCustomInt(	"CONFIG", "PC FORMAT ON",	FALSE );
		m_bPS2Format = ReadCustomInt(	"CONFIG", "PS2 FORMAT ON",	FALSE );
		m_bGCFormat = ReadCustomInt(	"CONFIG", "GC FORMAT ON",	FALSE );
		ReadCustomString( "CONFIG", "LAST INPUT DIR", "", szTempString, _MAX_PATH );
		m_sLastInputPath = szTempString;
		ReadCustomString( "CONFIG", "LAST OUTPUT NAME", "c:\\pasm_file_dump.csv", szTempString, _MAX_PATH );
		m_sLastOutputName = szTempString;
		m_fDither = ReadCustomFloat( "CONFIG", "DITHER", 0.6f );
		m_nAltColors = ReadCustomInt( "CONFIG", "ALT COLORS", 48 );
		m_nLeafNodeSize	= ReadCustomInt( "CONFIG", "LEAF NODE SIZE", 32 );
		ReadCustomString( "CONFIG", "LOG FILENAME", "c:\\pasm_error_log.txt", szTempString, _MAX_PATH );
		m_sLogFilename = szTempString;
		m_bAutoLeafSize = ReadCustomInt( "CONFIG", "AUTO LEAF SIZE", TRUE );

		// save our PASM2 settings
		m_bShowDifferences = ReadCustomInt( "PASM2", "SHOW DIFFERENCES", FALSE );
		m_bMustRunConfig = ReadCustomInt( "PASM2", "RUN CONFIG", TRUE );
		ReadCustomString( "PASM2", "MASTER FILE DIR", "", szTempString, _MAX_PATH );
		m_sMasterFileDir = szTempString;
		ReadCustomString( "PASM2", "LOG FILE DIR", "", szTempString, _MAX_PATH );
		m_sLogDir = szTempString;
		ReadCustomString( "PASM2", "EXTRACT FILE DIR", "", szTempString, _MAX_PATH );
		m_sExtractDir = szTempString;
		ReadCustomString( "PASM2", "CONFIG FILE DIR", "", szTempString, _MAX_PATH );
		m_sConfigDir = szTempString;
		ReadCustomString( "PASM2", "IGNORE FILE EXT", "*.mst;*.max;*.mb;*.ma;*.psd;*.xls;*.txt;*.doc;", szTempString, _MAX_PATH );
		m_sIgnoreExtensions = szTempString;
		ReadCustomString( "PASM2", "CONFIG FILE NAME", "default.cfg", szTempString, _MAX_PATH );
		m_sConfigFilename = szTempString;
		m_nSortColumn = ReadCustomInt( "PASM2", "SORT COLUMN", 0 );
		m_bAscendingSort = ReadCustomInt( "PASM2", "ASCENDING SORT", TRUE );	
		m_bHideNonCompliableFiles = ReadCustomInt( "PASM2", "HIDE NON-COMPLIABLE FILES", FALSE );
		m_bHoldCompileStats = ReadCustomInt( "PASM2", "HOLD COMPILE STATS", FALSE );
		m_bDisableLibrarySupport = ReadCustomInt( "PASM2", "DISABLE LIBRARY SUPPORT", FALSE );
		ReadCustomString( "PASM2", "GC MASTER FILE DIR", "", szTempString, _MAX_PATH );
		m_sGCMasterFileDir = szTempString;
		ReadCustomString( "PASM2", "XB MASTER FILE DIR", "", szTempString, _MAX_PATH );
		m_sXBMasterFileDir = szTempString;
#ifdef _MMI_TARGET_PS2
        ReadCustomString( "PASM2", "PS2 MASTER FILE DIR", "", szTempString, _MAX_PATH );
        m_sPS2MasterFileDir = szTempString;
#endif
		// !!Nate
		ReadCustomString( "PASM2", "XCLIENT FILE DIR", "h:\\tools\\ape_tools\\xclient.exe", szTempString, _MAX_PATH );
		m_sXClientFileDir = szTempString;
		m_bAutoRunXClient = ReadCustomInt( "PASM2", "XCLIENT AUTO RUN", FALSE );

		// if we haven't set a gc or xb file dir, copy it from the master file dir
		if( !m_sMasterFileDir.IsEmpty() ) {
			if( m_sGCMasterFileDir.IsEmpty() ) {
				m_sGCMasterFileDir = m_sMasterFileDir;
			}
			if( m_sXBMasterFileDir.IsEmpty() ) {
				m_sXBMasterFileDir = m_sMasterFileDir;
			}
#ifdef _MMI_TARGET_PS2
            if( m_sPS2MasterFileDir.IsEmpty() ) {
                m_sPS2MasterFileDir = m_sMasterFileDir;
			}
#endif
		}
				
		// mark that we don't need to read the data from the file anymore
		m_bReadCommonDataFromFile = FALSE;
	}
}