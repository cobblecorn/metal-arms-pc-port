#pragma once
#include "LightingSolution.h"

class CCompileDlg;

class CRayTracer :
	public CLightingSolution
{
public:
	CRayTracer(void);
	~CRayTracer(void);

	//Lighting solution specific Build(). This is different for each lighting solution.
	virtual BOOL Build(f32 fAmbRed, f32 fAmbGreen, f32 fAmbBlue);

	virtual void CalculateDialogUpdateParams();

protected:

	enum
	{
		_RAY_HIT = 0,
		_RAY_PASS = 1,
	};

	BOOL m_bFilterRayTrace;

	//Specific lighting function for Lambertian lighting.
	void BakeLighting(u16 nMotif);

	u32 PartitionWorld();
	u8 RayTrace(LN_Triangle_t *pTri, LN_Light_t *pLight, CFVec3& vStart, CFVec3& vEnd);
	BOOL PointInsideTri(LN_Triangle_t *pTri, CFVec3& vPoint);
	BOOL PointInsideTriSphere(LN_Triangle_t *pTri, CFVec3& vPoint);
	BOOL TriEffectedByLight(LN_Triangle_t *pTri, LN_Light_t *pLight);
};
