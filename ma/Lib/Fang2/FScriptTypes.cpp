//////////////////////////////////////////////////////////////////////////////////////
// FScriptTypes.cpp - Fang-based types to be used from within scripts.
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

#include "fang.h"
#include "fmesh.h"
#include "fresload.h"
#include "fworld.h"
#include "fmath.h"
#include "fsndfx.h"
#include "fanim.h"
#include "fscriptsystem.h"
#include "FNativeUtil.h"

#include "FV3OConst.h"
#include "FV3OLine1.h"
#include "FPointPath1.h"
#include "FPointPath2.h"

#include "FQuatObj.h"
#include "FQOTang1.h"
#include "FQuatTang2.h"
#include "FQuatTang3.h"
#include "FQuatComp.h"
#include "FQuatLookAt1.h"
#include "FQuatSLERP.h"

#include "FScriptTypes.h"
#include <stdlib.h>			// for atof()

AMX_NATIVE_INFO FScriptTypes_aFangEventNatives[] =
{
	// f32 functions...
	"f32_SetFromInt",			f32_SetFromInt,
	"f32_DevPrint",				f32_DevPrint,
	"f32_SetFromString",		f32_SetFromString,
	"f32_Sum",					f32_Sum,
	"f32_Difference",			f32_Difference,
	"f32_Product",				f32_Product,
	"f32_Quotient",				f32_Quotient,
	"f32_Sqrt",					f32_Sqrt,
	"f32_Recip",				f32_Recip,
	"f32_RecipSqrt",			f32_RecipSqrt,
	"f32_IsGreater",			f32_IsGreater,
	"f32_IsGreaterEq",			f32_IsGreaterEq,
	"f32_IsLess",				f32_IsLess,
	"f32_IsLessEq",				f32_IsLessEq,

	"s32_RandomRange",			s32_RandomRange,
	// CFVec3 functions...
	"FVec3_GetFromPool",		CFST_CFVec3Wrapper::CFVec3_GetFromPool,
	"FVec3_SetNull",			CFST_CFVec3Wrapper::CFVec3_SetNull,
	"FVec3_DevPrint",			CFST_CFVec3Wrapper::CFVec3_DevPrint,
	"FVec3_SetFromF32",			CFST_CFVec3Wrapper::CFVec3_SetFromF32,
	"FVec3_SetFromString",		CFST_CFVec3Wrapper::CFVec3_SetFromString,
	"FVec3_MulF32",				CFST_CFVec3Wrapper::CFVec3_MulF32,
	"FVec3_Dot",				CFST_CFVec3Wrapper::CFVec3_Dot,
	"FVec3_Cross",				CFST_CFVec3Wrapper::CFVec3_Cross,
	"FVec3_Sum",				CFST_CFVec3Wrapper::CFVec3_Sum,
	"FVec3_Diff",				CFST_CFVec3Wrapper::CFVec3_Diff,
	"FVec3_Mag",				CFST_CFVec3Wrapper::CFVec3_Mag,
	"FVec3_Mag2",				CFST_CFVec3Wrapper::CFVec3_Mag2,
	"FVec3_Unitize",			CFST_CFVec3Wrapper::CFVec3_Unitize,

	// MeshRes functions...
	"MeshRes_Load",				CFST_MeshWrapper::MeshRes_Load,

	// AudioRes functions...
	"AudioRes_Load",			CFST_AudioWrapper::AudioRes_Load,

	// AnimRes functions...
	"AnimRes_Load",				CFST_AnimWrapper::AnimRes_Load,
	"AnimInst_Acquire",			CFST_AnimWrapper::AnimInst_Acquire,
	"AnimInst_Init",			CFST_AnimWrapper::AnimInst_Init,
	"AnimInst_Release",			CFST_AnimWrapper::AnimInst_Release,
	"AnimInst_GetUnitPos",		CFST_AnimWrapper::AnimInst_GetUnitPos,
/*
	// CFWorldMesh functions...
	"FWM_Acquire",				CFST_WorldMeshWrapper::CFWM_Acquire,
	"FWM_Release",				CFST_WorldMeshWrapper::CFWM_Release,
	"FWM_Find",					CFST_WorldMeshWrapper::CFWM_Find,
	"FWM_IsValid",				CFST_WorldMeshWrapper::CFWM_IsValid,
	"FWM_Init",					CFST_WorldMeshWrapper::CFWM_Init,
	"FWM_AddToWorld",			CFST_WorldMeshWrapper::CFWM_AddToWorld,
	"FWM_RemoveFromWorld",		CFST_WorldMeshWrapper::CFWM_RemoveFromWorld,
*/
	// CFVec3Obj functions...
	"FVec3Obj_Start",			CFST_Vec3ObjWrapper::CFVec3Obj_Start,
	"FVec3Obj_Pause",			CFST_Vec3ObjWrapper::CFVec3Obj_Pause,
	"FVec3Obj_End",				CFST_Vec3ObjWrapper::CFVec3Obj_End,
	"FVec3Obj_Reset",			CFST_Vec3ObjWrapper::CFVec3Obj_Reset,
	"FVec3Obj_IsDone",			CFST_Vec3ObjWrapper::CFVec3Obj_IsDone,
	"FVec3Obj_IsWorking",		CFST_Vec3ObjWrapper::CFVec3Obj_IsWorking,
	"FVec3Obj_SetTimePos",		CFST_Vec3ObjWrapper::CFVec3Obj_SetTimePos,
	"FVec3Obj_GetTimePos",		CFST_Vec3ObjWrapper::CFVec3Obj_GetTimePos,

	// CFVec3Constant functions...
	"FV3Constant_Acquire",		CFST_Vec3ObjWrapper::CFV3Constant_Acquire,
	"FV3Constant_Release",		CFST_Vec3ObjWrapper::CFV3Constant_Release,
	"FV3Constant_Init",			CFST_Vec3ObjWrapper::CFV3Constant_Init,

	// CFVec3Linear functions...
	"FV3Linear_Acquire",		CFST_Vec3ObjWrapper::CFV3Linear_Acquire,
	"FV3Linear_Release",		CFST_Vec3ObjWrapper::CFV3Linear_Release,
	"FV3Linear_Init",			CFST_Vec3ObjWrapper::CFV3Linear_Init,

	// CFVec3PointPath1 functions...
	"FV3Points1_Acquire",		CFST_Vec3ObjWrapper::CFV3PointPath1_Acquire,
	"FV3Points1_Release",		CFST_Vec3ObjWrapper::CFV3PointPath1_Release,

	// CFVec3PointPath2 functions...
	"FV3Points2_Acquire",		CFST_Vec3ObjWrapper::CFV3PointPath2_Acquire,
	"FV3Points2_Release",		CFST_Vec3ObjWrapper::CFV3PointPath2_Release,
//	"FV3Points2_Init1",			CFST_Vec3ObjWrapper::CFV3PointPath2_Init1,

	// CFQuatObj functions...
	"FQuatObj_Start",			CFST_QuatObjWrapper::CFQuatObj_Start,
	"FQuatObj_Pause",			CFST_QuatObjWrapper::CFQuatObj_Pause,
	"FQuatObj_End",				CFST_QuatObjWrapper::CFQuatObj_End,
	"FQuatObj_Reset",			CFST_QuatObjWrapper::CFQuatObj_Reset,

	// CFQuatTwirlY functions...
	"FQuatTwirlY_Acquire",		CFST_QuatObjWrapper::CFQuatTwirlY_Acquire,
	"FQuatTwirlY_Release",		CFST_QuatObjWrapper::CFQuatTwirlY_Release,
	"FQuatTwirlY_Init",			CFST_QuatObjWrapper::CFQuatTwirlY_Init,

	// CFQuatTang1 functions...
	"FQuatTang1_Acquire",		CFST_QuatObjWrapper::CFQuatTang1_Acquire,
	"FQuatTang1_Release",		CFST_QuatObjWrapper::CFQuatTang1_Release,
	"FQuatTang1_Init",			CFST_QuatObjWrapper::CFQuatTang1_Init,

	// CFQuatTang2 functions...
	"FQuatTang2_Acquire",		CFST_QuatObjWrapper::CFQuatTang2_Acquire,
	"FQuatTang2_Release",		CFST_QuatObjWrapper::CFQuatTang2_Release,
	"FQuatTang2_Init",			CFST_QuatObjWrapper::CFQuatTang2_Init,

	// CFQuatTang3 functions...
	"FQuatTang3_Acquire",		CFST_QuatObjWrapper::CFQuatTang3_Acquire,
	"FQuatTang3_Release",		CFST_QuatObjWrapper::CFQuatTang3_Release,
	"FQuatTang3_Init",			CFST_QuatObjWrapper::CFQuatTang3_Init,

	// CFQuatComp functions...
	"FQuatComp_Acquire",		CFST_QuatObjWrapper::CFQuatComp_Acquire,
	"FQuatComp_Release",		CFST_QuatObjWrapper::CFQuatComp_Release,
	"FQuatComp_Init",			CFST_QuatObjWrapper::CFQuatComp_Init,

	// CFQuatLookAt1 functions...
	"FQuatLook1_Acquire",		CFST_QuatObjWrapper::CFQuatLookAt1_Acquire,
	"FQuatLook1_Release",		CFST_QuatObjWrapper::CFQuatLookAt1_Release,
	"FQuatLook1_Init",			CFST_QuatObjWrapper::CFQuatLookAt1_Init,

	// CFQuatSLERP functions...
	"FQuatSLERP_Acquire",		CFST_QuatObjWrapper::CFQuatSLERP_Acquire,
	"FQuatSLERP_Release",		CFST_QuatObjWrapper::CFQuatSLERP_Release,


	// End of list marker.
	NULL,						NULL
};
/*
//// Utility functions.
//
static cell ConvertF32ToCell(f32 fValue)
{
	cell oTemp;
	f32 *pf32 = (f32 *)(&oTemp);
	*pf32 = fValue;

	return(oTemp);
}

static f32 ConvertCellToF32(cell oCell)
{
	f32 fTemp;
	f32 *pf32 = (f32 *)(&oCell);
	fTemp = *pf32;

	return(fTemp);
}

// Do not use this function, it is evil, pure evil I tell you!!!!
// Use the function below.
static char *CreateAndFillStringFromCell(AMX *amx,cell params)
{
	FASSERT_NOW;
    char *pszDest;
    s32 nLen;
    cell *pString;

    // Get the real address of the string.
    amx_GetAddr(amx, params, &pString);

    // Find out how long the string is in characters.
    amx_StrLen(pString, (int *)&nLen);
//    pszDest = fnew char[nLen+1];
	pszDest = (char *)(fres_Alloc(nLen + 1));

    // Now convert the Small String into a C type null terminated string
    amx_GetString(pszDest, pString);

    return(pszDest);
}

// Use this function.
// It does the same as the above did, but it doesn't allocate space.
// uMaxStringSize is 1 less than the size of the buffer.
static void FillStringFromCell(char *pszDestString, u32 uMaxStringSize, AMX *pAMX, cell oParam)
{
	u32 uLen;
	cell *pString;

    // Get the real address of the string.
    amx_GetAddr(pAMX, oParam, &pString);

    // Find out how long the string is in characters.
    amx_StrLen(pString, (int *)(&uLen));
	FASSERT(uMaxStringSize >= uLen);

    // Now convert the Small String into a C type null terminated string
    amx_GetString(pszDestString, pString);
}

//
////
*/
// =============================================================================================================

//// f32 functions.
//
cell AMX_NATIVE_CALL f32_SetFromInt(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "f32_SetFromInt", 1 );
	f32 fValue = (f32)(aParams[1]);
	return(ConvertF32ToCell(fValue));
}

cell AMX_NATIVE_CALL f32_SetFromString(AMX *pAMX, cell *aParams)
{
	f32 fValue;
	char szFloat[50];

	SCRIPT_CHECK_NUM_PARAMS( "f32_SetFromString", 1 );

	FillStringFromCell(szFloat, 49, pAMX, aParams[1]);

	fValue = (f32)(atof(szFloat));

	return(ConvertF32ToCell(fValue));
}

cell AMX_NATIVE_CALL f32_Sum(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "f32_Sum", 2 );

	f32 fT1 = ConvertCellToF32(aParams[1]);
	f32 fT2 = ConvertCellToF32(aParams[2]);

	return(ConvertF32ToCell(fT1 + fT2));
}

cell AMX_NATIVE_CALL f32_Difference(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "f32_Difference", 2 );
	f32 fT1 = ConvertCellToF32(aParams[1]);
	f32 fT2 = ConvertCellToF32(aParams[2]);

	return(ConvertF32ToCell(fT1 - fT2));
}

cell AMX_NATIVE_CALL f32_Product(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "f32_Product", 2 );
	f32 fT1 = ConvertCellToF32(aParams[1]);
	f32 fT2 = ConvertCellToF32(aParams[2]);

	return(ConvertF32ToCell(fT1 * fT2));
}

cell AMX_NATIVE_CALL f32_Quotient(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "f32_Quotient", 2 );
	f32 fT1 = ConvertCellToF32(aParams[1]);
	f32 fT2 = ConvertCellToF32(aParams[2]);

	return(ConvertF32ToCell(fmath_Div(fT1, fT2)));
}

cell AMX_NATIVE_CALL f32_Sqrt(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "f32_Sqrt", 1 );

	f32 fT = ConvertCellToF32(aParams[1]);

	return(ConvertF32ToCell(fmath_Sqrt(fT)));
}

cell AMX_NATIVE_CALL f32_Recip(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "f32_Recip", 1 );

	f32 fT = ConvertCellToF32(aParams[1]);

	return(ConvertF32ToCell(fmath_Inv(fT)));
}

cell AMX_NATIVE_CALL f32_RecipSqrt(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "f32_Recip_Sqrt", 1 );

	f32 fT = ConvertCellToF32(aParams[1]);

	return(ConvertF32ToCell(fmath_InvSqrt(fT)));
}

cell AMX_NATIVE_CALL f32_DevPrint(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "f32_DevPrint", 1 );

	DEVPRINTF("%f\n", ConvertCellToF32(aParams[1]));
	return(0);
}

cell AMX_NATIVE_CALL f32_IsGreater(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "f32_IsGreater", 2 );

	f32 fT1 = ConvertCellToF32(aParams[1]);
	f32 fT2 = ConvertCellToF32(aParams[2]);

	return((cell)(fT1 > fT2));
}

cell AMX_NATIVE_CALL f32_IsGreaterEq(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "f32_IsGreaterEq", 2 );

	f32 fT1 = ConvertCellToF32(aParams[1]);
	f32 fT2 = ConvertCellToF32(aParams[2]);

	return((cell)(fT1 >= fT2));
}

cell AMX_NATIVE_CALL f32_IsLess(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "f32_IsLess", 2 );

	f32 fT1 = ConvertCellToF32(aParams[1]);
	f32 fT2 = ConvertCellToF32(aParams[2]);

	return((cell)(fT1 < fT2));
}

cell AMX_NATIVE_CALL f32_IsLessEq(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "f32_IsLessEq", 2 );
	f32 fT1 = ConvertCellToF32(aParams[1]);
	f32 fT2 = ConvertCellToF32(aParams[2]);

	return((cell)(fT1 <= fT2));
}
//
////

cell AMX_NATIVE_CALL s32_RandomRange(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "s32_RandomRange", 2 );
	return fmath_RandomRange( aParams[1], aParams[2] );
}

// =============================================================================================================

CFVec3A CFST_CFVec3Wrapper::m_avecPool[CFST_CFVec3Wrapper_uPoolSize];
u32 CFST_CFVec3Wrapper::m_uNumAllocated;

//// CFVec3 functions.
//
BOOL CFST_CFVec3Wrapper::ModuleStartup()
{
	m_uNumAllocated = 0;

	return(TRUE);
}

void CFST_CFVec3Wrapper::ModuleShutdown()
{
	m_uNumAllocated = 0;
}

cell AMX_NATIVE_CALL CFST_CFVec3Wrapper::CFVec3_GetFromPool(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FVec3_GetFromPool", 0 );

	if(m_uNumAllocated == CFST_CFVec3Wrapper_uPoolSize)
	{
		SCRIPT_ERROR("CFST_CFVec3Wrapper::CFVec3_GetFromPool() : Operation failed, pool completely allocated.\n");
		return((cell)(NULL));
	}

	cell oCell = (cell)(&(m_avecPool[m_uNumAllocated]));
	++m_uNumAllocated;
	return(oCell);
}

cell AMX_NATIVE_CALL CFST_CFVec3Wrapper::CFVec3_DevPrint(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FVec3_DevPrint", 1 );

	CFVec3A *pVec = (CFVec3A *)(aParams[1]);
	DEVPRINTF("<%f, %f, %f>\n", pVec->x, pVec->y, pVec->z);

	return(0);
}

cell AMX_NATIVE_CALL CFST_CFVec3Wrapper::CFVec3_SetFromF32(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FVec3_SetFromF32", 4 );

	CFVec3A *pVec = (CFVec3A *)(aParams[1]);
	pVec->x = ConvertCellToF32(aParams[2]);
	pVec->y = ConvertCellToF32(aParams[3]);
	pVec->z = ConvertCellToF32(aParams[4]);

	return(0);
}

cell AMX_NATIVE_CALL CFST_CFVec3Wrapper::CFVec3_SetFromString(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FVec3_SetFromString", 4 );

	CFVec3A *pvecIn = (CFVec3A *)(aParams[1]);

	char szTemp[50];

	// bug? should all three of these reference aParams[2]??
	FillStringFromCell(szTemp, 49, pAMX, aParams[2]);
	pvecIn->x = (f32)(atof(szTemp));

	FillStringFromCell(szTemp, 49, pAMX, aParams[2]);
	pvecIn->y = (f32)(atof(szTemp));

	FillStringFromCell(szTemp, 49, pAMX, aParams[2]);
	pvecIn->z = (f32)(atof(szTemp));

	return(0);
}

cell AMX_NATIVE_CALL CFST_CFVec3Wrapper::CFVec3_SetNull(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FVec3_SetNull", 1 );

	CFVec3A *pVec = (CFVec3A *)(aParams[1]);
	pVec->x = 0.0f;
	pVec->y = 0.0f;
	pVec->z = 0.0f;

	return(0);
}

cell AMX_NATIVE_CALL CFST_CFVec3Wrapper::CFVec3_MulF32(AMX *pAMX, cell *aParams)
{
	return(0);
}

cell AMX_NATIVE_CALL CFST_CFVec3Wrapper::CFVec3_Dot(AMX *pAMX, cell *aParams)
{
	return(0);
}

cell AMX_NATIVE_CALL CFST_CFVec3Wrapper::CFVec3_Cross(AMX *pAMX, cell *aParams)
{
	return(0);
}

cell AMX_NATIVE_CALL CFST_CFVec3Wrapper::CFVec3_Sum(AMX *pAMX, cell *aParams)
{
	return(0);
}

cell AMX_NATIVE_CALL CFST_CFVec3Wrapper::CFVec3_Diff(AMX *pAMX, cell *aParams)
{
	return(0);
}

cell AMX_NATIVE_CALL CFST_CFVec3Wrapper::CFVec3_Mag(AMX *pAMX, cell *aParams)
{
	return(0);
}

cell AMX_NATIVE_CALL CFST_CFVec3Wrapper::CFVec3_Mag2(AMX *pAMX, cell *aParams)
{
	return(0);
}

cell AMX_NATIVE_CALL CFST_CFVec3Wrapper::CFVec3_Unitize(AMX *pAMX, cell *aParams)
{
	return(0);
}

CFVec3A *CFST_CFVec3Wrapper::GetPool( void )
{
	return &m_avecPool[0];
}

s32 CFST_CFVec3Wrapper::GetPoolNumEntries( void )
{
	return CFST_CFVec3Wrapper_uPoolSize;
}


//
////

// =============================================================================================================

BOOL CFST_MeshWrapper::ModuleStartup()
{
	return(TRUE);
}

void CFST_MeshWrapper::ModuleShutdown()
{
}

//// FMesh_t functions.
//

cell AMX_NATIVE_CALL CFST_MeshWrapper::MeshRes_Load(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "MeshRes_Load", 1 );

	// Get the name of the mesh that they passed in.
	char szMeshName[12];
	FillStringFromCell(szMeshName, 11, pAMX, aParams[1]);

	// Load the resource.
	FMesh_t *pMesh = (FMesh_t *)(fresload_Load(FMESH_RESTYPE, szMeshName));
	if(pMesh == NULL)
	{
		SCRIPT_ERROR("CFST_MeshWrapper::MeshRes_Load() : Could not load mesh '%s'.\n", szMeshName);
		return(NULL);
	}

	return((cell)(pMesh));
}

//
////

// =============================================================================================================

BOOL CFST_AudioWrapper::ModuleStartup()
{
	return(TRUE);
}

void CFST_AudioWrapper::ModuleShutdown()
{
}

//// FMesh_t functions.
//

cell AMX_NATIVE_CALL CFST_AudioWrapper::AudioRes_Load(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "AudioRes_Load", 1 );

	// Get the name of the bank that they passed in.
	char szName[40];
	FillStringFromCell(szName, 39, pAMX, aParams[1]);

	FSndFx_FxHandle_t hSound = fsndfx_GetFxHandle(szName);

	return((cell)(hSound));
}

//
////

// =============================================================================================================

BOOL CFST_AnimWrapper::ModuleStartup()
{
	return(TRUE);
}

void CFST_AnimWrapper::ModuleShutdown()
{
}

//// FAnim_t/CFAnimInst functions.
//

cell AMX_NATIVE_CALL CFST_AnimWrapper::AnimRes_Load(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "AnimRes_Load", 1 );

	// Get the animation name that they passed in.
	char szName[40];
	FillStringFromCell(szName, 39, pAMX, aParams[1]);

	// Load the resource.
	FAnim_t *pAnim = (FAnim_t *)(fresload_Load(FANIM_RESNAME, szName));
	if(pAnim == NULL)
	{
		SCRIPT_ERROR("CFST_AnimWrapper::AnimRes_Load() : Could not load animation '%s'.\n", szName);
		return((cell)(NULL));
	}

	return((cell)(pAnim));
}


cell AMX_NATIVE_CALL CFST_AnimWrapper::AnimInst_Acquire(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "AnimInst_Acquire", 0 );

	CFAnimInst *pAnimInst = fnew CFAnimInst;
	if(pAnimInst == NULL)
	{
		SCRIPT_ERROR("CFST_AnimWrapper::AnimInst_Acquire() : Could not create AnimInst, out of memory.\n");
		return((cell)(NULL));
	}

	return((cell)(pAnimInst));
}

cell AMX_NATIVE_CALL CFST_AnimWrapper::AnimInst_Init(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "AnimInst_Init", 2 );

	CFAnimInst *pAnimInst = (CFAnimInst *)(aParams[1]);
	if(pAnimInst == NULL)
	{
		SCRIPT_ERROR("CFST_AnimWrapper::AnimInst_Init() : Cannot init null AnimInst.\n");
		return((cell)(0));
	}

	FAnim_t *pAnim = (FAnim_t *)(aParams[2]);
	if(pAnim == NULL)
	{
		SCRIPT_ERROR("CFST_AnimWrapper::AnimInst_Init() : Cannot init AnimInst with NULL AnimRes.\n");
		return((cell)(0));
	}

	if(!pAnimInst->Create(pAnim))
	{
		SCRIPT_ERROR("CFST_AnimWrapper::AnimInst_Init() : Failed on init of AnimInst.\n");
		return((cell)(0));
	}

	return((cell)(0));
}

cell AMX_NATIVE_CALL CFST_AnimWrapper::AnimInst_Release(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "AnimInst_Release", 1 );

	CFAnimInst *pAnimInst = (CFAnimInst *)(aParams[1]);
	if(pAnimInst == NULL)
	{
		SCRIPT_ERROR("CFST_AnimWrapper::AnimInst_Release() : Tried to release null CFAnimInst.\n");
		return((cell)(0));
	}

	fdelete(pAnimInst);
	return((cell)(0));
}


cell AMX_NATIVE_CALL CFST_AnimWrapper::AnimInst_GetUnitPos(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "AnimInst_GetUnitPos", 1 );

	CFAnimInst *pAnimInst = (CFAnimInst *)(aParams[1]);
	if(pAnimInst == NULL)
	{
		SCRIPT_ERROR("CFST_AnimWrapper::AnimInst_Release() : Passed in a null CFAnimInst.\n");
		return((cell)(0));
	}

	return(ConvertF32ToCell(pAnimInst->GetUnitTime()));
}

//
////

// =============================================================================================================
/*
//// CFWorldMesh functions.
//

CFWorldMesh *CFST_WorldMeshWrapper::m_aPool = NULL;
s32 CFST_WorldMeshWrapper::m_nNextInPool = -1;

BOOL CFST_WorldMeshWrapper::ModuleStartup()
{
	m_aPool = fnew CFWorldMesh[CFST_CFWorldMeshWrapper_uPoolSize];
	if(m_aPool == NULL)
	{
		SCRIPT_ERROR("CFST_WorldMeshWrapper::ModuleStartup() : Could not allocate pool of CFWMs.\n");
		m_nNextInPool = -1;
		return(TRUE);
	}
	m_nNextInPool = 0;	

	return(TRUE);
}

void CFST_WorldMeshWrapper::ModuleShutdown()
{
	fdelete_array(m_aPool);
}

cell AMX_NATIVE_CALL CFST_WorldMeshWrapper::CFWM_Acquire(AMX *pAMX, cell *aParams)
{
	// REPAIR: This should be getting taken from a pool.
//	CFWorldMesh *pWM = fnew CFWorldMesh;
	if((m_nNextInPool == -1) || (m_nNextInPool == CFST_CFWorldMeshWrapper_uPoolSize))
	{
		SCRIPT_ERROR("CFST_WorldMeshWrapper::CFWM_Acquire : Could not get world mesh.\n");
		return(NULL);
	}

	CFWorldMesh *pWM = &(m_aPool[m_nNextInPool]);
	++m_nNextInPool;

	return((cell)(pWM));
}

cell AMX_NATIVE_CALL CFST_WorldMeshWrapper::CFWM_Release(AMX *pAMX, cell *aParams)
{
	// REPAIR: This should be getting returned to the pool.
	return(0);
}

cell AMX_NATIVE_CALL CFST_WorldMeshWrapper::CFWM_Find(AMX *pAMX, cell *aParams)
{
	// REPAIR: This should be using the world system to find a specified mesh.
	return((cell)(NULL));
}

cell AMX_NATIVE_CALL CFST_WorldMeshWrapper::CFWM_IsValid(AMX *pAMX, cell *aParams)
{
	// This simply checks to see if a CFWorldMesh pointer is NULL.
	return((CFWorldMesh *)(aParams[1]) != NULL);
}

cell AMX_NATIVE_CALL CFST_WorldMeshWrapper::CFWM_AddToWorld(AMX *pAMX, cell *aParams)
{
	CFWorldMesh *pWM = (CFWorldMesh *)(aParams[1]);
	FASSERT(pWM != NULL);
	pWM->AddToWorld();

	return(0);
}

cell AMX_NATIVE_CALL CFST_WorldMeshWrapper::CFWM_RemoveFromWorld(AMX *pAMX, cell *aParams)
{
	CFWorldMesh *pWM = (CFWorldMesh *)(aParams[1]);
	FASSERT(pWM != NULL);
	pWM->RemoveFromWorld();

	return(0);
}

cell AMX_NATIVE_CALL CFST_WorldMeshWrapper::CFWM_Init(AMX *pAMX, cell *aParams)
{
	FMeshInit_t oMeshInit;

	oMeshInit.fCullDist2 = 0.0f;
	oMeshInit.Mtx.Identity();
	oMeshInit.nFlags = 0;
	oMeshInit.pMesh = (FMesh_t *)(aParams[2]);

	CFWorldMesh *pWM = (CFWorldMesh *)(aParams[1]);
	FASSERT(pWM != NULL);
	pWM->Init(&oMeshInit);

	return(0);
}

//
////
*/
// =============================================================================================================

BOOL CFST_Vec3ObjWrapper::ModuleStartup()
{
	return(TRUE);
}

void CFST_Vec3ObjWrapper::ModuleShutdown()
{
}

//// CFVec3Obj functions.
//

cell AMX_NATIVE_CALL CFST_Vec3ObjWrapper::CFVec3Obj_Start(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FVec3Obj_Start", 1 );

	CFVec3AObj *pVO = (CFVec3AObj *)(aParams[1]);

	if(pVO == NULL)
	{
		return((cell)(0));
	}

	if(!pVO->IsInitted())
	{
		return((cell)(0));
	}

	pVO->Start();

	return(0);
}

cell AMX_NATIVE_CALL CFST_Vec3ObjWrapper::CFVec3Obj_Pause(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FVec3Obj_Pause", 1 );

	CFVec3AObj *pVO = (CFVec3AObj *)(aParams[1]);

	if(pVO == NULL)
	{
		return((cell)(0));
	}

	if(!pVO->IsInitted())
	{
		return((cell)(0));
	}

	pVO->Pause();

	return(0);
}

cell AMX_NATIVE_CALL CFST_Vec3ObjWrapper::CFVec3Obj_End(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FVec3Obj_End", 1 );

	CFVec3AObj *pVO = (CFVec3AObj *)(aParams[1]);

	if(pVO == NULL)
	{
		return((cell)(0));
	}

	if(!pVO->IsInitted())
	{
		return((cell)(0));
	}

	pVO->End();

	return(0);
}

cell AMX_NATIVE_CALL CFST_Vec3ObjWrapper::CFVec3Obj_Reset(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FVec3Obj_Reset", 1 );

	CFVec3AObj *pVO = (CFVec3AObj *)(aParams[1]);

	if(pVO == NULL)
	{
		return((cell)(0));
	}

	if(!pVO->IsInitted())
	{
		return((cell)(0));
	}

	pVO->Reset();

	return(0);
}

cell AMX_NATIVE_CALL CFST_Vec3ObjWrapper::CFVec3Obj_IsDone(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FVec3Obj_IsDone", 1 );

	CFVec3AObj *pVO = (CFVec3AObj *)(aParams[1]);

	if(pVO == NULL)
	{
		return((cell)(0));
	}

	if(!pVO->IsInitted())
	{
		return((cell)(0));
	}

	return((cell)(pVO->IsDone()));
}

cell AMX_NATIVE_CALL CFST_Vec3ObjWrapper::CFVec3Obj_IsWorking(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FVec3Obj_IsWorking", 1 );

	CFVec3AObj *pVO = (CFVec3AObj *)(aParams[1]);

	if(pVO == NULL)
	{
		return((cell)(0));
	}

	if(!pVO->IsInitted())
	{
		return((cell)(0));
	}

	return((cell)(pVO->IsWorking()));
}

cell AMX_NATIVE_CALL CFST_Vec3ObjWrapper::CFVec3Obj_SetTimePos(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FVec3Obj_SetTimePos", 2 );

	CFVec3AObj *pVO = (CFVec3AObj *)(aParams[1]);

	if(pVO == NULL)
	{
		return((cell)(0));
	}

	if(!pVO->IsInitted())
	{
		return((cell)(0));
	}

	pVO->SetTimePos(ConvertCellToF32(aParams[2]));

	return(0);
}

cell AMX_NATIVE_CALL CFST_Vec3ObjWrapper::CFVec3Obj_GetTimePos(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FVec3Obj_GetTimePos", 1 );

	CFVec3AObj *pVO = (CFVec3AObj *)(aParams[1]);

	if(pVO == NULL)
	{
		return((cell)(0));
	}

	if(!pVO->IsInitted())
	{
		return((cell)(0));
	}

	return(ConvertF32ToCell(pVO->GetTimePos()));
}

cell AMX_NATIVE_CALL CFST_Vec3ObjWrapper::CFV3Constant_Acquire(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FV3Constant_Acquire", 0 );
	// REPAIR: This should get changed when the below function gets removed.
	CFV3OConst *pTemp = CFV3OConst::GetAvailable();

	return((cell)(pTemp));
}

cell AMX_NATIVE_CALL CFST_Vec3ObjWrapper::CFV3Constant_Release(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FV3Constant_Release", 0 );
	// REPAIR: This should get changed when a release function gets added to CFVec3Objs.
	return(0);
}

cell AMX_NATIVE_CALL CFST_Vec3ObjWrapper::CFV3Constant_Init(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FV3Constant_Init", 0 );
	// JUSTIN : Verify the incoming parameters.

	CFV3OConst *pVO = (CFV3OConst *)(aParams[1]);
	FASSERT(pVO != NULL);

	CFVec3A *pVec1 = (CFVec3A *)(aParams[2]);
	FASSERT(pVec1 != NULL);

	pVO->Init(*pVec1, aParams[3]);

	return(0);
}

cell AMX_NATIVE_CALL CFST_Vec3ObjWrapper::CFV3PointPath1_Acquire(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FV3Points1_Acquire", 0 );

	// REPAIR: This should get changed when the below function gets removed.
	CFV3OPointPath1 *pTemp = CFV3OPointPath1::GetAvailable();

	return((cell)(pTemp));
}

cell AMX_NATIVE_CALL CFST_Vec3ObjWrapper::CFV3PointPath1_Release(AMX *pAMX, cell *aParams)
{
	// REPAIR: This should get changed when a release function gets added to CFVec3Objs.
	return(0);
}

cell AMX_NATIVE_CALL CFST_Vec3ObjWrapper::CFV3PointPath2_Acquire(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FV3Points2_Acquire", 0 );
	// REPAIR: This should get changed when the below function gets removed.
	CFV3OPointPath2 *pTemp = CFV3OPointPath2::GetAvailable();

	return((cell)(pTemp));
}

cell AMX_NATIVE_CALL CFST_Vec3ObjWrapper::CFV3PointPath2_Release(AMX *pAMX, cell *aParams)
{
	// REPAIR: This should get changed when a release function gets added to CFVec3Objs.
	return(0);
}
/*
cell AMX_NATIVE_CALL CFST_Vec3ObjWrapper::CFV3PointPath2_Init1(AMX *pAMX, cell *aParams)
{
	// JUSTIN : Verify the incoming parameters.

	CFV3OPointPath2 *pVO = (CFV3OPointPath2 *)(aParams[1]);
	FASSERT(pVO != NULL);

//	char *pszPathName;
//	pszPathName = CreateAndFillStringFromCell(pAMX, aParams[2]);
	char szPathName[20];
	FillStringFromCell(szPathName, 19, pAMX, aParams[2]);

	f32 fTotalTime = ConvertCellToF32(aParams[3]);

	pVO->Init(szPathName, fTotalTime, aParams[4]);

	return(0);
}
*/
cell AMX_NATIVE_CALL CFST_Vec3ObjWrapper::CFV3Linear_Acquire(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FV3Linear_Acquire", 0 );
	// REPAIR: This should get changed when the below function gets removed.
	CFV3OLine1 *pTemp = CFV3OLine1::GetAvailable();

	return((cell)(pTemp));
}

cell AMX_NATIVE_CALL CFST_Vec3ObjWrapper::CFV3Linear_Release(AMX *pAMX, cell *aParams)
{
	// REPAIR: This should get changed when a release function gets added to CFVec3Objs.
	return(0);
}

cell AMX_NATIVE_CALL CFST_Vec3ObjWrapper::CFV3Linear_Init(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FV3Linear_Init", 5 );
	// JUSTIN : Verify the incoming parameters.

	CFV3OLine1 *pVO = (CFV3OLine1 *)(aParams[1]);
	FASSERT(pVO != NULL);

	CFVec3A *pVec1 = (CFVec3A *)(aParams[2]);
	FASSERT(pVec1 != NULL);

	CFVec3A *pVec2 = (CFVec3A *)(aParams[3]);
	FASSERT(pVec2 != NULL);

	f32 fTotalTime = ConvertCellToF32(aParams[4]);

	pVO->Init(*pVec1, *pVec2, fTotalTime, aParams[5]);

	return(0);
}

//
////

// =============================================================================================================

BOOL CFST_QuatObjWrapper::ModuleStartup()
{
	return(TRUE);
}

void CFST_QuatObjWrapper::ModuleShutdown()
{
}

///////////////////////////////////////////////////////////////
// CFQuatObj functions.

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatObj_Start(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FQuatObj_Start", 1 );

	CFQuatObj *pQO = (CFQuatObj *)(aParams[1]);

	if(pQO == NULL)
	{
		return((cell)(0));
	}

	if(!pQO->IsInitted())
	{
		return((cell)(0));
	}

	pQO->Start();

	return(0);
}

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatObj_Pause(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FQuatObj_Pause", 1 );

	CFQuatObj *pQO = (CFQuatObj *)(aParams[1]);

	if(pQO == NULL)
	{
		return((cell)(0));
	}

	if(!pQO->IsInitted())
	{
		return((cell)(0));
	}

	pQO->Pause();

	return(0);
}

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatObj_End(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FQuatObj_End", 1 );

	CFQuatObj *pQO = (CFQuatObj *)(aParams[1]);

	if(pQO == NULL)
	{
		return((cell)(0));
	}

	if(!pQO->IsInitted())
	{
		return((cell)(0));
	}

	pQO->End();

	return(0);
}

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatObj_Reset(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FQuatObj_Reset", 1 );

	CFQuatObj *pQO = (CFQuatObj *)(aParams[1]);

	if(pQO == NULL)
	{
		return((cell)(0));
	}

	if(!pQO->IsInitted())
	{
		return((cell)(0));
	}
	pQO->Reset();

	return(0);
}

//
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
// CFQuatTwirlY functions.

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatTwirlY_Acquire(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FQuatTwirlY_Acquire", 0 );

	// REPAIR: This should get changed when the below function gets removed.
	CFQOTwirlY *pTemp = CFQOTwirlY::GetAvailable();

	return((cell)(pTemp));
}

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatTwirlY_Release(AMX *pAMX, cell *aParams)
{
	// REPAIR: This should get changed when a release function gets added to CFVec3Objs.
	return(0);
}

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatTwirlY_Init(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FQuatTwirlY_", 3 );

	// JUSTIN : Verify the incoming parameters.

	CFQOTwirlY *pQO = (CFQOTwirlY *)(aParams[1]);
	FASSERT(pQO != NULL);

	f32 fOmegaY = ConvertCellToF32(aParams[2]);

	CFQuatA qTemp;
	qTemp.Identity();
	pQO->Init(qTemp, fOmegaY, aParams[3]);

	return(0);
}

//
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
// CFQuatTang1 functions.

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatTang1_Acquire(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FQuatTang1_Acquire", 0 );

	// REPAIR: This should get changed when the below function gets removed.
	CFQOTang1 *pTemp = CFQOTang1::GetAvailable();

	return((cell)(pTemp));
}

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatTang1_Release(AMX *pAMX, cell *aParams)
{
	// REPAIR: This should get changed when a release function gets added to CFVec3Objs.
	return(0);
}

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatTang1_Init(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FQuatTang1_Init", 2 );

	CFQOTang1 *pQO = (CFQOTang1 *)(aParams[1]);
	FASSERT(pQO != NULL);

	CFVec3AObj *pV3O = (CFVec3AObj *)(aParams[2]);

	if(pV3O == NULL)
	{
		return((cell)(0));
	}

	if(!pV3O->IsInitted())
	{
		return((cell)(0));
	}

	// Script definition of FQuatTang1_Init in fquattang1.inc accepts two parameters, not three.  The second 
	// parameter of Init() below is a boolean, but is unused by the Init() function...  -cjm
//	pQO->Init(pV3O, aParams[3]);
	pQO->Init(pV3O);

	return(0);
}

//
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
// CFQuatTang2 functions.

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatTang2_Acquire(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FQuatTang2_Acquire", 0 );

	// REPAIR: This should get changed when the below function gets removed.
	CFQuatTang2 *pTemp = CFQuatTang2::GetAvailable();

	return((cell)(pTemp));
}

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatTang2_Release(AMX *pAMX, cell *aParams)
{
	// REPAIR: This should get changed when a release function gets added to CFVec3Objs.
	return(0);
}

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatTang2_Init(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FQuatTang2_Init", 2 );

	CFQuatTang2 *pQO = (CFQuatTang2 *)(aParams[1]);
	FASSERT(pQO != NULL);

	CFVec3AObj *pV3O = (CFVec3AObj *)(aParams[2]);
	pQO->Init(pV3O, aParams[3]);

	return(0);
}

//
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
// CFQuatTang3 functions.

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatTang3_Acquire(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FQuatTang3_Acquire", 0 );

	// REPAIR: This should get changed when the below function gets removed.
	CFQuatTang3 *pTemp = CFQuatTang3::GetAvailable();

	return((cell)(pTemp));
}

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatTang3_Release(AMX *pAMX, cell *aParams)
{
	// REPAIR: This should get changed when a release function gets added to CFVec3Objs.
	return(0);
}

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatTang3_Init(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FQuatTang2_Init", 2 );

	CFQuatTang3 *pQO = (CFQuatTang3 *)(aParams[1]);
	FASSERT(pQO != NULL);

	CFVec3AObj *pV3O = (CFVec3AObj *)(aParams[2]);
	pQO->Init(pV3O, aParams[3]);

	return(0);
}

//
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
// CFQuatComp functions.

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatComp_Acquire(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FQuatComp_Acquire", 0 );

	// REPAIR: This should get changed when the below function gets removed.
	CFQuatComp *pTemp = CFQuatComp::GetAvailable();

	return((cell)(pTemp));
}

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatComp_Release(AMX *pAMX, cell *aParams)
{
	// REPAIR: This should get changed when a release function gets added to CFVec3Objs.
	return(0);
}

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatComp_Init(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FQuatComp_Init", 3 );
	CFQuatComp *pQO = (CFQuatComp *)(aParams[1]);
	FASSERT(pQO != NULL);

	CFQuatObj *pQO1 = (CFQuatObj *)(aParams[2]);
	CFQuatObj *pQO2 = (CFQuatObj *)(aParams[3]);


	// Script definition of FQuatComp_Init in fquatcomp.inc accepts three parameters, not four.  The third 
	// parameter of Init() below is a boolean, but is unused by the Init() function...  -cjm
//	pQO->Init(pQO1, pQO2, aParams[4]);
	pQO->Init(pQO1, pQO2);

	return(0);
}

//
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
// CFQuatLookAt1 functions.

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatLookAt1_Acquire(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FQuatLook1_Acquire", 0 );
	// REPAIR: This should get changed when the below function gets removed.
	CFQuatLookAt1 *pTemp = CFQuatLookAt1::GetAvailable();

	return((cell)(pTemp));
}

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatLookAt1_Release(AMX *pAMX, cell *aParams)
{
	// REPAIR: This should get changed when a release function gets added to CFVec3Objs.
	return(0);
}

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatLookAt1_Init(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FQuatLook1_Init", 3 );
	CFQuatLookAt1 *pQO = (CFQuatLookAt1 *)(aParams[1]);
	FASSERT(pQO != NULL);

	CFVec3AObj *pV3O1 = (CFVec3AObj *)(aParams[2]);

	if(pV3O1 == NULL)
	{
		return((cell)(0));
	}
	if(!pV3O1->IsInitted())
	{
		return((cell)(0));
	}

	CFVec3AObj *pV3O2 = (CFVec3AObj *)(aParams[3]);

	if(pV3O2 == NULL)
	{
		return((cell)(0));
	}
	if(!pV3O2->IsInitted())
	{
		return((cell)(0));
	}

	// Script definition of FQuatLook1_Init in fquatlookat1.inc accepts three parameters, not four.  The third 
	// parameter of Init() below is a boolean, but is unused by the Init() function...  -cjm
//	pQO->Init(pV3O1, pV3O2, aParams[4]);
	pQO->Init(pV3O1, pV3O2 );

	return(0);
}

//
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
// CFQuatSLERP functions.

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatSLERP_Acquire(AMX *pAMX, cell *aParams)
{
	// REPAIR: This should get changed when the below function gets removed.
	CFQuatSLERP *pTemp = CFQuatSLERP::GetAvailable();

	return((cell)(pTemp));
}

cell AMX_NATIVE_CALL CFST_QuatObjWrapper::CFQuatSLERP_Release(AMX *pAMX, cell *aParams)
{
	// REPAIR: This should get changed when a release function gets added to CFVec3Objs.
	return(0);
}

//
///////////////////////////////////////////////////////////////

// =============================================================================================================

BOOL CFST_ScalarObjWrapper::ModuleStartup()
{
	return(TRUE);
}

void CFST_ScalarObjWrapper::ModuleShutdown()
{
}

// =============================================================================================================
