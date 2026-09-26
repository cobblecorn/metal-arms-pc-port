//////////////////////////////////////////////////////////////////////////////////////
// fGCxfm.h - Fang Transformation module.
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

#ifndef _FGCXFM_H_
#define _FGCXFM_H_ 1

#include "fang.h"
#include "fxfm.h"
#include "fGC.h"

#include "fgcsh.h"

typedef f32 Mtx33[3][3];
typedef f32 Mtx24[2][4];

Mtx FXfm_GCInvView;

#define FGCXFM_MTX_BUFFER_SIZE			1152
#define FGCXFM_POS_MTX_FLOAT_COUNT		12
#define FGCXFM_NRM_MTX_FLOAT_COUNT		9
#define FGCXFM_TEX_MTX_FLOAT_COUNT		8

extern CFMtx43A FGCXfm_mtxGCLeftToRightHandConversionMtx;
extern CFMtx43A FGCXfm_mtxGCLeftToRightHandViewMtx;


extern BOOL fgcxfm_ModuleStartup( void );
extern void fgcxfm_ModuleShutdown( void );

extern void fgcxfm_SetGCMatrices( void );
extern void fgcxfm_SetViewGCMatrix( void );
extern void fgcxfm_SetModelGCMatrix( void );
extern void fgcxfm_SetGCMatricesIdentity( void );
extern void fgcxfm_SetCustomGCMatrix( const CFMtx43A *pMtx43 );
//extern void fgcxfm_SetCustomGCMatrix( GXPosNrmMtx nGCMtxRegister, const Mtx *pMtx );
extern void fgcxfm_SetGCMatricesModelIdentity( void );
extern void fgcxfm_InvalidateLocalMatrixCache( void );
extern void fgcxfm_Swap( void );

extern u32 FGCXfm_nLastMtxFlushIndex;
extern u32 FGCXfm_nCurrentMtxIndex;
extern u32 FGCXfm_nCurrentBuffer;
extern Mtx   *FGCXfm_apPosMtxBuffer[2];
extern Mtx33 *FGCXfm_apNrmMtxBuffer[2];
extern u8	 *FGCXfm_panMtxSlotInGP;
#if !FANG_PRODUCTION_BUILD
	extern u32 FGCXfm_nMatrixCountExceedingBuffer;
#endif
extern u16 FGCXfm_CurrentMtxInGP[10];
extern u8  FGCXfm_nCurrentGPMatrix;

//
//
FINLINE void fgcxfm_LoadGCPosMatrixImm( Mtx &PosMatrix, GXPosNrmMtx nGCMtxRegister ) 
{
	fgcxfm_InvalidateLocalMatrixCache();
	
	// Set the model matrix
	GXLoadPosMtxImm( PosMatrix, nGCMtxRegister );
}

//
//
FINLINE void fgcxfm_SetGCCurrentMatrix( GXPosNrmMtx nGCMtxRegister ) 
{
	GXSetCurrentMtx( nGCMtxRegister );
}

//
//
FINLINE void fgcxfm_LoadMatrixToGP( u16 nMtxBufferIdx, u16 nGPMatrixIdx )
{
	if ( FGCXfm_CurrentMtxInGP[nGPMatrixIdx] != 0xffff )
	{
		// Tell the matrix currently in the GP that it no longer is
		FGCXfm_panMtxSlotInGP[FGCXfm_CurrentMtxInGP[nGPMatrixIdx]] = 0xff;
	}
	
	// Tell the matrix going into the GP where it resides, there
	if ( FGCXfm_panMtxSlotInGP[nMtxBufferIdx] != 0xff )
	{
		FGCXfm_CurrentMtxInGP[FGCXfm_panMtxSlotInGP[nMtxBufferIdx]] = 0xffff;
	}
	FGCXfm_panMtxSlotInGP[nMtxBufferIdx] = nGPMatrixIdx;
	
	// Tell the GP slot what matrix it contains
	FGCXfm_CurrentMtxInGP[nGPMatrixIdx] = nMtxBufferIdx;
	
	// Do the load
	GXLoadPosMtxIndx( nMtxBufferIdx, GX_PNMTX0 + (nGPMatrixIdx * 3) );
	GXLoadNrmMtxIndx3x3( nMtxBufferIdx, GX_PNMTX0 + (nGPMatrixIdx * 3) );
	
	fgcsh_SetCurrentNrmlMtxIdx( nMtxBufferIdx );
}

//
//
FINLINE BOOL fgcxfm_SetCurrentGPMatrix( u16 nMtxBufferIdx )
{
	u8 nMatrixGPIdx = FGCXfm_panMtxSlotInGP[nMtxBufferIdx];
	if ( nMatrixGPIdx == 0xff )
	{
		return FALSE;
	}
	
	// Matrix is already loaded
	if ( FGCXfm_nCurrentGPMatrix != nMatrixGPIdx )
	{
		// The matrix needs to be set as the current GP matrix
		GXSetCurrentMtx( nMatrixGPIdx * 3 );
		FGCXfm_nCurrentGPMatrix = nMatrixGPIdx;
	}
	
	fgcsh_SetCurrentPosMtx( nMatrixGPIdx*3 );
	
	return TRUE;
}

//
//
FINLINE u16 fgcxfm_AllocateMtxBuffers( u32 nMtxCount, Mtx **pPosMtxBuffer, Mtx33 **pNrmMtxBuffer )
{
	// Make sure we don't exceed the size of the ring buffer
	if ( FGCXfm_nCurrentMtxIndex + nMtxCount > FGCXFM_MTX_BUFFER_SIZE )
	{
		#if !FANG_PRODUCTION_BUILD
			if ( FGCXfm_nMatrixCountExceedingBuffer )
			{
				FGCXfm_nMatrixCountExceedingBuffer += nMtxCount;
			}
			else
			{
				FGCXfm_nMatrixCountExceedingBuffer += nMtxCount - (FGCXFM_MTX_BUFFER_SIZE - FGCXfm_nCurrentMtxIndex);
			}
		#endif
		
		return 0xffff;
	}

	*pPosMtxBuffer = &FGCXfm_apPosMtxBuffer[FGCXfm_nCurrentBuffer][FGCXfm_nCurrentMtxIndex];
	*pNrmMtxBuffer = &FGCXfm_apNrmMtxBuffer[FGCXfm_nCurrentBuffer][FGCXfm_nCurrentMtxIndex];
	FGCXfm_nCurrentMtxIndex += nMtxCount;

	// Currently we have to keep this 32 byte
	// aligned to avoid issues with DCFlushRange.  Eventually, if we
	// precalculate ALL of the matrices before render, we could flush
	// all of them at once, which would be desireable.
//	FGCXfm_nCurrentMtxIndex = FMATH_BYTE_ALIGN_UP( FGCXfm_nCurrentMtxIndex, 32);

	return (FGCXfm_nCurrentMtxIndex - nMtxCount);
}

//
//
FINLINE void fgcxfm_FlushMatrixBufferMemory( BOOL bWithSync )
{
	FASSERT( FGCXfm_nLastMtxFlushIndex <= FGCXfm_nCurrentMtxIndex );
	if ( FGCXfm_nLastMtxFlushIndex == FGCXfm_nCurrentMtxIndex )
	{
		return;
	}
	
	if ( bWithSync )
	{
		DCFlushRange( &FGCXfm_apPosMtxBuffer[FGCXfm_nCurrentBuffer][FGCXfm_nLastMtxFlushIndex], FMATH_BYTE_ALIGN_UP((FGCXfm_nCurrentMtxIndex - FGCXfm_nLastMtxFlushIndex) * sizeof( Mtx ), 32) );
		DCFlushRange( &FGCXfm_apNrmMtxBuffer[FGCXfm_nCurrentBuffer][FGCXfm_nLastMtxFlushIndex], FMATH_BYTE_ALIGN_UP((FGCXfm_nCurrentMtxIndex - FGCXfm_nLastMtxFlushIndex) * sizeof( Mtx33 ), 32) );
	}
	else
	{
		DCFlushRangeNoSync( &FGCXfm_apPosMtxBuffer[FGCXfm_nCurrentBuffer][FGCXfm_nLastMtxFlushIndex], FMATH_BYTE_ALIGN_UP((FGCXfm_nCurrentMtxIndex - FGCXfm_nLastMtxFlushIndex) * sizeof( Mtx ), 32) );
		DCFlushRangeNoSync( &FGCXfm_apNrmMtxBuffer[FGCXfm_nCurrentBuffer][FGCXfm_nLastMtxFlushIndex], FMATH_BYTE_ALIGN_UP((FGCXfm_nCurrentMtxIndex - FGCXfm_nLastMtxFlushIndex) * sizeof( Mtx33 ), 32) );
	}
	
	// Increment one matrix to avoid cache flushing issues
	FGCXfm_nLastMtxFlushIndex = ++FGCXfm_nCurrentMtxIndex;
}

//
//
FINLINE void fgcxfm_Identity( Mtx24 &Matrix )
{
	Matrix[0][0] = 1.f;
	Matrix[0][1] = 0.f;
	Matrix[0][2] = 0.f;
	Matrix[0][3] = 0.f;

	Matrix[1][0] = 0.f;
	Matrix[1][1] = 1.f;
	Matrix[1][2] = 0.f;
	Matrix[1][3] = 0.f;
}

//
//
FINLINE void fgcxfm_TransposeMtxtoMtx33( Mtx &mtxIn, Mtx33 &mtx33Out )
{
	mtx33Out[0][0] = mtxIn[0][0];
	mtx33Out[0][1] = mtxIn[1][0];
	mtx33Out[0][2] = mtxIn[2][0];
	
	mtx33Out[1][0] = mtxIn[0][1];
	mtx33Out[1][1] = mtxIn[1][1];
	mtx33Out[1][2] = mtxIn[2][1];
	
	mtx33Out[2][0] = mtxIn[0][2];
	mtx33Out[2][1] = mtxIn[1][2];
	mtx33Out[2][2] = mtxIn[2][2];
}

//
//
FINLINE void fgcxfm_SetMtx33ToIdentity( Mtx33 &mtx33 )
{
	mtx33[0][0] = 1.f;
	mtx33[0][1] = 0.f;
	mtx33[0][2] = 0.f;
	
	mtx33[1][0] = 0.f;
	mtx33[1][1] = 1.f;
	mtx33[1][2] = 0.f;
	
	mtx33[2][0] = 0.f;
	mtx33[2][1] = 0.f;
	mtx33[2][2] = 1.f;
}

//
//
FINLINE void fgcxfm_Convert43AToGCNrmMtx33( Mtx33 &mtxGC, const CFMtx43A &mtxFang43 ) 
{
	mtxGC[0][0] = mtxFang43.aa[0][0];
	mtxGC[1][0] = mtxFang43.aa[0][1];
	mtxGC[2][0] = mtxFang43.aa[0][2];
	mtxGC[0][1] = mtxFang43.aa[1][0];
	mtxGC[1][1] = mtxFang43.aa[1][1];
	mtxGC[2][1] = mtxFang43.aa[1][2];
	mtxGC[0][2] = mtxFang43.aa[2][0];
	mtxGC[1][2] = mtxFang43.aa[2][1];
	mtxGC[2][2] = mtxFang43.aa[2][2];
}

//
//
FINLINE void fgcxfm_Convert43AToGCMtx( Mtx &mtxGC, const CFMtx43A &mtxFang43 ) 
{
	mtxGC[0][0] = mtxFang43.aa[0][0];
	mtxGC[1][0] = mtxFang43.aa[0][1];
	mtxGC[2][0] = mtxFang43.aa[0][2];
	mtxGC[0][3] = mtxFang43.aa[3][0];
	mtxGC[0][1] = mtxFang43.aa[1][0];
	mtxGC[1][1] = mtxFang43.aa[1][1];
	mtxGC[2][1] = mtxFang43.aa[1][2];
	mtxGC[1][3] = mtxFang43.aa[3][1];
	mtxGC[0][2] = mtxFang43.aa[2][0];
	mtxGC[1][2] = mtxFang43.aa[2][1];
	mtxGC[2][2] = mtxFang43.aa[2][2];
	mtxGC[2][3] = mtxFang43.aa[3][2];
}

FINLINE void fgcxfm_ConvertToGCTexMtx( Mtx &mtxGC, const CFMtx43 &mtxFang43 )
{
	mtxGC[0][0] = mtxFang43.aa[0][0];
	mtxGC[0][1] = mtxFang43.aa[1][0];
	mtxGC[0][2] = 0.0f;
	mtxGC[0][3] = mtxFang43.aa[2][0];
	mtxGC[1][0] = mtxFang43.aa[0][1];
	mtxGC[1][1] = mtxFang43.aa[1][1];
	mtxGC[1][2] = 0.0f;
	mtxGC[1][3] = mtxFang43.aa[2][1];
	mtxGC[2][0] = 0.0f;
	mtxGC[2][1] = 0.0f;
	mtxGC[2][2] = 1.0f;
	mtxGC[2][3] = mtxFang43.aa[2][2];
}

FINLINE void fgcxfm_ConvertToGCTexMtx( Mtx &mtxGC, const CFMtx43A &mtxFang43 )
{
	mtxGC[0][0] = 1.0f;
	mtxGC[0][1] = 0.0f;
	mtxGC[0][2] = 0.0f;
	mtxGC[0][3] = mtxFang43.aa[2][0];
	mtxGC[1][0] = 0.0f;
	mtxGC[1][1] = 1.0f;
	mtxGC[1][2] = 0.0f;
	mtxGC[1][3] = mtxFang43.aa[2][1];
	mtxGC[2][0] = 0.0f;
	mtxGC[2][1] = 0.0f;
	mtxGC[2][2] = 1.0f;
	mtxGC[2][3] = mtxFang43.aa[2][2];
}

//
//
FINLINE void fgcxfm_ConvertMtx33ToMtx( Mtx &mtxGC, const Mtx33 &mtx33 ) 
{
	mtxGC[0][0] = mtx33[0][0];
	mtxGC[1][0] = mtx33[0][1];
	mtxGC[2][0] = mtx33[0][2];
	mtxGC[0][3] = 0.f;
	mtxGC[0][1] = mtx33[1][0];
	mtxGC[1][1] = mtx33[1][1];
	mtxGC[2][1] = mtx33[1][2];
	mtxGC[1][3] = 0.f;
	mtxGC[0][2] = mtx33[2][0];
	mtxGC[1][2] = mtx33[2][1];
	mtxGC[2][2] = mtx33[2][2];
	mtxGC[2][3] = 0.f;
}

//
//
//
FINLINE void fgcxfm_ConvertXfmToGCMtx( Mtx &mtxGC, const CFMtx43A &mtxFang43 ) 
{
	mtxGC[0][0] = mtxFang43.aa[0][0];
	mtxGC[1][0] = mtxFang43.aa[0][1];
	mtxGC[2][0] = mtxFang43.aa[0][2];
	mtxGC[0][3] = mtxFang43.aa[3][0];
	mtxGC[0][1] = mtxFang43.aa[1][0];
	mtxGC[1][1] = mtxFang43.aa[1][1];
	mtxGC[2][1] = mtxFang43.aa[1][2];
	mtxGC[1][3] = mtxFang43.aa[3][1];
	mtxGC[0][2] = mtxFang43.aa[2][0];
	mtxGC[1][2] = mtxFang43.aa[2][1];
	mtxGC[2][2] = mtxFang43.aa[2][2];
	mtxGC[2][3] = mtxFang43.aa[3][2];
}


#endif

