//////////////////////////////////////////////////////////////////////////////////////
// pickdir.h - Windows directory picker.
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

#ifndef _PICKDIR_H_
#define _PICKDIR_H_ 1

#include "fang.h"


typedef enum {
	PICKDIR_RET_SUCCESS,		// psDestString contains the full directory path that the user chose.
	PICKDIR_RET_CANCEL,			// The user clicked the cancel button. psDestString was not modified.
	PICKDIR_RET_ERROR,			// The user chose a directory, but there was an error trying to select it.

	PICKDIR_RET_COUNT
} PickDirRet_e;


extern PickDirRet_e pickdir_PickDir( HWND hOwnerWnd,
									 CString *psDestString,
									 const char *pszTitle=NULL,
									 const char *pszInitialDir=NULL );



#endif

