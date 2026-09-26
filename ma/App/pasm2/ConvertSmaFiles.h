//////////////////////////////////////////////////////////////////////////////////////
// ConvertSmaFiles.h - "loads a .gam text file and compiles a binary .gam file"
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
// 10/25/01 Link     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _CONVERT_SMA_FILES_H_
#define _CONVERT_SMA_FILES_H_ 1

#include "fang.h"
#include "ConvertFile.h"


class CConvertSmaFile : public CConvertFile
{
public:
	CConvertSmaFile();
	~CConvertSmaFile();

	virtual BOOL ConvertFile( const CFileInfo *pFileInfo, BOOL bConvertToBigEndian=FALSE );
	virtual u32  GetDataCRC();
	virtual u32  GetSizeOfConvertedFile();
	virtual BOOL WriteConvertedFile( cchar *pszFilename, FILE *pFileStream=NULL );
	virtual void FreeData();

private:
	u8 *m_pCompiledFile;
	u32 m_nCompiledBytes;
	
protected:
	
};





#endif

