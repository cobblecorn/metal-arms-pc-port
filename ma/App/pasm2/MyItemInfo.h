//////////////////////////////////////////////////////////////////////////////////////
// MyItemInfo.h - my item info used with MyListCtrl
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
// 06/08/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#if !defined(AFX_MYITEMINFO_H__CBAA6794_F21D_11D1_9F9F_84E5D3000000__INCLUDED_)
#define AFX_MYITEMINFO_H__CBAA6794_F21D_11D1_9F9F_84E5D3000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "fang.h"
#include "ItemInfo.h"
#include "fdata.h"
#include "FileInfo.h"

class CMyItemInfo : public CItemInfo  
{
public:
	CMyItemInfo( int nItem, const CString &rsFilename, CFileInfo *pLibFile, CFileInfo *pLocFile, FDataPrjFile_Entry_t *pMstEntry );
	virtual ~CMyItemInfo();

	CString& GetFilename()		{ return m_sFilename; }
	CString& GetFileExtension()	{ return m_sExtension; }
	CString& GetLibTime()		{ return m_sLibFileTime; }
	CString& GetLocTime()		{ return m_sLocFileTime; }
	CString& GetMasterTime()	{ return m_sMasterFileTime; }

	CFileInfo *GetLibFileInfo()	{ return m_pLibFileInfo; }
	CFileInfo *GetLocFileInfo()	{ return m_pLocFileInfo; }
	FDataPrjFile_Entry_t *GetMasterFileEntry()	{ return m_pMasterFileEntry; }

private:

	CMyItemInfo();

	CString m_sFilename;// with extension
	CString m_sExtension;
	CFileInfo *m_pLibFileInfo;
	CFileInfo *m_pLocFileInfo;
	FDataPrjFile_Entry_t *m_pMasterFileEntry;
    
	CString m_sLibFileTime;
    CString m_sLocFileTime;
	CString m_sMasterFileTime;

	void FillTimeStringFromFileInfo( CString &rsString, CFileInfo *pFileInfo );
	void FillTimeStringFromEntry( CString &rsString, FDataPrjFile_Entry_t *pEntry );
};

#endif // !defined(AFX_MYITEMINFO_H__CBAA6794_F21D_11D1_9F9F_84E5D3000000__INCLUDED_)
