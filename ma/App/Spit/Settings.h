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
#include "fmath.h"

class CSpitDlg;
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
    static CSettings& GetCurrent();

	// helper functions to return common data items
	cchar *GetLastFileWorkedOn()		{ return (cchar *)m_sLastFileWorkedOn; }
	cchar *GetMasterFileLoc()			{ return (cchar *)m_sMasterFileLocation; }
	void GetCameraPos( CFVec3 &Pos )	{ Pos.Set( m_fCameraX, m_fCameraY, m_fCameraZ ); }
	f32 GetCameraHeading()				{ return m_fCameraHeading; }
	f32 GetCameraPitch()				{ return m_fCameraPitch; }
	u32 GetEmitY()						{ return m_nEmitY; }
	u32 GetEmitRot()					{ return m_nEmitRot; }
	BOOL GetAmbientOn()					{ return m_bAmbientOn; }
	f32 GetAmbientRed()					{ return m_fAmbientRed; }
	f32 GetAmbientGreen()				{ return m_fAmbientGreen; }
	f32 GetAmbientBlue()				{ return m_fAmbientBlue; }
	
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
	CString m_sLastFileWorkedOn;
	CString m_sMasterFileLocation;
	f32 m_fCameraX;
	f32 m_fCameraY;
	f32 m_fCameraZ;
	f32 m_fCameraHeading;
	f32 m_fCameraPitch;
	u32 m_nEmitY;
	u32 m_nEmitRot;
	BOOL m_bAmbientOn;
	f32 m_fAmbientRed;
	f32 m_fAmbientGreen;
	f32 m_fAmbientBlue;
	
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
		
	friend class CSpitDlg;	
	friend class CVidMode;
};

#endif



	
