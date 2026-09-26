//////////////////////////////////////////////////////////////////////////////////////
// ConvertWvbFile_XBox.h - 
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
// 06/07/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _CONVERT_WVB_FILE_XBOX_H_
#define _CONVERT_WVB_FILE_XBOX_H_ 1

#include "fang.h"
#include "ConvertFile.h"
#include "WavToADPCM_Xbox.h"
#include "fdata.h"

class CConvertWvbFile_XBox : public CConvertFile
{
public:
	CConvertWvbFile_XBox();
	~CConvertWvbFile_XBox();

	virtual BOOL ConvertFile( const CFileInfo *pFileInfo, BOOL bConvertToBigEndian=FALSE );
	virtual u32  GetDataCRC();
	virtual u32  GetSizeOfConvertedFile();
	virtual BOOL WriteConvertedFile( cchar *pszFilename, FILE *pFileStream=NULL );
	virtual void FreeData();

	u32 m_nNumWavs;
	u32 m_nLargestWav;
	u32 m_nSmallestWav;

private:
	u32 m_nBytesAllocated;
	u8 *m_pMemAllocated;				// m_nBytesAllocated bytes
	CWavToADPCM_XBox *m_paXboxWaves;	// m_nNumWavs of these

	FDataWvbFile_Bank_t *m_pBankHeader;
	FDataWvbFile_Wave_t *m_paWaves;
	
	u32 m_nCompiledBytes;
	u32 m_nNumPadBytes;

	
protected:
	
};

#endif

