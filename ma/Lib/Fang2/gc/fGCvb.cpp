//////////////////////////////////////////////////////////////////////////////////////
// fGCvb.cpp - Fang GameCube vertex buffer module.
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

#include "fang.h"
#include "fGC.h"
#include "fGCvb.h"
#include "fGCvid.h"

#include "fmath.h"
#include "flinklist.h"
#include "fperf.h"
#include "fres.h"



#if FANG_DEBUG_BUILD
u32 FGCVB_nSkinVertHighWM;
#endif
u16 FGCVB_nSkinVertCurrWM;
u16 FGCVB_nSkinVertCurrBuffer;
FGCSkinPosNorm_t *FGCVB_pSkinVertBuffer[2];
FGCSkinPosNorm_t *FGCVB_pLastVertBufferAllocated;

static BOOL _bModuleInitialized;
static BOOL _bWindowCreated;

FGCVB_t *FGCVB_pCurrentVB;

static BOOL _WindowCreatedCallback( FGCVidEvent_e nEvent );


f32 FGCVB_fPosFracAdjust[16] =
{
	1.f,
	(1.f / 2.f),
	(1.f / 4.f),//0.25f;
	(1.f / 8.f),//0.125f;
	(1.f / 16.f),//0.0625f,
	(1.f / 32.f),//0.03125f,
	(1.f / 64.f),
	(1.f / 128.f),
	(1.f / 256.f),
	(1.f / 512.f),
	(1.f / 1024.f),
	(1.f / 2048.f),
	(1.f / 4096.f),
	(1.f / 8192.f),
	(1.f / 16384.f),
	(1.f / 32768.f)
};
		
	

//
//
//
BOOL fgcvb_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

	fgcvid_RegisterWindowCallbackFunction( _WindowCreatedCallback );
	
//	if ( sizeof( FGCVB_t ) != 64 )
//	{
//		DEVPRINTF( "fgcvb_ModuleStartup() - GC Vertex Buffer size now %d. Should be 64.\n", sizeof( FGCVB_t ) );
//		FASSERT_NOW;
//	}
	
	FGCVB_pSkinVertBuffer[0] = (FGCSkinPosNorm_t *)fres_AlignedAllocAndZero( FGCVB_SKIN_VERT_BUFFER_SIZE * sizeof(FGCSkinPosNorm_t), 8 );
	FGCVB_pSkinVertBuffer[1] = (FGCSkinPosNorm_t *)fres_AlignedAllocAndZero( FGCVB_SKIN_VERT_BUFFER_SIZE * sizeof(FGCSkinPosNorm_t), 8 );
	if ( !FGCVB_pSkinVertBuffer[0] || !FGCVB_pSkinVertBuffer[1] )
	{
		DEVPRINTF( "fgcvb_ModuleStartup() - Unable to allocate memory for GC skinned vertex buffers.\n", sizeof( FGCVB_t ) );
		return FALSE;
	}

	_bWindowCreated = FALSE;
	_bModuleInitialized = TRUE;
	
	FGCVB_pCurrentVB = NULL;
#if FANG_DEBUG_BUILD	
	FGCVB_nSkinVertHighWM = 0;
#endif
	FGCVB_nSkinVertCurrWM = 0;
	FGCVB_nSkinVertCurrBuffer = 0;
	FGCVB_pLastVertBufferAllocated = NULL;

	return TRUE;
}


//
//
//
void fgcvb_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

	fgcvid_UnregisterWindowCallbackFunction( _WindowCreatedCallback );

	_bModuleInitialized = FALSE;
}


//
//
//
void fgcvb_Swap( void ) 
{
	FASSERT( _bModuleInitialized );

#if FANG_DEBUG_BUILD
	if ( FGCVB_nSkinVertCurrWM > FGCVB_nSkinVertHighWM )
		FGCVB_nSkinVertHighWM = FGCVB_nSkinVertCurrWM;
#endif
	
	FGCVB_nSkinVertCurrWM = 0;
	FGCVB_nSkinVertCurrBuffer = FGCVB_nSkinVertCurrBuffer & 0x01;
	FGCVB_pLastVertBufferAllocated = NULL;
	FASSERT( FGCVB_nSkinVertCurrBuffer == 1 || FGCVB_nSkinVertCurrBuffer == 0 );
}


//
//	This will override the cache settings
//
static void _SetupVertexFormats( void ) 
{
	fgc_ClearVtxDesc();
	
#if 0	
	u32 i, ii;
	for ( i = 0; i < FGCDATA_FIXED_VERTEX_FORMATS; i++ )
	{
		if ( FGCData_VertexFormatDesc[i].nPosBitDepth == 32 )
		{
			fgc_SetVtxPosFormat( (GXVtxFmt)(GX_VTXFMT0 + i), GX_F32, 0 );
		}
		else if ( FGCData_VertexFormatDesc[i].nPosBitDepth == 16 )
		{
			fgc_SetVtxPosFormat( (GXVtxFmt)(GX_VTXFMT0 + i), GX_S16, FGCData_VertexFormatDesc[i].nPosFracBits );
		}
		else if ( FGCData_VertexFormatDesc[i].nPosBitDepth == 8 )
		{
			fgc_SetVtxPosFormat( (GXVtxFmt)(GX_VTXFMT0 + i), GX_S8, FGCData_VertexFormatDesc[i].nPosFracBits );
		}
		else
		{
			FASSERT_NOW;
		}
		
		if ( FGCData_VertexFormatDesc[i].nNrmBitDepth == 8 )
		{
			fgc_SetVtxNrmFormat( (GXVtxFmt)(GX_VTXFMT0 + i), GX_S8, FGCData_VertexFormatDesc[i].nNrmFracBits );
		}
		else
		{
			FASSERT_NOW;
		}
		
		GXSetVtxAttrFmt( (GXVtxFmt)(GX_VTXFMT0 + i), GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0 );
		GXSetVtxAttrFmt( (GXVtxFmt)(GX_VTXFMT0 + i), GX_VA_CLR1, GX_CLR_RGBA, GX_RGBA8, 0 );
		
		if ( FGCData_VertexFormatDesc[i].nSTsBitDepth == 16 )
		{
			for ( ii = 0; ii < 8; ii++ )
			{
				fgc_SetVtxSTFormat( (GXVtxFmt)(GX_VTXFMT0 + i), (GXAttr)(GX_VA_TEX0 + ii), GX_S16, FGCData_VertexFormatDesc[i].nSTsFracBits );
			}
		}
		else
		{
			FASSERT_NOW;
		}
	}
#else
	u32 i, ii;
	for ( i = 0; i < FGCDATA_FIXED_VERTEX_FORMATS; i++ )
	{
		if ( FGCData_VertexFormatDesc[i].nPosBitDepth == 32 )
		{
			FGC_CurrPosType[i] = GX_F32;
			FGC_CurrPosFrac[i] = 0;
			GXSetVtxAttrFmt( (GXVtxFmt)(GX_VTXFMT0 + i), GX_VA_POS, GX_POS_XYZ, GX_F32, 0 );
		}
		else if ( FGCData_VertexFormatDesc[i].nPosBitDepth == 16 )
		{
			FGC_CurrPosType[i] = GX_S16;
			FGC_CurrPosFrac[i] = FGCData_VertexFormatDesc[i].nPosFracBits;
			GXSetVtxAttrFmt( (GXVtxFmt)(GX_VTXFMT0 + i), GX_VA_POS, GX_POS_XYZ, GX_S16, FGCData_VertexFormatDesc[i].nPosFracBits );
		}
		else if ( FGCData_VertexFormatDesc[i].nPosBitDepth == 8 )
		{
			FGC_CurrPosType[i] = GX_S8;
			FGC_CurrPosFrac[i] = FGCData_VertexFormatDesc[i].nPosFracBits;
			GXSetVtxAttrFmt( (GXVtxFmt)(GX_VTXFMT0 + i), GX_VA_POS, GX_POS_XYZ, GX_S8, FGCData_VertexFormatDesc[i].nPosFracBits );
		}
		else
		{
			FASSERT_NOW;
		}
		
		if ( FGCData_VertexFormatDesc[i].nNrmBitDepth == 8 )
		{
			FGC_CurrNrmType[i] = GX_S8;
			FGC_CurrNrmFrac[i] = FGCData_VertexFormatDesc[i].nNrmFracBits;
			GXSetVtxAttrFmt( (GXVtxFmt)(GX_VTXFMT0 + i), GX_VA_NRM, GX_NRM_XYZ, GX_S8, FGCData_VertexFormatDesc[i].nNrmFracBits );
		}
		else
		{
			FASSERT_NOW;
		}
		
		GXSetVtxAttrFmt( (GXVtxFmt)(GX_VTXFMT0 + i), GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0 );
		GXSetVtxAttrFmt( (GXVtxFmt)(GX_VTXFMT0 + i), GX_VA_CLR1, GX_CLR_RGBA, GX_RGBA8, 0 );
		
		if ( FGCData_VertexFormatDesc[i].nSTsBitDepth == 16 )
		{
			for ( ii = 0; ii < 8; ii++ )
			{
				FGC_SetVtxSTType[i][ii] = GX_S16;
				FGC_SetVtxSTFrac[i][ii] = FGCData_VertexFormatDesc[i].nSTsFracBits;
				GXSetVtxAttrFmt( (GXVtxFmt)(GX_VTXFMT0 + i), (GXAttr)(GX_VA_TEX0 + ii), GX_TEX_ST, GX_S16, FGCData_VertexFormatDesc[i].nSTsFracBits );
			}
		}
		else
		{
			FASSERT_NOW;
		}
	}
#endif
	
	GXSetVtxAttrFmt( FGCDATA_VARIABLE_VERTEX_FORMAT, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0 );
	GXSetVtxAttrFmt( FGCDATA_VARIABLE_VERTEX_FORMAT, GX_VA_CLR1, GX_CLR_RGBA, GX_RGBA8, 0 );
	for ( ii = 0; ii < 8; ii++ )
	{
		fgc_SetVtxSTFormat( FGCDATA_VARIABLE_VERTEX_FORMAT, GXAttr(GX_VA_TEX0 + ii), GX_S16, 8 );
	}
}


//
//
//
static BOOL _WindowCreatedCallback( FGCVidEvent_e nEvent ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( nEvent>=0 && nEvent<FGCVID_EVENT_COUNT );

	switch( nEvent ) 
	{
		case FGCVID_EVENT_WINDOW_CREATED:
			_SetupVertexFormats();
			_bWindowCreated = TRUE;
			break;

		case FGCVID_EVENT_WINDOW_DESTROYED:
			_bWindowCreated = FALSE;

			break;

		case FGCVID_EVENT_PRE_RESET:
			break;

		case FGCVID_EVENT_POST_RESET:
			break;
	}

	return TRUE;
}

