//////////////////////////////////////////////////////////////////////////////////////
// MLMesh_PC.h - Classes used to convert generic mesh data into Fang PC specific data
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
// 08/19/02 Lafleur		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __MLMESH_PC_H_
#define __MLMESH_PC_H_

#include "fang.h"
#include "dx\fdx8mesh.h"

#include "ML.h"
#include "MLMesh.h"
#include "MLMaterial_PC.h"


// This structure is used as an abstraction layer between the
// indices to be rendered by a strip and the actual indices into the 
// respective vertex buffer attribute arrays.  Given a particular 
// vert index, we can get attribute indices for that vert.
typedef struct PCVertAbstr_s
{
	u16 nVBIdx;
} PCVertAbstr_t;


//
//
//
class MLMesh_PC	: public MLMesh
{
	protected:
		FDX8Mesh_t		m_FPCMesh;

//		u32				m_nTotalVertIdxCount;
//		u32				m_nStripCount;

	protected:
		// PCMesh construction
		MLResults* GenerateExportData( BOOL bGenerateCollisionData );
		u32 DivideVertsByVB( void );

		//
		//
		void FreeScratchMemory( void )
		{
			MLMesh::FreeScratchMemory();
		}

		//
		//
		BOOL Next3VertsAreUnique( MLTriContainer *pVC, u32 iStartIdx ) 
		{ 
			PCVertAbstr_t *pAbstr = &((PCVertAbstr_t *)pVC->m_pVertAbstr)[iStartIdx];
			if (   pAbstr[0].nVBIdx == pAbstr[1].nVBIdx
				|| pAbstr[0].nVBIdx == pAbstr[2].nVBIdx
				|| pAbstr[2].nVBIdx == pAbstr[1].nVBIdx )
			{
				return FALSE; 
			}

			return TRUE;
		}

		//
		//
		u16 GetAbstrPosIdx( MLTriContainer *pVC, u32 iAbstrIdx ) 
		{
			return ((PCVertAbstr_t *)pVC->m_pVertAbstr)[iAbstrIdx].nVBIdx;
		}

	public:
		//
		//
		MLMesh_PC( char *pszName, BOOL bWorldGeo ) : MLMesh( pszName, bWorldGeo )
		{
//			m_nTotalVertIdxCount = 0;
//			m_nStripCount = 0;

			fang_MemZero( &m_FPCMesh, sizeof( FDX8Mesh_t ) );
		}

		//
		//
		~MLMesh_PC( void )
		{
		}

		//
		// Sets the at rest bounding sphere for the mesh
		BOOL SetBoundingSphere( CFSphere *pBoundingSphere )
		{
			FASSERT( pBoundingSphere );
			m_FPCMesh.AtRestBoundSphere_MS = *pBoundingSphere;
			m_FMesh.BoundSphere_MS = *pBoundingSphere;
			return TRUE;
		}

		//
		//
		u16 AllocateMLMaterial( KongMat_t *pKMat )
		{
			MLMaterial_PC *pNewMaterial = new MLMaterial_PC;
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
