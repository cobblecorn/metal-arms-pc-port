//////////////////////////////////////////////////////////////////////////////////////
// pickdir.cpp - Windows directory picker.
//
// Author: Steve Ranck     
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
// 02/13/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "pickdir.h"


// This is the default callback procedure for the SHBrowseForFolder function.
// It will set the current selection to the directory specified in the edit control
int CALLBACK FECCallBackProc( HWND hWnd, UINT Msg, LPARAM unused, LPARAM lpData ) {
    
	if( Msg == BFFM_INITIALIZED ) {
        ::SendMessage( hWnd, BFFM_SETSELECTION, TRUE, lpData );
	}
    return 0;
}

// Allows user to select a directory.
// Returns PICKDIR_RET_SUCCESS if successful.
// See PickDirRet_e for other return values.
PickDirRet_e pickdir_PickDir( HWND hOwnerWnd,
							  CString *psDestString,
							  const char *pszTitle/*=NULL*/,
							  const char *pszInitialDir/*=NULL*/ ) {
	BROWSEINFO BrowseInfo;
	char szDirName[MAX_PATH+1];
	char szDirPath[MAX_PATH+1];
	ITEMIDLIST *pItemList;
	IMalloc* pMalloc;
	BOOL bSuccess;
	PickDirRet_e nRet;

	BrowseInfo.hwndOwner = hOwnerWnd;
	BrowseInfo.pidlRoot = NULL;
	BrowseInfo.pszDisplayName = szDirName;
	if( !pszTitle ) {
		BrowseInfo.lpszTitle = "Browse for Folder";
	} else {
		BrowseInfo.lpszTitle = pszTitle;
	}
	BrowseInfo.ulFlags = BIF_RETURNONLYFSDIRS;
	if( !pszInitialDir ) {
		BrowseInfo.lpfn = NULL;
		BrowseInfo.lParam = 0;
	} else {
		// setup the callback function so that we can set an initial directory
		BrowseInfo.lParam = (LPARAM)pszInitialDir; // set lParam to point to pszInitialDir
		BrowseInfo.lpfn = FECCallBackProc;
	}
	BrowseInfo.iImage = 0;

	pItemList = SHBrowseForFolder( &BrowseInfo );

	if( pItemList ) {
		bSuccess = SHGetPathFromIDList( pItemList, szDirPath );

		if( bSuccess ) {
			*psDestString = szDirPath;
			nRet = PICKDIR_RET_SUCCESS;
		} else {
			nRet = PICKDIR_RET_ERROR;
		}

		if( SUCCEEDED( SHGetMalloc( &pMalloc ) ) ) {
			pMalloc->Free( (void *)pItemList );
			pMalloc->Release();
		}
	} else {
		nRet = PICKDIR_RET_CANCEL;
	}

	return nRet;
}


