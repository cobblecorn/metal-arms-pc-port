//////////////////////////////////////////////////////////////////////////////////////
// fGCvb.h - Fang GameCube vertex buffer module.
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

#ifndef _FGCVB_H_
#define _FGCVB_H_ 1

#include "fang.h"
#include "fmath.h"
#include "fgcdata.h"
#include "Dolphin\GX\GXEnum.h"


#define FGCVB_MAX_ST_SETS				4


//
// Position and normal are paired together to maximize ASM efficiency and minimize memory footprint
struct FGCSkinPosNorm_t
{
	s16	x,  y,  z;
	s16	nx, ny, nz;
};

/*
struct FGCBumpVert_t
{
	s16 x, y, z;
	s8	nx, ny, nz;
	s8	bx, by, bz;
	s8	tx, ty, tz;
};
*/

//
// For non-skinned verts, we separate the pos and norm
typedef struct FGCPosF32_s
{
	f32	x, y, z;

	void ChangeEndian( void )
	{
		x = fang_ConvertEndian( x );
		y = fang_ConvertEndian( y );
		z = fang_ConvertEndian( z );
	}
} FGCPosF32_t;

//
//
typedef struct FGCPosS16_s
{
	s16	x, y, z;

	void ChangeEndian( void )
	{
		x = fang_ConvertEndian( x );
		y = fang_ConvertEndian( y );
		z = fang_ConvertEndian( z );
	}
} FGCPosS16_t;

//
//
typedef struct FGCPosS8_s
{
	s8	x, y, z;
} FGCPosS8_t;

//
// Normal structure used for dynamic bump-mapping
typedef struct FGCNBT8_s
{
	s8	nx, ny, nz;
	s8	bx, by, bz;
	s8	tx, ty, tz;
} FGCNBT8_t;

//
//
typedef struct FGCColor_s
{
	u8	r, g, b, a;
} FGCColor_t;

//
// 8 bit UV's do not have enough resolution.  16 bit seems to be fine
typedef struct FGCST16_s
{
	s16	s, t;

	void ChangeEndian( void )
	{
		s = fang_ConvertEndian( s );
		t = fang_ConvertEndian( t );
	}
} FGCST16_t;

//
// Hopefully 8 bit weights will be enough.
typedef struct FGCWeights_s
{
	u8	w0, w1, w2, w3;
} FGCWeights_t;

//
// Structure that describes transforms for skin.
typedef struct FGCTransDesc_s
{
	u8	nMatrixCount;
	u8	__PAD;
	u16	nVertCount;
	u8	nMtxIdx[4];
	
	void ChangeEndian( void )
	{
		nVertCount = fang_ConvertEndian( nVertCount );
	};
	
} FGCTransDesc_t;

//
// Vertex Buffer flags
enum
{
	// If skinned, the position and normal are presumed 48-bits each
	FGCVB_SKINNED		= 0x01,	// position and normal composed of s16's
	// We assume fixed-point 16-bit normal, unless this flag is set:
	FGCVB_NORM_NBT		= 0x10,	// normal has binormal and tangent for bump-mapping
};
	
extern f32 FGCVB_fPosFracAdjust[16];

//
// GameCube "vertex buffer" format
typedef struct 
{
	u16				nFlags;			// See above.  Indicates format of vert attributes
	
	u16 			nPosCount;		// Number of positions in this vertex buffer
	u8 				nPosType;		// GC pos type (GX_F32, GX_S16, or GX_S8)
	u8 				nPosIdxType;	// GC position index type (GX_INDEX8 or GX_INDEX16)
	u8 				nPosStride;		// Byte size of the position vector
	u8				nPosFrac;		// Number of bits in the fractional component of position

	u16 			nDiffuseCount;	// Number of unique diffuse colors in this vertex buffer
	u8 				nColorIdxType;	// GC color index type (GX_INDEX8 or GX_INDEX16)
	
	u8				nGCVertexFormat;
	
	void			*pPosition;		// Pointer to the position data (in the case of skinned, position and normal)
	FGCColor_t		*pDiffuse;		// Pointer to the diffuse color data (if any)
	FGCST16_t		*pST;			// Pointer to the ST data
	FGCNBT8_t		*pNBT;			// For bumpmapped objects, Pointer to the normal, binormal and tangents

	void GetPoint( u32 nIdx, CFVec3 &vResult )
	{
		f32 fAdjust = FGCVB_fPosFracAdjust[nPosFrac];
		switch ( nPosType )
		{
			case GX_S8:		
			{
				vResult.x = (f32)((FGCPosS8_t *)pPosition)[nIdx].x * fAdjust;
				vResult.y = (f32)((FGCPosS8_t *)pPosition)[nIdx].y * fAdjust;
				vResult.z = (f32)((FGCPosS8_t *)pPosition)[nIdx].z * fAdjust;
				return;
			}
			
			case GX_S16:
			{
				vResult.x = (f32)((FGCPosS16_t *)pPosition)[nIdx].x * fAdjust;
				vResult.y = (f32)((FGCPosS16_t *)pPosition)[nIdx].y * fAdjust;
				vResult.z = (f32)((FGCPosS16_t *)pPosition)[nIdx].z * fAdjust;
				return;
			}
			
			case GX_F32:
			{
				vResult.x = ((FGCPosF32_t *)pPosition)[nIdx].x;
				vResult.y = ((FGCPosF32_t *)pPosition)[nIdx].y;
				vResult.z = ((FGCPosF32_t *)pPosition)[nIdx].z;
				return;
			}
			
			default:
			{
				FASSERT_NOW;
				break;
			}
		}
		
		return;
	}
	
	void ChangeEndian( void )
	{
		nFlags = fang_ConvertEndian( nFlags );
		nPosCount = fang_ConvertEndian( nPosCount );
		nDiffuseCount = fang_ConvertEndian( nDiffuseCount );
		pPosition = fang_ConvertEndian( pPosition );
		pDiffuse = (FGCColor_t *)fang_ConvertEndian( pDiffuse );
		pST = (FGCST16_t *)fang_ConvertEndian( pST );
		pNBT = (FGCNBT8_t *)fang_ConvertEndian( pNBT );
	}

} FGCVB_t;


extern BOOL fgcvb_ModuleStartup( void );
extern void fgcvb_ModuleShutdown( void );


//////////////////////////////////////////////////////////////////////////
//
//	Skinned Vertex Buffer allocation functions and data
//
//

#define	FGCVB_SKIN_VERT_BUFFER_SIZE		512


extern u16 FGCVB_nSkinVertCurrWM;
extern u16 FGCVB_nSkinVertCurrBuffer;
extern FGCSkinPosNorm_t *FGCVB_pSkinVertBuffer[2];
extern FGCSkinPosNorm_t *FGCVB_pLastVertBufferAllocated;

//
//
FINLINE u16 FGCVB_AllocateVertBuffers( u32 nVtxCount, FGCSkinPosNorm_t **pVertBuffer )
{
	// Make sure we don't exceed the size of the buffer
	if ( FGCVB_nSkinVertCurrWM + nVtxCount > FGCVB_SKIN_VERT_BUFFER_SIZE )
	{
		FASSERT_NOW;
		return 0xffff;
	}
	
	*pVertBuffer = &FGCVB_pSkinVertBuffer[FGCVB_nSkinVertCurrBuffer][FGCVB_nSkinVertCurrWM];
	FGCVB_pLastVertBufferAllocated = *pVertBuffer;
	
	// Increment our counter
	FGCVB_nSkinVertCurrWM += (u16)nVtxCount;
	
	return (u16)(FGCVB_nSkinVertCurrWM - nVtxCount);
}

extern void fgcvb_Swap( void );


extern FGCVB_t *FGCVB_pCurrentVB;


#endif

