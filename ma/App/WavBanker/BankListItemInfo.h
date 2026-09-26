//////////////////////////////////////////////////////////////////////////////////////
// BankListItemInfo.h - 
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
#ifndef _BANK_LIST_ITEM_INFO_H_
#define _BANK_LIST_ITEM_INFO_H_ 1


#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "fang.h"
#include "ItemInfo.h"
#include "WavBank.h"

class CBankListItemInfo : public CItemInfo  
{
public:
	CBankListItemInfo( int nItem, WavBank_Entry_t *pEntry );
	virtual ~CBankListItemInfo();

	CString &GetFilenameString()		{ return m_sFilename; }
	CString &GetFileTimeString()		{ return m_sFileTime; }
	CString &GetFileSizeString()		{ return m_sFileSize; }

	CTime GetFileTime()					{ return m_FileTime; }
	u32 GetFileSize()					{ return m_nFileSize; }
	WavBank_Entry_t *GetEntry()			{ return m_pEntry; }
		
private:

	CBankListItemInfo();

	CString m_sFilename;
	CString m_sFileTime;
	CString m_sFileSize;

	CTime m_FileTime;
	u32 m_nFileSize;
	WavBank_Entry_t *m_pEntry;
};


#endif
