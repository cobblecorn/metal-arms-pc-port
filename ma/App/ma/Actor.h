//////////////////////////////////////////////////////////////////////////////////////
// Actor.h - Script Actor classes for Mettle Arms.
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
#ifndef _ACTOR_H_
#define _ACTOR_H_ 1

#include "fang.h"
#include "amx.h"
#include "Workable.h"


// =============================================================================================================


const u32 CActor_uWorkListMaxSize = 35;

class CFVec3AObj;
class CFQuatObj;
class CFAnimInst;

// This abstract base class defines the interface for a set of actors that
//   maintain position and orientation information and control a game entity.
// These are primarily intended for use in scripts.
class CActor : public CWorkable
{
public:
	CActor();
	void _ClearCActorData();
	void SetPosObj(CFVec3AObj *pV3Obj) { m_pPosObj = pV3Obj; }
	void SetQuatObj(CFQuatObj *pQuatObj) { m_pQuatObj = pQuatObj;}

	static BOOL InitSystem();
	static void UninitSystem();
	static BOOL LevelInit();
	static void LevelUninit();
	static void Reset();

protected:
	CFVec3AObj *m_pPosObj;
	CFQuatObj *m_pQuatObj;
	BOOL m_bIsInitted;

private:
	// These get registered in the script system by InitSystem().
	static AMX_NATIVE_INFO m_aNatives[];

	static BOOL m_bSystemInitted;
};


// =============================================================================================================


class CMeshEntity;

const u32 CMeshActor_uPoolSize = 50;

FCLASS_ALIGN_PREFIX class CMeshActor : public CActor
{
public:
	CMeshActor(void);
	void _ClearCMeshActorData(void);

	void Init(CMeshEntity *pME, CFVec3AObj *pV3Obj, CFQuatObj *pQuatObj);
	void Work();
	void AddToWorld();
	void RemoveFromWorld();

	// Gets and returns objects from pool.
	static CMeshActor *Acquire();
	void Release();

	static BOOL InitSystem();
	static void UninitSystem();
	static BOOL LevelInit();
	static void LevelUninit();
	static void Reset();

private:
	CMeshEntity *m_pME;

	// This clears the pool.
	static void ClearPool();

	static cell AMX_NATIVE_CALL MeshActor_Acquire(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL MeshActor_Release(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL MeshActor_IsValid(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL MeshActor_Init(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL MeshActor_Activate(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL MeshActor_Deactivate(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL MeshActor_SetPosObj(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL MeshActor_SetQuatObj(AMX *pAMX, cell *aParams);

	// These get registered in the script system by InitSystem().
	static AMX_NATIVE_INFO m_aNatives[];

	static CMeshActor *m_aPool;
	static BOOL8 *m_abInUse;		// Derived classes must allocate this to record availability in their pool.

	FCLASS_STACKMEM_ALIGN(CMeshActor);
} FCLASS_ALIGN_SUFFIX;


// =============================================================================================================


class CBot;

const u32 CBotActor_uPoolSize = 20;

FCLASS_ALIGN_PREFIX class CBotActor : public CActor
{
public:
	CBotActor(void);
	void _ClearCBotActorData(void);

	void Init1(CBot *pBot, CFVec3AObj *pV3Obj, CFQuatObj *pQuatObj);
	void Init2(CBot *pBot, CFAnimInst *pAnimInst, s32 nUserAnimSlot);
	void Work();
	void AddToWorld();
	void RemoveFromWorld();

	void Activate();
	void Deactivate();

	// Gets and returns objects from pool.
	static CBotActor *Acquire();
	void Release();

	static BOOL InitSystem();
	static void UninitSystem();
	static BOOL LevelInit();
	static void LevelUninit();
	static void Reset();

private:
	CBot *m_pBot;
	CFAnimInst *m_pAnimInst;
	s32 m_nUserAnimSlot;
	f32 m_fAnimMult;

	// This clears the pool.
	static void ClearPool();

	static cell AMX_NATIVE_CALL BotActor_Acquire(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL BotActor_Release(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL BotActor_IsValid(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL BotActor_Init1(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL BotActor_Init2(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL BotActor_Activate(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL BotActor_Deactivate(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL BotActor_SetPosObj(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL BotActor_SetQuatObj(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL BotActor_SetAnimMult(AMX *pAMX, cell *aParams);

	// These get registered in the script system by InitSystem().
	static AMX_NATIVE_INFO m_aNatives[];

	static CBotActor *m_aPool;
	static BOOL8 *m_abInUse;		// Derived classes must allocate this to record availability in their pool.

	FCLASS_STACKMEM_ALIGN(CBotActor);
} FCLASS_ALIGN_SUFFIX;


// =============================================================================================================

// Moved to CamActor.cpp
/*
const u32 CCamActor_uPoolSize = 10;

FCLASS_ALIGN_PREFIX class CCamActor : public CActor
{
public:
	void Init(CFVec3Obj *pV3Obj, CFQuatObj *pQuatObj);
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
//	CCamMan *m_pCamMan;

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
*/
#endif
