//////////////////////////////////////////////////////////////////////////////////////
// FileLock.h - 
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
// 11/21/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FILELOCK_H_
#define _FILELOCK_H_ 1

#include "fang.h"

class CFileLock  
{
public:
	CFileLock( cchar *pszFilename=NULL );
	~CFileLock();

	BOOL SetFilename( cchar *pszFilename );
	BOOL LockFile();
	BOOL WaitToLockFile( CWnd* pParent );
	BOOL IsFileLocked();
	BOOL WhoHasFileLocked( CString &rsName );
	void UnlockFile();

protected:
	
private:
	BOOL m_bFileLocked;
	CFile m_File;
	CString m_sFilename;
	CString m_sComputerName;

};

#endif
