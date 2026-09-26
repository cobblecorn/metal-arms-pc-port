#include "StdAfx.h"
#include "..\Pasm.h"
#include "..\CompileDlg.h"
#include "RayTracer.h"
#include "LightmapGen.h"

CRayTracer::CRayTracer(void):CLightingSolution()
{
	m_fMaxLightEffectMult = 2.0f; //this is specific to lighting solutions
	m_bFilterRayTrace = FALSE;
}

CRayTracer::~CRayTracer(void)
{
	CLightingSolution::~CLightingSolution();
}

void CRayTracer::CalculateDialogUpdateParams()
{
	m_fUpdateCount = 0.f;
	m_fUpdateInterval = (f32)(m_nNumTri * (m_nMaxMotifIndex + 1)) / (f32)LMG_NUMBER_OF_PROGRESS_BAR_UPDATES;
}

BOOL CRayTracer::Build(f32 fAmbRed, f32 fAmbGreen, f32 fAmbBlue)
{
	BOOL bRetVal = TRUE;

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
		m_pDlg->ResetSubOp( "Applying Raytracing...", LMG_NUMBER_OF_PROGRESS_BAR_UPDATES + 1 );
		m_pDlg->PostMessage( CDLG_UPDATE_CONTROLS );
	}

	u32 i;
	for (i=0; i<=m_nMaxMotifIndex; i++)
	{
		WPRINT_BEGIN
		WPRINT("BakeLighting - Motif = %d.\n", i);
		WPRINT_END
		BakeLighting(i);
	}

	WPRINT_BEGIN
	WPRINT("OutputLightMaps...\n");
	WPRINT_END
	OutputLightMaps();

	return (bRetVal);
}

u32 CRayTracer::PartitionWorld()
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

typedef struct
{
	CFVec2 vOffs;
	CFVec2 vA, vB;
	f32 fPosA[3], fPosB[3];
	f32 fMinV, fMaxV;
} _EDGE_t;

void CRayTracer::BakeLighting(u16 nMotif)
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
			}

			if (fMinV < 0.0f) fMinV = 0.0f;
			fCurV = fMinV;
			u8 nIdx=0;
			CFVec2 vI[2]; //intersections.
			CFVec3 vI_Pos[3];
			CFVec3 vStart[7];
			static f32 fOO7 = (1.0f/7.0f);
			f32 fRA;
			u8 nRayTrace;
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
							vI_Pos[nIdx].x = Edge[i].fPosA[0] + fM*(Edge[i].fPosB[0] - Edge[i].fPosA[0]);
							vI_Pos[nIdx].y = Edge[i].fPosA[1] + fM*(Edge[i].fPosB[1] - Edge[i].fPosA[1]);
							vI_Pos[nIdx].z = Edge[i].fPosA[2] + fM*(Edge[i].fPosB[2] - Edge[i].fPosA[2]);
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
		
						CFVec3 v3Tmp = vI_Pos[0];
						vI_Pos[0] = vI_Pos[1];
						vI_Pos[1] = v3Tmp;
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
									vStart[0].Set(fX, fY, fZ);
									
									nRayTrace = RayTrace(pTri, pLight, vStart[0], pLight->vPos);
									if (m_bFilterRayTrace)
									{
										vStart[1].Set(fX+fDeltaX, fY, fZ);
										vStart[2].Set(fX-fDeltaX, fY, fZ);
										vStart[3].Set(fX, fY+fDeltaY, fZ);
										vStart[4].Set(fX, fY-fDeltaY, fZ);
										vStart[5].Set(fX, fY, fZ+fDeltaZ);
										vStart[6].Set(fX, fY, fZ-fDeltaZ);

										nRayTrace += RayTrace(pTri, pLight, vStart[1], pLight->vPos);
										nRayTrace += RayTrace(pTri, pLight, vStart[2], pLight->vPos);
										nRayTrace += RayTrace(pTri, pLight, vStart[3], pLight->vPos);
										nRayTrace += RayTrace(pTri, pLight, vStart[4], pLight->vPos);
										nRayTrace += RayTrace(pTri, pLight, vStart[5], pLight->vPos);
										nRayTrace += RayTrace(pTri, pLight, vStart[6], pLight->vPos);
									}
																	
									if ( nRayTrace )
									{
										vDiffVec.x = pLight->vPos.x - fX;
										vDiffVec.y = pLight->vPos.y - fY;
										vDiffVec.z = pLight->vPos.z - fZ;

										fDist2 = vDiffVec.Dot(vDiffVec);

										fA = pLight->fIntensity / (pLight->vDAtten.x + pLight->vDAtten.z*fDist2);
										if (m_bFilterRayTrace)
										{
											fRA = (f32)nRayTrace*fOO7;
											fA *= fRA;
										}

										afClr[2] += (fA * pLight->vColor.x);
										afClr[1] += (fA * pLight->vColor.y);
										afClr[0] += (fA * pLight->vColor.z);
									}
								}
							}

							ClampF3( afClr );

							pTri->pMaterial->pLightMaps[ nLMap ]->SetColor(afClr, fCurU, fCurV);

							fCurU += fDelta;
						};
					}
				}
				fCurV += fDelta;
			};
		}
	}
}

BOOL CRayTracer::PointInsideTri(LN_Triangle_t *pTri, CFVec3& vPoint)
{
	u8 nEdge, nA, nB;
	f32 fDot;
	LN_Vertex_t *pVtx[3];
	pVtx[0] = &m_VertexList[ pTri->nVtx[0] ];
	pVtx[1] = &m_VertexList[ pTri->nVtx[1] ];
	pVtx[2] = &m_VertexList[ pTri->nVtx[2] ];

	if (pTri->vNrml.x >= pTri->vNrml.y && pTri->vNrml.x >= pTri->vNrml.z)
	{
		//YZ
		for (nEdge=0; nEdge<3; nEdge++)
		{
			nA = nEdge; nB = (nA+1)%3;
			fDot = (vPoint.y-pVtx[nA]->fPos[1])*(pVtx[nA]->fPos[2]-pVtx[nB]->fPos[2]) + (vPoint.z-pVtx[nA]->fPos[2])*(pVtx[nB]->fPos[1]-pVtx[nA]->fPos[1]);
			if (fDot < 0.00001f) return (FALSE);
		}
	}
	else if (pTri->vNrml.y >= pTri->vNrml.x && pTri->vNrml.y >= pTri->vNrml.z)
	{
		//XZ
		for (nEdge=0; nEdge<3; nEdge++)
		{
			nA = nEdge; nB = (nA+1)%3;
			fDot = (vPoint.x-pVtx[nA]->fPos[0])*(pVtx[nA]->fPos[2]-pVtx[nB]->fPos[2]) + (vPoint.z-pVtx[nA]->fPos[2])*(pVtx[nB]->fPos[0]-pVtx[nA]->fPos[0]);
			if (fDot < 0.00001f) return (FALSE);
		}
	}
	else
	{
		//XY
		for (nEdge=0; nEdge<3; nEdge++)
		{
			nA = nEdge; nB = (nA+1)%3;
			fDot = (vPoint.x-pVtx[nA]->fPos[0])*(pVtx[nA]->fPos[1]-pVtx[nB]->fPos[1]) + (vPoint.y-pVtx[nA]->fPos[1])*(pVtx[nB]->fPos[0]-pVtx[nA]->fPos[0]);
			if (fDot < 0.00001f) return (FALSE);
		}
	}
	    
	return (TRUE);
}

BOOL CRayTracer::TriEffectedByLight(LN_Triangle_t *pTri, LN_Light_t *pLight)
{
	BOOL bRet = FALSE;

	for (u32 i=0; i<pTri->nNumLights; i++)
	{
		if (pTri->pLights[i] == pLight)
		{
			bRet = TRUE;
			break;
		}
	}

	return (bRet);
}

BOOL CRayTracer::PointInsideTriSphere(LN_Triangle_t *pTri, CFVec3& vPoint)
{
	CFVec3 vOffs = pTri->vCen - vPoint;
	if (vOffs.Mag2() > (pTri->fRadius * pTri->fRadius))
	{
		return (FALSE);
	}
	return (TRUE);
}

u8 CRayTracer::RayTrace(LN_Triangle_t *pTri, LN_Light_t *pLight, CFVec3& vStart, CFVec3& vEnd)
{
	u8 nRet = _RAY_PASS;
	LN_Triangle_t *pCurTri;

	static LN_Triangle_t *_pPrevTri=NULL;
	static LN_Light_t *_pPrevLight=NULL;
	static LN_Triangle_t *_pPVS[10000];
	static u32 _nNumPVS=0;

	CFVec3 vDp, vPoint;
	f32 fU, fDenom;
	u32 i;

	vDp = vEnd - vStart;

	if (_pPrevTri != pTri || _pPrevLight != pLight)
	{
		_nNumPVS = 0;
		for (i=0; i<m_nNumTri; i++)
		{
			pCurTri = &m_TriangleList[i];

			if (!pCurTri->pMaterial->bRayTest) { continue; }

			if (pCurTri != pTri && TriEffectedByLight(pCurTri, pLight))
			{
				_pPVS[_nNumPVS++ ] = pCurTri;
				//1. Find intersection of Ray with triangle plane.
				fDenom = pCurTri->vNrml.x*vDp.x + pCurTri->vNrml.y*vDp.y + pCurTri->vNrml.z*vDp.z;
				if (fDenom)
				{
					fU = -(pCurTri->fD + pCurTri->vNrml.x*vStart.x + pCurTri->vNrml.y*vStart.y + pCurTri->vNrml.z*vStart.z)/(fDenom);
					if (fU >= 0.001f && fU <= 1.0f)
					{
						vPoint.x = vStart.x + fU*vDp.x;
						vPoint.y = vStart.y + fU*vDp.y;
						vPoint.z = vStart.z + fU*vDp.z;

						if (PointInsideTriSphere(pCurTri, vPoint))
						{
							if ( PointInsideTri(pCurTri, vPoint) )
							{
								nRet = _RAY_HIT;
								break;
							}
						}
					}
				}
			}
		}
	}
	else
	{
		for (i=0; i<_nNumPVS; i++)
		{
			pCurTri = _pPVS[i];
			//1. Find intersection of Ray with triangle plane.
			fDenom = pCurTri->vNrml.x*vDp.x + pCurTri->vNrml.y*vDp.y + pCurTri->vNrml.z*vDp.z;
			if (fDenom)
			{
				fU = -(pCurTri->fD + pCurTri->vNrml.x*vStart.x + pCurTri->vNrml.y*vStart.y + pCurTri->vNrml.z*vStart.z)/(fDenom);
				if (fU >= 0.001f && fU <= 1.0f)
				{
					vPoint.x = vStart.x + fU*vDp.x;
					vPoint.y = vStart.y + fU*vDp.y;
					vPoint.z = vStart.z + fU*vDp.z;

					if (PointInsideTriSphere(pCurTri, vPoint))
					{
						if ( PointInsideTri(pCurTri, vPoint) )
						{
							nRet = _RAY_HIT;
							break;
						}
					}
				}
			}
		}
	}

	_pPrevTri = pTri;
	_pPrevLight = pLight;

	return (nRet);
}
