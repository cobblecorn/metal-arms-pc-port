//////////////////////////////////////////////////////////////////////////////////////
// ConvertApeFile.cpp - 
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 10/26/00 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ConvertApeFile.h"
#include "fmath.h"
#include "ftex.h"
#include "utils.h"
#include "fdata.h"

CConvertApeFile::CConvertApeFile() {
	m_nNumUVsBefore = 0;
	m_nNumUVsAfter = 0;
	m_nNumXYZsBefore = 0;
	m_nNumXYZsAfter = 0;
	m_nNumRGBsBefore = 0;
	m_nNumRGBsAfter = 0;
	m_nNumNormsBefore = 0;
	m_nNumNormsAfter = 0;
}

CConvertApeFile::~CConvertApeFile() {
}

FLightType_e CConvertApeFile::ConvertLightType( ApeLightType_e nApeLightType ) {
	switch( nApeLightType ) {
	case APE_LIGHT_TYPE_SPOT:
		return FLIGHT_TYPE_SPOT;
		break;
	case APE_LIGHT_TYPE_OMNI:
		return FLIGHT_TYPE_OMNI;
		break;
	case APE_LIGHT_TYPE_DIR:
		return FLIGHT_TYPE_DIR;
		break;
	case APE_LIGHT_TYPE_AMBIENT:
		return FLIGHT_TYPE_AMBIENT;
		break;
	}
	FASSERT_NOW;
	return FLIGHT_TYPE_DIR;
}

u32 CConvertApeFile::CreateTexInfoFlags( const ApeLayer_t *pLayer ) {
	u32 nFlags = 0;

	if( pLayer->abTile[0] ) {
		nFlags |= CFTexInst::FLAG_WRAP_S;
	}
	if( pLayer->abTile[1] ) {
		nFlags |= CFTexInst::FLAG_WRAP_T;
	}
	return nFlags;
}

BOOL CConvertApeFile::IsEmissiveNonBlack( const ApeLayer_t *pLayer ) {
	f32 fTotal = pLayer->SelfIllumRGB.fRed + 
				 pLayer->SelfIllumRGB.fGreen + 
				 pLayer->SelfIllumRGB.fBlue;
	return (fTotal > 0.0f);
}
/*
ShaderTypes_e CConvertApeFile::MapApeMatToShaderType( const ApeMaterial_t *pApeMat ) {
	BOOL bSpecularOn, bEmissiveOn;
	
	// determine if specular and emissive are "turned on" from the base layer
	bSpecularOn = ( pApeMat->aLayer[0].fShininess > 0.0f );
	bEmissiveOn = IsEmissiveNonBlack( &pApeMat->aLayer[0] );
	// MAP OUR APE MATERIAL TO A SHADER TYPE
	if( pApeMat->nLayersUsed == 1 ) {
		//////////////////
		// 1 layer shaders
		//////////////////
		switch( pApeMat->StarCommands.nShaderNum ) {
	
		case APE_SHADER_TYPE_oBASE:
			if( bEmissiveOn && bSpecularOn ) {
				return ShaderTypes_eoBASE_ADD_SPEC;
			} else if( bEmissiveOn ) {
				return ShaderTypes_eoBASE;
			} else if( bSpecularOn ) {
				return ShaderTypes_oBASE_ADD_SPEC;
			} else {
				return ShaderTypes_oBASE;
			}
			break;

		default:
		case APE_SHADER_TYPE_cBASE:
			if( bEmissiveOn && bSpecularOn ) {
				return ShaderTypes_ecBASE_ADD_SPEC;
			} else if( bEmissiveOn ) {
				return ShaderTypes_ecBASE;
			} else if( bSpecularOn ) {
				return ShaderTypes_cBASE_ADD_SPEC;
			} else {
				return ShaderTypes_cBASE;
			}
			break;

		case APE_SHADER_TYPE_tBASE:
			if( bEmissiveOn && bSpecularOn ) {
				return ShaderTypes_etBASE_ADD_SPEC;
			} else if( bEmissiveOn ) {
				return ShaderTypes_etBASE;
			} else if( bSpecularOn ) {
				return ShaderTypes_tBASE_ADD_SPEC;
			} else {
				return ShaderTypes_tBASE;
			}
			break;

		case APE_SHADER_TYPE_obsBASE:
			if( bEmissiveOn ) {
				return ShaderTypes_eoBASE_ADD_bSPEC;
			} else {
				return ShaderTypes_oBASE_ADD_bSPEC;
			}
			break;
			
		case APE_SHADER_TYPE_beoBASE:
			if( bSpecularOn ) {
				return ShaderTypes_beoBASE_ADD_SPEC;
			} else {
				return ShaderTypes_beoBASE;
			}
			break;

		case APE_SHADER_TYPE_etbsBASE:
			return ShaderTypes_etBASE_ADD_bSPEC;
			break;

		case APE_SHADER_TYPE_ADD_BASE:
			return ShaderTypes_ADD_BASE;
			break;
		}
	} else if( pApeMat->nLayersUsed == 2 ) {
		//////////////////
		// 2 layer shaders
		//////////////////
		switch( pApeMat->StarCommands.nShaderNum ) {
		
		default:
		case APE_SHADER_TYPE_oBASE_LERP_pLAYER:
			return ShaderTypes_oBASE_LERP_pLAYER;
			break;

		case APE_SHADER_TYPE_cBASE_LERP_vLAYER:
			return ShaderTypes_cBASE_LERP_vLAYER;
			break;
		
		case APE_SHADER_TYPE_oBASE_LERP_vLAYER:
			return ShaderTypes_oBASE_LERP_vLAYER;
			break;

		case APE_SHADER_TYPE_cBASE_LERP_pLAYER:
			return ShaderTypes_cBASE_LERP_pLAYER;
			break;

		case APE_SHADER_TYPE_oBASE_ADD_rbENV:
			return ShaderTypes_oBASE_ADD_rbENV;
			break;

		case APE_SHADER_TYPE_etBASE_ADD_rbENV:
			return ShaderTypes_etBASE_ADD_rbENV;
			break;

		case APE_SHADER_TYPE_oBASE_ADD_rbENV_MOD_SHADOWMAP:
			return ShaderTypes_oBASE_ADD_rbENV_MOD_SHADOWMAP;
			break;
		
		case APE_SHADER_TYPE_oBASE_LERP_tLAYER:
			if( bSpecularOn ) {
				return ShaderTypes_oBASE_ADD_SPEC_LERP_tLAYER;
			} else {
				return ShaderTypes_oBASE_LERP_tLAYER;
			}
			break;

		case APE_SHADER_TYPE_cBASE_LERP_tLAYER:
			if( bSpecularOn ) {
				return ShaderTypes_cBASE_ADD_SPEC_LERP_tLAYER;
			} else {
				return ShaderTypes_cBASE_LERP_tLAYER;
			}
			break;

		case APE_SHADER_TYPE_tBASE_LERP_tLAYER:
			if( bSpecularOn ) {
				return ShaderTypes_tBASE_ADD_SPEC_LERP_tLAYER;
			} else {
				return ShaderTypes_tBASE_LERP_tLAYER;
			}
			break;

		case APE_SHADER_TYPE_oBASE_MOD_SHADOWMAP:
			if( bSpecularOn ) {
				return ShaderTypes_oBASE_MOD_SHADOWMAP_ADD_SPEC;
			} else {
				return ShaderTypes_oBASE_MOD_SHADOWMAP;
			}
			break;

		case APE_SHADER_TYPE_cBASE_MOD_SHADOWMAP:
			if( bSpecularOn ) {
				return ShaderTypes_cBASE_MOD_SHADOWMAP_ADD_SPEC;
			} else {
				return ShaderTypes_cBASE_MOD_SHADOWMAP;
			}
			break;

		}
	} else {
		FASSERT_NOW;
	}
	return ShaderTypes_cBASE;
}
	*/
u32 CConvertApeFile::GetNumTextureCoords( const ApeMaterial_t *pApeMat ) {
	u32 i, nTextureCoords = 0;
	
	// count the texture coordinates
	for( i=0; i < pApeMat->nLayersUsed; i++ ) {
		if( pApeMat->aLayer[i].bTextured ) {
			nTextureCoords++;
		}
	}	
	return nTextureCoords;
}
