//////////////////////////////////////////////////////////////////////////////////////
// fangtypes.h - General common types for the Fang system.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 08/28/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FANGTYPES_H_
#define _FANGTYPES_H_ 1

#if FANG_PLATFORM_GC
#include <stdio.h>
#include <extras.h> //for _wcsicmp
#include <wchar.h>
#endif
// Define these to control which definitions should not be defined:
//
// FANGTYPES_NODEF_NUMERIC
// FANGTYPES_NODEF_NULL
// FANGTYPES_NODEF_LOGICAL
// FANGTYPES_NODEF_BOOL
// FANGTYPES_NODEF_XBOOL
// FANGTYPES_NODEF_VARG
// FANGTYPES_NODEF_128


//-------------------------------------------------------
#ifndef FANGTYPES_NODEF_NUMERIC

	#if !FANG_PLATFORM_GC
//ARG - >>>>>
	#if FANG_PLATFORM_PS2
		typedef unsigned char u8;
		typedef unsigned short u16;
		typedef unsigned int u32;
		typedef signed char s8;
		typedef signed short s16;
		typedef signed int s32;
		typedef float f32;
		typedef __wchar_t wchar;
	#else
//ARG - <<<<<
		typedef unsigned __int8 u8;
		typedef unsigned __int16 u16;
		typedef unsigned __int32 u32;
		typedef __int8 s8;
		typedef __int16 s16;
		typedef __int32 s32;
		typedef float f32;
		typedef u16 wchar;
	#endif	//ARG
	#else
		#include "dolphin/types.h"
		typedef wchar_t wchar;
	#endif

	typedef const unsigned char cu8;
	typedef const unsigned short cu16;
	typedef const unsigned long cu32;
	typedef const char cs8;
	typedef const short cs16;
	typedef const long cs32;
	typedef const float cf32;

//	#if ! FANG_PLATFORM_PS2
//		typedef unsigned int uint;
//	#endif

	typedef const char cchar;
	typedef const wchar cwchar;

//	typedef const int cint;
//	typedef const unsigned int cuint;

	#if FANG_PLATFORM_PS2
		typedef unsigned long u64;
		#if !FANGTYPES_NODEF_128
			typedef u_long128 u128;
		#endif
		typedef signed long s64;
		typedef const unsigned long cu64;
		#if !FANGTYPES_NODEF_128
			typedef const u_long128 cu128;
		#endif
		typedef const signed long cs64;
	#else
		// Careful with these 64-bit types - they are not as fast as true native 32-bit types...
		#if ! FANG_PLATFORM_GC
			typedef unsigned __int64 u64;
			typedef signed __int64 s64;
		#else
			typedef unsigned long long u64;
			typedef signed long long s64;	
		#endif

		#if FANG_PLATFORM_GC
			typedef const u64 cu64;
			typedef const s64 cs64;
		#else
			typedef const unsigned __int64 cu64;
			typedef const signed __int64 cs64;
		#endif
		
		#if ! FANG_PLATFORM_GC
			typedef double f64;
		#endif

		typedef const double cf64;
	#endif
	
	#if FANG_ENABLE_SIMD
		#if FANG_PLATFORM_PS2
			#if !FANGTYPES_NODEF_128
				typedef __m128 u128;
			#endif
		#else
			#if !FANGTYPES_NODEF_128
				#include <xmmintrin.h>
				typedef __m128 qf128;
				typedef const __m128 cqf128;
			#endif
		#endif
	#endif

#endif




//-------------------------------------------------------
#ifndef FANGTYPES_NODEF_NULL

	/*#ifndef NULL
		#define NULL 0
	#endif*/
	
	#ifdef NULL
	#undef NULL
	#endif
	#define NULL 0

#endif




//-------------------------------------------------------
#ifndef FANGTYPES_NODEF_LOGICAL

	#ifndef FALSE
		#define FALSE 0
	#endif

	#ifndef TRUE
		#define TRUE 1
	#endif

#endif





//-------------------------------------------------------
#ifndef FANGTYPES_NODEF_BOOL

//ARG - >>>>>
	#if FANG_PLATFORM_PS2
	typedef int BOOL4;

	#define BOOL	BOOL4

	#else
//ARG - <<<<<
	typedef int BOOL;
	#endif	//ARG

#endif





//-------------------------------------------------------
#ifndef FANGTYPES_NODEF_XBOOL

	typedef s8 BOOL8;
	typedef const int CBOOL;
	typedef cs8 CBOOL8;

#endif





//-------------------------------------------------------
#ifndef FANGTYPES_NODEF_VARG

#if FANG_PLATFORM_GC
	#define _vsnprintf		vsnprintf
	#define _vsnwprintf		vswprintf
	#define _snwprintf		swprintf
	#define _wcsicmp		wcsicmp
	typedef char* FANG_VA_LIST;
	#define FANG_VA_START	va_start
	#define FANG_VA_END		va_end
	#define FANG_VA_ARG		va_arg
//ARG - >>>>>
#elif FANG_PLATFORM_PS2
	int _vsnprintf(char *string, int maxlen, const char *format, char *args);
	int _vsnwprintf(wchar *string, int maxlen, const wchar *format, char *args);
	int _snwprintf(wchar *string, int maxlen, const wchar *format, ...);
	typedef char* FANG_VA_LIST;
	#define FANG_VA_START	va_start
	#define FANG_VA_END		va_end
	#define FANG_VA_ARG		va_arg
//ARG - <<<<<
#else
	typedef char *FANG_VA_LIST;
	#define FANG_INTSIZEOF(n) ( (sizeof(n) + sizeof(s32) - 1) & ~(sizeof(s32) - 1) )
	#define FANG_VA_START(ap,v)  ( ap = (FANG_VA_LIST)&v + FANG_INTSIZEOF(v) )
	#define FANG_VA_ARG(ap,t)    ( *(t *)((ap += FANG_INTSIZEOF(t)) - FANG_INTSIZEOF(t)) )
	#define FANG_VA_END(ap)      ( ap = (FANG_VA_LIST)0 )
#endif // FANG_PLATFORM_GC

#endif // FANGTYPES_NODEF_VARG


#if FANG_PLATFORM_DX
	// Under windows we need to define these ps2 vars so that tools can use ps2 headers:

	typedef struct {
		u32 ulong128[4];
	} u_long128;

	typedef struct {
		f32 flong128[4];
	} sceGifTag;

	#if !FANGTYPES_NODEF_128
		typedef u_long128 u128;
		typedef const u_long128 cu128;
	#endif
#endif



#endif

