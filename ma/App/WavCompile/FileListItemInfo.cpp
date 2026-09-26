//////////////////////////////////////////////////////////////////////////////////////
// FileListItemInfo.cpp - 
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
#include "stdafx.h"
#include "fang.h"
#include "FileListItemInfo.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CFileListItemInfo::CFileListItemInfo( int nItem, const CString &rsFilename, CFileInfo *pFile ) : CItemInfo( nItem ) {
	
	m_sFilename = rsFilename;

	f32 fFileSize;
	fFileSize = pFile->GetLength() * (1.0f/1024.0f);
	m_nFileSize = (u32)fFileSize;
	if( m_nFileSize < 0 ) {
		m_nFileSize = 1;
	} else if( (f32)m_nFileSize < fFileSize ) {
		m_nFileSize++;
	}
	m_sFileSize.Format( "%d KB", m_nFileSize );
	
	m_FileTime = pFile->GetLastWriteTime();

	m_sFileTime = m_FileTime.Format( "%#m/%#d/%Y %#I:%M %p" );

	m_pFile = pFile;
}

CFileListItemInfo::~CFileListItemInfo()
{

}


