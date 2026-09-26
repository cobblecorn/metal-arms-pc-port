//////////////////////////////////////////////////////////////////////////////////////
// fsh.h - Fang shader module.
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

#ifndef _FSH_H_
#define _FSH_H_ 1

#include "fang.h"
#include "fdata.h"
#include "flight.h"
#include "ftex.h"
#include "fxfm.h"
#include "fshaders.h"

//#define FSH_NAME_LEN				31
#define FSH_MAX_TEX_LAYER_COUNT		16				// Maximum number of texture layers supported by the shader system

#define FSH_MAX_TEX_LAYERS				8
#define FSH_MAX_LIGHTMAPS				8
#define FSH_MAX_SURFACEMAPS				4

#define FSH_INVALID_SURFACE_SHADER		0xffff
#define FSH_INVALID_LIGHT_SHADER		0xff
#define FSH_INVALID_SPECULAR_SHADER		0xff

extern BOOL FSh_bSurfaceOnly;
extern BOOL FSh_bAlphaOut;
class CFColorRGB;

typedef enum 
{
	FSh_RENDER_REFLECTIONMAP=(1<<1),			//Render Reflection Maps
	FSh_RENDER_DETAIL=(1<<2),					//Render Detail Maps
	FSh_RENDER_BUMP=(1<<3),						//Render BumpMaps (only if multipass lighting is enabled)
	FSh_RENDER_MULTIPASS_LIGHTING=(1<<4),		//Allow multipass lighting (per-pixel lights)
	FSh_RENDER_MOTIF_LIGHTMAPS=(1<<5),			//Allow multiple lightmaps with motifs (if not enabled, surfaces will only use the base lightmap)
	
	FSh_RENDER_FORCE_FASTPASS=(1<<6),
	FSh_RENDER_FORCE_NOFASTPASS=(1<<7),
	FSh_RENDER_FORCEBASE_SHADER=(1<<8),			//Force oBASE_??? to oBASE, tBASE_??? to tBASE, cBASE_??? to cBASE, etc.

	FSh_RENDER_ALL=FSh_RENDER_REFLECTIONMAP|FSh_RENDER_DETAIL|FSh_RENDER_BUMP|		//Render at highest quality.
		FSh_RENDER_MULTIPASS_LIGHTING|FSh_RENDER_MOTIF_LIGHTMAPS
} FSh_ShaderFlags_e;

typedef enum {
	FRMODE_NORMAL=0,
	FRMODE_DIFFUSE,
	FRMODE_SURFACE,
	FRMODE_SPECULAR,
	
	FRMODE_MAX	
} FRenderMode_e;


typedef enum {
	// (Registers 0-127 are shader-specific input registers)

	// Registers 128-255 are global registers:
	FSHREG_GLOBAL_OFFSET = 128,

	FSHREG_AMBIENT = FSHREG_GLOBAL_OFFSET,		// Pointer to global ambient motif

	FSHREG_MOTIF_A0_C0,							// Pointer to constant motif: A=0.0f, C=(0.0f, 0.0f, 0.0f)
	FSHREG_MOTIF_A0_C1,							// Pointer to constant motif: A=0.0f, C=(1.0f, 1.0f, 1.0f)
	FSHREG_MOTIF_A1_C0,							// Pointer to constant motif: A=1.0f, C=(0.0f, 0.0f, 0.0f)
	FSHREG_MOTIF_A1_C1,							// Pointer to constant motif: A=1.0f, C=(1.0f, 1.0f, 1.0f)

	FSHREG_ONE_PAST_LAST,						// (used to compute FSHREG_GLOBAL_COUNT)
	FSHREG_GLOBAL_COUNT = FSHREG_ONE_PAST_LAST - FSHREG_GLOBAL_OFFSET
} FShReg_e;

//--Jeremy--//
typedef enum {
	SHADERTYPE_DIFFUSE=0,
	SHADERTYPE_SURFACE,
	SHADERTYPE_SPECULAR,
	SHADERTYPE_TRANSLUCENCY,
	SHADERTYPE_NTYPES
} FShaderType_e;
//


//
//
//
struct FShTexInst_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FShTexInst_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	CFTexInst TexInst;					// Texture instance
	u16 nTextureNameOffset;				// Offset to texture name - used for embedded FShTexInt_t's
	u8 nTexLayerID;						// Layer ID (used to provide the game code with a way to access a texture layer): 255=none
	u8 __PAD;							// Padding

	void ChangeEndian( void )
	{
		TexInst.ChangeEndian();
		nTextureNameOffset = fang_ConvertEndian( nTextureNameOffset );
		nTexLayerID = fang_ConvertEndian( nTexLayerID );
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Shader registers
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern FShaderType_e FSh_shaderType;
extern FSh_ShaderFlags_e FSh_shaderFlags;
extern u32 FSh_anDynamicSurfaceRegisters[FDATA_MAX_SURFACE_SHADER_REGS];
extern u32 FSh_anDynamicLightRegisters[FDATA_MAX_LIGHT_SHADER_REGS + (FSH_MAX_LIGHTMAPS * 3)];
extern const u32 *FSh_pnSurfaceInputRegisters;				// Pointer to array of input registers
extern const u32 *FSh_pnLightInputRegisters;				// Pointer to array of input registers
extern const u32 *FSh_pnLightMapInputRegisters;				// Pointer to array of input registers
FINLINE void fsh_SetRegisters( const u32 *pnSurfaceInputRegisters, const u32 *pnLightInputRegisters, const u32 *pnLightMapInputRegisters )
{
	FSh_pnLightInputRegisters = pnLightInputRegisters;
	FSh_pnSurfaceInputRegisters = pnSurfaceInputRegisters;
	FSh_pnLightMapInputRegisters = pnLightMapInputRegisters;
}

//
//
FINLINE void fsh_UseDynamicRegisters( void )
{
	fang_MemZero( FSh_anDynamicSurfaceRegisters, sizeof(u32) * FDATA_MAX_SURFACE_SHADER_REGS );
	fang_MemZero( FSh_anDynamicLightRegisters, sizeof(u32) * FDATA_MAX_LIGHT_SHADER_REGS );
	FSh_pnSurfaceInputRegisters = FSh_anDynamicSurfaceRegisters;
	FSh_pnLightInputRegisters = FSh_anDynamicLightRegisters;
}

//
//
FINLINE void fsh_SetSurfaceRegister( u32 regID, u32 InputRegister )
{
	FASSERT( regID >= 0 && regID < FDATA_MAX_SURFACE_SHADER_REGS );
	FSh_anDynamicSurfaceRegisters[regID] = InputRegister;
}

//
//
FINLINE void fsh_SetLightRegister( u32 regID, u32 InputRegister )
{
	FASSERT( regID >= 0 && regID < FDATA_MAX_LIGHT_SHADER_REGS );
	FSh_anDynamicLightRegisters[regID] = InputRegister;
}

//
//
FINLINE void fsh_SetShaderType( FShaderType_e shaderType )
{
	FSh_shaderType = shaderType;
}

FINLINE void fsh_SetShaderFlags( FSh_ShaderFlags_e shaderFlags )
{
	FSh_shaderFlags = shaderFlags;
}

//
//
FINLINE void fsh_SetupShaderRegisters( u32 *pnShLightRegisters, u32 *pnSurfaceRegisters, u32 *pnLightmapRegisters, CFColorMotif *pEmissiveOverride )
{
	if ( pEmissiveOverride )
	{
		// Copy over the base light registers
		fang_MemCopy( FSh_anDynamicLightRegisters, pnShLightRegisters, sizeof(u32) * ((FSHADERS_LIGHT_REG_LMCOUNT + 1) + (pnShLightRegisters[FSHADERS_LIGHT_REG_LMCOUNT] * 3)) );

		// Set the override
		FSh_anDynamicLightRegisters[FSHADERS_LIGHT_REG_EMOTIF] = (u32)pEmissiveOverride;

		// Set the registers
		fsh_SetRegisters( pnSurfaceRegisters, FSh_anDynamicLightRegisters, pnLightmapRegisters );
	}
	else
	{
		// Set the registers
		fsh_SetRegisters( pnSurfaceRegisters, pnShLightRegisters, pnLightmapRegisters );
	}
}

// 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


extern BOOL fsh_ModuleStartup( void );
extern void fsh_ModuleShutdown( void );

extern u32 fsh_GetGlobalRegister( FShReg_e nRegNum );
extern u32 fsh_SetGlobalRegister( FShReg_e nRegNum, u32 nNewValue );

extern void fsh_Open( void );
extern void fsh_Close( void );

extern void fsh_SetDepthBias( u32 nBias );
extern u32 fsh_GetDepthBias( void );

extern void fsh_TexCoordMtx_ResetList( void );
extern void fsh_TexCoordMtx_Add( u32 nTexLayerID, const CFMtx43 *pTexCoordMtx );
extern u32 fsh_TexCoordMtx_GetCount( void );
extern u32 fsh_TexCoordMtx_GetMaxCount( void );

extern void fsh_TexOverride_ResetList( void );
extern void fsh_TexOverride_Add( u32 nTexLayerID, CFTexInst *pTexInst );
extern u32 fsh_TexOverride_GetCount( void );
extern u32 fsh_TexOverride_GetMaxCount( void );

extern void fsh_Light_ResetList( void );
extern void fsh_Light_Add( CFLight *pLight, BOOL bVolumeMesh );
extern u32 fsh_Light_GetCount( void );
extern u32 fsh_Light_GetMaxCount( void );

extern void fsh_Fog_Enable( BOOL bEnable=TRUE, BOOL bForce=FALSE );
extern BOOL fsh_Fog_IsEnabled( void );

extern void fsh_Fog_SetParam(f32 fNearZ, f32 fFarZ, f32 fMinY, f32 fMaxY, f32 fDensity, CFColorRGB& FogClr, CFTexInst *pFogTex=NULL);

//Renders a quad over the screen, using the destination alpha as the depth for fog calculations.
extern void fsh_RenderFogPass();

//extern void fsh_Execute( FShHandle_t hShader, u32 nPassNum, const u32 *pnInputRegisters );
//--Jeremy--//
//Lighting shaders are base types and the actual shader used will vary depending on lighting conditions.
extern BOOL fsh_SetShader(u32 shaderID, u32 diffuseID, u32 specID, u32 passIdx=0); //surface, diffuse and specular shaders.
extern BOOL fsh_SetSurfaceShader(u32 shaderID);
extern void fsh_FastSurfaceExecute();
extern void fsh_ExecuteCurrent(BOOL bShadowRender=FALSE, s32 nShadowID=0); //execute current shader
FINLINE void fsh_SetSurfaceOnly(BOOL bSurfaceOnly) { FSh_bSurfaceOnly = bSurfaceOnly; }
//TRUE if alpha will be output to a texture else FALSE.
FINLINE void fsh_EnableAlphaOut(BOOL bAlphaOut) { FSh_bAlphaOut = bAlphaOut; }
//ARG - >>>>>
#if FANG_PLATFORM_PS2
extern void fsh_ClearCurrent( void );
#endif
//ARG - <<<<<
extern void fsh_EndExecute();
extern void fsh_SetPassIdx(u32 nPass);
#if FANG_PLATFORM_DX
extern void fsh_CheckVB();
#endif

//Configurable Shader Funcs - defaults are listed.
extern f32  FSh_fDetailTile;
extern f32  FSh_fBumpTile;
extern BOOL FSh_bInvAlpha_Emissive;
extern void fsh_SetDetailMapTileFactor(f32 fTileFactor=4.0f);
extern void fsh_SetBumpMapTileFactor(f32 fTileFactor=1.0f);
extern f32 fsh_GetDetailMapTileFactor( void );
extern f32 fsh_GetBumpMapTileFactor( void );

FINLINE void fsh_SetInvAlphaToggle_Emissive(BOOL bToggle)
{
	FSh_bInvAlpha_Emissive = bToggle;
}

FINLINE BOOL fsh_GetInvAlphaToggle_Emissive( void )
{
	return FSh_bInvAlpha_Emissive;
}


extern void fsh_RenderMode( FRenderMode_e renderMode );
extern void *fsh_GetAttenMap( void );
extern u32  fsh_GetNumDiffusePasses(u32 shaderID, u32 diffuseID, BOOL *pbFastPass=NULL);
extern u32  fsh_GetNumSpecularPasses(u32 shaderID, u32 specularID);
extern u32  fsh_GetCurrentNumPasses(BOOL *pbFastPass=NULL);
extern void fsh_SetReflectTex(CFTexInst *pTex);
extern void fsh_CreateSReflectTarget( void ); 
extern void fsh_ClearSReflectTargets( void );
extern void fsh_ResetSReflect( void );
extern void fsh_ResetShaderCache( void );
extern CFTexInst *fsh_GetSReflectTarget(f32 fSize, void *pUser);
extern void fsh_CreateSReflectTarget(void *pUserData);

extern void fsh_PrepareFrame( void );

extern void fsh_DynamicSReflect_Callback(void *pUserData);

extern void fsh_ActivateFullScrTargets(BOOL bActivate);

enum 
{ 
	RP_INVALID=0,
	RP_REFLECT=1,
	RP_REFRACT=2,
	RP_TEXTURE=4,
	RP_EMISSIVE=8,
	MAX_RENDER_PLANES = 10 
};

//Full screen texture access.

extern CFVec3A FSh_PlayerPos;

extern CFTexInst *fsh_GetFullScrTexture();
extern u16 *fsh_GetFullScrData();
extern void fsh_ReleaseFullScrData();

extern void fsh_LoadProjMtx(f32 fovX, f32 fovY);
extern void fsh_ClearRenderPlanes();
extern u32 fsh_AddRenderPlane(CFVec3 *pQuad, u32 nType, BOOL bFlip=FALSE, CFColorRGB *fogClr=NULL, f32 fFogDensity=0.0f, f32 fGeoCullDist=100.0f, f32 fOpacity=1.0f, f32 fTile=4.0f, f32 fRadius=1.0f, CFTexInst *pLayer0=NULL, CFTexInst *pLayer1=NULL);
extern void fsh_ChangeRenderPlane(u32 nPlaneID, CFVec3 *pQuad, CFColorRGB *fogClr=NULL, f32 fFogDensity=0.0f);
extern void fsh_RenderPlane(u32 nPlaneID, f32 fScaleU=1.0f, f32 fScaleV=1.0f, f32 fScrollU=0.0f, f32 fScrollV=0.0f, CFTexInst *pEMBM=NULL, f32 fGlowScale=1.0f, f32 fBumpTile=1.0f);
extern void fsh_ActivateRenderPlane(u32 nPlaneID, BOOL bActive);

extern void fsh_DrawLiquidMesh(u32 nType, u16 nVtx, u16 nPrim, void *pMesh, u16 *pIdx, CFColorRGB *pClr, f32 fOpacity, CFVec3& vFwd, CFTexInst *pLayer0, CFTexInst *pLayer1, CFTexInst *pEMBM);

void fsh_ResetFullScrRenderTarget();

FINLINE void fsh_RegisterPlayerPos(CFVec3A& vPos)
{
	FSh_PlayerPos = vPos;
}

//

extern void fsh_SetLayerAlpha(f32 fAlpha);
extern void fsh_SetSurfaceColor(f32 fRed, f32 fGreen, f32 fBlue, f32 fAlpha);
extern void fsh_ZWriteEnable(BOOL bEnable=TRUE);

extern CFLight *FSh_pSpecularLight;
FINLINE void fsh_SetSpecularLight( CFLight *pLight )
{
	FSh_pSpecularLight = pLight;
	if ( pLight->m_nXfmKey_VS != FXfm_nViewKey )
	{
		FSh_pSpecularLight->m_vUnitDir_VS = FXfm_pView->m_MtxF.m44.MultDir( pLight->m_mtxOrientation_WS.m_vFront );
	}
}

extern void *FSh_pProcedural;
FINLINE void fsh_SetProcedural( void *pProcedural )
{
	FSh_pProcedural = pProcedural;
}

extern BOOL FSh_bUseClipPlane;

#endif

