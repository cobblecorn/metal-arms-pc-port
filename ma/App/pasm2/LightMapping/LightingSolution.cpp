#include "StdAfx.h"
#include "..\Pasm.h"
#include "..\CompileDlg.h"
#include "LightingSolution.h"

#include <math.h>
#include <Mmsystem.h>

u8 CLightingSolution::LN_Triangle_t::nEdge;
u8 CLightingSolution::LN_Triangle_t::nA;
u8 CLightingSolution::LN_Triangle_t::nB;
f32 CLightingSolution::LN_Triangle_t::fDot[3];

f32 _fSpace=0.04f;

CLightingSolution::CLightingSolution(void)
{
	m_nNumLightMaps = 0;
	m_nNumZMask = 0;
	m_nNumTri = 0;
	m_nNumVtx = 0;
	m_nNumLights = 0;
	m_nNumVtxGroup = 0;
	m_nNumTriGroup = 0;
	m_nNumVolumes = 0;
	m_nNumMaterials = 0;
	m_fPackQuality = 10.0f;

	m_nNumZMask = 0;
	m_nNumEmissive = 0;

	m_nCurVolume = -1;
	m_anVisited = NULL;
	m_Clusters = NULL;
//	m_pWinPrintf = NULL;

	m_TriangleList = NULL;
	m_VertexList = NULL;

	m_pakDOPPool = NULL;
	m_nSkyLightingQuality = 0;

	memset(m_VolumeList, 0, LS_MAX_VOLUMES*sizeof(LN_Volume_t));

	u32 i;
	for (i=0; i<LS_MAX_VOLUMES; i++)
	{
		m_nVolumeMtlIdx[i] = -1;
	}

	for (i=0; i<LS_MAX_MOTIFPERVOLUME; i++)
	{
		m_Clusters_Motif[i] = NULL;
	}
}

CLightingSolution::~CLightingSolution(void)
{
	if (m_anVisited)
	{
		delete [] m_anVisited;
		m_anVisited = NULL;
	}
	if (m_Clusters)
	{
		delete [] m_Clusters;
		m_Clusters = NULL;
	}
	u32 i;
	for (i=0; i<LS_MAX_MOTIFPERVOLUME; i++)
	{
		if (m_Clusters_Motif[i])
		{
			//free(m_Clusters_Motif[i]);
			delete [] m_Clusters_Motif[i];
			m_Clusters_Motif[i] = NULL;
		}
	}
	for (i=0; i<m_nNumLightMaps; i++)
	{
		delete m_pLightMapList[i];
		m_pLightMapList[i] = NULL;
	}

	for (i=0; i<m_nNumZMask; i++)
	{
		delete m_pZMaskList[i];
		m_pZMaskList[i] = NULL;
	}
	for (i=0; i<m_nNumEmissive; i++)
	{
		delete m_pEmissiveList[i];
		m_pEmissiveList[i] = NULL;
	}

	if (m_TriangleList)
	{
		free(m_TriangleList);
		m_TriangleList = NULL;
	}
	if (m_VertexList)
	{
		free(m_VertexList);
		m_VertexList = NULL;
	}

	if ( m_pakDOPPool )
	{
		delete[] m_pakDOPPool;
		m_pakDOPPool = NULL;
	}
}

void CLightingSolution::DispatchWinMsg()
{
	MSG msg;
	BOOL bRet=TRUE;
	static volatile u32 nIter;
	nIter = 0;
	do
	{
		bRet = GetMessage( &msg, NULL, 0, 0 );
		nIter++;

		if (nIter > 2) break;

		if (bRet != -1)
		{
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		}
	} while (bRet != -1 && nIter < 3);
}

char *CLightingSolution::GetLightMapDirectory()
{
	static char _szDir[256];

	sprintf(_szDir, "%s\\LightMaps\\%s\\", CLightMapTex::GetLocalDir(), m_szLevelName);

	return (_szDir);
}

void CLightingSolution::SetDataSize(u32 nVtx, u32 nTri)
{
	m_TriangleList = (LN_Triangle_t *)malloc(sizeof(LN_Triangle_t)*nTri);
	m_VertexList   =   (LN_Vertex_t *)malloc(sizeof(LN_Vertex_t)*nVtx);
}

void CLightingSolution::ClampF1(f32 *pfClr)
{
	*pfClr = (*pfClr > 0.0f)?(*pfClr):(0.0f);
}

void CLightingSolution::ClampF2(f32 *pfClr)
{
	pfClr[0] = (pfClr[0] > 0.0f)?( (pfClr[0] < 1.0f)?(pfClr[0]):(1.0f) ):(0.0f);
	pfClr[1] = (pfClr[1] > 0.0f)?( (pfClr[1] < 1.0f)?(pfClr[1]):(1.0f) ):(0.0f);
}

void CLightingSolution::ClampF3(f32 *pfClr)
{
	pfClr[0] = (pfClr[0] > 0.0f)?( (pfClr[0] < 1.0f)?(pfClr[0]):(1.0f) ):(0.0f);
	pfClr[1] = (pfClr[1] > 0.0f)?( (pfClr[1] < 1.0f)?(pfClr[1]):(1.0f) ):(0.0f);
	pfClr[2] = (pfClr[2] > 0.0f)?( (pfClr[2] < 1.0f)?(pfClr[2]):(1.0f) ):(0.0f);
}

void CLightingSolution::ClampF4(f32 *pfClr)
{
	pfClr[0] = (pfClr[0] > 0.0f)?( (pfClr[0] < 1.0f)?(pfClr[0]):(1.0f) ):(0.0f);
	pfClr[1] = (pfClr[1] > 0.0f)?( (pfClr[1] < 1.0f)?(pfClr[1]):(1.0f) ):(0.0f);
	pfClr[2] = (pfClr[2] > 0.0f)?( (pfClr[2] < 1.0f)?(pfClr[2]):(1.0f) ):(0.0f);
	pfClr[3] = (pfClr[3] > 0.0f)?( (pfClr[3] < 1.0f)?(pfClr[3]):(1.0f) ):(0.0f);
}

u16 CLightingSolution::GetLightMapIdx(CLightMapTex *pLightMap)
{
	u32 i;
	for (i=0; i<m_nNumLightMaps; i++)
	{
		if (m_pLightMapList[i] == pLightMap) return i;
	}
	return 0;
}

void CLightingSolution::SetVolumeVtxGroup(u32 nVolume, u32 nGroup)
{
	m_nVolumeVtxGroup[nVolume] = nGroup;
}

void CLightingSolution::GetVolumeVtxGroup(u32 nVolume, u32 nGroup, u32& nStart, u32& nEnd)
{
	nGroup = m_nVolumeVtxGroup[nVolume];
	nStart = m_nVtxGroup[nGroup];
	if (nGroup < (u32)(m_nNumVtxGroup-1))
	{
		nEnd = m_nVtxGroup[nGroup+1] - 1;
	}
	else
	{
		nEnd = m_nNumVtx-1;
	}
}

CLightingSolution::LN_UV_Set *_pUVSet=NULL;

CLightingSolution::LN_UV_Set *CLightingSolution::GenerateUVs()
{
	/*
	BuildLMapUVs(0, m_nNumTri-1);
	PackUV( 0, 0, m_nNumTri-1, &m_VolumeList[0], FALSE, 0, FALSE );
	if (_pUVSet)
	{
		free(_pUVSet);
		_pUVSet = NULL;
	}
	_pUVSet = (LN_UV_Set *)malloc( sizeof(LN_UV_Set) * m_nNumVtx );

	u32 i;
	for (i=0; i<m_nNumVtx; i++)
	{
		_pUVSet[i].fU = m_VertexList[i].fBaseUV[0];
		_pUVSet[i].fV = m_VertexList[i].fBaseUV[1];
	}

	return (_pUVSet);
	*/
	return (NULL);
}

void CLightingSolution::FreeGeneratedUVs()
{
	free(_pUVSet);
	_pUVSet = NULL;
}

//This is virtual function for the specific lighting solution.
BOOL CLightingSolution::Build(f32 fAmbRed, f32 fAmbGreen, f32 fAmbBlue)
{
	return TRUE;
}

u32 CLightingSolution::CreateMaterial(u32 nVolume, f32 fRed, f32 fGreen, f32 fBlue, BOOL bStaticUV, BOOL bLightMap, BOOL bRayTest, CFMtx43 *pMtx)
{
	m_CurMaterial = &m_MaterialList[m_nNumMaterials++];

	m_CurMaterial->nNLightMaps = 0;
	m_CurMaterial->nNumMotif = 0;
	m_CurMaterial->pLightMaps[0] = NULL;
	m_CurMaterial->pMask = NULL;
	m_CurMaterial->pColor = NULL;

	m_CurMaterial->pZMask = NULL;
	m_CurMaterial->pEmissive = NULL;

	m_CurMaterial->bStaticUV = (bStaticUV || !bLightMap)?(TRUE):(FALSE);
	m_CurMaterial->bHasLightMap = bLightMap;
	m_CurMaterial->bRayTest = bRayTest;

	m_CurMaterial->fRed = fRed;
	m_CurMaterial->fGreen = fGreen;
	m_CurMaterial->fBlue = fBlue;

	m_CurMaterial->fAreaI = 0.0f;
	m_CurMaterial->fAreaR = 0.0f;
	m_CurMaterial->fAreaG = 0.0f;
	m_CurMaterial->fAreaB = 0.0f;

	if (m_nVolumeMtlIdx[nVolume] == -1)
	{
		m_nVolumeMtlIdx[nVolume] = m_nNumMaterials-1;
	}

	if ( pMtx )
	{
		// PASM does not support non-uniform scale, so we only need to check the front matrix for scale
		f32 fInvMtxScale = pMtx->m_vFront.Mag2();
		FASSERT( fmath_Abs( fInvMtxScale - pMtx->m_vUp.Mag2() ) < 0.001f );
		FASSERT( fmath_Abs( fInvMtxScale - pMtx->m_vRight.Mag2() ) < 0.001f );
		if ( fInvMtxScale )
		{
			// Set the inverse of the matrix for these polys
			m_CurMaterial->MtxR = *pMtx;
			m_CurMaterial->MtxR.Invert();

			// Unitize the Inverse matrix for use in transforming the normal
			fInvMtxScale = fmath_AcuInvSqrt( fInvMtxScale );
			m_CurMaterial->UnitMtxR = *pMtx;
			m_CurMaterial->UnitMtxR.m_vRight.Mul( fInvMtxScale );
			m_CurMaterial->UnitMtxR.m_vUp.Mul( fInvMtxScale );
			m_CurMaterial->UnitMtxR.m_vFront.Mul( fInvMtxScale );
			m_CurMaterial->UnitMtxR.Invert();

			// Copy the geo matrix
			m_CurMaterial->MtxF = *pMtx;
		}
		else
		{
			m_CurMaterial->MtxR.Identity();
			m_CurMaterial->UnitMtxR.Identity();
			m_CurMaterial->MtxF.Identity();
		}
	}
	else
	{
		m_CurMaterial->MtxR.Identity();
		m_CurMaterial->UnitMtxR.Identity();
		m_CurMaterial->MtxF.Identity();
	}
    
	return m_nNumMaterials-1;
}

void CLightingSolution::SetAreaLightData(f32 fIntensity, f32 fRed, f32 fGreen, f32 fBlue)
{
	m_CurMaterial->fAreaI = fIntensity;
	m_CurMaterial->fAreaR = fRed;
	m_CurMaterial->fAreaG = fGreen;
	m_CurMaterial->fAreaB = fBlue;
}

CLightMapTex *CLightingSolution::AddZMask(cchar *pszZMask)
{
	u32 i;
	for (i=0; i<m_nNumZMask; i++)
	{
		if ( fclib_stricmp(m_pZMaskList[i]->GetName(), pszZMask) == 0 )
		{
			return m_pZMaskList[i];
		}
	}
	m_pZMaskList[m_nNumZMask] = new CLightMapTex();
	m_pZMaskList[m_nNumZMask]->LoadFile(pszZMask);
	m_pZMaskList[m_nNumZMask]->SetName((char*)pszZMask);
	m_nNumZMask++;

	return m_pZMaskList[m_nNumZMask-1];
}

void CLightingSolution::SetCurMaterial_ZMask(cchar *pszZMask)
{
	m_CurMaterial->pZMask = AddZMask(pszZMask);
}

CLightMapTex *CLightingSolution::AddEmissive(cchar *pszEmissive)
{
	u32 i;
	for (i=0; i<m_nNumEmissive; i++)
	{
		if ( fclib_stricmp(m_pEmissiveList[i]->GetName(), pszEmissive) == 0 )
		{
			return m_pEmissiveList[i];
		}
	}
	m_pEmissiveList[m_nNumEmissive] = new CLightMapTex();
	m_pEmissiveList[m_nNumEmissive]->LoadFile(pszEmissive);
	m_pEmissiveList[m_nNumEmissive]->SetName((char*)pszEmissive);
    m_nNumEmissive++;

	return m_pEmissiveList[m_nNumEmissive-1];
}

void CLightingSolution::SetCurMaterial_Emissive(cchar *pszEmissive)
{
	m_CurMaterial->pEmissive = AddEmissive(pszEmissive);

	m_CurMaterial->fRed = 1.0f;
	m_CurMaterial->fGreen = 1.0f;
	m_CurMaterial->fBlue = 1.0f;
}

void CLightingSolution::SetCurMaterial_EmissiveClr(f32 fRed, f32 fGreen, f32 fBlue)
{	
	m_CurMaterial->fRed = fRed;
	m_CurMaterial->fGreen = fGreen;
	m_CurMaterial->fBlue = fBlue;
}

void CLightingSolution::SetCurMaterial_EmissiveAlphaMask(BOOL bAlphaMask)
{
	m_CurMaterial->bAlphaMask = bAlphaMask;
}

u32 CLightingSolution::GetMaterial_NumLightMaps(u32 nVolume, u32 nMaterial)
{
	u32 nRet=0;
	s16 nIdx = m_nVolumeMtlIdx[nVolume];
	if (nIdx > -1)
	{
		nRet = m_MaterialList[nIdx + nMaterial].nNLightMaps;
	}
	return (nRet);
}

char *CLightingSolution::GetMaterialLightMap(u32 nVolume, u32 nMaterial, u32 nLightMap)
{
	char *pszName=NULL;
	s16 nIdx = m_nVolumeMtlIdx[nVolume];
	if (nIdx > -1)
	{
		pszName = m_MaterialList[nIdx + nMaterial].pLightMaps[ nLightMap ]->GetName();
	}
	return (pszName);
}

u32 CLightingSolution::GetVtxUserData(u32 nVolume, u32 nMaterial, u32 nVtx)
{
	LN_Material_t *pMtl;
	pMtl = &m_MaterialList[ m_nVolumeMtlIdx[nVolume] + nMaterial ];

	u32 i, nTri, _nVtx, nCurTri=0;
	LN_Triangle_t *pTri;

	nTri = nVtx/3;
	_nVtx = nVtx%3;

	u32 nStart = m_VolumeList[nVolume].nStartIdx;
	u32 nEnd = m_VolumeList[nVolume].nEndIdx;

	if (nEnd == 0 && (nStart > 0 || nVolume == m_nNumVolumes-1))
	{
		nEnd = m_nNumTri-1;
	}
	if (nEnd > m_nNumTri-1) nEnd = 0;

	for (i=nStart; i<=nEnd; i++)
	{
		pTri = &m_TriangleList[i];
		if (pTri->pMaterial == pMtl)
		{
			if (nCurTri == nTri)
			{
				return pTri->pVtx[ _nVtx ]->nUserData;
			}
			nCurTri++;
		}
	}
	return 0;
}

void CLightingSolution::GetVtxClr(u32 nVolume, u32 nMaterial, u32 nVtx, f32& fRed, f32& fGreen, f32& fBlue, f32& fAlpha)
{
	LN_Material_t *pMtl;
	pMtl = &m_MaterialList[ m_nVolumeMtlIdx[nVolume] + nMaterial ];

	u32 i, nTri, _nVtx, nCurTri=0;
	LN_Triangle_t *pTri;

	nTri = nVtx/3;
	_nVtx = nVtx%3;

	u32 nStart = m_VolumeList[nVolume].nStartIdx;
	u32 nEnd = m_VolumeList[nVolume].nEndIdx;

	if (nEnd == 0 && (nStart > 0 || nVolume == m_nNumVolumes-1))
	{
		nEnd = m_nNumTri-1;
	}
	if (nEnd > m_nNumTri-1) nEnd = 0;

	for (i=nStart; i<=nEnd; i++)
	{
		pTri = &m_TriangleList[i];
		if (pTri->pMaterial == pMtl)
		{
			if (nCurTri == nTri)
			{
				fRed = pTri->pVtx[ _nVtx ]->fClr[0];
				fGreen = pTri->pVtx[ _nVtx ]->fClr[1];
				fBlue = pTri->pVtx[ _nVtx ]->fClr[2];
				fAlpha = pTri->pVtx[ _nVtx ]->fClr[3];

				return;
			}
			nCurTri++;
		}
	}
}

CLightingSolution::LN_MaterialExport_t *CLightingSolution::ExportMaterial(u32 nVolume, u32 nMaterial)
{
	u32 i, j, n, k, nUVSet;
	LN_MaterialExport_t *pExpMtl=NULL;
	LN_Material_t *pMtl;
	LN_Volume_t *pVolume = &m_VolumeList[nVolume];

	pExpMtl = (LN_MaterialExport_t *)malloc(sizeof(LN_MaterialExport_t));
	pMtl = &m_MaterialList[ m_nVolumeMtlIdx[nVolume] + nMaterial ];

	u32 nMtl=0;

	for (i=0; i<pMtl->nNLightMaps; i++)
	{
		if (pMtl->pLightMaps[i]->GetCheckResult()) 
		{ 
			nMtl++; 
		}
	}

	pExpMtl->nNumLightMaps = nMtl;//pMtl->nNLightMaps;
	if ( pExpMtl->nNumLightMaps == 0 )
	{
		return NULL;
	}

	pExpMtl->anLightMapIdx = (u16 *)malloc(2*nMtl);//pMtl->nNLightMaps);
	pExpMtl->anMotifList = (u32 *)malloc(4*nMtl);//pMtl->nNLightMaps);
	for (i=0; i<pMtl->nNLightMaps; i++)
	{
		if (i == 0)
		{
			pExpMtl->anLightMapIdx[i] = 0;
			pExpMtl->anMotifList[i] = 0;
			n = 1;
		}
		else
		{
			//pExpMtl->anMotifList[i] = pMtl->anMotifArray[i-1];

			if (pMtl->pLightMaps[i]->GetCheckResult())
			{
				pExpMtl->anMotifList[n] = pMtl->anMotifArray[i-1];
								
				for (j=0; j<pVolume->nNumLightMaps; j++)
				{
					if (pVolume->pLightMaps[j] == pMtl->pLightMaps[i])
					{
						pExpMtl->anLightMapIdx[n] = j;
						break;
					}
				}
				n++;
			}
		}
	}

	//add pVolume->nNumVtx
	pExpMtl->aUVSet = (LN_UVExport_t **)malloc(sizeof(LN_UVExport_t *)*nMtl);
	n = 0;
	for (i=0; i<pMtl->nNLightMaps; i++)
	{
		if (pMtl->pLightMaps[i]->GetCheckResult())
		{
			pExpMtl->aUVSet[n++] = (LN_UVExport_t *)malloc(sizeof(LN_UVExport_t)*pVolume->nNumVtx);
		}
	}

	if (!m_anVisited) 
	{
		m_anVisited = new u8[m_nNumVtx];
	}

	LN_Triangle_t *pTri;
	memset(m_anVisited, 0, m_nNumVtx);

	u32 nVBIdx, nVtxIdx, n2;

	pExpMtl->nVtx = 0;

	for (i=0; i<m_nNumTri; i++)
	{
		pTri = &m_TriangleList[i];
		if (pTri->pMaterial == pMtl)
		{
			//add vertices to this list.
			for (k=0; k<3; k++)
			{
				if (!m_anVisited[ pTri->nVtx[k] ])
				{
					m_anVisited[ pTri->nVtx[k] ] = 1;
					n = pExpMtl->nVtx;

					nVBIdx = GetVBIndex(pTri->nVtx[k], nVolume);
					nVtxIdx = GetVtxIndex(pTri->nVtx[k], nVBIdx, nVolume);

					f32 fOOWm1 = (f32)pTri->pMaterial->pLightMaps[0]->GetWidth();
					fOOWm1 = 1.0f / (fOOWm1 - 1.0f);
					f32 fOOHm1 = (f32)pTri->pMaterial->pLightMaps[0]->GetHeight();
					fOOHm1 = 1.0f / (fOOHm1 - 1.0f);

					n2 = 0;
					for (j=0; j<pMtl->nNLightMaps; j++)
					{
						if (pMtl->pLightMaps[j]->GetCheckResult())
						{
							nUVSet = FindUVSet(pVolume, pExpMtl->anMotifList[j]);
							if (j==0)
							{
								pExpMtl->aUVSet[0][n].fU = m_VertexList[pTri->nVtx[k]].fBaseUV[0];// - fOOWm1*0.5f;
								pExpMtl->aUVSet[0][n].fV = m_VertexList[pTri->nVtx[k]].fBaseUV[1];// - fOOHm1*0.5f;

								//pExpMtl->aUVSet[0][n].fU = m_VertexList[pTri->nVtx[k]].fOutUV[0];
								//pExpMtl->aUVSet[0][n].fV = m_VertexList[pTri->nVtx[k]].fOutUV[1];
							}
							else
							{
								pExpMtl->aUVSet[n2][n].fU = pExpMtl->aUVSet[0][n].fU;//m_VertexList[pTri->nVtx[k]].aUVSet[nUVSet].fU;
								pExpMtl->aUVSet[n2][n].fV = pExpMtl->aUVSet[0][n].fV;//m_VertexList[pTri->nVtx[k]].aUVSet[nUVSet].fV;
							}
							pExpMtl->aUVSet[n2][n].nLMIndex = pExpMtl->anLightMapIdx[n2];
							pExpMtl->aUVSet[n2][n].nVBIndex = nVBIdx;
							pExpMtl->aUVSet[n2][n].nVtxIndex = nVtxIdx;
							n2++;
						}
					}

					pExpMtl->nVtx++;
				}
			}
		}
	}

	return (pExpMtl);
}

void CLightingSolution::FreeMaterial(LN_MaterialExport_t *pExpMtl)
{
	if (pExpMtl)
	{
		if (pExpMtl->anLightMapIdx) { free(pExpMtl->anLightMapIdx); }
		pExpMtl->anLightMapIdx = NULL;
		if (pExpMtl->anMotifList) { free(pExpMtl->anMotifList); }
		pExpMtl->anMotifList = NULL;

		if (pExpMtl->aUVSet)
		{
			u32 i;
			for (i=0; i<pExpMtl->nNumLightMaps; i++)
			{
				if (pExpMtl->aUVSet[i]) free(pExpMtl->aUVSet[i]);
				pExpMtl->aUVSet[i] = NULL;
			}
			free(pExpMtl->aUVSet);
			pExpMtl->aUVSet = NULL;
		}

		free(pExpMtl);
		pExpMtl = NULL;
	}
}

u32 CLightingSolution::GetVBIndex(u32 nVtx, u32 nVolume)
{
	u32 nVtxGroup = m_nVolumeVtxGroup[nVolume];
	for (; nVtxGroup<(u32)(m_nNumVtxGroup-1); nVtxGroup++)
	{
		if (nVtx >= m_nVtxGroup[nVtxGroup] && nVtx < m_nVtxGroup[nVtxGroup+1])
		{
			return (nVtxGroup - m_nVolumeVtxGroup[nVolume]);
		}
	}
	return 0;
}

u32 CLightingSolution::GetVtxIndex(u32 nVtx, u32 nVBIdx, u32 nVolume)
{
	return (nVtx - m_nVtxGroup[nVBIdx]);
}

//Basic AddTriangle(), override for more advanced features.
void CLightingSolution::AddTriangle(s32 nVolumeIdx, u32 nV0, u32 nV1, u32 nV2)
{
	if (nVolumeIdx > m_nCurVolume)
	{
		if (nVolumeIdx > 0)
		{
            m_VolumeList[m_nCurVolume].nEndIdx = m_nNumTri-1;
		}

		m_nCurVolume = nVolumeIdx;
		m_nNumVolumes = nVolumeIdx + 1;

		m_VolumeList[nVolumeIdx].nStartIdx = m_nNumTri;
		if (nVolumeIdx < m_nGeoInstanceVolume)
		{
			m_VolumeList[nVolumeIdx].bPackUV = TRUE;
		}
		else
		{
			m_VolumeList[nVolumeIdx].bPackUV = FALSE;
		}

		m_VolumeList[nVolumeIdx].afClr[0] = fmodf(m_nCurVolume*0.10f, 1.0f);
		m_VolumeList[nVolumeIdx].afClr[1] = fmodf(m_nCurVolume*0.20f, 1.0f);
		m_VolumeList[nVolumeIdx].afClr[2] = fmodf(m_nCurVolume*0.15f+0.25f, 1.0f);
		m_VolumeList[nVolumeIdx].afClr[3] = 1.0f;
	}

	m_TriangleList[m_nNumTri].nVtx[0] = nV0;
	m_TriangleList[m_nNumTri].nVtx[1] = nV1;
	m_TriangleList[m_nNumTri].nVtx[2] = nV2;

	m_TriangleList[m_nNumTri].pVolume = &m_VolumeList[nVolumeIdx];

	m_TriangleList[m_nNumTri].pMaterial = m_CurMaterial;

	m_nNumTri++;
}

//Basic AddVertex(), override for more advanced features.
void CLightingSolution::AddVertex(LS_LightingVtx_t Vtx, u32 nUserData)
{
	m_VertexList[m_nNumVtx].nUserData = nUserData;

	m_VertexList[m_nNumVtx].fPos[0] = Vtx.x; m_VertexList[m_nNumVtx].fPos[1] = Vtx.y; m_VertexList[m_nNumVtx].fPos[2] = Vtx.z;
	m_VertexList[m_nNumVtx].fNrml[0] = Vtx.nx; m_VertexList[m_nNumVtx].fNrml[1] = Vtx.ny; m_VertexList[m_nNumVtx].fNrml[2] = Vtx.nz;
	m_VertexList[m_nNumVtx].fMaskUV[0] = Vtx.fBaseU; m_VertexList[m_nNumVtx].fMaskUV[1] = Vtx.fBaseV; 
	if (m_CurMaterial->bStaticUV)
	{
		m_VertexList[m_nNumVtx].fBaseUV[0] = Vtx.u; m_VertexList[m_nNumVtx].fBaseUV[1] = Vtx.v;
	}
	else
	{
		m_VertexList[m_nNumVtx].fBaseUV[0] = 0; m_VertexList[m_nNumVtx].fBaseUV[1] = 0;
	}
	m_VertexList[m_nNumVtx].fClr[0] = Vtx.r;
	m_VertexList[m_nNumVtx].fClr[1] = Vtx.g;
	m_VertexList[m_nNumVtx].fClr[2] = Vtx.b;
	m_VertexList[m_nNumVtx].fClr[3] = Vtx.a;
	m_nNumVtx++;

	m_VolumeList[m_nCurVolume_Vtx].nNumVtx++;
}

void CLightingSolution::CopyUV_Motif(u32 nMotif, u32 nVolume)
{
	u32 nUVSet = m_VolumeList[ nVolume ].m_aMotifList[ nMotif ].nUVIndex;

	u32 i;
	for (i=0; i<m_nNumVtx; i++)
	{
		m_VertexList[i].aUVSet[nUVSet].fU = m_VertexList[i].fBaseUV[0];
		m_VertexList[i].aUVSet[nUVSet].fV = m_VertexList[i].fBaseUV[1];
	}
}

void CLightingSolution::FillVtxUV(u32 nUVSet, u32 nStart, u32 nEnd, LN_UV_Set **m_pUVBuffer)
{
	u32 i=0;
	for (i=nStart; i<=nEnd; i++)
	{
		m_pUVBuffer[0][i].fU = m_VertexList[m_nNumVtx].fBaseUV[0]; m_pUVBuffer[0][i].fV = m_VertexList[m_nNumVtx].fBaseUV[1]; 
	}
}

//Basic SetVtxGroup(), override for more advanced features.
void CLightingSolution::SetVtxGroup(u32 nGroup)
{
	m_nVtxGroup[nGroup] = m_nNumVtx;
	if (m_nNumVtxGroup < nGroup+1) m_nNumVtxGroup = nGroup+1;
}

//Basic SetTriGroup(), override for more advanced features.
void CLightingSolution::SetTriGroup(u32 nGroup)
{
	m_nTriGroup[nGroup] = m_nNumTri;
	if (m_nNumTriGroup < nGroup+1) m_nNumTriGroup = nGroup+1;
}

//Basic AddLight(), override for more advanced features.
void CLightingSolution::AddLight(u32 nVolume, LS_LightType_e LightType, CFVec3& vDir, CFVec3& vPos, CFVec3& vDAtten, CFVec3& vAAtten, CFVec3& vColor, f32 fColorIntens, f32 fAttenItens, f32 fOOR2, u16 nMotifIndex)
{
	s32 nLightIdx = FindLight(LightType, vDir, vPos, fOOR2);
	if (nLightIdx < 0)
	{
		m_LightList[m_nNumLights].nVolume[0] = nVolume;
		m_LightList[m_nNumLights].LightType = LightType;
		m_LightList[m_nNumLights].vDir = vDir;
		m_LightList[m_nNumLights].vPos = vPos;
		m_LightList[m_nNumLights].vDAtten = vDAtten;
		m_LightList[m_nNumLights].vAAtten = vAAtten;
		m_LightList[m_nNumLights].vColor = vColor * fColorIntens;
		m_LightList[m_nNumLights].fOOR2 = fOOR2;
		if ( fOOR2 > 0.00001f )
		{
			m_LightList[m_nNumLights].fRadius = fmath_InvSqrt(fOOR2);
		}
		else
		{
			m_LightList[m_nNumLights].fRadius = 500.0f;
		}
		m_LightList[m_nNumLights].fIntensity = fAttenItens;
		m_LightList[m_nNumLights].nNumVolumes = 0;
		m_LightList[m_nNumLights].nMotifIndex = nMotifIndex;

		m_nNumLights++;
	}
	else
	{
		if (m_LightList[m_nNumLights].nNumVolumes < 4)
		{
			m_LightList[m_nNumLights].nVolume[ m_LightList[m_nNumLights].nNumVolumes++ ] = nVolume;
		}
	}
}

void CLightingSolution::AssignLightsToTri()
{
	u32 nTri, nLight;
	LN_Triangle_t *pTri;
	LN_Vertex_t *pVtx[3];
	LN_Light_t *pLight;
	CFVec3 vMin, vMax, vOffs;
	f32 fDist;
	for (nTri=0; nTri<m_nNumTri; nTri++)
	{
		//Create triangle bounding spheres
		pTri = &m_TriangleList[nTri];
		pVtx[0] = &m_VertexList[ pTri->nVtx[0] ];
		pVtx[1] = &m_VertexList[ pTri->nVtx[1] ];
		pVtx[2] = &m_VertexList[ pTri->nVtx[2] ];
		vMin.Set(pVtx[0]->fPos[0], pVtx[0]->fPos[1], pVtx[0]->fPos[2]);
		vMax = vMin;

		if (pVtx[1]->fPos[0] < vMin.x) vMin.x = pVtx[1]->fPos[0];
		if (pVtx[1]->fPos[1] < vMin.y) vMin.y = pVtx[1]->fPos[1];
		if (pVtx[1]->fPos[2] < vMin.z) vMin.z = pVtx[1]->fPos[2];

		if (pVtx[2]->fPos[0] < vMin.x) vMin.x = pVtx[2]->fPos[0];
		if (pVtx[2]->fPos[1] < vMin.y) vMin.y = pVtx[2]->fPos[1];
		if (pVtx[2]->fPos[2] < vMin.z) vMin.z = pVtx[2]->fPos[2];

		if (pVtx[1]->fPos[0] > vMax.x) vMax.x = pVtx[1]->fPos[0];
		if (pVtx[1]->fPos[1] > vMax.y) vMax.y = pVtx[1]->fPos[1];
		if (pVtx[1]->fPos[2] > vMax.z) vMax.z = pVtx[1]->fPos[2];

		if (pVtx[2]->fPos[0] > vMax.x) vMax.x = pVtx[2]->fPos[0];
		if (pVtx[2]->fPos[1] > vMax.y) vMax.y = pVtx[2]->fPos[1];
		if (pVtx[2]->fPos[2] > vMax.z) vMax.z = pVtx[2]->fPos[2];

		pTri->vCen.x = ( vMin.x + vMax.x )*0.5f; 
		pTri->vCen.y = ( vMin.y + vMax.y )*0.5f;
		pTri->vCen.z = ( vMin.z + vMax.z )*0.5f;

		vOffs = vMax - pTri->vCen;
		pTri->fRadius = vOffs.Mag();
//		pTri->fRadius2 = pTri->fRadius*pTri->fRadius;
		pTri->nNumLights = 0;
		f32 fM2;
		//Check for overlapping triangle bounding spheres and light spheres.
		for (nLight=0; nLight<m_nNumLights; nLight++)
		{
			pLight = &m_LightList[nLight];
			if (pLight->nMotifIndex > 0)
			{
				vOffs = pLight->vPos - pTri->vCen;
				fDist = vOffs.Mag();

				fM2 = 2.0f;
				
				fDist = fDist - pTri->fRadius - pLight->fRadius*pLight->fIntensity*m_fMaxLightEffectMult*fM2;

				if (fDist <= 0.0f)
				{
					pTri->pLights[ pTri->nNumLights++ ] = pLight;
					if (pTri->nNumLights >= LS_MAX_LIGHTPERTRIANGLE)
					{
						break;
					}
				}
			}
		}
	}
}

//Basic OffsetTriIndexs(), override for more advanced features.
void CLightingSolution::OffsetTriIndexs(u32 nGroup, u32 nStartIdx, u32 nSize, u32 nOffs)
{
	u32 nStart, nEnd, i;
	if (nOffs)
	{
		nStart = m_nTriGroup[nGroup] + nStartIdx; nEnd = nStart + nSize;
		for (i=nStart; i<nEnd; i++)
		{
			m_TriangleList[i].nVtx[0] += nOffs;
			m_TriangleList[i].nVtx[1] += nOffs;
			m_TriangleList[i].nVtx[2] += nOffs;
		}
	}
}

void CLightingSolution::SetTriCurrentMaterial(u32 nGroup, u32 nStartIdx, u32 nSize)
{
	u32 nStart, nEnd, i;
	nStart = m_nTriGroup[nGroup] + nStartIdx; nEnd = nStart + nSize;
	for (i=nStart; i<nEnd; i++)
	{
		m_TriangleList[i].pMaterial = m_CurMaterial;
	}
}

void CLightingSolution::SetLevelName(cchar *pszName)
{
	fclib_strcpy(m_szLevelName, (cchar *)pszName);
}

void CLightingSolution::OutputLightMaps()
{
	u32 i, nMipLevel=0;
	char szFileName[16];
	char szMapNum[16];
	char szName[16];

	for ( i = 0; i < m_nNumLightMaps; i++ )
	{
		if ( !m_pLightMapList[i] ) 
		{ 
			continue; 
		}

		sprintf(szFileName, "%s.tga", m_szLevelName);
		sprintf(szMapNum, "%03d", i);
		szFileName[0] = szMapNum[0];
		szFileName[1] = szMapNum[1];
		szFileName[2] = szMapNum[2];

		sprintf(szName, "%s", m_szLevelName);
		szName[0] = szMapNum[0];
		szName[1] = szMapNum[1];
		szName[2] = szMapNum[2];

		BOOL bOutputLM = TRUE;
		if ( i < m_nNumVolumes )
		{
			if ( !m_VolumeList[i].bHasLM ) 
			{ 
				bOutputLM = FALSE; 
			}
		}
		else
		{
/*
		// This code used to detect when there was a minimum number of pixels above a
		// minimum intensity threshhold.  However, since we compact down lm's that
		// contain only the minimum ambient in the SaveFile(), below, this is not as important
			if ( !m_pLightMapList[i]->CheckData() )
			{
				bOutputLM = FALSE;	
			}
*/
		}
		m_pLightMapList[i]->SetName( szName );
		if ( bOutputLM )
		{
			m_pLightMapList[i]->SetCheck( TRUE );
			m_pLightMapList[i]->SaveFile( szFileName, m_szLevelName, (i>=m_nNumVolumes)?(TRUE):(FALSE), nMipLevel );
		}
	}
}

s32 CLightingSolution::FindLight(LS_LightType_e LightType, CFVec3& vDir, CFVec3& vPos, f32 fOOR2)
{
	s32 nIdx=-1;
	u32 i;
	for (i=0; i<m_nNumLights; i++)
	{
		if (m_LightList[i].LightType == LightType)
		{
			if (m_LightList[i].vDir == vDir && m_LightList[i].vPos == vPos && m_LightList[i].fOOR2 == fOOR2)
			{
				nIdx = i;
				break;
			}
		}
	}
	return (nIdx);
}

BOOL CLightingSolution::IndexInTri(LN_Triangle_t *pTri, u32 nIndex)
{
	if (pTri->nVtx[0] == nIndex || pTri->nVtx[1] == nIndex || pTri->nVtx[2] == nIndex)
	{
		return (TRUE);
	}
	return (FALSE);
}

CFVec2 CLightingSolution::FindMinUV_Tri(LN_Triangle_t *pTri)
{
	CFVec2 vMin;
	LN_Vertex_t *pVtx;
	u32 i;

	vMin.x = m_VertexList[ pTri->nVtx[0] ].fBaseUV[0];
	vMin.y = m_VertexList[ pTri->nVtx[0] ].fBaseUV[1];
	for (i=1; i<3; i++)
	{
		pVtx = &m_VertexList[ pTri->nVtx[i] ];
		if (pVtx->fBaseUV[0] < vMin.x) vMin.x = pVtx->fBaseUV[0];
		if (pVtx->fBaseUV[1] < vMin.y) vMin.y = pVtx->fBaseUV[1];
	}

	return (vMin);
}

CFVec2 CLightingSolution::FindMaxUV_Tri(LN_Triangle_t *pTri)
{
	CFVec2 vMax;
	LN_Vertex_t *pVtx;
	u32 i;

	vMax.x = m_VertexList[ pTri->nVtx[0] ].fBaseUV[0];
	vMax.y = m_VertexList[ pTri->nVtx[0] ].fBaseUV[1];
	for (i=1; i<3; i++)
	{
		pVtx = &m_VertexList[ pTri->nVtx[i] ];
		if (pVtx->fBaseUV[0] > vMax.x) vMax.x = pVtx->fBaseUV[0];
		if (pVtx->fBaseUV[1] > vMax.y) vMax.y = pVtx->fBaseUV[1];
	}

	return (vMax);
}

CFVec2 CLightingSolution::FindMinUV_Tri_Motif(LN_Triangle_t *pTri, u16 nUVSet)
{
	CFVec2 vMin(1000000.0f, 1000000.0f);
	LN_Vertex_t *pVtx;
	u32 i;

	for (i=0; i<3; i++)
	{
		pVtx = &m_VertexList[ pTri->nVtx[i] ];
		if (pVtx->aUVSet[nUVSet].fU < vMin.x) vMin.x = pVtx->aUVSet[nUVSet].fU;
		if (pVtx->aUVSet[nUVSet].fV < vMin.y) vMin.y = pVtx->aUVSet[nUVSet].fV;
	}

	return (vMin);
}

CFVec2 CLightingSolution::FindMaxUV_Tri_Motif(LN_Triangle_t *pTri, u16 nUVSet)
{
	CFVec2 vMax(-1000000.0f, -1000000.0f);
	LN_Vertex_t *pVtx;
	u32 i;

	for (i=0; i<3; i++)
	{
		pVtx = &m_VertexList[ pTri->nVtx[i] ];
		if (pVtx->aUVSet[nUVSet].fU > vMax.x) vMax.x = pVtx->aUVSet[nUVSet].fU;
		if (pVtx->aUVSet[nUVSet].fV > vMax.y) vMax.y = pVtx->aUVSet[nUVSet].fV;
	}

	return (vMax);
}

BOOL CLightingSolution::TriHasMotif(LN_Triangle_t *pTri, u32 nMotifIdx, u32 nVolume)
{
	LN_Material_t *pMtl = pTri->pMaterial;
	u32 nMotif = m_VolumeList[ nVolume ].m_aMotifList[nMotifIdx].nMotifIndex;

	for (u32 i=0; i<pMtl->nNumMotif; i++)
	{
		if (pMtl->anMotifArray[i] == nMotif)
		{
			return (TRUE);
		}
	}
	return (FALSE);
}

u16 CLightingSolution::FindUVSet(LN_Volume_t *pVolume, u32 nMotif)
{
	u32 i;
	if (nMotif > 0)
	{
		for (i=0; i<pVolume->nNumMotif; i++)
		{
			if (nMotif == pVolume->m_aMotifList[i].nMotifIndex)
			{
				return (pVolume->m_aMotifList[i].nUVIndex);
			}
		}
	}
	return 0;
}

void CLightingSolution::FindClusterExtents()
{
	u32 i;
	CFVec2 vMin, vMax;
	LN_Cluster_t *pCluster;
	LN_Triangle_t *pTri;
	for (i=0; i<m_nClusters; i++)
	{
		pCluster = &m_Clusters[i];
		pTri = pCluster->pHead;

		pCluster->vMin = FindMinUV_Tri(pTri);
		pCluster->vMax = FindMaxUV_Tri(pTri);

		pTri = pTri->pNext;

		while (pTri)
		{
			vMin = FindMinUV_Tri(pTri);
			vMax = FindMaxUV_Tri(pTri);

			if (vMin.x < pCluster->vMin.x) pCluster->vMin.x = vMin.x;
			if (vMin.y < pCluster->vMin.y) pCluster->vMin.y = vMin.y;

			if (vMax.x > pCluster->vMax.x) pCluster->vMax.x = vMax.x;
			if (vMax.y > pCluster->vMax.y) pCluster->vMax.y = vMax.y;

			pTri = pTri->pNext;
		};
	}
}

void CLightingSolution::FindClusterExtents_Motif(u32 nMotif, u32 nVolume)
{
	u32 i;
	CFVec2 vMin, vMax;
	LN_Cluster_t *pCluster;
	LN_Triangle_t *pTri;
	for (i=0; i<m_nClusters_Motif[nMotif]; i++)
	{
		pCluster = &m_Clusters_Motif[nMotif][i];
		pTri = pCluster->pHead;

		pCluster->vMin = FindMinUV_Tri_Motif(pTri, m_VolumeList[nVolume].m_aMotifList[nMotif].nUVIndex);
		pCluster->vMax = FindMaxUV_Tri_Motif(pTri, m_VolumeList[nVolume].m_aMotifList[nMotif].nUVIndex);

		pTri = pTri->pNext;

		while (pTri)
		{
			vMin = FindMinUV_Tri_Motif(pTri, m_VolumeList[nVolume].m_aMotifList[nMotif].nUVIndex);
			vMax = FindMaxUV_Tri_Motif(pTri, m_VolumeList[nVolume].m_aMotifList[nMotif].nUVIndex);

			if (vMin.x < pCluster->vMin.x) pCluster->vMin.x = vMin.x;
			if (vMin.y < pCluster->vMin.y) pCluster->vMin.y = vMin.y;

			if (vMax.x > pCluster->vMax.x) pCluster->vMax.x = vMax.x;
			if (vMax.y > pCluster->vMax.y) pCluster->vMax.y = vMax.y;

			pTri = pTri->pNext;
		};
	}
}

int CLightingSolution::CompareLess( const void *arg1, const void *arg2 )
{
	LN_Cluster_t *pC0, *pC1;
	f32 a, b;
	pC0 = (LN_Cluster_t *)arg1;
	pC1 = (LN_Cluster_t *)arg2;

	a = pC0->vMax.x - pC0->vMin.x;
	b = pC1->vMax.x - pC1->vMin.x;

	return (a < b)?(-1):(+1);
}

int CLightingSolution::CompareGreater( const void *arg1, const void *arg2 )
{
	LN_Cluster_t *pC0, *pC1;
	f32 a, b;
	pC0 = (LN_Cluster_t *)arg1;
	pC1 = (LN_Cluster_t *)arg2;

	a = pC0->vMax.x - pC0->vMin.x;
	b = pC1->vMax.x - pC1->vMin.x;

	return (a > b)?(-1):(+1);
}

void CLightingSolution::SortClusters(CompareFunc pFunc)
{
	qsort(m_Clusters, m_nClusters, sizeof(LN_Cluster_t), pFunc);
}

void CLightingSolution::MoveClusterUV(LN_Cluster_t *pCluster, f32 fDU, f32 fDV)
{
	//memset(m_anVisited, 0, m_nNumVtx);

	LN_Triangle_t *pTri = pCluster->pHead;
	u32 nIdx, i;
	while (pTri)
	{
		for (i=0; i<3; i++)
		{
			nIdx = pTri->nVtx[i];
			//if (!m_anVisited[nIdx])
			{
				//m_anVisited[nIdx] = 1;
				m_VertexList[nIdx].fBaseUV[0] += fDU;
				m_VertexList[nIdx].fBaseUV[1] += fDV;
			}
		}
		pTri = pTri->pNext;
	};
}

void CLightingSolution::MoveClusterUV_Motif(LN_Cluster_t *pCluster, f32 fDU, f32 fDV, u32 nMotif, u32 nVolume)
{
	memset(m_anVisited, 0, m_nNumVtx);

	LN_Triangle_t *pTri = pCluster->pHead;
	u32 nIdx, i;
	u16 nUVSet = m_VolumeList[ nVolume ].m_aMotifList[ nMotif ].nUVIndex;
	while (pTri)
	{
		for (i=0; i<3; i++)
		{
			nIdx = pTri->nVtx[i];
			if (!m_anVisited[nIdx])
			{
				m_anVisited[nIdx] = 1;
				m_VertexList[nIdx].aUVSet[nUVSet].fU += fDU;
				m_VertexList[nIdx].aUVSet[nUVSet].fV += fDV;
			}
		}
		pTri = pTri->pNext;
	};
}

void CLightingSolution::AddMaterialMotif(LN_Material_t *pMtl, u16 nMotifIndex)
{
	if (pMtl->nNumMotif < LS_MAX_LIGHTMAPS)
	{
		u16 nMotif;
		if (pMtl->nNumMotif > 0)
		{
			for (nMotif=0; nMotif<pMtl->nNumMotif; nMotif++)
			{
				if (pMtl->anMotifArray[nMotif] == nMotifIndex)
				{
					return;
				}
			}
		}
		pMtl->anMotifArray[ pMtl->nNumMotif++ ] = nMotifIndex;
	}
}

void CLightingSolution::GenerateMotifList(LN_Material_t *pMtl, u32 nStart, u32 nEnd)
{
	u32 nTri, nLight;
	LN_Triangle_t *pTri;
	LN_Light_t *pLight;

	pMtl->nNumMotif = 0;

	if (!pMtl->bHasLightMap) return;

	for (nTri=nStart; nTri<nEnd; nTri++)
	{
		pTri = &m_TriangleList[nTri];
		if (pTri->pMaterial == pMtl) //check for motifs.
		{
			for (nLight=0; nLight<pTri->nNumLights; nLight++)
			{
				pLight = pTri->pLights[nLight];
				if (pLight->nMotifIndex != 0)
				{
					AddMaterialMotif(pMtl, pLight->nMotifIndex);
				}
			}
		}
	}
}

BOOL CLightingSolution::ValueInArray16(u16 nArraySize, u16 *pArray, u16 nValue)
{
	BOOL bRet = FALSE;
	u16 i;
	for (i=0; i<nArraySize; i++)
	{
		if (pArray[i] == nValue)
		{
			bRet = TRUE;
			break;
		}
	}
	return (bRet);
}

BOOL CLightingSolution::ValueInArrayML(u16 nArraySize, LN_MotifIndex_t *pArray, u16 nValue)
{
	BOOL bRet = FALSE;
	u16 i;
	for (i=0; i<nArraySize; i++)
	{
		if (pArray[i].nMotifIndex == nValue)
		{
			bRet = TRUE;
			break;
		}
	}
	return (bRet);
}

BOOL CLightingSolution::ValueInArray32(u32 nArraySize, u32 *pArray, u32 nValue)
{
	BOOL bRet = FALSE;
	u32 i;
	for (i=0; i<nArraySize; i++)
	{
		if (pArray[i] == nValue)
		{
			bRet = TRUE;
			break;
		}
	}
	return (bRet);
}

u16 CLightingSolution::GetMtlMotifIdx(LN_Material_t *pMtl, u16 nMotif)
{
	if (nMotif == 0) return 0;
	u16 i;
	for (i=0; i<pMtl->nNumMotif; i++)
	{
		if (pMtl->anMotifArray[i] == nMotif) return (i+1);
	}
	return 0;
}

void CLightingSolution::ScaleClusterUV(LN_Cluster_t *pCluster, f32 fSX, f32 fSY)
{
	CFVec2 vCen = pCluster->vMin + pCluster->vMax;
	vCen *= 0.5f;

	LN_Triangle_t *pTri = pCluster->pHead;
	while (pTri)
	{
		//Render
		m_VertexList[ pTri->nVtx[0] ].fBaseUV[0] = (m_VertexList[ pTri->nVtx[0] ].fBaseUV[0] - vCen.x)*fSX + vCen.x;
		m_VertexList[ pTri->nVtx[0] ].fBaseUV[1] = (m_VertexList[ pTri->nVtx[0] ].fBaseUV[1] - vCen.y)*fSY + vCen.y;

		m_VertexList[ pTri->nVtx[1] ].fBaseUV[0] = (m_VertexList[ pTri->nVtx[1] ].fBaseUV[0] - vCen.x)*fSX + vCen.x;
		m_VertexList[ pTri->nVtx[1] ].fBaseUV[1] = (m_VertexList[ pTri->nVtx[1] ].fBaseUV[1] - vCen.y)*fSY + vCen.y;

		m_VertexList[ pTri->nVtx[2] ].fBaseUV[0] = (m_VertexList[ pTri->nVtx[2] ].fBaseUV[0] - vCen.x)*fSX + vCen.x;
		m_VertexList[ pTri->nVtx[2] ].fBaseUV[1] = (m_VertexList[ pTri->nVtx[2] ].fBaseUV[1] - vCen.y)*fSY + vCen.y;

		pTri = pTri->pNext;
	};

	pCluster->vMin.x = (pCluster->vMin.x - vCen.x)*fSX + vCen.x;
	pCluster->vMin.y = (pCluster->vMin.y - vCen.y)*fSY + vCen.y;

	pCluster->vMax.x = (pCluster->vMax.x - vCen.x)*fSX + vCen.x;
	pCluster->vMax.y = (pCluster->vMax.y - vCen.y)*fSY + vCen.y;
}

CFVec3 FindNp(CFVec3& vN)
{
	s8 nMax;
	CFVec3 vNp;

	nMax = ( fabsf(vN.x) >= fabsf(vN.y) && fabsf(vN.x) >= fabsf(vN.z) )?(0):( (fabsf(vN.y) >= fabsf(vN.x) && fabsf(vN.y) >= fabsf(vN.z))?(1):(2) );

	if ( vN.y > 0.0f && nMax == 1)//0.7071f )
	{
		vNp = CFVec3(0, 1, 0);
	}
	else if (vN.y < 0.0f && nMax == 1)//-0.7071f)
	{
		vNp = CFVec3(0, -1, 0);
	}
	else if ( vN.x > 0.0f && nMax == 0)//0.7071f )
	{
		vNp = CFVec3(1, 0, 0);
	}
	else if ( vN.x < 0.0f && nMax == 0)//-0.7071f )
	{
		vNp = CFVec3(-1, 0, 0);
	}
	else if ( vN.z > 0.0f && nMax == 2)//0.7071f )
	{
		vNp = CFVec3(0, 0, 1);
	}
	else if ( vN.z < 0.0f && nMax == 2)//-0.7071f )
	{
		vNp = CFVec3(0, 0, -1);
	}
	vNp = CFVec3(0,1,0);
	return vNp;
}

void FindST(CFVec3& vN, CFVec3& vS, CFVec3& vT, CFVec3& vNp)
{
	static s8 nMax;
	//static CFVec3 vNp;

	nMax = ( fabsf(vN.x) >= fabsf(vN.y) && fabsf(vN.x) >= fabsf(vN.z) )?(0):( (fabsf(vN.y) >= fabsf(vN.x) && fabsf(vN.y) >= fabsf(vN.z))?(1):(2) );

	if (nMax == 1)
	{
		vS.Set(1, 0, 0);// = CFVec3(1, 0, 0);
		vT.Set(0, 0, 1);
		vNp.Set(0, 1, 0);
	}
	else if (nMax == 0)
	{
		vS.Set(0, 1, 0);
		vT.Set(0, 0, 1);
		vNp.Set(1, 0, 0);
	}
	else
	{
		vS.Set(1, 0, 0);
		vT.Set(0, 1, 0);
		vNp.Set(0, 0, 1);
	}

	//return (vNp);
}

typedef struct _TriEdge_s _TriEdge_t;
struct _TriEdge_s 
{
	CLightingSolution::LN_Triangle_t *pTri;
	_TriEdge_t *apFellowEdges[2];// the other 2 points that make up this tri
	FLink_t BucketLink;

	FLinkRoot_t *pBucket;
};

#define _BUCKETS_X						16
#define _BUCKETS_Y						16
#define _BUCKETS_Z						16
#define _BUCKET_COUNT					( _BUCKETS_X * _BUCKETS_Y * _BUCKETS_Z )
#define _CLOSE_ENOUGH_TO_SHARE_AN_EDGE	(0.000001f)

FINLINE BOOL LN_DoTheseTrisAlreadyShareAnEdge( CLightingSolution::LN_Triangle_t *pTri0, CLightingSolution::LN_Triangle_t *pTri1 ) 
{
	u32 i;

	for( i=0; i < 3; i++ ) 
	{
		if( pTri1->pEdgeTri[i] == pTri0 ) 
		{
			return TRUE;
		}
	}

	return FALSE;
}

FINLINE BOOL LN_TriOverlap( CLightingSolution::LN_Triangle_t *pTri0, CLightingSolution::LN_Triangle_t *pTri1 )
{
	CFVec3 vOffs = pTri1->vCen - pTri0->vCen;
	f32 fR2 = vOffs.Mag2();

	if (fR2 <= (pTri0->fRadius * pTri0->fRadius) + (pTri1->fRadius * pTri1->fRadius) + 0.00001f)
	{
		return TRUE;
	}
	return FALSE;
}

// it is assumed that these tris bounding spheres intersect and that they don't already know about each other
FINLINE BOOL LN_AreTheseTrisAdjacentToEachOther( CLightingSolution::LN_Triangle_t *pTri0, CLightingSolution::LN_Triangle_t *pTri1, f32 fPointTolerance2, u32 *pnEdge0, u32 *pnEdge1 ) 
{
	u32 i, j, anIndex1[2], anIndex2[2], nCloseCount = 0;
	CFVec3 vOffs;
	CFVec3A Delta;

	// find 2 pts in rTri1 that are fPointTolerance2 away from 2 pts in rTri2
	for( i=0; i < 3; i++ ) 
	{
		for( j=0; j < 3; j++ ) 
		{
			vOffs = CFVec3(pTri0->pVtx[i]->fPos[0], pTri0->pVtx[i]->fPos[1], pTri0->pVtx[i]->fPos[2]) - CFVec3(pTri1->pVtx[j]->fPos[0], pTri1->pVtx[j]->fPos[1], pTri1->pVtx[j]->fPos[2]);
			Delta.Set(vOffs);
			if( Delta.MagSq() <= fPointTolerance2 ) 
			{
				anIndex1[nCloseCount] = i;
				anIndex2[nCloseCount] = j;
				nCloseCount++;
				if( nCloseCount == 2 ) 
				{
					// we found 2 pts that are close enough, make sure that we have found an edge and not a pt
					if( anIndex2[0] != anIndex2[1] ) 
					{
						if ( (anIndex1[0] == 0 && anIndex1[1] == 1) || (anIndex1[0] == 1 && anIndex1[1] == 0) ) { *pnEdge0 = 0; }
						else if ( (anIndex1[0] == 1 && anIndex1[1] == 2) || (anIndex1[0] == 2 && anIndex1[1] == 1) ) { *pnEdge0 = 1; }
						else { *pnEdge0 = 2; }

						if ( (anIndex2[0] == 0 && anIndex2[1] == 1) || (anIndex2[0] == 1 && anIndex2[1] == 0) ) { *pnEdge1 = 0; }
						else if ( (anIndex2[0] == 1 && anIndex2[1] == 2) || (anIndex2[0] == 2 && anIndex2[1] == 1) ) { *pnEdge1 = 1; }
						else { *pnEdge1 = 2; }

						return TRUE;
					} 
					else 
					{
						nCloseCount--;
					}
				} 
				else 
				{
					break;
				}
			}
		}
		if( i == 1 && nCloseCount == 0 ) 
		{
			// no need to continue;
			return FALSE;
		}
	}

	return FALSE;
}

FINLINE void LN_LinkTriangles( CLightingSolution::LN_Triangle_t *pTri0, CLightingSolution::LN_Triangle_t *pTri1, u32 nEdge0, u32 nEdge1 )
{
	//FASSERT(pTri0->pEdgeTri[nEdge0] == NULL && pTri1->pEdgeTri[nEdge1] == NULL);

	pTri0->pEdgeTri[nEdge0] = pTri1;
	pTri1->pEdgeTri[nEdge1] = pTri0;
}

#define LMCONNECTIVITY_FAST 0

void CLightingSolution::BuildConnectivityData()
{
#if LMCONNECTIVITY_FAST
	static FLinkRoot_t _aaaBuckets[_BUCKETS_X][_BUCKETS_Y][_BUCKETS_Z];
	u32 nNumEdges, i, j, k, nX, nY, nZ, nPtsToAdd, nNotFullyConnected, nTicks, nMatTime, nSetupTime, nConnectingTime;
	_TriEdge_t *paEdges, *pEdge, *pNext, *pSearch, *pNextSearch, *pCurrent;
	LN_Triangle_t *pKTri;
	CFVec3 Temp;
	f32 fX, fY, fZ, fOOX, fOOY, fOOZ;
	u32 nTotalMatches = 0;
	CFVec3 Min, Max;
	FLinkRoot_t *pBucket;

	// allocate enough edges to represent all of the tris in the mesh
	nNumEdges = m_nNumTri * 3;
	paEdges = (_TriEdge_t *)malloc( nNumEdges * sizeof( _TriEdge_t ) );
	if( !paEdges ) 
	{
		// couldn't allocate memory, we are done
		FASSERT_NOW;
		return;
	}

	// init the link lists
	pBucket = &_aaaBuckets[0][0][0];
	for( i=0; i < _BUCKET_COUNT; i++ ) 
	{
		flinklist_InitRoot( pBucket, (s32)FANG_OFFSETOF( _TriEdge_t, BucketLink ) );
		pBucket++;
	}

	//Find Axis Aligned bounding box that tightly fits all the triangles in the world.
	Min.Set( m_TriangleList[0].pVtx[0]->fPos[0], m_TriangleList[0].pVtx[0]->fPos[1], m_TriangleList[0].pVtx[0]->fPos[2] );
	Max = Min;
	for (i=0; i<m_nNumTri; i++)
	{
		for (j=0; j<3; j++)
		{
			if (m_TriangleList[i].pVtx[j]->fPos[0] < Min.x) Min.x = m_TriangleList[i].pVtx[j]->fPos[0];
			if (m_TriangleList[i].pVtx[j]->fPos[1] < Min.y) Min.y = m_TriangleList[i].pVtx[j]->fPos[1];
			if (m_TriangleList[i].pVtx[j]->fPos[2] < Min.z) Min.z = m_TriangleList[i].pVtx[j]->fPos[2];

			if (m_TriangleList[i].pVtx[j]->fPos[0] > Max.x) Max.x = m_TriangleList[i].pVtx[j]->fPos[0];
			if (m_TriangleList[i].pVtx[j]->fPos[1] > Max.y) Max.y = m_TriangleList[i].pVtx[j]->fPos[1];
			if (m_TriangleList[i].pVtx[j]->fPos[2] > Max.z) Max.z = m_TriangleList[i].pVtx[j]->fPos[2];
		}
		m_TriangleList[i].nEdgeMatches = 0;
		m_TriangleList[i].pEdgeTri[0] = m_TriangleList[i].pEdgeTri[1] = m_TriangleList[i].pEdgeTri[2] = NULL;
	}

	// build a min/max box
	fX = (Max.x - Min.x) * (1.01f/_BUCKETS_X);
	fOOX = 1.0f/fX;
	fY = (Max.y - Min.y) * (1.01f/_BUCKETS_Y);
	fOOY = 1.0f/fY;
	fZ = (Max.z - Min.z) * (1.01f/_BUCKETS_Z);
	fOOZ = 1.0f/fZ;

	// walk the tri list, setting up our EdgePts and putting them in the proper buckets
	pKTri = m_TriangleList;
	nNumEdges = 0;
	nNotFullyConnected = 0;
	for( i=0; i < m_nNumTri; i++ ) 
	{
		if( pKTri->nEdgeMatches < 3 ) 
		{
			// create upto 3 edges for the tri
			nPtsToAdd = 0;
			pEdge = &paEdges[nNumEdges];
			for( j=0; j < 3; j++ ) 
			{
				pCurrent = &pEdge[nPtsToAdd];
				
				// fill in the tri edge
				pCurrent->pTri = pKTri;

				// compute which bucket to put this pt into
				Temp = CFVec3(pKTri->pVtx[j]->fPos[0], pKTri->pVtx[j]->fPos[1], pKTri->pVtx[j]->fPos[2]) - Min;
				
				nX = (u32)( Temp.x * fOOX );
				FMATH_CLAMPMAX( nX, (_BUCKETS_X-1) );
				
				nY = (u32)( Temp.y * fOOY );
				FMATH_CLAMPMAX( nY, (_BUCKETS_Y-1) );
				
				nZ = (u32)( Temp.z * fOOZ );
				FMATH_CLAMPMAX( nZ, (_BUCKETS_Z-1) );

				pCurrent->pBucket = &_aaaBuckets[nX][nY][nZ];

				// make sure that this is a unique bucket from the other pts
				for( k=0; k < nPtsToAdd; k++ ) 
				{
					if( pEdge[k].pBucket == pCurrent->pBucket ) 
					{
						// don't add another bucket, we already have this one
						break;
					}
				}
				if( k == nPtsToAdd ) 
				{
					flinklist_AddTail( pCurrent->pBucket, pCurrent );
					nPtsToAdd++;
				}
			}
			// assign pts to each other
			switch( nPtsToAdd ) 
			{
				case 1:
					pEdge[0].apFellowEdges[0] = NULL;
					pEdge[0].apFellowEdges[1] = NULL;
					nNotFullyConnected++;
					break;
					
				case 2:
					pEdge[0].apFellowEdges[0] = &pEdge[1];
					pEdge[0].apFellowEdges[1] = NULL;
					
					pEdge[1].apFellowEdges[0] = &pEdge[0];
					pEdge[1].apFellowEdges[1] = NULL;
					nNotFullyConnected++;
					break;

				case 3:
					pEdge[0].apFellowEdges[0] = &pEdge[1];
					pEdge[0].apFellowEdges[1] = &pEdge[2];
					
					pEdge[1].apFellowEdges[0] = &pEdge[0];
					pEdge[1].apFellowEdges[1] = &pEdge[2];
					
					pEdge[2].apFellowEdges[0] = &pEdge[0];
					pEdge[2].apFellowEdges[1] = &pEdge[1];
					nNotFullyConnected++;
					break;
			}

			nNumEdges += nPtsToAdd;
		}
		pKTri++;
	}

	nSetupTime = 0;//GetTickCount() - (nTicks + nMatTime);

	if ( m_pDlg )
	{
		m_pDlg->ResetSubOp( "Building LightMap Triangle Connectivity...", _BUCKET_COUNT );
		m_pDlg->PostMessage( CDLG_UPDATE_CONTROLS );
	}

	u32 nEdge0, nEdge1;

	// walk each bucket
	pBucket = &_aaaBuckets[0][0][0];
	for( i=0; i < _BUCKET_COUNT; i++ ) {
		
		if ( m_pDlg )
		{
			m_pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );

			if ( m_pDlg->m_bAbortCompile )
			{
				return;
			}
		}

		// walk the edges in the bucket
		pEdge = (_TriEdge_t *)flinklist_GetHead( pBucket );
		while( pEdge ) 
		{
			pNext = (_TriEdge_t *)flinklist_GetNext( pBucket, pEdge );

			if( pEdge->pTri->nEdgeMatches < 3 ) 
			{
				// find any other tris close to this one
				pSearch = pNext;
				while( pSearch ) 
				{
					pNextSearch = (_TriEdge_t *)flinklist_GetNext( pBucket, pSearch );

					if( pSearch->pTri->nEdgeMatches < 3 ) 
					{
						// make sure that pEdge and pSearch don't already share an edge
						if( !LN_DoTheseTrisAlreadyShareAnEdge( pEdge->pTri, pSearch->pTri ) ) 
						{
							// do the tri's bounding spheres intersect
							if ( LN_TriOverlap(pEdge->pTri, pSearch->pTri) )
							{
								// do the tris share an edge?
								if( LN_AreTheseTrisAdjacentToEachOther( pEdge->pTri, pSearch->pTri, _CLOSE_ENOUGH_TO_SHARE_AN_EDGE, &nEdge0, &nEdge1 ) ) 
								{
									// link these 2 tris
									LN_LinkTriangles(pEdge->pTri, pSearch->pTri, nEdge0, nEdge1);

									pEdge->pTri->nEdgeMatches++;
									pSearch->pTri->nEdgeMatches++;

									nTotalMatches+=2;

									if( pEdge->pTri->nEdgeMatches == 3 ) 
									{
										break;
									}
								}
							}
						}
					}
					// move to the next edge
					pSearch = pNextSearch;
				}
			}

			// see if pEdge should be removed
			if( pEdge->pTri->nEdgeMatches == 3 ) 
			{
				nNotFullyConnected--;

				// remove pEdge from its bucket
				flinklist_Remove( pEdge->pBucket, pEdge );

				// remove any of pEdge's fellow edges from their buckets & the main list
				if( pEdge->apFellowEdges[0] ) 
				{
					flinklist_Remove( pEdge->apFellowEdges[0]->pBucket, pEdge->apFellowEdges[0] );
					
					if( pEdge->apFellowEdges[1] ) 
					{
						flinklist_Remove( pEdge->apFellowEdges[1]->pBucket, pEdge->apFellowEdges[1] );
					}
				}				
			}

			// move to the next edge
			pEdge = pNext;
		}

		// advance to the next bucket
		pBucket++;
	}

	nConnectingTime = 0;//GetTickCount() - (nTicks + nSetupTime);
	// free our memory
	free( paEdges );

	char stmp[80];
	sprintf(stmp, "%d matches for %d triangles", nTotalMatches, m_nNumTri );
	MessageBox(NULL, stmp, "INFO", MB_OK);
#else
	u32 i, j;
	s32 nEdgeT0, nEdgeT1;
	LN_Triangle_t *pTri, *pTriNext;

	for ( i = 0; i < m_nNumTri; i++ )
	{
		pTri = &m_TriangleList[i];
		pTri->pEdgeTri[0] = pTri->pEdgeTri[1] = pTri->pEdgeTri[2] = NULL;

		pTri->pVtx[0] = &m_VertexList[ pTri->nVtx[0] ];
		pTri->pVtx[1] = &m_VertexList[ pTri->nVtx[1] ];
		pTri->pVtx[2] = &m_VertexList[ pTri->nVtx[2] ];
		pTri->nEdgeMatches = 0;
	}

	if ( m_pDlg )
	{
		m_pDlg->ResetSubOp( "Building Lightmapping Connectivity Data...", (m_nNumTri / 500) + 1 );
		m_pDlg->PostMessage( CDLG_UPDATE_CONTROLS );
	}

	u32 nTotalMatches = 0;
	for ( i = 0; i < m_nNumTri; i++ )
	{
		if ( m_pDlg )
		{
			if ( (i % 500) == 0 )
			{
				m_pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );
			}
		}

		pTri = &m_TriangleList[i];
		for ( j = i + 1; j < m_nNumTri && pTri->nEdgeMatches < 3; j++ )
		{
			pTriNext = &m_TriangleList[j];
			if ( !EdgeMatch(pTri, pTriNext, nEdgeT0, nEdgeT1) )
			{
				continue;
			}
			FASSERT( nEdgeT0 >=0 && nEdgeT0 < 3 && nEdgeT1 >= 0 && nEdgeT1 < 3 );
			pTri->pEdgeTri[nEdgeT0] = pTriNext;
			pTri->nEdgeMatches++;
			pTriNext->pEdgeTri[nEdgeT1] = pTri;
			pTriNext->nEdgeMatches++;
			nTotalMatches++;
		}
	}
	
//	DEVPRINTF( "%d matches for %d triangles\n", nTotalMatches, m_nNumTri );
	//char stmp[80];
	//sprintf(stmp, "%d matches for %d triangles", nTotalMatches, m_nNumTri );
	//MessageBox(NULL, stmp, "INFO", MB_OK);
#endif
}

/////////////////////////////////////////////////////////////
// BUILD EDGE CONNECTIVITY INFORMATION ACROSS THE ENTIRE MESH
/////////////////////////////////////////////////////////////
/*
typedef struct _TriEdge_s _TriEdge_t;
struct _TriEdge_s {
	KongTri_t *pTri;
	_TriEdge_t *apFellowEdges[2];// the other 2 points that make up this tri
	FLink_t BucketLink;

	FLinkRoot_t *pBucket;
};

#define _BUCKETS_X						16
#define _BUCKETS_Y						16
#define _BUCKETS_Z						16
#define _BUCKET_COUNT					( _BUCKETS_X * _BUCKETS_Y * _BUCKETS_Z )
#define _CLOSE_ENOUGH_TO_SHARE_AN_EDGE	(0.001f * 0.001f)

void CApeToKongFormat::BuildEdgeDataAcrossEntireKong( CFVec3 &Min, CFVec3 &Max, CCompileDlg *pDlg ) 
{
	if ( !m_pKongMesh )
	{
		return;
	}

	static FLinkRoot_t _aaaBuckets[_BUCKETS_X][_BUCKETS_Y][_BUCKETS_Z];
	u32 nNumEdges, i, j, k, nX, nY, nZ, nPtsToAdd, nNotFullyConnected, nTicks, nMatTime, nSetupTime, nConnectingTime;
	_TriEdge_t *paEdges, *pEdge, *pNext, *pSearch, *pNextSearch, *pCurrent;
	KongTri_t *pKTri;
	CFVec3 Temp;
	f32 fX, fY, fZ, fOOX, fOOY, fOOZ;
	FLinkRoot_t *pBucket;

	// first build the material edge data 
	// (THIS CALL WILL START BY RESETING THE EDGE DATA)
	nTicks = GetTickCount();
	BuildEdgeDataPerMaterial();
	nMatTime = GetTickCount() - nTicks;

	// allocate enough edges to represent all of the tris in the mesh
	nNumEdges = m_pKongMesh->nTriangleCount * 3;
	paEdges = (_TriEdge_t *)fang_Malloc( nNumEdges * sizeof( _TriEdge_t ), 4 );
	if( !paEdges ) {
		// couldn't allocate memory, we are done
		FASSERT_NOW;
		return;
	}

	// init the link lists
	pBucket = &_aaaBuckets[0][0][0];
	for( i=0; i < _BUCKET_COUNT; i++ ) {
		flinklist_InitRoot( pBucket, (s32)FANG_OFFSETOF( _TriEdge_t, BucketLink ) );
		pBucket++;
	}

	// build a min/max box
	fX = (Max.x - Min.x) * (1.01f/_BUCKETS_X);
	fOOX = 1.0f/fX;
	fY = (Max.y - Min.y) * (1.01f/_BUCKETS_Y);
	fOOY = 1.0f/fY;
	fZ = (Max.z - Min.z) * (1.01f/_BUCKETS_Z);
	fOOZ = 1.0f/fZ;

	// walk the tri list, setting up our EdgePts and putting them in the proper buckets
	pKTri = m_pKongMesh->paTriangles;
	nNumEdges = 0;
	pEdge = &paEdges[0];
	nNotFullyConnected = 0;
	for( i=0; i < m_pKongMesh->nTriangleCount; i++ ) {

		if( pKTri->nNumEdgeTris < 3 ) {

			// create upto 3 edges for the tri
			nPtsToAdd = 0;
			pEdge = &paEdges[nNumEdges];
			for( j=0; j < 3; j++ ) {
				pCurrent = &pEdge[nPtsToAdd];
				
				// fill in the tri edge
				pCurrent->pTri = pKTri;

				// compute which bucket to put this pt into
				Temp = pKTri->apKongVerts[j]->Pos - Min;
				
				nX = (u32)( Temp.x * fOOX );
				FMATH_CLAMPMAX( nX, (_BUCKETS_X-1) );
				
				nY = (u32)( Temp.y * fOOY );
				FMATH_CLAMPMAX( nY, (_BUCKETS_Y-1) );
				
				nZ = (u32)( Temp.z * fOOZ );
				FMATH_CLAMPMAX( nZ, (_BUCKETS_Z-1) );

				pCurrent->pBucket = &_aaaBuckets[nX][nY][nZ];

				// make sure that this is a unique bucket from the other pts
				for( k=0; k < nPtsToAdd; k++ ) {
					if( pEdge[k].pBucket == pCurrent->pBucket ) {
						// don't add another bucket, we already have this one
						break;
					}
				}
				if( k == nPtsToAdd ) {
					flinklist_AddTail( pCurrent->pBucket, pCurrent );
					nPtsToAdd++;
				}
			}
			// assign pts to each other
			switch( nPtsToAdd ) {
			case 1:
				pEdge[0].apFellowEdges[0] = NULL;
				pEdge[0].apFellowEdges[1] = NULL;
				nNotFullyConnected++;
				break;
				
			case 2:
				pEdge[0].apFellowEdges[0] = &pEdge[1];
				pEdge[0].apFellowEdges[1] = NULL;
				
				pEdge[1].apFellowEdges[0] = &pEdge[0];
				pEdge[1].apFellowEdges[1] = NULL;
				nNotFullyConnected++;
				break;

			case 3:
				pEdge[0].apFellowEdges[0] = &pEdge[1];
				pEdge[0].apFellowEdges[1] = &pEdge[2];
				
				pEdge[1].apFellowEdges[0] = &pEdge[0];
				pEdge[1].apFellowEdges[1] = &pEdge[2];
				
				pEdge[2].apFellowEdges[0] = &pEdge[0];
				pEdge[2].apFellowEdges[1] = &pEdge[1];
				nNotFullyConnected++;
				break;
			}

			nNumEdges += nPtsToAdd;
		}
		pKTri++;
	}

	nSetupTime = GetTickCount() - (nTicks + nMatTime);
#if _SHOW_DETAILED_TIMINGS
	DEVPRINTF( "BuildEdgeDataAcrossEntireKong(): out of %d orig tris, %d remain after connecting each material's edges.\n", m_nNumKongTris, nNotFullyConnected );
#endif

	if ( pDlg )
	{
		pDlg->ResetSubOp( "Building Triangle Connectivity...", _BUCKET_COUNT );
		pDlg->PostMessage( CDLG_UPDATE_CONTROLS );
	}

	// walk each bucket
	pBucket = &_aaaBuckets[0][0][0];
	for( i=0; i < _BUCKET_COUNT; i++ ) {
		
		if ( pDlg )
		{
			pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );

			if ( pDlg->m_bAbortCompile )
			{
				return;
			}
		}

		// walk the edges in the bucket
		pEdge = (_TriEdge_t *)flinklist_GetHead( pBucket );
		while( pEdge ) {
			pNext = (_TriEdge_t *)flinklist_GetNext( pBucket, pEdge );

			if( pEdge->pTri->nNumEdgeTris < 3 ) {

				// find any other tris close to this one
				pSearch = pNext;
				while( pSearch ) {
					pNextSearch = (_TriEdge_t *)flinklist_GetNext( pBucket, pSearch );

					if( pSearch->pTri->nNumEdgeTris < 3 ) {
						// make sure that pEdge and pSearch don't already share an edge
						if( !utils_DoTheseTrisAlreadyShareAnEdge( *pEdge->pTri, *pSearch->pTri ) ) {

							// do the tri's bounding spheres intersect
							if( pEdge->pTri->BoundSphere.IsIntersecting( pSearch->pTri->BoundSphere ) ) {

								// do the tris share an edge
								if( utils_AreTheseTrisAdjacentToEachOther( *pEdge->pTri, *pSearch->pTri, _CLOSE_ENOUGH_TO_SHARE_AN_EDGE ) ) {
									// link these 2 tris
									pEdge->pTri->paEdgeTris[ pEdge->pTri->nNumEdgeTris++ ] = pSearch->pTri;
									pSearch->pTri->paEdgeTris[ pSearch->pTri->nNumEdgeTris++ ] = pEdge->pTri;

									if( pEdge->pTri->nNumEdgeTris == 3 ) {
										break;
									}
								}
							}
						}
					}

					// move to the next edge
					pSearch = pNextSearch;
				}
			}

			// see if pEdge should be removed
			if( pEdge->pTri->nNumEdgeTris == 3 ) {
				nNotFullyConnected--;

				// remove pEdge from its bucket
				flinklist_Remove( pEdge->pBucket, pEdge );

				// remove any of pEdge's fellow edges from their buckets & the main list
				if( pEdge->apFellowEdges[0] ) {
					flinklist_Remove( pEdge->apFellowEdges[0]->pBucket, pEdge->apFellowEdges[0] );
					
					if( pEdge->apFellowEdges[1] ) {
						flinklist_Remove( pEdge->apFellowEdges[1]->pBucket, pEdge->apFellowEdges[1] );
					}
				}				
			}

			// move to the next edge
			pEdge = pNext;
		}

		// advance to the next bucket
		pBucket++;
	}

	nConnectingTime = GetTickCount() - (nTicks + nSetupTime);
#if _SHOW_DETAILED_TIMINGS
	DEVPRINTF( "BuildEdgeDataAcrossEntireKong(): %d tris were not able to find all 3 edge connections.\n"
			   "\t%d ms to connect same material edges.\n"
			   "\t%d ms to setup the %d buckets.\n"
			   "\t%d ms to connect different material edges.\n",
			   nNotFullyConnected, nMatTime, nSetupTime, _BUCKET_COUNT, nConnectingTime );
#endif
	// free our memory
	fang_Free( paEdges );
}
*/

void CLightingSolution::AddTriEdgesToNeighborList(LN_Triangle_t *pTri, CFVec3& vN, LN_Triangle_t **pNeighborList, u32& nNeighbor)
{
	LN_Triangle_t *pT2;
	CFVec3 vNp, vS, vT, vNi, vNo;

	u32 i;
	for (i=0; i<3; i++)
	{
		if (nNeighbor >= 16384) return;

		pT2 = pTri->pEdgeTri[i];
		if (pT2)
		{
			vNi = pT2->vNrml;
			vNo = m_pOrigTri->vNrml;
			if ( pTri->pMaterial->bStaticUV )
			{
				vNi = pT2->pMaterial->UnitMtxR.MultDir(vNi);
				vNo = m_pOrigTri->pMaterial->UnitMtxR.MultDir(vNo);
			}

			FindST(vNi, vS, vT, vNp);
			//if (!pT2->bToggle && m_pOrigTri->vNrml.Dot(pT2->vNrml) > 0.9f && m_pOrigTri->pVolume == pT2->pVolume && pT2->pMaterial->bHasLightMap && vN == FindST(pT2->vNrml, vS, vT))
			if (!pT2->bToggle && vNo.Dot(vNi) > 0.9f && m_pOrigTri->pVolume == pT2->pVolume && pT2->pMaterial->bHasLightMap && vN == vNp)
			{
				pNeighborList[ nNeighbor++ ] = pT2;
				pT2->bToggle = TRUE;

				AddTriEdgesToNeighborList(pT2, vN, pNeighborList, nNeighbor);
			}
		}
	}
}

void CLightingSolution::AddMotif(u32 nMotif)
{
	if (m_nUniqueMotif < LS_MAX_UNIQUE_MOTIF && nMotif > 0)
	{
		if (m_nUniqueMotif == 0)
		{
			m_nMotifList[m_nUniqueMotif++] = nMotif;
		}
		else
		{
			u32 i;
			for (i=0; i<m_nUniqueMotif; i++)
			{
				if (m_nMotifList[i] == nMotif) { return; }
			}
			m_nMotifList[m_nUniqueMotif++] = nMotif;
		}
	}
}

u32 CLightingSolution::FindMotifLMap(u32 nMotif)
{
	u32 i;
	for (i=0; i<m_nUniqueMotif; i++)
	{
		if (m_nMotifList[i] == nMotif)
		{
			return (i);
		}
	}
	return 1;
}

f32 _fSampleSize = 2.0f; //ratio of Feet/Lumels
f32 _fOOSampleSize = 1.0f/_fSampleSize;

#define LIGHTMAP_WIDTH 1024
#define LIGHTMAP_HEIGHT 1024

s32 _allocated[LIGHTMAP_WIDTH];

BOOL CLightingSolution::AllocLMapBlock(u32 w, u32 h, u32 *x, u32 *y, u32 *nW, u32 *nH)
{
	s32 best = *nH, best2;
	u32 i, j;

	if (w > *nW || h > *nH)
	{
		return FALSE;
	}

	for (i=0; i<=*nW-w; i++)
	{
		best2 = 0;
		for (j=0; j<w; j++)
		{
			if (_allocated[i+j] >= best)
			{
				break;
			}
			if (_allocated[i+j] > best2)
			{
				best2 = _allocated[i+j];
			}
		}
		if (j == w)
		{
			*x = i;
			*y = best = best2;
		}
	}
	if (best + h > *nH)
	{
		return FALSE;
	}

	for (i=0; i<w; i++)
	{
		_allocated[*x + i] = best+h;
	}

	return TRUE;
}


//
//
//
void CLightingSolution::GenerateLightMapMatrix( LN_Triangle_t *pTri, f32 *fSampleSize )
{
	f32 ssize = *fSampleSize;

	// the two largest axis will be the lightmap size
	pTri->vLightMapMtx[0].Set( 0,0,0,0 );
	pTri->vLightMapMtx[1].Set( 0,0,0,0 );
	
	CFVec3 planeNormal;
	planeNormal.x = fabsf( pTri->vLightMapVecs[2].x );
	planeNormal.y = fabsf( pTri->vLightMapVecs[2].y );
	planeNormal.z = fabsf( pTri->vLightMapVecs[2].z );

	if ( planeNormal.x >= planeNormal.y && planeNormal.x >= planeNormal.z ) 
	{
		pTri->vLightMapMtx[0].y = pTri->fScaleX / ssize;
		pTri->vLightMapMtx[1].z = pTri->fScaleY / ssize;
	} 
	else if ( planeNormal.y >= planeNormal.x && planeNormal.y >= planeNormal.z ) 
	{
		pTri->vLightMapMtx[0].x = pTri->fScaleX / ssize;
		pTri->vLightMapMtx[1].z = pTri->fScaleY / ssize;
	} 
	else 
	{
		pTri->vLightMapMtx[0].x = pTri->fScaleX / ssize;
		pTri->vLightMapMtx[1].y = pTri->fScaleY / ssize;
	}
}

BOOL _bUVOnly=FALSE;

//
//
//
BOOL CLightingSolution::AllocateLightMapForVolume( u32 nVolume, u32 *nW, u32 *nH, f32 *fSampleSize )
{
	static LN_Triangle_t *_pNeighborList[16384];
	//for now do each triangle
	static CFVec3 vMin, vMax;
	static CFVec3 vN, vS, vT, vNp;
	static CFVec3 vecs[2];
	static u32 Size[3];

//	DEVPRINTF( "Allocating lightmap for volume %d\n", nVolume );

	// Get a pointer to the volume
	LN_Volume_t *pCurVolume = &m_VolumeList[nVolume];

	// Calc one over sample size
	f32 fOOSampleSize = 1.0f / *fSampleSize;

	u32 nStart, nEnd, i, j, w, h;
	u32 x, y;

	// Get start and end triangle indices for this volume
	nStart = m_VolumeList[nVolume].nStartIdx;
	nEnd = m_VolumeList[nVolume].nEndIdx;


	if (nEnd == 0 && (nStart > 0 || nVolume == m_nNumVolumes-1))
	{
		nEnd = m_nNumTri-1;
	}
	if (nEnd > m_nNumTri-1) 
	{
		nEnd = 0;
	}

	// Determine if any of the triangles' materials need lightmaps generated
	BOOL bHasLM = FALSE;
	for (i=nStart; i<=nEnd; i++)
	{
		LN_Triangle_t *pTri = &m_TriangleList[i];

		if (pTri->pMaterial->bHasLightMap)
		{
			bHasLM = TRUE;
			break;
		}
	}

	// Record the results of our test in the volume for later referencing
	m_VolumeList[nVolume].bHasLM = bHasLM;

	//if ( !bHasLM ) { return FALSE; }

	// Allocate clusters to hold the triangles
	m_nClusters = 0;
	if (!m_Clusters)
	{
		m_Clusters = new LN_Cluster_t[m_nNumTri];
		memset(m_Clusters, 0, sizeof(LN_Cluster_t)*m_nNumTri);
	}

	// Clear some processing vars
	for (i=nStart; i<=nEnd; i++)
	{
		m_TriangleList[i].bToggle = FALSE;
		m_TriangleList[i].pNext = NULL;
	}

	LN_Triangle_t *pTri;
	LN_Triangle_t *pT2;
	LN_Cluster_t *pCluster;

	u32 n, nNeighbor=0;

	// Proceed through all triangles in the volume building clusters of adjacent 
	// triangles that are within some normal tolerance
	for (i=nStart; i<=nEnd; i++)
	{
		pTri = &m_TriangleList[i];
		
		if (!pTri->bToggle && pTri->pMaterial->bHasLightMap)
		{
			pCluster = &m_Clusters[ m_nClusters++ ];
			pCluster->pHead = pTri;
			pTri->bToggle = TRUE;
			pTri->pNext = NULL;
			pCluster->nTri = 1;

			nNeighbor = 0;
			m_pOrigTri = pTri;
			vN = pTri->vNrml;
			if (pTri->pMaterial->bStaticUV)
			{
				vN = pTri->pMaterial->UnitMtxR.MultDir(vN);
			}
			FindST(vN, vS, vT, vNp);
			vN = vNp;

			AddTriEdgesToNeighborList(pTri, vN, _pNeighborList, nNeighbor);

			if (nNeighbor)
			{
				for (n=0; n<nNeighbor; n++)
				{
					pTri->pNext = _pNeighborList[n];

					pTri = _pNeighborList[n];
					pTri->pNext = NULL;

					pCluster->nTri++;
				}
			}
		}
	}

//	DEVPRINTF( "	Number of clusters: %d\n", m_nClusters );

	CFVec3 vPoint;
	u32 nTrianglesInLightmap = 0, nTotalTriangleCount = nEnd - nStart;
	for (i=0; i<m_nClusters; i++)
	{
		pTri = m_Clusters[i].pHead;
	
//		DEVPRINTF( "	Cluster: %d\n", i );

		//
		// Determine min and max vert positions for the cluster
		//

		vMin = CFVec3(pTri->pVtx[0]->fPos[0], pTri->pVtx[0]->fPos[1], pTri->pVtx[0]->fPos[2]);
		if (pTri->pMaterial->bStaticUV)
		{
			vMin = pTri->pMaterial->MtxR.MultPoint(vMin);
		}
        vMax = vMin;

		do
		{
			vPoint.Set(pTri->pVtx[0]->fPos[0], pTri->pVtx[0]->fPos[1], pTri->pVtx[0]->fPos[2]);
			if (pTri->pMaterial->bStaticUV)
			{
				vPoint = pTri->pMaterial->MtxR.MultPoint(vPoint);
			}
			if (vPoint.x < vMin.x) { vMin.x = vPoint.x; }
			if (vPoint.y < vMin.y) { vMin.y = vPoint.y; }
			if (vPoint.z < vMin.z) { vMin.z = vPoint.z; }

			if (vPoint.x > vMax.x) { vMax.x = vPoint.x; }
			if (vPoint.y > vMax.y) { vMax.y = vPoint.y; }
			if (vPoint.z > vMax.z) { vMax.z = vPoint.z; }

			vPoint.Set(pTri->pVtx[1]->fPos[0], pTri->pVtx[1]->fPos[1], pTri->pVtx[1]->fPos[2]);
			if (pTri->pMaterial->bStaticUV)
			{
				vPoint = pTri->pMaterial->MtxR.MultPoint(vPoint);
			}
			if (vPoint.x < vMin.x) { vMin.x = vPoint.x; }
			if (vPoint.y < vMin.y) { vMin.y = vPoint.y; }
			if (vPoint.z < vMin.z) { vMin.z = vPoint.z; }

			if (vPoint.x > vMax.x) { vMax.x = vPoint.x; }
			if (vPoint.y > vMax.y) { vMax.y = vPoint.y; }
			if (vPoint.z > vMax.z) { vMax.z = vPoint.z; }

			vPoint.Set(pTri->pVtx[2]->fPos[0], pTri->pVtx[2]->fPos[1], pTri->pVtx[2]->fPos[2]);
			if (pTri->pMaterial->bStaticUV)
			{
				vPoint = pTri->pMaterial->MtxR.MultPoint(vPoint);
			}
			if (vPoint.x < vMin.x) { vMin.x = vPoint.x; }
			if (vPoint.y < vMin.y) { vMin.y = vPoint.y; }
			if (vPoint.z < vMin.z) { vMin.z = vPoint.z; }

			if (vPoint.x > vMax.x) { vMax.x = vPoint.x; }
			if (vPoint.y > vMax.y) { vMax.y = vPoint.y; }
			if (vPoint.z > vMax.z) { vMax.z = vPoint.z; }
			
			pTri = pTri->pNext;
		} while (pTri);
		pTri = m_Clusters[i].pHead;

		// Round these numbers so that we don't have problems due to imprecision between
		// instances of geometry.  Imprecision can screw up the placement of the clusters.
		vMax.x = (f32)((s32)((vMax.x + 0.0005f) * 1000.f)) * (0.001f);
		vMax.y = (f32)((s32)((vMax.y + 0.0005f) * 1000.f)) * (0.001f);
		vMax.z = (f32)((s32)((vMax.z + 0.0005f) * 1000.f)) * (0.001f);
		vMin.x = (f32)((s32)((vMin.x + 0.0005f) * 1000.f)) * (0.001f);
		vMin.y = (f32)((s32)((vMin.y + 0.0005f) * 1000.f)) * (0.001f);
		vMin.z = (f32)((s32)((vMin.z + 0.0005f) * 1000.f)) * (0.001f);

//		DEVPRINTF( "		Pre-vMax  : %f, %f, %f\n", vMax.x, vMax.y, vMax.z );
//		DEVPRINTF( "		Pre-vMin  : %f, %f, %f\n", vMin.x, vMin.y, vMin.z );

		vN = pTri->vNrml;
		if (pTri->pMaterial->bStaticUV)
		{
			vN = pTri->pMaterial->UnitMtxR.MultDir(vN);
		}
		
		vMin.x = *fSampleSize * (f32)floor(vMin.x*fOOSampleSize);
		vMin.y = *fSampleSize * (f32)floor(vMin.y*fOOSampleSize);
		vMin.z = *fSampleSize * (f32)floor(vMin.z*fOOSampleSize);

		if ( fmath_Abs(vMin.z) < 0.000001f ) 
		{ 
			vMin.z = 0.0f; 
		}

		vMax.x = *fSampleSize * (f32)ceil(vMax.x*fOOSampleSize);
		vMax.y = *fSampleSize * (f32)ceil(vMax.y*fOOSampleSize);
		vMax.z = *fSampleSize * (f32)ceil(vMax.z*fOOSampleSize);

		Size[0] = (u32)( (vMax.x - vMin.x)*fOOSampleSize ) + 5;
		Size[1] = (u32)( (vMax.y - vMin.y)*fOOSampleSize ) + 5;
		Size[2] = (u32)( (vMax.z - vMin.z)*fOOSampleSize ) + 5;

		vecs[0].Set(0,0,0);
		vecs[1].Set(0,0,0);

		int axis;

		//
		// Determine the cluster width and height based on the min and max
		//

		if ( fabsf(vN.x) > fabsf(vN.y) && fabsf(vN.x) > fabsf(vN.z) )
		{
			w = Size[1];
			h = Size[2];

			vecs[0].y = fOOSampleSize;
			vecs[1].z = fOOSampleSize;

			axis = 0;
		}
		else if ( fabsf(vN.y) > fabsf(vN.x) && fabsf(vN.y) > fabsf(vN.z) )
		{
			w = Size[0];
			h = Size[2];

			vecs[0].x = fOOSampleSize;
			vecs[1].z = fOOSampleSize;

			axis = 1;
		}
		else
		{
			w = Size[0];
			h = Size[1];

			vecs[0].x = fOOSampleSize;
			vecs[1].y = fOOSampleSize;

			axis = 2;
		}

		pTri->fScaleX = pTri->fScaleY = 1.0f;

		if ( w > *nW )
		{
			pTri->fScaleX = (f32)(*nW)/(f32)w;
			vecs[0] *= pTri->fScaleX;
			w = (*nW);
		}

		if ( h > *nH )
		{
			pTri->fScaleY = (f32)(*nH)/(f32)h;
			vecs[1] *= pTri->fScaleY;
			h = (*nH);
		}

		// Make sure that I get atleast 1 pixel in each direction.
		if ( w < 6 )
		{
			pTri->fScaleX = 6.0f/(f32)w;
			vecs[0] *= pTri->fScaleX;
			w = 6;
		}

		if ( h < 6 )
		{
			pTri->fScaleY = 6.0f/(f32)h;
			vecs[1] *= pTri->fScaleY;
			h = 6;
		}
		//

		pTri->vMin = vMin;

		pTri->fLightMapW = (f32)w;
		pTri->fLightMapH = (f32)h;

		pT2 = pTri->pNext;

//		DEVPRINTF( "		pTri->fScaleX: %f\n", pTri->fScaleX );
//		DEVPRINTF( "		pTri->fScaleY: %f\n", pTri->fScaleY );

//		DEVPRINTF( "		vMax      : %f, %f, %f\n", vMax.x, vMax.y, vMax.z );
//		DEVPRINTF( "		pTri->vMin: %f, %f, %f\n", pTri->vMin.x, pTri->vMin.y, pTri->vMin.z );
//		DEVPRINTF( "		pTri->fLightMapW: %f\n", pTri->fLightMapW );
//		DEVPRINTF( "		pTri->fLightMapH: %f\n", pTri->fLightMapH );

		while (pT2)
		{
			pT2->fScaleX = pTri->fScaleX;
			pT2->fScaleY = pTri->fScaleY;

			pT2->vMin = pTri->vMin;
			pT2->fLightMapW = pTri->fLightMapW;
			pT2->fLightMapH = pTri->fLightMapH;
				
			pT2 = pT2->pNext;
		};
		
		//
		// Try to allocate some space in the lightmap based on the supplied width and height
		//

		CFVec3 vLightMapOrigin;
		CFVec3 vLightMapVecs[3];

		CFVec3 vDelta, vOrigin;
		f32 s, t;
		if ( AllocLMapBlock(w, h, &x, &y, nW, nH) )
		{
			pT2 = pTri;
			do
			{
				for (j=0; j<3; j++)
				{
					vPoint.Set(pT2->pVtx[j]->fPos[0], pT2->pVtx[j]->fPos[1], pT2->pVtx[j]->fPos[2]);
					if (pT2->pMaterial->bStaticUV)
					{
						vPoint = pT2->pMaterial->MtxR.MultPoint(vPoint);
					}
					vDelta.x = vPoint.x - pT2->vMin.x;
					vDelta.y = vPoint.y - pT2->vMin.y;
					vDelta.z = vPoint.z - pT2->vMin.z;

					s = vDelta.Dot(vecs[0]) + (f32)x + 0.5f;
					t = vDelta.Dot(vecs[1]) + (f32)y + 0.5f;

					pT2->pVtx[j]->fBaseUV[0] = s/(f32)(*nW);
					pT2->pVtx[j]->fBaseUV[1] = t/(f32)(*nH);
				}
				pT2 = pT2->pNext;
			} while (pT2);
		}
		else
		{
			f32 fPercentTrisSuccessful = (f32)nTrianglesInLightmap / (f32)nTotalTriangleCount;

			// The cluster won't fit in the lightmap, so we need to try again
			/*if ( fPercentTrisSuccessful > 0.95f )
			{
				// We've been able to fit the vast majority of the tris so let's try adjusting the sample size a little
				(*fSampleSize) *= (1.0f + (1.0f - fPercentTrisSuccessful));
				memset(_allocated, 0, sizeof(s32)*(*nW));
				AllocateLightMapForVolume(nVolume, nW, nH, fSampleSize);
			}
			else*/ if (*nW < 1024 || *nH < 1024)
			{
				// First we try increasing size until volume fits.
				if (*nW > *nH && *nH < 1024)
				{
					(*nH) <<= 1;
				}
				else
				{
					(*nW) <<= 1;
				}
				memset(_allocated, 0, sizeof(s32)*(*nW));
				AllocateLightMapForVolume(nVolume, nW, nH, fSampleSize);
			}
			else
			{
				// If we're already at max size, try a bigger sample size (which
				// has the effect of reducing poly sizes on the map).
				(*fSampleSize) *= 2.0f;
				memset(_allocated, 0, sizeof(s32)*(*nW));
				AllocateLightMapForVolume(nVolume, nW, nH, fSampleSize);
			}
			return TRUE;
		}

		FASSERT( x < *nW );
		FASSERT( y < *nH );

		pTri->fLightMapX = (f32)x;
		pTri->fLightMapY = (f32)y;

//		DEVPRINTF( "		pTri->fLightMapX: %f\n", pTri->fLightMapX );
//		DEVPRINTF( "		pTri->fLightMapY: %f\n", pTri->fLightMapY );

		//
		// We found space for the cluster in the lightmap, so now we need to determine individual
		// poly locations with the cluster's region of the lightmap
		//

		CFVec3 vVecA, vVecB;
		f32 fTrifD;

		vVecA = vecs[0];
		vVecB = vecs[1];

		do
		{
			vecs[0] = vVecA;
			vecs[1] = vVecB;

			vN = pTri->vNrml;
			if ( pTri->pMaterial->bStaticUV )
			{
				vN = pTri->pMaterial->UnitMtxR.MultDir( vN );
				vPoint.Set( pTri->pVtx[0]->fPos[0], pTri->pVtx[0]->fPos[1], pTri->pVtx[0]->fPos[2] );
				vPoint = pTri->pMaterial->MtxR.MultPoint( vPoint );

				fTrifD = -vN.x*vPoint.x - vN.y*vPoint.y - vN.z*vPoint.z;
			}
			else
			{
				fTrifD = pTri->fD;
			}

			if ( fmath_Abs( vN.a[axis] ) < 0.000001f )
			{
				pTri->vLightMapOrigin = vMin;
				pTri->vLightMapVecs[0] = CFVec3::m_UnitAxisX;
				pTri->vLightMapVecs[1] = CFVec3::m_UnitAxisY;
				pTri->vLightMapVecs[2] = CFVec3::m_UnitAxisZ;
			}
			else
			{
				f32 fD = vMin.Dot(vN) + fTrifD;
				fD /= vN.a[axis];

				vOrigin = vMin;
				vOrigin.a[axis] -= fD;

				CFVec3 vNz; f32 fLen;
				for (j=0; j<2; j++)
				{
					vNz = vecs[j];
					fLen = 1.0f / vNz.Mag2(); 
					
					vecs[j] = vNz * fLen;

					fD = vecs[j].Dot(vN);
					fD /= vN.a[axis];

					vecs[j].a[axis] -= fD;
				}

				FMATH_DEBUG_FCHECK( vecs[0].x );
				FMATH_DEBUG_FCHECK( vecs[0].y );
				FMATH_DEBUG_FCHECK( vecs[0].z );
				FMATH_DEBUG_FCHECK( vecs[1].x );
				FMATH_DEBUG_FCHECK( vecs[1].y );
				FMATH_DEBUG_FCHECK( vecs[1].z );
				FMATH_DEBUG_FCHECK( vN.x );
				FMATH_DEBUG_FCHECK( vN.y );
				FMATH_DEBUG_FCHECK( vN.z );

				pTri->vLightMapOrigin = vOrigin;
				pTri->vLightMapVecs[0] = vecs[0];
				pTri->vLightMapVecs[1] = vecs[1];
				pTri->vLightMapVecs[2] = vN;
			}

			pTri->fLightMapX = (f32)x;
			pTri->fLightMapY = (f32)y;
			
			GenerateLightMapMatrix( pTri, fSampleSize );

			nTrianglesInLightmap++;

			pTri = pTri->pNext;
		} while (pTri);
	}

	return TRUE;
}

void CLightingSolution::CalcSampleSize()
{
	f32 fArea;
	f32 fMagA, fMagB;
	CFVec3 vS, vT;
	LN_Triangle_t *pTri;

	fArea = 0.0f;

	u32 i, n;
	n = m_nGeoInstStart;
	if (n > m_nNumTri || n < 1) { n = m_nNumTri; }

	for (i=0; i<n; i++)
	{
		pTri = &m_TriangleList[i];
		if (pTri->pMaterial->bHasLightMap)
		{
			vS.x = pTri->pVtx[1]->fPos[0] - pTri->pVtx[0]->fPos[0];
			vS.y = pTri->pVtx[1]->fPos[1] - pTri->pVtx[0]->fPos[1];
			vS.z = pTri->pVtx[1]->fPos[2] - pTri->pVtx[0]->fPos[2];

			vT.x = pTri->pVtx[2]->fPos[0] - pTri->pVtx[0]->fPos[0];
			vT.y = pTri->pVtx[2]->fPos[1] - pTri->pVtx[0]->fPos[1];
			vT.z = pTri->pVtx[2]->fPos[2] - pTri->pVtx[0]->fPos[2];

			fMagA = vS.Mag(); fMagB = vT.Mag();

			fArea += (fMagA*fMagB);
		}
	}

	_fSampleSize = 2.0f*fArea / ((f32)m_nMaxMemory*1024.0f);

	_fSampleSize = 0.1f * ceilf(_fSampleSize/0.1f);
}

void CLightingSolution::GenerateLightMaps( BOOL bUVOnly )
{
	u32 i, j, k, w, h;
	s32 kk;
	f32 fSampleSize;
	s32 nMtlStart, nMtlEnd;

	m_nNumLightMaps = 0;

	if ( m_pDlg )
	{
		m_pDlg->ResetSubOp( "Generating Lightmap UV's...", (m_nNumVolumes * 2) + 1 );
		m_pDlg->PostMessage( CDLG_UPDATE_CONTROLS );
	}

	m_nMaxMotifIndex = 0;

	CalcSampleSize();

	_bUVOnly = bUVOnly;

	for (k=0; k<m_nNumVolumes; k++)
	{
		if ( m_pDlg )
		{
			m_pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );
		}

		memset(_allocated, 0, sizeof(s32)*LIGHTMAP_WIDTH);

		w = h = 16;
		fSampleSize = _fSampleSize;

		if (m_VolumeList[k].bStaticUV || _bUVOnly)
		{
			fSampleSize = 2.0f;
		}

		//if ( AllocateLightMapForVolume(k, &w, &h, &fSampleSize) )
		AllocateLightMapForVolume(k, &w, &h, &fSampleSize);
		if (1)
		{
			if ( m_pDlg )
			{
				m_pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );
			}

			m_pLightMapList[k] = new CLightMapTex();
			m_pLightMapList[k]->InitData(w, h, TRUE);
			m_pLightMapList[k]->SetSampleSize(fSampleSize);

			if (m_fSubSample > 0)
			{
				u32 nSubSample;

				nSubSample = (u32)( fSampleSize*m_fSubSample );

				if (nSubSample < 2) { nSubSample = 2; }
				else if (nSubSample > 64) { nSubSample = 64; }

				m_pLightMapList[k]->SetSubSamples(nSubSample);
			}
			else
			{
				m_pLightMapList[k]->SetSubSamples(0);
			}

			u32 nVolume = k;

			m_VolumeList[nVolume].nNumMotif = 0;

			nMtlStart = m_nVolumeMtlIdx[nVolume];
			if (nMtlStart > -1)
			{
				if (nVolume < (u32)(m_nNumVolumes-1))
				{
					nMtlEnd = -1;
					s32 nVolIdx = nVolume+1;
					do
					{
						if (m_nVolumeMtlIdx[nVolIdx] > -1)
						{
							nMtlEnd = m_nVolumeMtlIdx[nVolIdx]-1;
						}
						nVolIdx++;
					} while (nMtlEnd < -1 && nVolIdx < m_nNumVolumes);
					if (nMtlEnd == -1)
					{
						nMtlEnd = m_nNumMaterials-1;
					}
				}
				else
				{
					nMtlEnd = m_nNumMaterials-1;
				}

				u32 nStart = m_VolumeList[nVolume].nStartIdx;
				u32 nEnd = m_VolumeList[nVolume].nEndIdx;

				if (nEnd == 0 && (nStart > 0 || nVolume == m_nNumVolumes-1))
				{
					nEnd = m_nNumTri-1;
				}
				if (nEnd > m_nNumTri-1) nEnd = 0;

				for (j=(u32)nMtlStart; j<=(u32)nMtlEnd; j++)
				{
					GenerateMotifList(&m_MaterialList[j], nStart, nEnd);
				}
			}

			if (nMtlStart > -1)
			{
				m_VolumeList[nVolume].nNumMotif = 0;
						
				for (j=(u32)nMtlStart; j<=(u32)nMtlEnd; j++)
				{
					if (m_MaterialList[j].nNumMotif > 0)
					{
						for (kk=0; kk<m_MaterialList[j].nNumMotif; kk++)
						{	
							if (m_VolumeList[nVolume].nNumMotif == 0)
							{
								m_VolumeList[nVolume].nNumMotif = 1;
								m_VolumeList[nVolume].m_aMotifList[0].nMotifIndex = m_MaterialList[j].anMotifArray[kk];

								if (m_VolumeList[nVolume].m_aMotifList[0].nMotifIndex > m_nMaxMotifIndex) m_nMaxMotifIndex = m_VolumeList[nVolume].m_aMotifList[0].nMotifIndex;
							}
							else
							{
								if ( !ValueInArrayML(m_VolumeList[nVolume].nNumMotif, m_VolumeList[nVolume].m_aMotifList, m_MaterialList[j].anMotifArray[kk]) )
								{
									m_VolumeList[nVolume].m_aMotifList[m_VolumeList[nVolume].nNumMotif++].nMotifIndex = m_MaterialList[j].anMotifArray[kk];
									m_VolumeList[nVolume].m_aMotifList[m_VolumeList[nVolume].nNumMotif-1].nUVIndex = m_VolumeList[nVolume].nNumMotif-1;
									if (m_VolumeList[nVolume].m_aMotifList[m_VolumeList[nVolume].nNumMotif-1].nMotifIndex > m_nMaxMotifIndex) m_nMaxMotifIndex = m_VolumeList[nVolume].m_aMotifList[m_VolumeList[nVolume].nNumMotif-1].nMotifIndex;
								}
							}
						}
					}
				}

			}

			m_VolumeList[nVolume].pLightMaps[0] = m_pLightMapList[ nVolume ];
			m_VolumeList[nVolume].nNumLightMaps = m_VolumeList[nVolume].nNumMotif + 1;

			static u32 nMotifMaps = 0;
			
			for (j=(u32)nMtlStart; j<=(u32)nMtlEnd; j++)
			{
				if (m_MaterialList[j].bHasLightMap)
				{
					m_MaterialList[j].nNLightMaps = 1;
					m_MaterialList[j].pLightMaps[0] = m_pLightMapList[nVolume];
					//m_MaterialList[j].nNumMotif = 0;
				}
				else
				{
					m_MaterialList[j].nNLightMaps = 0;
					m_MaterialList[j].nNumMotif = 0;
					m_MaterialList[j].pLightMaps[0] = NULL;
				}
			}

			m_nNumLightMaps++;

			for (i=0; i<m_VolumeList[nVolume].nNumMotif; i++)
			{
				m_pLightMapList[nMotifMaps + m_nNumVolumes] = new CLightMapTex();
				m_pLightMapList[nMotifMaps + m_nNumVolumes]->InitData(w, h, TRUE);
				m_pLightMapList[nMotifMaps + m_nNumVolumes]->SetSampleSize(fSampleSize);
				m_pLightMapList[nMotifMaps + m_nNumVolumes]->SetSubSamples(0);

				m_nNumLightMaps++;

				m_VolumeList[nVolume].pLightMaps[i+1] = m_pLightMapList[ nMotifMaps + m_nNumVolumes ];

				for (kk=nMtlStart; kk<=nMtlEnd; kk++)
				{
					if (m_MaterialList[kk].bHasLightMap)
					{
						if ( ValueInArray16(m_MaterialList[kk].nNumMotif, m_MaterialList[kk].anMotifArray, m_VolumeList[nVolume].m_aMotifList[i].nMotifIndex) )
						{
							m_MaterialList[kk].pLightMaps[m_MaterialList[kk].nNLightMaps++] = m_pLightMapList[ nMotifMaps + m_nNumVolumes ];
						}
					}
				}

				nMotifMaps++;
			}

			for (u32 nMotif = 0; nMotif < m_VolumeList[nVolume].nNumMotif; nMotif++)
			{
				CopyUV_Motif(nMotif, nVolume);
			}
		}
		else
		{
			m_pLightMapList[k] = NULL;
		}
	}
}

#define _MAX_TRIS_PER_KDOP		12
#define _MAX_KDOPS				100000
BOOL CLightingSolution::BuildkDOPTree( void )
{
	u32 i, j, k;

//	OutputDebugString( "Starting to build kDOP tree...\n" );

	if ( !m_nNumTri )
	{
		return FALSE;
	}

	m_pakDOPPool = new LN_kDOP_t[_MAX_KDOPS];
	m_nkDOPPoolSize = _MAX_KDOPS;
	m_nPoolkDOPsUsed = 0;

	// First link together all of the triangles
	for ( i = 0; i < m_nNumTri-1; i++ )
	{
		m_TriangleList[i].pkDOPNext = &m_TriangleList[i+1];
	}
	m_TriangleList[i].pkDOPNext = NULL;

	// Calculate triangle vertex intervals
	for ( i = 0; i < m_nNumTri; i++ )
	{
		for ( j = 0; j < FkDOP_aDesc[LS_KDOP_TYPE].nNormalCount; j++ )
		{
			for ( k = 0; k < 3; k++ )
			{
				m_TriangleList[i].afVertIntervals[k][j] = FkDOP_aDesc[LS_KDOP_TYPE].avNormals[j].v3.Dot( *((CFVec3 *)&m_TriangleList[i].pVtx[k]->fPos) );
			}
		}
	}

	memset( &m_pakDOPPool[0], 0, sizeof ( LN_kDOP_t ) );
	m_pakDOPTree = &m_pakDOPPool[0];
	m_pakDOPTree->nTriCount = m_nNumTri;
	m_pakDOPTree->pFirstTriangle = &m_TriangleList[0];
	m_nPoolkDOPsUsed = 1;

	// Recursively build the kDOP tree
	GeneratekDOP( m_pakDOPTree, TRUE );

//	char szTemp[128];
//	sprintf( szTemp, "Finished building kDOP tree.  %d total kDOPs.\n", m_nPoolkDOPsUsed );
//	OutputDebugString( szTemp );

	return TRUE;
}

//
//
//
void CLightingSolution::GeneratekDOP( LN_kDOP_t *pkDOP, BOOL bRootkDOP )
{
	u32 i;
	f32 fAxisAverage[LS_KDOP_AXES];
	f32 fMinCentroid[3], fMaxCentroid[3];
	memset( fAxisAverage, 0, sizeof( f32 ) * (LS_KDOP_AXES) );

	fMinCentroid[0] = FMATH_MAX_FLOAT;
	fMaxCentroid[0] = -FMATH_MAX_FLOAT;
	fMinCentroid[1] = FMATH_MAX_FLOAT;
	fMaxCentroid[1] = -FMATH_MAX_FLOAT;
	fMinCentroid[2] = FMATH_MAX_FLOAT;
	fMaxCentroid[2] = -FMATH_MAX_FLOAT;

	// Determine the intervals along the axes of the kDOP
	for ( i = 0; i < FkDOP_aDesc[LS_KDOP_TYPE].nNormalCount; i++ )
	{
		f32 fMin = FMATH_MAX_FLOAT;
		f32 fMax = -FMATH_MAX_FLOAT;
		LN_Triangle_t *pTemp = pkDOP->pFirstTriangle;
		while ( pTemp )
		{
			if ( i < 3 )
			{
				f32 fCentroidInterval = FkDOP_aDesc[LS_KDOP_TYPE].avNormals[i].v3.Dot( pTemp->vCen );

				// Add in this tri's center point axis interval for later averaging
				fAxisAverage[i] += fCentroidInterval;

				if ( fCentroidInterval < fMinCentroid[i] )
				{
					fMinCentroid[i] = fCentroidInterval;
				}
				if ( fCentroidInterval > fMaxCentroid[i] )
				{
					fMaxCentroid[i] = fCentroidInterval;
				}
			}

			// Check vertex 0
			if ( pTemp->afVertIntervals[0][i] < fMin )
			{
				fMin = pTemp->afVertIntervals[0][i];
			}
			if ( pTemp->afVertIntervals[0][i] > fMax )
			{
				fMax = pTemp->afVertIntervals[0][i];
			}

			// Check vertex 1
			if ( pTemp->afVertIntervals[1][i] < fMin )
			{
				fMin = pTemp->afVertIntervals[1][i];
			}
			if ( pTemp->afVertIntervals[1][i] > fMax )
			{
				fMax = pTemp->afVertIntervals[1][i];
			}

			// Check vertex 2
			if ( pTemp->afVertIntervals[2][i] < fMin )
			{
				fMin = pTemp->afVertIntervals[2][i];
			}
			if ( pTemp->afVertIntervals[2][i] > fMax )
			{
				fMax = pTemp->afVertIntervals[2][i];
			}

			// On to the next triangle
			pTemp = pTemp->pkDOPNext;
		}

		// If our interval does not exceed some minimal threshhold, then force it to
		if ( fMin == fMax )
		{
			fMin = fMin - 0.005f;
			fMax = fMax + 0.005f;
		}

		// Calculate the average position along this axis
		fAxisAverage[i] = fAxisAverage[i] / pkDOP->nTriCount;

		// Store the intervals
		pkDOP->aIntervals[i].fMin = fMin;
		pkDOP->aIntervals[i].fMax = fMax;
	}

	// If we are already below the max tris per leaf, then this is a leaf and we
	// do not need to sub-divide the collision data any further
	if ( pkDOP->nTriCount < _MAX_TRIS_PER_KDOP )
	{
		return;
	} 

	// Don't exceed range!
	if ( m_nPoolkDOPsUsed + 2 > m_nkDOPPoolSize )
	{
		DEVPRINTF( "CLightingSolution::GeneratekDOP() - OUT OF KDOPS IN POOL!\n" );
		FASSERT_NOW;
		return;
	}

	// Subdivide this node into two child nodes

	enum
	{
		__CUT_X_AXIS = 0,
		__CUT_Y_AXIS,
		__CUT_Z_AXIS,
		__CUT_UNSPECIFIED = 0xffff,
	};

	u32 nCutAxis;

	//	Splatter Separation Implementation:
	f32 fVariance[3];
	fVariance[0] = fMaxCentroid[0] - fMinCentroid[0];
	fVariance[1] = fMaxCentroid[1] - fMinCentroid[1];
	fVariance[2] = fMaxCentroid[2] - fMinCentroid[2];

	// Now that we have the variances, determine the axis to cut along (the one with the greatest variance)
	if ( fVariance[0] > fVariance[1] && fVariance[0] > fVariance[2] )
	{
		nCutAxis = __CUT_X_AXIS;
	}
	else if ( fVariance[1] > fVariance[0] && fVariance[1] > fVariance[2] )
	{
		nCutAxis = __CUT_Y_AXIS;
	}
	else
	{
		nCutAxis = __CUT_Z_AXIS;
	}

	// Make sure that this axis results in some division of the tris
	u32 nChildTriCount[2] = { 0, 0 };
	LN_Triangle_t *pTemp = pkDOP->pFirstTriangle;
	while ( pTemp )
	{
		if ( pTemp->vCen.a[nCutAxis] < fAxisAverage[nCutAxis] )
		{
			// Give this tri to child 1
			nChildTriCount[0]++;
		}
		else
		{
			// Give this tri to child 2
			nChildTriCount[1]++;
		}

		if ( nChildTriCount[0] != 0 && nChildTriCount[1] != 0 )
		{
			break;
		}

		pTemp = pTemp->pkDOPNext;
	}

	// If we didn't get separation, then stop trying
	if ( pTemp == NULL )
	{
		return;
	}

	pkDOP->papChildren[0] = &m_pakDOPPool[m_nPoolkDOPsUsed++];
	pkDOP->papChildren[1] = &m_pakDOPPool[m_nPoolkDOPsUsed++];

	memset( pkDOP->papChildren[0], 0, sizeof ( LN_kDOP_t ) );
	memset( pkDOP->papChildren[1], 0, sizeof ( LN_kDOP_t ) );

	// Distribute all of the tris to the appropriate child dividing along the mean
	LN_Triangle_t *pChildLast[2];
	pChildLast[0] = NULL;
	pChildLast[1] = NULL;
	pTemp = pkDOP->pFirstTriangle;
	while ( pTemp )
	{
		u32 nAssignIdx;

		if ( pTemp->vCen.a[nCutAxis] < fAxisAverage[nCutAxis] )
		{
			// Give this tri to child 1
			nAssignIdx = 0;
		}
		else
		{
			// Give this tri to child 2
			nAssignIdx = 1;
		}

		if ( pkDOP->papChildren[nAssignIdx]->pFirstTriangle != NULL )
		{
			pChildLast[nAssignIdx]->pkDOPNext = pTemp;
		}
		else
		{
			pkDOP->papChildren[nAssignIdx]->pFirstTriangle = pTemp;
		}

		pkDOP->papChildren[nAssignIdx]->nTriCount++;
		pChildLast[nAssignIdx] = pTemp;

		pTemp = pTemp->pkDOPNext;
	}

	// We should have had SOME division
	FASSERT( pChildLast[0] && pChildLast[1] );

	pChildLast[0]->pkDOPNext = NULL;
	pChildLast[1]->pkDOPNext = NULL;

	// The total of all the child tris should equal that of the parent
	FASSERT( pkDOP->papChildren[0]->nTriCount + pkDOP->papChildren[1]->nTriCount == pkDOP->nTriCount );

	// Since we distributed all of the tris, set the parent node to have no tris
	pkDOP->nTriCount = 0;
	pkDOP->pFirstTriangle = NULL;

	// Call GeneratekDOP() per child
	GeneratekDOP( pkDOP->papChildren[0], FALSE );
	GeneratekDOP( pkDOP->papChildren[1], FALSE );
}
