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

class CPasmDlg;

class CSettings
{
public:
	CSettings();// don't call this, use GetCurrent(), we only 1 instance of this 
				// class in the application, and is declared locally to this .cpp file
	~CSettings();
	
	static void SetSettingsFilename( cchar *pszSettingsFilenameOnly );
    static CSettings& GetCurrent();

	// helper functions to return common data items
	cchar *GetXBoxPath()				{ return (cchar *)m_sXBoxPath; }
	cchar *GetPS2Path()					{ return (cchar *)m_sPS2Path; }
	cchar *GetPCPath()					{ return (cchar *)m_sPCPath; }
	BOOL GetXBoxFormatOn()				{ return m_bXBoxFormat; }
	BOOL GetPS2FormatOn()				{ return m_bPS2Format; }
	BOOL GetPCFormatOn()				{ return m_bPCFormat; }
	BOOL GetGCFormatOn()				{ return m_bGCFormat; }
	cchar *GetLastInputPath()			{ return (cchar *)m_sLastInputPath; }
	cchar *GetLastOutputName()			{ return (cchar *)m_sLastOutputName; }
	f32 GetDilter()						{ return m_fDither; }
	u32 GetAltColors()					{ return m_nAltColors; }
	u32 GetLeafNodeSize()				{ return m_nLeafNodeSize; }
	cchar *GetLogFilename()				{ return (cchar *)m_sLogFilename; }
	BOOL GetAutoLeafSizeOn()			{ return m_bAutoLeafSize; }
										
	BOOL GetShowDifferences()			{ return m_bShowDifferences; }
	BOOL GetMustRunConfig()				{ return m_bMustRunConfig; }
	cchar *GetMasterFileDir()			{ return (cchar *)m_sMasterFileDir; }
	cchar *GetLogDir()					{ return (cchar *)m_sLogDir; }
	cchar *GetExtractDir()				{ return (cchar *)m_sExtractDir; }
	cchar *GetConfigDir()				{ return (cchar *)m_sConfigDir; }
	cchar *GetIgnoreExtString()			{ return (cchar *)m_sIgnoreExtensions; }
	cchar *GetConfigFilename()			{ return (cchar *)m_sConfigFilename; }
	u32 GetSortColumn()					{ return m_nSortColumn; }
	BOOL GetAscending()					{ return m_bAscendingSort; }
	BOOL GetHideNonCompliableFiles()	{ return m_bHideNonCompliableFiles; }
	BOOL GetHoldCompileStats()			{ return m_bHoldCompileStats; }
	BOOL GetDisableLibrarySupport()		{ return m_bDisableLibrarySupport; }
	cchar *GetGCMasterFileDir()			{ return (cchar *)m_sGCMasterFileDir; }
	cchar *GetXBMasterFileDir()			{ return (cchar *)m_sXBMasterFileDir; }
#ifdef _MMI_TARGET_PS2
    cchar *GetPS2MasterFileDir()         { return (cchar *)m_sPS2MasterFileDir; }
#endif
	// !!Nate
	cchar *GetXClientFileDir()          { return (cchar *)m_sXClientFileDir; }
	BOOL GetXClientAutoRun()            { return m_bAutoRunXClient; }

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
	CString m_sXBoxPath;
	CString m_sPS2Path;
	CString m_sPCPath;
	BOOL m_bXBoxFormat;
	BOOL m_bPCFormat;
	BOOL m_bPS2Format;
	BOOL m_bGCFormat;
	CString m_sLastInputPath;
	CString m_sLastOutputName;
	f32 m_fDither;
	u32 m_nAltColors;
	u32 m_nLeafNodeSize;
	CString m_sLogFilename;
	BOOL m_bAutoLeafSize;

	//		The following members are part of the PASM2 section
	BOOL m_bShowDifferences;	
	BOOL m_bMustRunConfig;
	CString	m_sMasterFileDir;
	CString	m_sLogDir;
	CString	m_sExtractDir;
	CString	m_sConfigDir;
	CString	m_sIgnoreExtensions;
	CString m_sConfigFilename;
	u32 m_nSortColumn;
	BOOL m_bAscendingSort;
	BOOL m_bHideNonCompliableFiles;
	BOOL m_bHoldCompileStats;
	BOOL m_bDisableLibrarySupport;
	CString m_sGCMasterFileDir;
	CString m_sXBMasterFileDir;
#ifdef _MMI_TARGET_PS2
    CString m_sPS2MasterFileDir;
#endif
	// !!Nate
	CString m_sXClientFileDir;
	BOOL m_bAutoRunXClient;

	friend class CPasmDlg;
};

#endif
