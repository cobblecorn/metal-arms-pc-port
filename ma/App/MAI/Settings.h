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

class CK9Dlg;
class CVidMode;

class CSettings
{
public:
	CSettings();// don't call this, use GetCurrent(), we only 1 instance of this 
				// class in the application, and is declared locally to this .cpp file
	~CSettings();
	
	static void SetSettingsFilename( cchar *pszSettingsFilenameOnly );
	static void SetApplicationName( cchar *pszAppName );
	static cchar *GetApplicationName();
    static CSettings& GetCurrent(void);

	// helper functions to return common data items
	cchar *GetDisplayListPath()	{ return (cchar *)m_sDisplayListPath; }
	cchar *GetArtPath()			{ return (cchar *)m_sArtPath; }
	cchar *GetLastFile()		{ return (cchar *)m_sLastFile; }
	u32 GetRadius()				{ return m_nRadius; }
	f32 Get3dBGRed()			{ return m_f3dBGRed; }
	f32 Get3dBGGreen()			{ return m_f3dBGGreen; }
	f32 Get3dBGBlue()			{ return m_f3dBGBlue; }
	BOOL GetDrawPerfStats()		{ return m_bDrawPerfStats; }
	BOOL GetDrawFPS()			{ return m_bDrawFPS; }
	BOOL GetDrawCamInfo()		{ return m_bDrawCamInfo; }
	BOOL GetDrawLeafInfo()		{ return m_bDrawLeafInfo; }
	cchar *GetScreenshotPath()	{ return (cchar *)m_sScreenshotPath; }
	f32 GetBookmarkCamX()		{ return m_fBookmarkCamX; }
	f32 GetBookmarkCamY()		{ return m_fBookmarkCamY; }
	f32 GetBookmarkCamZ()		{ return m_fBookmarkCamZ; }
	f32 GetBookmarkCamZoom()	{ return m_fBookmarkCamZoom; }
	f32 GetBookmarkCamHeading()	{ return m_fBookmarkCamHeading; }
	f32 GetBookmarkCamPitch()	{ return m_fBookmarkCamPitch; }
	BOOL GetBookmarkValid()		{ return m_bBookmarkValid; }
	BOOL GetDrawCursor()		{ return m_bDrawCursor; }

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
	void GetCommonDataFromFile(cchar* pszLevelName);
	
private:	
	// common data items, custom application data will not have members, but will be read from 
	// the settings file everytime it is requested
	//		The following members are part of the CONFIG section
	CString m_sDisplayListPath;
	CString m_sArtPath;
	CString m_sLastFile;
	u32 m_nRadius;
	f32 m_f3dBGRed;
	f32 m_f3dBGGreen;
	f32 m_f3dBGBlue;
	BOOL m_bDrawPerfStats;
	BOOL m_bDrawFPS;
	BOOL m_bDrawCamInfo;
	BOOL m_bDrawLeafInfo;
	CString m_sScreenshotPath;
	f32 m_fBookmarkCamX;
	f32 m_fBookmarkCamY;
	f32 m_fBookmarkCamZ;
	f32 m_fBookmarkCamZoom;
	f32 m_fBookmarkCamHeading;
	f32 m_fBookmarkCamPitch;
	BOOL m_bBookmarkValid;
	BOOL m_bDrawCursor;

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

	friend class CK9Dlg;
	friend class CVidMode;
};

#endif



	
