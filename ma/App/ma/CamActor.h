//////////////////////////////////////////////////////////////////////////////////////
// CamActor.h - Actor type of camera.
//
// Author: Justin Link
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
// 04/01/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _CAMACTOR_H_
#define _CAMACTOR_H_ 1

// =============================================================================================================

#include "fang.h"
#include "Actor.h"
#include "CamManual.h"

// =============================================================================================================

//class CCamManualInfo;

// =============================================================================================================

const u32 CCamActor_uPoolSize = 10;

// =============================================================================================================

FCLASS_ALIGN_PREFIX class CCamActor : public CActor
{
public:
	CCamActor();
//	~CCamActor();

	void Init(CFVec3AObj *pV3Obj, CFQuatObj *pQuatObj);
	void Work();
	void AddToWorld();
	void RemoveFromWorld();

	// Gets and returns objects from pool.
	static CCamActor *Acquire();
	void Release();

	static BOOL InitSystem();
	static void UninitSystem();
	static void Reset();

private:
	CCamManualInfo m_oCamData;

	static cell AMX_NATIVE_CALL CamActor_Acquire(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CamActor_Release(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CamActor_IsValid(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CamActor_Init(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CamActor_Activate(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CamActor_Deactivate(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CamActor_SetPosObj(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CamActor_SetQuatObj(AMX *pAMX, cell *aParams);

	// These get registered in the script system by InitSystem().
	static AMX_NATIVE_INFO m_aNatives[];

	static CCamActor *m_pCamActor;

	FCLASS_STACKMEM_ALIGN(CCamActor);
} FCLASS_ALIGN_SUFFIX;

// =============================================================================================================

#endif
