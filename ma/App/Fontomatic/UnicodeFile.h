//////////////////////////////////////////////////////////////////////////////////////
// UnicodeFile.h - VERY SIMPLE class that reads and writes Unicode Files 
//
// Author: Russell A. Foushee   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 02/07/03 Foushee     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _UNICODEFILE_H_
#define _UNICODEFILE_H_ 1

enum UFOpenMode {
	UNICODEFILE_OPENMODE_READ,
	UNICODEFILE_OPENMODE_WRITE,
};

class CUnicodeFile {
public:
	CUnicodeFile();
	~CUnicodeFile();

	BOOL Open( LPCTSTR pszFilename, UFOpenMode dwMode );
	void Close( void );

	BOOL ReadString( LPWSTR pString, DWORD dwMaxChars );
	BOOL ReadString( CStringW &rString );
	BOOL WriteString( LPCWSTR pString );

	LPCTSTR GetErrorString();

private:
	FILE* m_pFile;
	CString m_sError;
};

#endif
