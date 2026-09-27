//////////////////////////////////////////////////////////////////////////////////////
// fdx8mesh.cpp - Fang mesh module (DX8 version).
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
// 10/16/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fresload.h"
#include "flight.h"
#include "frenderer.h"
#include "fshaders.h"
#include "fworld.h"
#include "fshadow.h"
#include "fdatastreaming.h"

#include "fdx8.h"
#include "fdx8shadow.h"
#include "fdx8mesh.h"
#include "fdx8load.h"
#include "fdx8vb.h"
#include "fdx8sh.h"
#include "fdx8vid.h"
#include "fdx8viewport.h"
#include "fdx8xfm.h"
#include "fdx8tex.h"
#include "gcmesh.h"

#include "fdx8vshader_const.h"



//////////////////////////////////////////////////////////////////////////////////////
// External dependencies:
//////////////////////////////////////////////////////////////////////////////////////

extern f32 FShadow_ShadowDist;
extern f32 FShader_ShadowRecieveDist;


//////////////////////////////////////////////////////////////////////////////////////
// Global variables:
//////////////////////////////////////////////////////////////////////////////////////

u32 FMesh_nLastSurfaceMaterialCRC;


//////////////////////////////////////////////////////////////////////////////////////
// Local defines:
//////////////////////////////////////////////////////////////////////////////////////

#define _D3D_VTX_BLEND_FLAG_COUNT	5
#define _SUPPORTING_PARTS_DRAWING	FALSE

//////////////////////////////////////////////////////////////////////////////////////
// Local classes and structures:
//////////////////////////////////////////////////////////////////////////////////////

typedef struct 
{
	u32 nD3DValue;
	u32 nFangValue;
} _MapTable_t;


static _MapTable_t _aMap_CullDir_D3DCULL[] = 
{
	D3DCULL_CW,						FMESH_CULLDIR_CW,
	D3DCULL_CCW,					FMESH_CULLDIR_CCW,
	D3DCULL_NONE,					FMESH_CULLDIR_NONE,
};


//////////////////////////////////////////////////////////////////////////////////////
// Static variables:
//////////////////////////////////////////////////////////////////////////////////////

static BOOL8 _bModuleInitialized;
static BOOL8 _bWindowCreated;
static BOOL8 _bStream3Active;

static FResLoadReg_t _ResLoadRegistration;

static FMeshCullDir_e _nCurrentCullDir;

static const DWORD _aD3DVertexBlendFlags[_D3D_VTX_BLEND_FLAG_COUNT] = 
{
	D3DVBF_DISABLE,		// 0 segment matrices
	D3DVBF_DISABLE,		// 1 segment matrix
	D3DVBF_1WEIGHTS,	// 2 segment matrices
	D3DVBF_2WEIGHTS,	// 3 segment matrices
	D3DVBF_3WEIGHTS		// 4 segment matrices
};


//////////////////////////////////////////////////////////////////////////////////////
// Static function prototypes:
//////////////////////////////////////////////////////////////////////////////////////

static BOOL _WindowCreatedCallback( FDX8VidEvent_e nEvent );
static BOOL _ResLoadCreate( FResHandle_t hRes, void *pLoadedBase, u32 nLoadedBytes, cchar *pszResName );
static void _ResLoadDestroy( void *pResMem );
static FINLINE void _SetCullDir_P( FMeshCullDir_e nCullDir );


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
BOOL fdx8mesh_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

	_bWindowCreated = FALSE;

	fdx8vid_RegisterWindowCallbackFunction( _WindowCreatedCallback );

	fres_CopyType( _ResLoadRegistration.sResType, FMESH_RESTYPE );
	_ResLoadRegistration.pszFileExtension = "ape";
	_ResLoadRegistration.nMemType = FRESLOAD_MEMTYPE_TEMP;
	_ResLoadRegistration.nAlignment = 16;
	_ResLoadRegistration.pFcnCreate = _ResLoadCreate;
	_ResLoadRegistration.pFcnDestroy = _ResLoadDestroy;

	_nCurrentCullDir = FMESH_CULLDIR_NONE;

	if ( !fresload_RegisterHandler( &_ResLoadRegistration ) ) 
	{
		// Registration failed...
		DEVPRINTF( "fdx8mesh_ModuleStartup(): Could not register resource.\n" );
		return FALSE;
	}

	FMesh_nMaxMeshLights = 6;
	
	_bModuleInitialized = TRUE;

	_bStream3Active = FALSE;

	return TRUE;
}


//
//
//
void fdx8mesh_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

	fdx8vid_UnregisterWindowCallbackFunction( _WindowCreatedCallback );

	_bModuleInitialized = FALSE;
}

BOOL _bFlipCullDir=FALSE;


//
//
//
static FINLINE void _SetCullDir_P( FMeshCullDir_e nCullDir )
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_MESH );
	FASSERT( nCullDir >=0 && nCullDir < FMESH_CULLDIR_COUNT );

//	if ( _nCurrentCullDir != nCullDir ) 
	{
		if (_bFlipCullDir)
		{
			if (nCullDir == FMESH_CULLDIR_CW) { nCullDir = FMESH_CULLDIR_CCW; }
			else if (nCullDir == FMESH_CULLDIR_CCW) { nCullDir = FMESH_CULLDIR_CW; }
		}
		_nCurrentCullDir = nCullDir;
		fdx8_SetRenderState_CULLMODE( _aMap_CullDir_D3DCULL[nCullDir].nD3DValue );
	}
}


//
//
//
void fmesh_SetCullDir( FMeshCullDir_e nCullDir ) 
{
	_SetCullDir_P( nCullDir );
}


//
//
//
FMeshCullDir_e fmesh_GetCullDir( void )
{
	return _nCurrentCullDir;
}


//
//
//
void fmesh_FlipCullDir( BOOL bFlip )
{
	_bFlipCullDir = bFlip;
}


//
//
//
BOOL fmesh_FlushDrawPrep( void )
{
	return TRUE;
}


//
//
//
CFMeshInst::~CFMeshInst( void )
{
	u32 i;
	for ( i = 0; i < m_nColorStreamCount; i++ )
	{
		if ( m_papColorStreams && ((IDirect3DVertexBuffer8 **)m_papColorStreams)[i] ) 
		{
			((IDirect3DVertexBuffer8 **)m_papColorStreams)[i]->Unlock();
			FDX8_SAFE_RELEASE( ((IDirect3DVertexBuffer8 **)m_papColorStreams)[i] );
			((IDirect3DVertexBuffer8 **)m_papColorStreams)[i] = NULL;
		}
	}

	m_nColorStreamCount = 0;
	m_papColorStreams = NULL;
}


//
//
//
void CFMeshInst::SetColorStreams( u32 nStreamCount, ColorStream_t *paStreams ) 
{
	u32 i;

	FASSERT( m_pMesh && m_pMesh->pMeshIS );

#if FANG_WINGC
	// GameCube data: the stream holds one color (RGBA8) per GameCube diffuse index; on the GameCube
	// it replaced the mesh's diffuse array. Build one DX color stream per converted VB from the
	// diffuse index each vertex was built from (FDX8VB_t::pGCDiffuseIndex).
	{
		FDX8Mesh_t *pMeshIS = m_pMesh->pMeshIS;
		if ( nStreamCount == 1 && pMeshIS->nGCDiffuseCount && paStreams[0].paVertexColors &&
			 paStreams[0].nColorCount == pMeshIS->nGCDiffuseCount )
		{
			const u32 *pnSource = (const u32 *)paStreams[0].paVertexColors;
			const u32 nSourceCount = paStreams[0].nColorCount;
			FResFrame_t GCFrame = fres_GetFrame();
			IDirect3DVertexBuffer8 **papVB = (IDirect3DVertexBuffer8 **)fres_AllocAndZero( sizeof( IDirect3DVertexBuffer8 * ) * pMeshIS->nVBCount );
			BOOL bOK = ( papVB != NULL );
			for ( i = 0; bOK && i < pMeshIS->nVBCount; i++ )
			{
				const FDX8VB_t *pVB = &pMeshIS->aVB[i];
				if ( !pVB->pGCDiffuseIndex || !pVB->nVtxCount )
				{
					continue;	// never drawn (collision)
				}
				DWORD *pnDest = NULL;
				if ( FDX8_pDev->CreateVertexBuffer( sizeof( DWORD ) * pVB->nVtxCount, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &papVB[i] ) != D3D_OK ||
					 papVB[i]->Lock( 0, 0, (void **)&pnDest, 0 ) != D3D_OK || !pnDest )
				{
					bOK = FALSE;
					break;
				}
				for ( u32 v = 0; v < pVB->nVtxCount; v++ )
				{
					const u16 nIndex = pVB->pGCDiffuseIndex[v];
					const u32 nRGBA = ( nIndex < nSourceCount ) ? pnSource[nIndex] : 0xffffffff;
					pnDest[v] = ( nRGBA >> 8 ) | ( nRGBA << 24 );	// GameCube RGBA8 to D3D ARGB
				}
				papVB[i]->Unlock();
			}
			if ( bOK )
			{
				m_papColorStreams = papVB;
				m_nColorStreamCount = pMeshIS->nVBCount;
				m_nFlags |= FMESHINST_FLAG_VERT_RADIOSITY;
				return;
			}
			DEVPRINTF( "CFMeshInst::SetColorStreams() - Mesh %s : Could not build the DX color streams.  Color streams ignored.\n", m_pMesh->szName );
			for ( i = 0; papVB && i < pMeshIS->nVBCount; i++ )
			{
				FDX8_SAFE_RELEASE( papVB[i] );
			}
			fres_ReleaseFrame( GCFrame );
			m_nFlags &= ~(FMESHINST_FLAG_NOLIGHT_AMBIENT|FMESHINST_FLAG_NOLIGHT_DYNAMIC|FMESHINST_FLAG_LM|FMESHINST_FLAG_VERT_RADIOSITY);
			return;
		}
	}
#endif

	if ( nStreamCount != m_pMesh->pMeshIS->nVBCount )
	{
		DEVPRINTF( "CFMeshInst::SetColorStreams() - Mesh %s : Color stream data assumes %d VB's, mesh data has %d.  Color streams ignored.\n", m_pMesh->szName, nStreamCount, m_pMesh->pMeshIS->nVBCount );
		// As below and as the GameCube code does: without its baked lighting the mesh must be lit
		// normally, or instances flagged for static lighting render black.
		m_nFlags &= ~(FMESHINST_FLAG_NOLIGHT_AMBIENT|FMESHINST_FLAG_NOLIGHT_DYNAMIC|FMESHINST_FLAG_LM|FMESHINST_FLAG_VERT_RADIOSITY);
		return;
	}

	FResFrame_t Frame = fres_GetFrame();

	m_papColorStreams = fres_Alloc( sizeof( IDirect3DVertexBuffer8 *) * nStreamCount );
	if ( !m_papColorStreams )
	{
		DEVPRINTF( "CFMeshInst::SetColorStreams() - Mesh %s : Unable to allocate heap memory for color streams.  Color streams ignored.\n", m_pMesh->szName );
		return;
	}

	// Make sure that the vertex radiosity flag is set
	m_nFlags |= FMESHINST_FLAG_VERT_RADIOSITY;

	m_nColorStreamCount = nStreamCount;

	for ( i = 0; i < nStreamCount; i++ )
	{
		// Build a DX vertex buffer
		if ( paStreams[i].nColorCount != m_pMesh->pMeshIS->aVB[i].nVtxCount )
		{
			DEVPRINTF( "CFMeshInst::SetColorStreams() - Mesh %s : Color stream data assumes %d vertices for VB %d, actual mesh has %d.  Color streams ignored.\n", m_pMesh->szName, paStreams[i].nColorCount, i, m_pMesh->pMeshIS->aVB[i].nVtxCount );
			goto EXITWITHOUTSTREAMS;
		}

		if ( paStreams[i].nColorCount == 0 )
		{
			FASSERT( !paStreams[i].paVertexColors );
			((IDirect3DVertexBuffer8 **)m_papColorStreams)[i] = NULL;
		}

		u32 nBytes = sizeof( DWORD ) * paStreams[i].nColorCount;
		HRESULT hResult = FDX8_pDev->CreateVertexBuffer( nBytes, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &((IDirect3DVertexBuffer8 **)m_papColorStreams)[i] );
		if ( hResult != D3D_OK )
		{
			// Failed to create VB...
			DEVPRINTF( "CFMeshInst::SetColorStreams() - Mesh %s : Unable to create DX VB for color streams.  Color streams ignored.\n", m_pMesh->szName );
			goto EXITWITHOUTSTREAMS;
		}

		u8 *pData;
		((IDirect3DVertexBuffer8 **)m_papColorStreams)[i]->Lock( 0, 0, (void **)&pData, 0 );
		if ( !pData )
		{
			DEVPRINTF( "CFMeshInst::SetColorStreams() - Mesh %s : Unable to lock DX VB for color streams.  Color streams ignored.\n", m_pMesh->szName );
			goto EXITWITHOUTSTREAMS;
		}
		fang_MemCopy( pData, paStreams[i].paVertexColors, sizeof( DWORD ) * paStreams[i].nColorCount );

		((IDirect3DVertexBuffer8 **)m_papColorStreams)[i]->Unlock();
	}

	return;

EXITWITHOUTSTREAMS:
	// Remove static lighting
	m_nFlags &= ~(FMESHINST_FLAG_NOLIGHT_AMBIENT|FMESHINST_FLAG_NOLIGHT_DYNAMIC|FMESHINST_FLAG_LM|FMESHINST_FLAG_VERT_RADIOSITY);
	
	m_nColorStreamCount = 0;
	m_papColorStreams = NULL;
}


//
//
//
void CFMeshInst::CacheMeshData( u32 nPriority ) 
{
#if FANG_PLATFORM_XB	
	u32 i, ii;

	if ( nPriority < 2 )
	{
		// Bring streaming textures into the cache
		u32 nLightMapCount;
		if ( m_panLightRegisterLMOverride )
		{
			nLightMapCount = m_panLightRegisterLMOverride[0];

			for ( ii = 0; ii < nLightMapCount; ii++ )
			{
				FShTexInst_t *pTexInst = (FShTexInst_t *)m_panLightRegisterLMOverride[1 + (ii * 3)];
				FASSERT( pTexInst );

				if ( pTexInst->TexInst.GetTexDef() && (pTexInst->TexInst.GetTexDef()->TexInfo.nFlags & FTEX_FLAG_STREAMING) )
				{
					FTexData_t *pTexData = pTexInst->TexInst.GetTexDef()->pTexData;
					if ( pTexData->nFlags & FDX8TEXFLAGS_NOT_IN_CACHE )
					{
						u8 *pCachedData = (u8 *)FDS_StreamMgr.AccessData( pTexData->pStreamingHandle );
						if ( pCachedData )
						{
							// Streamed texture just entered the cache, so register it with the texture header
							pTexData->pD3DTexture->BlockUntilNotBusy();
							pTexData->pD3DTexture->Lock = 0;
							pTexData->nFlags &= ~FDX8TEXFLAGS_NOT_IN_CACHE;
							pTexData->pD3DTexture->Register( pCachedData + 128 );
						}
					}
					else
					{
						void *pCachedData = FDS_StreamMgr.AccessData( pTexData->pStreamingHandle );
						if ( !pCachedData )
						{
							// Streamed texture is no longer in the local streaming cache
							pTexData->nFlags |= FDX8TEXFLAGS_NOT_IN_CACHE;
							pTexData->pD3DTexture->Data = NULL;
						}
					}
				}
			}
		}
		else if ( m_nFlags & FMESHINST_FLAG_WORLD_GEO )
		{
			FMeshMaterial_t *pMaterial = m_pMesh->aMtl;
			for ( i = 0; i < m_pMesh->nMaterialCount; i++, pMaterial++ )
			{
				nLightMapCount = pMaterial->pnShLightRegisters[FSHADERS_LIGHT_REG_LMCOUNT];

				for ( ii = 0; ii < nLightMapCount; ii++ )
				{
					FShTexInst_t *pTexInst = (FShTexInst_t *)pMaterial->pnShLightRegisters[FSHADERS_LIGHT_REG_LM + (ii * 3)];
					FASSERT( pTexInst );
					if ( pTexInst->TexInst.GetTexDef() && (pTexInst->TexInst.GetTexDef()->TexInfo.nFlags & FTEX_FLAG_STREAMING) )
					{
						FTexData_t *pTexData = pTexInst->TexInst.GetTexDef()->pTexData;
						if ( pTexData->nFlags & FDX8TEXFLAGS_NOT_IN_CACHE )
						{
							u8 *pCachedData = (u8 *)FDS_StreamMgr.AccessData( pTexData->pStreamingHandle );
							if ( pCachedData )
							{
								// Streamed texture just entered the cache, so register it with the texture header
								pTexData->pD3DTexture->BlockUntilNotBusy();
								pTexData->pD3DTexture->Lock = 0;
								pTexData->nFlags &= ~FDX8TEXFLAGS_NOT_IN_CACHE;
								pTexData->pD3DTexture->Register( pCachedData + 128 );
							}
						}
						else
						{
							void *pCachedData = FDS_StreamMgr.AccessData( pTexData->pStreamingHandle );
							if ( !pCachedData )
							{
								// Streamed texture is no longer in the local streaming cache
								pTexData->nFlags |= FDX8TEXFLAGS_NOT_IN_CACHE;
								pTexData->pD3DTexture->Data = NULL;
							}
						}
					}
				}
			}
		}
	}
#endif // FANG_PLATFORM_XB
}


//
//
//
BOOL CFMeshInst::DrawPrep_P( BOOL bFlushImmediate ) 
{
	if ( m_nFlags & (FMESHINST_FLAG_POSTER_Y|FMESHINST_FLAG_POSTER_X|FMESHINST_FLAG_POSTER_Z) )
	{
		u8 nNumPops = PushXfm();

		// Store the concatenated model matrix and scale for later use
		m_PosMtx.Set( FXfm_pModel->m_MtxF );
		m_fMatrixScale = FXfm_pModel->m_fScaleF;

		CFXfm::PopModel( nNumPops );
	}
	else
	{
		// Store the concatenated model matrix and scale for later use
		m_PosMtx.Set( m_Xfm.m_MtxF );
		m_fMatrixScale = m_Xfm.m_fScaleF;
	}

	return TRUE;
}



//
//
//
#if FANG_WINGC
static void _PortLogD3DState( cchar *pszTag, cchar *pszMesh, u32 nMtl );
#endif

BOOL CFMeshInst::DrawMaterialLight_P( FViewportPlanesMask_t nCrossesPlanesMask, FMeshMaterial_t *pMaterial, u32 nLODIndex )
{
	FASSERT( _bWindowCreated );
	FASSERT( nCrossesPlanesMask != -1 );

	CFMtx43A Mtx43A, *pMtx43A;
	u32 nBoneMtxNum, nStrip;
	s32 nShadowID;
	BOOL bFastPass = FALSE;

	#if FPERF_ENABLE
		u32 nPreStripCount = FPerf_nRawStripTriCount;
		u32 nPreListCount = FPerf_nRawListTriCount;
	#endif

	#if _SUPPORTING_PARTS_DRAWING
		if ( !(pMaterial->nPartIDMask & m_nDrawnPartsMask) )
		{
			return FALSE;
		}
	#endif

	#if FANG_PLATFORM_WIN
		// Setup clipping
		fdx8_SetRenderState_CLIPPING( nCrossesPlanesMask != 0 );
	#endif

	fdx8_SetRenderState_FILLMODE( FRenderer_nD3DFillMode );
	_SetCullDir_P( (FMeshCullDir_e)m_nCullDirection );
	FDX8Mesh_t *pMeshIS = m_pMesh->pMeshIS;
	u32 nMeshBoneCount = m_pMesh->nBoneCount;

	if ( !(m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH) )
	{
		if ( nMeshBoneCount == 0 ) 
		{
			// There are no bones in the entire mesh. We can just set a single model matrix...
			fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &m_PosMtx, TRUE );
			fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );
			fdx8xfm_SetRenormalizeNormalsIfNeeded( m_fMatrixScale );
		}
		nShadowID = 0x01;
	}
	else
	{
		nShadowID = 0x00;
	}

	// Setup color stream management
	s32 nColorStreamVBIndex = -1;
	if ( m_papColorStreams && !FMesh_bRenderShadows )
	{
		fdx8sh_ToggleExternalColorStream( TRUE );
		_bStream3Active = TRUE;
	}
	else if ( _bStream3Active )
	{
		FDX8_pDev->SetStreamSource( 3, NULL, 0 );
		fdx8sh_ToggleExternalColorStream( FALSE );
		_bStream3Active = FALSE;
	}

	if ( !SetupShaderParameters( pMaterial, FSHADERS_PASS_LIGHTING ) )
	{
		return FALSE;
	}

	// Submit the material's clusters once for each pass that is required
#if FANG_DEBUG_BUILD
	FDX8VB_t *pVB = NULL;
#endif
	FMeshSeg_t *pSeg = NULL;
	FMeshSeg_t *aSeg = m_pMesh->aSeg;
	s32 nCurrentIB = -1, nPass;
	s32 nTotalPasses = fsh_GetCurrentNumPasses( &bFastPass );
#if FMESH_ALLOW_POSTER_BONES
	BOOL bPosterBones = FALSE;
	CFMtx43A PosterMtx;

	if ( (m_nFlags & (FMESHINST_FLAG_POSTER_X | FMESHINST_FLAG_POSTER_Y | FMESHINST_FLAG_POSTER_Z)) && (m_nFlags & FMESHINST_FLAG_POSTER_BONES) )
	{
		if ( m_pMesh->nBoneCount )
		{
			bPosterBones = TRUE;
			PosterMtx.Mul( m_PosMtx, m_Xfm.m_MtxR );
		}
	}
#endif

	for ( nPass = 0; nPass < nTotalPasses; nPass++ )
	{
		fsh_SetPassIdx( nPass );
		fsh_ExecuteCurrent( FMesh_bRenderShadows, nShadowID );
#if FANG_WINGC
		_PortLogD3DState( bFastPass ? "LIGHT-FAST" : "LIGHT", m_pMesh->szName, (u32)(pMaterial - m_pMesh->aMtl) );
		{
			// Port diagnostic: segment/bone bindings of the player mesh.
			static u32 _nLoggedGlitch = 0;
			if ( Fang_bPortDiag && _nLoggedGlitch < 3 && fclib_stricmp( m_pMesh->szName, "grdggltch00" ) == 0 && nPass == 0 )
			{
				_nLoggedGlitch++;
				FDX8MeshMaterial_t *pDXMat = (FDX8MeshMaterial_t *)pMaterial->pPlatformData;
				DEVPRINTF( "PORT-GLITCH mtl=%u clusters=%u bones=%u usedBones=%u segs=%u instFlags=%08x palette=%p posMtx=(%.1f,%.1f,%.1f) scale=%.3f\n",
					(u32)(pMaterial - m_pMesh->aMtl), (u32)pDXMat->nClusterCount, (u32)m_pMesh->nBoneCount, (u32)m_pMesh->nUsedBoneCount,
					(u32)m_pMesh->nSegCount, (u32)m_nFlags, m_apBoneMtxPalette, m_PosMtx.m_vPos.x, m_PosMtx.m_vPos.y, m_PosMtx.m_vPos.z, m_fMatrixScale );
				for ( u32 c = 0; c < pDXMat->nClusterCount && c < 6; c++ )
				{
					const FDX8MeshCluster_t *pC = &pDXMat->aCluster[c];
					const FMeshSeg_t *pS = &m_pMesh->aSeg[pC->nSegmentIdx];
					const u32 nBone = pS->nBoneMtxCount ? pS->anBoneMtxIndex[0] : 0xff;
					const CFMtx43A *pBoneMtx = (m_apBoneMtxPalette && nBone < m_pMesh->nBoneCount) ? m_apBoneMtxPalette[nBone] : NULL;
					DEVPRINTF( "PORT-GLITCH   cluster %u: seg=%u segBones=%u bone=%u tris=%u vb=%u part=%u lod=%u boneMtxPos=(%.1f,%.1f,%.1f) right=(%.2f,%.2f,%.2f)\n",
						c, (u32)pC->nSegmentIdx, (u32)pS->nBoneMtxCount, nBone, (u32)pC->TriList.nTriCount, (u32)pC->nVBIndex, (u32)pC->nPartID, (u32)pC->nLODID,
						pBoneMtx ? pBoneMtx->m_vPos.x : 0.0f, pBoneMtx ? pBoneMtx->m_vPos.y : 0.0f, pBoneMtx ? pBoneMtx->m_vPos.z : 0.0f,
						pBoneMtx ? pBoneMtx->m_vRight.x : 0.0f, pBoneMtx ? pBoneMtx->m_vRight.y : 0.0f, pBoneMtx ? pBoneMtx->m_vRight.z : 0.0f );
				}
			}
		}
#endif

		// Progress through all the clusters for this material, submitting for render the relevant ones
		FDX8MeshCluster_t *pCluster = ((FDX8MeshMaterial_t *)pMaterial->pPlatformData)->aCluster;
		FDX8MeshCluster_t *pEndCluster = &pCluster[((FDX8MeshMaterial_t *)pMaterial->pPlatformData)->nClusterCount];
		for ( ;pCluster < pEndCluster; pCluster++ )
		{
			if ( !(m_nDrawnPartsMask & (1 << pCluster->nPartID)) )
			{
				continue;
			}

			if ( pCluster->nLODID != nLODIndex )
			{
				continue;
			}

			// Select the appropriate vertex buffer for this cluster
			BOOL bUseLightmaps = (FSh_shaderType == SHADERTYPE_DIFFUSE || FSh_shaderType == SHADERTYPE_TRANSLUCENCY);

			fdx8vb_Select( &pMeshIS->aVB[ pCluster->nVBIndex ], FALSE, bUseLightmaps, TRUE );
			fsh_CheckVB();

			// Activate the color stream for this cluster, if there is one
			if ( m_papColorStreams && nColorStreamVBIndex != pCluster->nVBIndex )
			{
				FDX8_pDev->SetStreamSource( 3, ((IDirect3DVertexBuffer8 **)m_papColorStreams)[pCluster->nVBIndex], sizeof(DWORD) );
				nColorStreamVBIndex = pCluster->nVBIndex;
				_bStream3Active = TRUE;
			}

			// Set the index buffer
			if ( nCurrentIB != pCluster->nIBIndex )
			{
				nCurrentIB = pCluster->nIBIndex;
				FDX8_pDev->SetIndices( (IDirect3DIndexBuffer8 *)pMeshIS->apDXIB[nCurrentIB], 0 );
			}

			D3DXMATRIX boneMtx;

			if ( nMeshBoneCount ) 
			{
				FASSERT( pCluster->nSegmentIdx < m_pMesh->nSegCount );

				if ( pSeg != &aSeg[pCluster->nSegmentIdx] )
				{
					pSeg = &aSeg[pCluster->nSegmentIdx];

					if ( pSeg->nBoneMtxCount == 0 )
					{
						// No bones for this segment, so just set this mesh instance's xfm
						// as the model matrix...
						fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );
						fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &m_PosMtx, TRUE );
						fdx8xfm_SetRenormalizeNormalsIfNeeded( m_fMatrixScale );
					} 
					else if ( pSeg->nBoneMtxCount == 1 ) 
					{
						// This is a non-skinned segment...

						fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );

						if ( !(m_nFlags & FMESHINST_FLAG_NOBONES) ) 
						{
							#if FMESH_ALLOW_POSTER_BONES
							if ( !bPosterBones )
							{
								fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ], TRUE );
							} 
							else
							{
								Mtx43A.Mul33( PosterMtx, *m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ] );
								Mtx43A.m_vPos = m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ]->m_vPos;
								fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &Mtx43A, TRUE );
							}
							#else
							fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ], TRUE );
							#endif

							fdx8xfm_SetRenormalizeNormalsIfNeeded( m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ] );
						} 
						else 
						{
							// The FMESHINST_FLAG_NOBONES flag is set and this is a non-skinned segment.
							// This means that the vertices have already been transformed into bone space,
							// and we need to transform them back into at-rest model space...
//							Mtx43A.Mul( m_Xfm.m_MtxF, m_pMesh->pBoneArray[ pSeg->anBoneMtxIndex[0] ].AtRestBoneToModelMtx );
							Mtx43A.Mul( m_PosMtx, m_pMesh->pBoneArray[ pSeg->anBoneMtxIndex[0] ].AtRestBoneToModelMtx );
							fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &Mtx43A, TRUE );
							fdx8xfm_SetRenormalizeNormalsIfNeeded( &Mtx43A );
						}
					} 
					else 
					{
						// This is a skinned segment...
						if ( !(m_nFlags & FMESHINST_FLAG_NOBONES) ) 
						{
							fdx8_SetRenderState_VERTEXBLEND( _aD3DVertexBlendFlags[pSeg->nBoneMtxCount] );

							for( nBoneMtxNum=0; nBoneMtxNum<pSeg->nBoneMtxCount; nBoneMtxNum++ ) 
							{
								pMtx43A = m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[nBoneMtxNum] ];
								D3DXMatrixTranspose(&boneMtx, (const D3DXMATRIX *)pMtx43A);
								FDX8_pDev->SetVertexShaderConstant(CV_BONEMTX0_0 + 3*nBoneMtxNum, &boneMtx(0, 0), 3);
							}
						} 
						else 
						{
							fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );
							fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &m_PosMtx, TRUE );
							fdx8xfm_SetRenormalizeNormalsIfNeeded( m_fMatrixScale );
						}
					}
				}
			}

			// Render the tri-list...
			if ( pCluster->TriList.nTriCount )
			{
				if (FAILED(FDX8_pDev->DrawIndexedPrimitive(
					D3DPT_TRIANGLELIST,
					pCluster->TriList.nVtxIndexMin,
					pCluster->TriList.nVtxIndexRange,
					pCluster->TriList.nStartVindex,
					pCluster->TriList.nTriCount
				)))
				{
					fang_DevPrintf("fdx8mesh::DrawInstanceMaterial_P - DrawPrimitive failed.\n");
				}

				#if FPERF_ENABLE
					FPerf_nRawTriListCount++;
					FPerf_nRawListTriCount += pCluster->TriList.nTriCount;
					FPerf_nRawVertexCount += (pCluster->TriList.nTriCount<<1) + pCluster->TriList.nTriCount;
				#endif
			}

			// Render the strip-list...
			FDX8MeshStrip_t *pStrip;
			for ( nStrip=0, pStrip = pCluster->paStripBuffer; nStrip < pCluster->nStripCount; nStrip++, pStrip++ ) 
			{
				if (FAILED(FDX8_pDev->DrawIndexedPrimitive(
					D3DPT_TRIANGLESTRIP,
					pStrip->nVtxIndexMin,
					pStrip->nVtxIndexRange,
					pStrip->nStartVindex,
					pStrip->nTriCount
				)))
				{
					fang_DevPrintf("fdx8mesh::DrawInstanceMaterial_P - DrawPrimitive failed.\n");
				}

				#if FPERF_ENABLE
					FPerf_nRawTriStripCount++;
					FPerf_nRawStripTriCount += pStrip->nTriCount;
					FPerf_nRawVertexCount += pStrip->nTriCount + 2;
				#endif
			}
		}
	}

	fsh_EndExecute();

	if ( m_pMesh->nTexLayerIDCount ) 
	{
		// Reset the texture matrices if there were texture layer effects...
		fsh_TexCoordMtx_ResetList();
		fsh_TexOverride_ResetList();
	}

#if FPERF_ENABLE
	u32 nTrisRendered = (FPerf_nRawStripTriCount - nPreStripCount) + (FPerf_nRawListTriCount - nPreListCount);
	FPerf_nTotalTrisLighting += nTrisRendered;
	FPerf_nMeshTris += nTrisRendered / nTotalPasses;
#endif

	return bFastPass;
}


//
//
//
#if FANG_WINGC
// Port diagnostic: dumps the D3D9 state a pass draws with (first few calls after frame ~300).
static void _PortLogD3DState( cchar *pszTag, cchar *pszMesh, u32 nMtl )
{
	// Enabled with MA_PORT_D3DPROBE=1.
	static int _nEnabled = -1;
	if ( _nEnabled < 0 )
	{
		char szVal[8];
		_nEnabled = GetEnvironmentVariableA( "MA_PORT_D3DPROBE", szVal, sizeof(szVal) ) > 0 ? 1 : 0;
	}
	static u32 _nCalls = 0, _anLogged[2] = { 0, 0 };
	u32 &nLogged = _anLogged[ pszTag[0] == 'S' ? 1 : 0 ];
	if ( !_nEnabled || ++_nCalls < 150000 || nLogged >= 8 )
	{
		return;
	}
	nLogged++;
	IDirect3DDevice9 *pDev = FDX8_pDev->GetD3D9Device();
	DWORD z, zf, zw, ab, sb, db, at, cull, cw, bias, fog;
	pDev->GetRenderState( D3DRS_ZENABLE, &z );
	pDev->GetRenderState( D3DRS_ZFUNC, &zf );
	pDev->GetRenderState( D3DRS_ZWRITEENABLE, &zw );
	pDev->GetRenderState( D3DRS_ALPHABLENDENABLE, &ab );
	pDev->GetRenderState( D3DRS_SRCBLEND, &sb );
	pDev->GetRenderState( D3DRS_DESTBLEND, &db );
	pDev->GetRenderState( D3DRS_ALPHATESTENABLE, &at );
	pDev->GetRenderState( D3DRS_CULLMODE, &cull );
	pDev->GetRenderState( D3DRS_COLORWRITEENABLE, &cw );
	pDev->GetRenderState( D3DRS_DEPTHBIAS, &bias );
	pDev->GetRenderState( D3DRS_FOGENABLE, &fog );
	IDirect3DBaseTexture9 *pT0 = NULL, *pT1 = NULL;
	pDev->GetTexture( 0, &pT0 );
	pDev->GetTexture( 1, &pT1 );
	DWORD op0, a10, a20, tci0, op1, a11, a21, tci1, aop0;
	pDev->GetTextureStageState( 0, D3DTSS_COLOROP, &op0 );
	pDev->GetTextureStageState( 0, D3DTSS_COLORARG1, &a10 );
	pDev->GetTextureStageState( 0, D3DTSS_COLORARG2, &a20 );
	pDev->GetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, &tci0 );
	pDev->GetTextureStageState( 0, D3DTSS_ALPHAOP, &aop0 );
	pDev->GetTextureStageState( 1, D3DTSS_COLOROP, &op1 );
	pDev->GetTextureStageState( 1, D3DTSS_COLORARG1, &a11 );
	pDev->GetTextureStageState( 1, D3DTSS_COLORARG2, &a21 );
	pDev->GetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, &tci1 );
	IDirect3DVertexShader9 *pVS = NULL;
	IDirect3DPixelShader9 *pPS = NULL;
	pDev->GetVertexShader( &pVS );
	pDev->GetPixelShader( &pPS );
	DWORD fvf = 0;
	pDev->GetFVF( &fvf );
	DEVPRINTF( "PORT-D3D %s mesh=%s mtl=%u z=%u zf=%u zw=%u ab=%u sb=%u db=%u at=%u cull=%u cw=%x bias=%08x fog=%u "
		"t0=%p op0=%u a1=%x a2=%x tci0=%x aop0=%u t1=%p op1=%u a1=%x a2=%x tci1=%x vs=%p ps=%p fvf=%x\n",
		pszTag, pszMesh, nMtl, z, zf, zw, ab, sb, db, at, cull, cw, bias, fog,
		pT0, op0, a10, a20, tci0, aop0, pT1, op1, a11, a21, tci1, pVS, pPS, fvf );
	{
		DWORD nMip, nMin, nMag, nMaxMip, nBias, nAddrU;
		pDev->GetSamplerState( 0, D3DSAMP_MIPFILTER, &nMip );
		pDev->GetSamplerState( 0, D3DSAMP_MINFILTER, &nMin );
		pDev->GetSamplerState( 0, D3DSAMP_MAGFILTER, &nMag );
		pDev->GetSamplerState( 0, D3DSAMP_MAXMIPLEVEL, &nMaxMip );
		pDev->GetSamplerState( 0, D3DSAMP_MIPMAPLODBIAS, &nBias );
		pDev->GetSamplerState( 0, D3DSAMP_ADDRESSU, &nAddrU );
		u32 nLevels = 0, nW = 0, nH = 0;
		IDirect3DBaseTexture9 *pTex = NULL;
		pDev->GetTexture( 0, &pTex );
		if ( pTex )
		{
			nLevels = pTex->GetLevelCount();
			if ( pTex->GetType() == D3DRTYPE_TEXTURE )
			{
				D3DSURFACE_DESC Desc;
				((IDirect3DTexture9 *)pTex)->GetLevelDesc( 0, &Desc );
				nW = Desc.Width; nH = Desc.Height;
			}
			pTex->Release();
		}
		DEVPRINTF( "PORT-D3D   s0 mip=%u min=%u mag=%u maxmip=%u lodbias=%08x addrU=%u tex=%ux%u levels=%u\n",
			nMip, nMin, nMag, nMaxMip, nBias, nAddrU, nW, nH, nLevels );
	}
	float afTexMtx[8];
	pDev->GetVertexShaderConstantF( 74, afTexMtx, 2 );
	DEVPRINTF( "PORT-D3D   texmtx0 = (%.3f %.3f %.3f %.3f) (%.3f %.3f %.3f %.3f)\n",
		afTexMtx[0], afTexMtx[1], afTexMtx[2], afTexMtx[3], afTexMtx[4], afTexMtx[5], afTexMtx[6], afTexMtx[7] );
	IDirect3DVertexDeclaration9 *pDecl = NULL;
	if ( SUCCEEDED( pDev->GetVertexDeclaration( &pDecl ) ) && pDecl )
	{
		D3DVERTEXELEMENT9 aElem[MAXD3DDECLLENGTH];
		UINT nElem = MAXD3DDECLLENGTH;
		if ( SUCCEEDED( pDecl->GetDeclaration( aElem, &nElem ) ) )
		{
			char szDecl[512];
			u32 nLen = 0;
			for ( UINT e = 0; e < nElem && aElem[e].Stream != 0xff && nLen < sizeof(szDecl) - 32; e++ )
			{
				nLen += _snprintf( szDecl + nLen, sizeof(szDecl) - nLen, "[s%u o%u t%u u%u.%u] ",
					aElem[e].Stream, aElem[e].Offset, aElem[e].Type, aElem[e].Usage, aElem[e].UsageIndex );
			}
			szDecl[sizeof(szDecl) - 1] = 0;
			DEVPRINTF( "PORT-D3D   decl %s\n", szDecl );
		}
		pDecl->Release();
	}
	for ( UINT s = 0; s < 3; s++ )
	{
		IDirect3DVertexBuffer9 *pStreamVB = NULL;
		UINT nOffset = 0, nStride = 0;
		if ( SUCCEEDED( pDev->GetStreamSource( s, &pStreamVB, &nOffset, &nStride ) ) )
		{
			DEVPRINTF( "PORT-D3D   stream%u vb=%p stride=%u\n", s, pStreamVB, nStride );
			if ( pStreamVB ) pStreamVB->Release();
		}
	}
	if ( nLogged == 1 )
	{
		// Dump the bound shaders' bytecode once per pass type (disassemble with fxc /dumpbin).
		CreateDirectoryA( "shaderdump", NULL );
		for ( u32 nWhich = 0; nWhich < 2; nWhich++ )
		{
			UINT nBytes = 0;
			void *pCode = NULL;
			if ( nWhich == 0 && pVS && SUCCEEDED( pVS->GetFunction( NULL, &nBytes ) ) && nBytes )
			{
				pCode = malloc( nBytes );
				pVS->GetFunction( pCode, &nBytes );
			}
			else if ( nWhich == 1 && pPS && SUCCEEDED( pPS->GetFunction( NULL, &nBytes ) ) && nBytes )
			{
				pCode = malloc( nBytes );
				pPS->GetFunction( pCode, &nBytes );
			}
			if ( pCode )
			{
				char szPath[128];
				_snprintf( szPath, sizeof(szPath) - 1, "shaderdump\\%s_%s.bin", pszTag, nWhich ? "ps" : "vs" );
				szPath[sizeof(szPath) - 1] = 0;
				FILE *pFile = fopen( szPath, "wb" );
				if ( pFile )
				{
					fwrite( pCode, nBytes, 1, pFile );
					fclose( pFile );
				}
				free( pCode );
			}
		}
	}
	if ( pT0 ) pT0->Release();
	if ( pT1 ) pT1->Release();
	if ( pVS ) pVS->Release();
	if ( pPS ) pPS->Release();
}
#endif

void CFMeshInst::DrawMaterialSurface_P( FViewportPlanesMask_t nCrossesPlanesMask, FMeshMaterial_t *pMaterial, u32 nLODIndex )
{
	FASSERT( _bWindowCreated );
	FASSERT( nCrossesPlanesMask != -1 );

	CFMtx43A Mtx43A, *pMtx43A;
	u32 nBoneMtxNum, nStrip;

	#if FPERF_ENABLE
		u32 nPreStripCount = FPerf_nRawStripTriCount;
		u32 nPreListCount = FPerf_nRawListTriCount;
	#endif

	#if _SUPPORTING_PARTS_DRAWING
		if ( !(pMaterial->nPartIDMask & m_nDrawnPartsMask) )
		{
			return;
		}
	#endif

	#if FANG_PLATFORM_WIN
		fdx8_SetRenderState_CLIPPING( nCrossesPlanesMask != 0 );
	#endif

//	fdx8_SetRenderState_FILLMODE( FRenderer_nD3DFillMode );
	_SetCullDir_P( (FMeshCullDir_e)m_nCullDirection );
	FDX8Mesh_t *pMeshIS = m_pMesh->pMeshIS;
	u32 nMeshBoneCount = m_pMesh->nBoneCount;

	if ( !(m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH) )
	{
		if ( nMeshBoneCount == 0 ) 
		{
			// There are no bones in the entire mesh. We can just set a single model matrix...
			fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &m_PosMtx, TRUE );
			fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );
			fdx8xfm_SetRenormalizeNormalsIfNeeded( m_fMatrixScale );
		} 
	}

	if ( !SetupShaderParameters( pMaterial, FSHADERS_PASS_SURFACE ) )
	{
		return;
	}

	// Pass in procedural data
	fsh_SetProcedural( m_pProcedural );

	// Submit the material's clusters once for each pass that is required
#if FANG_DEBUG_BUILD
	FDX8VB_t *pVB = NULL;
#endif
	FMeshSeg_t *pSeg = NULL;
	FMeshSeg_t *aSeg = m_pMesh->aSeg;
	s32 nCurrentIB = -1;

	fsh_SetSurfaceOnly( !!(m_nFlags&FMESHINST_FLAG_SURFACEONLY) );

	if ( FMesh_nLastSurfaceMaterialCRC == pMaterial->nDLHashKey )
	{
		fsh_FastSurfaceExecute();
#if FPERF_ENABLE
		FPerf_nFastSurfaceShaderCount++;
#endif
	}
	else
	{
		fsh_ExecuteCurrent( FALSE, 0 );
		FMesh_nLastSurfaceMaterialCRC = pMaterial->nDLHashKey;
#if FPERF_ENABLE
		FPerf_nFullSurfaceShaderCount++;
#endif
	}

	// Progress through all the clusters for this material, submitting for render the relevant ones
	FDX8MeshCluster_t *pCluster = ((FDX8MeshMaterial_t *)pMaterial->pPlatformData)->aCluster;
	FDX8MeshCluster_t *pEndCluster = &pCluster[((FDX8MeshMaterial_t *)pMaterial->pPlatformData)->nClusterCount];
#if FMESH_ALLOW_POSTER_BONES
	BOOL bPosterBones = FALSE;
	CFMtx43A PosterMtx;

	if ( (m_nFlags & (FMESHINST_FLAG_POSTER_X | FMESHINST_FLAG_POSTER_Y | FMESHINST_FLAG_POSTER_Z)) && (m_nFlags & FMESHINST_FLAG_POSTER_BONES) )
	{
		if ( m_pMesh->nBoneCount )
		{
			bPosterBones = TRUE;
			PosterMtx.Mul( m_PosMtx, m_Xfm.m_MtxR );
		}
	}
#endif

	for ( ;pCluster < pEndCluster; pCluster++ )
	{
		if ( !(m_nDrawnPartsMask & (1 << pCluster->nPartID)) )
		{
			continue;
		}

		if ( pCluster->nLODID != nLODIndex )
		{
			continue;
		}

		// Select the appropriate vertex buffer for this cluster
		fdx8vb_Select( &pMeshIS->aVB[ pCluster->nVBIndex ], FALSE, FALSE, TRUE );
		fsh_CheckVB();
#if FANG_WINGC
		if ( FSh_shaderType == SHADERTYPE_SURFACE )
		{
			_PortLogD3DState( "SURF", m_pMesh->szName, (u32)(pMaterial - m_pMesh->aMtl) );
			// Port debug: MA_PORT_RAWSURF=1 draws the surface pass opaque, ignoring depth.
			static int _nRawSurf = -1;
			if ( _nRawSurf < 0 )
			{
				char szVal[8];
				_nRawSurf = GetEnvironmentVariableA( "MA_PORT_RAWSURF", szVal, sizeof(szVal) ) > 0 ? 1 : 0;
			}
			if ( _nRawSurf )
			{
				FDX8_pDev->GetD3D9Device()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
				FDX8_pDev->GetD3D9Device()->SetRenderState( D3DRS_ZFUNC, D3DCMP_ALWAYS );
			}
		}
#endif

		// Set the index buffer
		if ( nCurrentIB != pCluster->nIBIndex )
		{
			nCurrentIB = pCluster->nIBIndex;
			FDX8_pDev->SetIndices( (IDirect3DIndexBuffer8 *)pMeshIS->apDXIB[nCurrentIB], 0 );
		}

		D3DXMATRIX boneMtx;

		if ( nMeshBoneCount ) 
		{
			FASSERT( pCluster->nSegmentIdx < m_pMesh->nSegCount );

			if ( pSeg != &aSeg[pCluster->nSegmentIdx] )
			{
				pSeg = &aSeg[pCluster->nSegmentIdx];

				if ( pSeg->nBoneMtxCount == 0 )
				{
					// No bones for this segment, so just set this mesh instance's xfm
					// as the model matrix...
					fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );
					fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &m_PosMtx, TRUE );
					fdx8xfm_SetRenormalizeNormalsIfNeeded( m_fMatrixScale );
				} 
				else if ( pSeg->nBoneMtxCount == 1 ) 
				{
					// This is a non-skinned segment...

					fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );

					if ( !(m_nFlags & FMESHINST_FLAG_NOBONES) ) 
					{
						#if FMESH_ALLOW_POSTER_BONES
						if ( !bPosterBones )
						{
							fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ], TRUE );
						} 
						else
						{
							Mtx43A.Mul33( PosterMtx, *m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ] );
							Mtx43A.m_vPos = m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ]->m_vPos;
							fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &Mtx43A, TRUE );
						}
						#else
						fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ], TRUE );
						#endif

						fdx8xfm_SetRenormalizeNormalsIfNeeded( m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ] );
					} 
					else 
					{
						// The FMESHINST_FLAG_NOBONES flag is set and this is a non-skinned segment.
						// This means that the vertices have already been transformed into bone space,
						// and we need to transform them back into at-rest model space...
//						Mtx43A.Mul( m_Xfm.m_MtxF, m_pMesh->pBoneArray[ pSeg->anBoneMtxIndex[0] ].AtRestBoneToModelMtx );
						Mtx43A.Mul( m_PosMtx, m_pMesh->pBoneArray[ pSeg->anBoneMtxIndex[0] ].AtRestBoneToModelMtx );
						fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &Mtx43A, TRUE );
						fdx8xfm_SetRenormalizeNormalsIfNeeded( &Mtx43A );
					}
				} 
				else 
				{
					// This is a skinned segment...

					if ( !(m_nFlags & FMESHINST_FLAG_NOBONES) ) 
					{
						fdx8_SetRenderState_VERTEXBLEND( _aD3DVertexBlendFlags[pSeg->nBoneMtxCount] );

						for( nBoneMtxNum=0; nBoneMtxNum<pSeg->nBoneMtxCount; nBoneMtxNum++ ) 
						{
							pMtx43A = m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[nBoneMtxNum] ];
							D3DXMatrixTranspose(&boneMtx, (const D3DXMATRIX *)pMtx43A);
							FDX8_pDev->SetVertexShaderConstant(CV_BONEMTX0_0 + 3*nBoneMtxNum, &boneMtx(0, 0), 3);
						}
					} 
					else 
					{
						fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );
						fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &m_PosMtx, TRUE );
						fdx8xfm_SetRenormalizeNormalsIfNeeded( m_fMatrixScale );
					}
				}
			}
		}

		// Render the tri-list...
		if ( pCluster->TriList.nTriCount )
		{
			if (FAILED(FDX8_pDev->DrawIndexedPrimitive(
				D3DPT_TRIANGLELIST,
				pCluster->TriList.nVtxIndexMin,
				pCluster->TriList.nVtxIndexRange,
				pCluster->TriList.nStartVindex,
				pCluster->TriList.nTriCount
			)))
			{
				fang_DevPrintf("fdx8mesh::DrawInstanceMaterial_P - DrawPrimitive failed.\n");
			}

			#if FPERF_ENABLE
				FPerf_nRawTriListCount++;
				FPerf_nRawListTriCount += pCluster->TriList.nTriCount;
				FPerf_nRawVertexCount += (pCluster->TriList.nTriCount<<1) + pCluster->TriList.nTriCount;
			#endif
		}

		// Render the strip-list...
		FDX8MeshStrip_t *pStrip;
		for ( nStrip=0, pStrip = pCluster->paStripBuffer; nStrip < pCluster->nStripCount; nStrip++, pStrip++ ) 
		{
			if (FAILED(FDX8_pDev->DrawIndexedPrimitive(
				D3DPT_TRIANGLESTRIP,
				pStrip->nVtxIndexMin,
				pStrip->nVtxIndexRange,
				pStrip->nStartVindex,
				pStrip->nTriCount
			)))
			{
				fang_DevPrintf("fdx8mesh::DrawInstanceMaterial_P - DrawPrimitive failed.\n");
			}

			#if FPERF_ENABLE
				FPerf_nRawTriStripCount++;
				FPerf_nRawStripTriCount += pStrip->nTriCount;
				FPerf_nRawVertexCount += pStrip->nTriCount + 2;
			#endif
		}
	}

	fsh_EndExecute();

	if ( m_pMesh->nTexLayerIDCount ) 
	{
		// Reset the lists if there were texture layer effects...
		fsh_TexCoordMtx_ResetList();
		fsh_TexOverride_ResetList();
	}

	#if FPERF_ENABLE
		FPerf_nTotalTrisSurface += (FPerf_nRawStripTriCount - nPreStripCount) + (FPerf_nRawListTriCount - nPreListCount);
	#endif
}


//
//
//
void CFMeshInst::DrawMaterialSpecular_P( FViewportPlanesMask_t nCrossesPlanesMask, FMeshMaterial_t *pMaterial, u32 nLODIndex )
{
	FASSERT( _bWindowCreated );
	FASSERT( nCrossesPlanesMask != -1 );

	CFMtx43A Mtx43A, *pMtx43A;
	u32 nBoneMtxNum, nStrip;

	#if FPERF_ENABLE
		u32 nPreStripCount = FPerf_nRawStripTriCount;
		u32 nPreListCount = FPerf_nRawListTriCount;
	#endif

	#if _SUPPORTING_PARTS_DRAWING
		if ( !(pMaterial->nPartIDMask & m_nDrawnPartsMask) )
		{
			return;
		}
	#endif

	#if FANG_PLATFORM_WIN
		fdx8_SetRenderState_CLIPPING( nCrossesPlanesMask != 0 );
	#endif

	fdx8_SetRenderState_FILLMODE( FRenderer_nD3DFillMode );
	_SetCullDir_P( (FMeshCullDir_e)m_nCullDirection );
	FDX8Mesh_t *pMeshIS = m_pMesh->pMeshIS;
	u32 nMeshBoneCount = m_pMesh->nBoneCount;

	if ( !(m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH) )
	{
		if ( nMeshBoneCount == 0 ) 
		{
			// There are no bones in the entire mesh. We can just set a single model matrix...
			fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &m_PosMtx, TRUE );
			fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );
			fdx8xfm_SetRenormalizeNormalsIfNeeded( m_fMatrixScale );
		} 
	}

	if ( !SetupShaderParameters( pMaterial, FSHADERS_PASS_SPECULAR ) )
	{
		return;
	}

	// Submit the material's clusters once for each pass that is required
#if FANG_DEBUG_BUILD
	FDX8VB_t *pVB = NULL;
#endif
	FMeshSeg_t *pSeg = NULL;
	FMeshSeg_t *aSeg = m_pMesh->aSeg;
	s32 nCurrentIB = -1;

	fsh_ExecuteCurrent( FALSE, 0 );

	// Progress through all the clusters for this material, submitting for render the relevant ones
	FDX8MeshCluster_t *pCluster = ((FDX8MeshMaterial_t *)pMaterial->pPlatformData)->aCluster;
	FDX8MeshCluster_t *pEndCluster = &pCluster[((FDX8MeshMaterial_t *)pMaterial->pPlatformData)->nClusterCount];
#if FMESH_ALLOW_POSTER_BONES
	BOOL bPosterBones = FALSE;
	CFMtx43A PosterMtx;

	if ( (m_nFlags & (FMESHINST_FLAG_POSTER_X | FMESHINST_FLAG_POSTER_Y | FMESHINST_FLAG_POSTER_Z)) && (m_nFlags & FMESHINST_FLAG_POSTER_BONES) )
	{
		if ( m_pMesh->nBoneCount )
		{
			bPosterBones = TRUE;
			PosterMtx.Mul( m_PosMtx, m_Xfm.m_MtxR );
		}
	}
#endif

	for ( ;pCluster < pEndCluster; pCluster++ )
	{
		if ( !(m_nDrawnPartsMask & (1 << pCluster->nPartID)) )
		{
			continue;
		}

		if ( pCluster->nLODID != nLODIndex )
		{
			continue;
		}

		// Select the appropriate vertex buffer for this cluster
		fdx8vb_Select( &pMeshIS->aVB[ pCluster->nVBIndex ], FALSE, FALSE, TRUE );
		fsh_CheckVB();
#if FANG_WINGC
		if ( FSh_shaderType == SHADERTYPE_SURFACE )
		{
			_PortLogD3DState( "SURF", m_pMesh->szName, (u32)(pMaterial - m_pMesh->aMtl) );
			// Port debug: MA_PORT_RAWSURF=1 draws the surface pass opaque, ignoring depth.
			static int _nRawSurf = -1;
			if ( _nRawSurf < 0 )
			{
				char szVal[8];
				_nRawSurf = GetEnvironmentVariableA( "MA_PORT_RAWSURF", szVal, sizeof(szVal) ) > 0 ? 1 : 0;
			}
			if ( _nRawSurf )
			{
				FDX8_pDev->GetD3D9Device()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
				FDX8_pDev->GetD3D9Device()->SetRenderState( D3DRS_ZFUNC, D3DCMP_ALWAYS );
			}
		}
#endif

		// Set the index buffer
		if ( nCurrentIB != pCluster->nIBIndex )
		{
			nCurrentIB = pCluster->nIBIndex;
			FDX8_pDev->SetIndices( (IDirect3DIndexBuffer8 *)pMeshIS->apDXIB[nCurrentIB], 0 );
		}

		D3DXMATRIX boneMtx;

		if ( nMeshBoneCount ) 
		{
			FASSERT( pCluster->nSegmentIdx < m_pMesh->nSegCount );

			if ( pSeg != &aSeg[pCluster->nSegmentIdx] )
			{
				pSeg = &aSeg[pCluster->nSegmentIdx];

				if ( pSeg->nBoneMtxCount == 0 )
				{
					// No bones for this segment, so just set this mesh instance's xfm
					// as the model matrix...
					fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );
					fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &m_PosMtx, TRUE );
					fdx8xfm_SetRenormalizeNormalsIfNeeded( m_fMatrixScale );
				} 
				else if ( pSeg->nBoneMtxCount == 1 ) 
				{
					// This is a non-skinned segment...

					fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );

					if ( !(m_nFlags & FMESHINST_FLAG_NOBONES) ) 
					{
						#if FMESH_ALLOW_POSTER_BONES
						if ( !bPosterBones )
						{
							fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ], TRUE );
						} 
						else
						{
							Mtx43A.Mul33( PosterMtx, *m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ] );
							Mtx43A.m_vPos = m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ]->m_vPos;
							fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &Mtx43A, TRUE );
						}
						#else
						fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ], TRUE );
						#endif
						fdx8xfm_SetRenormalizeNormalsIfNeeded( m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ] );
					} 
					else 
					{
						// The FMESHINST_FLAG_NOBONES flag is set and this is a non-skinned segment.
						// This means that the vertices have already been transformed into bone space,
						// and we need to transform them back into at-rest model space...
//						Mtx43A.Mul( m_Xfm.m_MtxF, m_pMesh->pBoneArray[ pSeg->anBoneMtxIndex[0] ].AtRestBoneToModelMtx );
						Mtx43A.Mul( m_PosMtx, m_pMesh->pBoneArray[ pSeg->anBoneMtxIndex[0] ].AtRestBoneToModelMtx );
						fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &Mtx43A, TRUE );
						fdx8xfm_SetRenormalizeNormalsIfNeeded( &Mtx43A );
					}
				} 
				else 
				{
					// This is a skinned segment...

					if ( !(m_nFlags & FMESHINST_FLAG_NOBONES) ) 
					{
						fdx8_SetRenderState_VERTEXBLEND( _aD3DVertexBlendFlags[pSeg->nBoneMtxCount] );

						for( nBoneMtxNum=0; nBoneMtxNum<pSeg->nBoneMtxCount; nBoneMtxNum++ ) 
						{
							pMtx43A = m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[nBoneMtxNum] ];
							D3DXMatrixTranspose(&boneMtx, (const D3DXMATRIX *)pMtx43A);
							FDX8_pDev->SetVertexShaderConstant(CV_BONEMTX0_0 + 3*nBoneMtxNum, &boneMtx(0, 0), 3);
						}
					} 
					else 
					{
						fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );
						fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &m_PosMtx, TRUE );
						fdx8xfm_SetRenormalizeNormalsIfNeeded( m_fMatrixScale );
					}
				}
			}
		}

		// Render the tri-list...
		if ( pCluster->TriList.nTriCount )
		{
			if (FAILED(FDX8_pDev->DrawIndexedPrimitive(
				D3DPT_TRIANGLELIST,
				pCluster->TriList.nVtxIndexMin,
				pCluster->TriList.nVtxIndexRange,
				pCluster->TriList.nStartVindex,
				pCluster->TriList.nTriCount
			)))
			{
				fang_DevPrintf("fdx8mesh::DrawInstanceMaterial_P - DrawPrimitive failed.\n");
			}

			#if FPERF_ENABLE
				FPerf_nRawTriListCount++;
				FPerf_nRawListTriCount += pCluster->TriList.nTriCount;
				FPerf_nRawVertexCount += (pCluster->TriList.nTriCount<<1) + pCluster->TriList.nTriCount;
			#endif
		}

		// Render the strip-list...
		FDX8MeshStrip_t *pStrip;
		for ( nStrip=0, pStrip = pCluster->paStripBuffer; nStrip < pCluster->nStripCount; nStrip++, pStrip++ ) 
		{
			if (FAILED(FDX8_pDev->DrawIndexedPrimitive(
				D3DPT_TRIANGLESTRIP,
				pStrip->nVtxIndexMin,
				pStrip->nVtxIndexRange,
				pStrip->nStartVindex,
				pStrip->nTriCount
			)))
			{
				fang_DevPrintf("fdx8mesh::DrawInstanceMaterial_P - DrawPrimitive failed.\n");
			}

			#if FPERF_ENABLE
				FPerf_nRawTriStripCount++;
				FPerf_nRawStripTriCount += pStrip->nTriCount;
				FPerf_nRawVertexCount += pStrip->nTriCount + 2;
			#endif
		}
	}

	fsh_EndExecute();

#if FPERF_ENABLE
	FPerf_nTotalTrisSpecular += (FPerf_nRawStripTriCount - nPreStripCount) + (FPerf_nRawListTriCount - nPreListCount);
#endif
}


//
//
//
void CFMeshInst::DrawMaterialTranslucency_P( FViewportPlanesMask_t nCrossesPlanesMask, FMeshMaterial_t *pMaterial, u32 nLODIndex )
{
	FASSERT( _bWindowCreated );
	FASSERT( nCrossesPlanesMask != -1 );

	CFMtx43A Mtx43A, *pMtx43A;
	u32 nBoneMtxNum, nStrip;

	#if FPERF_ENABLE
		u32 nPreStripCount = FPerf_nRawStripTriCount;
		u32 nPreListCount = FPerf_nRawListTriCount;
	#endif

	#if _SUPPORTING_PARTS_DRAWING
		if ( !(pMaterial->nPartIDMask & m_nDrawnPartsMask) )
		{
			return;
		}
	#endif

	#if FANG_PLATFORM_WIN
		fdx8_SetRenderState_CLIPPING( nCrossesPlanesMask != 0 );
	#endif

	fdx8_SetRenderState_FILLMODE( FRenderer_nD3DFillMode );
	_SetCullDir_P( (FMeshCullDir_e)m_nCullDirection );
	FDX8Mesh_t *pMeshIS = m_pMesh->pMeshIS;
	u32 nMeshBoneCount = m_pMesh->nBoneCount;

	if ( !(m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH) )
	{
		if ( nMeshBoneCount == 0 ) 
		{
			// There are no bones in the entire mesh. We can just set a single model matrix...
			fdx8xfm_SetViewDXMatrix( TRUE );
			fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &m_PosMtx, TRUE );
			fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );
			fdx8xfm_SetRenormalizeNormalsIfNeeded( m_fMatrixScale );
		} 
		else 
		{
			// This mesh has bones. Set only the view matrix because
			// the model matrix/matrices must be computed for each segment...
			fdx8xfm_SetViewDXMatrix( TRUE );
		}
	}

	if ( !SetupShaderParameters( pMaterial, FSHADERS_PASS_ALL ) )
	{
		return;
	}

	// Setup color stream management
	s32 nColorStreamVBIndex = -1;
	if ( m_papColorStreams && !FMesh_bRenderShadows )
	{
		FDX8_pDev->SetStreamSource( 3, ((IDirect3DVertexBuffer8 **)m_papColorStreams)[0], sizeof(DWORD) );
		fdx8sh_ToggleExternalColorStream( TRUE );
		_bStream3Active = TRUE;
	}
	else if ( _bStream3Active )
	{
		FDX8_pDev->SetStreamSource( 3, NULL, 0 );
		fdx8sh_ToggleExternalColorStream( FALSE );
		_bStream3Active = FALSE;
	}

	// Pass in procedural data
	fsh_SetProcedural( m_pProcedural );

	// Submit the material's clusters once for each pass that is required
#if FANG_DEBUG_BUILD
	FDX8VB_t *pVB = NULL;
#endif
	FMeshSeg_t *pSeg = NULL;
	FMeshSeg_t *aSeg = m_pMesh->aSeg;
	s32 nCurrentIB = -1, nPass;
	
	s32 nTotalPasses = fsh_GetCurrentNumPasses();
#if FMESH_ALLOW_POSTER_BONES
	BOOL bPosterBones = FALSE;
	CFMtx43A PosterMtx;

	if ( (m_nFlags & (FMESHINST_FLAG_POSTER_X | FMESHINST_FLAG_POSTER_Y | FMESHINST_FLAG_POSTER_Z)) && (m_nFlags & FMESHINST_FLAG_POSTER_BONES) )
	{
		if ( m_pMesh->nBoneCount )
		{
			bPosterBones = TRUE;
			PosterMtx.Mul( m_PosMtx, m_Xfm.m_MtxR );
		}
	}
#endif

	for ( nPass = 0; nPass < nTotalPasses; nPass++ )
	{
		fsh_SetPassIdx( nPass );
		fsh_ExecuteCurrent( FALSE, 0 );

		// Progress through all the clusters for this material, submitting for render the relevant ones
		FDX8MeshCluster_t *pCluster = ((FDX8MeshMaterial_t *)pMaterial->pPlatformData)->aCluster;
		FDX8MeshCluster_t *pEndCluster = &pCluster[((FDX8MeshMaterial_t *)pMaterial->pPlatformData)->nClusterCount];
		for ( ;pCluster < pEndCluster; pCluster++ )
		{
			if ( !(m_nDrawnPartsMask & (1 << pCluster->nPartID)) )
			{
				continue;
			}

			if ( pCluster->nLODID != nLODIndex )
			{
				continue;
			}

			// Select the appropriate vertex buffer for this cluster
			fdx8vb_Select( &pMeshIS->aVB[ pCluster->nVBIndex ], FALSE, TRUE, TRUE );
			fsh_CheckVB();

			// Activate the color stream for this cluster, if there is one
			if ( m_papColorStreams && nColorStreamVBIndex != pCluster->nVBIndex )
			{
				FDX8_pDev->SetStreamSource( 3, ((IDirect3DVertexBuffer8 **)m_papColorStreams)[pCluster->nVBIndex], sizeof(DWORD) );
				nColorStreamVBIndex = pCluster->nVBIndex;
			}

			// Set the index buffer
			if ( nCurrentIB != pCluster->nIBIndex )
			{
				nCurrentIB = pCluster->nIBIndex;
				FDX8_pDev->SetIndices( (IDirect3DIndexBuffer8 *)pMeshIS->apDXIB[nCurrentIB], 0 );
			}

			D3DXMATRIX boneMtx;

			if ( nMeshBoneCount ) 
			{
				FASSERT( pCluster->nSegmentIdx < m_pMesh->nSegCount );

				if ( pSeg != &aSeg[pCluster->nSegmentIdx] )
				{
					pSeg = &aSeg[pCluster->nSegmentIdx];

					if ( pSeg->nBoneMtxCount == 0 )
					{
						// No bones for this segment, so just set this mesh instance's xfm
						// as the model matrix...
						fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );
						fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &m_PosMtx, TRUE );
						fdx8xfm_SetRenormalizeNormalsIfNeeded( m_fMatrixScale );
					} 
					else if ( pSeg->nBoneMtxCount == 1 ) 
					{
						// This is a non-skinned segment...

						fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );

						if ( !(m_nFlags & FMESHINST_FLAG_NOBONES) ) 
						{
							#if FMESH_ALLOW_POSTER_BONES
							if ( !bPosterBones )
							{
								fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ], TRUE );
							} 
							else
							{
								Mtx43A.Mul33( PosterMtx, *m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ] );
								Mtx43A.m_vPos = m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ]->m_vPos;
								fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &Mtx43A, TRUE );
							}
							#else
							fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ], TRUE );
							#endif
							fdx8xfm_SetRenormalizeNormalsIfNeeded( m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ] );
						} 
						else 
						{
							// The FMESHINST_FLAG_NOBONES flag is set and this is a non-skinned segment.
							// This means that the vertices have already been transformed into bone space,
							// and we need to transform them back into at-rest model space...
							Mtx43A.Mul( m_Xfm.m_MtxF, m_pMesh->pBoneArray[ pSeg->anBoneMtxIndex[0] ].AtRestBoneToModelMtx );
							fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &Mtx43A, TRUE );
							fdx8xfm_SetRenormalizeNormalsIfNeeded( &Mtx43A );
						}
					} 
					else 
					{
						// This is a skinned segment...

						if ( !(m_nFlags & FMESHINST_FLAG_NOBONES) ) 
						{
							fdx8_SetRenderState_VERTEXBLEND( _aD3DVertexBlendFlags[pSeg->nBoneMtxCount] );

							for( nBoneMtxNum=0; nBoneMtxNum<pSeg->nBoneMtxCount; nBoneMtxNum++ ) 
							{
								pMtx43A = m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[nBoneMtxNum] ];
								D3DXMatrixTranspose(&boneMtx, (const D3DXMATRIX *)pMtx43A);
								FDX8_pDev->SetVertexShaderConstant(CV_BONEMTX0_0 + 3*nBoneMtxNum, &boneMtx(0, 0), 3);
							}
						} 
						else 
						{
							fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );
							fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &m_PosMtx, TRUE );
							fdx8xfm_SetRenormalizeNormalsIfNeeded( m_fMatrixScale );
						}
					}
				}
			}

			// Render the tri-list...
			if ( pCluster->TriList.nTriCount )
			{
				if (FAILED(FDX8_pDev->DrawIndexedPrimitive(
					D3DPT_TRIANGLELIST,
					pCluster->TriList.nVtxIndexMin,
					pCluster->TriList.nVtxIndexRange,
					pCluster->TriList.nStartVindex,
					pCluster->TriList.nTriCount
				)))
				{
					fang_DevPrintf("fdx8mesh::DrawInstanceMaterial_P - DrawPrimitive failed.\n");
				}

				#if FPERF_ENABLE
					FPerf_nRawTriListCount++;
					FPerf_nRawListTriCount += pCluster->TriList.nTriCount;
					FPerf_nRawVertexCount += (pCluster->TriList.nTriCount<<1) + pCluster->TriList.nTriCount;
				#endif
			}

			// Render the strip-list...
			FDX8MeshStrip_t *pStrip;
			for ( nStrip=0, pStrip = pCluster->paStripBuffer; nStrip < pCluster->nStripCount; nStrip++, pStrip++ ) 
			{
				if (FAILED(FDX8_pDev->DrawIndexedPrimitive(
					D3DPT_TRIANGLESTRIP,
					pStrip->nVtxIndexMin,
					pStrip->nVtxIndexRange,
					pStrip->nStartVindex,
					pStrip->nTriCount
				)))
				{
					fang_DevPrintf("fdx8mesh::DrawInstanceMaterial_P - DrawPrimitive failed.\n");
				}

				#if FPERF_ENABLE
					FPerf_nRawTriStripCount++;
					FPerf_nRawStripTriCount += pStrip->nTriCount;
					FPerf_nRawVertexCount += pStrip->nTriCount + 2;
				#endif
			}
		}
	}

	fsh_EndExecute();

	if ( m_pMesh->nTexLayerIDCount ) 
	{
		// Reset the lists if there were texture layer effects...
		fsh_TexCoordMtx_ResetList();
		fsh_TexOverride_ResetList();
	}

#if FPERF_ENABLE
	u32 nTrisRendered = (FPerf_nRawStripTriCount - nPreStripCount) + (FPerf_nRawListTriCount - nPreListCount);
	FPerf_nTotalTrisTranslucent += nTrisRendered;
	FPerf_nMeshTris += nTrisRendered / nTotalPasses;
#endif
}


//
//
//
void CFMeshInst::DrawAllMaterials_P( FViewportPlanesMask_t nCrossesPlanesMask ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( nCrossesPlanesMask != -1 );

	CFMtx43A Mtx43A, *pMtx43A;
	u32 i, nBoneMtxNum, nPass, nStrip, nPassCount = 0;

	#if FPERF_ENABLE
		u32 nPreStripCount = FPerf_nRawStripTriCount;
		u32 nPreListCount = FPerf_nRawListTriCount;
	#endif

	#if FANG_PLATFORM_WIN
		fdx8_SetRenderState_CLIPPING( nCrossesPlanesMask != 0 );
	#endif

	fdx8_SetRenderState_FILLMODE( FRenderer_nD3DFillMode );

	_SetCullDir_P( (FMeshCullDir_e)m_nCullDirection );

	FDX8Mesh_t *pMeshIS = m_pMesh->pMeshIS;
	u32 nMeshBoneCount = m_pMesh->nBoneCount;

	if ( nMeshBoneCount == 0 ) 
	{
		// There are no bones in the entire mesh. We can just set a single model matrix...
		fdx8xfm_SetViewDXMatrix( TRUE );
		fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &m_PosMtx, TRUE );
		fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );
		fdx8xfm_SetRenormalizeNormalsIfNeeded( m_fMatrixScale );
		// There are no bones in the entire mesh. We can just set a single model matrix...
//		fdx8xfm_SetDXMatrices( TRUE );
//		fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );
//		fdx8xfm_SetRenormalizeNormalsIfNeeded( FXfm_pModel->m_fScaleF );
	} 
	else 
	{
		// This mesh has bones. Set only the view matrix because
		// the model matrix/matrices must be computed for each segment...
		fdx8xfm_SetViewDXMatrix( TRUE );
	}

	if ( !FMesh_bRenderShadows )
	{
		AddShaderLights();
	}

	// Check if texture layer animations exist on this object...
	if ( !m_pMesh->nTexLayerIDCount ) 
	{
		// No texture layer animation on this object...
		fsh_TexCoordMtx_ResetList();
		fsh_TexOverride_ResetList();
	}

	// Determine the LOD we should be using
//	f32 fDistToCamSq = FXfm_pModel->m_MtxF.m_vPos.Sub( FXfm_pView->m_MtxR.m_vPos ).MagSq();
	f32 fDistToCamSq = m_PosMtx.m_vPos.Sub( FXfm_pView->m_MtxR.m_vPos ).MagSq();
	u8 nLODMask, nLODToUse = 0;
	if ( fDistToCamSq > 2.f && fDistToCamSq < 10000000.f )
	{
		fDistToCamSq = fmath_Sqrt( fDistToCamSq ) - m_pMesh->BoundSphere_MS.m_fRadius;
		for ( i = 1; i < m_pMesh->nLODCount ; i++ )
		{
			if ( fDistToCamSq < m_pMesh->afLODDistance[i] )
			{
				break;
			}
			nLODToUse++;
		}
	}

	if ( FMesh_bRenderShadows )
	{
		nLODToUse += m_pMesh->nShadowLODBias;
		if ( nLODToUse > m_pMesh->nLODCount - 1 )
		{
			nLODToUse = m_pMesh->nLODCount - 1;
		}
	}
	else
	{
		m_nCurrentLOD = nLODToUse;
	}

#if !FANG_PRODUCTION_BUILD
	if ( FMesh_nForceLOD != -1 )
	{
		nLODToUse = FMesh_nForceLOD;
	}
#endif
	nLODMask = 1 << nLODToUse;

	// Setup color stream management
	s32 nColorStreamVBIndex = -1;
	if ( m_papColorStreams )
	{
		FDX8_pDev->SetStreamSource( 3, ((IDirect3DVertexBuffer8 **)m_papColorStreams)[0], sizeof(DWORD) );
		fdx8sh_ToggleExternalColorStream( TRUE );
		_bStream3Active = TRUE;
	}
	else if ( _bStream3Active )
	{
		FDX8_pDev->SetStreamSource( 3, NULL, 0 );
		fdx8sh_ToggleExternalColorStream( FALSE );
		_bStream3Active = FALSE;
	}

	// Advance through all of the materials, drawing the relevant materials and clusters
	s32 nCurrentIB = -1;
#if FANG_DEBUG_BUILD
	FDX8VB_t *pVB = NULL;
#endif
	FMeshSeg_t *pSeg = NULL;
	FMeshSeg_t *aSeg = m_pMesh->aSeg;
	FMeshMaterial_t *pMtl = m_pMesh->aMtl;
	FMeshMaterial_t *pEndMtl = &m_pMesh->aMtl[m_pMesh->nMaterialCount];

#if FMESH_ALLOW_POSTER_BONES
	BOOL bPosterBones = FALSE;
	CFMtx43A PosterMtx;

	if ( (m_nFlags & (FMESHINST_FLAG_POSTER_X | FMESHINST_FLAG_POSTER_Y | FMESHINST_FLAG_POSTER_Z)) && (m_nFlags & FMESHINST_FLAG_POSTER_BONES) )
	{
		if ( m_pMesh->nBoneCount )
		{
			bPosterBones = TRUE;
			PosterMtx.Mul( m_PosMtx, m_Xfm.m_MtxR );
		}
	}
#endif

	for ( ;pMtl < pEndMtl; pMtl++ )
	{
		#if _SUPPORTING_PARTS_DRAWING
			if ( !(pMtl->nPartIDMask & m_nDrawnPartsMask) )
			{
				continue;
			}
		#endif

		if ( !(pMtl->nLODMask & nLODMask) )
		{
			continue;
		}

		if ( !SetupShaderParameters( pMtl, FSHADERS_PASS_LIGHTING ) )
		{
			continue;
		}

		// Change the shader type, if necessary and supported, based on the mesh alpha
		u32 nSurfaceShaderIdx = pMtl->nSurfaceShaderIdx;
		if ( nSurfaceShaderIdx == FSHADERS_oBASE && GetMeshTintAndAlpha()->fAlpha < 0.999f )
		{
			nSurfaceShaderIdx = FSHADERS_otBASE;
		}

		if ( nSurfaceShaderIdx == FSHADERS_cBASE && GetMeshTintAndAlpha()->fAlpha < 0.999f )
		{
			nSurfaceShaderIdx = FSHADERS_tBASE;
		}

		// Determine the number of necessary passes
		u32 nDiffusePasses = fsh_GetNumDiffusePasses( nSurfaceShaderIdx, pMtl->nLightShaderIdx );
		nPassCount = nDiffusePasses + 1 + (pMtl->nSpecularShaderIdx != FSH_INVALID_SPECULAR_SHADER);
		if ( FMesh_bRenderShadows )
		{
			nPassCount = 1;
			nDiffusePasses = 1;
		}

		// Render the material's clusters once per required pass
		for ( nPass = 0; nPass < nPassCount; nPass++ )
		{
			if ( nPass < nDiffusePasses )
			{
				fsh_SetShaderType( SHADERTYPE_DIFFUSE );

				if ( !fsh_SetShader( nSurfaceShaderIdx, pMtl->nLightShaderIdx, pMtl->nSpecularShaderIdx, nPass ) ) 
				{
					continue;
				}
				fsh_SetReflectTex( NULL );
			}
			else if ( nPass == nDiffusePasses )
			{
				fsh_SetShaderType( SHADERTYPE_SURFACE ); //diffuse, surface, specular or translucency

				if ( !SetupShaderParameters( pMtl, FSHADERS_PASS_SURFACE ) )
				{
					continue;
				}

				if ( !fsh_SetShader( nSurfaceShaderIdx, pMtl->nLightShaderIdx, pMtl->nSpecularShaderIdx ) )
				{
					continue;
				}
				fsh_SetReflectTex( m_pReflectTex );
			}
			else
			{
				fsh_SetShaderType( SHADERTYPE_SPECULAR ); //diffuse, surface, specular or translucency

				if ( !SetupShaderParameters( pMtl, FSHADERS_PASS_SURFACE ) )
				{
					SetupShaderParameters( pMtl, FSHADERS_PASS_SPECULAR );
				}

				if ( !fsh_SetShader( nSurfaceShaderIdx, pMtl->nLightShaderIdx, pMtl->nSpecularShaderIdx ) )
				{
					continue;
				}
			}

			// Setup the DX states for this shader in preparation for rendering
			fsh_ExecuteCurrent( FMesh_bRenderShadows, 0x01 );

			// Submit all relevant clusters for rendering
			FDX8MeshCluster_t *pCluster = ((FDX8MeshMaterial_t *)pMtl->pPlatformData)->aCluster;
			FDX8MeshCluster_t *pEndCluster = &pCluster[((FDX8MeshMaterial_t *)pMtl->pPlatformData)->nClusterCount];
			for ( ;pCluster < pEndCluster; pCluster++ )
			{
				if ( !(m_nDrawnPartsMask & (1 << pCluster->nPartID)) )
				{
					continue;
				}

				if ( pCluster->nLODID != nLODToUse )
				{
					continue;
				}

				// Setup the vertex buffers
				BOOL bUseLightmaps = (FSh_shaderType == SHADERTYPE_DIFFUSE || FSh_shaderType == SHADERTYPE_TRANSLUCENCY);
				fdx8vb_Select( &pMeshIS->aVB[ pCluster->nVBIndex ], FALSE, bUseLightmaps, TRUE );
				fsh_CheckVB();

				// Setup the color streams
				if ( m_papColorStreams && nColorStreamVBIndex != pCluster->nVBIndex )
				{
					FDX8_pDev->SetStreamSource( 3, ((IDirect3DVertexBuffer8 **)m_papColorStreams)[pCluster->nVBIndex], sizeof(DWORD) );
					nColorStreamVBIndex = pCluster->nVBIndex;
					_bStream3Active = TRUE;
				}

				// Setup the index buffer
				if ( nCurrentIB != pCluster->nIBIndex )
				{
					nCurrentIB = pCluster->nIBIndex;
					FDX8_pDev->SetIndices( (IDirect3DIndexBuffer8 *)pMeshIS->apDXIB[nCurrentIB], 0 );
				}

				D3DXMATRIX boneMtx;

				if ( nMeshBoneCount ) 
				{
					FASSERT( pCluster->nSegmentIdx < m_pMesh->nSegCount );

					if ( pSeg != &aSeg[pCluster->nSegmentIdx] )
					{
						pSeg = &aSeg[pCluster->nSegmentIdx];

						if ( pSeg->nBoneMtxCount == 0 )
						{
							// No bones for this segment, so just set this mesh instance's xfm
							// as the model matrix...
							fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );
							fdx8xfm_SetModelDXMatrix( TRUE );
							fdx8xfm_SetRenormalizeNormalsIfNeeded( m_fMatrixScale );
						} 
						else if ( pSeg->nBoneMtxCount == 1 ) 
						{
							// This is a non-skinned segment...

							fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );

							if ( !(m_nFlags & FMESHINST_FLAG_NOBONES) ) 
							{
								#if FMESH_ALLOW_POSTER_BONES
								if ( !bPosterBones )
								{
									fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ], TRUE );
								} 
								else
								{
									Mtx43A.Mul33( PosterMtx, *m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ] );
									Mtx43A.m_vPos = m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ]->m_vPos;
									fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &Mtx43A, TRUE );
								}
								#else
								fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ], TRUE );
								#endif
								fdx8xfm_SetRenormalizeNormalsIfNeeded( m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ] );
							} 
							else 
							{
								// The FMESHINST_FLAG_NOBONES flag is set and this is a non-skinned segment.
								// This means that the vertices have already been transformed into bone space,
								// and we need to transform them back into at-rest model space...
								Mtx43A.Mul( m_PosMtx, m_pMesh->pBoneArray[ pSeg->anBoneMtxIndex[0] ].AtRestBoneToModelMtx );
								fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &Mtx43A, TRUE );
								fdx8xfm_SetRenormalizeNormalsIfNeeded( &Mtx43A );
							}
						} 
						else 
						{
							// This is a skinned segment...

							if ( !(m_nFlags & FMESHINST_FLAG_NOBONES) ) 
							{
								fdx8_SetRenderState_VERTEXBLEND( _aD3DVertexBlendFlags[pSeg->nBoneMtxCount] );

								for( nBoneMtxNum=0; nBoneMtxNum<pSeg->nBoneMtxCount; nBoneMtxNum++ ) 
								{
									pMtx43A = m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[nBoneMtxNum] ];
									D3DXMatrixTranspose( &boneMtx, (const D3DXMATRIX *)pMtx43A );
									FDX8_pDev->SetVertexShaderConstant( CV_BONEMTX0_0 + 3 * nBoneMtxNum, &boneMtx(0, 0), 3 );
								}
							} 
							else 
							{
								fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );
								fdx8xfm_SetModelDXMatrix( TRUE );
								fdx8xfm_SetRenormalizeNormalsIfNeeded( m_fMatrixScale );
							}
						}
					}
				}

				// Render the tri-list...
				if ( pCluster->TriList.nTriCount )
				{
					if ( FAILED(FDX8_pDev->DrawIndexedPrimitive(
						D3DPT_TRIANGLELIST,
						pCluster->TriList.nVtxIndexMin,
						pCluster->TriList.nVtxIndexRange,
						pCluster->TriList.nStartVindex,
						pCluster->TriList.nTriCount
					)) )
					{
						fang_DevPrintf("fdx8mesh::DrawInstanceMaterial_P - DrawPrimitive failed.\n");
					}

					#if FPERF_ENABLE
						FPerf_nRawTriListCount++;
						FPerf_nRawListTriCount += pCluster->TriList.nTriCount;
						FPerf_nRawVertexCount += (pCluster->TriList.nTriCount<<1) + pCluster->TriList.nTriCount;
					#endif
				}

				// Render the strip-list...
				FDX8MeshStrip_t *pStrip;
				for ( nStrip=0, pStrip = pCluster->paStripBuffer; nStrip < pCluster->nStripCount; nStrip++, pStrip++ ) 
				{
					if ( FAILED(FDX8_pDev->DrawIndexedPrimitive(
						D3DPT_TRIANGLESTRIP,
						pStrip->nVtxIndexMin,
						pStrip->nVtxIndexRange,
						pStrip->nStartVindex,
						pStrip->nTriCount
					)) )
					{
						fang_DevPrintf("fdx8mesh::DrawInstanceMaterial_P - DrawPrimitive failed.\n");
					}

					#if FPERF_ENABLE
						FPerf_nRawTriStripCount++;
						FPerf_nRawStripTriCount += pStrip->nTriCount;
						FPerf_nRawVertexCount += pStrip->nTriCount + 2;
					#endif
				}
			}
		}
	}

	fsh_EndExecute();

	if ( m_pMesh->nTexLayerIDCount )
	{
		fsh_TexCoordMtx_ResetList();
		fsh_TexOverride_ResetList();
	}

	#if FPERF_ENABLE
		if ( nPassCount )
		{
			u32 nTrisRendered = (FPerf_nRawStripTriCount - nPreStripCount) + (FPerf_nRawListTriCount - nPreListCount);
			if ( FMesh_bRenderShadows )
			{
				FPerf_nMeshShadowTris += nTrisRendered / nPassCount;
			}
			else
			{
				FPerf_nMeshTris += nTrisRendered / nPassCount;
			}
		}
	#endif
}

//
//
//
void CFMeshInst::DrawShadow_P( FViewportPlanesMask_t nCrossesPlanesMask, FMeshMaterial_t *pMaterial, u32 nLODIndex )
{
	FASSERT( _bWindowCreated );
	FASSERT( nCrossesPlanesMask != -1 );

	CFMtx43A Mtx43A, *pMtx43A;
	u32 nBoneMtxNum, nStrip;

	if ( m_fShadowIntensity < 0.01f )
	{
		return;
	}

	#if _SUPPORTING_PARTS_DRAWING
		if ( !(pMaterial->nPartIDMask & m_nDrawnPartsMask) )
		{
			return;
		}
	#endif

	FDX8Mesh_t *pMeshIS = m_pMesh->pMeshIS;
	u32 nMeshBoneCount = m_pMesh->nBoneCount;

	#if FANG_PLATFORM_WIN
		fdx8_SetRenderState_CLIPPING( nCrossesPlanesMask != 0 );
	#endif

	fdx8_SetRenderState_FILLMODE( FRenderer_nD3DFillMode );

	_SetCullDir_P( (FMeshCullDir_e)m_nCullDirection );

	if ( !(m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH) )
	{
		if ( nMeshBoneCount == 0 ) 
		{
			// There are no bones in the entire mesh. We can just set a single model matrix...
			fdx8xfm_SetViewDXMatrix( TRUE );
			fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &m_PosMtx, TRUE );
			fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );
			fdx8xfm_SetRenormalizeNormalsIfNeeded( m_fMatrixScale );
		} 
		else 
		{
			// This mesh has bones. Set only the view matrix because
			// the model matrix/matrices must be computed for each segment...
			fdx8xfm_SetViewDXMatrix( TRUE );
		}
	}

	// Setup any material specific depth bias
	fsh_SetDepthBias( pMaterial->nDepthBiasLevel );

	s32 nTotalPasses = fshadow_GetNumShadowPasses(m_pRenderLights, m_nRenderLightCount, this, m_pMesh->nFlags&FMESH_FLAGS_VOLUME_MESH);

#if FANG_DEBUG_BUILD
	FDX8VB_t *pVB = NULL;
#endif
	s32 nPass, nCurrentIB = -1;
	FMeshSeg_t *pSeg = NULL;
	FMeshSeg_t *aSeg = m_pMesh->aSeg;
	s32 nColorStreamVBIndex = -1;
	if ( _bStream3Active )
	{
		FDX8_pDev->SetStreamSource( 3, NULL, 0 );
		fdx8sh_ToggleExternalColorStream( FALSE );
	}

	if (nTotalPasses < 1) 
	{ 
		return; 
	}

#if FMESH_ALLOW_POSTER_BONES
	BOOL bPosterBones = FALSE;
	CFMtx43A PosterMtx;

	if ( (m_nFlags & (FMESHINST_FLAG_POSTER_X | FMESHINST_FLAG_POSTER_Y | FMESHINST_FLAG_POSTER_Z)) && (m_nFlags & FMESHINST_FLAG_POSTER_BONES) )
	{
		if ( m_pMesh->nBoneCount )
		{
			bPosterBones = TRUE;
			PosterMtx.Mul( m_PosMtx, m_Xfm.m_MtxR );
		}
	}
#endif
	for (nPass = 0; nPass < nTotalPasses; nPass++)
	{
		BOOL bDirectionalOnly = fshadow_SetupShadowPass(nPass, m_pMesh->nFlags&FMESH_FLAGS_VOLUME_MESH);
		
		FDX8MeshCluster_t *pCluster = ((FDX8MeshMaterial_t *)pMaterial->pPlatformData)->aCluster;
		FDX8MeshCluster_t *pEndCluster = &pCluster[((FDX8MeshMaterial_t *)pMaterial->pPlatformData)->nClusterCount];
		for ( ;pCluster < pEndCluster; pCluster++ )
		{
			if ( !(m_nDrawnPartsMask & (1 << pCluster->nPartID)) )
			{
				continue;
			}

			if ( pCluster->nLODID != nLODIndex )
			{
				continue;
			}

			if ( bDirectionalOnly && (pCluster->nFlags & CLUSTER_FACING_OPP_DIR_LIGHT) )
			{
				continue;
			}

			fdx8vb_Select( &pMeshIS->aVB[ pCluster->nVBIndex ], FALSE, FALSE, FALSE );

			if ( nCurrentIB != pCluster->nIBIndex )
			{
				nCurrentIB = pCluster->nIBIndex;
				FDX8_pDev->SetIndices( (IDirect3DIndexBuffer8 *)pMeshIS->apDXIB[nCurrentIB], 0 );
			}

			D3DXMATRIX boneMtx;

			if ( nMeshBoneCount ) 
			{
				FASSERT( pCluster->nSegmentIdx < m_pMesh->nSegCount );

				if ( pSeg != &aSeg[pCluster->nSegmentIdx] )
				{
					pSeg = &aSeg[pCluster->nSegmentIdx];

					if ( pSeg->nBoneMtxCount == 0 )
					{
						// No bones for this segment, so just set this mesh instance's xfm
						// as the model matrix...
						fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );
						fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &m_PosMtx, TRUE );
						fdx8xfm_SetRenormalizeNormalsIfNeeded( m_fMatrixScale );
					} 
					else if ( pSeg->nBoneMtxCount == 1 ) 
					{
						// This is a non-skinned segment...

						fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );

						if ( !(m_nFlags & FMESHINST_FLAG_NOBONES) ) 
						{
							#if FMESH_ALLOW_POSTER_BONES
							if ( !bPosterBones )
							{
								fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ], TRUE );
							} 
							else
							{
								Mtx43A.Mul33( PosterMtx, *m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ] );
								Mtx43A.m_vPos = m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ]->m_vPos;
								fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &Mtx43A, TRUE );
							}
							#else
							fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ], TRUE );
							#endif
							fdx8xfm_SetRenormalizeNormalsIfNeeded( m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[0] ] );
						} 
						else 
						{
							// The FMESHINST_FLAG_NOBONES flag is set and this is a non-skinned segment.
							// This means that the vertices have already been transformed into bone space,
							// and we need to transform them back into at-rest model space...
							Mtx43A.Mul( m_PosMtx, m_pMesh->pBoneArray[ pSeg->anBoneMtxIndex[0] ].AtRestBoneToModelMtx );
							fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &Mtx43A, TRUE );
							fdx8xfm_SetRenormalizeNormalsIfNeeded( &Mtx43A );
						}
					} 
					else 
					{
						// This is a skinned segment...

						if ( !(m_nFlags & FMESHINST_FLAG_NOBONES) ) 
						{
							fdx8_SetRenderState_VERTEXBLEND( _aD3DVertexBlendFlags[pSeg->nBoneMtxCount] );

							for( nBoneMtxNum=0; nBoneMtxNum<pSeg->nBoneMtxCount; nBoneMtxNum++ ) 
							{
								pMtx43A = m_apBoneMtxPalette[ pSeg->anBoneMtxIndex[nBoneMtxNum] ];
								D3DXMatrixTranspose(&boneMtx, (const D3DXMATRIX *)pMtx43A);
								FDX8_pDev->SetVertexShaderConstant(CV_BONEMTX0_0 + 3*nBoneMtxNum, &boneMtx(0, 0), 3);
							}
						} 
						else 
						{
							fdx8_SetRenderState_VERTEXBLEND( D3DVBF_DISABLE );
							fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &m_PosMtx, TRUE );
							fdx8xfm_SetRenormalizeNormalsIfNeeded( m_fMatrixScale );
						}
					}
				}
			}

			// Render the tri-list...
			if ( pCluster->TriList.nTriCount )
			{
				if (FAILED(FDX8_pDev->DrawIndexedPrimitive(
					D3DPT_TRIANGLELIST,
					pCluster->TriList.nVtxIndexMin,
					pCluster->TriList.nVtxIndexRange,
					pCluster->TriList.nStartVindex,
					pCluster->TriList.nTriCount
				)))
				{
					fang_DevPrintf("fdx8mesh::DrawInstanceMaterial_P - DrawPrimitive failed.\n");
				}

				#if FPERF_ENABLE
					FPerf_nRawTriListCount++;
					FPerf_nRawListTriCount += pCluster->TriList.nTriCount;
					FPerf_nRawVertexCount += (pCluster->TriList.nTriCount<<1) + pCluster->TriList.nTriCount;
					FPerf_nTotalTrisShadow += pCluster->TriList.nTriCount;
				#endif
			}

			// Render the strip-list...
			FDX8MeshStrip_t *pStrip;
			for ( nStrip=0, pStrip = pCluster->paStripBuffer; nStrip < pCluster->nStripCount; nStrip++, pStrip++ ) 
			{
				if (FAILED(FDX8_pDev->DrawIndexedPrimitive(
					D3DPT_TRIANGLESTRIP,
					pStrip->nVtxIndexMin,
					pStrip->nVtxIndexRange,
					pStrip->nStartVindex,
					pStrip->nTriCount
				)))
				{
					fang_DevPrintf("fdx8mesh::DrawInstanceMaterial_P - DrawPrimitive failed.\n");
				}

				#if FPERF_ENABLE
					FPerf_nRawTriStripCount++;
					FPerf_nRawStripTriCount += pStrip->nTriCount;
					FPerf_nRawVertexCount += pStrip->nTriCount + 2;
					FPerf_nTotalTrisShadow += pStrip->nTriCount;
				#endif
			}
		}
	}

//	fdx8shadow_ClearTextures();
}


//
//
//
void CFMeshInst::AddShaderLights( void ) 
{
	u32 i;
	BOOL bLightPerPixel = (m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH) || (m_nFlags & FMESHINST_FLAG_LIGHT_PER_PIXEL);

	fsh_Light_ResetList();

	//Add Spotlights
	for ( i = 0; i < m_nRenderLightCount; i++ ) 
	{
		if (m_pRenderLights[i].pLight->m_nType == FLIGHT_TYPE_SPOT)
		{
			fsh_Light_Add( m_pRenderLights[i].pLight, bLightPerPixel );
		}
	}
	//Add Everything else
	for ( i = 0; i < m_nRenderLightCount; i++ ) 
	{
		if (m_pRenderLights[i].pLight->m_nType != FLIGHT_TYPE_SPOT)
		{
			fsh_Light_Add( m_pRenderLights[i].pLight, bLightPerPixel );
		}
	}

	if ( !(m_nFlags & (FMESHINST_FLAG_NOLIGHT_AMBIENT | FMESHINST_FLAG_LM | FMESHINST_FLAG_VERT_RADIOSITY)) ) 
	{
		fsh_SetGlobalRegister( FSHREG_AMBIENT, (u32)&FMesh_AmbientMotif );
	} 
	else 
	{
		fsh_SetGlobalRegister( FSHREG_AMBIENT, (u32)&FColor_MotifBlack );
	}
}


//
//
//
static BOOL _WindowCreatedCallback( FDX8VidEvent_e nEvent ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( nEvent>=0 && nEvent<FDX8VID_EVENT_COUNT );

	switch( nEvent ) 
	{
		case FDX8VID_EVENT_WINDOW_CREATED:
			_bWindowCreated = TRUE;

			FMesh_AmbientMotif.nMotifIndex = 0;
			FMesh_AmbientMotif.fAlpha = 1.0f;
			fmesh_Ambient_Set( 1.0f, 1.0f, 1.0f, 1.0f );

			break;

		case FDX8VID_EVENT_WINDOW_DESTROYED:
			_bWindowCreated = FALSE;
			break;

		case FDX8VID_EVENT_PRE_RESET:
			break;

		case FDX8VID_EVENT_POST_RESET:
			break;
	}

	return TRUE;
}


//
//
//
static BOOL _ResLoadCreate( FResHandle_t hRes, void *pLoadedBase, u32 nLoadedBytes, cchar *pszResName )
{
	FMesh_t *pMesh;
	void *pDxMeshData = NULL;
	u32 nDxMeshBytes = 0;
	if ( !gcmesh_ConvertToDx( pLoadedBase, nLoadedBytes, &pDxMeshData, &nDxMeshBytes, pszResName ) )
	{
		return FALSE;
	}

	// Create the runtime mesh data...
	pMesh = (FMesh_t *)fdx8load_Create( (FMesh_t *)pDxMeshData, pszResName );
	(void)nDxMeshBytes;
	if ( pMesh == NULL ) 
	{
		// Could not create mesh...
		return FALSE;
	}

#if FHEAP_TRACK_MEM_ALLOCATIONS		
	if ( pMesh->pMeshIS )
	{
		u32 i;
		if ( pMesh->pMeshIS->aVB )
		{
			u32 nVBCount = pMesh->pMeshIS->nVBCount;
			FDX8VB_t *aVB = pMesh->pMeshIS->aVB;
			u32 nVBMem = 0;
			for ( i = 0; i < nVBCount; i++ )
			{
				nVBMem += (aVB[i].nVtxCount * aVB[i].nBytesPerVertex);
			}
			Fheap_nTotalMeshMemTracked += nVBMem;
		}

		for ( i = 0; i < pMesh->pMeshIS->nIBCount; i++ )
		{
			if ( pMesh->pMeshIS->apDXIB[i] )
			{
				Fheap_nTotalMeshMemTracked += pMesh->pMeshIS->anIndicesCount[i] * sizeof(u16);
			}
		}
	}
#endif

	// Mesh created successfully...

	if ( hRes != FRES_NULLHANDLE )
	{
		fres_SetBase( hRes, pMesh );
	}

	return TRUE;
}


//
//
//
static void _ResLoadDestroy( void *pBase ) 
{

	FMesh_t *pMesh = (FMesh_t *)pBase;

#if FHEAP_TRACK_MEM_ALLOCATIONS
	if ( pMesh && pMesh->pMeshIS )
	{
		if ( pMesh->pMeshIS->aVB )
		{
			u32 nVBCount = pMesh->pMeshIS->nVBCount;
			FDX8VB_t *aVB = pMesh->pMeshIS->aVB;
			u32 i, nVBMem = 0;
			for ( i = 0; i < nVBCount; i++ )
			{
				nVBMem += (aVB[i].nVtxCount * aVB[i].nBytesPerVertex);
			}
			
			Fheap_nTotalMeshMemTracked -= nVBMem;
		}

		u32 i;
		for ( i = 0; i < pMesh->pMeshIS->nIBCount; i++ )
		{
			if ( pMesh->pMeshIS->apDXIB[i] )
			{
				Fheap_nTotalMeshMemTracked -= pMesh->pMeshIS->anIndicesCount[i] * sizeof(u16);
			}
		}
	}
#endif

	fdx8load_Destroy( (FMesh_t *)pBase );
}


//
//
//
void fmesh_Renderer_Open( void ) 
{
	fdx8_SetRenderState_NORMALIZENORMALS( FALSE );
	fmesh_SetCullDir( FMESH_CULLDIR_CW );
	fdx8_SetRenderState_SHADEMODE( D3DSHADE_GOURAUD );

	fsh_Open();
}


//
//
//
void fmesh_Renderer_Close( void ) 
{
	fsh_Close();
}


