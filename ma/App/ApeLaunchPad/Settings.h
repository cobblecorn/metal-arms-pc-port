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

class CApeLaunchPadDlg;

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
	cchar *GetPasmLoc()			{ return (cchar *)m_sPasmLoc; }
	cchar *GetJawsLoc()			{ return (cchar *)m_sJawsLoc; }
	cchar *GetFontomaticLoc()	{ return (cchar *)m_sFontomaticLoc; }
	cchar *GetK9Loc()			{ return (cchar *)m_sK9Loc; }
	cchar *GetCopyGoLoc()		{ return (cchar *)m_sCopyGoLoc; }
	cchar *GetMawinLoc()		{ return (cchar *)m_sMawinLoc; }
		
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
	CString m_sPasmLoc;
	CString m_sJawsLoc;
	CString m_sFontomaticLoc;
	CString m_sK9Loc;
	CString m_sCopyGoLoc;
	CString m_sMawinLoc;

	friend class CApeLaunchPadDlg;
};

#endif
