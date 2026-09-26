#include "stdafx.h"

//FANG STUFF
#include "fversion.h"
#include "fang.h"
#include "floop.h"
#include "fviewport.h"
#include "fmath.h"
#include "fresload.h"
#include "fres.h"
#include "fmesh.h"
#include "fworld.h"
#include "fworld_coll.h"
#include "ftimer.h"
#include "fboxfilter.h"
#include "ftext.h"
#include "fanim.h"
#include "fcoll.h"
#include "fperf.h"
#include "fdraw.h"
#include "fclib.h"
#include "frenderer.h"
#include "fpsprite.h"
#include "dx/fdx8mesh.h"
#include "fshadow.h"
#include "ffile.h"
#include "fvis.h"
#include "fangalign.h"
#include "dx/fdx8vid.h"
#include "dx/fdx8.h"
#include "dx/fdx8mesh.h"
#include "dx/fdx8xfm.h"
#include "dx/fdx8vb.h"
#include "fshaders.h"

#include "../KongToVisFile.h"
#include "../Pasm.h"
#include "../PasmDlg.h"
#include "../CompileDlg.h"

//LightMapping Stuff
#include "LightMapGen.h"
#include "Lambertian.h"
#include "RayTracer.h"
#include "Radiosity.h"
//

extern CPasmApp theApp;

#define _START_WITH_NO_VERT_COLOR				FALSE
#define _USE_ALL_LIGHTS_FOR_STATIC_LIGHTING		FALSE

CLightMapGen::CLightMapGen(void)
{
	m_LightMapType = LT_VERTEX;
	m_nLightMapQuality = 0;
	m_nMaxMemory = 2048; //2.0 MB

	m_LightingSolution = NULL;
	//m_pWinPrintf = NULL;
	m_pTempUVBuffer = NULL;

	m_fAmbRed = m_fAmbGreen = m_fAmbBlue = 0.0f;
}

CLightMapGen::~CLightMapGen(void)
{
	if (m_pTempUVBuffer)
	{
		free(m_pTempUVBuffer);
		m_pTempUVBuffer = NULL;
	}
	if (m_LightingSolution)
	{
		delete m_LightingSolution;
		m_LightingSolution = NULL;
	}
}

void CLightMapGen::FreeData()
{
	if (m_pTempUVBuffer)
	{
		free(m_pTempUVBuffer);
		m_pTempUVBuffer = NULL;
	}
	if (m_LightingSolution)
	{
		delete m_LightingSolution;
		m_LightingSolution = NULL;
	}
}

u32 _nTriIdxOffs=0;
u32 _nTriGroupOffs=0;
u32 _nVtxGroupOffs=0;

void CLightMapGen::SetParam(u32 nLightMapQuality, f32 fMaxMemory, f32 fSubSample, cchar *pszLevelName, BOOL bFilterLightMaps)
{
	//remove the old lighting solution

	FASSERT(pszLevelName);

	if (m_LightingSolution)
	{
		delete m_LightingSolution;
		m_LightingSolution = NULL;
	}

	if (nLightMapQuality == 0)
	{
		m_LightMapType = LT_NONE;
		m_LightingSolution = NULL;
		m_nLightMapQuality = 0;
	}
	else if (nLightMapQuality < 0xff)
	{
		m_LightMapType = LT_RADIOSITY;
		m_LightingSolution = new CRadiosity;
		m_nLightMapQuality = nLightMapQuality - 1;
	}
	else
	{
		m_LightMapType = LT_VERTEX;
		m_LightingSolution = new CLambertian;
		m_nLightMapQuality = 0;
		m_nLightMapQuality = 0xff;
	}
	m_nMaxMemory = (u32)( fMaxMemory*1024.0f );

	if (m_nMaxMemory < LT_MIN_MEMORY)
	{
		m_nMaxMemory = LT_MIN_MEMORY;
	}

	m_LightingSolution->SetMaxMemory(m_nMaxMemory);
	m_LightingSolution->SetSubSampling(fSubSample);
	m_LightingSolution->SetLevelName(pszLevelName);
	m_LightingSolution->SetQuality(m_nLightMapQuality);
	//m_LightingSolution->SetWinPrintf(m_pWinPrintf);

	_nTriIdxOffs=0;
	_nTriGroupOffs=0;
	_nVtxGroupOffs=0;
}

CLightingSolution::LS_LightingVtx_t CLightMapGen::GetLightingVtx(u32 nVtxType, void *pVtxData, u16 nVIdx)
{
	CLightingSolution::LS_LightingVtx_t Vtx;
	f32 fOO255 = (1.0f/255.0f);

	switch (nVtxType)
	{
		case FDX8VB_TYPE_N1C1T1:
			{
				FDX8VB_N1C1T1_t *pVB = (FDX8VB_N1C1T1_t *)pVtxData;
				Vtx.x = pVB[nVIdx].fPosX; Vtx.y = pVB[nVIdx].fPosY; Vtx.z = pVB[nVIdx].fPosZ;
				Vtx.u = pVB[nVIdx].fS0; Vtx.v = pVB[nVIdx].fT0;
				Vtx.fBaseU = pVB[nVIdx].fS0; Vtx.fBaseV = pVB[nVIdx].fT0;
			}
			break;
		case FDX8VB_TYPE_N1C1T2:
			{
				FDX8VB_N1C1T2_t *pVB = (FDX8VB_N1C1T2_t *)pVtxData;
				Vtx.x = pVB[nVIdx].fPosX; Vtx.y = pVB[nVIdx].fPosY; Vtx.z = pVB[nVIdx].fPosZ;
				Vtx.u = pVB[nVIdx].fS1; Vtx.v = pVB[nVIdx].fT1; 
				Vtx.fBaseU = pVB[nVIdx].fS0; Vtx.fBaseV = pVB[nVIdx].fT0;
			}
			break;
		case FDX8VB_TYPE_N1W3C1T1:
			{
				FDX8VB_N1W3C1T1_t *pVB = (FDX8VB_N1W3C1T1_t *)pVtxData;
				Vtx.x = pVB[nVIdx].fPosX; Vtx.y = pVB[nVIdx].fPosY; Vtx.z = pVB[nVIdx].fPosZ;
				Vtx.u = pVB[nVIdx].fS0; Vtx.v = pVB[nVIdx].fT0; 
				Vtx.fBaseU = pVB[nVIdx].fS0; Vtx.fBaseV = pVB[nVIdx].fT0;
			}
			break;
		case FDX8VB_TYPE_N1W3C1T2:
			{
				FDX8VB_N1W3C1T2_t *pVB = (FDX8VB_N1W3C1T2_t *)pVtxData;
				Vtx.x = pVB[nVIdx].fPosX; Vtx.y = pVB[nVIdx].fPosY; Vtx.z = pVB[nVIdx].fPosZ;
				Vtx.u = pVB[nVIdx].fS1; Vtx.v = pVB[nVIdx].fT1; 
				Vtx.fBaseU = pVB[nVIdx].fS0; Vtx.fBaseV = pVB[nVIdx].fT0;
			}
			break;
		case FDX8VB_TYPE_TL_C2T2:
			{
				FDX8VB_TL_C2T2_t *pVB = (FDX8VB_TL_C2T2_t *)pVtxData;
				Vtx.x = pVB[nVIdx].fPosX; Vtx.y = pVB[nVIdx].fPosY; Vtx.z = pVB[nVIdx].fPosZ;
				Vtx.u = pVB[nVIdx].fS1; Vtx.v = pVB[nVIdx].fT1; 
				Vtx.fBaseU = pVB[nVIdx].fS0; Vtx.fBaseV = pVB[nVIdx].fT0;
			}
			break;
		case FDX8VB_TYPE_C1:
			{
				FDX8VB_C1_t *pVB = (FDX8VB_C1_t *)pVtxData;
				Vtx.x = pVB[nVIdx].fPosX; Vtx.y = pVB[nVIdx].fPosY; Vtx.z = pVB[nVIdx].fPosZ;
				Vtx.u = 0; Vtx.v = 0; 
				Vtx.fBaseU = 0; Vtx.fBaseV = 0;
			}
			break;
		case FDX8VB_TYPE_C1T1:
			{
				FDX8VB_C1T1_t *pVB = (FDX8VB_C1T1_t *)pVtxData;
				Vtx.x = pVB[nVIdx].fPosX; Vtx.y = pVB[nVIdx].fPosY; Vtx.z = pVB[nVIdx].fPosZ;
				Vtx.u = pVB[nVIdx].fS0; Vtx.v = pVB[nVIdx].fT0; 
				Vtx.fBaseU = pVB[nVIdx].fS0; Vtx.fBaseV = pVB[nVIdx].fT0;
			}
			break;
		default:
			{
				Vtx.x = 0; Vtx.y = 0; Vtx.z = 0;
				Vtx.u = 0; Vtx.v = 0; 
			}
			break;
	};

	return (Vtx);
}

void CLightMapGen::TransformVtx(CLightingSolution::LS_LightingVtx_t& Vtx, CFMtx43A& Mtx)
{
	CFVec3A vXF, vPoint;
	vPoint.Set(Vtx.x, Vtx.y, Vtx.z);

	vXF = Mtx.MulPoint(vPoint);

	Vtx.x = vXF.x; Vtx.y = vXF.y; Vtx.z = vXF.z;
}

//
//
//
BOOL LMG_StaticallyLightMaterial( KongMat_t *pKMat )
{
	if ( pKMat->pProperties->nFlags & APE_MAT_FLAGS_NO_LM_USE )
	{
		if ( !(pKMat->pProperties->nFlags & APE_MAT_FLAGS_VERT_RADIOSITY) )
		{
			return FALSE;
		}
	}

	if ( pKMat->pProperties->nFlags & APE_MAT_FLAGS_NO_DRAW )
	{
		return FALSE;
	}
	
	if ( pKMat->pProperties->nShaderNum == APE_SHADER_TYPE_ADD_BASE || pKMat->pProperties->nShaderNum == APE_SHADER_TYPE_ADD_vBASE )
	{
		return FALSE;
	}

	return TRUE;
}


void CLightMapGen::AppendGeometryCount( KongMesh_t *pKongMesh, u32& nTri, u32& nVtx )
{
	if ( !pKongMesh )
	{
		FASSERT_NOW;
		return;
	}

	u32 i;
	for ( i = 0; i < pKongMesh->nSegmentCount; i++ ) 
	{
		KongSeg_t *pKSeg = &pKongMesh->paSegs[i];
		if ( !pKSeg )
		{
			continue;
		}

		// Walk the materials	
		KongMat_t *pKMat = pKongMesh->GetFirstMaterial( pKSeg );
		while ( pKMat )
		{
			nTri += pKMat->nNumTris;
			nVtx += (pKMat->nNumTris*3);
			pKMat = pKongMesh->GetNextMaterial( pKMat );
		}
	}
}

void CLightMapGen::AddVolumeKongMesh( s32 nVolume, KongMesh_t *pKongMesh, CFSphere *pSphere, CFMtx43 *pMtx/*=NULL*/, BOOL bAcceptLMs/*=TRUE*/ )
{
	if ( !pKongMesh )
	{
		FASSERT_NOW;
		return;
	}

	if (m_nCurrentVolume != nVolume)
	{
		m_LightingSolution->SetVolumeVtxGroup(nVolume, _nVtxGroupOffs);
	}
	m_nCurrentVolume = nVolume;

	u32 i, nTriIdx;
	CFVec3 vTemp;
	f32 fInvMatrixScale = 1.f;
	CLightingSolution::LS_LightingVtx_t Vtx;

	if ( pMtx )
	{
		fInvMatrixScale = pMtx->m_vFront.Mag2();
		if ( fInvMatrixScale > 0.000001f )
		{
			fInvMatrixScale = fmath_AcuInvSqrt( fInvMatrixScale );
//			pMtx->m_vRight.Mul( fInvMatrixScale );
//			pMtx->m_vUp.Mul( fInvMatrixScale );
//			pMtx->m_vFront.Mul( fInvMatrixScale );
		}
	}

	m_LightingSolution->SetCurrentVolume_Vtx( nVolume );
	m_LightingSolution->SetVolumeRadius( nVolume, pSphere->m_fRadius );
	m_LightingSolution->SetVolumeStaticUV( nVolume, !!(pMtx != NULL) );

	// Walk the segments
	for ( i = 0; i < pKongMesh->nSegmentCount; i++ ) 
	{
		KongSeg_t *pKSeg = &pKongMesh->paSegs[i];
		if ( !pKSeg )
		{
			continue;
		}

		// Walk the materials	
		KongMat_t *pKMat = pKongMesh->GetFirstMaterial( pKSeg );
		while ( pKMat )
		{
			if ( !pMtx && !LMG_StaticallyLightMaterial( pKMat ) )
			{
				pKMat = pKongMesh->GetNextMaterial( pKMat );
				continue;
			}

			// Mark this material as statically lit
			pKMat->nMatFlags |= KONGMAT_FLAGS_STATICALLY_LIT;

			BOOL bMatCastShadows = !(pKMat->pProperties->nFlags & APE_MAT_FLAGS_DO_NOT_BLOCK_LM);
			BOOL bMatAcceptLMs   =    !(pKMat->pProperties->nFlags & APE_MAT_FLAGS_DO_NOT_LM) 
								   && !(pKMat->pProperties->nFlags & APE_MAT_FLAGS_VERT_RADIOSITY) 
								   && bAcceptLMs;

			if ( !pMtx )
			{
				// Volume geometry
				m_LightingSolution->CreateMaterial( m_nCurrentVolume, pKMat->pProperties->TintRGB.fRed, pKMat->pProperties->TintRGB.fGreen, pKMat->pProperties->TintRGB.fBlue, FALSE/*pKongMesh->KHeader.bHasLightmapsSTs*/, bMatAcceptLMs, bMatCastShadows ); //generated uv and lightmaps.
				if (pKMat->pProperties->LightRGBI.fAlpha > 0.0f)
				{
					m_LightingSolution->SetAreaLightData(pKMat->pProperties->LightRGBI.fAlpha, pKMat->pProperties->LightRGBI.fRed, pKMat->pProperties->LightRGBI.fGreen, pKMat->pProperties->LightRGBI.fBlue);
				}
			}
			else 
			{
				// Object instance
				m_LightingSolution->CreateMaterial( m_nCurrentVolume, pKMat->pProperties->TintRGB.fRed, pKMat->pProperties->TintRGB.fGreen, pKMat->pProperties->TintRGB.fBlue, TRUE/*pKongMesh->KHeader.bHasLightmapsSTs*/, bMatAcceptLMs, bMatCastShadows, pMtx );
			}

			if ( pKMat->pProperties->nTransparencyMask != -1 )
			{
				if (pKMat->pProperties->aLayer[0].szTexnames[pKMat->pProperties->nTransparencyMask][0])
				{
					m_LightingSolution->SetCurMaterial_ZMask( pKMat->pProperties->aLayer[0].szTexnames[pKMat->pProperties->nTransparencyMask] );
				}
			}
			if ( pKMat->pProperties->bEmissiveOn )
			{
				CFColorRGBA ColorEmissive;
				if( pKMat->pProperties->bUseEmissiveColor ) 
				{
					ColorEmissive.Set( pKMat->pProperties->EmissiveRGB.fRed, pKMat->pProperties->EmissiveRGB.fGreen, pKMat->pProperties->EmissiveRGB.fBlue, 1.0f );
				} 
				else 
				{
					// put white into this layer's emmissive color
					ColorEmissive.Set( 1.0f, 1.0f, 1.0f, 1.0f );
				}

				if (ColorEmissive.fRed > 0.5f || ColorEmissive.fGreen > 0.5f || ColorEmissive.fBlue > 0.5f)
				{
					if ( pKMat->pProperties->nEmissiveMask != -1 )
					{
						if (pKMat->pProperties->aLayer[0].szTexnames[pKMat->pProperties->nEmissiveMask][0])
						{
							m_LightingSolution->SetCurMaterial_Emissive( pKMat->pProperties->aLayer[0].szTexnames[pKMat->pProperties->nEmissiveMask] );
							m_LightingSolution->SetCurMaterial_EmissiveAlphaMask( TRUE );
						}
					}
					else
					{
						if (pKMat->pProperties->aLayer[0].szTexnames[APE_LAYER_TEXTURE_DIFFUSE][0])
						{
							m_LightingSolution->SetCurMaterial_Emissive( pKMat->pProperties->aLayer[0].szTexnames[APE_LAYER_TEXTURE_DIFFUSE] );
							m_LightingSolution->SetCurMaterial_EmissiveClr( ColorEmissive.fRed, ColorEmissive.fGreen, ColorEmissive.fBlue );
							m_LightingSolution->SetCurMaterial_EmissiveAlphaMask( FALSE );
						}
					}
				}
			}

			u32 nFirstLMUV = FShaders_aShaderRegs[pKMat->pProperties->nSurfaceShaderID].nUVCount;

			nTriIdx = 0;
			KongTri_t *pTri = pKongMesh->GetFirstTri( pKMat );
			while ( pTri )
			{
#if 1
				CFVec3 vTemp;
				vTemp = pTri->apKongVerts[0]->Pos - pTri->apKongVerts[1]->Pos;
				if ( vTemp.Mag2() < 0.00000001f )
				{
					DEVPRINTF( "WARNING:  Zero Area Triangle detected!\n" );
				}
				else
				{
					vTemp = pTri->apKongVerts[1]->Pos - pTri->apKongVerts[2]->Pos;
					if ( vTemp.Mag2() < 0.00000001f )
					{
						DEVPRINTF( "WARNING:  Zero Area Triangle detected!\n" );
					}
					else
					{
						vTemp = pTri->apKongVerts[2]->Pos - pTri->apKongVerts[0]->Pos;
						if ( vTemp.Mag2() < 0.00000001f )
						{
							DEVPRINTF( "WARNING:  Zero Area Triangle detected!\n" );
						}
					}
				}
#endif

				// Add the vertex info
				if ( pMtx )
				{
					vTemp = pMtx->MultPoint( pTri->apKongVerts[0]->Pos );
					Vtx.x = vTemp.x;
					Vtx.y = vTemp.y;
					Vtx.z = vTemp.z;
					vTemp = pMtx->MultDir( pTri->apKongVerts[0]->Norm );
					Vtx.nx = vTemp.x * fInvMatrixScale;
					Vtx.ny = vTemp.y * fInvMatrixScale;
					Vtx.nz = vTemp.z * fInvMatrixScale;
//					DEVPRINTF( "Vert %04d: %f, %f, %f\n", pTri->apKongVerts[0]->nMeshVertIdx, Vtx.x, Vtx.y, Vtx.z );
				}
				else
				{
					Vtx.x = pTri->apKongVerts[0]->Pos.x;
					Vtx.y = pTri->apKongVerts[0]->Pos.y;
					Vtx.z = pTri->apKongVerts[0]->Pos.z;
					Vtx.nx = pTri->apKongVerts[0]->Norm.x;
					Vtx.ny = pTri->apKongVerts[0]->Norm.y;
					Vtx.nz = pTri->apKongVerts[0]->Norm.z;
				}

				#if _START_WITH_NO_VERT_COLOR
					Vtx.r = 0.f;
					Vtx.g = 0.f;
					Vtx.b = 0.f;
					Vtx.a = 0.f;
				#else
					Vtx.r = pTri->apKongVerts[0]->Color.fRed;
					Vtx.g = pTri->apKongVerts[0]->Color.fGreen;
					Vtx.b = pTri->apKongVerts[0]->Color.fBlue;
					Vtx.a = pTri->apKongVerts[0]->Color.fAlpha;
				#endif

				//fill these with pre-generated lightmap uvs for instance objects.
				if ( pKongMesh->KHeader.bHasLightmapsSTs )
				{
					Vtx.u = pTri->apKongVerts[0]->aUV[nFirstLMUV].a[0];
					Vtx.v = pTri->apKongVerts[0]->aUV[nFirstLMUV].a[1];
				}
				else
				{
					Vtx.u = pTri->apKongVerts[0]->aUV[0].a[0];
					Vtx.v = pTri->apKongVerts[0]->aUV[0].a[1];
				}

				Vtx.fBaseU = pTri->apKongVerts[0]->aUV[0].a[0];
				Vtx.fBaseV = pTri->apKongVerts[0]->aUV[0].a[1];
				m_LightingSolution->AddVertex(Vtx, pTri->apKongVerts[0]->nMeshVertIdx);

				// Add the vertex info
				if ( pMtx )
				{
					vTemp = pMtx->MultPoint( pTri->apKongVerts[1]->Pos );
					Vtx.x = vTemp.x;
					Vtx.y = vTemp.y;
					Vtx.z = vTemp.z;
					vTemp = pMtx->MultDir( pTri->apKongVerts[1]->Norm );
					Vtx.nx = vTemp.x * fInvMatrixScale;
					Vtx.ny = vTemp.y * fInvMatrixScale;
					Vtx.nz = vTemp.z * fInvMatrixScale;
//					DEVPRINTF( "Vert %04d: %f, %f, %f\n", pTri->apKongVerts[1]->nMeshVertIdx, Vtx.x, Vtx.y, Vtx.z );
				}
				else
				{
					Vtx.x = pTri->apKongVerts[1]->Pos.x;
					Vtx.y = pTri->apKongVerts[1]->Pos.y;
					Vtx.z = pTri->apKongVerts[1]->Pos.z;
					Vtx.nx = pTri->apKongVerts[1]->Norm.x;
					Vtx.ny = pTri->apKongVerts[1]->Norm.y;
					Vtx.nz = pTri->apKongVerts[1]->Norm.z;
				}

				//fill these with pre-generated lightmap uvs for instance objects.
				if ( pKongMesh->KHeader.bHasLightmapsSTs )
				{
					Vtx.u = pTri->apKongVerts[1]->aUV[nFirstLMUV].a[0];
					Vtx.v = pTri->apKongVerts[1]->aUV[nFirstLMUV].a[1];
				}
				else
				{
					Vtx.u = pTri->apKongVerts[1]->aUV[0].a[0];
					Vtx.v = pTri->apKongVerts[1]->aUV[0].a[1];
				}

				#if _START_WITH_NO_VERT_COLOR
					Vtx.r = 0.f;
					Vtx.g = 0.f;
					Vtx.b = 0.f;
					Vtx.a = 0.f;
				#else
					Vtx.r = pTri->apKongVerts[1]->Color.fRed;
					Vtx.g = pTri->apKongVerts[1]->Color.fGreen;
					Vtx.b = pTri->apKongVerts[1]->Color.fBlue;
					Vtx.a = pTri->apKongVerts[1]->Color.fAlpha;
				#endif

				Vtx.fBaseU = pTri->apKongVerts[1]->aUV[0].a[0];
				Vtx.fBaseV = pTri->apKongVerts[1]->aUV[0].a[1];
				m_LightingSolution->AddVertex(Vtx, pTri->apKongVerts[1]->nMeshVertIdx);

				if ( pMtx )
				{
					vTemp = pMtx->MultPoint( pTri->apKongVerts[2]->Pos );
					Vtx.x = vTemp.x;
					Vtx.y = vTemp.y;
					Vtx.z = vTemp.z;
					vTemp = pMtx->MultDir( pTri->apKongVerts[2]->Norm );
					Vtx.nx = vTemp.x * fInvMatrixScale;
					Vtx.ny = vTemp.y * fInvMatrixScale;
					Vtx.nz = vTemp.z * fInvMatrixScale;
//					DEVPRINTF( "Vert %04d: %f, %f, %f\n", pTri->apKongVerts[2]->nMeshVertIdx, Vtx.x, Vtx.y, Vtx.z );
				}
				else
				{
					Vtx.x = pTri->apKongVerts[2]->Pos.x;
					Vtx.y = pTri->apKongVerts[2]->Pos.y;
					Vtx.z = pTri->apKongVerts[2]->Pos.z;
					Vtx.nx = pTri->apKongVerts[2]->Norm.x;
					Vtx.ny = pTri->apKongVerts[2]->Norm.y;
					Vtx.nz = pTri->apKongVerts[2]->Norm.z;
				}

				//fill these with pre-generated lightmap uvs for instance objects.
				if ( pKongMesh->KHeader.bHasLightmapsSTs )
				{
					Vtx.u = pTri->apKongVerts[2]->aUV[nFirstLMUV].a[0];
					Vtx.v = pTri->apKongVerts[2]->aUV[nFirstLMUV].a[1];
				}
				else
				{
					Vtx.u = pTri->apKongVerts[2]->aUV[0].a[0];
					Vtx.v = pTri->apKongVerts[2]->aUV[0].a[1];
				}

				#if _START_WITH_NO_VERT_COLOR
					Vtx.r = 0.f;
					Vtx.g = 0.f;
					Vtx.b = 0.f;
					Vtx.a = 0.f;
				#else
					Vtx.r = pTri->apKongVerts[2]->Color.fRed;
					Vtx.g = pTri->apKongVerts[2]->Color.fGreen;
					Vtx.b = pTri->apKongVerts[2]->Color.fBlue;
					Vtx.a = pTri->apKongVerts[2]->Color.fAlpha;
				#endif

				Vtx.fBaseU = pTri->apKongVerts[2]->aUV[0].a[0];
				Vtx.fBaseV = pTri->apKongVerts[2]->aUV[0].a[1];
				m_LightingSolution->AddVertex(Vtx, pTri->apKongVerts[2]->nMeshVertIdx);

				u32 nIdx = (nTriIdx * 3) + _nTriIdxOffs;
				m_LightingSolution->AddTriangle( m_nCurrentVolume, nIdx, nIdx + 1, nIdx + 2 );
				pTri = pKongMesh->GetNextTri( pTri );
				nTriIdx++;
			}
			//m_LightingSolution->SetTriCurrentMaterial( 0 + _nTriGroupOffs, _nTriIdxOffs/3, pKMat->nNumTris );

			_nTriIdxOffs += pKMat->nNumTris * 3;

			pKMat = pKongMesh->GetNextMaterial( pKMat );
		}
	}
}

void CLightMapGen::AddApeLight( ApeLight_t *pLight )
{
	if (!pLight) return;

	CFVec3 vDir, vPos;
	CFVec3 vDAtten, vAAtten;
	CFVec3 vColor;
	f32 fOOR2=0.0f, fIntensity;
	CLightingSolution::LS_LightType_e LightType;

	switch ( pLight->nType )
	{
		case APE_LIGHT_TYPE_DIR:
			{
				LightType = CLightingSolution::LS_TYPE_NOATTEN;
				vDir = pLight->Dir;
				vPos.Set(-vDir.x*1000000.0f,-vDir.y*1000000.0f,-vDir.z*1000000.0f);

				fOOR2 = 1000000.0f * 10.0f; //everything.

				vDAtten.x = 1.0f;
				vDAtten.y = 0.0f;
				vDAtten.z = 0.0f;

				vAAtten.x = 1.0f;
				vAAtten.y = 0.f;
				vAAtten.z = 0.f;
			}
			break;
		case APE_LIGHT_TYPE_OMNI:
			{
				LightType = CLightingSolution::LS_LTYPE_POINT;
				vDir.Set(0,0,0);
				vPos = pLight->Sphere.m_Pos;
				fOOR2 = fmath_Inv( pLight->Sphere.m_fRadius * pLight->Sphere.m_fRadius );
				vDAtten.x = FLIGHT_ATTEN_K0;
				vDAtten.y = 0.0f;
				vDAtten.z = (FLIGHT_ATTEN_K2 * fOOR2);
				vAAtten.x = 1.0f;
				vAAtten.y = vAAtten.z = 0.0f;
			}
			break;
		case APE_LIGHT_TYPE_SPOT:
			{
				LightType = CLightingSolution::LS_LTYPE_SPOT;
				vDir = pLight->Dir;
				vPos = pLight->Sphere.m_Pos;

				fOOR2 = fmath_Inv( pLight->Sphere.m_fRadius * pLight->Sphere.m_fRadius );
				vDAtten.x = FLIGHT_ATTEN_K0;
				vDAtten.y = 0.0f;
				vDAtten.z = (FLIGHT_ATTEN_K2 * fOOR2);

				vAAtten.x = 1.0f;
				vAAtten.y = cosf(pLight->fSpotOuterAngle*0.5f);
				vAAtten.z = 1.0f/( cosf(pLight->fSpotInnerAngle*0.5f) - cosf(pLight->fSpotOuterAngle*0.5f) );
			}
			break;
		case APE_LIGHT_TYPE_AMBIENT:
			{
//				DEVPRINTF( "Using Ambient %f, %f, %f\n", pLight->Color.fRed, pLight->Color.fGreen, pLight->Color.fBlue );
				m_fAmbRed = pLight->Color.fRed*0.5f*pLight->fIntensity;
				m_fAmbGreen = pLight->Color.fGreen*0.5f*pLight->fIntensity;
				m_fAmbBlue = pLight->Color.fBlue*0.5f*pLight->fIntensity;
				return;				
			}
			break;
		default:
			{
				return;
			}
			break;
	}

	fIntensity = pLight->fIntensity;
	vColor.x = pLight->Color.fRed;
	vColor.y = pLight->Color.fGreen;
	vColor.z = pLight->Color.fBlue;

	if ( pLight->nFlags & APE_LIGHT_FLAG_UNIQUE_LIGHTMAP )
	{
		m_LightingSolution->AddLight(m_nCurrentVolume, LightType, vDir, vPos, vDAtten, vAAtten, vColor, fIntensity, 1.0f, fOOR2, ((u32)pLight->nLightID) << 16);
	}
	else
	{
		m_LightingSolution->AddLight(m_nCurrentVolume, LightType, vDir, vPos, vDAtten, vAAtten, vColor, fIntensity, 1.0f, fOOR2, pLight->nMotifID);
	}
}

void CLightMapGen::InitWithWorldData( CKongToVisFile *pKongToVis, CCompileDlg *pDlg, LMG_Object_t *pStaticObjects )
{
	KongMesh_t *pKongMesh;
	FASSERT( pKongToVis );

	if ( m_LightingSolution )
	{
		u32 i, j, nCount;
		LMG_Object_t *pLMObject;

		_nTriIdxOffs = 0;
		_nTriGroupOffs = 0;
		_nVtxGroupOffs = 0;

		m_LightingSolution->SetGeoInstanceVolume( pKongToVis->m_nStatsNumMeshes );
		m_LightingSolution->SetTriGroup( _nTriGroupOffs );
		m_LightingSolution->SetVtxGroup( _nVtxGroupOffs );

		// Add in the geometry counts for world data
		u32 nTri=0, nVtx=0;
		for ( i = 0; i < pKongToVis->m_nStatsNumMeshes; i++ ) 
		{
			FVisVolume_t *pVolume = &FVis_pVisData->paVolumes[i];
			pKongMesh = ((CApeToKongFormat *)pKongToVis->m_apVolKongMeshs[i])->m_pKongMesh;

			for ( j = 0; j < pKongMesh->nSegmentCount; j++ ) 
			{
				KongSeg_t *pKSeg = &pKongMesh->paSegs[j];
				if ( !pKSeg )
				{
					continue;
				}

				// Walk the materials	
				KongMat_t *pKMat = pKongMesh->GetFirstMaterial( pKSeg );
				while ( pKMat )
				{
					if ( !LMG_StaticallyLightMaterial( pKMat ) )
					{
						pKMat = pKongMesh->GetNextMaterial( pKMat );
						continue;
					}

					pKMat->nMatFlags |= KONGMAT_FLAGS_STATICALLY_LIT;

					nTri += pKMat->nNumTris;
					nVtx += (pKMat->nNumTris*3);
					pKMat = pKongMesh->GetNextMaterial( pKMat );
				}
			}
//
//			AppendGeometryCount( ((CApeToKongFormat *)pKongToVis->m_apVolKongMeshs[i])->m_pKongMesh, nTri, nVtx );
		}

		// Add in geometry count for instanced objects
		nCount = 0;
#if LMG_INCLUDE_STATIC_APE_OBJECTS
		pLMObject = pStaticObjects;
		while ( pLMObject )
		{
			u32 nObjectTriCount = 0, nObjectVertCount = 0;
			pKongMesh = NULL;
			if ( pDlg )
			{
				pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );
			}

			FASSERT( pLMObject->pFirstInstance );
			ApeObject_t *pObject = pKongToVis->GetObjectInfo( pLMObject->pFirstInstance->nApeObjectIdx );
			CString csPath = pObject->szName;

			if ( pLMObject->nCompileResult == CDLG_RESULT_COMPILE_SUCCESSFUL )
			{
				pKongMesh = new KongMesh_t;
				csPath = CPasmDlg::m_csPASMTempDirectory + csPath + ".kng";
			}

			if ( pKongMesh )
			{
				// Attempt to load the KNG for this object
				if ( pKongMesh->OpenFileAndLoadKong( csPath ) )
				{
					AppendGeometryCount( pKongMesh, nObjectTriCount, nObjectVertCount );

					// Gather some needed info
					pLMObject->nMaterialCount = 0;
					pLMObject->nVertexCount = 0;

					u16 anTempMatVertCounts[512];
					for ( i = 0; i < pKongMesh->nSegmentCount; i++ ) 
					{
						// Walk the materials	
						KongMat_t *pKMat = pKongMesh->GetFirstMaterial( &pKongMesh->paSegs[i] );
						while ( pKMat )
						{
							anTempMatVertCounts[pLMObject->nMaterialCount++] = pKMat->nNumTris * 3;
							pLMObject->nVertexCount += pKMat->nNumTris * 3;

							pKMat = pKongMesh->GetNextMaterial( pKMat );
						}
					}

					// Setup the material vertex counts
					pLMObject->panMaterialVertexCount = (u16 *)malloc( sizeof(u16) * pLMObject->nMaterialCount );
					if ( pLMObject->panMaterialVertexCount )
					{
						memcpy( pLMObject->panMaterialVertexCount, anTempMatVertCounts, sizeof(u16) * pLMObject->nMaterialCount );
					}

					// Add in the counts for each instance we will be submitting
					nCount += pLMObject->nInstanceCount;
					nTri += nObjectTriCount * pLMObject->nInstanceCount;
					nVtx += nObjectVertCount * pLMObject->nInstanceCount;
				}
				delete pKongMesh;
			}

			pLMObject = pLMObject->pNextObject;
		}
#endif // LMG_INCLUDE_STATIC_APE_OBJECTS
		char szTemp[128];
		sprintf( szTemp, "Triangles in static lighting phase: %d (incl. %d object instances)", nTri, nCount );
		pDlg->InfoString( szTemp );

		m_LightingSolution->SetDataSize((u32)(nVtx*1.1f), (u32)(nTri*1.1f));

		for ( i = 0; i < pKongToVis->m_nStatsNumMeshes; i++ ) 
		{
			if ( !((CApeToKongFormat *)pKongToVis->m_apVolKongMeshs[i])->m_pKongMesh )
			{
				continue;
			}

			FVisVolume_t *pVolume = &FVis_pVisData->paVolumes[i];

			AddVolumeKongMesh( i, ((CApeToKongFormat *)pKongToVis->m_apVolKongMeshs[i])->m_pKongMesh, &pVolume->spBoundingWS );
		}
		u32 nTotalVolumes = i;

		m_LightingSolution->SetGeoInstance_Start();

#if LMG_INCLUDE_STATIC_APE_OBJECTS
		nCount = 0;
		pLMObject = pStaticObjects;
		while ( pLMObject )
		{
			pKongMesh = NULL;
			if ( pDlg )
			{
				pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );
			}

			ApeObject_t *pObject = pKongToVis->GetObjectInfo( pLMObject->pFirstInstance->nApeObjectIdx );

			CString csPath = pObject->szName;
			if ( pLMObject->nCompileResult == CDLG_RESULT_COMPILE_SUCCESSFUL )
			{
				pKongMesh = new KongMesh_t;
				csPath = CPasmDlg::m_csPASMTempDirectory + csPath + ".kng";
			}
			else
			{
				pKongMesh = NULL;
			}

			if ( pKongMesh )
			{
				if ( pKongMesh->OpenFileAndLoadKong( csPath ) )
				{
					// Compute the object bounding sphere
					CFSphere spModel;
					pKongMesh->ComputeBoundingSphere( spModel.m_Pos, spModel.m_fRadius, FALSE );

					// Add each instance of this object
					LMG_ObjectInstance_t *pLMInstance = pLMObject->pFirstInstance;
					while ( pLMInstance )
					{
						pObject = pKongToVis->GetObjectInfo( pLMInstance->nApeObjectIdx );
						AddVolumeKongMesh( nTotalVolumes + nCount, pKongMesh, &spModel, &pObject->Orientation, pLMInstance->bAcceptsLightmaps );
						pLMInstance->nLMVolumeIdx = nTotalVolumes + nCount;
						nCount++;

						pLMInstance = pLMInstance->pNextInstance;
					}
				}
				delete pKongMesh;
			}

			pLMObject = pLMObject->pNextObject;
		}
#endif // LMG_INCLUDE_STATIC_APE_OBJECTS

		m_nCurrentVolume = 0;
		for (i=0; i<pKongToVis->GetNumLights(); i++)
		{
			ApeLight_t *pApeLight = pKongToVis->GetLightInfo(i);

			#if !_USE_ALL_LIGHTS_FOR_STATIC_LIGHTING
			if ( ((pApeLight->nFlags & APE_LIGHT_FLAG_LIGHTMAP_LIGHT) && !(pApeLight->nFlags & APE_LIGHT_FLAG_LIGHT_SELF)) )
			#endif
			{
				AddApeLight( pApeLight );
			}
		}

		m_LightingSolution->AssignLightsToTri();
	}
}

void CLightMapGen::InitWithKongData( KongMesh_t *pKongMesh, CCompileDlg *pDlg/*=NULL*/ )
{
	FASSERT( pKongMesh );

	if ( m_LightingSolution )
	{
		_nTriIdxOffs = 0;
		_nTriGroupOffs = 0;
		_nVtxGroupOffs = 0;

		m_LightingSolution->SetGeoInstanceVolume( 1 );
		m_LightingSolution->SetTriGroup( _nTriGroupOffs );
		m_LightingSolution->SetVtxGroup( _nVtxGroupOffs );

		u32 nTri=0, nVtx=0;

		AppendGeometryCount( pKongMesh, nTri, nVtx );

		m_LightingSolution->SetDataSize( (u32)(nVtx*1.1f), (u32)(nTri*1.1f) );

		CFSphere spModel;
		pKongMesh->ComputeBoundingSphere( spModel.m_Pos, spModel.m_fRadius, FALSE );
		AddVolumeKongMesh( 0, pKongMesh, &spModel );
	}

	m_LightingSolution->AssignLightsToTri();
}

CLightingSolution::LN_MaterialExport_t *CLightMapGen::ExportMaterialGeo(u32 nGeo, u32 nMaterial) 
{ 
	u32 nVolume = m_LightingSolution->GetGeoInstanceVolume(nGeo);
	return m_LightingSolution->ExportMaterial(nVolume, nMaterial); 
}

u32 CLightMapGen::GetNumLightMaps_Geo(u32 nGeo) 
{ 
	u32 nVolume = m_LightingSolution->GetGeoInstanceVolume(nGeo);
	return m_LightingSolution->GetNumLightMaps_Volume(nVolume); 
}

char *CLightMapGen::GetLightMap_Geo(u32 nGeo, u32 nLightMap) 
{ 
	u32 nVolume = m_LightingSolution->GetGeoInstanceVolume(nGeo);
	return m_LightingSolution->GetLightMap_Volume(nVolume, nLightMap); 
}

BOOL CLightMapGen::Build( CCompileDlg *pDlg, BOOL bUVsOnly/*=FALSE*/, BOOL bVertexOnly/*=FALSE*/ )
{
	if (m_LightingSolution)
	{
		m_LightingSolution->SetProgressDialog( pDlg );
		if ( bUVsOnly )
		{
			m_LightingSolution->BuildUVsOnly();
			return TRUE;
		}
		else if ( bVertexOnly )
		{
			return m_LightingSolution->Build_VertexOnly(m_fAmbRed, m_fAmbGreen, m_fAmbBlue);
		}
		else
		{
			return m_LightingSolution->Build(m_fAmbRed, m_fAmbGreen, m_fAmbBlue);
		}
	}
	return FALSE;
}
