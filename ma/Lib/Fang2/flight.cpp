//////////////////////////////////////////////////////////////////////////////////////
// flight.cpp - 
//
// Author: Steve Ranck     
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
// 11/20/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "flight.h"
#include "fclib.h"
#include "fxfm.h"
#include "ftex.h"
#include "fresload.h"
#include "fdraw.h"


static u8 _nPhase = 0;
extern u32 FVis_nCurLightIdx;

//These variables control how dynamic lighting is mapped from MAX->GAME

//Exposure, controls the intensity of the lighting, 1.0f = default.
f32 FLight_fExposure=1.0f;
//This value adjusts the actual radius of point lights, 1.0f = default
f32 FLight_fLightRadiusAdj=1.0f;//2.2f;
//This is the intensity clamp for lights after applying the current exposure adjustment, 1.0 = default
f32 FLight_fItensClamp=1.25f * 0.5f;//default = 2.0f * 0.5f, clamp intensity.
//Show light range.
BOOL FLight_bShowLightRange=FALSE;
//Light intensity = Clamp(I * fExposure, 0.0f, fItensClamp);
//Light radius = fRadius * fLightRadiusAdj;
//

//
//
//
u8 flight_GetPhase( void )
{
	_nPhase = (_nPhase+1)%8;
	return _nPhase;
}


//
//
//
CFLight::~CFLight( void )
{
	if ( m_pProjectionTex )
	{
		fdelete( m_pProjectionTex );
	}
	if ( m_pCoronaTex )
	{
		fdelete( m_pCoronaTex );
	}
}


//
//
//
void CFLight::LightDirectionalPoint( const CFVec3A *pPoint_WS, const CFVec3A *pUnitNormal_WS, CFColorRGB *pDestColorRGB ) 
{
	switch( m_nType ) 
	{
		case FLIGHT_TYPE_DIR:
			if ( !LightDirectionalPointWithDirLight( pPoint_WS, pUnitNormal_WS, pDestColorRGB ) )
			{
				pDestColorRGB->Black();
			}
			break;

		case FLIGHT_TYPE_OMNI:
			if ( !LightDirectionalPointWithOmniLight( pPoint_WS, pUnitNormal_WS, pDestColorRGB ) )
			{
				pDestColorRGB->Black();
			}
			break;

		case FLIGHT_TYPE_SPOT:
			if ( !LightDirectionalPointWithSpotLight( pPoint_WS, pUnitNormal_WS, pDestColorRGB ) )
			{
				pDestColorRGB->Black();
			}
			break;

		case FLIGHT_TYPE_AMBIENT:
			if ( !LightPointWithAmbientLight( pPoint_WS, pDestColorRGB ) )
			{
				pDestColorRGB->Black();
			}
			break;

		default:
			FASSERT_NOW;
	}
}


//
//
//
void CFLight::LightPoint( const CFVec3A *pPoint_WS, CFColorRGB *pDestColorRGB ) 
{
	switch( m_nType ) 
	{
		case FLIGHT_TYPE_DIR:
			if ( !LightPointWithDirLight( pPoint_WS, pDestColorRGB ) )
			{
				pDestColorRGB->Black();
			}
			break;

		case FLIGHT_TYPE_OMNI:
			if ( !LightPointWithOmniLight( pPoint_WS, pDestColorRGB ) )
			{
				pDestColorRGB->Black();
			}
			break;

		case FLIGHT_TYPE_SPOT:
			if ( !LightPointWithSpotLight( pPoint_WS, pDestColorRGB ) )
			{
				pDestColorRGB->Black();
			}
			break;

		case FLIGHT_TYPE_AMBIENT:
			if ( !LightPointWithAmbientLight( pPoint_WS, pDestColorRGB ) )
			{
				pDestColorRGB->Black();
			}
			break;

		default:
			FASSERT_NOW;
	}
}


// IMPORTANT: This function WILL ALLOCATE MEMORY if a per-pixel or corona texture
//            is specified in pLightInit. Consider yourself WARNED MOFO!!!!
//            Oh, and if either of those textures cannot be loaded, don't expect
//            this function to inform your pathic ass about it!
//
void CFLight::Init( const FLightInit_t *pLightInit ) 
{
	u32 nFlags;

	// Reset the default values for this light
	fang_MemZero( this, sizeof( CFLight ) );
	m_nLightID = 0xffff;

	//FASSERT_UNIT_FLOAT( pLightInit->fIntensity );

	FCOLOR_FASSERT_VALID_MOTIF( &pLightInit->Motif );

	nFlags = pLightInit->nFlags & ~(FLIGHT_FLAG_HASDIR | FLIGHT_FLAG_HASPOS | FLIGHT_FLAG_INCLUDE);

	if (pLightInit->nFlags & FLIGHT_FLAG_CORONA)
	{
		m_nRayCastPhase = flight_GetPhase();

		m_fFade = 0.0f;
		m_fFadeDelta = 0.0f;
	}

	switch( pLightInit->nType ) 
	{
		case FLIGHT_TYPE_DIR:
			InitDirLight( &pLightInit->mtxOrientation.m_vFront );

			if ( pLightInit->nFlags & FLIGHT_FLAG_CAST_SHADOWS )
			{
				SetMaxShadowCount( 16 );
			}
			
			m_Motif = pLightInit->Motif;
			m_fIntensity = pLightInit->fIntensity;

			break;

		case FLIGHT_TYPE_OMNI:
			InitOmniLight( &pLightInit->spInfluence );

			m_Motif = pLightInit->Motif;
			m_fIntensity = pLightInit->fIntensity;
			m_fCoronaScale = pLightInit->fCoronaScale;

			if ( pLightInit->nFlags & FLIGHT_FLAG_CAST_SHADOWS )
			{
				SetMaxShadowCount( 4 );
			}
			
			if ( pLightInit->szCoronaTexName[0] )
			{			
				AllocAndSetCoronaTexture( pLightInit->szCoronaTexName );
			}

			break;

		case FLIGHT_TYPE_SPOT:
			InitSpotLight( &pLightInit->spInfluence, &pLightInit->mtxOrientation, pLightInit->fSpotInnerRadians, pLightInit->fSpotOuterRadians );

			m_Motif = pLightInit->Motif;
			m_fIntensity = pLightInit->fIntensity;
			m_fCoronaScale = pLightInit->fCoronaScale;

			if ( pLightInit->nFlags & FLIGHT_FLAG_CAST_SHADOWS )
			{
				SetMaxShadowCount( 3 );
			}
			
			if ( pLightInit->szPerPixelTexName[0] )
			{			
				AllocAndSetPerPixelTexture( pLightInit->szPerPixelTexName );
			}

			if ( pLightInit->szCoronaTexName[0] )
			{			
				AllocAndSetCoronaTexture( pLightInit->szCoronaTexName );
			}

			break;

		default:
			FASSERT_NOW;
	}

	m_pShadowTex = NULL;

	#if FANG_LIGHTING_2X
		m_fIntensity = m_fIntensity * 0.5f * FLight_fExposure;
		FMATH_CLAMP(m_fIntensity, 0.0f, FLight_fItensClamp);
		m_fIntensity = (m_fIntensity < 1.0f)?(m_fIntensity):(1.0f);
	#else
		m_fIntensity = (m_fIntensity < 1.0f)?(m_fIntensity):(1.0f);
	#endif

	u8 nNameLen = fclib_strlen( pLightInit->szName );
	if( nNameLen > FLIGHT_NAME_LEN )
	{
		nNameLen = FLIGHT_NAME_LEN;
	}
	fclib_strncpy( m_szName, pLightInit->szName, nNameLen );
	m_szName[nNameLen] = 0;
	m_nFlags |= nFlags;
	m_nLightID = pLightInit->nLightID;

	m_fDeltaScale = 1.0f;
	
	m_nIndex = FVis_nCurLightIdx;
	FVis_nCurLightIdx++;
}


//
//
//
void CFLight::InitOmniLight( f32 fPosX_WS, f32 fPosY_WS, f32 fPosZ_WS, f32 fRadius_WS ) 
{
	if ( fRadius_WS < 0.001f )
	{
		FASSERT( fRadius_WS > 0.0f );
		fRadius_WS = 0.001f;
	}

	m_nType = FLIGHT_TYPE_OMNI;
	m_nFlags = FLIGHT_FLAG_ENABLE | FLIGHT_FLAG_HASPOS;

	m_Motif.OpaqueWhite();
	#if FANG_LIGHTING_2X
	m_fIntensity = 0.5f;
	#else
	m_fIntensity = 1.0f;
	#endif

	fRadius_WS *= FLight_fLightRadiusAdj;

	m_spInfluence_WS.m_Pos.x = fPosX_WS;
	m_spInfluence_WS.m_Pos.y = fPosY_WS;
	m_spInfluence_WS.m_Pos.z = fPosZ_WS;
	m_spInfluence_WS.m_fRadius = fRadius_WS;
	m_fOOR_WS = 1.0f / fRadius_WS;
	m_fOOR2_WS = m_fOOR_WS * m_fOOR_WS;

	m_nXfmKey_VS = FXfm_nViewKey - 1;
	m_nColorCalcKey = FVid_nFrameCounter - 1;
}


//
//
//
void CFLight::InitOmniLight( const CFVec3 *pPos_WS, f32 fRadius_WS ) 
{
	if ( fRadius_WS < 0.001f )
	{
		FASSERT( fRadius_WS > 0.0f );
		fRadius_WS = 0.001f;
	}

	InitOmniLight( pPos_WS->x, pPos_WS->y, pPos_WS->z, fRadius_WS );
}


//
//
//
void CFLight::InitOmniLight( const CFSphere *pSphere_WS ) 
{
	InitOmniLight( pSphere_WS->m_Pos.x, pSphere_WS->m_Pos.y, pSphere_WS->m_Pos.z, pSphere_WS->m_fRadius );
}


//
//
//
void CFLight::InitDirLight( f32 fDirX_WS, f32 fDirY_WS, f32 fDirZ_WS ) 
{
	m_nType = FLIGHT_TYPE_DIR;
	m_nFlags = FLIGHT_FLAG_ENABLE | FLIGHT_FLAG_HASDIR;

	m_Motif.OpaqueWhite();
	m_fIntensity = 1.0f;

	flight_BuildLightMatrixFromDirection( &m_mtxOrientation_WS, fDirX_WS, fDirY_WS, fDirZ_WS );

	m_nXfmKey_VS = FXfm_nViewKey - 1;
	m_nColorCalcKey = FVid_nFrameCounter - 1;
}


//
//
//
void CFLight::InitDirLight( const CFVec3 *pDir_WS ) 
{
	m_nType = FLIGHT_TYPE_DIR;
	m_nFlags = FLIGHT_FLAG_ENABLE | FLIGHT_FLAG_HASDIR;

	m_Motif.OpaqueWhite();
	m_fIntensity = 1.0f;

	flight_BuildLightMatrixFromDirection( &m_mtxOrientation_WS, pDir_WS->x, pDir_WS->y, pDir_WS->z );

	m_nXfmKey_VS = FXfm_nViewKey - 1;
	m_nColorCalcKey = FVid_nFrameCounter - 1;
}


//
//
//
void CFLight::InitSpotLight( f32 fPosX_WS, f32 fPosY_WS, f32 fPosZ_WS, f32 fRadius_WS,
						   	 f32 fDirX_WS, f32 fDirY_WS, f32 fDirZ_WS,
						   	 f32 fInnerRadians, f32 fOuterRadians ) 
{
	FASSERT( fInnerRadians >= 0.0f );
	FASSERT( fOuterRadians < FMATH_PI );
	FASSERT( fInnerRadians <= fOuterRadians );

	if ( fRadius_WS < 0.001f )
	{
		FASSERT( fRadius_WS > 0.0f );
		fRadius_WS= 0.001f;
	}

	m_nType = FLIGHT_TYPE_SPOT;
	m_nFlags = FLIGHT_FLAG_ENABLE | FLIGHT_FLAG_HASPOS | FLIGHT_FLAG_HASDIR;

	m_Motif.OpaqueWhite();
	m_fIntensity = 1.0f;

	m_spInfluence_WS.m_Pos.x = fPosX_WS;
	m_spInfluence_WS.m_Pos.y = fPosY_WS;
	m_spInfluence_WS.m_Pos.z = fPosZ_WS;
	m_spInfluence_WS.m_fRadius = fRadius_WS;//*2.0f;
	m_fOOR_WS = 1.0f / fRadius_WS;
	m_fOOR2_WS = m_fOOR_WS * m_fOOR_WS;

	flight_BuildLightMatrixFromDirection( &m_mtxOrientation_WS, fDirX_WS, fDirY_WS, fDirZ_WS );

	m_fSpotInnerRadians = fInnerRadians;
	m_fSpotOuterRadians = fOuterRadians;
	m_fSpotK0 = fmath_Cos(fInnerRadians*0.5f);
	m_fSpotK1 = fmath_Cos(fOuterRadians*0.5f);
	m_fSpotK2 = 1.0f / ( m_fSpotK0 - m_fSpotK1 );
	
	#if FANG_PLATFORM_GC
	f32 fA1, fA2, fA0 = 0.0f;
	f32 fOO1mK;
	
	fOO1mK = 1.0f / (1.0f - m_fSpotK1);
	fA1 = -m_fSpotK1 * fOO1mK;
	fA2 = fOO1mK;
	
	m_fSpotGCK0 = fA0;
	m_fSpotGCK1 = fA1;
	m_fSpotGCK2 = fA2;
	#endif

	m_nXfmKey_VS = FXfm_nViewKey - 1;
	m_nColorCalcKey = FVid_nFrameCounter - 1;
}


//
//
//
void CFLight::InitSpotLight( const CFVec3 *pPos_WS, f32 fRadius_WS, const CFVec3 *pDir_WS, f32 fInnerRadians, f32 fOuterRadians ) 
{
	FASSERT( fRadius_WS > 0.0f );
	InitSpotLight( pPos_WS->x, pPos_WS->y, pPos_WS->z, fRadius_WS, pDir_WS->x, pDir_WS->y, pDir_WS->z, fInnerRadians, fOuterRadians );
}


//
//
//
void CFLight::InitSpotLight( const CFSphere *pSphere_WS, const CFVec3 *pDir_WS, f32 fInnerRadians, f32 fOuterRadians ) 
{
	FASSERT( pSphere_WS->m_fRadius > 0.0f );
	InitSpotLight( pSphere_WS->m_Pos.x, pSphere_WS->m_Pos.y, pSphere_WS->m_Pos.z, pSphere_WS->m_fRadius, pDir_WS->x, pDir_WS->y, pDir_WS->z, fInnerRadians, fOuterRadians );
}


//
//
//
void CFLight::InitSpotLight( const CFSphere *pSphere, const CFMtx43 *pOrientation, f32 fInnerRadians, f32 fOuterRadians ) 
{
	FASSERT( pSphere->m_fRadius > 0.0f );
	FASSERT( fInnerRadians >= 0.0f );
	FASSERT( fOuterRadians < FMATH_PI );
	FASSERT( fInnerRadians <= fOuterRadians );

	m_nType = FLIGHT_TYPE_SPOT;
	m_nFlags = FLIGHT_FLAG_ENABLE | FLIGHT_FLAG_HASPOS | FLIGHT_FLAG_HASDIR;

	m_Motif.OpaqueWhite();
	m_fIntensity = 1.0f;

	m_spInfluence_WS = *pSphere;
	//m_spInfluence_WS.m_fRadius *= 2.0f;
	m_fOOR_WS = 1.0f / m_spInfluence_WS.m_fRadius;
	m_fOOR2_WS = m_fOOR_WS * m_fOOR_WS;

	m_mtxOrientation_WS = *pOrientation;

	m_fSpotInnerRadians = fInnerRadians;
	m_fSpotOuterRadians = fOuterRadians;
	m_fSpotK0 = fmath_Cos(fInnerRadians*0.5f);
	m_fSpotK1 = fmath_Cos(fOuterRadians*0.5f);
	m_fSpotK2 = 1.0f / ( m_fSpotK0 - m_fSpotK1 );
	
	#if FANG_PLATFORM_GC
	f32 fA1, fA2, fA0 = 0.0f;
	f32 fOO1mK;
	
	fOO1mK = 1.0f / (1.0f - m_fSpotK1);
	fA1 = -m_fSpotK1 * fOO1mK;
	fA2 = fOO1mK;
	
	m_fSpotGCK0 = fA0;
	m_fSpotGCK1 = fA1;
	m_fSpotGCK2 = fA2;
	#endif

	m_nXfmKey_VS = FXfm_nViewKey - 1;
	m_nColorCalcKey = FVid_nFrameCounter - 1;
}


//
//
//
BOOL CFLight::SetMaxShadowCount( u32 nShadowCount )
{
	FASSERT( !m_papShadowMeshInst && !m_paShadowBoundingSphere && !m_pafShadowObjDist );
	FASSERT( m_nMaxShadowCount == 0 );
	
	FResFrame_t Frame = fres_GetFrame();
	
	m_papShadowMeshInst = (CFMeshInst **)fres_Alloc( sizeof( CFMeshInst *) * nShadowCount );
	if ( !m_papShadowMeshInst )
	{
		m_nFlags &= ~FLIGHT_FLAG_CAST_SHADOWS;
		fres_ReleaseFrame( Frame );
		return FALSE;
	}
	
	m_paShadowBoundingSphere = (CFSphere *)fres_Alloc( sizeof( CFSphere ) * nShadowCount );
	if ( !m_paShadowBoundingSphere )
	{
		m_nFlags &= ~FLIGHT_FLAG_CAST_SHADOWS;
		fres_ReleaseFrame( Frame );
		return FALSE;
	}
	
	m_pafShadowObjDist = (f32 *)fres_Alloc( sizeof( f32 ) * nShadowCount );
	if ( !m_pafShadowObjDist )
	{
		m_nFlags &= ~FLIGHT_FLAG_CAST_SHADOWS;
		fres_ReleaseFrame( Frame );
		return FALSE;
	}
	
	m_nMaxShadowCount = nShadowCount;
	
	return TRUE;
}


//
//
//
void CFLight::SetName( cchar *pszName ) 
{
	FASSERT( fclib_strlen(pszName) <= FLIGHT_NAME_LEN );
	fclib_strcpy( m_szName, pszName );
}


//
//
//
// NKM - Changed this to include FANG_LIGHTING_2X.  The intensity 
// can be > 1.0f now.
void CFLight::SetIntensity( /*f32 fUnitIntensity*/f32 fIntensity ) 
{
//	FASSERT_UNIT_FLOAT( fUnitIntensity );
	FASSERT_POS_FLOAT( fIntensity );

	m_fIntensity = fIntensity;
	#if FANG_LIGHTING_2X
		m_fIntensity = m_fIntensity * 0.5f * FLight_fExposure;
		FMATH_CLAMP(m_fIntensity, 0.0f, FLight_fItensClamp);
		m_fIntensity = (m_fIntensity < 1.0f)?(m_fIntensity):(1.0f);
	#else
		m_fIntensity = (m_fIntensity < 1.0f)?(m_fIntensity):(1.0f);
	#endif

	m_nColorCalcKey = FVid_nFrameCounter - 1;
}

f32 CFLight::GetIntensity( void ) const
{
#if FANG_LIGHTING_2X
	return m_fIntensity * 2.0f;
#else
	return m_fIntensity;
#endif
}

//
//
//
void CFLight::SetColor( f32 fUnitRed, f32 fUnitGreen, f32 fUnitBlue, f32 fUnitAlpha ) 
{
	FCOLOR_FASSERT_UNIT_RGBA( &m_Motif );

	m_Motif.Set( fUnitRed, fUnitGreen, fUnitBlue, fUnitAlpha );
	m_nColorCalcKey = FVid_nFrameCounter - 1;
}


//
//
//
void CFLight::SetColor( const CFColorRGBA *pColor ) 
{
	FCOLOR_FASSERT_UNIT_RGBA( pColor );

	m_Motif.Set( *pColor );
	m_nColorCalcKey = FVid_nFrameCounter - 1;
}


//
//
//
void CFLight::SetColor( const CFColorRGB *pColor ) 
{
	FCOLOR_FASSERT_UNIT_RGB( pColor );

	m_Motif.Set( *pColor );
	m_nColorCalcKey = FVid_nFrameCounter - 1;
}


//
//
//
void CFLight::SetMotif( const CFColorMotif *pMotif ) 
{
	FCOLOR_FASSERT_VALID_MOTIF( pMotif );

	m_Motif = *pMotif;
	m_nColorCalcKey = FVid_nFrameCounter - 1;
}


//
//
//
void CFLight::SetMotif( u32 nMotifIndex ) 
{
	FASSERT( nMotifIndex>=0 && nMotifIndex<FCOLORMOTIF_COUNT );

	m_Motif.nMotifIndex = nMotifIndex;
	m_nColorCalcKey = FVid_nFrameCounter - 1;
}


//
//
//
void CFLight::SetPosition( f32 fPosX_WS, f32 fPosY_WS, f32 fPosZ_WS ) 
{
	m_spInfluence_WS.m_Pos.x = fPosX_WS;
	m_spInfluence_WS.m_Pos.y = fPosY_WS;
	m_spInfluence_WS.m_Pos.z = fPosZ_WS;

	m_nXfmKey_VS = FXfm_nViewKey - 1;
}


//
//
//
void CFLight::SetPosition( const CFVec3A *pPos_WS ) 
{
	SetPosition( pPos_WS->x, pPos_WS->y, pPos_WS->z );
}


//
//
//
void CFLight::SetPosition( const CFSphere *pSphere_WS ) 
{
	SetPosition( pSphere_WS->m_Pos.x, pSphere_WS->m_Pos.y, pSphere_WS->m_Pos.z );
}


//
//
//
void CFLight::SetRadius( f32 fRadius_WS ) 
{
	m_spInfluence_WS.m_fRadius = fRadius_WS;
	m_fOOR_WS = 1.0f / fRadius_WS;
	m_fOOR2_WS = m_fOOR_WS * m_fOOR_WS;

	m_nXfmKey_VS = FXfm_nViewKey - 1;
}


//
//
//
void CFLight::SetRadius( const CFSphere *pSphere ) 
{
	SetRadius( pSphere->m_fRadius );
}


//
//
//
void CFLight::SetPositionAndRadius( f32 fPosX_WS, f32 fPosY_WS, f32 fPosZ_WS, f32 fRadius_WS ) 
{
	m_spInfluence_WS.m_Pos.x = fPosX_WS;
	m_spInfluence_WS.m_Pos.y = fPosY_WS;
	m_spInfluence_WS.m_Pos.z = fPosZ_WS;

	m_spInfluence_WS.m_fRadius = fRadius_WS;
	m_fOOR_WS = 1.0f / fRadius_WS;
	m_fOOR2_WS = m_fOOR_WS * m_fOOR_WS;

	m_nXfmKey_VS = FXfm_nViewKey - 1;
}


//
//
//
void CFLight::SetPositionAndRadius( const CFVec3A *pPos_WS, f32 fRadius_WS ) 
{
	SetPositionAndRadius( pPos_WS->x, pPos_WS->y, pPos_WS->z, fRadius_WS );
}


//
//
//
void CFLight::SetPositionAndRadius( const CFSphere *pSphere ) 
{
	SetPositionAndRadius( pSphere->m_Pos.x, pSphere->m_Pos.y, pSphere->m_Pos.z, pSphere->m_fRadius );
}


//
//
//
void CFLight::SetDirection( f32 fDirX_WS, f32 fDirY_WS, f32 fDirZ_WS ) 
{
	flight_BuildLightMatrixFromDirection( &m_mtxOrientation_WS, fDirX_WS, fDirY_WS, fDirZ_WS );

	m_nXfmKey_VS = FXfm_nViewKey - 1;
}


//
//
//
void CFLight::SetDirection( const CFVec3A *pDir_WS ) 
{
	SetDirection( pDir_WS->x, pDir_WS->y, pDir_WS->z );
}


//
//
//
void CFLight::SetSpotCone( f32 fInnerRadians, f32 fOuterRadians ) 
{
	FASSERT( fInnerRadians >= 0.0f );
	FASSERT( fOuterRadians < FMATH_PI );
	FASSERT( fInnerRadians <= fOuterRadians );

	m_fSpotInnerRadians = fInnerRadians;
	m_fSpotOuterRadians = fOuterRadians;
	m_fSpotK0 = fmath_Cos(fInnerRadians*0.5f);
	m_fSpotK1 = fmath_Cos(fOuterRadians*0.5f);
	m_fSpotK2 = 1.0f / ( m_fSpotK0 - m_fSpotK1 );
	
	#if FANG_PLATFORM_GC
	f32 fA1, fA2, fA0 = 0.0f;
	f32 fOO1mK;
	
	fOO1mK = 1.0f / (1.0f - m_fSpotK1);
	fA1 = -m_fSpotK1 * fOO1mK;
	fA2 = fOO1mK;
	
	m_fSpotGCK0 = fA0;
	m_fSpotGCK1 = fA1;
	m_fSpotGCK2 = fA2;
	#endif

	m_nXfmKey_VS = FXfm_nViewKey - 1;
}


//
//
//
BOOL CFLight::AllocAndSetPerPixelTexture( const char *pszTextureName ) 
{
	FASSERT( m_nType == FLIGHT_TYPE_SPOT );

	m_pProjectionTex = NULL;

	if ( !pszTextureName )
	{
		return FALSE;
	}
	
	m_pProjectionTex = fnew( CFTexInst );
	
	if ( !m_pProjectionTex )
	{
		return FALSE;
	}
	
	FTexDef_t *pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, pszTextureName );

	if ( pTexDef == NULL )
	{
		m_pProjectionTex->SetTexDef( NULL );
		DEVPRINTF( "CFLight::SetProjTexture(): Could not load texture '%s' for light projection.\n", pszTextureName );
		return FALSE;
	}
		
	m_pProjectionTex->SetTexDef( pTexDef );
	
	return TRUE;	
}


//
//
//
BOOL CFLight::AllocAndSetCoronaTexture( const char *pszTextureName ) 
{
	FASSERT( m_nType == FLIGHT_TYPE_SPOT || m_nType == FLIGHT_TYPE_OMNI );

	m_pCoronaTex = NULL;

	if ( !pszTextureName )
	{
		return FALSE;
	}

	m_pCoronaTex = fnew( CFTexInst );

	if ( !m_pCoronaTex )
	{
		return FALSE;
	}

	FTexDef_t *pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, pszTextureName );

	if ( pTexDef == NULL )
	{
		DEVPRINTF( "CFLight::SetProjTexture(): Could not load texture '%s' for light projection.\n", pszTextureName );
		return FALSE;
	}

	m_pCoronaTex->SetTexDef( pTexDef );

	return TRUE;	
}


//
//
//
void CFLight::Enable( BOOL bEnable ) 
{
	if ( bEnable ) 
	{
		m_nFlags |= FLIGHT_FLAG_ENABLE;
	} 
	else 
	{
		m_nFlags &= ~FLIGHT_FLAG_ENABLE;
	}
}


//
//
//
f32 CFLight::GetWeight( const CFSphere *pSphere )
{
	static CFVec3A __vRay_WS;

	switch ( m_nType )
	{
		case FLIGHT_TYPE_DIR:
		{
			// Compute the light's color...
			ComputeColor();

			// Compute the final color at the point...
			return m_ScaledColor.ColorRGB.GetWeight();
		}
		
		case FLIGHT_TYPE_OMNI:
		{
			// Compute light vector from point to light (toward light)...
			__vRay_WS.v3 = m_spInfluence_WS.m_Pos - pSphere->m_Pos;

			// Compute light's radius squared...
			f32 fLightRadius2_WS = (m_spInfluence_WS.m_fRadius + pSphere->m_fRadius) * (m_spInfluence_WS.m_fRadius + pSphere->m_fRadius);

			// See if light is too far away from point to light it...
			f32 fRayLength2_WS = __vRay_WS.MagSq();
			if ( fRayLength2_WS > fLightRadius2_WS ) 
			{
				// Light does not intersect point...
				return 0.f;
			}

			f32 fRadialAtten = (fLightRadius2_WS - fRayLength2_WS) * m_fOOR2_WS;
			ComputeColor();
			return m_ScaledColor.ColorRGB.GetWeight() * fRadialAtten;
		}
		
		case FLIGHT_TYPE_SPOT:
		{
			// Compute light vector from light to point
			__vRay_WS.v3 = pSphere->m_Pos - m_spInfluence_WS.m_Pos;

			f32 fRayLength2_WS = __vRay_WS.MagSq();

			f32 fRadiusSq = pSphere->m_fRadius * pSphere->m_fRadius;

			// If the light center point is within the radius provided, light must affect the surface
			if ( fRayLength2_WS < fRadiusSq )
			{
				// Compute the light's color...
				ComputeColor();
				return m_ScaledColor.ColorRGB.GetWeight();
			}

			// Compute light's radius squared...
			f32 fLightRadius2_WS = (m_spInfluence_WS.m_fRadius + pSphere->m_fRadius) * (m_spInfluence_WS.m_fRadius + pSphere->m_fRadius);

			// See if light is too far away from point to light it...
			if ( fRayLength2_WS > fLightRadius2_WS ) 
			{
				// Light does not intersect point...
				return 0.f;
			}

			f32 fDistAlongAxis = __vRay_WS.v3.Dot( m_mtxOrientation_WS.m_vFront );
			if ( fDistAlongAxis + pSphere->m_fRadius < 0.f )
			{
				// Sphere is behind 
				return 0.f;
			}

			CFVec3 vClosestPoint, vSphereToAxisPoint;
			vClosestPoint = (m_mtxOrientation_WS.m_vFront * fDistAlongAxis) + m_spInfluence_WS.m_Pos;
			vSphereToAxisPoint = pSphere->m_Pos - vClosestPoint;
			f32 fDistToAxisSq = vSphereToAxisPoint.Mag2();
			if ( fDistToAxisSq < fRadiusSq )
			{
				// Compute the light's color...
				f32 fRadialAtten = (fLightRadius2_WS - fRayLength2_WS) * m_fOOR2_WS;
				ComputeColor();
				return m_ScaledColor.ColorRGB.GetWeight() * fRadialAtten;
			}

			// Calculate closest point on sphere to axis
			vClosestPoint = ((vSphereToAxisPoint * fmath_InvSqrt(fDistToAxisSq)) * pSphere->m_fRadius * 0.95f) + pSphere->m_Pos;
			__vRay_WS = vClosestPoint - m_spInfluence_WS.m_Pos;
			__vRay_WS.Unitize();

			//Multiply in a small fudge factor to account for accumulated error.
			f32 fDot = __vRay_WS.v3.Dot( m_mtxOrientation_WS.m_vFront )*1.25f;

			if ( fDot <= m_fSpotK1 )
			{
				return 0.f;
			}

			// Compute the light's color...
			f32 fRadialAtten = (fLightRadius2_WS - fRayLength2_WS) * m_fOOR2_WS * fDot;
			ComputeColor();
			return m_ScaledColor.ColorRGB.GetWeight();
		}
		
		case FLIGHT_TYPE_AMBIENT:
		{
			FASSERT_NOW;
			return 0.f;
		}
	}
	
	return 0.f;
}


//
//
//
BOOL CFLight::DoesSpotConeIntersectSphere( const CFSphere *pSphere ) const
{
	CFCone Cone;

	FASSERT( m_nType == FLIGHT_TYPE_SPOT );

	Cone.m_fAngle = m_fSpotOuterRadians * 0.5f;
	Cone.m_UnitDir = m_mtxOrientation_WS.m_vFront;
	Cone.m_Vertex = m_spInfluence_WS.m_Pos;

	return pSphere->IsIntersecting( Cone );
}


//
//
//
BOOL CFLight::LightPointWithAmbientLight( const CFVec3A *pPoint_WS, CFColorRGB *pDestColorRGB ) 
{
	CFVec3A UnitRay_WS;

	FASSERT( m_nType == FLIGHT_TYPE_AMBIENT );

	// Compute the light's color...
	ComputeColor();

	// Compute the final color at the point...
	*pDestColorRGB = m_ScaledColor.ColorRGB;
	
	return TRUE;
}


//
//
//
BOOL CFLight::LightDirectionalPointWithDirLight( const CFVec3A *pPoint_WS, const CFVec3A *pUnitNormal_WS, CFColorRGB *pDestColorRGB ) 
{
	CFVec3A vUnitRay_WS;
	f32 fDot;

	FASSERT( m_nType == FLIGHT_TYPE_DIR );

	// Compute the angle the light ray is hitting the point at...
	fDot = m_mtxOrientation_WS.m_vFront.Dot( pUnitNormal_WS->v3 );
	if ( fDot <= 0.00001f ) 
	{
		// Light is lighting the back side of the point...
		return FALSE;
	}

	// Compute the light's color...
	ComputeColor();

	// Compute the final color at the point...
	*pDestColorRGB = m_ScaledColor.ColorRGB * fDot;
	
	return TRUE;
}


//
//
//
BOOL CFLight::LightPointWithDirLight( const CFVec3A *pPoint_WS, CFColorRGB *pDestColorRGB )
{
	FASSERT( m_nType == FLIGHT_TYPE_DIR );

	// Compute the light's color...
	ComputeColor();

	// Compute the final color at the point...
	*pDestColorRGB = m_ScaledColor.ColorRGB;
	
	return TRUE;
}


//
//
//
BOOL CFLight::LightDirectionalPointWithOmniLight( const CFVec3A *pPoint_WS, const CFVec3A *pUnitNormal_WS, CFColorRGB *pDestColorRGB ) 
{
	CFVec3A vRay_WS;
	f32 fIntensity, fDot, fOORayLength_WS, fRayLength_WS, fRayLength2_WS, fLightRadius2_WS, fRadialAtten;

	FASSERT( m_nType == FLIGHT_TYPE_OMNI );

	// Compute light vector from point to light (toward light)...
	vRay_WS.v3 = m_spInfluence_WS.m_Pos - pPoint_WS->v3;

	// Compute light's radius squared...
	fLightRadius2_WS = m_spInfluence_WS.m_fRadius * m_spInfluence_WS.m_fRadius;

	// See if light is too far away from point to light it...
	fRayLength2_WS = vRay_WS.MagSq();
	if ( fRayLength2_WS > fLightRadius2_WS ) 
	{
		// Light does not intersect point...
		return FALSE;
	}

	// Compute the angle the light ray is hitting the point at...
	fDot = vRay_WS.Dot( *pUnitNormal_WS );
	if ( fDot <= 0.00001f ) 
	{
		// Light is lighting the back side of the point...
		return FALSE;
	}

	// Compute the light distance and the reciprocal light distance...
	fRayLength_WS = fmath_Sqrt( fRayLength2_WS );
	fOORayLength_WS = 1.0f / fRayLength_WS;

	// Normalize the dot product...
	fDot *= fOORayLength_WS;

	// Compute the radial attenuation...
	#if FANG_PLATFORM_DX
		// Xbox & PC attenuation model:

		#if _LIGHT_ATTEN_NO_K1
			fRadialAtten = 1.0f / (FLIGHT_ATTEN_K0 + FLIGHT_ATTEN_K2*m_fOOR2_WS*fRayLength2_WS);
		#else
			fRadialAtten = 1.0f / (FLIGHT_ATTEN_K0 + FLIGHT_ATTEN_K1*m_fOOR_WS*fRayLength_WS + FLIGHT_ATTEN_K2*m_fOOR2_WS*fRayLength2_WS);
		#endif
//ARG - >>>>>
	#elif FANG_PLATFORM_PS2
		// PS2 attenuation model:
		fRadialAtten = (fLightRadius2_WS - fRayLength2_WS) * m_fOOR2_WS;
//ARG - <<<<<
	#elif FANG_PLATFORM_GC
		// GC uses PS2 attenuation model for now
		fRadialAtten = (fLightRadius2_WS - fRayLength2_WS) * m_fOOR2_WS;
	#else
		#error <Fang Error: No valid platform identified for establishing light attenuation.>
	#endif

	// Compute intensity...
	FASSERT( fRadialAtten>=0.0f && fRadialAtten<=1.0f );
	fIntensity = fRadialAtten * fDot;

	// Compute the light's color...
	ComputeColor();

	// Compute the final color at the point...
	*pDestColorRGB = m_ScaledColor.ColorRGB * fIntensity;
	
	return TRUE;
}


//
//
//
BOOL CFLight::LightPointWithOmniLight( const CFVec3A *pPoint_WS, CFColorRGB *pDestColorRGB, f32 fRadius/*=0.f*/ ) 
{
	CFVec3A vRay_WS;
	f32 fRayLength2_WS, fLightRadius2_WS, fRadialAtten;
	
	FASSERT( m_nType == FLIGHT_TYPE_OMNI );

	// Compute light vector from point to light (toward light)...
	vRay_WS.v3 = m_spInfluence_WS.m_Pos - pPoint_WS->v3;

	// Compute light's radius squared...
	fLightRadius2_WS = (m_spInfluence_WS.m_fRadius + fRadius) * (m_spInfluence_WS.m_fRadius + fRadius);

	// See if light is too far away from point to light it...
	fRayLength2_WS = vRay_WS.MagSq();
	if ( fRayLength2_WS > fLightRadius2_WS ) 
	{
		// Light does not intersect point...
		return FALSE;
	}

	// Compute the radial attenuation...
	#if FANG_PLATFORM_DX
		// Xbox & PC attenuation model:

		#if _LIGHT_ATTEN_NO_K1
			fRadialAtten = 1.0f / (FLIGHT_ATTEN_K0 + FLIGHT_ATTEN_K2*m_fOOR2_WS*fRayLength2_WS);
		#else
			fRadialAtten = 1.0f / (FLIGHT_ATTEN_K0 + FLIGHT_ATTEN_K1*m_fOOR_WS*fmath_Sqrt(fRayLength2_WS) + FLIGHT_ATTEN_K2*m_fOOR2_WS*fRayLength2_WS);
		#endif
	#elif FANG_PLATFORM_GC
		#if _LIGHT_ATTEN_NO_K1
			fRadialAtten = 1.0f / (FLIGHT_ATTEN_K0 + FLIGHT_ATTEN_K2*m_fOOR2_WS*fRayLength2_WS);
		#else
			fRadialAtten = 1.0f / (FLIGHT_ATTEN_K0 + FLIGHT_ATTEN_K1*m_fOOR_WS*fmath_Sqrt(fRayLength2_WS) + FLIGHT_ATTEN_K2*m_fOOR2_WS*fRayLength2_WS);
		#endif
//ARG - >>>>>
//ARG - same as GC at the moment
	#elif FANG_PLATFORM_PS2
		#if _LIGHT_ATTEN_NO_K1
			fRadialAtten = 1.0f / (FLIGHT_ATTEN_K0 + FLIGHT_ATTEN_K2*m_fOOR2_WS*fRayLength2_WS);
		#else
			fRadialAtten = 1.0f / (FLIGHT_ATTEN_K0 + FLIGHT_ATTEN_K1*m_fOOR_WS*fmath_Sqrt(fRayLength2_WS) + FLIGHT_ATTEN_K2*m_fOOR2_WS*fRayLength2_WS);
		#endif
//ARG - <<<<<
	#else
		#error <Fang Error: No valid platform identified for establishing light attenuation.>
	#endif

	// Compute the light's color...
	ComputeColor();

	// Compute the final color at the point...
	FASSERT( fRadialAtten>=0.0f && fRadialAtten<=1.0f );
	*pDestColorRGB = m_ScaledColor.ColorRGB * fRadialAtten;
	
	return TRUE;
}


 //
//
//
BOOL CFLight::LightDirectionalPointWithSpotLight( const CFVec3A *pPoint_WS, const CFVec3A *pUnitNormal_WS, CFColorRGB *pDestColorRGB ) 
{
	CFVec3A vRay_WS, vUnitRay_WS, vNegUnitLightDir_WS;
	f32 fIntensity, fDot, fSpotDot, fOORayLength_WS, fRayLength_WS, fRayLength2_WS, fLightRadius2_WS, fRadialAtten, fSpot;

	FASSERT( m_nType == FLIGHT_TYPE_SPOT );

	// Compute light vector from point to light (toward light)...
	vRay_WS.v3 = m_spInfluence_WS.m_Pos - pPoint_WS->v3;

	// Compute light's radius squared...
	fLightRadius2_WS = m_spInfluence_WS.m_fRadius * m_spInfluence_WS.m_fRadius;

	// See if light is too far away from point to light it...
	fRayLength2_WS = vRay_WS.MagSq();
	if ( fRayLength2_WS > fLightRadius2_WS ) 
	{
		// Light does not intersect point...
		return FALSE;
	}

	// Compute the angle the light ray is hitting the point at...
	fDot = vRay_WS.Dot( *pUnitNormal_WS );
	if ( fDot <= 0.00001f ) 
	{
		// Light is lighting the back side of the point...
		return FALSE;
	}

	// Compute the light distance and the reciprocal light distance...
	fRayLength_WS = fmath_Sqrt( fRayLength2_WS );
	fOORayLength_WS = 1.0f / fRayLength_WS;

	// Compute the unit ray...
	vUnitRay_WS.Mul( vRay_WS, fOORayLength_WS );

	// Compute spot cone dot product...
	vNegUnitLightDir_WS.v3 = -m_mtxOrientation_WS.m_vFront;
	fSpotDot = vUnitRay_WS.Dot( vNegUnitLightDir_WS );
	if ( fSpotDot <= m_fSpotK1 ) 
	{
		// Point is outside the spotlight cone...
		return FALSE;
	}

	// Normalize the dot product...
	fDot *= fOORayLength_WS;

	// Compute the radial attenuation...
	#if FANG_PLATFORM_DX
		// Xbox & PC attenuation model:

		#if _LIGHT_ATTEN_NO_K1
			fRadialAtten = 1.0f / (FLIGHT_ATTEN_K0 + FLIGHT_ATTEN_K2*m_fOOR2_WS*fRayLength2_WS);
		#else
			fRadialAtten = 1.0f / (FLIGHT_ATTEN_K0 + FLIGHT_ATTEN_K1*m_fOOR_WS*fRayLength_WS + FLIGHT_ATTEN_K2*m_fOOR2_WS*fRayLength2_WS);
		#endif
//ARG - >>>>>
	#elif FANG_PLATFORM_PS2
		// PS2 attenuation model:
		fRadialAtten = (fLightRadius2_WS - fRayLength2_WS) * m_fOOR2_WS;
//ARG - <<<<<
	#elif FANG_PLATFORM_GC
		// GC uses PS2 attenuation model for now
		fRadialAtten = (fLightRadius2_WS - fRayLength2_WS) * m_fOOR2_WS;
	#else
		#error <Fang Error: No valid platform identified for establishing light attenuation.>
	#endif

	// Compute intensity...
	FASSERT( fRadialAtten>=0.0f && fRadialAtten<=1.0f );
	fIntensity = fRadialAtten * fDot;

	if ( fSpotDot <= m_fSpotK0 ) 
	{
		// Point is between inner and outer light cone...
		fSpot = (fSpotDot - m_fSpotK1) * m_fSpotK2;
		fIntensity *= fSpot;
	}

	// Compute the light's color...
	ComputeColor();

	// Compute the final color at the point...
	*pDestColorRGB = m_ScaledColor.ColorRGB * fIntensity;
	
	return TRUE;
}


//
//
//
BOOL CFLight::LightPointWithSpotLight( const CFVec3A *pPoint_WS, CFColorRGB *pDestColorRGB, f32 fRadius/*=0.f*/ ) 
{
	CFVec3A vRay_WS, vUnitRay_WS, vNegUnitLightDir_WS;
	f32 fIntensity, fSpotDot, fOORayLength_WS, fRayLength_WS, fRayLength2_WS, fLightRadius2_WS, fRadialAtten, fSpot;

	FASSERT( m_nType == FLIGHT_TYPE_SPOT );

	// Compute light vector from point to light (toward light)...
	vRay_WS.v3 = m_spInfluence_WS.m_Pos - pPoint_WS->v3;

	fRayLength2_WS = vRay_WS.MagSq();

	// If the light center point is within the radius provided, light must affect the surface
	if ( fRadius && fRayLength2_WS < fRadius * fRadius )
	{
		// Compute the light's color...
		ComputeColor();

		// Compute the final color at the point...
		*pDestColorRGB = m_ScaledColor.ColorRGB;
		
		return TRUE;
	}

	// Compute light's radius squared...
	fLightRadius2_WS = (m_spInfluence_WS.m_fRadius + fRadius) * (m_spInfluence_WS.m_fRadius + fRadius);

	// See if light is too far away from point to light it...
	if ( fRayLength2_WS > fLightRadius2_WS ) 
	{
		// Light does not intersect point...
		return FALSE;
	}

	// Compute the light distance and the reciprocal light distance...
	fRayLength_WS = fmath_Sqrt(fRayLength2_WS) - fRadius;
	fOORayLength_WS = 1.0f / fRayLength_WS;

	// Compute the unit ray...
	vUnitRay_WS.Mul( vRay_WS, fOORayLength_WS );

	// Compute spot cone dot product...
	vNegUnitLightDir_WS.v3 = -m_mtxOrientation_WS.m_vFront;
	fSpotDot = vUnitRay_WS.Dot( vNegUnitLightDir_WS );
	
	if ( fSpotDot <= m_fSpotK1 ) 
	{
		// Point is outside the spotlight cone...
		return FALSE;
	}

	// Compute the radial attenuation...
	#if FANG_PLATFORM_DX
		// Xbox & PC attenuation model:
		#if _LIGHT_ATTEN_NO_K1
			fRadialAtten = 1.0f / (FLIGHT_ATTEN_K0 + FLIGHT_ATTEN_K2*m_fOOR2_WS*fRayLength2_WS);
		#else
			fRadialAtten = 1.0f / (FLIGHT_ATTEN_K0 + FLIGHT_ATTEN_K1*m_fOOR_WS*fRayLength_WS + FLIGHT_ATTEN_K2*m_fOOR2_WS*fRayLength2_WS);
		#endif
//ARG - >>>>>
	#elif FANG_PLATFORM_PS2
		// PS2 attenuation model:
		fRadialAtten = (fLightRadius2_WS - fRayLength2_WS) * m_fOOR2_WS;
//ARG - <<<<<
	#elif FANG_PLATFORM_GC
		fRadialAtten = (fLightRadius2_WS - fRayLength2_WS) * m_fOOR2_WS;
	#else
		#error <Fang Error: No valid platform identified for establishing light attenuation.>
	#endif

	// Compute intensity...
	FASSERT( fRadialAtten>=0.0f && fRadialAtten<=1.01f );
	FMATH_CLAMP( fRadialAtten, 0.f, 1.f );
	fIntensity = fRadialAtten;

	if ( fSpotDot <= m_fSpotK0 ) 
	{
		// Point is between inner and outer light cone...
		fSpot = (fSpotDot - m_fSpotK1) * m_fSpotK2;
		fIntensity *= fSpot;
	}

	// Compute the light's color...
	ComputeColor();

	// Compute the final color at the point...
	*pDestColorRGB = m_ScaledColor.ColorRGB * fIntensity;
	
	return TRUE;
}

void CFLight::RenderRange()
{
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER_OR_EQUAL );
	fdraw_SetTexture( NULL );
    fdraw_Alpha_SetBlendOp(FDRAW_BLENDOP_SRC);
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );

	fdraw_FacetedWireSphere( &m_spInfluence_WS.m_Pos, (1.0f/m_fOOR_WS), 2, 2, &m_ScaledColor );
	fdraw_FacetedWireSphere( &m_spInfluence_WS.m_Pos, 1.0f, 2, 2, &m_ScaledColor );
}

//
//
//
void CFLight::RenderCorona(const CFMtx43A *pCamera, float fCamDistSq, u32 nPhase)
{
	if ( (m_nFlags&FLIGHT_FLAG_CORONA) && (m_nFlags&FLIGHT_FLAG_ENABLE) && (m_nFlags&FLIGHT_FLAG_HASPOS) )
	{
		fdraw_RenderCorona( pCamera, this, fCamDistSq, m_nFlags&FLIGHT_FLAG_CORONA_PROXFADE, nPhase );
	}
}
