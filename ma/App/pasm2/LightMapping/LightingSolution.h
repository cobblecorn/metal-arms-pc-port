#pragma once

#include "fang.h"
#include "fmath.h"
#include "fclib.h"
#include "fkdop.h"
#include "fcolor.h"

#include "LightMapTex.h"
//#include "WinPrintf.h"

//#define WPRINT_BEGIN if (m_pWinPrintf) { 
//#define WPRINT m_pWinPrintf->ForcePrintf
//#define WPRINT_END DispatchWinMsg(); }
#define WPRINT_BEGIN 
#define WPRINT 
#define WPRINT_END 

#define LS_KDOP_TYPE		FkDOP_14_DOP
#define LS_KDOP_AXES		FkDOP_14_DOP_AXES
#define LS_VE_EPS 0.01f

class CCompileDlg;

class CLightingSolution
{
public:

	enum
	{
		LS_MAX_LIGHTMAPS=8,
		LS_MAX_TRIANGLES=500000,
		LS_MAX_VERTICES=1000000,
		LS_MAX_LIGHTS=2000,
		LS_MAX_VTX_GROUP=10000,
		LS_MAX_TRI_GROUP=10000,
		LS_MAX_LIGHTMAPTEX=999, //999
		LS_MAX_VOLUMES=1000, //1000
		LS_MAX_MATERIALS=10000,
		LS_MAX_LIGHTPERTRIANGLE=32,//128,
		LS_MAX_MOTIFPERVOLUME=32,
		LS_MAX_UNIQUE_MOTIF=16,
		LS_GRID_SIZE=64,
		LS_GRID_AREA=4096,
		LS_MAX_MAPS=1000 //1000
	};

	typedef struct
	{
		f32 x, y, z;
		f32 nx, ny, nz;
		f32 u, v;
		f32 fBaseU, fBaseV;
		f32 r, g, b, a;
	} LS_LightingVtx_t;

	typedef enum
	{
		LS_LTYPE_POINT=0,
		LS_LTYPE_SPOT=1,
		LS_LTYPE_AREA=2,
		LS_TYPE_NOATTEN=3
	} LS_LightType_e;

	//Export data, to get lightmap data into the game.
	typedef struct
	{
		u16 nLMIndex;
		u16 nVBIndex;
		u32 nVtxIndex;

		f32 fU, fV;
	} LN_UVExport_t;

	typedef struct
	{
		u16 nNumLightMaps;
		u16 *anLightMapIdx;
		u32 *anMotifList;
		u32 nVtx;

		LN_UVExport_t **aUVSet; //aUVSet[nNumLightMaps][nVtx]
	} LN_MaterialExport_t;
	//

	typedef struct
	{
		float fU, fV;
	} LN_UV_Set;

	typedef struct
	{
		u16 nVolume[4];
		u16 nNumVolumes;
		u16 nMotifIndex;
		LS_LightType_e LightType;
		CFVec3 vDir, vPos;
		CFVec3 vDAtten, vAAtten;
		CFVec3 vColor;
		f32 fOOR2, fIntensity, fRadius;
	} LN_Light_t;

	CLightingSolution(void);
	virtual ~CLightingSolution(void);

	//Maximum memory in KB.
	void SetMaxMemory(u32 nMaxMem) { m_nMaxMemory = nMaxMem; }
	void SetSubSampling(f32 fSubSample) { m_fSubSample = fSubSample; }
	//Level name (used in generating lightmap names).
	void SetLevelName(cchar *pszName);
	//WinPrintf - used to output data to window
//	void SetWinPrintf(CWinPrintf *pWinPrintf) { m_pWinPrintf = pWinPrintf; }
	//Hwnd - used to output data to window.
	void SetHwnd(HWND hwnd) { m_MainHwnd = hwnd; }
	//Used to allow some windows processing without interupting the program flow.
	void DispatchWinMsg();

	void SetProgressDialog( CCompileDlg *pDlg ) { m_pDlg = pDlg; };

	//Fill vertex uv for output.
	void FillVtxUV(u32 nUVSet, u32 nStart, u32 nEnd, LN_UV_Set **m_pUVBuffer);
	
	void GetVolumeVtxGroup(u32 nVolume, u32 nGroup, u32& nStart, u32& nEnd);

	virtual BOOL BuildUVsOnly() { return FALSE; };
	virtual BOOL Build(f32 fAmbRed, f32 fAmbGreen, f32 fAmbBlue);
	virtual BOOL Build_VertexOnly(f32 fAmbRed, f32 fAmbGreen, f32 fAmbBlue) { return FALSE; }

	virtual LN_UV_Set *GenerateUVs();

	virtual void FreeGeneratedUVs();

	virtual u32 CreateMaterial(u32 nVolume, f32 fRed=1.0f, f32 fGreen=1.0f, f32 fBlue=1.0f, BOOL bStaticUV=FALSE, BOOL bLightMap=TRUE, BOOL bRayTest=TRUE, CFMtx43 *pMtx=NULL);
	virtual void SetAreaLightData(f32 fIntensity, f32 fRed, f32 fGreen, f32 fBlue);

	u32 GetMaterial_NumLightMaps(u32 nVolume, u32 nMaterial);
	char *GetMaterialLightMap(u32 nVolume, u32 nMaterial, u32 nLightMap);

	LN_MaterialExport_t *ExportMaterial(u32 nVolume, u32 nMaterial);
	void FreeMaterial(LN_MaterialExport_t *pExpMtl);

	void GetVtxClr(u32 nVolume, u32 nMaterial, u32 nVtx, f32& fRed, f32& fGreen, f32& fBlue, f32& fAlpha);
	u32 GetVtxUserData(u32 nVolume, u32 nMaterial, u32 nVtx);

	u32 GetNumLightMaps_Volume(u32 nVolume) { return m_VolumeList[nVolume].nNumLightMaps; }
	char *GetLightMap_Volume(u32 nVolume, u32 nLightMap) 
	{ 
		if ( nVolume >= m_nNumVolumes )
		{
			return NULL;
		}
		else if ( nLightMap >= m_VolumeList[nVolume].nNumLightMaps )
		{
			return NULL;
		}
		return m_VolumeList[nVolume].pLightMaps[nLightMap]->GetName(); 
	}
	u16 GetLightMapMotif_Volume(u32 nVolume, u32 nLightMap) 
	{
		if ( nVolume >= m_nNumVolumes )
		{
			return NULL;
		}
		else if ( nLightMap >= m_VolumeList[nVolume].nNumLightMaps )
		{
			return NULL;
		}
		return m_VolumeList[nVolume].m_aMotifList[nLightMap].nMotifIndex; 
	}

	char *GetLightMapDirectory();

	void SetTriCurrentMaterial(u32 nGroup, u32 nStartIdx, u32 nSize);

	void SetVolumeRadius(u32 nVolume, f32 fRadius) { m_VolumeList[nVolume].fRadius = fRadius; }
	void SetVolumeStaticUV(u32 nVolume, BOOL bStaticUV=FALSE) { m_VolumeList[nVolume].bStaticUV = bStaticUV; }
	void SetCurrentVolume_Vtx(u32 nVolume) { m_nCurVolume_Vtx = nVolume; }

	void SetVolumePackUV(u32 nVolume, BOOL bPackUV=TRUE) { m_VolumeList[nVolume].bPackUV = bPackUV; }
    	
	void SetGeoInstanceVolume(u32 nVolume) { m_nGeoInstanceVolume = nVolume; }
	u32 GetGeoInstanceVolume(u32 nGeoIdx) { return m_nGeoInstanceVolume + nGeoIdx; }

	void SetGeoInstance_Start() 
	{ 
		m_nGeoInstStart = m_nNumTri; 
	}

	void SetDataSize(u32 nVtx, u32 nTri);

	virtual void AddTriangle(s32 nVolumeIdx, u32 nV0, u32 nV1, u32 nV2);

	virtual void AddVertex(LS_LightingVtx_t Vtx, u32 nUserData=0);

	virtual void SetVtxGroup(u32 nGroup);

	virtual void SetTriGroup(u32 nGroup);

	virtual void AddLight(u32 nVolume, LS_LightType_e LightType, CFVec3& vDir, CFVec3& vPos, CFVec3& vDAtten, CFVec3& vAAtten, CFVec3& vColor, f32 fColorIntens, f32 fAttenItens, f32 fOOR2, u16 nMotifIndex);

	virtual void AssignLightsToTri();

	virtual void OffsetTriIndexs(u32 nGroup, u32 nStartIdx, u32 nSize, u32 nOffs);

	virtual void SetVolumeVtxGroup(u32 nVolume, u32 nGroup);

	virtual void CalculateDialogUpdateParams( void ) {};

	//Special Material things here//
	void SetCurMaterial_ZMask(cchar *pszZMask);
	void SetCurMaterial_Emissive(cchar *pszEmissive);
	void SetCurMaterial_EmissiveClr(f32 fRed, f32 fGreen, f32 fBlue);
	void SetCurMaterial_EmissiveAlphaMask(BOOL bAlphaMask);

	void SetQuality(u32 nQuality) { m_nQuality = nQuality; }

	void SetSkyLightParam(u32 nQuality, CFColorRGB& horizClr, CFColorRGB& zenithClr, f32 fIntens)
	{
		m_nSkyLightingQuality = (nQuality<=2)?(nQuality):(2);
		m_fHorizColor = horizClr;
		m_fZenithColor = zenithClr;
		m_fSkyLightIntens = fIntens;
	}
    
//protected:
	
	typedef struct
	{
		u16 nNLightMaps;
		CLightMapTex *pLightMaps[LS_MAX_LIGHTMAPS+1];

		u16 nNumMotif;
		u16 anMotifArray[LS_MAX_LIGHTMAPS];

		CLightMapTex *pMask; //r = ?, g = reflectivity, b = emissiveness, a = translucency
		CLightMapTex *pColor; //for translucencies only, color for rays passing through.

		CLightMapTex *pZMask;
		CLightMapTex *pEmissive;
		f32 fRed, fGreen, fBlue;
		BOOL bAlphaMask;
		BOOL bStaticUV; //uvs already calculated.
		BOOL bHasLightMap;
		BOOL bRayTest;

		f32 fAreaR, fAreaG, fAreaB, fAreaI;
		CFMtx43 MtxR, UnitMtxR, MtxF;
	} LN_Material_t;

	typedef struct
	{
		u16 nMotifIndex;
		u16 nUVIndex;
	} LN_MotifIndex_t;

	typedef struct
	{
		BOOL bPackUV;
		//Triangle list for volume
		u32 nStartIdx;
		u32 nEndIdx;
		//Global lightmap data.
		u16 nNumLightMaps;
		CLightMapTex *pLightMaps[LS_MAX_MOTIFPERVOLUME];
		//Global Motif List and UV index.
		u16 nNumMotif;
		LN_MotifIndex_t m_aMotifList[LS_MAX_MOTIFPERVOLUME];
		//
		u32 nNumVtx;
		f32 fRadius;

		f32 afClr[4];

		BOOL bHasLM;
		BOOL bStaticUV;
	} LN_Volume_t;

	typedef struct
	{
		f32 fPos[3];
		f32 fNrml[3];
		f32 fClr[4];
		f32 fMaskUV[2];
		f32 fBaseUV[2];
		f32 fOutUV[2];
		LN_UV_Set aUVSet[LS_MAX_MOTIFPERVOLUME];

		u32 nUserData;
	} LN_Vertex_t;

	typedef struct LN_Triangle_s
	{
		u32 nVtx[3];
		LN_Vertex_t *pVtx[3];
		LN_Material_t *pMaterial;
		CFVec3 vCen;
		f32 fRadius;//, fRadius2;
		BOOL8 bToggle;
		u8 nEdgeMatches;
		//What lights effect this triangle?
		u16 nNumLights;
		LN_Light_t *pLights[LS_MAX_LIGHTPERTRIANGLE];
		//Volume info
		LN_Volume_t *pVolume;
		//Plane Info: vNrml.x*X + vNrml.y*Y + vNrml.z*Z + fD = 0
		CFVec3 vNrml;
		f32 fD, fMag;

		//lightmap data
		CFVec3 vMin;
		f32 fLightMapX, fLightMapY;
		f32 fLightMapW, fLightMapH;
		
		CFVec3 vLightMapOrigin;
		CFVec3 vLightMapVecs[3];
		CFVec4 vLightMapMtx[3];
		f32 fScaleX, fScaleY;
		//
		s32 nEdgeIdx[3];
		struct LN_Triangle_s *pEdgeTri[3];

		struct LN_Triangle_s *pNext;

		// Link for creating linklists of kDOP triangles
		struct LN_Triangle_s *pkDOPNext;
		f32 afVertIntervals[3][LS_KDOP_AXES];

		typedef BOOL (*PointInsideTriFunc)(struct LN_Triangle_s*, CFVec3&);
		//for raytracing
		PointInsideTriFunc pPointInsideTri;

		static u8 nEdge, nA, nB;
		static f32 fDot[3];
		
		static inline BOOL PointInsideTriXY(struct LN_Triangle_s *pTri, CFVec3& vPoint)
		{
			for (nEdge=0; nEdge<3; nEdge++)
			{
				nA = nEdge; nB = (nA+1)%3;
				fDot[nEdge] = (vPoint.x-pTri->pVtx[nA]->fPos[0])*(pTri->pVtx[nA]->fPos[1]-pTri->pVtx[nB]->fPos[1]) + (vPoint.y-pTri->pVtx[nA]->fPos[1])*(pTri->pVtx[nB]->fPos[0]-pTri->pVtx[nA]->fPos[0]);
			}

			if (fDot[0] < 0.00001f && fDot[1] < 0.00001f && fDot[2] < 0.00001f) { return (TRUE); }
			if (fDot[0] > -0.00001f && fDot[1] > -0.00001f && fDot[2] > -0.00001f) { return (TRUE); }
			return (FALSE);
		}

		static inline BOOL PointInsideTriXZ(struct LN_Triangle_s *pTri, CFVec3& vPoint)
		{
			for (nEdge=0; nEdge<3; nEdge++)
			{
				nA = nEdge; nB = (nA+1)%3;
				fDot[nEdge] = (vPoint.x-pTri->pVtx[nA]->fPos[0])*(pTri->pVtx[nA]->fPos[2]-pTri->pVtx[nB]->fPos[2]) + (vPoint.z-pTri->pVtx[nA]->fPos[2])*(pTri->pVtx[nB]->fPos[0]-pTri->pVtx[nA]->fPos[0]);
			}
			if (fDot[0] < 0.00001f && fDot[1] < 0.00001f && fDot[2] < 0.00001f) { return (TRUE); }
			if (fDot[0] > -0.00001f && fDot[1] > -0.00001f && fDot[2] > -0.00001f) { return (TRUE); }
			return (FALSE);
		}

		static inline BOOL PointInsideTriYZ(struct LN_Triangle_s *pTri, CFVec3& vPoint)
		{
			for (nEdge=0; nEdge<3; nEdge++)
			{
				nA = nEdge; nB = (nA+1)%3;
				fDot[nEdge] = (vPoint.y-pTri->pVtx[nA]->fPos[1])*(pTri->pVtx[nA]->fPos[2]-pTri->pVtx[nB]->fPos[2]) + (vPoint.z-pTri->pVtx[nA]->fPos[2])*(pTri->pVtx[nB]->fPos[1]-pTri->pVtx[nA]->fPos[1]);
			}
			if (fDot[0] < 0.00001f && fDot[1] < 0.00001f && fDot[2] < 0.00001f) { return (TRUE); }
			if (fDot[0] > -0.00001f && fDot[1] > -0.00001f && fDot[2] > -0.00001f) { return (TRUE); }
			return (FALSE);
		}

		inline void SetTriFunc(PointInsideTriFunc pFunc) { pPointInsideTri = pFunc; }
		
	} LN_Triangle_t;

protected:

	typedef struct
	{
		CFVec2 vMin, vMax; //bounding box in UV space.
		CFVec3 vN;
		LN_Triangle_t *pHead; //linked list of triangles in the cluster.
		u32 nTri;

		s32 nGridX, nGridY;
		s16 nMap;
	} LN_Cluster_t;

	typedef struct
	{
		//max area free
		u32 nMaxX, nMaxY;
		//what cells are occupied?
		u8 anCell[LS_GRID_AREA];
	} LN_Map_t;

	struct LN_kDOP_t
	{
		FkDOP_Interval_t	aIntervals[LS_KDOP_AXES];
		u32 nTriCount;
		LN_Triangle_s *pFirstTriangle;
		LN_kDOP_t *papChildren[2];
	};

	LN_kDOP_t	*m_pakDOPTree;
	LN_kDOP_t	*m_pakDOPPool;
	u32			m_nkDOPPoolSize;
	u32			m_nPoolkDOPsUsed;

	//typedef BOOL (*CompareFunc)(float, float);
	typedef int (*CompareFunc)(const void *, const void *);

	u32 m_nNumTri;
	u32 m_nNumVtx;
	u32 m_nNumLights;

	u16 m_nNumVtxGroup;
	u16 m_nNumTriGroup;
	u16 m_nNumVolumes;
	s16 m_nCurVolume;
	s16 m_nCurVolume_Vtx;
	u32 m_nClusters;
	u32 m_nClusters_Motif[LS_MAX_MOTIFPERVOLUME];
	u16 m_nNumMaterials;

	f32 m_fUpdateInterval;
	f64 m_fUpdateCount;

	f32 m_fPackQuality;
	
	LN_Triangle_t *m_TriangleList;
	LN_Vertex_t *m_VertexList;
	LN_Light_t m_LightList[LS_MAX_LIGHTS];
	LN_Volume_t m_VolumeList[LS_MAX_VOLUMES];
	LN_Material_t m_MaterialList[LS_MAX_MATERIALS];
	LN_Map_t m_MapList[LS_MAX_MAPS];

	u32 m_nVtxGroup[LS_MAX_VTX_GROUP];
	u32 m_nTriGroup[LS_MAX_TRI_GROUP];
	u32 m_nVolumeVtxGroup[LS_MAX_VOLUMES];
	s16 m_nVolumeMtlIdx[LS_MAX_VOLUMES];
	u16 m_nGeoInstanceVolume;

	u8 *m_anVisited;
	LN_Cluster_t *m_Clusters;
	LN_Cluster_t *m_Clusters_Motif[LS_MAX_MOTIFPERVOLUME];
	LN_Material_t *m_CurMaterial;

	CFVec2 m_ClusterDim;

	CLightMapTex *m_pLightMapList[LS_MAX_LIGHTMAPTEX];
	CLightMapTex *m_pZMaskList[LS_MAX_LIGHTMAPTEX];
	CLightMapTex *m_pEmissiveList[LS_MAX_LIGHTMAPTEX];

	u32 m_nMaxMemory;
	f32 m_fSubSample;
	u16 m_nNumLightMaps;
	u16 m_nNumZMask;
	u16 m_nNumEmissive;
	f32 m_fMaxLightEffectMult;
	u32 m_nMaxMotifIndex;
	u32 m_nQuality;
	u32 m_nSkyLightingQuality; //0=no skylight, 1=12 samples, 2=92 samples.

	CFColorRGB m_fHorizColor;
	CFColorRGB m_fZenithColor;
	f32 m_fSkyLightIntens;

	u32 m_nGeoInstStart;

	u32 m_nMotifList[ LS_MAX_UNIQUE_MOTIF ];
	u8 m_nUniqueMotif;

	char m_szLevelName[12];

//	CWinPrintf *m_pWinPrintf;
	HWND m_MainHwnd;

	LN_Triangle_t *m_pOrigTri;

	CCompileDlg *m_pDlg;

	void ClampF1(f32 *pfClr);
    void ClampF2(f32 *pfClr);
	void ClampF3(f32 *pfClr);
	void ClampF4(f32 *pfClr);

	BOOL ValueInArray32(u32 nArraySize, u32 *pArray, u32 nValue);
	BOOL ValueInArray16(u16 nArraySize, u16 *pArray, u16 nValue);
	BOOL ValueInArrayML(u16 nArraySize, LN_MotifIndex_t *pArray, u16 nValue);

	s32 FindLight(LS_LightType_e LightType, CFVec3& vDir, CFVec3& vPos, f32 fOOR2);
	void AddTriEdgesToNeighborList(LN_Triangle_t *pTri, CFVec3& vN, LN_Triangle_t **pNeighborList, u32& nNeighbor);
	void BuildLMapUVs(u32 nStart, u32 nEnd);
	void ScaleClusterUV(LN_Cluster_t *pCluster, f32 fSX, f32 fSY);
	void GenerateLightMaps(BOOL bUVOnly=FALSE);
	void OutputLightMaps();

	//test
	void CalcSampleSize();
	BOOL AllocateLightMapForVolume(u32 nVolume, u32 *nW, u32 *nH, f32 *fSampleSize);
	BOOL AllocLMapBlock(u32 w, u32 h, u32 *x, u32 *y, u32 *nW, u32 *nH);
	void GenerateLightMapMatrix(LN_Triangle_t *pTri, f32 *fSampleSize);
	//

	void NewMap();
	BOOL AddToMap(LN_Cluster_t *pCluster, s16 nMap, BOOL bFirst=FALSE);

	void SetPackParam(f32 fMaxX, f32 fMaxY, u32 nLarge, f32 fTX, f32 fTY, f32 fArea);
	void PackIntoSingleMap();
	void RescaleClusters(u32 nMap, f32 fU, f32 fV, f32 fW, f32 fH);
	void PackUV(u32 nUVSet, u32 nStartTri, u32 nEndTri, LN_Volume_t *pVolume, BOOL bIter=FALSE, u8 nRePack=0, BOOL bScale=FALSE);
    void PackUV_Motif(u32 nUVSet, u32 nStartTri, u32 nEndTri, BOOL bIter, u16 nMotif, u16 nVolume);
	void ReformCluster(LN_Cluster_t *pCluster, f32 fU0, f32 fU1, f32 fV0, f32 fV1);
	void PackClusters();
	void BuildClusters(u32 nStartTri, u32 nEndTri);
	void BuildClusters_Motif(u32 nStartTri, u32 nEndTri, u32 nMotif, u32 nVolume);
	BOOL CheckRayIntersectV(f32 fV, u32 nStartTri, u32 nEndTri);
	BOOL CheckRayIntersectU(f32 fU, u32 nStartTri, u32 nEndTri);
	BOOL MoveV(f32 fStartV, f32 fEndV, u32 nStartTri, u32 nEndTri);
	BOOL MoveU(f32 fStartU, f32 fEndU, u32 nStartTri, u32 nEndTri);

	void CopyUV_Motif(u32 nMotif, u32 nVolume);

	u32 BuildCluster(u32 nTri, LN_Cluster_t *pCluster, u32 nStartTri, u32 nEndTri);
	void FindClusterExtents();
	void SortClusters(CompareFunc pFunc);
    void MoveClusters(f32 fDU, f32 fDV, CLightMapTex *pTex, BOOL bMoveFromCurrent=FALSE, u8 nRePack=0);
	void MoveClusterUV(LN_Cluster_t *pCluster, f32 fDU, f32 fDV);
	void ConvertClusterUV(LN_Cluster_t *pCluster, CLightMapTex *pLMap);
	BOOL CheckClusterOverlap(u32 nEnd, LN_Cluster_t *pCluster, CFVec2& vMin, CFVec2& vMax);
	BOOL CheckClusterOverlapDX(u32 nEnd, LN_Cluster_t *pCluster, CFVec2& vMin, CFVec2& vMax);
	BOOL CheckClusterOverlapDY(u32 nEnd, LN_Cluster_t *pCluster, CFVec2& vMin, CFVec2& vMax);
	BOOL GetOverlappingCluster(LN_Cluster_t *pCluster, LN_Cluster_t **pOverlap, u32 nIdx);
	void CombineClusters(LN_Cluster_t *pCluster, LN_Cluster_t *pOverlap);

	u32 BuildCluster_Motif(u32 nTri, LN_Cluster_t *pCluster, u32 nStartTri, u32 nEndTri, u32 nMotif, u32 nVolume);
	void FindClusterExtents_Motif(u32 nMotif, u32 nVolume);
	void SortClusters_Motif(CompareFunc pFunc, u32 nMotif, u32 nVolume);
    void MoveClusters_Motif(f32 fDU, f32 fDV, BOOL bMoveFromCurrent, u32 nMotif, u32 nVolume);
	void MoveClusterUV_Motif(LN_Cluster_t *pCluster, f32 fDU, f32 fDV, u32 nMotif, u32 nVolume);
	BOOL CheckClusterOverlap_Motif(u32 nEnd, LN_Cluster_t *pCluster, CFVec2& vMin, CFVec2& vMax, u32 nMotif, u32 nVolume);
	BOOL GetOverlappingCluster_Motif(LN_Cluster_t *pCluster, LN_Cluster_t **pOverlap, u32 nIdx, u32 nMotif, u32 nVolume);
	void CombineClusters_Motif(LN_Cluster_t *pCluster, LN_Cluster_t *pOverlap, u32 nMotif, u32 nVolume);

	CFVec2 FindMinUV_Tri(LN_Triangle_t *pTri);
	CFVec2 FindMaxUV_Tri(LN_Triangle_t *pTri);
	CFVec2 FindMinUV_Tri_Motif(LN_Triangle_t *pTri, u16 nUVSet);
	CFVec2 FindMaxUV_Tri_Motif(LN_Triangle_t *pTri, u16 nUVSet);
	BOOL IndexInTri(LN_Triangle_t *pTri, u32 nIndex);
	BOOL TriHasMotif(LN_Triangle_t *pTri, u32 nMotifIdx, u32 nVolume);
	u16 FindUVSet(LN_Volume_t *pVolume, u32 nMotif);

	u16 GetLightMapIdx(CLightMapTex *pLightMap);

	u32 GetVBIndex(u32 nVtx, u32 nVolume);
	u32 GetVtxIndex(u32 nVtx, u32 nVBIdx, u32 nVolume);

	void GenerateMotifList(LN_Material_t *pMtl, u32 nStart, u32 nEnd);
    void AddMaterialMotif(LN_Material_t *pMtl, u16 nMotifIndex);
	u16 GetMtlMotifIdx(LN_Material_t *pMtl, u16 nMotif);

	CLightMapTex *AddZMask(cchar *pszZMask);
	CLightMapTex *AddEmissive(cchar *pszEmissive);

	BOOL BuildkDOPTree( void );
	void GeneratekDOP( LN_kDOP_t *pkDOP, BOOL bRootkDOP );

	void BuildConnectivityData();

	void AddMotif(u32 nMotif);
	u32 FindMotifLMap(u32 nMotif);

	static inline int CompareLess( const void *arg1, const void *arg2 );
	static inline int CompareGreater( const void *arg1, const void *arg2 );

	FINLINE BOOL CLightingSolution::VtxEqual(CLightingSolution::LN_Vertex_t *pVtxA, CLightingSolution::LN_Vertex_t *pVtxB)
	{
		return ( fmath_Abs(pVtxA->fPos[0] - pVtxB->fPos[0]) < LS_VE_EPS && fmath_Abs(pVtxA->fPos[1] - pVtxB->fPos[1]) < LS_VE_EPS && fmath_Abs(pVtxA->fPos[2] - pVtxB->fPos[2]) < LS_VE_EPS )?(TRUE):(FALSE);
	}

	FINLINE BOOL CLightingSolution::EdgeMatch( LN_Triangle_t *pT0, LN_Triangle_t *pT1, s32 &nT0Edge, s32 &nT1Edge )
	{
		s32 nNumM=0;
		f32 fD = pT0->fRadius+pT1->fRadius;
		u8 nMatchT0[2];
		u8 nMatchT1[2];

		nT0Edge = nT1Edge = -1;

		if ( fmath_Abs(pT0->vCen.x - pT1->vCen.x) > fD || fmath_Abs(pT0->vCen.y - pT1->vCen.y) > fD || fmath_Abs(pT0->vCen.z - pT1->vCen.z) > fD )
		{
			return FALSE;
		}
		u8 a, b;
		for ( a = 0; a < 3 && nNumM < 2; a++)
		{
			for ( b = 0; b < 3; b++ )
			{
				if ( VtxEqual(pT0->pVtx[a], pT1->pVtx[b]) )
				{
					nMatchT0[ nNumM ] = a;
					nMatchT1[ nNumM ] = b;
					nNumM++;
					break;
				}
			}
		}

		if (nNumM != 2) 
		{
			return FALSE;
		}
			
		// found a match
		if ( (nMatchT0[0] == 0 && nMatchT0[1] == 1) || (nMatchT0[1] == 0 && nMatchT0[0] == 1) )
		{
			nT0Edge = 0;
		}
		else if ( (nMatchT0[0] == 1 && nMatchT0[1] == 2) || (nMatchT0[1] == 1 && nMatchT0[0] == 2) )
		{
			nT0Edge = 1;
		}
		else
		{
			nT0Edge = 2;
		}
		if ( (nMatchT1[0] == 0 && nMatchT1[1] == 1) || (nMatchT1[1] == 0 && nMatchT1[0] == 1) )
		{
			nT1Edge = 0;
		}
		else if ( (nMatchT1[0] == 1 && nMatchT1[1] == 2) || (nMatchT1[1] == 1 && nMatchT1[0] == 2) )
		{
			nT1Edge = 1;
		}
		else
		{
			nT1Edge = 2;
		}

		return TRUE;
	}
};
