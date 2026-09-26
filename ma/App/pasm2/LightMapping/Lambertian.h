#pragma once
#include "LightingSolution.h"

class CCompileDlg;

class CLambertian : public CLightingSolution
{
public:
	CLambertian(void);
	~CLambertian(void);

	//Lighting solution specific Build(). This is different for each lighting solution.
	virtual BOOL Build(f32 fAmbRed, f32 fAmbGreen, f32 fAmbBlue);

	//These are optional overrides for more advanced lighting solutions.
	virtual void AddTriangle(s32 nVolumeIdx, u32 nV0, u32 nV1, u32 nV2);
	virtual void AddVertex(LS_LightingVtx_t Vtx);

	virtual void SetVtxGroup(u32 nGroup);
	virtual void SetTriGroup(u32 nGroup);

	virtual void AddLight(u32 nVolume, LS_LightType_e LightType, CFVec3& vDir, CFVec3& vPos, CFVec3& vDAtten, CFVec3& vAAtten, CFVec3& vColor, f32 fColorIntens, f32 fAttenItens, f32 fOOR2, u16 nMotifIndex);
	virtual void OffsetTriIndexs(u32 nGroup, u32 nStartIdx, u32 nSize, u32 nOffs);

	virtual void CalculateDialogUpdateParams();

private:

	//Specific lighting function for Lambertian lighting.
	void BakeLighting(u16 nMotif);

	void RenderClusters();

	u32 PartitionWorld();
};
