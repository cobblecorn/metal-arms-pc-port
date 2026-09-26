//////////////////////////////////////////////////////////////////////////////////////
// MagnetDevice.h - Magnetic Devices in Coliseum Minigame.
//
// Author: Jeremy Chernobieff
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2002
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 03/27/03 Chernobieff	Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _MAGNET_DEVICE_H_
#define _MAGNET_DEVICE_H_ 1

// =============================================================================================================

#include "fang.h"
#include "fmath.h"
#include "fparticle.h"
#include "fvis.h"

#include "gamecam.h"
#include "damage.h"

class CBot;
class CMeshEntity;
class CFWorldMesh;
class CFWorldTracker;
class CEntity;

typedef void (*MagnetDeviceCallbackFunc_t)(void *, void *, CFVec3& vPos);

FCLASS_ALIGN_PREFIX class CMagnetDevice
{
public:

	enum MagCollisionTest
	{
		MCOL_LIST=0,
		MCOL_ALLBOTS,

		MCOL_NUM_COLTESTS
	};

	CMagnetDevice();
//	~CMagnetDevice();

	void SetMeshEntity(CMeshEntity *pEntity);
	void SetActivateCallback(void *pUserData, MagnetDeviceCallbackFunc_t pCallback) { m_pCallbackFunc = pCallback; m_pUserData = pUserData; }
	void SetMagnetFXCallback(MagnetDeviceCallbackFunc_t pCallback) { m_pFXCallbackFunc = pCallback; }
	void Reset();
	void Work();
	void Disable();
	FINLINE CFWorldMesh *GetWorldMesh() { return m_pWorldMesh; }

	BOOL RayCollidesWithTarget(const CFVec3A *vRayStart, const CFVec3A *vRayEnd, CFVec3A *vCenter);

	static void AddCollisionEntity(CEntity *pEntity);
	static void ClearCollisionEntity();
	static void SetCollisionTest(u32 nColTest) { m_nColTest = nColTest; }
	
private:
	
	typedef enum
	{
		STATE_IDLE=0,
		STATE_PULL,
		STATE_PUSH,

		MAX_COLLISION_SIZE=20
	} MagnetDeviceState_e;

	CMeshEntity *m_pMeshEntity;
	CFWorldMesh *m_pWorldMesh;
	CBot *m_pCurDamageBot;
	static CEntity *m_pCollisionList[MAX_COLLISION_SIZE];

	static u32 m_nNumCollisionEntity;
	f32 m_fAnimSpd;
	f32 m_fUnitLerp;
	f32 m_fUnitTargetRot;
	BOOL m_bParticleSpawn;

	CFVec3 m_vMagnetPos, m_vMagnetDir;
	CFVec3 m_vNewMagnetPos, m_vNewMagnetDir;
	CFVec3 m_vMagnetOrigin;
    
	FParticle_DefHandle_t m_hSparkParticleDef;
	FParticle_DefHandle_t m_hMagnetParticleDef;
	FParticle_EmitterHandle_t m_hEmitter;
	FParticle_EmitterHandle_t m_hMagnetEmitter;
	MagnetDeviceCallbackFunc_t m_pCallbackFunc, m_pFXCallbackFunc;
	void *m_pUserData;

	//Cone data
	f32 m_fSinReciprocal;
	f32 m_fCosSqr, m_fSinSqr;
	CFVec3 m_vAxis;
	//

	BOOL m_bDisable;
	static u32 m_nColTest;
	
	MagnetDeviceState_e m_State;
	CDamageProfile *m_pDamageProfile;

	gamecam_CamBotTransResult_t m_FixedCamera;

	static u32 m_nNumCollisions;
    static BOOL MagnetDeviceCollisionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	
	void ApplyDamage(CEntity *pEDamagee, CFVec3& vPos, CFVec3& vDir);
	BOOL InsideCone(const CFSphere *vSphere);

	void RotateTarget(f32 fRadians);
	void HandleInsideCone(CFVec3& vOffs, BOOL& bTurnOffSparks, f32 fMagXZ2, u32 i);
	void GenerateCollisionList();
	
	FCLASS_STACKMEM_ALIGN(CMagnetDevice);
} FCLASS_ALIGN_SUFFIX;

// =============================================================================================================

#endif
