//////////////////////////////////////////////////////////////////////////////////////
// ConfigFile.cpp - 
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
// 05/31/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "ConfigFile.h"
#include "fversion.h"

#define _CONFIG_VERSION 0x01234569	// 32 BIT NUMBER

CConfigFile::CConfigFile() {
	m_sMasterFilename = "";
	m_sLibDir = "";
	m_sLocalDir = "";
	m_sLibraryLockDir = "";
}

CConfigFile::~CConfigFile() {
}

BOOL CConfigFile::Read( cchar *pszFilename ) {
	FILE *pFile;
	u32 nSignature, nVersion;

	if( !pszFilename ) {
		return FALSE;
	}
	// reset our vars
	m_sMasterFilename = "";
	m_sLibDir = "";
	m_sLocalDir = "";
	m_sLibraryLockDir = "";

	// open the file
	pFile = fopen( pszFilename, "rb" );
	if( !pFile )  {
		return FALSE;
	}

	// Read signature
	if( fread( &nSignature, sizeof( u32 ), 1, pFile ) != 1 ) {
		fclose( pFile );
		return FALSE;
	}
	if( nSignature != FVERSION_FILE_SIGNATURE ) {
		// this is not a valid config file
		fclose( pFile );
		return FALSE;
	}
	
	// Read version
	if( fread( &nVersion, sizeof( u32 ), 1, pFile ) != 1 ) {
		fclose( pFile );
		return FALSE;
	}

	// Read the data
	switch( nVersion ) {
		
	case _CONFIG_VERSION:
		// THE LATEST VERSION
		if( fread( m_sString, TEMP_STRING_LEN, 1, pFile ) != 1 ) {
			fclose( pFile );
			return FALSE;
		}
		m_sMasterFilename = m_sString;

		if( fread( m_sString, TEMP_STRING_LEN, 1, pFile ) != 1 ) {
			fclose( pFile );
			return FALSE;
		}
		m_sLibDir = m_sString;

		if( fread( m_sString, TEMP_STRING_LEN, 1, pFile ) != 1 ) {
			fclose( pFile );
			return FALSE;
		}
		m_sLocalDir = m_sString;
	
		if( fread( m_sString, TEMP_STRING_LEN, 1, pFile ) != 1 ) {
			fclose( pFile );
			return FALSE;
		}
		m_sLibraryLockDir = m_sString;
		break;

	case 0x01234568:
		// the 2nd version - THIS VERSION WAS FLAWED, IGNORE THE LIB & LIBLOCK DIRS
		if( fread( m_sString, TEMP_STRING_LEN, 1, pFile ) != 1 ) {
			fclose( pFile );
			return FALSE;
		}
		m_sMasterFilename = m_sString;

		if( fread( m_sString, TEMP_STRING_LEN, 1, pFile ) != 1 ) {
			fclose( pFile );
			return FALSE;
		}
		//m_sLibDir = m_sString;

		if( fread( m_sString, TEMP_STRING_LEN, 1, pFile ) != 1 ) {
			fclose( pFile );
			return FALSE;
		}
		m_sLocalDir = m_sString;
		
		if( fread( m_sString, TEMP_STRING_LEN, 1, pFile ) != 1 ) {
			fclose( pFile );
			return FALSE;
		}
		//m_sLibraryLockDir = m_sString;
		
		break;

	case 0x01234567:
		// the 1st version
		if( fread( m_sString, TEMP_STRING_LEN, 1, pFile ) != 1 ) {
			fclose( pFile );
			return FALSE;
		}
		m_sMasterFilename = m_sString;

		if( fread( m_sString, TEMP_STRING_LEN, 1, pFile ) != 1 ) {
			fclose( pFile );
			return FALSE;
		}
		m_sLibDir = m_sString;

		if( fread( m_sString, TEMP_STRING_LEN, 1, pFile ) != 1 ) {
			fclose( pFile );
			return FALSE;
		}
		m_sLocalDir = m_sString;
		break;

	default:
		// unknown version, not a valid config file
		fclose( pFile );
		return FALSE; 
	}

	fclose( pFile );

	return TRUE;
}

BOOL CConfigFile::Write( cchar *pszFilename ) {
	FILE *pFile;
	u32 nSignature, nVersion;

	if( !pszFilename ) {
		return FALSE;
	}
	// open the file
	pFile = fopen( pszFilename, "wb" );
	if( !pFile )  {
		return FALSE;
	}

	// Write FVERSION_FILE_SIGNATURE
	nSignature = FVERSION_FILE_SIGNATURE;
	if( fwrite( &nSignature, sizeof( u32 ), 1, pFile ) != 1 ) {
		fclose( pFile );
		return FALSE;
	}

	// Write _CONFIG_VERSION
	nVersion = _CONFIG_VERSION;
	if( fwrite( &nVersion, sizeof( u32 ), 1, pFile ) != 1 ) {
		fclose( pFile );
		return FALSE;
	}

	// Write the data
	strncpy( m_sString, m_sMasterFilename, TEMP_STRING_LEN );
	if( fwrite( m_sString, TEMP_STRING_LEN, 1, pFile ) != 1 ) {
		fclose( pFile );
		return FALSE;
	}	

	strncpy( m_sString, m_sLibDir, TEMP_STRING_LEN );
	if( fwrite( m_sString, TEMP_STRING_LEN, 1, pFile ) != 1 ) {
		fclose( pFile );
		return FALSE;
	}

	strncpy( m_sString, m_sLocalDir, TEMP_STRING_LEN );
	if( fwrite( m_sString, TEMP_STRING_LEN, 1, pFile ) != 1 ) {
		fclose( pFile );
		return FALSE;
	}

	strncpy( m_sString, m_sLibraryLockDir, TEMP_STRING_LEN );
	if( fwrite( m_sString, TEMP_STRING_LEN, 1, pFile ) != 1 ) {
		fclose( pFile );
		return FALSE;
	}

	fclose( pFile );

	return TRUE;
}



	


