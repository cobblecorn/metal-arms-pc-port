//////////////////////////////////////////////////////////////////////////////////////
// fshaders.h - Fang shader implementation definitions.
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
// 11/09/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FSHADERS_H_
#define _FSHADERS_H_ 1

#include "fang.h"
#include "fdata.h"


struct FMesh_t;


typedef enum 
{
	FSHADERS_PASS_NONE		= 0x00000000,
	FSHADERS_PASS_LIGHTING	= 0x00000001,
	FSHADERS_PASS_SURFACE	= 0x00000002,
	FSHADERS_PASS_SPECULAR	= 0x00000004,
	FSHADERS_PASS_ALL		= (FSHADERS_PASS_LIGHTING|FSHADERS_PASS_SURFACE|FSHADERS_PASS_SPECULAR),
};

///////////////////////////////////////////////////////////////////////
// Structs for efficient processing of registers
///////////////////////////////////////////////////////////////////////

//
//	Surface Shaders
typedef enum 
{
	FSHADERS_oBASE = 0,
	FSHADERS_cBASE,
	FSHADERS_tBASE,
	FSHADERS_etBASE,
	FSHADERS_tBASE_ADD_SPEC,
	FSHADERS_etBASE_ADD_SPEC,
	FSHADERS_etBASE_ADD_bSPEC,
	FSHADERS_ADD_BASE,
	FSHADERS_oBASE_LERP_tLAYER,
	FSHADERS_oBASE_LERP_vLAYER,
	FSHADERS_oBASE_LERP_pLAYER,
	FSHADERS_cBASE_LERP_tLAYER,
	FSHADERS_cBASE_LERP_vLAYER,
	FSHADERS_cBASE_LERP_pLAYER,
	FSHADERS_tBASE_LERP_tLAYER,
	FSHADERS_tBASE_ADD_SPEC_LERP_tLAYER,	//15
	FSHADERS_oBASE_ADD_rbENV,
	FSHADERS_etBASE_ADD_rbENV,
	
	//DetailMap shader variations - Note detail map effects the base layer: Layer0_Out = (DetailMap+0.5f)*Base
	FSHADERS_oBASE_DETAIL,
	FSHADERS_cBASE_DETAIL,
	FSHADERS_tBASE_DETAIL,		//20
	FSHADERS_etBASE_DETAIL,
	FSHADERS_tBASE_ADD_SPEC_DETAIL,
	FSHADERS_etBASE_ADD_SPEC_DETAIL,
	FSHADERS_etBASE_ADD_bSPEC_DETAIL,
	FSHADERS_ADD_BASE_DETAIL,			//25
	FSHADERS_oBASE_LERP_tLAYER_DETAIL,
	FSHADERS_oBASE_LERP_vLAYER_DETAIL,
	FSHADERS_oBASE_LERP_pLAYER_DETAIL,
	FSHADERS_cBASE_LERP_tLAYER_DETAIL,
	FSHADERS_cBASE_LERP_vLAYER_DETAIL,	//30
	FSHADERS_cBASE_LERP_pLAYER_DETAIL,
	FSHADERS_tBASE_LERP_tLAYER_DETAIL,
	FSHADERS_tBASE_ADD_SPEC_LERP_tLAYER_DETAIL,
	FSHADERS_oBASE_ADD_rbENV_DETAIL,
	FSHADERS_etBASE_ADD_rbENV_DETAIL,	//35
	//DetailMap end
#if FSH_DYNAMIC_SREFLECT
	FSHADERS_oBASE_ADD_rbSREFLECT, //dynamic sphere reflect.
#endif
	FSHADERS_tBASE_vALPHA,		//37

	//Insert new shaders here if they go into PASM
	FSHADERS_LIQUID_ENV,		//38
	FSHADERS_LIQUID_LAYER_ENV,	//39
	FSHADERS_LIQUID_TEXTURE,
	FSHADERS_LIQUID_MOLTEN_1LAYER,
	FSHADERS_LIQUID_MOLTEN_2LAYER,

	//reflective blend shaders: 3 tex on XBox, 4-5 tex on GC.
	FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV,
	FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV,
	FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV,
	//reflective blend shaders with detail maps: 4 tex on XBox, 5-6 tex on GC.
	//NOTE: these shaders run at 1/2 fillrate on XBOX!
	FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV_DETAIL,
	FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV_DETAIL,
	FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV_DETAIL,
	//

	//new shaders using vertex alpha
	FSHADERS_pBASE,
	FSHADERS_epBASE,
	FSHADERS_pBASE_ADD_SPEC,
	FSHADERS_epBASE_ADD_SPEC,
	FSHADERS_epBASE_ADD_bSPEC,
	FSHADERS_ADD_vBASE,
	//
	
	//Shaders not directly set by artists, therefore PASM doesn't need to know.
	FSHADERS_LIGHT_MORPH,

	//FDRAW Shaders
	FSHADERS_DECAL_AI,
	FSHADERS_DECALTEX_AI,
	FSHADERS_DECALTEX_AT,
	FSHADERS_DIFFUSETEX_AI,
	FSHADERS_DIFFUSETEX_AT,
	FSHADERS_DIFFUSETEX_AIAT,
	FSHADERS_SPECULARTEX_AT,
	FSHADERS_ADD,
	FSHADERS_BLEND_AIPLUSAT,
	FSHADERS_CT_PLUS_CIAT_AI,
	
	#if FANG_PLATFORM_GC
	FSHADERS_INTENSITY,
	FSHADERS_tBASE_LM,
	FSHADERS_tBASE_DETAIL_LM,
	FSHADERS_tBASE_LERP_tLAYER_LM,
//ARG - >>>>>
	#elif FANG_PLATFORM_PS2
	FSHADERS_INTENSITY,
//ARG - <<<<<
	#endif
	//end FDRAW Shaders

	//Utility shader for making oBASE translucent based on Surface Alpha
	FSHADERS_otBASE,

	FSHADERS_SHADER_COUNT

} FShaders_Types_e;

//
//	Light Shaders
typedef enum
{
	FSHADERS_FULLBRIGHT,
	FSHADERS_FULLBRIGHT_CO,
	FSHADERS_VLIGHT_BASIC, // Ca + Cv + Cm[Me] + LITD(i)*Cm[Md]
	FSHADERS_VLIGHT_BASIC_CO, // Ca + Cv + Cm[Me] + LITD(i)*Cm[Md]
	FSHADERS_VLIGHT_MASK_EMISSIVE, //Ca + Cv + Cm[Me]*At0 + LITD(i)*Cm[Md]
	FSHADERS_VLIGHT_MASK_EMISSIVE_CO, //Ca + Cv + Cm[Me]*At0 + LITD(i)*Cm[Md]
	
#if FANG_PLATFORM_GC	
	FSHADERS_LM1,
	FSHADERS_LM1_ZMASK,
	FSHADERS_LM1_EMASK,
	FSHADERS_LM1_ZMASK_EMASK,
	FSHADERS_LM2,
	FSHADERS_LM2_ZMASK,
	FSHADERS_LM2_EMASK,
	FSHADERS_LM2_ZMASK_EMASK,
	FSHADERS_LM3,
	FSHADERS_LM3_ZMASK,
	FSHADERS_LM3_EMASK,
	FSHADERS_LM3_ZMASK_EMASK,
	FSHADERS_LM4,
	FSHADERS_LM4_ZMASK,
	FSHADERS_LM4_EMASK,
	FSHADERS_LM4_ZMASK_EMASK,
//ARG - >>>>>
#elif FANG_PLATFORM_PS2
	FSHADERS_LM1,
	FSHADERS_LM1_ZMASK,
	FSHADERS_LM1_EMASK,
	FSHADERS_LM1_ZMASK_EMASK,
	FSHADERS_LM2,
	FSHADERS_LM2_ZMASK,
	FSHADERS_LM2_EMASK,
	FSHADERS_LM2_ZMASK_EMASK,
	FSHADERS_LM3,
	FSHADERS_LM3_ZMASK,
	FSHADERS_LM3_EMASK,
	FSHADERS_LM3_ZMASK_EMASK,
	FSHADERS_LM4,
	FSHADERS_LM4_ZMASK,
	FSHADERS_LM4_EMASK,
	FSHADERS_LM4_ZMASK_EMASK,
//ARG - <<<<<
#endif
	
	//Per-pixel point light shaders
	//FSHADERS_PERPIXELPOINT_BASIC1,  //4 at a time.
	//FSHADERS_PERPIXELPOINT_BASIC2,  //3 at a time.
	//FSHADERS_PERPIXELPOINT_BASIC3,	//2 at a time.
	//FSHADERS_PERPIXELPOINT_BASIC4,	//1 at a time.
	
	FSHADERS_DIFFUSE_COUNT,
	FSHADERS_DIFFUSE_ROUTE = 6
} FShaders_Diffuse_Types_e;

//
//	Specular Shaders
typedef enum
{
	FSHADERS_VSPEC_BASIC,
	FSHADERS_VSPEC_MASK,
	FSHADERS_VSPEC_BASIC_CO,
	FSHADERS_VSPEC_BASIC_MASK_CO,
	
	#if FANG_PLATFORM_GC
//	FSHADERS_VSPEC_BASIC_BUMPMAP,
//	FSHADERS_VSPEC_MASK_BUMPMAP,
	#endif

	FSHADERS_SPECULAR_COUNT
} FShaders_Specular_Types_e;

//
// Surface Registers
typedef enum
{
	FSHADERS_REG_LAYER0,
	FSHADERS_REG_TC0,
	FSHADERS_REG_LAYER1,
	FSHADERS_REG_TC1,
	FSHADERS_REG_LAYER2,
	FSHADERS_REG_TC2,
	FSHADERS_REG_LAYER3,
	FSHADERS_REG_TC3,
	FSHADERS_REG_ENV_MOTIF,
	FSHADERS_REG_DETAILMAP,
	FSHADERS_REG_DETAILMAP_TILE_FACTOR,
	FSHADERS_REG_MAX,
	FSHADERS_REG_NULL = 0xff,
} FShaders_RegType_e;


//
// Light registers
typedef enum
{
	FSHADERS_LIGHT_REG_ZMASK=0,
	FSHADERS_LIGHT_REG_EMASK,
	FSHADERS_LIGHT_REG_SMASK,
	FSHADERS_LIGHT_REG_BUMPMAP,
	FSHADERS_LIGHT_REG_BUMPMAP_TILE_FACTOR,
	FSHADERS_LIGHT_REG_DMOTIF,
	FSHADERS_LIGHT_REG_EMOTIF,
	FSHADERS_LIGHT_REG_SMOTIF,
	FSHADERS_LIGHT_REG_SEXP,
	FSHADERS_LIGHT_REG_LMCOUNT,
	FSHADERS_LIGHT_REG_LM,
	FSHADERS_LIGHT_REG_LM_TC,
	FSHADERS_LIGHT_REG_LM_MOTIF,
	FSHADERS_LIGHT_REG_MAX,
	FSHADERS_LIGHT_REG_NULL = 0xff,
} FShaders_LightRegOrder_e;


enum
{
	FSHADERS_SURFACE_FLAG_OPAQUE		= 0x00000001,
	FSHADERS_SURFACE_FLAG_CUTOUT		= 0x00000002,
	FSHADERS_SURFACE_FLAG_TRANSLUCENT	= 0x00000004,
	FSHADERS_SURFACE_FLAG_DETAIL_MAP	= 0x00000008,
};


//
//
typedef struct 
{
	u16 nRegisterCount;
	u16 nTexLayerCount;
	u32 nUVCount;
	u32 nSurfaceTypeFlags;
	FShaders_RegType_e anRegType[FDATA_MAX_SURFACE_SHADER_REGS];

} FShaderReg_t;

//
//
typedef struct 
{
	u16 nRegisterCount;
	FShaders_LightRegOrder_e anRegType[FDATA_MAX_LIGHT_SHADER_REGS];

} FLightShaderReg_t;


extern BOOL fshaders_FixupLightRegisters( FMesh_t *pLoadBase, u32 nLightShaderIdx, u32 nSpecularShaderIdx, u32 *pnRegisters );
extern BOOL fshaders_FixupSurfaceRegisters( FMesh_t *pLoadBase, u32 nShaderIdx, u32 *pnRegisters );

extern const FShaderReg_t FShaders_aShaderRegs[FSHADERS_SHADER_COUNT];
extern const FLightShaderReg_t FShaders_aLightShaderRegs[FSHADERS_DIFFUSE_COUNT];

extern const FShaders_Types_e FShaders_anSurface_Detail[FSHADERS_SHADER_COUNT];




#endif

