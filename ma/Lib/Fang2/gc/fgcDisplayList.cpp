//////////////////////////////////////////////////////////////////////////////////////
// fGCdisplaylist.h - Fang GameCube Display List module.
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
// 03/19/02	Lafleur		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "Fang.h"
#include "FGCmesh.h"
#include "fgcdisplaylist.h"
#include "fdatastreaming.h"

u32	FGCDL_nFastDLCalls = 0;
u32	FGCDL_nFlushDLCalls = 0;
u8  FGCDL_ActiveTextures = 0;



//
//
//
void FGC_DLCont_t::Submit( CFMeshInst *pMeshInst, u16 nSTSets, BOOL bActivateChannel2 )
{
	FASSERT( pMeshInst && pMeshInst->m_pMesh && pMeshInst->m_pMesh->pMeshIS );
	
	u32 i;
	
	FGCVB_t *pVB = &pMeshInst->m_pMesh->pMeshIS->aVB[ nVBIndex ];
	
	void *pDLBuffer;
	if ( nFlags & FGCDL_FLAGS_STREAMING )
	{
		pDLBuffer = FDS_StreamMgr.AccessData( pBuffer );
	}
	else
	{
		pDLBuffer = pBuffer;
	}
	
	if ( !pDLBuffer )
	{
		return;
	}
		
	// Sanity Checks
	FASSERT( (pVB->nPosIdxType == GX_INDEX8 && pVB->nPosCount < 256) || pVB->nPosIdxType == GX_INDEX16 );
	FASSERT( (pVB->nColorIdxType == GX_INDEX8 && pVB->nDiffuseCount < 256) || pVB->nColorIdxType == GX_INDEX16 );
	FASSERT( pVB->pPosition );
	FASSERT(   (pVB->nPosType == GX_F32 && pVB->nPosStride == 12)
			|| (pVB->nPosType == GX_S16 && pVB->nPosStride == 6 )
			|| (pVB->nPosType == GX_S8  && pVB->nPosStride == 3 )
			|| (pVB->nPosType == GX_S16 && pVB->nPosStride == 12 && (nFlags & FGCDL_FLAGS_SKINNED) ));

	// Setup Position and Normal
	if ( !(nFlags & FGCDL_FLAGS_SKINNED) )
	{
		if ( !(nFlags & FGCDL_FLAGS_BUMPMAP) )
		{
			FASSERT( !pVB->pNBT );
			FASSERT( !(pVB->nFlags & FGCVB_NORM_NBT) );
			FASSERT( !(nFlags & FGCDL_FLAGS_BUMPMAP) );
			
			fgc_SetVtxPosDesc( (GXAttrType)pVB->nPosIdxType );
			fgc_SetVtxNrmDesc( GX_INDEX16 );
			
			GXSetArray( GX_VA_POS, pVB->pPosition, pVB->nPosStride );
			GXSetArray( GX_VA_NRM, FMesh_avCNormalSphere, sizeof(FMesh_CNorm8_t) );
			
			if ( pVB->nGCVertexFormat == FGCDATA_VARIABLE_VERTEX_FORMAT )
			{
				// Set the position info
				fgc_SetVtxPosFormat( FGCDATA_VARIABLE_VERTEX_FORMAT, (GXCompType)pVB->nPosType, pVB->nPosFrac );
				
				// Set the normal info
				fgc_SetVtxNrmFormat( FGCDATA_VARIABLE_VERTEX_FORMAT, GX_S8, GX_NRM_XYZ, 6 );
			}
		}
		else
		{
			FASSERT( pVB->nFlags & FGCVB_NORM_NBT );
			FASSERT( nFlags & FGCDL_FLAGS_BUMPMAP );
			
			//bumpmap display list here.
			fgc_SetVtxPosDesc( (GXAttrType)pVB->nPosIdxType );
			fgc_SetVtxNBTDesc( GX_INDEX16 );
			
			GXSetArray( GX_VA_POS, pVB->pPosition, pVB->nPosStride );
			
			// Set the position info
			fgc_SetVtxPosFormat( FGCDATA_VARIABLE_VERTEX_FORMAT, (GXCompType)pVB->nPosType, pVB->nPosFrac );
			
			if ( pVB->pNBT )
			{
				// Set the normal info using the supplied NBT's
				GXSetArray( GX_VA_NBT, pVB->pNBT, sizeof(FGCNBT8_t) );
				fgc_SetVtxNBTFormat( FGCDATA_VARIABLE_VERTEX_FORMAT, GX_S8, GX_NRM_NBT, 6 );
			}
			else
			{
				// Set the normal info using the normal sphere
				GXSetArray( GX_VA_NBT, FMesh_avCNormalSphere, sizeof(FMesh_CNorm8_t) );
				fgc_SetVtxNBTFormat( FGCDATA_VARIABLE_VERTEX_FORMAT, GX_S8, GX_NRM_NBT3, 6 );
			}
		}
	}
	else
	{
		// Set the position info
		fgc_SetVtxPosDesc( (GXAttrType)pVB->nPosIdxType );
		fgc_SetVtxPosFormat( FGCDATA_VARIABLE_VERTEX_FORMAT, (GXCompType)pVB->nPosType, pVB->nPosFrac );
		GXSetArray( GX_VA_POS, &FGCVB_pSkinVertBuffer[FGCVB_nSkinVertCurrBuffer][pMeshInst->SkinVB_GetStartingIndex()], sizeof(FGCSkinPosNorm_t) );
		
		// Set the normal info
		fgc_SetVtxNrmDesc( GX_INDEX16 );
		fgc_SetVtxNrmFormat( FGCDATA_VARIABLE_VERTEX_FORMAT, GX_S16, GX_NRM_XYZ, 14 );
		GXSetArray( GX_VA_NRM, ((u16 *)&FGCVB_pSkinVertBuffer[FGCVB_nSkinVertCurrBuffer][pMeshInst->SkinVB_GetStartingIndex()])+3, sizeof(FGCSkinPosNorm_t) );
	}	
	
	// Setup Color
	if ( (nFlags & FGCDL_FLAGS_CONSTANT_COLOR) )
	{
		fgc_SetVtxClrDesc( GX_VA_CLR0, GX_NONE );
		fgc_SetVtxClrDesc( GX_VA_CLR1, GX_NONE );
		u32 nClr[3];
		
		nClr[0] = (FMesh_AmbientMotif.fRed * 255.f) + ConstantColor.r;
		FMATH_CLAMP(nClr[0], 0, 255);
		nClr[1] = (FMesh_AmbientMotif.fGreen * 255.f) + ConstantColor.g;
		FMATH_CLAMP(nClr[1], 0, 255);
		nClr[2] = (FMesh_AmbientMotif.fBlue * 255.f) + ConstantColor.b;
		FMATH_CLAMP(nClr[2], 0, 255);
		
		// Adjust the ambient to reflect constant vertex color
		fgcsh_Light_SetAmbient( nClr[0], nClr[1], nClr[2],
								(FMesh_AmbientMotif.fAlpha * 255.f) + ConstantColor.a );

		if ( FGC_bUseVertexColor == TRUE )
		{
			fgcsh_Light_Activate( FALSE, bActivateChannel2 );
		}
	}
	else if ( !pMeshInst->GetColorStreamCount() )
	{
		FASSERT( pVB->pDiffuse );
		
		// If we don't have a constant color index, set the color info
		fgc_SetVtxClrDesc( GX_VA_CLR0, (GXAttrType)pVB->nColorIdxType );
		fgc_SetVtxClrDesc( GX_VA_CLR1, GX_NONE );
		GXSetArray( GX_VA_CLR0, pVB->pDiffuse, 4 );
		
		fgcsh_SetAmbientLight(FMesh_AmbientMotif.fRed, FMesh_AmbientMotif.fGreen, FMesh_AmbientMotif.fBlue, 1.0f);
		
		if ( FGC_bUseVertexColor == FALSE )
		{
			fgcsh_Light_Activate( TRUE, bActivateChannel2 );
		}
	}
	else
	{
		FASSERT( pMeshInst->GetColorStreams() && ((u32 **)pMeshInst->GetColorStreams())[0] );
		
		// If we don't have a constant color index, set the color info
		fgc_SetVtxClrDesc( GX_VA_CLR0, (GXAttrType)pVB->nColorIdxType );
		fgc_SetVtxClrDesc( GX_VA_CLR1, GX_NONE );
		GXSetArray( GX_VA_CLR0, ((u32 **)pMeshInst->GetColorStreams())[0], 4 );
		
		fgcsh_SetAmbientLight(FMesh_AmbientMotif.fRed, FMesh_AmbientMotif.fGreen, FMesh_AmbientMotif.fBlue, 1.0f);
		
		if ( FGC_bUseVertexColor == FALSE )
		{
			fgcsh_Light_Activate( TRUE, bActivateChannel2 );
		}
	}
	
	// Set the ST info
	for ( i = 0; i < nSTSets; i++ )
	{
		fgc_SetVtxSTDesc( (GXAttr)(GX_VA_TEX0 + i), GX_INDEX16 );
		GXSetArray( (GXAttr)(GX_VA_TEX0 + i), pVB->pST, 2 * 2 );
	}
	
	// Clear out any remaining ST info
	for ( ; i < 8 && FGC_CurrSTIdxType[i] != GX_NONE; i++ )
	{
		fgc_SetVtxSTDesc( (GXAttr)(GX_VA_TEX0 + i), GX_NONE );
	}
	
	// Submit this display list
	if ( FGC_bLazyRegisterSet == FALSE )
	{
		FGCDL_nFastDLCalls++;
		GXFastCallDisplayList( pDLBuffer, nSize );
	}
	else
	{
		FGCDL_nFlushDLCalls++;
		GXCallDisplayList( pDLBuffer, nSize );
		FGC_bLazyRegisterSet = FALSE;
	}
}
