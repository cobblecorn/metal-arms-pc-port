//////////////////////////////////////////////////////////////////////////////////////
// ML.cpp - Fang2MeshLib management class
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
// 08/15/02 Lafleur		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include "stdafx.h"
#include <mmsystem.h>

// Fang includes
#include "fclib.h"
#include "dx\fdx8vb.h"

// MeshLib Includes
#include "ML.h"
#include "MLHash.h"
#include "MLMesh_GC.h"
#include "MLMesh_XB.h"
#include "MLMesh_PC.h"
#include "MLMaterial_GC.h"
#include "MLMaterial_PC.h"
#ifdef _MMI_TARGET_PS2
#include "MMI_MLMesh_PS2.h"
#endif


//////////////////////////////////////////////////////////////////////////////////////
// Global variables:
//////////////////////////////////////////////////////////////////////////////////////

BOOL CMLManager::m_bInstantiated = 0;

//
// The one and only instance of the Mesh Lib Manager
CMLManager MLManager;


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
CMLManager::CMLManager( void )
{
	FASSERT( !m_bInstantiated );
	m_bInstantiated = TRUE;

	m_Results.fTotalTime = 0;
	m_Results.fVertAddTime = 0;
	m_Results.fCompressTime = 0;
	m_Results.fCollisionTime = 0;
	m_Results.fDLTime = 0;

	m_pDlg = NULL;

	m_nPlatformType = PASM_TARGET_NONE;

	m_pGCVertHash = NULL;
	m_pPCVertHash = NULL;

	m_pFirstMLMesh = NULL;
	m_pLastMLMesh = NULL;

	m_bVertexRadiosityLit = FALSE;
	m_bAccumStreamingData = FALSE;
	m_pStreamingData = NULL;
	m_nStreamingDataBytes = 0;
}


//
//
//
CMLManager::~CMLManager( void )
{
	FreeMeshes();

	FreeVertHash();

	ResetTimers();
}


//
//
//
void CMLManager::FreeVertHash( void )
{
	if ( m_pPCVertHash )
	{
		delete m_pPCVertHash;
		m_pPCVertHash = NULL;
	}

	if ( m_pGCVertHash )
	{
		delete m_pGCVertHash;
		m_pGCVertHash = NULL;
	}
#ifdef _MMI_TARGET_PS2
	if ( m_pPS2VertHash )
	{
		delete m_pPS2VertHash;
		m_pPS2VertHash = NULL;
	}
#endif
}


//
//
//
void CMLManager::ResetTimers( void )
{
	DEVPRINTF( "\nTotal MLManager Time: %5.2f - VertSubmission: %5.2f - Compression: %5.2f - Collision: %5.2f - Display Lists: %5.2f - Export Data: %5.2f\n\n", m_Results.fTotalTime, m_Results.fVertAddTime, m_Results.fCompressTime, m_Results.fCollisionTime, m_Results.fDLTime, m_Results.fDataGeneration );
	m_Results.fTotalTime = 0;
	m_Results.fVertAddTime = 0;
	m_Results.fCompressTime = 0;
	m_Results.fCollisionTime = 0;
	m_Results.fDLTime = 0;
	m_Results.fStrippingTime = 0;
}


//
//
//
void CMLManager::FreeMeshes( void )
{
	// Free the meshes
	MLMesh *pNextMesh, *pMesh = m_pFirstMLMesh;
	while ( pMesh )
	{
		pNextMesh = pMesh->m_pNext;
		delete pMesh;
		pMesh = pNextMesh;
	}

	m_pFirstMLMesh = NULL;
	m_pLastMLMesh = NULL;

	FreeVertHash();
}


//
//
//
void CMLManager::AccumStreamingData( void )
{
	FASSERT( !m_pStreamingData && !m_bAccumStreamingData );
	m_bAccumStreamingData = TRUE;
	m_nStreamingDataBytes = 0;
}

//
//
//
void CMLManager::FreeStreamingData( void )
{
	if ( !m_bAccumStreamingData )
	{
		return;
	}

	m_bAccumStreamingData = FALSE;
	if ( m_pStreamingData )
	{
		free( m_pStreamingData );
		m_pStreamingData = NULL;
	}

	m_nStreamingDataBytes = 0;
}

//
//
//
void CMLManager::AppendToStreamingData( u32 nDataSize, void **pStartAddress, void **pBaseAddress )
{
	nDataSize = FMATH_BYTE_ALIGN_UP( nDataSize, 32 );
	if ( !m_pStreamingData )
	{
		m_nStreamingDataBytes = nDataSize;
		m_pStreamingData = malloc( nDataSize );
		(*pBaseAddress) = m_pStreamingData;
		(*pStartAddress) = m_pStreamingData;
		return;
	}

	u8 *pNewData = (u8 *)malloc( nDataSize + m_nStreamingDataBytes );

	memcpy( pNewData, m_pStreamingData, m_nStreamingDataBytes );
	free( m_pStreamingData );
	m_pStreamingData = pNewData;

	(*pBaseAddress) = m_pStreamingData;
	(*pStartAddress) = &pNewData[m_nStreamingDataBytes];
	m_nStreamingDataBytes += nDataSize;
}

//
//
//
BOOL CMLManager::InitForPlatform( CCompileDlg *pDlg, TargetPlatform_e nPlatformType, BOOL bGenerateMeshStrips )
{
	FreeMeshes();

	m_pDlg = pDlg;

	if ( nPlatformType == PASM_TARGET_XB )
	{
		nPlatformType = PASM_TARGET_PC;
	}

	// Otherwise, we need to free the memory allocations and allocate 
	// scratch space for the new platform:
	if ( nPlatformType == PASM_TARGET_GC )
	{
		m_pGCVertHash = new CGCVertHash;
		if ( !m_pGCVertHash )
		{
			return FALSE;
		}
		if ( !m_pGCVertHash->Init() )
		{
			return FALSE;
		}
	}
#ifdef _MMI_TARGET_PS2
	else if ( nPlatformType == PASM_TARGET_PS2 )
	{
		FASSERT( !m_pPCVertHash );
		m_pPS2VertHash = new CPS2VertHash;
		if ( !m_pPS2VertHash )
		{
			return FALSE;
		}
		if ( !m_pPS2VertHash->Init() )
		{
			return FALSE;
		}
	}
#endif
	else if ( nPlatformType == PASM_TARGET_PC )
	{
		m_pPCVertHash = new CPCVertHash;
		if ( !m_pPCVertHash )
		{
			return FALSE;
		}
		if ( !m_pPCVertHash->Init() )
		{
			return FALSE;
		}
	}
	else if ( nPlatformType == PASM_TARGET_XB )
	{
		FASSERT_NOW;
	}
	else
	{
		FASSERT_NOW;
	}

	m_bGenerateMeshStrips = bGenerateMeshStrips;

	m_bVertexRadiosityLit = FALSE;

	m_nPlatformType = nPlatformType;
	return TRUE;
}


//
//
//
MLMesh*	CMLManager::AllocateMLMesh( char *pszName, BOOL bWorld )
{
	// Client must supply a name
	if ( !pszName )
	{
		FASSERT_NOW;
		return NULL;
	}

	// copy the name and remove the extension
	char szName[FDATA_MESHNAME_LEN+1];
	fclib_strncpy( szName, pszName, FDATA_MESHNAME_LEN+1 );
	char *pPeriod = fclib_strrchr( szName, '.' );
	if( pPeriod ) {
		*pPeriod = 0;
	}

	// Allocate the appropriate platform-based MLMesh
	MLMesh *pNewMesh = NULL;
	if ( m_nPlatformType == PASM_TARGET_GC )
	{
		pNewMesh = new MLMesh_GC( szName, bWorld );
	}
	else if ( m_nPlatformType == PASM_TARGET_PC )
	{
		pNewMesh = new MLMesh_PC( szName, bWorld );
	}
#ifdef _MMI_TARGET_PS2
	else if ( m_nPlatformType == PASM_TARGET_PS2 )
	{
		pNewMesh = new MLMesh_PS2( szName, bWorld );
	}
#endif
	else if ( m_nPlatformType == PASM_TARGET_XB )
	{
		FASSERT_NOW;
	}
	else
	{
		FASSERT_NOW;
	}

	if ( !pNewMesh )
	{
		return NULL;
	}

	// Append the mesh to the list
	if ( !m_pFirstMLMesh )
	{
		m_pFirstMLMesh = pNewMesh;
		m_pLastMLMesh = pNewMesh;
		return pNewMesh;
	}

	m_pLastMLMesh->m_pNext = pNewMesh;
	m_pLastMLMesh = pNewMesh;
	return pNewMesh;
}

//
//
//
void CMLManager::RemoveFromExportList( MLMesh *pMesh )
{
	if ( !pMesh || !m_pFirstMLMesh )
	{
		return;
	}

	// Append the mesh to the list

	MLMesh *pLastMesh = NULL;
	MLMesh *pTestMesh = m_pFirstMLMesh;

	while ( pTestMesh )
	{
		if ( pTestMesh == pMesh )
		{
			if ( !pLastMesh )
			{
				m_pFirstMLMesh = pMesh->m_pNext;
				return;
			}
			else
			{
				pLastMesh->m_pNext = pMesh->m_pNext;
				return;
			}
		}

		pLastMesh = pTestMesh;
		pTestMesh = pTestMesh->m_pNext;
	}

	FASSERT_NOW;
	return;
}

static MLResults _TempResults;
//
//
//
MLResults* CMLManager::GenerateExportData( BOOL bGenerateCollisionData /*= TRUE*/ )
{
	if ( !m_pLastMLMesh )
	{
		m_Results.nErrorCode = ML_GENERAL_ERROR;
		FASSERT_NOW;
		return &m_Results;
	}

	if ( m_pLastMLMesh != m_pFirstMLMesh )
	{
		m_Results.nErrorCode = ML_GENERAL_ERROR;
		DEVPRINTF( "CMLManager::GenerateExportData() - ERROR - Multiple Mesh export not supported.\n" );
		return &m_Results;
	}

	MLResults *pResults = m_pLastMLMesh->GenerateExportData( bGenerateCollisionData );
	fang_MemCopy( &_TempResults, pResults, sizeof( MLResults ));

	FreeMeshes();

	return &_TempResults;
}
