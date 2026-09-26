//////////////////////////////////////////////////////////////////////////////////////
// fdx8tex.h - Fang texture module (DX8 version).
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
// 10/02/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FDX8TEX_H_
#define _FDX8TEX_H_ 1

#include "fang.h"
#include "fdx8.h"
#include "fperf.h"
#include "ftex.h"


enum {
	FDX8TEXFLAGS_DXT				= 0x01,		// DXT texture

	FDX8TEXFLAGS_NOT_IN_CACHE		= 0x01,		// For streaming textures, this indicates that the texture is not in the cache
	FDX8TEXFLAGS_NONE				= 0x00
};


struct _FTexData_s {
	FTexDef_t TexDef;					// Public texture definition
	FLink_t Link;						// Link to other texture resources

	u8 nFlags;							// See FDX8TEXFLAGS_* for info
	u8 nD3DLodCount;					// D3D LOD count
	u16 nD3DWidth;						// D3D texture width
	u16 nD3DHeight;						// D3D texture height
	D3DFORMAT nD3DFormatColor;			// D3D texel format used for this texture
	D3DFORMAT nD3DFormatDepth;			// D3D depth/stencil format (render targets only)
	u32 nAttachedStages;				// Set bits indicate which stages this texture is selected into (0=none)
	u32 nTextureBytes;					// Approximate bytes consumed by this texture

	void *pStreamingHandle;
	void *pImageData;					// Pointer to the image data for the D3D texture if load-in-place
	IDirect3DTexture8 *pD3DTexture;		// Pointer to D3D texture object
	IDirect3DSurface8 *pD3DDepthStencil;// Pointer to D3D depth-stencil surface (NULL=none)

	#if FPERF_ENABLE
		u32 nPerfKey;					// Used to count the number of unique textures used
	#endif
};


extern u32 FDX8Tex_nStageCount;			// Number of texture stages for this hardware


extern void fdx8tex_SetTexture( u32 nStageNum, CFTexInst *pTexInst, u32 nTCIndex );
extern void fdx8tex_CopyRectFromBackBuffer( s32 nSrcUpperLeftX, s32 nSrcUpperLeftY, FTexDef_t *pDstRenderTarget );
extern void fdx8tex_UncacheSelected( void );

extern BOOL fdx8tex_GetCubeMapParam(u8& nCubeView);

#endif

