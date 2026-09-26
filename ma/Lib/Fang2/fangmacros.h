//////////////////////////////////////////////////////////////////////////////////////
// fangmacros.h - General purpose macros for the Fang system.
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
// 08/29/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FANGMACROS_H_
#define _FANGMACROS_H_ 1

// Returns the byte offset of the specified field from the start of the structure:
#define FANG_OFFSETOF( StructureName, FieldName ) ( (u32)( &(((StructureName *)0)->FieldName) ) )

// Helper functions to stuff a f32 into a u32 argument (and other variations):
#if FANG_PLATFORM_GC
	// Prototyping only to remove a Codewarrior warning:
	inline u32 FANG_F32TOU32( f32 f );
	inline s32 FANG_F32TOS32( f32 f );
	inline f32 FANG_U32TOF32( u32 n );
	inline f32 FANG_S32TOF32( s32 n );
#endif

inline u32 FANG_F32TOU32( f32 f ) { return *((u32*)&f); }
inline s32 FANG_F32TOS32( f32 f ) { return *((s32*)&f); }
inline f32 FANG_U32TOF32( u32 n ) { return *((f32*)&n); }
inline f32 FANG_S32TOF32( s32 n ) { return *((f32*)&n); }

// These are here because you never know what the compiler will do with a structure assignment:
#define ALIGNED_MEMCPY4(pDest, pSource, uCount) \
		{ \
			register u32	uModCount = (uCount >> 2); \
			register u32	*pRealDest = (u32 *)pDest; \
			register u32	*pRealSource = (u32 *)pSource; \
			while(uModCount--) {*pRealDest++ = *pRealSource++;} \
		}

#define ALIGNED_MEMCPY8(pDest, pSource, uCount) \
		{ \
			register u32	uModCount = (uCount >> 3); \
			register u64	*pRealDest = (u64 *)pDest; \
			register u64	*pRealSource = (u64 *)pSource; \
			while(uModCount--) {*pRealDest++ = *pRealSource++;} \
		}

#define ALIGNED_MEMCPY16(pDest, pSource, uCount) \
		{ \
			register u32	uModCount = (uCount >> 4); \
			register u128	*pRealDest = (u128 *)pDest; \
			register u128	*pRealSource = (u128 *)pSource; \
			while(uModCount--) {*pRealDest++ = *pRealSource++;} \
		}

#endif

