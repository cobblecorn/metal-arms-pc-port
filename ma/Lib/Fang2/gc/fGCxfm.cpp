//////////////////////////////////////////////////////////////////////////////////////
// fGCxfm.cpp - Fang Transformation module.
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
#include "fres.h"

#include "fGC.h"
#include "fGCxfm.h"
#include "fGCvid.h"


//////////////////////////////////////////////////////////////////////////////////////
// Global Variables:
//////////////////////////////////////////////////////////////////////////////////////

CFMtx43A FGCXfm_mtxGCLeftToRightHandConversionMtx;
CFMtx43A FGCXfm_mtxGCLeftToRightHandViewMtx;

#if FANG_DEBUG_BUILD
	u32 FGCXfm_nHighMtxIndex;
#endif

#if !FANG_PRODUCTION_BUILD
	u32 FGCXfm_nMatrixCountExceedingBuffer;
	u32 _nHighMatrixCountExceedingBuffer;
#endif

u32	FGCXfm_nLastMtxFlushIndex;
u32	FGCXfm_nCurrentMtxIndex;
u32	FGCXfm_nCurrentBuffer;
Mtx *FGCXfm_apPosMtxBuffer[2];
Mtx33 *FGCXfm_apNrmMtxBuffer[2];
u8	*FGCXfm_panMtxSlotInGP;
u16 FGCXfm_CurrentMtxInGP[10];
u16 FGCXfm_GPMtxUsesLeft[10];
u8  FGCXfm_nCurrentGPMatrix;


//////////////////////////////////////////////////////////////////////////////////////
// Local Variables:
//////////////////////////////////////////////////////////////////////////////////////

static BOOL _bModuleInitialized;
static BOOL _bWindowCreated;


//////////////////////////////////////////////////////////////////////////////////////
// Static Functions:
//////////////////////////////////////////////////////////////////////////////////////

static BOOL _WindowCreatedCallback( FGCVidEvent_e nEvent );


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
BOOL fgcxfm_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );
	
	FGCXfm_nCurrentBuffer = 0;
	FGCXfm_nCurrentMtxIndex = 0;
	FGCXfm_nLastMtxFlushIndex = 0;
	
#if FANG_DEBUG_BUILD
	FGCXfm_nHighMtxIndex = 0;
#endif

#if !FANG_PRODUCTION_BUILD
	FGCXfm_nMatrixCountExceedingBuffer = 0;
	_nHighMatrixCountExceedingBuffer = 0;
#endif

	FGCXfm_apPosMtxBuffer[0] = (Mtx *)fres_AlignedAlloc( FGCXFM_MTX_BUFFER_SIZE * sizeof( Mtx ), 32 );
	FGCXfm_apPosMtxBuffer[1] = (Mtx *)fres_AlignedAlloc( FGCXFM_MTX_BUFFER_SIZE * sizeof( Mtx ), 32 );
	if ( !FGCXfm_apPosMtxBuffer[0] || !FGCXfm_apPosMtxBuffer[1] )
	{
		DEVPRINTF( "FGCXFM.CPP - ERROR - Unable to allocated memory for Matrix Buffer.\n" );
		return FALSE;
	}
	
	FGCXfm_apNrmMtxBuffer[0] = (Mtx33 *)fres_AlignedAlloc( FGCXFM_MTX_BUFFER_SIZE * sizeof( Mtx33 ), 32 );
	FGCXfm_apNrmMtxBuffer[1] = (Mtx33 *)fres_AlignedAlloc( FGCXFM_MTX_BUFFER_SIZE * sizeof( Mtx33 ), 32 );
	if ( !FGCXfm_apNrmMtxBuffer[0] || !FGCXfm_apNrmMtxBuffer[1] )
	{
		DEVPRINTF( "FGCXFM.CPP - ERROR - Unable to allocated memory for Matrix Buffer.\n" );
		return FALSE;
	}
	
	FGCXfm_panMtxSlotInGP = (u8 *)fres_Alloc( FGCXFM_MTX_BUFFER_SIZE * sizeof( u8 ) );
	if ( !FGCXfm_panMtxSlotInGP )
	{
		DEVPRINTF( "FGCXFM.CPP - ERROR - Unable to allocated memory for Matrix order Buffer.\n" );
		return FALSE;
	}
	// Clear the gp slot related buffers
	fang_MemSet( FGCXfm_panMtxSlotInGP, 0xff, FGCXFM_MTX_BUFFER_SIZE * sizeof( u8 ) );
	fang_MemSet( FGCXfm_CurrentMtxInGP, 0xff, 10 * sizeof( u16 ) );
	fang_MemSet( FGCXfm_GPMtxUsesLeft, 0x00, 10 * sizeof( u16 ) );
	FGCXfm_nCurrentGPMatrix = 0xff;
	
	_bWindowCreated = FALSE;

	fgcvid_RegisterWindowCallbackFunction( _WindowCreatedCallback );

	_bModuleInitialized = TRUE;

	return TRUE;
}


//
//
//
void fgcxfm_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

	fgcvid_UnregisterWindowCallbackFunction( _WindowCreatedCallback );

	_bModuleInitialized = FALSE;
}


//
//
//
void fgcxfm_Swap( void ) 
{
	FASSERT( _bModuleInitialized );

#if FANG_DEBUG_BUILD
	if ( FGCXfm_nCurrentMtxIndex > FGCXfm_nHighMtxIndex )
	{
		FGCXfm_nHighMtxIndex = FGCXfm_nCurrentMtxIndex;
		DEVPRINTF( "High Matrix Allocation: %d (%2.1f percent)\n", FGCXfm_nHighMtxIndex, ((f32)FGCXfm_nHighMtxIndex / (f32)FGCXFM_MTX_BUFFER_SIZE) * 100.f );
	}
#endif

#if !FANG_PRODUCTION_BUILD
	if ( FGCXfm_nMatrixCountExceedingBuffer > _nHighMatrixCountExceedingBuffer )
	{
		_nHighMatrixCountExceedingBuffer = FGCXfm_nMatrixCountExceedingBuffer;
		DEVPRINTF( "FGCXFM.CPP - WARNING - New high exceeding matrix buffer by %d matrices. (Limit %d)\n", _nHighMatrixCountExceedingBuffer, FGCXFM_MTX_BUFFER_SIZE );
	}
	FGCXfm_nMatrixCountExceedingBuffer = 0;
#endif
	
	FGCXfm_nCurrentGPMatrix = 0xff;
	FGCXfm_nLastMtxFlushIndex = 0;
	FGCXfm_nCurrentMtxIndex = 0;
	FGCXfm_nCurrentBuffer = ~FGCXfm_nCurrentBuffer & 0x01;
	FASSERT( FGCXfm_nCurrentBuffer == 1 || FGCXfm_nCurrentBuffer == 0 );
	
	GXSetArray( GX_POS_MTX_ARRAY, FGCXfm_apPosMtxBuffer[FGCXfm_nCurrentBuffer], sizeof( Mtx ) );
	GXSetArray( GX_NRM_MTX_ARRAY, FGCXfm_apNrmMtxBuffer[FGCXfm_nCurrentBuffer], sizeof( Mtx33 ) );
	
	fgcxfm_InvalidateLocalMatrixCache();
}


//
//
//
void fgcxfm_InvalidateLocalMatrixCache( void )
{
	u32 i;
	
	// Clear the current matrix slots
	for ( i = 0; i < 10; i++ )
	{
		if ( FGCXfm_CurrentMtxInGP[i] != 0xffff )
		{
			// Tell the matrix currently in the GP that it no longer is
			FGCXfm_panMtxSlotInGP[FGCXfm_CurrentMtxInGP[i]] = 0xff;
			FGCXfm_CurrentMtxInGP[i] = 0xffff;
		}
		
		FASSERT( FGCXfm_GPMtxUsesLeft[i] == 0 );
	}
}


//
//
//
void fgcxfm_SetViewGCMatrix( void )
{
	// In order to convert the Fang left-handed view system
	// to the GC right-handed render system, we negate the
	// z-axis of the view matrix.  However, when we get the
	// matrix to send to the card, it is a modelview matrix.
	// So what we're going to do is calculate a matrix to 
	// convert the model+view matrix to a matrix that has 
	// the z of the view negated. 

	FGCXfm_mtxGCLeftToRightHandConversionMtx = FXfm_pModel->m_MtxR;
	
	FGCXfm_mtxGCLeftToRightHandConversionMtx.m_vFront.x = -FGCXfm_mtxGCLeftToRightHandConversionMtx.m_vFront.x;
	FGCXfm_mtxGCLeftToRightHandConversionMtx.m_vFront.y = -FGCXfm_mtxGCLeftToRightHandConversionMtx.m_vFront.y;
	FGCXfm_mtxGCLeftToRightHandConversionMtx.m_vFront.z = -FGCXfm_mtxGCLeftToRightHandConversionMtx.m_vFront.z;
	
	FGCXfm_mtxGCLeftToRightHandConversionMtx.Mul( FXfm_pModel->m_MtxF );
	
	FGCXfm_mtxGCLeftToRightHandViewMtx.Mul( FGCXfm_mtxGCLeftToRightHandConversionMtx, FXfm_pView->m_MtxF );
	
	//test
	CFMtx43A tmp, tmp2;
	Mtx mtxGC;
	tmp.Identity();
	tmp.m_vFront.x = -tmp.m_vFront.x;
	tmp.m_vFront.y = -tmp.m_vFront.y;
	tmp.m_vFront.z = -tmp.m_vFront.z;
	tmp2.Mul( tmp, FXfm_pView->m_MtxF );
	fgcxfm_ConvertXfmToGCMtx( mtxGC, tmp2 );	
	MTXInverse(mtxGC, FXfm_GCInvView);
	//
}


//
//
//
void fgcxfm_SetModelGCMatrix( void ) 
{
	FASSERT( _bWindowCreated );

	Mtx mtxGC;

	// Create the GC ModelView matrix (Concatenation of the ModelView matrix
	// and the GC left-to-right conversion matrix)
	CFMtx43A mtxConcat;
	mtxConcat.Mul( FGCXfm_mtxGCLeftToRightHandConversionMtx, FXfm_pModelView->m_MtxF );

	// Convert to GC
	fgcxfm_ConvertXfmToGCMtx( mtxGC, mtxConcat );
	
	// Set the model matrix
	GXLoadPosMtxImm( mtxGC, GX_PNMTX0 );
	
	// Invert, transpose and set the normal matrix based on the model matrix
	// NOTE: GC requires normals and lights to be in view space
	if ( fmath_Abs( 1.f - FXfm_pModelView->m_fScaleR ) > 0.01f )
	{
		mtxConcat.Mul( FXfm_pModelView->m_fScaleR );
		fgcxfm_ConvertXfmToGCMtx( mtxGC, mtxConcat );
	}
	GXLoadNrmMtxImm( mtxGC, GX_PNMTX0 );
	
	GXSetCurrentMtx( GX_PNMTX0 );
	
	// Make sure we clear the local matrix cache
	if ( FGCXfm_CurrentMtxInGP[GX_PNMTX0] != 0xffff )
	{
		// Tell the matrix currently in the GP that it no longer is
		FGCXfm_panMtxSlotInGP[FGCXfm_CurrentMtxInGP[GX_PNMTX0]] = 0xff;
		FGCXfm_CurrentMtxInGP[GX_PNMTX0] = 0xffff;
	}
	FGCXfm_nCurrentGPMatrix = 0xff;
}


//
//
//
void fxfm_SetViewAndWorldSpaceModelMatrices( void )
{
	Mtx mtxGC;

	if ( FXfm_pMirrorMtx )
	{
		fgcxfm_SetCustomGCMatrix( FXfm_pMirrorMtx );
		return;
	}
	
	// Convert to GC
	fgcxfm_ConvertXfmToGCMtx( mtxGC, FGCXfm_mtxGCLeftToRightHandViewMtx );
	
	// Set the model matrix
	GXLoadPosMtxImm( mtxGC, GX_PNMTX0 );
	
	// Set the normal matrix
	GXLoadNrmMtxImm( mtxGC, GX_PNMTX0 );
	
	GXSetCurrentMtx( GX_PNMTX0 );
	
	// Make sure we clear the local matrix cache
	if ( FGCXfm_CurrentMtxInGP[GX_PNMTX0] != 0xffff )
	{
		// Tell the matrix currently in the GP that it no longer is
		FGCXfm_panMtxSlotInGP[FGCXfm_CurrentMtxInGP[GX_PNMTX0]] = 0xff;
		FGCXfm_CurrentMtxInGP[GX_PNMTX0] = 0xffff;
	}
	FGCXfm_nCurrentGPMatrix = 0xff;
}


//
//
//
void fgcxfm_SetGCMatricesIdentity( void ) 
{
	FASSERT( _bWindowCreated );

	Mtx mtxGC;

	// Convert to GC
	fgcxfm_ConvertXfmToGCMtx( mtxGC, FGCXfm_mtxGCLeftToRightHandConversionMtx );
	
	// Set the model matrix
	GXLoadPosMtxImm( mtxGC, GX_PNMTX0 );
	
	// Set the normal matrix
	GXLoadNrmMtxImm( mtxGC, GX_PNMTX0 );
	
	GXSetCurrentMtx( GX_PNMTX0 );
	
	// Make sure we clear the local matrix cache
	if ( FGCXfm_CurrentMtxInGP[GX_PNMTX0] != 0xffff )
	{
		// Tell the matrix currently in the GP that it no longer is
		FGCXfm_panMtxSlotInGP[FGCXfm_CurrentMtxInGP[GX_PNMTX0]] = 0xff;
		FGCXfm_CurrentMtxInGP[GX_PNMTX0] = 0xffff;
	}
	FGCXfm_nCurrentGPMatrix = 0xff;
}


//
//
//
void fgcxfm_SetGCMatrices( void ) 
{
	FASSERT( _bWindowCreated );

	Mtx mtxGC;

	// Create the GC ModelView matrix (Concatenation of the ModelView matrix
	// and the GC left-to-right conversion matrix)
	CFMtx43A mtxConcat;
	mtxConcat.Mul( FGCXfm_mtxGCLeftToRightHandConversionMtx, FXfm_pModelView->m_MtxF );

	// Convert to GC
	fgcxfm_ConvertXfmToGCMtx( mtxGC, mtxConcat );
	
	// Set the model matrix
	GXLoadPosMtxImm( mtxGC, GX_PNMTX0 );
	
	// Invert, transpose and set the normal matrix based on the model matrix
	// NOTE: GC requires normals and lights to be in view space
	if ( fmath_Abs( 1.f - FXfm_pModelView->m_fScaleR ) > 0.01f )
	{
		mtxConcat.Mul( FXfm_pModelView->m_fScaleR );
		fgcxfm_ConvertXfmToGCMtx( mtxGC, mtxConcat );
	}
	GXLoadNrmMtxImm( mtxGC, GX_PNMTX0 );
	
	GXSetCurrentMtx( GX_PNMTX0 );
	
	// Make sure we clear the local matrix cache
	if ( FGCXfm_CurrentMtxInGP[GX_PNMTX0] != 0xffff )
	{
		// Tell the matrix currently in the GP that it no longer is
		FGCXfm_panMtxSlotInGP[FGCXfm_CurrentMtxInGP[GX_PNMTX0]] = 0xff;
		FGCXfm_CurrentMtxInGP[GX_PNMTX0] = 0xffff;
	}
	FGCXfm_nCurrentGPMatrix = 0xff;
}


//
//
//
void fgcxfm_SetCustomGCMatrix( const CFMtx43A *pMtx43 ) 
{
	FASSERT( _bWindowCreated );

	Mtx mtxGC;
	
	// Create the ModelView matrix (Concatenation of the model and view matrix
	// and the GC left-to-right conversion matrix)
	CFMtx43A mtxConcat;
	mtxConcat.Mul( FGCXfm_mtxGCLeftToRightHandConversionMtx, FXfm_pView->m_MtxF ).Mul( *pMtx43 );

	// Convert to GC
	fgcxfm_ConvertXfmToGCMtx( mtxGC, mtxConcat );
	
	// Set the model matrix
	GXLoadPosMtxImm( mtxGC, GX_PNMTX0 );
	
	// Invert, transpose and set the normal matrix based on the model matrix
	// NOTE: GC requires normals and lights to be in view space
	f32 fScale = pMtx43->m_vFront.MagSq();
	if ( fmath_Abs( 1.f - fScale ) > 0.01f )
	{
		mtxConcat.Mul( fmath_InvSqrt( fScale ) );
		fgcxfm_ConvertXfmToGCMtx( mtxGC, mtxConcat );
	}
	GXLoadNrmMtxImm( mtxGC, GX_PNMTX0 );

	GXSetCurrentMtx( GX_PNMTX0 );
	
	// Make sure we clear the local matrix cache
	if ( FGCXfm_CurrentMtxInGP[GX_PNMTX0] != 0xffff )
	{
		// Tell the matrix currently in the GP that it no longer is
		FGCXfm_panMtxSlotInGP[FGCXfm_CurrentMtxInGP[GX_PNMTX0]] = 0xff;
		FGCXfm_CurrentMtxInGP[GX_PNMTX0] = 0xffff;
	}
	FGCXfm_nCurrentGPMatrix = 0xff;
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
			_bWindowCreated = TRUE;

			CFXfm::InitStack();
//			fgcxfm_SetGCMatrices();
			Mtx mtxIdentity;
			C_MTXIdentity( mtxIdentity );
			GXLoadPosMtxImm( mtxIdentity, GX_PNMTX0 );
			GXLoadPosMtxImm( mtxIdentity, GX_PNMTX1 );
			GXLoadPosMtxImm( mtxIdentity, GX_PNMTX2 );
			GXLoadPosMtxImm( mtxIdentity, GX_PNMTX3 );
			GXLoadPosMtxImm( mtxIdentity, GX_PNMTX4 );
			GXLoadPosMtxImm( mtxIdentity, GX_PNMTX5 );
			GXLoadPosMtxImm( mtxIdentity, GX_PNMTX6 );
			GXLoadPosMtxImm( mtxIdentity, GX_PNMTX7 );
			GXLoadPosMtxImm( mtxIdentity, GX_PNMTX8 );
			GXLoadPosMtxImm( mtxIdentity, GX_PNMTX9 );
			
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


