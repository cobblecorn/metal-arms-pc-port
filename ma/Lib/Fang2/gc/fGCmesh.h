//////////////////////////////////////////////////////////////////////////////////////
// fGCmesh.h - Fang mesh module (GameCube version).
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

#ifndef _FGCMESH_H_
#define _FGCMESH_H_ 1

#include "fang.h"
#include "fGCvb.h"
#include "fGCdata.h"

#include "fmesh.h"
#include "ftex.h"
#include "fdata.h"
#include "fkdop.h"


struct FGC_DLCont_t;



//
//
typedef struct 
{
	FGC_DLCont_t *aDLContainer;			// Array of display list containers for this material
	u16 nDLContCount;					// Number of display list containers used
	
	void ChangeEndian( void )
	{
		aDLContainer = (FGC_DLCont_t *)fang_ConvertEndian( aDLContainer );
		nDLContCount = fang_ConvertEndian( nDLContCount );
	}

} FGCMeshMaterial_t;


//
//
typedef struct 
{
	u16 			nTransDescCount;	// Number of skin translations
	u16 			nTD1MtxCount;		// Number of verts weighted to 1 matrix
	u16 			nTD2MtxCount;		// Number of verts weighted to 2 matrices
	u16 			nTD3or4MtxCount;	// Number of verts weighted to 3 or 4 matrices
	FGCTransDesc_t	*pTransDesc;		// Pointer to the array of skin translations descriptions
	u32 			nSkinnedVerts;		// Number of skinned Verts
	FGCSkinPosNorm_t*pSkinnedVerts;		// Pointer to the array of skinned verts
	FGCWeights_t	*pSkinWeights;		// Pointer to the array of weights (one to one correspondence with position)
	
																//   Indices are into FMesh_t::pTexLayerIDArray[]
	void ChangeEndian( void )
	{
		nTD1MtxCount = fang_ConvertEndian( nTD1MtxCount );
		nTD2MtxCount = fang_ConvertEndian( nTD2MtxCount );
		nTD3or4MtxCount = fang_ConvertEndian( nTD3or4MtxCount );
		nTransDescCount = fang_ConvertEndian( nTransDescCount );
		pTransDesc = (FGCTransDesc_t *)fang_ConvertEndian( pTransDesc );
		nSkinnedVerts = fang_ConvertEndian( nSkinnedVerts );
		pSkinnedVerts = (FGCSkinPosNorm_t *)fang_ConvertEndian( pSkinnedVerts );
		pSkinWeights = (FGCWeights_t *)fang_ConvertEndian( pSkinWeights );
	}

} FGCMeshSkin_t;


//-------------------------------------------------------------------------------------------------------------------
// General Mesh Definitions:
//-------------------------------------------------------------------------------------------------------------------


typedef struct FGCMesh_s 
{
	FMesh_t 	*pMesh;					// Pointer to platform-independent base object

	CFSphere 	AtRestBoundSphere_MS;	// Used only when nSegCount is 0

	u8 			nFlags;					// See FGCMESH_FLAG_* for info
	u8 			nVBCount;				// Number of vertex buffers used by this mesh
	u16			nMtlCount;				// Number of materials in this node

	FGCVB_t 		*aVB;				// Array of vertex buffer descriptors
	FGCMeshSkin_t	*pMeshSkin;			// Pointer to the mesh skin, if there is one
	
	void ChangeEndian( void )
	{
		pMesh = (FMesh_t *)fang_ConvertEndian( pMesh );
		
		AtRestBoundSphere_MS.ChangeEndian();
		
		nFlags = fang_ConvertEndian( nFlags );
		nVBCount = fang_ConvertEndian( nVBCount );
		nMtlCount = fang_ConvertEndian( nMtlCount );
		
		aVB = (FGCVB_t *)fang_ConvertEndian( aVB );
		pMeshSkin = (FGCMeshSkin_t *)fang_ConvertEndian( pMeshSkin );
	}
	
} FGCMesh_t;



extern BOOL fgcmesh_ModuleStartup( void );
extern void fgcmesh_ModuleShutdown( void );

//===================================================================================================================


#endif

