//////////////////////////////////////////////////////////////////////////////////////
// ELiquidVolume.cpp - 
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
// 01/08/03 Chernobieff Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "ELiquidVolume.h"
#include "fliquid.h"
#include "entity.h"
#include "floop.h"
#include "fclib.h"
#include "fresload.h"

#include "MeshTypes.h"


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CELiquidVolumeBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CELiquidVolumeBuilder _ELiquidVolumeBuilder;
CDamageProfile *CELiquidVolume::m_pDamageProfile[3];

#define MAX_PENDING_DAMAGE 4

u32 _nNumPendingDamageForms = 0;
CDamageForm _PendingDamageForms[MAX_PENDING_DAMAGE];

BOOL _CollisionCallback(CFWorldMesh *pMesh, void *pUserData);

void CELiquidVolumeBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CEntityBuilder, ENTITY_BIT_LIQUIDVOLUME, pszEntityType );

	m_nShapeType = FWORLD_SHAPETYPE_BOX;
	m_fFog = 0.0f;
	m_fDropFreq = 0.0f;
	
	m_fDimX = 1.0f;
	m_fDimY = 1.0f;
	m_fDimZ = 1.0f;
	
	m_fOpacity = 1.0f;
	m_bEnableRender = TRUE;
	m_fDamageIntensity = 1.0f;
	m_fTile = 4.0f;
	m_fBumpTile = 1.0f;
	m_fGlowScale = 1.0f;
	
	m_Color.fRed = m_Color.fGreen = m_Color.fBlue = 1.0f;
	m_fCullDist = 500.0f;

	m_vCurrent.Set(0,0);
	
	m_pszTexLayer0 = NULL;
	m_pszTexLayer1 = NULL;
		
	m_bEmpty = TRUE;

	m_pszParticle = NULL;
	m_pszType = NULL;
	m_pszExpPart = NULL;
}


BOOL CELiquidVolumeBuilder::InterpretTable( void ) {
	f32 foo255 = (1.0f/255.0f);
	if( !fclib_stricmp( CEntityParser::m_pszTableName, "EnableRender" ) ) {
		f32 fEnable = 1.0f;
		BOOL bOK = CEntityParser::Interpret_F32( &fEnable );
		m_bEnableRender = fEnable != 0.0f;
		return bOK;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DamageIntensity" ) ) {
		return CEntityParser::Interpret_F32( &m_fDamageIntensity, 0.0f, 1000.0f, TRUE );
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "LiqType" ) ) {
		CEntityParser::Interpret_String( &m_pszType );

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "ColorRed" ) ) {
		CEntityParser::Interpret_F32( &m_Color.fRed, 0.0f, 255.0f, TRUE );

		// Convert from (0,255) to (0,1)
		m_Color.fRed *= foo255;
		FMATH_CLAMP_UNIT_FLOAT( m_Color.fRed );

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "ColorGreen" ) ) {
		CEntityParser::Interpret_F32( &m_Color.fGreen, 0.0f, 255.0f, TRUE );

		// Convert from (0,255) to (0,1)
		m_Color.fGreen *= foo255;
		FMATH_CLAMP_UNIT_FLOAT( m_Color.fGreen );

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "ColorBlue" ) ) {
		CEntityParser::Interpret_F32( &m_Color.fBlue, 0.0f, 255.0f, TRUE );

		// Convert from (0,255) to (0,1)
		m_Color.fBlue *= foo255;
		FMATH_CLAMP_UNIT_FLOAT( m_Color.fBlue );

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Fog" ) ) {
		CEntityParser::Interpret_F32( &m_fFog, 0.0f, 1000.0f, TRUE );

		// Convert from (100%) to (1.0)
		m_fFog *= 0.01f;

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DropFreq" ) ) {
		CEntityParser::Interpret_F32( &m_fDropFreq, 0.0f, 200.0f, TRUE );

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "CullDist" ) ) {
		CEntityParser::Interpret_F32( &m_fCullDist, 1.0f, 5000.0f, TRUE );

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Opacity" ) ) {
		CEntityParser::Interpret_F32( &m_fOpacity, 0.0f, 100.0f, TRUE );
		
		// Convert from [0,100] to [0.1]
		m_fOpacity *= 0.01f;
		FMATH_CLAMP_UNIT_FLOAT( m_fOpacity );

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Tile" ) ) {
		CEntityParser::Interpret_F32( &m_fTile, 0.0f, 128.0f, TRUE );
		
		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "BumpTile" ) ) {
		CEntityParser::Interpret_F32( &m_fBumpTile, 0.0f, 128.0f, TRUE );
		
		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "GlowScale" ) ) {
		CEntityParser::Interpret_F32( &m_fGlowScale, 0.0f, 128.0f, TRUE );
		
		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "CurrentX" ) ) {
		CEntityParser::Interpret_F32( &m_vCurrent.x, -128.0f, 128.0f, TRUE );
		
		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "CurrentY" ) ) {
		CEntityParser::Interpret_F32( &m_vCurrent.y, -128.0f, 128.0f, TRUE );
		
		return TRUE;
	}
	else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Texture1" ) || !fclib_stricmp( CEntityParser::m_pszTableName, "Texture" ) ) {
		CEntityParser::Interpret_String( &m_pszTexLayer0 );

		return TRUE;

	}
	else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Texture2" ) ) {
		CEntityParser::Interpret_String( &m_pszTexLayer1 );

		return TRUE;
	}
	else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Particle" ) ) {
		CEntityParser::Interpret_String( &m_pszParticle );

		return TRUE;
	}
	else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PartImpact" ) ) {
		CEntityParser::Interpret_String( &m_pszParticle );

		return TRUE;
	}
	else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PartExplode" ) ) {
		CEntityParser::Interpret_String( &m_pszExpPart );

		return TRUE;
	}
	
	return CEntityBuilder::InterpretTable();
}


BOOL CELiquidVolumeBuilder::PostInterpretFixup( void ) {

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
// CELiquidVolume
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CELiquidVolume::m_bSystemInitialized = FALSE;


BOOL CELiquidVolume::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );

	m_bSystemInitialized = TRUE;

	return TRUE;
}


void CELiquidVolume::UninitSystem( void ) {
	if( m_bSystemInitialized ) {
		m_bSystemInitialized = FALSE;
	}
}


CELiquidVolume::CELiquidVolume() {
	// Clear all pointers to NULL...
	_ClearMemberData();
	_nNumPendingDamageForms = 0;
}


CELiquidVolume::~CELiquidVolume() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CELiquidVolume::Create( cchar *pszEntityName/*=NULL*/,
						const CFMtx43A *pMtx/*=NULL*/,
						cchar *pszAIBuilderName/*=NULL*/ ) {
//	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CELiquidVolumeBuilder *pBuilder = (CELiquidVolumeBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_ELiquidVolumeBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_LIQUIDVOLUME );
	}

	return CEntity::Create( pszEntityName, pMtx, pszAIBuilderName );
}


CEntityBuilder *CELiquidVolume::GetLeafClassBuilder( void ) {
	// Do this and nothing more...
	return &_ELiquidVolumeBuilder;
}


void CELiquidVolume::ClassHierarchyDestroy( void ) {

	// Delete any objects that we may have created.

	// Set our allocation pointers to NULL...
	_ClearMemberData();

	// Call our parent class's ClassHierarchyDestroy() method...
	CEntity::ClassHierarchyDestroy();
}

BOOL _CollisionCallback(CFWorldMesh *pMesh, void *pUserData)
{
	CELiquidVolume *pSelf = (CELiquidVolume *)pUserData;

	if (pSelf && pMesh && pSelf->GetDamageIntensity() > 0.0f)
	{
		// See if the world mesh that we found is an entity...
		if ( pMesh->m_nUser == MESHTYPES_ENTITY )
		{
			// It's an Entity!
			CEntity *pEDamagee = (CEntity *)(pMesh->m_pUser);
			
			if (_nNumPendingDamageForms < MAX_PENDING_DAMAGE)
			{
				_PendingDamageForms[_nNumPendingDamageForms].m_fNormIntensity = pSelf->GetDamageIntensity();
				_PendingDamageForms[_nNumPendingDamageForms].m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_AMBIENT;
				_PendingDamageForms[_nNumPendingDamageForms].m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
				_PendingDamageForms[_nNumPendingDamageForms].m_pDamageProfile = pSelf->GetDamageProfile();
				_PendingDamageForms[_nNumPendingDamageForms].m_Damager.pWeapon = NULL;
				_PendingDamageForms[_nNumPendingDamageForms].m_Damager.pBot = NULL;
				_PendingDamageForms[_nNumPendingDamageForms].m_Damager.pEntity  = pSelf;
				_PendingDamageForms[_nNumPendingDamageForms].m_pDamageeEntity = pEDamagee;
				_nNumPendingDamageForms++;
			}			

			// Get an empty damage form...
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CELiquidVolume::ClassHierarchyBuild( void ) {

	//FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );
	
	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	m_pDamageProfile[0]	= CDamage::FindDamageProfile( "Water" );
	m_pDamageProfile[1] = CDamage::FindDamageProfile( "Mercury" );
	m_pDamageProfile[2] = CDamage::FindDamageProfile( "Magma" );

	// Get pointer to the leaf class's builder object...
	CELiquidVolumeBuilder *pBuilder = (CELiquidVolumeBuilder *)GetLeafClassBuilder();

	// Build our parent entity class...
	if( !CEntity::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set any defaults in our class...
	_ClearMemberData();
	m_fDamageIntensity = pBuilder->m_fDamageIntensity;

	// Initialize our entity from our builder object...
	if( !pBuilder->m_bEmpty ) {
		
		m_afRawValues[0] = m_afRawValues[1] = m_afRawValues[2] = 0.0f;
		m_afShapeValues[0] = m_afShapeValues[1] = m_afShapeValues[2] = 0.0f;

		// fill in the cfg
		switch( pBuilder->m_nShapeType ) {

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
			m_pLiquid = LiquidSystem.CreateLiquidVolume();
			
			if (!m_pLiquid)
			{
				goto _ExitWithError;
			}
			
			LiquidType_e nType;
			if (fclib_stricmp(pBuilder->m_pszType, "water")==0)
			{
				nType = LT_WATER;

				SetDamageProfile(m_pDamageProfile[0]);
			}
			else if (fclib_stricmp(pBuilder->m_pszType, "mercury")==0)
			{
				nType = LT_MERCURY;

				SetDamageProfile(m_pDamageProfile[1]);
			}
			else if (fclib_stricmp(pBuilder->m_pszType, "oil")==0)
			{
				nType = LT_OIL;

				SetDamageProfile(m_pDamageProfile[1]);
			}
			else if (fclib_stricmp(pBuilder->m_pszType, "molten")==0)
			{
				nType = LT_MOLTEN;

				SetDamageProfile(m_pDamageProfile[2]);
			}
			else if (fclib_stricmp(pBuilder->m_pszType, "texture")==0)
			{
				nType = LT_TEXTURE;

				SetDamageProfile(m_pDamageProfile[1]);
			}
			else
			{
				nType = LT_MERCURY;

				SetDamageProfile(m_pDamageProfile[1]);
			}
			
			m_pLiquid->m_bRenderEnabled = pBuilder->m_bEnableRender;
			m_pLiquid->SetLiquidType(nType);
			m_pLiquid->SetLiquidFog(pBuilder->m_Color, pBuilder->m_fFog);
			m_pLiquid->SetupVolume(m_vExt, m_vMtx);
			
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
			if (pBuilder->m_pszParticle)
			{
				m_hCollisionParticleDef = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, pBuilder->m_pszParticle );
				m_pLiquid->SetParticleHandle(m_hCollisionParticleDef, TRUE);
			}
			else
			{
				m_pLiquid->SetParticleHandle(FPARTICLE_INVALID_HANDLE, FALSE);
			}

			if (pBuilder->m_pszExpPart)
			{
				m_hExplosionParticleDef = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, pBuilder->m_pszExpPart );
				m_pLiquid->SetExplosionPartHandle(m_hExplosionParticleDef);
			}
			else
			{
				m_pLiquid->SetExplosionPartHandle(FPARTICLE_INVALID_HANDLE);
			}
			
			m_pLiquid->SetTexLayers(m_pTexInst[0], m_pTexInst[1]);
			m_pLiquid->SetCullDist(pBuilder->m_fCullDist);
			m_pLiquid->SetOpacity(pBuilder->m_fOpacity);
			m_pLiquid->SetTile(pBuilder->m_fTile);
			m_pLiquid->SetBumpTile(pBuilder->m_fBumpTile);
			m_pLiquid->SetCurrent(pBuilder->m_vCurrent);
			m_pLiquid->SetGlowScale(pBuilder->m_fGlowScale);
			
			m_pLiquid->Init();
			
			if (pBuilder->m_fDropFreq)
			{
				if (nType ==  LT_OIL)
				{
					m_pLiquid->AddRainDrops(pBuilder->m_fDropFreq, 4.0f);
				}
				else
				{
					m_pLiquid->AddRainDrops(pBuilder->m_fDropFreq, 2.0f);
				}
			}

			 FLiquid_pCollisionCallback = _CollisionCallback;
			 m_pLiquid->SetUserData(this);
			
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


void CELiquidVolume::ClassHierarchyRelocated( void *pIdentifier ) {
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
	if( m_pLiquid ) {
		m_vMtx = m_MtxToWorld;
		m_vMtx.m_vPos.y += m_fHeightOffset;
		m_pLiquid->SetupVolume( m_vExt, m_vMtx );
	}
}


void CELiquidVolume::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	
	//stop sim.

	CEntity::ClassHierarchyRemoveFromWorld();
}



void CELiquidVolume::ClassHierarchyAddToWorld( void ) {
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



BOOL CELiquidVolume::ClassHierarchyBuilt( void ) {
//	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CEntity::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

//	DisableOurWorkBit();
	EnableOurWorkBit();

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CELiquidVolume::ClassHierarchyResolveEntityPointerFixups( void ) {
	FASSERT( IsCreated() );

	// Call parent's function...
	CEntity::ClassHierarchyResolveEntityPointerFixups();
}

void CELiquidVolume::ClassHierarchyWork( void ) {
	FASSERT( IsCreated() );

	// Perform work on our parent...
	CEntity::ClassHierarchyWork();

	// Perform work for ourselves...
	if (m_fTime > 0.0f)
	{
		f32 fDelta = FMATH_MIN( FLoop_fPreviousLoopSecs, m_fTime ) * m_fDeltaHeightPerSec;
		
		m_fTime -= FLoop_fPreviousLoopSecs;
		if (m_fTime < 0) m_fTime = 0.0f;
		
		ChangeLiquidHeight(fDelta);
	}
	
	if (_nNumPendingDamageForms > 0)
	{
		u32 i;
		for (i=0; i<_nNumPendingDamageForms; i++)
		{
			CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

			if( pDamageForm )
			{
				// Fill out the form...
				pDamageForm->m_fNormIntensity = _PendingDamageForms[i].m_fNormIntensity;
				pDamageForm->m_nDamageLocale = _PendingDamageForms[i].m_nDamageLocale;
				pDamageForm->m_nDamageDelivery = _PendingDamageForms[i].m_nDamageDelivery;
				pDamageForm->m_pDamageProfile = _PendingDamageForms[i].m_pDamageProfile;
				pDamageForm->m_Damager.pWeapon = _PendingDamageForms[i].m_Damager.pWeapon;
				pDamageForm->m_Damager.pBot = _PendingDamageForms[i].m_Damager.pBot;
				pDamageForm->m_Damager.pEntity = _PendingDamageForms[i].m_Damager.pEntity;
				pDamageForm->m_pDamageeEntity = _PendingDamageForms[i].m_pDamageeEntity;
								
				CDamage::SubmitDamageForm( pDamageForm );
			}
		}
		
		_nNumPendingDamageForms = 0;
	}
}


// This private helper method NULLs any allocation pointers.
// It is extremely important that this get called by the
// entity class's constructor and ClassHierarchyDestroy().
void CELiquidVolume::_ClearMemberData( void ) {
	m_afRawValues[0] = m_afRawValues[1] = m_afRawValues[2] = 0.0f;
	m_afShapeValues[0] = m_afShapeValues[1] = m_afShapeValues[2] = 0.0f;
	
	m_fDeltaHeightPerSec = m_fTime = m_fHeightOffset = 0.0f;
	m_fDamageIntensity = 1.0f;
	m_pLiquid = NULL;
}

void CELiquidVolume::ChangeLiquidHeight(f32 fDelta)
{
	f32 fHDelta;
	
	fHDelta = fDelta*0.5f;
	m_vExt.y += fHDelta;
		
	m_vMtx.m_vPos.y += fHDelta;
	m_fHeightOffset += fHDelta;
	
	m_pLiquid->SetupVolume(m_vExt, m_vMtx);
}

void CELiquidVolume::ChangeLiquidHeight_Time(f32 fHeightChange, f32 fTime)
{
	FASSERT(fTime > 0.0f);
	m_fDeltaHeightPerSec = fHeightChange / fTime;
	m_fTime = fTime;
}

void CELiquidVolume::CheckpointSaveSelect( s32 nCheckpoint ) {
	CEntity::CheckpointSaveSelect( nCheckpoint );
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}

BOOL CELiquidVolume::CheckpointSave( void ) {
	CEntity::CheckpointSave();
	CFCheckPoint::SaveData( m_vExt );
	CFCheckPoint::SaveData( m_vMtx );
	CFCheckPoint::SaveData( m_fDeltaHeightPerSec );
	CFCheckPoint::SaveData( m_fTime );
	CFCheckPoint::SaveData( m_fHeightOffset );
	return TRUE;
}

void CELiquidVolume::CheckpointRestore( void ) {
	CEntity::CheckpointRestore();
	CFCheckPoint::LoadData( m_vExt );
	CFCheckPoint::LoadData( m_vMtx );
	CFCheckPoint::LoadData( m_fDeltaHeightPerSec );
	CFCheckPoint::LoadData( m_fTime );
	CFCheckPoint::LoadData( m_fHeightOffset );
	_nNumPendingDamageForms = 0;
	if( m_pLiquid ) m_pLiquid->SetupVolume( m_vExt, m_vMtx );
}
