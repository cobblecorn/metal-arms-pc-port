// Linux stand-in for the MSVC debug CRT, for the math differential test.
#pragma once
#include <stdio.h>
#include <stdlib.h>
#define _CRT_ASSERT 2
#define _CRT_WARN 0
#define _CRT_ERROR 1
static inline int _CrtDbgReport( int, const char *pszFile, int nLine, const char *, const char *pszMsg, ... ) { fprintf( stderr, "ASSERT %s(%d): %s\n", pszFile, nLine, pszMsg ? pszMsg : "" ); return 0; }
#define _CrtDbgBreak() ((void)0)
#define _ASSERTE(expr) ((void)0)
#define _ASSERT(expr) ((void)0)
