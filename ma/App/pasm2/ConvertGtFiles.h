//////////////////////////////////////////////////////////////////////////////////////
// ConvertGtFiles.h - 
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2002
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 02/25/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _CONVERT_GT_FILES_H_
#define _CONVERT_GT_FILES_H_ 1

#include "fang.h"
#include "ConvertFile.h"


class CConvertGtFile : public CConvertFile
{
public:
	CConvertGtFile();
	~CConvertGtFile();

	virtual BOOL ConvertFile( const CFileInfo *pFileInfo, BOOL bConvertToBigEndian=FALSE );
	virtual u32 GetSizeOfConvertedFile();
	virtual BOOL WriteConvertedFile( cchar *pszFilename, FILE *pFileStream=NULL );
	virtual void FreeData();

private:

	unsigned long m_uGraphHandle;  //not u32 on purpose
protected:
	
};


#endif

