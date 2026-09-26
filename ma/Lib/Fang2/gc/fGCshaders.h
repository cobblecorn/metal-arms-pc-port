//////////////////////////////////////////////////////////////////////////////////////
// fGCshaders.h - Fang GameCube shader programs.
//
// Author: John Lafleur 
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
// 02/18/02	Lafleur		Created from stubbed DX version
//////////////////////////////////////////////////////////////////////////////////////


#ifndef _FGCSHADERS_H_
#define _FGCSHADERS_H_ 1

typedef enum
{
	FSH_IND_DUDV=0,
	FSH_IND_WARP_ADD,
	FSH_IND_WARP_REPLACE,
	FSH_IND_XYZ,
	
	FSH_NUM_IND_TYPES,
	
	FSH_IND_INVALID=0xffff,
	
	MAX_IND_STAGES=4
} FShader_IndLookTypes_e;

typedef struct
{
	GXTevStageID TevStage;
    GXIndTexStageID IndStage;
    GXIndTexFormat Format;
    GXIndTexBiasSel BiasSel;
    GXIndTexMtxID MatrixSel;
    GXIndTexWrap WrapS;
    GXIndTexWrap WrapT;
    GXBool bAddPrev;
    GXBool bUTC_Lod; 
    GXIndTexAlphaSel AlphaSel;
} FShaderIndirectStage_t;

typedef struct
{
	u8 nNumIndStages, nNumIndReg;
	s8 nScaleExp, nPad8;
	FShaderIndirectStage_t aIndStages[MAX_IND_STAGES];
} FShaderIndirectLookup_t;

FShaderIndirectLookup_t _aIndLookupTypes[FSH_NUM_IND_TYPES];

typedef struct {
	u8 nStages, nTexGens, nTexures;
	u8 texGen[8];
	BOOL8 bAlphaTest;
	u8 nAlphaFunc;
	u8 nAlphaRef;
	
	u8 bBlendMode;		//u8
	u8 bSourceF;		//u8
	u8 bDestF;		//u8
	u8 bLogicOp;			//u8
	
	//lighting parameters.
	u8 nFactorReg;	
	
	//indirect parameters (warping, bumpmapping, tiled textures, etc).
	FShader_IndLookTypes_e IndLookupType;
} FShaderRenderStates_t;

typedef struct {
	//Konst registers
	u8 nColorKonst;	//u8
	u8 nAlphaKonst;	//u8
	//TevOrder: texcoord id, texmap id, color channel ID.
	u8 texCoordID;	//u8
	u16 texMapID;		//u16
	u8 channelID;		//u8

	//colorResArg = (cD colorOp cA(1-cC) + cB(cC) + colorBias)*colorScale;
	u8 cA, cB, cC, cD;	//u8
	u8 colorOp;				//u8	
	u8 colorBias;			//u8
	u8 colorScale;			//u8
	GXBool colorClamp;
	u8 colorResArg;			//u8
	
	//alphaResArg = (aD alphaOp aA(1-aC) + aB(aC) + alphaBias)*alphaScale;
	u8 aA, aB, aC, aD;	//u8
	u8 alphaOp;				//u8
	u8 alphaBias;			//u8
	u8 alphaScale;			//u8
	GXBool alphaClamp;
	u8 alphaResArg;			//u8
	
	//Swap Mode.
	u8 swapColor;			//u8
	u8 swapAlpha;			//u8

	//use Environment mapping	
	BOOL8 bEMBM;
} FShaderStageStates_t;

enum
{
	_USER_DATA_FLAG = 0x40,
	_DETAIL_SCALE = 0x40,
	_ADDR_LM = -2,
	
	_ZMASK = 0x01,
	_SMASK = 0x02,
	_EMASK = 0x03
};

typedef struct {
	//Generic data.
	s8 nTexReg, nTCIReg;
	u8 texGenType;	//u8
	u8 texGenSrc;		//u8
	u32 mtx, nMask; //0 = none, 1 = z mask, 2 = spec mask, 3 = emissive mask
} FShaderTexGen_t;

/*****default*****
Stage
{
	GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
	GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
	GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
	GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
	GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
	GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
	GX_TEV_SWAP0, GX_TEV_SWAP0						//swap table indices.
	FALSE								//envmapped texgen
}
TexGen
{
	0, 1,									//TexReg, TCIReg
	GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY   //TexGenType, TexSrc, TexMtx
}
******************/

static u16 *_pnShaderRemap = NULL;

static u16 _anSurfaceRemap[] =
{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 
	30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
	40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
	50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
	60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
	70, 71, 72, 73, 74, 75, 76, 77, 78, 79
};

static u16 _anDiffuseRemap[] =
{
	FSHADERS_SHADER_COUNT, FSHADERS_SHADER_COUNT + 1, FSHADERS_SHADER_COUNT + 2,
	FSHADERS_SHADER_COUNT + 3, FSHADERS_SHADER_COUNT + 4, FSHADERS_SHADER_COUNT + 5,
	FSHADERS_SHADER_COUNT + 6, FSHADERS_SHADER_COUNT + 7, FSHADERS_SHADER_COUNT + 8,
	FSHADERS_SHADER_COUNT + 9, FSHADERS_SHADER_COUNT +10, FSHADERS_SHADER_COUNT +11,
	FSHADERS_SHADER_COUNT +12, FSHADERS_SHADER_COUNT +13, FSHADERS_SHADER_COUNT +14,
	FSHADERS_SHADER_COUNT +15, FSHADERS_SHADER_COUNT +16, FSHADERS_SHADER_COUNT +17,
	FSHADERS_SHADER_COUNT +18, FSHADERS_SHADER_COUNT +19, FSHADERS_SHADER_COUNT +20,
	FSHADERS_SHADER_COUNT +21
};

static u16 _anSpecularRemap[] =
{
	FSHADERS_SHADER_COUNT + FSHADERS_DIFFUSE_COUNT, 
	FSHADERS_SHADER_COUNT + FSHADERS_DIFFUSE_COUNT + 1,
	FSHADERS_SHADER_COUNT + FSHADERS_DIFFUSE_COUNT + 2,
	FSHADERS_SHADER_COUNT + FSHADERS_DIFFUSE_COUNT + 3,
	FSHADERS_SHADER_COUNT + FSHADERS_DIFFUSE_COUNT + 4,
	FSHADERS_SHADER_COUNT + FSHADERS_DIFFUSE_COUNT + 5	
};

#if FANG_LIGHTING_2X
#define SRCBLEND GX_BL_DSTCLR
#else
#define SRCBLEND GX_BL_ZERO
#endif

static FShaderRenderStates_t _aShaderRenderStates[] =
{
	//FSHADERS_oBASE: 0
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_cBASE:
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_tBASE:
	{
		2, 1, 2,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_etBASE,
	{
		3, 1, 2,
		1, 2, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_tBASE_ADD_SPEC,
	{
		2, 1, 2,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_etBASE_ADD_SPEC, 5
	{
		3, 1, 2,
		1, 2, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_etBASE_ADD_bSPEC,
	{
		3, 1, 2,
		1, 2, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_ADD_BASE,
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_oBASE_LERP_tLAYER,
	{
		4, 2, 3,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_oBASE_LERP_vLAYER,
	{
		3, 2, 2,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_oBASE_LERP_pLAYER, 10
	{
		4, 2, 3,
		0, 1, 2, 4, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_cBASE_LERP_tLAYER,
	{
		4, 2, 3,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_cBASE_LERP_vLAYER,
	{
		3, 2, 2,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_cBASE_LERP_pLAYER,
	{
		4, 2, 3,
		0, 1, 2, 4, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_tBASE_LERP_tLAYER,
	{
		5, 2, 4,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_tBASE_ADD_SPEC_LERP_tLAYER, 15
	{
		5, 2, 4,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_oBASE_ADD_rbENV,
	{
		//3, 2, 3,
		5, 3, 3,
		1, 2, 3, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		4, FSH_IND_INVALID
	},
	//FSHADERS_etBASE_ADD_rbENV,
	{
		7, 3, 3,
		1, 2, 3, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		4, FSH_IND_INVALID
	},
	//FSHADERS_oBASE_DETAIL,
	{
		3, 2, 2,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADER_cBASE_DETAIL
	{
		3, 2, 2,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_tBASE_DETAIL 20
	{
		4, 2, 3,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_etBASE_DETAIL
	{
		5, 2, 3,
		1, 2, 3, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_tBASE_ADD_SPEC_DETAIL
	{
		4, 2, 3,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_etBASE_ADD_SPEC_DETAIL
	{
		5, 2, 3,
		1, 2, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_etBASE_ADD_bSPEC_DETAIL
	{
		5, 2, 3,
		1, 2, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_ADD_BASE_DETAIL 25
	{
		3, 2, 2,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
//		GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_oBASE_LERP_tLAYER_DETAIL
	{
		5, 3, 4,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_oBASE_LERP_vLAYER_DETAIL
	{
		4, 3, 3,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_oBASE_LERP_pLAYER_DETAIL
	{
		6, 3, 4,
		0, 1, 2, 4, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_cBASE_LERP_tLAYER_DETAIL
	{
		5, 3, 4,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_cBASE_LERP_vLAYER_DETAIL 30
	{
		4, 3, 3,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_cBASE_LERP_pLAYER_DETAIL -- too many
	{
		6, 3, 4,
		0, 1, 2, 4, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_tBASE_LERP_tLAYER_DETAIL -- too many
	{
		6, 3, 5,
		0, 1, 2, 4, 5, 6, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_tBASE_ADD_SPEC_LERP_tLAYER_DETAIL
	{
		6, 3, 5,
		0, 1, 2, 4, 5, 6, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_oBASE_ADD_rbENV_DETAIL
	{
		6, 4, 4,
		0, 1, 3, 4, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		6, FSH_IND_INVALID
	},
	//FSHADERS_etBASE_ADD_rbENV_DETAIL 35
	{
		5, 3, 3,
		1, 2, 3, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		6, FSH_IND_INVALID
	},
	#if FSH_DYNAMIC_SREFLECT
	//FSHADERS_oBASE_ADD_rbSREFLECT
	{
		3, 1, 2,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		4, FSH_IND_INVALID
	},
	#endif
	//FSHADERS_tBASE_vALPHA
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_LIQUID_ENV,		//38
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_LIQUID_LAYER_ENV,	//39
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_LIQUID_TEXTURE,	//40
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_LIQUID_MOLTEN_1LAYER,
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_LIQUID_MOLTEN_2LAYER,
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV,
	{
		6, 4, 5,
		0, 2, 3, 4, 5, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV,
	{
		5, 4, 4,
		0, 2, 3, 4, 5, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV, //45
	{
		6, 4, 5,
		0, 2, 3, 4, 5, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV_DETAIL,
	{
		8, 5, 6,
		0, 2, 3, 4, 5, 6, 6, 6,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV_DETAIL,
	{
		8, 5, 6,
		0, 2, 3, 4, 5, 6, 6, 6,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV_DETAIL,
	{
		8, 5, 6,
		0, 2, 3, 4, 5, 6, 6, 6,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADER_pBASE:
	{
		2, 1, 2,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_epBASE,
	{
		3, 1, 2,
		1, 2, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_pBASE_ADD_SPEC,
	{
		2, 1, 2,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_epBASE_ADD_SPEC, 5
	{
		3, 1, 2,
		1, 2, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_epBASE_ADD_bSPEC,
	{
		3, 1, 2,
		1, 2, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_ADD_vBASE,
	{
		2, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_LIGHT_MORPH,
	{
		1, 1, 1,
		1, 2, 3, 4, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, SRCBLEND, GX_BL_SRCCLR, GX_LO_SET,
		4, FSH_IND_INVALID
	},
	//FSHADERS_DECAL_AI,	//50
	{
		1, 0, 0,
		1, 2, 3, 4, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_DECALTEX_AI,
	{
		1, 1, 1,
		1, 2, 3, 4, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_DECALTEX_AT,
	{
		2, 1, 2,
		1, 2, 3, 4, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_DIFFUSETEX_AI,
	{
		1, 1, 1,
		1, 2, 3, 4, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_DIFFUSETEX_AT,
	{
		2, 1, 2,
		1, 2, 3, 4, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_DIFFUSETEX_AIAT,	//55
	{
		2, 1, 2,
		1, 2, 3, 4, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_SPECULARTEX_AT,
	{
		2, 1, 2,
		1, 2, 3, 4, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_ADD,
	{
		2, 1, 2,
		1, 2, 3, 4, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_BLEND_AIPLUSAT,
	{
		2, 1, 2,
		1, 2, 3, 4, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_CT_PLUS_CIAT_AI,	
	{
		2, 1, 2,
		1, 2, 3, 4, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_INTENSITY,			//60
	{
		4, 1, 1,
		1, 2, 3, 4, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_tBASE_LM:
	{
		3, 2, 3,
		1, 2, 3, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_tBASE_DETAIL_LM 20
	{
		5, 3, 4,
		0, 1, 3, 4, 5, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_tBASE_LERP_tLAYER_LM,
	{
		6, 3, 5,
		0, 1, 4, 4, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADER_otBASE:
	{
		2, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_FULLBRIGHT
	{
		1, 0, 0,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_FULLBRIGHT_CO
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		TRUE, 0, 0,
		GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_VLIGHT_BASIC, // Ca + Cv + Cm[Me] + LITD(i)*Cm[Md]
	{
		1, 0, 0,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_VLIGHT_BASIC_CO, // Ca + Cv + Cm[Me] + LITD(i)*Cm[Md]  65
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		TRUE, 0, 0,
		GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_VLIGHT_MASK_EMISSIVE, //Ca + Cv + Cm[Me]*At0 + LITD(i)*Cm[Md]
	{
		2, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_VLIGHT_MASK_EMISSIVE_CO, //Ca + Cv + Cm[Me]*At0 + LITD(i)*Cm[Md]
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		TRUE, 0, 0,
		GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_LM1,		//68
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_LM1_ZMASK,
	{
		2, 2, 2,
		0, 1, 2, 3, 4, 5, 5, 5,
		TRUE, 0, 0,
		GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_LM1_EMASK,
	{
		2, 2, 2,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_LM1_ZMASK_EMASK,
	{
		2, 1, 2,
		0, 1, 2, 3, 4, 5, 5, 5,
		TRUE, 0, 0,
		GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_LM2,
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_LM2_ZMASK,
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		TRUE, 0, 0,
		GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_LM2_EMASK,
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_LM2_ZMASK_EMASK,
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		TRUE, 0, 0,
		GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_LM3,
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_LM3_ZMASK,
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		TRUE, 0, 0,
		GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_LM3_EMASK,
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_LM3_ZMASK_EMASK,
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		TRUE, 0, 0,
		GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_LM4,
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_LM4_ZMASK,
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		TRUE, 0, 0,
		GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_LM4_EMASK,
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_LM4_ZMASK_EMASK,
	{
		1, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		TRUE, 0, 0,
		GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_VSPEC_BASIC,
	{
		1, 0, 0,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_VSPEC_MASK,
	{
		2, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_VSPEC_BASIC_CO,
	{
		1, 0, 0,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_VSPEC_BASIC_MASK_CO,
	{
		2, 1, 1,
		0, 1, 2, 3, 4, 5, 5, 5,
		FALSE, 0, 0,
		GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_SET,
		-1, FSH_IND_INVALID
	},
	//FSHADERS_VSPEC_BASIC_BUMPMAP,
	//FSHADERS_VSPEC_MASK_BUMPMAP,
};

static FShaderTexGen_t _aShaderTexGens[][8] =
{
	//FSHADERS_oBASE
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_cBASE,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_tBASE,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 1,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_etBASE,
	{
		-1, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 1,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_tBASE_ADD_SPEC,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 1,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_etBASE_ADD_SPEC,
	{
		-1, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 1,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_etBASE_ADD_bSPEC,
	{
		-1, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 1,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_ADD_BASE,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_oBASE_LERP_tLAYER,
	{
		2|_USER_DATA_FLAG, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		2, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_oBASE_LERP_vLAYER,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		2, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_oBASE_LERP_pLAYER,
	{
		2|_USER_DATA_FLAG, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		2, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_cBASE_LERP_tLAYER,
	{
		2|_USER_DATA_FLAG, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		2, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_cBASE_LERP_vLAYER,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		2, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_cBASE_LERP_pLAYER,
	{
		2|_USER_DATA_FLAG, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		2, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_tBASE_LERP_tLAYER,
	{
		2|_USER_DATA_FLAG, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		2, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 1,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_tBASE_ADD_SPEC_LERP_tLAYER,
	{
		2|_USER_DATA_FLAG, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		2, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 1,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_oBASE_ADD_rbENV,
	{
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 2,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		//3, -1, GX_TG_MTX2x4, GX_TG_NRM, GX_IDENTITY,
		2, -1, GX_TG_SRTG, GX_TG_COLOR0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		2, -1, GX_TG_SRTG, GX_TG_COLOR1, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
//		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_etBASE_ADD_rbENV,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,2,   //TexGenType, TexSrc, TexMtx
		2, -1, GX_TG_SRTG, GX_TG_COLOR0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		2, -1, GX_TG_SRTG, GX_TG_COLOR1, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},	
	//FSHADERS_oBASE_DETAIL,
	{
		2, 1|_DETAIL_SCALE, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		2, -1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		2, -1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},	
	//FSHADERS_cBASE_DETAIL,
	{
		2, 1|_DETAIL_SCALE, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		2, -1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		2, -1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_tBASE_DETAIL,
	{
		2, 1|_DETAIL_SCALE, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,1,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_etBASE_DETAIL,
	{
		-1, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		2, 1|_DETAIL_SCALE, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,1,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_tBASE_ADD_SPEC_DETAIL,
	{
		2, 1|_DETAIL_SCALE, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,1,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_etBASE_ADD_SPEC_DETAIL,
	{
		-1, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		2, 1|_DETAIL_SCALE, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 1,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_etBASE_ADD_bSPEC_DETAIL,
	{
		-1, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		2, 1|_DETAIL_SCALE, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,1,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_ADD_BASE_DETAIL,
	{
		2, 1|_DETAIL_SCALE, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_oBASE_LERP_tLAYER_DETAIL,
	{
		2|_USER_DATA_FLAG, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		4, 1|_DETAIL_SCALE, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		2, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_oBASE_LERP_vLAYER_DETAIL,
	{
		4, 1|_DETAIL_SCALE, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		2, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_oBASE_LERP_pLAYER_DETAIL,
	{
		2|_USER_DATA_FLAG, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		4, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		2, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_cBASE_LERP_tLAYER_DETAIL, - too many for now.
	{
		2|_USER_DATA_FLAG, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		4, 1|_DETAIL_SCALE, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		2, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_cBASE_LERP_vLAYER_DETAIL,
	{
		4, 1|_DETAIL_SCALE, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		2, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_cBASE_LERP_pLAYER_DETAIL
	{
		2|_USER_DATA_FLAG, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		4, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		2, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_tBASE_LERP_tLAYER_DETAIL, -- too many stages for now
	{
		2|_USER_DATA_FLAG, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		4, 1|_DETAIL_SCALE, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		2, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,1,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_tBASE_ADD_SPEC_LERP_tLAYER_DETAIL, -- too many stages for now.
	{
		2|_USER_DATA_FLAG, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		4, 1|_DETAIL_SCALE, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		2, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,1,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_oBASE_ADD_rbENV_DETAIL, -- too many stages
	{
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,2,   //TexGenType, TexSrc, TexMtx
		4, 1|_DETAIL_SCALE, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		2, -1, GX_TG_SRTG, GX_TG_COLOR0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		2, -1, GX_TG_SRTG, GX_TG_COLOR1, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_etBASE_ADD_rbENV_DETAIL, -- too many stages
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,1,   //TexGenType, TexSrc, TexMtx
		2, -1, GX_TG_SRTG, GX_TG_COLOR0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		2, -1, GX_TG_SRTG, GX_TG_COLOR1, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	#if FSH_DYNAMIC_SREFLECT
	//FSHADERS_oBASE_ADD_rbSREFLECT
	{
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,2,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		2, -1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	#endif
	//FSHADERS_tBASE_vALPHA
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,2,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_LIQUID_ENV,		//38
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,2,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_LIQUID_LAYER_ENV,	//39
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,2,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_LIQUID_TEXTURE,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,2,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_LIQUID_MOLTEN_1LAYER,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,2,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_LIQUID_MOLTEN_2LAYER,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,2,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		2, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		2|_USER_DATA_FLAG, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		4, -1, GX_TG_SRTG, GX_TG_COLOR0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		4, -1, GX_TG_SRTG, GX_TG_COLOR1, GX_IDENTITY,  0, //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		2, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		4, -1, GX_TG_SRTG, GX_TG_COLOR0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		4, -1, GX_TG_SRTG, GX_TG_COLOR1, GX_IDENTITY,  0, //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		2, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		2|_USER_DATA_FLAG, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		4, -1, GX_TG_SRTG, GX_TG_COLOR0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		4, -1, GX_TG_SRTG, GX_TG_COLOR1, GX_IDENTITY,  0, //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV_DETAIL,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		2, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		2|_USER_DATA_FLAG, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		5, 1|_DETAIL_SCALE, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		4, -1, GX_TG_SRTG, GX_TG_COLOR0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		4, -1, GX_TG_SRTG, GX_TG_COLOR1, GX_IDENTITY,  0, //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV_DETAIL,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		2, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		2|_USER_DATA_FLAG, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		5, 1|_DETAIL_SCALE, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		4, -1, GX_TG_SRTG, GX_TG_COLOR0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		4, -1, GX_TG_SRTG, GX_TG_COLOR1, GX_IDENTITY,  0, //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV_DETAIL,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		2, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		2|_USER_DATA_FLAG, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		5, 1|_DETAIL_SCALE, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		4, -1, GX_TG_SRTG, GX_TG_COLOR0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		4, -1, GX_TG_SRTG, GX_TG_COLOR1, GX_IDENTITY,  0, //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_pBASE,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 1,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_epBASE,
	{
		-1, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 1,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_pBASE_ADD_SPEC,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 1,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_epBASE_ADD_SPEC,
	{
		-1, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 1,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_epBASE_ADD_bSPEC,
	{
		-1, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 1,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_ADD_vBASE,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_LIGHT_MORPH
	{
		3, -1, GX_TG_SRTG, GX_TG_COLOR0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,  0, //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_DECAL_AI,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_DECALTEX_AI,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,  0, //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_DECALTEX_AT,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_DIFFUSETEX_AI,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_DIFFUSETEX_AT,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_DIFFUSETEX_AIAT,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_SPECULARTEX_AT,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_ADD,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_BLEND_AIPLUSAT,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_CT_PLUS_CIAT_AI,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_INTENSITY,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_tBASE_LM,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 1,//TexGenType, TexSrc, TexMtx
		0, -2, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_tBASE_DETAIL_LM,
	{
		2, 1|_DETAIL_SCALE, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,1,   //TexGenType, TexSrc, TexMtx
		0, -2, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_tBASE_LERP_tLAYER_LM,
	{
		2|_USER_DATA_FLAG, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		2, 3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 1,//TexGenType, TexSrc, TexMtx
		0, -2, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_otBASE,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 1,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_FULLBRIGHT,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_FULLBRIGHT_CO,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_VLIGHT_BASIC, // Ca + Cv + Cm[Me] + LITD(i)*Cm[Md]
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_VLIGHT_BASIC_CO, // Ca + Cv + Cm[Me] + LITD(i)*Cm[Md]
	{
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,1,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_VLIGHT_MASK_EMISSIVE, //Ca + Cv + Cm[Me]*At0 + LITD(i)*Cm[Md]
	{
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,3,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_VLIGHT_MASK_EMISSIVE_CO, //Ca + Cv + Cm[Me]*At0 + LITD(i)*Cm[Md]
	{
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,1,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_LM1,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_LM1_ZMASK,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,_ZMASK,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_LM1_EMASK,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,_EMASK,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_LM1_ZMASK_EMASK,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,1,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_LM2,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_LM2_ZMASK,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_LM2_EMASK,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_LM2_ZMASK_EMASK,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_LM3,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_LM3_ZMASK,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_LM3_EMASK,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_LM3_ZMASK_EMASK,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_LM4,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_LM4_ZMASK,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_LM4_EMASK,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_LM4_ZMASK_EMASK,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,  //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_VSPEC_BASIC,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_VSPEC_MASK
	{
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,2,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_VSPEC_BASIC_CO,
	{
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
	//FSHADERS_VSPEC_MASK_CO
	{
		0|_USER_DATA_FLAG, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,2,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,0,   //TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
		0, 1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,//TexGenType, TexSrc, TexMtx
	},
};

static FShaderStageStates_t _aShaderStageStates[][8] =
{
	//FSHADERS_oBASE
	{
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_cBASE,
	{
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_tBASE,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_etBASE,
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_KONST, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_tBASE_ADD_SPEC,
	{
		{
		GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
		GX_TEV_KCSEL_K0, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
		GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
		GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
		,
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_etBASE_ADD_SPEC,
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_KONST, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
		,
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_etBASE_ADD_bSPEC -- fine on GC, problem on XBox, will need pixel shader
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_KONST, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
		,
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_ADD_BASE,
	{
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
//			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
		,
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_oBASE_LERP_tLAYER,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_K3_A,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_TEXC, GX_CC_ZERO, GX_CC_APREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP2, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_oBASE_LERP_vLAYER,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_K3_A,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_RASA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_CPREV, GX_CC_TEXC, GX_CC_APREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
		,
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_oBASE_LERP_pLAYER, --come back to this
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_RASA, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_K3_A,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_APREV, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP2, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_CPREV, GX_CC_TEXC, GX_CC_APREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_cBASE_LERP_tLAYER,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_K3_A,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_TEXC, GX_CC_ZERO, GX_CC_APREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP2, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_cBASE_LERP_vLAYER,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_K3_A,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_RASA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_CPREV, GX_CC_TEXC, GX_CC_APREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
		,
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_cBASE_LERP_pLAYER, --come back to this
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_RASA, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_K3_A,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_APREV, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP2, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_CPREV, GX_CC_TEXC, GX_CC_APREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_tBASE_LERP_tLAYER,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_K3_A,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_TEXC, GX_CC_ZERO, GX_CC_APREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP3, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_APREV, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_tBASE_ADD_SPEC_LERP_tLAYER,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_K3_A,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_TEXC, GX_CC_ZERO, GX_CC_APREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP3, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_APREV, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_oBASE_ADD_rbENV,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR1A1, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			TRUE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_ONE, GX_CC_RASC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_CPREV, GX_CC_C0, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_C0, GX_CC_C1, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_etBASE_ADD_rbENV,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K2, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR1A1, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			//GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_APREV, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_C0, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_C0, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_oBASE_DETAIL,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ONE, GX_CC_HALF, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_cBASE_DETAIL,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ONE, GX_CC_HALF, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_tBASE_DETAIL,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ONE, GX_CC_HALF, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_CPREV, GX_CC_RASC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_APREV, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_etBASE_DETAIL,
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_KONST, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ONE, GX_CC_HALF, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_C0, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_APREV, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_tBASE_ADD_SPEC_DETAIL,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ONE, GX_CC_HALF, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_CPREV, GX_CC_RASC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_APREV, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_etBASE_ADD_SPEC_DETAIL,
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_KONST, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ONE, GX_CC_HALF, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_C0, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_APREV, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_etBASE_ADD_bSPEC_DETAIL
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_KONST, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ONE, GX_CC_HALF, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_C0, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_APREV, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_ADD_BASE_DETAIL,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ONE, GX_CC_HALF, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_oBASE_LERP_tLAYER_DETAIL,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_HALF, GX_CC_ZERO, GX_CC_TEXA, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_TEXC, GX_CC_ZERO, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP3, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_APREV, GX_CC_TEXC, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_oBASE_LERP_vLAYER_DETAIL,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ONE, GX_CC_HALF, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_CPREV, GX_CC_TEXC, GX_CC_APREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_oBASE_LERP_pLAYER_DETAIL,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_RASA, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ONE, GX_CC_HALF, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_CPREV, GX_CC_ZERO, GX_CC_APREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP3, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_cBASE_LERP_tLAYER_DETAIL,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_HALF, GX_CC_ZERO, GX_CC_TEXA, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_TEXC, GX_CC_ZERO, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP3, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_APREV, GX_CC_TEXC, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_cBASE_LERP_vLAYER_DETAIL,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ONE, GX_CC_HALF, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_APREV, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_cBASE_LERP_pLAYER_DETAIL,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_RASA, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ONE, GX_CC_HALF, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_CPREV, GX_CC_ZERO, GX_CC_APREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP3, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_tBASE_LERP_tLAYER_DETAIL,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ONE, GX_CC_HALF, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD3, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD3, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_CPREV, GX_CC_ZERO, GX_CC_APREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP3, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD2, GX_TEXMAP4, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_tBASE_ADD_SPEC_LERP_tLAYER_DETAIL,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ONE, GX_CC_HALF, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD3, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD3, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_CPREV, GX_CC_ZERO, GX_CC_APREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP3, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD2, GX_TEXMAP4, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_oBASE_ADD_rbENV_DETAIL,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ONE, GX_CC_HALF, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR1A1, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD3, GX_TEXMAP3, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_etBASE_ADD_rbENV_DETAIL,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K2, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR1A1, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_APREV, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
		
	},
	#if FSH_DYNAMIC_SREFLECT
	//FSHADERS_oBASE_ADD_rbSREFLECT,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD4, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			//GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			TRUE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR1A1, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	#endif
	//FSHADERS_tBASE_vALPHA,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_KONST, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_RASA, GX_CA_KONST, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD4, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			//GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			TRUE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR1A1, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_LIQUID_ENV,		//38
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_KONST, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_RASA, GX_CA_KONST, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD4, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			//GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			TRUE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR1A1, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_LIQUID_LAYER_ENV,	//39
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_KONST, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_RASA, GX_CA_KONST, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD4, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			//GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			TRUE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR1A1, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_LIQUID_TEXTURE,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_KONST, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_RASA, GX_CA_KONST, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD4, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			//GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			TRUE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR1A1, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_LIQUID_MOLTEN_1LAYER,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_KONST, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_RASA, GX_CA_KONST, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD4, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			//GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			TRUE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR1A1, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_LIQUID_MOLTEN_2LAYER,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_KONST, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_RASA, GX_CA_KONST, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD4, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			//GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			TRUE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR1A1, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP3, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_TEXC, GX_CC_ZERO, GX_CC_APREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},	
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP4, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD3, GX_TEXMAP4, GX_COLOR1A1, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_TEXC, GX_CC_ZERO, GX_CC_RASA, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},	
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP3, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD3, GX_TEXMAP3, GX_COLOR1A1, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP3, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP3, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_RASA, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_TEXC, GX_CC_ZERO, GX_CC_APREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},	
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP4, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD3, GX_TEXMAP4, GX_COLOR1A1, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV_DETAIL,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP3, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_TEXC, GX_CC_ZERO, GX_CC_APREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},	
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP4, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ONE, GX_CC_HALF, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD3, GX_TEXMAP5, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD4, GX_TEXMAP5, GX_COLOR1A1, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_C0, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
	},
	//FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV_DETAIL,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP3, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_TEXC, GX_CC_ZERO, GX_CC_APREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},	
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP4, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_HALF, GX_CC_ONE, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD3, GX_TEXMAP5, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD4, GX_TEXMAP5, GX_COLOR1A1, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_C0, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV_DETAIL,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP3, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_RASA, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_TEXC, GX_CC_ZERO, GX_CC_APREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},	
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP4, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_HALF, GX_CC_ONE, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD3, GX_TEXMAP5, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD4, GX_TEXMAP5, GX_COLOR1A1, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_C0, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_pBASE,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_RASA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_APREV, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_epBASE,
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_KONST, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_RASA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_APREV, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_pBASE_ADD_SPEC,
	{
		{
		GX_TEV_KCSEL_1, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_RASA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
		GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_APREV, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
		GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
		GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
		,
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_epBASE_ADD_SPEC,
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_KONST, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_APREV, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_APREV, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
		,
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_epBASE_ADD_bSPEC -- fine on GC, problem on XBox, will need pixel shader
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_KONST, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_APREV, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_APREV, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
		,
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_ADD_vBASE,
	{
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 		//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASA, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
		,
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_LIGHT_MORPH,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},	
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_DECAL_AI,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},	
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_DECALTEX_AI,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},	
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_DECALTEX_AT,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},	
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_DIFFUSETEX_AI,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},	
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_DIFFUSETEX_AT,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},	
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_DIFFUSETEX_AIAT,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_APREV, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},	
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_SPECULARTEX_AT,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_RASA, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},	
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_ADD,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ONE, GX_CC_TEXC, GX_CC_RASC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ONE, GX_CA_TEXA, GX_CA_RASA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},	
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_BLEND_AIPLUSAT,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_TEXC, GX_CC_RASC, GX_CC_RASA, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ONE, GX_CA_TEXA, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},	
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_CT_PLUS_CIAT_AI,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_TEXC, GX_CC_RASC, GX_CC_RASA, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXA, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},	
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_INTENSITY
	{
		{
			GX_TEV_KCSEL_K0_R, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP1,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0_G, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},	
		{
			GX_TEV_KCSEL_K0_B, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP3,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_tBASE_LM,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_tBASE_DETAIL_LM,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP3, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ZERO, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ONE, GX_CC_HALF, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_CPREV, GX_CC_C0, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_APREV, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_tBASE_LERP_tLAYER_LM,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD2, GX_TEXMAP4, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ZERO, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_K3_A,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_TEXC, GX_CC_ZERO, GX_CC_APREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP2, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP3, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_C0, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_KONST, GX_CA_APREV, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_otBASE,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_K0_A,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
		,
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_FULLBRIGHT
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,			
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_FULLBRIGHT_CO
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//BASIC lighting
	//Ca + Cm[Me] -> Konst (K1)
	//Amb source = vertex (Cv) -- Channel 0
	//Diffuse source = register (Cm[Md]) -- Channel 0
	//Ca + Cv + Cm[Me] + LITD(i)*Cm[Md]
	//FSHADERS_VLIGHT_BASIC, // Ca + Cv + Cm[Me] + LITD(i)*Cm[Md]
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_C0, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_VLIGHT_BASIC_CO, // Ca + Cv + Cm[Me] + LITD(i)*Cm[Md]
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_KONST, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_VLIGHT_MASK_EMISSIVE, //Ca + Cv + Cm[Me]*At0 + LITD(i)*Cm[Md]
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXA, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_VLIGHT_MASK_EMISSIVE_CO, //Ca + Cv + Cm[Me]*At0 + LITD(i)*Cm[Md]
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_KONST, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_LM1,
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_LM1_ZMASK,
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL,		//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_LM1_EMASK,
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXA, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_LM1_ZMASK_EMASK,
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL,		//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_LM2,
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_LM2_ZMASK,
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_LM2_EMASK,
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_LM2_ZMASK_EMASK,
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_LM3,
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_LM3_ZMASK,
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_LM3_EMASK,
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_LM3_ZMASK_EMASK,
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_LM4,
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_LM4_ZMASK,
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_LM4_EMASK,
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_LM4_ZMASK_EMASK,
	{
		{
			GX_TEV_KCSEL_K1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_TEXC, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_VSPEC_BASIC,
	{
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR1A1, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASA, GX_CC_KONST, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_VSPEC_MASK,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR1A1, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASA, GX_CC_TEXA, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_CPREV, GX_CC_KONST, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_VSPEC_BASIC_CO,
	{
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR1A1, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASA, GX_CC_KONST, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
	//FSHADERS_VSPEC_BASIC_MASK_CO,
	{
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR1A1, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASA, GX_CC_TEXA, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP2,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_K0, GX_TEV_KASEL_1,
			GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_CPREV, GX_CC_KONST, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //alpha: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		},
		{
			GX_TEV_KCSEL_1, GX_TEV_KASEL_1,
			GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0, 			//TexCoord, TexMap, Color Channel
			GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO, //color: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, //alpha: A, B, C, D
			GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV, //color: Op, Bias, Clamp, Scale
			GX_TEV_SWAP0, GX_TEV_SWAP0,						//swap table indices.
			FALSE											//envmapped texgen
		}
	},
};

FShaders_Types_e _aDetailShaderRemap[] =
{
	FSHADERS_oBASE,							//FSHADERS_oBASE_DETAIL,
	FSHADERS_cBASE,							//FSHADER_cBASE_DETAIL
	FSHADERS_tBASE,							//FSHADER_tBASE_DETAIL
	FSHADERS_etBASE,						//FSHADERS_etBASE_DETAIL
	FSHADERS_tBASE_ADD_SPEC,				//FSHADERS_tBASE_ADD_SPEC_DETAIL
	FSHADERS_etBASE_ADD_SPEC,				//FSHADERS_etBASE_ADD_SPEC_DETAIL
	FSHADERS_etBASE_ADD_bSPEC,				//FSHADERS_etBASE_ADD_bSPEC_DETAIL
	FSHADERS_ADD_BASE,						//FSHADERS_ADD_BASE_DETAIL
	FSHADERS_oBASE_LERP_tLAYER,				//FSHADERS_oBASE_LERP_tLAYER_DETAIL
	FSHADERS_oBASE_LERP_vLAYER,				//FSHADERS_oBASE_LERP_vLAYER_DETAIL
	FSHADERS_oBASE_LERP_pLAYER,				//FSHADERS_oBASE_LERP_pLAYER_DETAIL
	FSHADERS_cBASE_LERP_tLAYER,				//FSHADERS_cBASE_LERP_tLAYER_DETAIL
	FSHADERS_cBASE_LERP_vLAYER,				//FSHADERS_cBASE_LERP_vLAYER_DETAIL
	FSHADERS_cBASE_LERP_pLAYER,				//FSHADERS_cBASE_LERP_pLAYER_DETAIL
	FSHADERS_tBASE_LERP_tLAYER,				//FSHADERS_tBASE_LERP_tLAYER_DETAIL
	FSHADERS_tBASE_ADD_SPEC_LERP_tLAYER,	//FSHADERS_tBASE_ADD_SPEC_LERP_tLAYER_DETAIL
	FSHADERS_oBASE_ADD_rbENV, 				//FSHADERS_oBASE_ADD_rbENV_DETAIL
	FSHADERS_etBASE_ADD_rbENV, 				//FSHADERS_etBASE_ADD_rbENV_DETAIL
	FSHADERS_oBASE_ADD_rbSREFLECT,
	FSHADERS_tBASE_vALPHA,
	FSHADERS_LIQUID_ENV,
	FSHADERS_LIQUID_LAYER_ENV,
	FSHADERS_LIQUID_TEXTURE,
	FSHADERS_LIQUID_MOLTEN_1LAYER,
	FSHADERS_LIQUID_MOLTEN_2LAYER,
	FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV,
	FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV,
	FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV,
	FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV,	//FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV_DETAIL,
	FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV,	//FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV_DETAIL,
	FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV	//FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV_DETAIL,
};

FShaders_Types_e _aReflectionShaderRemap[] =
{
	FSHADERS_oBASE,						//FSHADERS_oBASE_ADD_rbENV,
	FSHADERS_etBASE,					//FSHADERS_etBASE_ADD_rbENV,
	FSHADERS_oBASE_DETAIL,
	FSHADERS_cBASE_DETAIL,
	FSHADERS_tBASE_DETAIL,
	FSHADERS_etBASE_DETAIL,
	FSHADERS_tBASE_ADD_SPEC_DETAIL,
	FSHADERS_etBASE_ADD_SPEC_DETAIL,
	FSHADERS_etBASE_ADD_bSPEC_DETAIL,
	FSHADERS_ADD_BASE_DETAIL,
	FSHADERS_oBASE_LERP_tLAYER_DETAIL,
	FSHADERS_oBASE_LERP_vLAYER_DETAIL,
	FSHADERS_oBASE_LERP_pLAYER_DETAIL,
	FSHADERS_cBASE_LERP_tLAYER_DETAIL,
	FSHADERS_cBASE_LERP_vLAYER_DETAIL,
	FSHADERS_cBASE_LERP_pLAYER_DETAIL,
	FSHADERS_tBASE_LERP_tLAYER_DETAIL,
	FSHADERS_tBASE_ADD_SPEC_LERP_tLAYER_DETAIL,
	FSHADERS_oBASE_DETAIL,				//FSHADERS_oBASE_ADD_rbENV_DETAIL
	FSHADERS_etBASE_DETAIL,				//FSHADERS_etBASE_ADD_rbENV_DETAIL 35
	FSHADERS_oBASE,						//FSHADERS_oBASE_ADD_rbSREFLECT
	FSHADERS_tBASE_vALPHA,
	FSHADERS_LIQUID_ENV,
	FSHADERS_LIQUID_LAYER_ENV,
	FSHADERS_LIQUID_TEXTURE,
	FSHADERS_LIQUID_MOLTEN_1LAYER,
	FSHADERS_LIQUID_MOLTEN_2LAYER,
	FSHADERS_oBASE_LERP_tLAYER,			//FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV,
	FSHADERS_oBASE_LERP_vLAYER,			//FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV,
	FSHADERS_oBASE_LERP_pLAYER,			//FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV,
	FSHADERS_oBASE_LERP_tLAYER_DETAIL,	//FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV_DETAIL,
	FSHADERS_oBASE_LERP_vLAYER_DETAIL,	//FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV_DETAIL,
	FSHADERS_oBASE_LERP_pLAYER_DETAIL	//FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV_DETAIL,
};

static u32 _anSTypeAlphaEnable[] =
{
	FALSE,
	TRUE,
	TRUE,
	TRUE
};

static GXTevStageID _anTevStage[] =
{
	GX_TEVSTAGE0,
	GX_TEVSTAGE1,
	GX_TEVSTAGE2,
	GX_TEVSTAGE3,
	GX_TEVSTAGE4,
	GX_TEVSTAGE5,
	GX_TEVSTAGE6,
	GX_TEVSTAGE7,
	GX_TEVSTAGE8,
	GX_TEVSTAGE9,
	GX_TEVSTAGE10,
	GX_TEVSTAGE11,
	GX_TEVSTAGE12,
	GX_TEVSTAGE13,
	GX_TEVSTAGE14,
	GX_TEVSTAGE15
};


#endif