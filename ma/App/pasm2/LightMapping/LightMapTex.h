#pragma once

#include "fang.h"
#include "fmath.h"
#include "fclib.h"

#include "atlstr.h"
#include "..\TGAFileLoader.h"

class CLightMapTex
{
public:

	typedef struct
	{
		f32 fU0[2];
		f32 fV0[2];

		f32 fU1[2];
		f32 fV1[2];
	} BLEND_EDGE;

	CLightMapTex(void);
	~CLightMapTex(void);

	BOOL InitData(u32 nWidth, u32 nHeight, BOOL bPos=FALSE);

	void GetColor(f32 *pfClr, f32 fU, f32 fV);
	void GetColor(f32 *pfClr, u32 nU, u32 nV);

	void SetColor(f32 *pfClr, f32 fU, f32 fV, CFVec3 *pPos=NULL);
	void SetColor(f32 *pfClr, u32 nU, u32 nV, CFVec3 *pPos=NULL);

	void SetTriPtr(f32 fU, f32 fV, void *ptr);

	void MipMap(u32 *pData, u32 nMipLevel, u32 nW, u32 nH);
	BOOL SaveFile(char *pszFileName, char *pszSubDir, BOOL bBlackOut=FALSE, u8 nMipLevel=0); //0=top, 1=m_nWidth/2, 2=m_nWidth/4, etc.

	void SetNumEdges(u32 nEdge) { m_nEdges = nEdge; }
	void SetEdge(u32 nEdge, f32 fU0, f32 fV0, f32 fU1, f32 fV1, f32 fU2, f32 fV2, f32 fU3, f32 fV3);
	void BlendEdges();

	void LoadFile(cchar *pszFileName);

	void SetName(char *pszName) { fclib_strcpy(m_szName, pszName); }
	char *GetName() { return (m_szName); }

	inline u16 GetWidth() { return (m_nWidth); }
	inline u16 GetHeight() { return (m_nHeight); }

	void ConvertUV(f32& fU, f32& fV, s32 nAdd=0);
	void ConvertU(f32& fU, s32 nAdd=0);
	void ConvertV(f32& fV, s32 nAdd=0);

	void SetSampleSize(f32 fSampleSize) { m_fSampleSize = fSampleSize; }

	inline f32 GetSampleSize() { return m_fSampleSize; }

	void SetSubSamples(u32 nSubSamples) { m_nSubSamples = nSubSamples; }

	inline u32 GetSubSamples() { return m_nSubSamples; }

	void Clear();

	BOOL CheckData();

	BOOL GetCheckResult() { return m_bDataCheck; }

	void SetCheck(BOOL bCheck) { m_bDataCheck = bCheck; }

	static void SetLocalDir(cchar *pszDir) { fclib_strcpy(m_szLocalDir, pszDir); }

	static char *GetLocalDir() { return m_szLocalDir; }

	static void SetMinValue(u8 nRed, u8 nGreen, u8 nBlue) { m_nMinRGB[0] = nRed; m_nMinRGB[1] = nGreen; m_nMinRGB[2] = nBlue; }

private:
	u32 *m_pData;
	CFVec3 *m_pPos;
	void **m_TriPtr;
	u16 m_nWidth;
	u16 m_nHeight;
	static u8 m_nMinRGB[4];

	f32 m_fOOWm1;
	f32 m_fOOHm1;

	f32 m_fSampleSize;
	u32 m_nSubSamples;

	BOOL m_bInit;

	char m_szName[32];
	static char m_szLocalDir[256];

	static BLEND_EDGE m_Edges[1000000];
	u32 m_nEdges;
	BOOL m_bDataCheck;

	CTgaFileLoader *m_pTGALoader;

	void GrowData(u32 *pData, u32 nW, u32 nH, u32 nGrowIter);
	u32 FindPixelPos(u32 nTap, CFVec3& vPos, s32 x, s32 y, s32 nW, s32 nH, u32& nOffs);
	void BlurFilter(u32 *pData, u32 nW, u32 nH, u8 nTap);
	void BlurPosFilter(u32 *pData, u32 nW, u32 nH);
	void BlackOut(u32 *pData, u8 nBlackPoint, u32 nW, u32 nH);
	BOOL ApplyMinRGB(u32 *pData, u32 nW, u32 nH);

	void AddNoise(u32 *pData, u32 nW, u32 nH);
	void BlendColor(u16 x0, u16 y0, u16 x1, u16 y1);
};
