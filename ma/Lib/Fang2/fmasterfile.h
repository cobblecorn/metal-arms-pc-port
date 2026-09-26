//////////////////////////////////////////////////////////////////////////////////////
// fmasterfile.cpp - Master file loader and updater
//
// Author: Nathan Miller
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
// 09/12/02 Miller		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FMASTERFILE_H_
#define _FMASTERFILE_H_ 1

#include "fang.h"
#include "fdata.h"
#include "ffile.h"

class CFMasterFile
{
public:
	CFMasterFile();
	virtual ~CFMasterFile();

	BOOL IsFileLoaded(void) {return m_bIsLoaded;}

	const u32 &GetNumFileEntries(void) const						{return m_FileHeader.nNumEntries;}
	const u32 &GetFileSize(void)       const                        {return m_FileHeader.nBytesInFile;}
	const FDataPrjFile_Entry_t *GetFileEntry(const u32 &uNdx) const 
	{
		return uNdx >= m_FileHeader.nNumEntries ? 0 : &m_pFileEntries[uNdx];
	}
	const FDataPrjFile_Entry_t *FindFileEntry(cchar *pszName);

	u32 GetTgaVersionFromHeader(void) {return m_FileHeader.nTgaCompilerVersion;}
	u32 GetApeVersionFromHeader(void) {return m_FileHeader.nApeCompilerVersion;}
	u32 GetMtxVersionFromHeader(void) {return m_FileHeader.nMtxCompilerVersion;}
	u32 GetCsvVersionFromHeader(void) {return m_FileHeader.nCsvCompilerVersion;}
	u32 GetFntVersionFromHeader(void) {return m_FileHeader.nFntCompilerVersion;}
	u32 GetSmaVersionFromHeader(void) {return m_FileHeader.nSmaCompilerVersion;}
	u32 GetGtVersionFromHeader(void)  {return m_FileHeader.nGtCompilerVersion;}
	u32 GetWvbVersionFromHeader(void) {return m_FileHeader.nWvbCompilerVersion;}
	u32 GetFprVersionFromHeader(void) {return m_FileHeader.nFprCompilerVersion;}

	void Reset(void);
	BOOL Load(cchar *pszFileName);
	void Close(void);

	u8 *LoadFile(const FDataPrjFile_Entry_t *entry);
	BOOL UpdateFile(cchar *pszFileName, cchar *pszSrcFile, const u32 &uModTime, const u32 &uCRC);

private:
	typedef struct _MasterHole_t_
	{
		u32 nStartOffset;
		u32 nNumBytes;
		_MasterHole_t_ *pPrev, *pNext;
	} _MasterHole_t;

	BOOL _ErrorReturn(void);
	BOOL _FileReadEntries(void);
	BOOL _FileOpenGetHeader(cchar *pszFileName);
	BOOL _FileIsValid(void);

	BOOL _AddNewEntry(cchar *pszEntryName, const u32 &uFileSize, u8 *puFileData, const u32 &uModTime, const u32 &uCRC);
	BOOL _WipeAppendFile(const FDataPrjFile_Entry_t *pEntry, const u32 &uFileSize, u8 *puSrcData, const u32 &uModTime, const u32 &uCRC);
	BOOL _UpdateEntry(const FDataPrjFile_Entry_t *pEntry, const u32 &uFileSize, u8 *puSrcData, const u32 &uModTime, const u32 &uCRC);
	BOOL _ZeroEntryInFile(const FDataPrjFile_Entry_t *pEntry);
	u32 _ZeroUntilBoundary(const u32 &uSize);

	u8 *_LoadDiskFile(const char *pszFileName, u32 &uSize);

	BOOL _HoleListBuild(void);
	BOOL _HoleListFindHole(const u32 &uSize, u32 &uOffset);

	BOOL _HoleListAddHole(const u32 &uStart, const u32 &uSize);
	BOOL _HoleListCheckHoleAfter(const _MasterHole_t *pCurr, const u32 &uStart, const u32 &uSize);
	BOOL _HoleListCheckHoleBefore(const _MasterHole_t *pCurr, const u32 &uStart, const u32 &uSize);

	BOOL m_bIsLoaded;

	FFileHandle m_hFileRead;
	FFileHandle m_hFileWrite;
	FDataPrjFile_Header_t m_FileHeader;
	FDataPrjFile_Entry_t *m_pFileEntries;

	_MasterHole_t *m_pMasterHoles;
};

#endif