//////////////////////////////////////////////////////////////////////////////////////
// MyItemInfo.cpp - 
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
#include "stdafx.h"
#include "fang.h"
#include "MyItemInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CMyItemInfo::CMyItemInfo( int nItem, const CString &rsFilename,
						  CFileInfo *pLibFile, CFileInfo *pLocFile,
						  FDataPrjFile_Entry_t *pMstEntry ) : CItemInfo( nItem ) {
	m_sFilename = rsFilename;
	m_sFilename.MakeLower();
	int nIndex = m_sFilename.ReverseFind( '.' );
	if( nIndex >= 0 ) {
		m_sExtension = m_sFilename.Mid( nIndex + 1 );
	} else {
		m_sExtension.Empty();
	}
	m_pLibFileInfo = pLibFile;
	m_pLocFileInfo = pLocFile;
	m_pMasterFileEntry = pMstEntry;

	FillTimeStringFromFileInfo( m_sLibFileTime, m_pLibFileInfo );
	FillTimeStringFromFileInfo( m_sLocFileTime, m_pLocFileInfo );
	FillTimeStringFromEntry( m_sMasterFileTime, m_pMasterFileEntry );
}

CMyItemInfo::~CMyItemInfo()
{

}

void CMyItemInfo::FillTimeStringFromFileInfo( CString &rsString, CFileInfo *pFileInfo ) {
	
	rsString.Empty();
	if( !pFileInfo ) {
		return;
	}
	CTime ModifiedTime( pFileInfo->GetLastWriteTime() );
	rsString.Format( "%d/%02d/%02d %02d:%02d:%02d", ModifiedTime.GetYear(),
													ModifiedTime.GetMonth(),
													ModifiedTime.GetDay(),													
													ModifiedTime.GetHour(),
													ModifiedTime.GetMinute(),
													ModifiedTime.GetSecond() );
}

void CMyItemInfo::FillTimeStringFromEntry( CString &rsString, FDataPrjFile_Entry_t *pEntry ) {
	
	rsString.Empty();
	if( !pEntry ) {
		return;
	}
	CTime ModifiedTime( (time_t )pEntry->nModifiedTime );
	rsString.Format( "%d/%02d/%02d %02d:%02d:%02d", ModifiedTime.GetYear(),
													ModifiedTime.GetMonth(),
													ModifiedTime.GetDay(),													
													ModifiedTime.GetHour(),
													ModifiedTime.GetMinute(),
													ModifiedTime.GetSecond() );
}
