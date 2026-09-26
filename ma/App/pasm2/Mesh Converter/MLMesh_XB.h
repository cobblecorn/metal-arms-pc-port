//////////////////////////////////////////////////////////////////////////////////////
// MLMesh_XB.h - Classes used to convert generic mesh data into Fang XB specific data
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
// 08/21/02 Lafleur		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __MLMESH_XB_H_
#define __MLMESH_XB_H_

#include "fang.h"
#include "dx\fdx8mesh.h"

#include "ML.h"
#include "MLMesh.h"
#include "MLMaterial_XB.h"


// This structure is used as an abstraction layer between the
// indices to be rendered by a strip and the actual indices into the 
// respective vertex buffer attribute arrays.  Given a particular 
// vert index, we can get attribute indices for that vert.
typedef struct XBVertAbstr_s
{
	u16 nVBIdx;
} XBVertAbstr_t;


//
//
//
class MLMesh_XB	: public MLMesh
{
	protected:

	protected:
		// XBMesh construction
		MLResults* GenerateExportData( BOOL bGenerateCollisionData );

		//
		//
		void FreeScratchMemory( void )
		{
			MLMesh::FreeScratchMemory();
		}

	public:
		//
		//
		MLMesh_XB( char *pszName, BOOL bWorldGeo ) : MLMesh( pszName, bWorldGeo )
		{
		}

		//
		//
		~MLMesh_XB( void )
		{
		}

		//
		//
		u16 AllocateMLMaterial( KongMat_t *pKMat )
		{
			MLMaterial_XB *pNewMaterial = new MLMaterial_XB;
			if ( !pNewMaterial )
			{
				return 0xffff;
			}

			u16 nIndex = 0;

			// Add the new material to the mesh and return it
			if ( !AddMaterial( pNewMaterial, pKMat, nIndex ) )
			{
				delete pNewMaterial;
				return nIndex;
			}

			pNewMaterial->Set( this, pKMat );

			return nIndex;
		}
};	


#endif
