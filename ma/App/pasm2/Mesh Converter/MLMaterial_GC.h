//////////////////////////////////////////////////////////////////////////////////////
// MLMaterial_GC.h - Classes used to convert generic mesh data into Fang gc specific data
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
// 03/17/02 Lafleur		Created as GCMaterial.
// 08/15/02 Lafleur		Genericized from GC code to support multi-platform.  Changed name to MLMaterial
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __MLMATERIAL_GC_H_
#define __MLMATERIAL_GC_H_

#include "fang.h"
#include "fshaders.h"
#include "MLMaterial.h"
#include "gc\fGCmesh.h"



// Forward class declarations
class MLMaterial;
class MLMesh;
struct GCVertAbstr_t;


//
//
//
class MLMaterial_GC : public MLMaterial
{
	public:	// Public members

		FGCMeshMaterial_t	m_FGCMaterial;

		BOOL			m_bUseNBT;

	public:	// Public Methods

		MLMaterial_GC( void )
		{
			m_FGCMaterial.nDLContCount = 0;
			m_FGCMaterial.aDLContainer = NULL;
			m_bUseNBT = FALSE;
		}

		u32  OptimizeDLs( void );
		BOOL RenderVertex( GCVertAbstr_t *pAbstr, MLTriContainer *pTriCont, FGCVB_t *pVB, FGC_DLCont_t *pDLCont );
		u32  GenerateDisplayList( u32 nDLContOffset, u8 **pDLCont, u8 **pData, u32 *pDataOffset, 
									u32 *pDataBytesLeft, u32 *pDLCount, FGCVB_t *pVB );
	protected:
		BOOL BuildDLAbstractionIndices( MLDLContainer *pDL, u32 &nIndexCount );
};


#endif