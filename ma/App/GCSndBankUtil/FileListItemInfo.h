//////////////////////////////////////////////////////////////////////////////////////
// FileListItemInfo.h - 
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
// 06/04/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FILE_LIST_ITEM_INFO_H_
#define _FILE_LIST_ITEM_INFO_H_ 1


#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "fang.h"
#include "ItemInfo.h"
#include "FileInfo.h"

class CFileListItemInfo : public CItemInfo  
{
public:
	CFileListItemInfo( int nItem, const CString &rsFilename, CFileInfo *pFile );
	virtual ~CFileListItemInfo();

	CString &GetFilenameString()		{ return m_sFilename; }
	CString &GetFileTimeString()		{ return m_sFileTime; }
	CString &GetFileSizeString()		{ return m_sFileSize; }

	CTime GetFileTime()					{ return m_FileTime; }
	u32 GetFileSize()					{ return m_nFileSize; }
	CFileInfo *GetFileInfo()			{ return m_pFile; }
		
private:

	CFileListItemInfo();

	CString m_sFilename;
	CString m_sFileTime;
	CString m_sFileSize;

	CTime m_FileTime;
	u32 m_nFileSize;
	CFileInfo *m_pFile;
};


#endif
