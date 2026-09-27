//////////////////////////////////////////////////////////////////////////////////////
// fRenderSort.cpp - Fang module for managing the ordering of render
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
// 10/14/02	Lafleur		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "Fang.h"
#include "fRenderSort.h"
#include "fsh.h"
#include "fvis.h"
#include "fmesh.h"
#include "ftext.h"
#include "fperf.h"
#include "fclib.h"
#include "fworld.h"
#include "fshaders.h"
#include "frenderer.h"
#include "fshadow.h"
#if FANG_PLATFORM_GC
	#include "gc\fgc.h"
	#include "gc\fgcsh.h"
#endif
#if FANG_PLATFORM_DX
	#include "dx\fdx8tex.h"
	#include "dx\fdx8xfm.h"
	#include "dx\fdx8vb.h"
#endif
//ARG - >>>>>
#if FANG_PLATFORM_PS2
	#include "ps2\fps2xfm.h"
#endif
//ARG - <<<<<


//////////////////////////////////////////////////////////////////////////////////////
// External Dependencies:
//////////////////////////////////////////////////////////////////////////////////////

extern u32 FMesh_nLastSurfaceMaterialCRC;


//////////////////////////////////////////////////////////////////////////////////////
// Local Defines:
//////////////////////////////////////////////////////////////////////////////////////

#define _RL_SURFACE_HASH_SIZE					32	// (must be a power of 2)

#define _DISPLAY_MATERIAL_CULLING_STATS			FALSE

#define _USE_LINKLIST_TRANSLUCENCY_ORDERING		FALSE

#define _USE_FPS_TO_ADJUST_MESH_LOD				FALSE

#define _RENDER_FOG								TRUE

//ARG - >>>>>
#if	FANG_PLATFORM_PS2
#define	_RENDER_LIGHTING_PASS					FALSE
#define _RENDER_SHADOW_PASS						FALSE
#define	_RENDER_SURFACE_PASS					TRUE
#define	_RENDER_SPECULAR_PASS					FALSE
#else
//ARG - <<<<<
#define	_RENDER_LIGHTING_PASS					(TRUE || FANG_PRODUCTION_BUILD)
#define _RENDER_SHADOW_PASS						(TRUE || FANG_PRODUCTION_BUILD)
#define	_RENDER_SURFACE_PASS					(TRUE || FANG_PRODUCTION_BUILD)
#define	_RENDER_SPECULAR_PASS					FALSE//(FALSE || FANG_PRODUCTION_BUILD)    The artists decided they will not use this
#endif	//ARG

#define	_RENDER_TRANSLUCENT_MATERIALS			(TRUE || FANG_PRODUCTION_BUILD)

#define _RENDER_VOLUME_GEOMETRY					(TRUE || FANG_PRODUCTION_BUILD)
#define _RENDER_NON_VOLUME_GEOMETRY				(TRUE || FANG_PRODUCTION_BUILD)
#define	_RENDER_POINT_SPRITES					(TRUE || FANG_PRODUCTION_BUILD)

enum
{
	_RL_ENTRY_FLAGS_NONE			= 0x0000,
	_RL_ENTRY_FLAGS_MESH_INST		= 0x0001,
	_RL_ENTRY_FLAGS_VOLUME_MESH		= 0x0002,
	_RL_ENTRY_FLAGS_POINT_SPRITES	= 0x0004,
	
	_RL_ENTRY_FLAGS_IN_USE			= 0x8000,
	
};

enum
{
	_RL_TYPE_OPAQUE_AND_CUTOUT = 0,
	_RL_TYPE_TRANSLUCENT,
	
};

typedef enum
{
	// Non-translucent entries must be listed first
	_RL_ENTRY_TYPE_NON_WORLD_SPACE = 0,
	_RL_ENTRY_TYPE_WORLD_SPACE,
	_RL_ENTRY_TYPE_NON_TRANSLUCENT_COUNT,	// Modifications to the count will require further modifications, below
	
	// Translucent types
	_RL_ENTRY_TYPE_POINT_SPRITE = _RL_ENTRY_TYPE_NON_TRANSLUCENT_COUNT,
	
} _EntryType_e;


//////////////////////////////////////////////////////////////////////////////////////
// Global variables:
//////////////////////////////////////////////////////////////////////////////////////

#if FANG_WINGC
u32 _nPortRSFrame = 0;	// Port diagnostic frame counter
#endif

BOOL FRS_bRenderFlags = FRS_RENDER_NONE
		#if _RENDER_LIGHTING_PASS
			| FRS_RENDER_PASS_LIGHTING
		#endif
		#if _RENDER_SURFACE_PASS
			| FRS_RENDER_PASS_SURFACE
		#endif
		#if _RENDER_SHADOW_PASS
			| FRS_RENDER_SHADOWS
		#endif
		#if _RENDER_SPECULAR_PASS
			| FRS_RENDER_PASS_SPECULAR
		#endif
		#if _RENDER_VOLUME_GEOMETRY
			| FRS_RENDER_VOLUME_GEO
		#endif
		#if _RENDER_NON_VOLUME_GEOMETRY
			| FRS_RENDER_OBJECT_GEO
		#endif
		#if _RENDER_POINT_SPRITES
			| FRS_RENDER_SPRITES
		#endif
		#if _RENDER_TRANSLUCENT_MATERIALS
			| FRS_RENDER_TRANSLUCENCIES
		#endif
			| FRS_RENDER_NONE;

//////////////////////////////////////////////////////////////////////////////////////
// Local Structures:
//////////////////////////////////////////////////////////////////////////////////////

//
//
struct _RL_Entry_t
{
	u16						nFlags;
	u8 						nCrossesPlanesMask;	// Which frustum planes it crosses
	u8						nLODIndex;
	
	union
	{
		CFMeshInst 			*pMeshInst;
		CFWorldPSGroup		*pPSGroup;
	};
	
	FMeshMaterial_t 		*pMaterial;
	f32 					fDistToCam;			// Distance from this object's origin to the camera

	// Mirror Info
	CFMtx43A 				*pMirrorMtx;

	// Linklist info	
	union
	{
		_RL_Entry_t				*pFreeNext;		// If not in a list, then this is the free list link
		_RL_Entry_t				*pLightNext;	
	};
	_RL_Entry_t				*pSurfaceNext;
	_RL_Entry_t				*pSpecularNext;
	_RL_Entry_t				*pShadowsNext;
};

//
//
struct _RL_PointSpriteEntry_t
{
	CFWorldPSGroup			*pPSGroup;
	FViewportPlanesMask_t 	nCrossesPlanesMask;	// Which frustum planes it crosses
	f32 					fDistToCam;			// Distance from this object's origin to the camera, squared

	// Mirror Info
	CFMtx43A 				*pMirrorMtx;
	
};

//
//
struct _SurfaceRouter_t 
{
	u16 	nListType;	// What type list this shader should be routed to
	u16		nListIdx;	// What index in the list to use for this shader
	
};

//
//
struct _LightingRouter_t
{
	u8		nListIdx;	// What index in the list to use for this shader
	
};

//
//
struct _SpecularRouter_t
{
	u8		nListIdx;	// What index in the list to use for this shader
	
};

//
//
struct _RL_LinkList_t
{
	_RL_Entry_t	*pHead;
	_RL_Entry_t	*pTail;
	u32			nCount;
};


//////////////////////////////////////////////////////////////////////////////////////
// Local Variables:
//////////////////////////////////////////////////////////////////////////////////////

//
//
static u8 _nLightShaderOrder[] = 
{
	FSHADERS_FULLBRIGHT,
	FSHADERS_VLIGHT_BASIC,
	FSHADERS_VLIGHT_MASK_EMISSIVE,
	// Begin cutout shaders
	FSHADERS_FULLBRIGHT_CO,
	FSHADERS_VLIGHT_BASIC_CO,
	FSHADERS_VLIGHT_MASK_EMISSIVE_CO,
	FSH_INVALID_LIGHT_SHADER,
};
#define _FIRST_CUTOUT_LIGHT_SHADER			FSHADERS_FULLBRIGHT_CO

//
//
static u8 _nSpecularShaderOrder[] = 
{
	FSHADERS_VSPEC_BASIC,
	FSHADERS_VSPEC_MASK,
	FSHADERS_VSPEC_BASIC_CO,
	FSHADERS_VSPEC_BASIC_MASK_CO,
	FSH_INVALID_SPECULAR_SHADER,
};

//
//
static u16 _nSurfaceShaderOrder[] = 
{
	FSHADERS_oBASE,
	FSHADERS_oBASE_LERP_tLAYER,
	FSHADERS_oBASE_LERP_vLAYER,
	FSHADERS_oBASE_LERP_pLAYER,
	FSHADERS_oBASE_ADD_rbENV,
	FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV,
	FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV,
	FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV,
#if FSH_DYNAMIC_SREFLECT
	FSHADERS_oBASE_ADD_rbSREFLECT,
#endif

	FSHADERS_LIQUID_MOLTEN_1LAYER,
	FSHADERS_LIQUID_MOLTEN_2LAYER,
	
	// Detail Mapping Shaders.
	FSHADERS_oBASE_DETAIL,
	FSHADERS_oBASE_LERP_tLAYER_DETAIL,
	FSHADERS_oBASE_LERP_vLAYER_DETAIL,
	FSHADERS_oBASE_LERP_pLAYER_DETAIL,
	FSHADERS_oBASE_ADD_rbENV_DETAIL,
	FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV_DETAIL,
	FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV_DETAIL,
	FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV_DETAIL,
    		
	// Begin cutout shaders
	FSHADERS_cBASE,
	FSHADERS_cBASE_LERP_tLAYER,
	FSHADERS_cBASE_LERP_vLAYER,
	FSHADERS_cBASE_LERP_pLAYER,
	
	//Detail Mapping Shaders
	FSHADERS_cBASE_DETAIL,
	FSHADERS_cBASE_LERP_tLAYER_DETAIL,
	FSHADERS_cBASE_LERP_vLAYER_DETAIL,
	FSHADERS_cBASE_LERP_pLAYER_DETAIL,
		
	FSH_INVALID_SURFACE_SHADER,
};
//#define _FIRST_CUTOUT_SURFACE_SHADER		FSHADERS_cBASE
#define _FIRST_CUTOUT_SURFACE_SHADER_IDX 10

// Entry lists
static _RL_Entry_t *_paDLEntries;
static _RL_Entry_t *_pFreeDLEntries;

// Cutout/opaque Render linklists
static _RL_LinkList_t _RL_Lighting[FSHADERS_DIFFUSE_ROUTE][_RL_ENTRY_TYPE_NON_TRANSLUCENT_COUNT];
static _RL_LinkList_t _RL_Shadows[1][_RL_ENTRY_TYPE_NON_TRANSLUCENT_COUNT];
static _RL_LinkList_t _RL_Specular[FSHADERS_SPECULAR_COUNT][_RL_ENTRY_TYPE_NON_TRANSLUCENT_COUNT];
static _RL_LinkList_t *_RL_paSurface[FSHADERS_SHADER_COUNT][_RL_ENTRY_TYPE_NON_TRANSLUCENT_COUNT];

// Point Sprite list
static _RL_PointSpriteEntry_t 	*_paPointSpriteList;
static u16						_nMaxPointSpriteCount;
static u16						_nPointSpriteCount;

// Sorted list
#if _USE_LINKLIST_TRANSLUCENCY_ORDERING
	static _RL_Entry_t 	*_papTranslucentList2;
#endif
static _RL_Entry_t 	**_papTranslucentList;
static u16			_nMaxTranslucentMaterialCount;
static u16			_nTranslucentMaterialCount;

static _SurfaceRouter_t _SurfaceRouter[FSHADERS_SHADER_COUNT];
static _LightingRouter_t _LightingRouter[FSHADERS_DIFFUSE_ROUTE];
static _LightingRouter_t _SpecularRouter[FSHADERS_SPECULAR_COUNT];

static u32 _nMaxDLEntries;
static u32 _nUsedDLEntryCount;

#if !FANG_PRODUCTION_BUILD
	static u32 _nHighestUsedDLEntryCount;
#endif

static BOOL _bModuleInitialized;

static BOOL _bWarningIssued;

static u32  _nLODBias;

// For tracking materials rejected as outside frustrum
static u32 _nVolMatsConsidered = 0;
static u32 _nVolMatsTested = 0;
static u32 _nVolMatsOutsideFrustrum = 0;
static u32 _nVolMatsClippingRemoved = 0;

// Render sort callbacks
static FRenderSortCallback_t *_aRenderSortCallbacks[FRS_MAX_CALLBACK_TYPES];


//////////////////////////////////////////////////////////////////////////////////////
// Static functions:
//////////////////////////////////////////////////////////////////////////////////////

FINLINE void _RL_ClearDisplayLists( void );
FINLINE _RL_Entry_t* _GetDLEntry( void );
FINLINE BOOL _AddDLEntry( _RL_Entry_t *pNewEntry, u32 nLightShaderIdx, u32 nSurfaceShaderIdx, u32 nSpecularShaderIdx, _EntryType_e nEntryType );
FINLINE BOOL _AddDLEntry_Shadow( _RL_Entry_t *pNewEntry, _EntryType_e nEntryType );

//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
BOOL frs_ModuleStartup( void )
{
	u32 i, ii;

#if !FANG_PRODUCTION_BUILD
	_nHighestUsedDLEntryCount = 0;
#endif
	_nUsedDLEntryCount = 0;
	_pFreeDLEntries = NULL;
#if _USE_LINKLIST_TRANSLUCENCY_ORDERING
	_papTranslucentList2 = NULL;
#endif

	FRS_bRenderFlags = FRS_RENDER_NONE;
#if _RENDER_LIGHTING_PASS
	FRS_bRenderFlags |= FRS_RENDER_PASS_LIGHTING;
#endif
#if _RENDER_SHADOW_PASS
	FRS_bRenderFlags |= FRS_RENDER_SHADOWS;
#endif
#if _RENDER_SURFACE_PASS
	FRS_bRenderFlags |= FRS_RENDER_PASS_SURFACE;
#endif
#if _RENDER_SPECULAR_PASS
	FRS_bRenderFlags |= FRS_RENDER_PASS_SPECULAR;
#endif
#if _RENDER_VOLUME_GEOMETRY
	FRS_bRenderFlags |= FRS_RENDER_VOLUME_GEO;
#endif
#if _RENDER_NON_VOLUME_GEOMETRY
	FRS_bRenderFlags |= FRS_RENDER_OBJECT_GEO;
#endif
#if _RENDER_POINT_SPRITES
	FRS_bRenderFlags |= FRS_RENDER_SPRITES;
#endif
#if _RENDER_TRANSLUCENT_MATERIALS
	FRS_bRenderFlags |= FRS_RENDER_TRANSLUCENCIES;
#endif

	_bModuleInitialized = FALSE;

	// Clear the render sort callbacks
	for ( i = 0; i < FRS_MAX_CALLBACK_TYPES; i++ )
	{
		_aRenderSortCallbacks[i] = NULL;
	}
	
	// Build the surface shader router
	for ( i = 0; i < FSHADERS_SHADER_COUNT; i++ )
	{
		if ( FShaders_aShaderRegs[i].nSurfaceTypeFlags & FSHADERS_SURFACE_FLAG_TRANSLUCENT )
		{
			_SurfaceRouter[i].nListType = _RL_TYPE_TRANSLUCENT;
			_SurfaceRouter[i].nListIdx = 0xffff;
		}
		else
		{
			_SurfaceRouter[i].nListType = _RL_TYPE_OPAQUE_AND_CUTOUT;
			
			ii = 0;
			while ( _nSurfaceShaderOrder[ii] != FSH_INVALID_SURFACE_SHADER )
			{
				if ( _nSurfaceShaderOrder[ii] == i )
				{
					_SurfaceRouter[i].nListIdx = (u16)ii;
					break;
				}
				ii++;
			}
		}
	}

	// Build the lighting shader router
	for ( i = 0; i < FSHADERS_DIFFUSE_ROUTE; i++ )
	{
		_LightingRouter[i].nListIdx = 0xff;
		
		ii = 0;
		while ( _nLightShaderOrder[ii] != FSH_INVALID_LIGHT_SHADER )
		{
			if ( _nLightShaderOrder[ii] == i )
			{
				_LightingRouter[i].nListIdx = (u8)ii;
				break;
			}
			ii++;
		}
		
		FASSERT( _LightingRouter[i].nListIdx != 0xff );
	}

	// Build the specular shader router
	for ( i = 0; i < FSHADERS_SPECULAR_COUNT; i++ )
	{
		_SpecularRouter[i].nListIdx = 0xff;

		ii = 0;
		while ( _nLightShaderOrder[ii] != FSH_INVALID_SPECULAR_SHADER )
		{
			if ( _nLightShaderOrder[ii] == i )
			{
				_SpecularRouter[i].nListIdx = (u8)ii;
				break;
			}
			ii++;
		}
		
		FASSERT( _SpecularRouter[i].nListIdx != 0xff );
	}

#if !FANG_PRODUCTION_BUILD		
	_bWarningIssued = FALSE;
#endif

	// Allocate display list entries:
	_nMaxDLEntries = Fang_ConfigDefs.nWorld_MaxMeshesPerDraw * 4;
	if ( _nMaxDLEntries < 1 ) 
	{
		DEVPRINTF( "frs_ModuleStartup(): Fang_ConfigDefs.nWorld_MaxMeshesPerDraw is too small.\n" );
		DEVPRINTF( "                        Max meshes per draw set to 1.\n" );
		_nMaxDLEntries = 1;
	}
	_paDLEntries = (_RL_Entry_t *)fres_AllocAndZero( _nMaxDLEntries * sizeof(_RL_Entry_t) );
	if ( _paDLEntries == NULL ) 
	{
		DEVPRINTF( "frs_ModuleStartup(): Insufficient memory to allocate %u bytes for display list entry buffer.\n", _nMaxDLEntries * sizeof(_RL_Entry_t) );
		return FALSE;
	}
	
	// Allocate point sprite list entries:
	_nMaxPointSpriteCount = 200;
	if ( _nMaxPointSpriteCount < 1 ) 
	{
		DEVPRINTF( "frs_ModuleStartup(): Fang_ConfigDefs.nWorld_MaxPointSpritesPerDraw is too small.\n" );
		DEVPRINTF( "                        Max point sprites per draw set to 1.\n" );
		_nMaxPointSpriteCount = 1;
	}
	_paPointSpriteList = (_RL_PointSpriteEntry_t *)fres_AllocAndZero( _nMaxPointSpriteCount * sizeof(_RL_PointSpriteEntry_t) );
	if ( _paPointSpriteList == NULL ) 
	{
		DEVPRINTF( "frs_ModuleStartup(): Insufficient memory to allocate %u bytes for display list entry buffer.\n", _nMaxPointSpriteCount * sizeof(_RL_PointSpriteEntry_t) );
		return FALSE;
	}
	_nPointSpriteCount = 0;
	
	// Allocate translucent list:
	_nMaxTranslucentMaterialCount = (u16)_nMaxDLEntries;
	_papTranslucentList = (_RL_Entry_t **)fres_AllocAndZero( _nMaxTranslucentMaterialCount * sizeof(_RL_Entry_t*) );
	if ( _papTranslucentList == NULL ) 
	{
		DEVPRINTF( "frs_ModuleStartup(): Insufficient memory to allocate %u bytes for translucent display list entry buffer.\n", _nMaxTranslucentMaterialCount * sizeof(_RL_Entry_t*) );
		return FALSE;
	}
	fang_MemSet( _papTranslucentList, 0, sizeof( _RL_Entry_t* ) * _nMaxTranslucentMaterialCount );
	_nTranslucentMaterialCount = 0;
	
	for ( i = 0; i < FSHADERS_SHADER_COUNT; i++ )
	{
		for ( ii = 0; ii < _RL_ENTRY_TYPE_NON_TRANSLUCENT_COUNT; ii++ )
		{
			_RL_paSurface[i][ii] = (_RL_LinkList_t *)fres_AllocAndZero( _RL_SURFACE_HASH_SIZE * sizeof(_RL_LinkList_t) );
			if ( _RL_paSurface[i][ii] == NULL )
			{
				DEVPRINTF( "frs_ModuleStartup(): Insufficient memory to allocate %u bytes for surface shader linklists.\n", _RL_SURFACE_HASH_SIZE * sizeof(_RL_LinkList_t) );
				return FALSE;
			}
		}
	}
	
	_bModuleInitialized = TRUE;
	
	_RL_ClearDisplayLists();

	_nLODBias = 0;
	
	return TRUE;
}


//
//
//
void frs_ModuleShutdown( void )
{
	_bModuleInitialized = FALSE;

#if !FANG_PRODUCTION_BUILD		
	DEVPRINTF( "FDISPLAYLIST - Highest Entry Count: %d (%3.1f of total entries)\n", _nHighestUsedDLEntryCount, (f32)_nHighestUsedDLEntryCount / (f32)_nMaxDLEntries );
#endif	
}


//
//
//
FRenderSortCallback_t* frs_RegisterRenderCallback( FRS_RenderCallbackType_e nType, FRenderSortCallback_t *pCallback )
{
	FASSERT( nType == 0 || nType < FRS_MAX_CALLBACK_TYPES );

	FRenderSortCallback_t *pPrior = _aRenderSortCallbacks[nType];
	_aRenderSortCallbacks[nType] = pCallback;

	return pPrior;
}


//
//
//
u32 frs_GenerateMaterialDLHash( FMeshMaterial_t *pMaterial )
{
	u32 nHashKey = 0;
	
	FASSERT( pMaterial && pMaterial->pnShSurfaceRegisters );

	u32 i;
	u32 *pnRegisters = pMaterial->pnShSurfaceRegisters;
	for ( i = 0; i < FShaders_aShaderRegs[pMaterial->nSurfaceShaderIdx].nRegisterCount; i++ )
	{
		switch ( FShaders_aShaderRegs[pMaterial->nSurfaceShaderIdx].anRegType[i] )
		{
			case FSHADERS_REG_ENV_MOTIF:
			{
				CFColorMotif *pMotif = (CFColorMotif *)(*pnRegisters);
				if ( !pMotif )
				{
					break;
				}

				nHashKey = fmath_Crc32( nHashKey, (u8 *)pMotif, sizeof( CFColorMotif ) );

				break;
			}
	
			case FSHADERS_REG_LAYER0:
			case FSHADERS_REG_LAYER1:
			case FSHADERS_REG_LAYER2:
			case FSHADERS_REG_LAYER3:
			case FSHADERS_REG_DETAILMAP:
			{
				FShTexInst_t *pShTexInst = (FShTexInst_t *)(*pnRegisters);
				if ( !pShTexInst )
				{
					break;
				}

				FTexDef_t *pTexDef = pShTexInst->TexInst.GetTexDef();
				if ( !pTexDef )
				{	
					break;
				}

				nHashKey = fmath_Crc32( nHashKey, (u8 *)&pTexDef->TexInfo, sizeof( FTexInfo_t ) );

				break;
			}

			case FSHADERS_REG_TC0:
			case FSHADERS_REG_TC1:
			case FSHADERS_REG_TC2:
			case FSHADERS_REG_TC3:
			case FSHADERS_REG_DETAILMAP_TILE_FACTOR:
					nHashKey = fmath_Crc32( nHashKey, (u8 *)pnRegisters, 4 );
				break;

			default:
				break;
		}

		pnRegisters++;
	}
/*	
	// The first register should always be a shtexinst:
	FShTexInst_t *pShTexInst = (FShTexInst_t *)pMaterial->pnShSurfaceRegisters[FSHADERS_REG_LAYER0];
	
	FASSERT( pShTexInst );

	// Get the texdef
	FTexDef_t *pTexDef = pShTexInst->TexInst.GetTexDef();
	if ( !pTexDef )
	{	
		return 0;
	}
	
	// Generate a hash key based on the texture name
	u32 i, nLen = fclib_strlen( pTexDef->TexInfo.szName );
	for ( i = 0; i < nLen; i++ )
	{
		nHashKey += ( pTexDef->TexInfo.szName[i] << (8 *(i % 4)) );
	}
*/
	return nHashKey;
}


#if FANG_DEBUG_BUILD
//
// This function checks the base texture names of two materials to see if they are the same
//
static BOOL _VerifyHashKeys( FMeshMaterial_t *pMaterial1, FMeshMaterial_t *pMaterial2 )
{
	FASSERT( pMaterial1 && pMaterial1->pnShSurfaceRegisters );

	// The first register should always be a shtexinst:
	FShTexInst_t *pShTexInst = (FShTexInst_t *)pMaterial1->pnShSurfaceRegisters[FSHADERS_REG_LAYER0];
	
	FASSERT( pShTexInst );
	
	FTexDef_t *pTexDef1 = pShTexInst->TexInst.GetTexDef();

	if ( !pTexDef1 )
	{	
		return FALSE;
	}
	
	FASSERT( pMaterial2 && pMaterial2->pnShSurfaceRegisters );

	// The first register should always be a shtexinst:
	pShTexInst = (FShTexInst_t *)pMaterial2->pnShSurfaceRegisters[FSHADERS_REG_LAYER0];
	
	FASSERT( pShTexInst );
	
	FTexDef_t *pTexDef2 = pShTexInst->TexInst.GetTexDef();

	if ( !pTexDef2 )
	{	
		return FALSE;
	}
	
	if ( !fclib_strcmp( pTexDef1->TexInfo.szName, pTexDef2->TexInfo.szName ) )
	{
		return TRUE;
	}
	
	DEVPRINTF( "fdisplaylist.cpp - WARNING! - Materials with non-matching textures have the same hash key.\n" );
	return FALSE;	
}
#endif

//
// Retrieves a free display list entry
//
FINLINE _RL_Entry_t* _GetDLEntry( void )
{
	FASSERT( _bModuleInitialized );
	
	if ( !_pFreeDLEntries )
	{
		return NULL;
	}

	_RL_Entry_t *pReturn = _pFreeDLEntries;
	_pFreeDLEntries = _pFreeDLEntries->pFreeNext;	
	
	_nUsedDLEntryCount++;
#if !FANG_PRODUCTION_BUILD
	if ( _nUsedDLEntryCount > _nHighestUsedDLEntryCount )
	{
		_nHighestUsedDLEntryCount = _nUsedDLEntryCount;
	}
#endif

	pReturn->nFlags = _RL_ENTRY_FLAGS_IN_USE;
	
	return pReturn;
}


//
// Returns a display list entry to the free list
//
FINLINE void _ReleaseDLEntry( _RL_Entry_t *pEntry )
{
	FASSERT( _bModuleInitialized );
	FASSERT( pEntry );

	pEntry->nFlags = _RL_ENTRY_FLAGS_NONE;
	
	pEntry->pFreeNext = _pFreeDLEntries;
	_pFreeDLEntries = pEntry;

	_nUsedDLEntryCount--;
}


//
//
//
FINLINE void _RL_ClearDisplayLists( void )
{
	FASSERT( _bModuleInitialized );
	u32 i, ii;
	
	// Return any used entries to the pool
	_pFreeDLEntries = NULL;
	for ( i = 0; i < _nMaxDLEntries; i++ )
	{
		_paDLEntries[i].pFreeNext = _pFreeDLEntries;
		_pFreeDLEntries = &_paDLEntries[i];
	}
	_nUsedDLEntryCount = 0;
	
	// Clear out the linklists
	for ( i = 0; i < FSHADERS_DIFFUSE_ROUTE; i++ )
	{
		_RL_Lighting[i][_RL_ENTRY_TYPE_NON_WORLD_SPACE].pHead = NULL;
		_RL_Lighting[i][_RL_ENTRY_TYPE_NON_WORLD_SPACE].pTail = NULL;
		_RL_Lighting[i][_RL_ENTRY_TYPE_NON_WORLD_SPACE].nCount = 0;
		
		_RL_Lighting[i][_RL_ENTRY_TYPE_WORLD_SPACE].pHead = NULL;
		_RL_Lighting[i][_RL_ENTRY_TYPE_WORLD_SPACE].pTail = NULL;
		_RL_Lighting[i][_RL_ENTRY_TYPE_WORLD_SPACE].nCount = 0;
	}
	
	_RL_Shadows[0][_RL_ENTRY_TYPE_NON_WORLD_SPACE].pHead = NULL;
	_RL_Shadows[0][_RL_ENTRY_TYPE_NON_WORLD_SPACE].pTail = NULL;
	_RL_Shadows[0][_RL_ENTRY_TYPE_NON_WORLD_SPACE].nCount = 0;
		
	_RL_Shadows[0][_RL_ENTRY_TYPE_WORLD_SPACE].pHead = NULL;
	_RL_Shadows[0][_RL_ENTRY_TYPE_WORLD_SPACE].pTail = NULL;
	_RL_Shadows[0][_RL_ENTRY_TYPE_WORLD_SPACE].nCount = 0;
	
	for ( i = 0; i < FSHADERS_SHADER_COUNT; i++ )
	{
		for ( ii = 0; ii < _RL_SURFACE_HASH_SIZE; ii++ )
		{
			_RL_paSurface[i][_RL_ENTRY_TYPE_NON_WORLD_SPACE][ii].pHead = NULL;
			_RL_paSurface[i][_RL_ENTRY_TYPE_NON_WORLD_SPACE][ii].pTail = NULL;
			_RL_paSurface[i][_RL_ENTRY_TYPE_NON_WORLD_SPACE][ii].nCount = 0;
		}
		
		for ( ii = 0; ii < _RL_SURFACE_HASH_SIZE; ii++ )
		{
			_RL_paSurface[i][_RL_ENTRY_TYPE_WORLD_SPACE][ii].pHead = NULL;
			_RL_paSurface[i][_RL_ENTRY_TYPE_WORLD_SPACE][ii].pTail = NULL;
			_RL_paSurface[i][_RL_ENTRY_TYPE_WORLD_SPACE][ii].nCount = 0;
		}
	}
	
	for ( i = 0; i < FSHADERS_SPECULAR_COUNT; i++ )
	{
		_RL_Specular[i][_RL_ENTRY_TYPE_NON_WORLD_SPACE].pHead = NULL;
		_RL_Specular[i][_RL_ENTRY_TYPE_NON_WORLD_SPACE].pTail = NULL;
		_RL_Specular[i][_RL_ENTRY_TYPE_NON_WORLD_SPACE].nCount = 0;
		
		_RL_Specular[i][_RL_ENTRY_TYPE_WORLD_SPACE].pHead = NULL;
		_RL_Specular[i][_RL_ENTRY_TYPE_WORLD_SPACE].pTail = NULL;
		_RL_Specular[i][_RL_ENTRY_TYPE_WORLD_SPACE].nCount = 0;
	}
	
#if _USE_LINKLIST_TRANSLUCENCY_ORDERING
	_papTranslucentList2 = NULL;
#endif	
	
	_nTranslucentMaterialCount = 0;
}


//
//
//
FINLINE BOOL _AddDLEntry_Shadow( _RL_Entry_t *pNewEntry, _EntryType_e nEntryType )
{
	FASSERT( _bModuleInitialized );
	_RL_Entry_t *pTestEntry;
	
	pTestEntry = _RL_Shadows[0][nEntryType].pTail;
	if ( !pTestEntry )
	{
		_RL_Shadows[0][nEntryType].pHead = pNewEntry;
		_RL_Shadows[0][nEntryType].pTail = pNewEntry;
		_RL_Shadows[0][nEntryType].nCount = 1;
		pNewEntry->pShadowsNext = NULL;
	}
	else
	{
		FASSERT( !pTestEntry->pShadowsNext );
		pTestEntry->pShadowsNext = pNewEntry;
		_RL_Shadows[0][nEntryType].pTail = pNewEntry;
		pNewEntry->pShadowsNext = NULL;
		
		_RL_Shadows[0][nEntryType].nCount++;
	}
	
	return TRUE;
}


//
// Adds a display list entry into the appropriate list
//
BOOL _AddDLEntry( _RL_Entry_t *pNewEntry, u32 nLightShaderIdx, u32 nSurfaceShaderIdx, u32 nSpecularShaderIdx, _EntryType_e nEntryType )
{
	FASSERT( _bModuleInitialized );
	_RL_Entry_t *pTestEntry;

	// We have to at least have a valid surface shader
	if ( nSurfaceShaderIdx == FSH_INVALID_SURFACE_SHADER )
	{
		// This shouldn't happen...
		FASSERT_NOW;
		return FALSE;
	}

	// Check for translucency	
	if ( _SurfaceRouter[nSurfaceShaderIdx].nListType == _RL_TYPE_TRANSLUCENT 
			|| nEntryType == _RL_ENTRY_TYPE_POINT_SPRITE 
			|| nSurfaceShaderIdx == FSHADERS_otBASE )
	{
		if ( !(FRS_bRenderFlags & FRS_RENDER_TRANSLUCENCIES) )
		{
			return FALSE;
		}
		
		// If the surface shader is translucent, we need to put the
		// entry in the translucent material list
		
		/////////////////////////////////////////
		// Add the entry to the translucent list

		if ( _nTranslucentMaterialCount == _nMaxTranslucentMaterialCount )
		{
			#if !FANG_PRODUCTION_BUILD		
				if ( !_bWarningIssued )
				{
					DEVPRINTF( "fDisplayList::_AddDLEntry() - Exceeded translucent material buffer.  Current Max is %d.\n", _nMaxTranslucentMaterialCount );
					_bWarningIssued = TRUE;
				}
			#endif
			
			return FALSE;
		}
		
		CFVec3 vPos;
		if ( nEntryType == _RL_ENTRY_TYPE_NON_WORLD_SPACE )
		{
			// Calculate actual distance of the "material vert average" from the camera
			vPos = pNewEntry->pMeshInst->m_Xfm.m_MtxF.m44.MultPoint( pNewEntry->pMaterial->vAverageVertPos );
			vPos -= FVis_mtxRenderCamera.m_vPos.v3;
			pNewEntry->fDistToCam = vPos.Mag();
		}
		else if ( nEntryType == _RL_ENTRY_TYPE_WORLD_SPACE )
		{
			vPos = pNewEntry->pMaterial->vAverageVertPos - FVis_mtxRenderCamera.m_vPos.v3;
			pNewEntry->fDistToCam = vPos.Mag();
		}
		
		#if _USE_LINKLIST_TRANSLUCENCY_ORDERING
			_RL_Entry_t *pLast = NULL, *pTest = _papTranslucentList2;
			while ( pTest )
			{
				if ( pNewEntry->fDistToCamSq < pTest->fDistToCamSq )
				{
					if ( pLast )
					{
						pLast->pSurfaceNext = pNewEntry;
					}
					else
					{
						_papTranslucentList2 = pNewEntry;
					}						
					pNewEntry->pSurfaceNext = pTest;
					break;
				}
				pLast = pTest;
				pTest = pTest->pSurfaceNext;
			}
			if ( !pTest )
			{
				if ( !pLast )
				{
					_papTranslucentList2 = pNewEntry;
				}
				else
				{
					pLast->pSurfaceNext = pNewEntry;
				}
				pNewEntry->pSurfaceNext = NULL;
			}
		#else
			_papTranslucentList[ _nTranslucentMaterialCount++ ] = pNewEntry;
		#endif
		
		return TRUE;
	}
	
	FASSERT( nEntryType >= 0 && nEntryType < _RL_ENTRY_TYPE_NON_TRANSLUCENT_COUNT );

	// Determine where this entry should be routed to in the display lists
	
	if ( (FRS_bRenderFlags & FRS_RENDER_PASS_LIGHTING) && !(pNewEntry->pMeshInst->m_nFlags & FMESHINST_FLAG_SURFACEONLY) )
	{
		///////////////////////////////////
		// Add the entry to the light list
		
		if ( nLightShaderIdx != FSH_INVALID_LIGHT_SHADER )
		{
			u16 nLightingReroute = (u16)_LightingRouter[nLightShaderIdx].nListIdx;
			
			// Add to the lighting link list
			pTestEntry = _RL_Lighting[nLightingReroute][nEntryType].pTail;
			if ( !pTestEntry )
			{
				_RL_Lighting[nLightingReroute][nEntryType].pHead = pNewEntry;
				_RL_Lighting[nLightingReroute][nEntryType].pTail = pNewEntry;
				_RL_Lighting[nLightingReroute][nEntryType].nCount = 1;
				pNewEntry->pLightNext = NULL;
			}
			else
			{
				FASSERT( !pTestEntry->pLightNext );
				pTestEntry->pLightNext = pNewEntry;
				_RL_Lighting[nLightingReroute][nEntryType].pTail = pNewEntry;
				pNewEntry->pLightNext = NULL;
				
				_RL_Lighting[nLightingReroute][nEntryType].nCount++;
			}
		}
	}
	
	if ( FRS_bRenderFlags & FRS_RENDER_PASS_SURFACE )
	{
		/////////////////////////////////////
		// Add the entry to the surface list

		if ( nSurfaceShaderIdx != FSH_INVALID_SURFACE_SHADER )
		{
			u16 nSurfaceReroute = _SurfaceRouter[nSurfaceShaderIdx].nListIdx;
			
			// Add to the surface link list
			// Note that the hash key is a portion of the material's hash key.  This
			// limits the size of the hash table, but we can still use the full
			// hash key within the hash table to determine more precise matches
			u32 nHKey = pNewEntry->pMaterial->nDLHashKey & (_RL_SURFACE_HASH_SIZE - 1);
			FASSERT( nHKey >= 0 && nHKey < _RL_SURFACE_HASH_SIZE );
			pTestEntry = _RL_paSurface[nSurfaceReroute][nEntryType][nHKey].pHead;
			if ( !pTestEntry )
			{
				// This is the first entry for this hash
				_RL_paSurface[nSurfaceReroute][nEntryType][nHKey].nCount = 1;
				_RL_paSurface[nSurfaceReroute][nEntryType][nHKey].pHead = pNewEntry;
				_RL_paSurface[nSurfaceReroute][nEntryType][nHKey].pTail = pNewEntry;
				pNewEntry->pSurfaceNext = NULL;
			}
			else
			{
				_RL_paSurface[nSurfaceReroute][nEntryType][nHKey].nCount++;
				
				// This is not the first entry for this hash so we need to check
				// to see if there are any like hashes in the list
				while ( pTestEntry )
				{
					FASSERT(pTestEntry->pMaterial);
					FASSERT(pNewEntry->pMaterial);
					if ( pTestEntry->pMaterial->nDLHashKey == pNewEntry->pMaterial->nDLHashKey )
					{
						// Found the same hash key
						#if FANG_DEBUG_BUILD
							_VerifyHashKeys( pTestEntry->pMaterial, pNewEntry->pMaterial );
						#endif				
						
						pNewEntry->pSurfaceNext = pTestEntry->pSurfaceNext;
						pTestEntry->pSurfaceNext = pNewEntry;
						if ( !pNewEntry->pSurfaceNext )
						{
							_RL_paSurface[nSurfaceReroute][nEntryType][nHKey].pTail = pNewEntry;
						}
						return TRUE;
					}
					pTestEntry = pTestEntry->pSurfaceNext;
				}
				
				// Unable to find the same hash key, so add it to the end
				_RL_paSurface[nSurfaceReroute][nEntryType][nHKey].pTail->pSurfaceNext = pNewEntry;
				pNewEntry->pSurfaceNext = NULL;
				_RL_paSurface[nSurfaceReroute][nEntryType][nHKey].pTail = pNewEntry;
			}
		}
	}

	if ( FRS_bRenderFlags & FRS_RENDER_PASS_SPECULAR && !(pNewEntry->pMeshInst->m_nFlags&FMESHINST_FLAG_SURFACEONLY) )
	{
		//////////////////////////////////////
		// Add the entry to the specular list

		if ( nSpecularShaderIdx != FSH_INVALID_SPECULAR_SHADER )
		{
			u16 nSpecularReroute = (u16)_SpecularRouter[nSpecularShaderIdx].nListIdx;
			
			// Add to the lighting link list
			_RL_Entry_t *pTestEntry = _RL_Specular[nSpecularReroute][nEntryType].pTail;
			if ( !pTestEntry )
			{
				_RL_Specular[nSpecularReroute][nEntryType].pHead = pNewEntry;
				_RL_Specular[nSpecularReroute][nEntryType].pTail = pNewEntry;
				_RL_Specular[nSpecularReroute][nEntryType].nCount = 1;
				pNewEntry->pSpecularNext = NULL;
			}
			else
			{
				FASSERT( !pTestEntry->pSpecularNext );
				pTestEntry->pSpecularNext = pNewEntry;
				_RL_Specular[nSpecularReroute][nEntryType].pTail = pNewEntry;
				pNewEntry->pSpecularNext = NULL;
				
				_RL_Specular[nSpecularReroute][nEntryType].nCount++;
			}
		}
	}
	
	return TRUE;
}

#define _SPEC_TEST_ 0

//
// This function adds a mesh into the display list linklists.  It will
// divide the object up by materials.
//
BOOL frs_AddMeshInst( CFMeshInst *pMeshInst, FViewportPlanesMask_t nPlanesMask, f32 fDistToCam, f32 fVPtoScreenPixelRatio )
{
	FASSERT( _bModuleInitialized );
	FASSERT( pMeshInst && pMeshInst->m_pMesh );
	
#if !FANG_PRODUCTION_BUILD
	if ( FMesh_nDrawCollisionGeoFlags != FMESH_DRAW_COLL_GEO_DISABLED )
	{
		return FALSE;
	}
#endif

	if ( !(FRS_bRenderFlags & FRS_RENDER_OBJECT_GEO) )
	{
		return FALSE;
	}
	
	FMeshMaterial_t *pMat = pMeshInst->m_pMesh->aMtl;
	FMeshMaterial_t *pLastMat = &pMeshInst->m_pMesh->aMtl[pMeshInst->m_pMesh->nMaterialCount];

	// Determine the appropriate LOD
	u8 i, nLODMask, nLODIndex = 0;
	if ( pMeshInst->m_pMesh->nLODCount > 1 )
	{
		if ( FMesh_bForceMinLOD )
		{
			nLODIndex = pMeshInst->m_pMesh->nLODCount - 1;
		}
		else
		{
#if _USE_FPS_TO_ADJUST_MESH_LOD
			f32 fPerfAdjust = fmath_Inv( FPerf_fAvgToTargetSPFRatio );
			FMATH_CLAMP( fPerfAdjust, 0.70f, 1.f );
			fPerfAdjust *= fVPtoScreenPixelRatio;
#else
			f32 fPerfAdjust = fVPtoScreenPixelRatio;
#endif
			f32 fLastDist = 0.f;
			for ( i = 1; i < pMeshInst->m_pMesh->nLODCount ; i++ )
			{
				f32 fHysteresis = 0.f;
#if _USE_FPS_TO_ADJUST_MESH_LOD
				f32 fSwitchDist = pMeshInst->m_pMesh->afLODDistance[i];
				if ( i > 1 )
				{
					fSwitchDist *= (fPerfAdjust * fmath_Div(i, pMeshInst->m_pMesh->nLODCount - 1.f));
				}
#else
				f32 fSwitchDist = pMeshInst->m_pMesh->afLODDistance[i] * fPerfAdjust;
#endif
				
				// Add a little bit of hysteresis
				fHysteresis = (fSwitchDist - fLastDist) * 0.1f;
				if ( pMeshInst->m_nCurrentLOD >= i )
				{
					// We're checking a higher LOD, so subtract a little bit of hysteresis
					fHysteresis = -fHysteresis;
				}
				
				fLastDist = fSwitchDist;
				
				if ( fDistToCam < fSwitchDist + fHysteresis  )
				{
					break;
				}
				nLODIndex++;
			}
		}
//		if ( fDistToCam > (pMeshInst->m_BoundSphere_MS.m_fRadius * 3) || nLODIndex != 0 )
//		{
//			nLODIndex += _nLODBias;
//		}
		if ( nLODIndex >= pMeshInst->m_pMesh->nLODCount )
		{
			nLODIndex = pMeshInst->m_pMesh->nLODCount - 1;
		}
	#if !FANG_PRODUCTION_BUILD
		if ( FMesh_nForceLOD != -1 )
		{
			nLODIndex = FMesh_nForceLOD;
		}
	#endif
		pMeshInst->m_nCurrentLOD = nLODIndex;
	}
	nLODMask = 1 << nLODIndex;

	// For each material in the mesh, we create a new linklist entry and
	// add it into the display lists
	_RL_Entry_t *pNewEntry;
	u16 nSurfaceShaderIdx;
	for ( ; pMat != pLastMat; pMat++ )
	{
		if ( !(pMat->nLODMask & nLODMask) )
		{
			continue;
		}

		// Get a new entry to represent this mesh mat		
		pNewEntry = _GetDLEntry();
		if ( !pNewEntry )
		{
			return FALSE;
		}

		#if FPERF_ENABLE
			FPerf_nMeshMtlDrawnCount++;
		#endif

		pNewEntry->nFlags |= _RL_ENTRY_FLAGS_MESH_INST;
		pNewEntry->pMeshInst = pMeshInst;
		pNewEntry->pMaterial = pMat;
		pNewEntry->nCrossesPlanesMask = (u8)nPlanesMask;
		pNewEntry->fDistToCam = fDistToCam;
		pNewEntry->pMirrorMtx = NULL;
		pNewEntry->nLODIndex = nLODIndex;

		nSurfaceShaderIdx = pMat->nSurfaceShaderIdx;
		if ( nSurfaceShaderIdx == FSHADERS_oBASE && pMeshInst->GetMeshTintAndAlpha()->fAlpha < 0.999f )
		{
			nSurfaceShaderIdx = FSHADERS_otBASE;
		}

		if ( nSurfaceShaderIdx == FSHADERS_cBASE && pMeshInst->GetMeshTintAndAlpha()->fAlpha < 0.999f )
		{
			nSurfaceShaderIdx = FSHADERS_tBASE;
		}

#if 1
		if (pMeshInst->m_nFlags & FMESHINST_FLAG_ACCEPT_SHADOWS)
		{
			_AddDLEntry_Shadow( pNewEntry, _RL_ENTRY_TYPE_NON_WORLD_SPACE);
		}
#endif

		#if _SPEC_TEST_
		if ( !_AddDLEntry( pNewEntry, pMat->nLightShaderIdx, nSurfaceShaderIdx, 0/*pMat->nSpecularShaderIdx*/, _RL_ENTRY_TYPE_NON_WORLD_SPACE ) )
		#else
		if ( !_AddDLEntry( pNewEntry, pMat->nLightShaderIdx, nSurfaceShaderIdx, pMat->nSpecularShaderIdx, _RL_ENTRY_TYPE_NON_WORLD_SPACE ) )
		//if ( !_AddDLEntry( pNewEntry, 0, 0, pMat->nSpecularShaderIdx, _RL_ENTRY_TYPE_NON_WORLD_SPACE ) )
		#endif
		{
			_ReleaseDLEntry( pNewEntry );
		}
	}
	
	return TRUE;
}


//
//
//
BOOL frs_AddVolumeMeshMaterial( CFMeshInst *pMeshInst, FMeshMaterial_t *pMat, FViewportPlanesMask_t nPlanesMask, f32 fDistToCam )
{
	FASSERT( _bModuleInitialized );
	FASSERT( pMeshInst && pMeshInst->m_pMesh );
	
#if !FANG_PRODUCTION_BUILD
	if ( FMesh_nDrawCollisionGeoFlags != FMESH_DRAW_COLL_GEO_DISABLED )
	{
		return FALSE;
	}
#endif

	if ( !(FRS_bRenderFlags & FRS_RENDER_VOLUME_GEO) )
	{
		return FALSE;
	}

	_RL_Entry_t *pNewEntry;

	// Get a new entry to represent this mesh mat
	pNewEntry = _GetDLEntry();
	if ( !pNewEntry )
	{
		return FALSE;
	}

	#if FPERF_ENABLE
		FPerf_nMeshMtlDrawnCount++;
	#endif

	pNewEntry->nCrossesPlanesMask = (u8)nPlanesMask;
	pNewEntry->nFlags |= _RL_ENTRY_FLAGS_VOLUME_MESH;
	pNewEntry->pMeshInst = pMeshInst;
	pNewEntry->pMaterial = pMat;
	pNewEntry->fDistToCam = fDistToCam;
	pNewEntry->pMirrorMtx = NULL;
	pNewEntry->nLODIndex = 0;
	
	if (pMeshInst->m_nFlags & FMESHINST_FLAG_ACCEPT_SHADOWS || 1)
	{
		_AddDLEntry_Shadow( pNewEntry, _RL_ENTRY_TYPE_WORLD_SPACE);
	}

	if ( !_AddDLEntry( pNewEntry, pMat->nLightShaderIdx, pMat->nSurfaceShaderIdx, pMat->nSpecularShaderIdx, _RL_ENTRY_TYPE_WORLD_SPACE ) )
	{
		_ReleaseDLEntry( pNewEntry );
	}
	
	return TRUE;
}


//
//
//
BOOL frs_AddVolumeMesh( CFMeshInst *pMeshInst, FViewportPlanesMask_t nPlanesMask, f32 fDistToCam )
{
	FASSERT( _bModuleInitialized );
	FASSERT( pMeshInst && pMeshInst->m_pMesh );
	
#if !FANG_PRODUCTION_BUILD
	if ( FMesh_nDrawCollisionGeoFlags != FMESH_DRAW_COLL_GEO_DISABLED )
	{
		return FALSE;
	}
#endif

	if ( !(FRS_bRenderFlags & FRS_RENDER_VOLUME_GEO) )
	{
		return FALSE;
	}

	FMeshMaterial_t *pMat;
	FMeshMaterial_t *pLastMat = &pMeshInst->m_pMesh->aMtl[pMeshInst->m_pMesh->nMaterialCount];

	_RL_Entry_t *pNewEntry;
	for ( pMat = pMeshInst->m_pMesh->aMtl; pMat != pLastMat; pMat++ )
	{
		// Get a new entry to represent this mesh mat
		pNewEntry = _GetDLEntry();
		if ( !pNewEntry )
		{
			return FALSE;
		}

		_nVolMatsConsidered++;
		if ( nPlanesMask != FVIEWPORT_PLANESMASK_NONE )
		{
			_nVolMatsTested++;

			// Cull the individual materials against the overall frustum
			f32 fRadius = pMat->nCompressedRadius * (1.f/255.f) * pMeshInst->m_pMesh->BoundSphere_MS.m_fRadius;

			CFSphere spTest( pMat->vAverageVertPos, fRadius );
			s32 nCrossesPlanesMask = fviewport_TestSphere_WS( FVis_pCurrentViewport, &spTest, FVIEWPORT_PLANESMASK_ALL );
			if ( nCrossesPlanesMask == FVIEWPORT_PLANESMASK_NONE )
			{
				_nVolMatsClippingRemoved++;
			}
			else if ( nCrossesPlanesMask == -1 )
			{
				_nVolMatsOutsideFrustrum++;
				continue;
			}
			pNewEntry->nCrossesPlanesMask = (u8)nCrossesPlanesMask;
		}
		else
		{
			pNewEntry->nCrossesPlanesMask = (u8)nPlanesMask;
		}
		
		#if FPERF_ENABLE
			FPerf_nMeshMtlDrawnCount++;
		#endif

		pNewEntry->nFlags |= _RL_ENTRY_FLAGS_VOLUME_MESH;
		pNewEntry->pMeshInst = pMeshInst;
		pNewEntry->pMaterial = pMat;
		pNewEntry->fDistToCam = fDistToCam;
		pNewEntry->pMirrorMtx = NULL;
		pNewEntry->nLODIndex = 0;
		
		if (pMeshInst->m_nFlags & FMESHINST_FLAG_ACCEPT_SHADOWS || 1)
		{
			_AddDLEntry_Shadow( pNewEntry, _RL_ENTRY_TYPE_WORLD_SPACE);
		}

		if ( !_AddDLEntry( pNewEntry, pMat->nLightShaderIdx, pMat->nSurfaceShaderIdx, pMat->nSpecularShaderIdx, _RL_ENTRY_TYPE_WORLD_SPACE ) )
		{
			_ReleaseDLEntry( pNewEntry );
		}
	}
	
	return TRUE;
}


//
//
//
BOOL frs_AddPSGroup( CFWorldPSGroup *pPSGroup, FViewportPlanesMask_t nPlanesMask, f32 fDistToCam )
{
	FASSERT( _bModuleInitialized );
	FASSERT( pPSGroup );
	
	if ( !(FRS_bRenderFlags & FRS_RENDER_SPRITES) )
	{
		return FALSE;
	}
	
	if ( pPSGroup->m_nPSFlags & FPSPRITE_FLAG_SORT )
	{
		// If the point sprite group is to be sorted, then we add it to the translucent list
		_RL_Entry_t *pNewEntry = _GetDLEntry();
		if ( !pNewEntry )
		{
			return FALSE;
		}

		pNewEntry->nFlags |= _RL_ENTRY_FLAGS_POINT_SPRITES;
		pNewEntry->pPSGroup = pPSGroup;
		pNewEntry->pMaterial = NULL;
		pNewEntry->nCrossesPlanesMask = (u8)nPlanesMask;
		pNewEntry->fDistToCam = fDistToCam;
		pNewEntry->pMirrorMtx = NULL;

		// Adding the entry indicating that it uses shader FSHADERS_DIFFUSETEX_AIAT will
		// result in it being added to the translucent list
		if ( !_AddDLEntry( pNewEntry, FSH_INVALID_LIGHT_SHADER, FSHADERS_DIFFUSETEX_AIAT, FSH_INVALID_SPECULAR_SHADER, _RL_ENTRY_TYPE_POINT_SPRITE ) )
		{
			_ReleaseDLEntry( pNewEntry );
		}
		
		return TRUE;
	}

	// This point sprite group is not sorted, so add it to the point sprite group list
	if ( _nPointSpriteCount == _nMaxPointSpriteCount )
	{
		return FALSE;
	}

	_paPointSpriteList[_nPointSpriteCount].pPSGroup = pPSGroup;
	_paPointSpriteList[_nPointSpriteCount].nCrossesPlanesMask = nPlanesMask;
	_paPointSpriteList[_nPointSpriteCount].fDistToCam = fDistToCam;
	_paPointSpriteList[_nPointSpriteCount].pMirrorMtx = NULL;
	_nPointSpriteCount++;

	return TRUE;
}


//
//
//
static int _QSortTranslucenciesCallback( const void *pElement1, const void *pElement2 ) 
{
	const _RL_Entry_t *pEntry1 = *(const _RL_Entry_t **)pElement1;
	const _RL_Entry_t *pEntry2 = *(const _RL_Entry_t **)pElement2;

	if ( pEntry1->fDistToCam > pEntry2->fDistToCam ) 
	{
		// Entry 1 is farther than entry 2, so draw entry 1 first...
		return -1;
	}

	if ( pEntry1->fDistToCam < pEntry2->fDistToCam ) 
	{
		// Entry 1 is closer than entry 2, so draw entry 2 first...
		return 1;
	}

	// Both entries are the same distance...

	return 0;
}

extern f32 FSh_fPerPixelFade;

//
//
//
FINLINE void _SetShaderLOD( _RL_Entry_t *pEntry )
{
/*	FSh_fPerPixelFade = 1.f;
	if ( pEntry->fDistToCam > 175 )
	{
		FSh_fPerPixelFade = 0.f;
		FSh_shaderFlags = (FSh_ShaderFlags_e)(FSh_RENDER_MOTIF_LIGHTMAPS|FSh_RENDER_REFLECTIONMAP);
	}
	else if ( pEntry->fDistToCam > 125 )
	{
		FSh_fPerPixelFade = (175.f - pEntry->fDistToCam) * (1.f/50.f);
		FSh_shaderFlags = (FSh_ShaderFlags_e)(FSh_RENDER_MULTIPASS_LIGHTING|FSh_RENDER_MOTIF_LIGHTMAPS|FSh_RENDER_REFLECTIONMAP);
	}
	else*/ if ( pEntry->fDistToCam > 75 )
	{
		FSh_shaderFlags = (FSh_ShaderFlags_e)(FSh_RENDER_DETAIL|FSh_RENDER_MULTIPASS_LIGHTING|FSh_RENDER_MOTIF_LIGHTMAPS|FSh_RENDER_REFLECTIONMAP);
//		FSh_shaderFlags = (FSh_ShaderFlags_e)((u32)FSh_shaderFlags & ~FSh_RENDER_BUMP);
	}
	else
	{
		FSh_shaderFlags = (FSh_ShaderFlags_e)FSh_RENDER_ALL;
	}
}


//
//
//
void frs_FlushRenderLists( void )
{
	u32 i, ii;
	_RL_Entry_t *pEntry;
	BOOL bWorldSetupActive = FALSE;
	//BOOL bFogEnable=FALSE;

	if ( FVis_bDrawCellsOnly )
	{
		_RL_ClearDisplayLists();
		return;
	}
	u16 nLightingMaterials = 0;
	u16 nSurfaceMaterials = 0;
	u16 nSpecularMaterials = 0;
	u16 nTranslucentMaterials = 0;
	u16 nNonSortedPointSprites = 0;

	#if FANG_PLATFORM_GC
	if (!FSh_bUseClipPlane)
	{
		fgc_SetZCompareOrder( TRUE );
	}
	else
	{
		fgc_SetZCompareOrder( FALSE );
	}
	#endif
	
#if _DISPLAY_MATERIAL_CULLING_STATS
	if ( _nVolMatsConsidered )
	{
		ftext_DebugPrintf( 0.502f, 0.062f, "~w1~C00000099Mats Submit: %d", _nVolMatsConsidered );
		ftext_DebugPrintf( 0.5f, 0.06f, "~w1~C99999999Mats Submit: %d", _nVolMatsConsidered );
		ftext_DebugPrintf( 0.502f, 0.082f, "~w1~C00000099Mats Tested: %d", _nVolMatsTested );
		ftext_DebugPrintf( 0.5f, 0.08f, "~w1~C99999999Mats Tested: %d", _nVolMatsTested );
		ftext_DebugPrintf( 0.502f, 0.102f, "~w1~C00000099Mats Culled: %d (%d%%)", _nVolMatsOutsideFrustrum, (u32)( ((f32)_nVolMatsOutsideFrustrum / (f32)_nVolMatsConsidered) * 100.f ) );
		ftext_DebugPrintf( 0.5f, 0.10f, "~w1~C99999999Mats Culled: %d (%d%%)", _nVolMatsOutsideFrustrum, (u32)(((f32)_nVolMatsOutsideFrustrum / (f32)_nVolMatsConsidered) * 100.f) );
		if ( _nVolMatsTested - _nVolMatsOutsideFrustrum > 0 )
		{
			ftext_DebugPrintf( 0.502f, 0.122f, "~w1~C00000099Mats Unclip: %d (%d%%)", _nVolMatsClippingRemoved, (u32)( ((f32)_nVolMatsClippingRemoved / (f32)(_nVolMatsTested - _nVolMatsOutsideFrustrum)) * 100.f ) );
			ftext_DebugPrintf( 0.5f, 0.12f, "~w1~C99999999Mats Unclip: %d (%d%%)", _nVolMatsClippingRemoved, (u32)(((f32)_nVolMatsClippingRemoved / (f32)(_nVolMatsTested - _nVolMatsOutsideFrustrum)) * 100.f) );
		}
		_nVolMatsClippingRemoved = _nVolMatsTested = _nVolMatsOutsideFrustrum = _nVolMatsConsidered = 0;
	}
#endif

	// Make sure the texture coordinate matrices are cleared
	fsh_TexCoordMtx_ResetList();
	fsh_TexOverride_ResetList();
	
	#if FANG_PLATFORM_DX
		fdx8xfm_SetViewDXMatrix( TRUE );
//ARG - >>>>>
	#elif FANG_PLATFORM_PS2
		fps2xfm_SetViewPS2Matrix();
//ARG - <<<<<
	#endif // FANG_PLATFORM_DX

	#if	_RENDER_LIGHTING_PASS	//ARG
	if ( FRS_bRenderFlags & FRS_RENDER_PASS_LIGHTING )
	{
		////////////////////////
		// Do the lighting pass	

		fsh_SetSurfaceOnly( FALSE );

		fsh_SetShaderType( SHADERTYPE_DIFFUSE );
		
		// Setup for rendering the volume geo
		fmesh_Ambient_Set( &FColor_MotifBlack.ColorRGB );
		fxfm_SetViewAndWorldSpaceModelMatrices();
		#if FANG_PLATFORM_GC
			fgcsh_SetWorldGeoFlag( TRUE );
		#endif
		bWorldSetupActive = TRUE;
		
		// Render volume geo
		for ( i = 0; _nLightShaderOrder[i] != FSH_INVALID_LIGHT_SHADER; i++ )
		{
			#if FANG_PLATFORM_GC
				if ( _nLightShaderOrder[i] == _FIRST_CUTOUT_LIGHT_SHADER && !FSh_bUseClipPlane)
				{
					fgc_SetZCompareOrder( FALSE );
				}
			#endif

			// Set the current shader to the appropriate light shader
			fsh_SetShader( 0, _nLightShaderOrder[i], 0, 0 );

			// Render world geometry
			pEntry = _RL_Lighting[i][_RL_ENTRY_TYPE_WORLD_SPACE].pHead;
			while ( pEntry )
			{
				FASSERT( pEntry->pMeshInst );
				
				_SetShaderLOD( pEntry );
	
				// Setup lights for this mesh
				pEntry->pMeshInst->AddShaderLights();

				pEntry->pMeshInst->DrawMaterialLight_P( pEntry->nCrossesPlanesMask, pEntry->pMaterial, pEntry->nLODIndex );
				
				pEntry = pEntry->pLightNext;
				nLightingMaterials++;
			}
		}

		// Setup for rendering non-volume geo
		bWorldSetupActive = FALSE;
		#if FANG_PLATFORM_GC
			fgc_SetZCompareOrder( TRUE );
			fgcsh_SetWorldGeoFlag( FALSE );
		#endif
			
		// Render non volume mesh geo
		for ( i = 0; _nLightShaderOrder[i] != FSH_INVALID_LIGHT_SHADER; i++ )
		{
			#if FANG_PLATFORM_GC
				if ( _nLightShaderOrder[i] == _FIRST_CUTOUT_LIGHT_SHADER && !FSh_bUseClipPlane)
				{
					fgc_SetZCompareOrder( FALSE );
				}
			#endif
			
			// Set the current shader to the appropriate light shader
			fsh_SetShader( 0, _nLightShaderOrder[i], 0, 0 );

			// Render non-world geometry
			pEntry = _RL_Lighting[i][_RL_ENTRY_TYPE_NON_WORLD_SPACE].pHead;
			while ( pEntry )
			{
				FASSERT( pEntry->pMeshInst );

				#if FANG_WINGC
				{
					// Port diagnostic: non-world meshes drawn in one frame, with their world position.
					extern u32 _nPortRSFrame;
					if ( _nPortRSFrame == 900 && Fang_bPortDiag )
					{
						const CFVec3A &vPos = pEntry->pMeshInst->m_Xfm.m_MtxF.m_vPos;
						DEVPRINTF( "PORT-RSMESH %s mtl=%u pos=(%.1f,%.1f,%.1f) flags=%08x\n", pEntry->pMeshInst->m_pMesh->szName,
							(u32)(pEntry->pMaterial - pEntry->pMeshInst->m_pMesh->aMtl), vPos.x, vPos.y, vPos.z, (u32)pEntry->pMeshInst->m_nFlags );
					}
				}
				#endif

				// Setup lights for this mesh
				if ( !(pEntry->pMeshInst->m_nFlags & FMESHINST_FLAG_SURFACEONLY) )
				{
					_SetShaderLOD( pEntry );
	
					fmesh_Ambient_Set( &pEntry->pMeshInst->m_FinalAmbientRGB );
					pEntry->pMeshInst->AddShaderLights();

					pEntry->pMeshInst->DrawMaterialLight_P( pEntry->nCrossesPlanesMask, pEntry->pMaterial, pEntry->nLODIndex );
					
					pEntry = pEntry->pLightNext;
					nLightingMaterials++;
				}
			}
		}
	}
	#endif	//ARG

	// Return shader LOD to full detail for the remaining passes
	FSh_shaderFlags = (FSh_ShaderFlags_e)FSh_RENDER_ALL;
	
	#if FANG_PLATFORM_GC
	if (!FSh_bUseClipPlane)
	{
		fgc_SetZCompareOrder( TRUE );
	}
	#endif
	
	if ( FRS_bRenderFlags & FRS_RENDER_SHADOWS )
	{
		fshadow_BeginShadowRender();
		// Setup the world space model matrices
		if ( _RL_Shadows[0][_RL_ENTRY_TYPE_WORLD_SPACE].pHead)// && !bWorldSetupActive )
		{
			fmesh_Ambient_Set( &FColor_MotifBlack.ColorRGB );
			fxfm_SetViewAndWorldSpaceModelMatrices();
			#if FANG_PLATFORM_GC
				fgcsh_SetWorldGeoFlag( TRUE );
			#endif
			bWorldSetupActive = TRUE;
		}
		
		// Render world geometry
		pEntry = _RL_Shadows[0][_RL_ENTRY_TYPE_WORLD_SPACE].pHead;
		while ( pEntry )
		{
			FASSERT( pEntry->pMeshInst );
			
			pEntry->pMeshInst->DrawShadow_P( pEntry->nCrossesPlanesMask, pEntry->pMaterial, pEntry->nLODIndex );
			
			pEntry = pEntry->pShadowsNext;
		}
		
		// Render non-world geometry
		pEntry = _RL_Shadows[0][_RL_ENTRY_TYPE_NON_WORLD_SPACE].pHead;
		while ( pEntry )
		{
			FASSERT( pEntry->pMeshInst );
			
			// Setup lights for this mesh
			FASSERT( pEntry->pMeshInst );
			
			pEntry->pMeshInst->DrawShadow_P( pEntry->nCrossesPlanesMask, pEntry->pMaterial, pEntry->nLODIndex );
			
			pEntry = pEntry->pShadowsNext;
		}
		
		fshadow_EndShadowRender();
	}

	#if FANG_PLATFORM_DX
		fdx8tex_UncacheSelected();
	#endif

	#if FANG_PLATFORM_DX
	//Uncache selected vertex buffer
	fdx8vb_UncacheSelected();
	#endif
	
	#if FANG_PLATFORM_GC
	//reset cache for color channels.
	FGC_bUseVertexColor = TRUE;
	fgcsh_Light_Activate( TRUE, TRUE );
	#endif
	
//	FRenderer_bFogEnabled = bFogEnable;

	bWorldSetupActive = FALSE;
	
	if ( FRS_bRenderFlags & FRS_RENDER_PASS_SURFACE )
	{
		FMesh_nLastSurfaceMaterialCRC = 0xffffffff;
		///////////////////////
		// Do the surface pass
		i = 0;
		fsh_SetShaderType( SHADERTYPE_SURFACE );
		for ( ; _nSurfaceShaderOrder[i] != FSH_INVALID_SURFACE_SHADER; i++ )
		{
			// Set the current shader to the appropriate surface shader
			fsh_SetShader( _nSurfaceShaderOrder[i], 0, 0 );

			for ( ii = 0; ii < _RL_SURFACE_HASH_SIZE; ii++ )
			{
				// Render non-world geometry
				pEntry = _RL_paSurface[i][_RL_ENTRY_TYPE_NON_WORLD_SPACE][ii].pHead;
				if ( pEntry )
				{
					bWorldSetupActive = FALSE;
				}

				while ( pEntry )
				{
					FASSERT( pEntry->pMeshInst );
					
//ARG - >>>>>
#if FANG_PLATFORM_PS2
					fmesh_Ambient_Set( &pEntry->pMeshInst->m_FinalAmbientRGB );
#endif
//ARG - <<<<<
					pEntry->pMeshInst->DrawMaterialSurface_P( pEntry->nCrossesPlanesMask, pEntry->pMaterial, pEntry->nLODIndex );
					pEntry = pEntry->pSurfaceNext;
					nSurfaceMaterials++;
				}

				// Setup the world space model matrices
				if ( _RL_paSurface[i][_RL_ENTRY_TYPE_WORLD_SPACE][ii].pHead && !bWorldSetupActive )
				{
//ARG - >>>>>
#if FANG_PLATFORM_PS2
					fmesh_Ambient_Set( &FColor_MotifBlack.ColorRGB );
#endif
//ARG - <<<<<
					fxfm_SetViewAndWorldSpaceModelMatrices();
					bWorldSetupActive = TRUE;
				}
				
				// Render world geometry
				pEntry = _RL_paSurface[i][_RL_ENTRY_TYPE_WORLD_SPACE][ii].pHead;
				while ( pEntry )
				{
					FASSERT( pEntry->pMeshInst );
					
					pEntry->pMeshInst->DrawMaterialSurface_P( pEntry->nCrossesPlanesMask, pEntry->pMaterial, pEntry->nLODIndex );
					pEntry = pEntry->pSurfaceNext;
					nSurfaceMaterials++;
				}
			}
		}
	}

	#if FANG_WINGC
	{
		// Port diagnostic: how many materials went through each pass.
		extern u32 _nPortRSFrame;
		if ( (++_nPortRSFrame % 600) == 1 && Fang_bPortDiag )
		{
			DEVPRINTF( "PORT-RS frame %u: flags=%08x lighting=%u surface=%u\n", _nPortRSFrame, (u32)FRS_bRenderFlags, (u32)nLightingMaterials, (u32)nSurfaceMaterials );
		}
	}
	#endif

	#if FANG_PLATFORM_GC
		// We will not do any changes to the z compare order for specular
	if (!FSh_bUseClipPlane)
	{
		fgc_SetZCompareOrder( FALSE );
	}
	#endif
	
	#if	_RENDER_SPECULAR_PASS	//ARG
	if ( FRS_bRenderFlags & FRS_RENDER_PASS_SPECULAR )
	{
		fsh_SetSurfaceOnly(FALSE);

		////////////////////////
		// Do the specular pass	
		i = 0;
		bWorldSetupActive = FALSE;
		fsh_SetShaderType( SHADERTYPE_SPECULAR );
		for ( ; _nSpecularShaderOrder[i] != FSH_INVALID_SPECULAR_SHADER; i++ )
		{
			// Set the current shader to the appropriate light shader
			fsh_SetShader( 0, 0, _nSpecularShaderOrder[i], 0 );

			// Render non-world geometry
			pEntry = _RL_Specular[i][_RL_ENTRY_TYPE_NON_WORLD_SPACE].pHead;
			if ( pEntry )
			{
				bWorldSetupActive = FALSE;
			}

			while ( pEntry )
			{
				FASSERT( pEntry->pMeshInst );
			
				if ( !(pEntry->pMeshInst->m_nFlags & FMESHINST_FLAG_SURFACEONLY) )
				{
					// Setup lights for this mesh
					fmesh_Ambient_Set( &pEntry->pMeshInst->m_FinalAmbientRGB );
					pEntry->pMeshInst->AddShaderLights();

					pEntry->pMeshInst->DrawMaterialSpecular_P( pEntry->nCrossesPlanesMask, pEntry->pMaterial, pEntry->nLODIndex );
					pEntry = pEntry->pSpecularNext;
					bWorldSetupActive = FALSE;
					nSpecularMaterials++;
				}
			}
			
			// Setup the world space model matrices
			if ( _RL_Specular[i][_RL_ENTRY_TYPE_WORLD_SPACE].pHead && !bWorldSetupActive )
			{
				fmesh_Ambient_Set( &FColor_MotifBlack.ColorRGB );
				fsh_SetSpecularLight( fvis_GetCurrentDirectional() );
				fxfm_SetViewAndWorldSpaceModelMatrices();
				#if FANG_PLATFORM_GC
					fgcsh_SetWorldGeoFlag( TRUE );
				#endif
				bWorldSetupActive = TRUE;
			}
			
			// Render world geometry
			pEntry = _RL_Specular[i][_RL_ENTRY_TYPE_WORLD_SPACE].pHead;
			while ( pEntry )
			{
				FASSERT( pEntry->pMeshInst );
				
				// Setup lights for this mesh
				pEntry->pMeshInst->AddShaderLights();

				pEntry->pMeshInst->DrawMaterialSpecular_P( pEntry->nCrossesPlanesMask, pEntry->pMaterial, pEntry->nLODIndex );
				pEntry = pEntry->pSpecularNext;
				nSpecularMaterials++;
			}
		}
	}
	#endif	//ARG

	//	FRenderer_bFogEnabled = FALSE;
	//Render fog pass here before the translucencies are rendered.
#if _RENDER_FOG
	fsh_RenderFogPass();
#endif
	//

	//insert liquid volumes before particles and translucencies.
	fvis_DrawLiquidVolumes();
	//

	// Call the pre-pointsprite render callback, if one was set
	if ( _aRenderSortCallbacks[FRS_PREPOINTSPRITE_CALLBACK] )
	{
		_aRenderSortCallbacks[FRS_PREPOINTSPRITE_CALLBACK]( &FVis_mtxRenderCamera, FVis_pCurrentViewport );
		FASSERT( frenderer_GetActive() == FRENDERER_MESH );
	}

	if ( FRS_bRenderFlags & FRS_RENDER_SPRITES )
	{
		fsh_SetSurfaceOnly(FALSE);

		//////////////////////
		// Draw point sprites
		frenderer_Push( FRENDERER_PSPRITE, NULL );
		_RL_PointSpriteEntry_t *pPS = _paPointSpriteList;
		for ( i = 0; i < _nPointSpriteCount; i++, pPS++ )
		{
			FASSERT( pPS );
			
			if (   pPS->pPSGroup->GetPreDrawCallback() == NULL 
				|| pPS->pPSGroup->GetPreDrawCallback()( pPS->pPSGroup, pPS->fDistToCam * pPS->fDistToCam ) ) 
			{
				FXfm_pMirrorMtx = pPS->pMirrorMtx;
				pPS->pPSGroup->Render( FALSE, pPS->nCrossesPlanesMask, pPS->pMirrorMtx );
				nNonSortedPointSprites++;
			}
		}
		_nPointSpriteCount = 0;
		frenderer_Pop();
	}

	// Call the pre-translucency render callback, if one was set
	if ( _aRenderSortCallbacks[FRS_PRETRANSLUCENCY_CALLBACK] )
	{
		_aRenderSortCallbacks[FRS_PRETRANSLUCENCY_CALLBACK]( &FVis_mtxRenderCamera, FVis_pCurrentViewport );
		FASSERT( frenderer_GetActive() == FRENDERER_MESH );
	}

	#if FANG_PLATFORM_DX
	//Uncache selected vertex buffer
	fdx8vb_UncacheSelected();
	#endif

	if ( FRS_bRenderFlags & FRS_RENDER_TRANSLUCENCIES )
	{
		fsh_SetSurfaceOnly(FALSE);

		////////////////////
		// Draw sorted list
		
		FRenderer_e nCurrentRenderer = frenderer_GetActive();

		u32 nZWrite = 0;
		fsh_ZWriteEnable( FALSE );
		
		#if _USE_LINKLIST_TRANSLUCENCY_ORDERING
			u32 nSurfaceShaderIdx;
			pEntry = _papTranslucentList2;
			while ( pEntry )
			{
				fsh_SetShaderType( SHADERTYPE_TRANSLUCENCY );

				if ( pEntry->nFlags & _RL_ENTRY_FLAGS_POINT_SPRITES )
				{
					// See if we need to switch renderers
					if ( nCurrentRenderer != FRENDERER_PSPRITE )
					{
						frenderer_Pop();
						frenderer_Push( FRENDERER_PSPRITE, NULL );
						nCurrentRenderer = FRENDERER_PSPRITE;
					}
				
					if (   pEntry->pPSGroup->GetPreDrawCallback() == NULL 
						|| pEntry->pPSGroup->GetPreDrawCallback()( pEntry->pPSGroup, pEntry->fDistToCam * pEntry->fDistToCam ) ) 
					{
						FXfm_pMirrorMtx = pEntry->pMirrorMtx;
						pEntry->pPSGroup->Render( FALSE, pEntry->nCrossesPlanesMask, pEntry->pMirrorMtx );
					}
				}
				else
				{
					// Setup lights for this mesh
					fmesh_Ambient_Set( &pEntry->pMeshInst->m_FinalAmbientRGB );
					pEntry->pMeshInst->AddShaderLights();
					
					// See if we need to switch renderers
					if ( nCurrentRenderer != FRENDERER_MESH )
					{
						frenderer_Pop();
						frenderer_Push( FRENDERER_MESH, NULL );
						nCurrentRenderer = FRENDERER_MESH;
					}
					
					// If this entry is a world mesh, setup the world space model matrices since
					// they will not be setup when calling DrawInstanceMaterial_P()
					if ( pEntry->pMeshInst->m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH )
					{
						fxfm_SetViewAndWorldSpaceModelMatrices();
					}

					// Make sure we're in the correct zMode
					if ( nZWrite != (pEntry->pMaterial->nMtlFlags & FMESH_MTLFLAG_ZWRITE_ON) )
					{
						nZWrite = (pEntry->pMaterial->nMtlFlags & FMESH_MTLFLAG_ZWRITE_ON);
						fsh_ZWriteEnable( !!nZWrite );
					}

					// Handle objects that have become translucent
					nSurfaceShaderIdx = pEntry->pMaterial->nLightShaderIdx;
					if ( nSurfaceShaderIdx == FSHADERS_oBASE )
					{
						nSurfaceShaderIdx = FSHADERS_otBase;
					}
					else if ( nSurfaceShaderIdx == FSHADERS_cBASE )
					{
						nSurfaceShaderIdx = FSHADERS_tBase;
					}
					
					// Set the shader and draw the mesh
					fsh_SetShader( pEntry->pMaterial->nSurfaceShaderIdx, pEntry->pMaterial->nLightShaderIdx, pEntry->pMaterial->nSpecularShaderIdx );
					pEntry->pMeshInst->DrawInstanceMaterial_P( pEntry->nCrossesPlanesMask, pEntry->pMaterial );
				}
				
				pEntry = pEntry->pSurfaceNext;
			}
		#else
			fclib_QSort( _papTranslucentList, _nTranslucentMaterialCount, sizeof(_RL_Entry_t *), _QSortTranslucenciesCallback );
			for ( i = 0; i < _nTranslucentMaterialCount; i++ )
			{
				FASSERT( _papTranslucentList[i] );
				pEntry = _papTranslucentList[i];
				
				fsh_SetShaderType( SHADERTYPE_TRANSLUCENCY );
				
				if ( pEntry->nFlags & _RL_ENTRY_FLAGS_POINT_SPRITES )
				{
					// See if we need to switch renderers
					if ( nCurrentRenderer != FRENDERER_PSPRITE )
					{
						frenderer_Pop();
						frenderer_Push( FRENDERER_PSPRITE, NULL );
						nCurrentRenderer = FRENDERER_PSPRITE;
					}
				
					if (   pEntry->pPSGroup->GetPreDrawCallback() == NULL 
						|| pEntry->pPSGroup->GetPreDrawCallback()( pEntry->pPSGroup, pEntry->fDistToCam * pEntry->fDistToCam ) ) 
					{
						FXfm_pMirrorMtx = pEntry->pMirrorMtx;
						pEntry->pPSGroup->Render( FALSE, pEntry->nCrossesPlanesMask, pEntry->pMirrorMtx );
						nTranslucentMaterials++;
					}
				}
				else
				{
					// See if we need to switch renderers
					if ( nCurrentRenderer != FRENDERER_MESH )
					{
						frenderer_Pop();
						frenderer_Push( FRENDERER_MESH, NULL );
						nCurrentRenderer = FRENDERER_MESH;
					}

					// Setup lights for this mesh
					fmesh_Ambient_Set( &pEntry->pMeshInst->m_FinalAmbientRGB );
//ARG - >>>>>
#if !FANG_PLATFORM_PS2
					pEntry->pMeshInst->AddShaderLights();
#endif
//ARG - <<<<<
					
					// If this entry is a world mesh, setup the world space model matrices since
					// they will not be setup when calling DrawInstanceMaterial_P()
					if ( pEntry->pMeshInst->m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH )
					{
						fxfm_SetViewAndWorldSpaceModelMatrices();
					}
					
					// Make sure we're in the correct zMode
					if ( nZWrite != (pEntry->pMaterial->nMtlFlags & FMESH_MTLFLAG_ZWRITE_ON) )
					{
						nZWrite = (pEntry->pMaterial->nMtlFlags & FMESH_MTLFLAG_ZWRITE_ON);
						fsh_ZWriteEnable( !!nZWrite );
					}

					// Set the shader and draw the mesh
					fsh_SetShader( pEntry->pMaterial->nSurfaceShaderIdx, pEntry->pMaterial->nLightShaderIdx, pEntry->pMaterial->nSpecularShaderIdx );
					pEntry->pMeshInst->DrawMaterialTranslucency_P( pEntry->nCrossesPlanesMask, pEntry->pMaterial, pEntry->nLODIndex );
					nTranslucentMaterials++;
				}
			}
			_nTranslucentMaterialCount = 0;

		#endif // _USE_LINKLIST_TRANSLUCENCY_ORDERING

			// Make sure we leave z writing on
			if ( nZWrite == 0 )
			{
				nZWrite = FMESH_MTLFLAG_ZWRITE_ON;
				fsh_ZWriteEnable( TRUE );
			}
	}

//	FRenderer_bFogEnabled = bFogEnable;

//ARG - >>>>>
#if FANG_PLATFORM_PS2
	void fshadow_Render(void);
	fshadow_Render();
#endif
//ARG - <<<<<

	///////////////////////////
	// DONE
	///////////////////////////
	
	frenderer_Pop();
	frenderer_Push( FRENDERER_MESH, NULL );
	
	_RL_ClearDisplayLists();

	// Setup the LOD bias for next frame
	if ( FPerf_bGPUBound && FPerf_fAvgToTargetSPFRatio > 1.3f )
	{
		_nLODBias = 2;
	}
	else if ( FPerf_bGPUBound && FPerf_fAvgToTargetSPFRatio > 1.15f )
	{
		_nLODBias = 1;
	}
	else
	{
		_nLODBias = 0;
	}
	
	#if FANG_PLATFORM_GC
		fgc_SetZCompareOrder( FALSE );
	#endif
}

