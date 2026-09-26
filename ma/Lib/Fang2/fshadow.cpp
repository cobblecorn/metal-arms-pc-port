//////////////////////////////////////////////////////////////////////////////////////
// fshadow.cpp - 
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 07/10/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fshadow.h"
#include "fmesh.h"


f32 FShadow_ShadowDist_MP;

f32 FShadow_fShadowStartFade_MP2;
f32 FShadow_fOOShadowFadeDist_MP2;

f32 FShadow_Max_Detail_Dist;
f32 FShadow_Max_Circle_Dist;
f32 FShader_ShadowRecieveDist;
f32 FShader_ShadowLightDist;

BOOL FShadow_bMultiplayer = FALSE;

static BOOL _bModuleInitialized;


//
//
//
BOOL fshadow_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

	FShadow_Max_Detail_Dist = 40.0f;
	FShadow_Max_Circle_Dist = 75.0f;
	FShader_ShadowRecieveDist = 100.0f;
	FShader_ShadowLightDist = 100.0f;

	FShadow_ShadowDist_MP = 60.0f;

	FShadow_fShadowStartFade_MP2 = (35.0f*35.0f);
	FShadow_fOOShadowFadeDist_MP2 = (1.0f/(25.0f*25.0f));

	_bModuleInitialized = TRUE;

	return TRUE;
}


//
//
//
void fshadow_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

	_bModuleInitialized = FALSE;
}


//
//
//
f32 fshadow_CheckObjDist( CFMeshInst *pMesh, f32 &fFade2 )
{
	f32 fRet = 1.0f, fD, fShD2;
	
	if (pMesh->m_nFlags & FMESHINST_FLAG_DONT_DRAW)
	{
		fRet = 0.0f;
	}
	else if (pMesh && !FShadow_bMultiplayer && !(pMesh->m_nFlags&FMESHINST_FLAG_NOSHADOWLOD) )
	{
		f32 fMul = 1.0f, fMul2 = 1.0f;
		f32 fRadius = pMesh->m_BoundSphere_MS.m_fRadius;
		
		if (fRadius > 10.0f)
		{
			fMul = fRadius*0.2f;
			fMul2 = fMul*fMul;
		}

#if 1
		CFVec3 vDist = (pMesh->m_Xfm.m_MtxF.m_vPos.v3 - FMesh_vShadowCam);
		fShD2 = (FShadow_Max_Detail_Dist * FShadow_Max_Detail_Dist) * fMul2;
		vDist.y *= FSHADOW_Y_DISTANCE_WEIGHT;
		fD = (vDist.x * vDist.x) + (vDist.y * vDist.y) + (vDist.z * vDist.z);
		if ( fD > fShD2 )
		{
			fRet = 0.0f;

			fShD2 = (FShadow_Max_Circle_Dist * FShadow_Max_Circle_Dist) * fMul2;
			if ( fD > fShD2 )
			{
				fFade2 = 0.f;
			}
			else
			{
				fFade2 = 0.75f * (1.0f - fmath_Div( fD, fShD2 ));
				FMATH_CLAMP( fFade2, 0.0f, 0.5f );
			}
		}
		else
		{
			fRet = 2.0f * (1.0f - fmath_Div( fD, fShD2 ));
			FMATH_CLAMP( fRet, 0.0f, 1.0f );
			
			if ( fRet <= 0.5f )
			{
				fFade2 = 0.5f - (fRet * fRet * 2.f);
			}
			else 
			{
				fFade2 = 0.0f;
			}
		}
#else
		f32 fDX = fmath_Abs( pMesh->m_Xfm.m_MtxF.m_vPos.v3.x - FMesh_vShadowCam.x );
		f32 fDY = fmath_Abs( (pMesh->m_Xfm.m_MtxF.m_vPos.v3.y - FMesh_vShadowCam.y) * FSHADOW_Y_DISTANCE_WEIGHT );
		f32 fDZ = fmath_Abs( pMesh->m_Xfm.m_MtxF.m_vPos.v3.z - FMesh_vShadowCam.z );

		if ( fDX > FShadow_Max_Detail_Dist*fMul || fDY > FShadow_Max_Detail_Dist*fMul || fDZ > FShadow_Max_Detail_Dist*fMul )
		{
			fRet = 0.0f;

			if ( fDX > FShadow_Max_Circle_Dist*fMul || fDY > FShadow_Max_Circle_Dist*fMul || fDZ > FShadow_Max_Circle_Dist*fMul )
			{
				fFade2 = 0.0f;
			}
			else
			{
				fShD2 = FShadow_Max_Circle_Dist*FShadow_Max_Circle_Dist*fMul2;
				fD = fDX*fDX + fDY*fDY + fDZ*fDZ;
				if (fD > fShD2)
				{
					fFade2 = 0.0f;
				}
				else
				{
					fFade2 = 2.0f * (1.0f - fD / fShD2);
					fFade2 = (fFade2 < 0.5f)?(fFade2):(0.5f);
				}
			}
		}
		else
		{
			fShD2 = FShadow_Max_Detail_Dist*FShadow_Max_Detail_Dist*fMul2;
			fD = fDX*fDX + fDY*fDY + fDZ*fDZ;
			if (fD > fShD2)
			{
				fRet = 0.0f;
				fFade2 = 0.5f;
			}
			else
			{
				fRet = 2.0f * (1.0f - fD / fShD2);
				fRet = (fRet < 1.0f)?(fRet):(1.0f);
				
				if (fRet <= 0.5f)
				{
					fFade2 = 0.5f - fRet;
				}
				else fFade2 = 0.0f;
			}
		}
#endif
	}
	else if ( FShadow_bMultiplayer || (pMesh->m_nFlags&FMESHINST_FLAG_NOSHADOWLOD) )
	{
		fRet = 1.0f;
		fFade2 = 0.0f;
	}
	else
	{
		fRet = 0.0f;
	}
	
	return fRet;
}


//
//
//
BOOL fshadow_LightCastShadow( CFLight *pLight )
{
	if (pLight->m_nType == FLIGHT_TYPE_DIR)
	{
		return TRUE;
	}
/*
	if ( !FShadow_bMultiplayer )
	{
		if (FMATH_FABS(FMesh_vShadowCam.x - pLight->m_spInfluence_WS.m_Pos.x) > FShader_ShadowLightDist || FMATH_FABS(FMesh_vShadowCam.y - pLight->m_spInfluence_WS.m_Pos.y) > FShader_ShadowLightDist ||
			FMATH_FABS(FMesh_vShadowCam.z - pLight->m_spInfluence_WS.m_Pos.z) > FShader_ShadowLightDist)
		{
			return FALSE;
		}
	}
*/
	return TRUE;
}


