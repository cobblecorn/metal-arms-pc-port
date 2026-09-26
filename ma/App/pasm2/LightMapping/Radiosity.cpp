#include "StdAfx.h"
#include "Radiosity.h"
#include "..\Pasm.h"
#include "..\CompileDlg.h"
#include "lightmapgen.h"
#include <math.h>

//Include hemisphere normal definitions for skylighting.
#include "Hemisphere.h"


#define _USE_KDOPS		TRUE


CRadiosity *CRadiosity::m_pSelf=NULL;
CLightingSolution::LN_Light_t *CRadiosity::m_pCurLight=NULL;
CRadiosity::LN_AreaLight_t *CRadiosity::m_pCurAreaLight=NULL;
CRadiosity::LN_RASTER_PRECALC_t *CRadiosity::m_pRasterPreCalc=NULL;
CRadiosity::LN_RASTER_PRECALC_t *CRadiosity::m_pRasterPreCalcMask=NULL;
CFVec3 CRadiosity::m_vMulClr;

BOOL _bSkyLight=FALSE;

u8 CRadiosity::m_nCurAreaQuad=0;

CRadiosity::CRadiosity(void):CLightingSolution()
{
	m_fMaxLightEffectMult = 6.0f; //this is specific to lighting solutions
	m_pElementMaps = NULL;
	m_pVtxMap = NULL;
	m_pSelf = this;
	m_fGamma = (1.0f/1.4f);
	//m_fGamma = 2.2f;

	m_nMaxIterations = 1;
	m_fLightInScene = 1.0f;
	m_fThreshold = 0.01f;

	m_fPI = 3.14159265359f;
	m_pRasterPreCalc = NULL;
	m_pRasterPreCalcMask = NULL;

	m_fPointLightMult = 2.85f;//20.0f;

	m_panMotifTargetCount = NULL;
	m_bInterpNrml = FALSE;

	m_bUseLightMapFlag = FALSE;

	m_nAreaLights = 0;
}

CRadiosity::~CRadiosity(void)
{
	if (m_pElementMaps)
	{
		if (m_nNumLightMaps)
		{
			u32 i;
			for (i=0; i<m_nNumLightMaps; i++)
			{
				m_pElementMaps[i].FreeData();
			}

			delete [] m_pElementMaps;
		}
		else
		{
			m_pElementMaps->FreeData();
			delete m_pElementMaps;
		}
		m_pElementMaps = NULL;
	}
	if (m_pVtxMap)
	{
		m_pVtxMap->FreeData();
		delete m_pVtxMap;
		m_pVtxMap = NULL;
	}
	if (m_pRasterPreCalc)
	{
		delete [] m_pRasterPreCalc;
		m_pRasterPreCalc = NULL;
	}
	if (m_pRasterPreCalcMask)
	{
		delete [] m_pRasterPreCalcMask;
		m_pRasterPreCalcMask = NULL;
	}
	CLightingSolution::~CLightingSolution();
}

void CRadiosity::AddLight(u32 nVolume, LS_LightType_e LightType, CFVec3& vDir, CFVec3& vPos, CFVec3& vDAtten, CFVec3& vAAtten, CFVec3& vColor, f32 fColorIntens, f32 fAttenItens, f32 fOOR2, u16 nMotifIndex)
{
	CLightingSolution::AddLight(nVolume, LightType, vDir, vPos, vDAtten, vAAtten, vColor, fColorIntens, m_fPointLightMult*fAttenItens, fOOR2, nMotifIndex);
}

u32 _anIterations[10] =
{
	1,		//
	1,		//
	10,		//
	50,		//
	100,	//
	1000,	// about  1 hour.
	10000,	// about  1 hour.
	100000, // about 10 hours.
	200000, // about 20 hours.
	1000000  // about ? hours, depends on the light energy in the scene.
};

void CRadiosity::CalculateDialogUpdateParams()
{
	u32 i, ii, nLightCount;

	FASSERT( !m_panMotifTargetCount );

//	m_nMaxMotifIndex = 0;

	m_fUpdateInterval = (f32)(m_nNumTri * (m_nMaxMotifIndex + 1)); // Tris processed for GenerateElementMaps() call + GenerateLightMaps();
	m_panMotifTargetCount = (u64 *)malloc( sizeof(u64) * (m_nMaxMotifIndex + 1) );
	for ( i = 0; i <= m_nMaxMotifIndex; i++ )
	{
		nLightCount = 0;
		for ( ii = 0; ii < m_nNumLights; ii++ )
		{
			if ( m_LightList[ii].nMotifIndex == i )
			{
				nLightCount++;
			}
		}

		if ( nLightCount || m_nAreaLights )
		{
			u32 j, nAreaLightQuadcount = 0;
			// Calc the number of quads
			for ( j = 0; j < m_nAreaLights; j++ )
			{
				nAreaLightQuadcount += m_AreaLights[j].nQuad;
			}

			m_fUpdateInterval += (nLightCount + nAreaLightQuadcount) * m_nNumTri;				// Tris processed for EmitLight() call
		}
	}

	m_fUpdateInterval += (m_nNumTri * 2 * _anIterations[m_nQuality]);	// Tris processed for DispenseLight() calls
	if ( m_panMotifTargetCount )
	{
		m_panMotifTargetCount[0] = (u32)m_fUpdateInterval;
	}

	m_fUpdateInterval += m_nNumTri * (m_nMaxMotifIndex + 1);								// Tris processed for RenderLightMaps() call

	m_fUpdateInterval = (f32)m_fUpdateInterval / (f32)LMG_NUMBER_OF_PROGRESS_BAR_UPDATES;
	m_fUpdateCount = 0;
}

BOOL CRadiosity::BuildUVsOnly()
{
	PartitionWorld();

	GenerateLightMaps( TRUE );

	return TRUE;
}

BOOL CRadiosity::Build_VertexOnly(f32 fAmbRed, f32 fAmbGreen, f32 fAmbBlue)
{
	if (m_nNumLights < 1)
	{
		return FALSE;
	}

	if (!m_bUseLightMapFlag)
	{
		PartitionWorld_Vertex();
	}

	if ( m_pDlg->m_bAbortCompile )
	{
		return FALSE;
	}

	if ( m_pDlg )
	{
		m_pDlg->ResetSubOp( "Generating Vertex Element Maps...", LMG_NUMBER_OF_PROGRESS_BAR_UPDATES + 1 );
		m_pDlg->PostMessage( CDLG_UPDATE_CONTROLS );
	}

	m_fUpdateInterval = (f32)m_nNumVtx;
	m_fUpdateInterval = (f32)m_fUpdateInterval / (f32)LMG_NUMBER_OF_PROGRESS_BAR_UPDATES;
	m_fUpdateCount = 0;

	GenerateElementMaps_Vertex();

	if ( m_pDlg )
	{
		m_pDlg->ResetSubOp( "Applying Vertex Radiosity...", LMG_NUMBER_OF_PROGRESS_BAR_UPDATES + 1 );
		m_pDlg->PostMessage( CDLG_UPDATE_CONTROLS );
	}

	m_fUpdateInterval = (f32)((m_nNumLights * m_pVtxMap->GetNumElements()) + (m_nMaxIterations * m_pVtxMap->GetNumElements()));
	m_fUpdateInterval = (f32)m_fUpdateInterval / (f32)LMG_NUMBER_OF_PROGRESS_BAR_UPDATES;
	m_fUpdateCount = 0;

	EmitLight_Vertex();

	m_pDlg->InfoString("Dispensing Indirect Light - Vertex Radiosity");

	m_fLightInScene = 1.0f;
	u32 nIter = 0;
	do
	{
		if ( m_pDlg->m_bAbortCompile )
		{
			return FALSE;
		}

		DispenseLight_Vertex(nIter);

		// Check for abort
		if ( m_pDlg->m_bAbortCompile )
		{
			m_pDlg->m_bAbortCompile = FALSE;
			break;
		}

		nIter++;
	} while (nIter < m_nMaxIterations && m_fLightInScene > m_fThreshold);

	RenderVertexColors(fAmbRed, fAmbGreen, fAmbBlue);

	return TRUE;
}

BOOL CRadiosity::Build(f32 fAmbRed, f32 fAmbGreen, f32 fAmbBlue)
{
	BOOL bRetVal = TRUE;
	u32 i;

	WPRINT_BEGIN
	WPRINT("Partition World...\n");
	WPRINT_END

	if (m_nNumLights < 1)
	{
		return FALSE;
	}

	PartitionWorld();

	if ( m_pDlg->m_bAbortCompile )
	{
		return FALSE;
	}

    WPRINT_BEGIN
	WPRINT("GenerateLightMaps...\n");
	WPRINT_END

	GenerateLightMaps();

//	MessageBox(NULL, "finished lightmaps", "DEBUG", MB_OK);

	RasterPreCalc(0);

//	MessageBox(NULL, "finished raster percalc.", "DEBUG", MB_OK);
		
	if ( m_pDlg->m_bAbortCompile )
	{
		return FALSE;
	}

	// Must occur after light maps are generated
	CalculateDialogUpdateParams();

	m_nMaxIterations = _anIterations[m_nQuality];

	WPRINT_BEGIN
	WPRINT("Generate Element Maps...\n");
	WPRINT_END

//	MessageBox(NULL, "Generate Element Maps", "DEBUG", MB_OK);

	GenerateElementMaps();

	//Do radiosity here.

	if ( m_pDlg->m_bAbortCompile )
	{
		return FALSE;
	}

	if ( m_pDlg )
	{
		m_pDlg->ResetSubOp( "Applying Lightmap Radiosity...", LMG_NUMBER_OF_PROGRESS_BAR_UPDATES + 1 );
		m_pDlg->PostMessage( CDLG_UPDATE_CONTROLS );
	}

	u32 nNumLMapMotif=0;
	u32 nLMapMotif[1000];

	//Emit radiance from all the lights in the scene.
	//m_nMaxMotifIndex = 0;
	for (i=0; i<=m_nMaxMotifIndex; i++)
	{
		if (i > 0)
		{
			RasterPreCalc(i);
		}

		WPRINT_BEGIN
		WPRINT("Emit Light...\n");
		WPRINT_END

		if ( m_pDlg->m_bAbortCompile )
		{
			return FALSE;
		}

//		MessageBox(NULL, "EmitLight()", "DEBUG", MB_OK);

		if ( EmitLight(i) )
		{
			nLMapMotif[nNumLMapMotif++] = i;
		}
	}

//	MessageBox(NULL, "Iterations", "DEBUG", MB_OK);

	m_pDlg->InfoString("Dispensing Indirect Light - LightMap Radiosity");

	m_fLightInScene = 1.0f;
	u32 nIter = 0;
	do
	{
		if ( m_pDlg->m_bAbortCompile )
		{
			return FALSE;
		}

		DispenseLight(nIter, 0);

		// Check for abort
		if ( m_pDlg->m_bAbortCompile )
		{
			m_pDlg->m_bAbortCompile = FALSE;
			break;
		}

		nIter++;
	} while (nIter < m_nMaxIterations && m_fLightInScene > m_fThreshold);

	// Make sure the progress bar is at the right place
	if ( m_pDlg && m_panMotifTargetCount )
	{
		m_pDlg->PostMessage( CDLG_SET_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp, (u32)(m_panMotifTargetCount[0] / m_fUpdateInterval) );
	}

//	MessageBox(NULL, "RenderLightMaps", "DEBUG", MB_OK);

	for (i=0; i<nNumLMapMotif; i++)
	{
		RasterPreCalc(nLMapMotif[i]);

		if ( m_pDlg->m_bAbortCompile )
		{
			m_pDlg->m_bAbortCompile = FALSE;
			break;
			//return FALSE;
		}

		RenderLightMaps(nLMapMotif[i]);
	}

	free( m_panMotifTargetCount );
	m_panMotifTargetCount = NULL;

	WPRINT_BEGIN
	WPRINT("OutputLightMaps...\n");
	WPRINT_END

	CLightMapTex::SetMinValue( (u8)(fAmbRed*255.0f), (u8)(fAmbGreen*255.0f), (u8)(fAmbBlue*255.0f) );

	OutputLightMaps();

	//Build vertex radiosity materials
	m_bUseLightMapFlag = TRUE;
	Build_VertexOnly(fAmbRed, fAmbGreen, fAmbBlue);
	//

	return (bRetVal);
}

CFVec3 _vEmissiveClr;
f32 _fEmissiveIntens;

void CRadiosity::TriEmissive(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx)
{
	CElementMap *pElementMap;
	CElementMap::ELEMENT_RGB *pIllum;
	CElementMap::ELEMENT_RGB *pRad;
	
	pElementMap = &m_pSelf->m_pElementMaps[nLMIdx];
	pRad   = pElementMap->GetRadiance(fU, fV);
	pIllum = pElementMap->GetIllumination(fU, fV);

	pRad->fRed += _vEmissiveClr.x * _fEmissiveIntens;
	pRad->fGreen += _vEmissiveClr.y * _fEmissiveIntens;
	pRad->fBlue += _vEmissiveClr.z * _fEmissiveIntens;

	pIllum->fRed += _vEmissiveClr.x * _fEmissiveIntens;
	pIllum->fGreen += _vEmissiveClr.y * _fEmissiveIntens;
	pIllum->fBlue += _vEmissiveClr.z * _fEmissiveIntens;
}

void CRadiosity::MakeTriangleEmissive(u32 nTri, f32 fIntensity, CFVec3& vColor)
{
	_vEmissiveClr = vColor;
	_fEmissiveIntens = fIntensity;
	RasterizeTriangle(0, TriEmissive, &m_TriangleList[nTri], nTri);
}

CFVec3 _vOne(1.0f, 1.0f, 1.0f);
#define EPS 100.0f
f32 _fE = 10.0f;

void CRadiosity::EmitAreaLight(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx)
{
	CLightMapTex *pLightMap;
	CElementMap *pElementMap;
	CElementMap::ELEMENT_RGB *pIllum;
	CElementMap::ELEMENT_RGB *pRad;
	LN_Triangle_t *pCollTri=NULL;
	f32 *pArea;
	CFVec2 vUV;
	u16 *pMask;
	u8 *pEmissive;
	f32 *pRfl;
	CFVec3 V, *pNrml, LEnd;
	f32 fH=1.0f, fA1, fA2, fR, E, F;
	
	pElementMap = &m_pSelf->m_pElementMaps[nLMIdx];
	pLightMap = m_pSelf->m_pLightMapList[nLMIdx];

	pElementMap->SetUV(fU, fV);
	pIllum = pElementMap->GetIllumination();
	pRad   = pElementMap->GetRadiance();
	pRfl   = pElementMap->GetReflectance();
	pArea   = pElementMap->GetArea();
	pMask  = pElementMap->GetMask();
	pEmissive  = pElementMap->GetEmissive();
	pNrml = pElementMap->GetNrml();

	m_pSelf->m_pOrigTri = pTri;

	if ( *pMask == 0 && !(*pEmissive) )
	{
		V = m_pSelf->m_pCurAreaLight->anQuads[ m_nCurAreaQuad ].vCen - vPos;
		if (1)//pNrml->Dot(m_pSelf->m_pCurAreaLight->vNrml) > 0)
		{
			m_vMulClr = _vOne;
			fR = V.ExtractUnitAndMag(V);
			fA1 = V.Dot(m_pCurAreaLight->vNrml); 
			if (fA1 < 0.0f) { fA1 = 0.0f; }
			fA2 = -V.Dot(*pNrml);
			if (fA2 < 0.0f) { fA2 = 0.0f; }
		
			if (fA1 > 0 && fA2 > 0)
			{
				if (m_pSelf->m_nQuality > 0)
				{
					fH = m_pSelf->RayTrace_AreaLight(m_pSelf->m_pCurAreaLight, vPos);
				}
				F = (fA1 * fA2 * fH * (*pArea)) / (m_pSelf->m_fPI * fR * fR);
				F = (F > 0.0f)?(F):(0.0f);

				E = m_pSelf->m_pCurAreaLight->fIntensity * (*pRfl) * F * (m_pSelf->m_pCurAreaLight->fArea / *pArea);
			}
			else
			{
				F = 0;
				E = 0;
			}

			pIllum->fRed += E * m_pSelf->m_pCurAreaLight->fRed * m_vMulClr.x;
			pIllum->fGreen += E * m_pSelf->m_pCurAreaLight->fGreen * m_vMulClr.y;
			pIllum->fBlue += E * m_pSelf->m_pCurAreaLight->fBlue * m_vMulClr.z;
			
			pRad->fRed += E* m_pSelf->m_pCurAreaLight->fRed * m_vMulClr.x;
			pRad->fGreen += E* m_pSelf->m_pCurAreaLight->fGreen * m_vMulClr.y;
			pRad->fBlue += E* m_pSelf->m_pCurAreaLight->fBlue * m_vMulClr.z;
		}

		*pMask = 1;
	}
}

void CRadiosity::AveSubSamples(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx)
{
	CLightMapTex *pLightMap;
	CElementMap *pElementMap;
	CElementMap::ELEMENT_RGB *pIllum;
	CElementMap::ELEMENT_RGB *pRad;
	CElementMap::ELEMENT_RGB *pBuffer;
	LN_Triangle_t *pCollTri=NULL;
	CFVec2 vUV;
	u16 *pMask;
	u8 *pEmissive;
	f32 *pRfl;
	CFVec3 V, *pNrml, LEnd;
	f32 fOOMask;
	
	pElementMap = &m_pSelf->m_pElementMaps[nLMIdx];
	pLightMap = m_pSelf->m_pLightMapList[nLMIdx];

	pElementMap->SetUV(fU, fV);
	pIllum = pElementMap->GetIllumination();
	pRad   = pElementMap->GetRadiance();
	pBuffer   = pElementMap->GetBuffer();
	pRfl   = pElementMap->GetReflectance();
	pMask  = pElementMap->GetMask();
	pEmissive  = pElementMap->GetEmissive();
	pNrml = pElementMap->GetNrml();

	if ( *pMask > 0 )
	{
		fOOMask = 1.0f / (f32)(*pMask);
		pBuffer->fRed *= fOOMask;
		pBuffer->fGreen *= fOOMask;
		pBuffer->fBlue *= fOOMask;

		pIllum->fRed += pBuffer->fRed;
		pIllum->fGreen += pBuffer->fGreen;
		pIllum->fBlue += pBuffer->fBlue;
		
		pRad->fRed += pBuffer->fRed;
		pRad->fGreen += pBuffer->fGreen;
		pRad->fBlue += pBuffer->fBlue;
		*pMask = 0;
	}
}

CFVec3 _vTmpNrml;

void CRadiosity::EmitAreaLight_SS(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx)
{
	CLightMapTex *pLightMap;
	CElementMap *pElementMap;
	//CElementMap::ELEMENT_RGB *pIllum;
	//CElementMap::ELEMENT_RGB *pRad;
	CElementMap::ELEMENT_RGB *pBuffer;
	LN_Triangle_t *pCollTri=NULL;
	f32 *pArea;
	CFVec2 vUV;
	u16 *pMask;
	u8 *pEmissive;
	f32 *pRfl;
	CFVec3 V, *pNrml, LEnd;
	f32 fH=1.0f, fA1, fA2, fR, E, F;
	
	pElementMap = &m_pSelf->m_pElementMaps[nLMIdx];
	pLightMap = m_pSelf->m_pLightMapList[nLMIdx];

	pElementMap->SetUV(fU, fV);
	//pIllum = pElementMap->GetIllumination();
	//pRad   = pElementMap->GetRadiance();
	pBuffer   = pElementMap->GetBuffer();
	pRfl   = pElementMap->GetReflectance();
	pArea   = pElementMap->GetArea();
	pMask  = pElementMap->GetMask();
	pEmissive  = pElementMap->GetEmissive();
	pNrml = pElementMap->GetNrml();

	m_pSelf->m_pOrigTri = pTri;

	if ( /**pMask == 0 && */!(*pEmissive) )
	{
		V = m_pSelf->m_pCurAreaLight->anQuads[ m_nCurAreaQuad ].vCen - vPos;
		if (1)//pNrml->Dot(m_pSelf->m_pCurAreaLight->vNrml) > 0)
		{
			m_vMulClr = _vOne;
			fR = V.ExtractUnitAndMag(V);
			fA1 = V.Dot(m_pCurAreaLight->vNrml); 
			if (fA1 < 0.0f) { fA1 = 0.0f; }
			fA2 = -V.Dot(*pNrml);
			if (fA2 < 0.0f) { fA2 = 0.0f; }
		
			if (fA1 > 0 && fA2 > 0)
			{
				if (m_pSelf->m_nQuality > 0)
				{
					fH = m_pSelf->RayTrace_AreaLight(m_pSelf->m_pCurAreaLight, vPos);
				}
				F = (fA1 * fA2 * fH * (*pArea)) / (m_pSelf->m_fPI * fR * fR);
				F = (F > 0.0f)?(F):(0.0f);

				E = m_pSelf->m_pCurAreaLight->fIntensity * (*pRfl) * F * (m_pSelf->m_pCurAreaLight->fArea / *pArea);
			}
			else
			{
				F = 0;
				E = 0;
			}

			pBuffer->fRed += E * m_pCurLight->vColor.x * m_vMulClr.x;
			pBuffer->fGreen += E * m_pCurLight->vColor.y * m_vMulClr.y;
			pBuffer->fBlue += E * m_pCurLight->vColor.z * m_vMulClr.z;				

			//pIllum->fRed += E * m_pSelf->m_pCurAreaLight->fRed * m_vMulClr.x;
			//pIllum->fGreen += E * m_pSelf->m_pCurAreaLight->fGreen * m_vMulClr.y;
			//pIllum->fBlue += E * m_pSelf->m_pCurAreaLight->fBlue * m_vMulClr.z;
			
			//pRad->fRed += E* m_pSelf->m_pCurAreaLight->fRed * m_vMulClr.x;
			//pRad->fGreen += E* m_pSelf->m_pCurAreaLight->fGreen * m_vMulClr.y;
			//pRad->fBlue += E* m_pSelf->m_pCurAreaLight->fBlue * m_vMulClr.z;
		}

		//*pMask = 1;
		(*pMask)++;
	}
}

void CRadiosity::EmitPointLight_SS(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx)
{
	CLightMapTex *pLightMap;
	CElementMap *pElementMap;
	//CElementMap::ELEMENT_RGB *pIllum;
	//CElementMap::ELEMENT_RGB *pRad;
	CElementMap::ELEMENT_RGB *pBuffer;
	LN_Triangle_t *pCollTri=NULL;
	CFVec2 vUV;
	u16 *pMask;
	u8 *pEmissive;
	f32 *pRfl;
	CFVec3 V, *pNrml, LEnd;
	f32 fH=1.0f, fA1, fAS, fR, E, F, S;
	
	pElementMap = &m_pSelf->m_pElementMaps[nLMIdx];
	pLightMap = m_pSelf->m_pLightMapList[nLMIdx];

	pElementMap->SetUV(fU, fV);
	//pIllum = pElementMap->GetIllumination();
	//pRad   = pElementMap->GetRadiance();
	pBuffer   = pElementMap->GetBuffer();
	pRfl   = pElementMap->GetReflectance();
	pMask  = pElementMap->GetMask();
	pEmissive  = pElementMap->GetEmissive();
	//pNrml = pElementMap->GetNrml();
	pNrml = &_vTmpNrml;//pElementMap->GetNrml();

	m_pSelf->m_pOrigTri = pTri;

	if ( /**pMask == 0 && */!(*pEmissive) )
	{
		V = m_pCurLight->vPos - vPos;
		if (m_pCurLight->LightType != LS_TYPE_NOATTEN)
		{
			_fE = m_pCurLight->fIntensity*2.0f;//0.5f;
		}
		
		if (fabsf(V.x) < _fE && fabsf(V.y) < _fE && fabsf(V.z) < _fE || m_pCurLight->LightType == LS_TYPE_NOATTEN)
		{
			m_vMulClr = _vOne;
			if (m_pSelf->m_nQuality > 0)
			{
				if (m_pCurLight->LightType != LS_TYPE_NOATTEN)
				{
					fH = m_pSelf->RayTrace(vPos, m_pCurLight->vPos, &vUV, &pCollTri, NULL, m_pCurLight);
				}
				else
				{
					LEnd = m_pCurLight->vDir;
					LEnd = vPos - LEnd;

					fH = m_pSelf->RayTrace(vPos, LEnd, &vUV, &pCollTri, NULL, m_pCurLight);
				}
			}
			if (fH)
			{
				//fR = V.ExtractUnitAndMag(V);
				fR = sqrtf(V.x*V.x + V.y*V.y + V.z*V.z);
				if (fR)
				{
					V.x /= fR;
					V.y /= fR;
					V.z /= fR;
				}
                                				
				if (m_pCurLight->LightType != LS_TYPE_NOATTEN)
				{
					fA1 = -V.Dot(*pNrml);
					if (fR)
					{
						F = (fA1 * fH) / (m_pSelf->m_fPI * fR * fR);
					}
					else
					{
						F = 1.0f;
					}
					if (m_pCurLight->LightType == LS_LTYPE_SPOT)
					{
						fAS = -V.Dot(m_pCurLight->vDir);
						if (fAS <= 0.0f)
						{
							S = 0.0f;
						}
						else
						{
							//S = (fAS * fAS)*m_pCurLight->vAAtten.z + (fAS)*m_pCurLight->vAAtten.y + m_pCurLight->vAAtten.x;
							S = (fAS - m_pCurLight->vAAtten.y)*m_pCurLight->vAAtten.z;
							S = ( S > 0.0f )?( (S<1.0f)?(S):(1.0f) ):( 0.0f );
						}
						F *= S;
					}
				}
				else
				{
					fA1 = m_pCurLight->vDir.Dot(*pNrml);
					F = (fA1 * fH);
				}

				F = (F > 0.0f)?(F):(0.0f);
				E = m_pCurLight->fIntensity * (*pRfl) * F;
			}
			else
			{
				E = 0.0f;
			}

			pBuffer->fRed += E * m_pCurLight->vColor.x * m_vMulClr.x;
			pBuffer->fGreen += E * m_pCurLight->vColor.y * m_vMulClr.y;
			pBuffer->fBlue += E * m_pCurLight->vColor.z * m_vMulClr.z;
			//pIllum->fRed += E * m_pCurLight->vColor.x * m_vMulClr.x;
			//pIllum->fGreen += E * m_pCurLight->vColor.y * m_vMulClr.y;
			//pIllum->fBlue += E * m_pCurLight->vColor.z * m_vMulClr.z;
			
			//pRad->fRed += E* m_pCurLight->vColor.x * m_vMulClr.x;
			//pRad->fGreen += E* m_pCurLight->vColor.y * m_vMulClr.y;
			//pRad->fBlue += E* m_pCurLight->vColor.z * m_vMulClr.z;
		}

		//*pMask = 1;
		(*pMask)++;
	}
}

void CRadiosity::EmitPointLight(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx)
{
	CLightMapTex *pLightMap;
	CElementMap *pElementMap;
	CElementMap::ELEMENT_RGB *pIllum;
	CElementMap::ELEMENT_RGB *pRad;
	//CElementMap::ELEMENT_RGB *pBuffer;
	LN_Triangle_t *pCollTri=NULL;
	CFVec2 vUV;
	u16 *pMask;
	u8 *pEmissive;
	f32 *pRfl;
	CFVec3 V, *pNrml, LEnd;
	f32 fH=1.0f, fA1, fAS, fR, E, F, S;
	
	pElementMap = &m_pSelf->m_pElementMaps[nLMIdx];
	pLightMap = m_pSelf->m_pLightMapList[nLMIdx];

	pElementMap->SetUV(fU, fV);
	pIllum = pElementMap->GetIllumination();
	pRad   = pElementMap->GetRadiance();
	//pBuffer   = pElementMap->GetBuffer();
	pRfl   = pElementMap->GetReflectance();
	pMask  = pElementMap->GetMask();
	pEmissive  = pElementMap->GetEmissive();
	pNrml = pElementMap->GetNrml();
	//pNrml = &_vTmpNrml;//pElementMap->GetNrml();

	m_pSelf->m_pOrigTri = pTri;

	if ( *pMask == 0 && !(*pEmissive) )
	{
		V = m_pCurLight->vPos - vPos;
		if (m_pCurLight->LightType != LS_TYPE_NOATTEN)
		{
			_fE = m_pCurLight->fIntensity*2.0f;//0.5f;
		}
		
		if (fabsf(V.x) < _fE && fabsf(V.y) < _fE && fabsf(V.z) < _fE || m_pCurLight->LightType == LS_TYPE_NOATTEN)
		{
			m_vMulClr = _vOne;
			if (m_pSelf->m_nQuality > 0)
			{
				if (m_pCurLight->LightType != LS_TYPE_NOATTEN)
				{
					fH = m_pSelf->RayTrace(vPos, m_pCurLight->vPos, &vUV, &pCollTri, NULL, m_pCurLight);
				}
				else
				{
					LEnd = m_pCurLight->vDir;
					LEnd = vPos - LEnd;

					fH = m_pSelf->RayTrace(vPos, LEnd, &vUV, &pCollTri, NULL, m_pCurLight);
				}
			}
			if (fH)
			{
				//fR = V.ExtractUnitAndMag(V);
				fR = sqrtf(V.x*V.x + V.y*V.y + V.z*V.z);
				if (fR)
				{
					V.x /= fR;
					V.y /= fR;
					V.z /= fR;
				}
                                				
				if (m_pCurLight->LightType != LS_TYPE_NOATTEN)
				{
					fA1 = -V.Dot(*pNrml);
					if (fR)
					{
						F = (fA1 * fH) / (m_pSelf->m_fPI * fR * fR);
					}
					else
					{
						F = 1.0f;
					}
					if (m_pCurLight->LightType == LS_LTYPE_SPOT)
					{
						fAS = -V.Dot(m_pCurLight->vDir);
						if (fAS <= 0.0f)
						{
							S = 0.0f;
						}
						else
						{
							//S = (fAS * fAS)*m_pCurLight->vAAtten.z + (fAS)*m_pCurLight->vAAtten.y + m_pCurLight->vAAtten.x;
							S = (fAS - m_pCurLight->vAAtten.y)*m_pCurLight->vAAtten.z;
							S = ( S > 0.0f )?( (S<1.0f)?(S):(1.0f) ):( 0.0f );
						}
						F *= S;
					}
				}
				else
				{
					fA1 = m_pCurLight->vDir.Dot(*pNrml);
					F = (fA1 * fH);
				}

				F = (F > 0.0f)?(F):(0.0f);
				E = m_pCurLight->fIntensity * (*pRfl) * F;
			}
			else
			{
				E = 0.0f;
			}

			//pBuffer->fRed += E * m_pCurLight->vColor.x * m_vMulClr.x;
			//pBuffer->fGreen += E * m_pCurLight->vColor.y * m_vMulClr.y;
			//pBuffer->fBlue += E * m_pCurLight->vColor.z * m_vMulClr.z;
			pIllum->fRed += E * m_pCurLight->vColor.x * m_vMulClr.x;
			pIllum->fGreen += E * m_pCurLight->vColor.y * m_vMulClr.y;
			pIllum->fBlue += E * m_pCurLight->vColor.z * m_vMulClr.z;
			
			pRad->fRed += E* m_pCurLight->vColor.x * m_vMulClr.x;
			pRad->fGreen += E* m_pCurLight->vColor.y * m_vMulClr.y;
			pRad->fBlue += E* m_pCurLight->vColor.z * m_vMulClr.z;
		}

		*pMask = 1;
		//(*pMask)++;
	}
}

void CRadiosity::HemisphereLight(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx)
{
	CLightMapTex *pLightMap;
	CElementMap *pElementMap;
	CElementMap::ELEMENT_RGB *pIllum;
	CElementMap::ELEMENT_RGB *pRad;
	LN_Triangle_t *pCollTri=NULL;
	CFVec2 vUV;
	u16 *pMask;
	u8 *pEmissive;
	f32 *pRfl;
	CFVec3 V, *pNrml, LEnd;
	CFColorRGB SkyClr;
	f32 fH=1.0f, fA1, E, F;
	f32 fSampMul;
	
	pElementMap = &m_pSelf->m_pElementMaps[nLMIdx];
	pLightMap = m_pSelf->m_pLightMapList[nLMIdx];

	pElementMap->SetUV(fU, fV);
	pIllum = pElementMap->GetIllumination();
	pRad   = pElementMap->GetRadiance();
	pRfl   = pElementMap->GetReflectance();
	pMask  = pElementMap->GetMask();
	pEmissive  = pElementMap->GetEmissive();
	pNrml = pElementMap->GetNrml();

	m_pSelf->m_pOrigTri = pTri;

	if ( *pMask == 0 && !(*pEmissive) )
	{
		if (m_pSelf->m_nSkyLightingQuality == 1)
		{
			fSampMul = m_pSelf->m_fSkyLightIntens * Hemisphere_afSampleMul[1];
		}
		else
		{
			fSampMul = m_pSelf->m_fSkyLightIntens * Hemisphere_afSampleMul[2];
		}

		u32 nSample;
		for (nSample=0; nSample<Hemisphere_anNumVtx[m_pSelf->m_nSkyLightingQuality]; nSample++)
		{
			if (m_pSelf->m_nSkyLightingQuality == 1)
			{
				V.Set( Hemisphere_afIco1[nSample][0], Hemisphere_afIco1[nSample][2], Hemisphere_afIco1[nSample][1] );
			}
			else
			{
				V.Set( Hemisphere_afIco2[nSample][0], Hemisphere_afIco2[nSample][2], Hemisphere_afIco2[nSample][1] );
			}

			SkyClr = m_pSelf->m_fZenithColor - m_pSelf->m_fHorizColor;
			SkyClr *= CFColorRGB( V.y, V.y, V.y );
			SkyClr += m_pSelf->m_fHorizColor;

			SkyClr.Clamp();

			LEnd = V*10000.0f;
			LEnd += vPos;

			m_vMulClr = _vOne;
			fH = 1.0f;
			if (m_pSelf->m_nQuality > 0)
			{
				_bSkyLight = TRUE;
				fH = m_pSelf->RayTrace(vPos, LEnd, &vUV, &pCollTri, NULL, NULL);
				_bSkyLight = FALSE;
			}

			if (fH)
			{
				fA1 = -V.Dot(*pNrml);
				F = (fA1 * fH);
				
				F = (F > 0.0f)?(F):(0.0f);
				E = fSampMul * (*pRfl) * F;
			}
			else
			{
				E = 0.0f;
			}

			pIllum->fRed += E * SkyClr.fRed * m_vMulClr.x;
			pIllum->fGreen += E * SkyClr.fGreen * m_vMulClr.y;
			pIllum->fBlue += E * SkyClr.fBlue * m_vMulClr.z;
			
			pRad->fRed += E* SkyClr.fRed * m_vMulClr.x;
			pRad->fGreen += E* SkyClr.fGreen * m_vMulClr.y;
			pRad->fBlue += E* SkyClr.fBlue * m_vMulClr.z;
		}

		*pMask = 1;
	}
}

void CRadiosity::ClearElementMasks()
{
	u32 i;
	for (i=0; i<m_nNumLightMaps; i++)
	{
		m_pElementMaps[i].ClearMask();
	}
}

void CRadiosity::ClearElementMasks2()
{
	u32 i;
	for (i=0; i<m_nNumLightMaps; i++)
	{
		m_pElementMaps[i].ClearMask2();
	}
}

void CRadiosity::ClearRadiance()
{
	u32 i;
	for (i=0; i<m_nNumLightMaps; i++)
	{
		m_pElementMaps[i].ClearRadiance();
	}
}

void CRadiosity::ClearBuffer()
{
	u32 i;
	for (i=0; i<m_nNumLightMaps; i++)
	{
		m_pElementMaps[i].ClearBuffer();
	}
}

BOOL CRadiosity::EmitLight(u32 nMotif)
{
	BOOL bEmit=FALSE;
	u32 i;
	//Emit from emissive surfaces.
	//if (nMotif == 0)
	{
		if (nMotif == 0)
		{
			bEmit = TRUE;
			ClearElementMasks();
			//RasterizeMask(EmissiveSurf);
		}

		m_bInterpNrml = TRUE;
		ClearElementMasks();
		Rasterize(0, TriNrml);
		m_bInterpNrml = FALSE;
	}
	//

	//Emit from point lights.
	for (i=0; i<m_nNumLights; i++)
	{
		WPRINT_BEGIN
		WPRINT("Emit Point Light[%d]\n", i);
		WPRINT_END

		m_pCurLight = &m_LightList[i];

		if (m_pCurLight->nMotifIndex == nMotif )//&& m_pCurLight->LightType == LS_LTYPE_SPOT)
		{
			bEmit = TRUE;
			ClearElementMasks();
			ClearBuffer();
			if (m_pLightMapList[0]->GetSubSamples() == 0 || nMotif > 0)
			{
				Rasterize(nMotif, EmitPointLight, FALSE, NULL);
			}
			else
			{
//				char stmp[80];
//				sprintf(stmp, "SubSamples = %d", m_pLightMapList[0]->GetSubSamples());
//				MessageBox(NULL, stmp, "", MB_OK);

				Rasterize_SubSample(0, EmitPointLight_SS, FALSE, NULL, m_pLightMapList[0]->GetSubSamples());
				Rasterize(0, AveSubSamples, FALSE, NULL);
			}
		}
	}

	if (nMotif == 0)
	{
		bEmit = TRUE;
		if (m_nSkyLightingQuality > 0) //what is my light contribution from the sky?
		{
			m_pDlg->InfoString("SkyLighting - LightMap Radiosity");

			ClearElementMasks();
			ClearBuffer();
			Rasterize(0, HemisphereLight, FALSE, NULL);
		}

		for (i=0; i<m_nAreaLights; i++)
		{
			bEmit = TRUE;
			m_pCurAreaLight = &m_AreaLights[i];
			for (m_nCurAreaQuad = 0; m_nCurAreaQuad < m_pCurAreaLight->nQuad; m_nCurAreaQuad++)
			{
				ClearElementMasks();
				Rasterize_SubSample(0, EmitAreaLight_SS, FALSE, NULL, m_pLightMapList[0]->GetSubSamples());
				Rasterize(0, AveSubSamples, FALSE, NULL);
			}
		}
	}

	return (bEmit);
}

void CRadiosity::ApplyRadiosityToElement()
{
	CElementMap::ELEMENT_RGB *pIllum;
	CElementMap::ELEMENT_RGB *pRad;
	LN_Triangle_t *pCollTri=NULL;
	CFVec2 vUV;
	u16 *pMask;
	u8 *pEmissive;
	f32 *pRfl;
	CFVec3 V, *pNrml, *pPos, LEnd;
	f32 fH=1.0f, fA1, fAS, fR, E, F, S;
	
	pIllum = m_pVtxMap->GetIllumination();
	pRad   = m_pVtxMap->GetRadiance();
	pRfl   = m_pVtxMap->GetReflectance();
	pMask  = m_pVtxMap->GetMask();
	pEmissive  = m_pVtxMap->GetEmissive();
	pNrml = m_pVtxMap->GetNrml();
	pPos = m_pVtxMap->GetPos();

	m_pOrigTri = NULL;
	
	//if ( *pMask == 0 && !(*pEmissive) )
	{
		V = m_pCurLight->vPos - *pPos;
		if (m_pCurLight->LightType != LS_TYPE_NOATTEN)
		{
			_fE = m_pCurLight->fIntensity*2.0f;
		}
		
		if ( fabsf(V.x) < _fE && fabsf(V.y) < _fE && fabsf(V.z) < _fE || m_pCurLight->LightType == LS_TYPE_NOATTEN )
		{
			m_vMulClr = _vOne;
			if (m_nQuality > 0)
			{
				if (m_pCurLight->LightType != LS_TYPE_NOATTEN)
				{
					fH = RayTrace(*pPos, m_pCurLight->vPos, &vUV, &pCollTri, NULL, m_pCurLight);
				}
				else
				{
					LEnd = m_pCurLight->vDir;
					LEnd = *pPos - LEnd;

					fH = RayTrace(*pPos, LEnd, &vUV, &pCollTri, NULL, m_pCurLight);
				}
			}
			if (fH)
			{
				//fR = V.ExtractUnitAndMag(V);
				fR = sqrtf(V.x*V.x + V.y*V.y + V.z*V.z);
				if (fR)
				{
					V.x /= fR;
					V.y /= fR;
					V.z /= fR;
				}
                                				
				if (m_pCurLight->LightType != LS_TYPE_NOATTEN)
				{
					fA1 = -V.Dot(*pNrml);
					if (fR)
					{
						F = (fA1 * fH) / (m_fPI * fR * fR);
					}
					else
					{
						F = 1.0f;
					}
					if (m_pCurLight->LightType == LS_LTYPE_SPOT)
					{
						fAS = -V.Dot(m_pCurLight->vDir);
						if (fAS <= 0.0f)
						{
							S = 0.0f;
						}
						else
						{
							S = (fAS - m_pCurLight->vAAtten.y)*m_pCurLight->vAAtten.z;
							S = ( S > 0.0f )?( (S<1.0f)?(S):(1.0f) ):( 0.0f );
						}
						F *= S;
					}
				}
				else
				{
					fA1 = m_pCurLight->vDir.Dot(*pNrml);
					F = (fA1 * fH);
				}

				F = (F > 0.0f)?(F):(0.0f);
				E = m_pCurLight->fIntensity * (*pRfl) * F;
			}
			else
			{
				E = 0.0f;
			}

			pIllum->fRed += E * m_pCurLight->vColor.x * m_vMulClr.x;
			pIllum->fGreen += E * m_pCurLight->vColor.y * m_vMulClr.y;
			pIllum->fBlue += E * m_pCurLight->vColor.z * m_vMulClr.z;
			
			pRad->fRed += E* m_pCurLight->vColor.x * m_vMulClr.x;
			pRad->fGreen += E* m_pCurLight->vColor.y * m_vMulClr.y;
			pRad->fBlue += E* m_pCurLight->vColor.z * m_vMulClr.z;
		}
	}
}

void CRadiosity::HemisphereLight_Element()
{
	CElementMap::ELEMENT_RGB *pIllum;
	CElementMap::ELEMENT_RGB *pRad;
	LN_Triangle_t *pCollTri=NULL;
	CFVec2 vUV;
	u16 *pMask;
	u8 *pEmissive;
	f32 *pRfl;
	CFVec3 V, *pNrml, *pPos, LEnd;
	CFColorRGB SkyClr;
	f32 fH=1.0f, fA1, E, F;
	f32 fSampMul;
	
	pIllum = m_pVtxMap->GetIllumination();
	pRad   = m_pVtxMap->GetRadiance();
	pRfl   = m_pVtxMap->GetReflectance();
	pMask  = m_pVtxMap->GetMask();
	pEmissive  = m_pVtxMap->GetEmissive();
	pNrml = m_pVtxMap->GetNrml();
	pPos = m_pVtxMap->GetPos();

	m_pOrigTri = NULL;
	
	//if ( *pMask == 0 && !(*pEmissive) )
	{
		if (m_pSelf->m_nSkyLightingQuality == 1)
		{
			fSampMul = m_pSelf->m_fSkyLightIntens * Hemisphere_afSampleMul[1];
		}
		else
		{
			fSampMul = m_pSelf->m_fSkyLightIntens * Hemisphere_afSampleMul[2];
		}

		u32 nSample;
		for (nSample=0; nSample<Hemisphere_anNumVtx[m_pSelf->m_nSkyLightingQuality]; nSample++)
		{
			if (m_pSelf->m_nSkyLightingQuality == 1)
			{
				V.Set( Hemisphere_afIco1[nSample][0], Hemisphere_afIco1[nSample][2], Hemisphere_afIco1[nSample][1] );
			}
			else
			{
				V.Set( Hemisphere_afIco2[nSample][0], Hemisphere_afIco2[nSample][2], Hemisphere_afIco2[nSample][1] );
			}

			LEnd = V*10000.0f;
			LEnd += (*pPos);

			SkyClr = m_pSelf->m_fZenithColor - m_pSelf->m_fHorizColor;
			SkyClr *= CFColorRGB( V.y, V.y, V.y );
			SkyClr += m_pSelf->m_fHorizColor;

			SkyClr.Clamp();

			m_vMulClr = _vOne;
			fH = 1.0f;
			if (m_pSelf->m_nQuality > 0)
			{
				_bSkyLight = TRUE;
				fH = m_pSelf->RayTrace(*pPos, LEnd, &vUV, &pCollTri, NULL, NULL);
				_bSkyLight = FALSE;
			}

			if (fH)
			{
				fA1 = -V.Dot(*pNrml);
				F = (fA1 * fH);
				
				F = (F > 0.0f)?(F):(0.0f);
				E = fSampMul * (*pRfl) * F;
			}
			else
			{
				E = 0.0f;
			}

			pIllum->fRed += E * SkyClr.fRed * m_vMulClr.x;
			pIllum->fGreen += E * SkyClr.fGreen * m_vMulClr.y;
			pIllum->fBlue += E * SkyClr.fBlue * m_vMulClr.z;
			
			pRad->fRed += E* SkyClr.fRed * m_vMulClr.x;
			pRad->fGreen += E* SkyClr.fGreen * m_vMulClr.y;
			pRad->fBlue += E* SkyClr.fBlue * m_vMulClr.z;
		}
	}
}

BOOL CRadiosity::EmitLight_Vertex()
{
	BOOL bEmit=FALSE;
	u32 nX, nY;
	u32 i, j;
	//Emit from emissive surfaces.
	bEmit = TRUE;
	ClearElementMasks();
	//

	//Emit from point lights.
	for (i=0; i<m_nNumLights; i++)
	{
		m_pCurLight = &m_LightList[i];

		if ( m_pCurLight->nMotifIndex != 0 )
		{
			if ( m_pDlg )
			{
				m_fUpdateCount += m_pVtxMap->GetNumElements();
				while ( m_fUpdateCount > m_fUpdateInterval )
				{
					m_pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );
					m_fUpdateCount -= m_fUpdateInterval;
				}
			}
			continue;
		}

		bEmit = TRUE;
			
		for (j=0; j<m_pVtxMap->GetNumElements(); j++)
		{
			nX = j%m_pVtxMap->m_nWidth;
			nY = j/m_pVtxMap->m_nHeight;
			m_pVtxMap->SetUV((f32)nX, (f32)nY);

			// Update the progress bar
			if ( m_pDlg )
			{
				m_fUpdateCount += 1.f;
				if ( m_fUpdateCount > m_fUpdateInterval )
				{
					m_pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );
					m_fUpdateCount -= m_fUpdateInterval;
				}
			}

			ApplyRadiosityToElement();
		}
	}

	//SkyLight
	if (m_nSkyLightingQuality > 0) //what is my light contribution from the sky?
	{
		bEmit = TRUE;
		m_pDlg->InfoString("SkyLighting - Vertex Radiosity");
		for (j=0; j<m_pVtxMap->GetNumElements(); j++)
		{
			nX = j%m_pVtxMap->m_nWidth;
			nY = j/m_pVtxMap->m_nHeight;
			m_pVtxMap->SetUV((f32)nX, (f32)nY);

			// Update the progress bar
			if ( m_pDlg )
			{
				m_fUpdateCount += 1.f;
				if ( m_fUpdateCount > m_fUpdateInterval )
				{
					m_pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );
					m_fUpdateCount -= m_fUpdateInterval;
				}
			}

			HemisphereLight_Element();
		}
	}

	/*for (i=0; i<m_nAreaLights; i++)
	{
		bEmit = TRUE;
		m_pCurAreaLight = &m_AreaLights[i];
		for (m_nCurAreaQuad = 0; m_nCurAreaQuad < m_pCurAreaLight->nQuad; m_nCurAreaQuad++)
		{
			ClearElementMasks();
			Rasterize(0, EmitAreaLight, FALSE, NULL);
		}
	}*/
	
	return (bEmit);
}

f32 CRadiosity::Intensity(CElementMap::ELEMENT_RGB *pRad)
{
	return ( pRad->fRed + pRad->fGreen + pRad->fBlue );
}

CFVec3 _vRadPos;
CFVec3 _vRadNrml;
CFVec3 _vRadRadiance;
f32 _vRadArea;

void CRadiosity::TriNrml(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx)
{
	CElementMap *pElementMap;
	CFVec3 *pNrml;
	u16 *pMask;
	
	pElementMap = &m_pSelf->m_pElementMaps[nLMIdx];
	
	pElementMap->SetUV(fU, fV);
	pMask  = pElementMap->GetMask();
	pNrml = pElementMap->GetNrml();

	if ( *pMask == 0 )
	{
		*pNrml = -vDelta;
		f32 fMag2 = pNrml->Dot(*pNrml);
		if (fMag2 > 0.0001f)
		{
			f32 fMag = 1.0f/fmath_AcuSqrt(fMag2);
			pNrml->x *= fMag; pNrml->y *= fMag; pNrml->z *= fMag;
		}
		else
		{
			pNrml->x = pNrml->z = 0.0f;
			pNrml->y = 1.0f;
		}
				
		*pMask = 1;
	}
}

void CRadiosity::EmissiveSurf(f32 fU, f32 fV, f32 fMaskU, f32 fMaskV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx)
{
	CLightMapTex *pEmissive;
	pEmissive = pTri->pMaterial->pEmissive;
	if (!pEmissive) return;

	CElementMap *pElementMap;
	CElementMap::ELEMENT_RGB *pIllum;
	CElementMap::ELEMENT_RGB *pRad;
	u8 *pElementEmissive;
	u16 *pMask;
	f32 fClr[4], fAlpha=1.0f;
	
	pElementMap = &m_pSelf->m_pElementMaps[nLMIdx];
	
	pIllum = pElementMap->GetIllumination(fU, fV);
	pRad   = pElementMap->GetRadiance(fU, fV);
	pMask  = pElementMap->GetMask(fU, fV);
	pElementEmissive  = pElementMap->GetEmissive(fU, fV);

	pEmissive->GetColor(fClr, fMaskU, fMaskV);

	if (pTri->pMaterial->bAlphaMask)
	{
		fAlpha = (fClr[3] >= 0.5f)?(fClr[3]):(0.0f);
	}
	
	if ( *pMask == 0 )
	{
		pIllum->fRed   = 0;
		pIllum->fGreen = 0;
		pIllum->fBlue  = 0;
			
		pRad->fRed   += fClr[0]*10.0f * pTri->pMaterial->fRed * fAlpha;
		pRad->fGreen += fClr[1]*10.0f * pTri->pMaterial->fGreen * fAlpha;
		pRad->fBlue  += fClr[2]*10.0f * pTri->pMaterial->fBlue * fAlpha;

		*pMask = 1;
		
		if (fAlpha >= 0.5f)
		{
			*pElementEmissive = 1;
		}
	}
}

CElementMap::ELEMENT_RGB Average_4(CElementMap::ELEMENT_RGB *e0, CElementMap::ELEMENT_RGB *e1, CElementMap::ELEMENT_RGB *e2, CElementMap::ELEMENT_RGB *e3)
{
	CElementMap::ELEMENT_RGB Ave;
	Ave.fRed = (e0->fRed + e1->fRed + e2->fRed + e3->fRed)*0.25f;
	Ave.fGreen = (e0->fGreen + e1->fGreen + e2->fGreen + e3->fGreen)*0.25f;
	Ave.fBlue = (e0->fBlue + e1->fBlue + e2->fBlue + e3->fBlue)*0.25f;

	return (Ave);
}

void CRadiosity::FindHighestEnergy(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx)
{
	CElementMap *pElementMap = NULL;
	CElementMap::ELEMENT_RGB *pRad;
	f32 *pArea, fIntens;
	u8 *pMask2;
	CFVec3 *pNrml;

	pElementMap = &m_pSelf->m_pElementMaps[nLMIdx];
    
	pElementMap->SetUV(fU, fV);
	pRad  = pElementMap->GetRadiance();
	pArea = pElementMap->GetArea(); 
	pMask2 = pElementMap->GetMask2();
	pNrml = pElementMap->GetNrml();

	if (*pMask2 == 0)
	{
		fIntens = pRad->fRed + pRad->fGreen + pRad->fBlue;
		if (fIntens > m_pSelf->m_BrightestElem->fIntens)
		{
			m_pSelf->m_BrightestElem->fRad = *pRad;
			m_pSelf->m_BrightestElem->fOrigRad = *pRad;

			m_pSelf->m_BrightestElem->fRad.fRed *= pTri->pMaterial->fRed;
			m_pSelf->m_BrightestElem->fRad.fGreen *= pTri->pMaterial->fGreen;
			m_pSelf->m_BrightestElem->fRad.fBlue *= pTri->pMaterial->fBlue;

			m_pSelf->m_BrightestElem->fIntens = fIntens;
			m_pSelf->m_BrightestElem->fU = fU;
			m_pSelf->m_BrightestElem->fV = fV;
			m_pSelf->m_BrightestElem->fArea = *pArea;
			m_pSelf->m_BrightestElem->pElementMap = pElementMap;
			m_pSelf->m_BrightestElem->vPos = vPos;
			m_pSelf->m_BrightestElem->vNrml = *pNrml;
			m_pSelf->m_BrightestElem->pTri = pTri;
		}
		*pMask2 = 1;
	}
}

BOOL _bIndirect=FALSE;

f32 _fMul[]=
{
	1.0f,
	1.0f,
	1.0f,
	50.0f,
	50.0f,
	5.0f,
	1.0f,
	1.0f,
	1.0f,
	1.0f,
	1.0f
};

void CRadiosity::EmitHighestEnergy(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx)
{
	CElementMap *pElementMap = NULL;
	pElementMap = &m_pSelf->m_pElementMaps[nLMIdx];

	CElementMap::ELEMENT_RGB *pIllum;
	CElementMap::ELEMENT_RGB *pRad;
	f32 *pArea;
	f32 *pRfl;
	u16 *pMask;
	u8 *pEmissive;
	CFVec3 V, *pNrml;
	CFVec2 vUV;
	LN_Triangle_t *pCollTri=NULL;
	f32 fH=1.0f, fA1, fA2, fR, F, E;

	pElementMap->SetUV(fU, fV);
	pIllum  = pElementMap->GetIllumination();
	pRad    = pElementMap->GetRadiance();
	pRfl    = pElementMap->GetReflectance();
	pArea   = pElementMap->GetArea();
	pMask   = pElementMap->GetMask();
	pEmissive  = pElementMap->GetEmissive();
	pNrml = pElementMap->GetNrml();

	m_pSelf->m_pOrigTri = m_pSelf->m_BrightestElem->pTri;//pTri;

	if ( *pMask == 0 && !(*pEmissive) )
	{
		if ( (fU != m_pSelf->m_BrightestElem->fU) || (fV != m_pSelf->m_BrightestElem->fV) || (pElementMap != m_pSelf->m_BrightestElem->pElementMap) )
		{
			V = m_pSelf->m_BrightestElem->vPos - vPos;
				
			_fE = 10000.0f;//EPS * (*pArea) * (m_pSelf->m_BrightestElem->fRad.fRed + m_pSelf->m_BrightestElem->fRad.fGreen + m_pSelf->m_BrightestElem->fRad.fBlue);
			if ( ( fabsf(V.x) < _fE && fabsf(V.y) < _fE && fabsf(V.z) < _fE ) && ( V.x!=0||V.y!=0||V.z!=0 ) )
			{
				m_vMulClr = _vOne;
				if (m_pSelf->m_nQuality > 0)
				{
					_bIndirect = TRUE;
					fH = m_pSelf->RayTrace(m_pSelf->m_BrightestElem->vPos, vPos, &vUV, &pCollTri, NULL, NULL);
					_bIndirect = FALSE;
				}
				if (fH)
				{
					fR = V.ExtractUnitAndMag(V);
					fA1 = -V.Dot(m_pSelf->m_BrightestElem->vNrml); 
					fA2 =  V.Dot(*pNrml);

					F = (fA1 * fA2 * fH) * fmath_Inv(m_pSelf->m_fPI * fR * fR);
					F *= _fMul[m_pSelf->m_nQuality];

					F = (F > 0.0f)?(F):(0.0f);
					F = (F < 1.0f)?(F):(1.0f);
					E = (*pRfl) * F;
					
					E = (E > 0)?(E):(0);
				}
				else
				{
					E = 0.0f;
				}

				pIllum->fRed += (E*m_pSelf->m_BrightestElem->fRad.fRed*m_vMulClr.x);
				pIllum->fGreen += (E*m_pSelf->m_BrightestElem->fRad.fGreen*m_vMulClr.y);
				pIllum->fBlue += (E*m_pSelf->m_BrightestElem->fRad.fBlue*m_vMulClr.z);

				pRad->fRed += (E*m_pSelf->m_BrightestElem->fRad.fRed*m_vMulClr.x);
				pRad->fGreen += (E*m_pSelf->m_BrightestElem->fRad.fGreen*m_vMulClr.y);
				pRad->fBlue += (E*m_pSelf->m_BrightestElem->fRad.fBlue*m_vMulClr.z);
			}
		}
		*pMask = 1;
	}
}

void CRadiosity::FindHighestEnergy_Vertex(f32 fU, f32 fV)
{
	CElementMap::ELEMENT_RGB *pRad;
	f32 *pArea, fIntens;
	u8 *pMask2;
	CFVec3 *pNrml, *pPos;

	pRad  = m_pVtxMap->GetRadiance();
	pArea = m_pVtxMap->GetArea(); 
	pMask2 = m_pVtxMap->GetMask2();
	pNrml = m_pVtxMap->GetNrml();
	pPos = m_pVtxMap->GetPos();

	fIntens = pRad->fRed + pRad->fGreen + pRad->fBlue;
	if (fIntens > m_BrightestElem->fIntens)
	{
		m_BrightestElem->fRad = *pRad;
		m_BrightestElem->fOrigRad = *pRad;

		//m_BrightestElem->fRad.fRed *= pTri->pMaterial->fRed;
		//m_BrightestElem->fRad.fGreen *= pTri->pMaterial->fGreen;
		//m_BrightestElem->fRad.fBlue *= pTri->pMaterial->fBlue;

		m_BrightestElem->fIntens = fIntens;
		m_BrightestElem->fU = fU;
		m_BrightestElem->fV = fV;
		m_BrightestElem->fArea = *pArea;
		m_BrightestElem->vPos = *pPos;
		m_BrightestElem->vNrml = *pNrml;
	}
}

void CRadiosity::EmitEnergyToElement()
{
	CElementMap::ELEMENT_RGB *pIllum;
	CElementMap::ELEMENT_RGB *pRad;
	f32 *pArea;
	f32 *pRfl;
	u16 *pMask;
	u8 *pEmissive;
	CFVec3 V, *pNrml, *pPos;
	CFVec2 vUV;
	LN_Triangle_t *pCollTri=NULL;
	f32 fH=1.0f, fA1, fA2, fR, F, E;

	pIllum  = m_pVtxMap->GetIllumination();
	pRad    = m_pVtxMap->GetRadiance();
	pRfl    = m_pVtxMap->GetReflectance();
	pArea   = m_pVtxMap->GetArea();
	pMask   = m_pVtxMap->GetMask();
	pEmissive  = m_pVtxMap->GetEmissive();
	pNrml = m_pVtxMap->GetNrml();
	pPos = m_pVtxMap->GetPos();

	m_pOrigTri = NULL;

	{
		{
			V = m_BrightestElem->vPos - *pPos;
				
			_fE = EPS * (*pArea) * (m_BrightestElem->fRad.fRed + m_BrightestElem->fRad.fGreen + m_BrightestElem->fRad.fBlue);
			if ( (fabsf(V.x) < _fE && fabsf(V.y) < _fE && fabsf(V.z)) && (V.x!=0||V.y!=0||V.z!=0) )
			{
				m_vMulClr = _vOne;
				if (m_nQuality > 0)
				{
					//fH = RayTrace(m_BrightestElem->vPos, *pPos, &vUV, &pCollTri);
					_bIndirect = TRUE;
					fH = RayTrace(m_pSelf->m_BrightestElem->vPos, *pPos, &vUV, &pCollTri);
					_bIndirect = FALSE;
				}
				if (fH)
				{
					fR = V.ExtractUnitAndMag(V);
					fA1 = -V.Dot(m_BrightestElem->vNrml); 
					fA2 = -V.Dot(*pNrml);

					F = (fA1 * fA2 * fH * (*pArea)) * fmath_Inv(m_fPI * fR * fR);
					F = (F > 0.0f)?(F):(0.0f);
					E = (*pRfl) * F * (m_BrightestElem->fArea / *pArea);
				}
				else
				{
					E = 0.0f;
				}

				pIllum->fRed += E*m_BrightestElem->fRad.fRed*m_vMulClr.x;
				pIllum->fGreen += E*m_BrightestElem->fRad.fGreen*m_vMulClr.y;
				pIllum->fBlue += E*m_BrightestElem->fRad.fBlue*m_vMulClr.z;

				pRad->fRed += E*m_BrightestElem->fRad.fRed*m_vMulClr.x;
				pRad->fGreen += E*m_BrightestElem->fRad.fGreen*m_vMulClr.y;
				pRad->fBlue += E*m_BrightestElem->fRad.fBlue*m_vMulClr.z;
			}
		}
	}
}

void CRadiosity::DispenseLight_Vertex(u32 nIter)
{
	u8 nE;
	u32 j, nX, nY;
	CElementMap::ELEMENT_RGB aRad[_NUM_BELEM];
	for (nE = 0; nE < _NUM_BELEM; nE++)
	{
		m_BrightestElem[nE].pElementMap = m_pVtxMap;
		m_BrightestElem[nE].fU = m_BrightestElem[nE].fV = 0.0f;
		m_BrightestElem[nE].fIntens = 0.0f;
		m_BrightestElem[nE].fRad.fRed = 0.0f;
		m_BrightestElem[nE].fRad.fGreen = 0.0f;
		m_BrightestElem[nE].fRad.fBlue = 0.0f;
	}

	for (j=0; j<m_pVtxMap->GetNumElements(); j++)
	{
        nX = j%m_pVtxMap->m_nWidth;
		nY = j/m_pVtxMap->m_nHeight;

		m_pVtxMap->SetUV((f32)nX, (f32)nY);
		FindHighestEnergy_Vertex((f32)nX, (f32)nY);
	}
	
	for (nE = 0; nE < _NUM_BELEM; nE++)
	{
		aRad[nE] = m_BrightestElem[nE].fOrigRad;
	}

	if (nIter == 0)
	{
		m_fMaxLightInScene = Intensity(&m_BrightestElem[0].fOrigRad)*2.0f;
	}

	m_fLightInScene = Intensity(&m_BrightestElem[0].fRad);

	if (m_fLightInScene <= m_fThreshold) return;

	if ( m_fLightInScene > m_fMaxLightInScene )
	{
		m_pDlg->InfoString( "Light in scene has increased! Aborting." );
		m_fLightInScene = 0.0f;
		return;
	}

	for (j=0; j<m_pVtxMap->GetNumElements(); j++)
	{
        nX = j%m_pVtxMap->m_nWidth;
		nY = j/m_pVtxMap->m_nHeight;

		// Update the progress bar
		if ( m_pDlg )
		{
			m_fUpdateCount += 1.f;
			if ( m_fUpdateCount > m_fUpdateInterval )
			{
				m_pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );
				m_fUpdateCount -= m_fUpdateInterval;
			}
		}

		if ( m_pDlg && m_pDlg->m_bAbortCompile )
		{
			return;
		}

		if (m_BrightestElem[nE].fU != (f32)nX && m_BrightestElem[nE].fV != (f32)nY)
		{
			m_pVtxMap->SetUV((f32)nX, (f32)nY);
			EmitEnergyToElement();
		}
	}
	
	for (nE = 0; nE < _NUM_BELEM; nE++)
	{
		CElementMap::ELEMENT_RGB *pRad = m_pVtxMap->GetRadiance(m_BrightestElem[nE].fU, m_BrightestElem[nE].fV);
		pRad->fRed -= aRad[nE].fRed; pRad->fGreen -= aRad[nE].fGreen; pRad->fBlue -= aRad[nE].fBlue;
	}
}

void CRadiosity::DispenseLight(u32 nIter, u32 nMotif)
{
	u8 nE;
	CElementMap::ELEMENT_RGB aRad[_NUM_BELEM];
	for (nE = 0; nE < _NUM_BELEM; nE++)
	{
		m_BrightestElem[nE].pElementMap = &m_pElementMaps[0];
		m_BrightestElem[nE].fU = m_BrightestElem[nE].fV = 0.0f;
		m_BrightestElem[nE].fIntens = 0.0f;
		m_BrightestElem[nE].fRad.fRed = 0.0f;
		m_BrightestElem[nE].fRad.fGreen = 0.0f;
		m_BrightestElem[nE].fRad.fBlue = 0.0f;
	}

	ClearElementMasks2();
	Rasterize(nMotif, FindHighestEnergy, TRUE);

	for (nE = 0; nE < _NUM_BELEM; nE++)
	{
		aRad[nE] = m_BrightestElem[nE].fOrigRad;
	}

	if (nIter == 0)
	{
		m_fMaxLightInScene = Intensity(&m_BrightestElem[0].fOrigRad)*2.0f;
	}

	m_fLightInScene = Intensity(&m_BrightestElem[0].fOrigRad);

	//m_pDlg->InfoString( "Light in scene = %f. [%d]", m_fLightInScene, nIter );

	if ( m_fLightInScene <= m_fThreshold ) { return; }

	if ( m_fLightInScene > m_fMaxLightInScene )
	{
		m_pDlg->InfoString( "Light in scene has increased! Aborting." );
		m_fLightInScene = 0.0f;
		return;
	}

	for (nE = 0; nE < _NUM_BELEM; nE++)
	{
		CElementMap::ELEMENT_RGB *pRad = m_BrightestElem[nE].pElementMap->GetRadiance(m_BrightestElem[nE].fU, m_BrightestElem[nE].fV);

		if (pRad->fRed < 0.0f) { pRad->fRed = 0.0f; }
		if (pRad->fGreen < 0.0f) { pRad->fGreen = 0.0f; }
		if (pRad->fBlue < 0.0f) { pRad->fBlue = 0.0f; }

		if ( pRad->fRed < m_fThreshold && pRad->fGreen < m_fThreshold && pRad->fBlue < m_fThreshold )
		{
			m_pDlg->InfoString( "MaxRadiance = (%f,%f,%f).", pRad->fRed, pRad->fGreen, pRad->fBlue );
			m_fLightInScene = 0.0f;
			return;
		}
	}

	ClearElementMasks();
	Rasterize(nMotif, EmitHighestEnergy);

	for (nE = 0; nE < _NUM_BELEM; nE++)
	{
		CElementMap::ELEMENT_RGB *pRad = m_BrightestElem[nE].pElementMap->GetRadiance(m_BrightestElem[nE].fU, m_BrightestElem[nE].fV);
		pRad->fRed -= aRad[nE].fRed; pRad->fGreen -= aRad[nE].fGreen; pRad->fBlue -= aRad[nE].fBlue;

		if (pRad->fRed < 0.0f) { pRad->fRed = 0.0f; }
		if (pRad->fGreen < 0.0f) { pRad->fGreen = 0.0f; }
		if (pRad->fBlue < 0.0f) { pRad->fBlue = 0.0f; }
	}
}

void CRadiosity::ElementArea(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx)
{
	CElementMap *pElementMap = NULL;
	f32 fArea, *pArea;

	pElementMap = &m_pSelf->m_pElementMaps[nLMIdx];
	
	fArea = vDelta.Mag2();
	if (fArea > 10.0f)
	{
		fArea = 0.05f;
	}
	pArea = pElementMap->GetArea(fU, fV);
	*pArea = fArea;
}

void CRadiosity::ApplyGamma(f32 *pfClr)
{
	//Normalize the color
	f32 fMag = sqrtf(pfClr[0]*pfClr[0] + pfClr[1]*pfClr[1] + pfClr[2]*pfClr[2]);
	if (fMag > 2.0f)
	{
		fMag = 2.0f / fMag;
		pfClr[0] *= fMag; pfClr[1] *= fMag; pfClr[2] *= fMag;
	}
	//

	ClampF3(pfClr);
}

void CRadiosity::RenderLMapTexel(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx)
{
	static CLightMapTex *pLightMap = NULL;
	static CElementMap *pElementMap = NULL;
	static f32 clr[3];
	CElementMap::ELEMENT_RGB *pIllum;

	pElementMap = &m_pSelf->m_pElementMaps[nLMIdx];
	pLightMap = m_pSelf->m_pLightMapList[nLMIdx];

	pIllum = pElementMap->GetIllumination(fU, fV);
	clr[2] = pIllum->fRed; clr[1] = pIllum->fGreen; clr[0] = pIllum->fBlue;
	m_pSelf->ApplyGamma(clr);

	//test//
	
	//CFVec3 *pN = pElementMap->GetNrml(fU, fV);
	//clr[0] = pN->x * 0.5f + 0.5f;
	//clr[1] = pN->y * 0.5f + 0.5f;
	//clr[2] = pN->z * 0.5f + 0.5f;

	//clr[0] = fmodf(fabsf(vPos.x/32.0f), 1.0f);
	//clr[1] = fmodf(fabsf(vPos.y/32.0f), 1.0f);
	//clr[2] = fmodf(fabsf(vPos.z/32.0f), 1.0f);
	
	//

	pLightMap->SetColor(clr, fU, fV, &vPos);
}

void CRadiosity::GenerateElementMaps()
{
	u32 i;
	m_pElementMaps = new CElementMap[m_nNumLightMaps];
	for (i=0; i<m_nNumLightMaps; i++)
	{
		m_pElementMaps[i].AttachLightMapTex(m_pLightMapList[i]);
	}
	for (i=0; i<=m_nMaxMotifIndex; i++)
	{
		Rasterize(i, ElementArea);
	}
}

f32 CRadiosity::GetVertexArea(u32 nVtx)
{
	return 0.1f;
}

void CRadiosity::GenerateElementMaps_Vertex()
{
	u32 i;
	m_pVtxMap = new CElementMap;
	m_pVtxMap->InitData(1024, 1024);
	//now assign each vertex a spot on the map.
	u32 nX, nY;
	CFVec3 vNrml, vPos;
	for (i=0; i<m_nNumVtx; i++)
	{
		// Update the progress bar
		if ( m_pDlg )
		{
			m_fUpdateCount += 1.f;
			if ( m_fUpdateCount > m_fUpdateInterval )
			{
				m_pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );
				m_fUpdateCount -= m_fUpdateInterval;
			}
		}

		vNrml.Set( m_VertexList[i].fNrml[0], m_VertexList[i].fNrml[1], m_VertexList[i].fNrml[2] );
		vPos.Set( m_VertexList[i].fPos[0], m_VertexList[i].fPos[1], m_VertexList[i].fPos[2] );

        m_pVtxMap->AddVertex(vNrml, vPos, nX, nY);

		m_VertexList[i].fOutUV[0] = (f32)nX;
		m_VertexList[i].fOutUV[1] = (f32)nY;
	}
}

#define MAX_LIGHTMAP_SIZE 1024
u32 LIGHTMAP_WIDTH=512, LIGHTMAP_HEIGHT=512;

void CRadiosity::BlendEdges(u32 nMotif)
{
	s32 i, k, nLMap;
	s32 x1, y1, xinc1, yinc1;
	f32 x2, y2, xinc2, yinc2, length;
	s32 nCoords1[3][2];
	LN_Triangle_t *pTri, *pTri2;
	f32 *p[2], fS, fT;
	f32 fCoords2[3][2];
	CFVec3 vDir, vCross;
	CLightMapTex *pLMap, *pLMap2;
	CElementMap *pElementMap, *pElementMap2;
	CElementMap::ELEMENT_RGB *pRGB1, *pRGB2;

	for (i=0; i<(s32)m_nNumTri; i++)
	{
		pTri = &m_TriangleList[i];
		pLMap = pTri->pMaterial->pLightMaps[0];
		nLMap = GetLightMapIdx( pLMap );
		pElementMap = &m_pElementMaps[nLMap];
		if (pLMap)
		{
			for (k=0; k<3; k++)
			{
				p[0] = pTri->pVtx[k]->fPos;
				p[1] = pTri->pVtx[(k+1)%3]->fPos;

				nCoords1[0][0] = (s32)( pTri->pVtx[k]->fBaseUV[0] * pLMap->GetWidth() );
				nCoords1[0][1] = (s32)( pTri->pVtx[k]->fBaseUV[1] * pLMap->GetHeight() );

				nCoords1[1][0] = (s32)( pTri->pVtx[(k+1)%3]->fBaseUV[0] * pLMap->GetWidth() );
				nCoords1[1][1] = (s32)( pTri->pVtx[(k+1)%3]->fBaseUV[1] * pLMap->GetHeight() );
			
				if (nCoords1[0][0] >= pLMap->GetWidth()) { nCoords1[0][0] = pLMap->GetWidth()-1; }
				if (nCoords1[0][1] >= pLMap->GetHeight()) { nCoords1[0][1] = pLMap->GetHeight()-1; }

				if (nCoords1[1][0] >= pLMap->GetWidth()) { nCoords1[1][0] = pLMap->GetWidth()-1; }
				if (nCoords1[1][1] >= pLMap->GetHeight()) { nCoords1[1][1] = pLMap->GetHeight()-1; }

				vDir.Set( p[1][0] - p[0][0], p[1][1] - p[0][1], p[1][2] - p[0][2] );
				
				f32 fMag2 = vDir.Mag2(); 
				f32 fInvMag = 1.0f/fmath_AcuSqrt(fMag2);

				vDir.x *= fInvMag; vDir.y *= fInvMag; vDir.z *= fInvMag;
				vCross = vDir.Cross(pTri->vNrml);

				if (nCoords1[0][0] - nCoords1[1][0] == 0)
				{
					fS = vCross.Dot( (CFVec3)pTri->vLightMapMtx[0]);
					nCoords1[0][0] += fS < 0 ? 1 : -1;
					nCoords1[1][0] += fS < 0 ? 1 : -1;

					if (nCoords1[0][0] < pTri->fLightMapX || nCoords1[0][0] >= pTri->fLightMapX + pTri->fLightMapW)
					{
						nCoords1[0][0] += fS < 0 ? -1 : 1;
						nCoords1[1][0] += fS < 0 ? -1 : 1;
					}
					length = (f32)abs(nCoords1[1][1] - nCoords1[0][1]);
				}
				else if (nCoords1[0][1] - nCoords1[1][1] == 0)
				{
					fT = vCross.Dot( (CFVec3)pTri->vLightMapMtx[1]);
					nCoords1[0][1] += fT < 0 ? 1 : -1;
					nCoords1[1][1] += fT < 0 ? 1 : -1;

					if (nCoords1[0][1] < pTri->fLightMapY || nCoords1[0][1] >= pTri->fLightMapY + pTri->fLightMapH)
					{
						nCoords1[0][1] += fT < 0 ? -1 : 1;
						nCoords1[1][1] += fT < 0 ? -1 : 1;
					}
					length = (f32)abs(nCoords1[1][0] - nCoords1[0][0]);
				}
				else
				{
					continue;
				}

				x1 = nCoords1[0][0];
				if (x1 < 0) x1 = 0;
				y1 = nCoords1[0][1];
				if (y1 < 0) y1 = 0;

				xinc1 = nCoords1[1][0] - nCoords1[0][0];
				if (xinc1 < 0) xinc1 = -1;
				if (xinc1 > 0) xinc1 = 1;
				yinc1 = nCoords1[1][1] - nCoords1[0][1];
				if (yinc1 < 0) yinc1 = -1;
				if (yinc1 > 0) yinc1 = 1;

				if (xinc1 != 0 && yinc1 != 0)
				{
					continue;
				}
				
				//Find Adjacent surface
				pTri2 = NULL;
				u32 point=0;

				if (pTri->pEdgeTri[k])
				{	
					if (pTri->pEdgeTri[k]->vNrml.Dot(pTri->vNrml) < 0.9f)
					{
						pTri2 = pTri->pEdgeTri[k];
						
						if (pTri2->pEdgeTri[0] == pTri)
						{
							point = 0;
						}
						else if (pTri2->pEdgeTri[1] == pTri)
						{
							point = 1;
						}
						else if (pTri2->pEdgeTri[2] == pTri)
						{
							point = 2;
						}
						else
						{
							pTri2 = NULL;
						}
					}
				}

				if (pTri2)
				{
					pLMap2 = pTri2->pMaterial->pLightMaps[0];
					nLMap = GetLightMapIdx( pLMap2 );
					pElementMap2 = &m_pElementMaps[nLMap];

					fCoords2[0][0] = pTri2->pVtx[(point+1)%3]->fBaseUV[0] * pLMap2->GetWidth();
					fCoords2[0][1] = pTri2->pVtx[(point+1)%3]->fBaseUV[1] * pLMap2->GetHeight();

					fCoords2[1][0] = pTri2->pVtx[point]->fBaseUV[0] * pLMap2->GetWidth();
					fCoords2[1][1] = pTri2->pVtx[point]->fBaseUV[1] * pLMap2->GetHeight();
				
					if (fCoords2[0][0] >= pLMap2->GetWidth()) { fCoords2[0][0] = (f32)(pLMap2->GetWidth()-1); }
					if (fCoords2[0][1] >= pLMap2->GetHeight()) { fCoords2[0][1] = (f32)(pLMap2->GetHeight()-1); }

					if (fCoords2[1][0] >= pLMap2->GetWidth()) { fCoords2[1][0] = (f32)(pLMap2->GetWidth()-1); }
					if (fCoords2[1][1] >= pLMap2->GetHeight()) { fCoords2[1][1] = (f32)(pLMap2->GetHeight()-1); }

					x2 = fCoords2[0][0]; y2 = fCoords2[0][1];
					xinc2 = fCoords2[1][0] - fCoords2[0][0];
					if (length) { xinc2 = xinc2 / length; }
					yinc2 = fCoords2[1][1] - fCoords2[0][1];
					if (length) { yinc2 = yinc2 / length; }

					if ( (s32)xinc2 != 0 && (s32)yinc2 != 0 )
					{
						continue;
					}

					while (1)
					{
						pRGB1 = pElementMap->GetIllumination((f32)x1, (f32)y1);
						pRGB2 = pElementMap2->GetIllumination((f32)x2, (f32)y2);

						pRGB1->fRed = pRGB2->fRed * 0.7f + pRGB1->fRed * 0.3f;
						pRGB1->fGreen = pRGB2->fGreen * 0.7f + pRGB1->fGreen * 0.3f;
						pRGB1->fBlue = pRGB2->fBlue * 0.7f + pRGB1->fBlue * 0.3f;

						if (x1 == nCoords1[1][0] && y1 == nCoords1[1][1]) { break; }

						x1 += xinc1; y1 += yinc1;
						x2 += xinc2; y2 += yinc2;

						if (x2 < pTri2->fLightMapX) { x2 = pTri2->fLightMapX; }
						if (x2 >= pTri2->fLightMapX + pTri2->fLightMapW) { x2 = pTri2->fLightMapX + pTri2->fLightMapW - 1; }

						if (y2 < pTri2->fLightMapY) { y2 = pTri2->fLightMapY; }
						if (y2 >= pTri2->fLightMapY + pTri2->fLightMapH) { y2 = pTri2->fLightMapY + pTri2->fLightMapH - 1; }
					};
				}
			}
		}
	}
}

void CRadiosity::FixUpLightValues(u32 nMotif)
{
	s32 i, j, x, y, foundvalue, height, width, index;
	s32 pos, nLMap;
	f32 color[3];
	u8 filled[(MAX_LIGHTMAP_SIZE+1)*(MAX_LIGHTMAP_SIZE+1)/8];
	f32 lightmap_edge_eps;
	CLightMapTex *pLMap;
	CElementMap *pElementMap;
	CElementMap::ELEMENT_RGB *pRGB, *pRGB_Top, *pRGB_Bot;

	LN_Triangle_t *pTri;

	for (i=0; i<(s32)m_nNumTri; i++)
	{
		pTri = &m_TriangleList[i];
		pLMap = pTri->pMaterial->pLightMaps[0];
		nLMap = GetLightMapIdx( pLMap );
		pElementMap = &m_pElementMaps[nLMap];
		if (pLMap)
		{
			lightmap_edge_eps = 0.1f * pLMap->GetSampleSize();
			width = (s32)pTri->fLightMapW;
			height = (s32)pTri->fLightMapH;

			if (pTri->fLightMapX+width > pElementMap->m_nWidth || pTri->fLightMapY+height > pElementMap->m_nHeight) { continue; }

			memset(filled, 0, sizeof(filled));
			for (x=0; x<width; x++)
			{
				for (y=0; y<height; y++)
				{
					pRGB = pElementMap->GetIllumination(x+pTri->fLightMapX, y+pTri->fLightMapY);
					index = x+(u32)pTri->fLightMapX + (y+(u32)pTri->fLightMapY)*pLMap->GetWidth();
					if (pRGB->fRed > 0.0f || pRGB->fGreen > 0.0f || pRGB->fBlue > 0.0f)
					{
						filled[index>>3] |= 1 << (index & 7);
					}
				}
			}
			for (y=0; y<height; y++)
			{
				pos = -2;
				for (x=0; x<width; x++)
				{
					index = x+(s32)pTri->fLightMapX + (y+(s32)pTri->fLightMapY)*pLMap->GetWidth();
					if (pos == -2)
					{
						if (filled[index>>3]&(1<<index&7))
						{
							pos = -1;
						}
					}
					else if (pos == -1)
					{
						if ( !(filled[index>>3]&(1<<(index&7))) )
						{
							pos = x - 1;
						}
					}
					else
					{
						if ( filled[index>>3]&(1<<(index&7)) )
						{
							pRGB_Top = pElementMap->GetIllumination(x+pTri->fLightMapX, y+pTri->fLightMapY);
							pRGB_Bot = pElementMap->GetIllumination(pos+pTri->fLightMapX, y+pTri->fLightMapY);
							for (j=0; j<(x-pos+1)/2; j++)
							{
								index = pos+j+1+(s32)pTri->fLightMapX + (y+(s32)pTri->fLightMapY)*pLMap->GetWidth();
								filled[index>>3] |= 1 << (index&7);

								pRGB = pElementMap->GetIllumination(pos+j+1+pTri->fLightMapX, y+pTri->fLightMapY);
								*pRGB = *pRGB_Top;
								pRGB = pElementMap->GetIllumination(x-j-1+pTri->fLightMapX, y+pTri->fLightMapY);
								*pRGB = *pRGB_Bot;
							}
							pos = -1;
						}
					}
				}
			}
			for (x=0; x<width; x++)
			{
				pos = -2;
				for (y=0; y<height; y++)
				{
					index = x+(s32)pTri->fLightMapX + (y+(s32)pTri->fLightMapY)*pLMap->GetWidth();
					if (pos == -2)
					{
						if (filled[index>>3]&(1<<index&7))
						{
							pos = -1;
						}
					}
					else if (pos == -1)
					{
						if ( !(filled[index>>3]&(1<<(index&7))) )
						{
							pos = x - 1;
						}
					}
					else
					{
						if ( filled[index>>3]&(1<<(index&7)) )
						{
							pRGB_Top = pElementMap->GetIllumination(x+pTri->fLightMapX, y+pTri->fLightMapY);
							pRGB_Bot = pElementMap->GetIllumination(x+pTri->fLightMapX, pos+pTri->fLightMapY);
							for (j=0; j<(y-pos+1)/2; j++)
							{
								index = (pos+j+1+(s32)pTri->fLightMapY)*pLMap->GetWidth() + x+(s32)pTri->fLightMapX;
								filled[index>>3] |= 1 << (index&7);

								pRGB = pElementMap->GetIllumination(x+pTri->fLightMapX, pos+j+1+pTri->fLightMapY);
								*pRGB = *pRGB_Top;
								pRGB = pElementMap->GetIllumination(x+pTri->fLightMapX, y-j-1+pTri->fLightMapY);
								*pRGB = *pRGB_Bot;
							}
							pos = -1;
						}
					}
				}
			}
			for (y=0; y<height; y++)
			{
				foundvalue = 0;
				for (x=0; x<width; x++)
				{
					index = ((s32)pTri->fLightMapY+y)*pLMap->GetWidth() + (s32)pTri->fLightMapX+x;
					pRGB = pElementMap->GetIllumination(x+pTri->fLightMapX, y+pTri->fLightMapY);
					if (foundvalue)
					{
						if ( filled[index>>3]&(1<<(index&7)) )
						{
							color[0] = pRGB->fRed;
							color[1] = pRGB->fGreen;
							color[2] = pRGB->fBlue;
						}
						else
						{	
							pRGB->fRed = color[0];
							pRGB->fGreen = color[1];
							pRGB->fBlue = color[2];

							filled[index>>3]|=(1<<(index&7));
						}
					}
					else
					{
						if ( filled[index>>3]&(1<<(index&7)) )
						{
							color[0] = pRGB->fRed;
							color[1] = pRGB->fGreen;
							color[2] = pRGB->fBlue;

							foundvalue = 1;
						}
					}
				}
				foundvalue = 0;
				for (x=width-1; x>=0; x--)
				{
					index = ((s32)pTri->fLightMapY+y)*pLMap->GetWidth() + (s32)pTri->fLightMapX+x;
					pRGB = pElementMap->GetIllumination(x+pTri->fLightMapX, y+pTri->fLightMapY);

					if (foundvalue)
					{
						if ( filled[index>>3]&(1<<(index&7)) )
						{
							color[0] = pRGB->fRed;
							color[1] = pRGB->fGreen;
							color[2] = pRGB->fBlue;
						}
						else
						{	
							pRGB->fRed = color[0];
							pRGB->fGreen = color[1];
							pRGB->fBlue = color[2];

							filled[index>>3]|=(1<<(index&7));
						}
					}
					else
					{
						if ( filled[index>>3]&(1<<(index&7)) )
						{
							color[0] = pRGB->fRed;
							color[1] = pRGB->fGreen;
							color[2] = pRGB->fBlue;

							foundvalue = 1;
						}
					}
				}
			}
			for (x=0; x<width; x++)
			{
				foundvalue = 0;
				for (y=0; y<height; y++)
				{
					index = ((s32)pTri->fLightMapY+y)*pLMap->GetWidth() + (s32)pTri->fLightMapX+x;
					pRGB = pElementMap->GetIllumination(x+pTri->fLightMapX, y+pTri->fLightMapY);

					if (foundvalue)
					{
						if ( filled[index>>3]&(1<<(index&7)) )
						{
							color[0] = pRGB->fRed;
							color[1] = pRGB->fGreen;
							color[2] = pRGB->fBlue;
						}
						else
						{	
							pRGB->fRed = color[0];
							pRGB->fGreen = color[1];
							pRGB->fBlue = color[2];

							filled[index>>3]|=(1<<(index&7));
						}
					}
					else
					{
						if ( filled[index>>3]&(1<<(index&7)) )
						{
							color[0] = pRGB->fRed;
							color[1] = pRGB->fGreen;
							color[2] = pRGB->fBlue;

							foundvalue = 1;
						}
					}
				}
				for (y=height-1; y>=0; y--)
				{
					index = ((s32)pTri->fLightMapY+y)*pLMap->GetWidth() + (s32)pTri->fLightMapX+x;
					pRGB = pElementMap->GetIllumination(x+pTri->fLightMapX, y+pTri->fLightMapY);

					if (foundvalue)
					{
						if ( filled[index>>3]&(1<<(index&7)) )
						{
							color[0] = pRGB->fRed;
							color[1] = pRGB->fGreen;
							color[2] = pRGB->fBlue;
						}
						else
						{	
							pRGB->fRed = color[0];
							pRGB->fGreen = color[1];
							pRGB->fBlue = color[2];

							filled[index>>3]|=(1<<(index&7));
						}
					}
					else
					{
						if ( filled[index>>3]&(1<<(index&7)) )
						{
							color[0] = pRGB->fRed;
							color[1] = pRGB->fGreen;
							color[2] = pRGB->fBlue;

							foundvalue = 1;
						}
					}
				}
			}
		}
	}
	//BlendEdges(nMotif);
}

void CRadiosity::FindMatchingVtx(LN_Triangle_t *pTri, LN_Vertex_t *pVtx0, LN_Vertex_t *pVtx1, u32& nVtx0, u32& nVtx1)
{
	u32 k;
	nVtx0 = 0xffffffff; nVtx1 = 0xffffffff;
	for (k=0; k<3; k++)
	{
		if ( fabsf(pTri->pVtx[k]->fPos[0]-pVtx0->fPos[0])<0.0001f && fabsf(pTri->pVtx[k]->fPos[1]-pVtx0->fPos[1])<0.0001f && fabsf(pTri->pVtx[k]->fPos[2]-pVtx0->fPos[2])<0.0001f )
		{
			nVtx0 = k;
			break;
		}
	}

	for (k=0; k<3; k++)
	{
		if ( fabsf(pTri->pVtx[k]->fPos[0]-pVtx1->fPos[0])<0.0001f && fabsf(pTri->pVtx[k]->fPos[1]-pVtx1->fPos[1])<0.0001f && fabsf(pTri->pVtx[k]->fPos[2]-pVtx1->fPos[2])<0.0001f )
		{
			nVtx1 = k;
			break;
		}
	}
}

void CRadiosity::RenderVertexColors(f32 fMinRed, f32 fMinGreen, f32 fMinBlue)
{
	u32 i;
	f32 fR, fG, fB;
	for (i=0; i<m_nNumVtx; i++)
	{
		m_pVtxMap->SetUV( m_VertexList[i].fOutUV[0], m_VertexList[i].fOutUV[1] );

		//get color from element map.

		fR = m_VertexList[i].fClr[0];
		fG = m_VertexList[i].fClr[1];
		fB = m_VertexList[i].fClr[2];

		m_VertexList[i].fClr[0] = m_pVtxMap->GetIllumination()->fRed;
		m_VertexList[i].fClr[1] = m_pVtxMap->GetIllumination()->fGreen;
		m_VertexList[i].fClr[2] = m_pVtxMap->GetIllumination()->fBlue;

		//apply color correction
		ApplyGamma(m_VertexList[i].fClr);

        //make sure its at or above the minimum color value.
		if (m_VertexList[i].fClr[0] < fMinRed)
		{
			m_VertexList[i].fClr[0] = fMinRed;
		}

		if (m_VertexList[i].fClr[1] < fMinGreen)
		{
			m_VertexList[i].fClr[1] = fMinGreen;
		}

		if (m_VertexList[i].fClr[2] < fMinBlue)
		{
			m_VertexList[i].fClr[2] = fMinBlue;
		}
		
		m_VertexList[i].fClr[0] += fR;
		m_VertexList[i].fClr[1] += fG;
		m_VertexList[i].fClr[2] += fB;

		ClampF3(m_VertexList[i].fClr);
	}
}

void CRadiosity::RenderLightMaps(u32 nMotif)
{
	u32 i, j, k;

	if (nMotif == 0)
	{
		for (i=0; i<m_nNumLightMaps; i++)
		{
			if (m_pLightMapList[i])
			{
				m_pLightMapList[i]->Clear();
			}
		}
	}

	FixUpLightValues(nMotif);

	if (nMotif == 0)
	{
		for (i=0; i<m_nNumVolumes; i++)
		{
			//if ( !m_VolumeList[i].bHasLM ) { continue; }

			u32 nStart, nEnd, nEdge, nVtx0, nVtx1;
			CFVec2 vEdge0[2], vEdge1[2];
			LN_Triangle_t *pTri;
			CLightMapTex *pLMap;
			nStart = m_VolumeList[i].nStartIdx;
			nEnd = m_VolumeList[i].nEndIdx;

			if (nEnd == 0 && (nStart > 0 || i == m_nNumVolumes-1))
			{
				nEnd = m_nNumTri-1;
			}
			if (nEnd > m_nNumTri-1) nEnd = 0;

			nEdge = 0;
			for (j=nStart; j<=nEnd; j++)
			{
				CFVec3 vDelta;
				f32 fS, fT;
				pTri = &m_TriangleList[j];
				pLMap = m_pLightMapList[i];

				if (pTri->pMaterial->pLightMaps[0] == pLMap)
				{
					for (k=0; k<3; k++)
					{
						if (pTri->pEdgeTri[k])
						{
							if (pTri->pEdgeTri[k]->pMaterial->pLightMaps[0] != pLMap) { continue; }
							if (pTri->pEdgeTri[k]->vNrml.Dot(pTri->vNrml) < 0.0f) { continue; }

							vDelta.x = pTri->pVtx[k]->fPos[0] - pTri->vMin.x;
							vDelta.y = pTri->pVtx[k]->fPos[1] - pTri->vMin.y;
							vDelta.z = pTri->pVtx[k]->fPos[2] - pTri->vMin.z;

							fS = vDelta.Dot( pTri->vLightMapMtx[0] ) + pTri->fLightMapX + 0.5f;
							fT = vDelta.Dot( pTri->vLightMapMtx[1] ) + pTri->fLightMapY + 0.5f;
							vEdge0[0].x = fS; vEdge0[0].y = fT;

							vDelta.x = pTri->pVtx[(k+1)%3]->fPos[0] - pTri->vMin.x;
							vDelta.y = pTri->pVtx[(k+1)%3]->fPos[1] - pTri->vMin.y;
							vDelta.z = pTri->pVtx[(k+1)%3]->fPos[2] - pTri->vMin.z;

							fS = vDelta.Dot( pTri->vLightMapMtx[0] ) + pTri->fLightMapX + 0.5f;
							fT = vDelta.Dot( pTri->vLightMapMtx[1] ) + pTri->fLightMapY + 0.5f;
							vEdge0[1].x = fS; vEdge0[1].y = fT;
		
							FindMatchingVtx(pTri->pEdgeTri[k], pTri->pVtx[k], pTri->pVtx[(k+1)%3], nVtx0, nVtx1);

							if (nVtx0 == 0xffffffff || nVtx1 == 0xffffffff) { continue; }

							vDelta.x = pTri->pEdgeTri[k]->pVtx[nVtx0]->fPos[0] - pTri->pEdgeTri[k]->vMin.x;
							vDelta.y = pTri->pEdgeTri[k]->pVtx[nVtx0]->fPos[1] - pTri->pEdgeTri[k]->vMin.y;
							vDelta.z = pTri->pEdgeTri[k]->pVtx[nVtx0]->fPos[2] - pTri->pEdgeTri[k]->vMin.z;

							fS = vDelta.Dot( pTri->pEdgeTri[k]->vLightMapMtx[0] ) + pTri->pEdgeTri[k]->fLightMapX + 0.5f;
							fT = vDelta.Dot( pTri->pEdgeTri[k]->vLightMapMtx[1] ) + pTri->pEdgeTri[k]->fLightMapY + 0.5f;
							vEdge1[0].x = fS; vEdge1[0].y = fT;

							vDelta.x = pTri->pEdgeTri[k]->pVtx[nVtx1]->fPos[0] - pTri->pEdgeTri[k]->vMin.x;
							vDelta.y = pTri->pEdgeTri[k]->pVtx[nVtx1]->fPos[1] - pTri->pEdgeTri[k]->vMin.y;
							vDelta.z = pTri->pEdgeTri[k]->pVtx[nVtx1]->fPos[2] - pTri->pEdgeTri[k]->vMin.z;

							fS = vDelta.Dot( pTri->pEdgeTri[k]->vLightMapMtx[0] ) + pTri->pEdgeTri[k]->fLightMapX + 0.5f;
							fT = vDelta.Dot( pTri->pEdgeTri[k]->vLightMapMtx[1] ) + pTri->pEdgeTri[k]->fLightMapY + 0.5f;
							vEdge1[1].x = fS; vEdge1[1].y = fT;

							pLMap->SetEdge(nEdge, vEdge0[0].x, vEdge0[0].y, vEdge0[1].x, vEdge0[1].y, vEdge1[0].x, vEdge1[0].y, vEdge1[1].x, vEdge1[1].y);
							nEdge++;
						}
					}
				}
			}
			m_pLightMapList[i]->SetNumEdges(nEdge);
		}
	}

    Rasterize(nMotif, RenderLMapTexel);

	if (nMotif == 0 && 0)
	{
		LN_Triangle_t *pTri;
		f32 fU[2], fV[2];
		for (i=0; i<m_nNumTri; i++)
		{
			pTri = &m_TriangleList[i];

			u8 nEdge, a, b;
			for (nEdge=0; nEdge<3; nEdge++)
			{
				a = nEdge; b = (nEdge+1)%3;
				fU[0] = pTri->pVtx[a]->fBaseUV[0];
				fU[1] = pTri->pVtx[b]->fBaseUV[0];

				fV[0] = pTri->pVtx[a]->fBaseUV[1];
				fV[1] = pTri->pVtx[b]->fBaseUV[1];

				//m_pLightMapList[0]->SetEdge(0, fU[0], fV[0], fU[1], fV[1]);
				//m_pLightMapList[0]->BlendEdges();
			}
		}
	}
}

u32 CRadiosity::PartitionWorld_Vertex()
{
	f32 fMagSq0, fMag;
	u32 i;
	CFVec3 vEdge0, vEdge1;
	CFVec3 vMin(1000000.0f, 1000000.0f, 1000000.0f), vMax(-1000000.0f, -1000000.0f, -1000000.0f);
	LN_Triangle_t *pTri;
	LN_Vertex_t *pVtx[3];

	//1. Generate triangle planes.
	for (i=0; i<m_nNumTri; i++)
	{
		pTri = &m_TriangleList[i];
		pVtx[0] = &m_VertexList[ pTri->nVtx[0] ];
		if (pVtx[0]->fPos[0] < vMin.x) vMin.x = pVtx[0]->fPos[0]; if (pVtx[0]->fPos[1] < vMin.y) vMin.y = pVtx[0]->fPos[1]; if (pVtx[0]->fPos[2] < vMin.z) vMin.z = pVtx[0]->fPos[2];
		if (pVtx[0]->fPos[0] > vMax.x) vMax.x = pVtx[0]->fPos[0]; if (pVtx[0]->fPos[1] > vMax.y) vMax.y = pVtx[0]->fPos[1]; if (pVtx[0]->fPos[2] > vMax.z) vMax.z = pVtx[0]->fPos[2];
		pVtx[1] = &m_VertexList[ pTri->nVtx[1] ];
		if (pVtx[1]->fPos[0] < vMin.x) vMin.x = pVtx[1]->fPos[0]; if (pVtx[1]->fPos[1] < vMin.y) vMin.y = pVtx[1]->fPos[1]; if (pVtx[1]->fPos[2] < vMin.z) vMin.z = pVtx[1]->fPos[2];
		if (pVtx[1]->fPos[0] > vMax.x) vMax.x = pVtx[1]->fPos[0]; if (pVtx[1]->fPos[1] > vMax.y) vMax.y = pVtx[1]->fPos[1]; if (pVtx[1]->fPos[2] > vMax.z) vMax.z = pVtx[1]->fPos[2];
		pVtx[2] = &m_VertexList[ pTri->nVtx[2] ];
		if (pVtx[2]->fPos[0] < vMin.x) vMin.x = pVtx[2]->fPos[0]; if (pVtx[2]->fPos[1] < vMin.y) vMin.y = pVtx[2]->fPos[1]; if (pVtx[2]->fPos[2] < vMin.z) vMin.z = pVtx[2]->fPos[2];
		if (pVtx[2]->fPos[0] > vMax.x) vMax.x = pVtx[2]->fPos[0]; if (pVtx[2]->fPos[1] > vMax.y) vMax.y = pVtx[2]->fPos[1]; if (pVtx[2]->fPos[2] > vMax.z) vMax.z = pVtx[2]->fPos[2];

		vEdge0.Set(pVtx[1]->fPos[0]-pVtx[0]->fPos[0], pVtx[1]->fPos[1]-pVtx[0]->fPos[1], pVtx[1]->fPos[2]-pVtx[0]->fPos[2]);
		vEdge1.Set(pVtx[2]->fPos[0]-pVtx[0]->fPos[0], pVtx[2]->fPos[1]-pVtx[0]->fPos[1], pVtx[2]->fPos[2]-pVtx[0]->fPos[2]);
		pTri->vNrml = vEdge0.Cross(vEdge1);
		fMagSq0 = pTri->vNrml.Mag2();
		if ( fMagSq0 )
		{
			fMag = 1.0f / sqrtf(fMagSq0);
			pTri->fMag = fMag;
			pTri->vNrml *= fMag;
		}
		else
		{
			pTri->fMag = 1.0f;
		}

		LN_Triangle_t::PointInsideTriFunc pTriFunc;

		pTri->fD = -(pTri->vNrml.x*pVtx[0]->fPos[0] + pTri->vNrml.y*pVtx[0]->fPos[1] + pTri->vNrml.z*pVtx[0]->fPos[2]);
		if ( fabsf(pTri->vNrml.x) >= fabsf(pTri->vNrml.y) && fabsf(pTri->vNrml.x) >= fabsf(pTri->vNrml.z))
		{
			pTriFunc = LN_Triangle_t::PointInsideTriYZ;
		}
		else if ( fabsf(pTri->vNrml.y) >= fabsf(pTri->vNrml.x) && fabsf(pTri->vNrml.y) >= fabsf(pTri->vNrml.z) )
		{
			pTriFunc = LN_Triangle_t::PointInsideTriXZ;
		}
		else
		{
			pTriFunc = LN_Triangle_t::PointInsideTriXY;
		}
		pTri->SetTriFunc(pTriFunc);

		pTri->pVtx[0] = pVtx[0];
		pTri->pVtx[1] = pVtx[1];
		pTri->pVtx[2] = pVtx[2];
	}

	BuildkDOPTree();

	//3. Fix up lights
	for (i=0; i<m_nNumLights; i++)
	{
		if (m_LightList[i].LightType != LS_TYPE_NOATTEN)
		{
			m_LightList[i].fIntensity *= (m_LightList[i].fRadius);
		}
		else
		{
			m_LightList[i].fIntensity = 1.0f;
		}
	}
	//

	return (0);
}

u32 CRadiosity::PartitionWorld()
{
	f32 fMagSq0, fMag;
	u32 i;
	CFVec3 vEdge0, vEdge1;
	CFVec3 vMin(1000000.0f, 1000000.0f, 1000000.0f), vMax(-1000000.0f, -1000000.0f, -1000000.0f);
	LN_Triangle_t *pTri;
	LN_Vertex_t *pVtx[3];

	//1. Generate triangle planes.
	for (i=0; i<m_nNumTri; i++)
	{
		pTri = &m_TriangleList[i];
		pVtx[0] = &m_VertexList[ pTri->nVtx[0] ];
		if (pVtx[0]->fPos[0] < vMin.x) vMin.x = pVtx[0]->fPos[0]; if (pVtx[0]->fPos[1] < vMin.y) vMin.y = pVtx[0]->fPos[1]; if (pVtx[0]->fPos[2] < vMin.z) vMin.z = pVtx[0]->fPos[2];
		if (pVtx[0]->fPos[0] > vMax.x) vMax.x = pVtx[0]->fPos[0]; if (pVtx[0]->fPos[1] > vMax.y) vMax.y = pVtx[0]->fPos[1]; if (pVtx[0]->fPos[2] > vMax.z) vMax.z = pVtx[0]->fPos[2];
		pVtx[1] = &m_VertexList[ pTri->nVtx[1] ];
		if (pVtx[1]->fPos[0] < vMin.x) vMin.x = pVtx[1]->fPos[0]; if (pVtx[1]->fPos[1] < vMin.y) vMin.y = pVtx[1]->fPos[1]; if (pVtx[1]->fPos[2] < vMin.z) vMin.z = pVtx[1]->fPos[2];
		if (pVtx[1]->fPos[0] > vMax.x) vMax.x = pVtx[1]->fPos[0]; if (pVtx[1]->fPos[1] > vMax.y) vMax.y = pVtx[1]->fPos[1]; if (pVtx[1]->fPos[2] > vMax.z) vMax.z = pVtx[1]->fPos[2];
		pVtx[2] = &m_VertexList[ pTri->nVtx[2] ];
		if (pVtx[2]->fPos[0] < vMin.x) vMin.x = pVtx[2]->fPos[0]; if (pVtx[2]->fPos[1] < vMin.y) vMin.y = pVtx[2]->fPos[1]; if (pVtx[2]->fPos[2] < vMin.z) vMin.z = pVtx[2]->fPos[2];
		if (pVtx[2]->fPos[0] > vMax.x) vMax.x = pVtx[2]->fPos[0]; if (pVtx[2]->fPos[1] > vMax.y) vMax.y = pVtx[2]->fPos[1]; if (pVtx[2]->fPos[2] > vMax.z) vMax.z = pVtx[2]->fPos[2];

		vEdge0.Set(pVtx[1]->fPos[0]-pVtx[0]->fPos[0], pVtx[1]->fPos[1]-pVtx[0]->fPos[1], pVtx[1]->fPos[2]-pVtx[0]->fPos[2]);
		vEdge1.Set(pVtx[2]->fPos[0]-pVtx[0]->fPos[0], pVtx[2]->fPos[1]-pVtx[0]->fPos[1], pVtx[2]->fPos[2]-pVtx[0]->fPos[2]);
		pTri->vNrml = vEdge0.Cross(vEdge1);
		fMagSq0 = pTri->vNrml.Mag2();
		if ( fMagSq0 )
		{
			fMag = 1.0f / sqrtf(fMagSq0);
			pTri->fMag = fMag;
			pTri->vNrml *= fMag;
		}
		else
		{
			pTri->fMag = 1.0f;
		}

		LN_Triangle_t::PointInsideTriFunc pTriFunc;

		pTri->fD = -(pTri->vNrml.x*pVtx[0]->fPos[0] + pTri->vNrml.y*pVtx[0]->fPos[1] + pTri->vNrml.z*pVtx[0]->fPos[2]);
		if ( fabsf(pTri->vNrml.x) >= fabsf(pTri->vNrml.y) && fabsf(pTri->vNrml.x) >= fabsf(pTri->vNrml.z))
		{
			pTriFunc = LN_Triangle_t::PointInsideTriYZ;
		}
		else if ( fabsf(pTri->vNrml.y) >= fabsf(pTri->vNrml.x) && fabsf(pTri->vNrml.y) >= fabsf(pTri->vNrml.z) )
		{
			pTriFunc = LN_Triangle_t::PointInsideTriXZ;
		}
		else
		{
			pTriFunc = LN_Triangle_t::PointInsideTriXY;
		}
		pTri->SetTriFunc(pTriFunc);

		pTri->pVtx[0] = pVtx[0];
		pTri->pVtx[1] = pVtx[1];
		pTri->pVtx[2] = pVtx[2];
	}

	BuildConnectivityData();

	CFVec3 vN0, vN1;
	u32 j, k;

	LN_Triangle_t *pT;
	LN_Material_t *pMat;
	for (i=0; i<m_nNumMaterials; i++)
	{
		pMat = &m_MaterialList[i];
		if (pMat->fAreaI > 0.0f)
		{
			for (j=0; j<m_nNumTri; j++)
			{
				if (m_TriangleList[j].pMaterial == pMat)
				{
					m_TriangleList[j].bToggle = FALSE;
				}
			}

			for (j=0; j<m_nNumTri; j++)
			{
				if (m_TriangleList[j].pMaterial == pMat && !m_TriangleList[j].bToggle)
				{
					pT = &m_TriangleList[j];
					for (k=0; k<3; k++)
					{
						if (pT->pEdgeTri[k])
						{
							if (pT->vNrml == pT->pEdgeTri[k]->vNrml && !pT->bToggle)
							{
								CreateAreaLight(pT, pT->pEdgeTri[k]);
								pT->bToggle = TRUE;
								pT->pEdgeTri[k]->bToggle = TRUE;
							}
						}
					}
				}
			}
		}
	}

	BuildkDOPTree();

	//3. Fix up lights
	for (i=0; i<m_nNumLights; i++)
	{
		if (m_LightList[i].LightType != LS_TYPE_NOATTEN)
		{
			m_LightList[i].fIntensity *= m_LightList[i].fRadius;
			if (m_LightList[i].LightType == LS_LTYPE_SPOT)
			{
				m_LightList[i].fIntensity *= 20.0f;
			}
		}
		else
		{
			m_LightList[i].fIntensity = 1.0f;
		}
	}
	//

	return (0);
}

BOOL CRadiosity::TriEffectedByLight(LN_Triangle_t *pTri, LN_Light_t *pLight)
{
	if (pLight->LightType == LS_TYPE_NOATTEN)
	{
		return TRUE;
	}
	for (u32 i=0; i<pTri->nNumLights; i++)
	{
		if (pTri->pLights[i] == pLight)
		{
			return (TRUE);
		}
	}
	return (FALSE);
}

static CFVec3 _vOffs;

void CRadiosity::SubDivideQuad(LN_AreaLight_t *pLight, LN_Quad_t *pQuad, u8 nQuad)
{
	LN_Quad_t *pNewQuad;
	CFVec3 vEdge, vVtx;
	u8 nNewIndices[5];
	
	//Edge0
	vEdge = pLight->vVtx[ pQuad->nVtx[1] ] - pLight->vVtx[ pQuad->nVtx[0] ];
	vVtx = vEdge * 0.5f; vVtx += pLight->vVtx[ pQuad->nVtx[0] ];

	nNewIndices[0] = pLight->nVtx;
	pLight->vVtx[ pLight->nVtx++ ] = vVtx;
	//Edge 1
	vEdge = pLight->vVtx[ pQuad->nVtx[2] ] - pLight->vVtx[ pQuad->nVtx[1] ];
	vVtx = vEdge * 0.5f; vVtx += pLight->vVtx[ pQuad->nVtx[1] ];

	nNewIndices[1] = pLight->nVtx;
	pLight->vVtx[ pLight->nVtx++ ] = vVtx;
	//Edge 2
	vEdge = pLight->vVtx[ pQuad->nVtx[3] ] - pLight->vVtx[ pQuad->nVtx[2] ];
	vVtx = vEdge * 0.5f; vVtx += pLight->vVtx[ pQuad->nVtx[2] ];

	nNewIndices[2] = pLight->nVtx;
	pLight->vVtx[ pLight->nVtx++ ] = vVtx;
	//Edge 3
	vEdge = pLight->vVtx[ pQuad->nVtx[0] ] - pLight->vVtx[ pQuad->nVtx[3] ];
	vVtx = vEdge * 0.5f; vVtx += pLight->vVtx[ pQuad->nVtx[3] ];

	nNewIndices[3] = pLight->nVtx;
	pLight->vVtx[ pLight->nVtx++ ] = vVtx;
	// Center of quad.
	nNewIndices[4] = pLight->nVtx;
	pLight->vVtx[ pLight->nVtx++ ] = pQuad->vCen;

	//Make new quads from quad.
	//Upper Left
	pNewQuad = &pLight->anQuads[ pLight->nQuad++ ];
	pNewQuad->nVtx[0] = pQuad->nVtx[0];
	pNewQuad->nVtx[1] = nNewIndices[0];
	pNewQuad->nVtx[2] = nNewIndices[4];
	pNewQuad->nVtx[3] = nNewIndices[3];
	pNewQuad->vCen = pLight->vVtx[ pNewQuad->nVtx[0] ] + pLight->vVtx[ pNewQuad->nVtx[1] ] + pLight->vVtx[ pNewQuad->nVtx[2] ] + pLight->vVtx[ pNewQuad->nVtx[3] ];
	pNewQuad->vCen *= 0.25f;
	//Upper Right
	pNewQuad = &pLight->anQuads[ pLight->nQuad++ ];
	pNewQuad->nVtx[0] = nNewIndices[0];
	pNewQuad->nVtx[1] = pQuad->nVtx[1];
	pNewQuad->nVtx[2] = nNewIndices[1];
	pNewQuad->nVtx[3] = nNewIndices[4];
	pNewQuad->vCen = pLight->vVtx[ pNewQuad->nVtx[0] ] + pLight->vVtx[ pNewQuad->nVtx[1] ] + pLight->vVtx[ pNewQuad->nVtx[2] ] + pLight->vVtx[ pNewQuad->nVtx[3] ];
	pNewQuad->vCen *= 0.25f;
	//Lower Left
	pNewQuad = &pLight->anQuads[ pLight->nQuad++ ];
	pNewQuad->nVtx[0] = nNewIndices[3];
	pNewQuad->nVtx[1] = nNewIndices[4];
	pNewQuad->nVtx[2] = nNewIndices[2];
	pNewQuad->nVtx[3] = pQuad->nVtx[3];
	pNewQuad->vCen = pLight->vVtx[ pNewQuad->nVtx[0] ] + pLight->vVtx[ pNewQuad->nVtx[1] ] + pLight->vVtx[ pNewQuad->nVtx[2] ] + pLight->vVtx[ pNewQuad->nVtx[3] ];
	pNewQuad->vCen *= 0.25f;
	//Lower Right
	pNewQuad = &pLight->anQuads[ pLight->nQuad++ ];
	pNewQuad->nVtx[0] = nNewIndices[4];
	pNewQuad->nVtx[1] = nNewIndices[1];
	pNewQuad->nVtx[2] = pQuad->nVtx[2];
	pNewQuad->nVtx[3] = nNewIndices[2];
	pNewQuad->vCen = pLight->vVtx[ pNewQuad->nVtx[0] ] + pLight->vVtx[ pNewQuad->nVtx[1] ] + pLight->vVtx[ pNewQuad->nVtx[2] ] + pLight->vVtx[ pNewQuad->nVtx[3] ];
	pNewQuad->vCen *= 0.25f;

	//Now remove parent quad.
	s32 i;
	for (i=(s32)nQuad; i<pLight->nQuad-1; i++)
	{
		pLight->anQuads[i] = pLight->anQuads[i+1];
	}
	pLight->nQuad--;
}	

void CRadiosity::CreateAreaLight(LN_Triangle_t *pTri0, LN_Triangle_t *pTri1)
{
	LN_AreaLight_t aLight;

	aLight.vNrml = pTri0->vNrml;
	aLight.fD = pTri0->fD;

	aLight.fRed = pTri0->pMaterial->fAreaR;
	aLight.fGreen = pTri0->pMaterial->fAreaG;
	aLight.fBlue = pTri0->pMaterial->fAreaB;
	aLight.fIntensity = pTri0->pMaterial->fAreaI;

    LN_Vertex_t *vVtx[4], *vNew;
	vVtx[0] = &m_VertexList[ pTri0->nVtx[0] ];
	vVtx[1] = &m_VertexList[ pTri0->nVtx[1] ];
	vVtx[3] = &m_VertexList[ pTri0->nVtx[2] ];
	
	u32 j;
	for (j=0; j<3; j++)
	{
		vNew = &m_VertexList[ pTri1->nVtx[j] ];
		if ( !VtxEqual(vNew, vVtx[0]) && !VtxEqual(vNew, vVtx[1]) && !VtxEqual(vNew, vVtx[3]) )
		{
			vVtx[2] = vNew;
			break;
		}
	}
	
	aLight.nX = 1; aLight.nY = 1; aLight.nVtx = 4; aLight.nQuad = 1;
	aLight.vVtx[0].Set( vVtx[0]->fPos[0], vVtx[0]->fPos[1], vVtx[0]->fPos[2]);
	aLight.vVtx[1].Set( vVtx[1]->fPos[0], vVtx[1]->fPos[1], vVtx[1]->fPos[2]);
	aLight.vVtx[2].Set( vVtx[2]->fPos[0], vVtx[2]->fPos[1], vVtx[2]->fPos[2]);
    aLight.vVtx[3].Set( vVtx[3]->fPos[0], vVtx[3]->fPos[1], vVtx[3]->fPos[2]);

	aLight.nX = 1; aLight.nY = 1; aLight.nVtx = 4; aLight.nQuad = 1;
	aLight.vVtx[0].Set( vVtx[0]->fPos[0], vVtx[0]->fPos[1], vVtx[0]->fPos[2]);
	aLight.vVtx[1].Set( vVtx[1]->fPos[0], vVtx[1]->fPos[1], vVtx[1]->fPos[2]);
	aLight.vVtx[2].Set( vVtx[2]->fPos[0], vVtx[2]->fPos[1], vVtx[2]->fPos[2]);
    aLight.vVtx[3].Set( vVtx[3]->fPos[0], vVtx[3]->fPos[1], vVtx[3]->fPos[2]);
	
	CFVec3 vEdge0, vEdge1;

	vEdge0 = aLight.vVtx[1] - aLight.vVtx[0];
	vEdge1 = aLight.vVtx[2] - aLight.vVtx[1];

	f32 fMag0, fMag1;

	fMag0 = vEdge0.Mag();
	fMag1 = vEdge1.Mag();

	aLight.fArea = fMag0 * fMag1;

	aLight.anQuads[0].nVtx[0] = 0; aLight.anQuads[0].nVtx[1] = 1; aLight.anQuads[0].nVtx[2] = 2; aLight.anQuads[0].nVtx[3] = 3;
	aLight.anQuads[0].vCen.x = ( vVtx[0]->fPos[0] + vVtx[1]->fPos[0] + vVtx[2]->fPos[0] + vVtx[3]->fPos[0] ) * 0.25f;
	aLight.anQuads[0].vCen.y = ( vVtx[0]->fPos[1] + vVtx[1]->fPos[1] + vVtx[2]->fPos[1] + vVtx[3]->fPos[1] ) * 0.25f;
	aLight.anQuads[0].vCen.z = ( vVtx[0]->fPos[2] + vVtx[1]->fPos[2] + vVtx[2]->fPos[2] + vVtx[3]->fPos[2] ) * 0.25f;

	//subdivide into 2x2
	aLight.fArea *= 0.25f;
	SubDivideQuad(&aLight, &aLight.anQuads[0], 0);
	//subdivide into 4x4
	aLight.fArea *= 0.25f;
	u32 n = aLight.nQuad;
	for (j=0; j<n; j++)
	{
		SubDivideQuad(&aLight, &aLight.anQuads[0], 0);
	}

    m_AreaLights[m_nAreaLights++] = aLight;
}

BOOL CRadiosity::PointInsideTriSphere(LN_Triangle_t *pTri, CFVec3& vPoint)
{
	_vOffs = pTri->vCen - vPoint;
	if (_vOffs.Mag2() > (pTri->fRadius * pTri->fRadius))
	{
		return (FALSE);
	}
	return (TRUE);
}

f32 CRadiosity::RayTrace_PatchVisibility(RA_Patch_t *pPatchSrc, RA_Patch_t *pPatchDst)
{
	CFVec3 vStart, vEnd;
	CFVec3 vClr;
	static f32 fOO13 = (1.0f/13.0f);
	f32 fVis=0.0f;

	vStart = pPatchSrc->vVtx[0]; vEnd = pPatchDst->vVtx[0];
	m_vMulClr.Set(1,1,1);	
	fVis += RayTrace(vStart, vEnd);
	vClr += m_vMulClr;
	vStart = pPatchSrc->vVtx[1]; vEnd = pPatchDst->vVtx[1];
	m_vMulClr.Set(1,1,1);
	fVis += RayTrace(vStart, vEnd);
	vClr += m_vMulClr;
	vStart = pPatchSrc->vVtx[2]; vEnd = pPatchDst->vVtx[2];
	m_vMulClr.Set(1,1,1);
	fVis += RayTrace(vStart, vEnd);
	vClr += m_vMulClr;
	vStart = pPatchSrc->vVtx[3]; vEnd = pPatchDst->vVtx[3];
	m_vMulClr.Set(1,1,1);
	fVis += RayTrace(vStart, vEnd);
	vClr += m_vMulClr;
	vStart = pPatchSrc->vCen; vEnd = pPatchDst->vCen;
	m_vMulClr.Set(1,1,1);
	fVis += RayTrace(vStart, vEnd);
	vClr += m_vMulClr;

	u32 i;
	vStart = pPatchSrc->vCen;
	for (i=0; i<4; i++)
	{
		vEnd = pPatchDst->vVtx[i];
		m_vMulClr.Set(1,1,1);
		fVis += RayTrace(vStart, vEnd);
		vClr += m_vMulClr;
	}

	vEnd = pPatchDst->vCen;
	for (i=0; i<4; i++)
	{
		vStart = pPatchSrc->vVtx[i];
		m_vMulClr.Set(1,1,1);
		fVis += RayTrace(vStart, vEnd);
		vClr += m_vMulClr;
	}

	fVis *= fOO13;
	m_vMulClr.Set( vClr.x*fOO13, vClr.y*fOO13, vClr.z*fOO13 );

	return (fVis);
}

f32 CRadiosity::RayTrace_AreaLight(LN_AreaLight_t *pAreaLight, CFVec3& vStart)
{
	CFVec3 vEnd, vClr;
	static f32 fOO5 = (1.0f/5.0f);
	f32 fVis=0.0f;
	CFVec2 vUV;
	LN_Triangle_t *pCollTri=NULL;

	vEnd = m_pCurAreaLight->vVtx[ m_pCurAreaLight->anQuads[m_nCurAreaQuad].nVtx[0] ];
	m_vMulClr.Set(1,1,1);	
	fVis += RayTrace(vStart, vEnd, &vUV, &pCollTri, NULL, NULL);
	vClr += m_vMulClr;
	vEnd = m_pCurAreaLight->vVtx[ m_pCurAreaLight->anQuads[m_nCurAreaQuad].nVtx[1] ];
	m_vMulClr.Set(1,1,1);
	fVis += RayTrace(vStart, vEnd, &vUV, &pCollTri, NULL, NULL);
	vClr += m_vMulClr;
	vEnd = m_pCurAreaLight->vVtx[ m_pCurAreaLight->anQuads[m_nCurAreaQuad].nVtx[2] ];
	m_vMulClr.Set(1,1,1);
	fVis += RayTrace(vStart, vEnd, &vUV, &pCollTri, NULL, NULL);
	vClr += m_vMulClr;
	vEnd = m_pCurAreaLight->vVtx[ m_pCurAreaLight->anQuads[m_nCurAreaQuad].nVtx[3] ];
	m_vMulClr.Set(1,1,1);
	fVis += RayTrace(vStart, vEnd, &vUV, &pCollTri, NULL, NULL);
	vClr += m_vMulClr;
	vEnd = m_pCurAreaLight->anQuads[m_nCurAreaQuad].vCen;
	m_vMulClr.Set(1,1,1);
	fVis += RayTrace(vStart, vEnd, &vUV, &pCollTri, NULL, NULL);
	vClr += m_vMulClr;

	fVis *= fOO5;
	m_vMulClr.Set( vClr.x*fOO5, vClr.y*fOO5, vClr.z*fOO5 );

	return (fVis);
}

BOOL CRadiosity::FindUVIntersect(LN_Triangle_t *pTri, CFVec3& vPoint, CFVec2 *pvMaskUV)
{
	//no need to calculate uv intersection if no masks or color information exists.
	if (!pTri->pMaterial->pZMask && !pTri->pMaterial->pMask && !pTri->pMaterial->pColor) { return FALSE; }

	//Calculate the Barycentric coordinates of the triangle given vPoint, and use that to calculate the resulting texture coordinates.
	CFVec3 vVec0, vVec1, vUnit;
	f32 fS, fT, fR;
	//calculate my barycentric coordinates for my pos (vBase)
	vVec0.x = vPoint.x - pTri->pVtx[0]->fPos[0];
	vVec0.y = vPoint.y - pTri->pVtx[0]->fPos[1];
	vVec0.z = vPoint.z - pTri->pVtx[0]->fPos[2];

	vVec1.x = pTri->pVtx[2]->fPos[0] - pTri->pVtx[0]->fPos[0];
	vVec1.y = pTri->pVtx[2]->fPos[1] - pTri->pVtx[0]->fPos[1];
	vVec1.z = pTri->pVtx[2]->fPos[2] - pTri->pVtx[0]->fPos[2];

	vUnit = vVec0.Cross(vVec1); vUnit = vUnit * pTri->fMag;
	fS = vUnit.Dot( pTri->vNrml );
	FMATH_CLAMP(fS, 0, 1);

	vVec1.x = pTri->pVtx[1]->fPos[0] - pTri->pVtx[0]->fPos[0];
	vVec1.y = pTri->pVtx[1]->fPos[1] - pTri->pVtx[0]->fPos[1];
	vVec1.z = pTri->pVtx[1]->fPos[2] - pTri->pVtx[0]->fPos[2];

	vUnit = vVec1.Cross(vVec0); vUnit = vUnit * pTri->fMag;
	fT = vUnit.Dot( pTri->vNrml );
	FMATH_CLAMP(fT, 0, 1);

	fR = 1.0f - (fS + fT);
	FMATH_CLAMP(fR, 0, 1);

	//calculate normal using barycentric coordinates (fS, fT, fR)
	pvMaskUV->x = pTri->pVtx[0]->fMaskUV[0]*fR + pTri->pVtx[1]->fMaskUV[0]*fS + pTri->pVtx[2]->fMaskUV[0]*fT;
	pvMaskUV->y = pTri->pVtx[0]->fMaskUV[1]*fR + pTri->pVtx[1]->fMaskUV[1]*fS + pTri->pVtx[2]->fMaskUV[1]*fT;

	pvMaskUV->x = fmodf( pvMaskUV->x, 1.0f );
	pvMaskUV->y = fmodf( pvMaskUV->y, 1.0f );

	pvMaskUV->x = (pvMaskUV->x < 0.0f) ? (pvMaskUV->x + 1.0f) : (pvMaskUV->x);
	pvMaskUV->y = (pvMaskUV->y < 0.0f) ? (pvMaskUV->y + 1.0f) : (pvMaskUV->y);

	return TRUE;
}

#if _USE_KDOPS

static u32 _nNumRayTraces=0;

f32 CRadiosity::RayTrace(CFVec3& vRayStart, CFVec3& vRayEnd, CFVec2 *pvMaskUV, LN_Triangle_t **pCollTri, CFVec3 *pNrml, LN_Light_t *pLight)
{
	LN_kDOP_t *apkDOPStack[1000];
	BOOL bDir=FALSE, bBreak=FALSE;
	f32 fRet = 1.0f;
	f32 fEps = 0.001f;
	f32 fMax = 1.0f;
	LN_Triangle_t *pCurTri;

	CFVec3A vStart(vRayStart);
	CFVec3A vEnd(vRayEnd);

	static CFVec3A __vDp, __vPoint;
	f32 fU, fDenom, fClr[4], fDist;

	static volatile BOOL bHasTrans=FALSE;

	__vDp.Sub( vEnd, vStart );

	if (pLight)
	{
		if ( pLight->LightType == LS_TYPE_NOATTEN ) 
		{
			fEps = 0.50f;
			vEnd.Mul( __vDp, 100000000.f ).Add( vStart );
			bDir = TRUE;
		}
	}
	else if (_bSkyLight)
	{
		fEps = 0.0001f;
	}

	// Prime the stack to prepare for the ray cast
	apkDOPStack[0] = m_pakDOPTree;
	u32 nStackCount = 1;

	_nNumRayTraces++;

	if (_bIndirect)
	{
		bDir = FALSE;
		fMax = 0.99f;
	}

	// Precalculate the kDOP intervals for the start and end points	
	f32 _afStartIntervals[FKDOP_MAX_AXES];	
	f32 _afEndIntervals[FKDOP_MAX_AXES];	
	_afStartIntervals[0] = vStart.a[0];
	_afEndIntervals[0]   = vEnd.a[0];
	_afStartIntervals[1] = vStart.a[1];
	_afEndIntervals[1]   = vEnd.a[1];
	_afStartIntervals[2] = vStart.a[2];
	_afEndIntervals[2]   = vEnd.a[2];
	u32 i;
	for ( i = 3; i < FkDOP_aDesc[LS_KDOP_TYPE].nNormalCount; i++ )
	{
		_afStartIntervals[i] = vStart.Dot( FkDOP_aDesc[LS_KDOP_TYPE].avNormals[i] );
		_afEndIntervals[i] 	 = vEnd.Dot( FkDOP_aDesc[LS_KDOP_TYPE].avNormals[i] );
	}	
	
	// Walk the kDOP tree looking for potential collisions
	while ( nStackCount )
	{
		LN_kDOP_t *pkDOP = apkDOPStack[--nStackCount];
		FASSERT( pkDOP );

		if ( fkdop_IntersectkDOPAndRay( _afStartIntervals, _afEndIntervals, pkDOP->aIntervals, LS_KDOP_TYPE ) == -1.f )
		{
			continue;
		}

		if ( !pkDOP->pFirstTriangle )
		{
			// This is not a leaf kDOP, so we should push the children onto the kDOP stack;
			apkDOPStack[nStackCount++] = pkDOP->papChildren[0];
			apkDOPStack[nStackCount++] = pkDOP->papChildren[1];
			continue;
		}

		// This is a leaf kDOP, so we should test the triangles in the leaf
		for ( pCurTri = pkDOP->pFirstTriangle; pCurTri; pCurTri = pCurTri->pkDOPNext )
		{
			if ( !(pCurTri->pMaterial->bRayTest) ) { continue; }

			if (m_pOrigTri)
			{
				if (pCurTri->pEdgeTri[0] == m_pOrigTri)
				{
					if (pCurTri->pEdgeTri[0]->vNrml.Dot(m_pOrigTri->vNrml) > 0.7071f)
					{
						continue;
					}
				}
				else if (pCurTri->pEdgeTri[1] == m_pOrigTri)
				{
					if (pCurTri->pEdgeTri[1]->vNrml.Dot(m_pOrigTri->vNrml) > 0.7071f)
					{
						continue;
					}
				}
				else if (pCurTri->pEdgeTri[2] == m_pOrigTri)
				{
					if (pCurTri->pEdgeTri[2]->vNrml.Dot(m_pOrigTri->vNrml) > 0.7071f)
					{
						continue;
					}
				}

				// See if this triangle is coplanar with the test triangle (Coplanar tris can't block light - This
				// code takes care of backface polys)
				u32 nTestIdx;
				CFVec3 vDiff;
				for ( nTestIdx = 0; nTestIdx < 3; nTestIdx++ )
				{
					vDiff.x = pCurTri->pVtx[nTestIdx]->fPos[0] - m_pOrigTri->pVtx[0]->fPos[0];
					vDiff.y = pCurTri->pVtx[nTestIdx]->fPos[1] - m_pOrigTri->pVtx[0]->fPos[1];
					vDiff.z = pCurTri->pVtx[nTestIdx]->fPos[2] - m_pOrigTri->pVtx[0]->fPos[2];
					if ( fmath_Abs( vDiff.Dot( m_pOrigTri->vNrml ) ) > 0.001f )
					{
						break;
					}
				}
				if ( nTestIdx == 3 )
				{
					continue;
				}
			}

			if (pLight&&0)
			{
				if ( !TriEffectedByLight(pCurTri, pLight) )
				{
					continue;
				}
			}

			//1. Find intersection of Ray with triangle plane.
			fDenom = pCurTri->vNrml.x*__vDp.x + pCurTri->vNrml.y*__vDp.y + pCurTri->vNrml.z*__vDp.z;
			if (fDenom)
			{
				fDist = (pCurTri->fD + pCurTri->vNrml.x*vStart.x + pCurTri->vNrml.y*vStart.y + pCurTri->vNrml.z*vStart.z);
				fU = -fDist /(fDenom);

				if ( fU >= fEps && fabsf(fDist) > 0.1f && (fU < fMax || bDir) )
				{
					__vPoint.Mul( __vDp, fU ).Add( vStart );
//					vPoint.x = vStart.x + fU*__vDp.x;
//					vPoint.y = vStart.y + fU*__vDp.y;
//					vPoint.z = vStart.z + fU*__vDp.z;

					if (PointInsideTriSphere(pCurTri, __vPoint.v3))
					{
						if ( pCurTri->pPointInsideTri(pCurTri, __vPoint.v3) )
						{
							if (pvMaskUV && pCollTri)
							{
								*pCollTri = pCurTri;
								if ((*pCollTri)->pMaterial->pZMask)
								{
									if (FindUVIntersect(*pCollTri, __vPoint.v3, pvMaskUV))
									{
										(*pCollTri)->pMaterial->pZMask->GetColor(fClr, pvMaskUV->x, pvMaskUV->y);
										if (fClr[3] < 1.0f)
										{
											//transparent or translucent pixel
											fRet *= (1.0f - fClr[3]);
											if (fClr[0] > 0.0f || fClr[1] > 0.0f || fClr[2] > 0.0f)
											{
												m_vMulClr.x *= fClr[0] * (*pCollTri)->pMaterial->fRed;
												m_vMulClr.y *= fClr[1] * (*pCollTri)->pMaterial->fGreen;
												m_vMulClr.z *= fClr[2] * (*pCollTri)->pMaterial->fBlue;
											}
											continue;
										}
									}
								}
							}

							return 0.0f;
						}
					}
				}
			}
		}
	}

	return (fRet);
}
#else
f32 CRadiosity::RayTrace(CFVec3& vStart, CFVec3& vEnd, CFVec2 *pvMaskUV, LN_Triangle_t **pCollTri, CFVec3 *pNrml, LN_Light_t *pLight)
{
	BOOL bDir=FALSE, bBreak=FALSE;
	f32 fRet = 1.0f;
	f32 fEps = 0.001f;
	LN_Triangle_t *pCurTri;

	//dumb method
	CFVec3 vDp, vPoint;
	f32 fU, fDenom, fClr[4];
	u32 i;

	static volatile BOOL bHasTrans=FALSE;

	vDp = vEnd - vStart;

	if (pLight)
	{
		if ( pLight->LightType == LS_TYPE_NOATTEN ) 
		{
			fEps = 0.50f;
			bDir = TRUE;
		}
	}

	for (i=0; i<m_nNumTri; i++)
	{
		pCurTri = &m_TriangleList[i];

		if ( !(pCurTri->pMaterial->bRayTest) ) { continue; }

		if (pLight&&0)
		{
			if ( !TriEffectedByLight(pCurTri, pLight) )
			{
				continue;
			}
		}

		//1. Find intersection of Ray with triangle plane.
		fDenom = pCurTri->vNrml.x*vDp.x + pCurTri->vNrml.y*vDp.y + pCurTri->vNrml.z*vDp.z;
		if (fDenom)
		{
			fU = -(pCurTri->fD + pCurTri->vNrml.x*vStart.x + pCurTri->vNrml.y*vStart.y + pCurTri->vNrml.z*vStart.z)/(fDenom);

			if ( fU >= fEps && (fU <= 1.0f || bDir) )
			{
				vPoint.x = vStart.x + fU*vDp.x;
				vPoint.y = vStart.y + fU*vDp.y;
				vPoint.z = vStart.z + fU*vDp.z;

				if (PointInsideTriSphere(pCurTri, vPoint))
				{
					if ( pCurTri->pPointInsideTri(pCurTri, vPoint) )
					{
						if (pvMaskUV && pCollTri)
						{
							*pCollTri = pCurTri;
							if ((*pCollTri)->pMaterial->pZMask)
							{
								if (FindUVIntersect(*pCollTri, vPoint, pvMaskUV))
								{
									(*pCollTri)->pMaterial->pZMask->GetColor(fClr, pvMaskUV->x, pvMaskUV->y);
									if (fClr[3] < 1.0f)
									{
										//transparent or translucent pixel
										fRet *= (1.0f - fClr[3]);
										if (fClr[0] > 0.0f || fClr[1] > 0.0f || fClr[2] > 0.0f)
										{
											m_vMulClr.x *= fClr[0] * (*pCollTri)->pMaterial->fRed;
											m_vMulClr.y *= fClr[1] * (*pCollTri)->pMaterial->fGreen;
											m_vMulClr.z *= fClr[2] * (*pCollTri)->pMaterial->fBlue;
										}
										continue;
									}
								}
							}
						}
						fRet = 0.0f;
						break;
					}
				}
			}
		}
	}

	return (fRet);
}
#endif
void CRadiosity::RasterPreCalc(u16 nMotif)
{
	if (!m_pRasterPreCalc)
	{
		m_pRasterPreCalc = (LN_RASTER_PRECALC_t *)malloc(sizeof(LN_RASTER_PRECALC_t)*m_nNumTri);
		memset(m_pRasterPreCalc, 0, sizeof(LN_RASTER_PRECALC_t)*m_nNumTri);
	}
	if (!m_pRasterPreCalcMask)
	{
		m_pRasterPreCalcMask = (LN_RASTER_PRECALC_t *)malloc(sizeof(LN_RASTER_PRECALC_t)*m_nNumTri);
		memset(m_pRasterPreCalcMask, 0, sizeof(LN_RASTER_PRECALC_t)*m_nNumTri);
	}

	BOOL bBase=FALSE;
	u32 nTri, i, nUVSet;
	u16 nLMap_Mat, nLMap;
	LN_EDGE_t Edge[3];
	f32 fMinV=1.0f, fMaxV=0.0f, fDelta;
	f32 fWm1;
	f32 fOOW;
	LN_Triangle_t *pTri;

	for (nTri=0; nTri<m_nNumTri; nTri++)
	{
		pTri = &m_TriangleList[nTri];

		if (!pTri->pMaterial->bHasLightMap || !pTri->pMaterial->pLightMaps[0])
		{
			m_pRasterPreCalc[nTri].bRenderTri = FALSE;
			continue;
		}
		
		nLMap_Mat = GetMtlMotifIdx(pTri->pMaterial, nMotif);
		nLMap = GetLightMapIdx( pTri->pMaterial->pLightMaps[nLMap_Mat] );

		fWm1 = (f32)(pTri->pMaterial->pLightMaps[ nLMap_Mat ]->GetWidth());
		fOOW = (1.0f/(f32)(pTri->pMaterial->pLightMaps[ nLMap_Mat ]->GetWidth()));
		fDelta = (1.0f/(f32)(pTri->pMaterial->pLightMaps[ nLMap_Mat ]->GetWidth()-1));
		
		m_pRasterPreCalc[nTri].nLMap = nLMap;

		m_pRasterPreCalc[nTri].fWm1 = fWm1;
		m_pRasterPreCalc[nTri].fOOW = fOOW;
		m_pRasterPreCalc[nTri].fDelta = fDelta;
		
		if ( ValueInArray16(pTri->pMaterial->nNumMotif, pTri->pMaterial->anMotifArray, nMotif) || nMotif == 0 )
		{
			m_pRasterPreCalc[nTri].bRenderTri = TRUE;
			nUVSet = FindUVSet(pTri->pVolume, nMotif);
			for (i=0; i<3; i++)
			{
				if (nMotif == 0)
				{
					Edge[i].vA.Set(m_VertexList[ pTri->nVtx[i] ].fBaseUV[0], m_VertexList[ pTri->nVtx[i] ].fBaseUV[1]);
					Edge[i].vB.Set(m_VertexList[ pTri->nVtx[(i+1)%3] ].fBaseUV[0], m_VertexList[ pTri->nVtx[(i+1)%3] ].fBaseUV[1]);
				}
				else
				{
					Edge[i].vA.Set(m_VertexList[ pTri->nVtx[i] ].aUVSet[nUVSet].fU, m_VertexList[ pTri->nVtx[i] ].aUVSet[nUVSet].fV);
					Edge[i].vB.Set(m_VertexList[ pTri->nVtx[(i+1)%3] ].aUVSet[nUVSet].fU, m_VertexList[ pTri->nVtx[(i+1)%3] ].aUVSet[nUVSet].fV);
				}

				Edge[i].fPosA[0] = m_VertexList[ pTri->nVtx[i] ].fPos[0];
				Edge[i].fPosA[1] = m_VertexList[ pTri->nVtx[i] ].fPos[1];
				Edge[i].fPosA[2] = m_VertexList[ pTri->nVtx[i] ].fPos[2];

				Edge[i].fPosB[0] = m_VertexList[ pTri->nVtx[(i+1)%3] ].fPos[0];
				Edge[i].fPosB[1] = m_VertexList[ pTri->nVtx[(i+1)%3] ].fPos[1];
				Edge[i].fPosB[2] = m_VertexList[ pTri->nVtx[(i+1)%3] ].fPos[2];

				Edge[i].fNrmlA[0] = m_VertexList[ pTri->nVtx[i] ].fNrml[0];
				Edge[i].fNrmlA[1] = m_VertexList[ pTri->nVtx[i] ].fNrml[1];
				Edge[i].fNrmlA[2] = m_VertexList[ pTri->nVtx[i] ].fNrml[2];

				Edge[i].fNrmlB[0] = m_VertexList[ pTri->nVtx[(i+1)%3] ].fNrml[0];
				Edge[i].fNrmlB[1] = m_VertexList[ pTri->nVtx[(i+1)%3] ].fNrml[1];
				Edge[i].fNrmlB[2] = m_VertexList[ pTri->nVtx[(i+1)%3] ].fNrml[2];
			
				Edge[i].vOffs = Edge[i].vB - Edge[i].vA;
				Edge[i].fMinV = (Edge[i].vA.y < Edge[i].vB.y)?(Edge[i].vA.y):(Edge[i].vB.y);
				Edge[i].fMaxV = (Edge[i].vA.y > Edge[i].vB.y)?(Edge[i].vA.y):(Edge[i].vB.y);

				if (Edge[i].fMinV < fMinV) fMinV = Edge[i].fMinV;
				if (Edge[i].fMaxV > fMaxV) fMaxV = Edge[i].fMaxV;

				if (Edge[i].vOffs.y)
				{
					Edge[i].fInvOffsY = (1.0f/Edge[i].vOffs.y);
				}
				else
				{
					Edge[i].fInvOffsY = 1.0f;
				}

				m_pRasterPreCalc[nTri].aEdges[i] = Edge[i];
			}

			if (fMinV < 0.0f) fMinV = 0.0f;

            m_pRasterPreCalc[nTri].fMinV = fMinV;
			m_pRasterPreCalc[nTri].fMaxV = fMaxV;

			//testing
			//pTri->pMaterial->pLightMaps[ nLMap_Mat ]->ConvertV(m_pRasterPreCalc[nTri].fMinV);
			//pTri->pMaterial->pLightMaps[ nLMap_Mat ]->ConvertV(m_pRasterPreCalc[nTri].fMaxV);
			m_pRasterPreCalc[nTri].pLightMap = pTri->pMaterial->pLightMaps[ nLMap_Mat ];
			//
		}
		else
		{
			m_pRasterPreCalc[nTri].bRenderTri = FALSE;
		}
	}	

	if (nMotif == 0)
	{
		for (nTri=0; nTri<m_nNumTri; nTri++)
		{
			pTri = &m_TriangleList[nTri];
			if (pTri->pMaterial->pEmissive)
			{
				fWm1 = (f32)(pTri->pMaterial->pEmissive->GetWidth());
				fOOW = (1.0f/(f32)(pTri->pMaterial->pEmissive->GetWidth()));
				fDelta = (1.0f/(f32)(pTri->pMaterial->pEmissive->GetWidth()-1));
				
				m_pRasterPreCalcMask[nTri].fWm1 = fWm1;
				m_pRasterPreCalcMask[nTri].fOOW = fOOW;
				m_pRasterPreCalcMask[nTri].fDelta = fDelta;
				
				m_pRasterPreCalcMask[nTri].bRenderTri = TRUE;
				for (i=0; i<3; i++)
				{
					if (nMotif == 0)
					{
						Edge[i].vA.Set(m_VertexList[ pTri->nVtx[i] ].fMaskUV[0], m_VertexList[ pTri->nVtx[i] ].fMaskUV[1]);
						Edge[i].vB.Set(m_VertexList[ pTri->nVtx[(i+1)%3] ].fMaskUV[0], m_VertexList[ pTri->nVtx[(i+1)%3] ].fMaskUV[1]);
					}
					
					Edge[i].fPosA[0] = m_VertexList[ pTri->nVtx[i] ].fPos[0];
					Edge[i].fPosA[1] = m_VertexList[ pTri->nVtx[i] ].fPos[1];
					Edge[i].fPosA[2] = m_VertexList[ pTri->nVtx[i] ].fPos[2];

					Edge[i].fPosB[0] = m_VertexList[ pTri->nVtx[(i+1)%3] ].fPos[0];
					Edge[i].fPosB[1] = m_VertexList[ pTri->nVtx[(i+1)%3] ].fPos[1];
					Edge[i].fPosB[2] = m_VertexList[ pTri->nVtx[(i+1)%3] ].fPos[2];
				
					Edge[i].vOffs = Edge[i].vB - Edge[i].vA;
					Edge[i].fMinV = (Edge[i].vA.y < Edge[i].vB.y)?(Edge[i].vA.y):(Edge[i].vB.y);
					Edge[i].fMaxV = (Edge[i].vA.y > Edge[i].vB.y)?(Edge[i].vA.y):(Edge[i].vB.y);

					if (Edge[i].fMinV < fMinV) fMinV = Edge[i].fMinV;
					if (Edge[i].fMaxV > fMaxV) fMaxV = Edge[i].fMaxV;

					if (Edge[i].vOffs.y)
					{
						Edge[i].fInvOffsY = (1.0f/Edge[i].vOffs.y);
					}
					else
					{
						Edge[i].fInvOffsY = 1.0f;
					}

					m_pRasterPreCalcMask[nTri].aEdges[i] = Edge[i];
				}

                if (fMinV < 0.0f) fMinV = 0.0f;

				m_pRasterPreCalcMask[nTri].fMinV = fMinV;
				m_pRasterPreCalcMask[nTri].fMaxV = fMaxV;

				//testing
				//pTri->pMaterial->pLightMaps[ nLMap_Mat ]->ConvertV(m_pRasterPreCalc[nTri].fMinV);
				//pTri->pMaterial->pLightMaps[ nLMap_Mat ]->ConvertV(m_pRasterPreCalc[nTri].fMaxV);
				m_pRasterPreCalc[nTri].pLightMap = pTri->pMaterial->pLightMaps[ nLMap_Mat ];
				//
			}
			else
			{
				m_pRasterPreCalcMask[nTri].bRenderTri = FALSE;
			}
		}	
	}
}

void CRadiosity::RasterizeTriangle(u16 nMotif, RasterizeCallbackFunc pFunc, LN_Triangle_t *pTri, u32 nTri)
{
	f32 fM;
	f32 fCurU, fCurV;
	f32 fEndU;
	f32 fDeltaM;
	u32 i;

	CFVec2 v2Tmp;
	CFVec3 v3Tmp;
	CFVec3 vDelta, vPos;

	LN_RASTER_PRECALC_t *pRaster = &m_pRasterPreCalc[nTri];
	fCurV = pRaster->fMinV;
	u8 nIdx=0;
	CFVec2 vI[2]; //intersections.
	CFVec3 vI_Pos[3];
	while (fCurV <= pRaster->fMaxV)
	{
		nIdx=0;
		for (i=0; i<3; i++)
		{	
			if (fCurV >= pRaster->aEdges[i].fMinV && fCurV <= pRaster->aEdges[i].fMaxV && (pRaster->aEdges[i].fMinV != pRaster->aEdges[i].fMaxV)) //horizontal ray intersects Edges.
			{
				fM = (fCurV - pRaster->aEdges[i].vA.y)*(pRaster->aEdges[i].fInvOffsY);
				if (fM >= 0.0f && fM <= 1.0f)
				{
					fCurU = pRaster->aEdges[i].vA.x + fM*(pRaster->aEdges[i].vOffs.x);

					vI[nIdx].x = fCurU; vI[nIdx].y = fCurV;
					vI_Pos[nIdx].x = pRaster->aEdges[i].fPosA[0] + fM*(pRaster->aEdges[i].fPosB[0] - pRaster->aEdges[i].fPosA[0]);
					vI_Pos[nIdx].y = pRaster->aEdges[i].fPosA[1] + fM*(pRaster->aEdges[i].fPosB[1] - pRaster->aEdges[i].fPosA[1]);
					vI_Pos[nIdx].z = pRaster->aEdges[i].fPosA[2] + fM*(pRaster->aEdges[i].fPosB[2] - pRaster->aEdges[i].fPosA[2]);
					nIdx++;
					if (nIdx == 2)
					{
						break;
					}
				}
			}
		}

		if (nIdx == 2)
		{
			if (vI[0].x > vI[1].x)
			{
				v2Tmp = vI[0];
				vI[0] = vI[1];
				vI[1] = v2Tmp;

				v3Tmp = vI_Pos[0];
				vI_Pos[0] = vI_Pos[1];
				vI_Pos[1] = v3Tmp;
			}

			if (vI[0].x != vI[1].x)
			{
				fCurU = vI[0].x;
				fEndU = vI[1].x;

				//Make sure my scalines are not too short, round <- ..... ->
				//fCurU = (f32)( (s32)(fCurU*pRaster->fWm1 - 0.5f) );
				//fCurU = fCurU * pRaster->fOOW;
				if (fCurU < 0.0f) fCurU = 0.0f;

				//fEndU = (f32)( (s32)(fEndU*pRaster->fWm1 + 0.5f) );
				//fEndU = fEndU * pRaster->fOOW;
				if (fEndU > 1.0f) fEndU = 1.0f;

				fDeltaM = (pRaster->fDelta)/(vI[1].x - vI[0].x);
				vDelta.x = fDeltaM*(vI_Pos[1].x - vI_Pos[0].x);
				vDelta.y = fDeltaM*(vI_Pos[1].y - vI_Pos[0].y);
				vDelta.z = fDeltaM*(vI_Pos[1].z - vI_Pos[0].z);

				vPos.x=vI_Pos[0].x; vPos.y=vI_Pos[0].y; vPos.z=vI_Pos[0].z;
				
				while (fCurU <= vI[1].x)
				{
					vPos.x += vDelta.x;
					vPos.y += vDelta.y;
					vPos.z += vDelta.z;

					pFunc(fCurU, fCurV, vPos, vDelta, pTri, pRaster->nLMap);

					fCurU += pRaster->fDelta;
				};
			}
		}

		fCurV += pRaster->fDelta;
	};
}

void CRadiosity::Rasterize(u16 nMotif, RasterizeCallbackFunc pFunc, BOOL bShoot, LN_Light_t *pLight)
{
	u32 nTri;
	LN_Triangle_t *pTri;

	s32 i;
	CFVec3 vBase, vDir, vDelta, vNrml, vFilter, vOrigin;
	s32 min_x[(MAX_LIGHTMAP_SIZE+2)], max_x[(MAX_LIGHTMAP_SIZE+2)];
	s32 min_y, max_y, x, y, n;
	f32 fCoords[4][2];
	f32 fNrml[4][3];
	
	LN_RASTER_PRECALC_t *pRaster;

	for (nTri=0; nTri<m_nNumTri; nTri++)
	{
		// Update the progress bar
		if ( m_pDlg )
		{
			m_fUpdateCount += 1.f;
			if ( m_fUpdateCount > m_fUpdateInterval )
			{
				m_pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );
				m_fUpdateCount -= m_fUpdateInterval;
			}
		}

		// Check for abort
		if ( m_pDlg->m_bAbortCompile )
		{
			return;
		}

		pTri = &m_TriangleList[nTri];
		pRaster = &m_pRasterPreCalc[nTri];

		if (!pTri->pMaterial->pLightMaps[0]) { continue; }
		if (!m_pRasterPreCalc[nTri].bRenderTri) { continue; }
		
		LIGHTMAP_WIDTH = m_pLightMapList[pRaster->nLMap]->GetWidth();
		LIGHTMAP_HEIGHT = m_pLightMapList[pRaster->nLMap]->GetHeight();

		CFVec3 vPos;
		f32 fS, fT;
		for (i=0; i<3; i++)
		{
			vPos.Set(pTri->pVtx[i]->fPos[0], pTri->pVtx[i]->fPos[1], pTri->pVtx[i]->fPos[2]);
			if (pTri->pMaterial->bStaticUV)
			{
				vPos = pTri->pMaterial->MtxR.MultPoint(vPos);
			}
			vDelta = vPos - pTri->vMin;
			
			fS = vDelta.Dot( pTri->vLightMapMtx[0] ) + pTri->fLightMapX + 0.5f;
			fT = vDelta.Dot( pTri->vLightMapMtx[1] ) + pTri->fLightMapY + 0.5f;
			
			if (fS > LIGHTMAP_WIDTH) { fS = LIGHTMAP_WIDTH - 0.5f; } 
			if (fS < 0.0f) { fS = 0.0f; }
			if (fT > LIGHTMAP_HEIGHT) { fT = LIGHTMAP_HEIGHT - 0.5f; }
			if (fT < 0.0f) { fT = 0.0f; }

			fCoords[i][0] = fS;
			fCoords[i][1] = fT;

			if (m_bInterpNrml)
			{
				fNrml[i][0] = pTri->pVtx[i]->fNrml[0];
				fNrml[i][1] = pTri->pVtx[i]->fNrml[1];
				fNrml[i][2] = pTri->pVtx[i]->fNrml[2];
			}
		}
		fCoords[i][0] = fCoords[0][0];
		fCoords[i][1] = fCoords[0][1];

		if (m_bInterpNrml)
		{
			fNrml[i][0] = fNrml[0][0];
			fNrml[i][1] = fNrml[0][1];
			fNrml[i][2] = fNrml[0][2];
		}

		min_y = LIGHTMAP_HEIGHT;
		max_y = 0;

		for (i=0; i<(s32)LIGHTMAP_HEIGHT; i++)
		{
			min_x[i] = LIGHTMAP_WIDTH;
			max_x[i] = 0;
		}

		for (i=0; i<3; i++)
		{
			f32 xf, yf, dx, dy, xstep, ystep, xfrac, yfrac;
			s32 xinc, yinc;

			xf = fCoords[i][0];
			yf = fCoords[i][1];

			dx = fCoords[i+1][0] - xf;
			dy = fCoords[i+1][1] - yf;

			if ( fabsf(dx) < 0.0000001f && fabsf(dy) < 0.0000001f )
			{
				continue;
			}

			x = (s32)xf;
			y = (s32)yf;

            if (y < min_y) { min_y = y; }
			if (y > max_y) { max_y = y; }

			if ( fabsf(dx) > fabsf(dy) )
			{
				if (dx > 0)
				{
					yfrac = yf + ((f32)floor(xf) - xf)*dy/dx;
					xinc = 1;
				}
				else if (dx < 0)
				{
					yfrac = yf + ((f32)floor(xf) + 1 - xf)*dy/dx;
					xinc = -1;
				}
				else
				{
					yfrac = yf;
					xinc = 0;
				}
				//step in y direction / x
				if (dx) { ystep = dy / fabsf(dx); }
				else { ystep = 0; }

				while (1)
				{
					n = y * LIGHTMAP_WIDTH + x;

					if (x < min_x[y]) { min_x[y] = x; }
					if (x > max_x[y]) { max_x[y] = x; }

					if (x == (s32)fCoords[i+1][0] || x < 0 || x > (s32)LIGHTMAP_WIDTH) //reached end of scaneline.
					{
						break; //reached end, exit.
					}
					yfrac += ystep; //what is y value for this x?

					if (dy > 0)
					{
						if (yfrac > (f32)y + 1.0f)
						{
							y += 1; //inc y
							n = y * LIGHTMAP_WIDTH + x;

							if (x < min_x[y]) { min_x[y] = x; }
							if (x > max_x[y]) { max_x[y] = x; }
						}
					}
					else
					{
						if (yfrac < (f32)y)
						{
							y -= 1; //dec y
							n = y * LIGHTMAP_WIDTH + x;

							if (x < min_x[y]) { min_x[y] = x; }
							if (x > max_x[y]) { max_x[y] = x; }
						}
					}
					x += xinc;
				};
			}
			else
			{
				if (dy > 0)
				{
					xfrac = xf + ((f32)floor(yf) - yf)*dx/dy;
					yinc = 1;
				}
				else if (dy < 0)
				{
					xfrac = xf + ((f32)floor(yf) + 1 - yf)*dx/dy;
					yinc = -1;
				}
				else
				{
					xfrac = xf;
					yinc = 0;
				}
				//step in x direction / y
				if (dy) { xstep = dx / fabsf(dy); }
				else { xstep = 0; }

				while (1)
				{
					n = y * LIGHTMAP_WIDTH + x;

					if (x < min_x[y]) { min_x[y] = x; }
					if (x > max_x[y]) { max_x[y] = x; }

					if (y == (s32)fCoords[i+1][1] || y < 0 || y > (s32)LIGHTMAP_HEIGHT) //reached end of scaneline.
					{
						break; //reached end, exit.
					}
					xfrac += xstep; //what is y value for this x?

					if (dx > 0)
					{
						if (xfrac > (f32)x + 1.0f)
						{
							x += 1; //inc y
							n = y * LIGHTMAP_WIDTH + x;

							if (x < min_x[y]) { min_x[y] = x; }
							if (x > max_x[y]) { max_x[y] = x; }
						}
					}
					else
					{
						if (xfrac < (f32)x)
						{
							x -= 1; //dec y
							n = y * LIGHTMAP_WIDTH + x;

							if (x < min_x[y]) { min_x[y] = x; }
							if (x > max_x[y]) { max_x[y] = x; }
						}
					}
					y += yinc;
				};
			}
		} //end for
		f32 fScale;
		f32 fR;
		CFVec3 vVec0, vVec1, vUnit;
		s32 mny, mxy, mnx, mxx;

		vDelta = pTri->vLightMapVecs[0] + pTri->vLightMapVecs[1];
		
		mny = min_y; mxy = max_y;

		if (mny < 0) { mny = 0; }
		if (mxy > (s32)LIGHTMAP_HEIGHT-1) { mxy = (s32)LIGHTMAP_HEIGHT-1; }

		for (y=mny; y<=mxy; y++)
		{
			mnx = min_x[y];
			mxx = max_x[y];

			if (mnx < 0) { mnx = 0; }
			if (mxx > (s32)LIGHTMAP_WIDTH-1) { mxx = (s32)LIGHTMAP_WIDTH-1; }
            
			for (x=mnx; x<=mxx; x++)
			{
				fScale = (f32)( (f32)x - pTri->fLightMapX );
				vBase.x = pTri->vLightMapOrigin.x + fScale*pTri->vLightMapVecs[0].x;
				vBase.y = pTri->vLightMapOrigin.y + fScale*pTri->vLightMapVecs[0].y;
				vBase.z = pTri->vLightMapOrigin.z + fScale*pTri->vLightMapVecs[0].z;

				fScale = (f32)( (f32)y - pTri->fLightMapY );
				vBase.x += (fScale*pTri->vLightMapVecs[1].x);
				vBase.y += (fScale*pTri->vLightMapVecs[1].y);
				vBase.z += (fScale*pTri->vLightMapVecs[1].z);

				if (pTri->pMaterial->bStaticUV)
				{
					vBase = pTri->pMaterial->MtxF.MultPoint(vBase);
				}
				
				if (m_bInterpNrml)
				{
					//calculate my barycentric coordinates for my pos (vBase)
					vVec0.x = vBase.x - pTri->pVtx[0]->fPos[0];
					vVec0.y = vBase.y - pTri->pVtx[0]->fPos[1];
					vVec0.z = vBase.z - pTri->pVtx[0]->fPos[2];

					vVec1.x = pTri->pVtx[2]->fPos[0] - pTri->pVtx[0]->fPos[0];
					vVec1.y = pTri->pVtx[2]->fPos[1] - pTri->pVtx[0]->fPos[1];
					vVec1.z = pTri->pVtx[2]->fPos[2] - pTri->pVtx[0]->fPos[2];

					vUnit = vVec0.Cross(vVec1); vUnit = vUnit * pTri->fMag;
					fS = vUnit.Dot( pTri->vNrml );
					FMATH_CLAMP(fS, 0, 1);

					vVec1.x = pTri->pVtx[1]->fPos[0] - pTri->pVtx[0]->fPos[0];
					vVec1.y = pTri->pVtx[1]->fPos[1] - pTri->pVtx[0]->fPos[1];
					vVec1.z = pTri->pVtx[1]->fPos[2] - pTri->pVtx[0]->fPos[2];

					vUnit = vVec1.Cross(vVec0); vUnit = vUnit * pTri->fMag;
					fT = vUnit.Dot( pTri->vNrml );
					FMATH_CLAMP(fT, 0, 1);

					fR = 1.0f - (fS + fT);
					FMATH_CLAMP(fR, 0, 1);

					//calculate normal using barycentric coordinates (fS, fT, fR)
					vNrml.x = pTri->pVtx[0]->fNrml[0]*fR + pTri->pVtx[1]->fNrml[0]*fS + pTri->pVtx[2]->fNrml[0]*fT;
					vNrml.y = pTri->pVtx[0]->fNrml[1]*fR + pTri->pVtx[1]->fNrml[1]*fS + pTri->pVtx[2]->fNrml[1]*fT;
					vNrml.z = pTri->pVtx[0]->fNrml[2]*fR + pTri->pVtx[1]->fNrml[2]*fS + pTri->pVtx[2]->fNrml[2]*fT;
				}

				if (m_bInterpNrml)
				{
					pFunc((f32)x, (f32)y, vBase, vNrml, pTri, pRaster->nLMap);
				}
				else
				{
					pFunc((f32)x, (f32)y, vBase, vDelta, pTri, pRaster->nLMap);
				}
			}
		}
	}
}

void CRadiosity::Rasterize_SubSample(u16 nMotif, RasterizeCallbackFunc pFunc, BOOL bShoot, LN_Light_t *pLight, u8 nSubSample)
{
	u32 nTri;
	LN_Triangle_t *pTri;

	s32 i;
	CFVec3 vBase, vDir, vDelta, vNrml, vFilter, vOrigin;
	s32 min_x[(MAX_LIGHTMAP_SIZE+2)*64], max_x[(MAX_LIGHTMAP_SIZE+2)*64];
	s32 min_y, max_y, x, y, n;
	f32 fCoords[4][2];
	f32 fNrml[4][3];
	//u8 polygonedges[ (MAX_LIGHTMAP_SIZE+1)*(MAX_LIGHTMAP_SIZE+1)/8 ];

	f32 fOOSubSample, fOOSubSample2;
	fOOSubSample = 1.0f/(f32)(nSubSample);
	fOOSubSample2 = fOOSubSample*fOOSubSample;
	
	LN_RASTER_PRECALC_t *pRaster;

	for (nTri=0; nTri<m_nNumTri; nTri++)
	{
		// Update the progress bar
		if ( m_pDlg )
		{
			m_fUpdateCount += 1.f;
			if ( m_fUpdateCount > m_fUpdateInterval )
			{
				m_pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );
				m_fUpdateCount -= m_fUpdateInterval;
			}
		}

		// Check for abort
		if ( m_pDlg->m_bAbortCompile )
		{
			return;
		}

		pTri = &m_TriangleList[nTri];
		pRaster = &m_pRasterPreCalc[nTri];

		if (!pTri->pMaterial->pLightMaps[0]) { continue; }
		if (!m_pRasterPreCalc[nTri].bRenderTri) { continue; }

		LIGHTMAP_WIDTH = m_pLightMapList[pRaster->nLMap]->GetWidth();//pRaster->pLightMap->GetWidth();
		LIGHTMAP_HEIGHT = m_pLightMapList[pRaster->nLMap]->GetHeight();

		f32 fS, fT;
		for (i=0; i<3; i++)
		{
			vDelta.x = pTri->pVtx[i]->fPos[0] - pTri->vMin.x;
			vDelta.y = pTri->pVtx[i]->fPos[1] - pTri->vMin.y;
			vDelta.z = pTri->pVtx[i]->fPos[2] - pTri->vMin.z;

			fS = vDelta.Dot( pTri->vLightMapMtx[0] ) + pTri->fLightMapX + 0.5f;
			fT = vDelta.Dot( pTri->vLightMapMtx[1] ) + pTri->fLightMapY + 0.5f;

			if (fS > LIGHTMAP_WIDTH) { fS = LIGHTMAP_WIDTH - 0.5f; } 
			if (fS < 0.0f) { fS = 0.0f; }
			if (fT > LIGHTMAP_HEIGHT) { fT = LIGHTMAP_HEIGHT - 0.5f; }
			if (fT < 0.0f) { fT = 0.0f; }

			fCoords[i][0] = fS*nSubSample;
			fCoords[i][1] = fT*nSubSample;

			if (m_bInterpNrml)
			{
				fNrml[i][0] = pTri->pVtx[i]->fNrml[0];
				fNrml[i][1] = pTri->pVtx[i]->fNrml[1];
				fNrml[i][2] = pTri->pVtx[i]->fNrml[2];
			}
		}
		fCoords[i][0] = fCoords[0][0];
		fCoords[i][1] = fCoords[0][1];

		if (m_bInterpNrml)
		{
			fNrml[i][0] = fNrml[0][0];
			fNrml[i][1] = fNrml[0][1];
			fNrml[i][2] = fNrml[0][2];
		}

		min_y = LIGHTMAP_HEIGHT*nSubSample;
		max_y = 0;

		for (i=0; i<(s32)LIGHTMAP_HEIGHT*nSubSample; i++)
		{
			min_x[i] = LIGHTMAP_WIDTH*nSubSample;
			max_x[i] = 0;
		}
		//memset(polygonedges, 0, sizeof(polygonedges));

		for (i=0; i<3; i++)
		{
			f32 xf, yf, dx, dy, xstep, ystep, xfrac, yfrac;
			s32 xinc, yinc;

			xf = fCoords[i][0];
			yf = fCoords[i][1];

			dx = fCoords[i+1][0] - xf;
			dy = fCoords[i+1][1] - yf;

			x = (s32)xf;
			y = (s32)yf;

            if (y < min_y) { min_y = y; }
			if (y > max_y) { max_y = y; }

			if ( fabsf(dx) > fabsf(dy) )
			{
				if (dx > 0)
				{
					yfrac = yf + ((f32)floor(xf) - xf)*dy/dx;
					xinc = 1;
				}
				else if (dx < 0)
				{
					yfrac = yf + ((f32)floor(xf) + 1 - xf)*dy/dx;
					xinc = -1;
				}
				else
				{
					yfrac = yf;
					xinc = 0;
				}
				//step in y direction / x
				if (dx) { ystep = dy / fabsf(dx); }
				else { ystep = 0; }

				while (1)
				{
					n = y * LIGHTMAP_WIDTH + x;
					//polygonedges[n >> 3] |= 1 << (n&7);

					if (x < min_x[y]) { min_x[y] = x; }
					if (x > max_x[y]) { max_x[y] = x; }

					if (x == (s32)fCoords[i+1][0]) //reached end of scaneline.
					{
						break; //reached end, exit.
					}
					yfrac += ystep; //what is y value for this x?

					if (dy > 0)
					{
						if (yfrac > (f32)y + 1.0f)
						{
							y += 1; //inc y
							n = y * LIGHTMAP_WIDTH + x;

                            //polygonedges[n >> 3] |= 1 << (n&7);

							if (x < min_x[y]) { min_x[y] = x; }
							if (x > max_x[y]) { max_x[y] = x; }
						}
					}
					else
					{
						if (yfrac < (f32)y)
						{
							y -= 1; //dec y
							n = y * LIGHTMAP_WIDTH + x;

							//polygonedges[n >> 3] |= 1 << (n&7);

							if (x < min_x[y]) { min_x[y] = x; }
							if (x > max_x[y]) { max_x[y] = x; }
						}
					}
					x += xinc;
				};
			}
			else
			{
				if (dy > 0)
				{
					xfrac = xf + ((f32)floor(yf) - yf)*dx/dy;
					yinc = 1;
				}
				else if (dy < 0)
				{
					xfrac = xf + ((f32)floor(yf) + 1 - yf)*dx/dy;
					yinc = -1;
				}
				else
				{
					xfrac = xf;
					yinc = 0;
				}
				//step in x direction / y
				if (dy) { xstep = dx / fabsf(dy); }
				else { xstep = 0; }

				while (1)
				{
					n = y * LIGHTMAP_WIDTH + x;
					//polygonedges[n >> 3] |= 1 << (n&7);

					if (x < min_x[y]) { min_x[y] = x; }
					if (x > max_x[y]) { max_x[y] = x; }

					if (y == (s32)fCoords[i+1][1]) //reached end of scaneline.
					{
						break; //reached end, exit.
					}
					xfrac += xstep; //what is y value for this x?

					if (dx > 0)
					{
						if (xfrac > (f32)x + 1.0f)
						{
							x += 1; //inc y
							n = y * LIGHTMAP_WIDTH + x;

							//polygonedges[n >> 3] |= 1 << (n&7);

							if (x < min_x[y]) { min_x[y] = x; }
							if (x > max_x[y]) { max_x[y] = x; }
						}
					}
					else
					{
						if (xfrac < (f32)x)
						{
							x -= 1; //dec y
							n = y * LIGHTMAP_WIDTH + x;

							//polygonedges[n >> 3] |= 1 << (n&7);

							if (x < min_x[y]) { min_x[y] = x; }
							if (x > max_x[y]) { max_x[y] = x; }
						}
					}
					y += yinc;
				};
			}
		} //end for
		f32 fScale;
		f32 fR;
		CFVec3 vVec0, vVec1, vUnit;

		vDelta = pTri->vLightMapVecs[0] + pTri->vLightMapVecs[1];
		vDelta = vDelta * fOOSubSample2;

		for (y=min_y; y<=max_y; y++)
		{
			for (x=min_x[y]; x<=max_x[y]; x++)
			{
				fScale = (f32)( (f32)x*fOOSubSample - pTri->fLightMapX );
				vBase.x = pTri->vLightMapOrigin.x + fScale*pTri->vLightMapVecs[0].x;
				vBase.y = pTri->vLightMapOrigin.y + fScale*pTri->vLightMapVecs[0].y;
				vBase.z = pTri->vLightMapOrigin.z + fScale*pTri->vLightMapVecs[0].z;

				fScale = (f32)( (f32)y*fOOSubSample - pTri->fLightMapY );
				vBase.x += (fScale*pTri->vLightMapVecs[1].x);
				vBase.y += (fScale*pTri->vLightMapVecs[1].y);
				vBase.z += (fScale*pTri->vLightMapVecs[1].z);

				if (m_bInterpNrml || 1)
				{
					//calculate my barycentric coordinates for my pos (vBase)
					vVec0.x = vBase.x - pTri->pVtx[0]->fPos[0];
					vVec0.y = vBase.y - pTri->pVtx[0]->fPos[1];
					vVec0.z = vBase.z - pTri->pVtx[0]->fPos[2];

					vVec1.x = pTri->pVtx[2]->fPos[0] - pTri->pVtx[0]->fPos[0];
					vVec1.y = pTri->pVtx[2]->fPos[1] - pTri->pVtx[0]->fPos[1];
					vVec1.z = pTri->pVtx[2]->fPos[2] - pTri->pVtx[0]->fPos[2];

					vUnit = vVec0.Cross(vVec1); vUnit = vUnit * pTri->fMag;
					fS = vUnit.Dot( pTri->vNrml );

					vVec1.x = pTri->pVtx[1]->fPos[0] - pTri->pVtx[0]->fPos[0];
					vVec1.y = pTri->pVtx[1]->fPos[1] - pTri->pVtx[0]->fPos[1];
					vVec1.z = pTri->pVtx[1]->fPos[2] - pTri->pVtx[0]->fPos[2];

					vUnit = vVec1.Cross(vVec0); vUnit = vUnit * pTri->fMag;
					fT = vUnit.Dot( pTri->vNrml );

					if (fS < 0.0f) fS = 0.0f;
					if (fS > 1.0f) fS = 1.0f;

					if (fT < 0.0f) fT = 0.0f;
					if (fT > 1.0f) fT = 1.0f;

					fR = 1.0f - (fS + fT);

					//calculate normal using barycentric coordinates (fS, fT, fR)
					vNrml.x = pTri->pVtx[0]->fNrml[0]*fR + pTri->pVtx[1]->fNrml[0]*fS + pTri->pVtx[2]->fNrml[0]*fT;
					vNrml.y = pTri->pVtx[0]->fNrml[1]*fR + pTri->pVtx[1]->fNrml[1]*fS + pTri->pVtx[2]->fNrml[1]*fT;
					vNrml.z = pTri->pVtx[0]->fNrml[2]*fR + pTri->pVtx[1]->fNrml[2]*fS + pTri->pVtx[2]->fNrml[2]*fT;
				}

				//vNrml = pTri->vNrml;

				_vTmpNrml = -vNrml;
				if (m_bInterpNrml)
				{
					pFunc((f32)x*fOOSubSample, (f32)y*fOOSubSample, vBase, vNrml, pTri, pRaster->nLMap);
				}
				else
				{
					pFunc((f32)x*fOOSubSample, (f32)y*fOOSubSample, vBase, vDelta, pTri, pRaster->nLMap);
					//if ( (s32)((f32)x*fOOSubSample+0.5f) > (s32)((f32)x*fOOSubSample) || (s32)((f32)y*fOOSubSample+0.5f) > (s32)((f32)y*fOOSubSample) )
					//{
					//	pFunc((f32)x*fOOSubSample+0.5f, (f32)y*fOOSubSample+0.5f, vBase, vDelta, pTri, pRaster->nLMap);
					//}
				}
			}
		}
	}
}

void CRadiosity::RasterizeMask(RasterizeCallbackFuncMask pFunc)
{
	f32 fM;
	u32 nTri, i;
	f32 fCurU, fCurV;
	f32 fCurMaskU, fCurMaskV;
	f32 fEndU;
	f32 fDeltaM;

	u8 nIdx=0;
	CFVec3 vDelta, vPos;
	CFVec2 vI[2]; //intersections.
	CFVec2 vIMask[2]; //intersections.
	CFVec3 vI_Pos[3];
	CFVec2 v2Tmp;
	CFVec3 v3Tmp;

	LN_Triangle_t *pTri;
	LN_RASTER_PRECALC_t *pRaster;
	LN_RASTER_PRECALC_t *pRasterMask;
	for (nTri=0; nTri<m_nNumTri; nTri++)
	{
		pTri = &m_TriangleList[nTri];
		pRaster = &m_pRasterPreCalc[nTri];
		pRasterMask = &m_pRasterPreCalcMask[nTri];
		
		if ( pRaster->bRenderTri && pRasterMask->bRenderTri )
		{
			fCurV = pRaster->fMinV;

			nIdx=0;
			while (fCurV <= pRaster->fMaxV)
			{
				nIdx=0;
				for (i=0; i<3; i++)
				{	
					if (fCurV >= pRaster->aEdges[i].fMinV && fCurV <= pRaster->aEdges[i].fMaxV && (pRaster->aEdges[i].fMinV != pRaster->aEdges[i].fMaxV)) //horizontal ray intersects edge.
					{
						fM = (fCurV - pRaster->aEdges[i].vA.y)*(pRaster->aEdges[i].fInvOffsY);
						if (fM >= 0.0f && fM <= 1.0f)
						{
							fCurU = pRaster->aEdges[i].vA.x + fM*(pRaster->aEdges[i].vOffs.x);
							vI[nIdx].x = fCurU; vI[nIdx].y = fCurV;

							fCurMaskU = pRasterMask->aEdges[i].vA.x + fM*(pRasterMask->aEdges[i].vOffs.x);
							fCurMaskV = pRasterMask->aEdges[i].vA.y + fM*(pRasterMask->aEdges[i].vOffs.y);
							vIMask[nIdx].x = fCurMaskU; vIMask[nIdx].y = fCurMaskV;

							vI_Pos[nIdx].x = pRaster->aEdges[i].fPosA[0] + fM*(pRaster->aEdges[i].fPosB[0] - pRaster->aEdges[i].fPosA[0]);
							vI_Pos[nIdx].y = pRaster->aEdges[i].fPosA[1] + fM*(pRaster->aEdges[i].fPosB[1] - pRaster->aEdges[i].fPosA[1]);
							vI_Pos[nIdx].z = pRaster->aEdges[i].fPosA[2] + fM*(pRaster->aEdges[i].fPosB[2] - pRaster->aEdges[i].fPosA[2]);
							nIdx++;
							if (nIdx == 2)
							{
								break;
							}
						}
					}
				}

				if (nIdx == 2)
				{
					if (vI[0].x > vI[1].x)
					{
						v2Tmp = vI[0];
						vI[0] = vI[1];
						vI[1] = v2Tmp;

						v2Tmp = vIMask[0];
						vIMask[0] = vIMask[1];
						vIMask[1] = v2Tmp;
		
						v3Tmp = vI_Pos[0];
						vI_Pos[0] = vI_Pos[1];
						vI_Pos[1] = v3Tmp;
					}

					if (vI[0].x != vI[1].x)
					{
						fCurU = vI[0].x;
						fCurMaskU = vIMask[0].x;
						fEndU = vI[1].x;

						//Make sure my scalines are not too short, round <- ..... ->
						//fCurU = (f32)( (s32)(fCurU*pRaster->fWm1 - 0.5f) );
						//fCurU = fCurU * pRaster->fOOW;
						if (fCurU < 0.0f) fCurU = 0.0f;

						//fCurMaskU = (f32)( (s32)(fCurMaskU*pRasterMask->fWm1 - 0.5f) );
						//fCurMaskU = fCurMaskU * pRasterMask->fOOW;
						
						//fEndU = (f32)( (s32)(fEndU*pRaster->fWm1 + 0.5f) );
						//fEndU = fEndU * pRaster->fOOW;
						if (fEndU > 1.0f) fEndU = 1.0f;

						fDeltaM = (pRaster->fDelta)/(vI[1].x - vI[0].x);
						vDelta.x = fDeltaM*(vI_Pos[1].x - vI_Pos[0].x);
						vDelta.y = fDeltaM*(vI_Pos[1].y - vI_Pos[0].y);
						vDelta.z = fDeltaM*(vI_Pos[1].z - vI_Pos[0].z);

						vPos.x=vI_Pos[0].x; vPos.y=vI_Pos[0].y; vPos.z=vI_Pos[0].z;
						
						while (fCurU <= vI[1].x)
						{
							vPos.x += vDelta.x;
							vPos.y += vDelta.y;
							vPos.z += vDelta.z;

							pFunc(fCurU, fCurV, fCurMaskU, fCurMaskV, vPos, vDelta, pTri, pRaster->nLMap);

							fCurU += pRaster->fDelta;
							fCurMaskU += pRasterMask->fDelta;
						};
					}
				}

				fCurV += pRaster->fDelta;
			};
		}
	}
}

void CRadiosity::Rasterize_2(u16 nMotif, RasterizeCallbackFunc_2 pFunc, BOOL bShoot)
{
	f32 fM;
	u32 nTri, i;
	f32 fCurU, fCurV;
	f32 fEndU;
	f32 fDeltaM;

	u8 nIdx=0;
	CFVec3 vDelta, vPos;
	CFVec2 vI[2]; //intersections.
	CFVec3 vI_Pos[3];
	CFVec2 v2Tmp;
	CFVec3 v3Tmp;

	LN_Triangle_t *pTri;
	LN_RASTER_PRECALC_t *pRaster;
	for (nTri=0; nTri<m_nNumTri; nTri++)
	{
		pTri = &m_TriangleList[nTri];
		pRaster = &m_pRasterPreCalc[nTri];
		
		if ( pRaster->bRenderTri )
		{
			fCurV = pRaster->fMinV;
			nIdx=0;
			while (fCurV <= pRaster->fMaxV)
			{
				nIdx=0;
				for (i=0; i<3; i++)
				{	
					if (fCurV >= pRaster->aEdges[i].fMinV && fCurV <= pRaster->aEdges[i].fMaxV && (pRaster->aEdges[i].fMinV != pRaster->aEdges[i].fMaxV)) //horizontal ray intersects edge.
					{
						fM = (fCurV - pRaster->aEdges[i].vA.y)*(pRaster->aEdges[i].fInvOffsY);
						if (fM >= 0.0f && fM <= 1.0f)
						{
							fCurU = pRaster->aEdges[i].vA.x + fM*(pRaster->aEdges[i].vOffs.x);

							vI[nIdx].x = fCurU; vI[nIdx].y = fCurV;
							vI_Pos[nIdx].x = pRaster->aEdges[i].fPosA[0] + fM*(pRaster->aEdges[i].fPosB[0] - pRaster->aEdges[i].fPosA[0]);
							vI_Pos[nIdx].y = pRaster->aEdges[i].fPosA[1] + fM*(pRaster->aEdges[i].fPosB[1] - pRaster->aEdges[i].fPosA[1]);
							vI_Pos[nIdx].z = pRaster->aEdges[i].fPosA[2] + fM*(pRaster->aEdges[i].fPosB[2] - pRaster->aEdges[i].fPosA[2]);
							nIdx++;
							if (nIdx == 2)
							{
								break;
							}
						}
					}
				}

				if (nIdx == 2)
				{
					if (vI[0].x > vI[1].x)
					{
						v2Tmp = vI[0];
						vI[0] = vI[1];
						vI[1] = v2Tmp;
		
						v3Tmp = vI_Pos[0];
						vI_Pos[0] = vI_Pos[1];
						vI_Pos[1] = v3Tmp;
					}

					if (vI[0].x != vI[1].x)
					{
						fCurU = vI[0].x;
						fEndU = vI[1].x;

						//Make sure my scalines are not too short, round <- ..... ->
						
						fDeltaM = (pRaster->fDelta)/(vI[1].x - vI[0].x) * 2.0f;
						vDelta.x = fDeltaM*(vI_Pos[1].x - vI_Pos[0].x);
						vDelta.y = fDeltaM*(vI_Pos[1].y - vI_Pos[0].y);
						vDelta.z = fDeltaM*(vI_Pos[1].z - vI_Pos[0].z);

						vPos.x=vI_Pos[0].x; vPos.y=vI_Pos[0].y; vPos.z=vI_Pos[0].z;
						
						while (fCurU <= vI[1].x)
						{
							vPos.x += vDelta.x;
							vPos.y += vDelta.y;
							vPos.z += vDelta.z;

							pFunc(fCurU, fCurV, pRaster->fDelta, vPos, vDelta, pTri, pRaster->nLMap);

							fCurU += pRaster->fDelta*2.0f;
						};
					}
				}

				fCurV += pRaster->fDelta*2.0f;
			};
		}
	}
}

void CRadiosity::Rasterize_4(u16 nMotif, RasterizeCallbackFunc_2 pFunc, BOOL bShoot)
{
	f32 fM;
	u32 nTri, i;
	f32 fCurU, fCurV;
	f32 fEndU;
	f32 fDeltaM;

	u8 nIdx=0;
	CFVec3 vDelta, vPos;
	CFVec2 vI[2]; //intersections.
	CFVec3 vI_Pos[3];
	CFVec2 v2Tmp;
	CFVec3 v3Tmp;

	LN_Triangle_t *pTri;
	LN_RASTER_PRECALC_t *pRaster;
	for (nTri=0; nTri<m_nNumTri; nTri++)
	{
		pTri = &m_TriangleList[nTri];
		pRaster = &m_pRasterPreCalc[nTri];
		
		if ( pRaster->bRenderTri )
		{
			fCurV = pRaster->fMinV;
			nIdx=0;
			while (fCurV <= pRaster->fMaxV)
			{
				nIdx=0;
				for (i=0; i<3; i++)
				{	
					if (fCurV >= pRaster->aEdges[i].fMinV && fCurV <= pRaster->aEdges[i].fMaxV && (pRaster->aEdges[i].fMinV != pRaster->aEdges[i].fMaxV)) //horizontal ray intersects edge.
					{
						fM = (fCurV - pRaster->aEdges[i].vA.y)*(pRaster->aEdges[i].fInvOffsY);
						if (fM >= 0.0f && fM <= 1.0f)
						{
							fCurU = pRaster->aEdges[i].vA.x + fM*(pRaster->aEdges[i].vOffs.x);

							vI[nIdx].x = fCurU; vI[nIdx].y = fCurV;
							vI_Pos[nIdx].x = pRaster->aEdges[i].fPosA[0] + fM*(pRaster->aEdges[i].fPosB[0] - pRaster->aEdges[i].fPosA[0]);
							vI_Pos[nIdx].y = pRaster->aEdges[i].fPosA[1] + fM*(pRaster->aEdges[i].fPosB[1] - pRaster->aEdges[i].fPosA[1]);
							vI_Pos[nIdx].z = pRaster->aEdges[i].fPosA[2] + fM*(pRaster->aEdges[i].fPosB[2] - pRaster->aEdges[i].fPosA[2]);
							nIdx++;
							if (nIdx == 2)
							{
								break;
							}
						}
					}
				}

				if (nIdx == 2)
				{
					if (vI[0].x > vI[1].x)
					{
						v2Tmp = vI[0];
						vI[0] = vI[1];
						vI[1] = v2Tmp;
		
						v3Tmp = vI_Pos[0];
						vI_Pos[0] = vI_Pos[1];
						vI_Pos[1] = v3Tmp;
					}

					if (vI[0].x != vI[1].x)
					{
						fCurU = vI[0].x;
						fEndU = vI[1].x;

						//Make sure my scalines are not too short, round <- ..... ->
						
						fDeltaM = (pRaster->fDelta)/(vI[1].x - vI[0].x) * 4.0f;
						vDelta.x = fDeltaM*(vI_Pos[1].x - vI_Pos[0].x);
						vDelta.y = fDeltaM*(vI_Pos[1].y - vI_Pos[0].y);
						vDelta.z = fDeltaM*(vI_Pos[1].z - vI_Pos[0].z);

						vPos.x=vI_Pos[0].x; vPos.y=vI_Pos[0].y; vPos.z=vI_Pos[0].z;
						
						while (fCurU <= vI[1].x)
						{
							vPos.x += vDelta.x;
							vPos.y += vDelta.y;
							vPos.z += vDelta.z;

							pFunc(fCurU, fCurV, pRaster->fDelta, vPos, vDelta, pTri, pRaster->nLMap);

							fCurU += pRaster->fDelta*4.0f;
						};
					}
				}

				fCurV += pRaster->fDelta*4.0f;
			};
		}
	}
}
