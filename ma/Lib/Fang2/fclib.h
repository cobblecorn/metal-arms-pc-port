//////////////////////////////////////////////////////////////////////////////////////
// fclib.h - Fang C library.
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
// 03/15/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FCLIB_H_
#define _FCLIB_H_ 1

#include <stdio.h>
#include "fang.h"

// This callback is used by fclib_QSort(). It should compare the two elements and return
// one of the following values based on the comparison result:
//
//   -1: if pElement1 is less than pElement2 
//    0: if pElement1 is equivalent to pElement2 
//    1: if pElement1 is greater than pElement2 
typedef int FClibQSortCallback_t( const void *pElement1, const void *pElement2 );


#if defined(__cplusplus)
extern "C" {
#endif

// NOTE: All these function emulate the standard C library, and while I could have fang-ized their
//       names, I thought it better for clarity to just prepend fclib_*. [BD]

extern void fclib_QSort( void *pArray, u32 nElementCount, u32 nBytesPerElement, FClibQSortCallback_t *pCallback );

extern char *fclib_strcpy( char *pszDest, cchar *pszSrc );
extern char *fclib_strncpy( char *pszDest, cchar *pszSrc, int nNumChars );
extern char *fclib_strncpyz( char *pszDest, cchar *pszSrc, int nNumChars );

extern char *fclib_strcat( char *pszDest, cchar *pszSrc );
extern char *fclib_strncat(char *pszDest,cchar *pszSrc, int nNumChars);

extern int fclib_strlen( cchar *pszString );

extern int fclib_strcmp( cchar *pszString1, cchar *pszString2 );
extern int fclib_stricmp( cchar *pszString1, cchar *pszString2 );
extern int fclib_strncmp( cchar *pszString1, cchar *pszString2, int nNumCmp );
extern int fclib_strnicmp( cchar *pszString1, cchar *pszString2, int nNumCmp );

extern char *fclib_strupr( char *pszString );
extern char *fclib_strlwr( char *pszString );
extern char fclib_tolower( char c );
extern char fclib_toupper( char c );

extern char *fclib_strrchr( cchar *pszString, char c );
extern char *fclib_strchr( cchar *pszString, char c );
extern char *fclib_strstr( cchar *pszString, cchar *pszSubStringToFind );

extern char *fclib_TrimLeadingWhiteSpace( cchar *pszString );

extern BOOL fclib_IsStringNullTerminated( cchar *pszString, u32 nMaxNonZeroCharacters );

extern cchar *fclib_FindFileNameInPath( cchar *pszPathName );

// wide (unicode) string functions.
extern wchar *fclib_wcsupr( wchar *pwszString );
extern wchar *fclib_wcslwr( wchar *pwszString );
extern wchar fclib_towlower( wchar wc );
extern wchar fclib_towupper( wchar wc );

extern wchar *fclib_wcschr( cwchar *pwszString, wchar c );								// wide strchr
extern int fclib_wcscmp( cwchar *pwszString1, cwchar *pwszString2 );					// wide strcmp
extern int fclib_wcsicmp( cwchar *pwszString1, cwchar *pwszString2 );					// wide strcmp
extern int fclib_wcslen( cwchar *pwszString );										// wide strlen
extern wchar *fclib_wcscpy( wchar *pwszDest, cwchar *pwszSrc );							// wide strcpy
extern wchar *fclib_wcsncpy( wchar *pwszDest, cwchar *pwszSrc, int nNumChars );			// wide strncpy
extern void fclib_WideToChar( char *pszDest, cwchar *pwszSrc );						// copies wide string to char string
extern void fclib_CharToWide( wchar *pwszDest, cchar *pszSrc );							// copies char string to wide string

extern wchar *fclib_wcscat( wchar *pszDest, cwchar *pszSrc );

extern s32 fclib_FindStringInArray( cchar *pszString, cchar **apszStringArray );

#if defined(__cplusplus)
}
#endif

#endif
