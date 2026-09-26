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

class CFontomaticDlg;
class CVidMode;

class CSettings
{
public:
	CSettings();// don't call this, use GetCurrent(), we only 1 instance of this 
				// class in the application, and is declared locally to this .cpp file
	~CSettings();
	
	static void SetSettingsFilename( LPCTSTR pszSettingsFilenameOnly );
	static void SetApplicationName( LPCTSTR pszAppName );
	static LPCTSTR GetApplicationName();
    static CSettings& GetCurrent();

	// helper functions to return common data items
	LPCTSTR GetInputFile()		{ return m_sInputFile; }
	LPCTSTR GetOutputDir()		{ return m_sOutputDir; }
	LPCTSTR GetCharSet()		{ return m_sCharSet; }
	LPCTSTR GetMasterFile()		{ return m_sMasterFile; }
	LPCTSTR GetDevName()		{ return m_sDevName; }
	f32 GetLetterSpacing()		{ return m_fLetterSpacing; }
	f32 GetWordSpacing()		{ return m_fWordSpacing; }
	f32 GetLineSpacing()		{ return m_fLineSpacing; }
	f32 GetNumberOfLines()		{ return m_fNumberOfLines; }
	f32 GetItalicSlant()		{ return m_fItalicSlant; }
	u32 GetDevFlags()			{ return m_nDevFlags; }
	u32 GetDevOrdinal()			{ return m_nDevOrdinal; }
	u32 GetDevRenderer()		{ return m_nDevRenderer; }
	u32 GetModeFlags()			{ return m_nModeFlags; }
	u32 GetModeColorBits()		{ return m_nModeColorBits; }
	u32 GetModeDepthBits()		{ return m_nModeDepthBits; }
	u32 GetModeStencilBits()	{ return m_nModeStencilBits; }
	u32 GetModePixelsAcross()	{ return m_nModePixelsAcross; }
	u32 GetModePixelsDown()		{ return m_nModePixelsDown; }
	u32 GetSwapInterval()		{ return m_nSwapInterval; }
	u32 GetOneToOne()			{ return m_bOneToOne; }
	u32 GetDrawGuides()			{ return m_bDrawGuides; }
	u32 GetFixedWidthFont()		{ return m_bFixedWidthFont; }
	u32 GetFirstRun()			{ return m_bFirstRun; }
	u32 GetItalicFont()			{ return m_bItalicFont; }
	f32 GetUnitFSAA()			{ return m_fUnitFSAA; }
	
	// helper functions to read/write custom data items
	void WriteCustomInt( LPCTSTR pszSectionName, LPCTSTR pszItemName, u32 nValue );
	void WriteCustomString( LPCTSTR pszSectionName, LPCTSTR pszItemName, LPCTSTR pszString );
	void WriteCustomFloat( LPCTSTR pszSectionName, LPCTSTR pszItemName, f32 fValue );
	u32 ReadCustomInt( LPCTSTR pszSectionName, LPCTSTR pszItemName, u32 nDefault );
	void ReadCustomString( LPCTSTR pszSectionName, LPCTSTR pszItemName, LPCTSTR pszDefault,
						   TCHAR *pszDest, u32 nSizeOfDest );
	f32 ReadCustomFloat( LPCTSTR pszSectionName, LPCTSTR pszItemName, f32 fDefault );

	void SaveCommonDataOutToFile();
	void GetCommonDataFromFile();
	
private:	
	BOOL m_bReadCommonDataFromFile;// do we need to read the settings file for common data items

	// common data items, custom application data will not have members, but will be read from 
	// the settings file everytime it is requested
	//		The following members are part of the CONFIG section
	CString m_sInputFile, m_sOutputDir, m_sCharSet, m_sMasterFile;
	BOOL m_bFirstRun, m_bOneToOne, m_bDrawGuides, m_bFixedWidthFont, m_bItalicFont;
	f32 m_fLetterSpacing, m_fWordSpacing, m_fLineSpacing, m_fNumberOfLines, m_fItalicSlant;

	//		The following members are part of the VIDEO section
	CString m_sDevName;
	u32 m_nDevFlags, m_nDevOrdinal, m_nDevRenderer, m_nModeFlags, m_nModeColorBits, m_nModeDepthBits, m_nModeStencilBits, m_nModePixelsAcross, m_nModePixelsDown, m_nSwapInterval;
	f32 m_fUnitFSAA;

	friend class CFontomaticDlg;
	friend class CVidMode;
};

#endif
