//////////////////////////////////////////////////////////////////////////////////////
// ConfigMgr.cpp - based on the skeleton exporter max exporter
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
// 06/05/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "SceneExport.h"

#define		CFG_FILENAME	"apetools.ini"
#define		CFG_SECTION		"APE EXPORTER"

static TSTR _sFilename;

static void _WriteCustomInt( cchar *pszSectionName, cchar *pszItemName, u32 nValue );
static void _WriteCustomString( cchar *pszSectionName, cchar *pszItemName, cchar *pszString );
static void _WriteCustomFloat( cchar *pszSectionName, cchar *pszItemName, f32 fValue );
static u32 _ReadCustomInt( cchar *pszSectionName, cchar *pszItemName, u32 nDefault );
static void _ReadCustomString( cchar *pszSectionName, 
							   cchar *pszItemName, 
							   cchar *pszDefault,
							   char *pszDest, 
							   u32 nSizeOfDest );
static f32 _ReadCustomFloat( cchar *pszSectionName, cchar *pszItemName, f32 fDefault );

/*===========================================================================*\
 |  Get the config file's full name on the disk
\*===========================================================================*/

TSTR ApeExporter::GetConfigFilename() {
	TSTR filename;
	
	filename += m_pInterface->GetDir( APP_PLUGCFG_DIR );
	filename += "\\";
	filename += CFG_FILENAME;

	return filename;
}

void ApeExporter::SaveExporterConfig() {
	
	_sFilename = GetConfigFilename();

	_WriteCustomInt( CFG_SECTION, "EXPORT_APE_LIGHTS",		m_bApeCfgExportLights );
	_WriteCustomInt( CFG_SECTION, "EXPORT_APE_GEOMETRY",	m_bApeCfgExportGeo );
	_WriteCustomInt( CFG_SECTION, "EXPORT_APE_SKIN",		m_bApeCfgExportSkin );
	_WriteCustomInt( CFG_SECTION, "EXPORT_WLD_GEOMETRY",	m_bWldCfgExportGeo );
	_WriteCustomInt( CFG_SECTION, "EXPORT_WLD_LIGHTS",		m_bWldCfgExportLights );
	_WriteCustomInt( CFG_SECTION, "EXPORT_WLD_OBJECTS",		m_bWldCfgExportObjects );
}

BOOL ApeExporter::LoadExporterConfig() {
	
	_sFilename = GetConfigFilename();

	m_bApeCfgExportLights	= _ReadCustomInt( CFG_SECTION, "EXPORT_APE_LIGHTS",		FALSE );
	m_bApeCfgExportGeo		= _ReadCustomInt( CFG_SECTION, "EXPORT_APE_GEOMETRY",	TRUE );
	m_bApeCfgExportSkin		= _ReadCustomInt( CFG_SECTION, "EXPORT_APE_SKIN",		FALSE );
	m_bWldCfgExportGeo		= _ReadCustomInt( CFG_SECTION, "EXPORT_WLD_GEOMETRY",	TRUE );
	m_bWldCfgExportLights	= _ReadCustomInt( CFG_SECTION, "EXPORT_WLD_LIGHTS",		FALSE );
	m_bWldCfgExportObjects	= _ReadCustomInt( CFG_SECTION, "EXPORT_WLD_OBJECTS",	FALSE );
	
	// filter out any values that are not 0 or 1
	if( m_bApeCfgExportLights > 1 ) {
		m_bApeCfgExportLights = FALSE;
	}
	if( m_bApeCfgExportGeo > 1 ) {
		m_bApeCfgExportGeo = FALSE;
	}
	if( m_bApeCfgExportSkin > 1 ) {
		m_bApeCfgExportSkin = FALSE;
	}
	if( m_bWldCfgExportGeo > 1 ) {
		m_bWldCfgExportGeo = FALSE;
	}
	if( m_bWldCfgExportLights > 1 ) {
		m_bWldCfgExportLights = FALSE;
	}
	if( m_bWldCfgExportObjects > 1 ) {
		m_bWldCfgExportObjects = FALSE;
	}
	
	return TRUE;
}

static void _WriteCustomInt( cchar *pszSectionName, cchar *pszItemName, u32 nValue ) {
	TSTR s;

	s.printf( "%d", nValue );
	_WriteCustomString( pszSectionName, pszItemName, (cchar *)s );
}

static void _WriteCustomString( cchar *pszSectionName, cchar *pszItemName, cchar *pszString ) {
	WritePrivateProfileString( pszSectionName, pszItemName, pszString, (cchar *)_sFilename );
}

static void _WriteCustomFloat( cchar *pszSectionName, cchar *pszItemName, f32 fValue ) {
	TSTR s;

	s.printf( "%f", fValue );
	_WriteCustomString( pszSectionName, pszItemName, (cchar *)s );
}

static u32 _ReadCustomInt( cchar *pszSectionName, cchar *pszItemName, u32 nDefault ) {
	u32 nReturnVal = GetPrivateProfileInt( pszSectionName, pszItemName, nDefault, (cchar *)_sFilename );
	return nReturnVal;
}

static void _ReadCustomString( cchar *pszSectionName, 
								  cchar *pszItemName, 
								  cchar *pszDefault,
								  char *pszDest, 
								  u32 nSizeOfDest ) {
	GetPrivateProfileString( pszSectionName, pszItemName, pszDefault, pszDest, nSizeOfDest, (cchar *)_sFilename );
}

static f32 _ReadCustomFloat( cchar *pszSectionName, cchar *pszItemName, f32 fDefault ) {
	char szTempString[32];
	TSTR s;
	f32 fReturnVal;

	s.printf( "%f", fDefault );
	GetPrivateProfileString( pszSectionName, pszItemName, (cchar *)s, szTempString, 32, (cchar *)_sFilename );
	sscanf( szTempString, "%f", &fReturnVal );
	return fReturnVal;
}
