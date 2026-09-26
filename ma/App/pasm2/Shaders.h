//////////////////////////////////////////////////////////////////////////////////////
// Shaders.h - tool shader structs for both DX and GC
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 02/21/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _SHADERS_H_
#define _SHADERS_H_ 1

#include "fang.h"
#include "fshaders.h"
#include "TriPacket.h"
/*
typedef enum {
	// 1 layer
	ShaderTypes_oBASE = 0,
	ShaderTypes_cBASE,
	ShaderTypes_tBASE,
	ShaderTypes_eoBASE,
	ShaderTypes_ecBASE,
	ShaderTypes_etBASE,
	ShaderTypes_oBASE_ADD_SPEC,
	ShaderTypes_cBASE_ADD_SPEC,
	ShaderTypes_tBASE_ADD_SPEC,
	ShaderTypes_eoBASE_ADD_SPEC,
	ShaderTypes_ecBASE_ADD_SPEC,
	ShaderTypes_etBASE_ADD_SPEC,
	ShaderTypes_oBASE_ADD_bSPEC,
	ShaderTypes_eoBASE_ADD_bSPEC,	
	ShaderTypes_etBASE_ADD_bSPEC,
	ShaderTypes_beoBASE,
	ShaderTypes_beoBASE_ADD_SPEC,
	ShaderTypes_ADD_BASE,
	// 2 layer
	ShaderTypes_oBASE_LERP_tLAYER,
	ShaderTypes_oBASE_LERP_vLAYER,
	ShaderTypes_oBASE_LERP_pLAYER,
	ShaderTypes_oBASE_ADD_SPEC_LERP_tLAYER,
	ShaderTypes_cBASE_LERP_tLAYER,
	ShaderTypes_cBASE_LERP_vLAYER,
	ShaderTypes_cBASE_LERP_pLAYER,
	ShaderTypes_cBASE_ADD_SPEC_LERP_tLAYER,
	ShaderTypes_tBASE_LERP_tLAYER,
	ShaderTypes_tBASE_ADD_SPEC_LERP_tLAYER,
	ShaderTypes_oBASE_MOD_SHADOWMAP,
	ShaderTypes_cBASE_MOD_SHADOWMAP,
	ShaderTypes_oBASE_MOD_SHADOWMAP_ADD_SPEC,
	ShaderTypes_cBASE_MOD_SHADOWMAP_ADD_SPEC,
	ShaderTypes_oBASE_ADD_rbENV,
	ShaderTypes_etBASE_ADD_rbENV,
	ShaderTypes_oBASE_ADD_rbENV_MOD_SHADOWMAP,
	
	ShaderTypes_Count
} ShaderTypes_e;// only exists so that we can ID various shaders

//////////////////
// DX SHADER STUFF
//////////////////

typedef union {
	FShaderReg_oBASE_t							oBASE;
	FShaderReg_cBASE_t							cBASE;
	FShaderReg_tBASE_t							tBASE;
	FShaderReg_eoBASE_t							eoBASE;
	FShaderReg_ecBASE_t							ecBASE;
	FShaderReg_etBASE_t							etBASE;
	FShaderReg_oBASE_ADD_SPEC_t					oBASE_ADD_SPEC;
	FShaderReg_cBASE_ADD_SPEC_t					cBASE_ADD_SPEC;
	FShaderReg_tBASE_ADD_SPEC_t					tBASE_ADD_SPEC;
	FShaderReg_eoBASE_ADD_SPEC_t				eoBASE_ADD_SPEC;
	FShaderReg_ecBASE_ADD_SPEC_t				ecBASE_ADD_SPEC;
	FShaderReg_etBASE_ADD_SPEC_t				etBASE_ADD_SPEC;
	FShaderReg_oBASE_ADD_bSPEC_t				oBASE_ADD_bSPEC;
	FShaderReg_eoBASE_ADD_bSPEC_t				eoBASE_ADD_bSPEC;
	FShaderReg_etBASE_ADD_bSPEC_t				etBASE_ADD_bSPEC;
	FShaderReg_beoBASE_t						beoBASE;
	FShaderReg_beoBASE_ADD_SPEC_t				beoBASE_ADD_SPEC;
	FShaderReg_oBASE_LERP_tLAYER_t				oBASE_LERP_tLAYER;
	FShaderReg_oBASE_LERP_vLAYER_t				oBASE_LERP_vLAYER;
	FShaderReg_oBASE_LERP_pLAYER_t				oBASE_LERP_pLAYER;
	FShaderReg_oBASE_ADD_SPEC_LERP_tLAYER_t		oBASE_ADD_SPEC_LERP_tLAYER;
	FShaderReg_cBASE_LERP_tLAYER_t				cBASE_LERP_tLAYER;
	FShaderReg_cBASE_LERP_vLAYER_t				cBASE_LERP_vLAYER;
	FShaderReg_cBASE_LERP_pLAYER_t				cBASE_LERP_pLAYER;
	FShaderReg_cBASE_ADD_SPEC_LERP_tLAYER_t		cBASE_ADD_SPEC_LERP_tLAYER;
	FShaderReg_tBASE_LERP_tLAYER_t				tBASE_LERP_tLAYER;
	FShaderReg_tBASE_ADD_SPEC_LERP_tLAYER_t		tBASE_ADD_SPEC_LERP_tLAYER;
	FShaderReg_oBASE_MOD_SHADOWMAP_t			oBASE_MOD_SHADOWMAP;
	FShaderReg_cBASE_MOD_SHADOWMAP_t			cBASE_MOD_SHADOWMAP;
	FShaderReg_oBASE_MOD_SHADOWMAP_ADD_SPEC_t	oBASE_MOD_SHADOWMAP_ADD_SPEC;
	FShaderReg_cBASE_MOD_SHADOWMAP_ADD_SPEC_t	cBASE_MOD_SHADOWMAP_ADD_SPEC;
	FShaderReg_oBASE_ADD_rbENV_t				oBASE_ADD_rbENV;
	FShaderReg_etBASE_ADD_rbENV_t				etBASE_ADD_rbENV;
	FShaderReg_oBASE_ADD_rbENV_MOD_SHADOWMAP_t	oBASE_ADD_rbENV_MOD_SHADOWMAP;
} DXShaders_t;// only exists so that we can compute the size of the largest shader

#define SIZEOF_LARGEST_SHADER	sizeof( DXShaders_t )
#define MAX_SHADER_REGS			(SIZEOF_LARGEST_SHADER >> 2)

#define _DMOTIF		1
#define _EMOTIF		2
#define _SMOTIF		3
#define _BASE		4
#define _LAYER1		5
#define _TC0		6
#define _TC1		7
#define _SEXP		8
#define _ENV_MOTIF	9
#define _LAYER2		10

typedef struct {
	u32 nRegType;// these are FDX8Data_MeshShRegType_e
	u32 nRegCode;
} DXShaderReg_t;

typedef struct {
	char *pszName;
	DXShaderReg_t anRegs[MAX_SHADER_REGS];
	u32 nNumRegs;
} DXShaderData_t;
*/
#endif

