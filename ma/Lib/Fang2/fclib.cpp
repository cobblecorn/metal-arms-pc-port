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

#include "fang.h"
#include "fclib.h" 

#include <stdlib.h>

// TODO: Replace this with our own qsort so we can get rid of stdlib.
void fclib_QSort( void *pArray, u32 nElementCount, u32 nBytesPerElement, FClibQSortCallback_t *pCallback ) {
	qsort( pArray, nElementCount, nBytesPerElement, pCallback );
}

// Implementation of the standard C library function, strcpy().
char *fclib_strcpy( char *pszDest, cchar *pszSrc ) {
	char *pszOrigDest = pszDest;

	while( (*pszDest++ = *pszSrc++) ) ;
	return pszOrigDest;
}

// Implementation of the standard C library function, strncpy().
char *fclib_strncpy( char *pszDest, cchar *pszSrc,int nNumChars ) {
	char *pszOrigDest = pszDest;

	while( *pszSrc && nNumChars ) {
		*pszDest++ = *pszSrc++;
		nNumChars--;
	}
	while( nNumChars ) {
		*pszDest++ = 0;
		nNumChars--;
	}
	return pszOrigDest;	
}

// Same as fclib_strncpy(), but always appends a null-terminator after nNumChars
// have been copied. pszDest must be large enough to hold nNumChars+1 characters.
char *fclib_strncpyz( char *pszDest, cchar *pszSrc, int nNumChars ) {
	char *pszOrigDest = pszDest;

	while( *pszSrc && nNumChars ) {
		*pszDest++ = *pszSrc++;
		nNumChars--;
	}
	nNumChars++;
	while( nNumChars ) {
		*pszDest++ = 0;
		nNumChars--;
	}
	return pszOrigDest;	
}

// Implementation of the standard C library function, strcat().
char *fclib_strcat( char *pszDest, cchar *pszSrc ) {
	char *pszOrigDest = pszDest;

	while( *pszDest ) {
		pszDest++;
	}

	while( (*pszDest++ = *pszSrc++) ) ;

	return pszOrigDest;	
}

// Implementation of the standard C library function, strncat().
char *fclib_strncat(char *pszDest,cchar *pszSrc,int nNumChars) {
	char *pszOrigDest = pszDest;

	while( *pszDest++ ) ;

	while( *pszSrc && nNumChars ) {
		*pszDest++ = *pszSrc++;
		nNumChars--;
	}
	*pszDest = 0;

	return pszOrigDest;
}

// Implementation of the standard C library function, strlen().
int fclib_strlen( cchar *pszString ) {
	int nLength;

	for( nLength=0; pszString[nLength]; nLength++ ) ;

	return nLength;	
}

// Implementation of the standard C library function, strcmp().
// (case-sensitive).
int fclib_strcmp( cchar *pszString1, cchar *pszString2 ) {
	
	while( TRUE ) {
		if( *pszString1==0 && *pszString2==0 ) {
			return 0;
		}
		if( *pszString1 < *pszString2 ) {
			return -1;
		}
		if( *pszString1 > *pszString2 ) {
			return 1;
		}
		pszString1++;
		pszString2++;
	}	
}

// Implementation of the standard C library function, stricmp().
// (case-insensitive).
int fclib_stricmp( cchar *pszString1, cchar *pszString2 ) {
	char c1, c2;

	while( TRUE ) {
		if( *pszString1==0 && *pszString2==0 ) {
			return 0;
		}
		if( *pszString1==0 ) {
			return -1;		// string 1 ended early
		}
		if( *pszString2==0 ) {
			return 1;		// string 2 ended early
		}
		c1 = fclib_toupper( *pszString1 );
		c2 = fclib_toupper( *pszString2 );
		if( c1 < c2 ) {
			return -1;
		}
		if( c1 > c2 ) {
			return 1;
		}
		pszString1++;
		pszString2++;
	}	
}

// Implementation of the standard C library function, strncmp().
// (case-sensitive).
int fclib_strncmp( cchar *pszString1, cchar *pszString2, int nNumCmp ) {
	int i;

	for( i=0; i < nNumCmp; i++ ) {
		if( *pszString1==0 && *pszString2==0 ) {
			return 0;
		}
		if( *pszString1 == 0 ) {
			return -1;
		}
		if( *pszString2 == 0 ) {
			return 1;
		}
		if( *pszString1 < *pszString2 ) {
			return -1;
		}
		if( *pszString1 > *pszString2 ) {
			return 1;
		}
		pszString1++;
		pszString2++;
	}
	return 0;
}

// Implementation of the standard C library function, strnicmp().
// (case-insensitive).
int fclib_strnicmp( cchar *pszString1, cchar *pszString2, int nNumCmp ) {
	int i;
	char c1, c2;

	for( i=0; i < nNumCmp; i++ ) {
		c1 = fclib_toupper( *pszString1 );
		c2 = fclib_toupper( *pszString2 );

		if( c1==0 && c2==0 ) {
			return 0;
		}
		if( c1 == 0 ) {
			return -1;
		}
		if( c2 == 0 ) {
			return 1;
		}
		if( c1 < c2 ) {
			return -1;
		}
		if( c1 > c2 ) {
			return 1;
		}
		pszString1++;
		pszString2++;
	}
	return 0;	
}

// Implementation of the standard C library function, strupr().
// Converts the specified string to uppercase.
char *fclib_strupr( char *pszString ) {
	char *pszCurChar = pszString;

	while( *pszCurChar ) {
		*pszCurChar = fclib_toupper( *pszCurChar );
		pszCurChar++;
	}
	return pszString;	
}

// Implementation of the standard C library function, strlwr().
// Converts the specified string to lowercase.
char *fclib_strlwr( char *pszString ) {
	char *pszCurChar = pszString;

	while( *pszCurChar ) {
		*pszCurChar = fclib_tolower( *pszCurChar );
		pszCurChar++;
	}
	return pszString;	
}

// Returns the lowercase equivalent of the character.
char fclib_tolower( char c ) {
	
	if( c >= 'A' && c <= 'Z' ) {
		c += ('a'-'A');
	}
	return c;	
}

// Returns the uppercase equivalent of the character.
char fclib_toupper( char c ) {

	if( c >= 'a' && c <= 'z' ) {
		c -= ('a'-'A');
	}
	return c;	
}

// Implementation of the standard C library function, strrchr().
// Finds the last occurence of c in pszString.
char *fclib_strrchr( cchar *pszString, char c ) {
	char *pszStart = (char *)pszString;

	// find the end of pszString
	while( *pszString++ ) ;
	//* search towards the front of pszString
	while( --pszString != pszStart && *pszString != c ) ; 

	if( *pszString == c ) {
		return (char *)pszString;
	}

	return NULL;
}

// Implementation of the standard C library function, strchr().
// Finds the first occurence of c in pszString.
char *fclib_strchr( cchar *pszString, char c ) {

	while( *pszString && *pszString != c ) {
		pszString++;
	}
	if( *pszString == c ) {
		return (char *)pszString;
	}
	return NULL;
}

// Implementation of the standard C library function, strstr().
// Finds the first occurence of string ct in string pszString. 
char *fclib_strstr( cchar *pszString, cchar *ct )
{
	char *pStart = NULL;

	if( !( *ct ) )
	{
		return NULL;
	}

	while( *pszString && *pszString != *ct ) {
		pszString++;
	}

	pStart = (char*) pszString;

	while( *pszString && *pszString == *ct )
	{
		pszString++;
		ct++;
	}

	if( *ct )
	{
		return NULL;
	}

	return pStart;
}

BOOL fclib_IsStringNullTerminated( cchar *pszString, u32 nMaxNonZeroCharacters ) {
	u32 i;

	for( i=0; i<nMaxNonZeroCharacters; i++ ) {
		if( pszString[i] == 0 ) {
			return TRUE;
		}
	}

	return (pszString[i] == 0);
}

// pszString must be null terminated, this function will
// return a pointer into pszString of the first non-space
// character.
char *fclib_TrimLeadingWhiteSpace( cchar *pszString ) {
	while( *pszString && *pszString == ' ' ) {
		pszString++;
	}
	if( *pszString ) {
		return (char *)pszString;
	}
	return NULL;
}

cchar *fclib_FindFileNameInPath( cchar *pszPathName ) {
	u32 nStringLength;
	cchar *pszFileName;

	if( pszPathName == NULL ) {
		return NULL;
	}

	nStringLength = fclib_strlen( pszPathName );
	pszFileName = &pszPathName[ nStringLength ];

	for( pszFileName=&pszPathName[ nStringLength ]; nStringLength > 0; pszFileName--, nStringLength-- ) {
		if( *pszFileName=='/' || *pszFileName=='\\' || *pszFileName==':' ) {
			return (pszFileName + 1);
		}
	}

	return pszPathName;
}

// wide (unicode) string functions.

// wide strchr().
wchar *fclib_wcschr( cwchar *pwszString, wchar c ) {

	while( *pwszString && *pwszString != c ) {
		pwszString++;
	}
	if( *pwszString == c ) {
		return (wchar *)pwszString;
	}
	return NULL;
}


// wide strupr().
wchar *fclib_wcsupr( wchar *pwszString ) {
	wchar *pwszCurChar = pwszString;

	while( *pwszCurChar ) {
		*pwszCurChar = fclib_towupper( *pwszCurChar );
		pwszCurChar++;
	}
	return pwszString;	
}

// wide strlwr().
wchar *fclib_wcslwr( wchar *pwszString ) {
	wchar *pwszCurChar = pwszString;

	while( *pwszCurChar ) {
		*pwszCurChar = fclib_towlower( *pwszCurChar );
		pwszCurChar++;
	}
	return pwszString;	
}

// wide tolower
wchar fclib_towlower( wchar c ) {
	
	if( c >= L'A' && c <= L'Z' ) {
		c += (L'a'-L'A');
	}
	return c;	
}

// wide tolower
wchar fclib_towupper( wchar c ) {

	if( c >= L'a' && c <= L'z' ) {
		c -= (L'a'-L'A');
	}
	return c;	
}



// wide strcmp. (case-sensitive).
int fclib_wcscmp( cwchar *pwszString1, cwchar *pwszString2 ) {
	
	while( TRUE ) {
		if( *pwszString1==0 && *pwszString2==0 ) {
			return 0;
		}
		if( *pwszString1 < *pwszString2 ) {
			return -1;
		}
		if( *pwszString1 > *pwszString2 ) {
			return 1;
		}
		pwszString1++;
		pwszString2++;
	}	
}


// wide strcmp. (case-insensitive).
int fclib_wcsicmp( cwchar *pwszString1, cwchar *pwszString2 ) {
	u16 c1, c2;

	while( TRUE ) {
		if( *pwszString1==0 && *pwszString2==0 ) {
			return 0;
		}
		if( *pwszString1==0 ) {
			return -1;		// string 1 ended early
		}
		if( *pwszString2==0 ) {
			return 1;		// string 2 ended early
		}
		c1 = fclib_towupper( *pwszString1 );
		c2 = fclib_towupper( *pwszString2 );
		if( c1 < c2 ) {
			return -1;
		}
		if( c1 > c2 ) {
			return 1;
		}
		pwszString1++;
		pwszString2++;
	}	
}


// wide strlen
int fclib_wcslen( cwchar *pwszString )
{
	int nLength;

	for( nLength=0; pwszString[nLength]; nLength++ ) ;

	return nLength;	
}

// wide strcpy
wchar *fclib_wcscpy( wchar *pwszDest, cwchar *pwszSrc )
{
	wchar *pwszOrigDest = pwszDest;

	while( (*pwszDest++ = *pwszSrc++) ) ;
	return pwszOrigDest;
}

// wide strncpy
wchar *fclib_wcsncpy( wchar *pwszDest, cwchar *pwszSrc, int nNumChars )
{
	wchar *pwszOrigDest = pwszDest;

	while( *pwszSrc && nNumChars ) {
		*pwszDest++ = *pwszSrc++;
		nNumChars--;
	}
	while( nNumChars ) {
		*pwszDest++ = 0;
		nNumChars--;
	}
	return pwszOrigDest;	
}

// wide strcat
wchar *fclib_wcscat( wchar *pwszDest, cwchar *pwszSrc ) {
	wchar *pwszOrigDest = pwszDest;

	while( *pwszDest ) {
		pwszDest++;
	}

	while( (*pwszDest++ = *pwszSrc++) ) ;

	return pwszOrigDest;	
}

// Utility function to copy a 16 bit (wide) string to an 8 bit (char) string.
// Number of characters in destination string must match number of
// characters (not bytes) in source string, including null termination.
// i.e. if wide string is 16 characters (32 bytes) including the null
// character, then destination string must be 16 bytes.
void fclib_WideToChar( char *pszDest, cwchar *pwszSrc )
{
	while( *pszDest++ = (char) *pwszSrc++ ) ;
}

// Utility function to copy an 8 bit (char) string to a 16 bit (wide) string.
// There must be as many characters (not bytes) available in the destination
// string as in the source string, including null termination.  i.e. if the
// source string is 16 characters (16 bytes) including null, then the
// destination string must by 32 bytes.
void fclib_CharToWide( wchar *pwszDest, cchar *pszSrc )
{
	while( *pwszDest++ = (wchar) *pszSrc++ ) ;
}


// Attempts to locate the string pszString in the array of strings apszStringArray.
// If found, the index into the array is returned. If not found, -1 is returned.
// The array must be terminated by a NULL string pointer.
s32 fclib_FindStringInArray( cchar *pszString, cchar **apszStringArray ) {
	if( apszStringArray ) {
		s32 i;

		for( i=0; apszStringArray[i]; ++i ) {
			if( !fclib_stricmp( pszString, apszStringArray[i] ) ) {
				// Found the string...
				return i;
			}
		}
	}

	// String not found...

	return -1;
}

