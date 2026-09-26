//////////////////////////////////////////////////////////////////////////////////////
// MLMaterial_GC.cpp - Classes used to convert generic mesh data into Fang GC specific data
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
// 03/17/02 Lafleur		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "stdafx.h"

#include "MLMesh_GC.h"
#include "MLMaterial_GC.h"

#include "gc\fGCDisplayList.h"

// From NVidia stripping library
#include "NvTriStrip.h"


//////////////////////////////////////////////////////////////////////////////////////
// Local defines:
//////////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////////
// Global variables:
//////////////////////////////////////////////////////////////////////////////////////

extern u32 GCMesh_nVertsRendered;
extern u32 GCMesh_nMatrixSubmissions;
extern u32 GCMesh_nDLChanges;
extern u32 GCMesh_nGDBeginCalls;


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//	Returns the number of new DL's created
//
u32 MLMaterial_GC::OptimizeDLs( void )
{
	u32 nDLCountAtStart = m_nDLContainerCount;

	// Check to see if there is a prevalence of color in a DL that
	// is currently not constant color.  If there is, then consider
	// splitting the DL in two.
/*
	LinkNode *pNode, *pNode2;

	MLDLContainer *pDL = m_pFirstDLContainer;
	while ( pDL )
	{
		u32 nNodeCounter = 0;
		if ( pDL->bUseConstantColor || pDL->nVertexCount < 250 )
		{
			pDL = pDL->pNextDLContainer;
			continue;
		}

		pNode = pDL->pFirstDrawLink;
		while ( pNode )
		{
			u32 nTotalTheSame = 0;
			if ( pNode->pTris->m_bConstantColor )
			{
				pNode2 = pDL->pFirstDrawLink;
				while ( pNode2 )
				{
					if ( pNode2->pTris->m_bConstantColor 
						&& pNode2->pTris->m_ConstantColor == pNode->pTris->m_ConstantColor )
					{
						nTotalTheSame++;
					}

					pNode2 = pNode2->pNext;
				}

			}

			// If this assert happens, then we have a problem adding tris - The 
			// constant color is not being calculated correctly:
			FASSERT( nTotalTheSame != pDL->nDrawLinkCount );

			if ( (f32)nTotalTheSame / (f32)pDL->nDrawLinkCount > 0.9f )
			{
				// 90% of the tri containers have the same color, so split it

				// Allocate a new DL container
				MLDLContainer *pNewDL = (MLDLContainer *)fang_Malloc( sizeof( MLDLContainer ), 4 );

				// Fill in the DL container data
				pNewDL->bUseConstantColor = FALSE;
				pNewDL->nVBKey = pDL->nVBKey;
				pNewDL->pFirstDrawLink = NULL;
				pNewDL->pNextDLContainer = NULL;
				pNewDL->nDrawLinkCount = 0;
				pNewDL->nVertexCount = 0;
				m_pLastDLContainer->pNextDLContainer = pNewDL;
				m_pLastDLContainer = pNewDL;
				m_nDLContainerCount++;

				pNode2 = pDL->pFirstDrawLink;
				LinkNode *pLast = NULL;
				LinkNode *pNext = NULL;
				LinkNode *pEndOfNewDL = NULL;
				while ( pNode2 )
				{
					pNext = pNode2->pNext;

					if ( !pNode2->pTris->m_bConstantColor 
						|| pNode2->pTris->m_ConstantColor != pNode->pTris->m_ConstantColor )
					{
						// If this node has one before it in the list, make sure we seal the list
						if ( pLast )
						{
							pLast->pNext = pNode2->pNext;
						}

						// If we're removing the first draw link, we need to fix the pointer
						if ( pDL->pFirstDrawLink == pNode2 )
						{
							pDL->pFirstDrawLink = pNode2->pNext;
						}

						// Initialize the nodes pointers
						pNode2->pNext = NULL;
						pNode2->pOwner = pNewDL;

						// If this is the first node, point to it
						if ( !pNewDL->pFirstDrawLink )
						{
							pNewDL->pFirstDrawLink = pNode2;
							pNewDL->bUseConstantColor = pNode2->pTris->m_bConstantColor;
							pNewDL->ConstantColor = pNode2->pTris->m_ConstantColor;
						}
						else
						{
							// Otherwise, add it to the end of the list
							pEndOfNewDL->pNext = pNode2;
							if ( pNewDL->bUseConstantColor && (!pNewDL->bUseConstantColor
								|| pNewDL->ConstantColor != pNode2->pTris->m_ConstantColor) )
							{
								pNewDL->bUseConstantColor = FALSE;
							}
						}

						// Keep track of the last node so that we can easily add more nodes
						pEndOfNewDL = pNode2;

						// Increment the counters in the new DL and decrement the old one
						pNewDL->nDrawLinkCount++;
						pNewDL->nVertexCount += pNode2->pTris->m_nVertCount;
						pDL->nVertexCount -= pNode2->pTris->m_nVertCount;
						pDL->nDrawLinkCount--;

						// Don't change pLast since pLast is the same when we remove a node!!
					}
					else
					{
						// We're keeping this node, so make it the last node visited in the old list
						pLast = pNode2;
					}

					pNode2 = pNext;
				}

				FASSERT( pNewDL->pFirstDrawLink );

#if FANG_DEBUG_BUILD
				// Verify that we didn't end up with a node in both lists
				LinkNode *pTestNode = pDL->pFirstDrawLink;
				while ( pTestNode )
				{
					LinkNode *pTestNode2 = pNewDL->pFirstDrawLink;
					while ( pTestNode2 )
					{
						FASSERT( pTestNode != pTestNode2 );
						pTestNode2 = pTestNode2->pNext;
					}
					pTestNode = pTestNode->pNext;
				}
#endif // FANG_DEBUG_BUILD

				// Our DL is now constant color
				pDL->bUseConstantColor = TRUE;
				pDL->ConstantColor = pNode->pTris->m_ConstantColor;

				break;
			}

			if ( (f32)nNodeCounter / (f32)pDL->nDrawLinkCount > 0.15f )
			{
				// If we've already made it through more than 15% of the nodes
				// and we haven't found a color shared by 90%, then we won't
				break;
			}

			pNode = pNode->pNext;
		}

		// Check the next DL
		pDL = pDL->pNextDLContainer;
	}
*/
	// Return the delta in display lists
	return m_nDLContainerCount - nDLCountAtStart;
}


static struct _GCDL 
{
    u8	*pStart;
    u32	nLength;
    u8	*pPtr;
    u8	*pEnd;

	BOOL Init( void *pData, u32 nSize )
	{
		pStart  = (u8 *)pData;
		pPtr    = (u8 *)pData;
		nLength = nSize;
		pEnd    = (u8 *)pData + nSize;
		return TRUE;
	}
	BOOL Write_u8( u8 nValue )
	{
        if ( pPtr + 1 > pEnd )
		{
			return FALSE;
		}
		*pPtr++ = nValue;
		return TRUE;
	}
	BOOL Write_u16( u16 nValue )
	{
        if ( pPtr + 2 > pEnd )
		{
			return FALSE;
		}
		*pPtr++ = (u8)(nValue >> 8);
		*pPtr++ = nValue & 0xff;
		return TRUE;
	}
	BOOL PadTo32B( void )
	{
		u32 i, nPad = FMATH_BYTE_ALIGN_UP( (u32)pPtr, 32 ) - (u32)pPtr;
        if ( pPtr + nPad > pEnd )
		{
			return FALSE;
		}
		for ( i = 0; i < nPad; i++ )
		{
			*pPtr++ = 0x00;
		}
		return TRUE;
	}

} _GCDisplayList;

//
//
//
u32 MLMaterial_GC::GenerateDisplayList( u32 nDLContOffset, u8 **pDLContBuffer, u8 **pData, u32 *pDataOffset, 
									u32 *pDataBytesLeft, u32 *pDLCount, FGCVB_t *paVB )
{
	u32 i;
	if ( !m_pFirstDLContainer )
	{
		m_FGCMaterial.nDLContCount = 0;
		m_FGCMaterial.aDLContainer = (FGC_DLCont_t *)0xffffffff;
		return ML_NO_ERROR;
	}

	// Set the offset for the DL array relative to the start.
	// We need to fix this up before packing the Materials so
	// that it points to the offset in the overall mesh mem
	m_FGCMaterial.aDLContainer = (FGC_DLCont_t *)nDLContOffset;

	// Render the tris into the display list
	u32 nCurrentVBKey = 0;
	FGC_DLCont_t *pDLCont = NULL;
	MLDLContainer *pDL = m_pFirstDLContainer;
	u16 nBaseSTSets = 0;
	u16 nLightmapSTSets = 0;
	while ( pDL )
	{
/*
		LinkNode *pNode = pDL->pFirstDrawLink;
		if ( !pNode )
		{
			FASSERT_NOW;
			pDL = pDL->pNextDLContainer;
			continue;
		}

		MLTriContainer *pTriCont = pNode->pTris;
*/
		MLTriContainer *pTriCont = pDL->pTris;

		// Create the new display list container
		m_FGCMaterial.nDLContCount++;
		(*pDLCount)++;

		// Clear out the DL
		pDLCont = (FGC_DLCont_t *)*pDLContBuffer;
		memset( pDLCont, 0, sizeof( FGC_DLCont_t ) );

		// The current offset will tell us where the DL data is in the
		// buffer based on the offset from the beginning of the data
		// for this mesh.  We will have to fix up this pointer once
		// the DL data is added to the export data.
		pDLCont->pBuffer = (void *)(*pDataOffset);
		if ( pDL->bUseConstantColor )
		{
			pDLCont->nFlags |= FGCDL_FLAGS_CONSTANT_COLOR;
			pDLCont->ConstantColor.r = (u8)(pDL->ConstantColor.fRed * 255.f);
			pDLCont->ConstantColor.g = (u8)(pDL->ConstantColor.fGreen * 255.f);
			pDLCont->ConstantColor.b = (u8)(pDL->ConstantColor.fBlue * 255.f);
			pDLCont->ConstantColor.a = (u8)(pDL->ConstantColor.fAlpha * 255.f);
		}

		// Set the vertex buffer index for the display list.  It must
		// be the same for all tri containers in the display list
		FASSERT( pTriCont->m_nVBIndex != -1 );
		pDLCont->nVBIndex = (u8)pTriCont->m_nVBIndex;
		pDLCont->nMatrixIdx = (u8)pTriCont->m_nMatrixIdx;
		pDLCont->nPartID = (u8)pTriCont->m_nPartID;
		pDLCont->nLODID = (u8)pTriCont->m_nLODID;
		pDLCont->nListTriCount = 0;
		pDLCont->nStripTriCount = 0;
		pDLCont->nStripCount = 0;
		pDLCont->nListCount = 0;
		nBaseSTSets = pTriCont->m_nBaseSTSets;
		nLightmapSTSets = pTriCont->m_nLightMapSTSets;

		// If this vert container contains skinned verts, flag the display list as such
		if ( pTriCont->m_nMtxWeightCount > 1 )
		{
			pDLCont->nFlags |= FGCDL_FLAGS_SKINNED;
		}

		if ( m_bUseNBT )
		{
			pDLCont->nFlags |= FGCDL_FLAGS_BUMPMAP;
		}

		if ( !pTriCont->m_bFacingDirectionalLight )
		{
			pDLCont->nFlags |= FGCDL_FLAGS_FACING_OPP_DIR_LIGHT;
		}

//		GDInitGDLObj( &GDDisplayList, *pData, RoundDownTo32B( *pDataBytesLeft ) );
		_GCDisplayList.Init( *pData, FMATH_BYTE_ALIGN_DOWN( *pDataBytesLeft, 32 ) );
//		dl->ptr    = (u8*) pData;
//		dl->top    = (u8*) pData + RoundDownTo32B( *pDataBytesLeft );
//		dl->length = RoundDownTo32B( *pDataBytesLeft );
//		GDSetCurrent( &GDDisplayList );
//		Sets internal pointer in GD lib

		nCurrentVBKey = pTriCont->m_nVBKey;

		GCMesh_nDLChanges++;

		u32 ii;
		GXVtxFmt nVtxFmt = (GXVtxFmt)FGCDATA_VARIABLE_VERTEX_FORMAT;
		if ( !m_bUseNBT && pTriCont->m_nMtxWeightCount < 2 )
		{
			switch( pTriCont->m_nPosType )
			{
				case GX_F32:
					for( ii = 0; ii < FGCDATA_FIXED_VERTEX_FORMATS; ii++ )
					{
						if ( FGCData_VertexFormatDesc[ii].nPosBitDepth == 32 )
						{
							nVtxFmt = (GXVtxFmt)ii;
							break;
						}
					}
					break;

				case GX_S16:
					for( ii = 0; ii < FGCDATA_FIXED_VERTEX_FORMATS; ii++ )
					{
						if (	FGCData_VertexFormatDesc[ii].nPosBitDepth == 16
							&&  FGCData_VertexFormatDesc[ii].nPosFracBits == pTriCont->m_nPosFrac )
						{
							nVtxFmt = (GXVtxFmt)ii;
							break;
						}
					}
					break;

				case GX_S8:
					for( ii = 0; ii < FGCDATA_FIXED_VERTEX_FORMATS; ii++ )
					{
						if (	FGCData_VertexFormatDesc[ii].nPosBitDepth == 8
							&&  FGCData_VertexFormatDesc[ii].nPosFracBits == pTriCont->m_nPosFrac )
						{
							nVtxFmt = (GXVtxFmt)ii;
							break;
						}
					}
					break;

				default:
					break;
			}
		}
			
		// Sanity Checks
		FASSERT( pDLCont );
		FASSERT( pDL->paPrimGroups );
		FASSERT( pTriCont->m_pVertAbstr );
		FASSERT( pTriCont->m_bFacingDirectionalLight == pDL->bFacingDirLight );
		FASSERT( pTriCont->m_nVBIndex == (s32)pDLCont->nVBIndex );
		FASSERT( pTriCont->m_nMatrixIdx == pDLCont->nMatrixIdx );
		FASSERT( pTriCont->m_nBaseSTSets == nBaseSTSets );
		FASSERT( pTriCont->m_nLightMapSTSets == nLightmapSTSets );
		FASSERT( pTriCont->m_nBaseSTSets == m_nBaseSTSets );
		FASSERT( pTriCont->m_nLightMapSTSets == m_nLightMapSTSets );
		FASSERT( pTriCont->m_nVBKey == nCurrentVBKey );
		FASSERT( !!(pDLCont->nFlags & FGCDL_FLAGS_BUMPMAP) == !!(pTriCont->m_bBumpMapped) );
		FASSERT( m_bUseNBT == pTriCont->m_bBumpMapped );

		// Advance through all of the primitives rendering them to the display list
		u32 nGroup;
		for ( nGroup = 0; nGroup < pDL->nPrimGroupCount; nGroup++ )
		{
			if ( _GCDisplayList.pPtr + 3 > _GCDisplayList.pEnd )
			{
				return ML_DISPLAY_LIST_DATA_BUFFER_OVERRUN;
			}

			// Execute the GDBegin

			if ( pDL->paPrimGroups[nGroup].type == PT_STRIP )
			{
				GCMesh_nGDBeginCalls++;

				// Verts will be rendered as a strip
				if ( !_GCDisplayList.Write_u8( nVtxFmt | GX_TRIANGLESTRIP ) )
				{
					return ML_DISPLAY_LIST_DATA_BUFFER_OVERRUN;
				}
				if ( !_GCDisplayList.Write_u16( pDL->paPrimGroups[nGroup].numIndices ) )
				{
					return ML_DISPLAY_LIST_DATA_BUFFER_OVERRUN;
				}
				pDLCont->nStripTriCount = pDL->paPrimGroups[nGroup].numIndices - 2;
				pDLCont->nStripCount++;
			}
			else
			{
				GCMesh_nGDBeginCalls++;

				// Verts will be rendered as a tri-list
				if ( !_GCDisplayList.Write_u8( nVtxFmt | GX_TRIANGLES ) )
				{
					return ML_DISPLAY_LIST_DATA_BUFFER_OVERRUN;
				}
				if ( !_GCDisplayList.Write_u16( pDL->paPrimGroups[nGroup].numIndices ) )
				{
					return ML_DISPLAY_LIST_DATA_BUFFER_OVERRUN;
				}
				pDLCont->nListTriCount = pDL->paPrimGroups[nGroup].numIndices / 3;
				pDLCont->nListCount++;
			}

			// Render the vertices
			GCVertAbstr_t *pAbstr = (GCVertAbstr_t *)pTriCont->m_pVertAbstr;

			// Submit all of the vert indices
			for ( i = 0; i < pDL->paPrimGroups[nGroup].numIndices; i++ )
			{
				if ( !RenderVertex( &pAbstr[pDL->paPrimGroups[nGroup].indices[i]], pTriCont, &paVB[pTriCont->m_nVBIndex], pDLCont ) )
				{
					return ML_DISPLAY_LIST_DATA_BUFFER_OVERRUN;
				}
			}

			GCMesh_nVertsRendered += pTriCont->m_nVertCount;
		}

		// Close the current DL
		FASSERT( pDLCont );

		// Pad the DL to a 32 byte boundary
		if ( !_GCDisplayList.PadTo32B() )
		{
			return ML_DISPLAY_LIST_DATA_BUFFER_OVERRUN;
		}

		// Get the size of the display list
		pDLCont->nSize = (u32)_GCDisplayList.pPtr - (u32)_GCDisplayList.pStart;

		// Update the memory trackers
		(*pDLContBuffer)	+= sizeof( FGC_DLCont_t );
		(*pDataOffset)		+= pDLCont->nSize;
		(*pData)			+= pDLCont->nSize;
		(*pDataBytesLeft)	-= pDLCont->nSize;

		pDLCont = NULL;

		// Advance to the next Display List Container in the material
		pDL = pDL->pNextDLContainer;
	}

	return ML_NO_ERROR;
}

extern u32 GCMesh_nTotalBumpedVertsRendered;

//
//
//
BOOL MLMaterial_GC::RenderVertex( GCVertAbstr_t *pAbstr, MLTriContainer *pTriCont, FGCVB_t *pVB, FGC_DLCont_t *pDLCont )
{
	u32 ii;

	// Submit the position index
	if ( pVB->nPosIdxType == GX_INDEX8 )
	{
		if ( !_GCDisplayList.Write_u8( (u8)pAbstr->nPosIdx ) )
		{
			return FALSE;
		}
	}
	else
	{
		if ( !_GCDisplayList.Write_u16( (u16)pAbstr->nPosIdx ) )
		{
			return FALSE;
		}
	}

	// Submit the normal index
	if ( !_GCDisplayList.Write_u16( (u16)pAbstr->nNormIdx ) )
	{
		return FALSE;
	}

#if MLMESHGC_USE_NORMAL_SPHERE_FOR_STREAMING_NBTS
	// If bump mapped, submit the binormal and tangent
	if ( pTriCont->m_bBumpMapped && MLManager.m_bAccumStreamingData )
	{
		GCMesh_nTotalBumpedVertsRendered++;
		if ( !_GCDisplayList.Write_u16( (u16)pAbstr->nBinormIdx ) )
		{
			return FALSE;
		}
		if ( !_GCDisplayList.Write_u16( (u16)pAbstr->nTangIdx ) )
		{
			return FALSE;
		}
	}
#endif

	// If we don't have a constant color, send out the color index
	if ( !(pDLCont->nFlags & FGCDL_FLAGS_CONSTANT_COLOR) )
	{
		if ( pVB->nColorIdxType == GX_INDEX8 )
		{
			if ( !_GCDisplayList.Write_u8( (u8)pAbstr->nDiffuseIdx ) )
			{
				return FALSE;
			}
		}
		else
		{
			if ( !_GCDisplayList.Write_u16( (u16)pAbstr->nDiffuseIdx ) )
			{
				return FALSE;
			}
		}
	}

	for ( ii = 0; ii < (u32)(pTriCont->m_nBaseSTSets + pTriCont->m_nLightMapSTSets); ii++ )
	{
		if ( !_GCDisplayList.Write_u16( (u16)pAbstr->nSTIdx[ii] ) )
		{
			return FALSE;
		}
	}

	return TRUE;
}


//
//
//
BOOL MLMaterial_GC::BuildDLAbstractionIndices( MLDLContainer *pDL, u32 &nIndexCount )
{
	u32 i, j, nUniqueAbstrCount = 0;
	GCVertAbstr_t *pNewAbstr, *pTestAbstr;

	u16 *paUniqueIndices = (u16 *)fang_Malloc( sizeof( u16 ) * pDL->nVertexCount, 4 );
	if ( !paUniqueIndices )
	{
		return FALSE;
	}

	for ( i = 0; i < pDL->pTris->m_nVertCount; i++ )
	{
		pNewAbstr = &((GCVertAbstr_t *)pDL->pTris->m_pVertAbstr)[i];

		for ( j = 0; j < nUniqueAbstrCount; j++ )
		{
			pTestAbstr = &((GCVertAbstr_t *)pDL->pTris->m_pVertAbstr)[paUniqueIndices[j]];

			if ( memcmp( pNewAbstr, pTestAbstr, sizeof( GCVertAbstr_t ) ) == 0 )
			{
				// We found a matching vert abstraction already in the unique list, so use that index
				pDL->paAbstrIndices[nIndexCount++] = paUniqueIndices[j];
				break;
			}
		}

		if ( j == nUniqueAbstrCount )
		{
			// We didn't find a matching abstraction in the unique list, so we need to add this one
			paUniqueIndices[nUniqueAbstrCount++] = i;
			pDL->paAbstrIndices[nIndexCount++] = i;                
		}
	}

	fang_Free( paUniqueIndices );

	return TRUE;
}

