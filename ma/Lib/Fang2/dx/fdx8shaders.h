//////////////////////////////////////////////////////////////////////////////////////
// fdx8shaders.h - Fang DX shader programs.
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
// 11/02/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#if FANG_PLATFORM_WIN
//VERTEX SHADER INCLUDES
//---------SURFACE SHADERS--------------//
#include "CompiledVShaderfdx8PassThru_1tc.h"
#include "CompiledVShaderfdx8PassThru_2tc.h"
#include "CompiledVShaderfdx8PassThru_2tc_Env.h"
#include "CompiledVShaderfdx8PassThru_1tc_blend.h"
#include "CompiledVShaderfdx8PassThru_2tc_blend.h"
#include "CompiledVShaderfdx8oBase_Add_rbENV.h"
#include "CompiledVShaderfdx8oBase_Add_rbENV_EMBM.h"
#include "CompiledVShaderfdx8oBase_Add_rbSReflect.h"
#include "CompiledVShaderfdx8oBase_Add_rbSReflect_EMBM.h"
#include "CompiledVShaderfdx8oBase_Add_rbENV_blend.h"
#include "CompiledVShaderfdx8Detail.h"
#include "CompiledVShaderfdx8Detail_2tc.h"
#include "CompiledVShaderfdx8oBase_Add_rbENV_Detail.h"
//#include "CompiledVShaderfdx8oBase_Add_rbENV_LM.h"
#include "CompiledVShaderfdx8oBase_Add_rbENV_P1D1.h"
#include "CompiledVShaderfdx8oBase_Add_rbENV_P2D1.h"
#include "CompiledVShaderfdx8oBase_Add_rbENV_P4D1.h"
//---------DIFFUSE SHADERS--------------//
#include "CompiledVShaderfdx8VtxColor.h"
#include "CompiledVShaderfdx8VtxColor_blend.h"
#include "CompiledVShaderfdx8Point1Dir1_Diffuse.h"
#include "CompiledVShaderfdx8Point1Dir1_Diffuse_Blend.h"
#include "CompiledVShaderfdx8Point2Dir1_Diffuse.h"
#include "CompiledVShaderfdx8Point2Dir1_Diffuse_Blend.h"
#include "CompiledVShaderfdx8Point4Dir1_Diffuse.h"
#include "CompiledVShaderfdx8Point4Dir1_Diffuse_Blend.h"
#include "CompiledVShaderfdx8Point6Dir1_Diffuse.h"
#include "CompiledVShaderfdx8Point6Dir1_Diffuse_Blend.h"
#include "CompiledVShaderfdx8Point7Dir1_Diffuse.h"
#include "CompiledVShaderfdx8Point7Dir1_Diffuse_Blend.h"
#include "CompiledVShaderfdx8Point8_Diffuse.h"
#include "CompiledVShaderfdx8PerPixelPoint1_Diffuse.h"
#include "CompiledVShaderfdx8PerPixelPoint2_Diffuse.h"
#include "CompiledVShaderfdx8PerPixelSpot1_Diffuse.h"
#include "CompiledVShaderfdx8PerPixelSpot2_Diffuse.h"
#include "CompiledPShaderfdx8PerPixelSpot1_Diffuse_Shadow.h"
#include "CompiledVShaderfdx8PerPixelSpot1_Diffuse_Bump.h"
#include "CompiledVShaderfdx8PerPixelPoint1_Diffuse_Bump.h"

#include "CompiledVShaderfdx8VtxLMap1_tc1.h"
#include "CompiledVShaderfdx8VtxLMap2_tc1.h"
#include "CompiledVShaderfdx8VtxLMap3_tc1.h"
#include "CompiledVShaderfdx8VtxLMap4_tc1.h"
#include "CompiledVShaderfdx8VtxLMap1_tc1_Mask1.h"
#include "CompiledVShaderfdx8VtxLMap2_tc1_Mask1.h"
#include "CompiledVShaderfdx8VtxLMap3_tc1_Mask1.h"
#include "CompiledVShaderfdx8VtxLMap1_tc1_Mask2.h"
#include "CompiledVShaderfdx8VtxLMap2_tc1_Mask2.h"
//---------SPECULAR SHADERS--------------//
#include "CompiledVShaderfdx8PerPixelDirectional1_Specular_Bump.h"
#include "CompiledVShaderfdx8Directional1_Specular.h"
#include "CompiledVShaderfdx8PerPixelSpot1_Spec_Bump.h"
//---------Special FX Shaders-----------//
#include "CompiledVShaderfdx8PlanarRefl.h"
#include "CompiledVShaderfdx8PlanarReflMesh.h"
//
//PIXEL SHADER INCLUDES
#include "CompiledPShaderfdx8PassThru.h"
#include "CompiledPShaderfdx8PassThru_VTAlpha.h"
#include "CompiledPShaderfdx8PassThru_VAMul.h"
#include "CompiledPShaderfdx8PassThru_VAlpha.h"
#include "CompiledPShaderfdx8PassThru_LM1.h"
#include "CompiledPShaderfdx8PassThru_LM2.h"
#include "CompiledPShaderfdx8PassThru_LM3.h"
#include "CompiledPShaderfdx8PassThru_AlphaMask.h"
#include "CompiledPShaderfdx8PassThru_SAlpha.h"
#include "CompiledPShaderfdx8oBase_Lerp_tLayer.h"
#include "CompiledPShaderfdx8oBase_Lerp_tLayer_AlphaMask.h"
#include "CompiledPShaderfdx8oBase_Lerp_vLayer.h"
#include "CompiledPShaderfdx8oBase_Lerp_pLayer.h"
#include "CompiledPShaderfdx8oBase_Add_rbENV.h"
#include "CompiledPShaderfdx8oBase_Add_rbENV_Lit.h"
#include "CompiledPShaderfdx8oBase_Add_rbENV_EMBM.h"
#include "CompiledPShaderfdx8oBase_Add_rbENV_AMask.h"
#include "CompiledPShaderfdx8oBase_Add_rbENV_SMask.h"
#include "CompiledPShaderfdx8oBase_Add_rbENV_SAMask.h"
#include "CompiledPShaderfdx8oBase_Lerp_tLayer_ADD_rbENV.h"
#include "CompiledPShaderfdx8oBase_Lerp_vLayer_ADD_rbENV.h"
#include "CompiledPShaderfdx8oBase_Lerp_pLayer_ADD_rbENV.h"
#include "CompiledPShaderfdx8Detail.h"
#include "CompiledPShaderfdx8oBase_Lerp_tLayer_Detail.h"
#include "CompiledPShaderfdx8oBase_Lerp_vLayer_Detail.h"
#include "CompiledPShaderfdx8oBase_Lerp_pLayer_Detail.h"
#include "CompiledPShaderfdx8oBase_Add_rbENV_Detail.h"
#include "CompiledPShaderfdx8oBase_Lerp_tLayer_ADD_rbENV_Detail.h"
#include "CompiledPShaderfdx8oBase_Lerp_vLayer_ADD_rbENV_Detail.h"
#include "CompiledPShaderfdx8oBase_Lerp_pLayer_ADD_rbENV_Detail.h"

#include "CompiledPShaderfdx8Color.h"
#include "CompiledPShaderfdx8ColorMask.h"
#include "CompiledPShaderfdx8ColorAlphaMask.h"
#include "CompiledPShaderfdx8ColorEMask.h"
#include "CompiledPShaderfdx8ColorEMask_Inv.h"
#include "CompiledPShaderfdx8ColorEmissive.h"
#include "CompiledPShaderfdx8PerPixelPoint1_Diffuse.h"
#include "CompiledPShaderfdx8PerPixelPoint2_Diffuse.h"
#include "CompiledPShaderfdx8PerPixelSpot1_Diffuse.h"
#include "CompiledPShaderfdx8PerPixelSpot2_Diffuse.h"
#include "CompiledPShaderfdx8PerPixelSpot1_Diffuse_Bump.h"
#include "CompiledPShaderfdx8PerPixelSpot1_Spec_Bump.h"
#include "CompiledPShaderfdx8PerPixelPoint1_Diffuse_Bump.h"
#include "CompiledPShaderfdx8PerPixelDirectional1_Specular_Bump.h"

#include "CompiledPShaderfdx8LMap1.h"
#include "CompiledPShaderfdx8LMap2.h"
#include "CompiledPShaderfdx8LMap3.h"
#include "CompiledPShaderfdx8LMap4.h"
#include "CompiledPShaderfdx8LMap1_ZMask_EMask.h"
#include "CompiledPShaderfdx8LMap2_ZMask_EMask.h"
#include "CompiledPShaderfdx8LMap1_ZMask.h"
#include "CompiledPShaderfdx8LMap2_ZMask.h"
#include "CompiledPShaderfdx8LMap3_ZMask.h"
#include "CompiledPShaderfdx8LMap1_EMask.h"
#include "CompiledPShaderfdx8LMap1_InvEMask.h"
#include "CompiledPShaderfdx8LMap2_EMask.h"
#include "CompiledPShaderfdx8Molten1LayerEMBM.h"
#include "CompiledPShaderfdx8Molten2LayerEMBM.h"
#include "CompiledPShaderfdx8Molten2LayerEMBMGlow.h"

//Special FX pixel shaders
#include "CompiledPShaderfdx8LMap3_EMask.h"
#else
//VERTEX SHADER INCLUDES
//---------SURFACE SHADERS--------------//
#include "CompiledVShaderfxbPassThru_1tc.h"
#include "CompiledVShaderfxbPassThru_2tc.h"
#include "CompiledVShaderfxbPassThru_2tc_Env.h"
#include "CompiledVShaderfxbPassThru_1tc_blend.h"
#include "CompiledVShaderfxbPassThru_2tc_blend.h"
#include "CompiledVShaderfxboBase_Add_rbENV.h"
#include "CompiledVShaderfxboBase_Add_rbENV_LM.h"
#include "CompiledVShaderfxboBase_Add_rbENV_P1D1.h"
#include "CompiledVShaderfxboBase_Add_rbENV_P2D1.h"
#include "CompiledVShaderfxboBase_Add_rbENV_P4D1.h"
#include "CompiledVShaderfxboBase_Add_rbSReflect.h"
#include "CompiledVShaderfxboBase_Add_rbENV_blend.h"
#include "CompiledVShaderfxbDetail.h"
#include "CompiledVShaderfxbDetail_2tc.h"
#include "CompiledVShaderfxboBase_Add_rbENV_Detail.h"
//---------DIFFUSE SHADERS--------------//
#include "CompiledVShaderfxbVtxColor.h"
#include "CompiledVShaderfxbVtxColor_blend.h"
#include "CompiledVShaderfxbPoint1Dir1_Diffuse.h"
#include "CompiledVShaderfxbPoint1Dir1_Diffuse_Blend.h"
#include "CompiledVShaderfxbPoint2Dir1_Diffuse.h"
#include "CompiledVShaderfxbPoint2Dir1_Diffuse_Blend.h"
#include "CompiledVShaderfxbPoint4Dir1_Diffuse.h"
#include "CompiledVShaderfxbPoint4Dir1_Diffuse_Blend.h"
#include "CompiledVShaderfxbPoint6Dir1_Diffuse.h"
#include "CompiledVShaderfxbPoint6Dir1_Diffuse_Blend.h"
#include "CompiledVShaderfxbPoint7Dir1_Diffuse.h"
#include "CompiledVShaderfxbPoint7Dir1_Diffuse_Blend.h"
#include "CompiledVShaderfxbPoint8_Diffuse.h"
#include "CompiledVShaderfxbPerPixelPoint1_Diffuse.h"
#include "CompiledVShaderfxbPerPixelPoint2_Diffuse.h"
#include "CompiledVShaderfxbPerPixelSpot1_Diffuse.h"
#include "CompiledVShaderfxbPerPixelSpot2_Diffuse.h"
#include "CompiledVShaderfxbPerPixelSpot1_Diffuse_Bump.h"
#include "CompiledVShaderfxbPerPixelPoint1_Diffuse_Bump.h"

#include "CompiledVShaderfxbVtxLMap1_tc1.h"
#include "CompiledVShaderfxbVtxLMap2_tc1.h"
#include "CompiledVShaderfxbVtxLMap3_tc1.h"
#include "CompiledVShaderfxbVtxLMap4_tc1.h"
#include "CompiledVShaderfxbVtxLMap1_tc1_Mask1.h"
#include "CompiledVShaderfxbVtxLMap2_tc1_Mask1.h"
#include "CompiledVShaderfxbVtxLMap3_tc1_Mask1.h"
#include "CompiledVShaderfxbVtxLMap1_tc1_Mask2.h"
#include "CompiledVShaderfxbVtxLMap2_tc1_Mask2.h"
//---------SPECULAR SHADERS--------------//
#include "CompiledVShaderfxbPerPixelDirectional1_Specular_Bump.h"
#include "CompiledVShaderfxbDirectional1_Specular.h"
//---------Special FX Shaders-----------//
#include "CompiledVShaderfxbPlanarRefl.h"
#include "CompiledVShaderfxbPlanarReflMesh.h"
#include "CompiledVShaderfxbPlanarRefr.h"
//PIXEL SHADER INCLUDES
#include "CompiledPShaderfxbPassThru.h"
#include "CompiledPShaderfxbPassThru_VTAlpha.h"
#include "CompiledPShaderfxbPassThru_VAMul.h"
#include "CompiledPShaderfxbPassThru_VAlpha.h"
#include "CompiledPShaderfxbPassThru_LM1.h"
#include "CompiledPShaderfxbPassThru_LM2.h"
#include "CompiledPShaderfxbPassThru_LM3.h"
#include "CompiledPShaderfxbPassThru_AlphaMask.h"
#include "CompiledPShaderfxbPassThru_SAlpha.h"
#include "CompiledPShaderfxboBase_Lerp_tLayer.h"
#include "CompiledPShaderfxboBase_Lerp_tLayer_AlphaMask.h"
#include "CompiledPShaderfxboBase_Lerp_vLayer.h"
#include "CompiledPShaderfxboBase_Lerp_pLayer.h"
#include "CompiledPShaderfxboBase_Add_rbENV.h"
#include "CompiledPShaderfxboBase_Add_rbENV_LIT.h"
#include "CompiledPShaderfxboBase_Add_rbENV_LIT_LM.h"
#include "CompiledPShaderfxboBase_Add_rbENV_AMask.h"
#include "CompiledPShaderfxboBase_Add_rbENV_SMask.h"
#include "CompiledPShaderfxboBase_Add_rbENV_SAMask.h"
#include "CompiledPShaderfxboBase_Lerp_tLayer_ADD_rbENV.h"
#include "CompiledPShaderfxboBase_Lerp_vLayer_ADD_rbENV.h"
#include "CompiledPShaderfxboBase_Lerp_pLayer_ADD_rbENV.h"
#include "CompiledPShaderfxbDetail.h"
#include "CompiledPShaderfxboBase_Lerp_tLayer_Detail.h"
#include "CompiledPShaderfxboBase_Lerp_vLayer_Detail.h"
#include "CompiledPShaderfxboBase_Lerp_pLayer_Detail.h"
#include "CompiledPShaderfxboBase_Add_rbENV_Detail.h"
#include "CompiledPShaderfxboBase_Lerp_tLayer_ADD_rbENV_Detail.h"
#include "CompiledPShaderfxboBase_Lerp_vLayer_ADD_rbENV_Detail.h"
#include "CompiledPShaderfxboBase_Lerp_pLayer_ADD_rbENV_Detail.h"

#include "CompiledPShaderfxbColor.h"
#include "CompiledPShaderfxbColorMask.h"
#include "CompiledPShaderfxbColorAlphaMask.h"
#include "CompiledPShaderfxbColorEMask.h"
#include "CompiledPShaderfxbColorEMask_Inv.h"
#include "CompiledPShaderfxbColorEmissive.h"
#include "CompiledPShaderfxbPerPixelPoint1_Diffuse.h"
#include "CompiledPShaderfxbPerPixelPoint2_Diffuse.h"
#include "CompiledPShaderfxbPerPixelSpot1_Diffuse.h"
#include "CompiledPShaderfxbPerPixelSpot2_Diffuse.h"
#include "CompiledPShaderfxbPerPixelSpot1_Diffuse_Bump.h"
#include "CompiledPShaderfxbPerPixelPoint1_Diffuse_Bump.h"
#include "CompiledPShaderfxbPerPixelDirectional1_Specular_Bump.h"

#include "CompiledPShaderfxbLMap1.h"
#include "CompiledPShaderfxbLMap2.h"
#include "CompiledPShaderfxbLMap3.h"
#include "CompiledPShaderfxbLMap4.h"
#include "CompiledPShaderfxbLMap1_ZMask_EMask.h"
#include "CompiledPShaderfxbLMap2_ZMask_EMask.h"
#include "CompiledPShaderfxbLMap1_ZMask.h"
#include "CompiledPShaderfxbLMap2_ZMask.h"
#include "CompiledPShaderfxbLMap3_ZMask.h"
#include "CompiledPShaderfxbLMap1_EMask.h"
#include "CompiledPShaderfxbLMap1_InvEMask.h"
#include "CompiledPShaderfxbLMap2_EMask.h"
#include "CompiledPShaderfxbLMap3_EMask.h"

//Special FX Shaders
#include "CompiledPShaderfxbMolten1LayerEMBM.h"
#include "CompiledPShaderfxbMolten2LayerEMBM.h"
#include "CompiledPShaderfxbMolten2LayerEMBMGlow.h"
#include "CompiledPShaderfxbCubeReflect.h"
#include "CompiledPShaderfxbCubeReflect_VtxAlpha.h"
#endif

typedef enum
{
	VSHADER_BASE_NONE=0,
	//------------SURFACE SHADERS-------------//
	VSHADER_BASE_PASSTHRU,
	VSHADER_BASE_PASSTHRU_ENV,
	VSHADER_BASE_oBASE_ADD_rbENV,
	VSHADER_BASE_oBASE_ADD_rbENV_EMBM,
	VSHADER_BASE_ENV_POINT1_DIR1,
	VSHADER_BASE_ENV_POINT2_DIR1,
	VSHADER_BASE_ENV_POINT4_DIR1,
    //------------DIFFUSE SHADERS-------------//
	VSHADER_BASE_VCOLOR,
	VSHADER_BASE_POINT1_DIR1,
	VSHADER_BASE_POINT2_DIR1,
	VSHADER_BASE_POINT4_DIR1,
	VSHADER_BASE_POINT6_DIR1,
	VSHADER_BASE_POINT7_DIR1,
	VSHADER_BASE_POINT8,
	VSHADER_BASE_PERPIXELPOINT1,
	VSHADER_BASE_PERPIXELPOINT2,
	VSHADER_BASE_PERPIXELSPOT1,
	VSHADER_BASE_PERPIXELSPOT2,
	VSHADER_BASE_PERPIXELSPOT1_BUMP,
	VSHADER_BASE_PERPIXELSPOT1_SPECBUMP,
	VSHADER_BASE_PERPIXELPOINT1_BUMP,
	VSHADER_BASE_PERPIXELDIRECTIONAL1_SPECBUMP,
	VSHADER_BASE_DIRECTIONAL1_SPECULAR,

	VSHADER_BASE_DETAIL,
	VSHADER_BASE_oBASE_ADD_rbENV_DETAIL,
	VSHADER_BASE_oBASE_ADD_rbSREFLECT,
	VSHADER_BASE_oBASE_ADD_rbSREFLECT_EMBM,

	VSHADER_BASE_LM4,
	VSHADER_BASE_ZMASK_EMASK_LM2,
	VSHADER_BASE_ZMASK_LM3,
	VSHADER_BASE_EMASK_LM3,

	VSHADER_BASE_REFLECT,

	VSHADER_BASE_NUM
} _VERTEX_SHADER_BASE_e;

typedef enum 
{
	PSHADER_NONE=0,
	PSHADER_PASSTHRU,
	PSHADER_PASSTHRU_VALPHA,
	PSHADER_PASSTHRU_VTALPHA,
	PSHADER_PASSTHRU_VAMUL,
	PSHADER_PASSTHRU_LM1, //5
	PSHADER_PASSTHRU_LM2,
	PSHADER_PASSTHRU_LM3,
	PSHADER_PASSTHRU_ALPHAMASK,
	PSHADER_PASSTHRU_SALPHA,
	PSHADER_oBASE_LERP_tLAYER, //10
	PSHADER_oBASE_LERP_tLAYER_ALPHAMASK,
	PSHADER_oBASE_LERP_vLAYER,
	PSHADER_oBASE_LERP_pLAYER,
	PSHADER_oBASE_ADD_rbENV,
	PSHADER_oBASE_ADD_rbENV_LIT, //15
	PSHADER_oBASE_ADD_rbENV_LIT_LM,
	PSHADER_oBASE_ADD_rbENV_EMBM,
	PSHADER_oBASE_ADD_rbENV_AMASK,
	PSHADER_oBASE_ADD_rbENV_SMASK,
	PSHADER_oBASE_ADD_rbENV_SAMASK, //20

	PSHADER_oBASE_LERP_tLAYER_ADD_rbENV,
	PSHADER_oBASE_LERP_vLAYER_ADD_rbENV,
	PSHADER_oBASE_LERP_pLAYER_ADD_rbENV,

	PSHADER_COLOR,
	PSHADER_PERPIXEL_POINT1,	//25
	PSHADER_PERPIXEL_POINT2,
	PSHADER_PERPIXEL_SPOT1,
	PSHADER_PERPIXEL_SPOT1_SHADOW,
	PSHADER_PERPIXEL_SPOT2,
	PSHADER_PERPIXEL_SPOT1_BUMP, //30
	PSHADER_PERPIXEL_SPOT1_SPECBUMP,
	PSHADER_PERPIXEL_POINT1_BUMP,
	PSHADER_PERPIXEL_DIRECTIONAL1_SPECBUMP,
	PSHADER_COLOR_MASK,
	PSHADER_COLORALPHA_MASK,//35
	PSHADER_COLOR_EMASK,
	PSHADER_COLOR_EMASK_INV,
	PSHADER_COLOR_EMISSIVE,

	PSHADER_DETAIL,
	PSHADER_oBASE_LERP_tLAYER_DETAIL,	//40
	PSHADER_oBASE_LERP_vLAYER_DETAIL,
	PSHADER_oBASE_LERP_pLAYER_DETAIL,
	PSHADER_oBASE_ADD_rbENV_DETAIL,
	PSHADER_oBASE_LERP_tLAYER_ADD_rbENV_DETAIL,
	PSHADER_oBASE_LERP_vLAYER_ADD_rbENV_DETAIL, //45
	PSHADER_oBASE_LERP_pLAYER_ADD_rbENV_DETAIL,

	PSHADER_LM1,
	PSHADER_LM2,
	PSHADER_LM3,
	PSHADER_LM4, //50
	PSHADER_ZMASK_EMASK_LM1,
	PSHADER_ZMASK_EMASK_LM2,
	PSHADER_ZMASK_LM1,
	PSHADER_ZMASK_LM2,
	PSHADER_ZMASK_LM3, //55
	PSHADER_EMASK_LM1,
	PSHADER_INV_EMASK_LM1,
	PSHADER_EMASK_LM2,
	PSHADER_EMASK_LM3,

	//Special FX Shaders
	MOLTEN_1LAYER_EMBM, //60
	MOLTEN_2LAYER_EMBM,
	MOLTEN_2LAYER_EMBM_GLOW,
	PSHADER_CUBE_REFLECT,
	PSHADER_CUBE_REFLECT_VTXALPLHA,
} PIXEL_SHADER_LIST_e;

//Map DetailMap Pixel Shader to non-DetailMap equivalents.
PIXEL_SHADER_LIST_e _DetailPShaderRemap[] = 
{
	PSHADER_PASSTHRU,						//PSHADER_DETAIL
	PSHADER_oBASE_LERP_tLAYER,				//PSHADER_oBASE_LERP_tLAYER_DETAIL
	PSHADER_oBASE_LERP_vLAYER,				//PSHADER_oBASE_LERP_vLAYER_DETAIL
	PSHADER_oBASE_LERP_pLAYER,				//PSHADER_oBASE_LERP_pLAYER_DETAIL
	PSHADER_oBASE_ADD_rbENV,				//PSHADER_oBASE_ADD_rbENV_DETAIL
	PSHADER_oBASE_LERP_tLAYER_ADD_rbENV,	//PSHADER_oBASE_LERP_tLAYER_ADD_rbENV_DETAIL
	PSHADER_oBASE_LERP_vLAYER_ADD_rbENV,	//PSHADER_oBASE_LERP_vLAYER_ADD_rbENV_DETAIL
	PSHADER_oBASE_LERP_pLAYER_ADD_rbENV		//PSHADER_oBASE_LERP_pLAYER_ADD_rbENV_DETAIL
};

PIXEL_SHADER_LIST_e _ReflectionPShaderRemap[] =
{
	PSHADER_PASSTHRU,				//PSHADER_oBASE_ADD_rbENV,
    PSHADER_PASSTHRU,				//PSHADER_oBASE_ADD_rbENV_LIT									
	PSHADER_PASSTHRU,				//PSHADER_oBASE_ADD_rbENV_LIT_LM
	PSHADER_PASSTHRU,				//PSHADER_oBASE_ADD_rbENV_EMBM,
	PSHADER_PASSTHRU,				//PSHADER_oBASE_ADD_rbENV_AMASK,
	PSHADER_PASSTHRU,				//PSHADER_oBASE_ADD_rbENV_SMASK,
	PSHADER_PASSTHRU,				//PSHADER_oBASE_ADD_rbENV_SAMASK, //15

	PSHADER_oBASE_LERP_tLAYER,		//PSHADER_oBASE_LERP_tLAYER_ADD_rbENV,
	PSHADER_oBASE_LERP_vLAYER,		//PSHADER_oBASE_LERP_vLAYER_ADD_rbENV,
	PSHADER_oBASE_LERP_pLAYER,		//PSHADER_oBASE_LERP_pLAYER_ADD_rbENV,

	PSHADER_COLOR,
	PSHADER_PERPIXEL_POINT1,
	PSHADER_PERPIXEL_POINT2,
	PSHADER_PERPIXEL_SPOT1,
	PSHADER_PERPIXEL_SPOT1_SHADOW,
	PSHADER_PERPIXEL_SPOT2,
	PSHADER_PERPIXEL_SPOT1_BUMP,
	PSHADER_PERPIXEL_SPOT1_SPECBUMP,
	PSHADER_PERPIXEL_POINT1_BUMP,
	PSHADER_PERPIXEL_DIRECTIONAL1_SPECBUMP,
	PSHADER_COLOR_MASK,
	PSHADER_COLORALPHA_MASK,
	PSHADER_COLOR_EMASK,
	PSHADER_COLOR_EMASK_INV,
	PSHADER_COLOR_EMISSIVE,

	PSHADER_DETAIL,
	PSHADER_oBASE_LERP_tLAYER_DETAIL,
	PSHADER_oBASE_LERP_vLAYER_DETAIL,
	PSHADER_oBASE_LERP_pLAYER_DETAIL,
	PSHADER_DETAIL,						//PSHADER_oBASE_ADD_rbENV_DETAIL,
	PSHADER_oBASE_LERP_tLAYER_DETAIL,	//PSHADER_oBASE_LERP_tLAYER_ADD_rbENV_DETAIL,
	PSHADER_oBASE_LERP_vLAYER_DETAIL,	//PSHADER_oBASE_LERP_vLAYER_ADD_rbENV_DETAIL,
	PSHADER_oBASE_LERP_pLAYER_DETAIL	//PSHADER_oBASE_LERP_pLAYER_ADD_rbENV_DETAIL,
};

PIXEL_SHADER_LIST_e _BasePShaderRemap[] =
{
	PSHADER_NONE,
	PSHADER_PASSTHRU,
	PSHADER_PASSTHRU_VALPHA,
	PSHADER_PASSTHRU_VTALPHA,
	PSHADER_PASSTHRU_VAMUL,
	PSHADER_PASSTHRU,//PSHADER_PASSTHRU_LM1,
	PSHADER_PASSTHRU,//PSHADER_PASSTHRU_LM2,
	PSHADER_PASSTHRU,//PSHADER_PASSTHRU_LM3,
	PSHADER_PASSTHRU_ALPHAMASK,	//5
	PSHADER_PASSTHRU_SALPHA,
	PSHADER_PASSTHRU,//PSHADER_oBASE_LERP_tLAYER,
	PSHADER_PASSTHRU,//PSHADER_oBASE_LERP_tLAYER_ALPHAMASK,
	PSHADER_PASSTHRU,//PSHADER_oBASE_LERP_vLAYER,
	PSHADER_PASSTHRU,//PSHADER_oBASE_LERP_pLAYER,	//10
	PSHADER_PASSTHRU,//PSHADER_oBASE_ADD_rbENV,
	PSHADER_PASSTHRU,//PSHADER_oBASE_ADD_rbENV_LIT,
	PSHADER_PASSTHRU,//PSHADER_oBASE_ADD_rbENV_LIT_LM,
	PSHADER_PASSTHRU,//PSHADER_oBASE_ADD_rbENV_EMBM,
	PSHADER_PASSTHRU,//PSHADER_oBASE_ADD_rbENV_AMASK,
	PSHADER_PASSTHRU,//PSHADER_oBASE_ADD_rbENV_SMASK,
	PSHADER_PASSTHRU,//PSHADER_oBASE_ADD_rbENV_SAMASK, //15

	PSHADER_PASSTHRU,//PSHADER_oBASE_LERP_tLAYER_ADD_rbENV,
	PSHADER_PASSTHRU,//PSHADER_oBASE_LERP_vLAYER_ADD_rbENV,
	PSHADER_PASSTHRU,//PSHADER_oBASE_LERP_pLAYER_ADD_rbENV,

	PSHADER_COLOR,
	PSHADER_PERPIXEL_POINT1,
	PSHADER_PERPIXEL_POINT2,
	PSHADER_PERPIXEL_SPOT1,
	PSHADER_PERPIXEL_SPOT1_SHADOW,
	PSHADER_PERPIXEL_SPOT2,//20
	PSHADER_PERPIXEL_SPOT1_BUMP,
	PSHADER_PERPIXEL_SPOT1_SPECBUMP,
	PSHADER_PERPIXEL_POINT1_BUMP,
	PSHADER_PERPIXEL_DIRECTIONAL1_SPECBUMP,
	PSHADER_COLOR_MASK,
	PSHADER_COLORALPHA_MASK,//25
	PSHADER_COLOR_EMASK,
	PSHADER_COLOR_EMASK_INV,
	PSHADER_COLOR_EMISSIVE,

	PSHADER_PASSTHRU,//PSHADER_DETAIL,
	PSHADER_PASSTHRU,//PSHADER_oBASE_LERP_tLAYER_DETAIL,
	PSHADER_PASSTHRU,//PSHADER_oBASE_LERP_vLAYER_DETAIL,
	PSHADER_PASSTHRU,//PSHADER_oBASE_LERP_pLAYER_DETAIL,//30
	PSHADER_PASSTHRU,//PSHADER_oBASE_ADD_rbENV_DETAIL,
	PSHADER_PASSTHRU,//PSHADER_oBASE_LERP_tLAYER_ADD_rbENV_DETAIL,
	PSHADER_PASSTHRU,//PSHADER_oBASE_LERP_vLAYER_ADD_rbENV_DETAIL,
	PSHADER_PASSTHRU,//PSHADER_oBASE_LERP_pLAYER_ADD_rbENV_DETAIL,

	PSHADER_LM1,
	PSHADER_LM2,
	PSHADER_LM3,
	PSHADER_LM4,
	PSHADER_ZMASK_EMASK_LM1,
	PSHADER_ZMASK_EMASK_LM2,
	PSHADER_ZMASK_LM1,
	PSHADER_ZMASK_LM2,
	PSHADER_ZMASK_LM3,
	PSHADER_EMASK_LM1,
	PSHADER_EMASK_LM2,
	PSHADER_EMASK_LM3,

	//Special FX Shaders
	MOLTEN_1LAYER_EMBM,
	MOLTEN_2LAYER_EMBM,
	MOLTEN_2LAYER_EMBM_GLOW,
	PSHADER_CUBE_REFLECT,
	PSHADER_CUBE_REFLECT_VTXALPLHA,
};

_VERTEX_SHADER_BASE_e _DetailVShaderRemap[] = 
{
	VSHADER_BASE_PASSTHRU,			//VSHADER_BASE_DETAIL
	VSHADER_BASE_oBASE_ADD_rbENV	//VSHADER_BASE_oBASE_ADD_rbENV_DETAIL
};

_VERTEX_SHADER_BASE_e _ReflectionVShaderRemap[] = 
{
	VSHADER_BASE_PASSTHRU,	//VSHADER_BASE_PASSTHRU_ENV,
	VSHADER_BASE_PASSTHRU,	//VSHADER_BASE_oBASE_ADD_rbENV,
	VSHADER_BASE_PASSTHRU,	//VSHADER_BASE_oBASE_ADD_rbENV_EMBM,
	VSHADER_BASE_POINT1_DIR1, //VSHADER_BASE_ENV_POINT1_DIR1
	VSHADER_BASE_POINT2_DIR1, //VSHADER_BASE_ENV_POINT2_DIR1
	VSHADER_BASE_POINT4_DIR1, //VSHADER_BASE_ENV_POINT4_DIR1
	VSHADER_BASE_VCOLOR,
	VSHADER_BASE_POINT1_DIR1,
	VSHADER_BASE_POINT2_DIR1,
	VSHADER_BASE_POINT4_DIR1,
	VSHADER_BASE_POINT6_DIR1,
	VSHADER_BASE_POINT7_DIR1,
	VSHADER_BASE_POINT8,
	VSHADER_BASE_PERPIXELPOINT1,
	VSHADER_BASE_PERPIXELPOINT2,
	VSHADER_BASE_PERPIXELSPOT1,
	VSHADER_BASE_PERPIXELSPOT2,
	VSHADER_BASE_PERPIXELSPOT1_BUMP,
	VSHADER_BASE_PERPIXELSPOT1_SPECBUMP,
	VSHADER_BASE_PERPIXELPOINT1_BUMP,
	VSHADER_BASE_PERPIXELDIRECTIONAL1_SPECBUMP,
	VSHADER_BASE_DIRECTIONAL1_SPECULAR,

	VSHADER_BASE_DETAIL,
	VSHADER_BASE_DETAIL,	//VSHADER_BASE_oBASE_ADD_rbENV_DETAIL,
	VSHADER_BASE_PASSTHRU,	//VSHADER_BASE_oBASE_ADD_rbSREFLECT,
	VSHADER_BASE_PASSTHRU,	//VSHADER_BASE_oBASE_ADD_rbSREFLECT_EMBM,
};

_VERTEX_SHADER_BASE_e _BaseVShaderRemap[] = 
{
	VSHADER_BASE_NONE,
	VSHADER_BASE_PASSTHRU,
	VSHADER_BASE_PASSTHRU,				//VSHADER_BASE_PASSTHRU_ENV,
	VSHADER_BASE_PASSTHRU,				//VSHADER_BASE_oBASE_ADD_rbENV,
	VSHADER_BASE_PASSTHRU,				//VSHADER_BASE_oBASE_ADD_rbENV_EMBM,
	VSHADER_BASE_PASSTHRU,				//VSHADER_BASE_ENV_POINT1_DIR1
	VSHADER_BASE_PASSTHRU,				//VSHADER_BASE_ENV_POINT2_DIR1
	VSHADER_BASE_PASSTHRU,				//VSHADER_BASE_ENV_POINT4_DIR1
    VSHADER_BASE_VCOLOR,
	VSHADER_BASE_POINT1_DIR1,
	VSHADER_BASE_POINT2_DIR1,
	VSHADER_BASE_POINT4_DIR1,
	VSHADER_BASE_POINT6_DIR1,
	VSHADER_BASE_POINT7_DIR1,
	VSHADER_BASE_POINT8,
	VSHADER_BASE_PERPIXELPOINT1,
	VSHADER_BASE_PERPIXELPOINT2,
	VSHADER_BASE_PERPIXELSPOT1,
	VSHADER_BASE_PERPIXELSPOT2,
	VSHADER_BASE_PERPIXELSPOT1_BUMP,
	VSHADER_BASE_PERPIXELSPOT1_SPECBUMP,
	VSHADER_BASE_PERPIXELPOINT1_BUMP,
	VSHADER_BASE_PERPIXELDIRECTIONAL1_SPECBUMP,
	VSHADER_BASE_DIRECTIONAL1_SPECULAR,
	VSHADER_BASE_PASSTHRU,				//VSHADER_BASE_DETAIL,
	VSHADER_BASE_PASSTHRU,				//VSHADER_BASE_oBASE_ADD_rbENV_DETAIL,
	VSHADER_BASE_PASSTHRU,				//VSHADER_BASE_oBASE_ADD_rbSREFLECT,
	VSHADER_BASE_PASSTHRU,				//VSHADER_BASE_oBASE_ADD_rbSREFLECT_EMBM,
	VSHADER_BASE_LM4,
	VSHADER_BASE_ZMASK_EMASK_LM2,
	VSHADER_BASE_ZMASK_LM3,
	VSHADER_BASE_EMASK_LM3,
	VSHADER_BASE_PASSTHRU,				//VSHADER_BASE_REFLECT,
};

//--Jeremy--//
typedef enum 
{
	//------------SURFACE SHADERS-------------//
	VSHADER_PASSTHRU_TC1=0,
	VSHADER_PASSTHRU_TC2,
	VSHADER_PASSTHRU_TC2_ENV,
	VSHADER_PASSTHRU_BLEND_TC1,
	VSHADER_PASSTHRU_BLEND_TC2,
	VSHADER_oBASE_ADD_rbENV, //5
	VSHADER_oBASE_ADD_rbENV_BLEND, //6
	VSHADER_oBASE_ADD_rbENV_EMBM, //7
	VSHADER_ENV_LM,	//8
	VSHADER_ENV_POINT1_DIR1,
	VSHADER_ENV_POINT2_DIR1,
	VSHADER_ENV_POINT4_DIR1,
	//------------DIFFUSE SHADERS-------------//
	VSHADER_VCOLOR_tc,
	VSHADER_VCOLOR_tc_blend,
	VSHADER_POINT1_DIR1_tc,
	VSHADER_POINT1_DIR1_tc_blend,
	VSHADER_POINT2_DIR1_tc,//15
	VSHADER_POINT2_DIR1_tc_blend,
	VSHADER_POINT4_DIR1_tc,
	VSHADER_POINT4_DIR1_tc_blend,
	VSHADER_POINT6_DIR1_tc,
	VSHADER_POINT6_DIR1_tc_blend, //20
	VSHADER_POINT7_DIR1_tc,
	VSHADER_POINT7_DIR1_tc_blend,
	VSHADER_POINT8_tc,
	VSHADER_POINT8_tc_blend,
	VSHADER_PERPIXELPOINT1_tc,
	VSHADER_PERPIXELPOINT1_tc_blend,
	VSHADER_PERPIXELPOINT2_tc,
	VSHADER_PERPIXELPOINT2_tc_blend,
	VSHADER_PERPIXELSPOT1_tc,
	VSHADER_PERPIXELSPOT1_tc_blend, //30
	VSHADER_PERPIXELSPOT2_tc,
	VSHADER_PERPIXELSPOT2_tc_blend,
	VSHADER_PERPIXELSPOT1_BUMP_tc,
	VSHADER_PERPIXELSPOT1_BUMP_tc_blend,
	VSHADER_PERPIXELSPOT1_SPECBUMP_tc,
	VSHADER_PERPIXELSPOT1_SPECBUMP_tc_blend,
	VSHADER_PERPIXELPOINT1_BUMP_tc,
	VSHADER_PERPIXELPOINT1_BUMP_tc_blend,
	VSHADER_PERPIXELDIRECTIONAL1_SPECBUMP_tc,
	VSHADER_PERPIXELDIRECTIONAL1_SPECBUMP_tc_blend, //40
	VSHADER_DIRECTIONAL1_SPECULAR_tc,
	VSHADER_DIRECTIONAL1_SPECULAR_tc_blend,

	VSHADER_DETAIL_tc,
	VSHADER_DETAIL_tc_blend,
	VSHADER_DETAIL_2tc,
	VSHADER_DETAIL_2tc_blend,
 
	VSHADER_oBASE_ADD_rbENV_DETAIL,
	VSHADER_oBASE_ADD_rbSREFLECT,
	VSHADER_oBASE_ADD_rbSREFLECT_EMBM,

	VSHADER_1LMAP, //50
	VSHADER_2LMAP,
	VSHADER_3LMAP,
	VSHADER_4LMAP,
	VSHADER_ZMASK_EMASK_LM1,
	VSHADER_ZMASK_EMASK_LM2,
	VSHADER_ZMASK_LM1,
	VSHADER_ZMASK_LM2,
	VSHADER_ZMASK_LM3,
	VSHADER_EMASK_LM1,
	VSHADER_EMASK_LM2,
	VSHADER_EMASK_LM3, //60

	VSHADER_PLANAR_REFLECT,
	VSHADER_PLANAR_REFLECT_MESH,
	VSHADER_PLANAR_REFRACT,

	VSHADER_VCOLOR_EXTSTREAM,
	VSHADER_POINT1_DIR1_EXTSTREAM,
	VSHADER_POINT2_DIR1_EXTSTREAM,
	VSHADER_POINT4_DIR1_EXTSTREAM,
	VSHADER_POINT6_DIR1_EXTSTREAM,
	VSHADER_POINT7_DIR1_EXTSTREAM, //70
	VSHADER_POINT8_EXTSTREAM,

	VSHADER_ENV_POINT1_DIR1_EXTSTREAM,
	VSHADER_ENV_POINT2_DIR1_EXTSTREAM,
	VSHADER_ENV_POINT4_DIR1_EXTSTREAM,
		
	VSHADER_NUM
} _VERTEX_SHADER_LIST_e;

u32 _aDecl_POS_TEX4[] =
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),    //Pos(xyz)
	D3DVSD_REG(1, D3DVSDT_FLOAT2),    //TC0(uv)
	D3DVSD_REG(2, D3DVSDT_FLOAT2),    //TC1(uv)
	D3DVSD_REG(3, D3DVSDT_FLOAT2),    //TC2(uv)
	D3DVSD_REG(4, D3DVSDT_FLOAT2),    //TC3(uv)
	D3DVSD_END()
};

u32 _aDecl_POS_NRML_CLR_TEX1[] =
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),    //Pos(xyz)
	D3DVSD_REG(1, D3DVSDT_FLOAT3),    //Nrml(xyz)
	D3DVSD_REG(2, D3DVSDT_D3DCOLOR ), //Clr(DWORD)
	D3DVSD_REG(3, D3DVSDT_FLOAT2),    //TC0(uv)
	D3DVSD_END()
};

u32 _aDecl_POS_NRML_CLR_BUMP_TEX1[] =
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),    //Pos(xyz)
	D3DVSD_REG(1, D3DVSDT_FLOAT3),    //Nrml(xyz)
	D3DVSD_REG(2, D3DVSDT_D3DCOLOR ), //Clr(DWORD)
	D3DVSD_REG(3, D3DVSDT_FLOAT2),    //TC0(uv)
	D3DVSD_STREAM(2),
	D3DVSD_REG(4, D3DVSDT_FLOAT3),    //Tangent
	D3DVSD_REG(5, D3DVSDT_FLOAT3),    //Binormal
	D3DVSD_END()
};

u32 _aDecl_POS_NRML_CLR_TEX2[] =
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),    //Pos(xyz)
	D3DVSD_REG(1, D3DVSDT_FLOAT3),    //Nrml(xyz)
	D3DVSD_REG(2, D3DVSDT_D3DCOLOR ), //Clr(DWORD)
	D3DVSD_REG(3, D3DVSDT_FLOAT2),    //TC0(uv)
	D3DVSD_REG(4, D3DVSDT_FLOAT2),    //TC1(uv)
	D3DVSD_END()
};

u32 _aDecl_POS_NRML_CLR_TEX3[] =
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),    //Pos(xyz)
	D3DVSD_REG(1, D3DVSDT_FLOAT3),    //Nrml(xyz)
	D3DVSD_REG(2, D3DVSDT_D3DCOLOR ), //Clr(DWORD)
	D3DVSD_REG(3, D3DVSDT_FLOAT2),    //TC0(uv)
	D3DVSD_REG(4, D3DVSDT_FLOAT2),    //TC1(uv)
	D3DVSD_REG(5, D3DVSDT_FLOAT2),    //TC2(uv)
	D3DVSD_END()
};

u32 _aDecl_POS_NRML_CLR_TEX4[] =
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),    //Pos(xyz)
	D3DVSD_REG(1, D3DVSDT_FLOAT3),    //Nrml(xyz)
	D3DVSD_REG(2, D3DVSDT_D3DCOLOR ), //Clr(DWORD)
	D3DVSD_REG(3, D3DVSDT_FLOAT2),    //TC0(uv)
	D3DVSD_REG(4, D3DVSDT_FLOAT2),    //TC1(uv)
	D3DVSD_REG(5, D3DVSDT_FLOAT2),    //TC2(uv)
	D3DVSD_REG(6, D3DVSDT_FLOAT2),    //TC2(uv)
	D3DVSD_END()
};

u32 _aDecl_POS_NRML_CLR_TEX5[] =
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),    //Pos(xyz)
	D3DVSD_REG(1, D3DVSDT_FLOAT3),    //Nrml(xyz)
	D3DVSD_REG(2, D3DVSDT_D3DCOLOR ), //Clr(DWORD)
	D3DVSD_REG(3, D3DVSDT_FLOAT2),    //TC0(uv)
	D3DVSD_REG(4, D3DVSDT_FLOAT2),    //TC1(uv)
	D3DVSD_REG(5, D3DVSDT_FLOAT2),    //TC2(uv)
	D3DVSD_REG(6, D3DVSDT_FLOAT2),    //TC2(uv)
	D3DVSD_REG(7, D3DVSDT_FLOAT2),    //TC2(uv)
	D3DVSD_END()
};

u32 _aDecl_POS_NRML_CLR_TEX6[] =
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),    //Pos(xyz)
	D3DVSD_REG(1, D3DVSDT_FLOAT3),    //Nrml(xyz)
	D3DVSD_REG(2, D3DVSDT_D3DCOLOR ), //Clr(DWORD)
	D3DVSD_REG(3, D3DVSDT_FLOAT2),    //TC0(uv)
	D3DVSD_REG(4, D3DVSDT_FLOAT2),    //TC1(uv)
	D3DVSD_REG(5, D3DVSDT_FLOAT2),    //TC2(uv)
	D3DVSD_REG(6, D3DVSDT_FLOAT2),    //TC2(uv)
	D3DVSD_REG(7, D3DVSDT_FLOAT2),    //TC2(uv)
	D3DVSD_REG(8, D3DVSDT_FLOAT2),    //TC2(uv)
	D3DVSD_END()
};

u32 _aDecl_POS_NRML_CLR_TEX7[] =
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),    //Pos(xyz)
	D3DVSD_REG(1, D3DVSDT_FLOAT3),    //Nrml(xyz)
	D3DVSD_REG(2, D3DVSDT_D3DCOLOR ), //Clr(DWORD)
	D3DVSD_REG(3, D3DVSDT_FLOAT2),    //TC0(uv)
	D3DVSD_REG(4, D3DVSDT_FLOAT2),    //TC1(uv)
	D3DVSD_REG(5, D3DVSDT_FLOAT2),    //TC2(uv)
	D3DVSD_REG(6, D3DVSDT_FLOAT2),    //TC2(uv)
	D3DVSD_REG(7, D3DVSDT_FLOAT2),    //TC2(uv)
	D3DVSD_REG(8, D3DVSDT_FLOAT2),    //TC2(uv)
	D3DVSD_REG(9, D3DVSDT_FLOAT2),    //TC2(uv)
	D3DVSD_END()
};

u32 _aDecl_POS_NRML_CLR_TEX8[] =
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),    //Pos(xyz)
	D3DVSD_REG(1, D3DVSDT_FLOAT3),    //Nrml(xyz)
	D3DVSD_REG(2, D3DVSDT_D3DCOLOR ), //Clr(DWORD)
	D3DVSD_REG(3, D3DVSDT_FLOAT2),    //TC0(uv)
	D3DVSD_REG(4, D3DVSDT_FLOAT2),    //TC1(uv)
	D3DVSD_REG(5, D3DVSDT_FLOAT2),    //TC2(uv)
	D3DVSD_REG(6, D3DVSDT_FLOAT2),    //TC2(uv)
	D3DVSD_REG(7, D3DVSDT_FLOAT2),    //TC2(uv)
	D3DVSD_REG(8, D3DVSDT_FLOAT2),    //TC2(uv)
	D3DVSD_REG(9, D3DVSDT_FLOAT2),    //TC2(uv)
	D3DVSD_REG(10, D3DVSDT_FLOAT2),    //TC2(uv)
	D3DVSD_END()
};

u32 _aDecl_POS_BLEND_NRML_CLR_TEX1[] =
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),    //Pos(xyz)
	D3DVSD_REG(1, D3DVSDT_FLOAT3),    //Blend(012)
	D3DVSD_REG(2, D3DVSDT_FLOAT3),    //Nrml(xyz)
	D3DVSD_REG(3, D3DVSDT_D3DCOLOR ), //Clr(DWORD)
	D3DVSD_REG(4, D3DVSDT_FLOAT2),    //TC0(uv)
	D3DVSD_END()
};

u32 _aDecl_POS_BLEND_NRML_CLR_BUMP_TEX1[] =
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),    //Pos(xyz)
	D3DVSD_REG(1, D3DVSDT_FLOAT3),    //Nrml(xyz)
	D3DVSD_REG(2, D3DVSDT_D3DCOLOR ), //Clr(DWORD)
	D3DVSD_REG(3, D3DVSDT_FLOAT2),    //TC0(uv)
	D3DVSD_REG(4, D3DVSDT_FLOAT2),    //TC0(uv)
	D3DVSD_STREAM(2),
	D3DVSD_REG(5, D3DVSDT_FLOAT3),    //Tangent
	D3DVSD_REG(6, D3DVSDT_FLOAT3),    //Binormal
	D3DVSD_END()
};

u32 _aDecl_POS_BLEND_NRML_CLR_TEX2[] =
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),    //Pos(xyz)
	D3DVSD_REG(1, D3DVSDT_FLOAT3),    //Blend(012)
	D3DVSD_REG(2, D3DVSDT_FLOAT3),    //Nrml(xyz)
	D3DVSD_REG(3, D3DVSDT_D3DCOLOR ), //Clr(DWORD)
	D3DVSD_REG(4, D3DVSDT_FLOAT2),    //TC0(uv)
	D3DVSD_REG(5, D3DVSDT_FLOAT2),    //TC1(uv)
	D3DVSD_END()
};

//Vertex Radiosity Decl

u32 _aDecl_POS_NRML_CLR_LMCLR[] =
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),    //Pos(xyz)
	D3DVSD_REG(1, D3DVSDT_FLOAT3),    //Nrml(xyz)
	D3DVSD_SKIP(1),				  	  //Skip DWORD (DWORD)
	D3DVSD_REG(3, D3DVSDT_FLOAT2),    //TC0(uv)
	D3DVSD_STREAM(3),
	D3DVSD_REG(2, D3DVSDT_D3DCOLOR ), //Clr(DWORD)
	D3DVSD_END()
};

//LightMap Declarations
u32 _aDecl_POS_NRML_CLR_TEX1_LM1[] =
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),    //Pos(xyz)
	D3DVSD_REG(1, D3DVSDT_FLOAT3),    //Nrml(xyz)
	D3DVSD_REG(2, D3DVSDT_D3DCOLOR ), //Clr(DWORD)
	D3DVSD_REG(3, D3DVSDT_FLOAT2),    //TC0(uv)
	D3DVSD_STREAM(1),
	D3DVSD_REG(4, D3DVSDT_FLOAT2),    //LMap TC0(uv)
	D3DVSD_END()
};
u32 _aDecl_POS_NRML_CLR_TEX1_LM2[] =
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),    //Pos(xyz)
	D3DVSD_REG(1, D3DVSDT_FLOAT3),    //Nrml(xyz)
	D3DVSD_REG(2, D3DVSDT_D3DCOLOR ), //Clr(DWORD)
	D3DVSD_REG(3, D3DVSDT_FLOAT2),    //TC0(uv)
	D3DVSD_STREAM(1),
	D3DVSD_REG(4, D3DVSDT_FLOAT2),    //LMap TC0(uv)
	D3DVSD_REG(5, D3DVSDT_FLOAT2),    //LMap TC1(uv)
	D3DVSD_END()
};

u32 _aDecl_POS_NRML_CLR_TEX1_LM3[] =
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),    //Pos(xyz)
	D3DVSD_REG(1, D3DVSDT_FLOAT3),    //Nrml(xyz)
	D3DVSD_REG(2, D3DVSDT_D3DCOLOR ), //Clr(DWORD)
	D3DVSD_REG(3, D3DVSDT_FLOAT2),    //TC0(uv)
	D3DVSD_STREAM(1),
	D3DVSD_REG(4, D3DVSDT_FLOAT2),    //LMap TC0(uv)
	D3DVSD_REG(5, D3DVSDT_FLOAT2),    //LMap TC1(uv)
	D3DVSD_REG(6, D3DVSDT_FLOAT2),    //LMap TC2(uv)
	D3DVSD_END()
};

u32 _aDecl_POS_NRML_CLR_TEX1_LM4[] =
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),    //Pos(xyz)
	D3DVSD_REG(1, D3DVSDT_FLOAT3),    //Nrml(xyz)
	D3DVSD_REG(2, D3DVSDT_D3DCOLOR ), //Clr(DWORD)
	D3DVSD_REG(3, D3DVSDT_FLOAT2),    //TC0(uv)
	D3DVSD_STREAM(1),
	D3DVSD_REG(4, D3DVSDT_FLOAT2),    //LMap TC0(uv)
	D3DVSD_REG(5, D3DVSDT_FLOAT2),    //LMap TC1(uv)
	D3DVSD_REG(6, D3DVSDT_FLOAT2),    //LMap TC2(uv)
	D3DVSD_REG(7, D3DVSDT_FLOAT2),    //LMap TC3(uv)
	D3DVSD_END()
};

u32 *_apVShaderDecl[] = 
{ 
	_aDecl_POS_NRML_CLR_TEX1,
	_aDecl_POS_NRML_CLR_TEX2,
	_aDecl_POS_NRML_CLR_TEX2,
	_aDecl_POS_BLEND_NRML_CLR_TEX1,
	_aDecl_POS_BLEND_NRML_CLR_TEX2,
	_aDecl_POS_NRML_CLR_TEX1, //5
	_aDecl_POS_BLEND_NRML_CLR_TEX1,
	_aDecl_POS_NRML_CLR_TEX1,
	_aDecl_POS_NRML_CLR_TEX1_LM1, //50
	_aDecl_POS_NRML_CLR_TEX1,       
	_aDecl_POS_NRML_CLR_TEX1,       
	_aDecl_POS_NRML_CLR_TEX1,      //10 
	_aDecl_POS_NRML_CLR_TEX1,
	_aDecl_POS_BLEND_NRML_CLR_TEX1,
	_aDecl_POS_NRML_CLR_TEX1,
	_aDecl_POS_BLEND_NRML_CLR_TEX1,
	_aDecl_POS_NRML_CLR_TEX1,		//15
	_aDecl_POS_BLEND_NRML_CLR_TEX1,
	_aDecl_POS_NRML_CLR_TEX1,
	_aDecl_POS_BLEND_NRML_CLR_TEX1,
	_aDecl_POS_NRML_CLR_TEX1,		
	_aDecl_POS_BLEND_NRML_CLR_TEX1, //20 
	_aDecl_POS_NRML_CLR_TEX1,
	_aDecl_POS_BLEND_NRML_CLR_TEX1,
	_aDecl_POS_NRML_CLR_TEX1,
	_aDecl_POS_BLEND_NRML_CLR_TEX1,
	_aDecl_POS_NRML_CLR_TEX1,		//25
	_aDecl_POS_BLEND_NRML_CLR_TEX1,
	_aDecl_POS_NRML_CLR_TEX1,
	_aDecl_POS_BLEND_NRML_CLR_TEX1,
	_aDecl_POS_NRML_CLR_TEX1,
	_aDecl_POS_BLEND_NRML_CLR_TEX1, //30 
	_aDecl_POS_NRML_CLR_TEX1,
	_aDecl_POS_BLEND_NRML_CLR_TEX1,
	_aDecl_POS_NRML_CLR_BUMP_TEX1,
	_aDecl_POS_BLEND_NRML_CLR_BUMP_TEX1,
	_aDecl_POS_NRML_CLR_BUMP_TEX1, //35
	_aDecl_POS_BLEND_NRML_CLR_BUMP_TEX1,
	_aDecl_POS_NRML_CLR_BUMP_TEX1,
	_aDecl_POS_BLEND_NRML_CLR_BUMP_TEX1,
	_aDecl_POS_NRML_CLR_BUMP_TEX1,
	_aDecl_POS_BLEND_NRML_CLR_BUMP_TEX1, //40
	_aDecl_POS_NRML_CLR_TEX1,
	_aDecl_POS_BLEND_NRML_CLR_TEX1,
	_aDecl_POS_NRML_CLR_TEX1,
	_aDecl_POS_BLEND_NRML_CLR_TEX1,
	_aDecl_POS_NRML_CLR_TEX2, //45
	_aDecl_POS_BLEND_NRML_CLR_TEX2,
	_aDecl_POS_NRML_CLR_TEX1,
	_aDecl_POS_NRML_CLR_TEX1,
	_aDecl_POS_NRML_CLR_TEX1,
	//Lightmapping - 1 tc
	_aDecl_POS_NRML_CLR_TEX1_LM1, //50
	_aDecl_POS_NRML_CLR_TEX1_LM2,
	_aDecl_POS_NRML_CLR_TEX1_LM3,
	_aDecl_POS_NRML_CLR_TEX1_LM4,
	_aDecl_POS_NRML_CLR_TEX1_LM1, //50
	_aDecl_POS_NRML_CLR_TEX1_LM2,
	_aDecl_POS_NRML_CLR_TEX1_LM1,
	_aDecl_POS_NRML_CLR_TEX1_LM2,
	_aDecl_POS_NRML_CLR_TEX1_LM3,
	_aDecl_POS_NRML_CLR_TEX1_LM1, //55
	_aDecl_POS_NRML_CLR_TEX1_LM2,
	_aDecl_POS_NRML_CLR_TEX1_LM3, //57
	
	_aDecl_POS_TEX4,		//58
	_aDecl_POS_NRML_CLR_TEX1,
	_aDecl_POS_TEX4,		//59

	//vertex radiosity
	_aDecl_POS_NRML_CLR_LMCLR,
	_aDecl_POS_NRML_CLR_LMCLR,
	_aDecl_POS_NRML_CLR_LMCLR,
	_aDecl_POS_NRML_CLR_LMCLR,
	_aDecl_POS_NRML_CLR_LMCLR,
	_aDecl_POS_NRML_CLR_LMCLR,
	_aDecl_POS_NRML_CLR_LMCLR,

	//for external vertex fast shader pass
	_aDecl_POS_NRML_CLR_LMCLR,
	_aDecl_POS_NRML_CLR_LMCLR,
	_aDecl_POS_NRML_CLR_LMCLR,

	NULL
};

#if FANG_PLATFORM_WIN
u32 *_apVShaderFunc[] =
{
	(u32*)dwFdx8PassThru_1tcVertexShader, //0
	(u32*)dwFdx8PassThru_2tcVertexShader,
	(u32*)dwFdx8PassThru_2tc_ENVVertexShader,
	(u32*)dwFdx8PassThru_1tc_blendVertexShader,
	(u32*)dwFdx8PassThru_2tc_blendVertexShader,
	(u32*)dwFdx8oBase_Add_rbENVVertexShader,
	(u32*)dwFdx8oBase_Add_rbENV_blendVertexShader,	//5
	(u32*)dwFdx8oBase_Add_rbENV_EMBMVertexShader,
	//(u32*)dwFdx8oBase_Add_rbENV_LMVertexShader,
	(u32*)dwFdx8oBase_Add_rbENV_P1D1VertexShader,
	(u32*)dwFdx8oBase_Add_rbENV_P1D1VertexShader,
	(u32*)dwFdx8oBase_Add_rbENV_P2D1VertexShader,
	(u32*)dwFdx8oBase_Add_rbENV_P4D1VertexShader,
	(u32*)dwFdx8VtxColorVertexShader,      
	(u32*)dwFdx8VtxColor_blendVertexShader,
	(u32*)dwFdx8Point1Dir1_DiffuseVertexShader,
	(u32*)dwFdx8Point1Dir1_Diffuse_BlendVertexShader, //10
	(u32*)dwFdx8Point2Dir1_DiffuseVertexShader,	
	(u32*)dwFdx8Point2Dir1_Diffuse_BlendVertexShader,
	(u32*)dwFdx8Point4Dir1_DiffuseVertexShader,
	(u32*)dwFdx8Point4Dir1_Diffuse_BlendVertexShader,
	(u32*)dwFdx8Point6Dir1_DiffuseVertexShader, //15
	(u32*)dwFdx8Point6Dir1_Diffuse_BlendVertexShader,
	(u32*)dwFdx8Point7Dir1_DiffuseVertexShader,
	(u32*)dwFdx8Point7Dir1_Diffuse_BlendVertexShader,
	(u32*)dwFdx8Point8_DiffuseVertexShader,
	(u32*)dwFdx8Point8_DiffuseVertexShader, //20
	(u32*)dwFdx8PerPixelPoint1_DiffuseVertexShader,
	(u32*)dwFdx8PerPixelPoint1_DiffuseVertexShader,
	(u32*)dwFdx8PerPixelPoint2_DiffuseVertexShader,
	(u32*)dwFdx8PerPixelPoint2_DiffuseVertexShader,
	(u32*)dwFdx8PerPixelSpot1_DiffuseVertexShader, //25
	(u32*)dwFdx8PerPixelSpot1_DiffuseVertexShader,
	(u32*)dwFdx8PerPixelSpot2_DiffuseVertexShader,
	(u32*)dwFdx8PerPixelSpot2_DiffuseVertexShader,
	(u32*)dwFdx8PerPixelSpot1_Diffuse_BumpVertexShader,
	(u32*)dwFdx8PerPixelSpot1_Diffuse_BumpVertexShader, //30
	(u32*)dwFdx8PerPixelSpot1_Spec_BumpVertexShader,
	(u32*)dwFdx8PerPixelSpot1_Spec_BumpVertexShader, //30
	(u32*)dwFdx8PerPixelPoint1_Diffuse_BumpVertexShader,
	(u32*)dwFdx8PerPixelPoint1_Diffuse_BumpVertexShader, 
	(u32*)dwFdx8PerPixelDirectional1_Specular_BumpVertexShader,
	(u32*)dwFdx8PerPixelDirectional1_Specular_BumpVertexShader,
	(u32*)dwFdx8Directional1_SpecularVertexShader, //35
	(u32*)dwFdx8Directional1_SpecularVertexShader, 
	(u32*)dwFdx8DetailVertexShader,
	(u32*)dwFdx8DetailVertexShader,
	(u32*)dwFdx8Detail_2tcVertexShader,
	(u32*)dwFdx8Detail_2tcVertexShader, //40
	(u32*)dwFdx8oBase_Add_rbENV_DetailVertexShader,
	(u32*)dwFdx8oBase_Add_rbSReflectVertexShader,
	(u32*)dwFdx8oBase_Add_rbSReflect_EMBMVertexShader,
	//Lightmapping - 1 tc
	(u32*)dwFdx8VtxLMap1_tc1VertexShader,
	(u32*)dwFdx8VtxLMap2_tc1VertexShader, //45
	(u32*)dwFdx8VtxLMap3_tc1VertexShader,
	(u32*)dwFdx8VtxLMap4_tc1VertexShader,
	(u32*)dwFdx8VtxLMap1_tc1_Mask2VertexShader,
	(u32*)dwFdx8VtxLMap2_tc1_Mask2VertexShader,
	(u32*)dwFdx8VtxLMap1_tc1_Mask1VertexShader, //50
	(u32*)dwFdx8VtxLMap2_tc1_Mask1VertexShader,
	(u32*)dwFdx8VtxLMap3_tc1_Mask1VertexShader,
	(u32*)dwFdx8VtxLMap1_tc1_Mask1VertexShader,
	(u32*)dwFdx8VtxLMap2_tc1_Mask1VertexShader,
    (u32*)dwFdx8VtxLMap3_tc1_Mask1VertexShader, //55
	(u32*)dwFdx8PlanarReflVertexShader, //55
	(u32*)dwFdx8PlanarReflMeshVertexShader, //55
	(u32*)dwFdx8PlanarReflVertexShader, //

	(u32*)dwFdx8VtxColorVertexShader,      
	(u32*)dwFdx8Point1Dir1_DiffuseVertexShader,
	(u32*)dwFdx8Point2Dir1_DiffuseVertexShader,	
	(u32*)dwFdx8Point4Dir1_DiffuseVertexShader,
	(u32*)dwFdx8Point6Dir1_DiffuseVertexShader, //15
	(u32*)dwFdx8Point7Dir1_DiffuseVertexShader,
	(u32*)dwFdx8Point8_DiffuseVertexShader,

	(u32*)dwFdx8oBase_Add_rbENV_P1D1VertexShader,
	(u32*)dwFdx8oBase_Add_rbENV_P2D1VertexShader,
	(u32*)dwFdx8oBase_Add_rbENV_P4D1VertexShader,
	NULL
};
#else
u32 *_apVShaderFunc[] =
{
	(u32*)dwCompiledVShaderfxbPassThru_1tcVertexShader, //0
	(u32*)dwCompiledVShaderfxbPassThru_2tcVertexShader,
	(u32*)dwCompiledVShaderfxbPassThru_2tc_ENVVertexShader,
	(u32*)dwCompiledVShaderfxbPassThru_1tc_blendVertexShader,
	(u32*)dwCompiledVShaderfxbPassThru_2tc_blendVertexShader,
	(u32*)dwCompiledVShaderfxboBase_Add_rbENVVertexShader,	//5
	(u32*)dwCompiledVShaderfxboBase_Add_rbENV_blendVertexShader,
	(u32*)dwCompiledVShaderfxboBase_Add_rbENVVertexShader, 
	(u32*)dwCompiledVShaderfxboBase_Add_rbENV_LMVertexShader, 
	(u32*)dwCompiledVShaderfxboBase_Add_rbENV_P1D1VertexShader, 
	(u32*)dwCompiledVShaderfxboBase_Add_rbENV_P2D1VertexShader, 
	(u32*)dwCompiledVShaderfxboBase_Add_rbENV_P4D1VertexShader, //10
	(u32*)dwCompiledVShaderfxbVtxColorVertexShader,
	(u32*)dwCompiledVShaderfxbVtxColor_blendVertexShader,
	(u32*)dwCompiledVShaderfxbPoint1Dir1_DiffuseVertexShader,
	(u32*)dwCompiledVShaderfxbPoint1Dir1_Diffuse_BlendVertexShader, //10
	(u32*)dwCompiledVShaderfxbPoint2Dir1_DiffuseVertexShader, 
	(u32*)dwCompiledVShaderfxbPoint2Dir1_Diffuse_BlendVertexShader,
	(u32*)dwCompiledVShaderfxbPoint4Dir1_DiffuseVertexShader,
	(u32*)dwCompiledVShaderfxbPoint4Dir1_Diffuse_BlendVertexShader,
	(u32*)dwCompiledVShaderfxbPoint6Dir1_DiffuseVertexShader, //15
	(u32*)dwCompiledVShaderfxbPoint6Dir1_Diffuse_BlendVertexShader,
	(u32*)dwCompiledVShaderfxbPoint7Dir1_DiffuseVertexShader,
	(u32*)dwCompiledVShaderfxbPoint7Dir1_Diffuse_BlendVertexShader,
	(u32*)dwCompiledVShaderfxbPoint8_DiffuseVertexShader,
	(u32*)dwCompiledVShaderfxbPoint8_DiffuseVertexShader, //20
	(u32*)dwCompiledVShaderfxbPerPixelPoint1_DiffuseVertexShader,
	(u32*)dwCompiledVShaderfxbPerPixelPoint1_DiffuseVertexShader,
	(u32*)dwCompiledVShaderfxbPerPixelPoint2_DiffuseVertexShader,
	(u32*)dwCompiledVShaderfxbPerPixelPoint2_DiffuseVertexShader,
	(u32*)dwCompiledVShaderfxbPerPixelSpot1_DiffuseVertexShader, //25
	(u32*)dwCompiledVShaderfxbPerPixelSpot1_DiffuseVertexShader,
	(u32*)dwCompiledVShaderfxbPerPixelSpot2_DiffuseVertexShader,
	(u32*)dwCompiledVShaderfxbPerPixelSpot2_DiffuseVertexShader,
	(u32*)dwCompiledVShaderfxbPerPixelSpot1_Diffuse_BumpVertexShader,
	(u32*)dwCompiledVShaderfxbPerPixelSpot1_Diffuse_BumpVertexShader, //30
	(u32*)dwCompiledVShaderfxbPerPixelSpot1_Diffuse_BumpVertexShader,		//spec
	(u32*)dwCompiledVShaderfxbPerPixelSpot1_Diffuse_BumpVertexShader, //30	//spec
	(u32*)dwCompiledVShaderfxbPerPixelPoint1_Diffuse_BumpVertexShader,
	(u32*)dwCompiledVShaderfxbPerPixelPoint1_Diffuse_BumpVertexShader,
	(u32*)dwCompiledVShaderfxbPerPixelDirectional1_Specular_BumpVertexShader,
	(u32*)dwCompiledVShaderfxbPerPixelDirectional1_Specular_BumpVertexShader,
	(u32*)dwCompiledVShaderfxbDirectional1_SpecularVertexShader, //35
	(u32*)dwCompiledVShaderfxbDirectional1_SpecularVertexShader,
	(u32*)dwCompiledVShaderfxbDetailVertexShader,
	(u32*)dwCompiledVShaderfxbDetailVertexShader,
	(u32*)dwCompiledVShaderfxbDetail_2tcVertexShader,
	(u32*)dwCompiledVShaderfxbDetail_2tcVertexShader, //40
	(u32*)dwCompiledVShaderfxboBase_Add_rbENV_DetailVertexShader,
	(u32*)dwCompiledVShaderfxboBase_Add_rbSReflectVertexShader,
	(u32*)dwCompiledVShaderfxboBase_Add_rbSReflectVertexShader,
	//Lightmapping - 1 tc
	(u32*)dwCompiledVShaderfxbVtxLMap1_tc1VertexShader,
	(u32*)dwCompiledVShaderfxbVtxLMap2_tc1VertexShader, //45
	(u32*)dwCompiledVShaderfxbVtxLMap3_tc1VertexShader,
	(u32*)dwCompiledVShaderfxbVtxLMap4_tc1VertexShader,
	(u32*)dwCompiledVShaderfxbVtxLMap1_tc1_Mask2VertexShader,
	(u32*)dwCompiledVShaderfxbVtxLMap2_tc1_Mask2VertexShader,
	(u32*)dwCompiledVShaderfxbVtxLMap1_tc1_Mask1VertexShader, //50
	(u32*)dwCompiledVShaderfxbVtxLMap2_tc1_Mask1VertexShader,
	(u32*)dwCompiledVShaderfxbVtxLMap3_tc1_Mask1VertexShader,
	(u32*)dwCompiledVShaderfxbVtxLMap1_tc1_Mask1VertexShader,
	(u32*)dwCompiledVShaderfxbVtxLMap2_tc1_Mask1VertexShader,
    (u32*)dwCompiledVShaderfxbVtxLMap3_tc1_Mask1VertexShader, //55
//	(u32*)dwCompiledVShaderfxbVtxLMap3_tc1_Mask1VertexShader, //55
	(u32*)dwCompiledVShaderfxbPlanarReflVertexShader, //55
	(u32*)dwCompiledVShaderfxbPlanarReflMeshVertexShader, //55
	(u32*)dwCompiledVShaderfxbPlanarRefrVertexShader, //55

	(u32*)dwCompiledVShaderfxbVtxColorVertexShader,
	(u32*)dwCompiledVShaderfxbPoint1Dir1_DiffuseVertexShader,
	(u32*)dwCompiledVShaderfxbPoint2Dir1_DiffuseVertexShader, 
	(u32*)dwCompiledVShaderfxbPoint4Dir1_DiffuseVertexShader,
	(u32*)dwCompiledVShaderfxbPoint6Dir1_DiffuseVertexShader, //15
	(u32*)dwCompiledVShaderfxbPoint7Dir1_DiffuseVertexShader,
	(u32*)dwCompiledVShaderfxbPoint8_DiffuseVertexShader,

	(u32*)dwCompiledVShaderfxboBase_Add_rbENV_P1D1VertexShader, 
	(u32*)dwCompiledVShaderfxboBase_Add_rbENV_P2D1VertexShader, 
	(u32*)dwCompiledVShaderfxboBase_Add_rbENV_P4D1VertexShader, //10
	NULL
};
#endif

#if FANG_PLATFORM_WIN
u32 *_apPShaderFunc[] =
{
	(u32*)dwFdx8PassThruPixelShader,			//0
	(u32*)dwFdx8PassThru_VAlphaPixelShader,			//0
	(u32*)dwFdx8PassThru_VTAlphaPixelShader,			//0
	(u32*)dwFdx8PassThru_VAMulPixelShader,			//0
	(u32*)dwFdx8PassThru_LM1PixelShader,
	(u32*)dwFdx8PassThru_LM2PixelShader,
	(u32*)dwFdx8PassThru_LM3PixelShader,
	(u32*)dwFdx8PassThru_AlphaMaskPixelShader,
	(u32*)dwFdx8PassThru_SAlphaPixelShader,		//5
	(u32*)dwFdx8oBase_Lerp_tLayerPixelShader,
	(u32*)dwFdx8oBase_Lerp_tLayer_AlphaMaskPixelShader,
	(u32*)dwFdx8oBase_Lerp_vLayerPixelShader,
	(u32*)dwFdx8oBase_Lerp_pLayerPixelShader,
	(u32*)dwFdx8oBase_Add_rbENVPixelShader,		//10
	(u32*)dwFdx8oBase_Add_rbENV_LitPixelShader,		//10
    //(u32*)dwFdx8oBase_Add_rbENV_Lit_LMPixelShader,		//10
	(u32*)dwFdx8oBase_Add_rbENV_LitPixelShader,		//10
	(u32*)dwFdx8oBase_Add_rbENV_EMBMPixelShader,
	(u32*)dwFdx8oBase_Add_rbENV_AMaskPixelShader,
	(u32*)dwFdx8oBase_Add_rbENV_SMaskPixelShader,
	(u32*)dwFdx8oBase_Add_rbENV_SAMaskPixelShader,
	(u32*)dwFdx8oBase_Lerp_tLayer_ADD_rbENVPixelShader,
	(u32*)dwFdx8oBase_Lerp_vLayer_ADD_rbENVPixelShader,
	(u32*)dwFdx8oBase_Lerp_pLayer_ADD_rbENVPixelShader,
	(u32*)dwFdx8ColorPixelShader,					//15
	(u32*)dwFdx8PerPixelPoint1_DiffusePixelShader,
	(u32*)dwFdx8PerPixelPoint2_DiffusePixelShader,
	(u32*)dwFdx8PerPixelSpot1_DiffusePixelShader,
	(u32*)dwFdx8PerPixelSpot1_Diffuse_ShadowPixelShader,
	(u32*)dwFdx8PerPixelSpot2_DiffusePixelShader,
	(u32*)dwFdx8PerPixelSpot1_Diffuse_BumpPixelShader,	//20
	(u32*)dwFdx8PerPixelSpot1_Spec_BumpPixelShader,	//20
	(u32*)dwFdx8PerPixelPoint1_Diffuse_BumpPixelShader,
	(u32*)dwFdx8PerPixelDirectional1_Specular_BumpPixelShader,
	(u32*)dwFdx8ColorMaskPixelShader,
	(u32*)dwFdx8ColorAlphaMaskPixelShader,
	(u32*)dwFdx8ColorEMaskPixelShader,	//25
	(u32*)dwFdx8ColorEMask_InvPixelShader,	//25
	(u32*)dwFdx8ColorEmissivePixelShader,	//25
	(u32*)dwFdx8DetailPixelShader,
	(u32*)dwFdx8oBase_Lerp_tLayer_DetailPixelShader,
	(u32*)dwFdx8oBase_Lerp_vLayer_DetailPixelShader,
	(u32*)dwFdx8oBase_Lerp_pLayer_DetailPixelShader,
	(u32*)dwFdx8oBase_Add_rbENV_DetailPixelShader, //30
	(u32*)dwFdx8oBase_Lerp_tLayer_ADD_rbENV_DetailPixelShader,
	(u32*)dwFdx8oBase_Lerp_vLayer_ADD_rbENV_DetailPixelShader,
	(u32*)dwFdx8oBase_Lerp_pLayer_ADD_rbENV_DetailPixelShader,
	(u32*)dwFdx8LMap1PixelShader,
	(u32*)dwFdx8LMap2PixelShader,
	(u32*)dwFdx8LMap3PixelShader,
	(u32*)dwFdx8LMap4PixelShader,
	(u32*)dwFdx8LMap1_ZMask_EMaskPixelShader, //35
	(u32*)dwFdx8LMap2_ZMask_EMaskPixelShader,
	(u32*)dwFdx8LMap1_ZMaskPixelShader,
	(u32*)dwFdx8LMap2_ZMaskPixelShader,
	(u32*)dwFdx8LMap3_ZMaskPixelShader,
	(u32*)dwFdx8LMap1_EMaskPixelShader, //40
	(u32*)dwFdx8LMap1_InvEMaskPixelShader, //40
	(u32*)dwFdx8LMap2_EMaskPixelShader,
	(u32*)dwFdx8LMap3_EMaskPixelShader, //42
	(u32*)dwFdx8Molten1LayerEMBMPixelShader,
	(u32*)dwFdx8Molten2LayerEMBMPixelShader,
	(u32*)dwFdx8Molten2LayerEMBMGlowPixelShader,
	(u32*)dwFdx8Molten2LayerEMBMGlowPixelShader,
	NULL
};
#else
D3DPIXELSHADERDEF *_apPShaderFunc[] =
{
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbPassThruPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbPassThru_VAlphaPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbPassThru_VTAlphaPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbPassThru_VAMulPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbPassThru_LM1PixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbPassThru_LM2PixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbPassThru_LM3PixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbPassThru_AlphaMaskPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbPassThru_SAlphaPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxboBase_Lerp_tLayerPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxboBase_Lerp_tLayer_AlphaMaskPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxboBase_Lerp_vLayerPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxboBase_Lerp_pLayerPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxboBase_Add_rbENVPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxboBase_Add_rbENV_LitPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxboBase_Add_rbENV_Lit_LMPixelShader,
	//(D3DPIXELSHADERDEF*)dwCompiledPShaderfxboBase_Add_rbENV_EMBMPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxboBase_Add_rbENV_AMaskPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxboBase_Add_rbENV_AMaskPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxboBase_Add_rbENV_SMaskPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxboBase_Add_rbENV_SAMaskPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxboBase_Lerp_tLayer_ADD_rbENVPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxboBase_Lerp_vLayer_ADD_rbENVPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxboBase_Lerp_pLayer_ADD_rbENVPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbColorPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbPerPixelPoint1_DiffusePixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbPerPixelPoint2_DiffusePixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbPerPixelSpot1_DiffusePixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbPerPixelSpot1_DiffusePixelShader,
	//(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbPerPixelSpot1_Diffuse_ShadowPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbPerPixelSpot2_DiffusePixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbPerPixelSpot1_Diffuse_BumpPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbPerPixelSpot1_Diffuse_BumpPixelShader, //spec
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbPerPixelPoint1_Diffuse_BumpPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbPerPixelDirectional1_Specular_BumpPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbColorMaskPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbColorAlphaMaskPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbColorEMaskPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbColorEMask_InvPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbColorEmissivePixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbDetailPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxboBase_Lerp_tLayer_DetailPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxboBase_Lerp_vLayer_DetailPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxboBase_Lerp_pLayer_DetailPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxboBase_Add_rbENV_DetailPixelShader, //20
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxboBase_Lerp_tLayer_ADD_rbENV_DetailPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxboBase_Lerp_vLayer_ADD_rbENV_DetailPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxboBase_Lerp_pLayer_ADD_rbENV_DetailPixelShader,
	//Lightmapping shaders
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbLMap1PixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbLMap2PixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbLMap3PixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbLMap4PixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbLMap1_ZMask_EMaskPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbLMap2_ZMask_EMaskPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbLMap1_ZMaskPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbLMap2_ZMaskPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbLMap3_ZMaskPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbLMap1_EMaskPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbLMap1_InvEMaskPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbLMap2_EMaskPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbLMap3_EMaskPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbMolten1LayerEMBMPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbMolten2LayerEMBMPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbMolten2LayerEMBMGlowPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbCubeReflectPixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbCubeReflect_VtxAlphaPixelShader,
	NULL
};
#endif

u32 _anVShaderRemap[][4] =
{
	//VSHADER_BASE_PASSTHRU
	{
		VSHADER_PASSTHRU_TC1,
		VSHADER_PASSTHRU_TC2,
		VSHADER_PASSTHRU_BLEND_TC1,
		VSHADER_PASSTHRU_BLEND_TC2
	},
	//VSHADER_BASE_PASSTHRU_ENV
	{
		VSHADER_PASSTHRU_TC2_ENV,
		VSHADER_PASSTHRU_TC2_ENV,
		VSHADER_PASSTHRU_TC2_ENV,
		VSHADER_PASSTHRU_TC2_ENV
	},
	{
		VSHADER_oBASE_ADD_rbENV,
		VSHADER_oBASE_ADD_rbENV,
		VSHADER_oBASE_ADD_rbENV_BLEND,
		VSHADER_oBASE_ADD_rbENV_BLEND
	},
	{
		VSHADER_oBASE_ADD_rbENV_EMBM,
		VSHADER_oBASE_ADD_rbENV_EMBM,
		VSHADER_oBASE_ADD_rbENV_EMBM,
		VSHADER_oBASE_ADD_rbENV_EMBM
	},
	{
		VSHADER_ENV_POINT1_DIR1,
		VSHADER_ENV_POINT1_DIR1,
		VSHADER_ENV_POINT1_DIR1,
		VSHADER_ENV_POINT1_DIR1
	},
	{
		VSHADER_ENV_POINT2_DIR1,
		VSHADER_ENV_POINT2_DIR1,
		VSHADER_ENV_POINT2_DIR1,
		VSHADER_ENV_POINT2_DIR1
	},
	{
		VSHADER_ENV_POINT4_DIR1,
		VSHADER_ENV_POINT4_DIR1,
		VSHADER_ENV_POINT4_DIR1,
		VSHADER_ENV_POINT4_DIR1
	},
	{
		VSHADER_VCOLOR_tc,
		VSHADER_VCOLOR_tc,
		VSHADER_VCOLOR_tc_blend,
		VSHADER_VCOLOR_tc_blend
	},
	{
		VSHADER_POINT1_DIR1_tc,
		VSHADER_POINT1_DIR1_tc,
		VSHADER_POINT1_DIR1_tc_blend,
		VSHADER_POINT1_DIR1_tc_blend
	},
	{
		VSHADER_POINT2_DIR1_tc,
		VSHADER_POINT2_DIR1_tc,
		VSHADER_POINT2_DIR1_tc_blend,
		VSHADER_POINT2_DIR1_tc_blend
	},
	{
		VSHADER_POINT4_DIR1_tc,
		VSHADER_POINT4_DIR1_tc,
		VSHADER_POINT4_DIR1_tc_blend,
		VSHADER_POINT4_DIR1_tc_blend
	},
	{
		VSHADER_POINT6_DIR1_tc,
		VSHADER_POINT6_DIR1_tc,
		VSHADER_POINT6_DIR1_tc_blend,
		VSHADER_POINT6_DIR1_tc_blend
	},
	{
		VSHADER_POINT7_DIR1_tc,
		VSHADER_POINT7_DIR1_tc,
		VSHADER_POINT7_DIR1_tc_blend,
		VSHADER_POINT7_DIR1_tc_blend
	},
	{
		VSHADER_POINT8_tc,
		VSHADER_POINT8_tc,
		VSHADER_POINT8_tc_blend,
		VSHADER_POINT8_tc_blend
	},
	{
		VSHADER_PERPIXELPOINT1_tc,
		VSHADER_PERPIXELPOINT1_tc,
		VSHADER_PERPIXELPOINT1_tc_blend,
		VSHADER_PERPIXELPOINT1_tc_blend
	},
	{
		VSHADER_PERPIXELPOINT2_tc,
		VSHADER_PERPIXELPOINT2_tc,
		VSHADER_PERPIXELPOINT2_tc_blend,
		VSHADER_PERPIXELPOINT2_tc_blend
	},
	{
		VSHADER_PERPIXELSPOT1_tc,
		VSHADER_PERPIXELSPOT1_tc,
		VSHADER_PERPIXELSPOT1_tc_blend,
		VSHADER_PERPIXELSPOT1_tc_blend
	},
	{
		VSHADER_PERPIXELSPOT2_tc,
		VSHADER_PERPIXELSPOT2_tc,
		VSHADER_PERPIXELSPOT2_tc_blend,
		VSHADER_PERPIXELSPOT2_tc_blend
	},
	{
		VSHADER_PERPIXELSPOT1_BUMP_tc,
		VSHADER_PERPIXELSPOT1_BUMP_tc,
		VSHADER_PERPIXELSPOT1_BUMP_tc_blend,
		VSHADER_PERPIXELSPOT1_BUMP_tc_blend
	},
	{
		VSHADER_PERPIXELSPOT1_SPECBUMP_tc,
		VSHADER_PERPIXELSPOT1_SPECBUMP_tc,
		VSHADER_PERPIXELSPOT1_SPECBUMP_tc_blend,
		VSHADER_PERPIXELSPOT1_SPECBUMP_tc_blend
	},
	{
		VSHADER_PERPIXELPOINT1_BUMP_tc,
		VSHADER_PERPIXELPOINT1_BUMP_tc,
		VSHADER_PERPIXELPOINT1_BUMP_tc_blend,
		VSHADER_PERPIXELPOINT1_BUMP_tc_blend
	},
	{
		VSHADER_PERPIXELDIRECTIONAL1_SPECBUMP_tc,
		VSHADER_PERPIXELDIRECTIONAL1_SPECBUMP_tc,
		VSHADER_PERPIXELDIRECTIONAL1_SPECBUMP_tc_blend,
		VSHADER_PERPIXELDIRECTIONAL1_SPECBUMP_tc_blend
	},
	{
		VSHADER_DIRECTIONAL1_SPECULAR_tc,
		VSHADER_DIRECTIONAL1_SPECULAR_tc,
		VSHADER_DIRECTIONAL1_SPECULAR_tc_blend,
		VSHADER_DIRECTIONAL1_SPECULAR_tc_blend
	},
	{
		VSHADER_DETAIL_tc,
		VSHADER_DETAIL_2tc,
		VSHADER_DETAIL_tc_blend,
		VSHADER_DETAIL_2tc_blend
	},
	{
		VSHADER_oBASE_ADD_rbENV_DETAIL,
		VSHADER_oBASE_ADD_rbENV_DETAIL,
		VSHADER_oBASE_ADD_rbENV_DETAIL,
		VSHADER_oBASE_ADD_rbENV_DETAIL
	},
	{
		VSHADER_oBASE_ADD_rbSREFLECT,
		VSHADER_oBASE_ADD_rbSREFLECT,
		VSHADER_oBASE_ADD_rbSREFLECT,
		VSHADER_oBASE_ADD_rbSREFLECT
	},
	{
		VSHADER_oBASE_ADD_rbSREFLECT_EMBM,
		VSHADER_oBASE_ADD_rbSREFLECT_EMBM,
		VSHADER_oBASE_ADD_rbSREFLECT_EMBM,
		VSHADER_oBASE_ADD_rbSREFLECT_EMBM
	},
	{
		VSHADER_PLANAR_REFLECT_MESH,
		VSHADER_PLANAR_REFLECT_MESH,
		VSHADER_PLANAR_REFLECT_MESH,
		VSHADER_PLANAR_REFLECT_MESH
	}
};

u32 _anLMPShader[4][4] = 
{
	{
		PSHADER_LM1,
		PSHADER_LM2,
		PSHADER_LM3,
		PSHADER_LM4
	},
	{
		PSHADER_ZMASK_EMASK_LM1,
		PSHADER_ZMASK_EMASK_LM2,
		PSHADER_ZMASK_EMASK_LM2,
		PSHADER_ZMASK_EMASK_LM2
	},
	{
		PSHADER_ZMASK_LM1,
		PSHADER_ZMASK_LM2,
		PSHADER_ZMASK_LM3,
		PSHADER_ZMASK_LM3
	},
	{
		PSHADER_EMASK_LM1,
		PSHADER_EMASK_LM2,
		PSHADER_EMASK_LM3,
		PSHADER_EMASK_LM3
	}
};

u32 _anLMPShader_Trans[4][4] = 
{
	{
		PSHADER_PASSTHRU_LM1,
		PSHADER_PASSTHRU_LM2,
		PSHADER_PASSTHRU_LM3,
		PSHADER_PASSTHRU_LM3
	},
	{
		PSHADER_PASSTHRU_LM1,
		PSHADER_PASSTHRU_LM2,
		PSHADER_PASSTHRU_LM3,
		PSHADER_PASSTHRU_LM3
	},
	{
		PSHADER_PASSTHRU_LM1,
		PSHADER_PASSTHRU_LM2,
		PSHADER_PASSTHRU_LM3,
		PSHADER_PASSTHRU_LM3
	},
	{
		PSHADER_PASSTHRU_LM1,
		PSHADER_PASSTHRU_LM2,
		PSHADER_PASSTHRU_LM3,
		PSHADER_PASSTHRU_LM3
	}
};

u32 _anLMVShader[4][4] = 
{
	{
		VSHADER_1LMAP,
		VSHADER_2LMAP,
		VSHADER_3LMAP,
		VSHADER_4LMAP
	},
	{
		VSHADER_ZMASK_EMASK_LM1,
		VSHADER_ZMASK_EMASK_LM2,
		VSHADER_ZMASK_EMASK_LM2,
		VSHADER_ZMASK_EMASK_LM2
	},
	{
		VSHADER_ZMASK_LM1,
		VSHADER_ZMASK_LM2,
		VSHADER_ZMASK_LM2,
		VSHADER_ZMASK_LM2
	},
	{
		VSHADER_EMASK_LM1,
		VSHADER_EMASK_LM2,
		VSHADER_EMASK_LM2,
		VSHADER_EMASK_LM2
	}
};
