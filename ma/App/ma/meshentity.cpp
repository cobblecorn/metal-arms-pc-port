//////////////////////////////////////////////////////////////////////////////////////
// meshentity.cpp - Mesh entity object.
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
// 04/17/02 Ranck       Created.
// 09/13/02	Link		Typed this line up here.
// 09/13/02	Link		Oh, I also added the animation blending.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "meshentity.h"
#include "entity.h"
#include "fres.h"
#include "fresload.h"
#include "floop.h"
#include "fclib.h"
#include "FCheckPoint.h"
#include "meshtypes.h"
#include "bot.h"
#include "Hud2.h"
#include "fvis.h"
#include "collectable.h"
#include "ai/aigroup.h"
#include "fwire.h"




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CMeshEntityBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CMeshEntityBuilder _MeshEntityBuilder;


void CMeshEntityBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CEntityBuilder, ENTITY_BIT_MESHENTITY, pszEntityType );

	m_nFlags = FLAG_DRIVE_MESH_WITH_ANIM;

	m_nMeshFlags = FMESHINST_FLAG_NONE;
	m_fCullDist2 = -1.0f;

	m_nMeshCount = 0;
	m_ppszMeshNames = NULL;
	m_nUserAnimCount = 0;
	m_ppszUserAnimNames = NULL;

	m_nStartMeshIndex = 0;
	m_nStartAnimIndex = 0;

	m_fMeshFlipsPerSec = 0.0f;
	m_nMeshFlipDir = CMeshEntity::DIR_NEXT;

	m_nAnimFlip = CMeshEntity::ANIMFLIP_OFF;
	m_fAnimSpeedMult = 1.0f;

	m_bUseAnimBlending = FALSE;

	m_papszLightMapNames = NULL;
	m_panLightMapMotifs = NULL;
	m_paColorStreams = NULL;
	m_nColorStreamCount = 0;

	// Let base class know that we're capable of being a tripwire...
	m_bEC_ClassIsTripwireCapable = TRUE;

	m_bAllowVerlet = TRUE;

	m_ColorTint.Set( 1.f, 1.f, 1.f );
}


BOOL CMeshEntityBuilder::InterpretTable( void ) {
	FGameData_VarType_e nVarType;
	const void *pData;
	cchar *pszString;

	if( !fclib_stricmp( CEntityParser::m_pszTableName, "Mesh" ) ) {
		// List of mesh names...

		CEntityParser::Interpret_StringArray_CheckSyntax( (FGameDataTableHandle_t *)&m_ppszMeshNames );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "MeshSeq" ) ) {
		// Mesh sequence specified...

		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			CEntityParser::Interpret_StringArray_CheckSyntax( (FGameDataTableHandle_t *)&m_ppszMeshNames );	
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Anim" ) ) {
		// List of animation names...

		CEntityParser::Interpret_StringArray_CheckSyntax( (FGameDataTableHandle_t *)&m_ppszUserAnimNames );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "AnimSeq" ) ) {
		// Animation sequence specified...

		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			CEntityParser::Interpret_StringArray_CheckSyntax( (FGameDataTableHandle_t *)&m_ppszUserAnimNames );
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "StartMesh" ) ) {
		// Starting mesh index...

		CEntityParser::Interpret_U32( &m_nStartMeshIndex );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "StartAnim" ) ) {
		// Starting animation index...

		CEntityParser::Interpret_U32( &m_nStartAnimIndex );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "MeshFlip" ) ) {
		// Mesh flipping...

		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
			return TRUE;
		}

		pData = fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, 0, nVarType );

		switch( nVarType ) {
		case FGAMEDATA_VAR_TYPE_STRING:
			pszString = (cchar *)pData;

			if( !fclib_stricmp( pszString, "Off" ) ) {
				m_fMeshFlipsPerSec = 0.0f;
			} else if( !fclib_stricmp( pszString, "AnimLoop" ) ) {
				m_fMeshFlipsPerSec = -1.0f;
			} else {
				CEntityParser::Error_InvalidParameterValue();
			}

			return TRUE;

		case FGAMEDATA_VAR_TYPE_FLOAT:
			CEntityParser::Interpret_F32( &m_fMeshFlipsPerSec, 0.0f );
			return TRUE;

		default:
			FASSERT_NOW;
			return TRUE;
		}
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "MeshFlipDir" ) ) {
		// Mesh flipping direction...

		if( CEntityParser::Interpret_String( &pszString ) ) {
			if( !fclib_stricmp( pszString, "Next" ) ) {
				m_nMeshFlipDir = CMeshEntity::DIR_NEXT;
			} else if( !fclib_stricmp( pszString, "Previous" ) ) {
				m_nMeshFlipDir = CMeshEntity::DIR_PREV;
			} else if( !fclib_stricmp( pszString, "Random" ) || !fclib_stricmp( pszString, "Random1" ) ) {
				m_nMeshFlipDir = CMeshEntity::DIR_RAND1;
			} else if( !fclib_stricmp( pszString, "Random2" ) ) {
				m_nMeshFlipDir = CMeshEntity::DIR_RAND2;
			} else {
				CEntityParser::Error_InvalidParameterValue();
			}
		}

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "AnimFlip" ) ) {
		// Animation flipping instructions...

		if( CEntityParser::Interpret_String( &pszString ) ) {
			if( !fclib_stricmp( pszString, "Next" ) ) {
				m_nAnimFlip = CMeshEntity::ANIMFLIP_NEXT;
			} else if( !fclib_stricmp( pszString, "Previous" ) ) {
				m_nAnimFlip = CMeshEntity::ANIMFLIP_PREV;
			} else if( !fclib_stricmp( pszString, "Random" ) || !fclib_stricmp( pszString, "Random1" ) ) {
				m_nAnimFlip = CMeshEntity::ANIMFLIP_RAND1;
			} else if( !fclib_stricmp( pszString, "Random2" ) ) {
				m_nAnimFlip = CMeshEntity::ANIMFLIP_RAND2;
			} else if( !fclib_stricmp( pszString, "MatchMesh" ) ) {
				m_nAnimFlip = CMeshEntity::ANIMFLIP_MATCHMESH;
			} else if( !fclib_stricmp( pszString, "Off" ) ) {
				m_nAnimFlip = CMeshEntity::ANIMFLIP_OFF;
			} else {
				CEntityParser::Error_InvalidParameterValue();
			}
		}

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "AnimSpeed" ) ) {
		// Animation playing speed multiplier...

		CEntityParser::Interpret_F32( &m_fAnimSpeedMult );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Animate" ) ) {
		// Drive mesh with animation flag...

		CEntityParser::Interpret_Flag( &m_nFlags, FLAG_DRIVE_MESH_WITH_ANIM );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "CullDist" ) ) {
		// Set cull distance...

		if( CEntityParser::Interpret_F32( &m_fCullDist2, 0.0f, 1e+20f, TRUE ) ) {
			m_fCullDist2 *= m_fCullDist2;
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "NoAnimLOD" ) ) {
		// Set order flag...

		if( CEntityParser::Interpret_Flag( &m_nMeshFlags, FMESHINST_FLAG_NOANIMLOD ) ) {
			FMATH_SETBITMASK( m_nFlags, FLAG_NOANIMLOD_COMMAND_SPECIFIED );
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PosterX" ) ) {
		// Set order flag...

		if( CEntityParser::Interpret_Flag( &m_nMeshFlags, FMESHINST_FLAG_POSTER_X ) ) {
			FMATH_SETBITMASK( m_nFlags, FLAG_POSTERX_COMMAND_SPECIFIED );
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PosterY" ) ) {
		// Set order flag...

		if( CEntityParser::Interpret_Flag( &m_nMeshFlags, FMESHINST_FLAG_POSTER_Y ) ) {
			FMATH_SETBITMASK( m_nFlags, FLAG_POSTERY_COMMAND_SPECIFIED );
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PosterZ" ) ) {
		// Set order flag...

		if( CEntityParser::Interpret_Flag( &m_nMeshFlags, FMESHINST_FLAG_POSTER_Z ) ) {
			FMATH_SETBITMASK( m_nFlags, FLAG_POSTERZ_COMMAND_SPECIFIED );
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Coll" ) ) {
		// Set order flag...

		if( CEntityParser::Interpret_Flag( &m_nMeshFlags, FMESHINST_FLAG_NOCOLLIDE, TRUE ) ) {
			FMATH_SETBITMASK( m_nFlags, FLAG_NOCOLL_COMMAND_SPECIFIED );
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Light" ) ) {
		// Set order flag...

		if( CEntityParser::Interpret_Flag( &m_nMeshFlags, FMESHINST_FLAG_NOLIGHT, TRUE ) ) {
			FMATH_SETBITMASK( m_nFlags, FLAG_NOLIGHT_COMMAND_SPECIFIED );
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "mesh_always_work" ) ) {
		// always call work function
		CEntityParser::Interpret_Flag( &m_nFlags, FLAG_ALWAYS_WORK );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "VehicleCollOnly" ) ) {
		// only collide with vehicles
		CEntityParser::Interpret_Flag( &m_nFlags, FLAG_VEHICLE_COLL_ONLY );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "AIAvoid" ) ) {
		// AI entities should avoid colliding with this object (so AI avoids ambient vehicles on splines, etc)
		CEntityParser::Interpret_Flag( &m_nFlags, FLAG_AI_AVOID );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "TranslatorBone" ) ) {
		// The specified bone's animation will drive the orientation of the mesh entity.
		// Usage: TranslatorBone=On  (Uses the first child of "Scene Root" as the translator bone)
		//        TranslatorBone=Off (Default)
		CEntityParser::Interpret_Flag( &m_nFlags, FLAG_TRANSLATOR_BONE );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "UpdateBoundingSphere" ) ) {
		// The specified bone's animation will drive the orientation of the mesh entity.
		// Usage: TranslatorBone=On  (Uses the first child of "Scene Root" as the translator bone)
		//        TranslatorBone=Off (Default)
		CEntityParser::Interpret_Flag( &m_nFlags, FLAG_TRANSLATOR_BONE );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "LoaderPickup" ) ) {
		// The loader can pick this object up
		CEntityParser::Interpret_Flag( &m_nFlags, FLAG_LOADER_PICKUP_ENABLED );
		return TRUE;
	}

	// We don't understand this command. Pass it on...

	return CEntityBuilder::InterpretTable();
}


BOOL CMeshEntityBuilder::PostInterpretFixup( void ) {
	FGameData_VarType_e nVarType;
	cchar *pszString;

	FASSERT( CEntityParser::m_pWorldShapeInit->m_nShapeType == FWORLD_SHAPETYPE_MESH );
	CFWorldShapeMesh *pShapeMesh = CEntityParser::m_pWorldShapeInit->m_pMesh;

	if( !CEntityBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	if( m_fCullDist2 < 0.0f ) {
		// Cull distance not specified in user props. Get from shape...
		m_fCullDist2 = pShapeMesh->m_fCullDist2;
	} else if( m_fCullDist2 == 0.0f ) {
		m_fCullDist2 = FMATH_MAX_FLOAT;
	}

	if( !(m_nFlags & FLAG_POSTERX_COMMAND_SPECIFIED) ) {
		// The "PosterX" command was not specified in user props. Get from shape...
		m_nMeshFlags |= (pShapeMesh->m_nMeshInstFlags & FMESHINST_FLAG_POSTER_X);
	}

	if( !(m_nFlags & FLAG_POSTERY_COMMAND_SPECIFIED) ) {
		// The "PosterY" command was not specified in user props. Get from shape...
		m_nMeshFlags |= (pShapeMesh->m_nMeshInstFlags & FMESHINST_FLAG_POSTER_Y);
	}

	if( !(m_nFlags & FLAG_POSTERZ_COMMAND_SPECIFIED) ) {
		// The "PosterZ" command was not specified in user props. Get from shape...
		m_nMeshFlags |= (pShapeMesh->m_nMeshInstFlags & FMESHINST_FLAG_POSTER_Z);
	}

	if( !(m_nFlags & FLAG_NOCOLL_COMMAND_SPECIFIED) ) {
		// The "NoColl" command was not specified in user props. Get from shape...
		m_nMeshFlags |= (pShapeMesh->m_nMeshInstFlags & FMESHINST_FLAG_NOCOLLIDE);
	}

	if( !(m_nFlags & FLAG_NOLIGHT_COMMAND_SPECIFIED) ) {
		// The "NoLight" command was not specified in user props. Get from shape...
		m_nMeshFlags |= (pShapeMesh->m_nMeshInstFlags & FMESHINST_FLAG_NOLIGHT);
	}

	// Add in any additional artist/designer specified mesh flags (these could have csv or user data overrides, as above, if so desired)
	m_nMeshFlags |= (pShapeMesh->m_nMeshInstFlags & FMESHINST_FLAG_DONT_DRAW);
	m_nMeshFlags |= (pShapeMesh->m_nMeshInstFlags & FMESHINST_FLAG_STATIC);
	m_nMeshFlags |= (pShapeMesh->m_nMeshInstFlags & (FMESHINST_FLAG_LM|FMESHINST_FLAG_VERT_RADIOSITY));
	m_nMeshFlags |= (pShapeMesh->m_nMeshInstFlags & FMESHINST_FLAG_LIGHT_PER_PIXEL);
	m_nMeshFlags |= (pShapeMesh->m_nMeshInstFlags & FMESHINST_FLAG_NOCOLLIDE);
	m_nMeshFlags |= (pShapeMesh->m_nMeshInstFlags & FMESHINST_FLAG_CAST_SHADOWS);
	m_nMeshFlags |= (pShapeMesh->m_nMeshInstFlags & FMESHINST_FLAG_ACCEPT_SHADOWS);
	m_nMeshFlags |= (pShapeMesh->m_nMeshInstFlags & FMESHINST_FLAG_TINT);
	m_nMeshFlags |= (pShapeMesh->m_nMeshInstFlags & FMESHINST_FLAG_NOSHADOW);

	// If the tint flag is set, use the artist/designer specified tint...
	if( m_nMeshFlags & FMESHINST_FLAG_TINT ) {
		m_ColorTint = pShapeMesh->m_TintRGB;
	}

	// Handle static lighting
	if ( pShapeMesh->m_papszLightMapName[0] )	{
		m_papszLightMapNames = pShapeMesh->m_papszLightMapName;
		m_panLightMapMotifs = pShapeMesh->m_anLightMapMotif;
	} else if ( pShapeMesh->m_nColorStreamCount ) {
		m_paColorStreams = pShapeMesh->m_paColorStreams;
		m_nColorStreamCount = pShapeMesh->m_nColorStreamCount;
	}

	if( m_ppszMeshNames == NULL ) {
		// No meshes were specified in user props. Get from shape...

		m_nMeshCount = 1;
		m_ppszMeshNames = &pShapeMesh->m_pszMeshName;
	} else {
		// One or more mesh names were specified...

		CEntityParser::SetTable( (FGameDataTableHandle_t)m_ppszMeshNames );

		if( !fclib_stricmp( CEntityParser::m_pszTableName, "Mesh" ) ) {
			// List of mesh names provided...

			if( !CEntityParser::Interpret_StringArray_BuildInFMem( &m_ppszMeshNames ) ) {
				// Out of memory...
				goto _ExitWithError;
			}

			m_nMeshCount = CEntityParser::m_nFieldCount;

		} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "MeshSeq" ) ) {
			// Mesh sequence provided...

			FASSERT( CEntityParser::m_nFieldCount == 1 );

			*m_ppszMeshNames = (cchar *)fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, 0, nVarType );
			FASSERT( nVarType == FGAMEDATA_VAR_TYPE_STRING );

			// Set wildcard flag...
			m_nMeshCount = -1;

		} else {
			FASSERT_NOW;
		}
	}

	if( m_ppszUserAnimNames ) {
		// One or more animation names have been specified...

		CEntityParser::SetTable( (FGameDataTableHandle_t)m_ppszUserAnimNames );
		FASSERT( CEntityParser::m_nFieldCount >= 1 );

		pszString = (cchar *)fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, 0, nVarType );
		FASSERT( nVarType == FGAMEDATA_VAR_TYPE_STRING );

		if( !fclib_stricmp( pszString, "None" ) ) {
			if( CEntityParser::m_nFieldCount != 1 ) {
				CEntityParser::Error_InvalidParameterCount();
			}

			m_ppszUserAnimNames = NULL;
			m_nUserAnimCount = 0;
		} else {
			if( !fclib_stricmp( CEntityParser::m_pszTableName, "Anim" ) ) {
				// List of animation names provided...

				if( !CEntityParser::Interpret_StringArray_BuildInFMem( &m_ppszUserAnimNames ) ) {
					// Out of memory...
					goto _ExitWithError;
				}

				m_nUserAnimCount = CEntityParser::m_nFieldCount;

			} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "AnimSeq" ) ) {
				// Animation sequence provided...

				FASSERT( CEntityParser::m_nFieldCount == 1 );

				*m_ppszUserAnimNames = (cchar *)fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, 0, nVarType );
				FASSERT( nVarType == FGAMEDATA_VAR_TYPE_STRING );

				// Set wildcard flag...
				m_nUserAnimCount = -1;

			} else {
				FASSERT_NOW;
			}
		}
	}

	return TRUE;

_ExitWithError:
	return FALSE;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CMeshEntity
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CMeshEntity::m_bSystemInitialized;


BOOL CMeshEntity::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );

	m_bSystemInitialized = TRUE;

	return TRUE;
}


void CMeshEntity::UninitSystem( void ) {
	if( m_bSystemInitialized ) {
		m_bSystemInitialized = FALSE;
	}
}


CMeshEntity::CMeshEntity() {
	_ClearDataMembers();
}


CMeshEntity::~CMeshEntity() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CMeshEntity::Create( cchar *pszMeshName, cchar *pszAnimName, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName, BOOL bUseAnimBlending, const CFVerlet::Init_t *pVerletInit ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( pszMeshName );

	if( pszAnimName ) {
		return Create( 1, &pszMeshName, 1, &pszAnimName, pszEntityName, pMtx, pszAIBuilderName, bUseAnimBlending, pVerletInit );
	} else {
		return Create( 1, &pszMeshName, 0, NULL, pszEntityName, pMtx, pszAIBuilderName, bUseAnimBlending, pVerletInit );
	}
}


BOOL CMeshEntity::Create( cchar *pszMeshName, u32 nAnimCount, cchar **ppszAnimNames, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName, BOOL bUseAnimBlending, const CFVerlet::Init_t *pVerletInit ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( pszMeshName );

	if( nAnimCount == 0 ) {
		return Create( 1, &pszMeshName, 0, NULL, pszEntityName, pMtx, pszAIBuilderName, bUseAnimBlending, pVerletInit );
	} else {
		FASSERT( ppszAnimNames );
		return Create( 1, &pszMeshName, nAnimCount, ppszAnimNames, pszEntityName, pMtx, pszAIBuilderName, bUseAnimBlending, pVerletInit );
	}
}


BOOL CMeshEntity::Create( u32 nMeshCount, cchar **ppszMeshNames, u32 nAnimCount, cchar **ppszAnimNames, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName, BOOL bUseAnimBlending, const CFVerlet::Init_t *pVerletInit ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( nMeshCount && ppszMeshNames );

	// Get pointer to the leaf class's builder object...
	CMeshEntityBuilder *pBuilder = (CMeshEntityBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_MeshEntityBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_MESH );
	}

	// Set our builder parameters...
	pBuilder->m_nMeshCount = nMeshCount;
	pBuilder->m_ppszMeshNames = ppszMeshNames;
	pBuilder->m_nUserAnimCount = nAnimCount;
	pBuilder->m_ppszUserAnimNames = ppszAnimNames;

	pBuilder->m_bUseAnimBlending = bUseAnimBlending;

	if( pVerletInit ) {
		pBuilder->m_bVerletEnabled = TRUE;
		fang_MemCopy( &pBuilder->m_VerletInit, pVerletInit, sizeof( CFVerlet::Init_t ) );
	}

	// Make sure we don't call the Verlet fixup function. We assume that since
	// the mesh entity is being created by a programmer (as opposed to the art path),
	// the programmer will be initializing the Verlet object.
	FMATH_SETBITMASK( m_nMeshEntityFlags, _ME_FLAG_NO_VERLET_INIT );

	// Create an entity...
	return CEntity::Create( pszEntityName, pMtx, pszAIBuilderName );
}


void CMeshEntity::ClassHierarchyDestroy( void ) {
	u32 i;

	fdelete( m_pVerlet );

	fdelete_array( m_pWorldMeshArray );

	if( m_ppUserAnim_InstArray ) {
		for( i=0; i<m_nUserAnim_InstCount; i++ ) {
			fdelete( m_ppUserAnim_InstArray[i] );
			m_ppUserAnim_InstArray[i] = NULL;
		}
	}

	fdelete_array( m_pUserAnim_CombinerArray );

	_ClearDataMembers();

	CEntity::ClassHierarchyDestroy();
}


BOOL CMeshEntity::ClassHierarchyBuild( void ) {
	FMeshInit_t MeshInit;
	FAnim_t *pAnimRes;
	u32 i;

	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();
	FMemFrame_t MemFrame = fmem_GetFrame();

	// Get pointer to the leaf class's builder object...
	CMeshEntityBuilder *pBuilder = (CMeshEntityBuilder *)GetLeafClassBuilder();

	// Build parent class...
	if( !CEntity::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Determine mesh and animation lists...
	if( !_DetermineList( FMESH_RESTYPE, &pBuilder->m_nMeshCount, &pBuilder->m_ppszMeshNames ) ) {
		// Out of memory...
		goto _ExitWithError;
	}
	if( !_DetermineList( FANIM_RESNAME, &pBuilder->m_nUserAnimCount, &pBuilder->m_ppszUserAnimNames ) ) {
		// Out of memory...
		goto _ExitWithError;
	}

	// Initialize from builder object...
	m_nWorldMeshCount = pBuilder->m_nMeshCount;
	m_nUserAnim_InstCount = pBuilder->m_nUserAnimCount;

	m_nMeshEntityFlags |= _ME_FLAG_USEANIMBLENDING * pBuilder->m_bUseAnimBlending;

	// Allocate external combiner pointer array...
	m_ppExtAnimCombinerArray = (CFAnimCombiner **)fres_AllocAndZero( sizeof(CFAnimCombiner *) * m_nWorldMeshCount );
	if( m_ppExtAnimCombinerArray == NULL ) {
		DEVPRINTF( "CMeshEntity::Create(): Not enough memory to create m_ppExtAnimCombinerArray.\n" );
		goto _ExitWithError;
	}

	// Build animation combiners if we have any user anims to manage...
	if( m_nUserAnim_InstCount ) {
		// Construct a simple animation combiner for each mesh...

		m_pUserAnim_CombinerArray = fnew CFAnimCombiner [m_nWorldMeshCount];

		if( m_pUserAnim_CombinerArray == NULL ) {
			DEVPRINTF( "CMeshEntity::Create(): Not enough memory to create m_pUserAnim_CombinerArray.\n" );
			goto _ExitWithError;
		}

		m_pnUserAnim_CombinerTapIDArray = (s32 *)fres_Alloc( m_nWorldMeshCount * sizeof(s32) );

		if( m_pnUserAnim_CombinerTapIDArray == NULL ) {
			DEVPRINTF( "CMeshEntity::Create(): Not enough memory to create m_pnUserAnim_CombinerTapIDArray.\n" );
			goto _ExitWithError;
		}

		if( UsesAnimBlending() ) {
			m_pnUserAnim_CombinerTapIDArray2 = (s32 *)fres_Alloc( m_nWorldMeshCount * sizeof(s32) );

			if( m_pnUserAnim_CombinerTapIDArray2 == NULL ) {
				DEVPRINTF( "CMeshEntity::Create(): Not enough memory to create m_pnUserAnim_CombinerTapIDArray2.\n" );
				goto _ExitWithError;
			}

			fang_MemSet( m_pnUserAnim_CombinerTapIDArray2, -1, m_nWorldMeshCount * sizeof(s32) );

			m_pnUserAnim_CombinerControlIDArray = (s32 *)fres_Alloc( m_nWorldMeshCount * sizeof(s32) );

			if( m_pnUserAnim_CombinerControlIDArray == NULL ) {
				DEVPRINTF( "CMeshEntity::Create(): Not enough memory to create m_pnUserAnim_CombinerControlIDArray.\n" );
				goto _ExitWithError;
			}

			fang_MemSet( m_pnUserAnim_CombinerControlIDArray, -1, m_nWorldMeshCount * sizeof(s32) );
		}

		fang_MemSet( m_pnUserAnim_CombinerTapIDArray, -1, m_nWorldMeshCount * sizeof(s32) );
	}

	m_pWorldMeshArray = fnew CFWorldMesh [m_nWorldMeshCount];
	if( m_pWorldMeshArray == NULL ) {
		DEVPRINTF( "CMeshEntity::Create(): Not enough memory to allocate m_pWorldMeshArray.\n" );
		goto _ExitWithError;
	}

	// Build meshes...
	if( pBuilder->m_fCullDist2 < 0.0f ) {
		pBuilder->m_fCullDist2 = FMATH_MAX_FLOAT;
	}
	MeshInit.fCullDist = fmath_Sqrt( pBuilder->m_fCullDist2 );
	MeshInit.Mtx.Set( pBuilder->m_EC_Mtx_WS );
	MeshInit.Mtx.m_vRight.Mul( pBuilder->m_fEC_Scale_WS );
	MeshInit.Mtx.m_vUp.Mul( pBuilder->m_fEC_Scale_WS );
	MeshInit.Mtx.m_vFront.Mul( pBuilder->m_fEC_Scale_WS );
	MeshInit.nFlags = pBuilder->m_nMeshFlags;

	for( i=0; i<m_nWorldMeshCount; i++ ) {
		MeshInit.pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pBuilder->m_ppszMeshNames[i] );

		if( MeshInit.pMesh == NULL ) {
			DEVPRINTF( "CMeshEntity::Create(): Could not find mesh '%s'.\n", pBuilder->m_ppszMeshNames[i] );

			if( m_pErrorMesh ) {
				MeshInit.pMesh = m_pErrorMesh;
			} else {
				goto _ExitWithError;
			}
		}

		m_pWorldMeshArray[i].Init( &MeshInit, TRUE );
		m_pWorldMeshArray[i].RemoveFromWorld();
		m_pWorldMeshArray[i].SetCollisionFlag( !(m_pWorldMeshArray[i].m_nFlags & FMESHINST_FLAG_NOCOLLIDE) );
		m_pWorldMeshArray[i].m_nUser = MESHTYPES_ENTITY;
		m_pWorldMeshArray[i].m_pUser = this;
		m_pWorldMeshArray[i].SetUserTypeBits( TypeBits() );

		if( pBuilder->m_nMeshFlags & FMESHINST_FLAG_TINT ) {
			// This object has been tinted in the tools...
			m_pWorldMeshArray[i].SetMeshTint( pBuilder->m_ColorTint.fRed, pBuilder->m_ColorTint.fGreen, pBuilder->m_ColorTint.fBlue );
		}
		// We can only apply static lighting data to the first mesh in a series since this is the mesh that
		// was used to perform the lighting in the tools. 
		if ( i == 0 ) {
			if ( pBuilder->m_papszLightMapNames ) {
				// This object has instance lightmaps
				m_pWorldMeshArray[i].SetLightmaps( pBuilder->m_papszLightMapNames, pBuilder->m_panLightMapMotifs, FWORLD_MAX_MESH_SHAPE_LIGHTMAPS );
			}
			if ( pBuilder->m_nColorStreamCount ) {
				// This object has instance color streams
				m_pWorldMeshArray[i].SetColorStreams( pBuilder->m_nColorStreamCount, pBuilder->m_paColorStreams );
			}
		}
	}

	if( pBuilder->m_nStartMeshIndex > 0 ) {
		--pBuilder->m_nStartMeshIndex;
	}

	m_nCurrentWorldMeshIndex = pBuilder->m_nStartMeshIndex < m_nWorldMeshCount ? pBuilder->m_nStartMeshIndex : 0;
	m_pCurrentWorldMesh = &m_pWorldMeshArray[m_nCurrentWorldMeshIndex];
	m_pCurrentExtAnimCombiner = m_ppExtAnimCombinerArray[m_nCurrentWorldMeshIndex];
	m_pUserAnim_CurrentCombiner = m_nUserAnim_InstCount ? &m_pUserAnim_CombinerArray[m_nCurrentWorldMeshIndex] : NULL;

	// Build user animation data...
	if( m_nUserAnim_InstCount ) {
		m_ppUserAnim_InstArray = (CFAnimInst **)fres_AllocAndZero( m_nUserAnim_InstCount * sizeof(CFAnimInst *) );

		if( m_ppUserAnim_InstArray == NULL ) {
			DEVPRINTF( "CMeshEntity::Create(): Not enough memory to allocate m_ppUserAnim_InstArray.\n" );
			goto _ExitWithError;
		}

		for( i=0; i<m_nUserAnim_InstCount; i++ ) {
			pAnimRes = (FAnim_t *)fresload_Load( FANIM_RESNAME, pBuilder->m_ppszUserAnimNames[i] );

			if( pAnimRes == NULL ) {
				// Could not load animation resource...

				if( pBuilder->m_pszEC_EntityName ) {
					DEVPRINTF( "CMeshEntity::Create(): Could not find animation '%s' for entity '%s'.\n", pBuilder->m_ppszUserAnimNames[i], pBuilder->m_pszEC_EntityName );
				} else {
					DEVPRINTF( "CMeshEntity::Create(): Could not find animation '%s'.\n", pBuilder->m_ppszUserAnimNames[i] );
				}
			} else {
				// Animation resource loaded...

				m_ppUserAnim_InstArray[i] = fnew CFAnimInst;

				if( m_ppUserAnim_InstArray[i] == NULL ) {
					DEVPRINTF( "CMeshEntity::Create(): Not enough memory to create m_ppUserAnim_InstArray.\n" );
					goto _ExitWithError;
				}

				if( !m_ppUserAnim_InstArray[i]->Create( pAnimRes ) ) {
					DEVPRINTF( "CMeshEntity::Create(): Trouble creating CFAnimInst for '%s'\n", pBuilder->m_ppszUserAnimNames[i] );
					goto _ExitWithError;
				}
			}
		}

		// Create one simple animation combiner for each mesh...
		for( i=0; i<m_nWorldMeshCount; i++ ) {
			if( m_ppUserAnim_InstArray[0] ) {
				if( !UsesAnimBlending() ) {
					if( !m_pUserAnim_CombinerArray[i].CreateSimple( m_ppUserAnim_InstArray[0], &m_pWorldMeshArray[i] ) ) {
						DEVPRINTF( "CMeshEntity::Create(): Could not create simple animation combiner.\n" );
						goto _ExitWithError;
					}

					m_pnUserAnim_CombinerTapIDArray[i] = m_pUserAnim_CombinerArray[i].GetTapID( FANIM_SIMPLE_COMBINER_CONFIG_TAPNAME );
					FASSERT( m_pnUserAnim_CombinerTapIDArray[i] >= 0 );
				} else {
					if( !m_pUserAnim_CombinerArray[i].CreateBlender( &m_pWorldMeshArray[i] ) ) {
						DEVPRINTF( "CMeshEntity::Create(): Could not create blender animation combiner.\n" );
						goto _ExitWithError;
					}

					m_pnUserAnim_CombinerTapIDArray[i] = m_pUserAnim_CombinerArray[i].GetTapID( FANIM_BLENDER_COMBINER_CONFIG_TAPNAME1 );
					FASSERT( m_pnUserAnim_CombinerTapIDArray[i] >= 0 );
					m_pnUserAnim_CombinerTapIDArray2[i] = m_pUserAnim_CombinerArray[i].GetTapID( FANIM_BLENDER_COMBINER_CONFIG_TAPNAME2 );
					FASSERT( m_pnUserAnim_CombinerTapIDArray2[i] >= 0 );
					m_pnUserAnim_CombinerControlIDArray[i] = m_pUserAnim_CombinerArray[i].GetControlID( FANIM_BLENDER_COMBINER_CONFIG_MIXERNAME );
					FASSERT( m_pnUserAnim_CombinerControlIDArray[i] >= 0 );

					m_pUserAnim_CombinerArray[i].AttachToTap( m_pnUserAnim_CombinerTapIDArray[i], m_ppUserAnim_InstArray[0] );
					m_pUserAnim_CombinerArray[i].AttachToTap( m_pnUserAnim_CombinerTapIDArray2[i], NULL );
					m_pUserAnim_CombinerArray[i].SetControlValue( m_pnUserAnim_CombinerControlIDArray[i], 0.0f );
				}
			}
		}

		FMATH_SETBITMASK( m_nMeshEntityFlags, _ME_FLAG_ANIM_DRIVING_MESH );

		if( pBuilder->m_nStartAnimIndex > 0 ) {
			--pBuilder->m_nStartAnimIndex;
		}

		if( pBuilder->m_nStartAnimIndex >= 0 ) {
			m_nUserAnim_CurrentInstIndex = pBuilder->m_nStartAnimIndex < m_nUserAnim_InstCount ? pBuilder->m_nStartAnimIndex : 0;
			m_pUserAnim_CurrentInst = m_ppUserAnim_InstArray[m_nUserAnim_CurrentInstIndex];
		} else {
			m_nUserAnim_CurrentInstIndex = -1;
			m_pUserAnim_CurrentInst = NULL;
		}

		if( m_pUserAnim_CurrentInst ) {
			if( m_pCurrentExtAnimCombiner == NULL ) {
				m_pUserAnim_CurrentCombiner->AttachToTap( m_pnUserAnim_CombinerTapIDArray[m_nCurrentWorldMeshIndex], m_pUserAnim_CurrentInst );
			}
		}
	} else {
		// No animations...
		m_nUserAnim_CurrentInstIndex = -1;
		m_pUserAnim_CurrentInst = NULL;
	}

	// Init mesh flipping stuff...
	m_fMeshFlipsPerSec = pBuilder->m_fMeshFlipsPerSec;
	m_nMeshFlipDir = pBuilder->m_nMeshFlipDir;
	m_nMeshFlipIndex = m_nCurrentWorldMeshIndex;
	m_fMeshFlipIndex = (f32)m_nMeshFlipIndex;

	// Init anim flipping stuff...
	m_nUserAnim_Flip = pBuilder->m_nAnimFlip;
	m_fUserAnim_SpeedMult = pBuilder->m_fAnimSpeedMult;
	m_nMeshEntityFlags &= ~_ME_FLAG_USEANIMBLENDING;

	_UpdateWorkFcn();

	if( m_pUserAnim_CurrentInst && (pBuilder->m_nFlags & CMeshEntityBuilder::FLAG_DRIVE_MESH_WITH_ANIM) ) {
		DriveMeshWithAnim( TRUE );
	}

	if( m_pUserAnim_CurrentInst && IsAnimDrivingMesh() ) {
		ClearMeshInstFlags( FMESHINST_FLAG_NOBONES );
	} else {
		SetMeshInstFlags( FMESHINST_FLAG_NOBONES );
	}

	// Initialize matrix palette (this is important for attached entities)...
	for( i=0; i<m_pCurrentWorldMesh->m_pMesh->nBoneCount; i++ ) {
		if( m_pCurrentWorldMesh->m_pMesh->pBoneArray[i].nFlags & FMESH_BONEFLAG_SKINNEDBONE ) {
			*m_pCurrentWorldMesh->GetBoneMtxPalette()[i] = m_pCurrentWorldMesh->m_Xfm.m_MtxF;
		} else {
			m_pCurrentWorldMesh->GetBoneMtxPalette()[i]->Mul( m_pCurrentWorldMesh->m_Xfm.m_MtxF, m_pCurrentWorldMesh->m_pMesh->pBoneArray[i].AtRestBoneToModelMtx );
		}
	}

	// Set up tripwire handling...
	if( IsTripwire() ) {
		// We're a tripwire. It's nice to be special!

		m_pTripwire->m_BoundingSphere_MS.m_Pos.Zero();
		m_pTripwire->m_BoundingSphere_MS.m_fRadius = GetMeshInst()->m_BoundSphere_MS.m_fRadius;
		SetCollisionFlag(FALSE);
	}

	if( pBuilder->m_nFlags & CMeshEntityBuilder::FLAG_ALWAYS_WORK ) {
		m_nMeshEntityFlags |= _ME_FLAG_ALWAYS_WORK;
	}

	if( pBuilder->m_nFlags & CMeshEntityBuilder::FLAG_VEHICLE_COLL_ONLY ) {
		m_nMeshEntityFlags |= _ME_FLAG_VEHICLE_COLL_ONLY;
	}

	if( pBuilder->m_nFlags & CMeshEntityBuilder::FLAG_LOADER_PICKUP_ENABLED ) {
		m_nMeshEntityFlags |= _ME_FLAG_LOADER_PICKUP_ENABLED;
	}

	if( pBuilder->m_nFlags & CMeshEntityBuilder::FLAG_AI_AVOID ) {
		m_nMeshEntityFlags |= _ME_FLAG_AI_AVOID;
	}

	if( pBuilder->m_nFlags & CMeshEntityBuilder::FLAG_TRANSLATOR_BONE ) {
		m_pTranslatorReferenceFrame = fnew CFMtx43A;
		if( m_pTranslatorReferenceFrame == NULL ) {
			DEVPRINTF( "CMeshEntity::ClassHierarchyBuild(): Not enough memory to allocate m_pTranslatorReferenceFrame.\n" );
			goto _ExitWithError;
		}

		m_nMeshEntityFlags |= _ME_FLAG_TRANSLATOR_BONE;

		_SetTranslatorInfoForCurrentMesh( NULL );

		*m_pTranslatorReferenceFrame = m_MtxToWorld;
	}

	if( pBuilder->m_bVerletEnabled ) {
		// Build a Verlet physics object...

		m_pVerlet = fnew CFVerlet;
		if( m_pVerlet == NULL ) {
			DEVPRINTF( "CMeshEntity::ClassHierarchyBuild(): Not enough memory to allocate a Verlet physics object.\n" );
			goto _ExitWithError;
		}

		CFVec3A VerletDim, *pVerletDim;
		if( pBuilder->m_VerletInit.fDimX<0.0f || pBuilder->m_VerletInit.fDimY<0.0f || pBuilder->m_VerletInit.fDimZ<0.0f ) {
			pVerletDim = NULL;
		} else {
			VerletDim.Set( pBuilder->m_VerletInit.fDimX, pBuilder->m_VerletInit.fDimY, pBuilder->m_VerletInit.fDimZ );
			pVerletDim = &VerletDim;
		}

		if( !m_pVerlet->Create( pBuilder->m_VerletInit.fMass, pVerletDim, m_pCurrentWorldMesh->m_pMesh, &m_MtxToWorld ) ) {
			DEVPRINTF( "CMeshEntity::ClassHierarchyBuild(): Could not create Verlet physics object.\n" );
			goto _ExitWithError;
		}

		if( !(m_nMeshEntityFlags & _ME_FLAG_NO_VERLET_INIT) ) {
			if( !pBuilder->InitVerlet( m_pVerlet ) ) {
				goto _ExitWithError;
			}
		}

		m_pVerlet->m_pUser = this;
		m_pVerlet->SetWorldMesh( m_pCurrentWorldMesh );

		m_pVerlet->SetMovedCallback( _VerletMovedCallback );
	}

	// Success...

	fmem_ReleaseFrame( MemFrame );

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fmem_ReleaseFrame( MemFrame );
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


BOOL CMeshEntity::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CEntity::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


CEntityBuilder *CMeshEntity::GetLeafClassBuilder( void ) {
	return &_MeshEntityBuilder;
}


void CMeshEntity::ClassHierarchyResolveEntityPointerFixups( void ) {
	CEntity::ClassHierarchyResolveEntityPointerFixups();

	// Reset our counters here...
	m_nLastAnimTicks = FLoop_nTotalLoopTicks;
	m_nLastFlipTicks = FLoop_nTotalLoopTicks;

	if( m_pVerlet ) {
		if( !(m_nMeshEntityFlags & _ME_FLAG_NO_VERLET_INIT) ) {
			CEntityBuilder::FixupVerlet( this, m_pVerlet );
		}
	}
}


BOOL CMeshEntity::_DetermineList( cchar *psResType, u32 *pnCount, cchar ***pppszNameArray ) {
	u32 nCount = *pnCount;
	u32 i, nStartSeqNumber, nSeqNumber;
	char c1, c10;

	if( nCount != -1 ) {
		// Specific number of things to load...
		return TRUE;
	}

	// Wildcard specified.
	// (*pppszNameArray)[0] points to the seed name...

	u32 nStringLen = fclib_strlen( (*pppszNameArray)[0] );

	if( nStringLen != 11 ) {
		// Wildcard not possible if not 11 characters...
		*pnCount = 1;
		return TRUE;
	}

	c10 = (*pppszNameArray)[0][ nStringLen - 2 ];
	c1 = (*pppszNameArray)[0][ nStringLen - 1 ];
	if( c1<'0' || c1>'9' || c10<'0' || c10>'9' ) {
		// Wildcard not possible unless last two digits are numbers...
		*pnCount = 1;
		return TRUE;
	}

	FMemFrame_t MemFrame = fmem_GetFrame();

	char *pszBaseName = (char *)fmem_Alloc( nStringLen + 1 );
	if( pszBaseName == NULL ) {
		goto _OutOfMemory;
	}

	fclib_strcpy( pszBaseName, (*pppszNameArray)[0] );

	// Compute starting sequence number...
	nStartSeqNumber = (c1-'0') + (c10-'0')*10;

	// Count number of meshes/anims...
	for( nCount=0, nSeqNumber=nStartSeqNumber; nSeqNumber<99; nCount++, nSeqNumber++ ) {
		pszBaseName[ nStringLen - 2 ] = (char)( ((nSeqNumber / 10) % 10) + '0' );
		pszBaseName[ nStringLen - 1 ] = (char)( (nSeqNumber % 10) + '0' );

		if( !fresload_ExistsOnDisk( psResType, pszBaseName ) ) {
			// Doesn't exist...
			break;
		}
	}

	if( nCount == 0 ) {
		// Could not find starting mesh/anim in sequence...

		fmem_ReleaseFrame( MemFrame );

		DEVPRINTF( "CMeshEntity::Create(): Could not find starting sequence name '%s'.\n", pszBaseName );

		if( fres_CompareTypes( psResType, FMESH_RESTYPE ) ) {
			*pnCount = 1;
			(*pppszNameArray)[0] = ENTITY_ERROR_MESH_NAME;
		} else {
			*pnCount = 0;
		}

		return TRUE;
	}

	*pppszNameArray = (cchar **)fmem_Alloc( sizeof(cchar *) * nCount );
	if( *pppszNameArray == NULL ) {
		goto _OutOfMemory;
	}

	for( i=0, nSeqNumber=nStartSeqNumber; i<nCount; i++, nSeqNumber++ ) {
		pszBaseName[ nStringLen - 2 ] = (char)( ((nSeqNumber / 10) % 10) + '0' );
		pszBaseName[ nStringLen - 1 ] = (char)( (nSeqNumber % 10) + '0' );

		(*pppszNameArray)[i] = (cchar *)fmem_Alloc( nStringLen + 1 );
		if( (*pppszNameArray)[i] == NULL ) {
			goto _OutOfMemory;
		}

		fclib_strcpy( (char *)(*pppszNameArray)[i], pszBaseName );
	}

	*pnCount = nCount;

	return TRUE;

_OutOfMemory:
	DEVPRINTF( "CMeshEntity::_DetermineList(): Out of memory.\n" );
	fmem_ReleaseFrame( MemFrame );
	return FALSE;
}


void CMeshEntity::_ClearDataMembers( void ) {
	m_nMeshEntityFlags = _ME_FLAG_NONE;

	m_nTranslatorBoneIndex = -1;
	m_pTranslatorReferenceFrame = NULL;

	m_nWorldMeshCount = 0;
	m_pWorldMeshArray = NULL;
	m_ppExtAnimCombinerArray = NULL;
	m_nCurrentWorldMeshIndex = 0;
	m_pCurrentWorldMesh = NULL;
	m_pCurrentExtAnimCombiner = NULL;

	m_pUserAnim_CurrentCombiner = NULL;
	m_pUserAnim_CombinerArray = NULL;
	m_pnUserAnim_CombinerTapIDArray = NULL;
	m_pnUserAnim_CombinerTapIDArray2 = NULL;
	m_pnUserAnim_CombinerControlIDArray = NULL;
	m_nUserAnim_InstCount = 0;
	m_ppUserAnim_InstArray = NULL;
	m_nUserAnim_CurrentInstIndex = -1;
	m_pUserAnim_CurrentInst = NULL;

	m_nLastAnimTicks = FLoop_nTotalLoopTicks;
	m_nLastFlipTicks = FLoop_nTotalLoopTicks;

	m_fMeshFlipsPerSec = 0.0f;
	m_nMeshFlipDir = DIR_NEXT;
	m_nMeshFlipIndex = 0;
	m_fMeshFlipIndex = 0.0f;

	m_nUserAnim_Flip = ANIMFLIP_OFF;
	m_fUserAnim_SpeedMult = 1.0f;

	m_fUnitAnimBlend = 0.0f;
	m_fOOBlendTimeSecs = 0.0f;
	m_pBlendOutAnimInst = NULL;

	m_pVerlet = NULL;

	m_pDieCallback = NULL;
	m_pDieUserParam = NULL;
}


void CMeshEntity::SelectMesh( u32 nMeshIndex, BOOL bResetIntraFlipAmount, BOOL bForceSelectMesh/* = FALSE*/ ) {
	FASSERT( IsCreated() );
	FASSERT( nMeshIndex>=0 && nMeshIndex<m_nWorldMeshCount );

	_SelectMesh( nMeshIndex, bForceSelectMesh );

	m_nMeshFlipIndex = nMeshIndex;

	if( bResetIntraFlipAmount ) {
		m_fMeshFlipIndex = (f32)m_nMeshFlipIndex;
	}

	m_nLastFlipTicks = FLoop_nTotalLoopTicks;
}


void CMeshEntity::_SelectMesh( u32 nMeshIndex, BOOL bForceSelectMesh/* = FALSE*/ ) {
	FASSERT(nMeshIndex < m_nWorldMeshCount);
	FASSERT(m_pWorldMeshArray != NULL);
	if( nMeshIndex != m_nCurrentWorldMeshIndex || bForceSelectMesh ) {
		CFWorldMesh *pPrevWorldMesh = m_pCurrentWorldMesh;

		FASSERT(nMeshIndex < m_nWorldMeshCount);
		FASSERT(m_pWorldMeshArray != NULL);

		m_pWorldMeshArray[nMeshIndex].m_Xfm = m_pCurrentWorldMesh->m_Xfm;
		m_pWorldMeshArray[nMeshIndex].m_fCullDist = m_pCurrentWorldMesh->m_fCullDist;
		m_pWorldMeshArray[nMeshIndex].m_nFlags = m_pCurrentWorldMesh->m_nFlags;
		m_pWorldMeshArray[nMeshIndex].m_pUser = m_pCurrentWorldMesh->m_pUser;
		m_pWorldMeshArray[nMeshIndex].m_nUser = m_pCurrentWorldMesh->m_nUser;
		m_pWorldMeshArray[nMeshIndex].SetCollisionFlag( m_pCurrentWorldMesh->IsCollisionFlagSet() );
		m_pWorldMeshArray[nMeshIndex].SetLineOfSightFlag( m_pCurrentWorldMesh->IsLineOfSightFlagSet() );
		m_pWorldMeshArray[nMeshIndex].SetID( m_pCurrentWorldMesh->GetID() );
		
		//Remove static lighting flags on mesh swaps otherwise the objects will look black - jc
		if ( nMeshIndex > 0 ) {
			m_pWorldMeshArray[nMeshIndex].m_nFlags &= ~(FMESHINST_FLAG_NOLIGHT_AMBIENT|FMESHINST_FLAG_NOLIGHT_DYNAMIC|FMESHINST_FLAG_LM|FMESHINST_FLAG_VERT_RADIOSITY);
		}

		FASSERT(nMeshIndex < m_nWorldMeshCount);
		FASSERT(m_pWorldMeshArray != NULL);

		m_pCurrentWorldMesh->RemoveFromWorld();

		m_nCurrentWorldMeshIndex = nMeshIndex;
		m_pCurrentWorldMesh = &m_pWorldMeshArray[nMeshIndex];
		m_pCurrentExtAnimCombiner = m_ppExtAnimCombinerArray[nMeshIndex];

		FASSERT(nMeshIndex < m_nWorldMeshCount);
		FASSERT(m_pWorldMeshArray != NULL);

		if( m_nUserAnim_InstCount ) {
			m_pUserAnim_CurrentCombiner = &m_pUserAnim_CombinerArray[m_nCurrentWorldMeshIndex];
		}

		FASSERT(nMeshIndex < m_nWorldMeshCount);
		FASSERT(m_pWorldMeshArray != NULL);
		FASSERT(m_pCurrentWorldMesh != NULL);

		if( IsInWorld() ) {
			m_pCurrentWorldMesh->AddToWorld();
		}

		if( m_pUserAnim_CurrentInst ) {
			// A user animation is currently selected...

			if( m_pCurrentExtAnimCombiner == NULL ) {
				// No external animation combiner specified. Use user animation combiner...

				if( m_pUserAnim_CurrentCombiner->GetSourceDrivingTap( m_pnUserAnim_CombinerTapIDArray[m_nCurrentWorldMeshIndex] ) != m_pUserAnim_CurrentInst ) {
					m_pUserAnim_CurrentCombiner->AttachToTap( m_pnUserAnim_CombinerTapIDArray[m_nCurrentWorldMeshIndex], m_pUserAnim_CurrentInst );
				}

				m_pUserAnim_CurrentCombiner->ComputeMtxPalette( FALSE );
			} else {
				m_pCurrentExtAnimCombiner->ComputeMtxPalette( FALSE );
			}
		} else {
			m_pCurrentWorldMesh->AtRestMtxPalette();
		}

		CEntity *pChild;
		const FMesh_t *pMesh = pPrevWorldMesh->m_pMesh;
		u32 nBoneCount = pMesh->nBoneCount;
		s32 nBoneIndex;

		for( pChild=GetFirstChild(); pChild; pChild=GetNextChild(pChild) ) {
			if( pChild->GetAttachBoneName() ) {
				// Child is attached to one of our bones...

				nBoneIndex = m_pCurrentWorldMesh->FindBone( pChild->GetAttachBoneName() );

				if( nBoneIndex >= 0 ) {
					// The same bone exists in the new mesh. Lets use it...
					pChild->m_pParentMtxToWorld = m_pCurrentWorldMesh->GetBoneMtxPalette()[nBoneIndex];
				} else {
					// The same bone doesn't exist in the new mesh. Let's use our model matrix...
					pChild->m_pParentMtxToWorld = &m_MtxToWorld;
				}
			}
		}

		RelocateAllChildren();

		if( m_pVerlet ) {
			m_pVerlet->SetWorldMesh( m_pCurrentWorldMesh );
		}

		_SetTranslatorInfoForCurrentMesh( pPrevWorldMesh );
	}
}


void CMeshEntity::_SetTranslatorInfoForCurrentMesh( CFWorldMesh *pPrevWorldMesh ) {
	if( m_nMeshEntityFlags & _ME_FLAG_TRANSLATOR_BONE ) {
		// Find the index of the first child of the bone with the name "Scene Root"...

		m_nTranslatorBoneIndex = m_pCurrentWorldMesh->FindBone( "Scene Root" );

		if( m_nTranslatorBoneIndex >= 0 ) {
			// Found bone "Scene Root"...

			if( m_pCurrentWorldMesh->m_pMesh->pBoneArray[m_nTranslatorBoneIndex].Skeleton.nChildBoneCount ) {
				// Bone "Scene Root" has a child...
				m_nTranslatorBoneIndex = m_pCurrentWorldMesh->m_pMesh->pnSkeletonIndexArray[ m_pCurrentWorldMesh->m_pMesh->pBoneArray[m_nTranslatorBoneIndex].Skeleton.nChildArrayStartIndex ];
			} else {
				// Bone "Scene Root" has no children...
				m_nTranslatorBoneIndex = -1;
			}
		}
	} else {
		m_nTranslatorBoneIndex = -1;
	}
}


void CMeshEntity::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CEntity::ClassHierarchyAddToWorld();
	m_pCurrentWorldMesh->UpdateTracker();

	if( AIShouldAvoid() ) {
		aigroup_RegisterForAIAvoidence( this );
	}
}


void CMeshEntity::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	CEntity::ClassHierarchyRemoveFromWorld();
	m_pCurrentWorldMesh->RemoveFromWorld();

	if( AIShouldAvoid() ) {
		aigroup_UnRegisterForAIAvoidence( this );
	}
}


void CMeshEntity::ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled ) {
	CEntity::ClassHierarchyDrawEnable( bDrawingHasBeenEnabled );

	u32 i;

	if( bDrawingHasBeenEnabled ) {
		// Enable drawing of this mesh...

		for( i=0; i<m_nWorldMeshCount; ++i ) {
			FMATH_CLEARBITMASK( m_pWorldMeshArray[i].m_nFlags, FMESHINST_FLAG_DONT_DRAW );
		}
	} else {
		// Disable drawing of this mesh...

		for( i=0; i<m_nWorldMeshCount; ++i ) {
			FMATH_SETBITMASK( m_pWorldMeshArray[i].m_nFlags, FMESHINST_FLAG_DONT_DRAW );
		}
	}
}


void CMeshEntity::ClassHierarchyRelocated( void *pIdentifier ) {
	CEntity::ClassHierarchyRelocated( pIdentifier );

	m_pCurrentWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );

	if( IsInWorld() ) {
		m_pCurrentWorldMesh->UpdateTracker();
	}

#if 1
	if( !IsAnimDrivingMesh() || ((m_pUserAnim_CurrentInst == NULL) && (m_pCurrentExtAnimCombiner == NULL)) ) {
		m_pCurrentWorldMesh->AtRestMtxPalette();
	}
#endif
}

u32 CMeshEntity::TripwireCollisionTest( const CFVec3A *pPrevPos_WS, const CFVec3A *pNewPos_WS ) {
	CFVec3A P1P2, P1ToCenter, P2ToCenter;
	f32 fV, fOOP1P2Dist2, fD2;
	BOOL bPrevIsInside, bNewIsInside;

	bPrevIsInside = bNewIsInside = FALSE;

	if( m_pTripwire->m_BoundingSphere_WS.IsIntersecting( *pPrevPos_WS ) ) {
		bPrevIsInside = TRUE;
	}

	if( m_pTripwire->m_BoundingSphere_WS.IsIntersecting( *pNewPos_WS ) ) {
		bNewIsInside = TRUE;
	}

	if( bPrevIsInside && bNewIsInside ) {
		// Previous and new points are both inside...
		return TRIPWIRE_COLLFLAG_NEWPOS_INSIDE;
	}

	if( bNewIsInside ) {
		// Only the new point is inside...
		return TRIPWIRE_COLLFLAG_NEWPOS_INSIDE | TRIPWIRE_COLLFLAG_ENTER_EVENT;
	}

	if( bPrevIsInside ) {
		// Only the previous point is inside...
		return TRIPWIRE_COLLFLAG_EXIT_EVENT;
	}

	// Both points are outside. We must perform a more expensive test...

	P1P2.Sub( *pNewPos_WS, *pPrevPos_WS );
	P1ToCenter.Sub( m_pTripwire->m_BoundingSphere_WS.m_Pos, *pPrevPos_WS );

	fV = P1ToCenter.Dot( P1P2 );
	if( fV <= 0.0f ) {
		// P1P2 is pointing away from the sphere (or P1 & P2 are the same point)...
		return TRIPWIRE_COLLFLAG_NONE;
	}

	P2ToCenter.Sub( m_pTripwire->m_BoundingSphere_WS.m_Pos, *pNewPos_WS );
	if( P2ToCenter.Dot( P1P2 ) >= 0.0f ) {
		// P1P2 is pointing away from the sphere...
		return TRIPWIRE_COLLFLAG_NONE;
	}

	fOOP1P2Dist2 = P1P2.InvMagSq();
	fD2 = m_pTripwire->m_BoundingSphere_WS.m_fRadius*m_pTripwire->m_BoundingSphere_WS.m_fRadius - ( P1ToCenter.MagSq() - fV*fV*fOOP1P2Dist2 );

	if( fD2 <= 0.0f ) {
		// Infinite line colinear with P1P2 doesn't intersect the sphere...
		return TRIPWIRE_COLLFLAG_NONE;
	}

	// Line segment intersects the sphere...

	return TRIPWIRE_COLLFLAG_ENTER_EVENT | TRIPWIRE_COLLFLAG_EXIT_EVENT;
}



BOOL CMeshEntity::GetCollected( CEntity *pCollector ) {
	// Check if it's a player bot.
	if( ( pCollector->TypeBits() & ENTITY_BIT_BOT ) && ( ((CBot *)(pCollector))->m_nPossessionPlayerIndex != -1 ) )
	{
		//CHud2::GetHudForPlayer(((CBot *)(pCollector))->m_nPossessionPlayerIndex)->PickupItemGeneric( GetMeshInst(), ITEMTYPE_WASHER, 1.0f );
		CCollectable::GiveToPlayer( (CBot *) pCollector, "washer", 1.0f );
	}

	return(TRUE);
}


void CMeshEntity::UserAnim_Select( s32 nUserAnimIndex ) {
	CFAnimInst *pPrevUserAnimInst;

	FASSERT( IsCreated() );
	FASSERT( nUserAnimIndex==-1 || nUserAnimIndex<(s32)m_nUserAnim_InstCount );

	if( nUserAnimIndex == m_nUserAnim_CurrentInstIndex ) {
		return;
	}

	m_nUserAnim_CurrentInstIndex = nUserAnimIndex;
	pPrevUserAnimInst = m_pUserAnim_CurrentInst;

	if( nUserAnimIndex != -1 ) {
		m_pUserAnim_CurrentInst = m_ppUserAnim_InstArray[m_nUserAnim_CurrentInstIndex];

		if( m_pUserAnim_CurrentInst ) {
			if( m_pCurrentExtAnimCombiner == NULL ) {
				// No external combiner provided. Use user combiner...

				if( m_pUserAnim_CurrentCombiner->GetSourceDrivingTap( m_pnUserAnim_CombinerTapIDArray[m_nCurrentWorldMeshIndex] ) != m_pUserAnim_CurrentInst ) {
					m_pUserAnim_CurrentCombiner->AttachToTap( m_pnUserAnim_CombinerTapIDArray[m_nCurrentWorldMeshIndex], m_pUserAnim_CurrentInst );
				}
			}

			if( pPrevUserAnimInst ) {
				m_pUserAnim_CurrentInst->UpdateUnitTime( pPrevUserAnimInst->GetUnitTime() );
			}

			m_nLastAnimTicks = FLoop_nTotalLoopTicks;
		}
	} else {
		m_pUserAnim_CurrentInst = NULL;
	}

	_UpdateWorkFcn();

	if( IsAnimDrivingMesh() ) {
		ClearMeshInstFlags( FMESHINST_FLAG_NOBONES );
	} else {
		SetMeshInstFlags( FMESHINST_FLAG_NOBONES );
	}
}


void CMeshEntity::UserAnim_SelectWithBlend( s32 nUserAnimIndex, f32 fBlendTimeSecs ) {
	CFAnimInst *pPrevUserAnimInst;

	FASSERT( IsCreated() );
	FASSERT( nUserAnimIndex==-1 || nUserAnimIndex<(s32)m_nUserAnim_InstCount );
	FASSERT( UsesAnimBlending() );

	if( nUserAnimIndex == m_nUserAnim_CurrentInstIndex ) {
		return;
	}

	m_nUserAnim_CurrentInstIndex = nUserAnimIndex;
	pPrevUserAnimInst = m_pUserAnim_CurrentInst;

	if( nUserAnimIndex != -1 ) {
		m_pUserAnim_CurrentInst = m_ppUserAnim_InstArray[m_nUserAnim_CurrentInstIndex];

		if( m_pUserAnim_CurrentInst ) {
			if( m_pCurrentExtAnimCombiner == NULL ) {
				// No external combiner provided. Use user combiner...

				if( m_pUserAnim_CurrentCombiner->GetSourceDrivingTap( m_pnUserAnim_CombinerTapIDArray[m_nCurrentWorldMeshIndex] ) != m_pUserAnim_CurrentInst ) {
					// Attach the new animation to the primary tap.
					m_pUserAnim_CurrentCombiner->AttachToTap( m_pnUserAnim_CombinerTapIDArray[m_nCurrentWorldMeshIndex], m_pUserAnim_CurrentInst );
					// Attach the old animation to the secondary tap.
					m_pUserAnim_CurrentCombiner->AttachToTap( m_pnUserAnim_CombinerTapIDArray2[m_nCurrentWorldMeshIndex], pPrevUserAnimInst );

					if (fBlendTimeSecs > 0.0f)
					{
						m_fOOBlendTimeSecs = 1.0f/fBlendTimeSecs;
					}
					else
					{
						m_fOOBlendTimeSecs = 1.0f;
					}
					m_fUnitAnimBlend = 1.0f - m_fUnitAnimBlend;
					m_pBlendOutAnimInst = pPrevUserAnimInst;
				}
			}

			if( pPrevUserAnimInst ) {
				m_pUserAnim_CurrentInst->UpdateUnitTime( pPrevUserAnimInst->GetUnitTime() );
			}

			m_nLastAnimTicks = FLoop_nTotalLoopTicks;
		}
	} else {
		m_pUserAnim_CurrentInst = NULL;
	}

	_UpdateWorkFcn();

	if( IsAnimDrivingMesh() ) {
		ClearMeshInstFlags( FMESHINST_FLAG_NOBONES );
	} else {
		SetMeshInstFlags( FMESHINST_FLAG_NOBONES );
	}
}


void CMeshEntity::UserAnim_UpdateTime( f32 fNewTime ) {
	FASSERT( IsCreated() );

	if( m_pUserAnim_CurrentInst ) {
		m_pUserAnim_CurrentInst->UpdateTime( fNewTime );
		m_nLastAnimTicks = FLoop_nTotalLoopTicks;
	}
}


void CMeshEntity::UserAnim_UpdateUnitTime( f32 fNewUnitTime ) {
	FASSERT( IsCreated() );

	if( m_pUserAnim_CurrentInst ) {
		m_pUserAnim_CurrentInst->UpdateUnitTime( fNewUnitTime );
		m_nLastAnimTicks = FLoop_nTotalLoopTicks;
	}
}


void CMeshEntity::UserAnim_Pause( BOOL bPause ) {
	FASSERT( IsCreated() );

	if( bPause ) {
		if( !UserAnim_IsPaused() ) {
			ComputeMtxPalette( FALSE );
			FMATH_SETBITMASK( m_nMeshEntityFlags, _ME_FLAG_USER_ANIM_PAUSED );
			_UpdateWorkFcn();
		}
	} else {
		if( UserAnim_IsPaused() ) {
			FMATH_CLEARBITMASK( m_nMeshEntityFlags, _ME_FLAG_USER_ANIM_PAUSED );
			_UpdateWorkFcn();
			m_nLastAnimTicks = FLoop_nTotalLoopTicks;
		}
	}
}


void CMeshEntity::DriveMeshWithAnim( BOOL bDrive ) {
	FASSERT( IsCreated() );

	if( bDrive ) {
		if( !IsAnimDrivingMesh() ) {
			FMATH_SETBITMASK( m_nMeshEntityFlags, _ME_FLAG_ANIM_DRIVING_MESH );
			_UpdateWorkFcn();

			if( m_pUserAnim_CurrentInst ) {
				ClearMeshInstFlags( FMESHINST_FLAG_NOBONES );

				if( m_pCurrentExtAnimCombiner == NULL ) {
					// A user combiner is driving our mesh...

					m_pUserAnim_CurrentCombiner->ComputeMtxPalette( FALSE );
				} else {
					// An external combiner is driving our mesh...

					m_pCurrentExtAnimCombiner->ComputeMtxPalette( FALSE );
				}
			} else {
				SetMeshInstFlags( FMESHINST_FLAG_NOBONES );
				m_pCurrentWorldMesh->AtRestMtxPalette();
			}

			RelocateAllChildren();
		}
	} else {
		if( IsAnimDrivingMesh() ) {
			FMATH_CLEARBITMASK( m_nMeshEntityFlags, _ME_FLAG_ANIM_DRIVING_MESH );
			_UpdateWorkFcn();
			SetMeshInstFlags( FMESHINST_FLAG_NOBONES );
		}
	}
}


void CMeshEntity::SetExternalCombiner( u32 nMeshIndex, CFAnimCombiner *pCombiner ) {
	FASSERT( IsCreated() );
	FASSERT( nMeshIndex < m_nWorldMeshCount );

	if( nMeshIndex == m_nCurrentWorldMeshIndex ) {
		if( m_ppExtAnimCombinerArray[nMeshIndex] && pCombiner==NULL ) {
			if( m_pUserAnim_CurrentInst ) {
				if( m_pUserAnim_CurrentCombiner->GetSourceDrivingTap( m_pnUserAnim_CombinerTapIDArray[m_nCurrentWorldMeshIndex] ) != m_pUserAnim_CurrentInst ) {
					m_pUserAnim_CurrentCombiner->AttachToTap( m_pnUserAnim_CombinerTapIDArray[m_nCurrentWorldMeshIndex], m_pUserAnim_CurrentInst );
				}
			}
		}

		m_pCurrentExtAnimCombiner = pCombiner;
	}

	m_ppExtAnimCombinerArray[nMeshIndex] = pCombiner;

	_UpdateWorkFcn();
}


void CMeshEntity::_UpdateWorkFcn( void ) {
	if(	(m_pUserAnim_CurrentInst && /*!UserAnim_IsPaused() &&*/ IsAnimDrivingMesh())
		|| (IsAnimDrivingMesh() && m_pCurrentExtAnimCombiner)
		|| (m_fMeshFlipsPerSec > 0.0f) ) {

		// We need our work to be called...
		EnableOurWorkBit();
	} else {
		// We don't need to do any work...
		DisableOurWorkBit();
	}
}


// Set fFlipsPerSec to >0.0f to flip at a rate of fFlipsPerSec flips per second.
// Set fFlipsPerSec to 0.0f for no flipping.
// Set fFlipsPerSec to <0.0f to flip when animation cycles
void CMeshEntity::MeshFlip_SetFlipsPerSec( f32 fFlipsPerSec ) {
	FASSERT( IsCreated() );

	m_fMeshFlipsPerSec = fFlipsPerSec;
}


void CMeshEntity::MeshFlip_SetFlipDirection( Dir_e nDirection ) {
	FASSERT( IsCreated() );
	FASSERT( nDirection>=0 && nDirection<DIR_COUNT );

	if( nDirection != m_nMeshFlipDir ) {
		if( (m_nMeshFlipDir==DIR_NEXT && nDirection==DIR_PREV) || (m_nMeshFlipDir==DIR_PREV && nDirection==DIR_NEXT) ) {
			if( m_fMeshFlipIndex > 0.0f ) {
				m_fMeshFlipIndex = (f32)m_nWorldMeshCount - m_fMeshFlipIndex;
			}
		}

		m_nMeshFlipDir = nDirection;
	}
}


CFMtx43A *CMeshEntity::ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName ) {
	s32 nBoneIndex;

	if( pszAttachBoneName == NULL /*||
		!IsAnimDrivingMesh()*/) {	   //pgm and Justin did this because non animating mesh entities will have unreliable bone mtx palettes
		// No bone to attach to...
		return NULL;
	}

	nBoneIndex = m_pCurrentWorldMesh->FindBone( pszAttachBoneName );

	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CMeshEntity::ClassHierarchyAttachChild(): Bone '%s' doesn't exist in object '%s'.\n", pszAttachBoneName, Name() );
		return NULL;
	}

	return m_pCurrentWorldMesh->GetBoneMtxPalette()[nBoneIndex];
}


void CMeshEntity::ClassHierarchyWork( void ) {
	f32 fElapsedSecs;
	u32 nPrevMeshFlipIndex, nRandomIndex;

	if( !(m_pCurrentWorldMesh->GetVolumeFlags() & FVIS_VOLUME_IN_ACTIVE_LIST) &&
		!( m_nMeshEntityFlags & _ME_FLAG_ALWAYS_WORK) ) {
		// We're not part of any active volume and the always work bit isn't set, 
		// so let's not do any work...
		return;
	}

	CEntity::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}

/*	if( m_fUnitAnimBlend > 0.0f ) {
		FASSERT( UsesAnimBlending() );
		m_fUnitAnimBlend -= m_fOOBlendTimeSecs * FLoop_fPreviousLoopSecs;

		FASSERT( m_pBlendOutAnimInst != NULL );
		m_pBlendOutAnimInst->DeltaTime( fElapsedSecs * m_fUserAnim_SpeedMult, UserAnim_GetClampMode() );
		FASSERT( m_nCurrentWorldMeshIndex != -1 );
		m_pUserAnim_CurrentCombiner->SetControlValue( m_pnUserAnim_CombinerControlIDArray[m_nCurrentWorldMeshIndex], m_fUnitAnimBlend );

		if( m_fUnitAnimBlend <= 0.0f ) {
			m_fUnitAnimBlend = 0.0f;
			m_fOOBlendTimeSecs = 0.0f;
			m_pBlendOutAnimInst = NULL;
		}
	}*/

	if( IsAnimDrivingMesh() ) {
		if( m_pCurrentExtAnimCombiner ) {
			// An external combiner is driving our mesh...

			m_pCurrentExtAnimCombiner->ComputeMtxPalette( FALSE );
			RelocateAllChildren();

		} else if( m_pUserAnim_CurrentInst ) {
			// A user combiner is driving our mesh...

			FASSERT( m_nUserAnim_CurrentInstIndex >= 0 );

			if( !UserAnim_IsPaused() ) {
				// Compute the elapsed time since we were last called...
				FMATH_CLAMPMAX( m_nLastAnimTicks, FLoop_nTotalLoopTicks );
				fElapsedSecs = (f32)(FLoop_nTotalLoopTicks - m_nLastAnimTicks) * FLoop_fSecsPerTick;
				m_nLastAnimTicks = FLoop_nTotalLoopTicks;

				if( m_pUserAnim_CurrentInst->DeltaTime( fElapsedSecs * m_fUserAnim_SpeedMult, UserAnim_GetClampMode() ) ) {
					// Animation wrapped...

					if( m_nUserAnim_InstCount > 1 ) {
						switch( m_nUserAnim_Flip ) {
						case ANIMFLIP_NEXT:
							// Goto next animation in list...

							if( ((u32)m_nUserAnim_CurrentInstIndex + 1) < m_nUserAnim_InstCount ) {
								UserAnim_Select( m_nUserAnim_CurrentInstIndex + 1 );
							} else {
								UserAnim_Select( 0 );
							}

							break;

						case ANIMFLIP_PREV:
							// Goto previous animation in list...

							if( m_nUserAnim_CurrentInstIndex ) {
								UserAnim_Select( m_nUserAnim_CurrentInstIndex - 1 );
							} else {
								UserAnim_Select( m_nUserAnim_InstCount - 1 );
							}

							break;

						case ANIMFLIP_RAND1:
							// Goto random animation in list...

							nRandomIndex = (u32)fmath_RandomRange( 0, m_nUserAnim_InstCount-1 );
							UserAnim_Select( nRandomIndex );

							break;

						case ANIMFLIP_RAND2:
							// Goto random animation in list with no repeat of previous value...

							if( m_nUserAnim_InstCount > 2 ) {
								nRandomIndex = (u32)fmath_RandomRange( 0, m_nUserAnim_InstCount-1 );

								if( nRandomIndex == (u32)m_nUserAnim_CurrentInstIndex ) {
									nRandomIndex = m_nUserAnim_CurrentInstIndex + 1;

									if( nRandomIndex >= m_nUserAnim_InstCount ) {
										nRandomIndex = 0;
									}
								}

								UserAnim_Select( nRandomIndex );
							} else {
								// m_nUserAnim_InstCount is equal to 2...
								FASSERT( m_nUserAnim_InstCount == 2 );
								UserAnim_Select( !m_nUserAnim_CurrentInstIndex );
							}

							break;
						}
					}

					if( m_fMeshFlipsPerSec < 0.0f ) {
						// Instructions are to flip the mesh when the animation loops...

						switch( m_nMeshFlipDir ) {
						case DIR_NEXT:
							if( (m_nCurrentWorldMeshIndex + 1) < m_nWorldMeshCount ) {
								_SelectMesh( m_nCurrentWorldMeshIndex + 1 );
							} else {
								_SelectMesh( 0 );
							}

							break;

						case DIR_PREV:
							if( m_nCurrentWorldMeshIndex ) {
								_SelectMesh( m_nCurrentWorldMeshIndex - 1 );
							} else {
								_SelectMesh( m_nWorldMeshCount - 1 );
							}

							break;

						case DIR_RAND1:
							_FlipMesh_Rand1();
							break;

						case DIR_RAND2:
							_FlipMesh_Rand2();
							break;

						default:
							FASSERT_NOW;
						}

						_MeshHasFlipped();
					}
				}

// Look at this classy Justin hack
if( m_fUnitAnimBlend > 0.0f ) {
	// There is some animation blending to be done.
	FASSERT( UsesAnimBlending() );
	m_fUnitAnimBlend -= m_fOOBlendTimeSecs * FLoop_fPreviousLoopSecs;

	FASSERT( m_pBlendOutAnimInst != NULL );
	m_pBlendOutAnimInst->DeltaTime( fElapsedSecs * m_fUserAnim_SpeedMult, UserAnim_GetClampMode() );
	FASSERT( m_nCurrentWorldMeshIndex != -1 );
	m_pUserAnim_CurrentCombiner->SetControlValue( m_pnUserAnim_CombinerControlIDArray[m_nCurrentWorldMeshIndex], fmath_UnitLinearToSCurve( m_fUnitAnimBlend ) );

	if( m_fUnitAnimBlend <= 0.0f ) {
		// We finished our animation blending this frame.
		m_fUnitAnimBlend = 0.0f;
		m_fOOBlendTimeSecs = 0.0f;
		m_pBlendOutAnimInst = NULL;
	}
}
			}

			if( m_nTranslatorBoneIndex < 0 ) {
				m_pUserAnim_CurrentCombiner->ComputeMtxPalette( FALSE );
				RelocateAllChildren();
			} else {
				CFMtx43A TranslatorBoneMtx_RS, TranslatorBoneMtx_WS;

				m_pUserAnim_CurrentCombiner->ComputeMtxPalette( FALSE, m_nTranslatorBoneIndex, &TranslatorBoneMtx_RS );

				TranslatorBoneMtx_WS.Mul( *m_pTranslatorReferenceFrame, TranslatorBoneMtx_RS );

				Relocate_RotXlatScaleFromScaledMtx_WS( &TranslatorBoneMtx_WS, 0.0f );
			}
		}
	}

	if( m_fMeshFlipsPerSec > 0.0f ) {
		if( m_nWorldMeshCount > 1 ) {
			// Compute the elapsed time since we were last called...
			FMATH_CLAMPMAX( m_nLastFlipTicks, FLoop_nTotalLoopTicks );
			fElapsedSecs = (f32)(FLoop_nTotalLoopTicks - m_nLastFlipTicks) * FLoop_fSecsPerTick;
			m_nLastFlipTicks = FLoop_nTotalLoopTicks;

			nPrevMeshFlipIndex = m_nMeshFlipIndex;

			m_fMeshFlipIndex += m_fMeshFlipsPerSec * fElapsedSecs;
			m_nMeshFlipIndex = (u8)( (u32)m_fMeshFlipIndex % (u32)m_nWorldMeshCount );
			m_fMeshFlipIndex = (f32)m_nMeshFlipIndex + (m_fMeshFlipIndex - (f32)((u32)m_fMeshFlipIndex) );

			if( m_nMeshFlipIndex != nPrevMeshFlipIndex ) {
				switch( m_nMeshFlipDir ) {
				case DIR_NEXT:
					_SelectMesh( m_nMeshFlipIndex );
					break;

				case DIR_PREV:
					_SelectMesh( m_nWorldMeshCount - m_nMeshFlipIndex - 1 );
					break;

				case DIR_RAND1:
					_FlipMesh_Rand1();
					break;

				case DIR_RAND2:
					_FlipMesh_Rand2();
					break;

				default:
					FASSERT_NOW;
				}

				_MeshHasFlipped();
			}
		}
	}
}


void CMeshEntity::ComputeMtxPalette( BOOL bApplyOffscreenOptimizations ) {
	FASSERT( IsCreated() );

	if( IsAnimDrivingMesh() ) {
		if( m_pCurrentExtAnimCombiner == NULL ) {
			// A user combiner is driving our mesh...

			m_pUserAnim_CurrentCombiner->ComputeMtxPalette(bApplyOffscreenOptimizations);
		} else {
			// An external combiner is driving our mesh...

			m_pCurrentExtAnimCombiner->ComputeMtxPalette(bApplyOffscreenOptimizations);
		}

		RelocateAllChildren();
	}
}


void CMeshEntity::_FlipMesh_Rand1( void ) {
	u32 nRandomIndex = (u32)fmath_RandomRange( 0, m_nWorldMeshCount-1 );
	_SelectMesh( nRandomIndex );
}


void CMeshEntity::_FlipMesh_Rand2( void ) {
	if( m_nWorldMeshCount > 2 ) {
		u32 nRandomIndex = (u32)fmath_RandomRange( 0, m_nWorldMeshCount-1 );

		if( nRandomIndex == m_nCurrentWorldMeshIndex ) {
			nRandomIndex = m_nCurrentWorldMeshIndex + 1;

			if( nRandomIndex >= m_nWorldMeshCount ) {
				nRandomIndex = 0;
			}
		}

		_SelectMesh( nRandomIndex );
	} else if( m_nWorldMeshCount == 2 ) {
		_SelectMesh( !m_nCurrentWorldMeshIndex );
	} else {
		FASSERT( m_nWorldMeshCount == 1 );
		_SelectMesh( 0 );
	}
}


void CMeshEntity::_MeshHasFlipped( void ) {
	if( m_nUserAnim_Flip == ANIMFLIP_MATCHMESH ) {
		if( m_nCurrentWorldMeshIndex < m_nUserAnim_InstCount ) {
			UserAnim_Select( m_nCurrentWorldMeshIndex );
		} else {
			if( m_nUserAnim_InstCount ) {
				UserAnim_Select( m_nUserAnim_InstCount - 1 );
			}
		}
	}
}


void CMeshEntity::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	FASSERT( IsCreated() );
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = m_pCurrentWorldMesh;
}


void CMeshEntity::CheckpointSaveSelect( s32 nCheckpoint ) {
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}


BOOL CMeshEntity::CheckpointSave( void ) {
	// save base class data
	CEntity::CheckpointSave();

	CFCheckPoint::SaveData( m_nLastAnimTicks );
	CFCheckPoint::SaveData( m_nLastFlipTicks );
	CFCheckPoint::SaveData( m_fUserAnim_SpeedMult );
	CFCheckPoint::SaveData( (u32&) m_nUserAnim_Flip );
	CFCheckPoint::SaveData( m_nMeshEntityFlags );
	CFCheckPoint::SaveData( m_nUserAnim_CurrentInstIndex );
	CFCheckPoint::SaveData( (u32&) m_pUserAnim_CurrentInst );
	CFCheckPoint::SaveData( m_nMeshFlipIndex);
	return TRUE;
}


void CMeshEntity::CheckpointRestore( void ) {
	// load base class data
	CEntity::CheckpointRestore();

	CFCheckPoint::LoadData( m_nLastAnimTicks );
	CFCheckPoint::LoadData( m_nLastFlipTicks );
	CFCheckPoint::LoadData( m_fUserAnim_SpeedMult );
	CFCheckPoint::LoadData( (u32&) m_nUserAnim_Flip );
	CFCheckPoint::LoadData( m_nMeshEntityFlags );
	CFCheckPoint::LoadData( m_nUserAnim_CurrentInstIndex );
	CFCheckPoint::LoadData( (u32&) m_pUserAnim_CurrentInst );
	u32 uMeshFlipIndex;
	CFCheckPoint::LoadData( uMeshFlipIndex);
	SelectMesh( uMeshFlipIndex, TRUE, TRUE );

	
}


void CMeshEntity::AlwaysWork( BOOL bAlways ) {
	if( bAlways ) {
		m_nMeshEntityFlags |= _ME_FLAG_ALWAYS_WORK;
	} else {
		m_nMeshEntityFlags &= ~_ME_FLAG_ALWAYS_WORK;
	}
}


void CMeshEntity::_VerletMovedCallback( CFVerlet *pVerlet, f32 fDeltaSecs ) {
	CMeshEntity *pMeshEntity = (CMeshEntity *)pVerlet->m_pUser;

	pMeshEntity->Relocate_RotXlatFromUnitMtx_WS( pVerlet->GetUnitMtx() );
}


void CMeshEntity::_DamageTack( CFVerletTack *pTack, CDamageData *pDamageData ) {
	CDamageResult DamageResult;

	pDamageData->ComputeDamageResult( (const CArmorProfile *)pTack->m_pUser3, &DamageResult, GetArmorModifier(), TRUE );

	f32 fAnchorUnitHealth = pTack->Anchor_GetUnitHealth();

	if( DamageResult.m_fDeltaHitpoints>0.0f && fAnchorUnitHealth<=0.0f ) {
		// Anchor health already at zero...
		return;
	} else if( DamageResult.m_fDeltaHitpoints<0.0f && fAnchorUnitHealth>=1.0f ) {
		// Anchor health already at max...
		return;
	}

	fAnchorUnitHealth -= DamageResult.m_fDeltaHitpoints;

	pTack->Anchor_SetUnitHealth( fAnchorUnitHealth );
}


void CMeshEntity::InflictDamage( CDamageData *pDamageData ) {
	if( m_pVerlet ) {
		CFVerletTack *pTack;
		BOOL bDamageAnchorsAttachedToMe;
		u32 i;

		bDamageAnchorsAttachedToMe = (pDamageData->m_nDamageLocale != CDamageForm::DAMAGE_LOCALE_BLAST);

		for( i=0; i<m_pVerlet->Tack_GetCount(); ++i ) {
			pTack = m_pVerlet->Tack_GetFromIndex(i);

			if( pTack->Anchor_GetType() != CFVerletTack::ANCHOR_TYPE_NONE ) {
				if( pTack->Anchor_IsEnabled() ) {
					_DamageTack( pTack, pDamageData );
				}
			}

			if( bDamageAnchorsAttachedToMe ) {
				pTack = m_pVerlet->Tack_GetFromIndex(i)->Anchor_GetTackAnchoredToMe();

				if( pTack ) {
					if( pTack->Anchor_IsEnabled() ) {
						_DamageTack( pTack, pDamageData );
					}
				}
			}
		}
	}

	CEntity::InflictDamage( pDamageData );
}


void CMeshEntity::InflictDamageResult( const CDamageResult *pDamageResult ) {
	CEntity::InflictDamageResult( pDamageResult );

	if( m_pVerlet ) {
		if( pDamageResult->m_pDamageData->m_nDamageLocale != CDamageForm::DAMAGE_LOCALE_AMBIENT ) {
			if( pDamageResult->m_fImpulseMag > 0.0f ) {
				m_pVerlet->ApplyImpulse( &pDamageResult->m_pDamageData->m_ImpactPoint_WS, &pDamageResult->m_Impulse_WS );
			}
		}
	}
}


void CMeshEntity::ClassHierarchyShatterWire( CFWire *pWire, BOOL bSilent ) {
	CFVerletTack *pTack = (CFVerletTack *)pWire->GetOwner();

	if( pTack ) {
		pTack->Anchor_SetUnitHealth( 0.0f, bSilent );
	}
}


void CMeshEntity::Die( BOOL bSpawnDeathEffects/*=TRUE*/, BOOL bSpawnGoodies/*=TRUE*/ ) {
	// Call the base class first...
	CEntity::Die( bSpawnDeathEffects, bSpawnGoodies );

	// Call the user supplied callback function...
	if( m_pDieCallback ) {
        m_pDieCallback( this, m_pDieUserParam );
	}
}


