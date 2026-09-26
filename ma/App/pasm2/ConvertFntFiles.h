//////////////////////////////////////////////////////////////////////////////////////
// ConvertFntFiles.h
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
// 10/10/01 ayale       Modified.
// 10/10/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef _CONVERT_FNT_FILES_H_
#define _CONVERT_FNT_FILES_H_ 1

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "fang.h"
#include "fdata.h"
#include "ConvertFile.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _Pasm_Font_t
{
	FDataFntFile_Font_t oFntInfo;

	FDataFntFile_LetterBucket_t aoBuckets[ FDATA_FNT_MAXBUCKETS_IN_CHAR_SET ];

	FDataFntFile_Letter_t *paoFntLetters;

	u32 *pauWidth, *pauHeight;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class CConvertFntFile : public CConvertFile
{
public:
	CConvertFntFile();
	~CConvertFntFile();

	virtual BOOL ConvertFile( const CFileInfo *pFileInfo, BOOL bConvertToBigEndian=FALSE );
	virtual u32 GetSizeOfConvertedFile();
	virtual BOOL WriteConvertedFile( cchar *pszFilename, FILE *pFileStream=NULL );
	virtual void FreeData();

private:
	_Pasm_Font_t m_oPasmFont;
	CPtrList m_BucketLetterList;

	BOOL m_bConvertToBigEndian;

	//PASM .FNT variables
	u8 *m_pCompiledFile;
	u32 m_nCompiledBytes;

	FDataFntFile_BucketLetter_t* FindBucketLetter( u16 uCharCode );

}; // CConvertFntFile

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
