//////////////////////////////////////////////////////////////////////////////////////
// fGC.h - Fang commonly used GameCube variables.
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
// 02/15/02 Lafleur		Created from stubbed DX version
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FGC_H_
#define _FGC_H_ 1

#include "fang.h"
#include "famem.h"
#include "fmath.h"
#include "dolphin.h"

#define FGC_MAX_TEX_STAGES			8
#define FGC_MAX_LIGHTS				8
#define FGC_MAX_ZBIAS				16
#define FGC_MAX_NUM_ARAM_BLOCKS		FAMEM_MAX_ALLOCATIONS + 4



#define FGC_ALIGN(num) __attribute__ ((aligned (num)))

extern OSHeapHandle FGC_HeapHandle;
extern BOOL FGC_bMemoryInitialized;
extern BOOL	FGC_DEVKitInExtendedMemoryMode;	// Dev Kit is not set to use the retail limited 24MB's of RAM
extern BOOL FGC_FlipCullMode;

extern Mtx44	FGC_Mtx44Identity;
extern Mtx		FGC_MtxIdentity;
extern Mtx		FGC_MtxLeftToRightIdentity;

extern void fgc_Init( void );
extern BOOL fgc_Startup( void );
extern void fgc_InitRenderState( void );
extern void fgc_Shutdown( void );
extern void fgc_InitializeMemory( void );


///////////////////////////////////////////////////////////////////////
//
//	Render state management functions for GameCube
//	
//
//

#define FGC_CACHE_STATES	TRUE

extern GXCullMode	FGC_CullMode;
extern GXBool		FGC_ClipMode;
extern GXBool		FGC_bDither;
extern GXBool		FGC_bZRead;
extern GXCompare	FGC_ZCompareMode;
extern GXBool		FGC_bZWrite;
extern GXBool 		FGC_bZComparePriorToTEV;
extern GXBool 		FGC_bColorBufferUpdate;
extern GXBlendMode		FGC_BlendMode;
extern GXBlendFactor	FGC_SourceFactor;
extern GXBlendFactor	FGC_DestFactor;
extern GXLogicOp		FGC_BitLogicOp;
extern GXCompare 		FGC_AlphaCompare1;
extern u8 				FGC_nAlphaCompareRef1;
extern GXAlphaOp 		FGC_AlphaCompareOp;
extern GXCompare 		FGC_AlphaCompare2;
extern u8 				FGC_nAlphaCompareRef2;
extern GXAttrType 		FGC_CurrPosIdxType;
extern GXCompType 		FGC_CurrPosType[GX_MAX_VTXFMT];
extern u8 				FGC_CurrPosFrac[GX_MAX_VTXFMT];
extern GXAttrType 		FGC_CurrNBTIdxType;
extern GXAttrType 		FGC_CurrNrmIdxType;
extern GXCompType		FGC_CurrNrmType[GX_MAX_VTXFMT];
extern u8 				FGC_CurrNrmFrac[GX_MAX_VTXFMT];
extern GXCompCnt		FGC_CurrNrmCompCnt[GX_MAX_VTXFMT];
extern GXAttrType		FGC_CurrSTIdxType[8];
extern GXCompType		FGC_SetVtxSTType[GX_MAX_VTXFMT][8];
extern u8				FGC_SetVtxSTFrac[GX_MAX_VTXFMT][8];
extern GXAttrType		FGC_CurrClrIdxType[2];
extern u32				FGC_CurrChannelCount;

extern BOOL8			FGC_bLazyRegisterSet;
extern BOOL8 			FGC_bUseVertexColor;
extern GXColor 			FGC_gxWhite;
extern GXColor 			FGC_gxBlack;


extern BOOL fgc_SetChannelsForShaders( BOOL8 bUseVertexColor, u8 nLightCount, BOOL8 bUseChannel2, BOOL8 bSpecular );
extern BOOL fgc_SetChannelsForFDraw( void );
extern BOOL fgc_SetChannelsForPointSprites( void );
extern BOOL fgc_SetChannelsForShadows( u32 nShadowCount );
extern BOOL fgc_SetChannelsForPerPixelSpot( u32 nPerPixelSpotCount );
extern BOOL fgc_DisableChannels( void );

FINLINE GXCullMode fgc_GetCullMode()
{
	return (FGC_CullMode);
}

FINLINE void fgc_FlipCullDir(BOOL bFlip)
{ 
	FGC_FlipCullMode = bFlip;
}

//
//
FINLINE void fgc_SetCullMode( GXCullMode mode )
{
#if FGC_CACHE_STATES
	if ( FGC_CullMode == mode )
		return;
#endif

	if (FGC_FlipCullMode)
	{
		if (mode == GX_CULL_FRONT) { mode = GX_CULL_BACK; }
		else if (mode == GX_CULL_BACK) { mode = GX_CULL_FRONT; }
	}
	
	FGC_CullMode = mode;
	
	GXSetCullMode( mode );
}


//
//
FINLINE void fgc_SetClipMode( GXBool bEnable )
{
#if FGC_CACHE_STATES
	if ( FGC_ClipMode == bEnable )
		return;
#endif

	FGC_ClipMode = bEnable;

	// GC Clip mode is enable = 0, disable = 1 - screwy
	GXSetClipMode( (GXClipMode)!FGC_ClipMode );
}

//
//
FINLINE void fgc_SetDitherMode( GXBool bEnable )
{
#if FGC_CACHE_STATES
	if ( FGC_bDither == bEnable )
		return;
#endif

	FGC_bDither = bEnable;

	GXSetDither( bEnable );
}

//
//
FINLINE void fgc_SetZMode( GXBool bEnableRead, GXCompare mode, GXBool bEnableWrite )
{
#if FGC_CACHE_STATES
	if ( FGC_bZRead == bEnableRead && FGC_ZCompareMode == mode && FGC_bZWrite == bEnableWrite )
		return;
#endif
	
	FGC_bZRead = bEnableRead;
	FGC_ZCompareMode = mode;
	FGC_bZWrite = bEnableWrite;

	GXSetZMode( FGC_bZRead, FGC_ZCompareMode, FGC_bZWrite  );
}

//
//
FINLINE void fgc_SetZReadMode( GXBool bEnable )
{
#if FGC_CACHE_STATES
	if ( FGC_bZRead == bEnable )
		return;
#endif
	
	FGC_bZRead = bEnable;

	GXSetZMode( FGC_bZRead, FGC_ZCompareMode, FGC_bZWrite  );
}

//
//
FINLINE void fgc_SetZCompareMode( GXCompare mode )
{
#if FGC_CACHE_STATES
	if ( FGC_ZCompareMode == mode )
		return;
#endif	

	FGC_ZCompareMode = mode;

	GXSetZMode( FGC_bZRead, FGC_ZCompareMode, FGC_bZWrite  );
}

//
//
FINLINE void fgc_SetZWriteMode( GXBool bEnable )
{
#if FGC_CACHE_STATES
	if ( FGC_bZWrite == bEnable )
		return;
#endif
	
	FGC_bZWrite = bEnable;

	GXSetZMode( FGC_bZRead, FGC_ZCompareMode, FGC_bZWrite  );
}

//
//
FINLINE void fgc_SetZCompareOrder( GXBool bPriorToTeV )
{
#if FGC_CACHE_STATES
	if ( FGC_bZComparePriorToTEV == bPriorToTeV )
		return;
#endif	

	FGC_bZComparePriorToTEV = bPriorToTeV;

	GXSetZCompLoc( FGC_bZComparePriorToTEV );
}

//
//
FINLINE void fgc_SetColorBufferUpdate( GXBool bUpdate )
{
#if FGC_CACHE_STATES
	if ( FGC_bColorBufferUpdate == bUpdate )
		return;
#endif	

	FGC_bColorBufferUpdate = bUpdate;

	GXSetColorUpdate( FGC_bColorBufferUpdate );
}

//
//
FINLINE void fgc_SetAlphaCompare( GXCompare nCompare1, u8 nAlphaRef1, GXAlphaOp nAOP, GXCompare nCompare2, u8 nAlphaRef2 )
{
#if FGC_CACHE_STATES
	if ( 	FGC_AlphaCompare1 == nCompare1 && FGC_nAlphaCompareRef1 == nAlphaRef1
		&&	FGC_AlphaCompareOp == nAOP
		&&  FGC_AlphaCompare2 == nCompare2 && FGC_nAlphaCompareRef2 == nAlphaRef2 )
		return;
#endif	

	FGC_AlphaCompare1 = nCompare1;
	FGC_nAlphaCompareRef1 = nAlphaRef1;
	FGC_AlphaCompareOp = nAOP;
	FGC_AlphaCompare2 = nCompare2;
	FGC_nAlphaCompareRef2 = nAlphaRef2;

	GXSetAlphaCompare( FGC_AlphaCompare1, FGC_nAlphaCompareRef1, FGC_AlphaCompareOp, FGC_AlphaCompare2, FGC_nAlphaCompareRef2 );
}

//
//
FINLINE void fgc_SetBlendOp( GXBlendMode bType, GXBlendFactor bSource, GXBlendFactor bDest, GXLogicOp bBitOp )
{
#if FGC_CACHE_STATES
	if ( 	FGC_BlendMode == bType && FGC_SourceFactor == bSource
		&&  FGC_DestFactor == bDest && FGC_BitLogicOp == bBitOp )
		return;
#endif	

	FGC_BlendMode = bType;
	FGC_SourceFactor = bSource;
	FGC_DestFactor = bDest;
	FGC_BitLogicOp = bBitOp;

	GXSetBlendMode( FGC_BlendMode, FGC_SourceFactor, FGC_DestFactor, FGC_BitLogicOp );
}

//
//
FINLINE void fgc_ClearVtxDesc( void )
{
	GXClearVtxDesc();
	FGC_CurrPosIdxType = GX_NONE;
	FGC_CurrNrmIdxType = GX_NONE;
	FGC_CurrClrIdxType[0] = GX_NONE;
	FGC_CurrClrIdxType[1] = GX_NONE;
	FGC_CurrSTIdxType[0] = GX_NONE;
	FGC_CurrSTIdxType[1] = GX_NONE;
	FGC_CurrSTIdxType[2] = GX_NONE;
	FGC_CurrSTIdxType[3] = GX_NONE;
	FGC_CurrSTIdxType[4] = GX_NONE;
	FGC_CurrSTIdxType[5] = GX_NONE;
	FGC_CurrSTIdxType[6] = GX_NONE;
	FGC_CurrSTIdxType[7] = GX_NONE;
}

//
//
FINLINE void fgc_SetVtxPosDesc( GXAttrType nAttrType )
{
#if FGC_CACHE_STATES
	if ( FGC_CurrPosIdxType == nAttrType )
	{
		return;
	}
#endif	
	FGC_bLazyRegisterSet = TRUE;
	FGC_CurrPosIdxType = nAttrType;
	GXSetVtxDesc( GX_VA_POS, FGC_CurrPosIdxType );
}

//
//
FINLINE void fgc_SetVtxNrmDesc( GXAttrType nAttrType )
{
#if FGC_CACHE_STATES
	if ( FGC_CurrNrmIdxType == nAttrType && FGC_CurrNBTIdxType == GX_NONE )
	{
		return;
	}
#endif	
	FGC_bLazyRegisterSet = TRUE;
	FGC_CurrNrmIdxType = nAttrType;
	GXSetVtxDesc( GX_VA_NRM, FGC_CurrNrmIdxType );
	if ( FGC_CurrNrmIdxType != GX_NONE && FGC_CurrNBTIdxType != GX_NONE )
	{
		FGC_CurrNBTIdxType = GX_NONE;
		GXSetVtxDesc( GX_VA_NBT, FGC_CurrNBTIdxType );
	}
}

FINLINE void fgc_SetVtxNBTDesc( GXAttrType nAttrType )
{
#if 0//FGC_CACHE_STATES
	if ( FGC_CurrNBTIdxType == nAttrType && FGC_CurrNrmIdxType == GX_NONE )
	{
		return;
	}
#endif	
	FGC_bLazyRegisterSet = TRUE;
	FGC_CurrNBTIdxType = nAttrType;
	GXSetVtxDesc( GX_VA_NBT, FGC_CurrNBTIdxType );
	if ( FGC_CurrNBTIdxType != GX_NONE && FGC_CurrNrmIdxType != GX_NONE )
	{
		FGC_CurrNrmIdxType = GX_NONE;
		GXSetVtxDesc( GX_VA_NRM, FGC_CurrNrmIdxType );
	}
}

//
//
FINLINE void fgc_SetVtxClrDesc( GXAttr nColorIdx, GXAttrType nAttrType )
{
#if FGC_CACHE_STATES
	if ( FGC_CurrClrIdxType[nColorIdx - GX_VA_CLR0] == nAttrType )
	{
		return;
	}
#endif	
	FGC_bLazyRegisterSet = TRUE;
	FGC_CurrClrIdxType[nColorIdx - GX_VA_CLR0] = nAttrType;
	GXSetVtxDesc( nColorIdx, nAttrType );
}

//
//
FINLINE void fgc_SetVtxSTDesc( GXAttr nSTIdx, GXAttrType nAttrType )
{
#if FGC_CACHE_STATES
	if ( FGC_CurrSTIdxType[nSTIdx - GX_VA_TEX0] == nAttrType )
	{
		return;
	}
#endif	
	FGC_bLazyRegisterSet = TRUE;
	FGC_CurrSTIdxType[nSTIdx - GX_VA_TEX0] = nAttrType;
	GXSetVtxDesc( nSTIdx, nAttrType );
}

//
//
FINLINE void fgc_SetVtxPosFormat( GXVtxFmt nVtxFmt, GXCompType nPosType, u8 nFrac )
{
	// All but the FGCDATA_VARIABLE_VERTEX_FORMAT are reserved on GC (See FGCVB.H)
	FASSERT( nVtxFmt == GX_VTXFMT7 );
#if FGC_CACHE_STATES
	if ( FGC_CurrPosType[nVtxFmt] == nPosType 
		&& FGC_CurrPosFrac[nVtxFmt] == nFrac )
	{
		return;
	}
#endif	
	FGC_bLazyRegisterSet = TRUE;
	FGC_CurrPosType[nVtxFmt] = nPosType;
	FGC_CurrPosFrac[nVtxFmt] = nFrac;
	GXSetVtxAttrFmt( nVtxFmt, GX_VA_POS, GX_POS_XYZ, nPosType, nFrac );
}

//
//
FINLINE void fgc_SetVtxNrmFormat( GXVtxFmt nVtxFmt, GXCompType nNrmType, GXCompCnt nCompCnt, u8 nFrac )
{
	// All but the FGCDATA_VARIABLE_VERTEX_FORMAT are reserved on GC (See FGCVB.H)
	FASSERT( nVtxFmt == GX_VTXFMT7 );
#if FGC_CACHE_STATES
	if ( FGC_CurrNrmType[nVtxFmt] == nNrmType 
		&& FGC_CurrNrmFrac[nVtxFmt] == nFrac 
		&& FGC_CurrNrmCompCnt[nVtxFmt] == nCompCnt )
	{
		return;
	}
#endif	
	FGC_bLazyRegisterSet = TRUE;
	FGC_CurrNrmType[nVtxFmt] = nNrmType;
	FGC_CurrNrmFrac[nVtxFmt] = nFrac;
	FGC_CurrNrmCompCnt[nVtxFmt] = nCompCnt;
	GXSetVtxAttrFmt( nVtxFmt, GX_VA_NRM, nCompCnt, nNrmType, nFrac );
}

//
//
FINLINE void fgc_SetVtxNBTFormat( GXVtxFmt nVtxFmt, GXCompType nNrmType, GXCompCnt nCompCnt, u8 nFrac )
{
	// All but the FGCDATA_VARIABLE_VERTEX_FORMAT are reserved on GC (See FGCVB.H)
	FASSERT( nVtxFmt == GX_VTXFMT7 );
#if FGC_CACHE_STATES
	if ( FGC_CurrNrmType[nVtxFmt] == nNrmType 
		&& FGC_CurrNrmFrac[nVtxFmt] == nFrac 
		&& FGC_CurrNrmCompCnt[nVtxFmt] == nCompCnt )
	{
		return;
	}
#endif	
	FGC_bLazyRegisterSet = TRUE;
	FGC_CurrNrmType[nVtxFmt] = nNrmType;
	FGC_CurrNrmFrac[nVtxFmt] = nFrac;
	FGC_CurrNrmCompCnt[nVtxFmt] = nCompCnt;
	GXSetVtxAttrFmt( nVtxFmt, GX_VA_NBT, nCompCnt, nNrmType, nFrac );
}

//
//
FINLINE void fgc_SetVtxSTFormat( GXVtxFmt nVtxFmt, GXAttr nSTIdx, GXCompType nNrmType, u8 nFrac )
{
	// All but the FGCDATA_VARIABLE_VERTEX_FORMAT are reserved on GC (See FGCVB.H)
	FASSERT( nVtxFmt == GX_VTXFMT7 );
#if FGC_CACHE_STATES
	if ( FGC_SetVtxSTType[nVtxFmt][nSTIdx - GX_VA_TEX0] == nNrmType 
		&& FGC_SetVtxSTFrac[nVtxFmt][nSTIdx - GX_VA_TEX0] == nFrac )
	{
		return;
	}
#endif	
	FGC_bLazyRegisterSet = TRUE;
	FGC_SetVtxSTType[nVtxFmt][nSTIdx - GX_VA_TEX0] = nNrmType;
	FGC_SetVtxSTFrac[nVtxFmt][nSTIdx - GX_VA_TEX0] = nFrac;
	GXSetVtxAttrFmt( nVtxFmt, nSTIdx, GX_TEX_ST, nNrmType, nFrac );
}

//
//
FINLINE void fgc_SetNumChannels( u32 nChannelCount )
{
#if FGC_CACHE_STATES
	if ( FGC_CurrChannelCount == nChannelCount ) 
	{
		return;
	}
#endif	
	FGC_bLazyRegisterSet = TRUE;
	FGC_CurrChannelCount = nChannelCount;
	GXSetNumChans( (u8)FGC_CurrChannelCount );
}



extern u32	FGC_nMaxLights;
extern u32	FGC_nStageCount;
extern void fgc_InitRenderState( void );


#endif

