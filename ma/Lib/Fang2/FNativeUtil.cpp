//////////////////////////////////////////////////////////////////////////////////////
// FNativeUtil.cpp - Native scripting function utilities.
//
// Author: Justin Link
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
// 04/02/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "FNativeUtil.h"
#include "fres.h"

//// Utility functions.
//

// Do not use this function, it is evil, pure evil I tell you!!!!
// Use the function below.
/*static */char *CreateAndFillStringFromCell(AMX *amx,cell params)
{
	FASSERT_NOW;
    char *pszDest;
    s32 nLen;
    cell *pString;

    // Get the real address of the string.
    amx_GetAddr(amx, params, &pString);

    // Find out how long the string is in characters.
    amx_StrLen(pString, (int *)&nLen);
//    pszDest = fnew char[nLen+1];
	pszDest = (char *)(fres_Alloc(nLen + 1));

    // Now convert the Small String into a C type null terminated string
    amx_GetString(pszDest, pString);

    return(pszDest);
}

// Use this function.
// It does the same as the above did, but it doesn't allocate space.
// uMaxStringSize is 1 less than the size of the buffer.
/*static */void FillStringFromCell(char *pszDestString, u32 uMaxStringSize, AMX *pAMX, cell oParam)
{
	u32 uLen;
	cell *pString;

    // Get the real address of the string.
    amx_GetAddr(pAMX, oParam, &pString);

    // Find out how long the string is in characters.
    amx_StrLen(pString, (int *)(&uLen));
	FASSERT(uMaxStringSize >= uLen);

    // Now convert the Small String into a C type null terminated string
    amx_GetString(pszDestString, pString);
}

//
////
