//////////////////////////////////////////////////////////////////////////////////////
// ConvertSfbFiles.h - 
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
// 06/20/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _CONVERT_SFB_FILES_H_
#define _CONVERT_SFB_FILES_H_ 1

#include "fang.h"
#include "ConvertFile.h"
#include "fdata.h"

class CConvertSfbFile : public CConvertFile
{
public:
	CConvertSfbFile();
	~CConvertSfbFile();

	virtual BOOL ConvertFile( const CFileInfo *pFileInfo, BOOL bConvertToBigEndian=FALSE );
	virtual u32  GetDataCRC();
	virtual u32  GetSizeOfConvertedFile();
	virtual BOOL WriteConvertedFile( cchar *pszFilename, FILE *pFileStream=NULL );
	virtual void FreeData();

	u32 m_nNumCmds;
	u32 m_nNumSfxs;
	CString m_sBasedOnWavBank;

private:
	u32 m_nBytesAllocated;
	u8 *m_pMemAllocated;
	u8 *m_pCompiledFile;
	u32 m_nCompiledBytes;

	void ConvertPtrsToOffsets( FData_SFxBank_Header_t *pHeader, BOOL bConvertToBigEndian );
	
protected:
	
};






#endif

