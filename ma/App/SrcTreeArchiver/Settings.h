//////////////////////////////////////////////////////////////////////////////////////
// Settings.h - "manages reading/writting/store data from the .ini file"
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
#ifndef _SETTINGS_H_
#define _SETTINGS_H_ 1

#include "fang.h"

#define SETTINGS_MAX_PROFILES		5

class CSrcTreeArchiverDlg;

class CSettings
{
public:
	CSettings();	// don't call this, use GetCurrent(), we only 1 instance of this 
					// class in the application, and is declared locally to this .cpp file
	~CSettings();
	
	static void SetSettingsFilename( cchar *pszSettingsFilenameOnly );
	static void SetApplicationName( cchar *pszAppName );
	static cchar *GetApplicationName();
    static CSettings& GetCurrent();

	// helper functions to return common data items
	u32 GetProfileIndex()					{ return m_nProfileIndex; }
	cchar *GetSrcPath( u32 nIndex )			{ FASSERT( nIndex < SETTINGS_MAX_PROFILES ); return (cchar *)m_asSrcPath[nIndex]; }
	cchar *GetDestFilename( u32 nIndex )	{ FASSERT( nIndex < SETTINGS_MAX_PROFILES ); return (cchar *)m_asDestFilename[nIndex]; }
	cchar *GetIgnoreExtList( u32 nIndex )	{ FASSERT( nIndex < SETTINGS_MAX_PROFILES ); return (cchar *)m_asIgnoreExtList[nIndex]; }
	BOOL GetRecurseSubDirs( u32 nIndex )	{ FASSERT( nIndex < SETTINGS_MAX_PROFILES ); return m_abRecurseSubDirs[nIndex]; }
	CTime GetDate( u32 nIndex )				{ FASSERT( nIndex < SETTINGS_MAX_PROFILES ); return m_aDate[nIndex]; }
	u32 GetDiskSpanIndex( u32 nIndex )		{ FASSERT( nIndex < SETTINGS_MAX_PROFILES ); return m_anDiskSpanIndex[nIndex]; }
	BOOL GetClickNGo()						{ return m_bClickNGo; }

	// helper functions to read/write custom data items
	void WriteCustomInt( cchar *pszSectionName, cchar *pszItemName, u32 nValue );
	void WriteCustomString( cchar *pszSectionName, cchar *pszItemName, cchar *pszString );
	void WriteCustomFloat( cchar *pszSectionName, cchar *pszItemName, f32 fValue );
	u32 ReadCustomInt( cchar *pszSectionName, cchar *pszItemName, u32 nDefault );
	void ReadCustomString( cchar *pszSectionName, cchar *pszItemName, cchar *pszDefault,
						   char *pszDest, u32 nSizeOfDest );
	f32 ReadCustomFloat( cchar *pszSectionName, cchar *pszItemName, f32 fDefault );

	void SaveCommonDataOutToFile();
	void GetCommonDataFromFile();
	
private:	
	BOOL m_bReadCommonDataFromFile;// do we need to read the settings file for common data items

	// common data items, custom application data will not have members, but will be read from 
	// the settings file everytime it is requested
	//		The following members are part of the CONFIG section
	u32 m_nProfileIndex;
	CString m_asSrcPath[SETTINGS_MAX_PROFILES];
	CString m_asDestFilename[SETTINGS_MAX_PROFILES];
	CString m_asIgnoreExtList[SETTINGS_MAX_PROFILES];
	BOOL m_abRecurseSubDirs[SETTINGS_MAX_PROFILES];
	CTime m_aDate[SETTINGS_MAX_PROFILES];
	u32 m_anDiskSpanIndex[SETTINGS_MAX_PROFILES];
	BOOL m_bClickNGo;
		
	friend class CSrcTreeArchiverDlg;
};

#endif
