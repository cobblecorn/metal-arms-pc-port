//////////////////////////////////////////////////////////////////////////////////////
// fGCmesh.cpp - Fang mesh module (GameCube version).
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
// 02/18/02	Lafleur		Created from stubbed DX version.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fGC.h"
#include "fGCmesh.h"
#include "fGCload.h"
#include "fGCtex.h"
#include "fGCvid.h"
#include "fGCviewport.h"
#include "fGCxfm.h"
#include "fGCdisplaylist.h"
#include "fGCData.h"
#include "fGCsh.h"

#include "fsh.h"
#include "fshaders.h"
#include "fshadow.h"

#include "fworld.h"
#include "fdata.h"
#include "fmesh.h"
#include "fres.h"
#include "fresload.h"
#include "flight.h"
#include "fcoll.h"
#include "fperf.h"
#include "frenderer.h"
#include "fclib.h"
#include "fmesh_coll.h"
#include "fdatastreaming.h"

#include "fdraw.h"
#include "FkDOP.h"


//////////////////////////////////////////////////////////////////////////////////////
// Global variables:
//////////////////////////////////////////////////////////////////////////////////////

u32 FMesh_nLastSurfaceMaterialCRC;


//////////////////////////////////////////////////////////////////////////////////////
// Local Structures:
//////////////////////////////////////////////////////////////////////////////////////

//
//
struct _MapTable_t
{
	u32 nFangValue;
	u32 nGCValue;
	
};


//////////////////////////////////////////////////////////////////////////////////////
// Local variables:
//////////////////////////////////////////////////////////////////////////////////////

static _MapTable_t _aMap_CullDir[] = 
{
	FMESH_CULLDIR_CW, 	GX_CULL_FRONT,
	FMESH_CULLDIR_CCW,	GX_CULL_BACK,		
	FMESH_CULLDIR_NONE,	GX_CULL_NONE,
};


//////////////////////////////////////////////////////////////////////////////////////
// Local defines:
//////////////////////////////////////////////////////////////////////////////////////


static BOOL _bModuleInitialized;
static BOOL _bWindowCreated;

static FResLoadReg_t _ResLoadRegistration;

static FMeshCullDir_e _nCurrentCullDir;


//////////////////////////////////////////////////////////////////////////////////////
// Static function prototypes:
//////////////////////////////////////////////////////////////////////////////////////

static BOOL _WindowCreatedCallback( FGCVidEvent_e nEvent );
static BOOL _ResLoadCreate( FResHandle_t hRes, void *pLoadedBase, u32 nLoadedBytes, cchar *pszResName );
static void _ResLoadDestroy( void *pResMem );

static u16 _TransformSkinnedVerts( FGCMeshSkin_t *pSkin, CFMtx43A **apBoneMtxList );
FINLINE void _TransformVert1Matrix( FGCSkinPosNorm_t *pResult, FGCSkinPosNorm_t *pVert, CFMtx43A *pMtx1 );
FINLINE void _TransformVert2Matrix( FGCSkinPosNorm_t *pResult, FGCSkinPosNorm_t *pVert, 
										FGCWeights_t *pWeights, CFMtx43A *pMtx1, CFMtx43A *pMtx2 );
FINLINE void _TransformVert3or4Matrix( FGCSkinPosNorm_t *pResult, FGCSkinPosNorm_t *pVert, 
										u8 *pWeights, CFMtx43A **apBoneMtxList, u8 *paMatrixIdx, u32 nWeights );
										
static FINLINE void _SetCullDir_P( FMeshCullDir_e nCullDir );


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
BOOL fgcmesh_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

	_bWindowCreated = FALSE;
	
	fgcvid_RegisterWindowCallbackFunction( _WindowCreatedCallback );

	fmesh_InitNormalSphere();
	DCFlushRange( FMesh_avCNormalSphere, sizeof( FMesh_CNorm8_t ) * FMESH_NORMAL_SPHERE_MAX_INDEX );
	
	fres_CopyType( _ResLoadRegistration.sResType, FMESH_RESTYPE );
	_ResLoadRegistration.pszFileExtension = "ape";
	_ResLoadRegistration.nMemType = FRESLOAD_MEMTYPE_PERM;
	_ResLoadRegistration.nAlignment = 32;
	_ResLoadRegistration.pFcnCreate = _ResLoadCreate;
	_ResLoadRegistration.pFcnDestroy = _ResLoadDestroy;

	_nCurrentCullDir = FMESH_CULLDIR_NONE;

	if ( !fresload_RegisterHandler( &_ResLoadRegistration ) ) 
	{
		// Registration failed...
		DEVPRINTF( "fgcmesh_ModuleStartup(): Could not register resource.\n" );
		return FALSE;
	}

	FMesh_nMaxMeshLights = 6;
	
	_bModuleInitialized = TRUE;

	return TRUE;
}


//
//
//
void fgcmesh_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

	fgcvid_UnregisterWindowCallbackFunction( _WindowCreatedCallback );

	_bModuleInitialized = FALSE;
}


//
//
//
static FINLINE void _SetCullDir_P( FMeshCullDir_e nCullDir ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_MESH );
	FASSERT( nCullDir>=0 && nCullDir<FMESH_CULLDIR_COUNT );

//	if ( _nCurrentCullDir!=nCullDir ) 
	{
		_nCurrentCullDir = nCullDir;
		fgc_SetCullMode( (GXCullMode)_aMap_CullDir[nCullDir].nGCValue );
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
BOOL fmesh_FlushDrawPrep( void )
{
	fgcxfm_FlushMatrixBufferMemory( TRUE );
	// Flush all of the matrix data from the CPU so that it is available to the GP
//	PPCSync();
	
	return TRUE;
}


//
//
//
CFMeshInst::~CFMeshInst( void ) 
{
}


//
//
//
void CFMeshInst::SetColorStreams( u32 nStreamCount, ColorStream_t *paStreams ) 
{
	u32 i;

	FASSERT( m_pMesh && m_pMesh->pMeshIS ); 

	FResFrame_t Frame = fres_GetFrame();

	if ( nStreamCount != 1 )
	{
		DEVPRINTF( "CFMeshInst::SetColorStreams() - Bad match between color stream data and mesh data for %s.  Color streams ignored.\n", m_pMesh->szName );
		goto _STREAM_CREATE_ERROR;
	}

	m_papColorStreams = fres_AlignedAlloc( sizeof( u32 *) * nStreamCount, 32 );
	if ( !m_papColorStreams )
	{
		goto _STREAM_CREATE_ERROR;
	}

	m_nColorStreamCount = nStreamCount;

	// Copy over the stream of vertex colors
	for ( i = 0; i < nStreamCount; i++ )
	{
		if ( paStreams[i].nColorCount != m_pMesh->pMeshIS->aVB[i].nDiffuseCount )
		{
			DEVPRINTF( "CFMeshInst::SetColorStreams() - Warning!!!  Mesh color count does not match vert rad color count for %s.\n", m_pMesh->szName );
			goto _STREAM_CREATE_ERROR;
		}
		u32 nBytes = sizeof(u32) * paStreams[i].nColorCount;
		((u32 **)m_papColorStreams)[i] = (u32 *)fres_Alloc( sizeof(u32) * paStreams[i].nColorCount );
		if ( !((u32 **)m_papColorStreams)[i] )
		{
			goto _STREAM_CREATE_ERROR;
		}
		fang_MemCopy( ((u32 **)m_papColorStreams)[i], paStreams[i].paVertexColors, sizeof(u32) * paStreams[i].nColorCount );
	}
	
	return;
	
_STREAM_CREATE_ERROR:
	fres_ReleaseFrame( Frame );
	m_nColorStreamCount = 0;
	m_papColorStreams = NULL;	
	
	//Remove static lighting?
	m_nFlags &= ~(FMESHINST_FLAG_NOLIGHT_AMBIENT|FMESHINST_FLAG_NOLIGHT_DYNAMIC|FMESHINST_FLAG_LM|FMESHINST_FLAG_VERT_RADIOSITY);
}


//
//
//
void CFMeshInst::AddShaderLights( void ) 
{
	u32 i;
	BOOL bLightPerPixel = (m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH) || (m_nFlags & FMESHINST_FLAG_LIGHT_PER_PIXEL);

	fsh_Light_ResetList();
	for ( i = 0; i < m_nRenderLightCount; i++ ) 
	{
		fsh_Light_Add( m_pRenderLights[i].pLight, bLightPerPixel );
	}
	
	if ( (m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH) )
	{
		fgcsh_Light_Activate( TRUE, TRUE );
		fgcsh_SetAmbientLight( FMesh_AmbientMotif.fRed, FMesh_AmbientMotif.fGreen, FMesh_AmbientMotif.fBlue, 1.0f );
	}
	else
	{
		fgcsh_Light_Activate( FALSE, TRUE );
		fgcsh_Light_SetAmbient( (u8)(FMesh_AmbientMotif.fRed * 255.f), 
								(u8)(FMesh_AmbientMotif.fGreen * 255.f),
								(u8)(FMesh_AmbientMotif.fBlue * 255.f),
								(u8)(FMesh_AmbientMotif.fAlpha * 255.f) );
	}
}


//
//
//
void CFMeshInst::CacheMeshData( u32 nPriority ) 
{
	u32 i, ii;
	
	if ( nPriority == 0 )
	{
		// Highest priority, so it's time to bring in the display lists (which
		// stream in fast since they're coming in from ARAM)
		
		FMeshMaterial_t *pMaterial = m_pMesh->aMtl;
		for ( i = 0; i < m_pMesh->nMaterialCount; i++, pMaterial++ )
		{
			FGCMeshMaterial_t *pGCMat = (FGCMeshMaterial_t *)pMaterial->pPlatformData;
			for ( ii = 0; ii < pGCMat->nDLContCount; ii++ )
			{
				FASSERT( pGCMat->aDLContainer );
				if ( pGCMat->aDLContainer[ii].nFlags & FGCDL_FLAGS_STREAMING )
				{
					FDS_StreamMgr.CacheData( pGCMat->aDLContainer[ii].pBuffer );
				}
			}
		}
	}
	
	if ( nPriority <= 1 )
	{
		// Low priority, but bring lightmaps into the cache since they take a 
		// long time to come in from DVD
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
					void *pCachedData = FDS_StreamMgr.AccessData( pTexData->pRawTexture );
					if ( pCachedData )
					{
						if ( !GXGetTexObjData( pTexData->pGCTexObj ) )
						{
							GXInitTexObjData( pTexData->pGCTexObj, pCachedData );
						}
					}
					else
					{
						GXInitTexObjData( pTexData->pGCTexObj, NULL );
					}
				}
			}
		}
		else if ( m_nFlags & FMESHINST_FLAG_WORLD_GEO )
		{
			FMeshMaterial_t *pMaterial = m_pMesh->aMtl;
			for ( i = 0; i < m_pMesh->nMaterialCount; i++, pMaterial++ )
			{
				for ( ii = 0; ii < pMaterial->pnShLightRegisters[FSHADERS_LIGHT_REG_LMCOUNT]; ii++ )
				{
					FShTexInst_t *pTexInst = (FShTexInst_t *)pMaterial->pnShLightRegisters[FSHADERS_LIGHT_REG_LM + (ii * 3)];
					FASSERT( pTexInst );
					if ( pTexInst->TexInst.GetTexDef() && (pTexInst->TexInst.GetTexDef()->TexInfo.nFlags & FTEX_FLAG_STREAMING) )
					{
						FTexData_t *pTexData = pTexInst->TexInst.GetTexDef()->pTexData;
						void *pCachedData = FDS_StreamMgr.AccessData( pTexData->pRawTexture );
						if ( pCachedData )
						{
							if ( !GXGetTexObjData( pTexData->pGCTexObj ) )
							{
								GXInitTexObjData( pTexData->pGCTexObj, pCachedData );
							}
						}
						else
						{
							GXInitTexObjData( pTexData->pGCTexObj, NULL );
						}
					}
				}
			}
		}
	}
}


//
//
//
BOOL CFMeshInst::DrawPrep_P( BOOL bFlushImmediate ) 
{
	u32 i;

	if ( m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH )
	{
		return TRUE;
	}
	
	BOOL8 bModelPushed;
	u8 nNumPops;
	CFMtx43A *pModelToWorldMtx;
	Mtx *pPosMtxBuffer;
	Mtx33 *pNrmMtxBuffer;
	f32 fInvScale = 1.f;
	
	if ( m_nFlags & (FMESHINST_FLAG_POSTER_Y|FMESHINST_FLAG_POSTER_X|FMESHINST_FLAG_POSTER_Z) )
	{
		// If this is postered, we need to develop a concatenated poster matrix
		nNumPops = PushXfm();
		bModelPushed = TRUE;

		// Store the concatenated model matrix and scale for later use
		pModelToWorldMtx = &FXfm_pModel->m_MtxF;
		fInvScale = FXfm_pModel->m_fScaleR;
	}
	else
	{
		// Store the concatenated model matrix and scale for later use
		pModelToWorldMtx = &m_Xfm.m_MtxF;
		fInvScale = m_Xfm.m_fScaleR;
		bModelPushed = FALSE;
	}
	
	CFMtx43A mtxBone;
	CFMtx43A mtxViewMatrix( FGCXfm_mtxGCLeftToRightHandViewMtx );
	
	#if FMESH_ALLOW_POSTER_BONES
	BOOL bPosterBones = FALSE;
	CFMtx43A PosterMtx;

	if ( (m_nFlags & (FMESHINST_FLAG_POSTER_X | FMESHINST_FLAG_POSTER_Y | FMESHINST_FLAG_POSTER_Z)) && (m_nFlags & FMESHINST_FLAG_POSTER_BONES) )
	{
		if ( m_pMesh->nBoneCount )
		{
			bPosterBones = TRUE;
			PosterMtx.Mul( FXfm_pModel->m_MtxF, m_Xfm.m_MtxR );
		}
	}
	#endif

	if ( FXfm_pMirrorMtx )
	{
		mtxViewMatrix.Mul( *FXfm_pMirrorMtx );
	}
	
	// How many matrices will we need to display this mesh?
	u32 nMatrixCount = m_pMesh->nUsedBoneCount;

	if ( nMatrixCount == 0 )
	{
		// Get an array from the matrix buffers
		m_nStartingMatrixBufferIdx = fgcxfm_AllocateMtxBuffers( 1, &pPosMtxBuffer, &pNrmMtxBuffer );
		if ( m_nStartingMatrixBufferIdx == 0xffff )
		{
			if ( bModelPushed )
			{
				// Pop the matrix
				CFXfm::PopModel( nNumPops );
			}
			return FALSE;
		}

		// Set the model view matrix
		mtxBone.Mul( mtxViewMatrix, *pModelToWorldMtx );
		fgcxfm_Convert43AToGCMtx( pPosMtxBuffer[ 0 ], mtxBone );
		
		// Invert, transpose and set the normal matrix based on the model matrix
		// NOTE: GC requires normals and lights to be in view space
		if ( fmath_Abs( 1.f - fInvScale ) > 0.01f )
		{
			mtxBone.Mul( fInvScale );
		}
		fgcxfm_Convert43AToGCNrmMtx33( pNrmMtxBuffer[ 0 ], mtxBone );
	}
	else
	{
		CFMtx43A **apBoneMtxList;//, *pMtx43, Mtx43;
		apBoneMtxList = GetBoneMtxPalette();
		FMeshBone_t *pBones = m_pMesh->pBoneArray;
		
		// If we have skinned verts, we need to include the base model-view matrix
		if ( m_pMesh->pMeshIS->pMeshSkin )
		{
			nMatrixCount++;
		}
		
		// Get an array from the matrix buffers
		m_nStartingMatrixBufferIdx = fgcxfm_AllocateMtxBuffers( nMatrixCount, &pPosMtxBuffer, &pNrmMtxBuffer );
		if ( m_nStartingMatrixBufferIdx == 0xffff )
		{
			if ( bModelPushed )
			{
				// Pop the matrix
				CFXfm::PopModel( nNumPops );
			}
			return FALSE;
		}

		// Set up the bone matrix arrays
		if ( !(m_nFlags & FMESHINST_FLAG_NOBONES) ) 
		{
			// Concatenate each bone's matrix with this mesh instance's
			// xfm and set it in the appropriate model matrix slot...

			for ( i = 0; i < m_pMesh->nUsedBoneCount; i++ ) 
			{
				if ( !(m_nDrawnPartsMask & (1 << pBones[i].nPartID)) )
				{
					continue;
				}
				
				#if FMESH_ALLOW_POSTER_BONES
				if ( bPosterBones ) 
				{
					mtxBone.Mul( PosterMtx, *apBoneMtxList[i] );
					mtxBone.m_vPos = apBoneMtxList[i]->m_vPos;
					mtxBone.Mul( mtxViewMatrix, mtxBone );
				}
				else 
				{
					mtxBone.Mul( mtxViewMatrix, *apBoneMtxList[i] );
				}
				#else				
				mtxBone.Mul( mtxViewMatrix, *apBoneMtxList[i] );
				#endif
				
				fgcxfm_Convert43AToGCMtx( pPosMtxBuffer[i], mtxBone );
				
				// Invert, transpose and set the normal matrix based on the model matrix
				// NOTE: GC requires normals and lights to be in view space
				f32 fScale = mtxBone.m_vRight.Mag();
				if ( fmath_Abs( 1.f - fScale ) > 0.01f )
				{
					mtxBone.Mul( fmath_Inv( fScale ) );
				}
				fgcxfm_Convert43AToGCNrmMtx33( pNrmMtxBuffer[i], mtxBone );
			}
		} 
		else 
		{
			CFMtx43A mtxModelView;
			mtxModelView.Mul( mtxViewMatrix, *pModelToWorldMtx );
			
			for ( i = 0; i < m_pMesh->nUsedBoneCount; i++ ) 
			{
				if ( !(m_nDrawnPartsMask & (1 << pBones[i].nPartID)) )
				{
					continue;
				}
				
				// Set the model view matrix
				mtxBone.Mul( mtxModelView, m_pMesh->pBoneArray[i].AtRestBoneToModelMtx );
				if ( m_pMesh->pMeshIS->pMeshSkin && apBoneMtxList )
				{
					apBoneMtxList[i]->Mul( *pModelToWorldMtx, m_pMesh->pBoneArray[i].AtRestBoneToModelMtx );
				}
				fgcxfm_Convert43AToGCMtx( pPosMtxBuffer[i], mtxBone );
				
				// Invert, transpose and set the normal matrix based on the model matrix
				// NOTE: GC requires normals and lights to be in view space
				f32 fScale = mtxBone.m_vRight.Mag();
				if ( fmath_Abs( 1.f - fScale ) > 0.01f && fScale > 0.0001f )
				{
					mtxBone.Mul( fmath_Inv( fScale ) );
				}
				fgcxfm_Convert43AToGCNrmMtx33( pNrmMtxBuffer[i], mtxBone );
			}
		}
		
		// If we have skinned verts, append the base model-view matrix to the end
		if ( m_pMesh->pMeshIS->pMeshSkin )
		{
			// Set the Model View matrix 
			fgcxfm_Convert43AToGCMtx( pPosMtxBuffer[ nMatrixCount - 1 ], mtxViewMatrix );
			
			// Invert, Set the normal matrix
			fgcxfm_SetMtx33ToIdentity( pNrmMtxBuffer[ nMatrixCount - 1 ] );

			// NOW TRANSFORM THE SKINNED VERTS
			m_nStartingSkinVertexBufferIdx = _TransformSkinnedVerts( m_pMesh->pMeshIS->pMeshSkin, apBoneMtxList );
			if ( m_nStartingSkinVertexBufferIdx == 0xffff )
			{
				if ( bModelPushed )
				{
					// Pop the matrix
					CFXfm::PopModel( nNumPops );
				}
				return FALSE;
			}
		}
	}

	if ( bModelPushed )
	{
		// Pop the matrix
		CFXfm::PopModel( nNumPops );
	}
	
	if ( bFlushImmediate )
	{
		fgcxfm_FlushMatrixBufferMemory( TRUE );
	}

	return TRUE;
}


//
//
//
void CFMeshInst::SetGPMatrix( FMeshMaterial_t *pMaterial, u16 nCurrentDL ) 
{
	u16 nMatrixIdx;
	FGCMeshMaterial_t *pGCMaterial = (FGCMeshMaterial_t *)pMaterial->pPlatformData;
	FASSERT( pGCMaterial->aDLContainer );
	FGC_DLCont_t *pDLCont = &pGCMaterial->aDLContainer[nCurrentDL];
	
	// Matrix is currently not loaded, so load the next 9 used matrices
	u32 i;
	u8 nGPMatrixIdx = 0;
	for ( i = nCurrentDL; i < pGCMaterial->nDLContCount && nGPMatrixIdx < 10; i++ )
	{
		if ( !(m_nDrawnPartsMask & (1 << pGCMaterial->aDLContainer[i].nPartID)) )
		{
			continue;
		}
		
		fgcxfm_LoadMatrixToGP( m_nStartingMatrixBufferIdx + pGCMaterial->aDLContainer[i].nMatrixIdx, nGPMatrixIdx );
		nGPMatrixIdx++;
	}

	// Try setting the matrix as current (This should not fail)	
	nMatrixIdx = m_nStartingMatrixBufferIdx + pDLCont->nMatrixIdx;
	
#if FANG_DEBUG_BUILD	
	if ( !fgcxfm_SetCurrentGPMatrix( nMatrixIdx ) )
	{
		// Something went horribly wrong - the set failed.
		FASSERT_NOW;
	}	
#else
	fgcxfm_SetCurrentGPMatrix( nMatrixIdx );
#endif
}


//
//
//
BOOL CFMeshInst::DrawMaterialLight_P( FViewportPlanesMask_t nCrossesPlanesMask, FMeshMaterial_t *pMaterial, u32 nLODIndex )
{
	u32 iii;
	s32 nShadowID=0;
	BOOL bMultiMatrix = TRUE;
	FGCMeshMaterial_t *pGCMaterial = (FGCMeshMaterial_t *)pMaterial->pPlatformData;

#if FMESH_COLL_ENABLE_COLLISION_DRAWS
	if ( FMesh_nDrawCollisionGeoFlags )
	{
		return FALSE;
	}
#endif

	if ( !(pMaterial->nPartIDMask & m_nDrawnPartsMask) )
	{
		return FALSE;
	}
	
	if ( m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH )
	{
//		fxfm_SetViewAndWorldSpaceModelMatrices();
		bMultiMatrix = FALSE;
		nShadowID = 0;
	}
	else 
	{
		if ( m_nStartingMatrixBufferIdx == 0xffff )
		{
			return FALSE;
		}
		
		if ( m_pMesh->nUsedBoneCount == 0 )
		{
			SetGPMatrix( pMaterial, 0 );
			bMultiMatrix = FALSE;
		}
		
		nShadowID = 1;
	}
	
	// Set clipping mode
	if ( (nCrossesPlanesMask != FVIEWPORT_PLANESMASK_NONE) || (m_nFlags & FMESHINST_FLAG_FORCE_CLIP) )
	{
		fgc_SetClipMode( TRUE );
	}
	else
	{
		fgc_SetClipMode( FALSE );
	}
	
	_SetCullDir_P( (FMeshCullDir_e)m_nCullDirection );

	SetupShaderParameters( pMaterial, FSHADERS_PASS_LIGHTING );
	
	// TEMP HACK TO SET BUMPMAP TILE FACTOR
	//fsh_SetBumpMapTileFactor( *((f32 *)&pMaterial->pnShLightRegisters[FSHADERS_LIGHT_REG_BUMPMAP_TILE_FACTOR]) );
	fgcsh_EnableColorKey( m_nFlags&FMESHINST_FLAG_ENABLECOLORKEY );
	
	BOOL bFastPass = FALSE;

	u32 nTotalPasses = fsh_GetCurrentNumPasses(&bFastPass);
	u32 nPass;
	
	for (nPass=0; nPass < nTotalPasses; nPass++)
	{
		fgcsh_SetLMapTCStart(pMaterial->nBaseSTSets);
		
		fsh_SetPassIdx( nPass );
		fsh_ExecuteCurrent( FMesh_bRenderShadows, nShadowID );

		// Add in each display list in the material
		for ( iii = 0; iii < pGCMaterial->nDLContCount; iii++ )
		{
			FASSERT( pGCMaterial->aDLContainer );
			FGC_DLCont_t *pDLCont = &pGCMaterial->aDLContainer[iii];
			FASSERT( pDLCont );
			
			if ( !(m_nDrawnPartsMask & (1 << pDLCont->nPartID)) )
			{
				continue;
			}
			
			if ( pDLCont->nLODID != nLODIndex )
			{
				continue;
			}

			#if FPERF_ENABLE
			if ( nPass == 0 )
			{
				FPerf_nMeshTris += pDLCont->nStripTriCount + pDLCont->nListTriCount;
			}
			FPerf_nTotalTrisLighting += pDLCont->nStripTriCount + pDLCont->nListTriCount;
			FPerf_nRawStripTriCount += pDLCont->nStripTriCount;
			FPerf_nRawListTriCount += pDLCont->nListTriCount;
			FPerf_nRawTriStripCount += pDLCont->nStripCount;
			FPerf_nRawTriListCount += pDLCont->nListCount;
			FPerf_nRawVertexCount += (pDLCont->nListTriCount<<1) + pDLCont->nListTriCount + pDLCont->nStripTriCount + 2;
			#endif			
			
			if ( bMultiMatrix )
			{
				if ( !fgcxfm_SetCurrentGPMatrix( m_nStartingMatrixBufferIdx + pDLCont->nMatrixIdx ) )
				{
					SetGPMatrix( pMaterial, iii );
				}
			}
			
			// Setup the vertex type
			pDLCont->Submit( this, pMaterial->nBaseSTSets + pMaterial->nLightMapSTSets, bFastPass );
		}
	}
	
	fsh_EndExecute();
	
	if ( m_pMesh->nTexLayerIDCount )
	{
		fsh_TexCoordMtx_ResetList();
		fsh_TexOverride_ResetList();
	}
	
	return bFastPass;
}


//
//
//
void CFMeshInst::DrawMaterialSurface_P( FViewportPlanesMask_t nCrossesPlanesMask, FMeshMaterial_t *pMaterial, u32 nLODIndex )
{
	u32 iii;
	BOOL bMultiMatrix = TRUE;
	FGCMeshMaterial_t *pGCMaterial = (FGCMeshMaterial_t *)pMaterial->pPlatformData;
	
#if FMESH_COLL_ENABLE_COLLISION_DRAWS
	if ( FMesh_nDrawCollisionGeoFlags )
	{
		return;
	}
#endif

	if ( !(pMaterial->nPartIDMask & m_nDrawnPartsMask) )
	{
		return;
	}
	
	if ( m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH )
	{
//		fxfm_SetViewAndWorldSpaceModelMatrices();
		bMultiMatrix = FALSE;
	}
	else 
	{
		if ( m_nStartingMatrixBufferIdx == 0xffff )
		{
			return;
		}
		
		if ( m_pMesh->nUsedBoneCount == 0 )
		{
			SetGPMatrix( pMaterial, 0 );
			bMultiMatrix = FALSE;
		}
	}
	
	// Set clipping mode
	if ( (nCrossesPlanesMask != FVIEWPORT_PLANESMASK_NONE) || (m_nFlags & FMESHINST_FLAG_FORCE_CLIP) )
	{
		fgc_SetClipMode( TRUE );
	}
	else
	{
		fgc_SetClipMode( FALSE );
	}
	
	_SetCullDir_P( (FMeshCullDir_e)m_nCullDirection );
	
	SetupShaderParameters( pMaterial, FSHADERS_PASS_SURFACE );
	
	// TEMP HACK TO SET DETAIL TILE FACTOR
	u32 nDetailTileRegister = 0;
	if ( FShaders_aShaderRegs[pMaterial->nSurfaceShaderIdx].nSurfaceTypeFlags & FSHADERS_SURFACE_FLAG_DETAIL_MAP )
	{
		while ( FShaders_aShaderRegs[pMaterial->nSurfaceShaderIdx].anRegType[nDetailTileRegister] != FSHADERS_REG_DETAILMAP_TILE_FACTOR )
		{
			nDetailTileRegister++;
		}
		fsh_SetDetailMapTileFactor( *((f32 *)&pMaterial->pnShSurfaceRegisters[nDetailTileRegister]) );
	}

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
	
	// Add in each display list in the material
	for ( iii = 0; iii < pGCMaterial->nDLContCount; iii++ )
	{
		FASSERT( pGCMaterial->aDLContainer );
		FGC_DLCont_t *pDLCont = &pGCMaterial->aDLContainer[iii];
		FASSERT( pDLCont );
		
		if ( !(m_nDrawnPartsMask & (1 << pDLCont->nPartID)) )
		{
			continue;
		}
		
		if ( pDLCont->nLODID != nLODIndex )
		{
			continue;
		}

		#if FPERF_ENABLE
		FPerf_nTotalTrisSurface += pDLCont->nStripTriCount + pDLCont->nListTriCount;
		FPerf_nRawStripTriCount += pDLCont->nStripTriCount;
		FPerf_nRawListTriCount += pDLCont->nListTriCount;
		FPerf_nRawTriStripCount += pDLCont->nStripCount;
		FPerf_nRawTriListCount += pDLCont->nListCount;
		FPerf_nRawVertexCount += (pDLCont->nListTriCount<<1) + pDLCont->nListTriCount + pDLCont->nStripTriCount + 2;
		#endif			
		
		if ( bMultiMatrix )
		{
			if ( !fgcxfm_SetCurrentGPMatrix( m_nStartingMatrixBufferIdx + pDLCont->nMatrixIdx ) )
			{
				SetGPMatrix( pMaterial, iii );
			}
		}
		
		// Setup the vertex type
		pDLCont->Submit( this, pMaterial->nBaseSTSets + pMaterial->nLightMapSTSets, TRUE );
	}
	
	if ( m_pMesh->nTexLayerIDCount )
	{
		fsh_TexCoordMtx_ResetList();
		fsh_TexOverride_ResetList();
	}
	
	fsh_EndExecute();
}


//
//
//
void CFMeshInst::DrawMaterialSpecular_P( FViewportPlanesMask_t nCrossesPlanesMask, FMeshMaterial_t *pMaterial, u32 nLODIndex )
{
	FASSERT_NOW;
/*
	u32 iii;
	BOOL bMultiMatrix = TRUE;
	FGCMeshMaterial_t *pGCMaterial = (FGCMeshMaterial_t *)pMaterial->pPlatformData;

#if FMESH_COLL_ENABLE_COLLISION_DRAWS
	if ( FMesh_nDrawCollisionGeoFlags )
	{
		return;
	}
#endif

	if ( !(pMaterial->nPartIDMask & m_nDrawnPartsMask) )
	{
		return;
	}
	
	if ( m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH )
	{
//		fxfm_SetViewAndWorldSpaceModelMatrices();
		bMultiMatrix = FALSE;
	}
	else 
	{
		if ( m_nStartingMatrixBufferIdx == 0xffff )
		{
			return;
		}
		
		if ( m_pMesh->nUsedBoneCount == 0 )
		{
			SetGPMatrix( pMaterial, 0 );
			bMultiMatrix = FALSE;
		}
	}
	
	// Set clipping mode
	if ( (nCrossesPlanesMask != FVIEWPORT_PLANESMASK_NONE) || (m_nFlags & FMESHINST_FLAG_FORCE_CLIP) )
	{
		fgc_SetClipMode( TRUE );
	}
	else
	{
		fgc_SetClipMode( FALSE );
	}
	
	_SetCullDir_P( (FMeshCullDir_e)m_nCullDirection );

	SetupShaderParameters( pMaterial, FSHADERS_PASS_SPECULAR );
	
	// TEMP HACK TO SET BUMPMAP TILE FACTOR
	fsh_SetBumpMapTileFactor( *((f32 *)&pMaterial->pnShLightRegisters[FSHADERS_LIGHT_REG_BUMPMAP_TILE_FACTOR]) );

	fsh_ExecuteCurrent( TRUE, 0 );

	// Add in each display list in the material
	for ( iii = 0; iii < pGCMaterial->nDLContCount; iii++ )
	{
		FASSERT( pGCMaterial->aDLContainer );
		FGC_DLCont_t *pDLCont = &pGCMaterial->aDLContainer[iii];
		FASSERT( pDLCont );
		
		if ( !(m_nDrawnPartsMask & (1 << pDLCont->nPartID)) )
		{
			continue;
		}
		
		if ( pDLCont->nLODID != nLODIndex )
		{
			continue;
		}

		#if FPERF_ENABLE
		FPerf_nTotalTrisSpecular += pDLCont->nStripTriCount + pDLCont->nListTriCount;
		FPerf_nRawStripTriCount += pDLCont->nStripTriCount;
		FPerf_nRawListTriCount += pDLCont->nListTriCount;
		FPerf_nRawTriStripCount += pDLCont->nStripCount;
		FPerf_nRawTriListCount += pDLCont->nListCount;
		FPerf_nRawVertexCount += (pDLCont->nListTriCount<<1) + pDLCont->nListTriCount + pDLCont->nStripTriCount + 2;
		#endif			
		
		if ( bMultiMatrix )
		{
			if ( !fgcxfm_SetCurrentGPMatrix( m_nStartingMatrixBufferIdx + pDLCont->nMatrixIdx ) )
			{
				SetGPMatrix( pMaterial, iii );
			}
		}
		
		// Setup the vertex type
		pDLCont->Submit( this, pMaterial->nBaseSTSets + pMaterial->nLightMapSTSets, TRUE );
	}
	
	if ( m_pMesh->nTexLayerIDCount )
	{
		fsh_TexCoordMtx_ResetList();
		fsh_TexOverride_ResetList();
	}
	
	fsh_EndExecute();
*/
}


//
//
//
void CFMeshInst::DrawMaterialTranslucency_P( FViewportPlanesMask_t nCrossesPlanesMask, FMeshMaterial_t *pMaterial, u32 nLODIndex )
{
	u32 iii;
	BOOL bMultiMatrix = TRUE;
	FGCMeshMaterial_t *pGCMaterial = (FGCMeshMaterial_t *)pMaterial->pPlatformData;
	
#if FMESH_COLL_ENABLE_COLLISION_DRAWS
	if ( FMesh_nDrawCollisionGeoFlags )
	{
		return;
	}
#endif

	if ( !(pMaterial->nPartIDMask & m_nDrawnPartsMask) )
	{
		return;
	}
	
	if ( m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH )
	{
		fgcsh_SetWorldGeoFlag( TRUE );
		bMultiMatrix = FALSE;
	}
	else 
	{
		fgcsh_SetWorldGeoFlag( FALSE );
		
		if ( m_nStartingMatrixBufferIdx == 0xffff )
		{
			return;
		}
		
		if ( m_pMesh->nUsedBoneCount == 0 )
		{
			SetGPMatrix( pMaterial, 0 );
			bMultiMatrix = FALSE;
		}
	}
	
	// Set clipping mode
	if ( (nCrossesPlanesMask != FVIEWPORT_PLANESMASK_NONE) || (m_nFlags & FMESHINST_FLAG_FORCE_CLIP) )
	{
		fgc_SetClipMode( TRUE );
	}
	else
	{
		fgc_SetClipMode( FALSE );
	}

	_SetCullDir_P( (FMeshCullDir_e)m_nCullDirection );

	if ( !SetupShaderParameters( pMaterial, FSHADERS_PASS_ALL ) ) 
	{ 
		return; 
	}

	fsh_SetProcedural( m_pProcedural );
	
	// TEMP HACK TO SET DETAIL TILE FACTOR
	u32 nDetailTileRegister = 0;
	if ( FShaders_aShaderRegs[pMaterial->nSurfaceShaderIdx].nSurfaceTypeFlags & FSHADERS_SURFACE_FLAG_DETAIL_MAP )
	{
		while ( FShaders_aShaderRegs[pMaterial->nSurfaceShaderIdx].anRegType[nDetailTileRegister] != FSHADERS_REG_DETAILMAP_TILE_FACTOR )
		{
			nDetailTileRegister++;
		}
		fsh_SetDetailMapTileFactor( *((f32 *)&pMaterial->pnShSurfaceRegisters[nDetailTileRegister]) );
	}
	
	fsh_GetCurrentNumPasses();

	fsh_ExecuteCurrent( FALSE, 0 );
	
	// Add in each display list in the material
	for ( iii = 0; iii < pGCMaterial->nDLContCount; iii++ )
	{
		FASSERT( pGCMaterial->aDLContainer );
		FGC_DLCont_t *pDLCont = &pGCMaterial->aDLContainer[iii];
		FASSERT( pDLCont );
		
		if ( !(m_nDrawnPartsMask & (1 << pDLCont->nPartID)) )
		{
			continue;
		}
		
		if ( pDLCont->nLODID != nLODIndex )
		{
			continue;
		}

		#if FPERF_ENABLE
		FPerf_nMeshTris += pDLCont->nStripTriCount + pDLCont->nListTriCount;
		FPerf_nTotalTrisTranslucent += pDLCont->nStripTriCount + pDLCont->nListTriCount;
		FPerf_nRawStripTriCount += pDLCont->nStripTriCount;
		FPerf_nRawListTriCount += pDLCont->nListTriCount;
		FPerf_nRawTriStripCount += pDLCont->nStripCount;
		FPerf_nRawTriListCount += pDLCont->nListCount;
		FPerf_nRawVertexCount += (pDLCont->nListTriCount<<1) + pDLCont->nListTriCount + pDLCont->nStripTriCount + 2;
		#endif			
		
		if ( bMultiMatrix )
		{
			if ( !fgcxfm_SetCurrentGPMatrix( m_nStartingMatrixBufferIdx + pDLCont->nMatrixIdx ) )
			{
				SetGPMatrix( pMaterial, iii );
			}
		}
		
		// Setup the vertex type
		pDLCont->Submit( this, pMaterial->nBaseSTSets + pMaterial->nLightMapSTSets, TRUE );
	}
	
	if ( m_pMesh->nTexLayerIDCount )
	{
		fsh_TexCoordMtx_ResetList();
		fsh_TexOverride_ResetList();
	}
	
	fsh_EndExecute();
}


//
//
//
void CFMeshInst::DrawAllMaterials_P( FViewportPlanesMask_t nCrossesPlanesMask ) 
{
	u32 i, ii, iii;
	BOOL bMultiMatrix = TRUE;

#if FMESH_COLL_ENABLE_COLLISION_DRAWS
	if ( FMesh_nDrawCollisionGeoFlags )
	{
		return;
	}
#endif

	if ( m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH )
	{
		fxfm_SetViewAndWorldSpaceModelMatrices();
		bMultiMatrix = FALSE;
	}
	else 
	{
		if ( m_nStartingMatrixBufferIdx == 0xffff )
		{
			return;
		}
		
		if ( m_pMesh->nUsedBoneCount == 0 )
		{
			SetGPMatrix( &m_pMesh->aMtl[0], 0 );
			bMultiMatrix = FALSE;
		}
	}
	
	_SetCullDir_P( (FMeshCullDir_e)m_nCullDirection );

	fgc_SetClipMode( TRUE );
	
	fgcsh_SetWorldGeoFlag( !!(m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH) );

	// Setup lights for this mesh
	if ( !FMesh_bRenderShadows )
	{
		AddShaderLights();
	}

	// Determine the LOD we should be using
	f32 fDistToCamSq = FXfm_pModel->m_MtxF.m_vPos.Sub( FXfm_pView->m_MtxR.m_vPos ).MagSq();
	u8 nLODMask, nLODToUse = 0;
	if ( fDistToCamSq > 2.f )
	{
		fDistToCamSq = fmath_Sqrt( fDistToCamSq );
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
		if ( nLODToUse < m_pMesh->nLODCount - 1 )
		{
			nLODToUse++;
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

	// Cycle through each material in each segment
	FMeshMaterial_t *pMaterial = m_pMesh->aMtl;
	for ( i = 0; i < m_pMesh->nMaterialCount; i++, pMaterial++ )
	{
		if ( !(pMaterial->nPartIDMask & m_nDrawnPartsMask) )
		{
			continue;
		}
		
		if ( !(pMaterial->nLODMask & nLODMask) )
		{
			continue;
		}

		FGCMeshMaterial_t *pGCMaterial = (FGCMeshMaterial_t *)pMaterial->pPlatformData;

		// Set shader registers (we have to do this here for the next call to work)
		fsh_SetupShaderRegisters( pMaterial->pnShLightRegisters, pMaterial->pnShSurfaceRegisters, m_panLightRegisterLMOverride, NULL );

		u32 nDiffusePasses = fsh_GetNumDiffusePasses(pMaterial->nSurfaceShaderIdx, pMaterial->nLightShaderIdx);
		u32 nPasses = nDiffusePasses + 1 + (pMaterial->nSpecularShaderIdx != FSH_INVALID_SPECULAR_SHADER);
		
		if ( FMesh_bRenderShadows )
		{
			nPasses = 1;
			nDiffusePasses = 1;
		}

		for ( ii = 0; ii < nPasses; ii++ )
		{
			fsh_SetPassIdx(ii);
			if ( ii < nDiffusePasses )
			{
				fgcsh_SetLMapTCStart( pMaterial->nBaseSTSets );
				fsh_SetShaderType( SHADERTYPE_DIFFUSE );
				if ( !SetupShaderParameters( pMaterial, FSHADERS_PASS_LIGHTING ) )
				{
					continue;
				}
				if ( !fsh_SetShader( pMaterial->nSurfaceShaderIdx, pMaterial->nLightShaderIdx, pMaterial->nSpecularShaderIdx, ii ) )
				{
					continue;
				}
			} 
			else  if ( ii == nDiffusePasses )
			{
				fsh_SetShaderType( SHADERTYPE_SURFACE );
				if (nDiffusePasses == 0)
				{
					if ( !SetupShaderParameters( pMaterial, FSHADERS_PASS_ALL ) )
					{
						continue;
					}
				}
				else if ( !SetupShaderParameters( pMaterial, FSHADERS_PASS_SURFACE ) )
				{
					continue;
				}
				if ( !fsh_SetShader( pMaterial->nSurfaceShaderIdx, pMaterial->nLightShaderIdx, pMaterial->nSpecularShaderIdx ) )
				{
					continue;
				}
			}
			else
			{
				fsh_SetShaderType( SHADERTYPE_SPECULAR ); //diffuse, surface, specular or translucency
				if ( !SetupShaderParameters( pMaterial, FSHADERS_PASS_SPECULAR ) )
				{
					continue;
				}
				if ( !fsh_SetShader( pMaterial->nSurfaceShaderIdx, pMaterial->nLightShaderIdx, pMaterial->nSpecularShaderIdx ) )
				{
					continue;
				}
			}
			fsh_ExecuteCurrent( FMesh_bRenderShadows, 0x01 );
			
			// Add in each display list in the material
			for ( iii = 0; iii < pGCMaterial->nDLContCount; iii++ )
			{
				FASSERT( pGCMaterial->aDLContainer );
				FGC_DLCont_t *pDLCont = &pGCMaterial->aDLContainer[iii];
				FASSERT( pDLCont );

				if ( !(m_nDrawnPartsMask & (1 << pDLCont->nPartID)) )
				{
					continue;
				}
				
				if ( pDLCont->nLODID != nLODToUse )
				{
					continue;
				}

				#if FPERF_ENABLE
					if ( ii == 0 )
					{
						if ( FMesh_bRenderShadows )
						{
							FPerf_nMeshShadowTris += pDLCont->nStripTriCount + pDLCont->nListTriCount;
						}
						else
						{
							FPerf_nMeshTris += pDLCont->nStripTriCount + pDLCont->nListTriCount;
						}
					}
					FPerf_nRawStripTriCount += pDLCont->nStripTriCount;
					FPerf_nRawListTriCount += pDLCont->nListTriCount;
					FPerf_nRawTriStripCount += pDLCont->nStripCount;
					FPerf_nRawTriListCount += pDLCont->nListCount;
					FPerf_nRawVertexCount += (pDLCont->nListTriCount<<1) + pDLCont->nListTriCount + pDLCont->nStripTriCount + 2;
				#endif			
				
				if ( bMultiMatrix )
				{
					if ( !fgcxfm_SetCurrentGPMatrix( m_nStartingMatrixBufferIdx + pDLCont->nMatrixIdx ) )
					{
						SetGPMatrix( pMaterial, iii );
					}
				}

				// Setup the vertex type
				pDLCont->Submit( this, pMaterial->nBaseSTSets + pMaterial->nLightMapSTSets, TRUE );//(ii >= nDiffusePasses) );
			}
		}
	}

	if ( m_pMesh->nTexLayerIDCount ) 
	{
		// No texture layer animation on this object...
		fsh_TexCoordMtx_ResetList();
		fsh_TexOverride_ResetList();
	}
	
	fsh_EndExecute();
}


//
//
//
void CFMeshInst::DrawShadow_P( FViewportPlanesMask_t nCrossesPlanesMask, FMeshMaterial_t *pMaterial, u32 nLODIndex )
{
	u32 iii;
	s32 nShadowID=0;
	BOOL bMultiMatrix = TRUE;
	FGCMeshMaterial_t *pGCMaterial = (FGCMeshMaterial_t *)pMaterial->pPlatformData;

	if ( m_fShadowIntensity < 0.01f )
	{
		return;
	}

	if ( !(pMaterial->nPartIDMask & m_nDrawnPartsMask) )
	{
		return;
	}
	
	if ( m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH )
	{
		fxfm_SetViewAndWorldSpaceModelMatrices();
		bMultiMatrix = FALSE;
		
		nShadowID = 0;
	}
	else 
	{
		if ( m_nStartingMatrixBufferIdx == 0xffff )
		{
			return;
		}
		
		if ( m_pMesh->nUsedBoneCount == 0 )
		{
			SetGPMatrix( pMaterial, 0 );
			bMultiMatrix = FALSE;
		}
		
		nShadowID = 1;
	}
	
	// Set clipping mode
	if ( (nCrossesPlanesMask != FVIEWPORT_PLANESMASK_NONE) || (m_nFlags & FMESHINST_FLAG_FORCE_CLIP) )
	{
		fgc_SetClipMode( TRUE );
	}
	else
	{
		fgc_SetClipMode( FALSE );
	}

	_SetCullDir_P( (FMeshCullDir_e)m_nCullDirection );
	
	// Setup any material specific depth bias
	fsh_SetDepthBias( pMaterial->nDepthBiasLevel );

	u32 nTotalPasses = fshadow_GetNumShadowPasses(m_pRenderLights, m_nRenderLightCount, this, m_pMesh->nFlags&FMESH_FLAGS_VOLUME_MESH);
	u32 nPass;
	
	for (nPass=0; nPass < nTotalPasses; nPass++)
	{
		BOOL bDirectionalOnly = fshadow_SetupShadowPass( nPass, m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH );
			
		// Add in each display list in the material
		for ( iii = 0; iii < pGCMaterial->nDLContCount; iii++ )
		{
			FASSERT( pGCMaterial->aDLContainer );
			FGC_DLCont_t *pDLCont = &pGCMaterial->aDLContainer[iii];
			FASSERT( pDLCont );
			
			if ( !(m_nDrawnPartsMask & (1 << pDLCont->nPartID)) )
			{
				continue;
			}
			
			if ( pDLCont->nLODID != nLODIndex )
			{
				continue;
			}

			if ( bDirectionalOnly && (pDLCont->nFlags & FGCDL_FLAGS_FACING_OPP_DIR_LIGHT) )
			{
				continue;
			}

			#if FPERF_ENABLE
			FPerf_nTotalTrisShadow += pDLCont->nStripTriCount + pDLCont->nListTriCount;
			FPerf_nRawStripTriCount += pDLCont->nStripTriCount;
			FPerf_nRawListTriCount += pDLCont->nListTriCount;
			FPerf_nRawTriStripCount += pDLCont->nStripCount;
			FPerf_nRawTriListCount += pDLCont->nListCount;
			FPerf_nRawVertexCount += (pDLCont->nListTriCount<<1) + pDLCont->nListTriCount + pDLCont->nStripTriCount + 2;
			#endif			
			
			if ( bMultiMatrix )
			{
				SetGPMatrix( pMaterial, iii );
			}
			
			// Setup the vertex type
			pDLCont->Submit( this, pMaterial->nBaseSTSets + pMaterial->nLightMapSTSets );
		}
	}
}



asm void _ASMTransformVert1Matrix(	register CFMtx43A *m, register s16 *srcBase, register s16 *dstBase, register u32 count );

//
//
//
static asm void __MTGQR7( register u32 val )
{
    nofralloc;
    mtspr       GQR7, val;
    blr                    ;
}


//
//
//
static void GQRSetup7( u32 loadScale, u32 loadType, u32 storeScale, u32 storeType )
{
    u32 reg;
    reg     =   ((loadScale << 8) + loadType) << 16;
    reg     |=  (storeScale << 8) + storeType;
    __MTGQR7(reg);
}


//
//
//
static u16 _TransformSkinnedVerts( FGCMeshSkin_t *pSkin, CFMtx43A **apBoneMtxList )
{
	u32 i;
	FGCSkinPosNorm_t *pVertBuffer;
	
	u16 nStartingIdx = FGCVB_AllocateVertBuffers( pSkin->nSkinnedVerts, &pVertBuffer );
	if ( nStartingIdx == 0xffff )
	{
		return 0xffff;
	}
		
	// Transform the verts
	FGCSkinPosNorm_t *pVert = pSkin->pSkinnedVerts;
	FGCTransDesc_t *pTransDesc = pSkin->pTransDesc;
	FGCSkinPosNorm_t *pCurrVertBuffer = pVertBuffer;
	
	GQRSetup7( 6, 7, 6, 7 );
	
	// Transform all the verts with one matrix weight
	FGCTransDesc_t *pEnd = pTransDesc + pSkin->nTD1MtxCount;
	while ( pTransDesc != pEnd )
	{
/*
		for ( i = 0; i < pTransDesc->nVertCount; i++, pVert++, pCurrVertBuffer++ )
		{
			_TransformVert1Matrix( pCurrVertBuffer, pVert, apBoneMtxList[ pTransDesc->nMtxIdx[0] ] );
		}
*/
//		CFMtx43A mtxTemp;
//		mtxTemp.Set( *apBoneMtxList[ pTransDesc->nMtxIdx[0] ] );
//		mtxTemp.Transpose33();
		_ASMTransformVert1Matrix( apBoneMtxList[ pTransDesc->nMtxIdx[0] ], (s16 *)pVert, (s16 *)pCurrVertBuffer, pTransDesc->nVertCount );
		pVert += pTransDesc->nVertCount;
		pCurrVertBuffer += pTransDesc->nVertCount;
		
		pTransDesc++;
	}
	
	// Verts with more than one weight uses the skin weights array
	FGCWeights_t *pWeights = pSkin->pSkinWeights;
	
	// Transform all the verts with two matrix weights
	pEnd = pTransDesc + pSkin->nTD2MtxCount;
	while ( pTransDesc != pEnd )
	{
		for ( i = 0; i < pTransDesc->nVertCount; i++, pVert++, pWeights++, pCurrVertBuffer++ )
		{
			_TransformVert2Matrix( pCurrVertBuffer, pVert, pWeights, apBoneMtxList[ pTransDesc->nMtxIdx[0] ], apBoneMtxList[ pTransDesc->nMtxIdx[1] ] );
		}
		
		pTransDesc++;
	}
	
	// Transform all the verts with three and four matrix weights
	pEnd = pTransDesc + pSkin->nTD3or4MtxCount;
	while ( pTransDesc != pEnd )
	{
		for ( i = 0; i < pTransDesc->nVertCount; i++, pVert++, pWeights++, pCurrVertBuffer++ )
		{
			_TransformVert3or4Matrix( pCurrVertBuffer, pVert, (u8 *)pWeights, apBoneMtxList, pTransDesc->nMtxIdx, pTransDesc->nMatrixCount );
		}
		
		pTransDesc++;
	}
	
	// Flush all of the vert data from the CPU so that it is available to the GP
	DCFlushRange( pVertBuffer, sizeof( FGCSkinPosNorm_t ) * pSkin->nSkinnedVerts );
	
	return nStartingIdx;
}


//
//
//
#define SKN_GQR_VERT 7
asm void _ASMTransformVert1Matrix(  register CFMtx43A *m, register s16 *srcBase, register s16 *dstBase, register u32 count )
{
    nofralloc
#define M00_M10 fp0
#define M20_nnn fp1
#define M01_M11 fp2
#define M21_nnn fp3
#define M02_M12 fp4
#define M22_nnn fp5
#define M03_M13 fp6
#define M23_nnn fp7

// source vectors - 2 3D vectors in 3 PS registers
#define SX0_SY0 fp8
#define SZ0_SX1 fp9
#define SY1_SZ1 fp10
// Destination registers - 2 3d vectors in 4 PS registers
#define DX0_DY0 fp11
#define DZ0_nnn fp12
#define DX1_DY1 fp13
#define DZ1_nnn fp14
// temp registers for writing back values.  These registers store the final
// results from the PREVIOUS loop
#define WX0_WY0 fp15
#define WZ0_nnn fp16
#define WX1_WY1 fp17
#define WZ1_nnn fp18

    stwu    r1, -64(r1)
    stfd    fp14, 8(r1)
    stfd    fp15, 16(r1)
    addi    count, count, -1 // unrolled
    stfd    fp16, 24(r1)
    stfd    fp17, 32(r1)
    stfd    fp18, 40(r1)
    mtctr   count
    // load matrix
    psq_l   M00_M10, 0(m),0,0  
    addi    srcBase, srcBase, -4
    psq_l   M20_nnn, 8(m),1,0  
    addi    dstBase, dstBase, -2
    psq_l   M03_M13, 48(m),0,0 
    psq_lu  SX0_SY0, 4(srcBase), 0, SKN_GQR_VERT
    psq_l   M23_nnn, 56(m),1,0 
    psq_lu  SZ0_SX1, 4(srcBase), 0, SKN_GQR_VERT

    // ------------------------------UNROLLED

    // Apply first column and translation term
    ps_madds0    DX0_DY0, M00_M10, SX0_SY0, M03_M13
      psq_l   M01_M11, 16(m),0,0
    ps_madds0    DZ0_nnn, M20_nnn, SX0_SY0, M23_nnn
      psq_l   M21_nnn, 24(m),1,0   
    ps_muls1     DX1_DY1, M00_M10, SZ0_SX1  // no trans for norms
      psq_lu  SY1_SZ1,4(srcBase), 0, SKN_GQR_VERT
    ps_muls1     DZ1_nnn, M20_nnn, SZ0_SX1  // no trans for norms
      psq_l   M22_nnn, 40(m),1,0 

    // Apply second column
    ps_madds1    DX0_DY0, M01_M11, SX0_SY0, DX0_DY0
    ps_madds1    DZ0_nnn, M21_nnn, SX0_SY0, DZ0_nnn
      psq_l   M02_M12, 32(m),0,0 
    ps_madds0    DX1_DY1, M01_M11, SY1_SZ1, DX1_DY1
      psq_lu SX0_SY0, 4(srcBase), 0, SKN_GQR_VERT
    ps_madds0    DZ1_nnn, M21_nnn, SY1_SZ1, DZ1_nnn

    // Apply third column and Write final values to temp W registers
    ps_madds0    WX0_WY0, M02_M12, SZ0_SX1, DX0_DY0
    ps_madds0    WZ0_nnn, M22_nnn, SZ0_SX1, DZ0_nnn
      psq_lu SZ0_SX1, 4(srcBase), 0, SKN_GQR_VERT
    ps_madds1    WX1_WY1, M02_M12, SY1_SZ1, DX1_DY1
    ps_madds1    WZ1_nnn, M22_nnn, SY1_SZ1, DZ1_nnn
      psq_lu SY1_SZ1,4(srcBase), 0, SKN_GQR_VERT

    // -------------------------- LOOP START
_mloop:
    ps_madds0    DX0_DY0, M00_M10, SX0_SY0, M03_M13
      psq_stu     WX0_WY0, 2(dstBase), 0, SKN_GQR_VERT
    ps_madds0    DZ0_nnn, M20_nnn, SX0_SY0, M23_nnn
      psq_stu     WZ0_nnn, 4(dstBase), 1, SKN_GQR_VERT
    ps_muls1     DX1_DY1, M00_M10, SZ0_SX1
      psq_stu     WX1_WY1, 2(dstBase), 0, SKN_GQR_VERT
    ps_muls1     DZ1_nnn, M20_nnn, SZ0_SX1
      psq_stu     WZ1_nnn, 4(dstBase), 1, SKN_GQR_VERT
    ps_madds1    DX0_DY0, M01_M11, SX0_SY0, DX0_DY0
    ps_madds1    DZ0_nnn, M21_nnn, SX0_SY0, DZ0_nnn
      psq_lu SX0_SY0, 4(srcBase), 0, SKN_GQR_VERT // NEXT SX0 SY0
    ps_madds0    DX1_DY1, M01_M11, SY1_SZ1, DX1_DY1
    ps_madds0    DZ1_nnn, M21_nnn, SY1_SZ1, DZ1_nnn

    // Write final values to temp registers
    ps_madds0    WX0_WY0, M02_M12, SZ0_SX1, DX0_DY0
    ps_madds0    WZ0_nnn, M22_nnn, SZ0_SX1, DZ0_nnn
      psq_lu SZ0_SX1, 4(srcBase), 0, SKN_GQR_VERT // NEXT SZ0 SX1
    ps_madds1    WX1_WY1, M02_M12, SY1_SZ1, DX1_DY1
    ps_madds1    WZ1_nnn, M22_nnn, SY1_SZ1, DZ1_nnn
      psq_lu SY1_SZ1,4(srcBase), 0, SKN_GQR_VERT // NEXT SY1 SZ1

    bdnz+ _mloop    // -------------------------- LOOP END

    psq_stu     WX0_WY0, 2(dstBase), 0, SKN_GQR_VERT
    psq_stu     WZ0_nnn, 4(dstBase), 1, SKN_GQR_VERT
    psq_stu     WX1_WY1, 2(dstBase), 0, SKN_GQR_VERT
    psq_stu     WZ1_nnn, 4(dstBase), 1, SKN_GQR_VERT


_return:    
    lfd     fp14, 8(r1)
    lfd     fp15, 16(r1)
    lfd     fp16, 24(r1)
    lfd     fp17, 32(r1)
    lfd     fp18, 40(r1)
    addi    r1, r1, 64
    blr

#undef M00_M10
#undef M20_nnn
#undef M01_M11
#undef M21_nnn
#undef M02_M12
#undef M22_nnn
#undef M03_M13
#undef M23_nnn
#undef SX0_SY0
#undef SZ0_SX1
#undef SY1_SZ1
#undef DX0_DY0
#undef DZ0_nnn
#undef DX1_DY1
#undef DZ1_nnn
#undef WX0_WY0
#undef WZ0_nnn
#undef WX1_WY1
#undef WZ1_nnn

}

/*
//
//
//
FINLINE void _TransformVert1Matrix( FGCSkinPosNorm_t *pResult, FGCSkinPosNorm_t *pVert, CFMtx43A *pMtx1 )
{
	CFVec3A vOrigPos, vTransPos;	
	CFVec3A vOrigNorm, vTransNorm;
	CFMtx43A TempMtx;
	
	vOrigPos.x = pVert->x * (1.f/64.f);
	vOrigPos.y = pVert->y * (1.f/64.f);
	vOrigPos.z = pVert->z * (1.f/64.f);
	vOrigNorm.x = pVert->nx * (1.f/16384.f);
	vOrigNorm.y = pVert->ny * (1.f/16384.f);
	vOrigNorm.z = pVert->nz * (1.f/16384.f);
	
//	TempMtx.ReceiveInverse( *pMtx1 );
//	TempMtx.Transpose33();
	pMtx1->MulPoint( vTransPos, vOrigPos );
	pMtx1->MulDir( vTransNorm, vOrigNorm );
//	TempMtx.MulDir( vTransNorm, vOrigNorm );
	
	pResult->x = vTransPos.x * 64;
	pResult->y = vTransPos.y * 64;
	pResult->z = vTransPos.z * 64;
	pResult->nx = vTransNorm.x * 16384;
	pResult->ny = vTransNorm.y * 16384;
	pResult->nz = vTransNorm.z * 16384;
#if 0
	*pResult = *pVert;
#endif
}
*/

//
//
//
FINLINE void _TransformVert2Matrix( FGCSkinPosNorm_t *pResult, FGCSkinPosNorm_t *pVert, 
										FGCWeights_t *pWeights, CFMtx43A *pMtx1, CFMtx43A *pMtx2 )
{
	CFVec3A vOrigPos, vPosResult, vTransPos;
	CFVec3A vOrigNorm, vNormResult, vTransNorm;
	
	vOrigPos.x = pVert->x * (1.f/64.f);
	vOrigPos.y = pVert->y * (1.f/64.f);
	vOrigPos.z = pVert->z * (1.f/64.f);
	vOrigNorm.x = pVert->nx * (1.f/16384.f);
	vOrigNorm.y = pVert->ny * (1.f/16384.f);
	vOrigNorm.z = pVert->nz * (1.f/16384.f);
	
	pMtx1->MulPoint( vTransPos, vOrigPos );
	pMtx1->MulDir( vTransNorm, vOrigNorm );
	
	vPosResult.Set( vTransPos.Mul( pWeights->w0 * (1.f/255.f) ) );
	vNormResult.Set( vTransNorm.Mul( pWeights->w0 * (1.f/255.f) ) );
	
	pMtx2->MulPoint( vTransPos, vOrigPos );
	pMtx2->MulDir( vTransNorm, vOrigNorm );
	
	vPosResult.Add( vTransPos.Mul( pWeights->w1 * (1.f/255.f) ) );
	vNormResult.Add( vTransNorm.Mul( pWeights->w1 * (1.f/255.f) ) );
	
	pResult->x = vPosResult.x * 64;
	pResult->y = vPosResult.y * 64;
	pResult->z = vPosResult.z * 64;
	pResult->nx = vNormResult.x * 16384;
	pResult->ny = vNormResult.y * 16384;
	pResult->nz = vNormResult.z * 16384;
#if 0
	*pResult = *pVert;
#endif
}


//
//
//
FINLINE void _TransformVert3or4Matrix( FGCSkinPosNorm_t *pResult, FGCSkinPosNorm_t *pVert, 
										u8 *pWeights, CFMtx43A **apBoneMtxList, u8 *paMatrixIdx, u32 nWeights )
{
	CFVec3A vOrigPos, vPosResult, vTransPos;
	CFVec3A vOrigNorm, vNormResult, vTransNorm;
	
	vOrigPos.x = pVert->x * (1.f/64.f);
	vOrigPos.y = pVert->y * (1.f/64.f);
	vOrigPos.z = pVert->z * (1.f/64.f);
	vOrigNorm.x = pVert->nx * (1.f/16384.f);
	vOrigNorm.y = pVert->ny * (1.f/16384.f);
	vOrigNorm.z = pVert->nz * (1.f/16384.f);
	
	apBoneMtxList[*paMatrixIdx]->MulPoint( vTransPos, vOrigPos );
	apBoneMtxList[*paMatrixIdx]->MulDir( vTransNorm, vOrigNorm );
	paMatrixIdx++;
	
	vPosResult.Set( vTransPos.Mul( (*pWeights) * (1.f/255.f) ) );
	vNormResult.Set( vTransNorm.Mul( (*pWeights) * (1.f/255.f) ) );
	pWeights++;

	// We already calculated one weight, so we pre-decrement
	while ( --nWeights > 0 )
	{
		FASSERT( *paMatrixIdx != 255 );
		apBoneMtxList[*paMatrixIdx]->MulPoint( vTransPos, vOrigPos );
		apBoneMtxList[*paMatrixIdx]->MulDir( vTransNorm, vOrigNorm );
		paMatrixIdx++;
		
		vPosResult.Add( vTransPos.Mul( (*pWeights) * (1.f/255.f) ) );
		vNormResult.Add( vTransNorm.Mul( (*pWeights) * (1.f/255.f) ) );
		pWeights++;
	}
	
	pResult->x = vPosResult.x * 64;
	pResult->y = vPosResult.y * 64;
	pResult->z = vPosResult.z * 64;
	pResult->nx = vNormResult.x * 16384;
	pResult->ny = vNormResult.y * 16384;
	pResult->nz = vNormResult.z * 16384;
#if 0
	*pResult = *pVert;
#endif
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

			FMesh_AmbientMotif.nMotifIndex = 0;
			FMesh_AmbientMotif.fAlpha = 1.0f;
			fmesh_Ambient_Set( 1.0f, 1.0f, 1.0f, 1.0f );

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


//
//
//
static BOOL _ResLoadCreate( FResHandle_t hRes, void *pLoadedBase, u32 nLoadedBytes, cchar *pszResName ) 
{
	FMesh_t *pMesh;

	// Create the runtime mesh data...
	pMesh = (FMesh_t *)fgcload_Create( (FMesh_t *)pLoadedBase, pszResName );
	if ( pMesh == NULL ) 
	{
		return FALSE;
	}

	// Mesh created successfully...

	// Flush the mesh data from the cache so the the GP can access it
	DCFlushRange( pLoadedBase, nLoadedBytes );

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
	fGCvid_Flush( TRUE );
}


//
//
//
void fmesh_Renderer_Open( void )
{
	fgc_SetZMode( GX_ENABLE, GX_LEQUAL, GX_ENABLE );
	fmesh_SetCullDir( FMESH_CULLDIR_CW );
	fgc_SetClipMode( GX_CLIP_ENABLE );
	
	u32 i;
	for ( i = 0; i < 8; i++ )
	{
		fgc_SetVtxSTFormat( FGCDATA_VARIABLE_VERTEX_FORMAT, GXAttr(GX_VA_TEX0 + i), GX_S16, 8 );
	}
	
	fsh_Open();
}


//
//
//
void fmesh_Renderer_Close( void ) 
{
	fsh_Close();
}


