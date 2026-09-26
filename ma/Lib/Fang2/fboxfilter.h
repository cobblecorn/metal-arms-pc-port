//////////////////////////////////////////////////////////////////////////////////////
// fboxfilter.h - Fang box filter module.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 11/28/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FBOXFILTER_H_
#define _FBOXFILTER_H_ 1

#include "fang.h"




typedef void *FBoxFilterHandle_t;



extern BOOL fboxfilter_ModuleStartup( void );
extern void fboxfilter_ModuleShutdown( void );

extern FBoxFilterHandle_t fboxfilter_Create_u32( u32 nMaxElements );
extern void fboxfilter_Reset_u32( FBoxFilterHandle_t hFBoxFilter );
extern u32 fboxfilter_GetFilterElementCount_u32( FBoxFilterHandle_t hFBoxFilter );
extern u32 fboxfilter_SetFilterElementCount_u32( FBoxFilterHandle_t hFBoxFilter, u32 nFilterElementCount );
extern void fboxfilter_Add_u32( FBoxFilterHandle_t hFBoxFilter, u32 nValue );
extern void fboxfilter_Get_u32( FBoxFilterHandle_t hFBoxFilter,
							   u32 *pnLastValue, u32 *pnAvgValue, float *pfAvgValue, u32 *pnSum, u32 *pnNumValues );

extern FBoxFilterHandle_t fboxfilter_Create_s32( u32 nMaxElements );
extern void fboxfilter_Reset_s32( FBoxFilterHandle_t hFBoxFilter );
extern u32 fboxfilter_GetFilterElementCount_s32( FBoxFilterHandle_t hFBoxFilter );
extern u32 fboxfilter_SetFilterElementCount_s32( FBoxFilterHandle_t hFBoxFilter, u32 nFilterElementCount );
extern void fboxfilter_Add_s32( FBoxFilterHandle_t hFBoxFilter, s32 nValue );
extern void fboxfilter_Get_s32( FBoxFilterHandle_t hFBoxFilter,
							   s32 *pnLastValue, s32 *pnAvgValue, float *pfAvgValue, s32 *pnSum, u32 *pnNumValues );

extern FBoxFilterHandle_t fboxfilter_Create_f32( u32 nMaxElements );
extern void fboxfilter_Reset_f32( FBoxFilterHandle_t hFBoxFilter );
extern u32 fboxfilter_GetFilterElementCount_f32( FBoxFilterHandle_t hFBoxFilter );
extern u32 fboxfilter_SetFilterElementCount_f32( FBoxFilterHandle_t hFBoxFilter, u32 nFilterElementCount );
extern void fboxfilter_Add_f32( FBoxFilterHandle_t hFBoxFilter, f32 fValue );
extern void fboxfilter_Get_f32( FBoxFilterHandle_t hFBoxFilter,
							   f32 *pfLastValue, float *pfAvgValue, f32 *pfSum, u32 *pnNumValues );





#endif

