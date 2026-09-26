#include "LightMapTex.h"

#include <stdio.h>
#include <string.h>
#include <io.h>
#include "fclib.h"

#include <math.h>

#define LM_FULL_PATH 0
char CLightMapTex::m_szLocalDir[256];
//CLightMapTex::BLEND_EDGE CLightMapTex::m_Edges[1000000];

u8 CLightMapTex::m_nMinRGB[4]={0x00, 0x00, 0x00, 0x00};

CLightMapTex::CLightMapTex(void)
{
	m_pData = NULL;
	m_pPos = NULL;
	m_nWidth = 0;
	m_nHeight = 0;
	m_nEdges = 0;
	m_bInit = FALSE;
	m_pTGALoader = NULL;
	m_TriPtr = NULL;
}

CLightMapTex::~CLightMapTex(void)
{
	if (m_pData)
	{
		delete [] m_pData;
		m_pData = NULL;
	}
	if (m_pPos)
	{
		free(m_pPos);
		m_pPos = NULL;
	}
	if (m_TriPtr)
	{
		free(m_TriPtr);
		m_TriPtr = NULL;
	}
	if (m_pTGALoader)
	{
		delete m_pTGALoader;
		m_pTGALoader = NULL;
	}
}

BOOL CLightMapTex::InitData(u32 nWidth, u32 nHeight, BOOL bPos)
{
	m_nWidth = nWidth; m_nHeight = nHeight;

	m_fOOWm1 = 1.0f / (f32)(m_nWidth-1);
	m_fOOHm1 = 1.0f / (f32)(m_nHeight-1);

	m_pData = new u32[nWidth*nHeight];
	m_TriPtr = (void **)malloc(4*nWidth*nHeight);

	memset(m_pData, 0x0, nWidth*nHeight*4);

	if (bPos)
	{
		//m_pPos = new CFVec3[nWidth*nHeight];
		m_pPos = (CFVec3*)malloc(sizeof(CFVec3)*nWidth*nHeight);
		memset(m_pPos, 0x0, nWidth*nHeight*sizeof(CFVec3));
	}
	else
	{
		m_pPos = NULL;
	}

	if (m_TriPtr)
	{
		memset(m_TriPtr, 0, 4*nWidth*nHeight);
	}

	if (!m_pData) 
	{
		return (FALSE);
	}

	m_bInit = TRUE;
	return (TRUE);
}

void CLightMapTex::GetColor(f32 *pfClr, f32 fU, f32 fV)
{
	if (m_bInit)
	{
		u32 nU, nV;
		nU = (u32)fU;//( fU*(f32)(m_nWidth-1) );
		nV = (u32)fV;//( fV*(f32)(m_nHeight-1) );

		if (nU > (u32)(m_nWidth-1)) nU = m_nWidth-1;
		if (nV > (u32)(m_nHeight-1)) nV = m_nHeight-1;

		if (nU < 0) nU = 0;
		if (nV < 0) nV = 0;

		GetColor(pfClr, nU, nV);
	}
}

f32 _fOO255 = (1.0f/255.0f);

void CLightMapTex::GetColor(f32 *pfClr, u32 nU, u32 nV)
{
	u32 nOffs = nV*m_nWidth + nU;
	u8 *pValue;

	pValue = (u8 *)&m_pData[nOffs];
	pfClr[0] = pValue[0] * _fOO255;
	pfClr[1] = pValue[1] * _fOO255;
	pfClr[2] = pValue[2] * _fOO255;
	pfClr[3] = pValue[3] * _fOO255;
}

void CLightMapTex::ConvertUV(f32& fU, f32& fV, s32 nAdd)
{
	s32 nU, nV;

	nU = (u32)( fU*(f32)(m_nWidth-1)+0.5f );
	nV = (u32)( fV*(f32)(m_nHeight-1)+0.5f );

	fU = (f32)(nU+nAdd) * m_fOOWm1;
	fU = (fU < 1.0f)?(fU):(1.0f);
	fV = (f32)(nV+nAdd) * m_fOOHm1;
	fV = (fV < 1.0f)?(fV):(1.0f);
}

void CLightMapTex::ConvertU(f32& fU, s32 nAdd)
{
	s32 nU;
	nU = (u32)( fU*(f32)(m_nWidth-1)+0.5f );
	fU = (f32)(nU+nAdd) * m_fOOWm1;

	fU = (fU < 1.0f)?(fU):(1.0f);
}

void CLightMapTex::ConvertV(f32& fV, s32 nAdd)
{
	s32 nV;
	nV = (u32)( fV*(f32)(m_nHeight-1)+0.5f );
	fV = (f32)(nV+nAdd) * m_fOOHm1;

	fV = (fV < 1.0f)?(fV):(1.0f);
}

void CLightMapTex::SetTriPtr(f32 fU, f32 fV, void *ptr)
{
	u32 nU, nV;
		
	nU = (u32)fU;
	nV = (u32)fV;

	if (nU > (u32)(m_nWidth-1)) nU = m_nWidth-1;
	if (nV > (u32)(m_nHeight-1)) nV = m_nHeight-1;

	if (nU < 0) nU = 0;
	if (nV < 0) nV = 0;

	m_TriPtr[nV*m_nWidth + nU] = ptr;
}

void CLightMapTex::SetColor(f32 *pfClr, f32 fU, f32 fV, CFVec3 *pPos)
{
	if (m_bInit)
	{
		u32 nU, nV;
		
		nU = (u32)fU;
		nV = (u32)fV;

		if (nU > (u32)(m_nWidth-1)) nU = m_nWidth-1;
		if (nV > (u32)(m_nHeight-1)) nV = m_nHeight-1;

		if (nU < 0) nU = 0;
		if (nV < 0) nV = 0;

		SetColor(pfClr, nU, nV, pPos);
	}
}

void CLightMapTex::SetColor(f32 *pfClr, u32 nU, u32 nV, CFVec3 *pPos)
{
	u32 nOffs = nV*m_nWidth + nU;
	u8 *pValue;

	pValue = (u8 *)&m_pData[nOffs];
	pValue[0] = (u8)( pfClr[0] * 255.0f );
	pValue[1] = (u8)( pfClr[1] * 255.0f );
	pValue[2] = (u8)( pfClr[2] * 255.0f );
	pValue[3] = (u8)( pfClr[3] * 255.0f );

	if (pPos && m_pPos)
	{
		m_pPos[nOffs] = *pPos;
	}
}

#define _TGA_SIGNATURE				"TRUEVISION-XFILE.\0"
#define _TGA_HEADER_SIZE_IN_BYTES	18
#define _TGA_FOOTER_SIZE_IN_BYTES	26

#pragma pack( push, 1 )

// TGA File Header (must not be padded to be 32 bit aligned)
typedef struct {                           
    u8 nIdLength;           // Image ID Field Length
    u8 nColorMapType;       // Color Map Type
    u8 nImageType;          // Image Type
    // Color Map Specification
    u16 nColorMapIndex;		// First Entry Index
    u16 nColorMapLength;	// Color Map Length
    u8 nColorMapEntrySize;  // Color Map Entry Size
    // Image Specification
    u16 nX_Origin;			// X-origin of Image
    u16 nY_Origin;			// Y-origin of Image
    u16 nImageWidth;		// Image Width
    u16 nImageHeight;		// Image Height
    u8 nPixelDepth;         // Pixel Depth
    u8 nImagDesc;           // Image Descriptor
} TgaHeader_t;

// TGA File Footer
typedef struct {
    u32 nExtensionOffset;	// Extension Area Offset
    u32 nDeveloperOffset;	// Developer Directory Offset
    char pszSignature[18];	// Signature, dot, and NULL
} TgaFooter_t;

#pragma pack( pop )

void CLightMapTex::MipMap(u32 *pData, u32 nMipLevel, u32 nW, u32 nH)
{
	u32 nPixAveX, nPixAveY, nCurAveR, nCurAveG, nCurAveB;
	u32 nX, nY, nMX, nMY;
	u32 nAX, nAY;
	u32 nAR, nAG, nAB;
	u8 *pDataPtr;
	nPixAveX = m_nWidth / nW; nPixAveY = m_nHeight / nH;

	for (nX=0, nMX=0; nX<m_nWidth; nX+=nPixAveX, nMX++)
	{
		for (nY=0, nMY=0; nY<m_nHeight; nY+=nPixAveY, nMY++)
		{
			//----------
			nAR=0; nAG=0; nAB=0; nCurAveR=0; nCurAveG=0; nCurAveB=0;
			for (nAX=nX; nAX<nX+nPixAveX; nAX++)
			{
				for (nAY=nY; nAY<nY+nPixAveY; nAY++)
				{
					pDataPtr = (u8 *)&m_pData[nAX+nAY*m_nWidth];
					if (pDataPtr[0])
					{
						nAR+=pDataPtr[0];
						nCurAveR++;
					}
					if (pDataPtr[1])
					{
						nAG+=pDataPtr[1];
						nCurAveG++;
					}
					if (pDataPtr[2])
					{
						nAB+=pDataPtr[2];
						nCurAveB++;
					}
				}
			}
			if (nCurAveR)
			{
				nAR /= nCurAveR;
			}
			if (nCurAveG)
			{
				nAG /= nCurAveG;
			}
			if (nCurAveB)
			{
				nAB /= nCurAveB;
			}
			pDataPtr = (u8 *)&pData[nMX + nMY*nW];
			pDataPtr[0] = nAR;
			pDataPtr[1] = nAG;
			pDataPtr[2] = nAB;
			pDataPtr[3] = 0xff;
			//--------------
		}
	}
}

void CLightMapTex::SetEdge(u32 nEdge, f32 fU0, f32 fV0, f32 fU1, f32 fV1, f32 fU2, f32 fV2, f32 fU3, f32 fV3)
{
/*	if (fU0 < 0) { fU0 = 0; }
	if (fU1 < 0) { fU1 = 0; }
	if (fV0 < 0) { fV0 = 0; }
	if (fV1 < 0) { fV1 = 0;	}
	if (fU2 < 0) { fU2 = 0;	}
	if (fU3 < 0) { fU3 = 0; }
	if (fV2 < 0) { fV2 = 0; }
	if (fV3 < 0) { fV3 = 0; }

	m_Edges[ nEdge ].fU0[0] = fU0;
	m_Edges[ nEdge ].fU0[0] = (m_Edges[ nEdge ].fU0[0] < m_nWidth)?(m_Edges[ nEdge ].fU0[0]):(m_nWidth-1);
	m_Edges[ nEdge ].fU0[1] = fU1;
	m_Edges[ nEdge ].fU0[1] = (m_Edges[ nEdge ].fU0[1] < m_nWidth)?(m_Edges[ nEdge ].fU0[1]):(m_nWidth-1);
	
	m_Edges[ nEdge ].fV0[0] = fV0;
	m_Edges[ nEdge ].fV0[0] = (m_Edges[ nEdge ].fV0[0] < m_nHeight)?(m_Edges[ nEdge ].fV0[0]):(m_nHeight-1);
	m_Edges[ nEdge ].fV0[1] = fV1;
	m_Edges[ nEdge ].fV0[1] = (m_Edges[ nEdge ].fV0[1] < m_nHeight)?(m_Edges[ nEdge ].fV0[1]):(m_nHeight-1);

	m_Edges[ nEdge ].fU1[0] = fU2;
	m_Edges[ nEdge ].fU1[0] = (m_Edges[ nEdge ].fU1[0] < m_nWidth)?(m_Edges[ nEdge ].fU1[0]):(m_nWidth-1);
	m_Edges[ nEdge ].fU1[1] = fU3;
	m_Edges[ nEdge ].fU1[1] = (m_Edges[ nEdge ].fU1[1] < m_nWidth)?(m_Edges[ nEdge ].fU1[1]):(m_nWidth-1);
	
	m_Edges[ nEdge ].fV1[0] = fV2;
	m_Edges[ nEdge ].fV1[0] = (m_Edges[ nEdge ].fV1[0] < m_nHeight)?(m_Edges[ nEdge ].fV1[0]):(m_nHeight-1);
	m_Edges[ nEdge ].fV1[1] = fV3;
	m_Edges[ nEdge ].fV1[1] = (m_Edges[ nEdge ].fV1[1] < m_nHeight)?(m_Edges[ nEdge ].fV1[1]):(m_nHeight-1);*/
}

void CLightMapTex::BlendColor(u16 x0, u16 y0, u16 x1, u16 y1)
{
	f32 fOO255=(1.0f/255.0f);
	if (x0 < 0 || x1 < 0 || x0 > m_nWidth-1 || x1 > m_nWidth-1) return;
	if (y0 < 0 || y1 < 0 || y0 > m_nHeight-1 || y1 > m_nHeight-1) return;

	u8 *pValues0 = (u8*)&m_pData[x0 + m_nWidth*y0];
	u8 *pValues1 = (u8*)&m_pData[x1 + m_nWidth*y1];

	f32 fVal0, fVal1;

	if (pValues0[0] && pValues0[1] && pValues0[2] && pValues1[0] && pValues1[1] && pValues1[2])
	{
		fVal0 = (f32)pValues0[0] * fOO255; fVal1 = (f32)pValues1[0] * fOO255;
		fVal0 = ( fVal0 * 0.70f + fVal1*0.30f );
		if (fVal0 < 0.0f) { fVal0 = 0.0f; } if (fVal0 > 1.0f) { fVal0 = 1.0f; }
		pValues0[0] = (u8)(fVal0 * 255.0f);

		fVal0 = (f32)pValues0[1] * fOO255; fVal1 = (f32)pValues1[1] * fOO255;
		fVal0 = ( fVal0 * 0.70f + fVal1*0.30f );
		if (fVal0 < 0.0f) { fVal0 = 0.0f; } if (fVal0 > 1.0f) { fVal0 = 1.0f; }
		pValues0[1] = (u8)(fVal0 * 255.0f);

		fVal0 = (f32)pValues0[2] * fOO255; fVal1 = (f32)pValues1[2] * fOO255;
		fVal0 = ( fVal0 * 0.70f + fVal1*0.30f );
		if (fVal0 < 0.0f) { fVal0 = 0.0f; } if (fVal0 > 1.0f) { fVal0 = 1.0f; }
		pValues0[2] = (u8)(fVal0 * 255.0f);
	}
}

void CLightMapTex::BlendEdges()
{
	/*
	if (m_nEdges && m_nWidth && m_nHeight)
	{
		s16 nX0[1024], nX1[1024];
		s16 nY0[1024], nY1[1024];
		s16 nInc0, nInc1;
		s32 i, k;
		for (i=0; i<(s32)m_nEdges; i++)
		{
			nInc0 = nInc1 = 0;

			f32 xf, yf, dx, dy, xstep, ystep, xfrac, yfrac;
			s32 xinc, yinc;
			s32 x, y, x0, y0, x1, y1;

			xf = m_Edges[i].fU0[0];
			yf = m_Edges[i].fV0[0];

			dx = m_Edges[i].fU0[1] - xf;
			dy = m_Edges[i].fV0[1] - yf;

			if (dx && dy) continue;

			if (fabsf(dx) > m_nWidth-1 || fabsf(dy) > m_nHeight-1) continue;

			x = (s32)xf;
			y = (s32)yf;

			nInc0 = 0;

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
					//x, y
					//m_pData[x + y*m_nWidth] = 0xffffffff;
					nX0[nInc0] = x; nY0[nInc0] = y;
					nInc0++;
					
					if (x == (s32)m_Edges[i].fU0[1] || x < 0 || x > m_nWidth-1) //reached end of scaneline.
					{
						break; //reached end, exit.
					}
					yfrac += ystep; //what is y value for this x?

					if (dy > 0)
					{
						if (yfrac > (f32)y + 1.0f)
						{
							y += 1; //inc y
						}
					}
					else
					{
						if (yfrac < (f32)y)
						{
							y -= 1; //dec y
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
					//x,y
					//m_pData[x + y*m_nWidth] = 0xffffffff;
					nX0[nInc0] = x; nY0[nInc0] = y;
					nInc0++;
					
					if (y == (s32)m_Edges[i].fV0[1] || y < 0 || y > m_nHeight-1) //reached end of scaneline.
					{
						break; //reached end, exit.
					}
					xfrac += xstep; //what is y value for this x?

					if (dx > 0)
					{
						if (xfrac > (f32)x + 1.0f)
						{
							x += 1; //inc y
						}
					}
					else
					{
						if (xfrac < (f32)x)
						{
							x -= 1; //dec y
						}
					}
					y += yinc;
				};
			}

			xf = m_Edges[i].fU1[0];
			yf = m_Edges[i].fV1[0];

			dx = m_Edges[i].fU1[1] - xf;
			dy = m_Edges[i].fV1[1] - yf;

			if (dx && dy) continue;

			if (fabsf(dx) > m_nWidth-1 || fabsf(dy) > m_nHeight-1) continue;

			x = (s32)xf;
			y = (s32)yf;

			nInc1 = 0;

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
					//x, y
					//m_pData[x + y*m_nWidth] = 0xffffffff;
					nX1[nInc1] = x; nY1[nInc1] = y;
					nInc1++;
					
					if (x == (s32)m_Edges[i].fU1[1] || x < 0 || x > m_nWidth-1) //reached end of scaneline.
					{
						break; //reached end, exit.
					}
					yfrac += ystep; //what is y value for this x?

					if (dy > 0)
					{
						if (yfrac > (f32)y + 1.0f)
						{
							y += 1; //inc y
						}
					}
					else
					{
						if (yfrac < (f32)y)
						{
							y -= 1; //dec y
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
					//x,y
					//m_pData[x + y*m_nWidth] = 0xffffffff;
					nX1[nInc1] = x; nY1[nInc1] = y;
					nInc1++;
					
					if (y == (s32)m_Edges[i].fV1[1] || y < 0 || y > m_nHeight-1) //reached end of scaneline.
					{
						break; //reached end, exit.
					}
					xfrac += xstep; //what is y value for this x?

					if (dx > 0)
					{
						if (xfrac > (f32)x + 1.0f)
						{
							x += 1; //inc y
						}
					}
					else
					{
						if (xfrac < (f32)x)
						{
							x -= 1; //dec y
						}
					}
					y += yinc;
				};
			}

			if ( abs(nInc0 - nInc1) < 1 && nInc0 > 0 && nInc1 > 0)
			{
				for (k=0; k<nInc0 && k<nInc1; k++)
				{
					x0 = nX0[k]; y0 = nY0[k];
					x1 = nX1[k]; y1 = nY1[k];

					BlendColor(x0, y0, x1, y1);
				}
			}
		}
	}
	*/
}

#include <math.h>

#define FABSF(a) ( ((a)>0)?(a):-(a) )
#define InEps(v0, v1, e) ( FABSF(v0.x-v1.x)<e && FABSF(v0.y-v1.y)<e && FABSF(v0.z-v1.z)<e )

void CLightMapTex::BlurPosFilter(u32 *pData, u32 nW, u32 nH)
{
	s32 i, j, nSize;
	u8 *pValuesI, *pValuesN[13];
	f32 fEps = 10.0f;
	u32 nJ = 0, nX, nY;

	u32 *nNumAve;
	CFVec3 *vAve;
	u32 *bEdge;

	nSize = nW * nH;

	nNumAve = new u32[nSize];
	vAve = (CFVec3*)malloc(sizeof(CFVec3)*nSize);

	bEdge = (u32 *)malloc(sizeof(u32)*nSize);

	memset(nNumAve, 0, 4*nSize);
	memset(vAve, 0, sizeof(CFVec3)*nSize);
	memset(bEdge, 0, 4*nSize);

	s32 nValue, nEdgeTexel=0;
	CFVec3 vMin(1000000,1000000,1000000), vMax(-1000000, -1000000, -1000000);
	CFVec3 vOffs, vScale;

	for (i=0; i<nSize; i++)
	{
		if (m_pPos[i].x || m_pPos[i].y || m_pPos[i].z)
		{
			if (m_pPos[i].x < vMin.x) vMin.x = m_pPos[i].x;
			if (m_pPos[i].y < vMin.y) vMin.y = m_pPos[i].y;
			if (m_pPos[i].z < vMin.z) vMin.z = m_pPos[i].z;

			if (m_pPos[i].x > vMax.x) vMax.x = m_pPos[i].x;
			if (m_pPos[i].y > vMax.y) vMax.y = m_pPos[i].y;
			if (m_pPos[i].z > vMax.z) vMax.z = m_pPos[i].z;
		}

		//now classify this texel as edge or interior/exterior.
		pValuesN[0] = (u8*)&pData[i];

		if (pValuesN[0][0] || pValuesN[0][1] || pValuesN[0][2])
		{
			nX = i%nW;
			nY = i/nH;

			if (nX > 0)
				pValuesN[1] = (u8*)&pData[nX-1 + nY*nH];
			else
				pValuesN[1] = pValuesN[0];

			if (nX < nW-1)
				pValuesN[2] = (u8*)&pData[nX+1 + nY*nH];
			else
				pValuesN[2] = pValuesN[0];

			if (nY > 0)
			{
				pValuesN[3] = (u8*)&pData[nX + (nY-1)*nH];

				if (nX > 0)
					pValuesN[5] = (u8*)&pData[nX-1 + (nY-1)*nH];
				else
					pValuesN[5] = pValuesN[0];

				if (nX < nW-1)
					pValuesN[6] = (u8*)&pData[nX+1 + (nY-1)*nH];
				else
					pValuesN[6] = pValuesN[0];
			}
			else
			{
				pValuesN[3] = pValuesN[0];

				pValuesN[5] = pValuesN[6] = pValuesN[0];
			}

			if (nY < nH-1)
			{
				pValuesN[4] = (u8*)&pData[nX + (nY+1)*nH];

				if (nX > 0)
					pValuesN[7] = (u8*)&pData[nX-1 + (nY+1)*nH];
				else
					pValuesN[7] = pValuesN[0];

				if (nX < nW-1)
					pValuesN[8] = (u8*)&pData[nX+1 + (nY+1)*nH];
				else
					pValuesN[8] = pValuesN[0];
			}
			else
			{
				pValuesN[4] = pValuesN[0];

				pValuesN[7] = pValuesN[8] = pValuesN[0];
			}

			if (nX-1 > 0)
				pValuesN[9] = (u8*)&pData[nX-2 + (nY)*nH];
			else
				pValuesN[9] = pValuesN[0];

			if (nX < nW-2)
				pValuesN[10] = (u8*)&pData[nX+2 + (nY)*nH];
			else
				pValuesN[10] = pValuesN[0];

			if (nY-1 > 0)
				pValuesN[11] = (u8*)&pData[nX + (nY-2)*nH];
			else
				pValuesN[11] = pValuesN[0];

			if (nY < nH-2)
				pValuesN[12] = (u8*)&pData[nX + (nY+2)*nH];
			else
				pValuesN[12] = pValuesN[0];

			for (j=1; j<13; j++)
			{
				if (pValuesN[j][0] == 0 && pValuesN[j][1] == 0 && pValuesN[j][2] == 0)
				{	
					bEdge[nEdgeTexel++] = i;
					break;
				}
			}
		}
		//
	}

	char stmp[80];
	sprintf(stmp, "nEdgeTexel = %d.", nEdgeTexel);
	MessageBox(NULL, stmp, "DEBUG", MB_OK);

	vOffs = vMin;
	vScale.x = 255.0f/(vMax.x - vMin.x);
	vScale.y = 255.0f/(vMax.y - vMin.y);
	vScale.z = 255.0f/(vMax.z - vMin.z);

	for (i=0; i<nSize; i++)
	{
        if (m_pPos[i].x || m_pPos[i].y || m_pPos[i].z)
		{
			pValuesI = (u8*)&pData[i];

			if (1)//!bEdge[i])
			{
				nValue = (s32)( (m_pPos[i].x - vOffs.x)*vScale.x );
				nValue = (nValue>0)?( (nValue<255)?(nValue):(255) ):(0);
				pValuesI[0] = (u8)(nValue);

				nValue = (s32)( (m_pPos[i].y - vOffs.y)*vScale.y );
				nValue = (nValue>0)?( (nValue<255)?(nValue):(255) ):(0);
				pValuesI[1] = (u8)(nValue);

				nValue = (s32)( (m_pPos[i].z - vOffs.z)*vScale.z );
				nValue = (nValue>0)?( (nValue<255)?(nValue):(255) ):(0);
				pValuesI[2] = (u8)(nValue);
			}
			else
			{
				pValuesI[0] = pValuesI[1] = pValuesI[2] = 0xff;
			}
		}
	}

	delete [] nNumAve;
	free(vAve);
	free(bEdge);
}

s32 XOffs[9] = 
{
	0, -1, +1,  0,  0, -1, -1, +1, +1
};

s32 YOffs[9] = 
{
	0,  0,  0, -1, +1, -1, +1, -1, +1
};

f32 fTapMult[9] = 
{
	(1.0f/1.0f), (1.0f/2.0f), (1.0f/3.0f), (1.0f/4.0f), (1.0f/5.0f), (1.0f/6.0f), (1.0f/7.0f), (1.0f/8.0f), (1.0f/9.0f)
};

#define _ABS(x) ( ((x)<0)?-(x):(x) )

f32 _fEPS2, _fEPS;

u32 CLightMapTex::FindPixelPos(u32 nTap, CFVec3& vPos, s32 x, s32 y, s32 nW, s32 nH, u32& nOffs)
{
	u32 nPix = 0;
	s32 nX, nY, nOffset;
	u8 *pValues;

	f32 fMinDist2=1000000.0f, fDist2;

	for (nY = 0, nOffset = 0; nY < nH; nY++, nOffset += nW)
	{
		for (nX = 0; nX < nW; nX++)
		{
			pValues = (u8*)&m_pData[nX+nOffset];
			if (pValues[0] || pValues[1] || pValues[2])
			{
				if ( _ABS(nX-x)>3 || _ABS(nY-y)>3 )
				{
					if ( _ABS(m_pPos[nX+nOffset].x - vPos.x)<_fEPS && _ABS(m_pPos[nX+nOffset].y - vPos.y)<_fEPS && _ABS(m_pPos[nX+nOffset].z - vPos.z)<_fEPS)
					{
						fDist2 = (m_pPos[nX+nOffset] - vPos).Mag2();
						if (fDist2 < _fEPS2 && fDist2 < fMinDist2)
						{
							fMinDist2 = fDist2;
							nOffs = nX+nOffset;
							nPix = 1;
						}
					}
				}
			}
		}
	}

	return (nPix);
}

void CLightMapTex::BlurFilter(u32 *pData, u32 nW, u32 nH, u8 nTap)
{
	f32 fMult;
	s32 nX, nY, nOffs, ntX, ntY;
	u8 *pValues, *pOldValues;//, *pNear;
	u8 n, nVal;
	f32 fAve[3];

	u32 *pOldData;

	_fEPS = m_fSampleSize;
	_fEPS2 = m_fSampleSize*m_fSampleSize;

	pOldData = (u32*)malloc(4*nW*nH);
	memcpy(pOldData, pData, 4*nW*nH);

	for (nY = 0, nOffs = 0; nY < (s32)nH; nY++, nOffs += (s32)nW)
	{
		for (nX = 0; nX < (s32)nW; nX++)
		{
			pValues = (u8*)&pData[nX+nOffs];

			if (pValues[0] || pValues[1] || pValues[2])
			{
				fAve[0] = fAve[1] = fAve[2] = 0.0f;
				nVal = 0;
				for (n=0; n<nTap; n++)
				{
					ntX = nX + XOffs[n]; ntX = (ntX >= 0)?( (ntX<(s32)nW)?(ntX):((s32)nW-1) ):(0);
					ntY = nY + YOffs[n]; ntY = (ntY >= 0)?( (ntY<(s32)nH)?(ntY):((s32)nH-1) ):(0);
				
					pOldValues = (u8*)&pOldData[ntX+ntY*nW];

					if (pOldValues[0] || pOldValues[1] || pOldValues[2]) 
					{ 
						fAve[0] += (f32)pOldValues[0];
						fAve[1] += (f32)pOldValues[1];
						fAve[2] += (f32)pOldValues[2];

						nVal++; 
					}
				}
				if (nVal > 0)
				{
					fMult = fTapMult[ nVal-1 ];

					fAve[0] *= fMult;
					fAve[1] *= fMult;
					fAve[2] *= fMult;
					
					pValues[0] = (u8)fAve[0];
					pValues[1] = (u8)fAve[1];
					pValues[2] = (u8)fAve[2];

					pValues[3] = 0xff;
				}
			}
		}
	}

	free(pOldData);
	pOldData = NULL;
}

void CLightMapTex::GrowData(u32 *pData, u32 nW, u32 nH, u32 nGrowIter)
{
	BOOL bFound;
	u32 nX, nY;
	u8 *pValues, *pOldValues, *pNear;

	u32 *pOldData;

	pOldData = (u32*)malloc(4*nW*nH);
	memcpy(pOldData, pData, 4*nW*nH);

	u32 nIter;

	for (nIter=0; nIter<nGrowIter; nIter++)
	{
		for (nX = 0; nX < nW; nX++)
		{
			for (nY = 0; nY < nH; nY++)
			{
				pValues = (u8*)&pData[nX+nY*nW];
				pOldValues = (u8*)&pOldData[nX+nY*nW];
				if (pOldValues[0] == 0 && pOldValues[1] == 0 && pOldValues[2] == 0)
				{
					bFound = FALSE;

					if (nX > 0 && !bFound)
					{
						pNear = (u8*)&pOldData[(nX-1)+nY*nW];
						if (pNear[0] || pNear[1] || pNear[2])
						{
							bFound = TRUE;
							pValues[0] = pNear[0]; pValues[1] = pNear[1]; pValues[2] = pNear[2];
						}
					}
					if (nX < nW-1 && !bFound)
					{
						pNear = (u8*)&pOldData[(nX+1)+nY*nW];
						if (pNear[0] || pNear[1] || pNear[2])
						{
							bFound = TRUE;
							pValues[0] = pNear[0]; pValues[1] = pNear[1]; pValues[2] = pNear[2];
						}
					}

					if (nY > 0 && !bFound)
					{
						pNear = (u8*)&pOldData[(nX)+(nY-1)*nW];
						if (pNear[0] || pNear[1] || pNear[2])
						{
							bFound = TRUE;
							pValues[0] = pNear[0]; pValues[1] = pNear[1]; pValues[2] = pNear[2];
						}
					}
					if (nY < nH-1 && !bFound)
					{
						pNear = (u8*)&pOldData[(nX)+(nY+1)*nW];
						if (pNear[0] || pNear[1] || pNear[2])
						{
							bFound = TRUE;
							pValues[0] = pNear[0]; pValues[1] = pNear[1]; pValues[2] = pNear[2];
						}
					}
				}
			}
		}
		memcpy(pOldData, pData, 4*nW*nH);
	}
	free(pOldData);
	pOldData = NULL;
}

void CLightMapTex::BlackOut(u32 *pData, u8 nBlackPoint, u32 nW, u32 nH)
{
	u32 nX, nY;
	u8 *pValues;

	for (nX = 0; nX < nW; nX++)
	{
		for (nY = 0; nY < nH; nY++)
		{
			pValues = (u8*)&pData[nX+nY*nW];
			if (pValues[0] <= nBlackPoint && pValues[1] <= nBlackPoint && pValues[2] <= nBlackPoint)
			{
				pValues[0] = pValues[1] = pValues[2] = 0x0;
			}
		}
	}
}

//
//  Returns TRUE if all of the texture is the same color
//
BOOL CLightMapTex::ApplyMinRGB(u32 *pData, u32 nW, u32 nH)
{
	u32 nX, nY;
	u8 *pValues;

	BOOL bSingleColor = TRUE;

	for (nX = 0; nX < nW; nX++)
	{
		for (nY = 0; nY < nH; nY++)
		{
			pValues = (u8*)&pData[nX+nY*nW];

			// Keep in mind the ARGB of TGA (little endian, too)
			if (pValues[2] < m_nMinRGB[0]) 
			{ 
				pValues[2] = m_nMinRGB[0]; 
			}
			if (pValues[1] < m_nMinRGB[1]) 
			{ 
				pValues[1] = m_nMinRGB[1]; 
			}
			if (pValues[0] < m_nMinRGB[2]) 
			{ 
				pValues[0] = m_nMinRGB[2]; 
			}

			if ( pData[nX+nY*nW] != pData[0] )
			{
				bSingleColor = FALSE;
			}
		}
	}

	return bSingleColor;
}

void CLightMapTex::Clear()
{
	memset(m_pData, 0, 4*m_nWidth*m_nHeight);
}

#if !LM_FULL_PATH
u16 _nNumLevels = 37;
char *_aszLevelName[] =
{
	"Access The Ruins",
	"Breakout",
	"Career Decision",
	"Clean Up",
	"Coliseum 1",
	"Coliseum 2",
	"Coliseum 3",
    "Coliseum 4",
	"Colonel Alloy",
	"Destroy Mil Base",
    "Destroy The Comm Array",
	"Escape Mil City",
	"Factory Demo",
	"Final Battle",
	"General Corrosive",
	"Hold Your Ground",
    "Mil City Hub",
	"Morbot Gateway",
	"Morbot Region",
	"Night Sneak",
	"Race To The Rocket",
	"RAT Race",
    "Reactor",
	"Research Facility",
	"Ruins",
    "Seal the Mines",
	"Secret Rendezvous",
	"Sniper",
    "Space Dock",
	"Space Station",
	"Spy Vs. Spy",
    "Swarmer Boss",
	"Warpath",
	"Wasteland Chase",
    "Wasteland Journey",
	"Window Washer",
	"Zombiebot Boss"
};
#endif

void CLightMapTex::LoadFile(cchar *pszFileName)
{
	if (!pszFileName) return;
	if (pszFileName[0] == 0) return;

	if (m_pTGALoader)
	{
		delete m_pTGALoader;
		m_pTGALoader = NULL;
	}
	m_pTGALoader = new CTgaFileLoader;

	char szNewName[256];

#if LM_FULL_PATH
	sprintf(szNewName, "%s.tga", pszFileName);
#else
	//Do a search for the file. This is a hack, define LM_FULL_PATH if the path is included in pszFileName

	FILE *f;
	u16 i;
	sprintf(szNewName, "%s\\TGA\\%s.tga", m_szLocalDir, pszFileName);
	f=fopen(szNewName, "rb");
	if (!f)
	{
		sprintf(szNewName, "%s\\TGA\\Effects\\%s.tga", m_szLocalDir, pszFileName);
		f=fopen(szNewName, "rb");
		if (f) { goto FOUND; }

		sprintf(szNewName, "%s\\TGA\\Layouts\\%s.tga", m_szLocalDir, pszFileName);
		f=fopen(szNewName, "rb");
		if (f) { goto FOUND; }

		sprintf(szNewName, "%s\\TGA\\Levels\\%s.tga", m_szLocalDir, pszFileName);
		f=fopen(szNewName, "rb");
		if (f) { goto FOUND; }

		for (i=0; i<_nNumLevels; i++)
		{
			sprintf(szNewName, "%s\\TGA\\Levels\\%s\\%s.tga", m_szLocalDir, _aszLevelName[i], pszFileName);
			f=fopen(szNewName, "rb");
			if (f) { goto FOUND; }
		}

		sprintf(szNewName, "%s\\TGA\\Props\\%s.tga", m_szLocalDir, pszFileName);
		f=fopen(szNewName, "rb");
		if (f) { goto FOUND; }

		sprintf(szNewName, "%s\\TGA\\Props\\MIL\\%s.tga", m_szLocalDir, pszFileName);
		f=fopen(szNewName, "rb");
		if (f) { goto FOUND; }

		sprintf(szNewName, "%s\\TGA\\Robots\\%s.tga", m_szLocalDir, pszFileName);
		f=fopen(szNewName, "rb");
		if (f) { goto FOUND; }

		sprintf(szNewName, "%s\\TGA\\texture anims\\%s.tga", m_szLocalDir, pszFileName);
		f=fopen(szNewName, "rb");
		if (f) { goto FOUND; }

		sprintf(szNewName, "%s\\TGA\\Weapons\\%s.tga", m_szLocalDir, pszFileName);
		f=fopen(szNewName, "rb");
	}
FOUND: 
	if (f) { fclose(f); }

#endif

    if ( m_pTGALoader->LoadTgaFile((cchar *)szNewName, FALSE) )
	{
		m_nWidth = m_pTGALoader->GetWidth();
		m_nHeight = m_pTGALoader->GetHeight();
		InitData(m_nWidth, m_nHeight);

		u32 nBPP = m_pTGALoader->GetBitsPerPixel();
		u32 nBytes = nBPP/8;
		u8 *pSrc = (u8*)m_pTGALoader->GetPixelData();

		u32 nX, nY, nOffs;
		u8 *pData;
		if (nBytes == 1)
		{
			for (nX=0; nX<m_nWidth; nX++)
			{
				for (nY=0; nY<m_nHeight; nY++)
				{
					nOffs = nX+nY*m_nWidth;
					pData = (u8*)&m_pData[nOffs];
					pData[0] = pSrc[nOffs];
					pData[1] = pData[2] = pData[0];
					pData[3] = 0xff;
				}
			}
		}
		else if (nBytes == 3)
		{
			for (nX=0; nX<m_nWidth; nX++)
			{
				for (nY=0; nY<m_nHeight; nY++)
				{
					nOffs = nX+nY*m_nWidth;
					pData = (u8*)&m_pData[nOffs];
					pData[0] = pSrc[nOffs*3+0];
					pData[1] = pSrc[nOffs*3+1];
					pData[2] = pSrc[nOffs*3+2];
					pData[3] = 0xff;
				}
			}
		}
		else
		{
			for (nX=0; nX<m_nWidth; nX++)
			{
				for (nY=0; nY<m_nHeight; nY++)
				{
					nOffs = nX+nY*m_nWidth;
					pData = (u8*)&m_pData[nOffs];
					pData[0] = pSrc[nOffs*4+0];
					pData[1] = pSrc[nOffs*4+1];
					pData[2] = pSrc[nOffs*4+2];
					pData[3] = pSrc[nOffs*4+3];
				}
			}
		}
	}
	else
	{
		DEVPRINTF( "CLightMapTex::LoadFile() - Unable to load texture %s.\n", pszFileName );
	}
	//m_pTGALoader->UnloadFile();
}

#include <direct.h>
#include <stdlib.h>
#include <stdio.h>

#define NOISE_MAG 0x05

void CLightMapTex::AddNoise(u32 *pData, u32 nW, u32 nH)
{
	srand( timeGetTime() );

	u32 nX, nY, nV, i;
	u8 *pValues;
	for (nX=0; nX<nW; nX++)
	{
		for (nY=0; nY<nH; nY++)
		{
			pValues = (u8*)&pData[nX + nY*nW];

			for (i=0; i<3; i++)
			{
				nV = (u32)pValues[i] + rand()%NOISE_MAG;
				if (nV > 0xff) nV = 0xff;
				pValues[i] = nV;
			}
		}
	}
}

BOOL CLightMapTex::CheckData()
{
	m_bDataCheck = FALSE;

	u32 nX, nY, i, nValues=0;
	u8 *pValues;

	for (nX=0; nX<m_nWidth; nX++)
	{
		for (nY=0; nY<m_nHeight; nY++)
		{
			pValues = (u8*)&m_pData[nX + nY*m_nWidth];

			for (i=0; i<3; i++)
			{
				if (pValues[i] > 0x10)
				{
					nValues++;
					if (nValues > 96) //32*3
					{
						break;
					}
				}
			}
		}
	}

	if (nValues > 96) //32*3
	{
		m_bDataCheck = TRUE;
	}

	return (m_bDataCheck);
}

BOOL CLightMapTex::SaveFile(char *pszFileName, char *pszSubDir, BOOL bBlackOut, u8 nMipLevel)
{
	if (m_nWidth == 1 || m_nHeight == 1) return (FALSE);

	u16 nW, nH;
	u32 *pData;
	TgaHeader_t _TGAHeader;
	TgaFooter_t _TGAFooter;
	char szNewName[256], szAid[256], szDir[256];

	sprintf(szDir, "%s\\LightMaps\\", m_szLocalDir);
	_mkdir(szDir);

	sprintf(szDir, "%s\\LightMaps\\%s", m_szLocalDir, pszSubDir);
	//remove directory and its contents
	_rmdir(szDir);
	//remake directory for new lightmaps.
	_mkdir(szDir);

	sprintf(szNewName, "%s\\%s", szDir, pszFileName);
	sprintf(szAid, "%s\\%s", szDir, pszFileName);
	szAid[ strlen(szAid) - 3 ] = 'a';
	szAid[ strlen(szAid) - 2 ] = 'i';
	szAid[ strlen(szAid) - 1 ] = 'd';
	FILE *pFile = fopen( szNewName, "wb" );
	if( !pFile ) 
	{
		return FALSE;
	}

	nW = m_nWidth; nH = m_nHeight;
	nW >>= nMipLevel; nH >>= nMipLevel;

	if (nMipLevel==0)
	{
		pData = m_pData;
	}
	else
	{
		pData = new u32[nW*nH];
		MipMap(pData, nMipLevel, nW, nH);
	}

	//BlurPosFilter(pData, nW, nH);
	//BlurFilter(pData, nW, nH, 9);
	GrowData(pData, nW, nH, 9); //4

//	AddNoise(pData, nW, nH);

	//BlendEdges();

	if (m_nSubSamples == 0)
	{
		BlurFilter(pData, nW, nH, 9);
	}
	//GrowData(pData, nW, nH, 3); //4

	// Adjust the color texels, if necessary, and determine whether or not we're dealing with a single color map
	BOOL bSingleColor = FALSE;
    if (bBlackOut)
	{
		BlackOut(pData, 0x10, nW, nH);
	}
	else
	{
		bSingleColor = ApplyMinRGB(pData, nW, nH);
	}
	
	if ( bSingleColor )
	{
		nW = 8;
		nH = 8;
	}

	// setup our header
	_TGAHeader.nIdLength = 0;
    _TGAHeader.nColorMapType = 0;
    _TGAHeader.nImageType = 2;
    _TGAHeader.nColorMapIndex = 0;
    _TGAHeader.nColorMapLength = 0;
    _TGAHeader.nColorMapEntrySize = 0;
    _TGAHeader.nX_Origin = 0;
    _TGAHeader.nY_Origin = 0;
	_TGAHeader.nImageWidth = nW;
	_TGAHeader.nImageHeight = nH;
    _TGAHeader.nPixelDepth = 32;
    _TGAHeader.nImagDesc = 0x20;// flip the image
	// fill in the footer
	_TGAFooter.nExtensionOffset = 0;
	_TGAFooter.nDeveloperOffset = 0;
	fclib_strcpy( _TGAFooter.pszSignature, _TGA_SIGNATURE );

	// write out our file to disk
	fwrite( &_TGAHeader, _TGA_HEADER_SIZE_IN_BYTES, 1, pFile );
	fwrite( pData, nW*nH*4, 1, pFile );
	fwrite( &_TGAFooter, _TGA_FOOTER_SIZE_IN_BYTES, 1, pFile );

	fclose( pFile );

	if (nMipLevel > 0)
	{
		delete pData;
	}

	FILE *pAid = fopen( szAid, "wt" );
	if( !pAid ) 
	{
		return FALSE;
	}

	fprintf(pAid, "generate_mips = 0\n");
	fprintf(pAid, "xb_tex_format = dxt3\n");
	fprintf(pAid, "gc_tex_format = s3tc\n");
	fprintf(pAid, "gc_tex_scale = 0\n");

	fclose(pAid);

	return (TRUE);
}
