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

class CMa_winDlg;
class CVidMode;

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
	cchar *GetArtPath()			{ return (cchar *)m_sArtPath; }
	cchar *GetLastFile()		{ return (cchar *)m_sLastFile; }
	f32 Get3dBGRed()			{ return m_f3dBGRed; }
	f32 Get3dBGGreen()			{ return m_f3dBGGreen; }
	f32 Get3dBGBlue()			{ return m_f3dBGBlue; }
	cchar *GetScreenshotPath()	{ return (cchar *)m_sScreenshotPath; }
	BOOL GetLevelSelect()		{ return m_bSkipLevelSelect; }
	BOOL GetXbox()				{ return m_bXbox; }
	BOOL GetGameCube()			{ return m_bGameCube; }
	BOOL GetMemSlot1()			{ return m_bMemSlot1; }
	BOOL GetMemSlot2()			{ return m_bMemSlot2; }
	BOOL GetShowFPS()			{ return m_bShowFPS; }
	cchar *GetMemCardPath()		{ return (cchar *)m_sMemCardPath; }
	BOOL GetInstallAudio()		{ return m_bInstallAudio; }

	// VIDEO
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

	// INPUT EMULATION
	cchar *GetInputEmulationDevName()	{ return (cchar *)m_sInputEmulationDevName; }
	u32 *GetInputEmulationMap()			{ return m_auInputEmulationMap; }

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
	CString m_sArtPath;
	CString m_sLastFile;
	f32 m_f3dBGRed;
	f32 m_f3dBGGreen;
	f32 m_f3dBGBlue;
	CString m_sScreenshotPath;
	BOOL m_bSkipLevelSelect;
	BOOL m_bXbox;
	BOOL m_bGameCube;
	BOOL m_bMemSlot1;
	BOOL m_bMemSlot2;
	BOOL m_bShowFPS;
	BOOL m_bInstallAudio;
	CString m_sMemCardPath;
	BOOL m_bDebugAI;
	BOOL m_bPlayerDeath;
	u32 m_nServoLegLevel;
	u32 m_nServoArmLevel;
	
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

	//		The following members are part of the INPUT EMULATION section
	CString m_sInputEmulationDevName;
	u32 m_auInputEmulationMap[ 36 ];

	friend class CMa_winDlg;
	friend class CVidMode;	
	friend class CInputEmulation;
};

#endif
