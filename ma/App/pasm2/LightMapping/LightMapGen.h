#pragma once

#include "fvid.h"
#include "fxfm.h"
//#include "WinPrintf.h"

#include "LightingSolution.h"
#include "LightMapTex.h"

class CFMeshInst;
class CFLight;
class CCompileDlg;

#define LMG_NUMBER_OF_PROGRESS_BAR_UPDATES		1000
#define LMG_INCLUDE_STATIC_APE_OBJECTS			TRUE


//
//
//
struct LMG_ObjectInstance_t
{
	u16   nApeObjectIdx;		// The index into the vis data's ape object array
	BOOL8 bAcceptsLightmaps;	// This object instance receives lightmaps
	u8    _PAD;
	s32   nLMVolumeIdx;			// The LM system volume that this object instance is represented by

	LMG_ObjectInstance_t *pNextInstance;
};


//
//
//
struct LMG_Object_t
{
	u32  nVertexCount;

	BOOL8 bAtLeastOneInstanceLightmapped;
	BOOL8 bAtLeastOneInstanceVertRadLit;
	u16  nCompileResult;	// The results from the precompile stage for this asset

    u16  nInstanceCount;
	u16  nMaterialCount;
	u16  *panMaterialVertexCount;

	LMG_ObjectInstance_t *pFirstInstance;
	LMG_Object_t *pNextObject;
};


BOOL LMG_StaticallyLightMaterial( KongMat_t *pKMat );


class CLightMapGen
{
public:
	typedef enum
	{
		LT_NONE=0,
		LT_VERTEX,
		LT_RAYTRACE,
		LT_RADIOSITY,

		LT_NUM_LTYPES,

		LT_MIN_MEMORY = 100, //100K
	} LightMapGen_LightingType_e;

	typedef enum
	{
		LIGHT_OUT_VERTEX=0,
		LIGHT_OUT_LMAP,

		LIGHT_OUT_NUM
	} LightMapGen_LightOut_e;

	CLightMapGen(void);
	~CLightMapGen(void);

	void FreeData();

	//void SetWinPrintf(CWinPrintf *pWinPrintf) { m_pWinPrintf = pWinPrintf; }
	void SetParam(u32 nLightMapQuality, f32 fMaxMemory, f32 fSubSample, cchar *pszLevelName, BOOL bFilterLightMaps);
	void SetSkyLightParam(u32 nQuality, CFColorRGB& horizClr, CFColorRGB& zenithClr, f32 fIntens) { m_LightingSolution->SetSkyLightParam(nQuality, horizClr, zenithClr, fIntens); }
	void InitWithWorldData( CKongToVisFile *pKongToVis, CCompileDlg *pDlg, LMG_Object_t *pStaticObjects );
	void InitWithKongData( KongMesh_t *pKongMesh, CCompileDlg *pDlg );
	BOOL Build( CCompileDlg *pDlg, BOOL bUVsOnly = FALSE, BOOL bVertexOnly=FALSE );
	//for volume nVolume, vertex buffer nVBIndex, returns the number of uv sets and an array of uv sets: pLightMapUV[nUVSet][nVtx]
	inline CLightingSolution::LN_MaterialExport_t *ExportMaterial(u32 nVolume, u32 nMaterial) { return m_LightingSolution->ExportMaterial(nVolume, nMaterial); }
	inline CLightingSolution::LN_MaterialExport_t *ExportMaterialGeo(u32 nGeo, u32 nMaterial);

	LightMapGen_LightOut_e GetLightOut(u32 nVolume, u32 nMaterial) { return LIGHT_OUT_VERTEX; }
	inline void GetVtxClr(u32 nVolume, u32 nMaterial, u32 nVtx, f32& fRed, f32& fGreen, f32& fBlue, f32& fAlpha) { m_LightingSolution->GetVtxClr(nVolume, nMaterial, nVtx, fRed, fGreen, fBlue, fAlpha); }
	inline u32 GetVtxUserData(u32 nVolume, u32 nMaterial, u32 nVtx ) { return m_LightingSolution->GetVtxUserData(nVolume, nMaterial, nVtx); }

	inline void FreeMaterial(CLightingSolution::LN_MaterialExport_t *pExpMtl) { m_LightingSolution->FreeMaterial(pExpMtl); }
	inline u32 GetNumLightMaps_Volume(u32 nVolume) { return m_LightingSolution->GetNumLightMaps_Volume(nVolume); }
	inline char *GetLightMap_Volume(u32 nVolume, u32 nLightMap) { return m_LightingSolution->GetLightMap_Volume(nVolume, nLightMap); }
	inline u16 GetLightMapMotif_Volume(u32 nVolume, u32 nLightMap) { return m_LightingSolution->GetLightMapMotif_Volume(nVolume, nLightMap); }
	inline u32 GetNumLightMaps_Geo(u32 nGeo);
	inline char *GetLightMap_Geo(u32 nGeo, u32 nLightMap);
    
	char *GetLightMapDirectory() { return m_LightingSolution->GetLightMapDirectory(); }

	inline void SetLocalDir(cchar *pszDir) { CLightMapTex::SetLocalDir(pszDir); }

	void SetHwnd(HWND hwnd) { if (m_LightingSolution) { m_LightingSolution->SetHwnd(hwnd); } }
    
private:
	LightMapGen_LightingType_e m_LightMapType;
	u32 m_nLightMapQuality;
	u32 m_nMaxMemory; //in KB

	s32 m_nCurrentVolume;

	CLightingSolution *m_LightingSolution;
	CLightingSolution::LN_UV_Set **m_pTempUVBuffer;
	f32 m_fAmbRed, m_fAmbGreen, m_fAmbBlue;

	void AddVolumeKongMesh( s32 nVolume, KongMesh_t *pKongMesh, CFSphere *pSphere, CFMtx43 *pMtx = NULL, BOOL bAcceptLMs = TRUE );
	void AppendGeometryCount( KongMesh_t *pKongMesh, u32& nTri, u32& nVtx );
	void AddApeLight( ApeLight_t *pLight );
	void TransformVtx(CLightingSolution::LS_LightingVtx_t& Vtx, CFMtx43A& Mtx);

	CLightingSolution::LS_LightingVtx_t GetLightingVtx(u32 nVtxType, void *pVtxData, u16 nVIdx);

	//CWinPrintf *m_pWinPrintf;
};

