//////////////////////////////////////////////////////////////////////////////////////
// fdx8load.cpp - Fang DX module responsible for converting data fresh off the disk
//                into the engine format.
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
#include "fGCload.h"
#include "fGCmesh.h"
#include "fGCvid.h"
#include "fGCsh.h"
#include "fGCdisplaylist.h"
#include "fdata.h"
#include "fres.h"
#include "fresload.h"
#include "fmesh_coll.h"
#include "fshaders.h"
#include "fRenderSort.h"
#include "fdatastreaming.h"



#define _VBSET( nVBBucketIndex ) ( (nVBBucketIndex) & ((FDX8DATA_VB_SET_COUNT)-1) )
#define _VBTYPE( nVBBucketIndex ) ( (nVBBucketIndex) >> (FDX8DATA_VB_SET_BITS) )

#define _VB_BUCKET_COUNT	( (FDX8VB_TYPE_COUNT) << (FDX8DATA_VB_SET_BITS) )

#define _MAX_ALLOWED_VTX_PER_BUF	65536




typedef struct {		// Vtx translation:
	u16 nNewIndex;		// New index for this FDX8Data_MeshVtx_t
	s8 nVBBucketIndex;	// Destination vertex buffer bucket (-1 means this vertex's index hasn't been translated yet)
	BOOL8 bBuilt;		// TRUE=already built this vertex
} _VtxXlat_t;

typedef struct {		// Vindex translation:
	u32 nNewVindex;		// New vindex
	s8 nVBBucketIndex;	// Destination vertex buffer bucket (-1 means this vertex's index hasn't been translated yet)
} _VindexXlat_t;


static BOOL _bModuleInitialized;
static BOOL _bWindowCreated;

static BOOL _bHardwareTnL = TRUE;	// TRUE: device supports hardware TnL
static u32 _nHardwareSkinMtxCount;	// Number of skinning matrices supported in hardware
static u32 _nSinglePassTexCount;	// Number of simultaneous textures supported in a single pass
static u32 _nTexStageCount;			// Number of texture stages
static u32 _nMaxPrimCount;			// Maximum number of primitives that can be rendered in a single call to DX
static u32 _nMaxVtxIndex;			// Maximum vertex index

static FMesh_t *_pMesh;
static FGCMesh_t *_pMeshIS;

static u32 _nLoadSegCount;
static u32 _nLoadNodeCount;
static u32 _nLoadLeafCount;
static u32 _nLoadMtlCount;
static u32 _nLoadStripCount;
static u32 _nLoadTriCount;
static u32 _nLoadVindexCount;
static u32 _nLoadVtxCount;
static u32 _nLoadTcCount;
static u32 _nLoadPosCount;
static u32 _nLoadVlCount;
static u32 _nLoadVnCount;
static u32 _nLoadShTexInstCount;
static u32 _nLoadShaderNameCount;
static u32 _nLoadMotifCount;
static u32 _nLoadBoneCount;
static u32 _nLoadLightCount;


#if FANG_DEBUG_BUILD
	static BOOL _CheckTriBoundingSpheres( void );
#endif

static BOOL _WindowCreatedCallback( FGCVidEvent_e nEvent );


//
//
//
BOOL fgcload_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

	fgcvid_RegisterWindowCallbackFunction( _WindowCreatedCallback );

	_bWindowCreated = FALSE;
	_bModuleInitialized = TRUE;

	return TRUE;
}


//
//
//
void fgcload_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

	fgcvid_UnregisterWindowCallbackFunction( _WindowCreatedCallback );

	_bModuleInitialized = FALSE;
}


//
//
//
FMesh_t* fgcload_Create( FMesh_t *pLoadMesh, cchar *pszResName ) 
{
	u32 i, ii, iii;

	#define __FIXUP_POINTER( pointer, type )	if (pointer) pointer = (type *)((u32)pLoadMesh + (u32)pointer)
	
	// Fixup FMesh_t pointers:
	__FIXUP_POINTER( pLoadMesh->aSeg, FMeshSeg_t );
	__FIXUP_POINTER( pLoadMesh->aMtl, FMeshMaterial_t );
	__FIXUP_POINTER( pLoadMesh->pBoneArray, FMeshBone_t );
	__FIXUP_POINTER( pLoadMesh->pnSkeletonIndexArray, u8 );
	__FIXUP_POINTER( pLoadMesh->pLightArray, FMeshLight_t );
	__FIXUP_POINTER( pLoadMesh->pTexLayerIDArray, FMeshTexLayerID_t );
	__FIXUP_POINTER( pLoadMesh->pMeshIS, FGCMesh_t );

	FGCMesh_t *pGCMesh = pLoadMesh->pMeshIS;
	
	// Track the usage of scrolling and flipping
	pLoadMesh->nTexLayerIDCount_ST = 0;
	pLoadMesh->nTexLayerIDCount_Flip = 0;
	for ( i = 0; i < pLoadMesh->nTexLayerIDCount; i++ ) 
	{
		if ( pLoadMesh->pTexLayerIDArray[i].nFlags & FMESH_TEXLAYERIDFLAG_USE_ST_INFO ) 
		{
			pLoadMesh->nTexLayerIDCount_ST++;
		}
			
		if ( pLoadMesh->pTexLayerIDArray[i].nFlags & FMESH_TEXLAYERIDFLAG_USE_FLIP_INFO ) 
		{
			pLoadMesh->nTexLayerIDCount_Flip++;
		}
	}
	
	for ( i = 0; i < pLoadMesh->nMaterialCount; i++ ) 
	{
		__FIXUP_POINTER( pLoadMesh->aMtl[i].pPlatformData, void );
		FGCMeshMaterial_t *pGCMat = (FGCMeshMaterial_t *)pLoadMesh->aMtl[i].pPlatformData;
		if ( !pGCMat )
		{
			continue;
		}
		__FIXUP_POINTER( pGCMat->aDLContainer, FGC_DLCont_t );
		
		for ( ii = 0; ii < pGCMat->nDLContCount; ii++ )
		{
			FASSERT( pGCMat->aDLContainer );
			FGC_DLCont_t *pDLCont = &pGCMat->aDLContainer[ii];
			if ( pDLCont->nFlags & FGCDL_FLAGS_STREAMING )
			{
				pDLCont->pBuffer = FDS_StreamMgr.GetDataHandle( (u32)pDLCont->pBuffer, pDLCont->nSize, FDS_LOCATION_ARAM );
			}
			else
			{
				__FIXUP_POINTER( pDLCont->pBuffer, void );
			}
		}
		
		__FIXUP_POINTER( pLoadMesh->aMtl[i].pnShLightRegisters, u32 );
		__FIXUP_POINTER( pLoadMesh->aMtl[i].pnShSurfaceRegisters, u32 );
		
		fshaders_FixupLightRegisters( pLoadMesh, pLoadMesh->aMtl[i].nLightShaderIdx, pLoadMesh->aMtl[i].nSpecularShaderIdx, pLoadMesh->aMtl[i].pnShLightRegisters );
		fshaders_FixupSurfaceRegisters( pLoadMesh, pLoadMesh->aMtl[i].nSurfaceShaderIdx, pLoadMesh->aMtl[i].pnShSurfaceRegisters );
		
		pLoadMesh->aMtl[i].nDLHashKey = frs_GenerateMaterialDLHash( &pLoadMesh->aMtl[i] );
	}

	__FIXUP_POINTER( pLoadMesh->paCollTree, FkDOP_Tree_t );
	if ( pLoadMesh->paCollTree )
	{
		for ( i = 0; i < pLoadMesh->nCollTreeCount; i++ )
		{
			FkDOP_Tree_t *pCollTree = &pLoadMesh->paCollTree[i];
			
			__FIXUP_POINTER( pCollTree->pakDOPNodes, FkDOP_Node_t );
			for ( ii = 0; ii < pCollTree->nTreeNodeCount; ii++ )
			{
				FkDOP_Node_t *pNode = &pCollTree->pakDOPNodes[ii];
				
				__FIXUP_POINTER( pNode->paPackets, FkDOP_TriPacket_t );
				__FIXUP_POINTER( pNode->pTriData, void );
#if FANG_DEBUG_BUILD
				if ( pNode->paPackets )
				{
					u16 *pVertIdx = (u16 *)pNode->pTriData;
					for ( iii = 0; iii < pNode->nTriPacketCount; iii++ )
					{
						u32 iv;
						u32 nStart = pNode->paPackets[iii].nStartVert;
						u32 nEnd = nStart + (pNode->paPackets[iii].nTriCount * 3);
						for ( iv = nStart; iv < nEnd; iv += 3 )
						{
							if ( pVertIdx[iv] == pVertIdx[iv+1] 
									|| pVertIdx[iv] == pVertIdx[iv+2]
									|| pVertIdx[iv+1] == pVertIdx[iv+2] )
							{
								DEVPRINTF( "FGCLOAD - Mesh %s - Bad vertex indices detected: %d, %d, %d.\n", pLoadMesh->szName, pVertIdx[iv], pVertIdx[iv+1], pVertIdx[iv+2] );
							}
						}
					}
				}
#endif // FANG_DEBUG_BUILD				
			}
			
			__FIXUP_POINTER( pCollTree->paRootkDOPVerts, CFVec3 );
			__FIXUP_POINTER( pCollTree->paIntervals, FkDOP_Interval_t );
		}
	}

	// Fixup FMesh_t::FGCMesh_t pointers:
	pGCMesh->pMesh = pLoadMesh; // Manually fixup this pointer, because it is always originally NULL
	__FIXUP_POINTER( pGCMesh->aVB, FGCVB_t );
	for ( i = 0; i < pGCMesh->nVBCount; i++ )
	{
		FGCVB_t *pTempVB = &pGCMesh->aVB[i];
		__FIXUP_POINTER( pTempVB->pPosition, void );
		__FIXUP_POINTER( pTempVB->pNBT, FGCNBT8_t );
		__FIXUP_POINTER( pTempVB->pDiffuse, FGCColor_t );
		__FIXUP_POINTER( pTempVB->pST, FGCST16_t );
	}
	
	__FIXUP_POINTER( pGCMesh->pMeshSkin, FGCMeshSkin_t );
	if ( pGCMesh->pMeshSkin )
	{
		__FIXUP_POINTER( pGCMesh->pMeshSkin->pTransDesc, FGCTransDesc_t );
		__FIXUP_POINTER( pGCMesh->pMeshSkin->pSkinnedVerts, FGCSkinPosNorm_t );
		__FIXUP_POINTER( pGCMesh->pMeshSkin->pSkinWeights, FGCWeights_t );
	}

	#undef __FIXUP_POINTER
	
	// Success!
	return pLoadMesh;
}


//
//
//
void fgcload_Destroy( FMesh_t *pMesh ) 
{
	FGCMesh_t *pMeshIS;

	FASSERT( pMesh );

	pMeshIS = pMesh->pMeshIS;
	if( pMeshIS == NULL ) 
	{
		return;
	}

	FASSERT_NOW;
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

