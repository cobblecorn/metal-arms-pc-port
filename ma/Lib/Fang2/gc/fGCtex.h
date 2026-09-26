//////////////////////////////////////////////////////////////////////////////////////
// fGCtex.h - Fang texture module (GameCube version).
//
// Author: John Lafleur
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
// 02/18/02	Lafleur		Created from stubbed DX version
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FGCTEX_H_
#define _FGCTEX_H_ 1

#include "fang.h"
#include "fGC.h"
#include "ftex.h"
#include "flinklist.h"
#include "fperf.h"


enum 
{
	FGCTEXFLAGS_RUNTIME		= 0x01,		// Texture was created at runtime, not loaded

	FGCTEXFLAGS_NONE		= 0x00
};


//
//
//
struct _FTexData_s 
{
	FTexDef_t TexDef;					// Public texture definition
	FLink_t Link;						// Link to other texture resources

	u8 nFlags;							// See FDX8TEXFLAGS_* for info
	u8 nLodCount;						// LOD count
	u16 nWidth;							// texture width
	u16 nHeight;						// texture height
	GXTexFmt nGCTexFormat;				// GC texel format used for this texture
	GXTexObj *pGCTexObj;				// Pointer to the GC platform texture object
	u32 nAttachedStages;				// Set bits indicate which stages this texture is selected into (0=none)
	u32 nTextureBytes;					// Approximate bytes consumed by this texture

	void *pRawTexture;					// Pointer to raw texture data

	#if FPERF_ENABLE
		u32 nPerfKey;					// Used to count the number of unique textures used
	#endif
};


extern u32 FGCTex_nStageCount;			// Number of texture stages for this hardware
extern f32 fgctex_fCopyMul;


extern void fgctex_InitGCTexObj( GXTexObj **pTexObj, CFTexInst *pTexInst ) ;
extern void _Swizzle_I8( s32 width, s32 height, void *pImage );
extern void _Swizzle_IA8( s32 width, s32 height, void *pImage );

FINLINE void fgctex_SetCopyMul(f32 fMul) { fgctex_fCopyMul = fMul; }

#endif
