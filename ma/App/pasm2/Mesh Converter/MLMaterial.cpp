//////////////////////////////////////////////////////////////////////////////////////
// MLMaterial.cpp - Classes used to convert generic material data into Fang specific data
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
// 03/17/02 Lafleur		Created.
// 08/15/02 Lafleur		Genericized from GC code to support multi-platform.  Changed name to MLMaterial
//////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "stdafx.h"

#include "MLMesh.h"
#include "MLMaterial.h"

#include "fclib.h"
#include "gc\fGCDisplayList.h"

// From NVidia stripping library
#include "NvTriStrip.h"



//////////////////////////////////////////////////////////////////////////////////////
// Local defines:
//////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// Local classes and structures:
//////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// Global variables:
//////////////////////////////////////////////////////////////////////////////////////

extern u32 GCMesh_nVertsRendered;
extern u32 GCMesh_nMatrixSubmissions;
extern u32 GCMesh_nDLChanges;
extern u32 GCMesh_nGDBeginCalls;

s32 nGroupsAllocated = 0;
s32 nGroupsDeleted = 0;

//////////////////////////////////////////////////////////////////////////////////////
// Static function prototypes:
//////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
MLMaterial::MLMaterial( void )
{
	u32 i;
	m_FMaterial.nMtlFlags = 0;
	m_FMaterial.nDepthBiasLevel = 0;
	m_FMaterial.nLightShaderIdx = FSH_INVALID_LIGHT_SHADER;
	m_FMaterial.nSpecularShaderIdx = FSH_INVALID_SPECULAR_SHADER;
	m_FMaterial.nSurfaceShaderIdx = FSH_INVALID_SURFACE_SHADER;
	m_FMaterial.pnShLightRegisters = NULL;
	m_FMaterial.pnShSurfaceRegisters = NULL;
	m_FMaterial.vAverageVertPos.Zero();
	m_FMaterial.nDLHashKey = 0;
	m_FMaterial.pPlatformData = NULL;
	m_FMaterial.nPartIDMask = 0;
	m_FMaterial.nLODMask = 0;
	m_FMaterial.fAffectAngle = 0;
	m_FMaterial.anCompAffectNormal[0] = 0;
	m_FMaterial.anCompAffectNormal[1] = 0;
	m_FMaterial.anCompAffectNormal[2] = 0;
	for ( i = 0; i < FDATA_MESH_MAX_TEXLAYERIDS_PER_MTL; i++ )
	{
		m_FMaterial.anTexLayerIDIndex[i] = 255;
	}

	m_pKongMat = NULL;

	m_nTotalMaterialVerts = 0;
	m_pNext = NULL;
	m_nBaseSTSets = 0;
	m_nLightMapSTSets = 0;

	m_nLightShaderID = FSH_INVALID_LIGHT_SHADER;
	m_nSurfaceShaderID = FSH_INVALID_SURFACE_SHADER;
	m_nSpecularShaderID = FSH_INVALID_SPECULAR_SHADER;

	m_nDLContainerCount = 0;
	m_pFirstDLContainer = NULL;
	m_pLastDLContainer = NULL;

	m_vVertNormalTotal.Set( 0.f, 0.f, 0.f );

	m_nLightMapCount = 0;

	for ( i = 0; i < FSH_MAX_SURFACEMAPS; i++ )
	{
		m_nBaseTexture[i] = 0;
	}

	for ( i = 0; i < FSH_MAX_SURFACEMAPS + FSH_MAX_LIGHTMAPS; i++ )
	{
		m_nADDR_TexInst[i] = 0;
		m_szTexture[i][0] = 0;
		fang_MemSet( &m_TexInst[i], 0, sizeof( FShTexInst_t ) );
		m_nSTIndexLayer[i] = i;
	}

	fang_MemSet( m_nLightMapMotifID, 0, sizeof( u32 ) * FSH_MAX_LIGHTMAPS );

	m_MotifEmissive.Zero();
	m_MotifDiffuse.Zero();
	m_MotifSpecular.Zero();
	m_MotifEnvironmental.Zero();

	m_bEmissiveOn = FALSE;
	m_bSpecularOn = FALSE;

	m_bEnviroMotifOn = FALSE;

	m_nSpecularMask = -1;
	m_nEmissiveMask = -1;
	m_nTransparencyMask = -1;
	m_nBumpMapTexture = -1;
	m_nDetailTexture = -1;

	m_fSpecularExponent = 0.f;

	m_nLayersUsed = 0;
	m_nTexturesUsed = 0;

	m_nStripCount = m_nStripTriCount = m_nListTriCount = 0;
}


//
//
//
MLMaterial::~MLMaterial( void )
{
	MLDLContainer *pDL = m_pFirstDLContainer;
	MLDLContainer *pNext;
	while ( pDL )
	{
		pNext = pDL->pNextDLContainer;
		if ( pDL->paAbstrIndices )
		{
			fang_Free( pDL->paAbstrIndices );
			pDL->paAbstrIndices = NULL;
		}
		if ( pDL->paPrimGroups )
		{
			nGroupsDeleted += pDL->nPrimGroupCount;
			delete[] pDL->paPrimGroups;
			pDL->paPrimGroups = NULL;
		}

		fang_Free( pDL );
		pDL = pNext;
	}

	m_pFirstDLContainer = NULL;
	m_pLastDLContainer = NULL;
	m_nDLContainerCount = 0;
}


//
//
//
BOOL MLMaterial::Set( MLMesh *pMesh, KongMat_t *pKongMat )
{
	FASSERT( pKongMat );
	FASSERT( pKongMat->pProperties );

	m_pKongMat = pKongMat;

	m_nLayersUsed = (u8)pKongMat->pProperties->nLayersUsed;

	s8 i;
	for ( i = 0; i < m_nLayersUsed; i++ )
	{
		AddLayer( pMesh, i, &pKongMat->pProperties->aLayer[i] );
	}
	u32 nLayerTextures = m_nTexturesUsed;

	// Set the appropriate textures
	u32 nBaseLayerFlags = 0;
	if ( pKongMat->pProperties->aLayer[0].abTile[0] )
	{
		nBaseLayerFlags |= CFTexInst::FLAG_WRAP_S;
	}
	if ( pKongMat->pProperties->aLayer[0].abTile[1] )
	{
		nBaseLayerFlags |= CFTexInst::FLAG_WRAP_T;
	}

	// Is there any Bump Map?
	if ( pKongMat->pProperties->aLayer[0].szTexnames[APE_LAYER_TEXTURE_BUMP][0] )
	{
		m_nBumpMapTexture = m_nTexturesUsed;
		ZeroMemory( &m_TexInst[m_nTexturesUsed], sizeof( FShTexInst_t ) );
		m_TexInst[m_nTexturesUsed].TexInst.SetFlags( nBaseLayerFlags | CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T );
		m_TexInst[m_nTexturesUsed].TexInst.SetMipmapBias( 0.0f );
		m_TexInst[m_nTexturesUsed].nTexLayerID = pKongMat->pProperties->aLayer[0].nID;
		fclib_strcpy( m_szTexture[m_nTexturesUsed], pKongMat->pProperties->aLayer[0].szTexnames[APE_LAYER_TEXTURE_BUMP] );
		m_nTexturesUsed++;
	}

	// Is there any detail map?
	if ( pKongMat->pProperties->aLayer[0].szTexnames[APE_LAYER_TEXTURE_DETAIL][0] )
	{
		m_nDetailTexture = m_nTexturesUsed;
		ZeroMemory( &m_TexInst[m_nTexturesUsed], sizeof( FShTexInst_t ) );
		m_TexInst[m_nTexturesUsed].TexInst.SetFlags( nBaseLayerFlags | CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T );
		m_TexInst[m_nTexturesUsed].TexInst.SetMipmapBias( 0.0f );
		m_TexInst[m_nTexturesUsed].nTexLayerID = pKongMat->pProperties->aLayer[0].nID;
		fclib_strcpy( (char *)m_szTexture[m_nTexturesUsed], pKongMat->pProperties->aLayer[0].szTexnames[APE_LAYER_TEXTURE_DETAIL] );
		m_nTexturesUsed++;
	}

	SetShaderTypes( pKongMat );

	// set some material flags
	if ( pKongMat->pProperties->bNoColl )
	{
		AddFlags( FMESH_MTLFLAG_NOCOLLIDE );
	}
	if ( pKongMat->pProperties->nOrderNum )
	{
		AddFlags( FMESH_MTLFLAG_RENDER_LAST );
	}
	if ( pKongMat->pProperties->nFlags & APE_MAT_FLAGS_ZWRITE_ON )
	{
		AddFlags( FMESH_MTLFLAG_ZWRITE_ON );
	}
	if ( pKongMat->pProperties->nFlags & APE_MAT_FLAGS_DO_NOT_TINT )
	{
		AddFlags( FMESH_MTLFLAG_DO_NOT_TINT );
	}
	if ( pKongMat->pProperties->nFlags & APE_LAYER_FLAGS_DO_NOT_CAST_SHADOWS )
	{
		AddFlags( FMESH_MTLFLAG_DO_NOT_CAST_SHADOWS );
	}
	if ( pKongMat->pProperties->nFlags & APE_LAYER_FLAGS_INVERT_EMISSIVE_MASK )
	{
		AddFlags( FMESH_MTLFLAG_INVERT_EMISSIVE_MASK );
	}
	if ( pKongMat->pProperties->nFlags & APE_LAYER_FLAGS_NO_ALPHA_SCROLL )
	{
		AddFlags( FMESH_MTLFLAG_NO_ALPHA_SCROLL );
	}
	if ( pKongMat->pProperties->nFlags & APE_LAYER_FLAGS_ANGULAR_EMISSIVE )
	{
		AddFlags( FMESH_MTLFLAG_ANGULAR_EMISSIVE );
		m_FMaterial.fAffectAngle = cosf( FMATH_DEG2RAD( (f32)pKongMat->pProperties->nAffectAngle * 0.5f ) );
	}
	if ( pKongMat->pProperties->nFlags & APE_LAYER_FLAGS_ANGULAR_TRANSLUCENCY )
	{
		AddFlags( FMESH_MTLFLAG_ANGULAR_TRANSLUCENCY );
		if ( pKongMat->pProperties->nAffectAngle == 180.f )
		{
			m_FMaterial.fAffectAngle = 0.f;
		}
		else
		{
			m_FMaterial.fAffectAngle = cosf( FMATH_DEG2RAD( (f32)pKongMat->pProperties->nAffectAngle * 0.5f ) );
		}
	}

	// set the mat depth bias
	SetDepthBiasLevel( pKongMat->pProperties->nZTugValue );

	// GRAB MOTIF DATA FROM LAYERS
	// the Diffuse always comes from the base layer
	m_MotifDiffuse.nMotifIndex = pKongMat->pProperties->nDiffuseMotifID;
	if( pKongMat->pProperties->bUseDiffuseColor )
	{
		m_MotifDiffuse.Set( pKongMat->pProperties->DiffuseRGB.fRed, pKongMat->pProperties->DiffuseRGB.fGreen, pKongMat->pProperties->DiffuseRGB.fBlue, 1.0f );
	}
	else
	{
		// put white into the diffuse color
		m_MotifDiffuse.Set( 1.0f, 1.0f, 1.0f, 1.0f );
	}

	// the env motif always come from the base layer
	m_MotifEnvironmental.nMotifIndex = 0;
	m_MotifEnvironmental.Set( pKongMat->pProperties->fUnitAlphaMultiplier,
						pKongMat->pProperties->fUnitAlphaMultiplier,
						pKongMat->pProperties->fUnitAlphaMultiplier,
						pKongMat->pProperties->fUnitAlphaMultiplier );

	// grab the base layer's emmissive motif/color
	m_MotifEmissive.nMotifIndex = pKongMat->pProperties->nEmissiveMotifID;
	if( pKongMat->pProperties->bEmissiveOn )
	{
		m_MotifEmissive.Set( pKongMat->pProperties->EmissiveRGB.fRed,
									 pKongMat->pProperties->EmissiveRGB.fGreen,
									 pKongMat->pProperties->EmissiveRGB.fBlue,
									 1.0f );
	}
	else
	{
		// put white into this layer's emmissive color
		m_MotifEmissive.Set( 1.0f, 1.0f, 1.0f, 1.0f );
	}

	// grab the base layer's specular motif/color
	m_MotifSpecular.nMotifIndex = pKongMat->pProperties->nSpecularMotifID;
	if ( pKongMat->pProperties->bUseSpecularColor )
	{
		m_MotifSpecular.Set( pKongMat->pProperties->SpecularRGB.fRed * pKongMat->pProperties->fShinStr,
									 pKongMat->pProperties->SpecularRGB.fGreen * pKongMat->pProperties->fShinStr,
									 pKongMat->pProperties->SpecularRGB.fBlue * pKongMat->pProperties->fShinStr,
									 1.0f );
	}
	else
	{
		// put white into this layer's specular color
		m_MotifSpecular.Set( pKongMat->pProperties->fShinStr,
									 pKongMat->pProperties->fShinStr,
									 pKongMat->pProperties->fShinStr,
									 1.0f );
	}

	m_fSpecularExponent = pKongMat->pProperties->fShininess;

	// Fill out ST counts
	m_FMaterial.nBaseSTSets = (u8)FShaders_aShaderRegs[pKongMat->pProperties->nSurfaceShaderID].nUVCount;
	m_FMaterial.nLightMapSTSets = (u8)pKongMat->nLightMapCount;

	m_FMaterial.MaterialTint = pKongMat->pProperties->TintRGB;

	m_nTransparencyMask = (s8)pKongMat->pProperties->nTransparencyMask;
	if ( m_nTransparencyMask != -1 && m_nTransparencyMask != APE_LAYER_TEXTURE_DIFFUSE )
	{
		m_nTransparencyMask = m_nTexturesUsed;
		ZeroMemory( &m_TexInst[m_nTexturesUsed], sizeof( FShTexInst_t ) );
		m_TexInst[m_nTexturesUsed].TexInst.SetFlags( nBaseLayerFlags );
		m_TexInst[m_nTexturesUsed].TexInst.SetMipmapBias( 0.0f );
		m_TexInst[m_nTexturesUsed].nTexLayerID = pKongMat->pProperties->aLayer[0].nID;
		fclib_strcpy( m_szTexture[m_nTexturesUsed], pKongMat->pProperties->aLayer[0].szTexnames[pKongMat->pProperties->nTransparencyMask] );
		m_nTexturesUsed++;
	}

	m_nSpecularMask = (s8)pKongMat->pProperties->nSpecularMask;
	if ( m_nSpecularMask != -1 )
	{
		if ( m_nSpecularMask == APE_LAYER_TEXTURE_DIFFUSE )
		{
			m_nSpecularMask = 0;
		}
		else if ( m_nSpecularMask == pKongMat->pProperties->nTransparencyMask )
		{
			m_nSpecularMask = m_nTransparencyMask;
		}
		else
		{
			m_nSpecularMask = m_nTexturesUsed;
			ZeroMemory( &m_TexInst[m_nTexturesUsed], sizeof( FShTexInst_t ) );
			m_TexInst[m_nTexturesUsed].TexInst.SetFlags( nBaseLayerFlags );
			m_TexInst[m_nTexturesUsed].TexInst.SetMipmapBias( 0.0f );
			m_TexInst[m_nTexturesUsed].nTexLayerID = pKongMat->pProperties->aLayer[0].nID;
			fclib_strcpy( m_szTexture[m_nTexturesUsed], pKongMat->pProperties->aLayer[0].szTexnames[pKongMat->pProperties->nSpecularMask] );
			m_nTexturesUsed++;
		}
	}

	m_nEmissiveMask = (s8)pKongMat->pProperties->nEmissiveMask;
	if ( m_nEmissiveMask != -1 && m_nEmissiveMask != APE_LAYER_TEXTURE_DIFFUSE )
	{
		if ( m_nEmissiveMask == APE_LAYER_TEXTURE_DIFFUSE )
		{
			m_nEmissiveMask = 0;
		}
		else if ( m_nEmissiveMask == pKongMat->pProperties->nTransparencyMask )
		{
			m_nEmissiveMask = m_nTransparencyMask;
		}
		else if ( m_nEmissiveMask == pKongMat->pProperties->nSpecularMask )
		{
			m_nEmissiveMask = m_nSpecularMask;
		}
		else
		{
			m_nEmissiveMask = m_nTexturesUsed;
			ZeroMemory( &m_TexInst[m_nTexturesUsed], sizeof( FShTexInst_t ) );
			m_TexInst[m_nTexturesUsed].TexInst.SetFlags( nBaseLayerFlags );
			m_TexInst[m_nTexturesUsed].TexInst.SetMipmapBias( 0.0f );
			m_TexInst[m_nTexturesUsed].nTexLayerID = pKongMat->pProperties->aLayer[0].nID;
			fclib_strcpy( m_szTexture[m_nTexturesUsed], pKongMat->pProperties->aLayer[0].szTexnames[pKongMat->pProperties->nEmissiveMask] );
			m_nTexturesUsed++;
		}
	}

	m_nEnvironmentMapTexture = (s8)pKongMat->pProperties->nEnvironmentMap;
	if ( m_nEnvironmentMapTexture != -1 && m_nEnvironmentMapTexture != APE_LAYER_TEXTURE_DIFFUSE )
	{
		if ( m_nEnvironmentMapTexture == APE_LAYER_TEXTURE_DIFFUSE )
		{
			m_nEnvironmentMapTexture = 0;
		}
		else if ( m_nEnvironmentMapTexture == pKongMat->pProperties->nTransparencyMask )
		{
			m_nEnvironmentMapTexture = m_nTransparencyMask;
		}
		else if ( m_nEnvironmentMapTexture == pKongMat->pProperties->nSpecularMask )
		{
			m_nEnvironmentMapTexture = m_nSpecularMask;
		}
		else if ( m_nEnvironmentMapTexture == pKongMat->pProperties->nEmissiveMask )
		{
			m_nEnvironmentMapTexture = m_nEmissiveMask;
		}
		else if ( m_nEnvironmentMapTexture == APE_LAYER_TEXTURE_DIFFUSE + APE_LAYER_TEXTURE_MAX )
		{
			// The environment texture is the diffuse texture of the second material in the
			// composite material (legacy support) so we use that texture:

			m_nEnvironmentMapTexture = m_nTexturesUsed;
			ZeroMemory( &m_TexInst[m_nTexturesUsed], sizeof( FShTexInst_t ) );
			m_TexInst[m_nTexturesUsed].TexInst.SetFlags( nBaseLayerFlags );
			m_TexInst[m_nTexturesUsed].TexInst.SetMipmapBias( 0.0f );
			m_TexInst[m_nTexturesUsed].nTexLayerID = pKongMat->pProperties->aLayer[0].nID;
			fclib_strcpy( m_szTexture[m_nTexturesUsed], pKongMat->pProperties->aLayer[1].szTexnames[APE_LAYER_TEXTURE_DIFFUSE] );
			m_nTexturesUsed++;

			if (m_nSurfaceShaderID == FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV || m_nSurfaceShaderID == FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV || m_nSurfaceShaderID == FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV ||
				m_nSurfaceShaderID == FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV_DETAIL || m_nSurfaceShaderID == FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV_DETAIL || m_nSurfaceShaderID == FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV_DETAIL)
			{
				m_nBaseTexture[2] = m_nEnvironmentMapTexture;
			}
		}
		else
		{
			m_nEnvironmentMapTexture = m_nTexturesUsed;
			ZeroMemory( &m_TexInst[m_nTexturesUsed], sizeof( FShTexInst_t ) );
			m_TexInst[m_nTexturesUsed].TexInst.SetFlags( nBaseLayerFlags );
			m_TexInst[m_nTexturesUsed].TexInst.SetMipmapBias( 0.0f );
			m_TexInst[m_nTexturesUsed].nTexLayerID = pKongMat->pProperties->aLayer[0].nID;
			fclib_strcpy( m_szTexture[m_nTexturesUsed], pKongMat->pProperties->aLayer[0].szTexnames[pKongMat->pProperties->nEnvironmentMap] );
			m_nTexturesUsed++;

			if (m_nSurfaceShaderID == FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV || m_nSurfaceShaderID == FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV || m_nSurfaceShaderID == FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV ||
				m_nSurfaceShaderID == FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV_DETAIL || m_nSurfaceShaderID == FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV_DETAIL || m_nSurfaceShaderID == FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV_DETAIL)
			{
				m_nBaseTexture[2] = m_nEnvironmentMapTexture;
			}
		}
	}

	// If we have a dynamic reflective shader, fill out the second layer texture with the diffuse, since it will be replaced anyway
	if ( m_nLayersUsed == 1 && m_nSurfaceShaderID == FSHADERS_oBASE_ADD_rbSREFLECT )
	{
		m_nBaseTexture[1] = 0;
	}

	// If we have one one of the static reflective shaders, make sure we have an environment map
	if ( m_nLayersUsed == 1 && (m_nSurfaceShaderID == FSHADERS_etBASE_ADD_rbENV || m_nSurfaceShaderID == FSHADERS_oBASE_ADD_rbENV || m_nSurfaceShaderID == FSHADERS_oBASE_ADD_rbENV_DETAIL || m_nSurfaceShaderID == FSHADERS_etBASE_ADD_rbENV_DETAIL
		|| m_nSurfaceShaderID == FSHADERS_LIQUID_ENV || m_nSurfaceShaderID == FSHADERS_LIQUID_LAYER_ENV || m_nSurfaceShaderID == FSHADERS_LIQUID_MOLTEN_2LAYER ||
		m_nSurfaceShaderID == FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV || m_nSurfaceShaderID == FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV || m_nSurfaceShaderID == FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV) )
	{
		if ( m_nEnvironmentMapTexture == -1 )
		{
			// We have a 2 layer reflective texture specified, but no secondary layer
			return FALSE;
		}
		if (m_nSurfaceShaderID != FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV && m_nSurfaceShaderID != FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV && m_nSurfaceShaderID != FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV)
		{
			m_nBaseTexture[1] = m_nEnvironmentMapTexture;
		}
		else
		{
			m_nBaseTexture[2] = m_nEnvironmentMapTexture;
		}
	}

	// Fill in the lightmaps
	if (m_nSurfaceShaderID < FSHADERS_LIQUID_LAYER_ENV || m_nSurfaceShaderID > FSHADERS_LIQUID_MOLTEN_2LAYER)
	{
		m_nLightMapCount = (u8)pKongMat->nLightMapCount;
		for ( i = 0; i < (s16)pKongMat->nLightMapCount; i++ )
		{
			strcpy( m_szTexture[m_nTexturesUsed + i], pKongMat->szLightMapTexname[i] );
			m_nLightMapMotifID[i] = pKongMat->anLightMapMotifID[i];
		}
	}
	else
	{
		m_nLightMapCount = 0;
	}

	return TRUE;
}


//
//
//
void MLMaterial::AddTexLayerIDIndexToMaterial( u32 nTexLayerIndex )
{
	// Find a slot in the mtl to stick the index
	u32 i;
	for ( i = 0; i < FDATA_MESH_MAX_TEXLAYERIDS_PER_MTL; i++ )
	{
		if ( m_FMaterial.anTexLayerIDIndex[i] == nTexLayerIndex )
		{
			// No need to add this tex layer index, it has already been added
			return;
		}
		if ( m_FMaterial.anTexLayerIDIndex[i] == 255 )
		{
			m_FMaterial.anTexLayerIDIndex[i] = (u8)nTexLayerIndex;
			return;
		}
	}
}


//
//
//
void MLMaterial::SetSurfaceShaderIdx( u32 nSetting )
{
	m_FMaterial.nSurfaceShaderIdx = (u16)nSetting;
	m_nSurfaceShaderID = (u16)nSetting;

	if (m_nDetailTexture > -1)
	{
		if ( (nSetting >= FSHADERS_oBASE && nSetting <= FSHADERS_etBASE_ADD_rbENV) || nSetting >= FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV)
		{
			m_nSurfaceShaderID = (u16)FShaders_anSurface_Detail[nSetting];
			m_FMaterial.nSurfaceShaderIdx = (u16)m_nSurfaceShaderID;
		}
		else
		{
			m_nDetailTexture = -1;
		}
	}
}

//
//
//
BOOL MLMaterial::AddLayer( MLMesh *pMesh, u32 nLayerIdx, const KongLayer_t *pLayer )
{
	FASSERT( nLayerIdx >= 0 && nLayerIdx < FSH_MAX_TEX_LAYERS );

	u32 nFlags = 0;
	if ( pLayer->abTile[0] )
	{
		nFlags |= CFTexInst::FLAG_WRAP_S;
	}
	if ( pLayer->abTile[1] )
	{
		nFlags |= CFTexInst::FLAG_WRAP_T;
	}

	fclib_strcpy( (char *)m_szTexture[m_nTexturesUsed], pLayer->szTexnames[APE_LAYER_TEXTURE_DIFFUSE] );
	m_nBaseTexture[nLayerIdx] = m_nTexturesUsed;

	// FILL IN OUR TEXINFO STRUCT
	ZeroMemory( &m_TexInst[m_nTexturesUsed], sizeof( FShTexInst_t ) );
	m_TexInst[m_nTexturesUsed].TexInst.SetFlags( nFlags );
	m_TexInst[m_nTexturesUsed].TexInst.SetMipmapBias( 0.0f );
	m_TexInst[m_nTexturesUsed].nTexLayerID = pLayer->nID;

	m_nTexturesUsed++;

	// CREATE A TEX LAYER ID IF WE NEED TO

	if ( pLayer->nID != 255 )
	{
		FMeshTexLayerID_t TexLayerID;

		ZeroMemory( &TexLayerID, sizeof( FMeshTexLayerID_t ) );
		TexLayerID.nTexLayerID = pLayer->nID;

		// see if we should setup texture rotation
		if ( pLayer->fUVRotation != 0.f )
		{
			TexLayerID.nFlags |= FMESH_TEXLAYERIDFLAG_BEGIN_ROTATING | FMESH_TEXLAYERIDFLAG_USE_ST_INFO;
			TexLayerID.m_fUVDegreeRotationPerSec = pLayer->fUVRotation;
			TexLayerID.m_nCompressedUVRotAnchor[0] = (u8)(pLayer->vUVRotAnchor.x * 255.f);
			TexLayerID.m_nCompressedUVRotAnchor[1] = (u8)(pLayer->vUVRotAnchor.y * 255.f);
		}

		// see if we should setup texture scrolling
		if ( pLayer->fDeltaUPerSec != 0.0f || pLayer->fDeltaVPerSec != 0.0f )
		{
			TexLayerID.nFlags |= FMESH_TEXLAYERIDFLAG_BEGIN_SCROLLING | FMESH_TEXLAYERIDFLAG_USE_ST_INFO;
			TexLayerID.m_ScrollSTPerSec.Set( pLayer->fDeltaUPerSec, pLayer->fDeltaVPerSec );
		}
		// see if we should setup a texture animation
		f32 fTemp = 0.0f;
		if ( pLayer->fFramesPerSecs > 0.0f )
		{
			fTemp = pLayer->fFramesPerSecs * (1.0f/60.0f);
			fTemp = 1.0f/fTemp;
			FMATH_CLAMP( fTemp, 1.0f, 255.0f );
		}
		if ( pLayer->nNumTexFrames > 1 && fTemp >= 1.0f )
		{
			TexLayerID.nFlags |= FMESH_TEXLAYERIDFLAG_BEGIN_FLIPPING | FMESH_TEXLAYERIDFLAG_USE_FLIP_INFO;
			TexLayerID.nFramesPerFlip = (u8)fTemp;
			TexLayerID.nFlipPageCount = (u8)pLayer->nNumTexFrames;
		}

		// add to the tex layer id
		u32 nTexLayerIndex = pMesh->AddTexLayerID( &TexLayerID );
		FASSERT( nTexLayerIndex >= 0 );

		AddTexLayerIDIndexToMaterial( nTexLayerIndex );
	}

	return TRUE;
}


//
//  This function will add the required counts of motifs, texture layers and registers for
//	this material's shader.
//
BOOL MLMaterial::AddShaderMSRCount( u32 *pnMotifCounter, u32 *pnShTexInstCounter, u32 *pnRegisterCounter )
{
	if ( m_nSurfaceShaderID == -1 )
	{
		FASSERT_NOW;
		return FALSE;
	}

	(*pnMotifCounter) += (!!m_bEmissiveOn) + (!!m_bSpecularOn) + (!!m_bEnviroMotifOn) + 1/*Diffuse Motif*/;
	(*pnShTexInstCounter) += m_nTexturesUsed + m_nLightMapCount;
	(*pnRegisterCounter) += FShaders_aShaderRegs[m_nSurfaceShaderID].nRegisterCount;
	(*pnRegisterCounter) += FShaders_aLightShaderRegs[m_nLightShaderID].nRegisterCount + (m_nLightMapCount * 3) + 1; // Add one for 0xffffffff to indicate end of registers

	return TRUE;
}


//
//
//
BOOL MLMaterial::CopyTextureData( u8 **pShTexInsts, u8 **pShTexNames, u8 *pExportBase, BOOL bChangeEndian )
{
	s8 i;
	for ( i = 0; i < m_nTexturesUsed + m_nLightMapCount; i++ )
	{
		m_nADDR_TexInst[i] = *pShTexInsts - pExportBase;

		fclib_strcpy( (char *)*pShTexNames, m_szTexture[i] );
		FASSERT( (u32)(*pShTexNames - pExportBase) < 65536 );
		m_TexInst[i].TexInst.SetTexDef( NULL );
		m_TexInst[i].nTextureNameOffset = (u16)(*pShTexNames - pExportBase);
		(*pShTexNames) += strlen( m_szTexture[i] ) + 1;

		if ( bChangeEndian )
		{
			m_TexInst[i].ChangeEndian();
		}
		fang_MemCopy( *pShTexInsts, &m_TexInst[i], sizeof( FShTexInst_t ) );
		(*pShTexInsts) += sizeof( FShTexInst_t );
	}

	return TRUE;
}


//
//
//
BOOL MLMaterial::CopySurfaceShaderMSR( u32 **pRegisters, u8 **pMotifs, u8 *pExportBase, BOOL bChangeEndian )
{
	FASSERT( pRegisters && pMotifs );

	if ( m_nSurfaceShaderID == FSH_INVALID_SURFACE_SHADER )
	{
		FASSERT_NOW;
		return FALSE;
	}

	u32 i;
	for ( i = 0; i < FShaders_aShaderRegs[m_nSurfaceShaderID].nRegisterCount; i++ )
	{
		if ( FShaders_aShaderRegs[m_nSurfaceShaderID].anRegType[i] == FSHADERS_REG_NULL )
		{
			break;
		}

		switch ( FShaders_aShaderRegs[m_nSurfaceShaderID].anRegType[i] )
		{
			case FSHADERS_REG_NULL:
				// The FShaders_aShaderRegs has an incorrect number of registers
				FASSERT_NOW;
				return TRUE;

			case FSHADERS_REG_LAYER0:
				FASSERT( m_nADDR_TexInst[m_nBaseTexture[0]] );
				**pRegisters = m_nADDR_TexInst[m_nBaseTexture[0]];
				break;

			case FSHADERS_REG_LAYER1:
				FASSERT( m_nADDR_TexInst[m_nBaseTexture[1]] );
				**pRegisters = m_nADDR_TexInst[m_nBaseTexture[1]];
				break;

			case FSHADERS_REG_LAYER2:
				FASSERT( m_nADDR_TexInst[m_nBaseTexture[2]] );
				**pRegisters = m_nADDR_TexInst[m_nBaseTexture[2]];
				break;

			case FSHADERS_REG_LAYER3:
				FASSERT( m_nADDR_TexInst[m_nBaseTexture[3]] );
				**pRegisters = m_nADDR_TexInst[m_nBaseTexture[3]];
				break;

			case FSHADERS_REG_TC0:
				**pRegisters = 0;
				break;

			case FSHADERS_REG_TC1:
				**pRegisters = 1;
				break;

			case FSHADERS_REG_TC2:
				**pRegisters = 2;
				break;

			case FSHADERS_REG_TC3:
				**pRegisters = 3;
				break;

			case FSHADERS_REG_ENV_MOTIF:
				if ( m_bEnviroMotifOn )
				{
					**pRegisters = *pMotifs - pExportBase;
					if ( bChangeEndian )
					{
						m_MotifEnvironmental.ChangeEndian();
					}
					fang_MemCopy( *pMotifs, &m_MotifEnvironmental, sizeof( CFColorMotif ) );
					CFColorMotif *pCurMotif = (CFColorMotif *)(*pMotifs);
					(*pMotifs) += sizeof( CFColorMotif );

					FASSERT( m_MotifEnvironmental.nMotifIndex == 0 );
					FASSERT( pCurMotif->nMotifIndex == 0 );
				}
				else
				{
					**pRegisters = 0;
				}
				break;

			case FSHADERS_REG_DETAILMAP:
				FASSERT( m_nADDR_TexInst[m_nDetailTexture] );
				**pRegisters = m_nADDR_TexInst[m_nDetailTexture];
				break;

			case FSHADERS_REG_DETAILMAP_TILE_FACTOR:
				FASSERT( m_nADDR_TexInst[m_nDetailTexture] );
				**pRegisters = (u32)(*(u32*)&m_pKongMat->pProperties->fDetailMapTileFactor);
				break;

			default:
				FASSERT_NOW;
				return FALSE;
		}

		if ( bChangeEndian )
		{
			(**pRegisters) = fang_ConvertEndian( **pRegisters );
		}

		*pRegisters += 1;
	}

	if ( i != FDATA_MAX_SURFACE_SHADER_REGS )
	{
		// If we didn't reach the max registers, verify that the next register is NULL.  Otherwise
		// something is off in the FShaders_aShaderRegs array.
		FASSERT( FShaders_aShaderRegs[m_nSurfaceShaderID].anRegType[i] == FSHADERS_REG_NULL );
	}

	return TRUE;
}


//
//
//
BOOL MLMaterial::CopyLightShaderMSR( u32 **pRegisters, u8 **pMotifs, u8 *pExportBase, BOOL bChangeEndian )
{
	FASSERT( pRegisters && pMotifs );
	u32 nTempLightMapMotif = 0;

	if ( m_nLightShaderID == FSH_INVALID_LIGHT_SHADER )
	{
		FASSERT_NOW;
		return FALSE;
	}

	u32 i;
	for ( i = 0; i < FShaders_aLightShaderRegs[m_nLightShaderID].nRegisterCount; i++ )
	{
		if ( FShaders_aLightShaderRegs[m_nLightShaderID].anRegType[i] == FSHADERS_LIGHT_REG_NULL )
		{
			break;
		}

		switch ( FShaders_aLightShaderRegs[m_nLightShaderID].anRegType[i] )
		{
			case FSHADERS_LIGHT_REG_NULL:
				// The FShaders_aShaderRegs has an incorrect number of registers
				FASSERT_NOW;
				return TRUE;

			case FSHADERS_LIGHT_REG_ZMASK:
				if ( m_nTransparencyMask != -1 )
				{
					FASSERT( m_nADDR_TexInst[m_nTransparencyMask] );
					**pRegisters = m_nADDR_TexInst[m_nTransparencyMask];
				}
				else
				{
					**pRegisters = 0;
				}

				break;

			case FSHADERS_LIGHT_REG_EMASK:
				if ( m_nEmissiveMask != -1 )
				{
					FASSERT( m_nADDR_TexInst[m_nEmissiveMask] );
					**pRegisters = m_nADDR_TexInst[m_nEmissiveMask];
				}
				else
				{
					**pRegisters = 0;
				}

				break;

			case FSHADERS_LIGHT_REG_SMASK:
				if ( m_nSpecularMask != -1 )
				{
					FASSERT( m_nADDR_TexInst[m_nSpecularMask] );
					**pRegisters = m_nADDR_TexInst[m_nSpecularMask];
				}
				else
				{
					**pRegisters = 0;
				}

				break;

			case FSHADERS_LIGHT_REG_BUMPMAP:
				if ( m_nBumpMapTexture != -1 )
				{
					FASSERT( m_nADDR_TexInst[m_nBumpMapTexture] );
					**pRegisters = m_nADDR_TexInst[m_nBumpMapTexture];
				}
				else
				{
					**pRegisters = 0;
				}

				break;

			case FSHADERS_LIGHT_REG_BUMPMAP_TILE_FACTOR:
				if ( m_nBumpMapTexture != -1 )
				{
					FASSERT( m_nADDR_TexInst[m_nBumpMapTexture] );
					**pRegisters = (u32)(*(u32*)&m_pKongMat->pProperties->fBumpMapTileFactor);
				}
				else
				{
					**pRegisters = 0;
				}
				break;

			case FSHADERS_LIGHT_REG_DMOTIF:
				**pRegisters = *pMotifs - pExportBase;
				nTempLightMapMotif = *pMotifs - pExportBase;
				if ( bChangeEndian )
				{
					m_MotifDiffuse.ChangeEndian();
				}
				fang_MemCopy( *pMotifs, &m_MotifDiffuse, sizeof( CFColorMotif ) );
				(*pMotifs) += sizeof( CFColorMotif );
				break;

			case FSHADERS_LIGHT_REG_EMOTIF:
				if ( m_bEmissiveOn )
				{
					**pRegisters = *pMotifs - pExportBase;
					if ( bChangeEndian )
					{
						m_MotifEmissive.ChangeEndian();
					}
					fang_MemCopy( *pMotifs, &m_MotifEmissive, sizeof( CFColorMotif ) );
					(*pMotifs) += sizeof( CFColorMotif );
				}
				else
				{
					**pRegisters = 0;
				}
				break;

			case FSHADERS_LIGHT_REG_SMOTIF:
				if ( m_bSpecularOn )
				{
					**pRegisters = *pMotifs - pExportBase;
					if ( bChangeEndian )
					{
						m_MotifSpecular.ChangeEndian();
					}
					fang_MemCopy( *pMotifs, &m_MotifSpecular, sizeof( CFColorMotif ) );
					(*pMotifs) += sizeof( CFColorMotif );
				}
				else
				{
					**pRegisters = 0;
				}
				break;

			case FSHADERS_LIGHT_REG_SEXP:
				**pRegisters = (u32)(*(u32*)&m_fSpecularExponent);
				break;

			case FSHADERS_LIGHT_REG_LMCOUNT:
				**pRegisters = m_nLightMapCount;
				break;

			default:
				FASSERT_NOW;
				return FALSE;
		}

		if ( bChangeEndian )
		{
			(**pRegisters) = fang_ConvertEndian( **pRegisters );
		}

		*pRegisters += 1;
	}

	if ( i != FDATA_MAX_LIGHT_SHADER_REGS )
	{
		// If we didn't reach the max registers, verify that the next register is NULL.  Otherwise
		// something is off in the FShaders_aShaderRegs array.
		FASSERT( FShaders_aLightShaderRegs[m_nLightShaderID].anRegType[i] == FSHADERS_LIGHT_REG_NULL );
	}

	// Stick in the lightmaps
	for ( i = 0; (s32)i < m_nLightMapCount; i++ )
	{
		// Copy over the pointer to the tex inst
		FASSERT( m_nADDR_TexInst[m_nTexturesUsed + i] );
		**pRegisters = m_nADDR_TexInst[m_nTexturesUsed + i];
		if ( bChangeEndian )
		{
			(**pRegisters) = fang_ConvertEndian( **pRegisters );
		}
		*pRegisters += 1;

		// Copy over the ST index
		**pRegisters = m_nTexturesUsed + i;
		if ( bChangeEndian )
		{
			(**pRegisters) = fang_ConvertEndian( **pRegisters );
		}
		*pRegisters += 1;

		// Copy over the motif index
		**pRegisters = m_nLightMapMotifID[i];
		if ( bChangeEndian )
		{
			(**pRegisters) = fang_ConvertEndian( **pRegisters );
		}
		*pRegisters += 1;
	}

	// Seal the end of the Light registers (We have to do this
	// because there is an undetermined number of lightmaps)
	**pRegisters = 0xffffffff;
	*pRegisters += 1;

	return TRUE;
}


//
//
//
void MLMaterial::SetShaderTypes( KongMat_t *pKongMat )
{
	m_bEmissiveOn = pKongMat->pProperties->bEmissiveOn;
	m_bSpecularOn = pKongMat->pProperties->bSpecularOn;
	m_bEnviroMotifOn = pKongMat->pProperties->bEnviroMotifOn;
	SetSurfaceShaderIdx( pKongMat->pProperties->nSurfaceShaderID );
	SetLightShaderIdx( pKongMat->pProperties->nLightShaderID );
	SetSpecularShaderIdx( pKongMat->pProperties->nSpecularShaderID );
}


//
//	Returns 1 if a new display list is required in order to draw the submitted material
//
u32 MLMaterial::AppendTris( MLTriContainer *pContainer )
{
	FASSERT( pContainer->m_pMaterial == this );

	MLDLContainer *pDL;

#ifndef _MMI_TARGET_PS2
//NS 040703
	if ( m_pKongMat->pProperties->nFlags & APE_MAT_FLAGS_NO_DRAW )
	{
		return 0;
	}
#endif

	// Do some verification checks if there are already verts in this material
	if ( m_nTotalMaterialVerts != 0 )
	{
		// I'm making an assumption that all tris in a material should have
		// the same number of ST sets
		if ( m_nBaseSTSets + m_nLightMapSTSets != (u8)(pContainer->m_nBaseSTSets + pContainer->m_nLightMapSTSets) )
		{
			FASSERT_NOW;
			return 0;
		}
	}

	// Set the number of STs
	m_nBaseSTSets = (u8)pContainer->m_nBaseSTSets;
	m_nLightMapSTSets = (u8)pContainer->m_nLightMapSTSets;

	m_nTotalMaterialVerts += pContainer->m_nVertCount;

	// Update the LOD mask
#ifndef _MMI_TARGET_PS2 //NS We'll do this later on the PS2 re throwing away top level LOD
	m_FMaterial.nLODMask |= (1 << pContainer->m_nLODID);
#endif

	if (   (m_pKongMat->pProperties->nFlags & APE_LAYER_FLAGS_ANGULAR_EMISSIVE)
		|| (m_pKongMat->pProperties->nFlags & APE_LAYER_FLAGS_ANGULAR_TRANSLUCENCY) )
	{
		u32 i;
		for ( i = 0; i < pContainer->m_nVertCount; i++ )
		{
			m_vVertNormalTotal += pContainer->m_paKongVerts[i].Norm;
		}

		if ( m_vVertNormalTotal.Mag2() > 0.001f )
		{
			CFVec3 vAvg;
			vAvg = m_vVertNormalTotal.Unit();
			m_FMaterial.anCompAffectNormal[0] = (u8)(vAvg.x * 64.f);
			m_FMaterial.anCompAffectNormal[1] = (u8)(vAvg.y * 64.f);
			m_FMaterial.anCompAffectNormal[2] = (u8)(vAvg.z * 64.f);
			m_FMaterial.nAffectBoneID = pContainer->m_nMatrixIdx;
		}
	}

	// Allocate a new DL container
	pDL = (MLDLContainer *)fang_Malloc( sizeof( MLDLContainer ), 4 );
	if ( !pDL )
	{
		return 0;
	}
	// Fill in the DL container data
	pDL->bUseConstantColor = pContainer->m_bConstantColor;
	pDL->bFacingDirLight = pContainer->m_bFacingDirectionalLight;
	pDL->ConstantColor.Set( pContainer->m_ConstantColor );
	pDL->nVBKey = pContainer->m_nVBKey;
	pDL->pTris = pContainer;
	pDL->pNextDLContainer = NULL;
	pDL->nMatrixCount = 1;
	pDL->nVertexCount = pContainer->m_nVertCount;
	pDL->nMatrixIdx = (u16)pContainer->m_nMatrixIdx;
	pDL->nLODID = pContainer->m_nLODID;
	pDL->nPartID = pContainer->m_nPartID;
	pDL->paAbstrIndices = NULL;
	pDL->paPrimGroups = NULL;
	pDL->nPrimGroupCount = NULL;
	pDL->nTotalVertIndices = 0;
	pDL->nStripGroupCount = 0;

	// Add this part ID to the material part ID mask
	FASSERT( pContainer->m_nPartID < 32 );
	m_FMaterial.nPartIDMask |= (1<<pContainer->m_nPartID );

	if ( m_pLastDLContainer )
	{
		m_pLastDLContainer->pNextDLContainer = pDL;
	}
	m_pLastDLContainer = pDL;
	if ( !m_pFirstDLContainer )
	{
		m_pFirstDLContainer = pDL;
	}

	m_nDLContainerCount++;

	return 1;
}


//
//
//
void MLMaterial::CalculateBoundingSphere( const CFSphere *pMeshBoundingSphere )
{
	u32 i, nTotalVerts = 0;
	CFVec3A vDiff, vCenter( 0.f, 0.f, 0.f );
	f32 fRadius = 0.f;

	if ( pMeshBoundingSphere->m_fRadius < 0.0001f )
	{
		m_FMaterial.vAverageVertPos = pMeshBoundingSphere->m_Pos;
		m_FMaterial.nCompressedRadius = 255;
		return;
	}

	// Not the best calculation of bounding sphere, but it'll work.

	// Cycle through the DL's generating the average position
	MLDLContainer *pDL = m_pFirstDLContainer;
	while ( pDL )
	{
		for ( i = 0; i < pDL->pTris->m_nVertCount; i++ )
		{
			vCenter.v3 += pDL->pTris->m_paKongVerts[i].Pos;
		}

		nTotalVerts += pDL->pTris->m_nVertCount;

		pDL = pDL->pNextDLContainer;
	}

	vCenter.Mul( 1.f / (f32)nTotalVerts );

	f32 fMeshRadSq = pMeshBoundingSphere->m_fRadius * pMeshBoundingSphere->m_fRadius;

	// Determine maximum distance of the verts from the average position
	CFVec3 vMin, vMax;
	vMin = vMax = m_pFirstDLContainer->pTris->m_paKongVerts[0].Pos;
	pDL = m_pFirstDLContainer;
	while ( pDL )
	{
		for ( i = 0; i < pDL->pTris->m_nVertCount; i++ )
		{
			vMin.x = FMATH_MIN( pDL->pTris->m_paKongVerts[i].Pos.x, vMin.x );
			vMin.y = FMATH_MIN( pDL->pTris->m_paKongVerts[i].Pos.y, vMin.y );
			vMin.z = FMATH_MIN( pDL->pTris->m_paKongVerts[i].Pos.z, vMin.z );

			vMax.x = FMATH_MAX( pDL->pTris->m_paKongVerts[i].Pos.x, vMax.x );
			vMax.y = FMATH_MAX( pDL->pTris->m_paKongVerts[i].Pos.y, vMax.y );
			vMax.z = FMATH_MAX( pDL->pTris->m_paKongVerts[i].Pos.z, vMax.z );

			vDiff.v3 = vCenter.v3 - pDL->pTris->m_paKongVerts[i].Pos;
			f32 fTemp = vDiff.MagSq();
			if ( fTemp > fRadius )
			{
				if ( fTemp > fMeshRadSq )
				{
					fTemp = fTemp;
				}
				fRadius = fTemp;
			}
		}

		pDL = pDL->pNextDLContainer;
	}

	if ( fRadius > 0.f )
	{
		fRadius = fmath_AcuSqrt( fRadius );

		// Take the lesser of the box surrounding the geo and the average vertex based methods of determining bounding spheres:
		vDiff = vMax - vMin;
		f32 fBoxRad = vDiff.MagSq();
		if ( fBoxRad > 0.f  )
		{
			fBoxRad = fmath_AcuSqrt( fBoxRad ) * 0.5f;
			if ( fBoxRad < fRadius )
			{
				fRadius = fBoxRad;
				vCenter.v3 = vMin + (vDiff.v3 * 0.5f);
			}
		}
	}

	// Convert to a unit representation as a percentage of the mesh bounding sphere radius
	fRadius = fRadius / pMeshBoundingSphere->m_fRadius;
	FMATH_CLAMP( fRadius, 0.f, 1.f );

	// Compress to u8 and store in material
	m_FMaterial.vAverageVertPos = vCenter.v3;
	m_FMaterial.nCompressedRadius = (u8)(fRadius * 255.f);
}


//
//
//
BOOL MLMaterial::BuildPrimitiveGroups( u32 nMinTrisInStrip, u32 nVertCacheSize, BOOL bCreateStrips )
{
	u32 i;

	// Set the stripper parameters
	SetCacheSize( nVertCacheSize );
	SetStitchStrips( FALSE );
	SetMinStripSize( nMinTrisInStrip );
	SetListsOnly( FALSE );

	u32 nGroupCount = 0;
	m_nStripCount = m_nStripTriCount = m_nListTriCount = 0;

	// Cycle through the DL's generating primitive lists
	MLDLContainer *pDL = m_pFirstDLContainer;
	while ( pDL )
	{
		u32 nIndexCount = 0;
		pDL->nTotalVertIndices = 0;
		pDL->nStripGroupCount = 0;

		pDL->paAbstrIndices = (u16 *)fang_Malloc( sizeof( u16 ) * pDL->nVertexCount, 4 );
		if ( !pDL->paAbstrIndices )
		{
			return FALSE;
		}

		if ( !BuildDLAbstractionIndices( pDL, nIndexCount ) )
		{
			fang_Free( pDL->paAbstrIndices );
			pDL->paAbstrIndices = NULL;
			return FALSE;
		}

		if ( pDL->nVertexCount < (nMinTrisInStrip * 3) || (!bCreateStrips) || m_pKongMat->pProperties->bTwoSided )
		{
			// This list doesn't exceed the minimum tri count for a strip, so we won't try to calc the strip, or
			// the requestor does not want strips or this is a two sided material, which the stripper doesn't handle well.
			// Manually build a primitive group
			pDL->paPrimGroups = new PrimitiveGroup[1];
			pDL->nPrimGroupCount = 1;
			pDL->paPrimGroups[0].numIndices = nIndexCount;
			pDL->paPrimGroups[0].type = PT_LIST;
			pDL->paPrimGroups[0].indices = new u16[nIndexCount];
			memcpy( pDL->paPrimGroups[0].indices, pDL->paAbstrIndices, sizeof( u16 ) * nIndexCount );
		}
		else
		{
			GenerateStrips( pDL->paAbstrIndices, nIndexCount, &pDL->paPrimGroups, &pDL->nPrimGroupCount );
		}

		nGroupsAllocated += pDL->nPrimGroupCount;
		nGroupCount += pDL->nPrimGroupCount;

		u32 nDLTriCount = 0;
		for ( i = 0; i < pDL->nPrimGroupCount; i++ )
		{
			pDL->nTotalVertIndices += pDL->paPrimGroups[i].numIndices;

			if ( pDL->paPrimGroups[i].type == PT_LIST )
			{
				nDLTriCount += pDL->paPrimGroups[i].numIndices / 3;
				m_nListTriCount += pDL->paPrimGroups[i].numIndices / 3;
			}
			else if ( pDL->paPrimGroups[i].type == PT_STRIP )
			{
				nDLTriCount += pDL->paPrimGroups[i].numIndices - 2;
				pDL->nStripGroupCount++;
				m_nStripTriCount += pDL->paPrimGroups[i].numIndices - 2;
			}
			else
			{
				FASSERT_NOW;
			}
		}

		m_nStripCount += pDL->nStripGroupCount;

		pDL = pDL->pNextDLContainer;
	}
/*
	DEVPRINTF( "Material primitive group calculation complete.\n" );
	DEVPRINTF( "	Total Groups	: %d\n", nGroupCount );
	DEVPRINTF( "	Total Tris		: %d\n", m_nStripTriCount + m_nListTriCount );
	DEVPRINTF( "	List Tris		: %d\n", m_nListTriCount );
	DEVPRINTF( "	Strip Tris		: %d\n", m_nStripTriCount );
	DEVPRINTF( "    %% Stripped      : %5.2f\n", (f32)(m_nStripTriCount * 100) / (f32)(m_nStripTriCount + m_nListTriCount) );
	if ( m_nStripTriCount )
	{
		DEVPRINTF( "	Avg. Tris/strip	: %4.1f\n\n", (f32)m_nStripTriCount / (f32)nGroupCount );
	}
	else
	{
		DEVPRINTF( "	Avg. Tris/strip	: 0.0\n\n" );
	}
*/
	return TRUE;
}
