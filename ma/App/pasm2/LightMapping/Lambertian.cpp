#include "StdAfx.h"
#include "..\Pasm.h"
#include "..\CompileDlg.h"
#include "Lambertian.h"
#include "LightmapGen.h"

#include <math.h>

CLambertian::CLambertian(void):CLightingSolution()
{
	m_fMaxLightEffectMult = 2.0f; //this is specific to lighting solutions
}

CLambertian::~CLambertian(void)
{
	CLightingSolution::~CLightingSolution();
}
#define _DETAIL_VERTEX 0

u32 CLambertian::PartitionWorld()
{
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
		vEdge0.Unitize();
		vEdge1.Set(pVtx[2]->fPos[0]-pVtx[0]->fPos[0], pVtx[2]->fPos[1]-pVtx[0]->fPos[1], pVtx[2]->fPos[2]-pVtx[0]->fPos[2]);
		vEdge1.Unitize();

		pTri->vNrml = vEdge0.Cross(vEdge1);
		pTri->vNrml.Unitize();

		pTri->fD = -(pTri->vNrml.x*pVtx[0]->fPos[0] + pTri->vNrml.y*pVtx[0]->fPos[1] + pTri->vNrml.z*pVtx[0]->fPos[2]);
	}

	//2. Partition world geometry into cells for raytracing
	//Build KDop Tree HERE.
	//

	return (0);
}

void CLambertian::CalculateDialogUpdateParams()
{
	m_fUpdateCount = 0.f;
	m_fUpdateInterval = (f32)(m_nNumTri * (m_nMaxMotifIndex + 1)) / (f32)(LMG_NUMBER_OF_PROGRESS_BAR_UPDATES - m_nNumVolumes);
}

BOOL CLambertian::Build(f32 fAmbRed, f32 fAmbGreen, f32 fAmbBlue)
{
	BOOL bRetVal = TRUE;
	CFVec3 vDiffVec;

	WPRINT_BEGIN
	WPRINT("Partition World...\n");
	WPRINT_END

	PartitionWorld();

	WPRINT_BEGIN
	WPRINT("GenerateLightMaps...\n");
	WPRINT_END

	GenerateLightMaps();

	// Must occur after light maps are generated
	CalculateDialogUpdateParams();

	if ( m_pDlg )
	{
		m_pDlg->ResetSubOp( "Applying distance based lighting...", LMG_NUMBER_OF_PROGRESS_BAR_UPDATES + 1 );
		m_pDlg->PostMessage( CDLG_UPDATE_CONTROLS );
	}

#if _DETAIL_VERTEX
	//Light each vertex.
	for (nVtx=0; nVtx<m_nNumVtx; nVtx++)
	{
		pVtx = &m_VertexList[nVtx];
		for (nLight=0; nLight<m_nNumLights; nLight++)
		{
			pLight = &m_LightList[nLight];
			vDiffVec.x = pLight->vPos.x - pVtx->fPos[0];
			vDiffVec.y = pLight->vPos.y - pVtx->fPos[1];
			vDiffVec.z = pLight->vPos.z - pVtx->fPos[2];

			fDist2 = vDiffVec.Dot(vDiffVec);

			fA = pLight->fIntensity / (pLight->vDAtten.x + pLight->vDAtten.z*fDist2);

			pVtx->fClr[0] += (fA * pLight->vColor.x);
			pVtx->fClr[1] += (fA * pLight->vColor.y);
			pVtx->fClr[2] += (fA * pLight->vColor.z);
		 }

		ClampF3( pVtx->fClr );
	}
	//
#endif

	u32 i;

	if ( m_nQuality == 0xff)
	{
		WPRINT_BEGIN
		WPRINT("BakeLighting - Motif = %d.\n", i);
		WPRINT_END

		RenderClusters();
	}
	else
	{
		for (i=0; i<=m_nMaxMotifIndex; i++)
		{
			WPRINT_BEGIN
			WPRINT("BakeLighting - Motif = %d.\n", i);
			WPRINT_END
			BakeLighting(i);
		}
	}

	WPRINT_BEGIN
	WPRINT("OutputLightMaps...\n");
	WPRINT_END
	OutputLightMaps();

	return (bRetVal);
}

void CLambertian::AddTriangle(s32 nVolumeIdx, u32 nV0, u32 nV1, u32 nV2)
{
	//for more advanced lighting systems, put additional data here.
	CLightingSolution::AddTriangle(nVolumeIdx, nV0, nV1, nV2);
}

void CLambertian::AddVertex(LS_LightingVtx_t Vtx)
{
	//for more advanced lighting systems, put additional data here.
	CLightingSolution::AddVertex(Vtx);
}

void CLambertian::SetVtxGroup(u32 nGroup)
{
	//for more advanced lighting systems, put additional data here.
	CLightingSolution::SetVtxGroup(nGroup);
}

void CLambertian::SetTriGroup(u32 nGroup)
{
	//for more advanced lighting systems, put additional data here.
	CLightingSolution::SetTriGroup(nGroup);
}

void CLambertian::OffsetTriIndexs(u32 nGroup, u32 nStartIdx, u32 nSize, u32 nOffs)
{
	//for more advanced lighting systems, put additional data here.
	CLightingSolution::OffsetTriIndexs(nGroup, nStartIdx, nSize, nOffs);
}

void CLambertian::AddLight(u32 nVolume, LS_LightType_e LightType, CFVec3& vDir, CFVec3& vPos, CFVec3& vDAtten, CFVec3& vAAtten, CFVec3& vColor, f32 fColorIntens, f32 fAttenIntens, f32 fOOR2, u16 nMotifIndex)
{
	//for more advanced lighting systems, put additional data here.
	CLightingSolution::AddLight(nVolume, LightType, vDir, vPos, vDAtten, vAAtten, vColor, fColorIntens, fAttenIntens, fOOR2, nMotifIndex);
}

typedef struct
{
	CFVec2 vOffs;
	CFVec2 vA, vB;
#if _DETAIL_VERTEX
	f32 fClrA[3], fClrB[3];
#else
	f32 fPosA[3], fPosB[3];
#endif
	f32 fMinV, fMaxV;
} _EDGE_t;

void CLambertian::RenderClusters()
{
	f32 afClr[4]={1.0f,1.0f,1.0f,1.0f};
	u32 nCluster;
	f32 fMinV=1.0f, fMaxV=0.0f, fDelta;
	f32 fWm1;
	f32 fOOW, fD;
	f32 fU0, fU1, fU, fV;
	LN_Cluster_t *pCluster;

	fWm1 = (f32)(m_VolumeList[0].pLightMaps[0]->GetWidth()-1);
	fOOW = (1.0f/(f32)(m_VolumeList[0].pLightMaps[0]->GetWidth()-1));
	fDelta = (1.0f/(f32)(m_VolumeList[0].pLightMaps[0]->GetWidth()));

	fD = fDelta*0.5f;

	for (nCluster=0; nCluster<m_nClusters; nCluster++)
	{
		pCluster = &m_Clusters[nCluster];

		fMinV = pCluster->vMin.y;
		fMaxV = pCluster->vMax.y;

		fU0 = pCluster->vMin.x;
		fU1 = pCluster->vMax.x;

		if (fMinV >= 0.0f && fMinV <= 1.0f && fMaxV >= 0.0f && fMaxV <= 1.0f && fU0 >= 0.0f && fU0 <= 1.0f && fU1 >= 0.0f && fU1 <= 1.0f)
		{
			fU = (f32)nCluster/(f32)(m_nClusters-1);
			afClr[0] = fU;
			afClr[1] = fmodf(fU*100, 1.0f);
			afClr[2] = 1.0f - fU;
			
			fV = fMinV - fD;
			do
			{
				fU = fU0 - fD;
				do
				{
					m_VolumeList[0].pLightMaps[0]->SetColor(afClr, fU, fV);
					fU += fDelta;
				} while (fU < fU1 + fD);

				fV += fDelta;
			} while (fV < fMaxV + fD);
		}
	}
}

void CLambertian::BakeLighting(u16 nMotif)
{
	f32 afClr[4]={1.0f,1.0f,1.0f,1.0f}, fM;
	u32 nTri, i, nUVSet;
	u16 nLMap;
	_EDGE_t Edge[3];
	f32 fMinV=1.0f, fMaxV=0.0f, fDelta, fCurU, fCurV;
	f32 fWm1;
	f32 fOOW;
	f32 fEndU;
	LN_Triangle_t *pTri;

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

		pTri = &m_TriangleList[nTri];

		if (!pTri->pMaterial->bHasLightMap) continue;
		
		nLMap = GetMtlMotifIdx(pTri->pMaterial, nMotif);

		fWm1 = (f32)(pTri->pMaterial->pLightMaps[ nLMap ]->GetWidth()-1);
		fOOW = (1.0f/(f32)(pTri->pMaterial->pLightMaps[ nLMap ]->GetWidth()-1));
		fDelta = (1.0f/(f32)(pTri->pMaterial->pLightMaps[ nLMap ]->GetWidth()));

		fMinV=1.0f; fMaxV=0.0f;
		
		if ( ValueInArray16(pTri->pMaterial->nNumMotif, pTri->pMaterial->anMotifArray, nMotif) || nMotif == 0 )
		{
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

			#if _DETAIL_VERTEX
				Edge[i].fClrA[0] = m_VertexList[ pTri->nVtx[i] ].fClr[0];
				Edge[i].fClrA[1] = m_VertexList[ pTri->nVtx[i] ].fClr[1];
				Edge[i].fClrA[2] = m_VertexList[ pTri->nVtx[i] ].fClr[2];

				Edge[i].fClrB[0] = m_VertexList[ pTri->nVtx[(i+1)%3] ].fClr[0];
				Edge[i].fClrB[1] = m_VertexList[ pTri->nVtx[(i+1)%3] ].fClr[1];
				Edge[i].fClrB[2] = m_VertexList[ pTri->nVtx[(i+1)%3] ].fClr[2];
			#else
				Edge[i].fPosA[0] = m_VertexList[ pTri->nVtx[i] ].fPos[0];
				Edge[i].fPosA[1] = m_VertexList[ pTri->nVtx[i] ].fPos[1];
				Edge[i].fPosA[2] = m_VertexList[ pTri->nVtx[i] ].fPos[2];

				Edge[i].fPosB[0] = m_VertexList[ pTri->nVtx[(i+1)%3] ].fPos[0];
				Edge[i].fPosB[1] = m_VertexList[ pTri->nVtx[(i+1)%3] ].fPos[1];
				Edge[i].fPosB[2] = m_VertexList[ pTri->nVtx[(i+1)%3] ].fPos[2];
			#endif

				Edge[i].vOffs = Edge[i].vB - Edge[i].vA;
				Edge[i].fMinV = (Edge[i].vA.y < Edge[i].vB.y)?(Edge[i].vA.y):(Edge[i].vB.y);
				Edge[i].fMaxV = (Edge[i].vA.y > Edge[i].vB.y)?(Edge[i].vA.y):(Edge[i].vB.y);

				if (Edge[i].fMinV < fMinV) fMinV = Edge[i].fMinV;
				if (Edge[i].fMaxV > fMaxV) fMaxV = Edge[i].fMaxV;
			}
			if (fMinV < 0.0f || fMinV > 1.0f || fMaxV < 0.0f || fMaxV > 1.0f) { continue; }

			if (fMinV < 0.0f) fMinV = 0.0f;
			fCurV = fMinV;
			u8 nIdx=0;
			CFVec2 vI[2]; //intersections.
		#if _DETAIL_VERTEX
			CFVec3 vI_Clr[2];
		#else
			CFVec3 vI_Pos[3];
		#endif
			while (fCurV <= fMaxV)
			{
				nIdx=0;
				for (i=0; i<3; i++)
				{	
					if (fCurV >= Edge[i].fMinV && fCurV <= Edge[i].fMaxV && (Edge[i].fMinV != Edge[i].fMaxV)) //horizontal ray intersects edge.
					{
						fM = (fCurV - Edge[i].vA.y)/(Edge[i].vOffs.y);
						if (fM >= 0.0f && fM <= 1.0f)
						{
							fCurU = Edge[i].vA.x + fM*(Edge[i].vOffs.x);

							vI[nIdx].x = fCurU; vI[nIdx].y = fCurV;
						#if _DETAIL_VERTEX
							vI_Clr[nIdx].x = Edge[i].fClrA[0] + fM*(Edge[i].fClrB[0] - Edge[i].fClrA[0]);
							vI_Clr[nIdx].y = Edge[i].fClrA[1] + fM*(Edge[i].fClrB[1] - Edge[i].fClrA[1]);
							vI_Clr[nIdx].z = Edge[i].fClrA[2] + fM*(Edge[i].fClrB[2] - Edge[i].fClrA[2]);
						#else
							vI_Pos[nIdx].x = Edge[i].fPosA[0] + fM*(Edge[i].fPosB[0] - Edge[i].fPosA[0]);
							vI_Pos[nIdx].y = Edge[i].fPosA[1] + fM*(Edge[i].fPosB[1] - Edge[i].fPosA[1]);
							vI_Pos[nIdx].z = Edge[i].fPosA[2] + fM*(Edge[i].fPosB[2] - Edge[i].fPosA[2]);
						#endif
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
						CFVec2 v2Tmp = vI[0];
						vI[0] = vI[1];
						vI[1] = v2Tmp;
		
					#if _DETAIL_VERTEX
						CFVec3 v3Tmp = vI_Clr[0];
						vI_Clr[0] = vI_Clr[1];
						vI_Clr[1] = v3Tmp;
					#else
						CFVec3 v3Tmp = vI_Pos[0];
						vI_Pos[0] = vI_Pos[1];
						vI_Pos[1] = v3Tmp;
					#endif
					}

					if (vI[0].x != vI[1].x)
					{
						fCurU = vI[0].x;
						fEndU = vI[1].x;

						//Make sure my scalines are not too short, round <- ..... ->
						fCurU = (f32)( (s32)(fCurU*fWm1 - 0.5f) );
						fCurU = fCurU * fOOW;
						if (fCurU < 0.0f) fCurU = 0.0f;

						fEndU = (f32)( (s32)(fEndU*fWm1 + 0.5f) );
						fEndU = fEndU * fOOW;
						if (fEndU > 1.0f) fEndU = 1.0f;

						afClr[3] = 1.0f;
					#if _DETAIL_VERTEX

						f32 fDeltaM = (fDelta)/(vI[1].x - vI[0].x);
						f32 fDeltaR = fDeltaM*(vI_Clr[1].x - vI_Clr[0].x);
						f32 fDeltaG = fDeltaM*(vI_Clr[1].y - vI_Clr[0].y);
						f32 fDeltaB = fDeltaM*(vI_Clr[1].z - vI_Clr[0].z);
						fM = 0.0f;

						afClr[0] = vI_Clr[0].x;
						afClr[1] = vI_Clr[0].y;
						afClr[2] = vI_Clr[0].z;

						while (fCurU <= fEndU)
						{
							afClr[0] += fDeltaR;
							afClr[1] += fDeltaG;
							afClr[2] += fDeltaB;

							m_pLightMapList[0]->SetColor(afClr, fCurU, fCurV);

							fCurU += fDelta;
						};
					#else
						f32 fDeltaM = (fDelta)/(vI[1].x - vI[0].x);
						f32 fDeltaX = fDeltaM*(vI_Pos[1].x - vI_Pos[0].x);
						f32 fDeltaY = fDeltaM*(vI_Pos[1].y - vI_Pos[0].y);
						f32 fDeltaZ = fDeltaM*(vI_Pos[1].z - vI_Pos[0].z);

						f32 fX=vI_Pos[0].x, fY=vI_Pos[0].y, fZ=vI_Pos[0].z, fDist2, fA;
						u32 nLight;
						LN_Light_t *pLight;
						CFVec3 vDiffVec;

						while (fCurU <= vI[1].x)
						{
							fX += fDeltaX;
							fY += fDeltaY;
							fZ += fDeltaZ;

							afClr[0] = afClr[1] = afClr[2] = 0.0f;

							for (nLight=0; nLight<pTri->nNumLights; nLight++)
							{
								pLight = pTri->pLights[nLight];
								if (pLight->nMotifIndex == nMotif)
								{
									vDiffVec.x = pLight->vPos.x - fX;
									vDiffVec.y = pLight->vPos.y - fY;
									vDiffVec.z = pLight->vPos.z - fZ;

									fDist2 = vDiffVec.Dot(vDiffVec);

									fA = pLight->fIntensity / (pLight->vDAtten.x + pLight->vDAtten.z*fDist2);

									afClr[2] += (fA * pLight->vColor.x);
									afClr[1] += (fA * pLight->vColor.y);
									afClr[0] += (fA * pLight->vColor.z);
								}
							}

							ClampF3( afClr );

							if ( m_nQuality < 0xff )
							{
								pTri->pMaterial->pLightMaps[ nLMap ]->SetColor(afClr, fCurU, fCurV);
							}
							else
							{
								afClr[1] = 1.0f; afClr[2] = 1.0f; afClr[0] = 1.0f; afClr[3] = 1.0f;
								pTri->pMaterial->pLightMaps[ nLMap ]->SetColor(afClr, fCurU, fCurV);
							}

							fCurU += fDelta;
						};
					#endif
					}
				}

				fCurV += fDelta;
			};
		}
	}
}
