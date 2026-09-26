#pragma once

#include "LightMapTex.h"

class CElementMap
{
public:
	typedef struct
	{
		f32 fRed;
		f32 fGreen;
		f32 fBlue;
	} ELEMENT_RGB;

	typedef struct
	{
		u32 nU, nV;
		u32 nOffs;
	} U32_COORD;

	CElementMap(void);
	~CElementMap(void);

	void AttachLightMapTex(CLightMapTex *pLightMap);
	void AddVertex(CFVec3& vNrml, CFVec3& vPos, u32& nX, u32& nY);
	void InitData(u32 nWidth, u32 nHeight);
	inline void F32Coord_U32Coord(f32 fU, f32 fV) { m_U32Coord.nU = (u32)fU; m_U32Coord.nV = (u32)fV; m_U32Coord.nOffs = m_U32Coord.nU + m_U32Coord.nV*m_nWidth; }//m_U32Coord.nU = (u32)(fU*m_fWMinus1); m_U32Coord.nV = (u32)(fV*m_fHMinus1); m_U32Coord.nOffs = m_U32Coord.nU + m_U32Coord.nV*m_nWidth; }
	inline ELEMENT_RGB *GetIllumination(f32 fU, f32 fV) { F32Coord_U32Coord(fU, fV); return GetIllumination(); }
	inline ELEMENT_RGB *GetIllumination() { return (&m_pIllumination[m_U32Coord.nOffs]); }

	inline ELEMENT_RGB *GetRadiance(f32 fU, f32 fV) { F32Coord_U32Coord(fU, fV); return GetRadiance(); }
	inline ELEMENT_RGB *GetRadiance() { return (&m_pRadiance[m_U32Coord.nOffs]); }

	inline ELEMENT_RGB *GetBuffer(f32 fU, f32 fV) { F32Coord_U32Coord(fU, fV); return GetBuffer(); }
	inline ELEMENT_RGB *GetBuffer() { return (&m_pBuffer[m_U32Coord.nOffs]); }

	inline f32 *GetReflectance(f32 fU, f32 fV) { F32Coord_U32Coord(fU, fV); return GetReflectance(); }
	inline f32 *GetReflectance() { return (&m_pReflectance[m_U32Coord.nOffs]); }

	inline f32 *GetArea(f32 fU, f32 fV) { F32Coord_U32Coord(fU, fV); return GetArea(); }
	inline f32 *GetArea() { return (&m_pArea[m_U32Coord.nOffs]); }

	inline u16 *GetMask(f32 fU, f32 fV) { F32Coord_U32Coord(fU, fV); return GetMask(); }
	inline u16 *GetMask() { return (&m_pMask[m_U32Coord.nOffs]); }

	inline u8 *GetMask2(f32 fU, f32 fV) { F32Coord_U32Coord(fU, fV); return GetMask2(); }
	inline u8 *GetMask2() { return (&m_pMask2[m_U32Coord.nOffs]); }

	inline u8 *GetEmissive(f32 fU, f32 fV) { F32Coord_U32Coord(fU, fV); return GetEmissive(); }
	inline u8 *GetEmissive() { return (&m_pEmissive[m_U32Coord.nOffs]); }

	inline CFVec3 *GetNrml(f32 fU, f32 fV) { F32Coord_U32Coord(fU, fV); return GetNrml(); }
	inline CFVec3 *GetNrml() { return (&m_pNrml[m_U32Coord.nOffs]); }

	inline CFVec3 *GetPos(f32 fU, f32 fV) { F32Coord_U32Coord(fU, fV); return GetPos(); }
	inline CFVec3 *GetPos() { return (&m_pPos[m_U32Coord.nOffs]); }

	inline void SetUV(f32 fU, f32 fV) { F32Coord_U32Coord(fU, fV); }

	inline u32 GetNumElements() { return (m_nNumElements); }

    void ClearMask();
	void ClearMask2();
	void ClearRadiance();
	void ClearBuffer();

	void FreeData();

	ELEMENT_RGB *m_pIllumination; //illumination of elements
	ELEMENT_RGB *m_pRadiance;     //radiance (outgoing energy) left
	ELEMENT_RGB *m_pBuffer;
	f32 *m_pReflectance;		  //reflectance of an element (0.0f to 1.0f)
	f32 *m_pArea;				  //area of each element
	u16 *m_pMask;
	u8 *m_pMask2, *m_pEmissive;
	CFVec3 *m_pNrml;
	CFVec3 *m_pPos;
	U32_COORD m_U32Coord;

	u16 m_nWidth, m_nHeight;
	u32 m_nNumElements;
	f32 m_fWMinus1, m_fHMinus1;

	CLightMapTex *m_pLightMap;
};
