//////////////////////////////////////////////////////////////////////////////////////
// ConvertCsvFiles.h - 
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
// 04/15/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _CONVERT_CSV_FILES_H_
#define _CONVERT_CSV_FILES_H_ 1

#include "fang.h"
#include "ConvertFile.h"
#include "fdata.h"

class CConvertCsvFile : public CConvertFile
{
public:
	CConvertCsvFile();
	~CConvertCsvFile();

	virtual BOOL ConvertFile( const CFileInfo *pFileInfo, BOOL bConvertToBigEndian=FALSE );
	virtual u32  GetDataCRC();
	virtual u32  GetSizeOfConvertedFile();
	virtual BOOL WriteConvertedFile( cchar *pszFilename, FILE *pFileStream=NULL );
	virtual void FreeData();

	u32 m_nNumTables;
	u32 m_nNumFields;
	u32 m_nNumStringFields;
	u32 m_nNumWideStringFields;
	u32 m_nNumFloatFields;

	//Generic Versions used externally (by other PASM modules) 
	static BOOL IsCommentField( const CString &rsField );
	static BOOL DoesKeyStringContainInvalidChars( const CString &rsString, BOOL bAllowSpaces );
	static BOOL IsStringNumeric( const CString &rsString );
	static f32 ExtractFloatValue( const CString &rsString );

	static void ConvertPtrsToOffsets( FDataGamFile_Header_t *pHeader );
	static void ConvertEndian( FDataGamFile_Header_t *pHeader );

private:
	u32 m_nNumRows;
	u32 m_nNumCols;
	CArray<CStringW, CStringW &> *m_paRowData;// points to m_nNumRows of CStringArray objects
	u32 m_nBytesAllocated;
	u8 *m_pMemAllocated;
	u8 *m_pCompiledFile;
	u8 *m_pWorkingMem;
	u32 m_nCompiledBytes;
	CUIntArray m_anFieldsPerTable;

	//Widestring Versions used internally
	static BOOL IsStringWide( const CStringW &rsString );
	static BOOL IsCommentFieldW( const CStringW &rsField );
	static BOOL DoesKeyStringContainInvalidCharsW( const CStringW &rsString, BOOL bAllowSpaces );
	static BOOL IsStringNumericW( const CStringW &rsString );
	static f32 ExtractFloatValueW( const CStringW &rsString );

protected:
	
};





#endif

