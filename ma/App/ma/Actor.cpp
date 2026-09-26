//////////////////////////////////////////////////////////////////////////////////////
// Actor.cpp - Script Actor classes for Mettle Arms.
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
// 03/21/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "Actor.h"

#include "FScriptSystem.h"
#include "fres.h"
#include "fclib.h"
#include "FNativeUtil.h"

#include "bot.h"
#include "meshentity.h"
#include "FQuatObj.h"
#include "FVec3Obj.h"

// =============================================================================================================

BOOL CActor::m_bSystemInitted = FALSE;
//BOOL8 *CActor::m_abInUse = NULL;
//u32 CActor::m_uPoolSize;

// =============================================================================================================

AMX_NATIVE_INFO CActor::m_aNatives[] =
{
	NULL,						NULL
};

// =============================================================================================================

CActor::CActor()
{
	_ClearCActorData();
}

void CActor::_ClearCActorData()
{
	m_pPosObj = NULL;
	m_pQuatObj = NULL;
	m_bIsInitted = FALSE;
}
// =============================================================================================================

BOOL CActor::InitSystem()
{
	m_bSystemInitted = TRUE;

	// TODO : Perhaps this script wrapping stuff should be controlled by an external system.
	// Register the script-accessible functions for this class.
	AMX_NATIVE_INFO *pCurNative = &(m_aNatives[0]);
	while(pCurNative->func != NULL)
	{
		FASSERT(pCurNative->name && fclib_strlen(pCurNative->name) < 20);  //HEHEHE 19 seems to be the magic number not to exceed.
		CFScriptSystem::RegisterNative(pCurNative->name, pCurNative->func);
		++pCurNative;
	}

	//// Call the InitSystem() methods of any derived classes.
	//
	CMeshActor::InitSystem();
	CBotActor::InitSystem();
//	CCamActor::InitSystem();
	//
	////

	return(TRUE);
}

// =============================================================================================================

void CActor::UninitSystem()
{
	//// Call the UninitSystem() methods of any derived classes.
	//
//	CCamActor::UninitSystem();
	CBotActor::UninitSystem();
	CMeshActor::UninitSystem();
	//
	////

	m_bSystemInitted = FALSE;
}

// =============================================================================================================

BOOL CActor::LevelInit()
{
//	if(!CCamActor::LevelInit())
//	{
//		return(FALSE);
//	}
	if(!CBotActor::LevelInit())
	{
		return(FALSE);
	}
	if(!CMeshActor::LevelInit())
	{
		return(FALSE);
	}

	return(TRUE);
}

// =============================================================================================================

void CActor::LevelUninit()
{
	CMeshActor::LevelUninit();
	CBotActor::LevelUninit();
//	CCamActor::LevelUninit();
}

// =============================================================================================================

void CActor::Reset()
{
	CMeshActor::Reset();
	CBotActor::Reset();
//	CCamActor::Reset();
}

// =============================================================================================================
/*
void CActor::ClearPool()
{
	if(m_abInUse == NULL)
	{
		return;
	}
	if(m_uPoolSize == 0)
	{
		return;
	}
	// Mark the entire pool as available.
	u32 uCurPoolIdx;
	for(uCurPoolIdx = 0; uCurPoolIdx < m_uPoolSize; ++uCurPoolIdx)
	{
		m_abInUse[uCurPoolIdx] = FALSE;
	}
}*/

// =============================================================================================================

CMeshActor *CMeshActor::m_aPool = NULL;
BOOL8 *CMeshActor::m_abInUse = NULL;

// =============================================================================================================

AMX_NATIVE_INFO CMeshActor::m_aNatives[] =
{
	"MeshActor_Acquire",		CMeshActor::MeshActor_Acquire,
	"MeshActor_Release",		CMeshActor::MeshActor_Release,
	"MeshActor_IsValid",		CMeshActor::MeshActor_IsValid,
	"MeshActor_Init",			CMeshActor::MeshActor_Init,
	"MeshActor_Activate",		CMeshActor::MeshActor_Activate,
	"MeshActor_Deactivat",		CMeshActor::MeshActor_Deactivate,
	"MeshActor_SetPosObj",		CMeshActor::MeshActor_SetPosObj,
	"MeshActor_SetQuatOb",		CMeshActor::MeshActor_SetQuatObj,

	NULL,						NULL
};

// =============================================================================================================
CMeshActor::CMeshActor(void)
: CActor()
{
	_ClearCMeshActorData();
}

void CMeshActor::_ClearCMeshActorData(void)
{
	m_pME = NULL;
}


void CMeshActor::Init(CMeshEntity *pME, CFVec3AObj *pV3Obj, CFQuatObj *pQuatObj)
{
	FASSERT(pME != NULL);
	FASSERT(pV3Obj != NULL);
//	FASSERT(pQuatObj != NULL);

	// Record the pointers that they pass in.
	m_pME = pME;
	SetPosObj(pV3Obj);
	SetQuatObj(pQuatObj);

	m_bIsInitted = TRUE;
}

// =============================================================================================================

// Get the position and orientation from the respective objects and update the WorldMesh with them.
void CMeshActor::Work()
{
	FASSERT(m_bIsInitted);
	if((m_pQuatObj != NULL) && (!m_pQuatObj->IsInitted()))
	{
		return;
	}
	if(!m_pPosObj->IsInitted())
	{
		return;
	}

	FASSERT(m_pME != NULL);

	CFMtx43A mtxTemp;
	if(m_pQuatObj != NULL)
	{
		mtxTemp.Identity();
		CFQuatA tmpA = m_pQuatObj->GetValue();
		f32 fMag = tmpA.v.MagSq();
		FASSERT(fMag > 0.0f);
		fMag = 1.0f/fmath_AcuSqrt(fMag);
		tmpA.x *= fMag;
		tmpA.y *= fMag;
		tmpA.z *= fMag;
		tmpA.w *= fMag;
		tmpA.BuildMtx(mtxTemp);
	}
	else
	{
		mtxTemp = *m_pME->MtxToWorld();
	}

	mtxTemp.m_vPos = m_pPosObj->GetValue();	

	m_pME->Relocate_RotXlatFromUnitMtx_WS(&mtxTemp);
}

// =============================================================================================================

void CMeshActor::AddToWorld()
{
	m_pME->AddToWorld();
}

// =============================================================================================================

void CMeshActor::RemoveFromWorld()
{
	m_pME->RemoveFromWorld();
}

// =============================================================================================================

CMeshActor *CMeshActor::Acquire()
{
	// Search through the pool for an available entry.
	u32 uPoolIdx;
	for(uPoolIdx = 0; uPoolIdx < CMeshActor_uPoolSize; ++uPoolIdx)
	{
		if(!m_abInUse[uPoolIdx])
			break;
	}

	// Check to see if we found an available entry.
	if(uPoolIdx == CMeshActor_uPoolSize)
	{
		// No available entries...
		DEVPRINTF("CMeshActor::Acquire() : No available entries in pool.\n");
		return(NULL);
	}

	// Mark the entry as used and pass it back.
	m_abInUse[uPoolIdx] = TRUE;
	return(&(m_aPool[uPoolIdx]));
}

// =============================================================================================================

void CMeshActor::Release()
{
	// Search through the pool to see where this came from.
	u32 uPoolIdx;
	for(uPoolIdx = 0; uPoolIdx < CMeshActor_uPoolSize; ++uPoolIdx)
	{
		if(&(m_aPool[uPoolIdx]) == this)
			break;
	}

	// If this asserts, the entry did not come from our pool.
	FASSERT(uPoolIdx != CMeshActor_uPoolSize);
	
	// If this asserts, the entry was already released or was never given out.
	// NKM - Changed this since the logic didn't make any sense.  We want to assert
	//		 that the object IS in use, not that it ISN'T in use.
	//FASSERT(!m_abInUse[uPoolIdx]);
	FASSERT(m_abInUse[uPoolIdx]);

	m_abInUse[uPoolIdx] = FALSE;
}

// =============================================================================================================

void CMeshActor::ClearPool()
{
	if(m_abInUse == NULL)
	{
		return;
	}

	// Mark the entire pool as available.
	u32 uCurPoolIdx;
	for(uCurPoolIdx = 0; uCurPoolIdx < CMeshActor_uPoolSize; ++uCurPoolIdx)
	{
		m_abInUse[uCurPoolIdx] = FALSE;
	}
}

// =============================================================================================================

BOOL CMeshActor::InitSystem()
{
	// Create the pool of CMeshActors.
	m_aPool = fnew CMeshActor[CMeshActor_uPoolSize];
	if(m_aPool == NULL)
	{
		DEVPRINTF("CMeshActor::InitSystem() : Failed to create CMeshActor pool.\n");
		return(TRUE);
	}

	// Create the usage array.
	m_abInUse = (BOOL8 *)(fres_Alloc(sizeof(BOOL8) * CMeshActor_uPoolSize));
	if(m_abInUse == NULL)
	{
		DEVPRINTF("CMeshActor::InitSystem() : Failed to create usage array.\n");
		return(TRUE);
	}

	// TODO: This should probably get moved to an external system.
	// Register the script-accessible functions for this class.
	AMX_NATIVE_INFO *pCurNative = &(m_aNatives[0]);
	while(pCurNative->func != NULL)
	{
		FASSERT(pCurNative->name && fclib_strlen(pCurNative->name) < 20);  //HEHEHE 19 seems to be the magic number not to exceed.
		CFScriptSystem::RegisterNative(pCurNative->name, pCurNative->func);
		++pCurNative;
	}

	return(TRUE);
}

// =============================================================================================================

void CMeshActor::UninitSystem()
{
	ClearPool();
/*	// Mark the entire pool as available.
	if(m_abInUse != NULL)
	{
		u32 uCurPoolIdx;
		for(uCurPoolIdx = 0; uCurPoolIdx < CMeshActor_uPoolSize; ++uCurPoolIdx)
		{
			m_abInUse[uCurPoolIdx] = FALSE;
		}
	}*/

	// Destroy the pool.
	if(m_aPool != NULL)
	{
		fdelete_array( m_aPool );
		m_aPool = NULL;
	}
}

// =============================================================================================================

void CMeshActor::Reset()
{
	ClearPool();
	// TODO : Clear out the pool.
}

// =============================================================================================================

BOOL CMeshActor::LevelInit()
{
	ClearPool();

	return(TRUE);
}

// =============================================================================================================

void CMeshActor::LevelUninit()
{
	ClearPool();
}

// =============================================================================================================

cell AMX_NATIVE_CALL CMeshActor::MeshActor_Acquire(AMX *pAMX, cell *aParams)
{
	CMeshActor *pMA = CMeshActor::Acquire();

	return((cell)(pMA));
}

// =============================================================================================================

cell AMX_NATIVE_CALL CMeshActor::MeshActor_Release(AMX *pAMX, cell *aParams)
{
	CMeshActor *pMA = (CMeshActor *)(aParams[1]);
	if(pMA == NULL)
	{
		DEVPRINTF("CMeshActor::MeshActor_Release(): Passed in a NULL actor.\n");
		return((cell)(0));
	}

	pMA->Release();

	return(0);
}

// =============================================================================================================

cell AMX_NATIVE_CALL CMeshActor::MeshActor_IsValid(AMX *pAMX, cell *aParams)
{
	return(((CMeshActor *)(aParams[1])) != NULL);
}

// =============================================================================================================

cell AMX_NATIVE_CALL CMeshActor::MeshActor_Init(AMX *pAMX, cell *aParams)
{
	CMeshActor *pMA = (CMeshActor *)(aParams[1]);
	if(pMA == NULL)
	{
		DEVPRINTF("CMeshActor::MeshActor_Init(): Passed in a NULL actor.\n");
		return((cell)(0));
	}

	CMeshEntity *pME = (CMeshEntity *)(aParams[2]);
	if(pME == NULL)
	{
		DEVPRINTF("CMeshActor::MeshActor_Init(): Passed in a NULL meshentity.\n");
		return((cell)(0));
	}

	CFVec3AObj *pVO = (CFVec3AObj *)(aParams[3]);
	if(pVO == NULL)
	{
		DEVPRINTF("CMeshActor::MeshActor_Init(): Passed in a NULL vec3obj.\n");
		return((cell)(0));
	}

	CFQuatObj *pQO = (CFQuatObj *)(aParams[4]);
/*	if(pQO == NULL)
	{
		DEVPRINTF("CMeshActor::MeshActor_Init(): Passed in a NULL quatobj.\n");
		return((cell)(0));
	}*/

	pMA->Init(pME, pVO, pQO);

	return(0);
}

// =============================================================================================================

cell AMX_NATIVE_CALL CMeshActor::MeshActor_Activate(AMX *pAMX, cell *aParams)
{
	CMeshActor *pMA = (CMeshActor *)(aParams[1]);
	if(pMA == NULL)
	{
		DEVPRINTF("CMeshActor::MeshActor_Activate() : Passed in a NULL actor.\n");
		return((cell)(0));
	}

	if(!pMA->m_bIsInitted)
	{
		DEVPRINTF("CMeshActor::MeshActor_Activate() : Cannot activate a MeshActor that has not been successfully initted.\n");
		return((cell)(0));
	}

	if(pMA->m_pME == NULL || !pMA->m_pME->IsCreated())
	{
		DEVPRINTF("CMeshActor::MeshActor_Activate() : Cannot activate a MeshActor with a NULL meshentity.\n");
		return((cell)(0));
	}

	pMA->Activate();

	return(0);
}

// =============================================================================================================

cell AMX_NATIVE_CALL CMeshActor::MeshActor_Deactivate(AMX *pAMX, cell *aParams)
{
	CMeshActor *pMA = (CMeshActor *)(aParams[1]);
	if(pMA == NULL)
	{
		DEVPRINTF("CMeshActor::MeshActor_Deactivate(): Passed in a NULL actor.\n");
		return((cell)(0));
	}

	pMA->Deactivate();

	return(0);
}

// =============================================================================================================

cell AMX_NATIVE_CALL CMeshActor::MeshActor_SetPosObj(AMX *pAMX, cell *aParams)
{
	CMeshActor *pMA = (CMeshActor *)(aParams[1]);
	if(pMA == NULL)
	{
		DEVPRINTF("CMeshActor::MeshActor_SetPosObj(): Passed in a NULL actor.\n");
		return((cell)(0));
	}

	CFVec3AObj *pVO = (CFVec3AObj *)(aParams[2]);
	if(pVO == NULL)
	{
		DEVPRINTF("CMeshActor::MeshActor_SetPosObj(): Passed in a NULL vec3obj.\n");
		return((cell)(0));
	}

	pMA->SetPosObj(pVO);

	return(0);
}

// =============================================================================================================

cell AMX_NATIVE_CALL CMeshActor::MeshActor_SetQuatObj(AMX *pAMX, cell *aParams)
{
	CMeshActor *pMA = (CMeshActor *)(aParams[1]);
	if(pMA == NULL)
	{
		DEVPRINTF("CMeshActor::MeshActor_SetQuatObj(): Passed in a NULL actor.\n");
		return((cell)(0));
	}

	CFQuatObj *pQO = (CFQuatObj *)(aParams[2]);
	if(pQO == NULL)
	{
		DEVPRINTF("CMeshActor::MeshActor_SetQuatObj(): Passed in a NULL quatobj.\n");
		return((cell)(0));
	}


	pMA->SetQuatObj(pQO);

	return(0);
}

// =============================================================================================================
// =============================================================================================================
// =============================================================================================================


CBotActor *CBotActor::m_aPool = NULL;
BOOL8 *CBotActor::m_abInUse = NULL;

// =============================================================================================================

AMX_NATIVE_INFO CBotActor::m_aNatives[] =
{
	"BotActor_Acquire",			CBotActor::BotActor_Acquire,
	"BotActor_Release",			CBotActor::BotActor_Release,
	"BotActor_IsValid",			CBotActor::BotActor_IsValid,
	"BotActor_Init1",			CBotActor::BotActor_Init1,
	"BotActor_Init2",			CBotActor::BotActor_Init2,
	"BotActor_Activate",		CBotActor::BotActor_Activate,
	"BotActor_Deactivate",		CBotActor::BotActor_Deactivate,
	"BotActor_SetPosObj",		CBotActor::BotActor_SetPosObj,
	"BotActor_SetQuatObj",		CBotActor::BotActor_SetQuatObj,
	"BotActor_SetAnimMul",		CBotActor::BotActor_SetAnimMult,

	NULL,						NULL
};


CBotActor::CBotActor(void)
: CActor()
{
  _ClearCBotActorData();
}


void CBotActor::_ClearCBotActorData(void)
{
	m_pBot = NULL;
	m_pAnimInst = NULL;
}


// =============================================================================================================
void CBotActor::Init1(CBot *pBot, CFVec3AObj *pV3Obj, CFQuatObj *pQuatObj)
{
	FASSERT(pBot != NULL);
	FASSERT(pV3Obj != NULL);
	FASSERT(pQuatObj != NULL);

	// Record the pointers that they pass in.
	m_pBot = pBot;
	SetPosObj(pV3Obj);
	SetQuatObj(pQuatObj);

	m_pAnimInst = NULL;
	m_nUserAnimSlot = -1;

	m_fAnimMult = 1.0f;

	m_bIsInitted = TRUE;
}

// =============================================================================================================

void CBotActor::Init2(CBot *pBot, CFAnimInst *pAnimInst, s32 nUserAnimSlot)
{
	FASSERT(pBot != NULL);
	FASSERT(pAnimInst != NULL);
	FASSERT(nUserAnimSlot != -1);

	m_pBot = pBot;
	m_pAnimInst = pAnimInst;
	pAnimInst->UpdateTime(0.0f);
	m_nUserAnimSlot = nUserAnimSlot;

	m_pPosObj = NULL;
	m_pQuatObj = NULL;

	m_fAnimMult = 1.0f;

	m_bIsInitted = TRUE;
}

// =============================================================================================================

// Get the position and orientation from the respective objects and update the Bot with them.
void CBotActor::Work()
{
	FASSERT(m_bIsInitted);
	if(m_pQuatObj != NULL && m_pQuatObj->IsInitted() && m_pPosObj != NULL && m_pPosObj->IsInitted())
	{
		// I don't really think this stuff works, I just left it here.
		CFMtx43A mtxTemp;
		mtxTemp.Identity();
		CFQuatA tmpA = m_pQuatObj->GetValue();
		f32 fMag = tmpA.v.MagSq();
		FASSERT(fMag > 0.0f);
		fMag = 1.0f/fmath_AcuSqrt(fMag);
		tmpA.x *= fMag;
		tmpA.y *= fMag;
		tmpA.z *= fMag;
		tmpA.w *= fMag;
		f32 fMagSecond = tmpA.v.MagSq();
		tmpA.BuildMtx(mtxTemp);
		mtxTemp.m_vPos = m_pPosObj->GetValue();	

		m_pBot->Relocate_RotXlatFromUnitMtx_WS(&mtxTemp);
	}

	if(m_pAnimInst != NULL)
	{
		FASSERT(m_nUserAnimSlot != -1);

		m_pAnimInst->DeltaTime(FLoop_fPreviousLoopSecs * m_fAnimMult, TRUE);
		f32 fAnimUnitTime = m_pAnimInst->GetUnitTime();

		if(fAnimUnitTime >= 0.999f)
		{
			m_pAnimInst = NULL;
			m_pBot->UserAnim_Attach(0, NULL);
			m_pBot->UserAnim_SetControlValue(m_nUserAnimSlot, 0.0f);
			m_pBot->UserAnim_DisableAllBones(m_nUserAnimSlot);
		}
		else if(fAnimUnitTime > 0.9f)
		{
			m_pBot->UserAnim_SetControlValue(m_nUserAnimSlot, 10.0f * (1.0f - fAnimUnitTime));
		}
		else if(fAnimUnitTime < 0.1f)
		{
			m_pBot->UserAnim_SetControlValue(m_nUserAnimSlot, 10.0f * fAnimUnitTime);
		}
		else
		{
			m_pBot->UserAnim_SetControlValue(m_nUserAnimSlot, 1.0f);
		}
	}
}

// =============================================================================================================

void CBotActor::AddToWorld()
{
	// Probably don't need to do anything here.
//	m_pBot->AddToWorld();
}

// =============================================================================================================

void CBotActor::RemoveFromWorld()
{
	// Probably don't need to do anything here.
//	m_pME->RemoveFromWorld();
}

// =============================================================================================================

void CBotActor::Activate()
{
	if(m_pAnimInst != NULL)
	{
		FASSERT(m_pBot != NULL);
		FASSERT(m_nUserAnimSlot != -1);
		m_pBot->UserAnim_Attach(m_nUserAnimSlot, m_pAnimInst);
	}

	CActor::Activate();
}

// =============================================================================================================

void CBotActor::Deactivate()
{
	if(m_pAnimInst != NULL)
	{
		FASSERT(m_pBot != NULL);
		FASSERT(m_nUserAnimSlot != -1);
		m_pBot->UserAnim_Attach(m_nUserAnimSlot, NULL);
	}

	CActor::Deactivate();
}

// =============================================================================================================

CBotActor *CBotActor::Acquire()
{
	// Search through the pool for an available entry.
	u32 uPoolIdx;
	for(uPoolIdx = 0; uPoolIdx < CBotActor_uPoolSize; ++uPoolIdx)
	{
		if(!m_abInUse[uPoolIdx])
			break;
	}

	// Check to see if we found an available entry.
	if(uPoolIdx == CBotActor_uPoolSize)
	{
		// No available entries...
		DEVPRINTF("CBotActor::Acquire : No available entries in pool.\n");
		return(NULL);
	}

	// Mark the entry as used and pass it back.
	m_abInUse[uPoolIdx] = TRUE;
	return(&(m_aPool[uPoolIdx]));
}

// =============================================================================================================

void CBotActor::Release()
{
	// Search through the pool to see where this came from.
	u32 uPoolIdx;
	for(uPoolIdx = 0; uPoolIdx < CBotActor_uPoolSize; ++uPoolIdx)
	{
		if(&(m_aPool[uPoolIdx]) == this)
			break;
	}

	// If this asserts, the entry did not come from our pool.
	FASSERT(uPoolIdx != CBotActor_uPoolSize);

	// If this asserts, the entry was already released or was never given out.
	// NKM - Changed this since the logic didn't make any sense.  We want to assert
	//		 that the object IS in use, not that it ISN'T in use.
	//FASSERT(!m_abInUse[uPoolIdx]);
	FASSERT(m_abInUse[uPoolIdx]);

	m_abInUse[uPoolIdx] = FALSE;
}

// =============================================================================================================

void CBotActor::ClearPool()
{
	if(m_abInUse == NULL)
	{
		return;
	}

	// Mark the entire pool as available.
	u32 uCurPoolIdx;
	for(uCurPoolIdx = 0; uCurPoolIdx < CBotActor_uPoolSize; ++uCurPoolIdx)
	{
		m_abInUse[uCurPoolIdx] = FALSE;
	}
}

// =============================================================================================================

BOOL CBotActor::InitSystem()
{
	// Create the pool of CMeshActors.
	m_aPool = fnew CBotActor[CBotActor_uPoolSize];
	if(m_aPool == NULL)
	{
		DEVPRINTF("CBotActor::InitSystem() : Failed to create CBotActor pool.\n");
		return(TRUE);
	}

	// Create the usage array.
	m_abInUse = (BOOL8 *)(fres_Alloc(sizeof(BOOL8) * CBotActor_uPoolSize));
	if(m_abInUse == NULL)
	{
		DEVPRINTF("CBotActor::InitSystem() : Failed to create usage array.\n");
		return(TRUE);
	}

	// TODO: This should probably get moved to an external system.
	// Register the script-accessible functions for this class.
	AMX_NATIVE_INFO *pCurNative = &(m_aNatives[0]);
	while(pCurNative->func != NULL)
	{
		FASSERT(pCurNative->name && fclib_strlen(pCurNative->name) < 20);  //HEHEHE 19 seems to be the magic number not to exceed.
		CFScriptSystem::RegisterNative(pCurNative->name, pCurNative->func);
		++pCurNative;
	}

	return(TRUE);
}

// =============================================================================================================

void CBotActor::UninitSystem()
{
	ClearPool();
/*	// Mark the entire pool as available.
	if(m_abInUse != NULL)
	{
		u32 uCurPoolIdx;
		for(uCurPoolIdx = 0; uCurPoolIdx < CMeshActor_uPoolSize; ++uCurPoolIdx)
		{
			m_abInUse[uCurPoolIdx] = FALSE;
		}
	}*/

	// Destroy the pool.
	if(m_aPool != NULL)
	{
		fdelete_array(m_aPool);
		m_aPool = NULL;
	}
}

// =============================================================================================================

void CBotActor::Reset()
{
	ClearPool();
	// TODO : Clear out the pool.
}

// =============================================================================================================

BOOL CBotActor::LevelInit()
{
	ClearPool();

	return(TRUE);
}

// =============================================================================================================

void CBotActor::LevelUninit()
{
	ClearPool();
}

// =============================================================================================================

cell AMX_NATIVE_CALL CBotActor::BotActor_Acquire(AMX *pAMX, cell *aParams)
{
	CBotActor *pMA = CBotActor::Acquire();

	return((cell)(pMA));
}

// =============================================================================================================

cell AMX_NATIVE_CALL CBotActor::BotActor_Release(AMX *pAMX, cell *aParams)
{
	CBotActor *pMA = (CBotActor *)(aParams[1]);
	if(pMA == NULL)
	{
		DEVPRINTF("CBotActor::BotActor_Release(): Passed in a NULL actor.\n");
		return((cell)(0));
	}

	pMA->Release();

	return(0);
}

// =============================================================================================================

cell AMX_NATIVE_CALL CBotActor::BotActor_IsValid(AMX *pAMX, cell *aParams)
{
	return(((CBotActor *)(aParams[1])) != NULL);
}

// =============================================================================================================

cell AMX_NATIVE_CALL CBotActor::BotActor_Init1(AMX *pAMX, cell *aParams)
{
	CBotActor *pMA = (CBotActor *)(aParams[1]);
	if(pMA == NULL)
	{
		DEVPRINTF("CBotActor::BotActor_Init1(): Passed in a NULL actor.\n");
		return((cell)(0));
	}

	CBot *pBot = (CBot *)(aParams[2]);
	if(pBot == NULL)
	{
		DEVPRINTF("CBotActor::BotActor_Init1(): Passed in a NULL bot.\n");
		return((cell)(0));
	}
	// Replace with CFAnimInst *, user slot, etc.
/*
	CFVec3AObj *pVO = (CFVec3AObj *)(aParams[3]);
	if(pVO == NULL)
	{
		DEVPRINTF("CMeshActor::MeshActor_Init(): Passed in a NULL vec3obj.\n");
		return((cell)(0));
	}

	CFQuatObj *pQO = (CFQuatObj *)(aParams[4]);
	if(pQO == NULL)
	{
		DEVPRINTF("CMeshActor::MeshActor_Init(): Passed in a NULL quatobj.\n");
		return((cell)(0));
	}

	pMA->Init(pME, pVO, pQO);*/

	return((cell)(0));
}

// =============================================================================================================

cell AMX_NATIVE_CALL CBotActor::BotActor_Init2(AMX *pAMX, cell *aParams)
{
	CBotActor *pBA = (CBotActor *)(aParams[1]);
	if(pBA == NULL)
	{
		DEVPRINTF("CBotActor::BotActor_Init2(): Passed in a NULL actor.\n");
		return((cell)(0));
	}

	CBot *pBot = (CBot *)(aParams[2]);
	if(pBot == NULL)
	{
		DEVPRINTF("CBotActor::BotActor_Init2(): Passed in a NULL bot.\n");
		return((cell)(0));
	}

	CFAnimInst *pAnimInst = (CFAnimInst *)(aParams[3]);
	if(pAnimInst == NULL)
	{
		DEVPRINTF("CBotActor::BotActor_Init2(): Passed in a NULL AnimInst.\n");
		return((cell)(0));
	}

	s32 nUserAnimSlot = (s32)(aParams[4]);
	if(nUserAnimSlot == -1)
	{
		DEVPRINTF("CBotActor::BotActor_Init2(): Passed in an invalid user anim slot.\n");
		return((cell)(0));
	}

	pBA->Init2(pBot, pAnimInst, nUserAnimSlot);
	return((cell)(0));
}

// =============================================================================================================

cell AMX_NATIVE_CALL CBotActor::BotActor_Activate(AMX *pAMX, cell *aParams)
{
	CBotActor *pBA = (CBotActor *)(aParams[1]);
	if(pBA == NULL)
	{
		DEVPRINTF("CBotActor::BotActor_Activate() : Passed in a NULL actor.\n");
		return((cell)(0));
	}

	if(!pBA->m_bIsInitted)
	{
		DEVPRINTF("CBotActor::BotActor_Activate() : Cannot activate a CBotActor that has not been successfully initted.\n");
		return((cell)(0));
	}

	pBA->Activate();

	return(0);
}

// =============================================================================================================

cell AMX_NATIVE_CALL CBotActor::BotActor_Deactivate(AMX *pAMX, cell *aParams)
{
	CBotActor *pBA = (CBotActor *)(aParams[1]);
	if(pBA == NULL)
	{
		DEVPRINTF("CBotActor::BotActor_Deactivate(): Passed in a NULL actor.\n");
		return((cell)(0));
	}

	pBA->Deactivate();

	return(0);
}

// =============================================================================================================

cell AMX_NATIVE_CALL CBotActor::BotActor_SetPosObj(AMX *pAMX, cell *aParams)
{
	CBotActor *pBA = (CBotActor *)(aParams[1]);
	if(pBA == NULL)
	{
		DEVPRINTF("CBotActor::BotActor_SetPosObj(): Passed in a NULL actor.\n");
		return((cell)(0));
	}

	CFVec3AObj *pVO = (CFVec3AObj *)(aParams[2]);
	if(pVO == NULL)
	{
		DEVPRINTF("CBotActor::BotActor_SetPosObj(): Passed in a NULL vec3obj.\n");
		return((cell)(0));
	}

	pBA->SetPosObj(pVO);

	return(0);
}

// =============================================================================================================

cell AMX_NATIVE_CALL CBotActor::BotActor_SetQuatObj(AMX *pAMX, cell *aParams)
{
	CBotActor *pBA = (CBotActor *)(aParams[1]);
	if(pBA == NULL)
	{
		DEVPRINTF("CBotActor::BotActor_SetQuatObj(): Passed in a NULL actor.\n");
		return((cell)(0));
	}

	CFQuatObj *pQO = (CFQuatObj *)(aParams[2]);
	if(pQO == NULL)
	{
		DEVPRINTF("CBotActor::BotActor_SetQuatObj(): Passed in a NULL quatobj.\n");
		return((cell)(0));
	}


	pBA->SetQuatObj(pQO);

	return(0);
}

// =============================================================================================================

cell AMX_NATIVE_CALL CBotActor::BotActor_SetAnimMult(AMX *pAMX, cell *aParams)
{
	CBotActor *pBA = (CBotActor *)(aParams[1]);
	if(pBA == NULL)
	{
		DEVPRINTF("CBotActor::BotActor_SetQuatObj(): Passed in a NULL actor.\n");
		return((cell)(0));
	}

	pBA->m_fAnimMult = ConvertCellToF32(aParams[2]);

	return((cell)(0));
}
