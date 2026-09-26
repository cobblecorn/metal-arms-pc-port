//////////////////////////////////////////////////////////////////////////////////////
// loadingscreen.h - Loading Screen class.
//
// Author: Russell Foushee     
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
// 01/11/03 Foushee     Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _LOADINGSCREEN_H_
#define _LOADINGSCREEN_H_ 1

#include "fang.h"
#include "fresload.h"


BOOL loadingscreen_Init( cchar *pszLoadingMovie, cwchar *pwszHeading=NULL );
BOOL loadingscreen_Update( cchar *pszDebugString=NULL );
void loadingscreen_Uninit();

#endif //_LOADINGSCREEN_H_