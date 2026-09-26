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

class CWavCompileDlg;

class CSettings
{
public:
	CSettings();// don't call this, use GetCurrent(), we only 1 instance of this 
				// class in the application, and is declared locally to this .cpp file
	~CSettings();
	
	static void SetSettingsFilename( cchar *pszSettingsFilenameOnly );
	static void SetApplicationName( cchar *pszAppName );
	static cchar *GetApplicationName();
    static CSettings& GetCurrent();

	// helper functions to return common data items
	cchar *GetXBDir()			{ return (cchar *)m_sXBDir; }
	cchar *GetGCDir()			{ return (cchar *)m_sGCDir; }
	u32 GetFileSortColumn()		{ return m_nFileSortColumn; }
	BOOL GetFileAscendingSort()	{ return m_bFileAscendingSort; }
	
	
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
	CString m_sXBDir;
	CString m_sGCDir;
	u32 m_nFileSortColumn;
	BOOL m_bFileAscendingSort;

	friend class CWavCompileDlg;	
};

#endif



	
