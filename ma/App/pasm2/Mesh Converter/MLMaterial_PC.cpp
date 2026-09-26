//////////////////////////////////////////////////////////////////////////////////////
// MLMaterial_PC.cpp - Class used to convert generic material data into Fang PC specific data
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
// 08/16/02 Lafleur		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "stdafx.h"

#include "MLMesh_PC.h"
#include "MLMaterial_PC.h"


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
BOOL MLMaterial_PC::BuildDLAbstractionIndices( MLDLContainer *pDL, u32 &nIndexCount )
{
	u32 i, j, nUniqueAbstrCount = 0;
	PCVertAbstr_t *pNewAbstr, *pTestAbstr;

	u16 *paUniqueIndices = (u16 *)fang_Malloc( sizeof( u16 ) * pDL->nVertexCount, 4 );
	if ( !paUniqueIndices )
	{
		return FALSE;
	}

	for ( i = 0; i < pDL->pTris->m_nVertCount; i++ )
	{
		pNewAbstr = &((PCVertAbstr_t *)pDL->pTris->m_pVertAbstr)[i];

		for ( j = 0; j < nUniqueAbstrCount; j++ )
		{
			pTestAbstr = &((PCVertAbstr_t *)pDL->pTris->m_pVertAbstr)[paUniqueIndices[j]];

			if ( memcmp( pNewAbstr, pTestAbstr, sizeof( PCVertAbstr_t ) ) == 0 )
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

