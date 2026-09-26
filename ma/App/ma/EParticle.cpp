//////////////////////////////////////////////////////////////////////////////////////
// EParticle.cpp - 
//
// Author: Justin Link      
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
// 09/14/02 Link        Created.
// 09/15/02 Ranck		Took ownership.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "EParticle.h"
#include "fparticle.h"
#include "fresload.h"
#include "fclib.h"
#include "entity.h"
#include "floop.h"


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEParticleBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CEParticleBuilder _EParticleBuilder;


void CEParticleBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CEntityBuilder, ENTITY_BIT_PARTICLE, pszEntityType );

	m_fDimX = 0.0f;
	m_fDimY = 0.0f;
	m_fDimZ = 0.0f;
	m_fRadius = 0.0f;

	m_nShapeType = FWORLD_SHAPETYPE_POINT;
	
	m_pszPartDefFile = NULL;
	
	m_fUnitIntensity = 1.0f;
	m_bDelaySpawn = FALSE;
	m_bBurstSoundEnabled = FALSE;
	m_bEmitterCouldChangeVolumes = FALSE;

	m_bEmpty = TRUE;
}


BOOL CEParticleBuilder::InterpretTable( void ) {
	BOOL bValue;
	
	if( !fclib_stricmp( CEntityParser::m_pszTableName, "PartDef" ) ) {
		CEntityParser::Interpret_String( &m_pszPartDefFile );

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Intensity" ) ) {
		CEntityParser::Interpret_F32( &m_fUnitIntensity, 0.0f, 100.0f, TRUE );

		// Convert from a percent to a unit float.
		m_fUnitIntensity *= 0.01f;
		FMATH_CLAMP_UNIT_FLOAT( m_fUnitIntensity );

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DelaySpawn" ) ) {
		CEntityParser::Interpret_BOOL( &bValue );
		m_bDelaySpawn = bValue;

		return TRUE;
		
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "BurstSound" ) ) {
		CEntityParser::Interpret_BOOL( &bValue );
		m_bBurstSoundEnabled = bValue;

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "CanChangeVolumes") ) {
		CEntityParser::Interpret_BOOL( &bValue );
		m_bEmitterCouldChangeVolumes = bValue;

		return TRUE;
	}

	return CEntityBuilder::InterpretTable();
}


BOOL CEParticleBuilder::PostInterpretFixup( void ) {

	const CFWorldShapeInit *pWorldShapeInit = CEntityParser::m_pWorldShapeInit;
	
	if( !CEntityBuilder::PostInterpretFixup() ) {
		goto _OutOfMemory;
	}
	
	switch( pWorldShapeInit->m_nShapeType ) {

	case FWORLD_SHAPETYPE_BOX:
		m_fDimX = pWorldShapeInit->m_pBox->m_fDimX;
		m_fDimY = pWorldShapeInit->m_pBox->m_fDimY;
		m_fDimZ = pWorldShapeInit->m_pBox->m_fDimZ;
		
		m_nShapeType = FWORLD_SHAPETYPE_BOX;
		break;

	case FWORLD_SHAPETYPE_SPHERE:
		m_fRadius = pWorldShapeInit->m_pSphere->m_fRadius;

		m_nShapeType = FWORLD_SHAPETYPE_SPHERE;
		break;

	case FWORLD_SHAPETYPE_CYLINDER:
		m_fDimY = pWorldShapeInit->m_pCylinder->m_fDimY;
		m_fRadius = pWorldShapeInit->m_pCylinder->m_fRadius;

		m_nShapeType = FWORLD_SHAPETYPE_CYLINDER;
		break;

	case FWORLD_SHAPETYPE_POINT:
		m_nShapeType = FWORLD_SHAPETYPE_POINT;
		break;

	default:
		FASSERT_NOW_MSG( "EParticle - Only Boxes, Spheres, Points, and Cylinders are currently supported as particle emitters." );
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
// CEParticle
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CEParticle::m_bSystemInitialized = FALSE;


BOOL CEParticle::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );

	m_bSystemInitialized = TRUE;

	return TRUE;
}

void CEParticle::UninitSystem( void ) {
	if( m_bSystemInitialized ) {
		m_bSystemInitialized = FALSE;
	}
}

CEParticle::CEParticle() {
	// Clear all pointers to NULL...
	_ClearMemberData();
}

CEParticle::~CEParticle() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}

BOOL CEParticle::Create( cchar *pszEntityName/*=NULL*/,
						const CFMtx43A *pMtx/*=NULL*/,
						cchar *pszAIBuilderName/*=NULL*/ ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CEParticleBuilder *pBuilder = (CEParticleBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_EParticleBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_PARTICLE );
	}

	return CEntity::Create( pszEntityName, pMtx, pszAIBuilderName );
}

CEntityBuilder *CEParticle::GetLeafClassBuilder( void ) {
	// Do this and nothing more...
	return &_EParticleBuilder;
}

void CEParticle::ClassHierarchyDestroy( void ) {

	// Delete any objects that we may have created.

	// Set our allocation pointers to NULL...
	_ClearMemberData();

	// Call our parent class's ClassHierarchyDestroy() method...
	CEntity::ClassHierarchyDestroy();
}

BOOL CEParticle::ClassHierarchyBuild( void ) {

	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CEParticleBuilder *pBuilder = (CEParticleBuilder *)GetLeafClassBuilder();

	// Build our parent entity class...
	if( !CEntity::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set any defaults in our class...
	_ClearMemberData();

	// Initialize our entity from our builder object...
	if( !pBuilder->m_bEmpty ) {
		// make sure the builder object contains valid init data
        if( !pBuilder->m_pszPartDefFile ) {
			goto _ExitWithError;
		}
		// load the particle def file
		m_hPartDef = fresload_Load( FPARTICLE_RESTYPE, pBuilder->m_pszPartDefFile );
		if( m_hPartDef == FPARTICLE_INVALID_HANDLE ) {
			goto _ExitWithError;
		}
		// make sure that the def file is not a one shot effect, unless it is a delayed spawn
		if( (fparticle_GetFlags( m_hPartDef ) & FPARTICLE_DEF_FLAGS_ONE_SHOT) && !pBuilder->m_bDelaySpawn ) {
			goto _ExitWithError;
		}

		m_fInitialUnitIntensity = pBuilder->m_fUnitIntensity;		

		FParticleEmitCfg_t PartConf, *pPartConfig;
        fparticle_SetCfgToDefaults( &PartConf );

		if( pBuilder->m_bDelaySpawn ) {
			// allocate from fres a FParticleEmitCfg_t
			m_pPartConf = (FParticleEmitCfg_t *)fres_AllocAndZero( sizeof( FParticleEmitCfg_t ) );
			if( !m_pPartConf ) {
				goto _ExitWithError;
			}
			// set working pointer
			pPartConfig = m_pPartConf;
		} else {
			// just fill in the config and spawn now
			pPartConfig = &PartConf;
		}

		// fill in the cfg
		switch( pBuilder->m_nShapeType ) {

		case FWORLD_SHAPETYPE_POINT:
			pPartConfig->bEmitShapeWillMove = TRUE;// assume that the point type could change volumes
			pPartConfig->fUnitIntensity = m_fInitialUnitIntensity;
			pPartConfig->pEmitShape = &m_ParticleShape;
            
			// setup the point
			m_ParticleShape.nShapeType = FPARTICLE_SHAPETYPE_POINT;
			m_ParticleShape.Point.pPoint = &m_MtxToWorld.m_vPos;
			m_ParticleShape.Point.pDir = &m_MtxToWorld.m_vUp;			
			break;

		case FWORLD_SHAPETYPE_BOX:
			pPartConfig->bEmitShapeWillMove = pBuilder->m_bEmitterCouldChangeVolumes;
			pPartConfig->fUnitIntensity = m_fInitialUnitIntensity;
			pPartConfig->pEmitShape = &m_ParticleShape;
			
			// setup the box
			m_ParticleShape.nShapeType = FPARTICLE_SHAPETYPE_BOX;
			m_ParticleShape.Box.pCenter = &m_MtxToWorld.m_vPos;
			m_ParticleShape.Box.pNormX = &m_MtxToWorld.m_vRight;
			m_ParticleShape.Box.pNormY = &m_MtxToWorld.m_vUp;
			m_ParticleShape.Box.pNormZ = &m_MtxToWorld.m_vFront;
			
			m_afRawValues[0] = pBuilder->m_fDimX * 0.5f;
			m_afShapeValues[0] = m_afRawValues[0] * m_fScaleToWorld;	
			m_ParticleShape.Box.pfHalfLenX = &m_afShapeValues[0];
			
			m_afRawValues[1] = pBuilder->m_fDimY * 0.5f;
			m_afShapeValues[1] = m_afRawValues[1] * m_fScaleToWorld;	
			m_ParticleShape.Box.pfHalfLenY = &m_afShapeValues[1];
			
			m_afRawValues[2] = pBuilder->m_fDimZ * 0.5f;
			m_afShapeValues[2] = m_afRawValues[2] * m_fScaleToWorld;
			m_ParticleShape.Box.pfHalfLenZ = &m_afShapeValues[2];
			break;

		case FWORLD_SHAPETYPE_SPHERE:
			pPartConfig->bEmitShapeWillMove = pBuilder->m_bEmitterCouldChangeVolumes;
			pPartConfig->fUnitIntensity = m_fInitialUnitIntensity;
			pPartConfig->pEmitShape = &m_ParticleShape;
            
			// setup the sphere
			m_ParticleShape.nShapeType = FPARTICLE_SHAPETYPE_SPHERE;
			m_ParticleShape.Sphere.pCenter = &m_MtxToWorld.m_vPos;

			m_afRawValues[0] = pBuilder->m_fRadius;
			m_afShapeValues[0] = pBuilder->m_fRadius * m_fScaleToWorld;
			m_ParticleShape.Sphere.pfRadius = &m_afShapeValues[0];
			break;

		case FWORLD_SHAPETYPE_CYLINDER:
			pPartConfig->bEmitShapeWillMove = pBuilder->m_bEmitterCouldChangeVolumes;
			pPartConfig->fUnitIntensity = m_fInitialUnitIntensity;
			pPartConfig->pEmitShape = &m_ParticleShape;

			// setup the cylinder
			m_ParticleShape.nShapeType = FPARTICLE_SHAPETYPE_CYLINDER;
			m_ParticleShape.Cylinder.pCenterBase = &m_MtxToWorld.m_vPos;

			m_afRawValues[0] = pBuilder->m_fRadius;
			m_afShapeValues[0] = pBuilder->m_fRadius * m_fScaleToWorld;
			m_ParticleShape.Cylinder.pfRadius = &m_afShapeValues[0];

			m_afRawValues[1] = pBuilder->m_fDimY;
			m_afShapeValues[1] = pBuilder->m_fDimY * m_fScaleToWorld;
			m_ParticleShape.Cylinder.pfHeight = &m_afShapeValues[1];

			m_ParticleShape.Cylinder.pNorm = &m_MtxToWorld.m_vUp;
			break;

		default:
			FASSERT_NOW;
			break;
		}

		m_bBurstSoundEnabled = pBuilder->m_bBurstSoundEnabled;

		if( !pBuilder->m_bDelaySpawn ) {
			// we are not delaying emission, go ahead and start
			m_hEmitter = fparticle_SpawnEmitter( m_hPartDef, pPartConfig );
			if( m_hEmitter == FPARTICLE_INVALID_HANDLE ) {
				goto _ExitWithError;
			}

			m_bEmissionEnabled = TRUE;

			fparticle_EnableBurstSound( m_hEmitter, m_bBurstSoundEnabled );
		}

		// all particles placed in Max, delayed or not, need to be restored
		m_bNeedsRestored = TRUE;
	}

	// We've successfully built our object, so return success code...
	return TRUE;


_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CEParticle::ClassHierarchyRelocated( void *pIdentifier ) {
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

void CEParticle::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	
	StopEmission( TRUE );

	CEntity::ClassHierarchyRemoveFromWorld();
}

void CEParticle::ClassHierarchyAddToWorld( void ) {
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

BOOL CEParticle::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
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

void CEParticle::ClassHierarchyResolveEntityPointerFixups( void ) {
	FASSERT( IsCreated() );

	// Call parent's function...
	CEntity::ClassHierarchyResolveEntityPointerFixups();
}

void CEParticle::ClassHierarchyWork( void ) {
	FASSERT( IsCreated() );

	// Perform work on our parent...
	CEntity::ClassHierarchyWork();

	// Perform work for ourselves...

	if( m_fSecsRemaining >= 0.0f ) {
		// Time our emission...

		m_fSecsRemaining -= FLoop_fPreviousLoopSecs;

		if( m_fSecsRemaining > 0.0f ) {
			// Duration not up...

			if( m_bReduceIntensityWithTime ) {
				m_fUnitIntensity = m_fInitialUnitIntensity * (m_fSecsRemaining * m_fOOMaxTime);
				FMATH_CLAMPMAX( m_fUnitIntensity, 1.0f );
			}
		} else {
			// Duration is up...

			m_fSecsRemaining = -1.0f;

			StopEmission( FALSE );
			DetachFromParent();
			RemoveFromWorld();
		}
	}
}

// This private helper method NULLs any allocation pointers.
// It is extremely important that this get called by the
// entity class's constructor and ClassHierarchyDestroy().
void CEParticle::_ClearMemberData( void ) {
	m_hEmitter = FPARTICLE_INVALID_HANDLE;
	m_bReduceIntensityWithTime = FALSE;

	m_fSecsRemaining = -1.0f;
	m_fOOMaxTime = 1.0f;

	m_fUnitIntensity = 1.0f;
	m_fInitialUnitIntensity = 1.0f;

	m_hPartDef = FPARTICLE_INVALID_HANDLE;
	m_afRawValues[0] = m_afRawValues[1] = m_afRawValues[2] = 0.0f;
	m_afShapeValues[0] = m_afShapeValues[1] = m_afShapeValues[2] = 0.0f;

	m_pPartConf = NULL;
	m_bNeedsRestored = FALSE;
	m_bEmissionEnabled = FALSE;
	m_bBurstSoundEnabled = FALSE;
}

BOOL CEParticle::StartEmission( FParticle_DefHandle_t hParticleDef,
							   f32 fUnitIntensity/*=1.0f*/,
							   f32 fDuration/*=-1.0f*/,
							   BOOL bReduceIntensityWithTime/*=FALSE*/ ) {
	FASSERT( IsCreated() );

	StopEmission( TRUE );

	m_bNeedsRestored = FALSE;

	if( fDuration == 0.0f ) {
		return FALSE;
	}

	if( hParticleDef != FPARTICLE_INVALID_HANDLE ) {
		m_hEmitter = fparticle_SpawnEmitter( hParticleDef, &m_MtxToWorld.m_vPos.v3, &m_MtxToWorld.m_vFront.v3, NULL, fUnitIntensity );
		
		if( m_hEmitter == FPARTICLE_INVALID_HANDLE ) {
			DEVPRINTF("CEParticle::StartEmission failed\n");	
			// Could not create emitter...
			return FALSE;
		}
		m_bEmissionEnabled = TRUE;

		fparticle_EnableBurstSound( m_hEmitter, m_bBurstSoundEnabled );

		if( fDuration > 0.0f ) {
			// Time emission...

			m_bReduceIntensityWithTime = bReduceIntensityWithTime;

			m_fSecsRemaining = fDuration;
			m_fOOMaxTime = fmath_Inv( fDuration );

			m_fUnitIntensity = fUnitIntensity;
			m_fInitialUnitIntensity = fUnitIntensity;

			EnableOurWorkBit();
		} else {
			// Don't time emission...

			m_bReduceIntensityWithTime = FALSE;

			m_fSecsRemaining = -1.0f;
			m_fOOMaxTime = 1.0f;

			m_fUnitIntensity = fUnitIntensity;
			m_fInitialUnitIntensity = fUnitIntensity;
		}

		fparticle_SetDirection( m_hEmitter, &m_MtxToWorld.m_vFront.v3 );
		fparticle_SetIntensity( m_hEmitter, &m_fUnitIntensity );

		AddToWorld();
	}

	return TRUE;
}

void CEParticle::StopEmission( BOOL bImmediately/*=FALSE*/ ) {
	FASSERT( IsCreated() );

	if( m_hEmitter != FPARTICLE_INVALID_HANDLE ) {
		// Stop the current emission...

		if( bImmediately ) {
			fparticle_KillEmitter( m_hEmitter );
		} else {
            fparticle_StopEmitter( m_hEmitter );
		}		
		m_hEmitter = FPARTICLE_INVALID_HANDLE;

		DisableOurWorkBit();
	}
	m_bEmissionEnabled = FALSE;
}

void CEParticle::EnableEmission( BOOL bEnable ) {

	FASSERT( IsCreated() );

	if( m_hEmitter == FPARTICLE_INVALID_HANDLE && m_pPartConf ) {

		m_hEmitter = fparticle_SpawnEmitter( m_hPartDef, m_pPartConf );		
		m_bEmissionEnabled = TRUE;

		fparticle_EnableBurstSound( m_hEmitter, m_bBurstSoundEnabled );

		//check to see if this is a one shot emitter.
		//if it is, then it should be a fire-and-forget type scenario, so
		//reset the handle back to FPARTICLE_INVALID_HANDLE
		if( fparticle_GetFlags( m_hPartDef ) & FPARTICLE_DEF_FLAGS_ONE_SHOT ) {
			m_hEmitter = FPARTICLE_INVALID_HANDLE;
			m_bEmissionEnabled = FALSE;
		}
	} else if( m_hEmitter != FPARTICLE_INVALID_HANDLE ) {
		fparticle_EnableEmission( m_hEmitter, bEnable );
		m_bEmissionEnabled = bEnable;
	}
}

void CEParticle::SetUnitIntensity( f32 fUnitIntensity ) {
	FASSERT( IsCreated() );
    FASSERT_UNIT_FLOAT( fUnitIntensity );

	m_fUnitIntensity = fUnitIntensity;

	if( m_hEmitter != FPARTICLE_INVALID_HANDLE ) {		
		fparticle_SetIntensity( m_hEmitter, fUnitIntensity );
	}
}

void CEParticle::CheckpointSaveSelect( s32 nCheckpoint ) {
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}

// saves state for checkpoint
BOOL CEParticle::CheckpointSave( void ) {

	// save parent class data
	CEntity::CheckpointSave();

	CFCheckPoint::SaveData( m_fUnitIntensity );
	CFCheckPoint::SaveData( m_bNeedsRestored );
	CFCheckPoint::SaveData( m_bEmissionEnabled );
	
	return TRUE;
}

// loads state for checkpoint
void CEParticle::CheckpointRestore( void ) {
	
	// load parent class data
	CEntity::CheckpointRestore();

	// all particles are removed from the world and possibly readded, although their particle handle will be invalid at this time
	m_afShapeValues[0] = m_afRawValues[0] * m_fScaleToWorld;
	m_afShapeValues[1] = m_afRawValues[1] * m_fScaleToWorld;
	m_afShapeValues[2] = m_afRawValues[2] * m_fScaleToWorld;

	BOOL bNeedsRestored;
	BOOL bEmissionEnabled;
	CFCheckPoint::LoadData( m_fUnitIntensity );
	CFCheckPoint::LoadData( bNeedsRestored );
	CFCheckPoint::LoadData( bEmissionEnabled );
	
	if( !bNeedsRestored ) {
		// stop the emission of this particle 
		StopEmission( TRUE );
		DetachFromParent();
		RemoveFromWorld();
	} else {
		// these are all Max placed emitters, match the current state to the saved state
		if( m_pPartConf ) {
			// should it be restarted?
			if( bEmissionEnabled ) {
				EnableEmission( bEmissionEnabled );
			}
		} else {
			// this is a regular placed fx
			FParticleEmitCfg_t PartConf;
			fparticle_SetCfgToDefaults( &PartConf );
			PartConf.bEmitShapeWillMove = FALSE;
			PartConf.fUnitIntensity = m_fInitialUnitIntensity;
			PartConf.pEmitShape = &m_ParticleShape;
			
			m_hEmitter = fparticle_SpawnEmitter( m_hPartDef, &PartConf );
			EnableEmission( bEmissionEnabled );				

			fparticle_EnableBurstSound( m_hEmitter, m_bBurstSoundEnabled );
		}
	}	
}



