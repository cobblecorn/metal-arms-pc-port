//////////////////////////////////////////////////////////////////////////////////////
// ConvertFile.h - abstract base class for any fang file converter
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
// 07/25/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _CONVERTFILE_H_
#define _CONVERTFILE_H_ 1

#include "fang.h"
#include "FileInfo.h"

// abstract base class for fang file converters
class CConvertFile
{
public:
	CConvertFile() {};
	~CConvertFile() {};

	virtual BOOL ConvertFile( const CFileInfo *pFileInfo, BOOL bConvertToBigEndian=FALSE ) = 0;
	virtual u32 GetSizeOfConvertedFile() = 0;
	virtual BOOL WriteConvertedFile( cchar *pszFilename, FILE *pFileStream=NULL ) = 0;
	virtual void FreeData() = 0;
		
private:
	
protected:
	
};


#endif

