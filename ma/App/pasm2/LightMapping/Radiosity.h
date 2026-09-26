#pragma once
#include "LightingSolution.h"
#include "ElementMap.h"

class CCompileDlg;

class CRadiosity : public CLightingSolution
{
public:
	CRadiosity(void);
	~CRadiosity(void);

	//Lighting solution specific Build(). This is different for each lighting solution.
	virtual BOOL Build(f32 fAmbRed, f32 fAmbGreen, f32 fAmbBlue);
	virtual BOOL BuildUVsOnly();
	virtual BOOL Build_VertexOnly(f32 fAmbRed, f32 fAmbGreen, f32 fAmbBlue);

	virtual void AddLight(u32 nVolume, LS_LightType_e LightType, CFVec3& vDir, CFVec3& vPos, CFVec3& vDAtten, CFVec3& vAAtten, CFVec3& vColor, f32 fColorIntens, f32 fAttenItens, f32 fOOR2, u16 nMotifIndex);

	virtual void CalculateDialogUpdateParams();

	void CreateAreaLight(LN_Triangle_t *pTri0, LN_Triangle_t *pTri1);
	
protected:

	enum
	{
		MAX_SUBDIV_DIM = 8,
		MAX_SUBDIV_QUAD = MAX_SUBDIV_DIM*MAX_SUBDIV_DIM,
		MAX_SUBDIV_VTX = (MAX_SUBDIV_DIM+1)*(MAX_SUBDIV_DIM+1),
		MAX_AREA = 1000
	};

    typedef struct
	{
		f32 fArea;
		CFVec3 vVtx[4];
		CFVec3 vCen;
	} RA_Patch_t;

	enum
	{
		_RAY_HIT = 0,
		_RAY_PASS = 1,
		_NUM_BELEM=1
	};

	typedef struct
	{
		u8 nVtx[4]; //vtx indices for edges.
		CFVec3 vCen; //center
	} LN_Quad_t;

	typedef struct
	{
		f32 fRed, fGreen, fBlue;
		f32 fIntensity;

		CFVec3 vNrml;
		f32 fD;
		f32 fArea; //Individual area.

		u8 nX, nY, nQuad, nVtx; //number of quad on X and Y
		CFVec3 vVtx[MAX_SUBDIV_VTX];
		LN_Quad_t anQuads[MAX_SUBDIV_QUAD];
	} LN_AreaLight_t;

	typedef struct
	{
		CElementMap *pElementMap;
		f32 fU, fV;
		CElementMap::ELEMENT_RGB fRad;
		CElementMap::ELEMENT_RGB fOrigRad;
		f32 fIntens;
		f32 fArea;
		CFVec3 vPos, vNrml;
		LN_Triangle_t *pTri;
	} LN_Brightest_Element_t;

	typedef struct
	{
		CFVec2 vOffs;
		CFVec2 vA, vB;
		f32 fInvOffsY;
		f32 fPosA[3], fPosB[3];
		f32 fNrmlA[3], fNrmlB[3];
		f32 fMinV, fMaxV;
	} LN_EDGE_t;

	typedef struct
	{
		u16 nLMap;
		u16	bRenderTri;
		f32 fWm1, fOOW, fDelta;
		f32 fMinV, fMaxV;
		
		LN_EDGE_t aEdges[3];
		CLightMapTex *pLightMap;
	} LN_RASTER_PRECALC_t;

	typedef void (*RasterizeCallbackFunc)(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx);
	typedef void (*RasterizeCallbackFunc_2)(f32 fU, f32 fV, f32 fDelta, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx);
	typedef void (*RasterizeCallbackFuncMask)(f32 fU, f32 fV, f32 fMaskU, f32 fMaskV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx);

	BOOL m_bInterpNrml;
	BOOL m_bUseLightMapFlag;

	f32 m_fGamma;
	u32 m_nMaxIterations;
	f32 m_fLightInScene;
	f32 m_fMaxLightInScene;
	f32 m_fThreshold;
	f32 m_fPI;
	f32 m_fPointLightMult;
	static CFVec3 m_vMulClr;

	u64 *m_panMotifTargetCount;

	static LN_Light_t *m_pCurLight;
	static LN_RASTER_PRECALC_t *m_pRasterPreCalc;
	static LN_RASTER_PRECALC_t *m_pRasterPreCalcMask;

	LN_Brightest_Element_t m_BrightestElem[_NUM_BELEM];

	u32 m_nAreaLights;
	LN_AreaLight_t m_AreaLights[MAX_AREA];

	static LN_AreaLight_t *m_pCurAreaLight;
	static u8 m_nCurAreaQuad;

	CElementMap *m_pElementMaps;
	CElementMap *m_pVtxMap;
	static CRadiosity *m_pSelf;

	u32 PartitionWorld();
	u32 PartitionWorld_Vertex();
	f32 RayTrace_PatchVisibility(RA_Patch_t *pPatchSrc, RA_Patch_t *pPatchDst);
	f32 RayTrace_AreaLight(LN_AreaLight_t *pAreaLight, CFVec3& vStart);
	BOOL FindUVIntersect(LN_Triangle_t *pTri, CFVec3& vPoint, CFVec2 *pvMaskUV);
	f32 RayTrace(CFVec3& vStart, CFVec3& vEnd, CFVec2 *pvMaskUV=NULL, LN_Triangle_t **pCollTri=NULL, CFVec3 *pNrml=NULL, LN_Light_t *pLight=NULL);
	
	inline BOOL PointInsideTriSphere(LN_Triangle_t *pTri, CFVec3& vPoint);
	BOOL TriEffectedByLight(LN_Triangle_t *pTri, LN_Light_t *pLight);

	void RasterPreCalc(u16 nMotif);
	void Rasterize(u16 nMotif, RasterizeCallbackFunc pFunc, BOOL bShoot=FALSE, LN_Light_t *pLight=NULL);
	void Rasterize_SubSample(u16 nMotif, RasterizeCallbackFunc pFunc, BOOL bShoot, LN_Light_t *pLight, u8 nSubSample);
	void RasterizeMask(RasterizeCallbackFuncMask pFunc);
	void Rasterize_2(u16 nMotif, RasterizeCallbackFunc_2 pFunc, BOOL bShoot);
	void Rasterize_4(u16 nMotif, RasterizeCallbackFunc_2 pFunc, BOOL bShoot);
	void RasterizeTriangle(u16 nMotif, RasterizeCallbackFunc pFunc, LN_Triangle_t *pTri, u32 nTri);
	void GenerateElementMaps();
	void GenerateElementMaps_Vertex();
	f32 GetVertexArea(u32 nVtx);
	void RenderLightMaps(u32 nMotif);
	void RenderVertexColors(f32 fMinRed=0.0f, f32 fMinGreen=0.0f, f32 fMinBlue=0.0f);
	void FixUpLightValues(u32 nMotif);
	void BlendEdges(u32 nMotif);

	inline f32 Intensity(CElementMap::ELEMENT_RGB *pRad);
	void MakeTriangleEmissive(u32 nTri, f32 fIntensity, CFVec3& vColor);

	void SubDivideQuad(LN_AreaLight_t *pLight, LN_Quad_t *pQuad, u8 nQuad);

	BOOL EmitLight(u32 nMotif);
	BOOL EmitLight_Vertex();
	void ApplyRadiosityToElement();
	void HemisphereLight_Element();
	void DispenseLight(u32 nIter, u32 nMotif);
	void DispenseLight_Vertex(u32 nIter);
	void EmitEnergyToElement();
	void FindHighestEnergy_Vertex(f32 fU, f32 fV);

	void ClearElementMasks();
	void ClearElementMasks2();
	void ClearRadiance();
	void ClearBuffer();
	inline void ApplyGamma(f32 *pfClr);

	void FindMatchingVtx(LN_Triangle_t *pTri, LN_Vertex_t *pVtx0, LN_Vertex_t *pVtx1, u32& nVtx0, u32& nVtx1);
    
	static inline void TriNrml(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx);
	static inline void EmissiveSurf(f32 fU, f32 fV, f32 fMaskU, f32 fMaskV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx);
	static inline void TriEmissive(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx);
	static inline void ElementArea(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx);
	static inline void RenderLMapTexel(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx);
	static inline void EmitPointLight(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx);
	static inline void HemisphereLight(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx);
	static inline void EmitAreaLight(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx);
	static inline void EmitPointLight_SS(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx);
	static inline void EmitAreaLight_SS(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx);
	static inline void FindHighestEnergy(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx);
	static inline void EmitHighestEnergy(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx);
	static inline void AveSubSamples(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx);

	static void TestFunc(f32 fU, f32 fV, CFVec3& vPos, CFVec3& vDelta, LN_Triangle_t *pTri, u32 nLMIdx);
};
