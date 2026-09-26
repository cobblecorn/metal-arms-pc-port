//////////////////////////////////////////////////////////////////////////////////////
// fdx8vb.cpp - Fang DX vertex buffer module.
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
// 11/09/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fdx8.h"
#include "fdx8sh.h"
#include "fdx8vb.h"
#include "fdx8vid.h"

#include "fperf.h"




const FDX8VB_Info_t FDX8VB_InfoTable[FDX8VB_TYPE_COUNT] = {
	// (no TL): W=0, N=1, C=1, TC=1
	D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0),
	36,			// nVtxBytes = 4*(3p + 3n + 0w + 1c + 1*2t)
	FALSE,		// Vertex contains untransformed and unlit data

	1,			// nNormalCount
	0,			// nWeightCount
	1,			// nColorCount
	1,			// nTCCount

	0,			// nOffsetPos
	255,		// (nOffsetWeight)
	12,			// nOffsetNorm
	24,			// nOffsetColor
	28,			// nOffsetTC


	// (no TL): W=0, N=1, C=1, TC=2
	D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE2(1),
	44,			// nVtxBytes = 4*(3p + 3n + 0w + 1c + 2*2t)
	FALSE,		// Vertex contains untransformed and unlit data

	1,			// nNormalCount
	0,			// nWeightCount
	1,			// nColorCount
	2,			// nTCCount

	0,			// nOffsetPos
	255,		// (nOffsetWeight)
	12,			// nOffsetNorm
	24,			// nOffsetColor
	28,			// nOffsetTC


	// (no TL): W=3, N=1, C=1, TC=1
	D3DFVF_XYZB3 | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0),
	48,			// nVtxBytes = 4*(3p + 3n + 3w + 1c + 1*2t)
	FALSE,		// Vertex contains untransformed and unlit data

	1,			// nNormalCount
	3,			// nWeightCount
	1,			// nColorCount
	1,			// nTCCount

	0,			// nOffsetPos
	12,			// nOffsetWeight
	24,			// nOffsetNorm
	36,			// nOffsetColor
	40,			// nOffsetTC


	// (no TL): W=3, N=1, C=1, TC=2
	D3DFVF_XYZB3 | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE2(1),
	56,			// nVtxBytes = 4*(3p + 3n + 3w + 1c + 2*2t)
	FALSE,		// Vertex contains untransformed and unlit data

	1,			// nNormalCount
	3,			// nWeightCount
	1,			// nColorCount
	2,			// nTCCount

	0,			// nOffsetPos
	12,			// nOffsetWeight
	24,			// nOffsetNorm
	36,			// nOffsetColor
	40,			// nOffsetTC


	// (TL): W=0, N=0, C=2, TC=1
	D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE2(1),
	40,			// nVtxBytes = 4*(4p + 0n + 0w + 2c + 2*2t)
	TRUE,		// Vertex contains already transformed and lit data

	0,			// nNormalCount
	0,			// nWeightCount
	2,			// nColorCount
	2,			// nTCCount

	0,			// nOffsetPos
	255,		// (nOffsetWeight)
	255,		// (nOffsetNorm)
	16,			// nOffsetColor
	24,			// nOffsetTC


	// (no TL): W=0, N=0, C=1, TC=0
	D3DFVF_XYZ | D3DFVF_DIFFUSE,
	16,			// nVtxBytes = 4*(3p + 0n + 0w + 1c + 0*2t)
	FALSE,		// Vertex contains untransformed and unlit data

	0,			// nNormalCount
	0,			// nWeightCount
	1,			// nColorCount
	0,			// nTCCount

	0,			// nOffsetPos
	255,		// (nOffsetWeight)
	255,		// (nOffsetNorm)
	12,			// nOffsetColor
	255,		// (nOffsetTC)


	// (no TL): W=0, N=0, C=1, TC=1
	D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0),
	24,			// nVtxBytes = 4*(3p + 0n + 0w + 1c + 1*2t)
	FALSE,		// Vertex contains untransformed and unlit data

	0,			// nNormalCount
	0,			// nWeightCount
	1,			// nColorCount
	1,			// nTCCount

	0,			// nOffsetPos
	255,		// (nOffsetWeight)
	255,		// (nOffsetNorm)
	12,			// nOffsetColor
	16,			// nOffsetTC
};


static BOOL8 _bModuleInitialized;
static BOOL8 _bWindowCreated;
static BOOL8 _bStream1Set = TRUE;
static BOOL8 _bStream2Set = TRUE;


static FLinkRoot_t _LinkRoot;

static FDX8VB_t *_pSelectedVB;
static BOOL _bSoftwareVBEnabled;


static void _PreReset( void );
static void _PostReset( void );
static BOOL _WindowCreatedCallback( FDX8VidEvent_e nEvent );



BOOL fdx8vb_ModuleStartup( void ) {
	FASSERT( !_bModuleInitialized );

	fdx8vid_RegisterWindowCallbackFunction( _WindowCreatedCallback );

	_bSoftwareVBEnabled = FALSE;
	_bWindowCreated = FALSE;
	_bModuleInitialized = TRUE;

	return TRUE;
}

void fdx8vb_ModuleShutdown( void ) {
	FASSERT( _bModuleInitialized );

	fdx8vid_UnregisterWindowCallbackFunction( _WindowCreatedCallback );

	_bModuleInitialized = FALSE;
}


// Given the desired field counts, returns the index into FDX8VB_InfoTable[]
// of the entry that describes the best vertex format to use. If a vertex
// format doesn't exist that can accommodate the data, -1 is returned.
s32 fdx8vb_FindFormat( BOOL bPostTL, u32 nNormalCount, u32 nWeightCount, u32 nColorCount, u32 nTCCount ) {
	const FDX8VB_Info_t *pInfo;
	u32 nSmallestBytes;
	s32 i, nBestInfoIndex;

	FASSERT( _bModuleInitialized );

	nSmallestBytes = 0xffffffff;
	nBestInfoIndex = -1;

	for( i=0, pInfo=FDX8VB_InfoTable; i<FDX8VB_TYPE_COUNT; i++, pInfo++ ) {
		if( !pInfo->bPostTL == !bPostTL &&
			pInfo->nNormalCount >= nNormalCount &&
			pInfo->nWeightCount >= nWeightCount &&
			pInfo->nColorCount >= nColorCount &&
			pInfo->nTCCount >= nTCCount
			) {
			// This VB format can accommodate the specified data...

			if( pInfo->nVtxBytes < nSmallestBytes ) {
				nSmallestBytes = pInfo->nVtxBytes;
				nBestInfoIndex = i;
			}
		}
	}

	return nBestInfoIndex;
}

// Returns the number of bytes per vertex for the specified vertex buffer type.
u32 fdx8vb_GetVertexByteCount( FDX8VB_Type_e nType ) {
	const FDX8VB_Info_t *pInfo;
	u32 nByteCount;

	FASSERT( _bModuleInitialized );
	FASSERT( nType>=0 && nType<FDX8VB_TYPE_COUNT );

	pInfo = FDX8VB_InfoTable + nType;

	nByteCount = sizeof(CFVec3)
			   + pInfo->nNormalCount * sizeof(CFVec3)
			   + pInfo->nTCCount * sizeof(CFVec2)
			   + pInfo->nColorCount * sizeof(u32)
			   + pInfo->nWeightCount * sizeof(f32);

	if( pInfo->bPostTL ) {
		nByteCount += sizeof(f32);
	}


	return nByteCount;
}


// Creates a vertex buffer for use with the fixed function pipeline.
//
// If bDynamic is TRUE, a dynamic vertex buffer is created.
// Otherwise, a static vertex buffer is created.
//
// Fills *pVB with data.
//
// Returns TRUE if successful, or FALSE if a failure occurred.
BOOL fdx8vb_CreateFVF( FDX8VB_t *pVB, u32 nVtxCount, s32 nInfoIndex, BOOL bDynamic, BOOL bSoftwareVertexProcessing, BOOL bPointSprites ) {
	const FDX8VB_Info_t *pInfo;
	HRESULT hr;
	DWORD nUsage;
	D3DPOOL nPool;

	FASSERT( _bWindowCreated );
	FASSERT( pVB );
	FASSERT( nVtxCount );
	FASSERT( nInfoIndex>=0 && nInfoIndex<FDX8VB_TYPE_COUNT );

	#if FANG_PLATFORM_XB
		// Xbox doesn't support software vertex processing...
		bSoftwareVertexProcessing = FALSE;
	#else
		if( !(FDX8_Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) ) {
			// This PC video card doesn't support hardware TnL...
			bSoftwareVertexProcessing = TRUE;
		}
	#endif

	pInfo = &FDX8VB_InfoTable[ nInfoIndex ];

	pVB->nVtxCount = nVtxCount;
	pVB->nBytesPerVertex = pInfo->nVtxBytes;
	pVB->nInfoIndex = (s8)nInfoIndex;
	pVB->bDynamic = bDynamic;
	pVB->bSoftwareVP = bSoftwareVertexProcessing;
	pVB->bLocked = FALSE;
	pVB->pLockBuf = NULL;
	pVB->nLockOffset = 0;
	pVB->nLockBytes = 0;
	pVB->hVertexShader = pInfo->nFVF;

	if( bDynamic ) {
		nUsage = D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC;
		nPool = D3DPOOL_DEFAULT;
	} else {
		nUsage = D3DUSAGE_WRITEONLY;
		nPool = D3DPOOL_MANAGED;
	}

	#if FANG_PLATFORM_WIN
		if( bSoftwareVertexProcessing ) {
			nUsage |= D3DUSAGE_SOFTWAREPROCESSING;
		}
		if( bPointSprites ) {
			nUsage |= D3DUSAGE_POINTS;
		}
	#endif

	hr = FDX8_pDev->CreateVertexBuffer( pVB->nBytesPerVertex * pVB->nVtxCount, nUsage, pInfo->nFVF, nPool, &pVB->pDXVB );
	if( FAILED(hr) ) {
		pVB->pDXVB = NULL;
		return FALSE;
	}

	if( bDynamic ) {
		flinklist_AddTail( &_LinkRoot, pVB );
	}

	return TRUE;
}

// Creates a vertex buffer for use with a vertex shader.
//
// If bDynamic is TRUE, a dynamic vertex buffer is created.
// Otherwise, a static vertex buffer is created.
//
// Fills *pVB with data.
//
// Returns TRUE if successful, or FALSE if a failure occurred.
BOOL fdx8vb_CreateVSH( FDX8VB_t *pVB, u32 nVtxCount, u32 nBytesPerVertex, BOOL bDynamic, BOOL bPointSprites ) {
	HRESULT hr;
	DWORD nUsage;
	D3DPOOL nPool;

	FASSERT( _bWindowCreated );
	FASSERT( pVB );
	FASSERT( nVtxCount );
	FASSERT( nBytesPerVertex );

	pVB->nVtxCount = nVtxCount;
	pVB->nBytesPerVertex = nBytesPerVertex;
	pVB->nInfoIndex = -1;
	pVB->bDynamic = bDynamic;
	pVB->bSoftwareVP = !FDX8_bHardwareVertexShaders;
	pVB->bLocked = FALSE;
	pVB->pLockBuf = NULL;
	pVB->nLockOffset = 0;
	pVB->nLockBytes = 0;
	pVB->hVertexShader = NULL;

	if( bDynamic ) {
		nUsage = D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC;
		nPool = D3DPOOL_DEFAULT;
	} else {
		nUsage = D3DUSAGE_WRITEONLY;
		nPool = D3DPOOL_MANAGED;
	}

	#if FANG_PLATFORM_WIN
		if( !FDX8_bHardwareVertexShaders ) {
			nUsage |= D3DUSAGE_SOFTWAREPROCESSING;
		}
		if( bPointSprites ) {
			nUsage |= D3DUSAGE_POINTS;
		}
	#endif

	hr = FDX8_pDev->CreateVertexBuffer( pVB->nBytesPerVertex * pVB->nVtxCount, nUsage, 0, nPool, &pVB->pDXVB );
	if( FAILED(hr) ) {
		pVB->pDXVB = NULL;
		return FALSE;
	}

	if( bDynamic ) {
		flinklist_AddTail( &_LinkRoot, pVB );
	}

	return TRUE;
}

// Destroys the specified vertex buffer.
void fdx8vb_Destroy( FDX8VB_t *pVB ) {
	FASSERT( _bWindowCreated );

	if( pVB->bDynamic ) {
		flinklist_Remove( &_LinkRoot, pVB );
	}

	if( pVB && pVB->pDXVB ) {
		fdx8vb_Unlock( pVB );

		FDX8_SAFE_RELEASE( pVB->pDXVB );
	}

	if ( pVB && pVB->pLMUVStream )
	{
		((IDirect3DVertexBuffer8 *)pVB->pLMUVStream)->Release();
		pVB->pLMUVStream = NULL;
	}

	if ( pVB && pVB->pBasisStream )
	{
		((IDirect3DVertexBuffer8 *)pVB->pBasisStream )->Release();
		pVB->pBasisStream = NULL;

	}

	if( pVB == _pSelectedVB ) {
		_pSelectedVB = NULL;
	}
}

void *fdx8vb_Lock( FDX8VB_t *pVB, u32 nStartVtxIndex, u32 nVtxCount ) {
	BYTE *pnLockedDestBuf;
	u32 nVtxBytes;

	FASSERT( _bWindowCreated );
	FASSERT( pVB );
	FASSERT( nStartVtxIndex <= pVB->nVtxCount );
	FASSERT( nVtxCount <= pVB->nVtxCount );
	FASSERT( (nStartVtxIndex + nVtxCount) <= pVB->nVtxCount );

	fdx8vb_Unlock( pVB );

	nVtxBytes = pVB->nBytesPerVertex;

	pVB->nLockOffset = nVtxBytes * nStartVtxIndex;
	pVB->nLockBytes = nVtxBytes * nVtxCount;

	if( !pVB->bDynamic ) {
		pVB->pDXVB->Lock( pVB->nLockOffset, pVB->nLockBytes, (void **)&pnLockedDestBuf, 0 );
	} else {
		#if FANG_PLATFORM_WIN
			pVB->pDXVB->Lock( pVB->nLockOffset, pVB->nLockBytes, (void **)&pnLockedDestBuf, nStartVtxIndex ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD );
		#else
			pVB->pDXVB->Lock( pVB->nLockOffset, pVB->nLockBytes, &pnLockedDestBuf, D3DLOCK_NOOVERWRITE );
		#endif
	}

	pVB->bLocked = TRUE;
	pVB->pLockBuf = (void *)pnLockedDestBuf;

	return pVB->pLockBuf;
}

void *fdx8vb_LockEntireVB( FDX8VB_t *pVB ) {
	FASSERT( _bWindowCreated );
	FASSERT( pVB );
	return fdx8vb_Lock( pVB, 0, pVB->nVtxCount );
}

void fdx8vb_Unlock( FDX8VB_t *pVB ) {
	FASSERT( _bWindowCreated );
	FASSERT( pVB );

	if( pVB->bLocked ) {
		pVB->pDXVB->Unlock();
		pVB->bLocked = FALSE;
		pVB->pLockBuf = NULL;
		pVB->nLockOffset = 0;
		pVB->nLockBytes = 0;
	}
}

BOOL fdx8vb_IsBusy( FDX8VB_t *pVB ) {
	FASSERT( _bWindowCreated );
	FASSERT( pVB );

	#if FANG_PLATFORM_XB
		return pVB->pDXVB->IsBusy();
	#else
		return FALSE;
	#endif
}

void fdx8vb_BlockUntilNotBusy( FDX8VB_t *pVB ) {
	FASSERT( _bWindowCreated );
	FASSERT( pVB );

	#if FANG_PLATFORM_XB
		pVB->pDXVB->BlockUntilNotBusy();
	#endif
}

// Returns the previously-selected VB, or NULL if none.
FDX8VB_t *fdx8vb_Select( FDX8VB_t *pVB, BOOL bSetVShader/*=FALSE*/, BOOL bUseLightMapSTs/*=FALSE*/, BOOL bUseBasisVectors/*=FALSE*/ ) 
{
	FDX8VB_t *pPrevSelectedVB;

	FASSERT( _bWindowCreated );

	pPrevSelectedVB = _pSelectedVB;

	if( pVB != _pSelectedVB )
	{
		if( pVB ) 
		{
			// A new VB has been specified...

			if( !_bSoftwareVBEnabled != !pVB->bSoftwareVP ) 
			{
				_bSoftwareVBEnabled = !!pVB->bSoftwareVP;
				#if FANG_PLATFORM_WIN
					FDX8_pDev->SetRenderState( D3DRS_SOFTWAREVERTEXPROCESSING, _bSoftwareVBEnabled );
				#endif
			}

			FDX8_pDev->SetStreamSource( 0, pVB->pDXVB, pVB->nBytesPerVertex );
			if (bSetVShader)
			{
				FDX8_SetVertexShader( pVB->hVertexShader );
			}

			fdx8sh_SetVertexType(pVB->hVertexShader);
		} 
		else 
		{
			// No VB has been specified...
			FDX8_pDev->SetStreamSource( 0, NULL, 0 );
			FDX8_pDev->SetStreamSource( 1, NULL, 0 );
			FDX8_pDev->SetStreamSource( 2, NULL, 0 );

			_bStream1Set = FALSE;

			if( _bSoftwareVBEnabled ) 
			{
				#if FANG_PLATFORM_WIN
					FDX8_pDev->SetRenderState( D3DRS_SOFTWAREVERTEXPROCESSING, FALSE );
				#endif
				_bSoftwareVBEnabled = FALSE;
			}
		}

		_pSelectedVB = pVB;

		#if FPERF_ENABLE
			FPerf_nRawVBSwitchCount++;
		#endif
	}

	if ( bUseLightMapSTs && pVB->pLMUVStream )
	{
		FASSERT( (!pVB->nLMTCCount && !pVB->pLMUVStream) || (pVB->nLMTCCount && pVB->pLMUVStream) );
		FDX8_pDev->SetStreamSource( 1, (IDirect3DVertexBuffer8 *)pVB->pLMUVStream, pVB->nLMTCCount * sizeof(FDX8LightMapST_t) );
		_bStream1Set = TRUE;
	}
	else if ( _bStream1Set )
	{
		FDX8_pDev->SetStreamSource( 1, NULL, 0 );
		_bStream1Set = FALSE;
	}

	fdx8sh_Stream1Set( _bStream1Set );

	if ( bUseBasisVectors && pVB->pBasisStream )
	{
		FDX8_pDev->SetStreamSource( 2, (IDirect3DVertexBuffer8 *)pVB->pBasisStream, sizeof(FDX8BasisVectors_t) );
		_bStream2Set = TRUE;
	}
	else if ( _bStream2Set )
	{
		FDX8_pDev->SetStreamSource( 2, NULL, 0 );
		_bStream2Set = FALSE;
	}

	return pPrevSelectedVB;
}

FDX8VB_t *fdx8vb_GetSelected( void ) {
	FASSERT( _bWindowCreated );

	return _pSelectedVB;
}

void fdx8vb_UncacheSelected( void ) {
	FASSERT( _bWindowCreated );

	_pSelectedVB = NULL;
}

void fdx8vb_AttachToVertexShader( FDX8VB_t *pVB, DWORD hVertexShader ) {
	FASSERT( _bWindowCreated );
	FASSERT( pVB->nInfoIndex == -1 );

	pVB->hVertexShader = hVertexShader;

	if( pVB == _pSelectedVB ) {
		// Changed the vertex shader of the currently-selected VB...

		FDX8_SetVertexShader( pVB->hVertexShader );
		fdx8sh_SetVertexType(pVB->hVertexShader);
	}
}


static void _PreReset( void ) {
	FDX8VB_t *pVB;

	for( pVB=(FDX8VB_t *)flinklist_GetHead(&_LinkRoot); pVB; pVB=(FDX8VB_t *)flinklist_GetNext(&_LinkRoot, pVB) ) {
		if( pVB->bDynamic && pVB->pDXVB ) {
			if( pVB->bLocked ) {
				pVB->pDXVB->Unlock();
			}

			if( pVB == _pSelectedVB ) {
				FDX8_pDev->SetStreamSource( 0, NULL, 0 );
			}

			FDX8_SAFE_RELEASE( pVB->pDXVB );
		}
	}
}

static void _PostReset( void ) {
	FDX8VB_t *pVB;
	DWORD nUsage;

	for( pVB=(FDX8VB_t *)flinklist_GetHead(&_LinkRoot); pVB; pVB=(FDX8VB_t *)flinklist_GetNext(&_LinkRoot, pVB) ) {
		if( pVB->bDynamic ) {
			// Let's restore this dynamic VB...

			FASSERT( pVB->pDXVB == NULL );

			nUsage = D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC;
			#if FANG_PLATFORM_WIN
				if( pVB->bSoftwareVP ) {
					nUsage |= D3DUSAGE_SOFTWAREPROCESSING;
				}
			#endif

			// Re-create the VB...
			FDX8_pDev->CreateVertexBuffer(
						pVB->nBytesPerVertex * pVB->nVtxCount,
						nUsage,
						pVB->hVertexShader,
						D3DPOOL_DEFAULT,
						&pVB->pDXVB
					);

			// Re-lock the VB...
			if( pVB->bLocked ) {
				pVB->pDXVB->Lock( pVB->nLockOffset, pVB->nLockBytes, (void **)&pVB->pLockBuf, D3DLOCK_NOOVERWRITE );
			}
		}
	}

	// Restore the currently-selected VB...
	if( _pSelectedVB ) {
		FDX8_pDev->SetStreamSource( 0, _pSelectedVB->pDXVB, _pSelectedVB->nBytesPerVertex );
		FDX8_SetVertexShader( _pSelectedVB->hVertexShader );
		fdx8sh_SetVertexType(_pSelectedVB->hVertexShader);
	} else {
		FDX8_pDev->SetStreamSource( 0, NULL, 0 );
	}

	#if FANG_PLATFORM_WIN
		FDX8_pDev->SetRenderState( D3DRS_SOFTWAREVERTEXPROCESSING, _bSoftwareVBEnabled );
	#endif
}

static BOOL _WindowCreatedCallback( FDX8VidEvent_e nEvent ) {
	FDX8VB_t *pVB;

	FASSERT( _bModuleInitialized );
	FASSERT( nEvent>=0 && nEvent<FDX8VID_EVENT_COUNT );

	switch( nEvent ) {
	case FDX8VID_EVENT_WINDOW_CREATED:
		flinklist_InitRoot( &_LinkRoot, (s32)FANG_OFFSETOF( FDX8VB_t, Link ) );
		_pSelectedVB = NULL;

		#if FANG_PLATFORM_WIN
			FDX8_pDev->SetRenderState( D3DRS_SOFTWAREVERTEXPROCESSING, FALSE );
		#endif
		_bSoftwareVBEnabled = FALSE;

		_bWindowCreated = TRUE;
		break;

	case FDX8VID_EVENT_WINDOW_DESTROYED:
		if( _LinkRoot.nCount ) {
			if( !fdx8vid_bResetting ) {
				// Destroy all VBs...
				while( pVB=(FDX8VB_t *)flinklist_GetTail( &_LinkRoot ) ) {
					fdx8vb_Destroy( pVB );
				}
			}
		}

		_bWindowCreated = FALSE;

		break;

	case FDX8VID_EVENT_PRE_RESET:
		// Must release all dynamic VBs...
		_PreReset();
		break;

	case FDX8VID_EVENT_POST_RESET:
		// Must restore all dynamic VBs...
		_PostReset();
		break;
	}

	return TRUE;
}

