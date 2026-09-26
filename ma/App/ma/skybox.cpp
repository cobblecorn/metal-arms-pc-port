//////////////////////////////////////////////////////////////////////////////////////
// skybox.cpp - Sky box module.
//
// Author: Steve Ranck     
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
// 08/21/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "skybox.h"
#include "fresload.h"
#include "floop.h"
#include "fclib.h"
#include "fgamedata.h"
#include "fmesh.h"
#include "fvis.h"
#include "fsh.h"


const FGameData_TableEntry_t CSkyBox::m_aUserPropVocab[] = {
	// pszMeshName:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_ONLY,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fScale:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fDispX:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fDispY:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fDispZ:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fSpinAxisX:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fSpinAxisY:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fSpinAxisZ:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fSpinSpeed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


CSkyBox::CSkyBox() {
	m_pMeshInst = NULL;
	m_Displacement.Zero();
	m_RotUnitAxis = CFVec3A::m_UnitAxisX;
	m_fRotSpeed = 0.0f;
	m_fRotAngle = 0.0f;
	m_fScale = 1.0f;
}


BOOL CSkyBox::Create( cchar *pszMeshName ) {
	FMesh_t *pMesh;
	FMeshInit_t MeshInit;

	FResFrame_t ResFrame = fres_GetFrame();

	m_pMeshInst = fnew CFMeshInst;
	if( m_pMeshInst == NULL ) {
		DEVPRINTF( "CSkyBox::Create(): Not enough memory to create CFMeshInst.\n" );
		goto _ExitWithError;
	}

	pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pszMeshName );
	if( pMesh == NULL ) {
		DEVPRINTF( "CSkyBox::Create(): Mesh '%s' not found.\n", pszMeshName );
		goto _ExitWithError;
	}

	MeshInit.pMesh = pMesh;
	MeshInit.nFlags = FMESHINST_FLAG_NOLIGHT_AMBIENT | FMESHINST_FLAG_NOLIGHT_DYNAMIC | FMESHINST_FLAG_NOCOLLIDE;
	MeshInit.fCullDist = FMATH_MAX_FLOAT;
	MeshInit.Mtx.Identity();

	m_pMeshInst->Init( &MeshInit );

	m_fRotAngle = 0.0f;

	return TRUE;

_ExitWithError:
	fdelete( m_pMeshInst );
	m_pMeshInst = NULL;

	fres_ReleaseFrame( ResFrame );

	return FALSE;
}


BOOL CSkyBox::CreateFromCSV( FGameDataFileHandle_t hGameDataFile, cchar *pszTableName ) {
	FGameDataTableHandle_t hTableHandle;
	FGameDataWalker_t GameDataWalker;
	cchar *pszFileTableName;
	UserPropData_t UserPropData;

	FResFrame_t ResFrame = fres_GetFrame();

	if( pszTableName == NULL ) {
		pszTableName = "SkyBox";
	}

	hTableHandle = fgamedata_GetFirstTable( hGameDataFile, GameDataWalker );
	for( ; hTableHandle != FGAMEDATA_INVALID_TABLE_HANDLE; hTableHandle = fgamedata_GetNextTable( GameDataWalker ) ) {
		// Find the table in the file...

		pszFileTableName = fgamedata_GetTableName( hTableHandle );

		if( !fclib_stricmp( pszFileTableName, pszTableName ) ) {
			// Found table...
			break;
		}
	}

	if( hTableHandle == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		// Table not found...
		goto _ExitWithError;
	}

	// Found table...

	if( !fgamedata_GetTableData( hTableHandle, m_aUserPropVocab, &UserPropData, sizeof(UserPropData) ) ) {
		DEVPRINTF( "CSkyBox::CreateFromCSV(): Error in definition of table '%s' found in '%s'.\n", pszTableName, fgamedata_GetFileNameFromFileHandle(hGameDataFile) );
		goto _ExitWithError;
	}

	if( !Create( UserPropData.pszMeshName ) ) {
		DEVPRINTF( "CSkyBox::CreateFromCSV(): Could not find mesh '%s' as specified in table '%s' of '%s'.\n", UserPropData.pszMeshName, pszTableName, fgamedata_GetFileNameFromFileHandle(hGameDataFile) );
		goto _ExitWithError;
	}

	m_fScale = UserPropData.fScale;
	m_Displacement.Set( UserPropData.fDispX, UserPropData.fDispY, UserPropData.fDispZ );
	m_fRotSpeed = UserPropData.fSpinSpeed * FMATH_2PI;

	m_RotUnitAxis.Set( UserPropData.fSpinAxisX, UserPropData.fSpinAxisY, UserPropData.fSpinAxisZ );
	if( m_RotUnitAxis.Mag() < 0.01f ) {
		m_RotUnitAxis = CFVec3A::m_UnitAxisX;
	} else {
		m_RotUnitAxis.Unitize();
	}

	// Success...

	fvis_RegisterSkyBox( m_pMeshInst );
	
	return TRUE;

_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CSkyBox::Destroy( void ) {
	fdelete( m_pMeshInst );
	m_pMeshInst = NULL;
}


void CSkyBox::Work( void ) {
	if( m_pMeshInst == NULL ) {
		return;
	}

	if( m_fRotSpeed == 0.0f ) {
		return;
	}

	// Rotation is specified...

	if( m_fRotSpeed > 0.0f ) {
		// Positive rotation...

		m_fRotAngle += FLoop_fPreviousLoopSecs * m_fRotSpeed;

		while( m_fRotAngle >= FMATH_2PI ) {
			m_fRotAngle -= FMATH_2PI;
		}
	} else {
		// Negative rotation...

		m_fRotAngle += FLoop_fPreviousLoopSecs * m_fRotSpeed;

		while( m_fRotAngle < 0.0f ) {
			m_fRotAngle += FMATH_2PI;
		}
	}
}


// Assumes the mesh renderer is active and the camera is already set up with
// nothing on the xfm stack.
void CSkyBox::Draw( const CFVec3A *pPos_WS ) {
	if( m_pMeshInst == NULL ) {
		return;
	}

	CFQuatA Quat;
	CFVec3A Pos_WS;

	Quat.BuildQuat( m_RotUnitAxis, m_fRotAngle );
	Pos_WS.Add( *pPos_WS, m_Displacement );

	m_pMeshInst->m_Xfm.BuildFromQuat( Quat, Pos_WS, m_fScale );

	BOOL bFogEnabled = fsh_Fog_IsEnabled();
	fsh_Fog_Enable(FALSE);
	m_pMeshInst->Draw( FVIEWPORT_PLANESMASK_ALL, TRUE );
	fsh_Fog_Enable(bFogEnabled);
}

