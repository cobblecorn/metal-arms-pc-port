//////////////////////////////////////////////////////////////////////////////////////
// MLMaterial_XB.h - Classes used to convert generic mesh data into Fang XB specific data
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
// 08/20/02 Lafleur		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __MLMATERIAL_XB_H_
#define __MLMATERIAL_XB_H_

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
class MLMaterial_XB : public MLMaterial
{
	friend class MLSegment;
	friend class MLMesh;

	public:
		FDX8MeshMaterial_t	m_FXBMaterial;

		MLMaterial_XB( void )
		{
			m_FXBMaterial.aCluster = NULL;
			m_FXBMaterial.nClusterCount = 0;
		}

		void AddTexLayerIDIndexToMaterial( u32 nTexLayerIndex );

};


#endif