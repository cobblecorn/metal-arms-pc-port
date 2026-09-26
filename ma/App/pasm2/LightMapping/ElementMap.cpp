#include "ElementMap.h"

#include <memory.h>
#include <math.h>

CElementMap::CElementMap(void)
{
	m_pIllumination = NULL;
	m_pRadiance = NULL;
	m_pReflectance = NULL;
	m_pArea = NULL;
	m_pMask = NULL;
	m_pMask2 = NULL;
	m_pNrml = NULL;
	m_pEmissive = NULL;
	m_pBuffer = NULL;
	m_pPos = NULL;
	m_pLightMap = NULL;

	m_nWidth = 0;
	m_nHeight = 0;
	m_nNumElements = 0;

	m_fWMinus1 = 0.0f;
	m_fHMinus1 = 0.0f;
}

CElementMap::~CElementMap(void)
{
	if (m_pIllumination)
	{
		free(m_pIllumination);
		m_pIllumination = NULL;
	}
	if (m_pRadiance)
	{
		free(m_pRadiance);
		m_pRadiance = NULL;
	}
	if (m_pBuffer)
	{
		free(m_pBuffer);
		m_pBuffer = NULL;
	}
	if (m_pReflectance)
	{
		free(m_pReflectance);
		m_pReflectance = NULL;
	}
	if (m_pArea)
	{
		free(m_pArea);
		m_pArea = NULL;
	}
	if (m_pMask)
	{
		free(m_pMask);
		m_pMask = NULL;
	}
	if (m_pMask2)
	{
		free(m_pMask2);
		m_pMask2 = NULL;
	}
	if (m_pEmissive)
	{
		free(m_pEmissive);
		m_pEmissive = NULL;
	}
	if (m_pNrml)
	{
		free(m_pNrml);
		m_pNrml = NULL;
	}
	if (m_pPos)
	{
		free(m_pPos);
		m_pPos = NULL;
	}
}

void CElementMap::FreeData()
{
	if (m_pIllumination)
	{
		free(m_pIllumination);
		m_pIllumination = NULL;
	}
	if (m_pRadiance)
	{
		free(m_pRadiance);
		m_pRadiance = NULL;
	}
	if (m_pBuffer)
	{
		free(m_pBuffer);
		m_pBuffer = NULL;
	}
	if (m_pReflectance)
	{
		free(m_pReflectance);
		m_pReflectance = NULL;
	}
	if (m_pArea)
	{
		free(m_pArea);
		m_pArea = NULL;
	}
	if (m_pMask)
	{
		free(m_pMask);
		m_pMask = NULL;
	}
	if (m_pMask2)
	{
		free(m_pMask2);
		m_pMask2 = NULL;
	}
	if (m_pEmissive)
	{
		free(m_pEmissive);
		m_pEmissive = NULL;
	}
	if (m_pNrml)
	{
		free(m_pNrml);
		m_pNrml = NULL;
	}
	if (m_pPos)
	{
		free(m_pPos);
		m_pPos = NULL;
	}
}

void CElementMap::AttachLightMapTex(CLightMapTex *pLightMap)
{
	if (pLightMap)
	{
		u32 i, nSize;

		m_nWidth = pLightMap->GetWidth();
        m_nHeight = pLightMap->GetHeight();

		m_fWMinus1 = (f32)( m_nWidth - 1 );
		m_fHMinus1 = (f32)( m_nHeight - 1 );

		nSize = m_nWidth*m_nHeight;
		m_pIllumination = (ELEMENT_RGB *)malloc(sizeof(ELEMENT_RGB)*nSize);
		m_pRadiance     = (ELEMENT_RGB *)malloc(sizeof(ELEMENT_RGB)*nSize);
		if ( pLightMap->GetSubSamples() )
		{
			m_pBuffer	= (ELEMENT_RGB *)malloc(sizeof(ELEMENT_RGB)*nSize);
		}
		else
		{
			m_pBuffer = NULL;
		}
		m_pReflectance  =         (f32 *)malloc(sizeof(f32)*nSize);
		m_pArea         =         (f32 *)malloc(sizeof(f32)*nSize);
		m_pMask			=		  (u16 *)malloc(nSize*2);
		m_pMask2		=		  ( u8 *)malloc(nSize);
		m_pEmissive     =		  ( u8 *)malloc(nSize);
		m_pNrml			=		  (CFVec3*)malloc(nSize*sizeof(CFVec3));
		m_pPos			=		  NULL;

		//Now set default values
		for (i=0; i<nSize; i++)
		{
            m_pIllumination[i].fRed = 0.01f; m_pIllumination[i].fGreen = 0.01f; m_pIllumination[i].fBlue = 0.01f;
			m_pRadiance[i].fRed = 0.0f; m_pRadiance[i].fGreen = 0.0f; m_pRadiance[i].fBlue = 0.0f;
			if (m_pBuffer)
			{
				m_pBuffer[i].fRed = 0.0f; m_pBuffer[i].fGreen = 0.0f; m_pBuffer[i].fBlue = 0.0f;
			}
			
			m_pReflectance[i] = 1.0f;
			m_pArea[i] = 0.1f;
			m_pMask[i] = 0; m_pMask2[i] = 0;
			m_pEmissive[i] = 0;
			m_pNrml[i].x = m_pNrml[i].y = m_pNrml[i].z = 0.0f;
		}
	}
}

BOOL VecEq(CFVec3 *v0, CFVec3 *v1, f32 fMul, f32 fEps)
{
	f32 fDX, fDY, fDZ;
	
	fDX = fabsf( v0->x*fMul - v1->x );
	fDY = fabsf( v0->y*fMul - v1->y );
	fDZ = fabsf( v0->z*fMul - v1->z );

	if (fDX < fEps && fDY < fEps && fDZ < fEps)
	{
		return TRUE;
	}

	return FALSE;
}

typedef struct Bucket_s
{
	u32 nIdx;
	struct Bucket_s *pNext;
} Bucket_t;

#define _VTX_BUCKET_COUNT	512
Bucket_t *apVtxBuckets[_VTX_BUCKET_COUNT];
Bucket_t aBucketPool[1024*1024];
u32 nBucketPool;

void CElementMap::AddVertex(CFVec3& vNrml, CFVec3& vPos, u32& nX, u32& nY)
{
	u32 nBucket;
	nBucket = ((u32)(vPos.x * _VTX_BUCKET_COUNT))%_VTX_BUCKET_COUNT;
	if (m_nNumElements == 0)
	{
		memset(apVtxBuckets, 0, sizeof(Bucket_t)*_VTX_BUCKET_COUNT);

		nX = nY = 0;
		SetUV((f32)nX, (f32)nY);
		*GetNrml() = vNrml;
		*GetPos() = vPos;
		*GetArea() = 1.0f;

		nBucketPool = 1;
		aBucketPool[0].nIdx = 0;

		apVtxBuckets[nBucket] = &aBucketPool[0];
		aBucketPool[0].pNext = NULL;

		m_nNumElements++;
	}
	else
	{
		if (apVtxBuckets[nBucket] == NULL)
		{
			nX = m_nNumElements%m_nWidth;
			nY = m_nNumElements/m_nWidth;

			SetUV((f32)nX, (f32)nY);
			*GetNrml() = -vNrml;
			*GetPos() = vPos;
			*GetArea() = 1.0f;

			aBucketPool[nBucketPool].nIdx = m_nNumElements;
			apVtxBuckets[nBucket] = &aBucketPool[nBucketPool];
			aBucketPool[nBucketPool].pNext = NULL;
            nBucketPool++;

			m_nNumElements++;
		}
		else
		{
			Bucket_t *pBucket = apVtxBuckets[nBucket];
			Bucket_t *pLastBucket = NULL;
			while ( pBucket )
			{
				nX = pBucket->nIdx%m_nWidth;
				nY = pBucket->nIdx/m_nWidth;

				SetUV((f32)nX, (f32)nY);

				if ( VecEq(GetNrml(), &vNrml, -1.0f, 0.1f) && VecEq(GetPos(), &vPos, 1.0f, 0.25f) )
				{
					//found already in map.
					return;
				}

				pLastBucket = pBucket;
				pBucket = pBucket->pNext;
			};

			FASSERT( pLastBucket );

			if ( pLastBucket )
			{
				nX = m_nNumElements%m_nWidth;
				nY = m_nNumElements/m_nWidth;

				SetUV((f32)nX, (f32)nY);
				*GetNrml() = -vNrml;
				*GetPos() = vPos;
				*GetArea() = 1.0f;

				aBucketPool[nBucketPool].nIdx = m_nNumElements;
				pLastBucket->pNext = &aBucketPool[nBucketPool];
				aBucketPool[nBucketPool].pNext = NULL;
				nBucketPool++;

				m_nNumElements++;
			}
		}
	}
}

void CElementMap::InitData(u32 nWidth, u32 nHeight)
{
	if (nWidth && nHeight)
	{
		u32 i, nSize;

		m_nWidth = nWidth;
        m_nHeight = nHeight;

		m_fWMinus1 = (f32)( m_nWidth - 1 );
		m_fHMinus1 = (f32)( m_nHeight - 1 );

		nSize = m_nWidth*m_nHeight;
		m_pIllumination = (ELEMENT_RGB *)malloc(sizeof(ELEMENT_RGB)*nSize);
		m_pRadiance     = (ELEMENT_RGB *)malloc(sizeof(ELEMENT_RGB)*nSize);
		m_pBuffer		= (ELEMENT_RGB *)malloc(sizeof(ELEMENT_RGB)*nSize);
		m_pReflectance  =         (f32 *)malloc(sizeof(f32)*nSize);
		m_pArea         =         (f32 *)malloc(sizeof(f32)*nSize);
		m_pMask			=		  (u16 *)malloc(nSize*2);
		m_pMask2		=		  ( u8 *)malloc(nSize);
		m_pEmissive     =		  ( u8 *)malloc(nSize);
		m_pNrml			=		  (CFVec3*)malloc(nSize*sizeof(CFVec3));
		m_pPos			=		  (CFVec3*)malloc(nSize*sizeof(CFVec3));

		//Now set default values
		for (i=0; i<nSize; i++)
		{
            m_pIllumination[i].fRed = 0.01f; m_pIllumination[i].fGreen = 0.01f; m_pIllumination[i].fBlue = 0.01f;
			m_pRadiance[i].fRed = 0.0f; m_pRadiance[i].fGreen = 0.0f; m_pRadiance[i].fBlue = 0.0f;
			m_pBuffer[i].fRed = 0.0f; m_pBuffer[i].fGreen = 0.0f; m_pBuffer[i].fBlue = 0.0f;
			
			m_pReflectance[i] = 0.95f;
			m_pArea[i] = 0.1f;
			m_pMask[i] = 0; m_pMask2[i] = 0;
			m_pEmissive[i] = 0;
			m_pNrml[i].x = m_pNrml[i].y = m_pNrml[i].z = 0.0f;
		}
	}
}

void CElementMap::ClearMask()
{
	memset(m_pMask, 0, m_nWidth*m_nHeight*2);
}

void CElementMap::ClearMask2()
{
	memset(m_pMask2, 0, m_nWidth*m_nHeight);
}

void CElementMap::ClearRadiance()
{
	memset(m_pRadiance, 0, sizeof(ELEMENT_RGB)*m_nWidth*m_nHeight);
}

void CElementMap::ClearBuffer()
{
	if (m_pBuffer)
	{
		memset(m_pBuffer, 0, sizeof(ELEMENT_RGB)*m_nWidth*m_nHeight);
	}
}
