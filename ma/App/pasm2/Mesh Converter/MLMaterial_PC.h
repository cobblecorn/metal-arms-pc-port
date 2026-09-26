//////////////////////////////////////////////////////////////////////////////////////
// MLMaterial_PC.h - Classes used to convert generic mesh data into Fang PC specific data
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

#ifndef __MLMATERIAL_PC_H_
#define __MLMATERIAL_PC_H_

#include "fang.h"
#include "fshaders.h"
#include "dx\fdx8mesh.h"
#include "MLMaterial.h"


// Forward class declarations
class MLMaterial;
class MLMesh;



//
//
//
class MLMaterial_PC : public MLMaterial
{
	friend class MLSegment;
	friend class MLMesh;

	public:
		FDX8MeshMaterial_t	m_FPCMaterial;

		MLMaterial_PC( void )
		{
			m_FPCMaterial.aCluster = NULL;
			m_FPCMaterial.nClusterCount = 0;
		}

	protected:
		BOOL BuildDLAbstractionIndices( MLDLContainer *pDL, u32 &nIndexCount );
};


#endif