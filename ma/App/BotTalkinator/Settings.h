//////////////////////////////////////////////////////////////////////////////////////
// Settings.h - "manages reading/writting/store data from the .ini file"
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS.
// Copyright (c) 1999
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 12/14/99 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _SETTINGS_H_
#define _SETTINGS_H_ 1

#include "fang.h"

class CBotTalkinatorDlg;
class CVidMode;
class COptionsWnd;

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
	cchar *GetOutputDir()		{ return (cchar *)m_sOutputDir; }
	cchar *GetMasterFile()		{ return (cchar *)m_sMasterFile; }
	cchar *GetInputDir()		{ return (cchar *)m_sInputDir; }
	cchar *GetLastSelectedBot()	{ return (cchar *)m_sLastSelectedBot; }
	cchar *GetLastSelectedTitle() { return (cchar *)m_sLastSelectedTitle; }
	
	cchar *GetDevName()			{ return (cchar *)m_sDevName; }
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
	f32 GetUnitFSAA()			{ return m_fUnitFSAA; }
	
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
	CString m_sOutputDir;
	CString m_sMasterFile;
	CString m_sInputDir;
	CString m_sLastSelectedBot;
	CString m_sLastSelectedTitle;

	//		The following members are part of the VIDEO section
	CString m_sDevName;
	u32 m_nDevFlags;
	u32 m_nDevOrdinal;
	u32 m_nDevRenderer;
	u32 m_nModeFlags;
	u32 m_nModeColorBits;
	u32 m_nModeDepthBits;
	u32 m_nModeStencilBits;
	u32 m_nModePixelsAcross;
	u32 m_nModePixelsDown;
	u32 m_nSwapInterval;
	f32 m_fUnitFSAA;

	friend class CBotTalkinatorDlg;
	friend class CVidMode;
	friend class COptionsWnd;
};

#endif
