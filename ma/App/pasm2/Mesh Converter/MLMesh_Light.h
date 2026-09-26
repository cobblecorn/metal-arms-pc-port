//////////////////////////////////////////////////////////////////////////////////////
// MLMesh_Light.h - Classes used to convert generic mesh data into Fang platform specific data
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
// 08/15/02 Lafleur		Adapted from GCMeshLight to be multi-platform
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __MLMESH_LIGHT_H_
#define __MLMESH_LIGHT_H_

#include "fang.h"
#include "fmesh.h"


// Forward class declarations
class MLMesh;


//
//
//
class MLMeshLight
{
	public:
		FMeshLight_t	m_FMeshLight;
		cchar			*m_pszParentBoneName;

		// Link to the next Light - used MLMesh
		MLMeshLight		*m_pNext;

	public:
		MLMeshLight( ApeLight_t *pApeLight )
		{
			m_pszParentBoneName = NULL;
			strncpy( m_FMeshLight.LightInit.szName, pApeLight->szLightName, FLIGHT_NAME_LEN );
			m_FMeshLight.LightInit.szName[FLIGHT_NAME_LEN] = 0;

			m_FMeshLight.LightInit.nFlags = 0;
			if ( MLMesh_bVolumeMesh ) 
			{
				// handle world light flags
				if ( (pApeLight->nFlags & APE_LIGHT_FLAG_LIGHT_SELF) == 0 ) 
				{
					m_FMeshLight.LightInit.nFlags |= FLIGHT_FLAG_NOLIGHT_TERRAIN;	
				}
			} 
			else 
			{
				// handle object light flags
				if ( pApeLight->nFlags & APE_LIGHT_FLAG_LIGHT_SELF ) 
				{
					m_FMeshLight.LightInit.nFlags |= FLIGHT_FLAG_LIGHT_ATTACHED;	
				}
				if ( pApeLight->nFlags & APE_LIGHT_FLAG_OBJ_DONT_LIGHT_TERRAIN ) 
				{
					m_FMeshLight.LightInit.nFlags |= FLIGHT_FLAG_NOLIGHT_TERRAIN;	
				}
			}

			if( pApeLight->nFlags & APE_LIGHT_FLAG_PER_PIXEL ) 
			{
				m_FMeshLight.LightInit.nFlags |= FLIGHT_FLAG_PER_PIXEL;	
			}

			if( pApeLight->nFlags & APE_LIGHT_FLAG_MESH_MUST_BE_PER_PIXEL ) 
			{
				m_FMeshLight.LightInit.nFlags |= FLIGHT_FLAG_MESH_MUST_BE_PER_PIXEL;
			}
			
			FASSERT( !(pApeLight->nFlags & APE_LIGHT_FLAG_LIGHTMAP_ONLY_LIGHT) );
			m_FMeshLight.LightInit.nFlags |= FLIGHT_FLAG_ENGINE_LIGHT;	
			
			if( pApeLight->nFlags & APE_LIGHT_FLAG_DYNAMIC_ONLY ) 
			{
				m_FMeshLight.LightInit.nFlags |= FLIGHT_FLAG_DYNAMIC_ONLY;	
			}
			
			if( pApeLight->nFlags & APE_LIGHT_FLAG_LIGHTMAP_LIGHT ) 
			{
				m_FMeshLight.LightInit.nFlags |= FLIGHT_FLAG_LIGHTMAP_LIGHT;	
			}
			
			if( pApeLight->nFlags & APE_LIGHT_FLAG_UNIQUE_LIGHTMAP ) 
			{
				m_FMeshLight.LightInit.nFlags |= FLIGHT_FLAG_UNIQUE_LIGHTMAP;	
			}
			
			if( pApeLight->nFlags & APE_LIGHT_FLAG_CORONA ) 
			{
				m_FMeshLight.LightInit.nFlags |= FLIGHT_FLAG_CORONA;	
			}
			
			if( pApeLight->nFlags & APE_LIGHT_FLAG_CORONA_PROXFADE ) 
			{
				m_FMeshLight.LightInit.nFlags |= FLIGHT_FLAG_CORONA_PROXFADE;
			}
			
			if( pApeLight->nFlags & APE_LIGHT_FLAG_CAST_SHADOWS ) 
			{
				m_FMeshLight.LightInit.nFlags |= FLIGHT_FLAG_CAST_SHADOWS;
			}

			switch( pApeLight->nType ) 
			{
				default:
					FASSERT_NOW;
				case APE_LIGHT_TYPE_DIR:
					m_FMeshLight.LightInit.nType = FLIGHT_TYPE_DIR;
					m_FMeshLight.LightInit.nFlags |= FLIGHT_FLAG_HASDIR;
					m_FMeshLight.LightInit.mtxOrientation = pApeLight->mtxOrientation;
//					flight_BuildLightMatrixFromDirection( &m_FMeshLight.LightInit.mtxOrientation_WS, pApeLight->Dir.x, pApeLight->Dir.y, pApeLight->Dir.z );
					break;

				case APE_LIGHT_TYPE_OMNI:
					m_FMeshLight.LightInit.nType = FLIGHT_TYPE_OMNI;
					m_FMeshLight.LightInit.nFlags |= FLIGHT_FLAG_HASPOS;
					m_FMeshLight.LightInit.mtxOrientation.Identity();
					break;

				case APE_LIGHT_TYPE_SPOT:
					m_FMeshLight.LightInit.nType = FLIGHT_TYPE_SPOT;
					m_FMeshLight.LightInit.nFlags |= (FLIGHT_FLAG_HASPOS | FLIGHT_FLAG_HASDIR);
					m_FMeshLight.LightInit.mtxOrientation = pApeLight->mtxOrientation;
//					flight_BuildLightMatrixFromDirection( &m_FMeshLight.LightInit.mtxOrientation_WS, pApeLight->Dir.x, pApeLight->Dir.y, pApeLight->Dir.z );
					break;

				case APE_LIGHT_TYPE_AMBIENT:
					m_FMeshLight.LightInit.nType = FLIGHT_TYPE_AMBIENT;
					m_FMeshLight.LightInit.mtxOrientation.Identity();
					break;
			}

			if ( pApeLight->szParentBoneName[0] != 0 )
			{
				m_pszParentBoneName = pApeLight->szParentBoneName;
			}

			m_FMeshLight.LightInit.Motif.nMotifIndex = pApeLight->nMotifID;

			if ( pApeLight->nFlags & APE_LIGHT_FLAG_DONT_USE_RGB ) 
			{
				// don't use the light's rgb, fill with white
				m_FMeshLight.LightInit.Motif.White();			
			} 
			else 
			{
				// copy the light's rgb
				m_FMeshLight.LightInit.Motif.ColorRGB = pApeLight->Color;
			}

			m_FMeshLight.LightInit.nLightID = pApeLight->nLightID;
			m_FMeshLight.LightInit.fCoronaScale = pApeLight->fCoronaScale;
			m_FMeshLight.LightInit.Motif.fAlpha = 1.0f;
			m_FMeshLight.LightInit.fIntensity = pApeLight->fIntensity;
			m_FMeshLight.LightInit.spInfluence = pApeLight->Sphere;
			m_FMeshLight.LightInit.fSpotInnerRadians = pApeLight->fSpotInnerAngle;
			m_FMeshLight.LightInit.fSpotOuterRadians = pApeLight->fSpotOuterAngle;

			strncpy( m_FMeshLight.LightInit.szCoronaTexName, pApeLight->szCoronaTexture, FLIGHT_TEXTURE_NAME_LEN );
			strncpy( m_FMeshLight.LightInit.szPerPixelTexName, pApeLight->szPerPixelTexture, FLIGHT_TEXTURE_NAME_LEN );

			m_pNext = NULL;
		}

};



#endif
