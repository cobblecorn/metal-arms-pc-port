//////////////////////////////////////////////////////////////////////////////////////
// MLMesh.cpp - Classes used to convert generic mesh data into Fang specific data
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
// 08/15/02 Lafleur		Created as shared MLMesh code for multi-platform support.
//////////////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include "stdafx.h"
#include <mmsystem.h>

// PASM includes
#include "..\pasm.h"
#include "..\pasmDlg.h"

// MeshLib Includes
#include "MLHash.h"
#include "MLMesh.h"


//////////////////////////////////////////////////////////////////////////////////////
// Global variables:
//////////////////////////////////////////////////////////////////////////////////////

// Conversion counters for collision descriptions/geometry
u32	MLMesh_nTotalTreeCount;
u32	MLMesh_nTotalNodeCount;
u32	MLMesh_nTotalIntervalCount;
u32	MLMesh_nTotalTriCount;
u32	MLMesh_nTotalTriPackets;
u32	MLMesh_nTotalTriDataBytes;
u32	MLMesh_nTotalRootDOPVertBytes;
u32 MLMesh_nTotalkDOPFaceCount;

BOOL MLMesh_bVolumeMesh;


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
u32 MLMesh::ResolveMaterials( u32 *pDLCount /* = NULL*/ )
{
	if ( !m_pFirstSegment )
	{
		return ML_NO_SEGMENTS_ESTABLISHED_FOR_EXPORT;
	}

	// Cycle through the segments adding the  up the number of unique materials
	MLSegment *pTempSegment = m_pFirstSegment;
	while ( pTempSegment )
	{
		MLTriContainer *pTC = pTempSegment->m_pFirstTriContainer;
		while ( pTC )
		{
#ifdef _MMI_TARGET_PS2
//NS PS2 LOD Removal
            if((m_FMesh.nLODCount==1)||(m_FMesh.nLODCount > 1 && pTC->m_nLODID != 0)) //NS
			{
#endif

			pTC->CalculateVBKey( MLManager.GetPlatformType() );

			FASSERT( pTC->m_pMaterial );

			if ( pDLCount )
			{
				(*pDLCount) += pTC->m_pMaterial->AppendTris( pTC );
			}
			else
			{
				pTC->m_pMaterial->AppendTris( pTC );
			}

#ifdef _MMI_TARGET_PS2
//NS PS2 LOD Removal
				if(m_FMesh.nLODCount > 1)
                    pTC->m_pMaterial->m_FMaterial.nLODMask |= (1 << (pTC->m_nLODID-1)); //NS On PS2 we are throwing away the top level LOD
				else
#endif
                    pTC->m_pMaterial->m_FMaterial.nLODMask |= (1 << (pTC->m_nLODID));

#ifdef _MMI_TARGET_PS2
//NS PS2 LOD Removal
            }
#endif

			pTC = pTC->m_pNext;
		}

		pTempSegment = pTempSegment->m_pNext;
	}

	return ML_NO_ERROR;
}


//
//
//
u32 MLMesh::CalculateMeshLODSwitches( void )
{
	u32 i;

	if ( m_FMesh.afLODDistance[1] != -1.f )
	{
		for ( i = 1; i < m_FMesh.nLODCount; i++ )
		{
			if ( m_FMesh.afLODDistance[i] == -1.f )
			{
				return ML_LOD_COUNT_IN_AID_DOES_NOT_MATCH_MESH;
			}
		}
	}
	else
	{
		f32 fDiameter = m_FMesh.BoundSphere_MS.m_fRadius * 1.8f;
		m_FMesh.afLODDistance[0] = 0;
		for ( i = 1; i < FDATA_MAX_LOD_MESH_COUNT; i++ )
		{
			m_FMesh.afLODDistance[i] = fDiameter + (2.f * i * i * fDiameter);
		}
	}

	return ML_NO_ERROR;
}


//
//
//
u32 MLMesh::WriteVertRemapFile( u32 nVBs )
{
	u32 nTotalBytesWritten = 0;
	u32 i, nBytesWritten;

	// Create/Overwrite the file for this kong
	CString csName;
	if ( MLManager.GetPlatformType() == PASM_TARGET_GC )
	{
		csName = CPasmDlg::m_csPASMTempDirectory + m_szVertRemapFilename + "_GC.rmp";
	}
	else if ( MLManager.GetPlatformType() == PASM_TARGET_PC )
	{
		csName = CPasmDlg::m_csPASMTempDirectory + m_szVertRemapFilename + "_PC.rmp";
	}
	else if ( MLManager.GetPlatformType() == PASM_TARGET_XB )
	{
		csName = CPasmDlg::m_csPASMTempDirectory + m_szVertRemapFilename + "_XB.rmp";
	}
//ARG - >>>>>
#ifdef _MMI_TARGET_PS2
	else if ( MLManager.GetPlatformType() == PASM_TARGET_PS2 )
	{
		csName = CPasmDlg::m_csPASMTempDirectory + m_szVertRemapFilename + "_PS2.rmp";
	}
#endif
//ARG - <<<<<
	csName.MakeLower();
	HANDLE hFile = CreateFile( csName, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
	{
		return 0;
	}

	// Write out the time stamp for the ape file
    WriteFile( hFile, &APELastWriteTime, sizeof( SYSTEMTIME ), (LPDWORD)&nBytesWritten, NULL );
	if ( nBytesWritten != sizeof( SYSTEMTIME ) )
	{
		goto SAVEERROR;
	}
	nTotalBytesWritten += nBytesWritten;

	// Write out the number of VB's
    WriteFile( hFile, &nVBs, sizeof( u32 ), (LPDWORD)&nBytesWritten, NULL );
	if ( nBytesWritten != sizeof( u32 ) )
	{
		goto SAVEERROR;
	}
	nTotalBytesWritten += nBytesWritten;

	// Write out the total vertex count
	u32 nVertCount = m_nRemapCount;
    WriteFile( hFile, &nVertCount, sizeof( u32 ), (LPDWORD)&nBytesWritten, NULL );
	if ( nBytesWritten != sizeof( u32 ) )
	{
		goto SAVEERROR;
	}
	nTotalBytesWritten += nBytesWritten;

	// Write out all of the remap structures
	for ( i = 0; i < nVertCount; i++ )
	{
		WriteFile( hFile, &m_paVertRemaps[i], sizeof( KongVertRemap_t ), (LPDWORD)&nBytesWritten, NULL );
		if ( nBytesWritten != sizeof( KongVertRemap_t ) )
		{
			goto SAVEERROR;
		}
		if ( m_paVertRemaps[i].nVBIndex >= nVBs )
		{
			DEVPRINTF( "MLMesh::WriteVertRemapFile() - Vertex remap with VB index %d exceeds actual VB count of %d.\n", m_paVertRemaps[i].nVBIndex, nVBs );
		}
		if ( m_paVertRemaps[i].nVBIndex >= nVBs )
		{
			DEVPRINTF( "MLMesh::WriteVertRemapFile() - Vertex remap with color index %d exceeds actual vert count of %d.\n", m_paVertRemaps[i].nVertColorIndex, nVertCount );
		}
		nTotalBytesWritten += nBytesWritten;
	}

	// Write out a key for later verification
	u16 nKey = 0xBEEF;
	WriteFile( hFile, &nKey, sizeof(u16), (LPDWORD)&nBytesWritten, NULL );
	if ( nBytesWritten != sizeof( u16 ) )
	{
		goto SAVEERROR;
	}
	nTotalBytesWritten += nBytesWritten;

	CloseHandle( hFile );

	return nTotalBytesWritten;

SAVEERROR:
	CloseHandle( hFile );
	DeleteFile( csName );
	return 0;
}
