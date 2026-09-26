//////////////////////////////////////////////////////////////////////////////////////
// ConfigFile.h - 
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
#ifndef _CONFIG_FILE_H_
#define _CONFIG_FILE_H_ 1

#include "fang.h"

class CConfigFile
{
public:
	CConfigFile();
	~CConfigFile();
	
	BOOL Read( cchar *pszFilename );
	BOOL Write( cchar *pszFilename );

	CString	m_sMasterFilename;
	CString	m_sLibDir;
	CString	m_sLocalDir;
	CString m_sLibraryLockDir;
	
private:
	enum { TEMP_STRING_LEN = 256 };

	char m_sString[TEMP_STRING_LEN];// used for reading and writting from the file
		
protected:

	
};


#endif

