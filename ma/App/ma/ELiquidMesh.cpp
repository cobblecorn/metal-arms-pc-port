//////////////////////////////////////////////////////////////////////////////////////
// ELiquidMesh.cpp - 
//
// Author: Jeremy Chernobieff
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
// 01/19/03 Chernobieff Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "ELiquidMesh.h"
#include "fliquid.h"
#include "entity.h"
#include "floop.h"
#include "fresload.h"
#include "fclib.h"

#include "MeshTypes.h"


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CELiquidVolumeBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CELiquidMeshBuilder _ELiquidMeshBuilder;
//CDamageProfile *CELiquidMesh::m_pDamageProfile[3];

void CELiquidMeshBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) 
{
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CEntityBuilder, ENTITY_BIT_LIQUIDMESH, pszEntityType );

	m_nShapeType = FWORLD_SHAPETYPE_BOX;
		
	m_fDimX = 1.0f;
	m_fDimY = 1.0f;
	m_fDimZ = 1.0f;
	
	m_fOpacity = 1.0f;
	m_fTile = 4.0f;
	m_fSpeed = 1.0f;
	m_fCurvature = 4.0f;
	
	m_Color.fRed = m_Color.fGreen = m_Color.fBlue = 1.0f;
	
	m_pszTexLayer0 = NULL;
	m_pszTexLayer1 = NULL;
	m_pszType = NULL;
		
	m_bEmpty = TRUE;
}


BOOL CELiquidMeshBuilder::InterpretTable( void ) 
{
	f32 foo255 = (1.0f/255.0f);
	if( !fclib_stricmp( CEntityParser::m_pszTableName, "LiqType" ) ) 
	{
		CEntityParser::Interpret_String( &m_pszType );

		return TRUE;

	}
	else if( !fclib_stricmp( CEntityParser::m_pszTableName, "ColorRed" ) ) 
	{
		CEntityParser::Interpret_F32( &m_Color.fRed, 0.0f, 255.0f, TRUE );

		// Convert from (0,255) to (0,1)
		m_Color.fRed *= foo255;
		FMATH_CLAMP_UNIT_FLOAT( m_Color.fRed );

		return TRUE;
	} 
	else if( !fclib_stricmp( CEntityParser::m_pszTableName, "ColorGreen" ) ) 
	{
		CEntityParser::Interpret_F32( &m_Color.fGreen, 0.0f, 255.0f, TRUE );

		// Convert from (0,255) to (0,1)
		m_Color.fGreen *= foo255;
		FMATH_CLAMP_UNIT_FLOAT( m_Color.fGreen );

		return TRUE;
	}
	else if( !fclib_stricmp( CEntityParser::m_pszTableName, "ColorBlue" ) ) 
	{
		CEntityParser::Interpret_F32( &m_Color.fBlue, 0.0f, 255.0f, TRUE );

		// Convert from (0,255) to (0,1)
		m_Color.fBlue *= foo255;
		FMATH_CLAMP_UNIT_FLOAT( m_Color.fBlue );

		return TRUE;
	}
	else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Opacity" ) ) 
	{
		CEntityParser::Interpret_F32( &m_fOpacity, 0.0f, 100.0f, TRUE );
		
		// Convert from [0,100] to [0.1]
		m_fOpacity *= 0.01f;
		FMATH_CLAMP_UNIT_FLOAT( m_fOpacity );

		return TRUE;
	}
	else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Tile" ) ) 
	{
		CEntityParser::Interpret_F32( &m_fTile, 0.0f, 128.0f, TRUE );
		
		return TRUE;
	} 
	else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Curvature" ) ) 
	{
		CEntityParser::Interpret_F32( &m_fCurvature, 2.0f, 32.0f, TRUE );
		
		return TRUE;
	}
	else if ( !fclib_stricmp( CEntityParser::m_pszTableName, "Texture1" ) ) 
	{
		CEntityParser::Interpret_String( &m_pszTexLayer0 );

		return TRUE;

	}
	else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Texture2" ) ) 
	{
		CEntityParser::Interpret_String( &m_pszTexLayer1 );

		return TRUE;

	}
	
	return CEntityBuilder::InterpretTable();
}


BOOL CELiquidMeshBuilder::PostInterpretFixup( void ) 
{

	const CFWorldShapeInit *pWorldShapeInit = CEntityParser::m_pWorldShapeInit;
	
	if( !CEntityBuilder::PostInterpretFixup() ) 
	{
		goto _OutOfMemory;
	}
	
	switch( pWorldShapeInit->m_nShapeType ) 
	{
		case FWORLD_SHAPETYPE_BOX:
			m_fDimX = pWorldShapeInit->m_pBox->m_fDimX;
			m_fDimY = pWorldShapeInit->m_pBox->m_fDimY;
			m_fDimZ = pWorldShapeInit->m_pBox->m_fDimZ;
			
			m_nShapeType = FWORLD_SHAPETYPE_BOX;
			break;

		default:
			FASSERT_NOW_MSG( "ELiquidVolume - Only Boxes are currently supported as liquid volumes." );
			break;
	}

	m_bEmpty = FALSE;

	return TRUE;

_OutOfMemory:
	return FALSE;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CELiquidMesh
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CELiquidMesh::m_bSystemInitialized = FALSE;


BOOL CELiquidMesh::InitSystem( void ) 
{
	FASSERT( !m_bSystemInitialized );

	m_bSystemInitialized = TRUE;

	return TRUE;
}


void CELiquidMesh::UninitSystem( void ) 
{
	if( m_bSystemInitialized ) 
	{
		m_bSystemInitialized = FALSE;
	}
}


CELiquidMesh::CELiquidMesh() 
{
	// Clear all pointers to NULL...
	_ClearMemberData();
}


CELiquidMesh::~CELiquidMesh() 
{
	if( IsSystemInitialized() && IsCreated() ) 
	{
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld(TRUE);
		ClassHierarchyDestroy();
	}
}


BOOL CELiquidMesh::Create( cchar *pszEntityName/*=NULL*/,
						const CFMtx43A *pMtx/*=NULL*/,
						cchar *pszAIBuilderName/*=NULL*/ ) 
{
//	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CELiquidMeshBuilder *pBuilder = (CELiquidMeshBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_ELiquidMeshBuilder ) 
	{
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_LIQUIDMESH );
	}

	return CEntity::Create( pszEntityName, pMtx, pszAIBuilderName );
}


CEntityBuilder *CELiquidMesh::GetLeafClassBuilder( void ) 
{
	// Do this and nothing more...
	return &_ELiquidMeshBuilder;
}


void CELiquidMesh::ClassHierarchyDestroy( void ) 
{

	// Delete any objects that we may have created.

	// Set our allocation pointers to NULL...
	_ClearMemberData();

	// Call our parent class's ClassHierarchyDestroy() method...
	CEntity::ClassHierarchyDestroy();
}

BOOL CELiquidMesh::ClassHierarchyBuild( void ) 
{
	//FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );
	
	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CELiquidMeshBuilder *pBuilder = (CELiquidMeshBuilder *)GetLeafClassBuilder();

	// Build our parent entity class...
	if( !CEntity::ClassHierarchyBuild() ) 
	{
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set any defaults in our class...
	_ClearMemberData();

	// Initialize our entity from our builder object...
	if( !pBuilder->m_bEmpty ) 
	{
		m_afRawValues[0] = m_afRawValues[1] = m_afRawValues[2] = 0.0f;
		m_afShapeValues[0] = m_afShapeValues[1] = m_afShapeValues[2] = 0.0f;

		// fill in the cfg
		switch( pBuilder->m_nShapeType ) 
		{
			case FWORLD_SHAPETYPE_BOX:
				
				// setup the box
				m_vMtx.Set(m_MtxToWorld);
							
				m_afRawValues[0] = pBuilder->m_fDimX * 0.5f;
				m_afShapeValues[0] = m_afRawValues[0] * m_fScaleToWorld;	
				m_vExt.x = m_afShapeValues[0];
				
				m_afRawValues[1] = pBuilder->m_fDimY * 0.5f;
				m_afShapeValues[1] = m_afRawValues[1] * m_fScaleToWorld;	
				m_vExt.y = m_afShapeValues[1];
				
				m_afRawValues[2] = pBuilder->m_fDimZ * 0.5f;
				m_afShapeValues[2] = m_afRawValues[2] * m_fScaleToWorld;
				m_vExt.z = m_afShapeValues[2];
				
				//build liquid volume here.
				m_pLiquid = LiquidSystem.CreateLiquidMesh();
				
				if (!m_pLiquid)
				{
					goto _ExitWithError;
				}
				
				LiquidType_e nType;
				if (fclib_stricmp(pBuilder->m_pszType, "water")==0)
				{
					nType = LT_WATER;
				}
				else if (fclib_stricmp(pBuilder->m_pszType, "mercury")==0)
				{
					nType = LT_MERCURY;
				}
				else if (fclib_stricmp(pBuilder->m_pszType, "oil")==0)
				{
					nType = LT_OIL;
				}
				else if (fclib_stricmp(pBuilder->m_pszType, "molten")==0)
				{
					nType = LT_MOLTEN;
				}
				else if (fclib_stricmp(pBuilder->m_pszType, "texture")==0)
				{
					nType = LT_TEXTURE;
				}
				else
				{
					nType = LT_MERCURY;
				}
				
				m_pLiquid->SetLiquidType(nType);
				m_pLiquid->SetupMesh(m_vExt, m_vMtx, pBuilder->m_fCurvature);
				
				m_pTexInst[0] = NULL;
				m_pTexInst[1] = NULL;
				
				// Load the mesh resource...
				if (pBuilder->m_pszTexLayer0)
				{
					FTexDef_t *pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, pBuilder->m_pszTexLayer0 );
					if (pTexDef)
					{
						m_pTexInst[0] = fnew CFTexInst;
						m_pTexInst[0]->SetTexDef(pTexDef);
						m_pTexInst[0]->SetFlags( CFTexInst::FLAG_WRAP_S|CFTexInst::FLAG_WRAP_T );
					}
				}
				if (pBuilder->m_pszTexLayer1)
				{
					FTexDef_t *pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, pBuilder->m_pszTexLayer1 );
					if (pTexDef)
					{
						m_pTexInst[1] = fnew CFTexInst;
						m_pTexInst[1]->SetTexDef(pTexDef);
						m_pTexInst[1]->SetFlags( CFTexInst::FLAG_WRAP_S|CFTexInst::FLAG_WRAP_T );
					}
				}
				
				m_pLiquid->SetTexLayers(m_pTexInst[0], m_pTexInst[1]);
				m_pLiquid->SetOpacity(pBuilder->m_fOpacity);
							
				m_pLiquid->Init();
				
				break;

			default:
				FASSERT_NOW;
				break;
		}
	}

	// We've successfully built our object, so return success code...
	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CELiquidMesh::ClassHierarchyRelocated( void *pIdentifier ) 
{
	FASSERT( IsCreated() );

	// Step #1:
	// First, inform our parent...
	CEntity::ClassHierarchyRelocated( pIdentifier );

	// Step #2:
	// m_MtxToWorld, m_MtxToParent, m_fScaleToWorld, and m_fScaleToParent
	// have been relocated.  Use this info to relocate our internal things...
	m_afShapeValues[0] = m_afRawValues[0] * m_fScaleToWorld;
	m_afShapeValues[1] = m_afRawValues[1] * m_fScaleToWorld;
	m_afShapeValues[2] = m_afRawValues[2] * m_fScaleToWorld;
}


void CELiquidMesh::ClassHierarchyRemoveFromWorld( void ) 
{
	FASSERT( IsCreated() );
	
	//stop sim.

	CEntity::ClassHierarchyRemoveFromWorld();
}



void CELiquidMesh::ClassHierarchyAddToWorld( void ) 
{
	FASSERT( IsCreated() );
	
	// Step #1:
	// First, inform our parent...
	CEntity::ClassHierarchyAddToWorld();

	// Step #2:
	// m_MtxToWorld, m_MtxToParent, m_fScaleToWorld, and m_fScaleToParent
	// have been relocated.  Use this info to relocate our internal things...
	m_afShapeValues[0] = m_afRawValues[0] * m_fScaleToWorld;
	m_afShapeValues[1] = m_afRawValues[1] * m_fScaleToWorld;
	m_afShapeValues[2] = m_afRawValues[2] * m_fScaleToWorld;
}



BOOL CELiquidMesh::ClassHierarchyBuilt( void ) 
{
//	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CEntity::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	DisableOurWorkBit();

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CELiquidMesh::ClassHierarchyResolveEntityPointerFixups( void ) 
{
	FASSERT( IsCreated() );

	// Call parent's function...
	CEntity::ClassHierarchyResolveEntityPointerFixups();
}

void CELiquidMesh::ClassHierarchyWork( void ) 
{
	FASSERT( IsCreated() );

	// Perform work on our parent...
	CEntity::ClassHierarchyWork();

	// Perform work for ourselves...
	if (m_bStopFlow)
	{
		m_fDeltaTime -= FLoop_fPreviousLoopSecs;
		if (m_fDeltaTime <= 0.0f)
		{
			m_bStopFlow = FALSE;
			//m_pLiquid->Disable();
		}
	}
	else if (m_bStartFlow)
	{
		m_fDeltaTime -= FLoop_fPreviousLoopSecs;
		if (m_fDeltaTime <= 0.0f)
		{
			m_fDeltaTime = 0.0f;

			m_bStartFlow = FALSE;
			//m_pLiquid->Enable();
		}
	}
}


// This private helper method NULLs any allocation pointers.
// It is extremely important that this get called by the
// entity class's constructor and ClassHierarchyDestroy().
void CELiquidMesh::_ClearMemberData( void ) 
{
	m_afRawValues[0] = m_afRawValues[1] = m_afRawValues[2] = 0.0f;
	m_afShapeValues[0] = m_afShapeValues[1] = m_afShapeValues[2] = 0.0f;

	m_bStopFlow = FALSE;
	m_bStartFlow = FALSE;
	
	m_pLiquid = NULL;
}

void CELiquidMesh::StopFlow(f32 fTime)
{
	m_bStopFlow = TRUE; m_bStartFlow = FALSE;

	m_fDeltaTime = fTime;
	m_fTotalTime = fTime;
}

void CELiquidMesh::StartFlow(f32 fTime)
{
	m_bStopFlow = FALSE; m_bStartFlow = TRUE;

	m_fDeltaTime = fTime;
	m_fTotalTime = fTime;
}
