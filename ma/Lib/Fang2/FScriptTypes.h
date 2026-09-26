//////////////////////////////////////////////////////////////////////////////////////
// FScriptTypes.h - Fang-based types to be used from within scripts.
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
// 03/18/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FSCRIPTTYPES_H_
#define _FSCRIPTTYPES_H_ 1

#include "fang.h"
#include "fmath.h"
#include "amx.h"

extern AMX_NATIVE_INFO FScriptTypes_aFangEventNatives[];

// =============================================================================================================

//class CFWorldMesh;

// =============================================================================================================

cell AMX_NATIVE_CALL f32_SetFromInt(AMX *pAMX, cell *aParams);
cell AMX_NATIVE_CALL f32_SetFromString(AMX *pAMX, cell *aParams);
cell AMX_NATIVE_CALL f32_DevPrint(AMX *pAMX, cell *aParams);

// Return the result.
cell AMX_NATIVE_CALL f32_Sum(AMX *pAMX, cell *aParams);
cell AMX_NATIVE_CALL f32_Difference(AMX *pAMX, cell *aParams);
cell AMX_NATIVE_CALL f32_Product(AMX *pAMX, cell *aParams);
cell AMX_NATIVE_CALL f32_Quotient(AMX *pAMX, cell *aParams);
cell AMX_NATIVE_CALL f32_Sqrt(AMX *pAMX, cell *aParams);
cell AMX_NATIVE_CALL f32_Recip(AMX *pAMX, cell *aParams);
cell AMX_NATIVE_CALL f32_RecipSqrt(AMX *pAMX, cell *aParams);
cell AMX_NATIVE_CALL f32_IsGreater(AMX *pAMX, cell *aParams);
cell AMX_NATIVE_CALL f32_IsGreaterEq(AMX *pAMX, cell *aParams);
cell AMX_NATIVE_CALL f32_IsLess(AMX *pAMX, cell *aParams);
cell AMX_NATIVE_CALL f32_IsLessEq(AMX *pAMX, cell *aParams);

cell AMX_NATIVE_CALL s32_RandomRange(AMX *pAMX, cell *aParams);
/*
// Place the result in the 'calling object'.
cell AMX_NATIVE_CALL f32_Add(AMX *pAMX, cell *aParams);
cell AMX_NATIVE_CALL f32_Subtract(AMX *pAMX, cell *aParams);
cell AMX_NATIVE_CALL f32_Multiply(AMX *pAMX, cell *aParams);
cell AMX_NATIVE_CALL f32_Divide(AMX *pAMX, cell *aParams);
*/

// =============================================================================================================

const u32 CFST_CFVec3Wrapper_uPoolSize = 25;

class CFST_CFVec3Wrapper
{
public:
	static BOOL ModuleStartup();
	static void ModuleShutdown();

	static cell AMX_NATIVE_CALL CFVec3_GetFromPool(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFVec3_DevPrint(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFVec3_SetFromF32(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFVec3_SetNull(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFVec3_SetFromString(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFVec3_MulF32(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFVec3_Dot(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFVec3_Cross(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFVec3_Sum(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFVec3_Diff(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFVec3_Mag(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFVec3_Mag2(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFVec3_Unitize(AMX *pAMX, cell *aParams);
	static CFVec3A *GetPool( void );
	static s32 GetPoolNumEntries( void );

private:
	static CFVec3A m_avecPool[CFST_CFVec3Wrapper_uPoolSize];
	static u32 m_uNumAllocated;
};

// =============================================================================================================

class CFST_MeshWrapper
{
public:
	static BOOL ModuleStartup();
	static void ModuleShutdown();

	static cell AMX_NATIVE_CALL MeshRes_Load(AMX *pAMX, cell *aParams);
private:
};

// =============================================================================================================

class CFST_AudioWrapper
{
public:
	static BOOL ModuleStartup();
	static void ModuleShutdown();

	static cell AMX_NATIVE_CALL AudioRes_Load(AMX *pAMX, cell *aParams);
private:
};

// =============================================================================================================

class CFST_AnimWrapper
{
public:
	static BOOL ModuleStartup();
	static void ModuleShutdown();

	static cell AMX_NATIVE_CALL AnimRes_Load(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL AnimInst_Acquire(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL AnimInst_Init(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL AnimInst_Release(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL AnimInst_GetUnitPos(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL AnimInst_Advance(AMX *pAMX, cell *aParams);
private:
};

// =============================================================================================================
/*
const u32 CFST_CFWorldMeshWrapper_uPoolSize = 25;

class CFST_WorldMeshWrapper
{
public:
	static BOOL ModuleStartup();
	static void ModuleShutdown();

	static cell AMX_NATIVE_CALL CFWM_Acquire(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFWM_Release(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFWM_Find(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFWM_IsValid(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFWM_AddToWorld(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFWM_RemoveFromWorld(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFWM_Init(AMX *pAMX, cell *aParams);

private:
	static s32 m_nNextInPool;
	static CFWorldMesh *m_aPool;
};
*/
// =============================================================================================================

class CFST_Vec3ObjWrapper
{
public:
	static BOOL ModuleStartup();
	static void ModuleShutdown();

	static cell AMX_NATIVE_CALL CFV3Constant_Acquire(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFV3Constant_Release(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFV3Constant_Init(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL CFV3Linear_Acquire(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFV3Linear_Release(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFV3Linear_Init(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL CFV3PointPath1_Acquire(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFV3PointPath1_Release(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL CFV3PointPath2_Acquire(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFV3PointPath2_Release(AMX *pAMX, cell *aParams);
//	static cell AMX_NATIVE_CALL CFV3PointPath2_Init1(AMX *pAMX, cell *aParams);			// Found in MAScriptTypes

	static cell AMX_NATIVE_CALL CFVec3Obj_Start(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFVec3Obj_Pause(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFVec3Obj_End(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFVec3Obj_Reset(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFVec3Obj_IsDone(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFVec3Obj_IsWorking(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFVec3Obj_SetTimePos(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFVec3Obj_GetTimePos(AMX *pAMX, cell *aParams);

private:
};

// =============================================================================================================

class CFST_QuatObjWrapper
{
public:
	static BOOL ModuleStartup();
	static void ModuleShutdown();

	static cell AMX_NATIVE_CALL CFQuatObj_Start(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFQuatObj_Pause(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFQuatObj_End(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFQuatObj_Reset(AMX *pAMX, cell *aParams);
//	static cell AMX_NATIVE_CALL CFQuatObj_IsDone(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFQuatObj_SetTimePos(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL CFQuatTwirlY_Acquire(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFQuatTwirlY_Release(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFQuatTwirlY_Init(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL CFQuatTang1_Acquire(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFQuatTang1_Release(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFQuatTang1_Init(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL CFQuatTang2_Acquire(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFQuatTang2_Release(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFQuatTang2_Init(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL CFQuatTang3_Acquire(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFQuatTang3_Release(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFQuatTang3_Init(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL CFQuatComp_Acquire(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFQuatComp_Release(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFQuatComp_Init(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL CFQuatLookAt1_Acquire(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFQuatLookAt1_Release(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFQuatLookAt1_Init(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL CFQuatSLERP_Acquire(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CFQuatSLERP_Release(AMX *pAMX, cell *aParams);
//	static cell AMX_NATIVE_CALL CFQuatSLERP_Init(AMX *pAMX, cell *aParams);				// Found in MAScriptTypes

private:
};

// =============================================================================================================

class CFST_ScalarObjWrapper
{
public:
	static BOOL ModuleStartup();
	static void ModuleShutdown();

//	static cell AMX_NATIVE_CALL Scalar_GetFromPool(AMX *pAMX, cell *aParams);
private:
};

// =============================================================================================================

#endif
