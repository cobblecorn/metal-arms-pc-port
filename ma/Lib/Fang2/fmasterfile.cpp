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
#include "fang.h"
#include "fclib.h"
#include "fmasterfile.h"


static int _MQSortCompareFcn( const void *arg1, const void *arg2 )
{
	FDataPrjFile_Entry_t *p1, *p2;

	p1 = (FDataPrjFile_Entry_t *) arg1;
	p2 = (FDataPrjFile_Entry_t *) arg2;

   // Compare all of both strings:
   return fclib_stricmp(p1->szFilename, p2->szFilename);
}

static int _MIntQSortCompareFcn( const void *arg1, const void *arg2 )
{
	FDataPrjFile_Entry_t *p1, *p2;

	p1 = (FDataPrjFile_Entry_t *) arg1;
	p2 = (FDataPrjFile_Entry_t *) arg2;

	if (p1->nStartOffset < p2->nStartOffset)
		return -1;
	else if (p1->nStartOffset > p2->nStartOffset)
		return 1;

	return 0;
}


CFMasterFile::CFMasterFile()
{
	m_hFileRead = FFILE_INVALID_HANDLE;
	m_hFileWrite = FFILE_INVALID_HANDLE;
	m_pFileEntries = 0;

	m_pMasterHoles = 0;

	m_bIsLoaded = FALSE;
}

CFMasterFile::~CFMasterFile()
{
	Reset();
}

const FDataPrjFile_Entry_t *CFMasterFile::FindFileEntry(cchar *pszName)
{
	FDataPrjFile_Entry_t tempEntry;

	fclib_strcpy(tempEntry.szFilename, (cchar *) pszName);
	return (FDataPrjFile_Entry_t *) bsearch(&tempEntry, m_pFileEntries, m_FileHeader.nNumEntries,
		sizeof(FDataPrjFile_Entry_t), _MQSortCompareFcn);
}

void CFMasterFile::Reset(void)
{
	Close();

	delete [] m_pFileEntries;
	m_pFileEntries = 0;

	_MasterHole_t *pCurr = m_pMasterHoles, *pPrev = 0;

	while (pCurr)
	{
		pPrev = pCurr;
		pCurr = pCurr->pNext;

		delete pPrev;
	}

	m_pMasterHoles = 0;
	m_bIsLoaded = FALSE;
}

BOOL CFMasterFile::Load(cchar *pszFileName)
{
	FASSERT(pszFileName);

	if (FFILE_IS_VALID_HANDLE(m_hFileRead) || FFILE_IS_VALID_HANDLE(m_hFileWrite) || m_pFileEntries)
	{
		Reset();
	}

	if (!_FileOpenGetHeader(pszFileName))
	{
		return FALSE;
	}

	if (!_FileIsValid())
	{
		return FALSE;
	}

	if (!_FileReadEntries())
	{
		return FALSE;
	}

	if (!_HoleListBuild())
	{
		return FALSE;
	}

	m_bIsLoaded = TRUE;

	return TRUE;
}

void CFMasterFile::Close(void)
{
	if (FFILE_IS_VALID_HANDLE(m_hFileRead))
	{
		ffile_Close(m_hFileRead);
		m_hFileRead = FFILE_INVALID_HANDLE;
	}

	if (FFILE_IS_VALID_HANDLE(m_hFileWrite))
	{
		ffile_Close(m_hFileWrite);
		m_hFileWrite = FFILE_INVALID_HANDLE;
	}
}

u8 *CFMasterFile::LoadFile(const FDataPrjFile_Entry_t *entry)
{
	FASSERT(entry);

	if (!FFILE_IS_VALID_HANDLE(m_hFileRead))
	{
		return 0;
	}

	u8 *puData = new u8[entry->nNumBytes];

	if (!puData)
	{
		return 0;
	}

	ffile_Seek(m_hFileRead, entry->nStartOffset, FFILE_SEEK_SET);
	ffile_Read(m_hFileRead, entry->nNumBytes, puData);

	return puData;
}

BOOL CFMasterFile::UpdateFile(cchar *pszFileName, cchar *pszSrcFile, const u32 &uModTime, const u32 &uCRC)
{
	if (!FFILE_IS_VALID_HANDLE(m_hFileWrite))
		return FALSE;

	const FDataPrjFile_Entry_t *pEntry = FindFileEntry(pszFileName);
	u32 uSrcFileSize = 0;
	u8 *puSrcFileData = _LoadDiskFile(pszSrcFile, uSrcFileSize);

	if (!puSrcFileData)
	{
		DEVPRINTF("CFMasterFile::Failed to load disk file \"%s\"\n", pszSrcFile);
		return FALSE;
	}

	if (!pEntry)
	{
		//DEVPRINTF("CFMasterFile::Add new entry %s\n", pszFileName);
		_AddNewEntry(pszFileName, uSrcFileSize, puSrcFileData, uModTime, uCRC);
	}
	else
	{
		u32 uDestFileSize = pEntry->nNumBytes;
		u32 uDestFileDiskSize = FMATH_BYTE_ALIGN_UP(uDestFileSize, FDATA_PRJFILE_FILE_ALIGNMENT);
		
		/*DEVPRINTF("src  size      %d\n", uSrcFileSize);
		DEVPRINTF("dest size      %d\n", uDestFileSize);
		DEVPRINTF("dest size disk %d\n", uDestFileDiskSize);*/

		// compare against disk size sice we have that much space to use
		if (uSrcFileSize <= uDestFileDiskSize)
		{
			//DEVPRINTF("CFMasterFile::Update entry %s\n", pszFileName);
			_UpdateEntry(pEntry, uSrcFileSize, puSrcFileData, uModTime, uCRC);
		}
		else if (uSrcFileSize > uDestFileDiskSize)
		{
			//DEVPRINTF("CFMasterFile::Wipe append entry %s\n", pszFileName);
			_WipeAppendFile(pEntry, uSrcFileSize, puSrcFileData, uModTime, uCRC);
		}
	}

	delete [] puSrcFileData;

	return TRUE;
}

BOOL CFMasterFile::_ErrorReturn(void)
{
	Close();
	return FALSE;
}

BOOL CFMasterFile::_FileReadEntries(void)
{
	m_pFileEntries = new FDataPrjFile_Entry_t[m_FileHeader.nNumEntries];

	if (!m_pFileEntries)
	{
		DEVPRINTF("memory error\n");
		return FALSE;
	}

	for (u32 i = 0; i < m_FileHeader.nNumEntries; ++i)
	{
		s32 sRes = ffile_Read(m_hFileRead, sizeof(FDataPrjFile_Entry_t), &m_pFileEntries[i]);

		if (sRes != sizeof(FDataPrjFile_Entry_t))
		{
			DEVPRINTF("CFMasterFile::Failed read on entries\n");
			return _ErrorReturn();
		}
	}

	return TRUE;
}

BOOL CFMasterFile::_FileOpenGetHeader(cchar *pszFileName)
{
	m_hFileRead = ffile_Open(pszFileName, FFILE_OPEN_RONLY, TRUE);

	if (!FFILE_IS_VALID_HANDLE(m_hFileRead))
	{
		DEVPRINTF("CFMasterFile::Failed to open %s for reading\n", pszFileName);
		return FALSE;
	}

	m_hFileWrite = ffile_Open(pszFileName, FFILE_OPEN_WONLY, TRUE);

	if (!FFILE_IS_VALID_HANDLE(m_hFileWrite))
	{
		DEVPRINTF("CFMasterFile::Failed to open %s for writing\n", pszFileName);
		return FALSE;
	}

	if (ffile_Read(m_hFileRead, sizeof(FDataPrjFile_Header_t), &m_FileHeader) != sizeof(FDataPrjFile_Header_t))
	{
		DEVPRINTF("CFMasterFile::ffile_Read() failed.\n");
		return _ErrorReturn();
	}

	return TRUE;
}

BOOL CFMasterFile::_FileIsValid(void)
{
	if (m_FileHeader.Version.nSignature != FVERSION_FILE_SIGNATURE)
	{
		DEVPRINTF("CFMasterFile::Version signatures do not match.\n");
		return _ErrorReturn();
	}

	if ((FVERSION_GET_MAJOR_NUMBER(m_FileHeader.Version.nVersion) != FDATA_PRJFILE_MAJOR) ||
		(FVERSION_GET_MINOR_NUMBER(m_FileHeader.Version.nVersion) != FDATA_PRJFILE_MINOR) ||
		(FVERSION_GET_SUB_NUMBER  (m_FileHeader.Version.nVersion) != FDATA_PRJFILE_SUB ))
	{
		DEVPRINTF("CFMasterFile::Version numbers do not match.\n");
		return _ErrorReturn();
	}

	ffile_Seek(m_hFileRead, 0, FFILE_SEEK_END);

	if (ffile_Tell(m_hFileRead) != (int) m_FileHeader.nBytesInFile)
	{
		DEVPRINTF("CFMasterFile::File sizes do not match. %d %d\n", ffile_Tell(m_hFileRead), (int) m_FileHeader.nBytesInFile);
		return _ErrorReturn();
	}
	
	ffile_Seek(m_hFileRead, sizeof(FDataPrjFile_Header_t), FFILE_SEEK_SET);

	return TRUE;
}

BOOL CFMasterFile::_AddNewEntry(cchar *pszEntryName, const u32 &uFileSize, u8 *puFileData, const u32 &uModTime, const u32 &uCRC)
{
	// This code works without the hole filling
	/*
	// nothing free
	if (m_FileHeader.nNumFreeEntries <= 0)
	{
		DEVPRINTF("CFMasterFile::No free entries in master file\n");
		return FALSE;
	}

	FDataPrjFile_Entry_t *pTempEntries = new FDataPrjFile_Entry_t[m_FileHeader.nNumEntries + 1];

	if (!pTempEntries)
	{
		DEVPRINTF("CFMasterFile::Memory error\n");
		return FALSE;
	}

	// Copy over existing entries
	fang_MemZero(pTempEntries, sizeof(FDataPrjFile_Entry_t) * (m_FileHeader.nNumEntries + 1));
	fang_MemCopy(pTempEntries, m_pFileEntries, sizeof(FDataPrjFile_Entry_t) * m_FileHeader.nNumEntries);

	s32 sRes;

	// The new entry
	FDataPrjFile_Entry_t *NewEntry = &pTempEntries[m_FileHeader.nNumEntries];

	NewEntry->nModifiedTime = uModTime;
	NewEntry->nNumBytes = uFileSize;
	NewEntry->nStartOffset = m_FileHeader.nBytesInFile;
	fclib_strcpy(NewEntry->szFilename, pszEntryName);

	// Sort the entries with the added one
	fclib_QSort(pTempEntries, m_FileHeader.nNumEntries + 1, sizeof(FDataPrjFile_Entry_t), _MQSortCompareFcn);

	// Write the new entries to file
	ffile_Seek(m_hFileWrite, sizeof(FDataPrjFile_Header_t), FFILE_SEEK_SET);
	sRes = ffile_Write(m_hFileWrite, sizeof(FDataPrjFile_Entry_t) * (m_FileHeader.nNumEntries + 1), pTempEntries);

	if ((u32) sRes != sizeof(FDataPrjFile_Entry_t) * (m_FileHeader.nNumEntries + 1))
	{
		DEVPRINTF("CFMasterFile::File write error\n");
		return FALSE;
	}

	// Write the new data to the file
	ffile_Seek(m_hFileWrite, m_FileHeader.nBytesInFile, FFILE_SEEK_SET);
	sRes = ffile_Write(m_hFileWrite, uFileSize, puFileData);

	if ((u32) sRes != uFileSize)
	{
		DEVPRINTF("CFMasterFile::File write error\n");
		return FALSE;
	}

	// Fill with zeros until byte boundary
	sRes = _ZeroUntilBoundary(uFileSize);

	// Write the updated header
	m_FileHeader.nNumEntries += 1;
	m_FileHeader.nNumFreeEntries -= 1;
	m_FileHeader.nBytesInFile += (uFileSize + sRes);

	ffile_Seek(m_hFileWrite, 0, FFILE_SEEK_SET);
	sRes = ffile_Write(m_hFileWrite, sizeof(FDataPrjFile_Header_t), &m_FileHeader);

	if (sRes != sizeof(FDataPrjFile_Header_t))
	{
		DEVPRINTF("CFMasterFile::File write error\n");
		return FALSE;
	}

	// We have a new entry set now, make use of it
	delete [] m_pFileEntries;
	m_pFileEntries = pTempEntries;

	return TRUE;*/
	// nothing free
	if (m_FileHeader.nNumFreeEntries <= 0)
	{
		DEVPRINTF("CFMasterFile::No free entries in master file\n");
		return FALSE;
	}

	FDataPrjFile_Entry_t *pTempEntries = new FDataPrjFile_Entry_t[m_FileHeader.nNumEntries + 1];

	if (!pTempEntries)
	{
		DEVPRINTF("CFMasterFile::Memory error\n");
		return FALSE;
	}

	// Copy over existing entries
	fang_MemZero(pTempEntries, sizeof(FDataPrjFile_Entry_t) * (m_FileHeader.nNumEntries + 1));
	fang_MemCopy(pTempEntries, m_pFileEntries, sizeof(FDataPrjFile_Entry_t) * m_FileHeader.nNumEntries);

	s32 sRes;
	u32 uHoleOffset = 0;
	BOOL bHole = _HoleListFindHole(uFileSize, uHoleOffset);

	if (bHole)
	{
		FASSERT(uHoleOffset);
	}

	// The new entry
	FDataPrjFile_Entry_t *NewEntry = &pTempEntries[m_FileHeader.nNumEntries];

	NewEntry->nModifiedTime = uModTime;
	NewEntry->nNumBytes = uFileSize;
	NewEntry->nStartOffset = bHole ? uHoleOffset : m_FileHeader.nBytesInFile;
	NewEntry->nCRC = uCRC;
	fclib_strcpy(NewEntry->szFilename, pszEntryName);

	// Sort the entries with the added one
	fclib_QSort(pTempEntries, m_FileHeader.nNumEntries + 1, sizeof(FDataPrjFile_Entry_t), _MQSortCompareFcn);

	// Write the new entries to file
	ffile_Seek(m_hFileWrite, sizeof(FDataPrjFile_Header_t), FFILE_SEEK_SET);
	sRes = ffile_Write(m_hFileWrite, sizeof(FDataPrjFile_Entry_t) * (m_FileHeader.nNumEntries + 1), pTempEntries);

	if ((u32) sRes != sizeof(FDataPrjFile_Entry_t) * (m_FileHeader.nNumEntries + 1))
	{
		DEVPRINTF("CFMasterFile::File write error\n");
		return FALSE;
	}

	// Write the new data to the file
	//ffile_Seek(m_hFileWrite, m_FileHeader.nBytesInFile, FFILE_SEEK_SET);
	ffile_Seek(m_hFileWrite, bHole ? uHoleOffset : m_FileHeader.nBytesInFile, FFILE_SEEK_SET);

	sRes = ffile_Write(m_hFileWrite, uFileSize, puFileData);

	if ((u32) sRes != uFileSize)
	{
		DEVPRINTF("CFMasterFile::File write error\n");
		return FALSE;
	}

	// Fill with zeros until byte boundary
	sRes = _ZeroUntilBoundary(uFileSize);

	// Write the updated header
	m_FileHeader.nNumEntries += 1;
	m_FileHeader.nNumFreeEntries -= 1;

	if (!bHole)
	{
		m_FileHeader.nBytesInFile += (uFileSize + sRes);
	}

	ffile_Seek(m_hFileWrite, 0, FFILE_SEEK_SET);
	sRes = ffile_Write(m_hFileWrite, sizeof(FDataPrjFile_Header_t), &m_FileHeader);

	if (sRes != sizeof(FDataPrjFile_Header_t))
	{
		DEVPRINTF("CFMasterFile::File write error\n");
		return FALSE;
	}

	// We have a new entry set now, make use of it
	delete [] m_pFileEntries;
	m_pFileEntries = pTempEntries;

	return TRUE;
}

BOOL CFMasterFile::_WipeAppendFile(const FDataPrjFile_Entry_t *pEntry, const u32 &uFileSize, u8 *puFileData, const u32 &uModTime, const u32 &uCRC)
{
	// This code works without the hole filling
	/*u32 uWriteIndex;

	for (uWriteIndex = 0; uWriteIndex < m_FileHeader.nNumEntries; ++uWriteIndex)
	{
		if (pEntry == (m_pFileEntries + uWriteIndex))
		{
			break;
		}
	}

	if (uWriteIndex >= m_FileHeader.nNumEntries)
	{
		DEVPRINTF("CFMasterFile::Passed file entry not found in master file!\n");
		return FALSE;
	}

	// Zero out the entry
	_ZeroEntryInFile(pEntry);

	// Update the entry to have a new position and size
	m_pFileEntries[uWriteIndex].nModifiedTime = uModTime;
	m_pFileEntries[uWriteIndex].nStartOffset = m_FileHeader.nBytesInFile;
	m_pFileEntries[uWriteIndex].nNumBytes = uFileSize;

	s32 sSeekOffset = sizeof(FDataPrjFile_Header_t) + (uWriteIndex * sizeof(FDataPrjFile_Entry_t));
	s32 sRes;

	// Write the header entry
	ffile_Seek(m_hFileWrite, sSeekOffset, FFILE_SEEK_SET);
	sRes = ffile_Write(m_hFileWrite, sizeof(FDataPrjFile_Entry_t), (void *) pEntry);

	if (sRes != sizeof(FDataPrjFile_Entry_t))
	{
		DEVPRINTF("CFMasterFile::File write error\n");
		return FALSE;
	}

	// Write the data for the file to the end of the file
	ffile_Seek(m_hFileWrite, m_FileHeader.nBytesInFile, FFILE_SEEK_SET);
	sRes = ffile_Write(m_hFileWrite, uFileSize, puFileData);

	if ((u32) sRes != uFileSize)
	{
		DEVPRINTF("CFMasterFile::File write error\n");
		return FALSE;
	}

	// Fill with zeros until byte boundary
	sRes = _ZeroUntilBoundary(uFileSize);

	// Update the number of bytes in the file
	m_FileHeader.nBytesInFile += (uFileSize + sRes);

	// Write the updated header
	ffile_Seek(m_hFileWrite, 0, FFILE_SEEK_SET);
	sRes = ffile_Write(m_hFileWrite, sizeof(FDataPrjFile_Header_t), &m_FileHeader);

	if (sRes != sizeof(FDataPrjFile_Header_t))
	{
		DEVPRINTF("CFMasterFile::File write error\n");
		return FALSE;
	}

	return TRUE;*/
	u32 uWriteIndex;

	for (uWriteIndex = 0; uWriteIndex < m_FileHeader.nNumEntries; ++uWriteIndex)
	{
		if (pEntry == (m_pFileEntries + uWriteIndex))
		{
			break;
		}
	}

	if (uWriteIndex >= m_FileHeader.nNumEntries)
	{
		DEVPRINTF("CFMasterFile::Passed file entry not found in master file!\n");
		return FALSE;
	}

	// Zero out the entry
	_ZeroEntryInFile(pEntry);

	if (!_HoleListAddHole(pEntry->nStartOffset, pEntry->nNumBytes))
		return FALSE;

	u32 uHoleOffset = 0;
	BOOL bHole = _HoleListFindHole(uFileSize, uHoleOffset);

	if (bHole)
	{
		FASSERT(uHoleOffset);
	}

	// Update the entry to have a new position, size and CRC
	m_pFileEntries[uWriteIndex].nModifiedTime = uModTime;
	m_pFileEntries[uWriteIndex].nStartOffset = bHole ? uHoleOffset : m_FileHeader.nBytesInFile;
	m_pFileEntries[uWriteIndex].nNumBytes = uFileSize;
	m_pFileEntries[uWriteIndex].nCRC = uCRC;

	s32 sSeekOffset = sizeof(FDataPrjFile_Header_t) + (uWriteIndex * sizeof(FDataPrjFile_Entry_t));
	s32 sRes;

	// Write the header entry
	ffile_Seek(m_hFileWrite, sSeekOffset, FFILE_SEEK_SET);
	sRes = ffile_Write(m_hFileWrite, sizeof(FDataPrjFile_Entry_t), (void *) pEntry);

	if (sRes != sizeof(FDataPrjFile_Entry_t))
	{
		DEVPRINTF("CFMasterFile::File write error\n");
		return FALSE;
	}

	// Write the data for the file to the end of the file
	//ffile_Seek(m_hFileWrite, m_FileHeader.nBytesInFile, FFILE_SEEK_SET);
	ffile_Seek(m_hFileWrite, bHole ? uHoleOffset : m_FileHeader.nBytesInFile, FFILE_SEEK_SET);
	sRes = ffile_Write(m_hFileWrite, uFileSize, puFileData);

	if ((u32) sRes != uFileSize)
	{
		DEVPRINTF("CFMasterFile::File write error\n");
		return FALSE;
	}

	// Fill with zeros until byte boundary
	sRes = _ZeroUntilBoundary(uFileSize);

	if (!bHole)
	{
		// Update the number of bytes in the file
		m_FileHeader.nBytesInFile += (uFileSize + sRes);

		// Write the updated header
		ffile_Seek(m_hFileWrite, 0, FFILE_SEEK_SET);
		sRes = ffile_Write(m_hFileWrite, sizeof(FDataPrjFile_Header_t), &m_FileHeader);

		if (sRes != sizeof(FDataPrjFile_Header_t))
		{
			DEVPRINTF("CFMasterFile::File write error\n");
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CFMasterFile::_UpdateEntry(const FDataPrjFile_Entry_t *pEntry, const u32 &uFileSize, u8 *puSrcData, const u32 &uModTime, const u32 &uCRC)
{
	u32 uWriteIndex;

	for (uWriteIndex = 0; uWriteIndex < m_FileHeader.nNumEntries; ++uWriteIndex)
	{
		if (pEntry == (m_pFileEntries + uWriteIndex))
		{
			break;
		}
	}

	if (uWriteIndex >= m_FileHeader.nNumEntries)
	{
		DEVPRINTF("CFMasterFile::Passed file entry not found in master file!\n");
		return FALSE;
	}

	// Update the entry to have a new timestamp, size and CRC
	m_pFileEntries[uWriteIndex].nModifiedTime = uModTime;
	m_pFileEntries[uWriteIndex].nNumBytes = uFileSize;
	m_pFileEntries[uWriteIndex].nCRC = uCRC;

	s32 sSeekOffset = sizeof(FDataPrjFile_Header_t) + (uWriteIndex * sizeof(FDataPrjFile_Entry_t));
	s32 sRes;

	// Write the header entry
	ffile_Seek(m_hFileWrite, sSeekOffset, FFILE_SEEK_SET);
	sRes = ffile_Write(m_hFileWrite, sizeof(FDataPrjFile_Entry_t), (void *) pEntry);

	if (sRes != sizeof(FDataPrjFile_Entry_t))
	{
		DEVPRINTF("CFMasterFile::File write error\n");
		return FALSE;
	}

	// Write the file data
	ffile_Seek(m_hFileWrite, pEntry->nStartOffset, FFILE_SEEK_SET);
	sRes = ffile_Write(m_hFileWrite, uFileSize, puSrcData);

	if ((u32) sRes != uFileSize)
	{
		DEVPRINTF("CFMasterFile::File write error\n");
		return FALSE;
	}

	return TRUE;
}

// Write zeros to the file where pEntry resides
BOOL CFMasterFile::_ZeroEntryInFile(const FDataPrjFile_Entry_t *pEntry)
{
	u8 uZero[FDATA_PRJFILE_FILE_ALIGNMENT];
	u32 uSize = pEntry->nNumBytes;
	s32 sWritten = 0;

	fang_MemZero(uZero, sizeof(uZero));

	ffile_Seek(m_hFileWrite, pEntry->nStartOffset, FFILE_SEEK_SET);

	while (uSize > 0)
	{
		if (uSize >= FDATA_PRJFILE_FILE_ALIGNMENT)
		{
			sWritten = ffile_Write(m_hFileWrite, FDATA_PRJFILE_FILE_ALIGNMENT, &uZero);
		} 
		else 
		{
			sWritten = ffile_Write(m_hFileWrite, uSize, &uZero);
		}

		if (sWritten == FFILE_ERROR_RET)
		{
			return FALSE;
		}

		uSize -= sWritten;
	}

	return TRUE;
}

// Writes zeros from current file position until the next FDATA_PRJFILE_FILE_ALIGNMENT boundary
u32 CFMasterFile::_ZeroUntilBoundary(const u32 &uSize)
{
	u32 uDestSize = FMATH_BYTE_ALIGN_UP(uSize, FDATA_PRJFILE_FILE_ALIGNMENT);
	u32 uTotal = uDestSize - uSize;

	if (uTotal > 0)
	{
		u8 uZero[FDATA_PRJFILE_FILE_ALIGNMENT];
		s32 sWritten = 0;

		fang_MemZero(uZero, sizeof(uZero));

		uDestSize -= uSize;

		while (uDestSize > 0)
		{
			if (uDestSize >= FDATA_PRJFILE_FILE_ALIGNMENT)
			{
				sWritten = ffile_Write(m_hFileWrite, FDATA_PRJFILE_FILE_ALIGNMENT, &uZero);
			}
			else
			{
				sWritten = ffile_Write(m_hFileWrite, uDestSize, &uZero);
			}

			if (sWritten == FFILE_ERROR_RET)
			{
				FASSERT(0);
				// we failed, what should we do?
				return uTotal;
			}

			uDestSize -= sWritten;
		}
	}

	return uTotal;
}

u8 *CFMasterFile::_LoadDiskFile(const char *pszFileName, u32 &uSize)
{
	FFileHandle hSrcFile = ffile_Open(pszFileName, FFILE_OPEN_RONLY, TRUE);

	uSize = 0;

	if (!FFILE_IS_VALID_HANDLE(hSrcFile))
	{
		DEVPRINTF("CFMasterFile::Failed to load disk file\"%s\"\n", pszFileName);
		return 0;
	}

	u32 uSrcFileSize = ffile_GetFileSize(hSrcFile);
	
	if (uSrcFileSize == -1)
	{
		DEVPRINTF("CFMasterFile::Failed to get file size of \"%s\"\n", pszFileName);
		return 0;	
	}
	
	u8 *puSrcFileData = new u8[uSrcFileSize];

	if (!puSrcFileData)
	{
		DEVPRINTF("CFMasterFile::Memory error\n");
		return 0;
	}

	if (ffile_Read(hSrcFile, uSrcFileSize, puSrcFileData) != (s32) uSrcFileSize)
	{
		DEVPRINTF("CFMasterFile::File read error.  Read data does not match file size\n");
		delete [] puSrcFileData;
		return 0;
	}

	ffile_Close(hSrcFile);

	uSize = uSrcFileSize;

	return puSrcFileData;
}

// !!TODO use a circularly linked list for all hole code to cut out all the extra code and tests
BOOL CFMasterFile::_HoleListBuild(void)
{
	// First, sort the header entries by their start offset
	fclib_QSort(m_pFileEntries, m_FileHeader.nNumEntries, sizeof(FDataPrjFile_Entry_t), _MIntQSortCompareFcn);

	u32 uEnd = m_FileHeader.nNumEntries - 1;

	for (u32 uCnt = 0; uCnt < uEnd; ++uCnt)
	{
		u32 uAlignSize = FMATH_BYTE_ALIGN_UP(m_pFileEntries[uCnt].nNumBytes, FDATA_PRJFILE_FILE_ALIGNMENT);

		if ((m_pFileEntries[uCnt].nStartOffset + uAlignSize) != m_pFileEntries[uCnt + 1].nStartOffset)
		{
			u32 uHoleStart = m_pFileEntries[uCnt].nStartOffset + uAlignSize;

			//DEVPRINTF("%s %s\n", m_pFileEntries[uCnt].szFilename, m_pFileEntries[uCnt + 1].szFilename);

			if (!_HoleListAddHole(uHoleStart, m_pFileEntries[uCnt + 1].nStartOffset - uHoleStart))
			{
				// MUST do this to put things back in proper order
				fclib_QSort(m_pFileEntries, m_FileHeader.nNumEntries, sizeof(FDataPrjFile_Entry_t), _MQSortCompareFcn);
				return FALSE;
			}
		}
	}

	// MUST do this to put things back in proper order
	fclib_QSort(m_pFileEntries, m_FileHeader.nNumEntries, sizeof(FDataPrjFile_Entry_t), _MQSortCompareFcn);

	return TRUE;
}

BOOL CFMasterFile::_HoleListFindHole(const u32 &uSize, u32 &uOffset)
{
	u32 uWantSize = FMATH_BYTE_ALIGN_UP(uSize, FDATA_PRJFILE_FILE_ALIGNMENT);
	_MasterHole_t *pCurr = m_pMasterHoles;

	while (pCurr)
	{
		if (uWantSize <= pCurr->nNumBytes)
			break;

		pCurr = pCurr->pNext;
	}

	if (!pCurr)
	{
		DEVPRINTF("no hole found for %d\n", uWantSize);
		return FALSE;
	}

	//DEVPRINTF("found hole at %d w/ size %d - needed %d\n", pCurr->nStartOffset, pCurr->nNumBytes, uSize);

	uOffset = pCurr->nStartOffset;
	pCurr->nNumBytes -= uWantSize;

	if (pCurr->nNumBytes <= 0)
	{
		_MasterHole_t *pPrev = pCurr->pPrev;
		_MasterHole_t *pNext = pCurr->pNext;

		if (pPrev && pNext)
		{
			// Middle
			pPrev->pNext = pNext;
			pNext->pPrev = pPrev;
		}
		else if (pPrev && !pNext)
		{
			// Last
			pPrev->pNext = 0;
		}
		else if (!pPrev && pNext)
		{
			// First
			pNext->pPrev = 0;
			m_pMasterHoles = pNext;
		}
		else
		{
			m_pMasterHoles = 0;
		}

		delete pCurr;
	}
	else
	{
		pCurr->nStartOffset += uWantSize;
		//DEVPRINTF("hole leftover of %d at %d\n", pCurr->nNumBytes, pCurr->nStartOffset);
	}

	return TRUE;
}

BOOL CFMasterFile::_HoleListAddHole(const u32 &uStart, const u32 &uSize)
{
	u32 uHoleSize = FMATH_BYTE_ALIGN_UP(uSize, FDATA_PRJFILE_FILE_ALIGNMENT);

	//DEVPRINTF("Add hole start %d with size %d\n", uStart, uHoleSize);

	_MasterHole_t *pCurrHole = m_pMasterHoles;
	_MasterHole_t *pPrevHole = 0;

	while (pCurrHole)
	{
		FASSERT(pCurrHole->nStartOffset != uStart);

		if (pPrevHole)
		{
			FASSERT(pPrevHole->nStartOffset < pCurrHole->nStartOffset);
		}

		if (uStart > pCurrHole->nStartOffset)
		{
			pPrevHole = pCurrHole;
			pCurrHole = pCurrHole->pNext;
		}
		else
		{
			break;
		}

	}

	BOOL bHoleBefore = _HoleListCheckHoleBefore(pPrevHole, uStart, uHoleSize);
	BOOL bHoleAfter  = _HoleListCheckHoleAfter(pCurrHole, uStart, uHoleSize);

	//  bHoleBefore &&  bHoleAfter - Hole above and below new hole
	//  bHoleBefore && !bHoleAfter - Hole below new hole
	// !bHoleBefore &&  bHoleAfter - Hole above new hole
	// !bHoleBefore && !bHoleAfter - Just insert a new hole
	if (bHoleBefore && bHoleAfter)
	{
		DEVPRINTF("Found merge hole before and after\n");

		pPrevHole->nNumBytes += (uHoleSize + pCurrHole->nNumBytes);
		pPrevHole->pNext = pCurrHole->pNext;

		if (pCurrHole->pNext)
			pCurrHole->pNext->pPrev = pPrevHole;

		delete pCurrHole;
	} 
	else if (bHoleBefore && !bHoleAfter)
	{
		//DEVPRINTF("Found merge hole before\n");

		pPrevHole->nNumBytes += uHoleSize;
	}
	else if (!bHoleBefore && bHoleAfter)
	{
		//DEVPRINTF("Found merge hole after\n");

		pCurrHole->nStartOffset = uStart;
		pCurrHole->nNumBytes += uHoleSize;
	}
	else
	{
		// Can't merge any holes, need a new one
		_MasterHole_t *pNewHole = new _MasterHole_t;

		if (!pNewHole)
		{
			DEVPRINTF("CMasterFile : Memory allocation error!\n");
			return FALSE;
		}

		pNewHole->pPrev = pNewHole->pNext = 0;
		pNewHole->nStartOffset = uStart;
		pNewHole->nNumBytes = uHoleSize;

		if (pCurrHole && pPrevHole)
		{
			// Put new hole inbetween two others
			pNewHole->pNext = pCurrHole;
			pNewHole->pPrev = pPrevHole;

			pCurrHole->pPrev = pNewHole;
			pPrevHole->pNext = pNewHole;
		}
		else if (pCurrHole && !pPrevHole)
		{
			// Before first item in list
			pNewHole->pNext = pCurrHole;
			pNewHole->pPrev = 0;

			pCurrHole->pPrev = pNewHole;

			m_pMasterHoles = pNewHole;
		}
		else if (!pCurrHole && pPrevHole)
		{
			// After last item in list
			pPrevHole->pNext = pNewHole;
			pNewHole->pPrev = pPrevHole;
		}
		else
		{
			// Only item in list
			m_pMasterHoles = pNewHole;
		}
	}
	
	return TRUE;
}

// Returns true if the passed hole comes after our new hole
// This means that the hole is below our new hole.
BOOL CFMasterFile::_HoleListCheckHoleAfter(const _MasterHole_t *pCurr, const u32 &uStart, const u32 &uSize)
{
	if (!pCurr)
		return FALSE;

	return (uStart + uSize) == pCurr->nStartOffset;
}

// Returns true if the passed hole comes before our new hole
// This means that the hole is above our new hole.
BOOL CFMasterFile::_HoleListCheckHoleBefore(const _MasterHole_t *pCurr, const u32 &uStart, const u32 &uSize)
{
	if (!pCurr)
		return FALSE;

	return (pCurr->nStartOffset + pCurr->nNumBytes) == uStart;
}

