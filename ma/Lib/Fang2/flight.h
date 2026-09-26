//////////////////////////////////////////////////////////////////////////////////////
// flight.h - Fang light module.
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
// 09/13/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FLIGHT_H_
#define _FLIGHT_H_ 1

#include "fang.h" 
#include "fcolor.h"
#include "fvid.h"


// Constants used for exponential
// radial attenuation of light energy:
#define FLIGHT_ATTEN_K0			1.0f
#define FLIGHT_ATTEN_K1			0.0f		// Gets multiplied by (1.0f / LightRadius_WS) --- Set FLIGHT_ATTEN_NO_K1 to TRUE if this is 0.0f
#define FLIGHT_ATTEN_K2			10.0f		// Gets multiplied by (1.0f / (LightRadius_WS * LightRadius_WS))
#define FLIGHT_ATTEN_NO_K1		TRUE		// Set this to TRUE if FLIGHT_ATTEN_K1 is 0.0f


const s32 FLIGHT_NAME_LEN			= 15;	// Maximum number of characters in the light name (excluding the NULL terminator)
const s32 FLIGHT_TEXTURE_NAME_LEN	= 15;	// Maximum number of characters in the light texture name (excluding the NULL terminator)

class CFTexInst;
class CFMeshInst;

//These variables control how dynamic lighting is mapped from MAX->GAME

//Exposure, controls the intensity of the lighting, 1.0f = default.
extern f32 FLight_fExposure;
//This value adjusts the actual radius of point lights.
extern f32 FLight_fLightRadiusAdj;
//This is the intensity clamp for lights after applying the current exposure adjustment, 2.0 = default
extern f32 FLight_fItensClamp;
//Show the radius of lights, for debugging purposes.
extern BOOL FLight_bShowLightRange;

//Light intensity = Clamp(I * fExposure, 0.0f, fItensClamp);
//Light radius = fRadius * fLightRadiusAdj;
//

//
//
typedef enum 
{
	FLIGHT_TYPE_DIR = 0,		// Directional light
	FLIGHT_TYPE_OMNI,			// Omnidirectional light
	FLIGHT_TYPE_SPOT,			// Spot light
	FLIGHT_TYPE_AMBIENT,		// Ambient light

	FLIGHT_TYPE_COUNT,
	FLIGHT_TYPE_FORCE_DWORD
} FLightType_e;

//
//
enum 
{
	FLIGHT_FLAG_NONE					= 0x00000000,
	FLIGHT_FLAG_ENABLE					= 0x00000001,	// Enable this light
	FLIGHT_FLAG_HASDIR					= 0x00000002,	// Light has a directional component
	FLIGHT_FLAG_HASPOS					= 0x00000004,	// Light has a positional component

	FLIGHT_FLAG_LIGHT_ATTACHED			= 0x00000008,	// Light the mesh that this light is attached to (see CFLight::pOwnerMeshInst) (ignored for lights attached to the world)
	FLIGHT_FLAG_NOLIGHT_TERRAIN			= 0x00000010,	// Don't light the terrain (used for lights attached to both terrain and objects)
	FLIGHT_FLAG_DONT_LIGHT_UNATTACHED	= 0x00000020,	// Don't light meshes that this light isn't attached to

	FLIGHT_FLAG_PER_PIXEL				= 0x00000040,	// This light casts a per pixel projection on the environment (may or may not have a texture).  If it has
														// a texture, it will apply per pixel to all objects.  Otherwise it will only apply per pixel to those
														// objects that are flagged to be lit per pixel.

	FLIGHT_FLAG_MESH_MUST_BE_PER_PIXEL	= 0x00000080,	// For per-pixel lights that have a projected texture.  If this flag is set, only objects that are flagged
														// as per pixel lit will have the texture projected on them.  Others will just apply as a dynamic vertex light

	FLIGHT_FLAG_ENGINE_LIGHT			= 0x00000100,	// All lights that are used in the engine have this flag (they may also be lightmap lights)
	FLIGHT_FLAG_LIGHTMAP_LIGHT			= 0x00000200,	// This light is to be used for generating lightmaps (If it is not dynamic, it can be discarded prior to the engine)
	FLIGHT_FLAG_UNIQUE_LIGHTMAP			= 0x00000400,	// This light will generate its own unique lightmap in the lightmapping phase (it must also have a unique m_nLightID)

	FLIGHT_FLAG_CORONA					= 0x00000400,   // This light has a corona
	FLIGHT_FLAG_CORONA_PROXFADE			= 0x00000800,   // Fade the corona as the camera gets closer.
	FLIGHT_FLAG_CORONA_ONLY				= 0x00001000,	// Draw the corona only - don't actually cast a dynamic light
	
	FLIGHT_FLAG_CAST_SHADOWS			= 0x00002000,	// This light will cast shadows

	FLIGHT_FLAG_CORONA_WORLDSPACE		= 0x00004000,	// Corona's act like "glow" cards and stay constant in screenspace.

	FLIGHT_FLAG_GAMEPLAY_LIGHT			= 0x00008000,	// Flags this light as necessary for gameplay, so it has higher priority then non-gameplay lights.

	FLIGHT_FLAG_DYNAMIC_ONLY			= 0x40000000,	// This light will only light dynamic objects

	FLIGHT_FLAG_INCLUDE					= 0x80000000,	// Don't use this flag! (it's used internally by the mesh module)
};

//
//
struct FLightInit_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FLightInit_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	char szName[FLIGHT_NAME_LEN+1];	// ASCIIZ name of the light

	char szPerPixelTexName[FLIGHT_TEXTURE_NAME_LEN + 1]; // texture that is projected by this light.  MAKE SURE YOU NULL TERMINATE THIS, EVEN IF YOU DON"T WANT A TEXTURE
	char szCoronaTexName[FLIGHT_TEXTURE_NAME_LEN + 1]; // texture used to create the corona.  MAKE SURE YOU NULL TERMINATE THIS, EVEN IF YOU DON"T WANT A TEXTURE

	u32 nFlags;						// See FLIGHT_FLAG_* for info

	u16 nLightID;					// LIGHT ID SHOULD BE SET TO 0xffff, UNLESS BEING SET FROM WITHIN A TOOL
	u8  nType;						// Light type (see FLightType_e for info)
	s8  nParentBoneIdx;				// Index into the parent model's bone (-1 if there is no parent bone)

	f32 fIntensity;					// Light intensity to be multiplied by each component (0.0f to 1.0f)
	CFColorMotif Motif;				// Light color motif (RGBA components range from 0.0f to 1.0f). Alpha is not used.
	CFSphere spInfluence;			// Light position and radius in model space (ignored for directional lights)
	CFMtx43 mtxOrientation;			// Light orientation in model space (or world space if not attached to an object).  Direction (away from source) is in m_vFront (dir and spot)
	f32 fSpotInnerRadians;			// Spotlight inner full-angle in radians
	f32 fSpotOuterRadians;			// Spotlight outer full-angle in radians
	f32 fCoronaScale;

	// IF YOU ADD ANY FIELDS TO THIS STRUCTURE, YOU MUST UPDATE THIS FUNCTION.
	void ChangeEndian( void )
	{
		nLightID = fang_ConvertEndian( nLightID );
		nType = fang_ConvertEndian( nType );
		nParentBoneIdx = fang_ConvertEndian( nParentBoneIdx );
		nFlags = fang_ConvertEndian( nFlags );
		Motif.ChangeEndian();
		fIntensity = fang_ConvertEndian( fIntensity );
		spInfluence.ChangeEndian();
		mtxOrientation.ChangeEndian();
		fSpotInnerRadians = fang_ConvertEndian( fSpotInnerRadians );
		fSpotOuterRadians = fang_ConvertEndian( fSpotOuterRadians );
		fCoronaScale = fang_ConvertEndian( fCoronaScale );
	}
};


//
//
//
FCLASS_NOALIGN_PREFIX class CFLight 
{
	public:
		char m_szName[FLIGHT_NAME_LEN+1];	// ASCIIZ name of the light

		u32 m_nFlags;					// See FLIGHT_FLAG_* for info

		u16 m_nLightID;					// Assigned to some lights in Tools (Unassigned setting is 0xffff).
		u8  m_nNumShadows;
		u8  m_nIndex;					// Light index for global lookups.

		u8 m_nType;						// Light type (see FLightType_e for info)
		u8 m_nRayCastPhase;				// Used to determine when to cast rays for lights with corona's - and for visibility (0xff if visible set)
		u8 m_nMaxShadowCount;			// Maximum number of shadows this light can cast
		u8 __PAD;

		CFColorMotif m_Motif;			// Light color motif supplied by engine (RGBA components range from 0.0f to 1.0f). Alpha is not used.
		CFSphere m_spInfluence_WS;		// Light position and radius in world space (ignored for directional lights)
		CFMtx43 m_mtxOrientation_WS;	// Orientation of the light (m_fFront is the unit light direction (away from source) in world space (dir and spot)).
		f32 m_fSpotInnerRadians;		// Spotlight inner full-angle in radians
		f32 m_fSpotOuterRadians;		// Spotlight outer full-angle in radians
		CFMeshInst *m_pOwnerMeshInst;	// Points to the mesh instance that owns this light (used by the mesh system to prevent self-lighting) (NULL=none)
		CFMeshInst **m_papShadowMeshInst;
		CFSphere *m_paShadowBoundingSphere;
		f32 *m_pafShadowObjDist;

		CFTexInst *m_pProjectionTex;	// The texture which this light projects (If NULL, use the per pixel default)
		CFTexInst *m_pCoronaTex;		// The texture to be used as the corona (If NULL, use the default)
		CFTexInst *m_pShadowTex;		// The shadow map used for this light (If NULL, this light does not cast shadows).

		// Misc:
		f32 m_fOOR_WS;					// 1.0f/(Sphere_WS.fRadius)
		f32 m_fOOR2_WS;					// 1.0f/(Sphere_WS.fRadius * Spher_WS.fRadius)
		f32 m_fSpotK0;					// cos(fSpotInnerRadians/2)
		f32 m_fSpotK1;					// cos(fSpotOuterRadians/2)
		f32 m_fSpotK2;					// 1.0f/( cos(fSpotInnerRadians/2) - cos(fSpotOuterRadians/2) )
		
		#if FANG_PLATFORM_GC
		f32 m_fSpotGCK0;
		f32 m_fSpotGCK1;
		f32 m_fSpotGCK2;
		#endif

		// View space:
		// INTERNAL PLATFORM HARDWARE USE ONLY
		// VIEWSPACE COORDINATES MIGHT NOT BE LEFT-HANDED
		u32 m_nXfmKey_VS;				// Frame when light was last transformed into viewspace
		CFVec3 m_vPos_VS;				// Light position in view space (ignored for directional lights)
		CFVec3 m_vUnitDir_VS;			// Unit direction in view space (dir and spot).

		// Color:
		u32 m_nColorCalcKey;			// Value of FVid_nFrameCounter when ScaledColor and fSpecIntensity were last computed
		CFColorRGBA m_ScaledColor;		// Actual color of light, after intensity and motif. Alpha is not used.

		// Other Corona data
		f32 m_fCoronaScale;
		f32 m_fFade;					// Multiplier for fading in/out coronas
		f32 m_fFadeDelta;				// Fade delta (per second), used to determine fade "direction" and speed.
		f32 m_fDeltaScale;				// Used to scale the rate of speed at which coronas are scaled in world space, 1.0 = default.
		
	public:

		CFLight( void )
		{
			fang_MemZero( this, sizeof( CFLight ) );
			m_nLightID = 0xffff;
		}

		~CFLight( void );

		void Init( const FLightInit_t *pLightInit );
		BOOL ComputeModelSpace( BOOL bModelSpaceIsWorldSpace );
		FINLINE BOOL ComputeColor( void );

		void LightDirectionalPoint( const CFVec3A *pPoint_WS, const CFVec3A *pUnitNormal_WS, CFColorRGB *pDestColorRGB );
		void LightPoint( const CFVec3A *pPoint_WS, CFColorRGB *pDestColorRGB );

		void InitOmniLight( f32 fPosX_WS, f32 fPosY_WS, f32 fPosZ_WS, f32 fRadius_WS );
		void InitOmniLight( const CFVec3 *pPos_WS, f32 fRadius_WS );
		void InitOmniLight( const CFSphere *pSphere_WS );

		void InitDirLight( f32 fDirX_WS, f32 fDirY_WS, f32 fDirZ_WS );
		void InitDirLight( const CFVec3 *pvDir_WS );

		void InitSpotLight( f32 fPosX_WS, f32 fPosY_WS, f32 fPosZ_WS, f32 fRadius_WS, 
							f32 fDirX_WS, f32 fDirY_WS, f32 fDirZ_WS, f32 fInnerRadians, f32 fOuterRadians );
		void InitSpotLight( const CFVec3 *pPos_WS, f32 fRadius_WS, const CFVec3 *pDir_WS, f32 fInnerRadians, f32 fOuterRadians );
		void InitSpotLight( const CFSphere *pSphere_WS, const CFVec3 *pDir_WS, f32 fInnerRadians, f32 fOuterRadians );
		void InitSpotLight( const CFSphere *pSphere_WS, const CFMtx43 *pOrientation_WS, f32 fInnerRadians, f32 fOuterRadians );

		void SetName( cchar *pszName );

		void SetIntensity( f32 fUnitIntensity );
		f32 GetIntensity( void ) const;
		void SetColor( f32 fUnitRed, f32 fUnitGreen, f32 fUnitBlue, f32 fUnitAlpha = 1.0f );
		void SetColor( const CFColorRGBA *pColor );
		void SetColor( const CFColorRGB *pColor );
		void SetMotif( const CFColorMotif *pMotif );
		void SetMotif( u32 nMotifIndex );

		void SetPosition( f32 fPosX_WS, f32 fPosY_WS, f32 fPosZ_WS );
		void SetPosition( const CFVec3A *pPos_WS );
		void SetPosition( const CFSphere *pSphere_WS );

		void SetRadius( f32 fRadius_WS );
		void SetRadius( const CFSphere *pSphere_WS );

		void SetPositionAndRadius( f32 fPosX_WS, f32 fPosY_WS, f32 fPosZ_WS, f32 fRadius_WS );
		void SetPositionAndRadius( const CFVec3A *pPos_WS, f32 fRadius_WS );
		void SetPositionAndRadius( const CFSphere *pSphere );

		void SetDirection( f32 fDirX_WS, f32 fDirY_WS, f32 fDirZ_WS );
		void SetDirection( const CFVec3A *pDir_WS );

		void SetSpotCone( f32 fInnerRadians, f32 fOuterRadians );

		BOOL AllocAndSetPerPixelTexture( const char *pszTextureName );
		BOOL AllocAndSetCoronaTexture( const char *pszTextureName );

		void Enable( BOOL bEnable );

		BOOL DoesSpotConeIntersectSphere( const CFSphere *pSphere ) const;
		f32  GetWeight( const CFSphere *pSphere );

		// For rendering coronas
		void RenderCorona(const CFMtx43A *pCamera, float fCamDistSq, u32 nPhase);
		
		BOOL SetMaxShadowCount( u32 nShadowCount );

		void RenderRange();

	private:

		BOOL LightPointWithAmbientLight( const CFVec3A *pPoint_WS, CFColorRGB *pDestColorRGB );
		BOOL LightPointWithDirLight( const CFVec3A *pPoint_WS, CFColorRGB *pDestColorRGB );
		BOOL LightPointWithOmniLight( const CFVec3A *pPoint_WS, CFColorRGB *pDestColorRGB, f32 fRadius = 0.f );
		BOOL LightPointWithSpotLight( const CFVec3A *pPoint_WS, CFColorRGB *pDestColorRGB, f32 fRadius = 0.f );

		BOOL LightDirectionalPointWithDirLight( const CFVec3A *pPoint_WS, const CFVec3A *pUnitNormal_WS, CFColorRGB *pDestColorRGB );
		BOOL LightDirectionalPointWithOmniLight( const CFVec3A *pPoint_WS, const CFVec3A *pUnitNormal_WS, CFColorRGB *pDestColorRGB );
		BOOL LightDirectionalPointWithSpotLight( const CFVec3A *pPoint_WS, const CFVec3A *pUnitNormal_WS, CFColorRGB *pDestColorRGB );

		f32 m_fIntensity;				// Light intensity to be multiplied by each component (0.0f to 1.0f)


	FCLASS_STACKMEM_NOALIGN( CFLight );
} FCLASS_NOALIGN_SUFFIX;

//
//
FINLINE BOOL CFLight::ComputeColor( void ) 
{
	if ( m_nColorCalcKey == FVid_nFrameCounter ) 
	{
		return FALSE;
	}
	
	m_nColorCalcKey = FVid_nFrameCounter;

	m_Motif.ComputeColor( &m_ScaledColor );
	m_ScaledColor.ColorRGB *= m_fIntensity;
	m_ScaledColor.fAlpha = 1.0f;

	FCOLOR_FASSERT_UNIT_RGBA( &m_ScaledColor );

//	m_fSpecIntensity = m_ScaledColor.CalcIntensity();

	return TRUE;
}






//---------------------------------------------------------------------------------------------------------------------
// flight_ModuleStartup()
// flight_ModuleShutdown()
//
// Starts up and shuts down this module, respectively.
// Called by the Fang initialization system.
// Returns TRUE if successful, or FALSE if the module could not successfully initialized.
extern BOOL flight_ModuleStartup( void );
extern void flight_ModuleShutdown( void );

extern u8 flight_GetPhase( void );


//
//
//
FINLINE void flight_BuildLightMatrixFromDirection( CFMtx43 *pMtx, f32 fX, f32 fY, f32 fZ )
{
	pMtx->m_vFront.x = fX;
	pMtx->m_vFront.y = fY;
	pMtx->m_vFront.z = fZ;
	pMtx->m_vFront.Unitize();

	if ( fmath_Abs( pMtx->m_vFront.y) > 0.999f )
	{
		pMtx->m_vRight = CFVec3::m_UnitAxisX;
		pMtx->m_vUp = -CFVec3::m_UnitAxisZ;
	}
	else
	{
		pMtx->m_vRight = CFVec3::m_UnitAxisY.Cross( pMtx->m_vFront );
		pMtx->m_vRight.Unitize();
		pMtx->m_vUp = pMtx->m_vFront.Cross( pMtx->m_vRight );
	}
	pMtx->m_vPos.Set( 0.f, 0.f, 0.f );
}


//---------------------------------------------------------------------------------------------------------------------
// flight_SetIntensity()
// flight_SetColor()
// flight_SetColorAndIntensity()
// flight_SetMotif()
// flight_SetPosition()
// flight_SetDirection()
// flight_SetRadius()
// flight_SetRadialAttenuation()
// flight_SetSpotFocusExp()
// flight_SetUserId()
// flight_TurnOn()
// flight_ChangeUserDataPointer()
//
// The app should never modify the data within the light structure directly. Instead, use
// these functions to modify attributes of existing lights.





#endif

